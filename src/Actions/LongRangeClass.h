#ifndef LONG_RANGE_CLASS_H
#define LONG_RANGE_CLASS_H

#include "ActionBase.h"
#include "../Common/PairAction/PAFit.h"

typedef enum {JOB_U, JOB_DU, JOB_V} JobType; 

/// The LongRangeClass is an action class responsible for the long
/// wavelength components of the action that are summed in k-space.
/// This class has member functions which perform the optimized
/// short range/long range breakup of the action and its
/// beta-derivative.  This is based on modified version of the method
/// by Natoli and Ceperley (J. Comp. Phys. 117, 171-178 [1995]).
class LongRangeClass : public ActionBaseClass
{
protected:
  Array<PairActionFitClass*,2> &PairMatrix;
  Array<PairActionFitClass*,1> &PairArray;
  LinearGrid LongGrid;
  void OptimizedBreakup_U(int numKnots);
  void OptimizedBreakup_dU(int numKnots);
  void OptimizedBreakup_V(int numKnots);


  /// This calculates the quantity 
  /// \f$ X_k \equiv -\frac{4\pi}{\Omega k} \int_{r_c}^\infty dr \, r \sin(kr) V(r).\f$
  double CalcXk (int paIndex, int level, double k, double rc, JobType type);
  // This must be called after all of the OptimizedBreakup_x's

  int Level, ki;
public:
  void Init(IOSectionClass &in);
  void Read (IOSectionClass &in);
  double Action (int slice1, int slice2, 
		   const Array<int,1> &activeParticles, int level);
  double d_dBeta (int slice1, int slice2,  int level);
  LongRangeClass (PathDataClass &pathData,
		   Array<PairActionFitClass*, 2> &pairMatrix,
		  Array<PairActionFitClass*, 1> &pairArray);

};

#endif
