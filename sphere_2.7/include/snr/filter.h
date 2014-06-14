/* filter.h */

/* Contains definition of filterbank structure */

#ifndef _FILTER_H_
#define _FILTER_H_

#define DONT_FILTER (Filterbank *) NULL
#define NULL_FILTER (double *) NULL

typedef struct fbank
{
  int bands;      /* bands */
  int *taps;      /* array of number of taps per filter */
  double *weights; /* array of passband weights */
  double **coeff; /* array of filter coefficients for each band */
} Filterbank;

/* filter.c */	void fir_filter_fft  PROTO((short int *input, int size, double *coeff, int taps));
/* filter.c */	void fft_filter_init PROTO((double *coeff, int size,int taps));
/* filter.c */	short fir_filter  PROTO((short int sample, double *coeff, int taps, double *memory));


#endif
