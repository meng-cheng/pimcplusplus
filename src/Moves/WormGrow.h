#ifndef WORM_GROW_MOVE_H
#define WORM_GROW_MOVE_H

#include "MultiStage.h"
#include "../PathDataClass.h"


/// This stage attempts to displace a list of whole paths.  It should
/// only be used for non-permuting particles.
class WormGrowStageClass : public LocalStageClass
{
public:
  int MaxSlicesToGrow;
  /// This is the width of the gaussian distribution for the
  /// displacement vector.
  double Sigma;
  int PathHead;

  /// This does the actual displacement of the path.  All processors
  /// within a single close must displace by the same amount.
  double Sample (int &slice1, int &slice2,
		 Array <int,1> &activeParticles);
  WormGrowStageClass (PathDataClass &pathData,IOSectionClass &outSection) :
    LocalStageClass (pathData,outSection) 
  {
    MaxSlicesToGrow=10;
    PathData.Path.HeadParticle=0;
    // Do nothing for now.
  }
};



/// This is the displace move, which attempts to pick up a whole
/// single-particle path and displace it by a random amount.
class WormGrowMoveClass : public MultiStageClass
{
private:
  /// This is the standard distribution of the displacement gaussian
  double Sigma;
  WormGrowStageClass WormGrowStage;
public:
  // Read the parameters from the input file
  void Read (IOSectionClass &in);
  // Actually attempts the move and accepts or rejects
  //  void MakeMove();
  WormGrowMoveClass (PathDataClass &pathData, IOSectionClass &outSection) :
    MultiStageClass(pathData, outSection),
    WormGrowStage(pathData, outSection)
  {
    DumpFreq = 20;
  }
};


#endif