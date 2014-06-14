
/*
 * Call by value complex number routines.
 *
 * Bill Gardner, May 1991
 */
#include	<snr/snrlib.h>

complex cAdd(complex a, complex b)
{
complex c;
	c.re = a.re + b.re;
	c.im = a.im + b.im;
	return c;
}

complex cSub(complex a, complex b)
{
complex c;
	c.re = a.re - b.re;
	c.im = a.im - b.im;
	return c;
}

complex cMul(complex a, complex b)
{
complex c;
	c.re = a.re * b.re - a.im * b.im;
	c.im = a.im * b.re + a.re * b.im;
	return c;
}

complex cDiv(complex a, complex b)
{
complex c;
double d;
	d = b.re * b.re + b.im * b.im;
	if (d == 0) {
		printf("complex division by zero\n");
		exit(0);
	}
	c.re = (a.re * b.re + a.im * b.im) / d;
	c.im = (a.im * b.re - a.re * b.im) / d;
	return c;
}

complex cConj(complex a)
{
	a.im = -a.im;
	return a;
}

complex cConjSquare(complex a)
{
	a.re = cMagSquare(a);
	a.im = 0;
	return a;
}

complex cSquare(complex a)
{
complex b;
	b.re = a.re * a.re - a.im * a.im;
	b.im = 2 * a.re * a.im;
	return b;
}

complex cInvert(complex a)
{
double mag_square;
	if ((mag_square = cMagSquare(a)) > 0) {
		a.re /= mag_square;
		a.im /= -mag_square;
	}
	else {
		/* stick in a big number */
		a.re = 1e10;
		a.im = 0;
	}
	return a;
}

void cPrint(complex a)
{
	printf("%f + %f * j\n", (double) a.re, (double) a.im);
}

complex cPolar(double r, double w)
{
complex a;
	a.re = r * cos(w);
	a.im = r * sin(w);
	return a;
}

complex cUnit(double w)
{
	return cPolar(1.0,w);
}

complex cNum(double re, double im)
{
complex c;
	c.re = re;
	c.im = im;
	return c;
}

/*
 * Creates complex polynomial:
 *
 * coeff[0] + coeff[1] * Z^-1 + coeff[2] * Z^-2 ...+ coeff[n-1] * Z^-(n-1)
 *
 * where Z is the complex exponential e^jw
 */
complex cPolyZ(double w, int n, double *coeff)
{
int i;
complex c;
	c = cNum(0.0,0.0);
	for (i = 0; i < n; i++)
		c = cAdd(c,cMul(cNum((double) coeff[i],0.0),cUnit(-w * i)));
	return c;
}

double cAngle(complex a)
{
	return atan2((double) a.im, (double) a.re);
}

double cMagSquare(complex a)
{
	return a.re * a.re + a.im * a.im;
}

double cMag(complex a)
{
	return sqrt(a.re * a.re + a.im * a.im);
}
