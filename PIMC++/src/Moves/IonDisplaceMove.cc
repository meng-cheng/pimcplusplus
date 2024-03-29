#include "IonDisplaceMove.h"

double IonDisplaceStageClass::Sample (int &slice1, int &slice2,
				      Array<int,1> &activeParticles)
{
#if NDIM==3
  SpeciesClass &ionSpecies = Path.Species(IonSpeciesNum);
  double beta = PathData.Path.GetBeta();
  int first = ionSpecies.FirstPtcl;
  int last  = ionSpecies.LastPtcl;
  int N     = last - first + 1;
  if (DeltaRions.size() != N) {
    DeltaRions.resize(N);
    DriftForw.resize(N);
    DriftRev.resize(N);
    Weights.resize(N);
    Forces.resize(N);
    Rions.resize(N);
    rhat.resize(N);
  }
  dVec zero(0.0);
  Forces = 0.0;
  DeltaRions = zero;
  DriftForw = zero;
  DriftRev = zero;
  /// Store present ion positions
  for (int i=0; i<N; i++)
    Rions(i) = Path(0,i+first);

  if (UseSmartMC) {
    // Calculate the forward drift term
    assert (PathData.Actions.NodalActions(IonSpeciesNum) != NULL);
    FixedPhaseActionClass *fp = dynamic_cast<FixedPhaseActionClass*>
      (PathData.Actions.NodalActions(IonSpeciesNum));
    assert (fp != NULL);
    fp->GetIonForces(Forces);
  }
  else
    Forces = zero;

  /// Now, choose a random displacement 
  for (int ptclIndex=0; ptclIndex<IonsToMove.size(); ptclIndex++) {
    int ptcl = IonsToMove(ptclIndex);
    Vec3 delta;
    PathData.Path.Random.CommonGaussianVec (Sigma, delta);
    DriftForw(ptcl-first) = 0.5*beta*Sigma*Sigma*Forces(ptcl-first);
    DeltaRions(ptcl-first) = DriftForw(ptcl-first) + delta;


    // Actually displace the path
    SetMode(NEWMODE);
    for (int slice=0; slice<PathData.NumTimeSlices(); slice++)
      PathData.Path(slice, ptcl) = 
	PathData.Path(slice, ptcl) + DeltaRions(ptcl-first);
  }
  /// Now compute reverse move forces
  double logTratio = 0.0;
  if (UseSmartMC) {
    // Calculate the forward drift term
    assert (PathData.Actions.NodalActions(IonSpeciesNum) != NULL);
    FixedPhaseActionClass *fp = dynamic_cast<FixedPhaseActionClass*>
      (PathData.Actions.NodalActions(IonSpeciesNum));
    assert (fp != NULL);
    fp->GetIonForces(Forces);
    for (int ptclIndex=0; ptclIndex<IonsToMove.size(); ptclIndex++) {
      int ptcl = IonsToMove(ptclIndex);
      int i = ptcl - first;
      Vec3 delta;
      DriftRev(i) = 0.5*beta*Sigma*Sigma*Forces(i);
      logTratio += 
	-dot(DriftRev(i),  DriftRev(i)) +
	dot(DriftForw(i), DriftForw(i)) -
	2.0*dot(DeltaRions(i), DriftForw(i)+DriftRev(i));
    }
    perr << "DriftForw = " << DriftForw << endl;
  }

  logTratio /= (2.0*Sigma*Sigma);
  // The transition prob ratios from each processor get multiplied, so
  // they will be double counted if we do this this way.
  if (Path.Communicator.MyProc() != 0)
    logTratio = 0.0;

  if (WarpElectrons)
    return exp(logTratio)*NewElectronWarp();
  else
    return exp(logTratio);

  // And return sample probability ratio
  return 1.0;
#else
  cerr<<"Ion displace stage Sample not implemented in 2d"<<endl;
  assert(1==2);
#endif
}


double
IonDisplaceStageClass::NewElectronWarp()
{
#if NDIM==3
  SpeciesClass &ionSpecies = Path.Species(IonSpeciesNum);
  int N = ionSpecies.NumParticles;

  // Setup the warp
  bool warpForw = (PathData.Random.Common() > 0.5);
  cerr << "Starting " << (warpForw ? "forward" : "reverse") << " move.\n";
  SpaceWarp.Set (Rions, DeltaRions, Path.GetBox(), warpForw);

  // First stage:  warp every other slice
  Mat3 jMat;
  double jWarp = 0.0;
  for (int si=0; si<Path.NumSpecies(); si++) {
    SpeciesClass &species = Path.Species(si);
    if (species.lambda > 1.0e-10) 
      //      for (int slice=0; slice<Path.NumTimeSlices();slice+=2) {
      for (int slice=0; slice<Path.NumTimeSlices();slice++) {
	double factor = 
	  ((slice==0)||(slice==Path.NumTimeSlices()-1)) ? 0.5 : 1.0;
	for (int elec=species.FirstPtcl; elec<=species.LastPtcl; elec++) {
	  SetMode (OLDMODE);
	  Vec3 r = Path(slice, elec);
	  SetMode (NEWMODE);
	  Path(slice,elec) = SpaceWarp.Warp (r, jMat);
	  double d = det(jMat);
	  if (d <= 0) {
	    cerr <<"*****************************************************\n";
	    cerr << "det(jMat) = " << d << " at slice=" << slice
		 << " and elec = " << elec << endl;
	    cerr << "r = " << r << endl;
	    cerr <<"*****************************************************\n";
	    return (1.0e-300);
	  }
	  jWarp += factor * log(d);
	}
      }
  }
  
  cerr << "jWarp = " << jWarp << endl;
//   // Second stage: similar triangle construction
//   double gamma, h;
//   double jTri = 0.0;
//   double A, B, C;
//   A = B = C = 0.0;
//   double deltaK = 0.0;
//   for (int si=0; si<Path.NumSpecies(); si++) {
//     SpeciesClass &species = Path.Species(si);
//     double fourLambdaTauInv = 1.0/(4.0*species.lambda*Path.tau);
//     if (species.lambda > 1.0e-10) 
//       for (int slice=0; slice<Path.NumTimeSlices()-2;slice+=2) 
// 	for (int elec=species.FirstPtcl; elec<=species.LastPtcl; elec++) {
// 	  SetMode (OLDMODE);
// 	  Vec3 &r0 = Path(slice,   elec);	  
// 	  Vec3 &r1 = Path(slice+1, elec);
// 	  Vec3 &r2 = Path(slice+2, elec);
// 	  SetMode (NEWMODE);
// 	  Vec3 &r0p = Path(slice,   elec);	  
// 	  Vec3 &r1p = Path(slice+1, elec);
// 	  Vec3 &r2p = Path(slice+2, elec);
// 	  double alpha, beta, ratio;
// 	  SpaceWarp.SimilarTriangles (r0, r1, r2, r0p, r1p, r2p,
// 				      alpha, beta, h, gamma);
// 	  A -= 2.0*fourLambdaTauInv * gamma*gamma*h*h;
// 	  jTri += 2.0*log(gamma);
// 	  B += 1.0;
	    
// // 	  cerr << "alpha = " << alpha << endl;
// // 	  cerr << "beta = " << beta << endl;
// // 	  cerr << "gamma = " << gamma << endl;
// // 	  cerr << "h = " << h << endl;
// 	  C -= fourLambdaTauInv * 
// 	    ((gamma*gamma-1.0)*(alpha*alpha + beta*beta) - 2.0*h*h);
// 	  deltaK += fourLambdaTauInv * 
// 	    ((gamma*gamma-1.0)*(alpha*alpha + beta*beta) - 2.0*h*h);
// 	}
//   }
//   C += jWarp + jTri;

//   // Now, solve the scale equation
//   cerr << "A = " << A << "   B = " << B << "   C = " << C << endl;
//   double s = SpaceWarp.SolveScaleEquation (A, B, C);
//   cerr << "s = " << s <<  endl;
//   /// HACK HACK HACK
//   s = 1.0;
//   if (s < -1.0e10)
//     return 1.0e-300;
//   SetMode (NEWMODE);
//   // And scale the triangles
//   for (int si=0; si<Path.NumSpecies(); si++) {
//     SpeciesClass &species = Path.Species(si);
//     double fourLambdaTauInv = 1.0/(4.0*species.lambda*Path.tau);
//     if (species.lambda > 1.0e-10) 
//       for (int slice=0; slice<Path.NumTimeSlices()-2;slice+=2) 
// 	for (int elec=species.FirstPtcl; elec<=species.LastPtcl; elec++) {
// 	  Vec3 &r0 = Path(slice,   elec);	  
// 	  Vec3 &r1 = Path(slice+1, elec);
// 	  Vec3 &r2 = Path(slice+2, elec);
// 	  double hpp = SpaceWarp.ScaleTriangleHeight (r0, r1, r2, s);
// 	  deltaK += fourLambdaTauInv * 2.0 * hpp*hpp;
// 	}
//   }
//   double jScale = B * log(s);
//   cerr << "jTri = " << jTri << endl;
//   cerr << "jScale = " << jScale << endl;
//   cerr << "Estimate deltaK = " << deltaK << endl;
//  return (exp(jWarp + jTri + jScale));
  return (exp(jWarp));
#else
  cerr<<"IONDisplace Move does not work in 2d!!!"<<endl;
  assert(NDIM==3);
#endif
}



double
IonDisplaceStageClass::DoElectronWarp()
{
#if NDIM==3
  cerr << "DoElectronWarp...\n";
  SpeciesClass &ionSpecies = Path.Species(IonSpeciesNum);
  int ionFirst = ionSpecies.FirstPtcl;
  int ionLast  = ionSpecies.LastPtcl;
  int N     = ionLast - ionFirst + 1;
  dVec disp;
  double dist;
  double logJProdForw = 0.0;
  double logJProdRev  = 0.0;
  // The jacobian matrices for the forward and reverse move
  TinyMatrix<double,3,3> J;
  Array<double,1> g(N);
  Array<Vec3,1> wgr(N);
  for (int si=0; si<Path.NumSpecies(); si++) {
    SpeciesClass &species = Path.Species(si);    
    if (species.lambda > 1.0e-10) {
      for (int slice=0; slice<Path.NumTimeSlices(); slice++) {
	for (int elec=species.FirstPtcl; elec<=species.LastPtcl; elec++) {
	  SetMode (OLDMODE);
	  Weights = 0.0;
	  double totalWeight = 0.0;
	  for (int ion=ionFirst; ion <= ionLast; ion++) {
	    Path.DistDisp(slice, ion, elec, dist, disp);
	    double d4 = dist*dist*dist*dist;
	    Weights(ion-ionFirst) = 1.0/d4;
	    g(ion-ionFirst)      = -4.0/dist;
	    rhat(ion-ionFirst )   = (1.0/dist)*disp;
	    totalWeight += Weights(ion-ionFirst);
	  }
	  Weights *= (1.0/totalWeight);
	  SetMode (NEWMODE);
	  for (int ion=ionFirst; ion <= ionLast; ion++) 
	    Path(slice, elec) = Path(slice,elec) + 
	      Weights(ion-ionFirst)*DeltaRions(ion-ionFirst);

	  // Calculate Jacobian for forward move
	  J = 1.0, 0.0, 0.0,
  	      0.0, 1.0, 0.0,
	      0.0, 0.0, 1.0;
	  Vec3 sumwgr(0.0, 0.0, 0.0);
	  for (int j=0; j<N; j++) {
	    wgr(j) = Weights(j)*g(j)*rhat(j);
	    sumwgr += wgr(j);
	  }
	  for (int i=0; i<N; i++) {
	    // Vec3 dwdr = Weights(i)*(g(i)*rhat(i) - sumwgr);
	    Vec3 dwdr = (wgr(i) - Weights(i)*sumwgr);
	    for (int alpha=0; alpha<3; alpha++)
	      for (int beta=0; beta<3; beta++) 
		J(alpha,beta) += DeltaRions(i)[alpha] * dwdr[beta];
	  }
	  logJProdForw += log(det(J));
	}
      }
    }
  }
  // Repeat the loop for the reverse move to calculate the reverse
  // Jacobian 
  DeltaRions = -1.0*DeltaRions;
  for (int si=0; si<Path.NumSpecies(); si++) {
    SpeciesClass &species = Path.Species(si);    
    if (species.lambda > 1.0e-10) {
      for (int slice=0; slice<Path.NumTimeSlices(); slice++) {
	for (int elec=species.FirstPtcl; elec<=species.LastPtcl; elec++) {
	  Weights = 0.0;
	  double totalWeight = 0.0;
	  for (int ion=ionFirst; ion <= ionLast; ion++) {
	    Path.DistDisp(slice, ion, elec, dist, disp);
	    double d4 = dist*dist*dist*dist;
	    Weights(ion-ionFirst) = 1.0/d4;
	    g(ion-ionFirst)      = -4.0/dist;
	    rhat(ion-ionFirst )   = (1.0/dist)*disp;
	    totalWeight += Weights(ion-ionFirst);
	  }
	  Weights *= (1.0/totalWeight);
	  
	  // Calculate Jacobian for forward move
	  J = 1.0, 0.0, 0.0,
  	      0.0, 1.0, 0.0,
	      0.0, 0.0, 1.0;
	  Vec3 sumwgr(0.0, 0.0, 0.0);
	  for (int j=0; j<N; j++) {
	    wgr(j) = Weights(j)*g(j)*rhat(j);
	    sumwgr += wgr(j);
	  }
	  for (int i=0; i<N; i++) {
	    Vec3 dwdr = (wgr(i) - Weights(i)*sumwgr);
	    for (int alpha=0; alpha<3; alpha++)
	      for (int beta=0; beta<3; beta++) 
		J(alpha,beta) += DeltaRions(i)[alpha] * dwdr[beta];
	  }
	  logJProdRev += log(det(J));
	}
      }
    }
  }
  cerr << "JProdForw = " << exp(logJProdForw) << endl;
  cerr << "JProdRev =  " << exp(logJProdRev)  << endl;
  cerr << "JProdRev/JProdForw = " << exp(logJProdRev-logJProdForw) << endl;
  //  return exp(logJProdRev-logJProdForw);
  return 1.0;
#else 
  cerr<<"Not Implemented in 2d"<<endl;
  assert(NDIM==3);
#endif
}


void
IonDisplaceMoveClass::MakeMove ()
{
  cerr << "Start IonDisplaceMove  MyProc = " 
       << Path.Communicator.MyProc() << endl;
  // Move the Join out of the way.
  PathData.MoveJoin (PathData.Path.NumTimeSlices()-1);

  // First, displace the particles in the new copy
  SetMode(NEWMODE);

  // Construct list of particles to move
  vector<int> ptclList;
  int numLeft=0;
  SpeciesClass &species = PathData.Path.Species(IonSpeciesNum);
  for (int ptcl=species.FirstPtcl; ptcl<=species.LastPtcl; ptcl++) {
    ptclList.push_back(ptcl);
    numLeft++;
  }
  
  // First, choose particle to move
  for (int i=0; i<NumIonsToMove; i++) {
    int index = PathData.Path.Random.CommonInt(numLeft);
    vector<int>::iterator iter = ptclList.begin();
    IonDisplaceStage.IonsToMove(i) = ptclList[index];
    for (int j=0; j<index; j++)
      iter++;
    ptclList.erase(iter);
    numLeft--;
  }
  // Next, set timeslices
  Slice1 = 0;
  Slice2 = PathData.Path.NumTimeSlices()-1;
  // Now call MultiStageClass' MakeMove

  MultiStageClass::MakeMove();
  cerr << "End IonDisplaceMove  MyProc = " 
       << Path.Communicator.MyProc() << endl;

}


void
IonDisplaceMoveClass::Read (IOSectionClass &in)
{
  assert(in.ReadVar ("Sigma", Sigma));
  string ionSpeciesStr;
  assert (in.ReadVar("IonSpecies",   ionSpeciesStr));
  IonSpeciesNum  = Path.SpeciesNum(ionSpeciesStr);
  if (IonSpeciesNum == -1) {
    cerr << "Unrecogonized species, """ << ionSpeciesStr 
	 << """ in IonDisplaceMoveClass::Read.\n";
    abort();
  }
  assert (in.ReadVar ("NumToMove", NumIonsToMove));
  in.ReadVar("WarpElectrons", WarpElectrons, false);
  in.ReadVar("UseSmartMC", IonDisplaceStage.UseSmartMC, false);

  IonDisplaceStage.Sigma = Sigma;
  IonDisplaceStage.IonSpeciesNum = IonSpeciesNum;
  IonDisplaceStage.WarpElectrons = WarpElectrons;

  // Construct action list
  IonDisplaceStage.Actions.push_back(&PathData.Actions.ShortRange);
  if (PathData.Path.LongRange) 
    if (PathData.Actions.UseRPA)
      IonDisplaceStage.Actions.push_back(&PathData.Actions.LongRangeRPA);
    else
      IonDisplaceStage.Actions.push_back(&PathData.Actions.LongRange);

  if ((PathData.Actions.NodalActions(IonSpeciesNum)!=NULL)) {
    cerr << "IonDisplaceMove adding fermion node action for species " 
	 << ionSpeciesStr << endl;
    IonDisplaceStage.Actions.push_back
      (PathData.Actions.NodalActions(IonSpeciesNum));
  }
    
  // Now construct stage list
  Stages.push_back(&IonDisplaceStage);

  IonDisplaceStage.IonsToMove.resize(NumIonsToMove);
  if (WarpElectrons) {
    IonDisplaceStage.Actions.push_back(&PathData.Actions.Kinetic);
    ActiveParticles.resize(Path.NumParticles());
    for (int i=0; i<Path.NumParticles(); i++)
      ActiveParticles(i) = i;
  }
  else {
    SpeciesClass &species = Path.Species(IonSpeciesNum);
    ActiveParticles.resize(species.NumParticles);
    for (int ptcl=species.FirstPtcl; ptcl<=species.LastPtcl; ptcl++)
      ActiveParticles(ptcl-species.FirstPtcl) = ptcl;
  }
}
