#include "PathDataClass.h"
#include "BisectionMoveClass.h"
#include "Common.h"





void ShiftMove::MakeMove()
{//Remember to mark Actions dirty!!!
  //int numTimeSlicesToShift=(int)floor(sprng()*PathData->NumTimeSlices);
  int numTimeSlicesToShift = 5;

  if (numTimeSlicesToShift > 0){
    for (int counter=0;counter<PathData.NumSpecies();counter++){
      PathData(counter).MoveJoin(1);
      PathData(counter).ShiftData(numTimeSlicesToShift,PathData.Communicator);
    }
  }
  else if (numTimeSlicesToShift<=0){
    for (int counter=0;counter<PathData.NumSpecies();counter++){
      PathData(counter).MoveJoin(PathData.NumTimeSlices()-2); //< -1 so you don't overflow and -1 again because you 
                                                              //don't actually want to be at the entire end
      PathData(counter).ShiftData(numTimeSlicesToShift,PathData.Communicator);
    }
  }
    
}

    

void BisectionMoveClass::MakeMove()
{
  bool toAccept=true;
  double oldLogSampleProb;
  double newLogSampleProb;
  Array<int,1> theParticles;
  int EndTimeSlice=(1<<NumLevels)+StartTimeSlice;
  double prevActionChange=0;
  
  //  cerr<<"At the beginning fo the makeMove the size is ";
  //  cerr <<  PathData->SpeciesArray.size()<<endl;
  ChooseParticles();   
  //////////  for (int levelCounter=NumLevels;levelCounter>0;levelCounter--){
  int levelCounter=NumLevels-1;

  while (levelCounter>=0 && toAccept==true){
    // cerr<<"At the level Counter in Bisection makeMove being  "
    // <<levelCounter<<" the size is ";
    //    cerr <<  PathData->SpeciesArray.size()<<endl;
    SetMode(OLDMODE);
    toAccept=true;
    double oldAction = PathData.Action.calcTotalAction
      (ActiveParticles,StartTimeSlice,EndTimeSlice,levelCounter);
    oldLogSampleProb = PathData.Action.LogSampleProb
      (ActiveParticles,StartTimeSlice,EndTimeSlice,levelCounter);
    SetMode(NEWMODE);
    newLogSampleProb = PathData.Action.SampleParticles
      (ActiveParticles,StartTimeSlice,EndTimeSlice,levelCounter);
    //cerr << "newLogSampleProb = " << newLogSampleProb << endl;
    //cerr << "oldLogSampleProb = " << oldLogSampleProb << endl;
    double newAction = PathData.Action.calcTotalAction
      (ActiveParticles,StartTimeSlice,EndTimeSlice, levelCounter);
    double currActionChange=newAction-oldAction;
    double logAcceptProb=
      -oldLogSampleProb+newLogSampleProb+currActionChange-prevActionChange;
    //cerr << "prevActionChange = " << prevActionChange << endl;
    //cerr << "logAcceptProb = " << logAcceptProb << endl;
    //cerr<<"My new action is "<<newAction<<" and my old action was "<<oldAction<<endl;
    if (-logAcceptProb<log(sprng())){///reject conditin
      toAccept=false;
      //      break;

    }

    prevActionChange=currActionChange;
    levelCounter--;
  }
  if (toAccept ==true ){
    PathData.AcceptMove(ActiveParticles,StartTimeSlice,EndTimeSlice);
    //cout<<"I'm accepting! I'm accepting!"<<endl;
  }
  else {
    PathData.RejectMove(ActiveParticles,StartTimeSlice,EndTimeSlice);
    //  cout<<"I'm rejecting! I'm rejecting!"<<endl;
  }
    

}
