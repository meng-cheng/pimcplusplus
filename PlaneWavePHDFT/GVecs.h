#ifndef GVECS_H
#define GVECS_H

#include "VectorOps.h"

class GVecsClass
{
protected:
  // This stores the actual G-Vectors
  Array<Vec3,1> GVecs;
  Array<Int3,1> Indices;
  // This stores the differences between g-vectors:
  Array<Vec3,1> GDiff;
  Array<Int3,1> IDiff;
  Vec3 Box, kBox;
  int Nx, Ny, Nz;
  double kCut;

public:
  void Set (Vec3 box, double kcut);

  inline Vec3 operator()(int i) const
  { return GVecs(i); }

  inline Int3 Index(int i) const
  { return Indices(i); }
  
  inline Vec3 DeltaG (int i) const
  { return GDiff(i); }

  inline Int3 DeltaI (int i) const
  { return IDiff(i); }

  inline int size()
  { return GVecs.size(); }

  inline int DeltaSize() 
  { return GDiff.size(); }

  inline double GetBoxVol() 
  { return Box[0]*Box[1]*Box[2]; }

  inline double GetkCut() 
  { return kCut; }

  inline Vec3 GetBox()
  { return Box; }

  inline Vec3 GetkBox()
  { return kBox; }

  inline bool Allowed (int ix, int iy, int iz) 
  {
    Vec3 G(kBox[0]*ix, kBox[1]*iy, kBox[2]*iz);
    return dot (G,G) < (kCut*kCut);
  }

  void GetFFTBoxSize (int &nx, int &ny, int &nz);
};


#endif

