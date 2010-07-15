#include <math.h>

#ifdef DOUBLE
typedef double REAL;
#else
typedef float REAL;
#endif
/****************************************************************************/

/* returns a normally distributed deviate with zero mean and unit
   variance, using drand48() as the source of uniform deviates */

REAL gasdev()
{
   static int iset=0;
   static REAL gset;
   REAL fac,r,v1,v2;
   double drand48();

   if (iset == 0) {
     do {                          /* We don't have an extra deviate handy,so */
        v1=2.0*drand48()-1.0;     /* pick two uniform variates in the square */
        v2=2.0*drand48()-1.0;   /* extending from -1 to +1 in each direction */
        r=v1*v1+v2*v2;             /* see if they are in the unit circle, and */
     } while (r >=1.0 || r == 0.0);             /* if they are not, try again */

     fac = sqrt(-2.0*log(r)/r);
     /* Now make the Box-Muller transformation to get two normal deviates.
        Return one and save the other for next time. */
     gset = v1*fac;
     iset = 1;                 /* Set flag */
     return(v2*fac);
   } else {                   /* We have an extra deviate handy, so */
      iset=0;                 /* unset the flag,                    */
      return(gset);           /* and return it.                     */
   }
}

/**************************************************************************/

 
                                
 
/*
#define M1 259200
#define IA1 7141
#define IC1 54773
#define RM1 (1.0/M1)
#define M2 134456
#define IA2 8121
#define IC2 28411
#define RM2 (1.0/M2)
#define M3 243000
#define IA3 4561
#define IC3 51349
*/

/* returns a uniform random deviate between 0.0 and 1.0. set idum to any
   negative value to initialize or reinitialize the sequence. */


/* REAL ran1(idum)
int *idum;
{
    static long ix1,ix2,ix3;
    static REAL r[98];
    REAL temp;
    static int iff=0;
    int j;

    if (*idum < 0 || iff == 0) {
        iff =1;
        ix1=(IC1-(*idum)) % M1;
        ix1=(IA1*ix1+IC1) % M1;
        ix2=ix1 % M2;
        ix1=(IA1*ix1+IC1) % M1;
        ix3=ix1 % M3;
        for (j=1; j<=97; j++) {
          ix1=(IA1*ix1+IC1) % M1;
          ix2=(IA2*ix2+IC2) % M2;
          r[j]=(ix1+ix2*RM2)*RM1;
        }
        *idum=1;
    }
    ix1=(IA1*ix1+IC1) % M1;
    ix2=(IA2*ix2+IC2) % M2;
    ix3=(IA3*ix3+IC3) % M3;
    j=1 + ((97*ix3)/M3);
    if (j > 97 || j < 1) perror("RAN1: This cannot happen.");
    temp=r[j];
    r[j]=(ix1+ix2*RM2)*RM1;
    return(temp);
}
*/


