#include "BicubicSpline.h"

void BCSplineTest1()
{
  LinearGrid xgrid(0.0, 4.0*M_PI, 200);
  LinearGrid ygrid(0.0, 4.0*M_PI, 100);
  
  Array<double,2> F(200,100);
  for (int i=0; i<200; i++) {
    double x = xgrid(i);
    for (int j=0; j<100; j++) {
      double y = ygrid(j);
      F(i,j) = sin(x)*sin(y);
    }
  }
  BicubicSpline BCspline;   
  BCspline.Init(&xgrid, &ygrid, F);
  FILE *exout = fopen ("BCexact", "w");
  FILE *spout = fopen ("BCspline", "w");
  /*LinearGrid finegrid(0.0,4.0*M_PI, 1000);
  for (int i=0; i<1000; i++) {
    double x = finegrid(i);
    for (int j=0; j<1000; j++) {
      double y = finegrid(j);      
      double ex = sin(x)*sin(y);
      fprintf (exout, "%1.16e ", ex);
      BCspline(x,y);
      fprintf (spout, "%1.16e ", BCspline(x,y));
    }
    fprintf (exout, "\n");
    fprintf (spout, "\n");
  }
  fclose(exout);
  fclose(spout);*/
  /*for (int k=0; k<10000000; k++) {
    double x = drand48()*4.0*M_PI;
    double y = drand48()*4.0*M_PI;
    BCspline(x,y);
    }*/
  


  LinearGrid x2(0.0, 4.0*M_PI, 10000);
  for (int i=0; i<10000; i++) {
    double x = xgrid(20);
    double y = x2(i);
    fprintf (stderr, "%1.17e %1.17e %1.17e %1.17e %1.17e %1.17e %1.17e %1.17e %1.17e\n", y,
	     BCspline(20, y), BCspline.Deriv(20,y), BCspline.Deriv2(20,y),
	     BCspline.Deriv3(20,y),
	     sin(x) * sin(y), sin(x)*cos(y), -sin(x)*sin(y), -sin(x)*cos(y));
  }
}


void BCSplineTest2()
{
  LinearGrid xgrid(0.0, 4.0*M_PI, 200);
  LinearGrid ygrid(0.0, 4.0*M_PI, 200);
  
  Array<double,2> F(200,200);
  for (int i=0; i<200; i++) {
    double x = xgrid(i);
    for (int j=0; j<200; j++) {
      double y = ygrid(j);
      F(i,j) = sin(x)*sin(y);
    }
  }
  BicubicSpline BCspline;   
  BCspline.Init(&xgrid, &ygrid, F);
  FILE *exout = fopen ("BCexact", "w");
  FILE *spout = fopen ("BCspline", "w");
  
  LinearGrid x2(0.0, 4.0*M_PI, 10000);
  for (int i=0; i<10000; i++) {
    double x = 2.1234567;
    double y = x2(i);
    fprintf (stderr, "%1.17e %1.17e %1.17e %1.17e %1.17e %1.17e %1.17e %1.17e %1.17e %1.17e, %1.17e %1.17e %1.17e\n", y,
	     BCspline(x, y),        sin(x)*sin(y),
	     BCspline.d_dx(x,y),    cos(x)*sin(y),
	     BCspline.d_dy(x,y),    sin(x)*cos(y),
	     BCspline.d2_dxdy(x,y), cos(x)*cos(y),
	     BCspline.d2_dx2(x,y),  -sin(x)*sin(y),
	     BCspline.d2_dy2(x,y),  -sin(x)*sin(y));
  }
}



main()
{
  BCSplineTest2();


}
