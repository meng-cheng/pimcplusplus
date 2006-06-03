#ifndef MYBLITZ_H
#define MYBLITZ_H

#include <blitz/array.h>
#include <blitz/tinyvec-et.h>
typedef double scalar;

// #define NDIM 3

using namespace blitz;
typedef TinyVector<scalar,2> Vec2;
typedef TinyVector<scalar,3> Vec3;
typedef TinyVector<scalar,4> Vec4;
typedef TinyMatrix<scalar,2,2> Mat2;
typedef TinyMatrix<scalar,3,3> Mat3;
typedef TinyVector<complex<double>,3>   cVec3;
typedef TinyMatrix<complex<double>,3,3> cMat3;

//typedef TinyVector<scalar,NDIM> dVec;
//typedef TinyVector<int,NDIM> dVecInt;

#ifdef MAC
//  extern "C" double isnan (double x);
#define isnan(x) __isnand(x)
#define fpclassify(x) __fpclassifyd(x)
#define isnormal(x) __isnormald(x)
#define isinf(x) __isinfd(x)
#endif

template <class T, int size>
inline TinyVector<T,size> operator-(TinyVector<T,size> v)
{
  TinyVector<T,size> minusv;
  for (int i=0; i<size; i++)
    minusv[i] = -v[i];
  return (minusv);
}

// template <class T, int size>
// inline bool operator==(TinyVector<T,size> v1,TinyVector<T,size> v2)
// {
//   bool equals=true;


//   for (int i=0; i<size; i++)
//     equals = (v1[i]==v2[i]) && equals;
//   return (equals);
// }

inline Vec2 operator*(const Vec2 &v, scalar s)
{
  Vec2 result;
  result[0] = s*v[0];
  result[1] = s*v[1];
  return (result);
}

inline Vec2 operator*(scalar s, const Vec2 &v)
{
  return (v * s);
}

inline Vec2 operator+(const Vec2 &v1, const Vec2 &v2)
{
  Vec2 result;
  result[0] = v1[0]+v2[0];
  result[1] = v1[1]+v2[1];
  return (result);
}


inline Vec2 operator-(const Vec2 &v1, const Vec2 &v2)
{
  Vec2 result;
  result[0] = v1[0]-v2[0];
  result[1] = v1[1]-v2[1];
  return (result);
}


inline Vec3 operator*(scalar s, const Vec3 &v)
{
  Vec3 result;
  result[0] = s*v[0];
  result[1] = s*v[1];
  result[2] = s*v[2];
  return (result);
}

inline Vec3 operator*(const Vec3 &v, scalar s)
{
  Vec3 result;
  result[0] = s*v[0];
  result[1] = s*v[1];
  result[2] = s*v[2];
  return (result);
}


inline Vec3 operator+(const Vec3 &v1, const Vec3 &v2)
{
  Vec3 result;
  result[0] = v1[0]+v2[0];
  result[1] = v1[1]+v2[1];
  result[2] = v1[2]+v2[2];
  return (result);
}

inline Vec3 operator-(const Vec3 &v1, const Vec3 &v2)
{
  Vec3 result;
  result[0] = v1[0]-v2[0];
  result[1] = v1[1]-v2[1];
  result[2] = v1[2]-v2[2];
  return (result);
}


inline Vec4 operator*(scalar s, const Vec4 &v)
{
  Vec4 result;
  result[0] = s*v[0];
  result[1] = s*v[1];
  result[2] = s*v[2];
  result[3] = s*v[3];
  return (result);
}

inline Vec4 operator*(const Vec4 &v, scalar s)
{
  Vec4 result;
  result[0] = s*v[0];
  result[1] = s*v[1];
  result[2] = s*v[2];
  result[3] = s*v[3];
  return (result);
}


inline Vec4 operator+(const Vec4 &v1, const Vec4 &v2)
{
  Vec4 result;
  result[0] = v1[0]+v2[0];
  result[1] = v1[1]+v2[1];
  result[2] = v1[2]+v2[2];
  result[3] = v1[3]+v2[3];
  return (result);
}

inline Vec4 operator-(const Vec4 &v1, const Vec4 &v2)
{
  Vec4 result;
  result[0] = v1[0]-v2[0];
  result[1] = v1[1]-v2[1];
  result[2] = v1[2]-v2[2];
  result[3] = v1[3]-v2[3];
  return (result);
}


inline Mat3 operator*(scalar s, const Mat3 &M)
{
  Mat3 sM;
  sM(0,0)=s*M(0,0); sM(0,1)=s*M(0,1); sM(0,2)=s*M(0,2);
  sM(1,0)=s*M(1,0); sM(1,1)=s*M(1,1); sM(1,2)=s*M(1,2);
  sM(2,0)=s*M(2,0); sM(2,1)=s*M(2,1); sM(2,2)=s*M(2,2);
  return (sM);
}

inline Mat3 operator*(const Mat3 &A, const Mat3 &B)
{
  Mat3 AB;
  for (int i=0; i<3; i++)
    for (int j=0; j<3; j++)
      AB(i,j) = 0.0;
  for (int i=0; i<3; i++)
    for (int j=0; j<3; j++)
      for (int k=0; k<3; k++)
	AB(i,j) += A(i,k)*B(k,j);
  return (AB);
}

inline Mat3 operator+(const Mat3 &A, const Mat3 &B)
{
  Mat3 ApB;
  for (int i=0; i<3; i++)
    for (int j=0; j<3; j++)
      ApB(i,j) = A(i,j) + B(i,j);
  return (ApB);
}


inline Mat3 operator-(const Mat3 &A, const Mat3 &B)
{
  Mat3 AmB;
  for (int i=0; i<3; i++)
    for (int j=0; j<3; j++)
      AmB(i,j) = A(i,j) - B(i,j);
  return (AmB);
}

inline Vec3 operator*(const Mat3& A, const Vec3 &v)
{
  Vec3 Av;
  Av[0] = A(0,0)*v[0] + A(0,1)*v[1] + A(0,2)*v[2];
  Av[1] = A(1,0)*v[0] + A(1,1)*v[1] + A(1,2)*v[2];
  Av[2] = A(2,0)*v[0] + A(2,1)*v[1] + A(2,2)*v[2];
  return Av;
}

inline cMat3 operator+(const cMat3 &A, const cMat3 &B)
{
  cMat3 ApB;
  ApB(0,0)=A(0,0)+B(0,0); ApB(0,1)=A(0,1)+B(0,1); ApB(0,2)=A(0,2)+B(0,2);
  ApB(1,0)=A(1,0)+B(1,0); ApB(1,1)=A(1,1)+B(1,1); ApB(1,2)=A(1,2)+B(1,2);
  ApB(2,0)=A(2,0)+B(2,0); ApB(2,1)=A(2,1)+B(2,1); ApB(2,2)=A(2,2)+B(2,2);
  return ApB;
}

inline cMat3 operator-(const cMat3 &A, const cMat3 &B)
{
  cMat3 AmB;
  AmB(0,0)=A(0,0)-B(0,0); AmB(0,1)=A(0,1)-B(0,1); AmB(0,2)=A(0,2)-B(0,2);
  AmB(1,0)=A(1,0)-B(1,0); AmB(1,1)=A(1,1)-B(1,1); AmB(1,2)=A(1,2)-B(1,2);
  AmB(2,0)=A(2,0)-B(2,0); AmB(2,1)=A(2,1)-B(2,1); AmB(2,2)=A(2,2)-B(2,2);
  return AmB;
}

inline cMat3& operator+=(cMat3 &A, const cMat3 &B)
{
  A(0,0)+=B(0,0); A(0,1)+=B(0,1); A(0,2)+=B(0,2);
  A(1,0)+=B(1,0); A(1,1)+=B(1,1); A(1,2)+=B(1,2);
  A(2,0)+=B(2,0); A(2,1)+=B(2,1); A(2,2)+=B(2,2);
  return A;
}

inline cMat3& operator-=(cMat3 &A, const cMat3 &B)
{
  A(0,0)-=B(0,0); A(0,1)-=B(0,1); A(0,2)-=B(0,2);
  A(1,0)-=B(1,0); A(1,1)-=B(1,1); A(1,2)-=B(1,2);
  A(2,0)-=B(2,0); A(2,1)-=B(2,1); A(2,2)-=B(2,2);
  return A;
}

inline double operator*(const Vec3 &v1, const Vec3 &v2)
{
  return (v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2]);
}

inline cMat3 operator*(complex<double> z, const Mat3 &A)
{
  cMat3 zA;
  zA(0,0)=z*A(0,0); zA(0,1)=z*A(0,1); zA(0,2)=z*A(0,2);
  zA(1,0)=z*A(1,0); zA(1,1)=z*A(1,1); zA(1,2)=z*A(1,2);
  zA(2,0)=z*A(2,0); zA(2,1)=z*A(2,1); zA(2,2)=z*A(2,2);
  return zA;
}

inline cMat3 operator*(const Mat3 &A, complex<double> z)
{
  return z*A;
}

inline cVec3 operator*(const cMat3 &A, const cVec3 &x)
{
  cVec3 Ax;
  Ax[0] = A(0,0)*x[0] + A(0,1)*x[1] + A(0,2)*x[2];
  Ax[1] = A(1,0)*x[0] + A(1,1)*x[1] + A(1,2)*x[2];
  Ax[2] = A(2,0)*x[0] + A(2,1)*x[1] + A(2,2)*x[2];
  return Ax;
}

inline double distSqrd(Vec2 a,Vec2 b)
{
  return dot(a-b,a-b);
}

inline double distSqrd(Vec3 a,Vec3 b)
{
  return dot(a-b,a-b);
}


template<class T> class SymmArray
{
private:
  Array<T,1> A;
  int N;
  inline int index(int row, int col) const 
  { return ((row > col) ? ((row*(row+1)>>1)+col) : ((col*(col+1)>>1)+row)); }
public:

  inline void resize(int n) 
  {
    N = (n*(n+1))>>1;
    A.resize(N);
  }

  inline int rows() const
  { return N; }
  
  inline T operator()(int i, int j) const
  { return (A(index(i,j))); }

  inline T& operator()(int i, int j)
  { return (A(index(i,j))); }

  inline SymmArray<T> (const SymmArray<T> &B)
  {
    resize(B.N);
    A=B.A;
  }
  inline SymmArray<T>& operator=(const SymmArray<T>& B)
  {
    A=B.A;
    return *this;
  }
  inline SymmArray<T>()
  { N=0; }
};

  
inline 
void Vec2Array (Array<Vec2,1> &vec, Array<double,1> &array)
{
  assert (array.extent(0) == (2*vec.size()));
  memcpy(array.data(), vec.data(), 
	 array.size()*sizeof(double));
}

inline 
void Vec2Array (Array<Vec3,1> &vec, Array<double,1> &array)
{
  assert (array.extent(0) == (3*vec.size()));
  for (int i=0; i<vec.size(); i++) {
    array(3*i+0) = vec(i)[0];
    array(3*i+1) = vec(i)[1];
    array(3*i+2) = vec(i)[2];
  }
//   memcpy(array.data(), vec.data(), 
// 	 array.size()*sizeof(double));
}

inline 
void Array2Vec (Array<double,1> &array, Array<Vec2,1> &vec)
{
  assert (array.extent(0) == (2*vec.size()));
  memcpy (vec.data(), array.data(), 
	  array.size()*sizeof(double));
}

inline 
void Array2Vec (Array<double,1> &array, Array<Vec3,1> &vec)
{
  assert (array.extent(0) == (3*vec.size()));
  memcpy (vec.data(), array.data(), 
	  array.size()*sizeof(double));
}


inline 
void Vec2Array (Array<Vec2,1> &vec, Array<double,2> &array)
{
  assert (array.extent(0) == vec.size());
  assert (array.extent(1) == 2);
  memcpy(array.data(), vec.data(), 
	 array.size()*sizeof(double));
}

inline
void Vec2Array (Array<Vec3,1> &vec, Array<double,2> &array)
{
  assert (array.extent(0) == vec.size());
  assert (array.extent(1) == 3);
  for (int i=0; i<vec.size(); i++) {
    array(i,0) = vec(i)[0];
    array(i,1) = vec(i)[1];
    array(i,2) = vec(i)[2];
  }
//   memcpy(array.data(), vec.data(), 
// 	 array.size()*sizeof(double));
}


inline 
void Array2Vec (Array<double,2> &array, Array<Vec2,1> &vec)
{
  assert (array.extent(0) == vec.size());
  assert (array.extent(1) == 2);
  memcpy (vec.data(), array.data(), 
	  array.size()*sizeof(double));
}

inline
void Array2Vec (Array<double,2> &array, Array<Vec3,1> &vec)
{
  assert (array.extent(0) == vec.size());
  assert (array.extent(1) == 3);
  memcpy(vec.data(), array.data(), 
	 array.size()*sizeof(double));
}

inline Array<Vec3,1> operator+(const Array<Vec3,1> &array, Vec3 vec)
{
  Array<Vec3,1> result(array.size());
  for (int i=0; i<array.size(); i++)
    result(i) = vec + array(i);
  return result;
}

inline Array<Vec3,1> operator+(Vec3 vec, const Array<Vec3,1> &array)
{
  Array<Vec3,1> result(array.size());
  for (int i=0; i<array.size(); i++)
    result(i) = vec + array(i);
  return result;
}

template<typename T, int N>
inline bool operator==(const TinyVector<T,N> &a, 
		       const TinyVector<T,N> &b)
{
  bool equals = true;
  for (int i=0; i<N; i++)
    equals = equals && (a[i] == b[i]);
  return equals;
}

template<typename T, int N>
inline bool operator!=(const TinyVector<T,N> &a, 
		       const TinyVector<T,N> &b)
{
  return !(a == b);
}



#ifndef NAN
#define NAN sqrt(-1.0)
#endif


#endif
