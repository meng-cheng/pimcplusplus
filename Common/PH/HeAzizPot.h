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

#ifndef HEAZIZ_POT_H
#define HEAZIZ_POT_H

#include "PotentialBase.h"

class HeAzizPot : public Potential
{
private:
  static const double bb[7];
public:
  
  double V      (double r);
  double dVdr   (double r);
  double d2Vdr2 (double r);
  void Write(IOSectionClass &out);
  void Read (IOSectionClass &in);
};

#endif
