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

#include "MoleculeInteractionsClass.h"
#include "../PathDataClass.h"

MoleculeInteractionsClass::MoleculeInteractionsClass (PathDataClass &pathData) :
  ActionBaseClass (pathData)
{
  elementary_charge = 1.602*pow(10.0,-19);
  N_Avogadro = 6.022*pow(10.0,23.0);
  kcal_to_joule = 4184;
  epsilon_not = 8.85*pow(10.0,-12);
  angstrom_to_m = pow(10.0,10);
  SI = 1/(4*M_PI*epsilon_not);
  k_B = 1.3807*pow(10.0,-23);
  erg_to_eV = 1.6*pow(10.0,12);
  joule_to_eV = pow(10.0,19)/1.602;

	// radial cutoffs for ST2 modulation function
	RL = 2.0160;
	RU = 3.1287;
	ReadComplete = false;
}

string MoleculeInteractionsClass::GetName(){
	return("MoleculeInteractionsClass");
}

double MoleculeInteractionsClass::SingleAction (int startSlice, int endSlice, 
	       const Array<int,1> &activeParticles, int level){

	assert(ReadComplete);
	bool IsAction = true;
	double TotalU = ComputeEnergy(startSlice, endSlice, activeParticles, level, TruncateAction, IsAction);
  return(TotalU*PathData.Path.tau);
}

double MoleculeInteractionsClass::d_dBeta (int startSlice, int endSlice,  int level)
{
	cerr << "MoleculeInteractions::d_dBeta__________________ for slices " << startSlice << " to " << endSlice << endl;
  Array<int,1> activeParticles(PathData.Path.NumParticles());
  for (int i=0;i<PathData.Path.NumParticles();i++)
    activeParticles(i)=i;
	
	bool IsAction = false;
	double TotalU = ComputeEnergy(startSlice, endSlice-1, activeParticles, level, TruncateEnergy, IsAction);
	cerr << "RETURNING " << TotalU << endl;
  return TotalU;
}

double MoleculeInteractionsClass::ComputeEnergy(int startSlice, int endSlice, 
	       const Array<int,1> &activeParticles, int level, bool with_truncations, bool isAction){

	Updated = false;
  for (int counter=0; counter<Path.DoPtcl.size(); counter++)
    Path.DoPtcl(counter)=true;

  double TotalU = 0.0;
	double TotalLJ = 0.0;
	double TotalCharge = 0.0;
	double TotalSpring = 0.0;
	double TotalKinetic = 0.0;
  int numChangedPtcls = activeParticles.size();
  int skip = 1<<level;

	//cerr << "Hello.  This is MoleculeInteractionsClass::ComputeEnergy.  ActivePtcls are " << activeParticles << endl;
	//cerr << "I'm doing Intramolecular " << IntraMolecular << " and with_truncations " << with_truncations << " and with modulation " << withS << endl;
	//cerr << "CUTOFF is " << CUTOFF << endl;
  for (int ptcl1Index=0; ptcl1Index<numChangedPtcls; ptcl1Index++){
    int ptcl1 = activeParticles(ptcl1Index);
    Path.DoPtcl(ptcl1) = false;
		//cerr << "DoPtcl bools are after killing " << ptcl1 << Path.DoPtcl << endl;
    int species1=Path.ParticleSpeciesNum(ptcl1);

		//	Calculate Lennard-Jones interaction
    if (Interacting(species1,0)){
			//cerr << "LJ for ptcl " << ptcl1 << " of species " << species1 << endl;
  		for (int ptcl2=0; ptcl2<PathData.Path.NumParticles(); ptcl2++){
    		int species2=Path.ParticleSpeciesNum(ptcl2);
				//cerr << "Considering LJ between " << ptcl1 << " and " << ptcl2 << " for which DoPtcl is " << Path.DoPtcl(ptcl2) << "...";
    		if (Interacting(species2,0) && Path.DoPtcl(ptcl2)){
					//cerr << " Going to compute between " << ptcl1 << " and " << ptcl2;
	  			for (int slice=startSlice;slice<=endSlice;slice+=skip){
	    			dVec r;
	    			double rmag;
	    			PathData.Path.DistDisp(slice, ptcl1, ptcl2, rmag, r);
						//cerr << "... rmag is " << rmag;
						//if(ptcl1==Path.MolRef(ptcl1) && ptcl2==Path.MolRef(ptcl2) && !Updated(ptcl1,ptcl2)){
						//	Updated(ptcl1,ptcl2) = Updated(ptcl2,ptcl1) = true;
						//	COMTable(ptcl1,ptcl2) = COMTable(ptcl2,ptcl1) = rmag;
						//	COMVecs(ptcl1,ptcl2) = r;
						//	COMVecs(ptcl2,ptcl1) = -1*r;
						//}
						//	disregard interactions outside spherical cutoff
            if (rmag <= CUTOFF){
							//cerr << "<"<<CUTOFF;
              double rinv = 1.0/rmag;
              double sigR = PathData.Species(species1).Sigma*rinv;
              double sigR6 = sigR*sigR*sigR*sigR*sigR*sigR;
							double offset = 0.0;
							if(with_truncations){
  							double sigma_over_cutoff = PathData.Species(species1).Sigma/CUTOFF;
  							offset = pow(sigma_over_cutoff,12) - pow(sigma_over_cutoff,6);
							}
							//cerr << "; using offset " << offset << endl;
	      			double lj = 4*PathData.Species(species1).Epsilon*(sigR6*(sigR6-1) - offset); // this is in kcal/mol 
							TotalLJ += lj;
	      			TotalU += lj;
							//cerr  << TotalU << " added " << lj << " from LJ interaction between " << ptcl1 << " and " << ptcl2 << endl;
            }
						//cerr << "... outside" << endl;
	  			}
				}
				else{
					//cerr << "Skipped" << endl;
				}
      }
    }
		if(Interacting(species1,1)){
      /// calculating coulomb interactions
  		for (int ptcl2=0; ptcl2<PathData.Path.NumParticles(); ptcl2++){
    		int species2=Path.ParticleSpeciesNum(ptcl2);
    		if (Interacting(species2,1) && Path.DoPtcl(ptcl2)){
					//	don't compute intramolecular interactions
					//  unless told to
					if(IntraMolecular || Path.MolRef(ptcl1)!=Path.MolRef(ptcl2)){
	  				for (int slice=startSlice;slice<=endSlice;slice+=skip){
	    				double rmag;
	    				dVec r;
	    				PathData.Path.DistDisp(slice,ptcl1,ptcl2,rmag,r);
							// implement spherical cutoff
            	double Ormag = COMSeparation(slice,ptcl1,ptcl2);
            	if (Ormag <= CUTOFF){
								double truncate = 0.0;
              	double ptclCutoff = 1.0;
								if(with_truncations){
									truncate = 1.0;
									ptclCutoff = CalcCutoff(ptcl1,ptcl2,slice,CUTOFF);
								}
								double modulation=1.0;
								if(withS)
									modulation = S(Ormag);
              	double coulomb = prefactor*PathData.Species(species1).Charge
																*PathData.Species(species2).Charge*modulation*(1.0/rmag - truncate/ptclCutoff);
								TotalCharge += coulomb;
	      				TotalU += coulomb;
								//cerr  << TotalU << " added " << coulomb << " from charge-charge interaction between " << ptcl1 << " and " << ptcl2 << endl;
  	          }
  	        }
					}
  	    }
  	  }
  	}
		if(Interacting(species1,2)){
			// harmonic intermolecular potential
			// parameters for ST2 water dimer
			// could be generalized
			double omega = 26;
			double m_H2O = 0.043265;
			double R0 = 2.85;
  		for (int ptcl2=0; ptcl2<PathData.Path.NumParticles(); ptcl2++){
    		int species2=Path.ParticleSpeciesNum(ptcl2);
    		if (Interacting(species2,2) && Path.DoPtcl(ptcl2)){
	  			for (int slice=startSlice;slice<=endSlice;slice+=skip){
  					dVec COMr;
  					double COMrmag;
  					PathData.Path.DistDisp(slice, ptcl1, ptcl2, COMrmag, COMr);
						TotalSpring += 0.5*m_H2O*omega*omega*(COMrmag - R0)*(COMrmag - R0);
						TotalU += TotalSpring;
					}
				}
			}
		}
		if(Interacting(species1,3)){
			// compute kinetic action: imaginary time spring term
			// Based on KineticClass::SingleAction, execpt that displacements are WRT the molecule COM

			//cerr << "species " << species1 << ", ptcl " << ptcl1 << " computing kinetic...";
		  double TotalK = 0.0;
		  int skip = 1<<level;
		  double levelTau = Path.tau* (1<<level);
		  double lambda = lambdas(species1);
		  if (lambda != 0){
		    double FourLambdaTauInv=1.0/(4.0*lambda*levelTau*levelTau);
		    for (int slice=startSlice; slice < endSlice;slice+=skip) {
		      dVec vel;
					vel = COMVelocity(slice, slice+skip, ptcl1);
			
		      double GaussProd = 1.0;
		      for (int dim=0; dim<NDIM; dim++) {
						double GaussSum=0.0;
						for (int image=-NumImages; image<=NumImages; image++) {
						  double dist = vel[dim]+(double)image*Path.GetBox()[dim];
						  GaussSum += exp(-dist*dist*FourLambdaTauInv);
						}
						GaussProd *= GaussSum;
		      }
					TotalK -= log(GaussProd);
		    }
		  }
			TotalKinetic += TotalK;
			TotalU += TotalK;
			//cerr << TotalKinetic << endl;
		}
	}
	// write additional output file
	if(!isAction && special){
		outfile << TotalU << " " << TotalLJ << " " << TotalCharge << " " << TotalSpring << " " << TotalKinetic << endl;
	}
  return (TotalU);
}


double MoleculeInteractionsClass::CalcCutoff(int ptcl1, int ptcl2, int slice, double Rcmag){
  // get oxygen particle ids
  int Optcl1 = Path.MolRef(ptcl1);
  int Optcl2 = Path.MolRef(ptcl2);
  // get vectors of oxygens
  dVec O1 = Path(slice,Optcl1);
  dVec O2 = Path(slice,Optcl2);
  // get vector between oxygens
  dVec Roo;
  double Ormag;
	//if(!Updated(Optcl1,Optcl2)){
  	Path.DistDisp(slice, Optcl1, Optcl2, Ormag, Roo);
	//	Updated(Optcl1,Optcl2) = Updated(Optcl2,Optcl1) = true;
	//	COMTable(Optcl1,Optcl2) = COMTable(Optcl2,Optcl1) = Ormag;
	//	COMVecs(Optcl1,Optcl2) = Roo;
	//	COMVecs(Optcl2,Optcl1) = -1*Roo;
	//} else {
	//	Ormag = COMTable(Optcl1,Optcl2);
	//	Roo = COMVecs(Optcl1,Optcl2);
	//}
  dVec Rc = Scale(Roo,Rcmag);
  // get constituent coordinates WRT oxygen COM
  dVec P1 = Path(slice,ptcl1);
  dVec P2 = Path(slice,ptcl2);
  P1 -= O1;
  P2 -= O2;
  // solve for vector between constituents (if molecule 2 is at cutoff radius)
  dVec R12 = Rc + P2 - P1;
  // obtain cutoff magnitude
  double r12 = Mag(R12);
  return r12;
}

double MoleculeInteractionsClass::COMSeparation (int slice,int ptcl1,int ptcl2)
{
  // get oxygen particle ids
  int Optcl1 = Path.MolRef(ptcl1);
  int Optcl2 = Path.MolRef(ptcl2);
  dVec Or;
  double Ormag;

	//if(!Updated(Optcl1,Optcl2)){
  	PathData.Path.DistDisp(slice, Optcl1, Optcl2, Ormag, Or);
//		Updated(Optcl1,Optcl2) = Updated(Optcl2,Optcl1) = true; COMTable(Optcl1,Optcl2) = COMTable(Optcl2,Optcl1) = Ormag;
//		COMVecs(Optcl1,Optcl2) = Or;
//		COMVecs(Optcl2,Optcl1) = -1*Or;
//	} else {
//		Ormag = COMTable(Optcl1,Optcl2);
//	}

  return Ormag;
}

double MoleculeInteractionsClass::S(double r)
{
  double mod;
  if(r<RL)
    mod = 0.0;
  else if(r>RU)
    mod = 1.0;
  else{
    double diff1 = r - RL;
    double diff2 = 3*RU - RL - 2*r;
    double diff3 = RU - RL;
    mod = diff1*diff1*diff2/(diff3*diff3*diff3);
  }
  return mod;
}

dVec MoleculeInteractionsClass::COMVelocity(int sliceA, int sliceB, int ptcl){
	dVec p1 = Path(sliceB, ptcl);
	dVec p2 = Path(sliceA, ptcl);
	if(ptcl != PathData.Path.MolRef(ptcl)){
		int COMptcl = PathData.Path.MolRef(ptcl);
 		p1 -= Path(sliceB, COMptcl);
 		p2 -= Path(sliceA, COMptcl);
	}
 	dVec vel = p1 - p2;
  PathData.Path.PutInBox(vel);
	return vel;
}

void MoleculeInteractionsClass::Read (IOSectionClass &in)
{
	if(!ReadComplete){
		cerr << "In MoleculeInteractionsClass::Read" << endl;
		// DEFAULTS
		prefactor = SI*angstrom_to_m*elementary_charge*
								elementary_charge*N_Avogadro/kcal_to_joule;
		CUTOFF = Path.GetBox()(0)/2;
		IntraMolecular = false;
		withS = true;
		TruncateAction = true;
		TruncateEnergy = false;

		in.ReadVar("Cutoff",CUTOFF);
		in.ReadVar("Prefactor",prefactor);
		in.ReadVar("Modulated",withS);
		in.ReadVar("Intramolecular",IntraMolecular);
		in.ReadVar("TruncateAction",TruncateAction);
		in.ReadVar("TruncateEnergy",TruncateEnergy);

		Interacting.resize(PathData.NumSpecies(),4);
		Interacting = false;
		LJSpecies.resize(0);
		ChargeSpecies.resize(0);
		SpringSpecies.resize(0);
		KineticSpecies.resize(0);
		in.ReadVar("LJSpecies",LJSpecies);
		in.ReadVar("ChargeSpecies",ChargeSpecies);
		in.ReadVar("SpringSpecies",SpringSpecies);
		in.ReadVar("KineticActionSpecies",KineticSpecies);

		if(KineticSpecies.size() > 0){
			assert(in.ReadVar("Lambdas",lambdas));
			cerr << "Read lambda for each species: " << lambdas << endl;
		}

		for(int s=0; s<LJSpecies.size(); s++)
			Interacting(Path.SpeciesNum(LJSpecies(s)), 0) = true;
		for(int s=0; s<ChargeSpecies.size(); s++)
			Interacting(Path.SpeciesNum(ChargeSpecies(s)), 1) = true;
		for(int s=0; s<SpringSpecies.size(); s++)
			Interacting(Path.SpeciesNum(SpringSpecies(s)), 2) = true;
		for(int s=0; s<KineticSpecies.size(); s++)
			Interacting(Path.SpeciesNum(KineticSpecies(s)), 3) = true;
		cerr << "MoleculeInteractions::Read I have loaded Interacting table " << Interacting << endl;
	
		Updated.resize(PathData.Path.numMol);
		COMTable.resize(PathData.Path.numMol);
		COMVecs.resize(PathData.Path.numMol);

		special = false;
		in.ReadVar("ExtraOutput",special);
		if(special){
			outfile.open("MoleculeEnergyBreakdown.dat");
			outfile << "# Total LJ Coulomb Spring Kinetic" << endl;
		}
			
		
		ReadComplete = true;
	}
}

void MoleculeInteractionsClass::SetNumImages (int num)
{
	NumImages = num;
}
