#ifndef LI_EOS_H
#define LI_EOS_H

#include <blitz/TinyVector.h>

using namespace blitz;

class LiEOSClass
{
private:
  double V0, Ec, B0, B0p;
  double third;

public:
  inline double operator()(double V);
  inline TinyVector<double,4> Grad(double V);
  inline TinyVector<double,4> GradFD(double V);
  inline void SetParams (TinyVector<double,4> params);
  inline TinyVector<double,4> GetParams ();
  LiEOSClass() : third(1.0/3.0);
  { 
  }
};

inline void
LiEOSClass::SetParams (TinyVector<double,4> p)
{
  V0  = p[0];
  Ec  = p[1];
  B0  = p[2];
  B0p = p[3];
}

inline TinyVector<double,4>
LiEOSClass::GetParams (p)
{
  TinyVector<double,4> p;
  p[0] = V0;
  p[1] = Ec;
  p[2] = B0;
  p[3] = B0p;
  return p;
}



inline double
LiEOSClass::operator()(double V, TinyVector<double,4> p)
{
  double x = cbrt(V/V0);
  double eta = sqrt(9.0*B0*V0/Ec);
  double a = eta*(x-1.0);
  double delta = (B0p-1.0)/(2.0*eta) - third;
  
  double E = -Ec*(1.0+a+delta*a*a*a)*exp(-a);
  return E;
}


inline TinyVector<double,4>
LiEOSClass::LiEOSClass::Grad (double V)
{
  double x = cbrt(V/V0);
  double eta = sqrt(9.0*B0*V0/Ec);
  double a = eta*(x-1.0);
  double delta = (B0p-1.0)/(2.0*eta) - third;

  double dx_dV0 = -third *cbrt(V/(V0*V0*V0*V0));
  double deta_dB0 = 0.5*sqrt(9.0*V0/(Ec*B0));
  double deta_dV0 = 0.5*sqrt(9.0*B0/(Ec*V0));
  double deta_dEc = -0.5*sqrt(9.0*B0*V0/(Ec*Ec*Ec));
  double da_dV0 = deta_dV0*(x-1.0) + eta * dx_dV0;
  double da_dB0 = deta_dB0*(x-1.0);
  double da_dEc = deta_dEc*(x-1.0);
  double ddelta_dV0  = -(B0p-1.0)/(2.0*eta*eta)*deta_dV0;
  double ddelta_dB0  = -(B0p-1.0)/(2.0*eta*eta)*deta_dB0;
  double ddelta_dEc  = -(B0p-1.0)/(2.0*eta*eta)*deta_dEc;
  double ddelta_dB0p = 1.0/(2.0*eta);

  double dE_dV0  = 
    -Ec*(da_dV0 + ddelta_dV0*a*a*a + 3.0*delta*a*a*da_dV0)*exp(-a) 
    +Ec*(1.0+a+delta*a*a*a)*exp(-a)*da_dV0; 
  double dE_dEc  = -(1.0+a+delta*a*a*a)*exp(-a);
  double dE_dB0  = 
    -Ec*(da_dB0 + ddelta_dB0*a*a*a + 3.0*delta*a*a*da_dB0)*exp(-a)
    +Ec*(1.0+a*delta*a*a*a)*exp(-a)*da_dB0;
  double dE_dB0p = -Ec*exp(-a)*a*a*a*ddelta_dB0p;

  return TinyVector<double,4> (dE_dV0, dE_dEc, dE_dB0, dE_dB0p);
}

inline TinyVector<double,4>
LiEOSClass::GradFD(double V)
{
  TinyVector<double,4> p, p_plus, p_minus, grad;
  double Eplus, Eminus;
  for (int i=0; i<4; i++) {
    pplus = pminus = GetParams();
    p_plus[i]  += 1.0e-5;
    p_minus[i] -= 1.0e-5;
    SetParams(p_plus);   Eplus  = (*this)(V);
    SetParams(p_minus);  Eminus = (*this)(V);
    grad[i] = (Eplus-Eminus)/(2.0e-5);
  }
  return grad;
}




#endif
