#include "MoveBase.h"


void MoveClass::WriteRatio()
{
  RatioVar.Write(AcceptanceRatio());
}

void MoveClass::MakeMove()
{
  TimesCalled++;
  if ((PathData.Path.Communicator.MyProc()==0) && 
      (TimesCalled % DumpFreq) == 0)
    WriteRatio();
}


void ParticleMoveClass::SetActiveSpecies (Array<int,1> ActSpecies)
{
  ActiveSpecies.resize(ActSpecies.size());
  ActiveSpecies = ActSpecies;
  ///This calculates the total number of particles
  TotalParticles = 0;
  for (int i=0; i<ActSpecies.size(); i++) {
    int CurrentNumPtcls = 
      PathData.Path.Species(i).NumParticles; 
    TotalParticles += CurrentNumPtcls;
  }
}


inline int ParticleMoveClass::RandInt (int Max) //Hopefully this didn't break anything
{
  double myRandNum;
  double myNum;
  myRandNum=PathData.Path.Random.Common();
  myNum=(double)Max * myRandNum;
    
  return ((int)floor(myNum));
}


//void ParticleMoveClass::ChooseParticles()
//{
//  for (int i=0;i<NumParticlesToMove;i++){
//    bool Redundant;
//    do {
//      MyParticleIndices(i)=PathData.Path.Random.CommonInt(TotalParticles);
      


//}

/// So do we still want to choose particles by dumping everything
/// into some mapping array from the active particles and dealing 
// with it that way? I think this is doing duplicate stuff in here.
void ParticleMoveClass::ChooseParticles()
{
  for (int i=0; i<NumParticlesToMove; i++) { 
    bool Redundant;
    do {
      MyParticleIndices(i) = RandInt(TotalParticles);
      while (PathData.Path.OpenPaths && 
	     MyParticleIndices(i)==(int)(PathData.Path.OpenPtcl)){ 
	//HACK!HACK!
	MyParticleIndices(i) = RandInt(TotalParticles);
      } 
      Redundant = false;
      for (int j=0; j<i; j++){
	  if (MyParticleIndices(i) == MyParticleIndices(j)){
	      Redundant = true;
	      break;
	  }
      }      
    } while (Redundant); 
  }
  for (int i=0; i<NumParticlesToMove; i++) 
    ActiveParticles(i) = MyParticleIndices(i);
  /// HACK HACK HACK
  //ActiveParticles(0) = 0;
}

/// So do we still want to choose particles by dumping everything
/// into some mapping array from teh active particles and dealing 
// with it that way? I think this is doing duplicate stuff in here.
void ParticleMoveClass::ChooseParticlesOpen()
{
  for (int i=0; i<NumParticlesToMove; i++) { 
    bool Redundant;
    do {
      MyParticleIndices(i) = RandInt(TotalParticles);
      Redundant = false;
      for (int j=0; j<i; j++){
	  if (MyParticleIndices(i) == MyParticleIndices(j)){
	      Redundant = true;
	      break;
	  }
      }      
    } while (Redundant); 
  }
  for (int i=0; i<NumParticlesToMove; i++) 
    ActiveParticles(i) = MyParticleIndices(i);
  /// HACK HACK HACK
  //ActiveParticles(0) = 0;
}
