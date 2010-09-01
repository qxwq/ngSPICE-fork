/**********
Copyright 2008 Holger Vogt
**********/
/* Care about random numbers
   The seed value is set as random number in main.c, line 746.
   A fixed seed value may be set by 'set rndseed=value'.
*/


/*
	CombLCGTaus()
	Combined Tausworthe & LCG random number generator
	Algorithm has been suggested in: 
	GPUGems 3, Addison Wesley, 2008, Chapter 37.
	It combines a three component Tausworthe generator taus88 
	(see P. L'Ecuyer: "Maximally equidistributed combined Tausworthe
	generators", Mathematics of Computation, 1996, 
	www.iro.umontreal.ca/~lecuyer/myftp/papers/tausme.ps)
	and a quick linear congruent generator (LCG), decribed in:
	Press: "Numerical recipes in C", Cambridge, 1992, p. 284.
	Generator has passed the bbattery_SmallCrush(gen) test of the
	TestU01 library from Pierre L�Ecuyer and Richard Simard,
	http://www.iro.umontreal.ca/~simardr/testu01/tu01.html
*/


/* TausSeed creates three start values for Tausworthe state variables.
   Uses rand() from <stdlib.h>, therefore values depend on the value of 
   seed in srand(seed). A constant seed will result in a reproducible
   series of random variates.
   
   Calling sequence:
   srand(seed);
   TausSeed();
   // generate random variates randvar uniformly distributed in 
   // [0.0 .. 1.0[ by calls to CombLCGTaus().
   double randvar = CombLCGTaus(void);
*/
//#define HVDEBUG

#include "ngspice.h"
#include "cpdefs.h"
#include "ftedefs.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef _MSC_VER
#include <process.h>
#else
#include <unistd.h>
#endif
#include <sys/timeb.h>
#include <stdarg.h>			// var. argumente

/* Tausworthe state variables for double variates*/
static unsigned CombState1 = 129, CombState2 = 130, CombState3 = 131;  
static unsigned CombState4 = 132; /* LCG state variable */ 

/* Tausworthe state variables for integer variates*/
static unsigned CombState5 = 133, CombState6 = 135, CombState7 = 137;  
static unsigned CombState8 = 138; /* LCG state variable */ 

static unsigned TauS(unsigned *state, int C1, int C2, int C3, unsigned m);
static unsigned LGCS(unsigned *state, unsigned A1, unsigned A2);

void TausSeed(void);
double CombLCGTaus(void);
unsigned int CombLCGTausInt(void);

void checkseed(void);
double drand(void);
double gauss(void);


/* Check if a seed has been set by the command 'set rndseed=value'
   in spinit with integer value > 0. If available, call srand(value).
   This will override the call to srand in main.c.
   Checkseed should be put in front of any call to random or rand.
*/
void checkseed(void)
{
   int newseed;
   static int oldseed;
/*   printf("Enter checkseed()\n"); */
   if (cp_getvar("rndseed", CP_NUM, &newseed)) {
      if ((newseed > 0) && (oldseed != newseed)) {
         srand(newseed); //srandom(newseed);
         TausSeed();
         oldseed = newseed;
         printf("Seed value for random number generator is set to %d\n", newseed);
      }
/*      else printf("Oldseed %d, newseed %d\n", oldseed, newseed); */
   }
   
}

/* uniform random number generator, interval -1 .. +1 */
double drand(void)
{
   checkseed();
//   return ( 2.0*((double) (RR_MAX-abs(rand())) / (double)RR_MAX-0.5));
   return 2.0 * CombLCGTaus() - 1.0;
}


void TausSeed(void)
{    
   /* The Tausworthe initial states should be greater than 128.
      We restrict the values up to 32767 */
   CombState1 = (unsigned int)((double)rand()/(double)RR_MAX * 32638.) + 129;
   CombState2 = (unsigned int)((double)rand()/(double)RR_MAX * 32638.) + 129;
   CombState3 = (unsigned int)((double)rand()/(double)RR_MAX * 32638.) + 129;
   CombState4 = (unsigned int)((double)rand()/(double)RR_MAX * 32638.) + 129;
   CombState5 = (unsigned int)((double)rand()/(double)RR_MAX * 32638.) + 129;
   CombState6 = (unsigned int)((double)rand()/(double)RR_MAX * 32638.) + 129;
   CombState7 = (unsigned int)((double)rand()/(double)RR_MAX * 32638.) + 129;
   CombState8 = (unsigned int)((double)rand()/(double)RR_MAX * 32638.) + 129;

#ifdef HVDEBUG
   printf("\nTausworthe Double generator init states: %d, %d, %d, %d\n", 
      CombState1, CombState2, CombState3, CombState4);
   printf("Tausworthe Integer generator init states: %d, %d, %d, %d\n", 
      CombState5, CombState6, CombState7, CombState8);
#endif
}   

static unsigned TauS(unsigned *state, int C1, int C2, int C3, unsigned m)
{
   unsigned b = (((*state << C1) ^ *state) >> C2);
   return *state = (((*state & m) << C3) ^ b);
}

static unsigned LGCS(unsigned *state, unsigned A1, unsigned A2)
{
   return *state = (A1 * *state + A2);
}


double CombLCGTaus()
{
   return 2.3283064365387e-10 * (
   TauS(&CombState1, 13, 19, 12, 4294967294UL) ^
   TauS(&CombState2, 2, 25, 4, 4294967288UL) ^
   TauS(&CombState3, 3, 11, 17, 4294967280UL) ^
   LGCS(&CombState4, 1664525, 1013904223UL)
   );
}
   
unsigned int CombLCGTausInt()
{
   return (
   TauS(&CombState5, 13, 19, 12, 4294967294UL) ^
   TauS(&CombState6, 2, 25, 4, 4294967288UL) ^
   TauS(&CombState7, 3, 11, 17, 4294967280UL) ^
   LGCS(&CombState8, 1664525, 1013904223UL)
   );
}
  

float CombLCGTaus2()
{
   unsigned long b;
   b = (((CombState1 << 13) ^ CombState1) >> 19);
   CombState1 = (((CombState1 & 4294967294UL) << 12) ^ b);
   b = (((CombState2 << 2) ^ CombState2) >> 25);
   CombState2 = (((CombState2 & 4294967288UL) << 4) ^ b);   
   b = (((CombState3 << 3) ^ CombState3) >> 11);
   CombState3 = (((CombState3 & 4294967280UL) << 17) ^ b);
   CombState4 = (1664525 * CombState4 + 1013904223UL);   
   return ((CombState1 ^ CombState2 ^ CombState3 ^ CombState4) *  2.3283064365387e-10f);
}


unsigned int CombLCGTausInt2()
{
   unsigned long b;
   b = (((CombState5 << 13) ^ CombState5) >> 19);
   CombState5 = (((CombState5 & 4294967294UL) << 12) ^ b);
   b = (((CombState6 << 2) ^ CombState6) >> 25);
   CombState6 = (((CombState6 & 4294967288UL) << 4) ^ b);   
   b = (((CombState7 << 3) ^ CombState7) >> 11);
   CombState7 = (((CombState7 & 4294967280UL) << 17) ^ b);
   CombState8 = (1664525 * CombState8 + 1013904223UL);   
   return (CombState5 ^ CombState6 ^ CombState7 ^ CombState8);
}


/***  gauss  ***/

double gauss(void)
{
  static bool gliset = TRUE;
  static double glgset = 0.0;
  double fac,r,v1,v2;
  if (gliset) {
    do {
      v1 = drand();  v2 = drand();
      r = v1*v1 + v2*v2;
    } while (r >= 1.0);
/*    printf("v1 %f, v2 %f\n", v1, v2); */
    fac = sqrt(-2.0 * log(r) / r);
    glgset = v1 * fac;
    gliset = FALSE;
    return v2 * fac;
  } else {
    gliset = TRUE;
    return glgset;
  }
}
