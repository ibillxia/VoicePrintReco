

#ifndef _COMPLEX_H_
#define _COMPLEX_H_

typedef struct dude {
	double	re;
	double	im;
} complex;

/* complex.c */	complex cAdd PROTO((complex a, complex b));
/* complex.c */	complex cSub PROTO((complex a, complex b));
/* complex.c */	complex cMul PROTO((complex a, complex b));
/* complex.c */	complex cDiv PROTO((complex a, complex b));
/* complex.c */	complex cConj PROTO((complex a));
/* complex.c */	complex cConjSquare PROTO((complex a));
/* complex.c */	complex cSquare PROTO((complex a));
/* complex.c */	complex cInvert PROTO((complex a));
/* complex.c */	void cPrint PROTO((complex a));
/* complex.c */	complex cPolar PROTO((double r, double w));
/* complex.c */	complex cUnit PROTO((double w));
/* complex.c */	complex cNum PROTO((double re, double im));
/* complex.c */	complex cPolyZ PROTO((double w, int n, double *coeff));
/* complex.c */	double cAngle PROTO((complex a));
/* complex.c */	double cMagSquare PROTO((complex a));
/* complex.c */	double cMag PROTO((complex a));


#endif
