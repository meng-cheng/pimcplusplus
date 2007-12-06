#include "VinetEOS2.h"
#include "NonlinearFit.h"
#include "../IO/IO.h"
#include <vector>
#include "PhononFreeEnergy.h"

void
TestGrad()
{
  TinyVector<double,4> params(80.0, -0.57, 0.011726, 3.378);
  VinetEOSClass eos;
  eos.SetParams (params);
  TinyVector<double,4> grad   = eos.Grad(10.0);
  TinyVector<double,4> gradFD = eos.GradFD(10.0);

  cerr << "grad   = " << grad << endl;
  cerr << "gradFD = " << gradFD << endl;
}

void
TestVal()
{
  TinyVector<double,4> params(80.0, -0.5, 0.296471, 3.378);
  VinetEOSClass eos;
  eos.SetParams (params);
  
  for (double V=15.0; V<=65.0; V+=0.1)
    fprintf (stderr, "%18.14f %18.14f\n", V, eos(V));
}

void
TestFit()
{
  TinyVector<double,4> params(80.0, -0.5, 0.296471, 3.378);
  TinyVector<double,4> params2(81.0, -0.6, 0.286471, 3.9);
  VinetEOSClass eos;
  eos.SetParams (params);
  NonlinearFitClass<4,VinetEOSClass> fitter(eos);

  Array<double,1> V(5), E(5), sigma(5);
  V = 15.0, 25.0, 35.0, 50.0, 65.0;
  for (int i=0; i<E.size(); i++) {
    E(i) = eos(V(i));
    sigma(i) = 0.01;
  }

  fitter.Fit (V, E, sigma, params2);
  cerr << "params2 = " << params2 << endl;
}


using namespace IO;
void
TestFit2(string fname)
{
  TinyVector<double,4> params(80.0, 0.51, 0.0155), errors;  
  VinetEOSClass eos;
  eos.SetParams (params);
  NonlinearFitClass<4,VinetEOSClass> fitter(eos);
  Array<double,1> V, E, sigma;


  IOSectionClass in;
  assert(in.OpenFile (fname));
  assert(in.ReadVar("V0",  params[0]));
  assert(in.ReadVar("E0",  params[1]));
  assert(in.ReadVar("B0",  params[2]));
  assert(in.ReadVar("B0p", params[3]));
  params[2] /= 29421.01;
	
  assert(in.ReadVar("V", V));
  assert(in.ReadVar("E", E));
  assert(in.ReadVar("Sigma", sigma));
  cerr << "V = " << V << endl;
  cerr << "E = " << E << endl;
  cerr << "Sigma = " << sigma << endl;

  fprintf (stdout, "V0  = %12.8f\n", params[0]);
  fprintf (stdout, "Ec  = %12.8f\n", params[1]);
  fprintf (stdout, "B0  = %12.8f GPa\n", params[2] * 29421.01);
  eos.SetParams (params);
  fprintf (stdout, "B0p = %12.8f\n", params[3]);
  fitter.Fit (V, E, sigma, params);

  Array<double,2> &covar = fitter.GetCovariance();
  errors[0] = sqrt (covar(0,0));
  errors[1] = sqrt (covar(1,1));
  errors[2] = sqrt (covar(2,2));
  errors[3] = sqrt (covar(3,3));
  fprintf (stdout, "V0  = %12.8f +/- %12.8f\n", params[0], errors[0]);
  fprintf (stdout, "E0  = %12.8f +/- %12.8f\n", params[1], errors[1]);
  fprintf (stdout, "B0  = %12.8f +/- %12.8f\n", 
	   params[2]* 29421.01, errors[2]* 29421.01);
  eos.SetParams(params);
  fprintf (stdout, "B0p = %12.8f +/- %12.8f\n", params[3], errors[3]);
  fprintf (stdout, "Delta = %1.5f\n", 0.0);
  double V0 = params[0];
  double latConst = cbrt(4.0*V0);
  double latConstError = cbrt(4.0)/(3.0*cbrt(V0)*cbrt(V0)) * errors[0];
  fprintf (stdout, "Lattice const. = %1.6f +/- %1.6f bohr radii\n",
	   latConst, latConstError);
  fprintf (stdout, "               = %1.6f +/- %1.6f angstrom\n",  
	   0.52917721*latConst, 0.52917721*latConstError);
  string Ename = fname + ".dat";


  FILE *fout = fopen (Ename.c_str(), "w");
  double delta = (V(0) - V(V.size()-1))/1000.0;
  for (double v=V(V.size()-1); v<=V(0); v+=delta)
    fprintf (fout, "%20.16f %20.16f %20.16f %20.16f\n", v, eos(v), eos.Pressure(v), eos.PressureFD(v));
  fclose (fout);
}


void 
StaticFit (string fname, VinetEOSClass &eos)
{
  TinyVector<double,4> params(80.0, 0.51, 0.0155), errors;  
  eos.SetParams (params);
  NonlinearFitClass<4,VinetEOSClass> fitter(eos);
  Array<double,1> V, E, sigma;

  IOSectionClass in;
  assert(in.OpenFile (fname));
  assert(in.ReadVar("V0",  params[0]));
  assert(in.ReadVar("E0",  params[1]));
  assert(in.ReadVar("B0",  params[2]));
  assert(in.ReadVar("B0p", params[3]));
  params[2] /= 29421.01;
	
  assert(in.ReadVar("V", V));
  assert(in.ReadVar("E", E));
  assert(in.ReadVar("Sigma", sigma));
  cerr << "V = " << V << endl;
  cerr << "E = " << E << endl;
  cerr << "Sigma = " << sigma << endl;

  fprintf (stdout, "V0  = %12.8f\n", params[0]);
  fprintf (stdout, "Ec  = %12.8f\n", params[1]);
  fprintf (stdout, "B0  = %12.8f GPa\n", params[2] * 29421.01);
  eos.SetParams (params);
  fprintf (stdout, "B0p = %12.8f\n", params[3]);
  fitter.Fit (V, E, sigma, params);

  Array<double,2> &covar = fitter.GetCovariance();
  errors[0] = sqrt (covar(0,0));
  errors[1] = sqrt (covar(1,1));
  errors[2] = sqrt (covar(2,2));
  errors[3] = sqrt (covar(3,3));
  fprintf (stdout, "V0  = %12.8f +/- %12.8f\n", params[0], errors[0]);
  fprintf (stdout, "E0  = %12.8f +/- %12.8f\n", params[1], errors[1]);
  fprintf (stdout, "B0  = %12.8f +/- %12.8f\n", 
	   params[2]* 29421.01, errors[2]* 29421.01);
  eos.SetParams(params);
  fprintf (stdout, "B0p = %12.8f +/- %12.8f\n", params[3], errors[3]);
  fprintf (stdout, "Delta = %1.5f\n", 0.0);
  double V0 = params[0];
  double latConst = cbrt(4.0*V0);
  double latConstError = cbrt(4.0)/(3.0*cbrt(V0)*cbrt(V0)) * errors[0];
  fprintf (stdout, "Lattice const. = %1.6f +/- %1.6f bohr radii\n",
	   latConst, latConstError);
  fprintf (stdout, "               = %1.6f +/- %1.6f angstrom\n",  
	   0.52917721*latConst, 0.52917721*latConstError);
  string Ename = fname + ".dat";


  FILE *fout = fopen (Ename.c_str(), "w");
  double delta = (V(0) - V(V.size()-1))/1000.0;
  for (double v=V(V.size()-1); v<=V(0); v+=delta)
    fprintf (fout, "%20.16f %20.16f %20.16f %20.16f\n", v, eos(v), eos.Pressure(v), eos.PressureFD(v));
  fclose (fout);
}


void
ThermalFit (string fname, PhononFreeEnergy &phonons)
{
  FILE *fin = fopen (fname.c_str(), "r");
  std::vector<double> Vvec, Tvec, Fvec, Uvec;
  bool done = false;
  while (!done) {
    double v,t,f,u;
    int retval = fscanf (fin, "%lf %lf %lf %lf", &v, &t, &f, &u);
    if (retval == 4) {
      Vvec.push_back (v);
      Tvec.push_back (t);
      Fvec.push_back (f);
      Uvec.push_back (u);
    }
    else
      done = true;
  }
  fclose (fin);
  int N = Vvec.size();
  Array<double,1> F(N), U(N), V(N), T(N);
  for (int i=0; i<N; i++) {
    V(i) = Vvec[i];
    T(i) = Tvec[i];
    F(i) = Fvec[i];
    U(i) = Uvec[i];
  }
  
  phonons.FitF(3, 3, F, V, T);
}

void
CalcProperties (VinetEOSClass &staticEOS,
		PhononFreeEnergy &phonons)
{
  

}



main(int argc, char **argv)
{
  TestGrad();
  //TestVal();
  if (argc > 2) {
    VinetEOSClass staticEOS;
    StaticFit (argv[1], staticEOS);
    PhononFreeEnergy thermalEOS;
  }
  else if (argc > 1) {
    VinetEOSClass staticEOS;
    StaticFit (argv[1], staticEOS);
  }

  else
    cerr << "Usage:  VinetFit2 static.in phonon.dat\n";
}
