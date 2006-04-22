#include "Pressure.h"

double
PressureClass::KineticPressure()
{
  PathClass &Path = PathData.Path;
  int numImages = PathData.Actions.Kinetic.NumImages;

  double P = 0.0;
  for (int ptcl=0; ptcl<Path.NumParticles(); ptcl++){
    int species=Path.ParticleSpeciesNum(ptcl);
    double lambda = Path.Species(species).lambda;
    if (lambda != 0){
      P += 3.0*(Path.NumTimeSlices()-1);
      double TwoLambdaTauInv=1.0/(2.0*Path.Species(species).lambda*Path.tau);
      for (int slice=0; slice < (Path.NumTimeSlices()-1);slice++) {
        dVec vel;
	vel = PathData.Path.Velocity(slice, slice+1, ptcl);
        double GaussProd = 1.0;
        for (int dim=0; dim<NDIM; dim++) {
	  double GaussSum=0.0;
	  for (int image=-numImages; image<=numImages; image++) {
	    double dist = vel[dim]+(double)image*Path.GetBox()[dim];
	    GaussSum += exp(-dist*dist*TwoLambdaTauInv);
	  }
	  GaussProd *= GaussSum;
        }
	P += log(GaussProd);    
      }
    }
  }
  
  //We are ignoring the \$\frac{3N}{2}*\log{4*\Pi*\lambda*\tau}
  P /= (3.0*Path.tau*Path.GetVol());
  return (P);
}

// double 
// PressureClass::ShortRangePressure()
// {
//   PathClass &Path = PathData.Path;
//   double P = 0.0;
//   int M = Path.NumTimeSlices();
//   for (int ptcl1=0; ptcl1 < Path.NumParticles(); ptcl1++) {
//     int species1 = Path.ParticleSpeciesNum(ptcl1);
//     for (int ptcl2=ptcl1+1; ptcl2<Path.NumParticles(); ptcl2++) {
//       int species2 = Path.ParticleSpeciesNum(ptcl2);
//       PairActionFitClass &PA=*(PathData.Actions.PairMatrix(species1,species2));
//       for (int slice=0; slice<(M-1); slice++) {
// 	dVec r, rp, grad, gradp;
// 	double rmag, rpmag, du_dq, du_dz;
// 	Path.DistDisp(slice, slice+1, ptcl1, ptcl2, rmag, rpmag, r, rp);
// 	double q = 0.5*(rmag+rpmag);
// 	double z = (rmag-rpmag);
// 	double s2 = dot (r-rp, r-rp);
// 	PA.Derivs(q,z,s2,0,du_dq, du_dz);
// 	Vec3 rhat  = (1.0/rmag)*r;
// 	Vec3 rphat = (1.0/rpmag)*rp;
	
// 	grad  = -(0.5*du_dq + du_dz)*rhat;
// 	gradp = -(0.5*du_dq - du_dz)*rphat;
// 	// gradVec(pi) -= (0.5*du_dq*(rhat+rphat) + du_dz*(rhat-rphat));
// 	/// Now, subtract off long-range part that shouldn't be in
// 	/// here 
// 	if (PA.IsLongRange() && PathData.Actions.UseLongRange) {
// 	  grad  += 0.5*(PA.Ulong(0).Deriv(rmag) *rhat);
// 	  gradp += 0.5*(PA.Ulong(0).Deriv(rpmag)*rphat);
// 	}
// 	P += dot(grad,r) + dot(rp, gradp);
//       }
//     }
//   }
//   /// HACK HACK HACK - minus sign
//   //P /= -3.0*Path.GetVol()*Path.tau;  
//   P /= 3.0*Path.GetVol()*Path.tau;
//   return P;
// }


double 
PressureClass::ShortRangePressure()
{
  PathClass &Path = PathData.Path;
  double P = 0.0;
  int M = Path.NumTimeSlices();
  for (int ptcl1=0; ptcl1 < Path.NumParticles(); ptcl1++) {
    int species1 = Path.ParticleSpeciesNum(ptcl1);
    for (int ptcl2=ptcl1+1; ptcl2<Path.NumParticles(); ptcl2++) {
      int species2 = Path.ParticleSpeciesNum(ptcl2);
      PairActionFitClass &PA=*(PathData.Actions.PairMatrix(species1,species2));
      for (int slice=0; slice<(M-1); slice++) {
	dVec r, rp;
	double rmag, rpmag, du_dq, du_dz, du_ds;
	Path.DistDisp(slice, slice+1, ptcl1, ptcl2, rmag, rpmag, r, rp);
	double q = 0.5*(rmag+rpmag);
	double z = (rmag-rpmag);
	double s2 = dot (r-rp, r-rp);
	PA.Derivs(q,z,s2,0,du_dq, du_dz, du_ds);
	P += q*du_dq + z*du_dz + sqrt(s2)*du_ds;
	if (PA.IsLongRange() && PathData.Actions.UseLongRange)
	  P -= 0.5*(rmag * PA.Ulong(0).Deriv(rmag) + 
		    rpmag* PA.Ulong(0).Deriv(rpmag));
      }
    }
  }
  /// HACK HACK HACK - minus sign
  //P /= -3.0*Path.GetVol()*Path.tau;  
  P /= -3.0*Path.GetVol()*Path.tau;
  return P;
}

inline double mag2(complex<double> z)
{
  return z.real()*z.real() + z.imag()*z.imag();
}

double
PressureClass::LongRangePressure()
{
  PathClass &Path = PathData.Path;
  double homo = 0.0;
  double hetero = 0.0;
  double background = 0.0;
  double k0Homo = 0.0;
  double k0Hetero = 0.0;
  int M = PathData.Path.NumTimeSlices();
  double volInv = 1.0/Path.GetVol();
  double third = 1.0/3.0;
  for (int slice=0; slice<M; slice++) {
    double factor = ((slice == 0) || (slice==(M-1))) ? 0.5 : 1.0;

    // First, do the homologous (same species) terms
    for (int species=0; species<Path.NumSpecies(); species++) {
      PairActionFitClass &pa = *PathData.Actions.PairMatrix(species,species);
      if (pa.IsLongRange()) {
	for (int ki=0; ki<Path.kVecs.size(); ki++) {
	  double k = Path.MagK(ki);
	  double rhok2 = mag2(Path.Rho_k(slice,species,ki));
	  homo -= 3.0 * factor * 0.5 * 2.0 * rhok2 * pa.Ulong_k  (0, ki);
	  homo -=  k  * factor * 0.5 * 2.0 * rhok2 * pa.dUlong_dk(0, ki);
	}
      }
      int N = Path.Species(species).NumParticles;
      // Or the neutralizing background term
      background += 3.0*factor * 0.5*N*N*pa.Ushort_k0(0);
      k0Homo     -= 3.0*factor * 0.5*N*N*pa.Ulong_k0 (0);
    }
    
    // Now do the heterologous terms
    for (int species1=0; species1<Path.NumSpecies(); species1++)
      for (int species2=species1+1; species2<Path.NumSpecies(); species2++) {
	PairActionFitClass &pa = 
	  *PathData.Actions.PairMatrix(species1, species2);
	if (pa.IsLongRange()) {
	  for (int ki=0; ki<Path.kVecs.size(); ki++) {
	    double k = Path.MagK(ki);
	    double rhorho = 
	      Path.Rho_k(slice, species1, ki).real() *
	      Path.Rho_k(slice, species2, ki).real() + 
	      Path.Rho_k(slice, species1, ki).imag() *
	      Path.Rho_k(slice, species2, ki).imag();
	    hetero -= 3.0 * factor * 2.0 * rhorho * pa.Ulong_k(0,ki);
	    hetero -=  k  * factor * 2.0 * rhorho * pa.dUlong_dk(0,ki);
	  }
	  int N1 = Path.Species(species1).NumParticles;
	  int N2 = Path.Species(species2).NumParticles;
	  background  += 3.0 * factor * N1*N2*pa.Ushort_k0(0);
	  k0Hetero    -= 3.0 * factor * N1*N2*pa.Ulong_k0(0);
	}
      }
  }
  double P = homo+hetero;
  if (PathData.Actions.LongRange.UseBackground)
    P += background;
  else
    P += (k0Homo+k0Hetero);
  P /= (-3.0*Path.GetVol()*Path.tau);
  //  return (homo+hetero);
  return (P);
}

void
PressureClass::Accumulate()
{
  KineticSum    += KineticPressure();
  ShortRangeSum += ShortRangePressure();
  if (PathData.Actions.HaveLongRange())
    LongRangeSum  += LongRangePressure();

  NumSamples++;
}

void
PressureClass::WriteBlock()
{
  PathClass &Path = PathData.Path;
  KineticSum    /= (double)(NumSamples*Path.TotalNumSlices);
  ShortRangeSum /= (double)(NumSamples*Path.TotalNumSlices);
  LongRangeSum  /= (double)(NumSamples*Path.TotalNumSlices);

  /// Sum over all processors in my clone
  PathData.Path.Communicator.Sum (KineticSum);
  PathData.Path.Communicator.Sum (ShortRangeSum);
  PathData.Path.Communicator.Sum (LongRangeSum);

  int numClassical = 0;
  for (int si=0; si<Path.NumSpecies(); si++)
    if (Path.Species(si).lambda != 0.0)
      numClassical += Path.Species(si).NumParticles;

  /// Add on classical kinetic energy contribution
  double beta = Path.tau*Path.TotalNumSlices;
  KineticSum += (double)numClassical/(Path.GetVol()*beta);

  double total =  KineticSum + ShortRangeSum + LongRangeSum;


  KineticVar.Write    (Prefactor*KineticSum);
  ShortRangeVar.Write (Prefactor*ShortRangeSum);
  LongRangeVar.Write  (Prefactor*LongRangeSum);
  PressureVar.Write   (Prefactor*total);

  KineticSum    = 0.0;
  ShortRangeSum = 0.0;
  LongRangeSum  = 0.0;
  NumSamples = 0;
}

void
PressureClass::Read (IOSectionClass &in)
{
  ObservableClass::Read(in);
}