#include "bspline.h"
#include <stdio.h>

void
Test_1d_s()
{
  Ugrid grid;
  grid.start = 1.0;
  grid.end   = 3.0;
  grid.num = 11;
  float data[] = { 3.0, -4.0, 2.0, 1.0, -2.0, 0.0, 3.0, 2.0, 0.5, 1.0, 4.0 };
  BCtype_s bc;
  bc.lCode = NATURAL;
  bc.rCode = FLAT;
  
  UBspline_1d_s *spline = (UBspline_1d_s*) create_UBspline_1d_s (grid, bc, data);
/*   for (int i=0; i<13; i++) */
/*     fprintf (stderr, "%10.12e\n", spline->coefs[i]); */
  for (double x=1.0; x<3.0; x+=0.001) {
    float val;
    eval_UBspline_1d_s (spline, x, &val);
    fprintf (stderr, "%1.5f %20.14f\n", x, val);
  }

}


main()
{
  Test_1d_s();

}
