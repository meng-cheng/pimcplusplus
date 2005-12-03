#ifndef CORRELATED_BISECTION_BLOCK_CLASS_H
#define CORRELATED_BISECTION_BLOCK_CLASS_H


#include "MoveBase.h"
#include "../PathDataClass.h"
#include "PermuteStage.h"
#include "CoupledPermuteStage.h"
#include "BisectionStage.h"
#include "../Observables/ObservableVar.h"

/// This is the bisection move class inherited from ParticleMoveClass
/// Explanation of how bisection moves work is in  
/// Path Integrals in the theory of condensed helium
/// by D.M. Ceperley  (Review of Modern Physics 1995) section V.H
class CorrelatedBisectionBlockClass : public MultiStageClass
{

private:
  int StepNum;
  int NumLevels;
  int StepsPerBlock;
  bool HaveRefslice;
  int SpeciesNum;
  void ChooseTimeSlices();
  PermuteStageClass* PermuteStage;
  Array<BisectionStageClass*,1> BisectionStages;
  list<ActionBaseClass*> BosonActions;
  list<ActionBaseClass*> NodalActions;
  
  FILE *EAout, *EBout, *SAout, *SBout;
  ObservableDouble wAEAvar, wBEBvar, wAvar, wBvar;
  Array<double,1> AllSumVecIn, AllSumVecOut;
  //  ObservableDouble AcceptanceRatioVar;
public:
  /// Number of levels the bisection move works on 
  void Read(IOSectionClass &in);

  /// Override base class MakeMove to do a block of moves
  void MakeMove();

  CorrelatedBisectionBlockClass(PathDataClass &pathData, IOSectionClass &out) : 
    MultiStageClass(pathData, out),StepNum(0), 
    wAEAvar("wAEA", IOSection, pathData.Path.Communicator),
    wBEBvar("wBEB", IOSection, pathData.Path.Communicator),
    wAvar  ("wA",   IOSection, pathData.Path.Communicator),
    wBvar  ("wB",   IOSection, pathData.Path.Communicator),
    AllSumVecIn(2), AllSumVecOut(2)
  { 
    EAout = fopen ("EA.dat", "w");
    SAout = fopen ("SA.dat", "w");
    EBout = fopen ("EB.dat", "w");
    SBout = fopen ("SB.dat", "w");
    // do nothing for now
  }
};


#endif
