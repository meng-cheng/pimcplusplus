#ifndef PLANE_WAVES_H
#define PLANE_WAVES_H

#include "ConjGrad2.h"

class SystemClass
{
protected:
  Array<Vec3,1> Rions;
  FFTBox FFT;
  HamiltonianClass H;
  Array<complex<double>,2> Bands;
  ConjGrad CG;
  Vec3 Box;
  int NumBands;
public:
  GVecsClass GVecs;
  void Setup(Vec3 box, Vec3 k, double kcut, Potential &ph, bool useFFT=true);
  void Setup(Vec3 box, Vec3 k, double kcut, double z, bool useFFT=true);
  void SetIons (const Array<Vec3,1> &rions);
  void Setk (Vec3 k);
  void DiagonalizeH();
  void CalcChargeDensity(Array<double,3> &rho);
  void WriteXSFFile(string filename);

  SystemClass(int numBands) 
    : CG(H, Bands), FFT(GVecs), H(GVecs, FFT), NumBands(numBands)
  {

  }
};

#endif
