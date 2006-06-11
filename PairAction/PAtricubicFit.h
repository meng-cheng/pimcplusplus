/////////////////////////////////////////////////////////////
// Copyright (C) 2003-2006 Bryan Clark and Kenneth Esler   //
//                                                         //
// This program is free software; you can redistribute it  //
// and/or modify it under the terms of the GNU General     //
// Public License as published by the Free Software        //
// Foundation; either version 2 of the License, or         //
// (at your option) any later version.  This program is    //
// distributed in the hope that it will be useful, but     //
// WITHOUT ANY WARRANTY; without even the implied warranty //
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. //  
// See the GNU General Public License for more details.    //
// For more information, please see the PIMC++ Home Page:  //
//           http://pathintegrals.info                     //
/////////////////////////////////////////////////////////////

#ifndef PA_TRICUBIC_FIT_H
#define PA_TRICUBIC_FIT_H
#include "PAFitBase.h"
#include "../Splines/MyTricubicSpline.h"
#include "../MPI/Communication.h"

class PAtricubicFitClass : public PairActionFitClass
{
private:
  bool GridsAreMine;
  CommunicatorClass Comm;

  /** These arrays hold the coefficients of the expansion of the tail
      of the diagonal part of the action/potential in inverse powers
      of q or r.  The first index is for the inverse power, and the
      second gives the level.  That is, outside rcut, 
      \f[ U_\text{diag}(q, \text{level}) \approx Ucoef(0,level)/q +
      Ucoef(1,level)/r^2 + Ucoef(2,leve)/r^3 \f] */
  Array<double,2> Ucoefs, dUcoefs;
  Array<double,1> Vcoefs;
  /// The cutoff radius for the long range breakup.
  double rcut;  
public:
  Grid *qgrid, *ygrid, *tgrid;
  Array<MyTricubicSpline,1> Usplines, dUsplines;
  Array<double,1> sMax, sMaxInv;

  void ReadParams  (IOSectionClass &inSection);
  void WriteBetaIndependentInfo (IOSectionClass &outSection);
  /// Returns weighter RMS error
  void Error (Rho &rho, double &Uerror, double &dUerror);
  void DoFit (Rho &rho);
  void WriteFit(IOSectionClass &outSection);

  void Write (IOSectionClass &outSection);
  bool Read  (IOSectionClass &inSection, double lowestBeta,
	      int NumBetas);
  double U(double q, double z, double s2, int level);
  double dU(double q, double z, double s2, int level);
  /// The diagonal action only -- used for long-range breakup
  double Udiag(double q, int level);
  /// The q-derivative of the above
  double Udiag_p(double q, int level);
  /// The q-derivative of the above
  double Udiag_pp(double q, int level);
  /// The beta-derivative of the diagonal action
  double dUdiag    (double q, int level);
  /// The q-derivative of the above
  double dUdiag_p  (double q, int level);
  /// The q-derivative of the above
  double dUdiag_pp (double q, int level);
  /// These derivatives are needed to the gradient with respect to ion
  /// position.  
  void Derivs (double q, double z, double s2, int level,
	       double &d_dq, double &d_dz);


  bool IsLongRange();


  double Xk_U  (double k, int level);
  double dXk_U_dk (double k, int level);
  double Xk_dU (double k, int level);
  double Xk_V  (double k);
  double Vk    (double k);
  void Setrc (double rc);

  PAtricubicFitClass()
  { 
    Comm.SetWorld();
    GridsAreMine = false; 
    NumBetas=0;
  }
  ~PAtricubicFitClass()
  { if (GridsAreMine){ delete qgrid; delete ygrid; } }
};

#endif
