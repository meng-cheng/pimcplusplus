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

#include "InputOutput.h"

void TestCommaSplit();
void TestStripWhiteSpace();

void TestXMLRead()
{
  IOSectionClass xmlFile;

  xmlFile.OpenFile ("Test1.xml");
  xmlFile.PrintTree();
  cerr << "Past PrintTree()\n";
}

main()
{
  TestCommaSplit();
  TestStripWhiteSpace();
  TestXMLRead();
}
