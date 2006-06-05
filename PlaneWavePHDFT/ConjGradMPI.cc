#include "ConjGradMPI.h"
#include "../MatrixOps/MatrixOps.h"
#include "../MPI/Communication.h"

void ConjGradMPI::Setup()
{
  int numBands = Bands.rows();
  int N      = H.GVecs.size();
  int NDelta = H.GVecs.DeltaSize();

  // Figure out which bands I'm responsible for
  int numProcs = BandComm.NumProcs();
  int myProc = BandComm.MyProc();
  int band = 0;
  for (int proc=0; proc<numProcs; proc++) {
    int procBands = numBands/numProcs + ((numBands % numProcs)>proc);
    if (proc==myProc) {
      MyFirstBand = band;
      MyLastBand = band + procBands-1;
    }
    band += procBands;
  }

  // Now allocate memory for everything
  Bands.resize (numBands, N);
  lastPhis.resize(numBands, N);
  Energies.resize(numBands);
  Residuals.resize(numBands);
  EtaXiLast.resize(numBands);
  EtaXiLast = complex<double>(0.0, 0.0);
  cnext.resize (N);
  Hc.resize (N);
  Phi.resize (N);
  Phip.resize(N);
  Phipp.resize(N);
  Xi.resize(N);
  Eta.resize(N);
  T.resize(N);
  if (UseLDA) {
    FFT.GetDims    (Nx, Ny, Nz);
    Phip_r.resize  (Nx, Ny, Nz);
    Psi_r.resize   (Nx, Ny, Nz);
    NewRho.resize  (Nx, Ny, Nz);
    TempRho.resize (Nx, Ny, Nz);
    VH.resize      (Nx, Ny, Nz);
    VXC.resize     (Nx, Ny, Nz);
    Rho.resize(Nx, Ny, Nz);
    h_G.resize(NDelta);
  }
    

  Bands = 0.0;
  Vec3 kBox = H.GVecs.GetkBox();
  double maxk = max(kBox[0], max(kBox[1], kBox[2]));
  double maxE = 1.0*maxk*maxk;
  int numNonZero;
  for (int band=0; band<numBands; band++) {
    numNonZero = 0;
    c.reference(Bands(band,Range::all()));
    c = 0.0;
    for (int i=0; i<N; i++) {
      double dist2 = dot(H.GVecs(i), H.GVecs(i));
      c(i) = exp(-2.0*dist2);
    }
    Normalize(c);
  }
  InitBands();
  IsSetup = true;
}

void ConjGradMPI::InitBands()
{
  int numBands = Bands.rows();
  int numVecs = 8 * numBands;
  assert (numVecs <= H.GVecs.size());
  Array<complex<double>,2> Hmat(numVecs, numVecs);
  Array<complex<double>,2> EigVecs(numBands, numVecs);
  Array<double,1> EigVals (numBands);

  H.Vion->Vmatrix(Hmat);
  perr << "kPoint = " << H.kPoint << endl;
  for (int i=0; i<numVecs; i++) {
    Vec3 Gpk = H.GVecs(i) + H.kPoint;
    Hmat(i,i) += 0.5 * dot (Gpk,Gpk);
  }
  SymmEigenPairs (Hmat, numBands, EigVals, EigVecs);

  if (BandComm.MyProc() == 0)
    for (int i=0; i<numBands; i++)
      perr << "Mini energy(" << i << ") = " << 27.211383*EigVals(i) << endl;

  // Now put results in Bands
  for (int band=0; band<numBands; band++)
    for (int vec=0; vec<Bands.cols(); vec++)
      Bands(band,vec) = 1.0e-6*(drand48()-0.5);
  for (int band=0; band<numBands; band++) 
    for (int i=0; i<numVecs; i++)
      Bands(band, i) = EigVecs(band, i);
  GramSchmidt(Bands);
  // We have to broadcast in order to make sure everyone is starting
  // with the exact same bands.
  BandComm.Broadcast(0,Bands);
}

double ConjGradMPI::CalcPhiSD()
{
  H.Apply(c, Hc);
  E0 = realconjdot (c, Hc);
  perr << "E = " << E0 << endl;
  Phip = E0*c - Hc;
  double nrm = norm (Phip);
  Normalize (Phip);  
  return nrm;
}

void ConjGradMPI::Precondition()
{
  double Tinv = 1.0/T(CurrentBand);
  Vec3 k = H.kPoint;
  for (int i=0; i<c.size(); i++) {
    double x = 0.5*dot(H.GVecs(i)+k, H.GVecs(i)+k)*Tinv;
    double num = 27.0 + 18.0*x +12.0*x*x + 8.0*x*x*x;
    double denom = num + 16.0*x*x*x*x;
    Eta(i) = (num/denom)* Xi(i);
  }
}

// Returns the norm of the residual Hc - Ec
double 
ConjGradMPI::CalcPhiCG()
{
  //  Hc.reference(Hcs(CurrentBand, Range::all()));
  Hc = 0.0;
  H.Kinetic.Apply (c, Hc);
  T = realconjdot (c, Hc);
  if (UseLDA)
    H.Vion->Apply (c, Hc, VHXC);
  else
    H.Vion->Apply (c, Hc);
  E0 = realconjdot (c, Hc);
  // Steepest descent direction: (5.10)
  Xi = E0*c - Hc;
  double residualNorm = norm (Xi);
  /// Orthonalize to other bands lower than me here (5.12)
  zVec &Xip = Xi;
  OrthogLower (Bands, Xip, CurrentBand);

  Precondition();

  // Now, orthogonalize to all bands, (including present band);
  // rename for clarity (5.18)
  zVec &Etap = Eta;
  OrthogLower(Bands, Etap, CurrentBand+1);

  // Compute conjugate direction (5.20)
  complex<double> etaxi = conjdot(Etap, Xip);
  complex<double> gamma; 
  if (EtaXiLast(CurrentBand) != complex<double>(0.0, 0.0)) 
    gamma = etaxi/EtaXiLast(CurrentBand);
  else
    gamma = 0.0;

  EtaXiLast(CurrentBand) = etaxi;
  
  // (5.19)
  Phi = Etap + gamma * lastPhis(CurrentBand,Range::all());
  lastPhis(CurrentBand,Range::all()) = Phi;

  // Orthogonalize to all lower bands band: (5.21)
  Phip = Phi;
  //  OrthogExcluding(Bands, Phip, -1);
  OrthogLower(Bands, Phip, CurrentBand+1);
  // (5.22)
  Normalize (Phip);

  Energies(CurrentBand) = E0;
  return residualNorm;
}

inline double max (const Array<double,1> &v)
{
  double mval = v(0);
  for (int i=1; i<v.size(); i++)
    if (v(i) > mval)
      mval = v(i);
  return mval;
}

double
ConjGradMPI::Iterate()
{
  if (!IsSetup)
    Setup();
  for (int band=MyFirstBand; band<=MyLastBand; band++) {
    CurrentBand = band;
    c.reference     (Bands(band, Range::all()));
    Residuals(band) = CalcPhiCG();

    // Now, pick optimal theta for 
    double dE_dtheta = 2.0*realconjdot(Phip, Hc);
    if (UseLDA)
      H.Apply (Phip, Hc, VHXC);
    else
      H.Apply (Phip, Hc);
    double d2E_dtheta2 = 2.0*(realconjdot(Phip, Hc) - E0);
    double thetaMin = 0.5*atan(-dE_dtheta/(0.5*d2E_dtheta2));

    double costhetaMin, sinthetaMin;
    sincos(thetaMin, &sinthetaMin, &costhetaMin);
    c = costhetaMin*c + sinthetaMin*Phip;
  }
  CollectBands();
  GramSchmidt(Bands);
  // CheckOverlaps();
  return max(Residuals);
}

void ConjGradMPI::Solve()
{
  /// Hamiltonian has changed, so we can't assume we're conjugate to
  /// previous directions.
  EtaXiLast = complex<double>(0.0, 0.0);
  int iter = 0;
  double residual = 1.0;
  while ((iter < 100) && (residual > Tolerance)) {
    residual = Iterate();
    iter++;
  }

  if (residual > Tolerance)
    perr << "Warning:  conjugate gradient residual norm = " 
	 << residual << endl;
  perr << "# of iterations = " << iter << endl;
}

void
ConjGradMPI::PrintOverlaps()
{
  perr << "Overlaps = \n";
  zVec x, y;
  for (int i=0; i<Bands.rows(); i++) {
    x.reference (Bands(i, Range::all()));
    for (int j=0; j<Bands.rows(); j++) {
      y.reference (Bands(j,Range::all()));
      double s = realconjdot(x,y);
      fprintf (stderr, "%12.6e ", s);
    }
    fprintf (stderr, "\n");
  }
}

void 
ConjGradMPI::CheckOverlaps()
{
  zVec x, y;
  for (int i=0; i<Bands.rows(); i++) {
    x.reference (Bands(i, Range::all()));
    for (int j=0; j<Bands.rows(); j++) {
      y.reference (Bands(j,Range::all()));
      double s = realconjdot(x,y);
      if (i==j)
	assert (fabs(s-1.0)<1.0e-12);
      else
	if (fabs(s) > 1.0e-12) {
	  perr << "Overlap(" << i << "," << j << ") = " << s << endl;
	  abort();
	}
     }
   }
}

void
ConjGradMPI::CollectBands()
{
  BandComm.AllGatherRows(Bands);
  BandComm.AllGatherVec(Residuals);
  BandComm.AllGatherVec(Energies);
}


void
ConjGradMPI::CalcChargeDensity()
{
  int nx, ny, nz;
  FFT.GetDims(nx,ny,nz);
  if (TempRho.shape() != TinyVector<int,3>(nx,ny,nz))
    TempRho.resize(nx,ny,nz);
  if (NewRho.shape() != TinyVector<int,3>(nx,ny,nz))
    NewRho.resize(nx,ny,nz);
  TempRho = 0.0;
  zVec band;

  /// We assume that each of the bands is already normalized.
  for (int bi=MyFirstBand; bi<MyLastBand; bi++) {
    band.reference(Bands(bi, Range::all()));
    FFT.PutkVec (band);
    FFT.k2r();
    for (int ix=0; ix<nx; ix++)
      for (int iy=0; iy<ny; iy++)
	for (int iz=0; iz<nz; iz++)
	  TempRho(ix,iy,iz) += norm (FFT.rBox(ix,iy,iz));
  }
  /// First, sum over all the band procs
  BandComm.AllSum (TempRho, NewRho);
  TempRho = NewRho;
  // Now sum over all the kPoints
  if (BandComm.MyProc() == 0) 
    kComm.AllSum(TempRho, NewRho);
  // And broadcast;
  BandComm.Broadcast(0, NewRho);
  /// Multiply by the right prefactor;
  double volInv = 1.0/H.GVecs.GetBoxVol();
  double prefactor = 2.0*volInv/(double)kComm.NumProcs();
  NewRho *= prefactor;
}

template<typename T1,typename T2>
inline void copy(Array<T1,3> &src,
		 Array<T2,3> &dest)
{
  assert (src.shape() == dest.shape());
  for (int ix=0; ix<src.extent(0); ix++)
    for (int iy=0; iy<src.extent(1); iy++)
      for (int iz=0; iz<src.extent(2); iz++)
	dest(ix,iy,iz) = src(ix,iy,iz);
}


/// Calculate the hartree, exchange, and correlation potentials.
/// We assume that upon entry, Rho contains the charge density for
/// which we wish to calculate V_H and V_XC.
void 
ConjGradMPI::CalcVHXC()
{
  ///////////////////////
  // Hartree potential //
  ///////////////////////
  // FFT charge density into k-space
  copy (Rho, FFT.rBox);
  FFT.r2k();
  FFT.GetkVec(h_G);
  // Compute V_H in reciporical space
  double volInv = 1.0/H.GVecs.GetBoxVol();
  double hartreeTerm = 0.0;
  for (int i=0; i<h_G.size(); i++)
    h_G(i) = norm(h_G(i))*H.GVecs.DeltaGInv2(i);
  h_G *= (4.0*M_PI/H.GVecs.GetBoxVol());
  // FFT back to real space
  FFT.PutkVec(h_G);
  FFT.k2r();
  copy (FFT.rBox, VH);

  ////////////////////////////////////
  // Exchange-correlation potential //
  ////////////////////////////////////
  

  
}

double
ConjGradMPI::CalcHartreeTerm(int band)
{
  zVec psi;
  psi.reference (Bands(band, Range::all()));
  FFT.PutkVec (Phip);
  FFT.k2r();
  copy (FFT.rBox, Phip_r);
//   for (int ix=0; ix<Nx; ix++)
//     for (int iy=0; iy<Ny; iy++)
//       for (int iz=0; iz<Nz; iz++)
// 	Phip_r(ix,iy,iz) = FFT.rBox(ix,iy,iz);
  FFT.PutkVec (psi);
  FFT.k2r();
  
  for (int ix=0; ix<Nx; ix++)
    for (int iy=0; iy<Ny; iy++)
      for (int iz=0; iz<Nz; iz++)
	FFT.rBox(ix,iy,iz) *= conj(Phip_r(ix,iy,iz));
  FFT.r2k();
  FFT.GetkVec(h_G);
  double volInv = 1.0/H.GVecs.GetBoxVol();
  double e2_over_eps0 = 4.0*M_PI; // ???????
  double hartreeTerm = 0.0;
  for (int i=0; i<h_G.size(); i++)
    hartreeTerm += norm(h_G(i))*H.GVecs.DeltaGInv2(i);
  hartreeTerm *= 2.0 * e2_over_eps0 * volInv;
  return hartreeTerm;
}


