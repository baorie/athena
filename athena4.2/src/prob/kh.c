#include "copyright.h"
/*============================================================================*/
/*! \file kh.c
 *  \brief Problem generator for KH instability. 
 *
 * PURPOSE: Problem generator for KH instability.  Sets up two versions:
 * - iprob=1: slip surface with random perturbations
 * - iprob=2: tanh profile at interface, with single-mode perturbation	      */
/*============================================================================*/

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "athena.h"
#include "globals.h"
#include "prototypes.h"

/*==============================================================================
 * GLOBAL VARIABLES
 *============================================================================*/

static int iprob;
static Real vflow;

/*==============================================================================
 * PRIVATE FUNCTION PROTOTYPES:
 * ran2()
 *============================================================================*/

static double ran2(long int *idum);
#ifdef MHD
static Real hst_Bx(const GridS *pG, const int i, const int j, const int k);
static Real hst_By(const GridS *pG, const int i, const int j, const int k);
static Real hst_Bz(const GridS *pG, const int i, const int j, const int k);
#endif

/*=========================== PUBLIC FUNCTIONS ===============================*/
/*----------------------------------------------------------------------------*/
/* problem:  */

void problem(DomainS *pDomain)
{
  GridS *pGrid = pDomain->Grid;
  int i=0,j=0,k=0;
  int is,ie,js,je,ks,ke;
  Real amp,drat,b0,a,width,sigma,x1,x2,x3;
  long int iseed = -1;
  static int frst=1;  /* flag so new history variables enrolled only once */

  is = pGrid->is; ie = pGrid->ie;
  js = pGrid->js; je = pGrid->je;
  ks = pGrid->ks; ke = pGrid->ke;

/* Read problem parameters */

  iprob = par_geti("problem","iprob");
  vflow = par_getd("problem","vflow");
  drat  = par_getd("problem","drat");
  amp   = par_getd("problem","amp");
  width = par_getd("problem","width");
  sigma = par_getd("problem","sigma");
#ifdef MHD
  b0  = par_getd("problem","b0");
#endif

/* iprob=1.  Two uniform streams moving at +/- vflow, random perturbations */

  if (iprob == 1) {
    for (k=ks; k<=ke; k++) {
      for (j=js; j<=je; j++) {
        for (i=is; i<=ie; i++) {
          cc_pos(pGrid,i,j,k,&x1,&x2,&x3);
          pGrid->U[k][j][i].d = 1.0;
          pGrid->U[k][j][i].M1 = vflow + amp*(ran2(&iseed) - 0.5);
          pGrid->U[k][j][i].M2 = amp*(ran2(&iseed) - 0.5);
          pGrid->U[k][j][i].M3 = 0.0;
          if (fabs(x2) < 0.25) {
  	    pGrid->U[k][j][i].d = drat;
            pGrid->U[k][j][i].M1 = -drat*(vflow + amp*(ran2(&iseed) - 0.5));
            pGrid->U[k][j][i].M2 = drat*amp*(ran2(&iseed) - 0.5);
          }
/* Pressure scaled to give a sound speed of 1 with gamma=1.4 */
#ifndef BAROTROPIC
          pGrid->U[k][j][i].E = 2.5/Gamma_1
             + 0.5*(SQR(pGrid->U[k][j][i].M1) + SQR(pGrid->U[k][j][i].M2)
             + SQR(pGrid->U[k][j][i].M3))/pGrid->U[k][j][i].d;
#endif /* BAROTROPIC */
#ifdef MHD
          pGrid->B1i[k][j][i] = b0;
          pGrid->U[k][j][i].B1c = b0;
#ifndef BAROTROPIC
          pGrid->U[k][j][i].E += 0.5*b0*b0;
#endif /* BAROTROPIC */
#endif /* MHD */
        }
#ifdef MHD
      pGrid->B1i[k][j][ie+1] = b0;
#endif
      }
    }
  }

/* iprob=2.  Test suggested by E. Zweibel, based on Ryu & Jones.
 * Two uniform density flows with single mode perturbation
 */

  if (iprob == 2) {
    a = 0.05;
    sigma = 0.2;
    for (k=ks; k<=ke; k++) {
      for (j=js; j<=je; j++) {
        for (i=is; i<=ie; i++) {
          cc_pos(pGrid,i,j,k,&x1,&x2,&x3);
          pGrid->U[k][j][i].d = 1.0;
          pGrid->U[k][j][i].M1 = vflow*tanh(x2/a);
          pGrid->U[k][j][i].M2 = amp*sin(2.0*PI*x1)*exp(-(x2*x2)/(sigma*sigma));
          pGrid->U[k][j][i].M3 = 0.0;
#ifndef BAROTROPIC
          pGrid->U[k][j][i].E = 1.0/Gamma_1
             + 0.5*(SQR(pGrid->U[k][j][i].M1) + SQR(pGrid->U[k][j][i].M2)
             + SQR(pGrid->U[k][j][i].M3))/pGrid->U[k][j][i].d;
#endif /* BAROTROPIC */
#ifdef MHD
          pGrid->B1i[k][j][i] = b0;
          pGrid->U[k][j][i].B1c = b0;
#ifndef BAROTROPIC
          pGrid->U[k][j][i].E += 0.5*b0*b0;
#endif /* BAROTROPIC */
#endif /* MHD */
/* Use passive scalar to keep track of the fluids, since densities are same */
#if (NSCALARS > 0)
          pGrid->U[k][j][i].s[0] = 0.0;
          if (x2 > 0) pGrid->U[k][j][i].s[0] = 1.0;
#endif
        }
#ifdef MHD
      pGrid->B1i[k][j][ie+1] = b0;
#endif
      }
    }
  }

/* iprob=3. Modification of test suggested by E. Zweibel, based on Ryu & Jones.
 * Combination of iprob 1 and 2.
 */

  if (iprob == 3) {
    a     = width;
    for (k=ks; k<=ke; k++) {
      for (j=js; j<=je; j++) {
        for (i=is; i<=ie; i++) {
          cc_pos(pGrid,i,j,k,&x1,&x2,&x3);
          pGrid->U[k][j][i].d = 1.0;
          pGrid->U[k][j][i].M1 = vflow*tanh((fabs(x2)-0.5)/a);
          pGrid->U[k][j][i].M2 = amp*sin(2.0*PI*x1)*exp(-(SQR(fabs(x2)-0.5))/(sigma*sigma));
          pGrid->U[k][j][i].M3 = 0.0;
          if (fabs(x2) < 0.5) {
  	    pGrid->U[k][j][i].d = drat;
            pGrid->U[k][j][i].M1 = pGrid->U[k][j][i].M1*drat;
            pGrid->U[k][j][i].M2 = pGrid->U[k][j][i].M2*drat;
          }
	  /* pressure scaled to give sound speed = 1 at gamma = 1.4 */
#ifndef BAROTROPIC
          pGrid->U[k][j][i].E = 2.5/Gamma_1
             + 0.5*(SQR(pGrid->U[k][j][i].M1) + SQR(pGrid->U[k][j][i].M2)
             + SQR(pGrid->U[k][j][i].M3))/pGrid->U[k][j][i].d;
#endif /* BAROTROPIC */
#ifdef MHD
          pGrid->B1i[k][j][i] = b0;
          pGrid->U[k][j][i].B1c = b0;
#ifndef BAROTROPIC
          pGrid->U[k][j][i].E += 0.5*b0*b0;
#endif /* BAROTROPIC */
#endif /* MHD */
/* Use passive scalar to keep track of the fluids, since densities are same */
#if (NSCALARS > 0)
          pGrid->U[k][j][i].s[0] = 0.0;
          if (fabs(x2) < 0.5) pGrid->U[k][j][i].s[0] = 1.0;
#endif
        }
#ifdef MHD
      pGrid->B1i[k][j][ie+1] = b0;
#endif
      }
    }
  }

/* With viscosity and/or resistivity, read diffusion coeffs */

#ifdef RESISTIVITY
  eta_Ohm = par_getd_def("problem","eta_O",0.0);
  Q_Hall  = par_getd_def("problem","Q_H",0.0);
  Q_AD    = par_getd_def("problem","Q_AD",0.0);
#endif
#ifdef VISCOSITY
  nu_iso = par_getd_def("problem","nu_iso",0.0);
  nu_aniso = par_getd_def("problem","nu_aniso",0.0);
#endif

/* enroll new history variables, only once  */

  if (frst == 1) {
#ifdef MHD
    dump_history_enroll(hst_Bx, "<Bx>");
    dump_history_enroll(hst_By, "<By>");
    dump_history_enroll(hst_Bz, "<Bz>");
#endif /* MHD */
    frst = 0;
  }

}

/*==============================================================================
 * PROBLEM USER FUNCTIONS:
 * problem_write_restart() - writes problem-specific user data to restart files
 * problem_read_restart()  - reads problem-specific user data from restart files
 * get_usr_expr()          - sets pointer to expression for special output data
 * get_usr_out_fun()       - returns a user defined output function pointer
 * get_usr_par_prop()      - returns a user defined particle selection function
 * Userwork_in_loop        - problem specific work IN     main loop
 * Userwork_after_loop     - problem specific work AFTER  main loop
 *----------------------------------------------------------------------------*/

void problem_write_restart(MeshS *pM, FILE *fp)
{
  return;
}

void problem_read_restart(MeshS *pM, FILE *fp)
{
#ifdef RESISTIVITY
  eta_Ohm = par_getd_def("problem","eta_O",0.0);
  Q_Hall  = par_getd_def("problem","Q_H",0.0);
  Q_AD    = par_getd_def("problem","Q_AD",0.0);
#endif
#ifdef VISCOSITY
  nu_iso = par_getd_def("problem","nu_iso",0.0);
  nu_aniso = par_getd_def("problem","nu_aniso",0.0);
#endif
  return;
}

#if (NSCALARS > 0)
/*! \fn static Real color(const GridS *pG, const int i, const int j,const int k)
 *  \brief Returns first passively advected scalar s[0] */
static Real color(const GridS *pG, const int i, const int j, const int k)
{
  return pG->U[k][j][i].s[0]/pG->U[k][j][i].d;
}
#endif

ConsFun_t get_usr_expr(const char *expr)
{
#if (NSCALARS > 0)
  if(strcmp(expr,"color")==0) return color;
#endif
  return NULL;
}

VOutFun_t get_usr_out_fun(const char *name){
  return NULL;
}

void Userwork_in_loop(MeshS *pM)
{

  DomainS *pD = (DomainS*)&(pM->Domain[0][0]);
  GridS   *pG = pD->Grid;

  int i,is=pG->is,ie=pG->ie;
  int j,js=pG->js,je=pG->je;
  int k,ks=pG->ks,ke=pG->ke;
  Real x1,x2,x3,dim3,scaldisp,scalwidth,scalweigh,velowidth,x2lo,x2hi;
  Real** sprof;
  Real** vprof;

  if (iprob == 3) {
    scalwidth = 0.0;
    scaldisp  = 0.0;
    scalweigh = 0.0;
    velowidth = 0.0;
    dim3 = (ke>ks);
    sprof= (Real**)calloc_2d_array(ke-ks+1,je-js+1,sizeof(Real));
    vprof= (Real**)calloc_2d_array(ke-ks+1,je-js+1,sizeof(Real));
    for (k=ks;k<=ke;k++) {
      for (j=js;j<=je;j++) {
        sprof[k-ks][j-js] = 0.0;
        vprof[k-ks][j-js] = 0.0;
      }
    }
    for (k=ks; k<=ke; k++) {
      for (j=js; j<=je; j++) {
        for (i=is; i<=ie; i++) {
          cc_pos(pG,i,j,k,&x1,&x2,&x3);
          scaldisp         += (SQR(x2)+SQR(dim3*x3))*pG->U[k][j][i].s[0]/(1.0+dim3);
	  scalweigh        += pG->U[k][j][i].s[0];
          sprof[k-ks][j-js]+= pG->U[k][j][i].s[0];
          vprof[k-ks][j-js]+= pG->U[k][j][i].M1/pG->U[k][j][i].d;
	}
        sprof[k-ks][j-js] /= ((Real)(ie-is+1));
        vprof[k-ks][j-js] /= ((Real)(ie-is+1));
      }
    }
    if (dim3) {
      ath_error("[Userwork_in_loop]: on-the-fly analysis does not work for 3d.");
    } else {
      for (j=1;j<(je-js+1)/2;j++){
        sprof[0][j]       += sprof[0][j-1];
        sprof[0][je-js-j] += sprof[0][je-js-j+1];
      }
      for (j=0;j<(je-js+1)/2;j++) {
        sprof[0][j]        = 0.5*(sprof[0][j]+sprof[0][je-js-j]);
        vprof[0][j]        = 0.5*(vprof[0][j]+vprof[0][je-js-j]);
      }
      for (j=0;j<(je-js+1)/2;j++)
        sprof[0][j]       /= sprof[0][(je-js+1)/2-1]; 
      j = 0;
      while (sprof[0][j] < 0.5)
        j++;
      cc_pos(pG,i,j-1,k,&x1,&x2lo,&x3);
      cc_pos(pG,i,j  ,k,&x1,&x2hi,&x3); 
      scalwidth = x2lo*(sprof[0][j  ]-0.5)/(sprof[0][j]-sprof[0][j-1]) 
                 +x2hi*(0.5-sprof[0][j-1])/(sprof[0][j]-sprof[0][j-1]);
      j = 0;
      while (vprof[0][j] > 0.9*vflow)
        j++;
      cc_pos(pG,i,j-1,k,&x1,&x2lo,&x3);
      cc_pos(pG,i,j  ,k,&x1,&x2hi,&x3);
      velowidth = x2lo*(0.9*vflow-vprof[0][j  ])/(vprof[0][j-1]-vprof[0][j])
                 +x2hi*(vprof[0][j-1]-0.9*vflow)/(vprof[0][j-1]-vprof[0][j]);
      while (vprof[0][j] > -0.9*vflow)
        j++;
      cc_pos(pG,i,j-1,k,&x1,&x2lo,&x3);
      cc_pos(pG,i,j  ,k,&x1,&x2hi,&x3);
      velowidth = x2lo*(-0.9*vflow-vprof[0][j  ])/(vprof[0][j-1]-vprof[0][j])
                 +x2hi*(vprof[0][j-1]+0.9*vflow)/(vprof[0][j-1]-vprof[0][j])
                 - velowidth;
      
    }
    scalwidth= fabs(scalwidth);
    scaldisp = sqrt(scaldisp/scalweigh);
    fprintf(stdout,"[Userwork_in_loop]: scalar dispersion, width = %13.5e %13.5e %13.5e\n",scaldisp,scalwidth,velowidth);
    free_2d_array(sprof);
    free_2d_array(vprof);
  }
  return;
}

void Userwork_after_loop(MeshS *pM)
{
  return;
}

/*=========================== PRIVATE FUNCTIONS ==============================*/
/*------------------------------------------------------------------------------
 */

#define IM1 2147483563
#define IM2 2147483399
#define AM (1.0/IM1)
#define IMM1 (IM1-1)
#define IA1 40014
#define IA2 40692
#define IQ1 53668
#define IQ2 52774
#define IR1 12211
#define IR2 3791
#define NTAB 32
#define NDIV (1+IMM1/NTAB)
#define RNMX (1.0-DBL_EPSILON)

/*! \fn double ran2(long int *idum)
 *  \brief  Extracted from the Numerical Recipes in C (version 2) code. Modified
 *   to use doubles instead of floats. -- T. A. Gardiner -- Aug. 12, 2003
 *
 * Long period (> 2 x 10^{18}) random number generator of L'Ecuyer
 * with Bays-Durham shuffle and added safeguards.  Returns a uniform
 * random deviate between 0.0 and 1.0 (exclusive of the endpoint
 * values).  Call with idum = a negative integer to initialize;
 * thereafter, do not alter idum between successive deviates in a
 * sequence.  RNMX should appriximate the largest floating point value
 * that is less than 1. 

 */
double ran2(long int *idum){
  int j;
  long int k;
  static long int idum2=123456789;
  static long int iy=0;
  static long int iv[NTAB];
  double temp;

  if (*idum <= 0) { /* Initialize */
    if (-(*idum) < 1) *idum=1; /* Be sure to prevent idum = 0 */
    else *idum = -(*idum);
    idum2=(*idum);
    for (j=NTAB+7;j>=0;j--) { /* Load the shuffle table (after 8 warm-ups) */
      k=(*idum)/IQ1;
      *idum=IA1*(*idum-k*IQ1)-k*IR1;
      if (*idum < 0) *idum += IM1;
      if (j < NTAB) iv[j] = *idum;
    }
    iy=iv[0];
  }
  k=(*idum)/IQ1;                 /* Start here when not initializing */
  *idum=IA1*(*idum-k*IQ1)-k*IR1; /* Compute idum=(IA1*idum) % IM1 without */
  if (*idum < 0) *idum += IM1;   /* overflows by Schrage's method */
  k=idum2/IQ2;
  idum2=IA2*(idum2-k*IQ2)-k*IR2; /* Compute idum2=(IA2*idum) % IM2 likewise */
  if (idum2 < 0) idum2 += IM2;
  j=(int)(iy/NDIV);              /* Will be in the range 0...NTAB-1 */
  iy=iv[j]-idum2;                /* Here idum is shuffled, idum and idum2 */
  iv[j] = *idum;                 /* are combined to generate output */
  if (iy < 1) iy += IMM1;
  if ((temp=AM*iy) > RNMX) return RNMX; /* No endpoint values */
  else return temp;
}

#undef IM1
#undef IM2
#undef AM
#undef IMM1
#undef IA1
#undef IA2
#undef IQ1
#undef IQ2
#undef IR1
#undef IR2
#undef NTAB
#undef NDIV
#undef RNMX

/*------------------------------------------------------------------------------
 * MHD history variables
 * hst_Bx, etc.: Net flux, and Maxwell stress, added as history variables
 */

#ifdef MHD
/*! \fn static Real hst_Bx(const GridS *pG, const int i,const int j,const int k)
 *  \brief x-component of B-field */
static Real hst_Bx(const GridS *pG, const int i, const int j, const int k)
{
  return pG->U[k][j][i].B1c;
}

/*! \fn static Real hst_By(const GridS *pG, const int i,const int j,const int k)
 *  \brief y-component of B-field */
static Real hst_By(const GridS *pG, const int i, const int j, const int k)
{
  return pG->U[k][j][i].B2c;
}

/*! \fn static Real hst_Bz(const GridS *pG, const int i,const int j,const int k)
 *  \brief z-component of B-field */
static Real hst_Bz(const GridS *pG, const int i, const int j, const int k)
{
  return pG->U[k][j][i].B3c;
}
#endif
