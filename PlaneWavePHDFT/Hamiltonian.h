#ifndef HAMILTONIAN_H
#define HAMILTONIAN_H

#include "VectorOps.h"
#include "../PH/kSpacePH.h"
#include "FFTBox.h"



class HamiltonianBase
{
protected:
  GVecsClass &GVecs;
  bool IsSetup;
public:
  // Adds H*c to the Hc vector.  Does not zero Hc before accumulating
  virtual void Apply   (const zVec &c, zVec &Hc) = 0;
  virtual void SetIons (const Array<Vec3,1> rions) { }
  HamiltonianBase (GVecsClass &gvecs) 
    : GVecs(gvecs), IsSetup(false)
  {
    // do nothing for now
  }
};


class KineticClass : public HamiltonianBase
{
private:
  Array<double,1> halfG2;
  void Setup();
public:
  void Apply (const zVec &c, zVec &Kc);
  KineticClass (GVecsClass &gvecs) : 
    HamiltonianBase (gvecs)
  {
  }
};





class CoulombClass : public HamiltonianBase
{
private:
  double Z;
public:
  void Apply (const zVec &c, zVec &Hc);
  void SetIons (const Array<Vec3,1> &rions);
  CoulombClass (double z, GVecsClass &gvecs) : 
    HamiltonianBase (gvecs), Z(z)
  {
    // nothing for now
  }
};

class CoulombFFTClass : public HamiltonianBase
{
private:
  double Z;
  void Setup();
  Array<complex<double>,3> Vr;
public:
  FFTBox FFT;
  void Apply (const zVec &c, zVec &Hc);
  void SetIons (const Array<Vec3,1> &rions);

  CoulombFFTClass (double z, GVecsClass &gvecs) : 
    HamiltonianBase (gvecs), Z(z), FFT(gvecs)
  {
    // nothing for now
  }
};

class PHPotClass : public HamiltonianBase
{
private:
  kSpacePH kPH;
  void Setup();
  Array<complex<double>,2> VGGp;
public:
  void Apply (const zVec &c, zVec &Hc);
  void SetIons (const Array<Vec3,1> &rions);

  PHPotClass (Potential &ph, GVecsClass &gvecs) :
    HamiltonianBase (gvecs), kPH(ph)
  {

  }
};


class PHPotFFTClass : public HamiltonianBase
{
private:
  kSpacePH kPH;
  Array<cMat3,3> Fr;
  zVec Vc;
  zVec Vk, StructureFactor;
  zVecVec Gc;
  zMatVec Fk;
  Array<complex<double>,3> Vr;
  FFTBox      cFFT;
  FFTVecBox VecFFT;
  //  FFTMatBox MatFFT;
  Vec3 k;
public:
  void Setup();

  void Setk (Vec3 kvec);
  void Apply (const zVec &c, zVec &Hc);
  PHPotFFTClass (Potential &ph, GVecsClass &gvecs) :
    HamiltonianBase (gvecs), kPH(ph),
    cFFT(gvecs), VecFFT(gvecs),/* MatFFT(gvecs),*/
    k(0.0, 0.0, 0.0)
  {

  }
};


class Hamiltonian : public HamiltonianBase
{
private:
public:
  CoulombClass Coulomb;
  CoulombFFTClass CoulombFFT;
  KineticClass Kinetic;
  PHPotClass PH;
  PHPotFFTClass PHFFT;
  GVecsClass GVecs;
  void Apply (const zVec &c, zVec &Hc);
  Hamiltonian (Vec3 box, Vec3 k, double kcut, double z, Potential &ph) :
    Kinetic (GVecs), Coulomb(z, GVecs), HamiltonianBase(GVecs),
    CoulombFFT(z, GVecs), PH(ph, GVecs), PHFFT(ph, GVecs)
  {
    GVecs.Set (box, k, kcut);
  }
};


#endif
