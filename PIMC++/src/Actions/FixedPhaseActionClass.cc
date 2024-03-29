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

#include <Common/MPI/Communication.h>
#include "FixedPhaseActionClass.h"
#include "../PathDataClass.h"
#include <Common/MatrixOps/MatrixOps.h>

FixedPhaseClass::FixedPhaseClass(PathDataClass &pathData) :
  PathData (pathData), Path(pathData.Path), UseMDExtrap(false),
  UseLDA(true)
{
  
}

void
FixedPhaseClass::Setk(Vec3 k)
{
  kVec = k;
  System[0].Setk(k);
  if (MonteCarloIons)
    System[1].Setk(k);
  UpdateBands();
}

void
FixedPhaseClass::GetBandEnergies(Array<double,1> &energies)
{
  int numBands = System().GetNumBands();
  if (energies.size() != numBands)
    energies.resize(numBands);
  for (int band=0; band<numBands; band++)
    energies(band) = System().GetEnergy(band);
}

inline double mag2 (complex<double> z)
{
  return (z.real()*z.real()+z.imag()*z.imag());
}

void
FixedPhaseClass::CalcDensity(Array<double,3> &rho)
{
  int nx = rho.extent(0); double nxInv = 1.0/(nx-1);
  int ny = rho.extent(1); double nyInv = 1.0/(ny-1);
  int nz = rho.extent(2); double nzInv = 1.0/(nz-1);
  int nBands  = BandSplines().N;
  dVec box = PathData.Path.GetBox();
  Array<complex<double>,1> vals(nBands);
  
  rho = 0.0;

  for (int ix=0; ix<nx; ix++) {
    double x = (nxInv*ix-0.5)*box[0];
    for (int iy=0; iy<ny; iy++) {
      double y = (nyInv*iy-0.5)*box[1];
      for (int iz=0; iz<nz; iz++) {
	double z = (nzInv*iz-0.5)*box[2];
	BandSplines()(x, y, z, vals);
	for (int i=0; i<nBands; i++)
	  rho(ix, iy, iz) += mag2(vals(i));
      }
    }
  }
  // Now normalize;
  double sum = 0.0;
  double dV = box[0]*box[1]*box[2]/(nx*ny*nz);
  for (int ix=0; ix<nx; ix++)
    for (int iy=0; iy<ny; iy++)
      for (int iz=0; iz<nz; iz++)
	sum += rho(ix,iy,iz);

  double norm = (double)(NumUp+NumDown)/(dV*sum);
  rho *= norm;
  // This guarantees that when we integrate rho over the box, we get
  // the number of electrons.
}

void
FixedPhaseClass::CalcBandDensity(Array<double,4> &rho)
{
  int nx = rho.extent(0); double nxInv = 1.0/(nx-1);
  int ny = rho.extent(1); double nyInv = 1.0/(ny-1);
  int nz = rho.extent(2); double nzInv = 1.0/(nz-1);
  int nBands  = BandSplines().N;
  dVec box = PathData.Path.GetBox();
  Array<complex<double>,1> vals(nBands);
  
  rho = 0.0;

  for (int ix=0; ix<nx; ix++) {
    double x = (nxInv*ix-0.5)*box[0];
    for (int iy=0; iy<ny; iy++) {
      double y = (nyInv*iy-0.5)*box[1];
      for (int iz=0; iz<nz; iz++) {
	double z = (nzInv*iz-0.5)*box[2];
	BandSplines()(x, y, z, vals);
	for (int i=0; i<nBands; i++)
	  rho(ix, iy, iz, i) = mag2(vals(i));
      }
    }
  }
}



void 
FixedPhaseClass::ShiftData(int slicesToShift, int speciesNum)
{
//   if (speciesNum == UpSpeciesNum) {
//     UpMatrixCache.ShiftData  (slicesToShift, Path.Communicator);
//     UpGradMatCache.ShiftData (slicesToShift, Path.Communicator);
//     UpAction.ShiftLinkData   (slicesToShift, Path.Communicator);
//     UpGrad2.ShiftData        (slicesToShift, Path.Communicator);
//   }
//   if (speciesNum == DownSpeciesNum) {
//     DownMatrixCache.ShiftData (slicesToShift, Path.Communicator);
//     DownGradMatCache.ShiftData(slicesToShift, Path.Communicator);
//     DownAction.ShiftLinkData  (slicesToShift, Path.Communicator);
//     DownGrad2.ShiftData       (slicesToShift, Path.Communicator);
//   }
  // Recalculate actions everywhere instead of shifting.
  if (speciesNum == IonSpeciesNum) {
    SetMode(NEWMODE);
    Action (0, Path.NumTimeSlices()-1, Path.Species(IonSpeciesNum).Ptcls, 
	    0, IonSpeciesNum);
    AcceptCopy(0, Path.NumTimeSlices()-1,   UpSpeciesNum);
    AcceptCopy(0, Path.NumTimeSlices()-1, DownSpeciesNum);
  }
  
}

void
FixedPhaseClass::MoveJoin (int oldJoinPos, int newJoinPos, int speciesNum)
{
  if ((speciesNum != UpSpeciesNum) && (speciesNum != DownSpeciesNum))
    return;
  Mirrored3DClass<complex<double> > &matCache = (speciesNum == UpSpeciesNum) ?
    UpMatrixCache : DownMatrixCache;
  Mirrored3DClass<cVec3> &gradCache = (speciesNum == UpSpeciesNum) ?
    UpGradMatCache : DownGradMatCache;
  int first   = Path.Species(speciesNum).FirstPtcl;
  int numPtcl = Path.Species(speciesNum).NumParticles;

  if (newJoinPos > oldJoinPos) {
    for (int slice=oldJoinPos+1; slice <= newJoinPos; slice++)
      for (int pi=0; pi<matCache[OLDMODE].extent(1); pi++) {
	int ptcl = pi+first;
	int perm = Path.Permutation(ptcl)-first;
	matCache[OLDMODE](slice,pi,Range::all()) = 
	  matCache[NEWMODE](slice,perm,Range::all());
	gradCache[OLDMODE](slice,pi,Range::all()) = 
	  gradCache[NEWMODE](slice,perm,Range::all());
      }
    matCache[NEWMODE](Range(oldJoinPos+1,newJoinPos),Range::all(),Range::all())
      = matCache[OLDMODE](Range(oldJoinPos+1,newJoinPos),Range::all(),Range::all());
    gradCache[NEWMODE](Range(oldJoinPos+1,newJoinPos),Range::all(),Range::all())
      = gradCache[OLDMODE](Range(oldJoinPos+1,newJoinPos),Range::all(),Range::all());
  }
  else if (oldJoinPos > newJoinPos) {
    for (int slice=newJoinPos+1; slice<=oldJoinPos; slice++) 
      for (int pi=0; pi<matCache[OLDMODE].extent(1); pi++) {
	int ptcl = pi+first;
	int perm = Path.Permutation(ptcl)-first;
	matCache[OLDMODE](slice,perm,Range::all()) = 
	  matCache[NEWMODE](slice,pi,Range::all());
	gradCache[OLDMODE](slice,perm,Range::all()) = 
	  gradCache[NEWMODE](slice,pi,Range::all());
      }
    matCache[NEWMODE](Range(newJoinPos+1,oldJoinPos),Range::all(),Range::all())
      = matCache[OLDMODE](Range(newJoinPos+1,oldJoinPos),Range::all(),Range::all());
    gradCache[NEWMODE](Range(newJoinPos+1,oldJoinPos),Range::all(),Range::all())
      = gradCache[OLDMODE](Range(newJoinPos+1,oldJoinPos),Range::all(),Range::all());
  }
}


void 
FixedPhaseClass::AcceptCopy (int slice1, int slice2, int speciesNum)
{
  if (speciesNum == UpSpeciesNum) {
    UpGrad2.AcceptCopy         (slice1, slice2);
    UpAction.AcceptCopy        (slice1, slice2-1);
    UpMatrixCache[OLDMODE](Range(slice1,slice2), Range::all(), Range::all()) = 
      UpMatrixCache[NEWMODE](Range(slice1,slice2), Range::all(), Range::all());
    UpGradMatCache[OLDMODE](Range(slice1,slice2),Range::all(),Range::all()) =
      UpGradMatCache[NEWMODE](Range(slice1,slice2),Range::all(),Range::all());
  }
  else if (speciesNum == DownSpeciesNum) {
    DownGrad2.AcceptCopy       (slice1, slice2);
    DownAction.AcceptCopy      (slice1, slice2-1);
    DownMatrixCache[OLDMODE](Range(slice1,slice2),Range::all(),Range::all())=
      DownMatrixCache[NEWMODE](Range(slice1,slice2),Range::all(),Range::all());
    DownGradMatCache[OLDMODE](Range(slice1,slice2),Range::all(),Range::all())= 
     DownGradMatCache[NEWMODE](Range(slice1,slice2),Range::all(),Range::all());
  }
  else if (speciesNum == IonSpeciesNum) {
    Rions.AcceptCopy();
    if (MonteCarloIons)
      System[OLDMODE] = System[NEWMODE];
    BandSplines.AcceptCopy();
  }
}

void 
FixedPhaseClass::RejectCopy (int slice1, int slice2, int speciesNum)
{
  if (speciesNum == UpSpeciesNum) {
    UpGrad2.RejectCopy         (slice1, slice2);
    UpAction.RejectCopy        (slice1, slice2-1);
    UpMatrixCache[NEWMODE](Range(slice1,slice2),Range::all(),Range::all()) = 
      UpMatrixCache[OLDMODE](Range(slice1,slice2),Range::all(),Range::all());
    UpGradMatCache[NEWMODE](Range(slice1,slice2),Range::all(),Range::all()) = 
      UpGradMatCache[OLDMODE](Range(slice1,slice2),Range::all(),Range::all());
  }
  else if (speciesNum == DownSpeciesNum) {
    DownGrad2.RejectCopy       (slice1, slice2);
    DownAction.RejectCopy      (slice1, slice2-1);
    DownMatrixCache[NEWMODE](Range(slice1,slice2),Range::all(),Range::all()) = 
      DownMatrixCache[OLDMODE](Range(slice1,slice2),Range::all(),Range::all());
    DownGradMatCache[NEWMODE](Range(slice1,slice2),Range::all(),Range::all())= 
     DownGradMatCache[OLDMODE](Range(slice1,slice2),Range::all(),Range::all());
  }
  else if (speciesNum == IonSpeciesNum) {
    Rions.RejectCopy();
    if (MonteCarloIons)
      System[NEWMODE] = System[OLDMODE];
    BandSplines.RejectCopy();
  }

}





void 
FixedPhaseClass::Init(int speciesNum)
{
  ModeType mode = GetMode();
  SetMode (NEWMODE);
  if (IonsHaveMoved()) {
    UpdateBands();
    UpdateCache();
  }
  SetMode (OLDMODE);
  if (IonsHaveMoved()) {
    UpdateBands();
    UpdateCache();
  }
  SetMode (mode);
  
  
  if (speciesNum == UpSpeciesNum) {
    perr << "Initializing up species.\n";  
    UpdateCache();
//     for (int slice=0; slice<Path.NumTimeSlices(); slice++) 
//       CalcGrad2(slice, UpSpeciesNum);
  }
  else if (speciesNum == DownSpeciesNum) {
    perr << "Initializing down species.\n";
    UpdateCache();
//     for (int slice=0; slice<Path.NumTimeSlices(); slice++) 
//       CalcGrad2(slice, DownSpeciesNum);
  }
}


bool
FixedPhaseClass::IonsHaveMoved()
{
  SpeciesClass &species = Path.Species(IonSpeciesNum);
  int first = species.FirstPtcl;
  int last = species.LastPtcl;
  int ptcl = first;
  bool changed = false;

  for (int ptcl=first; ptcl<=last; ptcl++) {
    dVec diff = Path(0,ptcl) - System().GetIonPos(ptcl-first);
    if (dot (diff,diff) > 1.0e-16)
      changed = true;
  }
  if (changed)
    perr << "Ions have moved.\n";
  return changed;
}

inline Vec3 real(cVec3 v)
{
  return Vec3(v[0].real(), v[1].real(), v[2].real());
}

inline Vec3 imag(cVec3 v)
{
  return Vec3(v[0].imag(), v[1].imag(), v[2].imag());
}


double
FixedPhaseClass::CalcGrad2 (int slice, int species, const Array<int,1> &activeParticles,
			    UpdateType update)
{
  if ((species != UpSpeciesNum) && (species != DownSpeciesNum))
    return 0.0;
  Mirrored2DClass<Vec3> &Grad    = (species == UpSpeciesNum) ? UpGrad  : DownGrad;
  Mirrored1DClass<double> &Phase = (species == UpSpeciesNum) ? UpPhase : DownPhase;
  int N = Path.Species(species).NumParticles;
  /// calculate \f$\det|u|\f$ and \f$ \nabla det|u| \f$.

  complex<double> detu = GradientDet(slice, species, activeParticles, update);

#ifdef DEBUG
  complex<double> detuOld = GradientDet(slice, species, activeParticles, UPDATE_ALL);
  if (mag2(detu-detuOld)>1.0e-12*mag2(detuOld)) {
    cerr << "Inconsistent cached gradient at slice = " << slice 
	 << " and species = " << species << endl;
    cerr << "activeParticles = " << activeParticles 
	 << " detu=" << detu << " detuOld=" << detuOld << endl;
    fprintf (stderr, "Old matrix = \n");
    for (int i=0; i<8; i++) {
      for (int j=0; j<8; j++)
	fprintf (stderr, "%8.4f ", real(Matrix(i,j)));
      fprintf (stderr, "\n");
    }
    fprintf (stderr, "New matrix = \n");
    for (int i=0; i<8; i++) {
      for (int j=0; j<8; j++)
	fprintf (stderr, "%8.4f ", (species == UpSpeciesNum) ? 
		 real(UpMatrixCache(slice, i, j)) :
		 real(DownMatrixCache(slice, i, j)));
      fprintf (stderr, "\n");
    }
    fprintf (stderr, "\n");
  }
  else
    perr << "Passed matrix check.\n";
#endif

  double detu2 = mag2(detu);
  double detu2Inv = 1.0/detu2;
  
  double grad2 = 0.0;
  Phase(slice) = atan2(detu.imag(), detu.real());
  for (int i=0; i<N; i++) {
    Grad (slice, i) = 
      (detu.real()*imag(Gradient(i)) - detu.imag()*real(Gradient(i)))*detu2Inv - kVec;
    grad2 += dot(Grad(slice,i),Grad(slice,i));
  }
  return grad2;
}

double
FixedPhaseClass::PhaseGrad (int slice, int species, 
			    const Array<int,1> &activeParticles,
			    Array<Vec3,1> &gradPhase, UpdateType update)
{
  int N = Path.Species(species).NumParticles;
  /// calculate \f$\det|u|\f$ and \f$ \nabla det|u| \f$.

  complex<double> detu = GradientDet(slice, species, activeParticles, update);

#ifdef DEBUG
  complex<double> detuOld = GradientDet (slice, species, activeParticles, UPDATE_ALL);
  if (mag2(detu-detuOld)>1.0e-12*mag2(detuOld)) 
    cerr << "Cache inconsistency in PhaseGrad!.\n";
  else
    perr << "Matrix check passed.\n";
#endif 

  double detu2 = mag2(detu);
  double detu2Inv = 1.0/detu2;

  double phase = atan2(detu.imag(), detu.real());
  for (int i=0; i<N; i++) 
    gradPhase(i) = detu2Inv *
      (detu.real()*imag(Gradient(i)) - detu.imag()*real(Gradient(i))) - kVec;
  return phase;
}


inline 
double dot (Array<Vec3,1> &v1, Array<Vec3,1> &v2)
{
  double val = 0.0;
  for (int i=0; i<v1.size(); i++)
    val += dot(v1(i), v2(i));
  return val;
}


double
FixedPhaseClass::CalcAction (Array<Vec3,1> &G1, Array<Vec3,1> &G2,
			     double phase1, double phase2, Array<Vec3,1> &dR)
{
  int N = G1.size();
  static Array<Vec3,1> G1T, G2T, u;
  if (G1T.size() != N) {
    G1T.resize(N);
    G2T.resize(N);
    u.resize(N);
  }

  double dRMag, dRMag2;
  dRMag2 = dot (dR, dR);
  dRMag  = sqrt(dRMag2);
  double dv = (phase2-phase1);
  /// Add on the e^{-i k \cdot r} term
  for (int i=0; i<G1.size(); i++) 
    dv -= dot(kVec, dR(i));

  while (dv > M_PI)
    dv -= 2.0*M_PI;
  while (dv < -M_PI)
    dv += 2.0*M_PI;

  u = (1.0/dRMag)*dR;
  double g1 = dot (G1, u);
  double g2 = dot (G2, u);
  double g1T = g1*dRMag;
  double g2T = g2*dRMag;
  G1T = G1 - g1*u;
  G2T = G2 - g2*u;
  double action = 0.0;
  // Up and down electrons must have the same mass.
  double lambda = PathData.Path.Species(UpSpeciesNum).lambda;
  for (int i=0; i<N; i++)
    action += 0.5*lambda*(dot(G1T(i), G1T(i))+dot(G2T(i), G2T(i)));
  action += lambda/(15.0*dRMag2)*
    (2.0*(g1T*g1T+g2T*g2T) -3.0*(g1T+g2T)*dv + 18.0*dv*dv - g1T*g2T);
  // HACK HACK HACK
//   for (int i=0; i<N; i++)
//     action += 0.5*lambda*(dot(G1(i),G1(i)) + dot (G2(i),G2(i)));
  return action;
}


double
FixedPhaseClass::Action (int slice1, int slice2, 
			 const Array<int,1> &activeParticles, 
			 int level, int speciesNum)
{
#if NDIM==3
  int skip = 1<<level;
  double levelTau = ldexp(Path.tau, level);
  const double lambda = Path.Species(UpSpeciesNum).lambda;

  // The nodal action should only be used at level = 0;
  bool doUp   = false;
  bool doDown = false;
  if (IonsHaveMoved()) {
    if (!MonteCarloIons) {
      ModeType mode = GetMode();
      UpdateBands();
      if (mode == OLDMODE) {
	BandSplines.RejectCopy();
	UpdateCache();
	SetMode(NEWMODE);
	UpdateCache();
	SetMode (OLDMODE);
      }      
      else {
	BandSplines.AcceptCopy();
	UpdateCache();
	SetMode (OLDMODE);
	UpdateCache();
	SetMode(NEWMODE);
      }
    }
  }
  
  doUp   = (speciesNum == UpSpeciesNum);
  doDown = (speciesNum == DownSpeciesNum);
  if (speciesNum == IonSpeciesNum) {
    doUp   = true;
    doDown = true;
  }
  
  // If it's the ions that have moved, update everything.  Else,
  // update just the active electrons
  UpdateType update = 
    (speciesNum == IonSpeciesNum) ? UPDATE_ALL : UPDATE_ACTIVE;
  double action = 0.0;
  
  if (doUp) {
    static Array<Vec3,1> G1, G2, dR;
    int first = PathData.Path.Species(UpSpeciesNum).FirstPtcl;
    if (G1.size() != NumUp) {
      G1.resize(NumUp); G2.resize(NumUp); dR.resize(NumUp);
    }
    double v1, v2;
    v1 = PhaseGrad (slice1, UpSpeciesNum, activeParticles, G1, update);
    for (int link=slice1; link<slice2; link+=skip) {
      v2 = PhaseGrad (link+skip, UpSpeciesNum, activeParticles, G2, update);
      for (int ptcl=0; ptcl<NumUp; ptcl++)
	dR(ptcl) = PathData.Path.Velocity(link, link+skip, ptcl+first);
      UpAction(link) = CalcAction(G1, G2, v1, v2, dR);
      action += levelTau * UpAction(link);
      v1 = v2;
      G1 = G2;
    }
  }

  if (doDown) {
    static Array<Vec3,1> G1, G2, dR;
    int first = PathData.Path.Species(DownSpeciesNum).FirstPtcl;
    if (G1.size() != NumDown) {
      G1.resize(NumDown); G2.resize(NumDown); dR.resize(NumDown);
    }
    double v1, v2;
    v1 = PhaseGrad (slice1, DownSpeciesNum, activeParticles, G1, update);
    for (int link=slice1; link<slice2; link+=skip) {
      v2 = PhaseGrad (link+skip, DownSpeciesNum, activeParticles, G2, update);
      for (int ptcl=0; ptcl<NumDown; ptcl++)
	dR(ptcl) = PathData.Path.Velocity(link, link+skip, ptcl+first);
      DownAction(link) = CalcAction (G1, G2, v1, v2, dR);
      action += levelTau * DownAction(link);
      v1 = v2;
      G1 = G2;
    }
  }
  return action;
#endif
}

double
FixedPhaseClass::d_dBeta (int slice1, int slice2, int level,
			  int speciesNum)
{
  int skip = 1<<level;
  const double lambda = Path.Species(UpSpeciesNum).lambda;

  // The nodal action should only be used at level = 0;
  bool doUp   = false;
  bool doDown = false;
  if (IonsHaveMoved()) {
    UpdateBands();
    UpdateCache();
  }

  double dU = 0.0;
  if (speciesNum == UpSpeciesNum) 
    for (int link=slice1; link < slice2; link+=skip) {
      // dU += 0.5*lambda*(UpGrad2(link)+UpGrad2(link+skip));
      dU += UpAction(link);
    }
  if (speciesNum == DownSpeciesNum) 
    for (int link=slice1; link < slice2; link+=skip) {
      // dU += 0.5*lambda*(DownGrad2(link)+DownGrad2(link+skip));
      dU += DownAction(link);
    }
  return dU;
}

void 
FixedPhaseClass::Read(IOSectionClass &in)
{
#if NDIM==3
  UseMDExtrap = false;
  in.ReadVar("UseMDExtrap", UseMDExtrap);
  if (UseMDExtrap) 
    perr << "Using MD wavefunction extrapolation.\n";
  else
    perr << "Not using MD wavefunction extrapolation.\n";
  in.ReadVar ("MonteCarloIons", MonteCarloIons, false);
  if (MonteCarloIons) 
    perr << "Using duplicate plane wave objects for MC on ions.\n";
  else
    perr << "Using single plane wave object for non-MC ion moves.\n";

  string speciesString;
  assert (in.ReadVar ("UpSpecies", speciesString));
  UpSpeciesNum = Path.SpeciesNum (speciesString);
  if (in.ReadVar ("DownSpecies", speciesString))
    DownSpeciesNum = Path.SpeciesNum(speciesString);
  else
    DownSpeciesNum = -1;

  assert (in.ReadVar ("IonSpecies", speciesString));
  IonSpeciesNum = Path.SpeciesNum (speciesString);

  assert (in.ReadVar ("kCut", kCut));
  /// Note that the twist angles are defined between 0 and 1.  Hence,
  /// k_x = pi*twist_angle_x/L_x
  /// NOTE:  I think this should be 2*pi*twist_angle_x/L_x
  Array<double,2> twistAngles;
  if (in.ReadVar ("TwistAngles", twistAngles)) {
    if (PathData.IntraComm.MyProc()==0)
      if (twistAngles.extent(0) != PathData.InterComm.NumProcs()) {
	cerr << "Error:  Number of twist angles must match number of clones.\n" 
	     << "        Number of twist angles:  "  << twistAngles.extent(0) << endl 
	     << "        Number of clones:        "  << PathData.InterComm.NumProcs() 
	     << endl;
	abort();
      }
    assert (twistAngles.extent(1) == 3);
    kVec[0] = twistAngles(PathData.GetCloneNum(),0) * 2.0*M_PI/PathData.Path.GetBox()[0];
    kVec[1] = twistAngles(PathData.GetCloneNum(),1) * 2.0*M_PI/PathData.Path.GetBox()[1];
    kVec[2] = twistAngles(PathData.GetCloneNum(),2) * 2.0*M_PI/PathData.Path.GetBox()[2];
  }
  else {
    Array<double,1> tmpkVec;
    assert(in.ReadVar ("kVec", tmpkVec));
    kVec = Vec3 (tmpkVec(0), tmpkVec(1), tmpkVec(2));
  }
  
  NumIons =  Path.Species(IonSpeciesNum).NumParticles;
  NumUp   =  Path.Species(UpSpeciesNum).NumParticles;
  NumDown = (DownSpeciesNum != -1) ? 
    Path.Species(DownSpeciesNum).NumParticles : -1;

  if (DownSpeciesNum != -1)
    assert (NumUp == NumDown);
  
  Workspace.resize (ComplexDetCofactorsWorksize(NumUp));
  Matrix.resize(NumUp, NumUp);
  Cofactors.resize(NumUp, NumUp);
  GradMat.resize(NumUp, NumUp);
  Gradient.resize(NumUp);
  UpGrad.resize    (PathData.NumTimeSlices(), NumUp);
  DownGrad.resize  (PathData.NumTimeSlices(), NumDown);
  UpPhase.resize   (PathData.NumTimeSlices());
  DownPhase.resize (PathData.NumTimeSlices());
  UpGrad2.resize   (PathData.NumTimeSlices());
  DownGrad2.resize (PathData.NumTimeSlices());
  UpAction.resize  (PathData.NumTimeSlices()-1);
  DownAction.resize(PathData.NumTimeSlices()-1);
  OrbitalValues.resize(NumUp);

  /////////////////////
  // Allocate caches //
  /////////////////////
  UpMatrixCache.resize   (PathData.NumTimeSlices(), NumUp, NumUp);
  UpGradMatCache.resize  (PathData.NumTimeSlices(), NumUp, NumUp);
  DownMatrixCache.resize (PathData.NumTimeSlices(), NumDown, NumDown);
  DownGradMatCache.resize(PathData.NumTimeSlices(), NumDown, NumDown);
  UpParticles.resize(NumUp);
  DownParticles.resize(NumDown);
  for (int i=0; i<NumUp; i++) 
    UpParticles(i) = i+Path.Species(UpSpeciesNum).FirstPtcl;
  for (int i=0; i<NumDown; i++) 
    DownParticles(i) = i+Path.Species(DownSpeciesNum).FirstPtcl;

  Rions.resize(NumIons);
  Rions[0] = Vec3(0.0, 0.0, 0.0);
  Rions[1] = Vec3(0.0, 0.0, 0.0);

  /////////////////////////////////
  // Setup the plane wave system //
  /////////////////////////////////
  NumFilled = max(NumUp, NumDown);
  NumBands = 2*NumFilled;
  // The last true indicates using MD extrapolation to initialize
  // wavefunctions. 
  System.SetOld (new MPISystemClass
		 (NumBands,NumUp+NumDown, PathData.IntraComm,
		  PathData.InterComm, UseLDA, UseMDExtrap));
  if (MonteCarloIons) {
    System.SetNew (new MPISystemClass
		   (NumBands,NumUp+NumDown, PathData.IntraComm,
		    PathData.InterComm, UseLDA, UseMDExtrap));
  }
  else 
    System.SetNew (&(System[OLDMODE]));
    
  V_elec_ion = &PathData.Actions.GetPotential (IonSpeciesNum,  UpSpeciesNum);
  V_ion_ion  = &PathData.Actions.GetPotential (IonSpeciesNum, IonSpeciesNum);
  System[0].Setup (Path.GetBox(), kVec, kCut, 
		 *V_elec_ion, *V_ion_ion, UseLDA);
  System[0].Read (in);
  /////////////////////////////
  // Setup the ion positions //
  /////////////////////////////
  SpeciesClass& ionSpecies = Path.Species(IonSpeciesNum);
  int first = ionSpecies.FirstPtcl;
  for (int i=0; i<NumIons; i++)
    Rions[0](i)= Rions[1](i) = Path(0,i+first);
  Rions[0] = Vec3(0.0, 0.0, 0.0);  
  Rions[1] = Vec3(0.0, 0.0, 0.0);
  System[0].SetIons (Rions[0]);

  if (MonteCarloIons)
    System[1] = System[0];
  
  ////////////////////////////////////////
  // Setup real space grids and splines //
  ////////////////////////////////////////
  int nx, ny, nz;
  System().GetBoxDims (nx, ny, nz);
  // Increment sizes to compensate for PBC
  nx++; ny++; nz++;
  Vec3 box;
  box = Path.GetBox();
  xGrid.Init (-0.5*box[0], 0.5*box[0], nx);
  yGrid.Init (-0.5*box[1], 0.5*box[1], ny);
  zGrid.Init (-0.5*box[2], 0.5*box[2], nz);
  Array<complex<double>,4> initData(nx,ny,nz,NumFilled);
  initData = 0.0;
  BandSplines[0].Init (&xGrid, &yGrid, &zGrid, initData, true);
  BandSplines[1].Init (&xGrid, &yGrid, &zGrid, initData, true);
  
  //  UpdateBands();
#endif
}


complex<double>
FixedPhaseClass::GradientDet(int slice, int speciesNum,
			     const Array<int,1> &activeParticles,
			     UpdateType update)
{
#if NDIM==3
  SpeciesClass &species = Path.Species(speciesNum);
  int N = species.NumParticles;
  int first = species.FirstPtcl;
  if (N != Matrix.rows()) {
    cerr << "N = " << N << endl;
    cerr << "Matrix.rows() = " << Matrix.rows() << endl;
  }
  assert (N == Matrix.rows());

  Array<complex<double>,3> &matData = 
    (speciesNum==UpSpeciesNum) ? UpMatrixCache.data() : DownMatrixCache.data();
  Array<cVec3,3> gradData = 
    (speciesNum==UpSpeciesNum) ? UpGradMatCache.data() : DownGradMatCache.data();


  if (update != UPDATE_NONE) {
    // First, update determinant matrix for the active particles
    Array<complex<double>,1> vals;
    Array<cVec3,1> grads;
    if (update == UPDATE_ALL) 
      for (int j=0; j<N; j++) {
	Vec3 r_j = Path(slice, j+first);
	Path.PutInBox(r_j);
	vals.reference ( matData(slice, j, Range::all()));
	grads.reference(gradData(slice, j, Range::all()));
	BandSplines().FValGrad(r_j[0], r_j[1], r_j[2], vals, grads);
      }
    else if (update == UPDATE_ACTIVE)
      for (int i=0; i<activeParticles.size(); i++) {
	int j = activeParticles(i) - first;
	if ((j >= 0) && (j<N)) {
	  Vec3 r_j = Path(slice, activeParticles(i));
	  Path.PutInBox(r_j);
	  vals.reference ( matData(slice, j, Range::all()));
	  grads.reference(gradData(slice, j, Range::all()));
	  BandSplines().FValGrad(r_j[0], r_j[1], r_j[2], vals, grads);
	}
      }
    else {
      cerr << "Unrecognized update type.\n";
      abort();
    }
      
  }

  // Compute determinant and cofactors
  Cofactors = matData(slice, Range::all(), Range::all());
  complex<double> det = ComplexDetCofactors (Cofactors, Workspace);

  // Now, compute gradient
  Gradient = cVec3(0.0, 0.0, 0.0);
  for (int i=0; i<N; i++) {
    for (int j=0; j<N; j++)
      Gradient(i) += Cofactors(i,j)*gradData(slice,i,j);
  }
  return det;
#endif
}




complex<double>
FixedPhaseClass::GradientDetFD(int slice, int speciesNum)
{
#if NDIM==3
  SpeciesClass &species = Path.Species(speciesNum);
  int N = species.NumParticles;
  int first = species.FirstPtcl;
  assert (N == Matrix.rows());

  const double eps = 1.0e-6;

  // First, fill up determinant matrix
  Array<complex<double>,1> row;
  for (int j=0; j<N; j++) {
    Vec3 r_j = Path(slice, first+j);
    Path.PutInBox(r_j);
    row.reference(Matrix(j,Range::all()));
    BandSplines()(r_j[0], r_j[1], r_j[2], row);
  }

  complex<double> det = Determinant (Matrix);

  for (int i=0; i<N; i++) {
    cVec3 plus, minus;
    plus = 0.0; minus = 0.0;
    Vec3 r_i = Path(slice, first+i);
    Vec3 r;
    row.reference(Matrix(i,Range::all()));
    for (int dim=0; dim<NDIM; dim++) {
      r = r_i;
      r[dim] += eps;
      Path.PutInBox(r);
      BandSplines()(r[0], r[1], r[2], row);
      plus[dim] = Determinant (Matrix);
      
      r = r_i;
      r[dim] -= eps;
      Path.PutInBox(r);
      BandSplines()(r[0], r[1], r[2], row);
      minus[dim] = Determinant (Matrix);
    }
    // Reset row to orignal value
    r = r_i;
    Path.PutInBox(r);
    BandSplines()(r[0], r[1], r[2], row);
    // And compute gradient
    Gradient(i) = (plus-minus)/(2.0*eps);
  }
  return det;
#endif
}


// Note:  This function assumes that the matrix caches are up to date.
void
FixedPhaseClass::CalcWFratios (int slice, int ptcl, const Array<Vec3,1> &pos, 
			       Array<complex<double>,1> &ratios)
{
#if NDIM==3
  // Find which determinant to update
  int speciesNum = Path.ParticleSpeciesNum(ptcl);
  if (speciesNum != UpSpeciesNum && speciesNum != DownSpeciesNum) {
    cerr << "Invalid particle in CalcWFratios.\n";
    abort();
  }
  int first = Path.Species(speciesNum).FirstPtcl;

  Array<complex<double>,3> &matData = 
    (speciesNum==UpSpeciesNum) ? UpMatrixCache.data() : DownMatrixCache.data();

  // Compute determinant and cofactors
  Cofactors = matData(slice, Range::all(), Range::all());
  complex<double> det = ComplexDetCofactors (Cofactors, Workspace);
  complex<double> detInv = complex<double>(1.0, 0.0) / det;

  // Make sure we're doing this right:
  Vec3 rnow = Path(slice, ptcl);
  Path.PutInBox(rnow);
  BandSplines()(rnow[0], rnow[1], rnow[2], OrbitalValues);
  complex<double> shouldBone(0.0, 0.0);
  for (int j=0; j<OrbitalValues.size(); j++)
    shouldBone += Cofactors(ptcl-first,j)*OrbitalValues(j);
  shouldBone *= detInv;
  if ((fabs(1.0-shouldBone.real()) > 1.0e-10) ||
      (fabs(shouldBone.imag()) > 1.0e-10)) {
    cerr << "Error in CalcWFratios.!\n";
    cerr << "shouldBone = " << shouldBone << "     ";
    cerr << "ptcl = " << ptcl << "  slice = " << slice <<  endl;
  }
    

  // Now loop through positions and compute ratios
  ratios = complex<double>();
  for (int ipos=0; ipos<pos.size(); ipos++) {
    Vec3 r = pos(ipos);
    Path.PutInBox(r);
    BandSplines()(r[0], r[1], r[2], OrbitalValues);
    for (int j=0; j<OrbitalValues.size(); j++)
      ratios(ipos) += Cofactors(ptcl-first,j)*OrbitalValues(j);
    ratios(ipos) *= detInv;
  }
#endif
#if NDIM==2
  cerr<<"FixedPhaseActionClass.cc doesn't work in 2d"<<endl;
#endif
}


void
FixedPhaseClass::UpdateBands()
{
#if NDIM==3
  // Now, make bands real and put into splines
  Array<complex<double>,4> data(xGrid.NumPoints, 
				yGrid.NumPoints, 
				zGrid.NumPoints, NumFilled);
  SpeciesClass& ionSpecies = Path.Species(IonSpeciesNum);
  int first = ionSpecies.FirstPtcl;
  for (int i=0; i<NumIons; i++)
    Rions(i) = Path(0,i+first);
  System().SetIons (Rions);

  /// The diagonalization is now done in parallel
  perr << "Updating bands.\n";
  // System().DiagonalizeH();
  System().DoMDExtrap();
  System().SolveLDA();  

  for (int band=0; band<NumFilled; band++) {
    System().SetRealSpaceBandNum(band);
    for (int ix=0; ix<xGrid.NumPoints-1; ix++)
      for (int iy=0; iy<yGrid.NumPoints-1; iy++)
	for (int iz=0; iz<zGrid.NumPoints-1; iz++)
	  data(ix,iy,iz,band) = System().RealSpaceBand(ix,iy,iz);
  }
  MakePeriodic (data);
  Path.Communicator.Broadcast(0, data);
  
  // DEBUG DEBUG DEBUG DEBUG
//   for (int band=0; band<NumBands; band++) {
//     char fname[100];
//     int proc = 
//     snprintf (fname, 100, "band%d-%d.dat", band, Path.Communicator.MyProc());
//     FILE *fout = fopen (fname, "w");
//     for (int ix=0; ix<xGrid.NumPoints; ix++)
//       for (int iy=0; iy<yGrid.NumPoints; iy++)
// 	for (int iz=0; iz<zGrid.NumPoints; iz++)
// 	  fprintf (fout, "%1.12e\n", data(ix,iy,iz,band));
//     fclose (fout);
//   }

  if (MonteCarloIons)
    BandSplines().Init (&xGrid, &yGrid, &zGrid, data, true);
  else {
    BandSplines[0].Init (&xGrid, &yGrid, &zGrid, data, true);
    BandSplines[1].Init (&xGrid, &yGrid, &zGrid, data, true);
  }
#endif
}

void
FixedPhaseClass::UpdateCache()
{
#if NDIM==3
  clock_t start, stop;
  perr << "Starting cache update.\n";
  start = clock();
  int Nup    = Path.Species(UpSpeciesNum).NumParticles;
  int Ndown = Path.Species(DownSpeciesNum).NumParticles;
  Array<complex<double>,1> vals;
  Array<cVec3,1> grads;
  int upFirst   = Path.Species(UpSpeciesNum).FirstPtcl;
  int downFirst = Path.Species(DownSpeciesNum).FirstPtcl;
  for (int slice=0; slice<Path.NumTimeSlices(); slice++) {
    for (int j=0; j < Nup; j++) {
      vals.reference(UpMatrixCache[GetMode()](slice, j, Range::all()));
      grads.reference(UpGradMatCache[GetMode()](slice, j, Range::all()));
      Vec3 r_j = Path(slice, j+upFirst);
      Path.PutInBox(r_j);
      BandSplines().FValGrad(r_j[0], r_j[1], r_j[2], vals, grads);
    }

    for (int j=0; j < Ndown; j++) {
      vals.reference (DownMatrixCache[GetMode()](slice, j, Range::all()));
      grads.reference(DownGradMatCache[GetMode()](slice, j, Range::all()));
      Vec3 r_j = Path(slice, j+downFirst);
      Path.PutInBox(r_j);
      BandSplines().FValGrad(r_j[0], r_j[1], r_j[2], vals, grads);
    }
  }
//   UpMatrixCache   [1] = UpMatrixCache   [0];
//   UpGradMatCache  [1] = UpGradMatCache  [0];
//   DownMatrixCache [1] = DownMatrixCache [0];
//   DownGradMatCache[1] = DownGradMatCache[0];

  Action (0, Path.NumTimeSlices()-1, Path.Species(IonSpeciesNum).Ptcls, 0, IonSpeciesNum);
//   UpAction.AcceptCopy();
//   DownAction.AcceptCopy();
  
  stop = clock();
  perr << "Finished cache update.  Time = "
       << (double)(stop-start)/(double)CLOCKS_PER_SEC << " seconds.\n";
#endif
}


bool
FixedPhaseClass::IsPositive (int slice, int speciesNum)
{
  if (IonsHaveMoved()) 
    UpdateBands();

  return true;
}

complex<double>
FixedPhaseClass::Det (int slice, int speciesNum)
{
#if NDIM==3
  if (IonsHaveMoved()) 
    UpdateBands();

  SpeciesClass& species = Path.Species(speciesNum);
  int first = species.FirstPtcl;
  int N = species.NumParticles;
  // First, fill up determinant matrix
  Array<complex<double>,1> vals;
  for (int j=0; j<N; j++) {
    Vec3 r_j = Path(slice, first+j);
    Path.PutInBox(r_j);
    vals.reference(Matrix(j,Range::all()));
    BandSplines()(r_j[0], r_j[1], r_j[2], vals);
  }
  return Determinant (Matrix);
#endif
}

void
FixedPhaseClass::GetIonForces (Array<Vec3,1> &F)
{
  PathClass &Path = PathData.Path;
  SpeciesClass &ionSpecies = Path.Species(IonSpeciesNum);
  PairActionFitClass& paIonIon = 
    *PathData.Actions.PairMatrix(IonSpeciesNum, IonSpeciesNum);

  // First get the force on the ions from the electrons
  System().CalcIonForces(F);

  // The following contributions are now done from within the plane
  // wave part of the code.

//   int first = ionSpecies.FirstPtcl;
//   int last  = ionSpecies.LastPtcl;

//   // Now, add on the ion-ion forces
//   // Short-range part
//   for (int ptcl1=first; ptcl1<=last; ptcl1++) {
//     Vec3 r1 = Rions(ptcl1-first);
//     for (int ptcl2=ptcl1+1; ptcl2 <= ionSpecies.LastPtcl; ptcl2++) {
//       Vec3 r2  = Rions(ptcl2-first);
//       Vec3 disp = r2-r1;
//       Path.PutInBox (disp);
//       double dist = sqrt(dot(disp, disp));
//       disp = (1.0/dist)*disp;
//       double dVshort = paIonIon.Vp(dist) - paIonIon.Vlong.Deriv(dist);
//       // DO I HAVE THESE SIGNS RIGHT????????????????
//       F(ptcl1-first) += dVshort *disp;
//       F(ptcl2-first) -= dVshort *disp;
//     }
//   }
//   // Long-range part
//   for (int ptcl=first; ptcl<=last; ptcl++) {
//     for (int ki=0; ki<Path.kVecs.size(); ki++) {
//       dVec r = Rions(ptcl-first);
//       dVec G = Path.kVecs(ki);
//       double phi = dot(r,G);
//       double s, c;
//       sincos(phi, &s, &c);
//       complex<double> z(c,s); // z = e^{iG dot r}
//       complex<double> rho_mk = conj(Path.Rho_k(0,IonSpeciesNum, ki));
//       // DO I HAVE THE SIGN RIGHT????????????????
//       F(ptcl-first) += 
// 	2.0*paIonIon.Vlong_k(ki)*imag(z*rho_mk)*G; 
//     }
//   }
      
    

}


bool
FixedPhaseActionClass::IsPositive (int slice)
{
  if (PathData.Path.GetConfig() == 0)
    return FixedPhaseA.IsPositive(slice, SpeciesNum);
  else
    return FixedPhaseB.IsPositive(slice, SpeciesNum);
}

complex<double>
FixedPhaseActionClass::Det (int slice)
{
  if (PathData.Path.GetConfig() == 0)
    return FixedPhaseA.Det(slice, SpeciesNum);
  else
    return FixedPhaseB.Det(slice, SpeciesNum);
}

double
FixedPhaseActionClass::SingleAction (int slice1, int slice2, 
				     const Array<int,1> &activeParticles,
				     int level)
{
  if (PathData.Path.GetConfig() == 0) return FixedPhaseA.Action 
    (slice1, slice2, activeParticles, level, SpeciesNum);
  else return FixedPhaseB.Action 
    (slice1, slice2, activeParticles, level, SpeciesNum);
}

double
FixedPhaseActionClass::d_dBeta(int slice1, int slice2, int level)
{
  if (PathData.Path.GetConfig() == 0)
    return FixedPhaseA.d_dBeta (slice1, slice2, level, SpeciesNum);
  else
    return FixedPhaseB.d_dBeta (slice1, slice2, level, SpeciesNum);
}


void
FixedPhaseActionClass::Update()
{
  if (PathData.Path.GetConfig() == 0) {
    if (FixedPhaseA.IonsHaveMoved()) {
      FixedPhaseA.UpdateBands();
      FixedPhaseA.UpdateCache();
    }
  }  
  else if (FixedPhaseB.IonsHaveMoved()) {
    FixedPhaseB.UpdateBands();
    FixedPhaseB.UpdateCache();
  }
}


void 
FixedPhaseActionClass::Init()
{
  FixedPhaseA.Init (SpeciesNum);
  if (&FixedPhaseB != &FixedPhaseA)
    FixedPhaseB.Init (SpeciesNum);
}

bool
FixedPhaseActionClass::IsGroundState()
{
  return true;
}


NodeType
FixedPhaseActionClass::Type()
{
  return GROUND_STATE_FP;
}

void
FixedPhaseActionClass::WriteInfo (IOSectionClass &out)
{
  out.WriteVar ("Type", "GROUND_STATE_FP");
  Array<double,1> outkVec(3);
  outkVec(0) = Getk()[0];  outkVec(1) = Getk()[1];  outkVec(2) = Getk()[2];
  out.WriteVar ("kVec", outkVec);
  double energy = 0.0;
  if ((SpeciesNum==FixedPhaseA.UpSpeciesNum) || (SpeciesNum == FixedPhaseA.DownSpeciesNum))
    for (int i=0; i<FixedPhaseA.System().GetNumBands(); i++) {
      cerr << "Band Energy = " << FixedPhaseA.System().GetEnergy(i) << endl;
      energy += FixedPhaseA.System().GetEnergy(i);
    }
  out.WriteVar ("Energy", energy);
//   int nx = FixedPhase.BandSplines().Nx;
//   int ny = FixedPhase.BandSplines().Ny;
//   int nz = FixedPhase.BandSplines().Nz;
//   int n  = FixedPhase.BandSplines().N;
//   Array<double,4> Bands(nx, ny, nz, 2*n);
//   for (int ix=0; ix<nx; ix++)
//     for (int iy=0; iy<ny; iy++)
//       for (int iz=0; iz<nz; iz++)
// 	for (int i=0; i<n; i++) {
// 	  Bands(ix,iy,iz,2*i)   = FixedPhase.BandSplines()(ix,iy,iz,i).real();
// 	  Bands(ix,iy,iz,2*i+1) = FixedPhase.BandSplines()(ix,iy,iz,i).imag();
// 	}
//   out.WriteVar("Bands", Bands);
//   Vec3 r;
//   for (int ix=0; ix<nx; ix++) {
//     r[0] = (*FixedPhase.BandSplines().Xgrid)(ix);
//     for (int iy=0; iy<ny; iy++) {
//       r[1] = (*FixedPhase.BandSplines().Ygrid)(iy);
//       for (int iz=0; iz<nz; iz++) {
// 	r[2] = (*FixedPhase.BandSplines().Zgrid)(iz);
// 	double phi = dot(Getk(), r);
// 	for (int i=0; i<n; i++) {
// 	  Bands(ix,iy,iz,2*i)   = cos(phi);
// 	  Bands(ix,iy,iz,2*i+1) = sin(phi);
// 	}
//       }
//     }
//   }
//   out.WriteVar("e2ikr", Bands);
  out.FlushFile();
}

void
FixedPhaseActionClass::Setk(Vec3 k)
{
  FixedPhaseA.Setk(k);
  if (&FixedPhaseB != &FixedPhaseA)
    FixedPhaseB.Setk(k);
}


double 
FixedPhaseActionClass::CalcGrad2 (int slice, const Array<int,1> &activeParticles,
				  UpdateType update) 
{ 
  if (PathData.Path.GetConfig() == 0)
    return FixedPhaseA.CalcGrad2(slice, SpeciesNum, activeParticles, update); 
  else
    return FixedPhaseB.CalcGrad2(slice, SpeciesNum, activeParticles, update); 
}


void
FixedPhaseActionClass::ShiftData (int slices2Shift)
{
  FixedPhaseA.ShiftData (slices2Shift, SpeciesNum);
  if (&FixedPhaseB != &FixedPhaseA)
    FixedPhaseB.ShiftData (slices2Shift, SpeciesNum);
}

void
FixedPhaseActionClass::MoveJoin (int oldJoinPos, int newJoinPos)
{
  FixedPhaseA.MoveJoin (oldJoinPos, newJoinPos, SpeciesNum);
  if (&FixedPhaseB != &FixedPhaseA)
    FixedPhaseB.MoveJoin (oldJoinPos, newJoinPos, SpeciesNum);
}

void
FixedPhaseActionClass::AcceptCopy (int slice1, int slice2)
{
  FixedPhaseA.AcceptCopy (slice1, slice2, SpeciesNum);
  if (&FixedPhaseB != &FixedPhaseA)
    FixedPhaseB.AcceptCopy (slice1, slice2, SpeciesNum);
}

void
FixedPhaseActionClass::RejectCopy (int slice1, int slice2)
{
  FixedPhaseA.RejectCopy (slice1, slice2, SpeciesNum);
  if (&FixedPhaseB != &FixedPhaseA)
    FixedPhaseB.RejectCopy (slice1, slice2, SpeciesNum);
}



void
FixedPhaseActionClass::CalcDensity(Array<double,3> &rho)
{
  if (PathData.Path.GetConfig() == 0)
    FixedPhaseA.CalcDensity(rho);
  else
    FixedPhaseB.CalcDensity(rho);
}

const Array<double,3>&
FixedPhaseActionClass::GetDensity()
{
  if (PathData.Path.GetConfig() == 0)
    return FixedPhaseA.GetDensity();
  else
    return FixedPhaseB.GetDensity();
}

void
FixedPhaseActionClass::CalcBandDensity(Array<double,4> &rho)
{
  if (PathData.Path.GetConfig() == 0)
    FixedPhaseA.CalcBandDensity(rho);
  else
    FixedPhaseB.CalcBandDensity(rho);
}

void
FixedPhaseActionClass::GetBandEnergies(Array<double,1> &energies)
{
  if (PathData.Path.GetConfig() == 0)
    FixedPhaseA.GetBandEnergies(energies);
  else
    FixedPhaseB.GetBandEnergies(energies);
}

void
FixedPhaseActionClass::GetIonForces(Array<Vec3,1> &F)
{
  // Make sure wave functions are up to date.
  Update();
  if (PathData.Path.GetConfig() == 0)
    FixedPhaseA.GetIonForces(F);
  else
    FixedPhaseB.GetIonForces(F);
}


string
FixedPhaseActionClass::GetName()
{
  string speciesName = Path.Species(SpeciesNum).Name;
  return "FixedPhase(" + speciesName + ")";
}
