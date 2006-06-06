#include "PlaneWavesMPI.h"
#include "../MatrixOps/MatrixOps.h"
#include "../MPI/Communication.h"
#include "../DFT/Functionals.h"

void
MPISystemClass::InitLDA()
{
  int NDelta = GVecs.DeltaSize();
  FFT.GetDims    (Nx, Ny, Nz);
  NewRho.resize  (Nx, Ny, Nz);
  TempRho.resize (Nx, Ny, Nz);
  VH.resize      (Nx, Ny, Nz);
  VXC.resize     (Nx, Ny, Nz);
  VHXC.resize    (Nx, Ny, Nz);
  Rho_r.resize   (Nx, Ny, Nz);
  h_G.resize     (NDelta);
  Rho_G.resize   (NDelta);
  CalcRadialChargeDensity();
}


void
MPISystemClass::SolveLDA()
{
  int numSCIters = 0;
  bool SC = false;
  while (!SC) {
    CalcOccupancies();
    CalcChargeDensity();
    MixChargeDensity();
    CalcVHXC();
    CG.SetTolerance(1.0e-6);

    numSCIters ++;
  }

}

void
MPISystemClass::CalcOccupancies()
{


}

void
MPISystemClass::CalcChargeDensity()
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
  for (int bi=CG.GetFirstBand(); bi<CG.GetLastBand(); bi++) {
    band.reference(Bands(bi, Range::all()));
    FFT.PutkVec (band);
    FFT.k2r();
    for (int ix=0; ix<nx; ix++)
      for (int iy=0; iy<ny; iy++)
	for (int iz=0; iz<nz; iz++)
	  TempRho(ix,iy,iz) += Occupancies(bi)*norm (FFT.rBox(ix,iy,iz));
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
  double prefactor = 2.0*volInv/((double)kComm.NumProcs()*(nx*ny*nz));
  NewRho *= prefactor;
}



/// Calculate the hartree, exchange, and correlation potentials.
/// We assume that upon entry, Rho_r contains the charge density 
/// in real space and Rho_G contains it in reciporical space.
/// On exit, VH will contain the Hartree potential and VXC will
/// contain the exchange-correlation potential, both in real space.
void 
MPISystemClass::CalcVHXC()
{
  ///////////////////////
  // Hartree potential //
  ///////////////////////
  h_G = Rho_G;
  // Compute V_H in reciporical space
  double volInv = 1.0/H.GVecs.GetBoxVol();
  double hartreeTerm = 0.0;
  double prefact = (4.0*M_PI/H.GVecs.GetBoxVol());
  EH = 0.0;
  for (int i=0; i<h_G.size(); i++) {
    h_G(i) *= prefact*H.GVecs.DeltaGInv2(i);
    EH += norm(h_G(i)) * H.GVecs.DeltaGInv2(i);
  }
  EH *= 0.5*prefact;

  // FFT back to real space
  FFT.PutkVec(h_G);
  FFT.k2r();
  copy (real(FFT.rBox), VH);

  ////////////////////////////////////
  // Exchange-correlation potential //
  ////////////////////////////////////
  EXC = 0.0;
  double exc;
  for (int ix=0; ix<Rho_r.extent(0); ix++)
    for (int iy=0; iy<Rho_r.extent(1); iy++)
      for (int iz=0; iz<Rho_r.extent(2); iz++) {
	FortranExCorr (Rho_r(ix,iy,iz), exc, VXC(ix,iy,iz));
	EXC += exc;
      }
  VHXC = VH + VXC;
}


void
MPISystemClass::CalcRadialChargeDensity()
{
  cerr << "Calculating atomic charge density:\n";
  double maxBox = max(Box[0], max(Box[1], Box[2]));
  double rmax = max (maxBox, 100.0);
  AtomGrid.Init (1.0, rmax);
  DFTAtom atom;
  atom.NewMix = 0.5;
  atom.RadialWFs.resize(1);
  atom.RadialWFs(0).n = 1;
  atom.RadialWFs(0).l = 0;
  atom.RadialWFs(0).Occupancy = 1.0;
  atom.RadialWFs(0).Energy = -0.15;
  atom.SetGrid (&AtomGrid);
  atom.SetBarePot (PH);
  //atom.SolveInit();
  atom.Solve();
  Array<double,1> rho(AtomGrid.NumPoints);
  for (int i=0; i<rho.size(); i++) {
    double r = AtomGrid(i);
    double u = atom.RadialWFs(0).u(i);
    rho(i) = u*u/(r*r);
  }
  RadialChargeDensity.Init (&AtomGrid, rho);
  cerr << "Done atomic charge density.\n";
}

/// Initializes the density to a superposition of the atomic charge
/// densities from an Atomic DFT calculation.
void
MPISystemClass::InitRho_r()
{
  Rho_r = 0.0;
  double nxInv = 1.0/(double)(Nx-1);
  double nyInv = 1.0/(double)(Ny-1);
  double nzInv = 1.0/(double)(Nz-1);
  Vec3 boxInv = Vec3(1.0/Box[0], 1.0/Box[1], 1.0/Box[2]);
  Vec3 r;
  double chargePerAtom = (double)NumElecs/(double)Rions.size();
  double totalCharge = 0.0;
  double cellVol = Box[0]*Box[1]*Box[2]/(double)(Nx*Ny*Nz);
  for (int ix=0; ix<Nx; ix++) {
    r[0] = (nxInv*(double)ix-0.5)*Box[0];
    for (int iy=0; iy<Ny; iy++) {
      r[1] = (nyInv* (double)iy-0.5)*Box[1];
      for (int iz=0; iz<Nz; iz++) {
	r[2] = (nzInv*(double)iz-0.5)*Box[2];
	for (int ion=0; ion<Rions.size(); ion++) {
	  Vec3 disp = r-Rions(ion);
	  /// Find nearest image
	  disp[0] -= nearbyint(disp[0]*boxInv[0])*Box[0];
	  disp[1] -= nearbyint(disp[1]*boxInv[1])*Box[1];
	  disp[2] -= nearbyint(disp[2]*boxInv[2])*Box[2];
	  double dist = sqrt(dot(disp, disp));
	  Rho_r(ix,iy,iz) += chargePerAtom*RadialChargeDensity(dist);
	}
	totalCharge += Rho_r(ix,iy,iz);
      }
    }
  }
  totalCharge *= cellVol;
  Rho_r *= (double)NumElecs/totalCharge;
}
	 
void 
MPISystemClass::MixChargeDensity()
{

}



// double
// PlaneWavesMPI::CalcHartreeTerm(int band)
// {
//   zVec psi;
//   psi.reference (Bands(band, Range::all()));
//   FFT.PutkVec (Phip);
//   FFT.k2r();
//   copy (FFT.rBox, Phip_r);
// //   for (int ix=0; ix<Nx; ix++)
// //     for (int iy=0; iy<Ny; iy++)
// //       for (int iz=0; iz<Nz; iz++)
// // 	Phip_r(ix,iy,iz) = FFT.rBox(ix,iy,iz);
//   FFT.PutkVec (psi);
//   FFT.k2r();
  
//   for (int ix=0; ix<Nx; ix++)
//     for (int iy=0; iy<Ny; iy++)
//       for (int iz=0; iz<Nz; iz++)
// 	FFT.rBox(ix,iy,iz) *= conj(Phip_r(ix,iy,iz));
//   FFT.r2k();
//   FFT.GetkVec(h_G);
//   double volInv = 1.0/H.GVecs.GetBoxVol();
//   double e2_over_eps0 = 4.0*M_PI; // ???????
//   double hartreeTerm = 0.0;
//   for (int i=0; i<h_G.size(); i++)
//     hartreeTerm += norm(h_G(i))*H.GVecs.DeltaGInv2(i);
//   hartreeTerm *= 2.0 * e2_over_eps0 * volInv;
//   return hartreeTerm;
// }


