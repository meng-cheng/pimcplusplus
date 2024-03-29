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

#include "BCSNodalActionClass.h"
#include "../PathDataClass.h"
#include <Common/MatrixOps/MatrixOps.h>

double 
BCSNodalActionClass::ActionImageSum (double L, double lambdaBeta, 
				      double disp)
{
  int numImages = 10;
  double sum = 0.0;
  double fourLambdaBetaInv = (lambdaBeta!=0.0) ?  1.0/(4.0*lambdaBeta) : 0.0;
  // If the images won't contributed anything, let's not worry
  // about image sums.
  if ((disp*disp*fourLambdaBetaInv) > 50.0)
    return (disp*disp*fourLambdaBetaInv);
  for (int image=-numImages; image<=numImages; image++) {
    double x = disp + (double)image*L;
    sum += exp (-(x*x)*fourLambdaBetaInv);
  }
  return (-log(sum));
}

double 
BCSNodalActionClass::ActionkSum (double L, double lambdaBeta, 
				  double disp)
{
  double kmax = sqrt (50.0/lambdaBeta);
  double kInc = 2.0*M_PI/L;

  if (lambdaBeta == 0.0)
    return (0.0);

  double sum = 0.5;
  for (double k=kInc; k<=kmax; k+=kInc)
    sum += cos(k*disp)*exp(-lambdaBeta*k*k);
  return (-log(sum));
}


void 
BCSNodalActionClass::SetupFreeActions()
{
  const int nPoints = 1000;
  // Setup grids
  for (int i=0; i<NDIM; i++)
    ActionGrids[i].Init (-0.5*Path.GetBox()[i], 0.5*Path.GetBox()[i], nPoints);

  Array<double,1> actionData(nPoints);
  int nSplines = Path.TotalNumSlices/2 + (Path.TotalNumSlices%2)+1;
  double lambdaTau = Path.tau * Path.Species(SpeciesNumA).lambda;


  /// DEBUG
  FILE *fout;
  fout = fopen ("FreeImageActions.dat", "w");
  // Now, setup up actions
  ActionSplines.resize(nSplines);
  for (int spline=0; spline<nSplines; spline++) {
    double lambdaBeta = lambdaTau * (double)spline;
    for (int dim=0; dim<NDIM; dim++) {
      double L = Path.GetBox()[dim];
      for (int i=0; i<nPoints; i++) {
	double disp = ActionGrids[dim](i);
	actionData(i) = ActionImageSum (L, lambdaBeta, disp) -
	  ActionImageSum(L, lambdaBeta, 0.0);
	fprintf (fout, "%1.12e ", actionData(i));
      }
      fprintf (fout, "\n");
      // Since the action is periodic, the slope should be zero
      // at the boundaries
      ActionSplines(spline)[dim].Init (&ActionGrids[dim], actionData,
				       0.0, 0.0);
    }
  }
  fclose (fout);

}

///maps number to array of columns
// void 
// BCSNodalActionClass::DetNum2Cols(int i,Array<int,1> cols)
// {
//   if (i==0){
//     cols[0]=0;
//     cols[1]=1;
//   }
//   else if (i==1){
//     cols[0]=0;
//     cols[1]=2;
//   }
//   else if (i==2){
//     cols[0]=1;
//     cols[1]=2;
//   }
//   return;
   

// }

//Assuming partciel A is large
// Array<complex<double>,1>
// BCSNodalActionClass::CalcAllDet(int slice)
// {

//   Array<complex<double>,2> M=DetInformation(0,slice,Range(0,matSize),Range(0,matSize));
//   Array<complex<double>,2> kColumns=DetInformation(0,slice,Range(matSize+1,DetInformation.size()),Range(matSize+1,DetInformation.size()));

//   MCofactorArray[0]=M;
//   DetArray[0]=ComplexDetCofactors (MCofactorArray[0],WorkSize);
//   Array<int,1> cols;
  
//   for (i=1;i<MCofactorArray.size();i++){
//     MCofactorArray[i]=M;
//     DetNum2Cols(i,cols);
//     SwapCols(M(
//     int firstVecToChange=M.size()-cols.size()+1;
//     for (int j=0;j<cols.size();j++)
//       M(firstVecToChange+j,Range::all())=kColumns(j,Range::all());
//     DetArray[i]=ComplexDetCofactors(MCofactorArray[i],WorkSize);
//     //    DetArray[i]=UpdateDet(
//   }
  
     
    



  



//   Array<complex<double>, 2> MInverse;
//   Array<complex<double>,2> M;
//   M.resize(matSize,matSize);
//   M=origMat;
//   Array<complex<double>,1> changedCol;
//   int colNum;
//   Array<complex<double>,2> MinverseUpdate;
  
//   ///Update and compute all the actions..
//   for (int i=0;i<numDets;i++){
//     col1=col1Array(i);
//     inverseIndex=inverseIndexArray(i);
//     changedCol=changedColArray(i);
//     M(Range::all(),changedCol)=DetInformation(0,slice,Range::all(),col1);
//     MInverse=&MInverseArray(inverseIndex);
//     MInverseUpdate=&MInverseArray(i);
//     CalcNewDet(M,MInverse,DetInformation(0,slice,Range::all(),changedCol),changedCol,
// 	       DetArray(inverseIndex),true,MinverseUpdate);
//   }
//   return DetArray;

//}

// void 
// CalcNewDet(Array<complex<double>,2> origMat,Array<complex<double>,2> origMatInverse,
// 	   Array<complex<double>,1> changedCol,int colNum,double oldDet,
// 	   bool inverseWanted,Array<complex<double>,2> updatedInverse)
// {
//   Array<complex<double>,1> MInverseU=MatVecProd(origMatInverse,changedCol-origMat(colNum,Range::all()));
//   newDet=oldDet*(1+MInverseU);
//   if (inverseWanted){
//     updatedInverse=origMatInverse+OuterProduct(MInverseU,origMatInverse(colNum,Range::all()));
//   }
  
  
// }

dVec GetK(int knum)
{
  dVec k;
  k[0]=0.0;
  k[1]=0.0;
  k[2]=0.0;
  return k;
  

}

void 
BCSNodalActionClass::BuildDetInformation(int slice)
{
  RInfo=DetInfo(slice,Range::all(),Range(0,NB-1));
  kInfo=DetInfo(slice,Range::all(),Range(NB,toEnd));
  SpeciesClass &speciesA = PathData.Path.Species(SpeciesA);
  SpeciesClass &speciesB = PathData.Path.Species(SpeciesB); 
  double dist;
  dVec diff;
  //do the \exp(-alpha*(r_a-r_b)^2 part
  for (int ptclA=speciesA.FirstPtcl;ptclA<=speciesA.LastPtcl;ptclA++){
    for (int ptclB=speciesB.FirstPtcl;ptclB<=speciesB.LastPtcl;ptclB++){
      PathData.Path.DistDisp(slice,ptclA,ptclB,dist,diff);
      RInfo(ptclA-speciesA.FirstPtcl,ptclB-speciesB.FirstPtcl)=
	exp(-alpha*dist*dist);
    }
  }
  //do the \exp(-ik\cdot r_a) part
  double r,i;
  for (int ptclA=speciesA.FirstPtcl;ptclA<=speciesA.LastPtcl;ptclA++){
    for (int kNum=0;kNum<NumK;kNum++){
      dVec disp=PathData.Path(slice,ptclA);
      dVec k=GetK(kNum);
      double phi=dot(k,disp);
      sincos(phi,&i,&r);
      complex<double> num(r,i);
      kInfo(ptclA-speciesA.FirstPtcl,kNum)=num;
    }
  }
}  

//   int firstSpeciesA=PathData.Path.Species(SpeciesNumA).FirstPtcl;
//   int firstSpeciesB=PathData.Path.Species(SpeciesNumB).FirstPtcl;
//   for (int ptclA=PathData.Path.Species(SpeciesNumA).FirstPtcl;
//        ptclA<=PathData.Path.Species(SpeciesNumA).LastPtcl;
//        ptclA++){
//     for (int ptclB=PathData.Path.Species(SpeciesNumB).FirstPtcl;
// 	 ptclB<=PathData.Path.Species(SpeciesNumB).LastPtcl;
// 	 ptclB++){
//       double dist;
//       dVec diff;
//       PathData.Path.DistDisp(slice,ptclA,ptclB,dist,diff);
//       int indexB=ptclB-firstSpeciesB;
//       int indexA=ptclA-firstSpeciesA;
//       DetInformation(0,slice,indexB,indexA)=
// 	exp(-alpha*dist*dist);
//       //      DetInformation(1,slice,indexB,indexA)=
//       //	-alpha*diff[0]*DetInformation(0,slice,ptclB,ptclA)
// 	//      DetInformation(2,slice,ptclB-firstSpeciesB,ptclA-firstSpeciesA)=
// 	//	-alpha*diff[1]*DetInformation(0,slice,ptclB-firstSpeciesB,ptclA-firstSpeciesA)
// 	//      DetInformation(3,slice,ptclB-firstSpeciesB,ptclA-firstSpeciesA)=
// 	//	-alpha*diff[2]*DetInformation(0,slice,ptclB-firstSpeciesB,ptclA-firstSpeciesA)
//     }
//   }
//   dVec k;
//   for (int j=PathData.Path.Species(SpeciesNumB).LastPtcl+1;
//        j<PathData.Path.Species(SpeciesNumB).LastPtcl+1+numK;
//        j++){
//     k=GetK(j-numK);
//     DetInformation(0,slice,j-firstSpeciesB,ptclA-firstSpeciesA).real=
//       cos(dot(k,diff));
//     DetInformation(0,slice,j-firstSpeciesB,ptclA-firstSpeciesA).imag=
//       sin(dot(k,diff));
//   }
  
void
BCSNodalActionClass::GetCols(int detNum,Array<int,1> cols)
{
  
}
  

void 
BCSNodalActionClass::CalcDetArray(int slice)
{
  Array<int,1> cols;
  for (int detNum=0;detNum<NumDets;detNum++){
    CofactorStorage(detNum,Range::all(),Range(0,NB-1))=RInfo;
    GetCols(detNum,cols);
    ///Put the matrix you wish to invert in the cofactor
    for (int i=0;i<cols.size();i++)
      CofactorStorage(detNum,Range::all(),i+NB)=kInfo(Range::all(),cols(i));
    Array<complex<double>,2> cofactor=CofactorStorage(detNum,Range::all(),Range::all());
    DetStorage(slice,detNum)=ComplexDetCofactors (cofactor,WorkSize);
  }
}


BCSNodalActionClass::BCSNodalActionClass (PathDataClass &pathData,
					    int speciesNum) :
  NodalActionClass (pathData), 
  Path (pathData.Path)
{

//   int NA = Path.Species(SpeciesNumA).LastPtcl - 
//     Path.Species(SpeciesNumA).FirstPtcl+1;
//   int NB = Path.Species(SpeciesNumB).LastPtcl - 
//     Path.Species(SpeciesNumB).FirstPtcl+1;
//   int sizeCol=max(NA,NB);
//   int sizeRow=min(NA,NB)+numK;
//   int NumDets=choose(NumK,abs(NA-NB));
//   //DetInverse.resize(PathData.NumTimeSlices(),numInverse,max(NA,NB),max(NA,NB));
//   DetInformation.resize(PathData.NumTimeSlices(),4,sizeRow,sizeCol);
//  RefSliceDets.resize(numInverse);
//  DetMatrix.resize(N,N);
//  Cofactors.resize(N,N);
//  GradVec.resize(N);
//  SavePath.resize(N);
//  NumLineDists = NumGradDists = 0;
}


double
BCSNodalActionClass::Det (int slice)
{
  SpeciesClass &species = Path.Species(SpeciesNumA);
  int first = species.FirstPtcl;
  int last = species.LastPtcl;

  int myStartSlice, myEndSlice;
  int myProc = PathData.Path.Communicator.MyProc();
  Path.SliceRange (myProc, myStartSlice, myEndSlice);
  int refSlice = Path.GetRefSlice()-myStartSlice;
  int sliceDiff = abs(slice-refSlice);
  sliceDiff = min (sliceDiff, Path.TotalNumSlices-sliceDiff);
  assert (sliceDiff <= Path.TotalNumSlices);
  assert (sliceDiff > 0);

  // Fill up determinant matrix
  for (int refPtcl=species.FirstPtcl; refPtcl<=species.LastPtcl; refPtcl++) {
    for (int ptcl=species.FirstPtcl; ptcl<=species.LastPtcl; ptcl++) {
      dVec diff;
      double dist;
      Path.RefDistDisp (slice, refPtcl, ptcl, dist, diff);
      double action = 0.0;
      for (int dim=0; dim<NDIM; dim++)
	action += ActionSplines(sliceDiff)[dim](diff[dim]);
      DetMatrix(refPtcl-first, ptcl-first) = exp(-action);
    }
  }
  cerr<<DetMatrix<<endl;

  return Determinant (DetMatrix);
}

// Array<double,2>
// BCSNodalActionClass::GetMatrix (int slice)
// {
//   SpeciesClass &species = Path.Species(SpeciesNum);
//   int first = species.FirstPtcl;
//   int last = species.LastPtcl;

//   int myStartSlice, myEndSlice;
//   int myProc = PathData.Path.Communicator.MyProc();
//   Path.SliceRange (myProc, myStartSlice, myEndSlice);
//   int refSlice = Path.GetRefSlice()-myStartSlice;
//   int sliceDiff = abs(slice-refSlice);
//   sliceDiff = min (sliceDiff, Path.TotalNumSlices-sliceDiff);
//   assert (sliceDiff <= Path.TotalNumSlices);
//   assert (sliceDiff > 0);

//   // Fill up determinant matrix
//   for (int refPtcl=species.FirstPtcl; refPtcl<=species.LastPtcl; refPtcl++) {
//     for (int ptcl=species.FirstPtcl; ptcl<=species.LastPtcl; ptcl++) {
//       dVec diff;
//       double dist;
//       Path.RefDistDisp (slice, refPtcl, ptcl, dist, diff);
//       double action = 0.0;
//       for (int dim=0; dim<NDIM; dim++)
// 	action += ActionSplines(sliceDiff)[dim](diff[dim]);
//       DetMatrix(refPtcl-first, ptcl-first) = exp(-action);
//     }
//   }
//   return DetMatrix;
// }

bool
BCSNodalActionClass::IsPositive (int slice)
{
  int myStartSlice, myEndSlice;
  int myProc = PathData.Path.Communicator.MyProc();
  Path.SliceRange (myProc, myStartSlice, myEndSlice);
  int refSlice = Path.GetRefSlice()-myStartSlice;
  int sliceDiff = slice - refSlice;
  sliceDiff = min(sliceDiff, Path.TotalNumSlices-sliceDiff);
  cerr << "slice=" << slice << " sliceDiff = " << sliceDiff << endl;
  if (sliceDiff == 0)
    return (true);
  else
    return (Det(slice) > 0.0);
}


void 
BCSNodalActionClass::GradientDet (int slice, double &det, 
				   Array<dVec,1> &gradient)
{
  SpeciesClass &species = Path.Species(SpeciesNumA);
  int first = species.FirstPtcl;
  int last = species.LastPtcl;
  // Fill up determinant matrix
  int myStartSlice, myEndSlice;
  int myProc = PathData.Path.Communicator.MyProc();
  Path.SliceRange (myProc, myStartSlice, myEndSlice);
  int refSlice = Path.GetRefSlice()-myStartSlice;
  //double t = abs(refSlice-slice) * PathData.Action.tau;
  //double beta = PathData.Path.TotalNumSlices * PathData.Action.tau;
  int sliceDiff = abs(slice-refSlice);
  sliceDiff = min (sliceDiff, Path.TotalNumSlices-sliceDiff);
  assert (sliceDiff <= Path.TotalNumSlices);
  assert (sliceDiff > 0);
  //t = min (t, fabs(beta-t));
  //assert (t <= 0.500000001*beta);
  //double lambda = species.lambda;
  //double C = 1.0/(4.0*M_PI * lambda * t);

  // HACK HACK HACK for now;  should work for serial mode.
  //   if (Path.GetRefSlice() < Path.NumTimeSlices())
  //     for (int ptcl=0; ptcl<Path.NumParticles(); ptcl++)
  //       Path.RefPath(ptcl) = Path(Path.GetRefSlice(), ptcl);
  //   Path.RefPath.AcceptCopy();

  bool singular = false;
  for (int refPtcl=species.FirstPtcl; refPtcl<=species.LastPtcl; refPtcl++) {
    for (int ptcl=species.FirstPtcl; ptcl<=species.LastPtcl; ptcl++) {
      dVec diff;
      double dist;
      Path.RefDistDisp (slice, refPtcl, ptcl, dist, diff);
      double action = 0.0;
      for (int dim=0; dim<NDIM; dim++)
	action += ActionSplines(sliceDiff)[dim](diff[dim]);
      DetMatrix(refPtcl-first, ptcl-first) = exp(-action);
//        if (fabs(DetMatrix(refPtcl-first, ptcl-first)) < 1.0e-30) {
// 	 cerr << "detmatrix = " << DetMatrix(refPtcl-first, ptcl-first)<<endl;
// 	 cerr << "action = " << action << endl;
// 	 cerr << "diff = " << diff << endl;
// 	 cerr << "slicediff = " << sliceDiff << endl;
//        }
//       singular = singular || 
// 	(fabs(DetMatrix(refPtcl-first, ptcl-first)) < 1.0e-30)
	/* || isnan (DetMatrix(refPtcl-first, ptcl-first))*/;
    }
  }

//   cerr << "slice = " << slice << endl;
//   cerr << "RefSlice = " << Path.GetRefSlice() << endl;
//   cerr << "DetMatrix = " << endl << DetMatrix << endl;

  if (singular)  
    cerr << "Singular at slice=" << slice << endl;
  // Compute determinant
  det = Determinant (DetMatrix);
  if (singular) {
    gradient(0)[0] = sqrt(-1.0);
    return;
  }
  if (fabs(det)<1.0e-40) {
    cerr << "DetMatrix = " << DetMatrix << endl;
    for (int ptcl=first; ptcl<=last; ptcl++)
      cerr << "ptcl " << ptcl << " = " << Path(slice, ptcl) << endl;
    cerr << "Permutation = " << Path.Permutation.data() << endl;
  }
  Cofactors = DetMatrix;
  GJInverse (Cofactors);
  Transpose (Cofactors);
  Cofactors = det * Cofactors;


  // Now compute gradient of determinant
  for (int ptcl=species.FirstPtcl; ptcl<=species.LastPtcl; ptcl++) {
    gradient(ptcl-first) = 0.0;
    for (int refPtcl=species.FirstPtcl; refPtcl<=species.LastPtcl; refPtcl++) {
      dVec diff;
      double dist;
      Path.RefDistDisp (slice, refPtcl, ptcl, dist, diff);
      dVec gradPhi;
      for (int dim=0; dim<NDIM; dim++)
	gradPhi[dim] = -ActionSplines(sliceDiff)[dim].Deriv(diff[dim]) 
	  * DetMatrix(refPtcl-first, ptcl-first);
      //dVec gradPhi = -2.0*C*diff*DetMatrix(refPtcl-first,ptcl-first);

      gradient(ptcl-first) = 
	gradient(ptcl-first)+ gradPhi*Cofactors(refPtcl-first, ptcl-first);
    }
  }
}



void 
BCSNodalActionClass::GradientDetFD (int slice, double &det, 
				     Array<dVec,1> &gradient)
{
  SpeciesClass &species = Path.Species(SpeciesNumA);
  int first = species.FirstPtcl;
  int last = species.LastPtcl;
  // Fill up determinant matrix
  int myStartSlice, myEndSlice;
  int myProc = PathData.Path.Communicator.MyProc();
  Path.SliceRange (myProc, myStartSlice, myEndSlice);
  int refSlice = Path.GetRefSlice()-myStartSlice;
  double t = abs(refSlice-slice) * PathData.Path.tau;
  double beta = PathData.Path.TotalNumSlices * PathData.Path.tau;
  t = min (t, fabs(beta-t));
  double lambda = species.lambda;
  double C = 1.0/(4.0*M_PI * lambda * t);

  int sliceDiff = abs(slice-refSlice);
  sliceDiff = min (sliceDiff, Path.TotalNumSlices-sliceDiff);
  assert (sliceDiff <= Path.TotalNumSlices);

  // HACK HACK HACK for now;  should work for serial mode.
//   if (Path.GetRefSlice() < Path.NumTimeSlices())
//     for (int ptcl=0; ptcl<Path.NumParticles(); ptcl++)
//       Path.RefPath(ptcl) = Path(Path.GetRefSlice(), ptcl);

  for (int refPtcl=species.FirstPtcl; refPtcl<=species.LastPtcl; refPtcl++) {
    for (int ptcl=species.FirstPtcl; ptcl<=species.LastPtcl; ptcl++) {
      dVec diff;
      double dist;
      Path.RefDistDisp (slice, refPtcl, ptcl, dist, diff);
      double action = 0.0;
      for (int dim=0; dim<NDIM; dim++)
	action += ActionSplines(sliceDiff)[dim](diff[dim]);
      DetMatrix(refPtcl-first, ptcl-first) = exp(-action);
    }
  }


  // Compute determinant
  det = Determinant (DetMatrix);

  dVec disp;
  double dist;

  double eps = 1.0e-5;
  for (int ptcl=first; ptcl <= last; ptcl++) {
    dVec delta = 0.0;
    dVec &r = Path(slice, ptcl);
    double dplus, dminus;
    for (int dim=0; dim<NDIM; dim++) {
      delta = 0.0;
      delta[dim] = 0.5*eps;
      for (int ref=first; ref <= last; ref++) {
	Path.RefDistDisp (slice, ref, ptcl, dist, disp);
	dVec diff = disp + delta;
	double action = 0.0;
	for (int dim=0; dim<NDIM; dim++)
	  action += ActionSplines(sliceDiff)[dim](diff[dim]);
	DetMatrix(ref-first, ptcl-first) = exp(-action);
      }
      dplus = Determinant (DetMatrix);
      for (int ref=first; ref <= last; ref++) {
	Path.RefDistDisp (slice, ref, ptcl, dist, disp);
	dVec diff = disp - delta;
	double action = 0.0;
	for (int dim=0; dim<NDIM; dim++)
	  action += ActionSplines(sliceDiff)[dim](diff[dim]);
	DetMatrix(ref-first, ptcl-first) = exp(-action);
      }
      dminus = Determinant(DetMatrix);
      for (int ref=first; ref <= last; ref++) {
	Path.RefDistDisp (slice, ref, ptcl, dist, disp);
	dVec diff = disp;
	double action = 0.0;
	for (int dim=0; dim<NDIM; dim++)
	  action += ActionSplines(sliceDiff)[dim](diff[dim]);
	DetMatrix(ref-first, ptcl-first) = exp(-action);
      }
      gradient(ptcl-first)[dim] = (dplus-dminus)/eps;
    }
  }
}


double 
BCSNodalActionClass::NodalDist (int slice)
{
  SpeciesClass &species = Path.Species(SpeciesNumA);
  int first = species.FirstPtcl;
  int last = species.LastPtcl;

  double det;
  int N = last-first+1;

  GradientDet (slice, det, GradVec);
//   Array<dVec,1> gradFD(N);
//   double det2;
//   GradientDetFD (slice, det2, gradFD);
//   assert (det2 == det);
//   for (int i=0; i<N; i++) {
//     fprintf (stderr, "(%1.6e %1.6e %1.6e) (%1.6e %1.6e %1.6e) \n", 
//  	     gradFD(i)[0], gradFD(i)[1], gradFD(i)[2], 
//  	     GradVec(i)[0], GradVec(i)[1], GradVec(i)[2]);
//     for (int dim=0; dim<NDIM; dim++) 
//       assert (fabs(gradFD(i)[dim] - GradVec(i)[dim]) < 1.0e-7);
//   }

  double grad2 = 0.0;    
  for (int i=0; i<N; i++)
    grad2 += dot (GradVec(i), GradVec(i));

  
  double dist = det/sqrt(grad2);
  //   cerr << "grad = " << GradVec << endl;
  //   cerr << "dist = " << dist << endl;
//   dVec delta, deltaStar;
//   double d, dStar;
//   deltaStar = Path.RefPath(1) - Path.RefPath(0);
//   delta = Path(slice,1) - Path(slice,0);
//   Path.PutInBox (deltaStar);
//   Path.PutInBox (delta);
//   double trueDist = dot(delta, deltaStar)/sqrt(dot(deltaStar,deltaStar));


//   int myStartSlice, myEndSlice;
//   int myProc = PathData.Path.Communicator.MyProc();
//   Path.SliceRange (myProc, myStartSlice, myEndSlice);
//   int refSlice = Path.GetRefSlice()-myStartSlice;
//   int sliceDiff = abs(slice-refSlice);
//   sliceDiff = min (sliceDiff, Path.TotalNumSlices-sliceDiff);
  
//   if (sliceDiff < -2000000000)
//     return (trueDist);
//   else 
    return (dist);  
}



/// HybridDist first computes a distance with the 1/grad(ln(det))
/// method.  If this method says we're far from the nodes, we go with
/// that.  If it says we're close, call LineSearchDist to get a more
/// accurate value.
double BCSNodalActionClass::HybridDist (int slice, double lambdaTau)
{
  SpeciesClass &species = Path.Species(SpeciesNumA);
  int first = species.FirstPtcl;
  int last = species.LastPtcl;
  double det;
  int N = last-first+1;

  GradientDet (slice, det, GradVec);
  double grad2 = 0.0;    
  for (int i=0; i<N; i++)
    grad2 += dot (GradVec(i), GradVec(i));

  if (det < 0.0)
    return -1.0;

  double gradDist = det/sqrt(grad2);

  if (((NumGradDists+NumLineDists)%1000000) == 999999) {
    cerr << "Percent line searches = "
	 << (double)NumLineDists/(NumGradDists+NumLineDists) << endl;
  }
    
  // gradDist will almost always be a lower bound to the real
  // distance.  Therefore, if says we are far from the nodes, we
  // probably are and we can just use its value.
  if (gradDist > sqrt(5.0*lambdaTau)) {
    NumGradDists++;
    return (gradDist);
  }
  // However, if gradDist says we are close, we should check to see if
  // it is correct with a more accurate bisection search.
  else {
    NumLineDists++;
    return LineSearchDist(slice);
  }
}



double 
BCSNodalActionClass::MaxDist(int slice)
{
  if (Det(slice) < 0.0)
    return (-1.0);

  SpeciesClass &species = Path.Species(SpeciesNumA);
  int first = species.FirstPtcl;
  int last = species.LastPtcl;
  
  dVec disp;
  double dist;
  double minDist = 1.0e300;
  for (int ptcl1=first; ptcl1<last; ptcl1++)
    for (int ptcl2=ptcl1+1; ptcl2<=last; ptcl2++) {
      Path.DistDisp (slice, ptcl1, ptcl2, dist, disp);
      minDist = (dist < minDist) ? dist : minDist;
    }
  return minDist*M_SQRT1_2;
}


double 
BCSNodalActionClass::LineSearchDist (int slice)
{
  SpeciesClass &species = Path.Species(SpeciesNumA);
  double epsilon = 1.0e-4 * sqrt (4.0*species.lambda*Path.tau);
  int first = species.FirstPtcl;
  int last = species.LastPtcl;

  //  double maxDist = MaxDist (slice);
  double det0, det;
  int N = last-first+1;
  double retVal;

  // Save the current path
  for (int i=0; i < N; i++)
    SavePath(i) = Path(slice,i+first);

  GradientDet (slice, det0, GradVec);
  if (det0 < 0.0)
    return -1.0;

  double grad2=0.0;
  for (int i=0; i<N; i++)
    grad2 += dot (GradVec(i), GradVec(i));
  double gradMag = sqrt (grad2);

  for (int i=0; i<N; i++)
    GradVec(i) = (1.0/gradMag)*GradVec(i);

  double dist = det0/gradMag;


  double minFactor, maxFactor, tryFactor, newDet;
  minFactor = 0.0;
  maxFactor = 0.5;

  bool done = false;
  // First, find first sign change
  det = det0;
  double maxDist=sqrt(dot(PathData.Path.GetBox(),PathData.Path.GetBox()));
  while ((det*det0 > 0.0) && ((maxFactor*dist)<maxDist)) {
    maxFactor *= 2.0;
    for (int i=0; i<N; i++) 
      Path (slice, i+first) = SavePath(i) - maxFactor*dist*GradVec(i);
    det = Det(slice);
  }

  if (det*det0 >= 0.0)
    retVal = maxDist;
  else {
    // Now, do a bisection search for the sign change.
    while (((maxFactor-minFactor)*dist > epsilon) 
	   && (minFactor*dist < maxDist)) {
      tryFactor = 0.5*(maxFactor+minFactor);
      for (int i=0; i<N; i++)
	Path (slice, i+first) = SavePath(i) - tryFactor*dist*GradVec(i);
      det = Det (slice);
      if (det*det0 > 0.0)
	minFactor = tryFactor;
      else
	maxFactor = tryFactor;
    }
    if (minFactor*dist >= maxDist)
      retVal = maxDist;
    else 
      retVal = dist * tryFactor;
  }

  // Restore original Path position
  for (int i=0; i<N; i++)
    Path (slice, i+first) = SavePath(i);
  return retVal; //min (maxDist, retVal);
}



double 
BCSNodalActionClass::NewtonRaphsonDist (int slice)
{
  SpeciesClass &species = Path.Species(SpeciesNumA);
  int first = species.FirstPtcl;
  int last = species.LastPtcl;
  const int maxIter = 15;

  double det;
  int N = last-first+1;
  double retVal = 0.0;
  double maxBox = sqrt (dot(Path.GetBox(), Path.GetBox()));

  if (Det(slice) < 0.0)
    return (-1.0);

  // Save the current path
  Array<dVec,1> savePath(N), lastPath(N);
  for (int i=0; i<N; i++) {
    savePath(i) = Path(slice,i+first);
    lastPath(i) = Path(slice,i+first);
  }

  bool done = false;
  int numIter = 0;
  // Do Newton-Raphson iterations
  GradientDet (slice, det, GradVec);
  double grad2=0.0;
  for (int i=0; i<N; i++)
    grad2 += dot (GradVec(i), GradVec(i));
  double gradMag = sqrt (grad2);
  double firstDist = det/gradMag;
  double dist;
  
  while (!done && (numIter < maxIter)) {
    dist = 2.0*det/gradMag;
    do {
      dist *= 0.5;
      for (int i=0; i<N; i++)
	Path(slice, first+i) = lastPath(i) -(dist/gradMag)*GradVec(i);
    } while (Det (slice) < 0.0);

    for (int i=0; i<N; i++)
      lastPath(i) = Path(slice, first+i);
    
    if (dist < 0.001*firstDist)
      done = true;
    GradientDet (slice, det, GradVec);
    // If we come to a singular determinant matrix...
    if (isnan(GradVec(0)[0])) {
      for (int i=0; i<N; i++)
	Path(slice,first+i) = savePath(i);
      return (LineSearchDist(slice));
    }
    grad2 = 0.0;
    for (int i=0; i<N; i++)
      grad2 += dot (GradVec(i), GradVec(i));
    gradMag = sqrt (grad2);
    numIter++;
  }
    

//     if (det < 0.0) {
//       for (int i=0; i<N; i++)
// 	Path(slice, first+i) -= (dist/gradMag)*GradVec(i);
//       done = true;
//     }
//     else {
//       for (int i=0; i<N; i++)
// 	lastPath(i) = Path(slice, first+i);
//       double grad2=0.0;
//       for (int i=0; i<N; i++)
// 	grad2 += dot (GradVec(i), GradVec(i));
//       double gradMag = sqrt (grad2);
//       double dist = det/gradMag;
//       for (int i=0; i<N; i++)
// 	Path(slice, first+i) -= (dist/gradMag)*GradVec(i);
//       if (numIter > maxIter)
// 	done = true;
      
//       if ((det > 0.0) && (det < 0.0001*det0))
// 	done = true;
//       numIter++;
//     }
//   }
//   if (retVal != -1.0) {
  double totalDist = 0.0;
  for (int i=0; i<N; i++) {
    dVec diff = Path(slice, first+i) - savePath(i);
    totalDist += dot (diff,diff);
  }
  totalDist = sqrt (totalDist);

  // Restore original Path
  for (int i=0; i<N; i++)
    Path(slice,i+first)  = savePath(i);

  return totalDist;
}



//typedef struct
  //{
  //  size_t size;
  //  size_t stride;
  //  double *data;
  //  gsl_block *block;
  //  int owner;
  //}
  // gsl_vector;

  //int gsl_blas_dgemv (CBLAS_TRANSPOSE_t TransA, double alpha, const gsl_matrix * A, const gsl_vector * x, double beta, gsl_vector * y)  
  //  return gsl_blas_dgemv(CblasNoTrans,1,(gsl_matrix*)params,&Xgsl,0,&Bgsl);  
//}/*end matvec_gsl_matrix*/

// void calc_u(double* u, const gsl_matrix *Aold,const gsl_matrix *Anew, int moveParticle, int N){
//   for(int i=0;i<N;i++)
//     u[i]=gsl_matrix_get(Anew,moveParticle,i) - gsl_matrix_get(Aold,moveParticle,i);
// }/*end calc_u*/

void 
BCSNodalActionClass::Read (IOSectionClass &in)
{
  // Do nothing for now
  //  SetupFreeActions();
  //  assert(in.ReadVar("alpha",alpha));
  //  int matSize=DetInformation.extent(3);
  //  assert(in.ReadVar("NumAssymetry",NumAssymetry));
  //  WorkSize.resize (ComplexDetCofactorsWorksize(matSize));
  //  DetArray.resize(NumDets);
  //  MCofactorArray.resize(NumDets);
  //  for (i=0;i<MCofactorArray.size();i++)
  //    MCofactorArray[i].resize(matSize,matSize);
  //  cols.resize(NumAssymetry);
  string species1;
  string species2;
  assert(in.ReadVar("SpeciesA",species1));
  assert(in.ReadVar("SpeciesB",species2));
  SpeciesA=PathData.Path.SpeciesNum(species1);
  SpeciesB=PathData.Path.SpeciesNum(species2);
  NA=(PathData.Path.Species(SpeciesA).LastPtcl-PathData.Path.Species(SpeciesA).FirstPtcl+1);
  NB=(PathData.Path.Species(SpeciesB).LastPtcl-PathData.Path.Species(SpeciesB).FirstPtcl+1);
  NumAssymetry=NA-NB;
  //We assume Species A has more things
  assert(NumAssymetry>=0);
  assert(in.ReadVar("NumK",NumK));
  assert(in.ReadVar("NumDets",NumDets));
  DetInfo.resize(PathData.Path.NumTimeSlices(),NA,NB+NumK);
  CofactorStorage.resize(NumDets,NA,NA);
  //  for (int det=0;det<NumDets;det++)
  //    CofactorStorage[det].resize(NA,NA);
  DetStorage.resize(PathData.Path.NumTimeSlices(),NumDets);
  NodalDet.resize(NumDets);
  assert(in.ReadVar("alpha",alpha));
  WorkSize.resize (ComplexDetCofactorsWorksize(NA));
}


/// Return essentiall 0 or infinity
double 
BCSNodalActionClass::SingleAction (int startSlice, int endSlice,
				    const Array<int,1> &changePtcls,
				    int level)
{
  int skip = 1<<level;
  int myStart, myEnd;
  Path.SliceRange(PathData.Path.Communicator.MyProc(), myStart, myEnd);
  int refSlice = Path.GetRefSlice() - myStart;

  //if the reference slice hasn't been moved then do something normal
  if (!(startSlice<=refSlice && refSlice<=endSlice)){
    

  double uNode=0.0;
  for (int slice=startSlice; slice <= endSlice; slice+=skip) {
    if ((slice != refSlice) && (slice != refSlice+Path.TotalNumSlices))
      if (Det(slice) < 0.0)
	uNode += 1.0e50;
  }
  return uNode;
}


double 
BCSNodalActionClass::SimpleAction (int startSlice, int endSlice,
				    const Array<int,1> &changePtcls, 
				    int level)
{ 
  cerr<<"Entering into here"<<endl;
  /// HACK HACK HACK HACK
//   startSlice = 0;
//   endSlice = Path.NumTimeSlices()-1;
//   string mode = GetMode()==OLDMODE ? " Old mode" : " New mode";

  double uNode=0.0;

  SpeciesClass &species = Path.Species(SpeciesNumA);
  double lambda = species.lambda;
  int skip = 1<<level;
  double levelTau = PathData.Path.tau * (double)skip;

  int myStart, myEnd;
  Path.SliceRange(PathData.Path.Communicator.MyProc(), myStart, myEnd);
  int refSlice = Path.GetRefSlice() - myStart;
  //  cerr << "refSlice = " << refSlice << "  startSlice = " << startSlice << endl;

  double dist1, dist2;
  bool slice1IsRef, slice2IsRef;
  slice1IsRef = (startSlice == refSlice) || 
    (startSlice==refSlice+Path.TotalNumSlices);
  if (!slice1IsRef) {
    dist1 = HybridDist (startSlice, lambda*levelTau);
    // dist1 = NodalDist (startSlice);
    if (dist1 < 0.0) {
      // cerr << "node cross in species = " << species.Name << endl;
//       uNode = 1.0e100;
//       cerr << species.Name << " " << mode << " uNode = " << uNode << endl;
      return 1.0e100;
    }
  }
  else
    dist1 = sqrt(-1.0);
  
  int totalSlices = Path.TotalNumSlices;
  for (int slice=startSlice; slice < endSlice; slice+=skip) {
    //    cerr << "slice = " << slice << endl;
    if ((slice!=refSlice) && (slice != refSlice+Path.TotalNumSlices))
      if (Det(slice) < 0.0) 
	uNode += 1.0e50;
    slice2IsRef = (slice+skip == refSlice) || 
      (slice+skip == refSlice+totalSlices);
    if (!slice2IsRef) {
      //dist2 = MaxDist(slice+skip);//LineSearchDist (slice+skip);
      dist2 = HybridDist (slice+skip, lambda*levelTau);
      //dist2 = NodalDist (slice+skip);
//       cerr << "slice+skip = " << slice+skip << endl;
//       cerr << "dist2 = " << dist2 << endl;
      //fprintf (stderr, "%1.12e %1.12e\n", lineDist, dist2);
      if (dist2 < 0.0) {
	return 1.0e100;
      }
    }
    
    if (!slice1IsRef && (dist1<0.0)) {
      cerr << "slice = " << slice << "  dist1 = " << dist1 
	   << " refslice = " << refSlice << endl;
      uNode += 1.0e100;
    }
    else if (!slice2IsRef && (dist2 < 0.0))
      uNode += 1.0e100;
    else if (slice1IsRef || (dist1==0.0))
      uNode -= log1p(-exp(-dist2*dist2/(lambda*levelTau)));
    else if (slice2IsRef || (dist2==0.0))
      uNode -= log1p(-exp(-dist1*dist1/(lambda*levelTau)));
    else 
      uNode -= log1p(-exp(-dist1*dist2/(lambda*levelTau)));
    slice1IsRef = slice2IsRef;
    dist1 = dist2;
  }
//   cerr << species.Name << " " << mode << " uNode = " << uNode << endl;
  return uNode;
}


double 
BCSNodalActionClass::d_dBeta (int slice1, int slice2, int level)
{ 
  Array<int,1> changedPtcls(1);
  
  SpeciesClass &species = Path.Species(SpeciesNumA);
  double lambda = species.lambda;
  int skip = 1<<level;
  double levelTau = PathData.Path.tau * (double)skip;

  int myStart, myEnd;
  Path.SliceRange(PathData.Path.Communicator.MyProc(), myStart, myEnd);
  int refSlice = Path.GetRefSlice() - myStart;
  int sliceDiff1 = abs(slice1-refSlice);
  sliceDiff1 = min (sliceDiff1, PathData.Path.TotalNumSlices-sliceDiff1);
  
  double dist1, dist2;

  bool slice1IsRef, slice2IsRef;
  slice1IsRef = (slice1 == refSlice) || (slice1==refSlice+Path.TotalNumSlices);

  if (!slice1IsRef) {
    dist1 = HybridDist (slice1, lambda*levelTau);
    if (dist1 < 0.0) {
      cerr << "slice1 = " << slice1 << " refSlice = " << refSlice << endl;
      return 1.0e100;
    }
  }
  else
    dist1 = sqrt(-1.0);
  
  int totalSlices = Path.TotalNumSlices;
  double uNode=0.0;
  for (int slice=slice1; slice < slice2; slice+=skip) {
    int sliceDiff = slice-refSlice;
    sliceDiff = min (sliceDiff, PathData.Path.TotalNumSlices-sliceDiff);
    slice2IsRef = (slice+skip == refSlice) || 
      (slice+skip==refSlice+Path.TotalNumSlices);
    if (!slice2IsRef) {
      dist2 = HybridDist (slice+skip, lambda*levelTau);
      if (dist2 < 0.0){
	cerr << "slice2 = " << slice+skip << " refSlice = " << refSlice
	     << " species = " << species.Name << endl;
	return 1.0e100;
      }
    }

    double prod;
    if (slice1IsRef || (dist1==0.0))
      prod = dist2*dist2;
    else if (slice2IsRef || (dist2==0.0))
      prod = dist1*dist1;
    else
      prod = dist1*dist2;

    double prod_llt = prod/(lambda*levelTau);
    double exp_m1 = expm1 (prod_llt);
    if (fpclassify(exp_m1)==FP_NORMAL) 
      if (!isnan(exp_m1) && !isinf(exp_m1))
        if (isnormal(exp_m1))
	  if ((!isnan(prod_llt)) && (exp_m1 != 0.0))
	    uNode += prod_llt / (levelTau*exp_m1);
    if (isnan(uNode))
      cerr << "uNode broken again!\n";
    dist1 = dist2;
    slice1IsRef = slice2IsRef;
  }
  //  return 0.0;
  return uNode/(double)Path.TotalNumSlices;
}

NodeType BCSNodalActionClass::Type()
{
  return FREE_PARTICLE;
}


bool
BCSNodalActionClass::IsGroundState()
{
  return (false);
}

void
BCSNodalActionClass::WriteInfo (IOSectionClass &out)
{
  out.WriteVar ("Type", "FREE_PARTICLE");
}

string
BCSNodalActionClass::GetName()
{
  return "BCSNodal";
}
