#ifndef ACTIONS_CLASS_H
#define ACTIONS_CLASS_H
#include "ShortRangeClass.h"
#include "LongRangeClass.h"
#include "LongRangeRPAClass.h"
#include "ShortRangePotClass.h"
#include "LongRangePotClass.h"
#include "KineticClass.h"
#include "NodalActionClass.h"

/// ActionsClass is a shell of a class holding all of the necessary
/// ActionBaseClass derivatives representing the different actions.
/// It includes kinetic, short range, long range, long range RPA
/// version, and nodal actions.  It may later contain importance
/// sampling "actions" as well.
class ActionsClass
{
private:
  Array<PairActionFitClass*,1> PairArray;
  Array<PairActionFitClass*,2> PairMatrix;
  PathDataClass &PathData;
  int MaxLevels; //is this the right place for this?
public:
  // Actions

  /// The Kinetic action
  KineticClass Kinetic;

  /// The short range part of the pair action.  This is the complete
  /// pair action in the case of short-range potententials.  The
  /// short-range action is summed in real space. 
  ShortRangeClass ShortRange;

  /// The long range part of the action, which is summed in k-space.  
  LongRangeClass LongRange;

  /// The Random Phase Approximation-corrected form of the above.
  LongRangeRPAClass LongRangeRPA;

  /// This array of actions are used for Restricted PIMC for
  /// fermions.  These effective actions ensure that the paths do not
  /// cross the nodes of some trial density matrix with respective to
  /// the reference slice.
  Array<ActionBaseClass *,1> NodalActions;
  //DiagonalClass Diagonal;
  //ImportanceSampleClass ImportanceSample;

  // Potentials
  ShortRangePotClass ShortRangePot;
  LongRangePotClass  LongRangePot;
  
  /// Stores whether we use Random Phase Approximation corrections to
  /// the long range action.
  bool UseRPA;

  /// Read the action parameters from the input file and do the
  /// necessary initialization.  This reads the pair actions, and does
  /// long range breakups and RPA corrections if necessary.
  void Read(IOSectionClass &in);
  ActionsClass(PathDataClass &pathData) : 
    ShortRange(pathData,PairMatrix),
    ShortRangePot(pathData, PairMatrix),
    LongRange(pathData,PairMatrix,PairArray), 
    LongRangeRPA(pathData, PairMatrix, PairArray),
    LongRangePot(pathData, PairMatrix),
    Kinetic(pathData),
    PathData(pathData)
  {
    ///Do nothing for now
  }





};

#endif
