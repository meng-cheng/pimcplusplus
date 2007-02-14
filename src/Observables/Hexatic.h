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

#ifndef HEXATIC_H
#define HEXATIC_H

#include "ObservableBase.h"

class HexaticClass : public ObservableClass
{
protected:
  int q;
  double DistCutoff;
  Array<complex<double>,1> ParticleOrder;
  void ReadGrid(IOSectionClass &in);
  complex<double> OrderParamater(int slice,int ptcl);
  LinearGrid grid;
  Array<complex<double>,1> Histogram;
  Array<double,1> HistDouble;
  Array<double,1> HistSum;
  ObservableVecDouble1 HexaticRealVar;
  ObservableVecDouble1 HexaticImagVar;
  int NumSamples;
public:
  void Accumulate();
  void WriteBlock();
  void Read(IOSectionClass &in);
  HexaticClass(PathDataClass &pathData, IOSectionClass &ioSection) :
    ObservableClass (pathData, ioSection), 
    HexaticRealVar("HexaticReal",IOSection,pathData.Path.Communicator),
    HexaticImagVar("HexaticImag",IOSection,pathData.Path.Communicator),
    q(6), DistCutoff(3.5)
    {}

};

#endif