#include "RefSliceMove.h"

void RefSliceMoveClass::Read(IOSectionClass &in)
{
  string permuteType, speciesName;
  StageClass *permuteStage;
  assert (in.ReadVar ("NumLevels", NumLevels));
  assert (in.ReadVar ("Species", speciesName));
  assert (in.ReadVar ("name", Name));
  SpeciesNum = PathData.Path.SpeciesNum (speciesName);

  /// Set up permutation
  assert (in.ReadVar ("PermuteType", permuteType));
  if (permuteType == "TABLE") 
    permuteStage = new TablePermuteStageClass (PathData,SpeciesNum,NumLevels);
  else if (permuteType == "NONE") 
    permuteStage = new NoPermuteStageClass(PathData, SpeciesNum, NumLevels);
  else {
    cerr << "Unrecognized PermuteType, """ << permuteType << """\n";
    exit(EXIT_FAILURE);
  }
  permuteStage->Read (in);
  Stages.push_back (permuteStage);
  
  for (int level=NumLevels-1; level>=0; level--) {
    BisectionStageClass *newStage = new BisectionStageClass (PathData, level);
    newStage->Actions.push_back(&PathData.Actions.ShortRange);
    //newStage->Actions.push_back(&PathData.Actions.LongRange);
    newStage->Actions.push_back(&PathData.Actions.Kinetic);
    newStage->BisectionLevel = level;
    Stages.push_back (newStage);
  }

  // Add the second stage of the permutation step
  Stages.push_back (permuteStage);
}


/// This version is for the processor with the reference slice
void RefSliceMoveClass::MakeMoveMaster()
{
  PathClass &Path = PathData.Path;
  int myProc = PathData.Communicator.MyProc();

  int firstSlice, lastSlice;
  Path.SliceRange (myProc, firstSlice, lastSlice);
  // Choose time slices
  int localRef = Path.GetRefSlice() - firstSlice;
  int bisectSlices = (1 << NumLevels);
  int minSlice = max(0, localRef - (bisectSlices>>1));
  int maxSlice = min (Path.NumTimeSlices()-1, localRef + (bisectSlices>>1));
  int slice1 = Path.Random.LocalInt(minSlice-maxSlice) + minSlice;
  int slice2 = slice1 + bisectSlices;

  // Go through local stages
  bool toAccept=true;
  list<StageClass*>::iterator stageIter=Stages.begin();
  double prevActionChange=0.0;
  while (stageIter!=Stages.end() && toAccept){
    toAccept = (*stageIter)->Attempt(Slice1,Slice2,
				     ActiveParticles,prevActionChange);
    stageIter++;
  }
    // Broadcast acceptance or rejection 
  int accept = toAccept ? 1 : 0;
  PathData.Communicator.BroadCast (myProc, accept);

  // Now, if we accept local stages, move on to global nodal
  // decision. 
  if (toAccept) {
    // Broadcast the new reference path to all the other processors
    PathData.Path.BroadcastRefPath();

    // Calculate local nodal action
    double localNodeAction = 
      PathData.Actions.NodalActions(SpeciesNum)->Action
      (0, Path.NumTimeSlices()-1, ActiveParticles, 0);
  }
  // Otherwise, reject the whole move
  else 
    Reject();

}


/// This version is for processors that do no own the reference slice 
void RefSliceMoveClass::MakeMoveSlave()
{
  /// Choose time slices for local bisections

  /// Choose local permuation
  
  /// Bisect locally

  /// Receive broadcast from Master.

  /// 

  /// If master has accepted, discard my local bisection


}


void RefSliceMoveClass::MakeMove()
{
  


}
