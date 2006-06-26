#include "IonMoveStage.h"
#include "MoveUtils.h"

IonStageClass::IonStageClass(PathDataClass &pathData, int level, IOSectionClass outSection, string SpeciesName) : 
    LocalStageClass(pathData,outSection)
{ 
  //cerr << "in IonStage constructor" << endl;
  // Load ptcl ids belonging to the given species
  specID = PathData.Path.SpeciesNum(SpeciesName);
  //cerr << "specid " << specID << " corresponds to " << SpeciesName << endl;

  int NumPtclsInSpecies =  PathData.Path.Species(specID).LastPtcl - PathData.Path.Species(specID).FirstPtcl + 1;
  PtclRoster.resize(NumPtclsInSpecies);
  int index = 0;
  for(int i=0; i<PathData.Path.NumParticles(); i++){
    if(PathData.Path.ParticleSpeciesNum(i) == specID){
      PtclRoster(index) = i;
      index++;
    }
  }
  //cerr << "PtclRoster is " << PtclRoster << endl;

  BisectionLevel = level;
  //cerr << "leaving constructor" << endl;
}

void IonStageClass::Read(IOSectionClass& in){
  double setMag; 
  assert (in.ReadVar ("SetMoveMag", setMag));
  Set(setMag);
  string setAction;

  Array<string,1> ActionList;
  assert (in.ReadVar ("Actions", ActionList));
	for(int a=0; a<ActionList.size(); a++){
		string setAction = ActionList(a);
		#ifdef USE_QMC
	  if(setAction == "CEIMCAction"){
  		Actions.push_back(&PathData.Actions.CEIMCAction);
			cerr << "Added CEIMCAction" << endl;
			list<ActionBaseClass*>::const_iterator act = Actions.end();
			act--;
			(*act)->Read(in);
		}else 
		#endif
		if(setAction == "IonInteraction"){
  		Actions.push_back(&PathData.Actions.IonInteraction);
			cerr << "Added IonInteraction" << endl;
		} else
    	cerr << "You specified " << setAction << ", which is not supported for this type of move" << endl;
	}
}

void IonStageClass::WriteRatio()
{ 
  AcceptRatioVar.Write((double)NumAccepted/(double)NumAttempted);
  AcceptRatioVar.Flush();
}

/*
void IonStageClass::Accept()
{
  //do nothing for now
  
}

void IonStageClass::Reject()
{
  //do nothing for now

}
*/

void IonStageClass::Set(double setdRMag){
  dRMag = setdRMag;
}

// very simple translations of activeParticles
double IonStageClass::Sample(int &slice1,int &slice2, Array<int,1>& activeParticles)
{
  activeParticles.resize(PtclRoster.size());
  for(int i=0; i<PtclRoster.size(); i++) activeParticles(i) = PtclRoster(i);
		//cerr << "  IonStage::Sample... PtclRoster is " << PtclRoster << endl;
		//cerr << "  IonStage::Sample... activePtcls is " << activeParticles << endl;
  // for now this is strictly classical
  slice1 = 0;
  slice2 = 1;
  int slice = slice1;
  for(int slice=slice1; slice<slice2; slice++){
    for(int i=0; i<activeParticles.size(); i++){
			//cerr << "  Move: ptcl " << activeParticles(i) << " to ";
      dVec dR;
      for(int x=0; x<3; x++) dR(x) = (PathData.Path.Random.Local() - 0.5);
      dR = Scale(dR,dRMag);
      dR += PathData.Path(slice,activeParticles(i));
			//cerr << dR << endl;
      PathData.Path.SetPos(slice,activeParticles(i),dR);
    }
  }
  return 1; // transition probability is constant inside the box of length dRMag
}

bool IonStageClass::Attempt(int &slice1, int &slice2, Array<int,1> &activeParticles, double &prevActionChange)
{
  SetMode (NEWMODE);
  double sampleRatio=Sample(slice1,slice2,activeParticles);
  SetMode(OLDMODE);
  double oldAction=StageAction(slice1,slice2,activeParticles);
  SetMode(NEWMODE);
  double newAction =StageAction(slice1,slice2,activeParticles);
  double currActionChange=newAction-oldAction;
  double logAcceptProb=log(sampleRatio)-currActionChange+prevActionChange;
  bool toAccept = logAcceptProb>=log(PathData.Path.Random.Local()); /// Accept condition
  if (toAccept){
    NumAccepted++;
	}
  NumAttempted++;
  prevActionChange=currActionChange;

  return toAccept;
}