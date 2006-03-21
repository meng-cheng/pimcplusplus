
#include "Josephson.h"
#include "../PathDataClass.h"
#include <fftw3.h>

///This has to be called after pathdata knows how many
///particles it has
void HermeleClass::Read(IOSectionClass& in)
{
  assert(in.ReadVar("alpha",alpha));
  TotalTime=0;

  int N=PathData.Path.NumTimeSlices();
  inArray = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)*(N-1));
  out=(fftw_complex*)fftw_malloc(sizeof(fftw_complex)*(N-1));

  p=fftw_plan_dft_1d(PathData.Path.NumTimeSlices()-1,inArray,out,FFTW_BACKWARD,FFTW_MEASURE);
  c=1.0;
  K_s=1.0;
  //  omega_c2=(10.0/PathData.Path.tau)*(10.0/PathData.Path.tau);
  omega_c2=100.0;
  //  cerr<<"My omega_c2 is "<<omega_c2<<endl;
  
}

void HermeleClass::Phi2Omega()
{
  //  cerr<<"INto function"<<endl;
  int N=PathData.Path.NumTimeSlices()-1;
  for (int slice=0;slice<N;slice++){
    inArray[slice][0]=Path(slice,0)[0];
    inArray[slice][1]=0.0;
  }
  //  cerr<<"Filled up the slice arrawy"<<endl;
  fftw_execute(p);
  //  cerr<<"executed"<<endl;
  for (int counter=0;counter<N;counter++){
    out[counter][0]=out[counter][0]*PathData.Path.tau;
    out[counter][1]=out[counter][1]*PathData.Path.tau;
  }
  //  cerr<<"dividing out the counter"<<endl;

}

double HermeleClass::ComputeFourierAction()
{
  double T=1.0/(PathData.Path.tau*(PathData.Path.NumTimeSlices()-1));
  double total=0.0;
  double prefactor=(2*c*K_s*T)/(M_PI*M_PI);
  Phi2Omega();
  for (int slice=1;slice<PathData.Path.NumTimeSlices()-1;slice++){
    double omega_n=2.0*M_PI*slice*T;
    total+=1.0/log((omega_n*omega_n+omega_c2)/(omega_n*omega_n))*
      (out[slice][0]*out[slice][0]+out[slice][1]*out[slice][1]);
    //    cerr<<total<<" "<<1.0/log((omega_n*omega_n+omega_c2)/(omega_n*omega_n))<<" "
    //	<<(out[slice][0]*out[slice][0]+out[slice][1]*out[slice][1])<<" "<<T<<" "
    //	<<omega_n<<" "<<((omega_n*omega_n+omega_c2)/(omega_n*omega_n))<<endl;
  }
  //  for (int slice=0;slice<PathData.Path.NumTimeSlices()-1;slice++){
  //    cerr<<out[slice][0]<<" "<<out[slice][1]<<endl;
  //  }
  return prefactor*total;
}


HermeleClass::HermeleClass(PathDataClass &pathData) : 
  ActionBaseClass (pathData)
{
  

}

double 
HermeleClass::SingleAction (int slice1, int slice2,
			       const Array<int,1> &changedParticles,
			       int level)
{ 
//   ifstream infile;
//   infile.open("myPath");
//   for (int slice=0;slice<PathData.Path.NumTimeSlices();slice++)
//     infile>>Path(slice,0)[0];
//   infile.close();
//   slice1=0;
//   slice2=PathData.Path.NumTimeSlices()-2;
//   level=0;
  double levelTau=Path.tau* (1<<level);
  double totalU=0.0;
  int skip = 1<<level;
  PathClass &Path=PathData.Path;
  //  cerr<<"My total U starts at "<<totalU<<endl;
  //  double J=0.1;
  double J=5.0;
  for (int slice=slice1;slice<slice2;slice+=skip){
    totalU -= (J*cos(Path(slice,0)[0])*levelTau/2.0+J*cos(Path(slice+skip,0)[0])*levelTau/2.0);
    //    totalU -= (2.0*(Path(slice,0)[0]*Path(slice,0)[0]/2.0)
    //	       *levelTau/2.0+
    //	       2.0*(Path(slice+skip,0)[0]*Path(slice+skip,0)[0]/2.0)
    //	       *levelTau/2.0);
    //    cerr<<"ACTION: "<<slice<<" "<<slice+skip<<" "<<totalU<<" "<<Path(slice,0)[0]<<" "<<Path(slice+skip,0)[0]<<endl;
    
  }
  
  double totalPotential=ComputeFourierAction();
  //  cerr<<"Start paths"<<endl;
  //  for (int slice=0;slice<PathData.Path.NumTimeSlices();slice++)
  //    cerr<<Path(slice,0)[0]<<endl;
  //  cerr<<"End paths "<<totalU<<" "<<totalPotential<<endl;
  //  cerr<<"My totalU is "<<totalU<<endl;
  //  cerr<<totalU<<" "<<totalPotential<<endl;
  return totalU+totalPotential;
}



double 
HermeleClass::d_dBeta (int slice1, int slice2, int level)
{
  return 0.0;
}