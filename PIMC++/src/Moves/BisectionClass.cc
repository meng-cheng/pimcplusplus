/////////////////////////////////////////////////////////////
// Copyright (C) 2003-2006 Bryan Clark and Kenneth Esler   //
//                                                         //
// This program is free software; you can redistribute it  //
// and/or modify it under the terms of the GNU General     //
// Public License as published by the Free Software        //
// Foundation; either version 2 of the License, or         //
// (at your option) any later version.  This program is    //
// distributed in the hope that it will be useful, but     //
// WITHOUT ANY WARRANTY; without even the implied warranty //
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. //  
// See the GNU General Public License for more details.    //
// For more information, please see the PIMC++ Home Page:  //
//           http://pathintegrals.info                     //
/////////////////////////////////////////////////////////////

#include "BisectionClass.h"

/*! Samples the particles in "particles" and the time slices between
 startSlice and endSlice (not inclusive). Samples from a free gaussian
 of variance \f[$\sigma=\sqrt{2*\lambda*\tau} \$]. Return 
 the logarithm of the total sampling probability of the density (i.e \f[\$
\frac{-NDIM}{2}*\log{2*\pi*\sigma^2}-0.5*\frac{\delta \cdot
\delta}{\sigma * \sigma}  \$] )

*/
double BisectionClass::SamplePaths(int startSlice, int endSlice, Array<int,1> particles, int level)
{
  dVec rpp;
  int skip = 1<<(level+1);
  double logNewSampleProb=0.0;
  PathClass &Path = PathData.Path;
  ActionClass &Action = PathData.Action;
  double levelTau = 0.5*PathData.Action.tau*skip;
  for (int ptclIndex=0;ptclIndex<particles.size();ptclIndex++){
    int  ptcl=particles(ptclIndex);
    double lambda=PathData.Path.ParticleSpecies(ptcl).lambda;
    double sigma2=(1.0*lambda*levelTau);
    double sigma=sqrt(sigma2);
    double prefactorOfSampleProb=0.0;//-NDIM/2.0*log(2*M_PI*sigma2);
    for (int slice=startSlice;slice<endSlice;slice+=skip){
      dVec r = Path(slice,ptcl);
      //\\     dVec rp= Path(slice+skip,ptcl);
      dVec rpp; //\\=Path(slice+(skip>>1),ptcl);
      //      Path.PutInBox(r);
      //      Path.PutInBox(rp);
      //      Path.PutInBox(rpp);
      dVec rdiff=Path.Velocity(slice, slice+skip, ptcl);
      dVec rbar = r + 0.5*rdiff;
      dVec newDelta;
      Path.Random.LocalGaussianVec(sigma,newDelta);
      PathData.Path.PutInBox(newDelta);
      double GaussProd=1.0;
      for (int dim=0; dim<NDIM; dim++) {
	  double GaussSum = 0.0;
	  int NumImage = 1;
	  for (int image=-NumImage; image <= NumImage; image++) {
	    double dist = newDelta[dim]+(double)image*Path.GetBox()[dim];
	    GaussSum += exp(-0.5*dist*dist/sigma2);
	  }
	  GaussProd *= GaussSum;
      }
      logNewSampleProb += prefactorOfSampleProb + log(GaussProd);
      rpp=rbar+newDelta;

      ///Here we've stored the new position in the path
      Path.SetPos(slice+(skip>>1),ptcl,rpp);
    }
  }
  //  cerr<<"My logNewSampleProb is "<<logNewSampleProb<<endl;
  return logNewSampleProb;
}


/// This calculates the sample probability for going from the state
/// that is currently in the newMode of MirroredArrayClass to the
/// state that is currently in oldMode of MirroredArrayClass 
double BisectionClass::LogSampleProb(int startSlice, int endSlice, 
				     Array<int,1> particles, int level)
{
  PathClass &Path = PathData.Path;
  dVec rpp;
  int skip = 1<<(level+1);
  double logSampleProb=0.0;

  double levelTau = 0.5*PathData.Action.tau*skip;
  for (int ptclIndex=0;ptclIndex<particles.size();ptclIndex++){
    int ptcl = particles(ptclIndex);
    double lambda=Path.ParticleSpecies(ptcl).lambda;
    double sigma2=(1.0*lambda*levelTau);
    double sigma=sqrt(sigma2);
    double prefactorOfSampleProb=0.0;//-NDIM/2.0*log(2*M_PI*sigma2);
    for (int slice=startSlice;slice<endSlice;slice+=skip){
      dVec r = Path(slice,ptcl);
      dVec rdiff = Path.Velocity(slice, slice+skip, ptcl);
      //dVec rdiff = Path(slice+skip,ptcl)-Path(slice,ptcl);
      //\\      dVec rp= Path(slice+skip,ptcl);
      dVec rpp=Path(slice+(skip>>1),ptcl);
      //      Path.PutInBox(r);
      //      Path.PutInBox(rp);
      //      Path.PutInBox(rpp);
      
      ///We've ignored boundary conditions here (well we think this is fixed but we're not sure)
      dVec rbar=r + 0.5*rdiff;
      dVec Delta= rpp - rbar;
      Path.PutInBox(Delta);
      
      double GaussProd=1.0;
      for (int dim=0; dim<NDIM; dim++) {
	double GaussSum = 0.0;
	int NumImage = 1;
	for (int image=-NumImage; image <= NumImage; image++) {
	  double dist = Delta[dim]+(double)image*Path.GetBox()[dim];
	  GaussSum += exp(-0.5*dist*dist/sigma2);
	}
	GaussProd *= GaussSum;
      }
      logSampleProb += prefactorOfSampleProb + log(GaussProd);

    }
  }
  //  cerr<<"My logSampleProb is "<<logSampleProb<<endl;
  return logSampleProb;
}



// This does a multilevel construction of a path.
bool BisectionClass::Bisect(int startSlice, int numLevels, 
			    Array<int,1> activeParticles,
			    double permActionChange)
{
  assert (numLevels <= PathData.Action.MaxLevels);
  bool toAccept=true;
  double oldLogSampleProb;
  double newLogSampleProb;
  Array<int,1> theParticles;
  int endSlice=(1<<numLevels)+startSlice;
  double prevActionChange=permActionChange;

  int levelCounter=numLevels-1;

  while (levelCounter>=0 && toAccept==true){
    SetMode(OLDMODE);
    toAccept=true;
    double oldAction = PathData.Action.TotalAction
      (startSlice,endSlice,activeParticles,levelCounter);
    oldLogSampleProb = LogSampleProb
      (startSlice,endSlice,activeParticles,levelCounter);
    SetMode(NEWMODE);
    newLogSampleProb = 
      SamplePaths(startSlice,endSlice,activeParticles,levelCounter);
    double testNewLogSampleProb=
      LogSampleProb(startSlice,endSlice,activeParticles,levelCounter);

    if (fabs(newLogSampleProb-testNewLogSampleProb)>1e-10){
      cerr<<newLogSampleProb<<" "<<testNewLogSampleProb<<" "<<fabs(newLogSampleProb-testNewLogSampleProb)<<endl;
    }
    assert(fabs(newLogSampleProb-testNewLogSampleProb)<1e-10);

    double newAction = PathData.Action.TotalAction
      (startSlice,endSlice, activeParticles,levelCounter);
    double currActionChange=newAction-oldAction;
    double neglogAcceptProb=
      -oldLogSampleProb+newLogSampleProb+currActionChange-prevActionChange;

    if (-neglogAcceptProb<log(PathData.Path.Random.Local())) ///reject conditin
      toAccept=false;
    if (toAccept)
      NumAccepted(levelCounter)++;
    else
      NumRejected(levelCounter)++;
    prevActionChange=currActionChange;
    levelCounter--;
  }
  return (toAccept);
}



bool BisectionClass::Bisect(int startSlice,int numLevels, 
			    Array<int,1> activeParticles)
{ 
  return Bisect (startSlice, numLevels, activeParticles, 0.0); 
}


void BisectionClass::AcceptanceRatios(Array<double,1> &ratios)
{
  ratios.resize(NumAccepted.size());
  for (int i=0; i<NumAccepted.size(); i++)
    ratios(i) = (double)NumAccepted(i)/(double)(NumAccepted(i)+NumRejected(i));
}
