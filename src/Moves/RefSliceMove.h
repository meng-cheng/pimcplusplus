#ifndef REF_SLICE_MOVE_H
#define REF_SLICE_MOVE_H

#include "MoveBase.h"
#include "../PathDataClass.h"
#include "PermuteStage.h"
#include "BisectionStage.h"

/// This move, inherited from ParticleMoveClass, performs a set of
/// bisection stages over a set of time slices which contains the
/// reference slice.  The processor which owns the reference slice is
/// the only one that actually moves particles, while the other
/// processors simply compute the change in the nodal action.
/// Explanation of how bisection moves work is in  
/// Path Integrals in the theory of condensed helium
/// by D.M. Ceperley  (Review of Modern Physics 1995) section V.H
class RefSliceMoveClass : public MultiStageClass
{
private:
  /// Number of bisection stage levels
  int NumLevels;

  int NodeAccept, NodeReject;

  /// Holds the current master processor
  int MasterProc;

  /// The species this particular instance is working on
  int SpeciesNum;



  /// This function checks to see if we should accept based on the
  /// change in the node action
  bool NodeCheck();

  /// This function is called if I have the reference slice
  void MakeMoveMaster();

  /// This move is called if I don't have the reference slice
  void MakeMoveSlave();

public:
  /// Read in the parameters this class needs from the input file.
  void Read(IOSectionClass &in);
  void WriteRatio();  

  /// Override base class MakeMove to do a block of moves
  void MakeMove();

  RefSliceMoveClass(PathDataClass &pathData, IOSectionClass &out) : 
    MultiStageClass(pathData, out)
  { 
    NodeAccept = NodeReject = 0;
    // do nothing for now
  }
};



#endif
