#include "ConjGrad.h"

main()
{
  Vec3 box(25.0, 25.0, 25.0);
  IOSectionClass in;
  in.OpenFile ("NaUnscreenedPH_Feb18_05.h5");
  Potential *ph = ReadPotential(in);
  in.CloseFile();
  Hamiltonian H(box, 5.0, 1.0, *ph);
  ConjGrad CG(H);
  clock_t start, end;
  start = clock();
  for (int i=0; i<30; i++)
    CG.Iterate();
  end = clock();

  FILE *fout = fopen ("Narho.dat", "w");
  Vec3 r(0.0, 0.0, 0.0);
  for (double x=-0.5*box[0]; x<=0.5*box[0]; x+=0.01) {
    r[0] = x;
    complex<double> psi(0.0, 0.0);
    for (int i=0; i<H.GVecs.size(); i++) {
      double phase = dot (H.GVecs(i), r);
      complex<double> z(cos(phase), sin(phase));
      psi += z * CG.c(i);
    }
    psi /= sqrt(H.GVecs.GetBoxVol());
    fprintf (fout, "%1.12e %1.12e\n", x, real(conj(psi)*psi));
  }
  fclose (fout);
     

  fprintf (stderr, "Time = %1.3f\n", 
	   (double)(end-start)/(double)CLOCKS_PER_SEC);

}
