#include "ConjGrad2.h"


void ConjGrad::Setup()
{
  int numBands = Bands.rows();
  int N = H.GVecs.size();
  Bands.resize (numBands, N);
  Energies.resize(numBands);
  cnext.resize (N);
  Hc.resize (N);
  Phi.resize (N);
  Phip.resize(N);
  Phipp.resize(N);
  Xi.resize(N);
  Eta.resize(N);
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
//       if (dot (H.GVecs(i), H.GVecs(i)) < maxE) {
// 	c(i) = 1.0;
// 	numNonZero++;
//       }
    }
    Normalize(c);
  }
  cerr << "numNonZero = " << numNonZero << endl;
//   Orthogonalize (Bands);
//   for (int band=0; band<numBands; band++) {
//     c.reference(Bands(band,Range::all()));
//     Normalize (c);
//   }

  PrintOverlaps();
  IsSetup = true;
}

void ConjGrad::CalcPhiSD()
{
  H.Apply(c, Hc);
  E0 = realconjdot (c, Hc);
  cerr << "E = " << E0 << endl;
  Phip = E0*c - Hc;
  Normalize (Phip);  
}

void ConjGrad::Precondition()
{
  double Tinv = 1.0/T;
  for (int i=0; i<c.size(); i++) {
    double x = 0.5*dot(H.GVecs(i), H.GVecs(i))*Tinv;
    double num = 27.0 + 18.0*x +12.0*x*x + 8.0*x*x*x;
    double denom = num + 16.0*x*x*x*x;
    Eta(i) = (num/denom)* Xi(i);
  }
}

void ConjGrad::CalcPhiCG()
{
  Hc = 0.0;
  H.Kinetic.Apply (c, Hc);
  T = realconjdot (c, Hc);
  H.Vion->Apply (c, Hc);
  E0 = realconjdot (c, Hc);
  Xi = E0*c - Hc;
  /// Orthonalize to other bands here
  zVec &Xip = Xi;
  Orthogonalize2 (Bands, Xip, CurrentBand);

  Precondition();

  // Now, orthogonalize to all bands
  // rename for clarity
  zVec &Etap = Eta;
  Orthogonalize2 (Bands, Etap, CurrentBand);
  // Etap = Eta - conjdot (c, Eta)*c;

  // Compute conjugate direction
  complex<double> etaxi = conjdot(Etap, Xip);
  complex<double> gamma; 
  if (EtaXiLast != complex<double>(0.0, 0.0)) 
    gamma = etaxi/EtaXiLast;
  else
    gamma = 0.0;
  EtaXiLast = etaxi;
  
  Phi = Etap + gamma * Phi;
  
  // Orthogonalize to present band
  Phipp = Phi - conjdot(c, Phi) * c;
  Phip = Phipp;
  Normalize (Phip);
  Energies(CurrentBand) = E0;
}


void ConjGrad::Solve(int band)
{
  CurrentBand = band;
  if (!IsSetup)
    Setup();
  c.reference (Bands(band,Range::all()));
  EtaXiLast = 0.0;
  Orthogonalize2 (Bands, c, band-1);
  Normalize(c);

  double Elast = 1.0e100;
  while (fabs (Elast - Energies(band)) > Tolerance) {
    //    cerr << "Energy = " << 27.211383*Energies(band) << endl;
    Elast = Energies(band);
    // First, calculate conjugate gradient direction
    CalcPhiCG();
    
    // Now, pick optimal theta for 
    double dE_dtheta = 2.0*realconjdot(Phip, Hc);
    double theta1 = M_PI/300.0;
    cnext = cos(theta1)*c + sin(theta1)*Phip;
    H.Apply (cnext, Hc);
    double E1 = realconjdot (cnext, Hc);
    double A1=(E0 - E1 + 0.5*sin(2.0*theta1)*dE_dtheta)/(1.0-cos(2.0*theta1));
    double B1 = 0.5*dE_dtheta;
    double thetaMin = 0.5*atan (B1/A1);
    
    c = cos(thetaMin)*c + sin(thetaMin)*Phip;
  }
}

void
ConjGrad::PrintOverlaps()
{
  cerr << "Overlaps = \n";
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
