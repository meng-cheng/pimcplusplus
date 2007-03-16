#include "bspline.h"
#include <stdio.h>
#include <stdlib.h>

void
Test_1d_s()
{
  Ugrid grid;
  grid.start = 1.0;
  grid.end   = 3.0;
  grid.num = 11;
  float data[] = { 3.0, -4.0, 2.0, 1.0, -2.0, 0.0, 3.0, 2.0, 0.5, 1.0, 3.0 };
  BCtype_s bc;
  bc.lCode = DERIV2; bc.lVal = 10.0;
  bc.rCode = DERIV2; bc.rVal = -10.0;
  
  UBspline_1d_s *spline = (UBspline_1d_s*) create_UBspline_1d_s (grid, bc, data);
  for (double x=1.0; x<=3.00001; x+=0.001) {
    float val, grad, lapl;
    eval_UBspline_1d_s_vgl (spline, x, &val, &grad, &lapl);
    fprintf (stdout, "%1.5f %20.14f %20.14f %20.14f\n", x, val, grad, lapl);
  }

}


void
Test_2d_s()
{
  Ugrid x_grid, y_grid;
  x_grid.start = 1.0;  x_grid.end   = 3.0;  x_grid.num = 30;
  y_grid.start = 1.0;  y_grid.end   = 3.0;  y_grid.num = 30;
  
  float *data = malloc (x_grid.num * y_grid.num * sizeof(float));
  for (int ix=0; ix<x_grid.num; ix++)
    for (int iy=0; iy<y_grid.num; iy++)
      *(data + ix*y_grid.num + iy) = -1.0 + 2.0*drand48();
  BCtype_s x_bc, y_bc;
//   x_bc.lCode = NATURAL; x_bc.lVal = 10.0;
//   x_bc.rCode = NATURAL; x_bc.rVal = -10.0;
//   y_bc.lCode = NATURAL; y_bc.lVal = 10.0;
//   y_bc.rCode = NATURAL; y_bc.rVal = -10.0;
  x_bc.lCode = PERIODIC; x_bc.lVal = 10.0;
  x_bc.rCode = PERIODIC; x_bc.rVal = -10.0;
  y_bc.lCode = PERIODIC; y_bc.lVal = 10.0;
  y_bc.rCode = PERIODIC; y_bc.rVal = -10.0;
  
  UBspline_2d_s *spline = (UBspline_2d_s*) create_UBspline_2d_s (x_grid, y_grid, x_bc, y_bc, data); 

  for (double x=x_grid.start; x<=x_grid.end; x+=0.005) {
    for (double y=y_grid.start; y<=y_grid.end; y+=0.005) {
      float val, grad[2], hess[4];
	eval_UBspline_2d_s_vgh (spline, x, y, &val, grad, hess);
      fprintf (stdout, "%20.14f ", val);
    }
    fprintf (stdout, "\n");
  }

  int ix=5;
  int iy=7;
  float exval = data[ix*y_grid.num+iy];
  double x = x_grid.start + (double)ix * spline->x_grid.delta;
  double y = y_grid.start + (double)iy * spline->y_grid.delta;
  float spval;
  eval_UBspline_2d_s (spline, x, y, &spval);
  fprintf (stderr, "exval = %20.15f   spval = %20.15f\n", exval, spval);

}

void
Test_3d_s()
{
  Ugrid x_grid, y_grid, z_grid;
  x_grid.start = 1.0;  x_grid.end   = 3.0;  x_grid.num = 30;
  y_grid.start = 1.0;  y_grid.end   = 3.0;  y_grid.num = 30;
  z_grid.start = 1.0;  z_grid.end   = 3.0;  z_grid.num = 30;
  
  float *data = malloc (x_grid.num * y_grid.num * z_grid.num * sizeof(float));
  for (int ix=0; ix<x_grid.num; ix++)
    for (int iy=0; iy<y_grid.num; iy++)
      for (int iz=0; iz<z_grid.num; iz++)
	*(data + ((ix*y_grid.num) + iy)*z_grid.num + iz) = -1.0 + 2.0*drand48();
  BCtype_s x_bc, y_bc, z_bc;
  x_bc.lCode = PERIODIC; x_bc.rCode = PERIODIC; 
  y_bc.lCode = PERIODIC; y_bc.rCode = PERIODIC; 
  z_bc.lCode = PERIODIC; z_bc.rCode = PERIODIC; 
  
  UBspline_3d_s *spline = (UBspline_3d_s*) create_UBspline_3d_s 
    (x_grid, y_grid, z_grid, x_bc, y_bc, z_bc, data); 

  double z = 1.92341;
  FILE *fout = fopen ("3dspline.dat", "w");
  for (double x=x_grid.start; x<=x_grid.end; x+=0.005) {
    for (double y=y_grid.start; y<=y_grid.end; y+=0.005) {
      float val, grad[3], hess[9];
      eval_UBspline_3d_s (spline, x, y, z, &val);
      fprintf (fout, "%20.14f ", val);
    }
    fprintf (fout, "\n");
  }
  fclose (fout);

  int ix=15;  int iy=19; int iz = 24;
  float exval = data[(ix*y_grid.num+iy)*z_grid.num+iz];
  double x = x_grid.start + (double)ix * spline->x_grid.delta;
  double y = y_grid.start + (double)iy * spline->y_grid.delta;
  z =        z_grid.start + (double)iz * spline->z_grid.delta;
  float spval;
  eval_UBspline_3d_s (spline, x, y, z, &spval);
  fprintf (stderr, "exval = %20.15f   spval = %20.15f\n", exval, spval);

}


#include <time.h>
void
Speed_3d_s()
{
  Ugrid x_grid, y_grid, z_grid;
  x_grid.start = 1.0;  x_grid.end   = 3.0;  x_grid.num = 200;
  y_grid.start = 1.0;  y_grid.end   = 5.0;  y_grid.num = 200;
  z_grid.start = 1.0;  z_grid.end   = 7.0;  z_grid.num = 200;
  
  float *data = malloc (x_grid.num * y_grid.num * z_grid.num * sizeof(float));
  for (int ix=0; ix<x_grid.num; ix++)
    for (int iy=0; iy<y_grid.num; iy++)
      for (int iz=0; iz<z_grid.num; iz++)
	*(data + ((ix*y_grid.num) + iy)*z_grid.num + iz) = -1.0 + 2.0*drand48();
  BCtype_s x_bc, y_bc, z_bc;
  x_bc.lCode = PERIODIC; x_bc.rCode = PERIODIC; 
  y_bc.lCode = PERIODIC; y_bc.rCode = PERIODIC; 
  z_bc.lCode = PERIODIC; z_bc.rCode = PERIODIC; 
  
  UBspline_3d_s *spline = (UBspline_3d_s*) create_UBspline_3d_s 
    (x_grid, y_grid, z_grid, x_bc, y_bc, z_bc, data); 

  float val, grad[3], hess[9];
  clock_t start, end, rstart, rend;
  rstart = clock();
  for (int i=0; i<10000000; i++) {
    double x = x_grid.start+ drand48()*(x_grid.end - x_grid.start);
    double y = y_grid.start+ drand48()*(y_grid.end - y_grid.start);
    double z = z_grid.start+ drand48()*(z_grid.end - z_grid.start);
  }
  rend = clock();
  start = clock();
  for (int i=0; i<10000000; i++) {
    double x = x_grid.start+ drand48()*(x_grid.end - x_grid.start);
    double y = y_grid.start+ drand48()*(y_grid.end - y_grid.start);
    double z = z_grid.start+ drand48()*(z_grid.end - z_grid.start);
    eval_UBspline_3d_s_vgh (spline, x, y, z, &val, grad, hess);
  }
  end = clock();
  fprintf (stderr, "10,000,000 evalations in %f seconds.\n", 
	   (double)(end-start-(rend-rstart))/(double)CLOCKS_PER_SEC);
}


main()
{
  // Test_1d_s();
  // Test_2d_s();
  // Test_3d_s();
  Speed_3d_s();
}
