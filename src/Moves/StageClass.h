#ifndef STAGE_H
#define STAGE_H




#include <list>
#include "../Observables/ObservableBase.h"
#include "../Actions/ActionBase.h"
 
///Some moves are built out of stages.  This class is the parent class
///for a stage.  If it is the first stage it will typically need to
///set the slices adn the activeparticles for the other stages to
///perform correctly. Each stage has a list of actions that lets it
///calculate its total action by summing the value of each action in
///its list.
class StageClass
{
protected:
  PathDataClass &PathData;
  IOSectionClass OutSection;
  ObservableDouble AcceptRatioVar;
public:
  int NumAccepted, NumAttempted;
  int BisectionLevel;
  list<ActionBaseClass*> Actions;

  ///The first stage will set the slices and activeParticles
  ///This returns transition probability ratio T(new->old)/T(old->new)
  virtual double Sample (int &slice1,int &slice2,
			 Array<int,1> &activeParticles) = 0; 
  virtual void Read (IOSectionClass &in);
  virtual void Accept();
  virtual void Reject();
  virtual void WriteRatio();
  inline double StageAction(int startSlice,int endSlice,
			    const Array<int,1> &changedParticles);
  inline double GlobalStageAction (const Array<int,1> &changeParticles);
  inline double AcceptRatio () 
  { return (double)NumAccepted / (double) NumAttempted; }
  
  virtual bool Attempt(int &slice1, int &slice2, 
		       Array<int,1> &activeParticles,
		       double &prevActionChange) = 0;


  StageClass(PathDataClass &pathData,IOSectionClass outSection) :
    PathData(pathData), NumAccepted(0), NumAttempted(0),
    BisectionLevel(0),
    OutSection(outSection),
    AcceptRatioVar("AcceptRatio",OutSection,pathData.Path.Communicator)
  {
    if (PathData.Path.Communicator.MyProc()==0)
      OutSection.NewSection("Stage");
  }
};

class CommonStageClass : public StageClass
{
public:
  bool Attempt(int &slice1, int &slice2,
	       Array<int,1> &activeParticles,
	       double &prevActionChange);
  CommonStageClass(PathDataClass &pathData,IOSectionClass &outSection) :
    StageClass(pathData,outSection)
  {
    // Do nothing for now
  }
	       
};

class LocalStageClass : public StageClass
{
public:
  bool Attempt(int &slice1, int &slice2,
	       Array<int,1> &activeParticles,
	       double &prevActionChange);
  LocalStageClass(PathDataClass &pathData,IOSectionClass &outSection) :
    StageClass(pathData,outSection)
  {

  }
	       
};


inline double StageClass::StageAction(int startSlice,int endSlice,
				      const Array<int,1> &changedParticles)
{
  double TotalAction=0.0;
  list<ActionBaseClass*>::iterator actionIter=Actions.begin();
  while (actionIter!=Actions.end()){
    TotalAction += 
      ((*actionIter)->Action(startSlice, endSlice, changedParticles,
			     BisectionLevel));
    actionIter++;
  }
  return TotalAction;
}

inline double 
StageClass::GlobalStageAction (const Array<int,1> &changedParticles)
{
  int slice1 = 0;
  int slice2 = PathData.Path.NumTimeSlices()-1;
  double localAction = StageAction (slice1, slice2, changedParticles);
  double globalAction = PathData.Path.Communicator.AllSum (localAction);

  return globalAction;
}



#endif