#ifndef FFT_H
#define FFT_H

#include "../Blitz.h"
#include <fftw3.h>

#ifdef FFT_USE_SINGLE
#define FFT_FLOAT float
#define FFTNAME(name) fftwf_ ## name
#else
#define FFT_FLOAT double
#define FFTNAME(name) fftw_ ## name
#endif

#ifdef USE_FFTW2MKL
#define FFT_MALLOC malloc
#define FFT_FREE   free
#define FFT_EXTRA_MEM 16
inline complex<FFT_FLOAT>* FFTAlign (complex<FFT_FLOAT>* ptr)
{
  size_t offset = 16 - (size_t)((size_t)ptr)&0x0f;
  return (complex<FFT_FLOAT>*) ((size_t)ptr+offset);
}
#else
#define FFT_MALLOC FFTNAME(malloc)
#define FFT_FREE   FFTNAME(free)
#define FFT_EXTRA_MEM 0
inline complex<FFT_FLOAT>* FFTAlign (complex<FFT_FLOAT>* ptr)
{ return ptr; }
#endif

class FFT1D
{
private:
  complex<FFT_FLOAT> *rData, *kData;
  FFTNAME(plan) r2kPlan, k2rPlan;
  bool Allocated, InPlace;
  FFT_FLOAT sqrtNinv;
public:
  Array<complex<FFT_FLOAT>,1> rBox, kBox;

  void resize (int n);
  inline int size()
  { return rBox.size(); }
  void r2k();
  void k2r();

  FFT1D(bool inPlace=true) : Allocated(false), InPlace(inPlace)
  {
    // Do nothing for now
  }
  ~FFT1D()
  {
    if (Allocated) {
      FFT_FREE(rData);
      if (!InPlace)
	FFT_FREE(kData);
      FFTNAME(destroy_plan)(r2kPlan);
      FFTNAME(destroy_plan)(k2rPlan);
    }
  }
};

template<int DIM>
class FFTVec1D
{
private:
  TinyVector<complex<FFT_FLOAT>,DIM> *rData, *kData;
  FFTNAME(plan) r2kPlan, k2rPlan;
  bool Allocated, InPlace;
  FFT_FLOAT sqrtNinv;
public:
  Array<TinyVector<complex<FFT_FLOAT>,DIM>,1> rBox, kBox;

  void resize (int n)
  {
    if (Allocated) {
      FFT_FREE(rData);
      if (!InPlace)
	FFT_FREE(kData);
      FFTNAME(destroy_plan)(r2kPlan);
      FFTNAME(destroy_plan)(k2rPlan);
    }
    rData = (TinyVector<complex<FFT_FLOAT>,DIM>*) 
      FFT_MALLOC(DIM*sizeof(FFTNAME(complex))*n+FFT_EXTRA_MEM);
    if (!InPlace)
      kData = (TinyVector<complex<FFT_FLOAT>,DIM>*) 
	FFT_MALLOC(DIM*sizeof(FFTNAME(complex))*n+FFT_EXTRA_MEM);
    else
      kData = rData;
    
    Array<TinyVector<complex<FFT_FLOAT>,DIM>,1> *temp;
    temp = new Array<TinyVector<complex<FFT_FLOAT>,DIM>,1>
      (FFT_ALIGN(rData), shape(n), neverDeleteData);
    rBox.reference (*temp);
    delete temp;
    
    temp = new Array<TinyVector<complex<FFT_FLOAT>,DIM>,1>
      (FFT_ALIGN(kData), shape(n), neverDeleteData);
    kBox.reference(*temp);
    delete temp;
    
    r2kPlan =      
      FFTNAME(plan_many_dft) 
      (1, &n, DIM, reinterpret_cast<FFTNAME(complex)*>(FFT_ALIGN(rData)),
       &n, DIM, 1,  reinterpret_cast<FFTNAME(complex)*>(FFT_ALIGN(kData)), 
       &n, DIM, 1, 1, FFTW_MEASURE);
    assert (r2kPlan != NULL);
    k2rPlan = 
      FFTNAME(plan_many_dft) 
      (1, &n, DIM, reinterpret_cast<FFTNAME(complex)*>(FFT_ALIGN(kData)),
       &n, DIM, 1, reinterpret_cast<FFTNAME(complex)*>(FFT_ALIGN(rData)), 
       &n, DIM, 1, -1, FFTW_MEASURE);
    assert (k2rPlan != NULL);
    
    sqrtNinv = sqrt(1.0/(FFT_FLOAT)n);
    Allocated = true;
  }
  
  inline int size()
  { return rBox.size(); }
  void r2k() { FFTNAME(execute)(r2kPlan); }
  void k2r() { FFTNAME(execute)(k2rPlan); }
  
  FFTVec1D(bool inPlace=true) : Allocated(false), InPlace(inPlace)
  {
    // Do nothing for now
  }
  ~FFTVec1D()
  {
    if (Allocated) {
      FFT_FREE(rData);
      if (!InPlace)
	FFT_FREE(kData);
      FFTNAME(destroy_plan)(r2kPlan);
      FFTNAME(destroy_plan)(k2rPlan);
    }
  }
};



class FFT3D
{
private:
  complex<FFT_FLOAT> *rData, *kData;
  FFTNAME(plan) r2kPlan, k2rPlan;
  bool Allocated, InPlace;
  FFT_FLOAT sqrtNinv;
public:
  Array<complex<FFT_FLOAT>,3> rBox, kBox;

  void resize (int nx, int ny, int nz);
  inline int size()
  { return rBox.size(); }
  void r2k();
  void k2r();

  FFT3D(bool inPlace=true) : Allocated(false), InPlace(inPlace)
  {
    // Do nothing for now
  }
  ~FFT3D()
  {
    if (Allocated) {
      FFT_FREE(rData);
      if (!InPlace)
	FFT_FREE(kData);
      FFTNAME(destroy_plan)(r2kPlan);
      FFTNAME(destroy_plan)(k2rPlan);
    }
  }
};


// class FFT3Df
// {
// private:
//   complex<float> *rData, *kData;
//   fftwf_plan r2kPlan, k2rPlan;
//   bool Allocated, InPlace;
//   float sqrtNinv;
// public:
//   Array<complex<float>,3> rBox, kBox;

//   void resize (int nx, int ny, int nz);
//   inline int size()
//   { return rBox.size(); }
//   void r2k();
//   void k2r();

//   FFT3Df(bool inPlace=true) : Allocated(false), InPlace(inPlace)
//   {
//     // Do nothing for now
//   }
//   ~FFT3Df()
//   {
//     if (Allocated) {
//       fftwf_free(rData);
//       if (!InPlace)
// 	FFTNAME(free(kData);
//       fftwf_destroy_plan(r2kPlan);
//       fftwf_destroy_plan(k2rPlan);
//     }
//   }
// };


class FFTVec3D
{
private:
  TinyVector<complex<FFT_FLOAT>,3> *rData, *kData;
  FFTNAME(plan) r2kPlan, k2rPlan;
  bool Allocated, InPlace;
  FFT_FLOAT sqrtNinv;
public:
  Array< TinyVector<complex<FFT_FLOAT>,3>,3> rBox, kBox;

  void resize (int nx, int ny, int nz);
  inline int size()
  { return rBox.size(); }
  void r2k();
  void k2r();

  FFTVec3D(bool inPlace=true) : Allocated(false), InPlace(inPlace)
  {
    // Do nothing for now
  }
  ~FFTVec3D()
  {
    if (Allocated) {
      FFT_FREE(rData);
      if (!InPlace)
	FFT_FREE(kData);
      FFTNAME(destroy_plan)(r2kPlan);
      FFTNAME(destroy_plan)(k2rPlan);
    }
  }
};

class FFTMat3D
{
private:
  TinyMatrix<complex<FFT_FLOAT>,3,3> *rData, *kData;
  FFTNAME(plan) r2kPlan, k2rPlan;
  bool Allocated, InPlace;
  FFT_FLOAT sqrtNinv;
public:
  Array<TinyMatrix<complex<FFT_FLOAT>,3,3>,3> rBox, kBox;

  void resize (int nx, int ny, int nz);
  inline int size()
  { return rBox.size(); }
  void r2k();
  void k2r();

  FFTMat3D(bool inPlace=true) : Allocated(false), InPlace(inPlace)
  {
    // Do nothing for now
  }
  ~FFTMat3D()
  {
    if (Allocated) {
      FFTNAME(free)(rData);
      if (!InPlace)
	FFTNAME(free)(kData);
      FFTNAME(destroy_plan)(r2kPlan);
      FFTNAME(destroy_plan)(k2rPlan);
    }
  }
};


#endif
