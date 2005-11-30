#include "PermuteTableClass.cc"
#include "PermuteTableClass.h"
#include "PermuteTableOnClass.h"


// double PermuteTableOnClass::Sample (int &slice1, int &slice2,
// 				  Array<double,1> &activeParticles)
// {
//   if (activeParticles(0) == -1) {
//     // We need to choose the permutation now


//     return 1.0;
//   }
//   else {
//     // We need to compute the transition probability now
    
//     // Construct the reverse table;
    
//     // Calculate the reverse probability
    
//     // Return ratio
//   }
  
// }

    
Array<int,1> PermuteTableOnClass::CurrentParticles()
{
  Array<int,1> tempPtcl(CurrentCycle.Length);
  int firstPtcl=PathData.Species(SpeciesNum).FirstPtcl;
  for (int i=0;i<CurrentCycle.Length;i++){
    tempPtcl(i)=CurrentCycle.CycleRep(i)+firstPtcl;
  }
  return tempPtcl;
}

void PermuteTableOnClass::ConstructHTable()
{
  int firstPtcl = PathData.Species(SpeciesNum).FirstPtcl;
  int lastPtcl = PathData.Species(SpeciesNum).LastPtcl;
  double lambda = PathData.Species(SpeciesNum).lambda;
  double beta = PathData.Path.tau * (double) (Slice2-Slice1);
  double fourLambdaBetaInv = 1.0/(4.0*lambda*beta);
  int N = lastPtcl-firstPtcl+1;
  //    cerr<<"My N is "<<N<<endl;
  HTable.resize(N,N);
  for (int i=0; i<N; i++) {
      dVec disp_ii = PathData(Slice2,i+firstPtcl)-PathData(Slice1,i+firstPtcl);
      PathData.Path.PutInBox(disp_ii);
      double dist_ii = dot (disp_ii,disp_ii);
      for (int j=0; j<N; j++) {
	dVec disp_ij = PathData(Slice2,j+firstPtcl)-PathData(Slice1,i+firstPtcl);
	PathData.Path.PutInBox(disp_ij);
	double dist_ij = dot (disp_ij,disp_ij);
	HTable(i,j) = exp((-dist_ij + dist_ii)*fourLambdaBetaInv);
	if (HTable(i,j) < epsilon)
	  HTable(i,j) = 0.0;
      }
    // Now we should really sort with respect to j, but we won't for
    // now because we are far too lazy and it's a Friday.
  }
}

 
///// This constructs the Htable for the reverse move from the Htable
///// for the forward move.  i.e. Constructs Hrev(i,j) = H(P(i),P(j)).
// void PermuteTableOnClass::PermuteHTable(const CycleClass &myPerm,
// 				      const PermuteTableOnClass &forwardTable)
// {
//   int N=HTable.extent(0);
//   Array<int,1> P(PathData.NumParticles());
//   myPerm.CanonicalPermRep(P);
//   for (int i=0; i<N; i++) {
//     int P_i = P(i);
//     for (int j=0; j<N; j++) {
//       int P_j = P(j);
//       HTable(P_i,P_j) = forwardTable.HTable(i,j);
//     }
//   }
 
// } 
 
 
// Actually applies my cyclic permutation to a path at a given time
// slice and the permutation vector in the path
// void CycleClass::Apply(PathClass &path, int firstPtcl, int slice)
// {
//   SetMode(NEWMODE);
//   //  cerr<<CycleRep(0)<<" "<<firstPtcl<<" "<<slice<<endl;
//   dVec tempPos = path(slice, CycleRep(0)+firstPtcl);
//   int tempPtcl = path.Permutation(CycleRep(0)+firstPtcl);
//   for(int i=0;i<Length-1;i++) {
//     path.SetPos(slice, CycleRep(i)+firstPtcl, 
// 		path(slice,CycleRep(i+1)+firstPtcl));
//     path.Permutation(CycleRep(i)+firstPtcl) = 
//       path.Permutation(CycleRep(i+1)+firstPtcl);
//   }
//   path.SetPos(slice,CycleRep(Length-1)+firstPtcl,tempPos);
//   path.Permutation(CycleRep(Length-1)+firstPtcl) = tempPtcl;


// }


double PermuteTableOnClass::AttemptPermutation()
{
  //Get a random number number from the local processor stream.
//   Array<int,1> TestArray(NumEntries);
//   TestArray=0;
//   int index;
//   for (int counter=0;counter<1000000;counter++){
//     double xi=PathData.Path.Random.Local(); 
//     index=FindEntry(xi);
//     TestArray(index)++;
//   }
//   //  PrintTable();
//   //  cerr<<"Printing stuff: "<<endl;
//   for (int counter=0;counter<TestArray.size();counter++){
//     cerr<<TestArray(counter)<<endl;
//   }
  double xi=PathData.Path.Random.Local(); 
  int index=FindEntry(xi);
  CurrentCycle = CycleTable(index);
  //  cerr<<"Index is "<<xi<<" "<<index<<" "<<endl;
  //  for (int j=0; j<CurrentCycle.Length; j++)
  //    cerr << CurrentCycle.CycleRep[j] << " ";
  //  cerr<<"Done"<<endl;
  if (CurrentCycle.CycleRep[0]<0 || CurrentCycle.CycleRep[0]>10000){
    PrintTable();
  }
  //  cerr<<"After find entry"<<endl;
  // Now, apply the permutation to the Path
  int firstPtcl=PathData.Species(SpeciesNum).FirstPtcl;
  CurrentCycle.Apply(PathData.Path,firstPtcl,Slice2);
  
//   int diff = Slice2-Slice1;
//   int numLevels = 0;
//   while (diff != 1) {
//     diff >>= 1;
//     numLevels++;
//   }
  ///Remember to update distance table after permutation


  return (CurrentCycle.P * NormInv);
}

double PermuteTableOnClass::CalcReverseProb(const PermuteTableOnClass &forwardTable)
{
  
  // Reverse probabily for a single ptcl move is the same as that for the
  // forward move.
  if (forwardTable.CurrentCycle.Length == 1)
    return (forwardTable.CurrentCycle.P*forwardTable.NormInv);
  //We reconstruct things from scratch to make sure we do the right
  //thing. We can try to incrementally make this faster.
  ConstructCycleTable(forwardTable.SpeciesNum,
		      forwardTable.Slice1,forwardTable.Slice2,
		      forwardTable.ExcludeParticle); 
  //  cerr << "Forward Table:\n";
  //  forwardTable.PrintTable();
  //  cerr << "Reverse Table:\n";
  //  PrintTable();
  ///Correct for gamma factors here


//   int i=0;
//   while (!(CycleTable(i)==forwardTable.CurrentCycle))
//     i++;
//   if (fabs(CycleTable(i).P-
// 	   (1.0/(forwardTable.CurrentCycle.P)))>1e-12)
//     cerr <<"BAD! BAD! BAD! "
// 	 << CycleTable(i).P<<" "
// 	 <<1.0/(forwardTable.CurrentCycle.P);
//   }
  
  return (1.0/(forwardTable.CurrentCycle.P)*NormInv);
	 
	   
}


void PermuteTableOnClass::Read(IOSectionClass &inSection)
{
  Array<double,1> tempGamma;
  assert(inSection.ReadVar("Gamma",tempGamma));
  assert(tempGamma.size() == 4);
  for (int counter=0;counter<tempGamma.size();counter++){
    Gamma(counter)=tempGamma(counter);
  }
  assert(inSection.ReadVar("epsilon",epsilon));
  //  assert(inSection.ReadVar("SpeciesNum",SpeciesNum));
  
  
}


void PermuteTableOnClass::PrintTable() const
{
  for (int i=0; i<NumEntries; i++) {
    const CycleClass &cycle = CycleTable(i);
    cerr << "Length = " << cycle.Length << endl;
    cerr << "Cycle = [";
    for (int j=0; j<cycle.Length; j++)
      cerr << cycle.CycleRep[j] << " ";
    cerr << "]\n";
    cerr << "P = " << cycle.P << " C = " << cycle.C << endl;
    cerr << endl;
  }    
}

void PermuteTableOnClass::ConstructCycleTable(int speciesNum,
					    int slice1, int slice2)
{
  ConstructCycleTable(speciesNum,slice1,slice2,-1);

}


///the Gamma's must all be greater then 1 or this won't do the correct
///thing.  Allows you to exclude a particle when producing the table
///(say for example the open particle).
void PermuteTableOnClass::ConstructCycleTable(int speciesNum,
					    int slice1, int slice2,
					    int particleExclude)
{
  Slice1=slice1;
  Slice2=slice2;
  SpeciesNum=speciesNum;
  ExcludeParticle=particleExclude;
  ConstructHTable();
  int firstPtcl = PathData.Species(SpeciesNum).FirstPtcl;
  int lastPtcl= PathData.Species(SpeciesNum).LastPtcl;
  CycleTable.resize(TableSize);
  NumEntries = 0;
  int N=HTable.extent(0);
  int xBox,yBox,zBox;
  int xEffect=PathData.Path.Cell.Xeffect;
  int yEffect=PathData.Path.Cell.Yeffect;
  int zEffect=PathData.Path.Cell.Zeffect;
  double lambda = PathData.Species(SpeciesNum).lambda;
  double beta = PathData.Path.tau * (double) (Slice2-Slice1);
  double fourLambdaBetaInv = 1.0/(4.0*lambda*beta);
  for (int ptcl1=firstPtcl;ptcl1<lastPtcl;ptcl1++){
    CycleClass tempPerm;
    tempPerm.Length=1;
    tempPerm.CycleRep[0]=ptcl1;
    tempPerm.P=Gamma(0);
    tempPerm.C=tempPerm.P;
    if (NumEntries!=0){
      tempPerm.C+=CycleTable(NumEntries-1).C;
    }
    AddEntry(tempPerm);
    PathData.Path.Cell.FindBox(PathData.Path(Slice1,ptcl1),xBox,yBox,zBox);
    for (int xCell=xBox-xEffect;xCell<=xBox+xEffect;xCell++){
      for (int yCell=yBox-yEffect;yCell<=yBox+yEffect;yCell++){
	for (int zCell=zBox-zEffect;zCell<=zBox+zEffect;zCell++){
	  int rxbox,rybox,rzbox;
	  rxbox=(xCell+PathData.Path.Cell.GridsArray.extent(0)) % PathData.Path.Cell.GridsArray.extent(0);
	  rybox=(yCell+PathData.Path.Cell.GridsArray.extent(1)) % PathData.Path.Cell.GridsArray.extent(1);
	  rzbox=(zCell+PathData.Path.Cell.GridsArray.extent(2)) % PathData.Path.Cell.GridsArray.extent(2);
	  //	    cerr<<rxbox<<" "<<rybox<<" "<<rzbox<<endl;
	  list<int> &ptclList=PathData.Path.Cell.GridsArray(rxbox,rybox,rzbox).Particles(Slice2);
	  for (list<int>::iterator i=ptclList.begin();i!=ptclList.end();i++) {
	    int ptcl2=*i;
	    if (ptcl2!=ptcl1){
	      tempPerm.Length=2;
	      tempPerm.CycleRep[1]=ptcl2;
	      
	      dVec disp_ii = PathData(Slice2,ptcl1)-PathData(Slice1,ptcl1);
	      PathData.Path.PutInBox(disp_ii);
	      double dist_ii = dot (disp_ii,disp_ii);
	      dVec disp_ij = PathData(Slice2,ptcl2)-PathData(Slice1,ptcl1);
	      PathData.Path.PutInBox(disp_ij);
	      double dist_ij = dot (disp_ij,disp_ij);
	      double dist1 = exp((-dist_ij + dist_ii)*fourLambdaBetaInv);
	      
	      
	      disp_ii = PathData(Slice2,ptcl2)-PathData(Slice1,ptcl2);
	      PathData.Path.PutInBox(disp_ii);
	      dist_ii = dot (disp_ii,disp_ii);
	      disp_ij = PathData(Slice2,ptcl1)-PathData(Slice1,ptcl2);
	      PathData.Path.PutInBox(disp_ij);
	      dist_ij = dot (disp_ij,disp_ij);
	      double dist2 = exp((-dist_ij + dist_ii)*fourLambdaBetaInv);
	      
	      
	      tempPerm.P=Gamma(1)*dist1*dist2;
	      tempPerm.C=tempPerm.P+CycleTable(NumEntries-1).C;     
	      if (tempPerm.P > epsilon)
		AddEntry(tempPerm);
	    }
	  }
	}
      }
    }
  }
  if (OnlyOdd || PathData.Path.Species(SpeciesNum).GetParticleType()==FERMION){
    int currSpot=0;
    for (int counter=0;counter<NumEntries;counter++){
      if (CycleTable(counter).Length % 2 ==1){
	CycleTable(currSpot)=CycleTable(counter);
	currSpot++;
      }
    }
    NumEntries=currSpot;
    CycleTable(0).C=CycleTable(0).P;
    for (int counter=1;counter<NumEntries;counter++){
      CycleTable(counter).C=CycleTable(counter-1).C+CycleTable(counter).P;
    }
  }
  else if (OnlyEven) {
    int currSpot=0;
    for (int counter=0;counter<NumEntries;counter++){
      if (CycleTable(counter).Length % 2 ==0){
	CycleTable(currSpot)=CycleTable(counter);
	currSpot++;
      }
    }
    NumEntries=currSpot;
    CycleTable(0).C=CycleTable(0).P;
    for (int counter=1;counter<NumEntries;counter++){
      CycleTable(counter).C=CycleTable(counter-1).C+CycleTable(counter).P;
    }
  }

  Norm = CycleTable(NumEntries-1).C;
  NormInv = 1.0/Norm;
  //  cerr<<"The number of my table is "<<NumEntries<<endl;
}