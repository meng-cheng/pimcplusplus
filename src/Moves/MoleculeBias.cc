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

#include "MoleculeBias.h"
#include "MoveUtils.h"

MoleculeForceBiasMove::MoleculeForceBiasMove(PathDataClass &myPathData,IOSectionClass outSection)
  : MolMoveClass (myPathData,outSection)
{
	cerr << "MoleculeForceBias constructor" << endl;
}

MoleculeForceBiasMove::MoleculeForceBiasMove(PathDataClass &myPathData,IOSectionClass outSection, int numToRead, int start)
  : MolMoveClass (myPathData,outSection, numToRead, start)
{
	cerr << "MoleculeForceBias constructor" << endl;
}

void MoleculeForceBiasMove::Read(IOSectionClass &moveInput) {
  cerr << "  Force Bias read in" << endl;
  moveInput.ReadVar("DoTranslation",doTrans);
  moveInput.ReadVar("DoRotation",doRot);
  assert(moveInput.ReadVar("SetStep",Step));
  assert(moveInput.ReadVar("SetAngle",Theta));
	MolMoveClass::Read(moveInput);
  doTrans = true;
  doRot = true;
  Theta *= M_PI;
  numGen = 0;
  numInProp = 0.0;
}

double MoleculeForceBiasMove::Sample(int &slice1,int &slice2, Array<int,1> &activeParticles) {
  double Tau = PathData.Path.tau;

	if(mode == SINGLE){
  	int choosemol = (int)floor(PathData.Path.Random.Local()*PathData.Path.numMol);
		MoveList(0) = choosemol;
		activeParticles.resize(PathData.Path.MolMembers(MoveList(0)).size());
		for(int i=0; i<activeParticles.size(); i++) activeParticles(i) = PathData.Path.MolMembers(MoveList(0))(i);
	}
	else if(mode == SEQUENTIAL){
		activeParticles.resize(PathData.Path.MolMembers(MoveList(0)).size());
		for(int i=0; i<activeParticles.size(); i++) activeParticles(i) = PathData.Path.MolMembers(MoveList(0))(i);
	}
	else if(mode == GLOBAL){
		activeParticles.resize(PathData.Path.NumParticles());
		for(int i=0; i<activeParticles.size(); i++) activeParticles(i) = i;
	}

  double b, numAdded;
  bool noMoveMade = true;
  while(noMoveMade){
    numGen++;
	  // choose a time slice to move
	  int numSlices = PathData.Path.TotalNumSlices;
	  int slice=0;
	  slice1 = 0;
	  slice2 = 0;
	  if(numSlices>1){
	    int P_max = numSlices - 1;
	    slice = (int)floor(P_max*PathData.Path.Random.Local()) + 1;
      //slice1 = slice;
      //slice2 = slice;
	    slice1 = slice-1;
	    slice2 = slice+1;
	  }

    numAdded = 0;
    dVec sumF,sumN;
    b = 1.0;

    // for each molecule to be moved, calculate force and torque on each ptcl
	  for(int activeMolIndex=0; activeMolIndex<MoveList.size(); activeMolIndex++){
      // need to generate move
      // Generate a random unit vector to specify point of reflection
      dVec A;
      for (int x = 0; x < 3; x++)
        A(x) = (PathData.Path.Random.Local()-0.5);
      A = Normalize(A);
      // Generate an angle and a displacement vector
      double theta = 2*(PathData.Path.Random.Local()-0.5)*Theta;
      double x = 2*(PathData.Path.Random.Local()-0.5)*Step;
      double y = 2*(PathData.Path.Random.Local()-0.5)*Step;
      double z = 2*(PathData.Path.Random.Local()-0.5)*Step;
      //double V = step*step*step*4*M_PI*angle_step*2*M_PI; // need probability that a move is generated = 1/V
      sumF(0) = 0;
      sumF(1) = 0;
      sumF(2) = 0;
      sumN(0) = 0;
      sumN(1) = 0;
      sumN(2) = 0;
      int activeMol = MoveList(activeMolIndex);
      dVec O = PathData.Path(slice,activeMol);
      for(int i=0;i<PathData.Path.MolMembers(activeMol).size(); i++){
        int ptcl = PathData.Path.MolMembers(activeMol)(i);
        dVec coord = PathData.Path(slice,ptcl);
        coord -= O;
        dVec F = PathData.Actions.MoleculeInteractions.Force(slice,ptcl); // calculate force
        sumF += F;
        dVec lever = CalcLever(A,coord);
        sumN += cross(lever,F);
      }   

      dVec dR;
      dR(0) = x;
      dR(1) = y;
      dR(2) = z;

      double Pprop = 1.0;
      if(doTrans)
        Pprop *= exp(Tau*dot(sumF,dR));
      if(doRot)
        Pprop *= exp(Tau*dot(sumN,A)*theta);

      double T = Pprop;  // Transition probability
      if (Pprop > 1)
        T = 1.0;  // Transition probability
        //  double Pprop = 1; // this is just to test*************************************************************

      // Propose the move
      if(Pprop>=PathData.Path.Random.Local()){
        noMoveMade = false;
        numAdded ++;
        // do translation
        if(doTrans){
          dVec move = TranslateMol(slice,PathData.Path.MolMembers(activeMol),dR); 
        }
        // do rotation
        if(doRot){  
          RotateMol(slice,PathData.Path.MolMembers(activeMol), A, theta);
        }

        dVec Orev = PathData.Path(slice,activeMol);
        dVec sumFrev, sumNrev;
        sumFrev(0) = 0;
        sumFrev(1) = 0;
        sumFrev(2) = 0;
        sumNrev(0) = 0;
        sumNrev(1) = 0;
        sumNrev(2) = 0;
        for(int i=0;i<PathData.Path.MolMembers(activeMol).size(); i++){
          int ptcl = PathData.Path.MolMembers(activeMol)(i);
          dVec coord = PathData.Path(slice,ptcl);
          coord -= Orev;
          dVec F = PathData.Actions.MoleculeInteractions.Force(slice,ptcl); // calculate force
          sumFrev += F;
          dVec lever = CalcLever(A,coord);
          sumNrev += cross(lever,F);
        }   
        double PpropRev = 1.0;
        if(doTrans)
          PpropRev *= exp(Tau*dot(sumFrev,(-1*dR)));
        if(doRot)
          PpropRev *= exp(Tau*dot(sumNrev,A)*(-theta));  //Reverse Transition probability
        double Trev = PpropRev;
        if (PpropRev > 1)
          Trev = 1.0;
        b *= Trev/T;
      }
    }
  }

  if(numAdded > 0){
    numMoves++;
    numInProp += numAdded;
  }
  else
    cerr << "umm something's weird numAdded is " << numAdded << endl;

  if (numMoves%10000 == 0 && numMoves>0){
    cerr << numMoves << " moves; current FORCE-BIAS ratio is " << double(numAccepted)/numMoves << endl;
    cerr << "NOTE: please disregard acceptance output from individual moves; it will erroneously state 0" << endl;
  }

  if (numGen%10000 == 0){
    cerr << "Force-Bias:  Generated " << numGen << " moves.  Accepted " << numMoves << " with ratio " << double(numMoves)/numGen  << endl;
    cerr << "Avg number of molecules in proposed moves is " << numInProp/numMoves << endl;
  }

	if(mode == SEQUENTIAL) Advance();

	return b; 
}

dVec CalcLever(dVec axis, dVec coord){
  dVec coord_perp;
  dVec coord_aligned;
  Strip(axis,coord,coord_aligned,coord_perp);
  return coord_perp; 
}

//void ForceBias::MakeMove(){
//  numGen++;
//  double Tau = PathData.Path.tau;
//  int slice =0; //classical for now
//
//  int numMol = PathData.Path.numMol;
//  Array<int,1> ActiveParticles(5);
//  int choosemol = (int)floor(PathData.Path.Random.Local()*numMol);
//  AssignPtcl(choosemol,ActiveParticles);
//
//  // Generate a random unit vector to specify point of reflection
//  dVec A;
//  for (int x = 0; x < 3; x++)
//    A(x) = (PathData.Path.Random.Local()-0.5);
//  A = Normalize(A);
//
//  // Generate an angle and a displacement vector
//  double angle_step = 0.3;
//  double theta = angle_step*(PathData.Path.Random.Local()-0.5)*2*M_PI;
//  double step = 0.3;
//  double V = step*step*step*4*M_PI*angle_step*2*M_PI; // need probability that a move is generated = 1/V
//  double x = (PathData.Path.Random.Local()-0.5)*step;
//  double y = (PathData.Path.Random.Local()-0.5)*step;
//  double z = (PathData.Path.Random.Local()-0.5)*step;
//  dVec dR;
//  dR(0) = x;
//  dR(1) = y;
//  dR(2) = z;
//
//  dVec sumF,sumN;
//  sumF(0) = 0;
//  sumF(1) = 0;
//  sumF(2) = 0;
//  sumN(0) = 0;
//  sumN(1) = 0;
//  sumN(2) = 0;
//  // for each particle in the molecule to be moved, calculate force and torque
//  dVec O = PathData.Path(slice,ActiveParticles(0));
//  for(int i=0;i<ActiveParticles.size();i++){
//    int ptcl = ActiveParticles(i);
//    dVec coord = PathData.Path(slice,ptcl);
//    coord -= O;
//    dVec F = PathData.Actions.ST2Water.Force(slice,ptcl,0); // calculate force
//    sumF += F;
//    dVec lever = CalcLever(A,coord);
//    sumN += Cross(lever,F);
//  }
//
//  double Pprop = exp(Tau*(Dot(sumF,dR) + Dot(sumN,A)*theta));
//  //double Pprop = exp(Tau*(Dot(sumF,dR)));// + Dot(sumN,A)*theta));  // Probability of proposing the generated move
//  double T = Pprop;  // Transition probability
//  if (Pprop > 1)
//    T = 1.0;  // Transition probability
////  double Pprop = 1; // this is just to test*************************************************************
//  if(Pprop>=PathData.Path.Random.Local()){  // Propose the move
//    numProp ++;
////cerr << "Proposing move on molecule " << choosemol << "; dR is " << dR << " and theta is " << theta << endl;
////cerr << "  I have force F " << sumF << " and torque " << sumN << endl;
//    // Calculate old action
//    double oldAction = 0.0;
//    oldAction += PathData.Actions.ST2Water.Action(slice,slice,ActiveParticles,0);
////    oldAction += PathData.Actions.Kinetic.Action(startSlice,endSlice,OActiveParticles,0); 
////cerr << "    Old action is " << oldAction << endl;
//
//    // Make Move: rotate then translate coordinates
//    PathData.Path.SetPos(slice,ActiveParticles(0),(O+dR)); // translate oxygen
//    for(int i=1;i<ActiveParticles.size();i++){  
//      dVec old_coord=PathData.Path(slice,ActiveParticles(i));
//      old_coord -= O;  // WRT COM
////cerr << "    " << i << ": old coord is " << old_coord << endl;
//      dVec new_coord = ArbitraryRotate(A,old_coord,theta); // rotate
////cerr << "     new coord is " << new_coord << "; rotating about " << A << endl;
//     // dVec new_coord = old_coord;
//      new_coord += O;
//      new_coord += dR;  // translate
//      PathData.Path.SetPos(slice,ActiveParticles(i),new_coord);
//    }
//
//    // Calculate new action
//    double newAction = 0.0;
//    newAction += PathData.Actions.ST2Water.Action(slice,slice,ActiveParticles,0);
////    newAction += PathData.Actions.Kinetic.Action(startSlice,endSlice,OActiveParticles,0); 
////cerr << "    New action is " << newAction << endl;
//
//    // Need reverse Transition probability
//    dVec Orev = PathData.Path(slice,ActiveParticles(0));
//    dVec sumFrev, sumNrev;
//    sumFrev(0) = 0;
//    sumFrev(1) = 0;
//    sumFrev(2) = 0;
//    sumNrev(0) = 0;
//    sumNrev(1) = 0;
//    sumNrev(2) = 0;
//    for(int i=0;i<ActiveParticles.size();i++){
//      int ptcl = ActiveParticles(i);
//      dVec coordrev = PathData.Path(slice,ptcl);
//      coordrev -= Orev;
//      dVec Frev = PathData.Actions.ST2Water.Force(slice,ptcl,0); // calculate force
//      sumFrev += Frev;
//      dVec lever = CalcLever(A,coordrev);
//      sumNrev += Cross(lever,Frev);
//    }
//    double PpropRev = exp(Tau*(Dot(sumFrev,(-1*dR)) + Dot(sumNrev,A)*(-theta)));  //Reverse Transition probability
//    //double PpropRev = exp(Tau*(Dot(sumFrev,(-1*dR))));// + Dot(sumNrev,A)*(-theta)));  //Reverse Transition probability
//    double Trev = PpropRev;
//    if (PpropRev > 1)
//      Trev = 1.0;
//
//    // Acceptance Probability
////    double P = exp(-(newAction - oldAction));  // just to test: should recover uniform Metropolis sampling
//    double P = exp(-(newAction - oldAction))*Trev/T;
////cerr << "  P is " << P << endl;
//
//    // Metropolis Accept/ Reject
//    if (P>=PathData.Path.Random.Local()){
//      PathData.AcceptMove(slice,slice,ActiveParticles);
//      NumAccepted++;
//    }
//    else {
//      PathData.RejectMove(slice,slice,ActiveParticles);
//    }
//
//    NumMoves++;
//    if (NumMoves%10000 == 0){
//      cerr << NumMoves << " moves; current force-bias ratio is " << double(NumAccepted)/NumMoves << " with step size " << step << " and angle " << angle_step << endl;
//    }
//  }  // end propose move conditional
//  if (numGen%10000 == 0)
//    cerr << "Force-Bias:  Generated " << numGen << " moves.  Accepted " << numMoves << " with ratio " << double(numMoves)/numGen  << endl;
//}