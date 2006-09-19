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

#include "SpecificHeatA.h"

void SpecificHeatAClass::Accumulate()
{
  PathData.MoveJoin(PathData.NumTimeSlices()-1);
  NumSamples++;
  
  PairActionFitClass *specificHeat1;
  PairActionFitClass *specificHeat2;
  double tauValue1;
  double tauValue2;
  ///Get pair array for specific heat 1
  for (int i=0;i<PathData.Actions.SpecificHeatPairArray.size();i++){
    if (PathData.Actions.SpecificHeatPairArray(i)->Particle1.Name=="SpecificHeatHe1"){
      specificHeat1=PathData.Actions.SpecificHeatPairArray(i);
      tauValue1=PathData.Actions.TauValues(i);
      assert(PathData.Actions.SpecificHeatPairArray(i)->Particle2.Name=="SpecificHeatHe1");
    }
    if (PathData.Actions.SpecificHeatPairArray(i)->Particle1.Name=="SpecificHeatHe2"){
      specificHeat2=PathData.Actions.SpecificHeatPairArray(i);
      tauValue2=PathData.Actions.TauValues(i);
      assert(PathData.Actions.SpecificHeatPairArray(i)->Particle2.Name=="SpecificHeatHe2");
    }
  }
  Array<int,1> changedParticles;
  changedParticles.resize(PathData.Path.NumParticles());
  for (int i=0;i<changedParticles.size();i++)
    changedParticles(i)=i;

  double srA1=
    PathData.Actions.ShortRange.SingleActionForcedPairAction(0, PathData.Path.NumTimeSlices()-1,
							   *specificHeat1);
  double srA2 =
    PathData.Actions.ShortRange.SingleActionForcedPairAction(0, PathData.Path.NumTimeSlices()-1,
							   *specificHeat2);
  

  double kA1= 
    PathData.Actions.Kinetic.SingleActionForcedTau  (0, PathData.Path.NumTimeSlices()-1,
						  changedParticles, 
						  0, tauValue1);
  double kA2 =
    PathData.Actions.Kinetic.SingleActionForcedTau (0, PathData.Path.NumTimeSlices()-1,
						    changedParticles, 
						    0, tauValue2);
    
  double kE1=PathData.Actions.Kinetic.d_dBetaForcedTau (0,PathData.Path.NumTimeSlices()-1,
							 0,tauValue1);
  
  double kE2=PathData.Actions.Kinetic.d_dBetaForcedTau (0,PathData.Path.NumTimeSlices()-1,
							 0,tauValue2);

  double srE1=PathData.Actions.ShortRange.d_dBetaForcedPairAction(0, PathData.Path.NumTimeSlices()-1,
								  *specificHeat1);

  double srE2=PathData.Actions.ShortRange.d_dBetaForcedPairAction (0, 
								   PathData.Path.NumTimeSlices()-1,
								   *specificHeat2);


  double kinetic, dUShort, dULong, node, vShort, vLong, tip5p;
  PathData.Actions.Energy (kinetic, dUShort, dULong, node, vShort, vLong);

  double E=kinetic+dUShort;
  cerr<<"KE AAA"<<kinetic<<" "<<kE1<<" "<<dUShort<<" "<<srE1<<endl;
  TotalE+=E;
  ds_dBeta2+=E*E;
  d2s_dBeta2+=((srE1+kE1)-(srE2+kE2))/(tauValue1-tauValue2);

}

void SpecificHeatAClass::ShiftData (int NumTimeSlices)
{
  // Do nothing
}

void SpecificHeatAClass::WriteBlock()
{
  int nslices=PathData.Path.TotalNumSlices;
  double norm = 1.0/((double)NumSamples);
  TotalEVar.Write(Prefactor*PathData.Path.Communicator.Sum(TotalE)*norm);
  ds_dBeta2Var.Write(Prefactor*PathData.Path.Communicator.Sum(ds_dBeta2)*norm);
  d2sdBeta2Var.Write(Prefactor*PathData.Path.Communicator.Sum(d2s_dBeta2)*norm);
  
  //  TotalVar.Write   (Prefactor*PathData.Path.Communicator.Sum(TotalSum)*norm);
  //  KineticVar.Write (Prefactor*PathData.Path.Communicator.Sum(KineticSum)*norm);
  //  dUShortVar.Write (Prefactor*PathData.Path.Communicator.Sum(dUShortSum)*norm);

  //  dULongVar.Write  (Prefactor*PathData.Path.Communicator.Sum(dULongSum)*norm);
  //  NodeVar.Write    (Prefactor*PathData.Path.Communicator.Sum(NodeSum)*norm);
  //  VShortVar.Write  (Prefactor*PathData.Path.Communicator.Sum(VShortSum)*norm);
  //  VLongVar.Write   (Prefactor*PathData.Path.Communicator.Sum(VLongSum)*norm);
 
  TotalE=0.0;
  ds_dBeta2=0.0;
  d2s_dBeta2=0.0;
  NumSamples=0;
}

void SpecificHeatAClass::Read(IOSectionClass &in)
{  
  ObservableClass::Read(in);

  TotalE=0.0;
  ds_dBeta2=0.0;
  d2s_dBeta2=0.0;
  
  if (PathData.Path.Communicator.MyProc()==0){
    WriteInfo();
    IOSection.WriteVar("Type","Scalar");
  }
}

