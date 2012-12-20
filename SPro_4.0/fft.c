/******************************************************************************/
/*                                                                            */
/*                                   fft.c                                    */
/*                                                                            */
/*                               SPro Library                                 */
/*                                                                            */
/* Guig                                                             Apr. 1997 */
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
 * $Date: 2003/08/22 12:44:09 $
 * $Revision: 1.2 $
 *
 */

/*
 * FFT signal analysis.
 *
 */

#define _fft_c_

#include <spro.h>

#define OM_EPSILON 0.00001
#define PI2 6.283185307179586

#define round(x) ((int)ceil(x) - x < x - (int)floor(x)) ? (int)ceil(x) : (int)floor(x)

/* ---------------------------------------- */
/* ----- FFT and DCT global variables ----- */
/* ---------------------------------------- */
static float *w1c = NULL, *w3c = NULL;
static long *jx0 = NULL;
static unsigned long _fftn = 0;
static int _fftm = 0;
static float *_fftbuf = NULL;

static int _dctnin = 0;
static int _dctnout = 0;
static float _dctz = 0.0;
static float **_dctk = NULL;

/* --------------------------------------- */
/* ----- int fft_init(unsigned long) ----- */
/* --------------------------------------- */
/*
 * Initialize and reset FFT kernel.
 */
int fft_init(unsigned long npts)
{
  int m;
  int i, j, ip, nb, lnb, llnb, n2, n4, n6, n8, n12, n16;
  double angd, c, s;
  float ang;

  if (npts) {
    if (frexp((double)npts, &m) != (double)0.5) {
      fprintf(stderr, "[SPro error %d] FFTInit(): FFT length (%lu) is not a power of two", SPRO_FFT_INIT_ERR, npts);
      return(SPRO_FFT_INIT_ERR);
    }
    m--;
    
    if ((_fftbuf = (float *)realloc(_fftbuf, npts * sizeof(float))) == NULL) {
      fprintf(stderr, "[SPro error %d] FFTInit(): cannot allocate FFT buffer", SPRO_ALLOC_ERR);
      return(SPRO_ALLOC_ERR);
    }

    n2 = npts / 2;
    n4 = npts / 4;
    n6 = npts / 6;
    n8 = npts / 8;
    n12 = npts / 12;
    n16 = npts / 16;
    
    if ((w1c = (float *)realloc(w1c, n4 * sizeof(float))) == NULL) {
      fprintf(stderr, "[SPro error %d] FFTInit(): cannot allocate FFT kernel", SPRO_ALLOC_ERR);
      free(_fftbuf);
      return(SPRO_ALLOC_ERR);
    }
    
    if ((w3c = (float *)realloc(w3c, n4 * sizeof(float))) == NULL) {
      fprintf(stderr, "[SPro error %d] FFTInit(): cannot allocate FFT kernel", SPRO_ALLOC_ERR);
      free(_fftbuf); free(w1c);
      return(SPRO_ALLOC_ERR);
    }
    
    if ((jx0 = (long *)realloc(jx0, npts * sizeof(long) / 3)) == NULL) {
      fprintf(stderr, "[SPro error %d] FFTInit(): cannot allocate FFT kernel", SPRO_ALLOC_ERR);
      free(_fftbuf); free(w1c); free(w3c);
      return(SPRO_ALLOC_ERR);
    }
    
    ang = PI2 / (float)npts;
    angd = PI2 / (double)npts;
    c = cos(angd);
    s = sin(angd);
    w1c[1] = c;
    w1c[n4-1] = s;
    w1c[npts/8] = 0.707106781186547;
    for (i = 2; i <= n16; i++) {
      w1c[i] = w1c[i-1] * c - w1c[n4-i+1] * s;
      w1c[n4-i] = w1c[n4-i+1] * c + w1c[i-1] * s;
      w1c[n8+i-1] = w1c[n8+i-2] * c - w1c[n8-i+2] * s;
      w1c[n8-i+1] = w1c[n8-i+2] * c + w1c[n8+i-2] * s;
    }
    for (i = 1; i <= n12; i++)
      w3c[i] = w1c[3*i];
    for (i = n12 + 1; i <= n6; i++)
      w3c[i] = -w1c[n2-3*i];
    for (i = n6 + 1; i <= n4 - 1; i++)
      w3c[i] = -w1c[3*i-n2];
    jx0[0] = 0;
    jx0[1] = 0;
    jx0[2] = 0;
    jx0[3] = n2;
    jx0[4] = 3 * n4;
    ip = 5;
    nb = 3;
    lnb = 1;
    for (i = 1; i <= m - 4; i++) {
      for (j = 0; j < nb; j++)
	jx0[ip+j] = jx0[ip-nb+j] / 2;
      ip = ip + nb;
      for (j = 0; j < lnb; j++) {
	jx0[ip+j] = jx0[ip-nb-nb-lnb+j] / 4 + n2;
	jx0[ip+j+lnb] = jx0[ip+j] + n4;
      }
      ip = ip+lnb+lnb;
      llnb = lnb;
      lnb = nb;
      nb = lnb + llnb + llnb;
    }

    _fftn = npts;
    _fftm = m;

  }
  else {
    if (_fftbuf) {
      free(_fftbuf);
      free(w1c);
      free(w3c);
      free(jx0);
    }
    _fftn = _fftm = 0;
  }

  return(0);
}

/* ------------------------------------------------ */
/* ----- int fft(spsig_t *, float *, float *) ----- */
/* ------------------------------------------------ */
/*
 * Performs FFT on "signal" x -- either module or phase can be NULL 
 * if one is not interested in this result.
 */
int fft(spsig_t *s, float *m, float *ph)
{
  int i, j, n2;
  sample_t *p = s->s;
  float a, b;

  n2 = _fftn >> 1; /* _fftn / 2 */

  /* ----- sanity check ----- */
  if (_fftn == 0) {
    fprintf(stderr, "fft(): FFT kernel uninitalized");
    return(SPRO_KERNEL_INIT_ERR);
  }

  /* ----- copy signal to buffers and apply fft ----- */
  for (i = 0; i < s->n && i < _fftn; i++)
    *(_fftbuf+i) = (float)*(p+i);

  for (; i < _fftn; i++)
    *(_fftbuf+i) = (float)0.0;
  
  _brx(_fftbuf, _fftm);
  _fft(_fftbuf, _fftm);
  
  /* ----- compute modulus and phase ----- */
  if (m || ph) {
    if (m)
      *m = (spf_t)fabs(*_fftbuf);
    if (ph)
      *ph = (spf_t)0.0;

    for (i = 1, j = _fftn - 1; i < n2; i++, j--) {
      a = *(_fftbuf+i);
      b = *(_fftbuf+j);
      if (m)
	*(m+i) = (float)sqrt(a * a + b * b);
      if (ph)
	*(ph+i) = (float)atan(b / a);
    }
  }
  
  return(0);
}

/* -------------------------------------------------------- */
/* ----- int dct_init(unsigned short, unsigned short) ----- */
/* -------------------------------------------------------- */
/*
 * Initialize or reset DCT kernel
 */
int dct_init(unsigned short nin, unsigned short nout)
{
  float *kp;
  unsigned short i, j;

  if (nin && nout) {
    if ((_dctk = (float **)realloc(_dctk, nout * sizeof(float *))) == NULL) {
      fprintf(stderr, "[SPro erro r%d] DCTInit(): cannot allocate DCT kernel", SPRO_ALLOC_ERR);
      return(SPRO_ALLOC_ERR);
    }
    
    for (i = 0; i < nout; i++) {

      if ((_dctk[i] = (float *)realloc(_dctk[i], nin * sizeof(float))) == NULL) {
	fprintf(stderr, "[SPro error %d] DCTInit(): cannot allocate DCT kernel", SPRO_ALLOC_ERR);
	while(i)
	  free(_dctk[--i]);
	free(_dctk);
	return(SPRO_ALLOC_ERR);
      }
      
      kp = *(_dctk+i);
      for (j = 0; j < nin; j++)
	*(kp+j) = (float)cos(M_PI * (i + 1.0) * (j + 0.5) / nin);
    }
    
    _dctz = (float)sqrt(2.0 / nin);
    _dctnin = nin;
    _dctnout = nout;
  }
  else {
    if (_dctk) {
      for (i = 0; i < _dctnout; i++)
	if (*(_dctk+i))
	  free(*(_dctk+i));
      free(_dctk);
    }
    _dctnin = _dctnout = 0;
  }

  return(0);
}

/* ------------------------------------- */
/* ----- int dct(spf_t *, spf_t *) ----- */
/* ------------------------------------- */
/*
 * Apply DCT according to 
 *
 *  c[i]=sqrt(2/N) * sum_{j=1}^{N}(m[j] * cos(M_PI*i*(j-0.5)/N)     i=1,...,p
 */
int dct(spf_t *ip, spf_t *op)
{
  int i, j;
  double v;
  float *kp;
  
  if (! _dctnout) {
    fprintf(stderr, "fft(): DCT kernel uninitalized");
    return(SPRO_KERNEL_INIT_ERR);    
  }

  for (i = 0; i < _dctnout; i++) {
    kp = *(_dctk+i);
    v = 0.0;
    for (j = 0; j < _dctnin; j++)
      v += ( (*(ip+j)) * (*(kp+j)) );
    *(op+i) = (spf_t)(v * _dctz);
  }

  return(0);
}

/* ------------------------------------------------------------------------------ */
/* ----- unsigned short *set_alpha_idx(unsigned short, float, float, float) ----- */
/* ------------------------------------------------------------------------------ */
/*
 * Set cut-off indexes on a alpha-transformed scale. Input buffer length is
 * taken from the initialized FFT kernel.
 */
unsigned short *set_alpha_idx(unsigned short n, float a, float fmin, float fmax)
{
  unsigned short *idx, i;
  float o, d, omin, omax, z;

  if ((idx = (unsigned short *)malloc((n + 2) * sizeof(unsigned short))) == NULL) {
    fprintf(stderr, "set_alpha_idx(): cannot allocate memory\n");
    return(NULL);
  }

  if (a <= -1.0 || a >= 1) {
    fprintf(stderr, "set_alpha_idx(): invalid resolution parameter value %f (not in [0,1])\n", a);
    return(NULL);
  }

  if (fmax <= fmin)
    fmax = 0.5;

  if (fmin < 0 || fmin > 0.5 || fmax < 0 || fmax > 0.5) {
    fprintf(stderr, "set_alpha_idx(): invalid frequency range [%f,%f]\n", fmin, fmax);
    return(NULL);
  }

  *idx = (unsigned short)round(2 * fmin * ((float)(_fftn / 2 - 1)));
  *(idx+n+1) = (unsigned short)round(2 * fmax * (float)(_fftn / 2 - 1));
  
  omin = (fmin) ? theta(2.0 * M_PI * fmin, a) : 0.0; /* pulses in transform domain */
  omax = (fmax < 0.5) ? theta(2.0 * M_PI * fmax, a) : M_PI;

  d = (omax - omin) / (float)(n + 1);
  z = (float)((_fftn / 2) - 1) / M_PI;
  o = omin;

  i = 0;
  for (i = 1; i <= n; i++) {
    o += d;
    *(idx+i) = (unsigned short)round(theta_inv(o, a) * z); /* index in the original domain */
  }

  return(idx);
}

/* ---------------------------------------------------------------------------- */
/* ----- unsigned short *set_mel_idx(unsigned short, float, float, float) ----- */
/* ---------------------------------------------------------------------------- */
/*
 * Set cut-off indexes on a MEL scale. Input buffer length is taken from the
 * initialized FFT kernel.
 */
unsigned short *set_mel_idx(unsigned short n, float fmin, float fmax, float srate)
{
  unsigned short *idx, i;
  float f, min, max, d, z;

  if ((idx = (unsigned short *)malloc((n + 2) * sizeof(unsigned short))) == NULL) {
    fprintf(stderr, "set_mel_idx(): cannot allocate memory\n");
    return(NULL);
  }

  if (fmax <= fmin)
    fmax = 0.5;

  if (fmin < 0 || fmin > 0.5 || fmax < 0 || fmax > 0.5) {
    fprintf(stderr, "set_mel_idx(): invalid frequency range [%f,%f]\n", fmin, fmax);
    return(NULL);
  }

  *idx = (unsigned short)round(2 * fmin * ((float)(_fftn / 2 - 1)));
  *(idx+n+1) = (unsigned short)round(2 * fmax * (float)(_fftn / 2 - 1));

  min = mel(fmin * srate); /* bounds and df in transform domain */
  max = mel(fmax * srate); 
  d = (max - min) / (float)(n + 1);
  z = (float)(_fftn / 2 - 1) * 2.0 / srate;
  f = min;

  for (i = 1; i <= n; i++) {
    f += d;
    *(idx+i) = (unsigned short)round(mel_inv(f) * z); /* index in the original domain */
  }

  return(idx);
}

/* ------------------------------------------------------------------------------------- */
/* ----- int log_filter_bank(spsig_t *, unsigned short, unsigned short *, spf_t *) ----- */
/* ------------------------------------------------------------------------------------- */
/*
 * Apply triangular filter bank to module vector and return the log of
 * the energy in each band. Table p_index contains the indexes of the
 * cut-off frequencies. Looks like something like this:
 * 
 *                      filter 2   
 *                   <------------->
 *                filter 1           filter 3
 *             <----------->       <------------->
 *        | | | | | | | | | | | | | | | | | | | | | ..........
 *         0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9  ..........
 *             ^     ^     ^       ^             ^
 *            |     |     |       |             |
 *          idx[0]  |  idx[2]    |           idx[4]
 *                idx[1]       idx[3]  
 *
 */
int log_filter_bank(spsig_t *x, unsigned short nfilt, unsigned short *idx, spf_t *e)
{
  int i, j, from, to, status;
  double a, s, m, re, im;

  if ((status = fft(x, NULL, NULL)) != 0) {
    fprintf(stderr, "log_filter_bank(): cannot run FFT\n");
    return(status);
  }

  for (i = 0; i < nfilt; i++) {
    s = 0.0;

    /* ascending step from=idx[i] to=idx[i+1]-1: a = 1 / (idx[i+1] - idx[i] + 1) */
    from = *(idx+i);
    to = *(idx+i+1);
    a = 1.0 / (float)(to - from + 1);

    for (j = from; j < to; j++) {
      if (j) {
	re = *(_fftbuf+j);
	im = *(_fftbuf+_fftn-j);
	m = sqrt(re * re + im * im);
      }
      else
	m = fabs(*_fftbuf);

      s += m * (1.0 - a * (to - j));
    }

    /* descending step from=idx[i+1] to=idx[i+2]: a = 1 / (idx[i+2] - idx[i+1] + 1) */
    from = to;
    to = *(idx+i+2);
    a = 1.0 / (float)(to - from + 1);

    for (j = from; j <= to; j++) {
      if (j) {
	re = *(_fftbuf+j);
	im = *(_fftbuf+_fftn-j);
	m = sqrt(re * re + im * im);
      }
      else 
	m = fabs(*_fftbuf);

      s += m * (1.0 - a * (j - from));
    }

    *(e+i) = (s < SPRO_ENERGY_FLOOR) ? (spf_t)log(SPRO_ENERGY_FLOOR) : (spf_t)log(s);
  }

  return(0);
}

/* ------------------------------------- */
/* ----- float theta(float, float) ----- */
/* ------------------------------------- */
/*
 * The function determines the pulsation value on the transformed
 * axis (omega), assuming a spectral factor alpha.
 * 
 * For details on this, refer to:
 *       "Analyse spectrale a resolution variable", Ph.D. Thesis,
 *       Christian Chouzenoux, ENST
 * In this Ph.D., we are given omega'=theta(omega).
 */
float theta(float o, float a)
{
  double v, a2;

  if (a == 0.0)
    return(o);

  if (o == M_PI)
    return(M_PI);

  a2 = a * a;
  v = atan(((1.0 - a2) * sin(o)) / ((1.0 + a2) * cos(o) - 2.0 * a));

  return((float)(v < (double)0.0 ? v + M_PI : v));
}

/* ----------------------------------------- */
/* ----- float theta_inv(float, float) ----- */
/* ----------------------------------------- */
/*
 * The function determines the pulsation value on the original
 * axis (omega), assuming a spectral factor alpha.
 * 
 * For details on this, refer to:
 *       "Analyse spectrale a resolution variable", Ph.D. Thesis,
 *       Christian Chouzenoux, ENST
 *
 * In this Ph.D., we are given omega'=theta(omega). As I don't feel
 * like finding out an analytical solution to the inverse problem,
 * I simply look for a dichotomic solution to the problem by using
 * omega in the range [0,PI], as the transformation is monotonous.
 *
 * oop: original omega'
 * op:  omega'
 * opmem: omega' memory
 *
 * NOTE: If someone feels like programming an analytical solution
 * to the problem, he is welcome! Have fun ...
 *
 */
float theta_inv(float oop,float a)
{
  float o, op, oinf, osup;
  double v, a2, b;

  if (a == 0.0)
    return(oop);

  if (oop <= 0.0)
    return(0.0);

  if(oop >= M_PI)
    return(M_PI);

  oinf = 0.0;
  osup = M_PI;
  a2 = a * a;
  b = 2.0 * a;
  do {
    o = oinf + (osup - oinf) / 2.0;
    v = atan(((1.0 - a2) * sin(o)) / ((1.0 + a2) * cos(o) - b));
    op = (v < 0) ? (float)v + M_PI : (float)v;
    if(op > oop)
      osup = o;
    else
      oinf = o;
  }
  while ((float)fabs(oop - op) > OM_EPSILON);

  return(o);
}

/* ---------------------------- */
/* ----- float mel(float) ----- */
/* ---------------------------- */
float mel(float f)
{
  return(2595.0 * log10(1 + f / 700.0));
}

/* -------------------------------- */
/* ----- float mel_inv(float) ----- */
/* -------------------------------- */
/*
 * Return the value corresponding to Mel frequency f on a 
 * linear scale.
 *   Mel(f) = 2595 * log10(1 + f / 700)
 *   mel_inv(f) = 700 * (10 ^ (f / 2595) - 1)
 */
float mel_inv(float f)
{
  return((float)(700.0 * (pow(10, f / 2595.0) - 1)));
}


/* ----------------------------------- */
/* ----- void _brx(float *, int) ----- */
/* ----------------------------------- */
/*
 * Rearranges data in the FFT buffer - see _fft() for comments.
 */
void _brx(float *x, int m)
{
  int n, n1, m1, i, ipair, ibr, j, jbr, jbri, k, ia1, ia2, ia3, nh;
  int b;
  float xt;
  
  n = 1 << m;
  m1 = m / 2;
  n1 = 1 << m1;
  ia1 = n1 / 2;
  ia2 = n / n1;
  ia3 = ia1 + ia2;
  nh = n / 2;

  b = (m - m1 - m1) * n1;
  for (ipair = 0; ipair <= b; ipair += n1) {
    ibr = 0;
    xt = x[ipair+ia1];
    x[ipair+ia1] = x[ipair+ia2];
    x[ipair+ia2] = xt;
    for (i = 1 + ipair; i < ia1 + ipair; i++) {
      k = nh;
      if (k <= ibr)
	do {
	  ibr -= k;
	  k = k/2;
	}
	while (k <= ibr);
      ibr += k;
      xt = x[ibr+i+ia1];
      x[ibr+i+ia1] = x[ibr+i+ia2];
      x[ibr+i+ia2] = xt;
      jbr = 0;
      
      if (m < 4) 
	continue;
      
      for (j = ibr + ipair; j < ibr + i; j++) {
	jbri = jbr + i;
	xt = x[jbri];
	x[jbri] = x[j];
	x[j] = xt;
	xt = x[jbri+ia1];
	x[jbri+ia1] = x[j+ia2];
	x[j+ia2] = xt;
	xt = x[jbri+ia2];
	x[jbri+ia2] = x[j+ia1];
	x[j+ia1] = xt;
	xt = x[jbri+ia3];
	x[jbri+ia3] = x[j+ia3];
	x[j+ia3] = xt;
	k = nh;
	if(k <= jbr)
	  do {
	    jbr -= k;
	    k = k/2;
	  }
	  while (k <= jbr);
	jbr += k;
      }
    }
  }
}

/* ----------------------------------- */
/* ----- void _fft(float *, int) ----- */
/* ----------------------------------- */
/*
 * perform FFT after rearrangments. 

 * This piece of code was kindly contributed by Pierre Duhamel and implements
 * the algorithm described in P. Duhamel and M. Vetterli, "Improved Fourier
 * and Hartley Transform Algorithms: Application to CycliC Convolution of Real
 * Data", IEEE Trans on ASSP, 35(6), June 1987.
 *
 * NOTE: this code could be subsequently speed up by using some simple
 * C tricks (*(p+i) instead of p[i], temp variable for storing p[i] values, etc).
 */
void _fft(float *x, int m)
{
  int i, i0, i1, i2, i3, i4, i5, i6, i7, ib, istep, ia0, ia1, ia2, ia3;
  int n, ib0, ib1, ib2, ib3, j, jstep, n2, n4 ,n8, nd4, nb, lnb, llnb, k, sgn;
  float c2, c3, d2, d3, r1, r2, r3, r4, t0, t1, t2;
  const float rac2s2 = 0.707106781186547;
  
  n = 1 << m;
  nd4 = n / 4;
  sgn = ((m%2) == 0) ? 1 : -1;
  nb = (n / 2 + sgn) / 3;
  lnb = (n - sgn) / 3;
  ib = n / 6;

  for (i = ib; i < ib + nb; i++) {
    i0 = jx0[i];
    i1 = i0 + 1;
    i2 = i1 + 1;
    i3 = i2 + 1;
    r1 = x[i0] + x[i1];
    t0 = x[i2] + x[i3];
    x[i3] = x[i3] - x[i2];
    x[i1] = x[i0] - x[i1];
    x[i2] = r1 - t0;
    x[i0] = r1 + t0;
  }
  llnb = lnb;
  lnb = nb;
  nb = (llnb - lnb) / 2;
  ib = ib - nb;
  
  for (i = ib; i < ib + nb; i++) {
    i0 = jx0[i];
    i4 = i0 + 4;
    i5 = i0 + 5;
    i6 = i0 + 6;
    i7 = i0 + 7;
    r1 = x[i4] - x[i5];
    r3 = x[i4] + x[i5];
    r2 = x[i7] - x[i6];
    r4 = x[i6] + x[i7];
    t0 = r3 + r4;
    x[i6] = r4 - r3;
    x[i4] = x[i0] - t0;
    x[i0] = x[i0] + t0;

    t1 = (r1 + r2) * rac2s2;
    t2 = (r2 - r1) * rac2s2;
    i3 = i0 + 3;
    x[i5] = t2 - x[i3];
    x[i7] = t2 + x[i3];
    i1 = i0 + 1;
    x[i3] = x[i1] - t1;
    x[i1] = x[i1] + t1;
  }

  istep = n / 16;
  n8 = 1;
  n4 = 2;
  n2 = 4;
  
  for (k = 4; k <= m; k++) {
    llnb = lnb;
    lnb = nb;
    nb = (llnb - lnb) / 2;
    ib = ib - nb;
    n8 = n4;
    n4 = n2;
    n2 = n2 + n2;
    
    for (i = ib; i < ib + nb; i++) {
      i0 = jx0[i];
      i1 = i0 + n4;
      i2 = i1 + n4;
      i3 = i2 + n4;
      t0 = x[i2] + x[i3];
      x[i3] = -x[i2] + x[i3];
      x[i2] = x[i0] - t0;
      x[i0] = x[i0] + t0;

      i0 = i0 + n8;
      i1 = i0 + n4;
      i2 = i1 + n4;
      i3 = i2 + n4;
      t1 = (x[i2] - x[i3]) * rac2s2;
      t2 = (x[i2] + x[i3]) * rac2s2;
      x[i2] = -t2 - x[i1];
      x[i3] = -t2 + x[i1];
      x[i1] = x[i0] - t1;
      x[i0] = x[i0] + t1;
    }
    
    if (n4 < 4) 
      continue;
    
    for (i = ib; i < ib + nb; i++) {
      jstep = 0;
      for (j = 1; j <= n8 - 1; j++) {
	jstep = jstep + istep;
	ia0 = jx0[i] + j;

	ia2 = ia0 + n2;
	ib2 = ia2 + n4 - j - j;
	c2 = x[ia2] * w1c[jstep] + x[ib2] * w1c[nd4-jstep];
	d2 = -x[ia2] * w1c[nd4-jstep] + x[ib2] * w1c[jstep];
	ia3 = ia2 + n4;
	ib3 = ib2 + n4;
	c3 = x[ia3] * w3c[jstep] - x[ib3] * w3c[nd4-jstep];
	d3 = x[ia3] * w3c[nd4-jstep] + x[ib3] * w3c[jstep];
	ib1 = ia0 + n4;
	t1 = c2 + c3;
	c3 = c2 - c3;
	x[ib2] = -x[ib1] - c3;
	x[ia3] = x[ib1] - c3;
	t2 = d2 - d3;
	ia1 = ib1 - j - j;
	x[ib1] = x[ia1] + t2;
	x[ia1] = x[ia1] - t2;
	d3 = d2 + d3;
	ib0 = ia1 + n4;
	x[ia2] = -x[ib0] + d3;
	x[ib3] = x[ib0] + d3;
	x[ib0] = x[ia0] - t1;
	x[ia0] = x[ia0] + t1;
      }
    }
    istep = istep / 2;
  }
}

#undef _fft_c_
