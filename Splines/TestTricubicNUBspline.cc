#include "TricubicNUBspline.h"
#include "TricubicBspline.h"

void
TestLinear()
{
  LinearGrid xGrid (1.0, 5.0, 22);
  LinearGrid yGrid (2.0, 9.0, 31);
  LinearGrid zGrid (3.1, 5.7, 19);

  TricubicNUBspline<double> NUBspline;
  TricubicBspline<double> Bspline;
  Array<double,3> data (xGrid.NumPoints-1, yGrid.NumPoints-1, zGrid.NumPoints-1);
  for (int ix=0; ix<xGrid.NumPoints-1; ix++)
    for (int iy=0; iy<yGrid.NumPoints-1; iy++)
      for (int iz=0; iz<zGrid.NumPoints-1; iz++) 
	data(ix,iy,iz) = 2.0*(drand48()-0.5);
  Bspline.Init (xGrid.Start, xGrid.End, 
		  yGrid.Start, yGrid.End, 
		  zGrid.Start, zGrid.End, data);
  NUBspline.Init (&xGrid, &yGrid, &zGrid, data);

  FILE *Bfile, *NUBfile;
  Bfile = fopen ("triB.dat", "w");
  NUBfile = fopen ("triNUB.dat", "w");


  for (double x=1.0; x<5.0; x+=0.001) {
    double y = 2.342;
    double z = 5.2341;
    TinyVector<double,3> r(x,y,z), Bgrad, NUBgrad;
    double Bval, NUBval, Blapl, NUBlapl;
    TinyMatrix<double,3,3> B2, NUB2;
    Bspline.Evaluate(r, Bval, Bgrad, B2);
    NUBspline.Evaluate(r, NUBval, NUBgrad, NUB2);
    fprintf (Bfile, "%20.16e %20.16e %20.16e %20.16e %20.16e ", 
	     x, Bval, Bgrad[0], Bgrad[1], Bgrad[2]);
    fprintf (NUBfile, "%20.16e %20.16e %20.16e %20.16e %20.16e ", 
	     x, NUBval, NUBgrad[0], NUBgrad[1], NUBgrad[2]);
    for (int i=0; i<3; i++)
      for (int j=0; j<3; j++) {
	fprintf (NUBfile, "%20.16e ", NUB2(i,j));
	fprintf (Bfile, "%20.16e ", B2(i,j));
      }
    fprintf (Bfile, "\n");
    fprintf (NUBfile, "\n");
  }
  fclose (NUBfile);
  fclose(Bfile);

}

void
TestNonlinear()
{
  OptimalGrid2 xGrid, yGrid, zGrid;  
  xGrid.Init (1.1, 5.5, 3.0, 22);
  yGrid.Init (3.1, 8.3, 4.0, 25);
  zGrid.Init (2.1, 7.9, 5.0, 26);
  //   LinearGrid xGrid, yGrid, zGrid;
  //   xGrid.Init (1.1, 5.5, 22);
  //   yGrid.Init (3.1, 8.3, 25);
  //   zGrid.Init (2.1, 7.9, 26);
//   Array<double,1> points(11);
//   points = 0.0, 1.0, 1.4, 1.8, 6.0, 7.0, 9.0, 10.0, 12.8, 13.0, 15.0;
//   GeneralGrid xGrid, yGrid, zGrid;
//   xGrid.Init (points);
//   yGrid.Init (points);
//   zGrid.Init (points);

//  TricubicNUBspline<double, LinearGrid, LinearGrid, LinearGrid> spline;
//  TricubicNUBspline<double, GeneralGrid, GeneralGrid, GeneralGrid>
//  spline;
  TricubicNUBspline<double, OptimalGrid2, OptimalGrid2, OptimalGrid2> spline;

  Array<double,3> data (xGrid.NumPoints-1, yGrid.NumPoints-1, zGrid.NumPoints-1);
  for (int ix=0; ix<xGrid.NumPoints-1; ix++)
    for (int iy=0; iy<yGrid.NumPoints-1; iy++)
      for (int iz=0; iz<zGrid.NumPoints-1; iz++) 
	data(ix,iy,iz) = 2.0*(drand48()-0.5);
  spline.Init (&xGrid, &yGrid, &zGrid, data, PERIODIC, PERIODIC, PERIODIC);

  for (double x=1.1; x<=5.5; x+=0.001) {
    double y = 3.342;
    double z = 5.2341;
    TinyVector<double,3> r(x,y,z);
    double val = spline (r);
    fprintf (stderr, "%20.16e %20.16e\n", x, val);
  }

}

void
TestComplexNonlinear()
{
  OptimalGrid2 xGrid, yGrid, zGrid;  
  xGrid.Init (1.1, 5.5, 3.0, 22);
  yGrid.Init (3.1, 8.3, 4.0, 25);
  zGrid.Init (2.1, 7.9, 5.0, 26);
  TricubicNUBspline<complex<double>, OptimalGrid2, OptimalGrid2, OptimalGrid2> spline;
  
  Array<complex<double>,3> data (xGrid.NumPoints-1, yGrid.NumPoints-1, zGrid.NumPoints-1);
  for (int ix=0; ix<xGrid.NumPoints-1; ix++)
    for (int iy=0; iy<yGrid.NumPoints-1; iy++)
      for (int iz=0; iz<zGrid.NumPoints-1; iz++) 
	data(ix,iy,iz) = complex<double>(2.0*(drand48()-0.5),2.0*(drand48()-0.5)); 
  spline.Init (&xGrid, &yGrid, &zGrid, data, PERIODIC, PERIODIC, PERIODIC);

  for (double x=1.1; x<=5.5; x+=0.001) {
    double y = 3.342;
    double z = 5.2341;
    TinyVector<double,3> r(x,y,z);
    complex<double> val = spline (r);
    fprintf (stderr, "%20.16e %20.16e %20.16e\n", x, val.real(), val.imag());
  }

}



void SpeedTest()
{
  double xi=3.1, xf=7.9, yi=2.9, yf=10.0, zi=4.6, zf=14.0;
  int nx=35, ny=29, nz=44;
//   GeneralGrid xGrid, yGrid, zGrid;
//   Array<double,1> xp(nx), yp(ny), zp(nz);
//   for (int ix=0; ix<nx; ix++)
//     xp(ix) = xi + (double)ix*(xf-xi)/(nx-1);
//   for (int iy=0; iy<ny; iy++)
//     yp(iy) = yi + (double)iy*(yf-yi)/(ny-1);
//   for (int iz=0; iz<nz; iz++)
//     zp(iz) = zi + (double)iz*(zf-zi)/(nz-1);
  
//   xGrid.Init(xp);
//   yGrid.Init(yp);
//   zGrid.Init(zp);
  LinearGrid xGrid(xi, xf, nx);
  LinearGrid yGrid(yi, yf, ny);
  LinearGrid zGrid(zi, zf, nz);

  Array<double,3> data(nx-1,ny-1,nz-1);
  for (int ix=0; ix<nx-1; ix++)
    for (int iy=0; iy<ny-1; iy++)
      for (int iz=0; iz<nz-1; iz++)
	data(ix,iy,iz) = 2.0*(drand48()-0.5);
  TricubicBspline<double> B;
  TricubicNUBspline<double,LinearGrid,LinearGrid,LinearGrid> NUB;
  B.Init (xi, xf, yi, yf, zi, zf, data);
  NUB.Init (&xGrid, &yGrid, &zGrid, data);

  double val;
  TinyVector<double,3> r, grad;
  TinyMatrix<double,3,3> secDerivs;

  clock_t Bstart, Bend, NUBstart, NUBend;
  const int N = 10000000;
  Bstart = clock();
  for (int i=0; i<N; i++) {
    r[0] = xi + (xf-xi)*drand48();
    r[1] = yi + (yf-yi)*drand48();
    r[2] = zi + (zf-zi)*drand48();
    B.Evaluate(r, val, grad, secDerivs);
  }
  Bend   = clock();

  NUBstart = clock();
  for (int i=0; i<N; i++) {
    r[0] = xi + (xf-xi)*drand48();
    r[1] = yi + (yf-yi)*drand48();
    r[2] = zi + (zf-zi)*drand48();
    NUB.Evaluate(r, val, grad, secDerivs);
  }
  NUBend   = clock();

  fprintf (stderr, "B-spline time   = %0.5f\n", 
	   (double)(Bend-Bstart)/(double)CLOCKS_PER_SEC);
  fprintf (stderr, "NUB-spline time = %0.5f\n", 
	   (double)(NUBend-NUBstart)/(double)CLOCKS_PER_SEC);

}

void TestCenterGrid()
{
  CenterGrid grid;
  grid.Init (-1.0, 1.0, 10, 24);
//   for (int i=0; i<50; i++)
//     fprintf (stderr, "%22.16e\n", grid(i));

  // Check ReverseMap for correctness
  for (int i=0; i<10000; i++) {
    double x = 2.0*drand48()-1.0;
    int j = grid.ReverseMap(x);
    if (grid(j) > x) 
      cerr << "x=" << x << "  j=" << j << " grid(j)=" << grid(j) << endl;
    if (x >= grid(j+1)) 
      cerr << "x=" << x << "  j=" << j << " grid(j)=" << grid(j) << endl;
  }

  // Write out a nonuniform 
  NUBsplineBasis<CenterGrid> basis;
  basis.Init (&grid);
  TinyVector<double,4> b;
  for (double x=-1.0; x<1.0; x+= 0.001) {
    basis (x, b);
    int i0 = (100-grid.ReverseMap(x)+0)%4;
    int i1 = (100-grid.ReverseMap(x)+1)%4;
    int i2 = (100-grid.ReverseMap(x)+2)%4;
    int i3 = (100-grid.ReverseMap(x)+3)%4;
    fprintf (stdout, "%22.16e %22.16e %22.16e %22.16e %22.16e\n",
	     x, b[i0], b[i1], b[i2], b[i3]);
  }

}


main()
{
  //SpeedTest();
  TestCenterGrid();
  //  TestLinear();
  //  TestComplexNonlinear();
}
