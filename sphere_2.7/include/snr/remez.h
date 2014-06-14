
#ifndef _REMEZ_H_
#define _REMEZ_H_

/* remez.c */	double dw PROTO((double x));
/* remez.c */	double ds PROTO((double x));
/* remez.c */	double d PROTO((int k, int n, int m));
/* remez.c */	double gee PROTO((int k, int n));
/* remez.c */	double eff PROTO((double temp, double *fx, double *wtx, int lband, int jtype));
/* remez.c */	double wate PROTO((double temp, double *fx, double *wtx, int lband, int jtype));
/* remez.c */	int grad PROTO((double dd, double ds, double df));
/* remez.c */	void fir1 PROTO((double *edge, int nfilt, int nbands, int *error, double *fx, double *wtx, int jtype, double *h));
/* remez.c */	void design_bandpass_filter PROTO((double *h, double start, double end, int taps, double weight));



#endif

