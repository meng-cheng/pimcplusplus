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

#ifndef MOLECULE_INTERACTIONS_CLASS_H
#define MOLECULE_INTERACTIONS_CLASS_H

#include "ActionBase.h"
#include "../Moves/MoveUtils.h"

class MoleculeInteractionsClass : public ActionBaseClass
{
	bool ReadComplete;
	bool withS;
	bool IntraMolecular;
	enum Interactions{LJ,CHARGE,SPRING};
	Array<bool, 2> Interacting;
	Array<string, 1> LJSpecies;
	Array<string, 1> ChargeSpecies;
	Array<string, 1> SpringSpecies;
	Array<bool,2> Updated;
	Array<double,2> COMTable;
	Array<dVec,2> COMVecs;

	double RL, RU;

public:
 	// Parameters that can be specified on input
	// Defaults in constructor
	double prefactor;
	double CUTOFF;

	// hardwired units and unit conversions
	// set in constructor
  double elementary_charge, N_Avogadro, kcal_to_joule,
		epsilon_not, angstrom_to_m, SI, k_B, erg_to_eV, joule_to_eV;

	double sigma_over_cutoff, offset;

  void Read (IOSectionClass &in);
  double SingleAction (int slice1, int slice2, 
		       const Array<int,1> &activeParticles, int level);
  double d_dBeta (int slice1, int slice2, int level);
	double ComputeEnergy(int slice1, int slice2,
						const Array<int,1> &activeParticles, int level,
						bool with_truncations);
  double CalcCutoff(int ptcl1, int ptcl2, int slice, double Rcmag);
	double COMSeparation(int slice,int ptcl1,int ptcl2);
	double S(double r);

  string GetName();

	MoleculeInteractionsClass (PathDataClass &pathData);
};

#endif