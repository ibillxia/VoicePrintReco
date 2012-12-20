/******************************************************************************/
/*                                                                            */
/*                                   lpc.c                                    */
/*                                                                            */
/*                               SPro Library                                 */
/*                                                                            */
/* Guig                                                             Oct. 2002 */
/* -------------------------------------------------------------------------- */
/*  Copyright (C) 2002 Guillaume Gravier (ggravier@irisa.fr)                  */
/*                                                                            */
/*  This program is free software; you can redistribute it and/or             */
/*  modify it under the terms of the GNU General Public License               */
/*  as published by the Free Software Foundation; either version 2            */
/*  of the License, or (at your option) any later version.                    */
/*                                                                            */
/*  This program is distributed in the hope that it will be useful,           */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/*  GNU General Public License for more details.                              */
/*                                                                            */
/*  You should have received a copy of the GNU General Public License         */
/*  along with this program; if not, write to the Free Software               */
/*  Foundation, Inc., 59 Temple Place - Suite 330,                            */
/*  Boston, MA  02111-1307, USA.                                              */
/*                                                                            */
/******************************************************************************/
/*
 * CVS log:
 *
 * $Author: ggravier $
 * $Date: 2003/04/09 16:11:50 $
 * $Revision: 1.1 $
 *
 */

/*
 * Auto regressive modelling procedure for speech processing.
 *
 * The implied AR(p) model throughout this code is 
 * x[n] + a_0 x[n-1] + ... + a_{p-1} x[n-p] = e[n].
 * 
 * Analysis method is Levinson-Durbin direct algorithm which can be found in:
 *       "Traitement de la parole, R. BOITE et M. KUNT", Presses Polytechniques Romandes
 *
 * The general method, using first order dephasing filters, can be found in:
 *       "Analyse spectrale a resolution variable", Ph.D. Thesis,
 *       Christian Chouzenoux, ENST

 */

#define _lpc_c_

#include <spro.h>

/* --------------------------------------------------------------------- */
/* ----- int sig_correl(spsig_t *, float, float *, unsigned short) ----- */
/* --------------------------------------------------------------------- */
/*
 * Generalized "correlation sequence".
 *
 * First apply the corrective filter 
 *     \mu(z) = (1 - \alpha^2) / (1 - (\alpha z^{-1}))^2
 * Successively filter p times the (corrected) signal by 
 *     H(z) = (z^{-1} - \alpha) / (1 - \alpha z^{-1})
 * and estimate correlation r(i) by computing the correlation between
 * the original signal and the signal filtered i times by H(z). 
 * Cf. Ph.D. thesis of Christian Chouzenoux, ENST, 1983.
 *
 * If spectral scale factor \alpha is null, the standard correlation 
 * estimator is used.
 */
int sig_correl(spsig_t *sig, float a, float *r, unsigned short p)
{
  double a2, b;
  unsigned short i, j;
  float **buf, *xp, *yp;
  sample_t *s = sig->s;

  if (a == 0.0) {

    for (i = 0; i <= p; i++) {
      b = 0.0;
      for (j = i; j < sig->n; j++)
	b += *(s+j) * *(s+j-i);
      *(r+i) = b;
    }
  }
  else {

    /* allocate buffer */
    if ((buf = (float **)malloc((p+1) * sizeof(float *))) == NULL)
      return(SPRO_ALLOC_ERR);
    
    for (i = 0; i <= p; i++) {
      if ((*(buf+i) = (float *)malloc(sig->n * sizeof(float))) == NULL) {
	while(i)
	  free(buf[--i]);
	free(buf);
	return(SPRO_ALLOC_ERR);
      }
    }

    /* apply corrective filter */
    a2 = a * a;
    b = 1.0 - a2;
    xp = *buf;
    /* buf[0][0] = (1.0 - a * a) * s[0] */
    *xp = b * (*s);
    /* buf[0][1] = (1.0 - a * a) * s[1] + 2 * a * buf[0][0] */
    buf[0][1] = b * (*(s+1)) + 2.0 * a * (*xp); 
    for (j = 2; j <sig->n; j++)
      /* buf[0][j] = (1.0 - a * a) * s[j] + 2 * a * buf[0][j-1] - a * a * buf[0][j-2] */
      *(xp+j) = b * (*(s+j)) + 2.0 * a * (*(xp+j-1)) - a2 * (*(xp+j-2));
    
    /*
     * Succesively apply the H(z) first order filter ...
     * x ---> H(z) ---> y: y[j] = -alpha . x[j] + x[j-1] + alpha . y[j-1]
     * Here, y is buf[i] and x is the output from previous filter, i.e.
     * buf[i-1] ...
     */
    for (i = 1; i <= p; i++) {
      yp = *(buf+i);
      xp = *(buf+i-1);
      /* buf[i][0] = -a * buf[i-1][0] */
      *yp = -a * (*xp);
      for (j = 1; j < sig->n; j++)
	/* buf[i][j] = -a * buf[i-1][j] + buf[i-1][j-1] + a * buf[i][j-1]; */
	*(yp+j) = -a * (*(xp+j)) + (*(xp+j-1)) + a * (*(yp+j-1));
    }
    
    /* compute the correlation vector r */
    xp = *buf;
    for (i = 0; i <= p; i++) {
      yp = *(buf+i);
      *(r+i) = 0.0;
      for (j = 0; j < sig->n; j++)
	*(r+i) = *(r+i) + (*(xp+j)) * (*(yp+j));
    }
    
    /* clean and return */
    for (i = 0; i <= p; i++)
      free(buf[i]);
    free(buf);
  }
  
  return(0);
}

/* ------------------------------------------------------------------------ */
/* ----- void lpc(float *, unsigned short, spf_t *, spf_t *, float *) ----- */
/* ------------------------------------------------------------------------ */
/*
 * Solve the set of normal equations using the Levinson-Robinson direct 
 * algorithm, given the correlation sequence.
 */
void lpc(float *r, unsigned short p, spf_t *a, spf_t *k, float *e)
{
  unsigned short i, j, jmax;
  float v, tmp;
  int warn = 1;

  *e = *r;

  for (i = 1; i <= p; i++) {

    if (*e == 0.0)
      v = 0.0;
    else {
      v = -r[i];
      for (j = 1; j < i; j++)
        v -= a[j-1] * r[i-j];
      v /= *e;

      if ((v > 1.0 || v < -1.0) && warn) {
	fprintf(stderr, "<<<<< WARNING >>>>> unstable filter k[%d]=%.4f", i, v);
	warn = 0;
      }
    } 
    
    k[i-1] = v;

    /* update prediction coefficients */
    jmax = (i - 1) / 2;
    for (j = 1; j <= jmax; j++) {
      tmp = a[j-1];
      a[j-1] += v * a[(i-j)-1];
      a[(i-j)-1] += v * tmp;
    }
    if ((i-1) % 2)
      a[i/2-1] += v * a[i/2-1];
    a[i-1] = v;
    
    *e *= (1.0 - v * v);
  }

  *e = (float)sqrt(*e);
}

/* ----------------------------------------------------------------------------- */
/* ----- void lpc_to_cep(spf_t *, unsigned short, unsigned short, spf_t *) ----- */
/* ----------------------------------------------------------------------------- */
/*
 * Return p cepstral coefficients into vector c, computed
 * from prediction vector a.
 *
 * The formula is (assuming {a[1],...,a[N]}):
 *    c[i]=-a[i]+(1/i)*sum for j=1 to i-1 of((i-j)*a[j]*c[i-j]) i=1,..,p
 *
 * This is what is implemented with arrays indexed from 0 to N-1.
 *
 */
void lpc_to_cep(spf_t *a, unsigned short n, unsigned short p, spf_t *c)
{
  unsigned short i, j;
  double v;

  for (i = 0; i < p; i++) {
    v = 0.0;
    for (j = 0; j < i; j++)
      v -= ((float)(i - j) * *(a+j) * *(c+i-j-1));
    v *= (1.0 / (float)(i+1));
    *(c+i) = (spf_t)v - *(a+i);
  }
}

/* -------------------------------------------------------------- */
/* ----- void refc_to_lar(spf_t *, unsigned short, spf_t *) ----- */
/* -------------------------------------------------------------- */
/*
 * Compute log area ratios from reflexion coefficients.
 */
void refc_to_lar(spf_t *k, unsigned short p, spf_t *g)
{
  unsigned short i;
  double v;

  for (i = 0; i < p; i++) {
    v = *(k+i);
    *(g+i) = (spf_t)(10.0 * log10((1.0 + v) / (1.0 - v)));
  }
}

/* ------------------------------------------------------------ */
/* ----- int lpc_to_lsf(spf_t *, unsigned short, spf_t *) ----- */
/* ------------------------------------------------------------ */
/*
 * The implementation follows the description made in
 * F. K. Soong and B.-H. Juang, "LSP and Speech Data 
 * Compression", ICASSP, pp. 1.10.1--1.10.3, 1984.
 *
 * Part of the code was inspired from SPTK 1.0
 * (see http://kt-lab.ics.nitech.ac.jp/~tokuda/SPTK)
 */
int lpc_to_lsf(spf_t *a, unsigned short p, spf_t *lsf)
{
  int i, j, mp, mh, nf, mb;
  double fr, pxr, tpxr, tfr, pxm, pxl, fl, qxl, tqxr;
  double fm, qxm, qxr, tqxl;
  double *P, *Q;
       
  mp = p + 1;
  mh = p / 2;

  if ((P = (double *)malloc(mh * sizeof(double))) == NULL)
    return(SPRO_ALLOC_ERR);
  if ((Q = (double *)malloc(mh * sizeof(double))) == NULL) {
    free(P);
    return(SPRO_ALLOC_ERR);
  }
  
  /* generate p and q polynomials */
  for (i = 0; i < mh; i++) {
    P[i] = a[i] + a[p-i-1];
    Q[i] = a[i] - a[p-i-1];
  }

  /* compute p at f=0.0 */
  fl = 0.0; pxl = 1.0;
  for (j = 0; j < mh; j++)
    pxl += P[j];

  /* search for zeros of P(z) */
  nf = 0;
  for (i = 1; i <= 128; i++) {
    mb = 0;
    fr = i * (0.5 / 128);

    /* cosine transform @ fr */
    pxr = cos(mp * M_PI * fr);
    for (j = 0; j < mh; j++)
      pxr += cos(mp - (j + 1) * 2 * M_PI * fr) * P[j];
    tpxr = pxr;
    tfr = fr;
    
    /* check same sign - if yes, continue! */
    if (pxl * pxr > 0.0)
      continue;

    /* 
     * There is a zero in the range [fl,fr].
     * Run dichotomic search...
     */
    do {
      mb++;
      fm = fl + (fr - fl) / (pxl - pxr) * pxl;

      /* cosine transform @ fm */
      pxm = cos(mp * M_PI * fm);
      for (j = 0; j < mh; j++)
        pxm += cos(mp - (j + 1) * 2 * M_PI * fm) * P[j];

      if (pxm * pxl > 0.0) {
	pxl = pxm; fl = fm;
      }
      else {
	pxr = pxm; fr = fm;
      }
    } while(fabs(pxm) > 10e-6 && mb < 4);
    
    lsf[nf] = fl + (fr - fl) / (pxl - pxr) * pxl;
    nf += 2;

    if (nf > p-2)
      break;

    pxl = tpxr;
    fl = tfr;    
  }

  /* search for the zeros of Q(z) */
  fl = lsf[0];
  qxl = sin(mp * M_PI * fl);
  for (j = 0; j < mh; j++)
    qxl += sin(mp - (j + 1) * 2 * M_PI * fl) * Q[j];

  for (i = 2; i < mp; i += 2) {
    mb = 0;
    fr = (i == p) ? 0.5 : lsf[i];

    qxr = sin(mp * M_PI * fr);
    for (j = 0; j < mh; j++)
      qxr += sin(mp - (j + 1) * 2 * M_PI * fr) * Q[j];

    tqxl = qxl;
    tfr = fr;
    tqxr = qxr;
    do {
      mb++;
      fm = (fl + fr) * 0.5;
      qxm = sin(mp * M_PI * fm);
      for (j = 0; j < mh; j++)
        qxm += sin(mp - (j + 1) * 2 * M_PI * fm) * Q[j];
      if (qxm*qxl > 0.0) {
	qxl = qxm;
	fl = fm;
      } 
      else {
	qxr = qxm;
	fr = fm;
      }
    } while (fabs(qxm) > 10e-6 && mb < 15);
    
    lsf[i-1] = fl + (fr - fl) / (qxl - qxr) * qxl;
    qxl = tqxr;
    fl = tfr;
  }
  
  free(P);
  free(Q);

  return(0);
}


#undef _lpc_c_
