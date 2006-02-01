#include "CoulombFFT.h"

void 
CoulombFFTClass::SetVr()
{
  FFT.kBox = 0.0;
  Vec3 Gdiff;
  int nx, ny, nz;
  FFT.GetDims(nx, ny, nz);
  double prefact = -4.0*M_PI*Z/GVecs.GetBoxVol();
  for (int i=0; i<GVecs.DeltaSize(); i++) {
    Vec3 dG = GVecs.DeltaG(i);
    FFT.kBox(GVecs.DeltaI(i)) = StructureFactor(i)* prefact/dot(dG,dG);
  }
  FFT.kBox(0,0,0) = 0.0;
  FFT.k2r();
  Vr.resize(nx, ny, nz);
  Vr = FFT.rBox;
}

void 
CoulombFFTClass::Setup()
{
  SetVr();
  IsSetup = true;
}


void 
CoulombFFTClass::Vmatrix (Array<complex<double>,2> &vmat)
{
  double volInv = 1.0/GVecs.GetBoxVol();
  for (int i=0; i<vmat.rows(); i++) 
    for (int j=0; j<=i; j++) {
      Vec3 diff = GVecs(i) - GVecs(j);
      complex<double> s(0.0,0.0);
      for (int zi=0; zi<Rions.size(); zi++) {
	double cosVal, sinVal, phase;
	phase = dot (diff, Rions(zi));
	sincos(phase, &sinVal, &cosVal);
	s += complex<double> (cosVal,sinVal);
      }
      complex<double> val = -4.0*volInv*s*M_PI*Z/dot(diff,diff);
      vmat(i,j) = val;
      vmat(j,i) = conj(val);
    }
  for (int i=0; i<vmat.rows(); i++)
    vmat(i,i) = 0.0;
}


void 
CoulombFFTClass::SetIons(const Array<Vec3,1> &rions)
{
  VionBase::SetIons(rions);
  if (!IsSetup)
    Setup();
  else
    SetVr();
}

void 
CoulombFFTClass::Apply(const zVec &c, zVec &Vc)
{
  if (!IsSetup)
    Setup();

  int Nx, Ny, Nz;
  FFT.GetDims(Nx, Ny, Nz);
  double Ninv = 1.0/(Nx*Ny*Nz);
  // First, put c into FFTbox
  FFT.PutkVec (c);
  // Now, transform to real space
  FFT.k2r();
  // Now, multiply by V
  FFT.rBox *= Vr;
  // Transform back
  FFT.r2k();
  FFT.kBox *= Ninv;
  // And put into Vc
  FFT.AddToVec (Vc);
}
