
/*
 * FFT include file.
 */
#ifndef _FFT_H_
#define _FFT_H_

#ifndef PI
#define PI 3.1415926535
#endif

#ifndef _COMPLEX_H_
#include	"complex.h"
#endif

typedef struct lnode {
	struct lnode *next;
	long size;
	complex *table;
} LNODE;

/* fft.c */	complex *FindTable PROTO((long int n));
/* fft.c */	complex *AssignBasis PROTO((complex *ex, long int n));
/* fft.c */	void reverseDig PROTO((complex *x, long int n, long int skip));
/* fft.c */	void FFT2dimensional PROTO((complex *x, long int w, long int h, complex *ex));
/* fft.c */	void FFT2torl PROTO((complex *x, long int n, long int skip, double scale, complex *ex));
/* fft.c */	void FFT2torl_pack PROTO((complex *x, long int n, long int skip, double scale, complex *ex));
/* fft.c */	void ConjScale PROTO((complex *x, long int n, double scale));
/* fft.c */	void FFT2real PROTO((complex *x, long int n, long int skip, complex *ex));
/* fft.c */	void Reals PROTO((complex *x, long int n, long int skip, int sign, complex *ex));
/* fft.c */	void FFT2 PROTO((complex *x, long int n, long int skip, complex *ex));
/* fft.c */	void FFT2raw PROTO((complex *x, long int n, long int dilate, long int skip, complex *ex));
/* fft.c */	void DFT PROTO((complex *data, complex *result, long int n, complex *ex));
/* fft.c */	void fft_real PROTO((complex *x, long int n));
/* fft.c */	void ifft_real PROTO((complex *x, long int n));
/* fft.c */	void fft_skip PROTO((complex *x, long int n, long int skip));
/* fft.c */	void fft PROTO((complex *x, long int n));
/* fft.c */	void ifft_skip PROTO((complex *x, long int n, long int skip));
/* fft.c */	void ifft PROTO((complex *x, long int n));
/* fft.c */	void fft_2d PROTO((complex *x, long int n1, long int n2));
/* fft.c */	void ifft_2d PROTO((complex *x, long int n1, long int n2));
/* fft.c */	void fft_real_pack PROTO((complex *x, long int n));
/* fft.c */	void ifft_real_pack PROTO((complex *x, long int n));


#endif
