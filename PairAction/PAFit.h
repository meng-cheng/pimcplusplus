#ifndef PA_FIT_H
#define PA_FIT_H

//#include "PAszFit.h"
//#include "PAcoulombFit.h"
#include "PAcoulombBCFit.h"
#include "PAclassicalFit.h"
//#include "PAsFit.h"
//#include "PAtricubicFit.h"
//#include "PAzeroFit.h"
//#include "DavidPAClass.h"

inline PairActionFitClass *ReadPAFit (IOSectionClass &in, 
				      double smallestBeta, int numLevels)
{
  assert (in.OpenSection("Fits"));
  string type;
  assert (in.ReadVar("Type", type));
  in.CloseSection (); // "Fits"
  PairActionFitClass *fit;
  //  if (type == "szfit")
  //  fit = new PAszFitClass;
  //else if (type == "coulombfit")
  //  fit = new PAcoulombFitClass;
  /*else*/ if (type == "coulombBCfit")
    fit = new PAcoulombBCFitClass;
  else if (type == "classical")
    fit = new PAclassicalFitClass;
  //else if (type == "sfit")
  //  fit = new PAsFitClass;
  //else if (type == "tricubicfit")
  //  fit = new PAtricubicFitClass;
  //else if (type == "zerofit")
  //  fit=new PAzeroFitClass;
  //else if (type=="DavidFit")
  //  fit = new DavidPAClass;
  else {
    cerr << "Unrecognize pair action fit type \"" 
	 << type << "\".  Exitting.\n";
    exit(1);
  }
  fit->Read(in, smallestBeta, numLevels);
  return (fit);
}


#endif
