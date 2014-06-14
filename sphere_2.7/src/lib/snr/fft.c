/*
 * fft.c portions rewritten by bg, 1/18/91.
 * New routines are:
 *	fft_skip	: complex FFT with skip
 *	ifft_skip	: complex IFFT with skip
 *	fft			: complex FFT
 *	ifft		: complex IFFT
 *	fft_2d		: 2d complex FFT
 *	ifft_2d		: 2d complex IFFT
 *	fft_real	: FFT, real data (stored complex with im = 0)
 *  ifft_real	: IFFT, real data (stored complex with im = 0)
 *	fft_real_pack	: FFT, real data
 *	fft_real_pack	: IFFT, real data
 */

/*
 *	fft.c
 *	Fast Fourier Transform C library
 *	Based on RECrandell's.
 *	dpwe 22jan90
 *	08apr90 With experimental FFT2torl - converse of FFT2real
 */

/*
	Routines include:
	FFT2raw : Radix-2 FFT, in-place, with yet-scrambled result.
	FFT2	: Radix-2 FFT, in-place and in-order.
	FFTreal : Radix-2 FFT, with real data assumed,
		   in-place and in-order (this routine is the fastest of the lot).
	FFT2dimensional : Image FFT, for real data, easily modified for other
		purposes.

	To call an FFT, one must first assign the complex exponential factors:
	A call such as
	e = AssignBasis(NULL, size)
	will set up the complex *e to be the array of cos, sin pairs corresponding
	to total number of complex data = size.	 This call allocates the
	(cos, sin) array memory for you.  If you already have such memory
	allocated, pass the allocated pointer instead of NULL.
 */

#include <snr/snrlib.h>

static LNODE *lroot = NULL;	/* root of look up table list */
/* fft.c */	static char *alloc_err PROTO((long int n));

/*
 * Search our list of existing LUT's.
 */
complex *FindTable(long int n)
{
LNODE *plnode;

	plnode = lroot;
	while (plnode != NULL) {
		if (plnode->size == n) return plnode->table;
		plnode = plnode->next;
	}
	return NULL;
}

static char *alloc_err(long int n)
{
char *ptr;
	if ((ptr = (char *) malloc(n)) == NULL) {
		fprintf(stderr,"fft: can't allocate %ld bytes\n",n);
		exit(0);
	}
return ptr;
}

complex *AssignBasis(complex *ex, long int n)
{
register long i;
register double a;
LNODE *plnode;
complex	*expn;

	if ((expn = FindTable(n)))
		return expn;
	if (ex) expn = ex;
	else expn = (complex *) alloc_err(n * sizeof(complex));
	for (i = 0; i < n; i++) {
		a = 2 * PI * i / n;
		expn[i].re = cos(a);
		expn[i].im = -sin(a);
	}
	plnode = lroot;
	lroot = (LNODE *) alloc_err((long)sizeof(LNODE));
	lroot->next = plnode;
	lroot->size = n;
	lroot->table = expn;
	return expn;
}

void reverseDig(complex *x, long int n, long int skip)
{
register long i, j, k, jj, ii;
complex tmp;
	for (i = 0, j = 0; i < n - 1; i++) {
		if (i < j) {
			jj = j * skip;
			ii = i * skip;
			tmp = x[jj];
			x[jj] = x[ii];
			x[ii] = tmp;
		}
		k = n/2;
		while (k <= j) {
			j -= k;
			k >>= 1;
		}
		j += k;
	}
}

/*
 * Perform 2D FFT on image of width w, height h (both powers of 2).
 * IMAGE IS ASSUMED PURE-REAL.	If not, the FFT2real call should be just FFT2.
 */
void FFT2dimensional(complex *x, long int w, long int h, complex *ex)
{
long i;
	for (i = 0; i < h; i++) FFT2real(x + i * w, w, 1, ex);
	for (i = 0; i < w; i++) FFT2(x + i, h, w, ex);
}

/*
 * Performs FFT on data assumed complex conjugate to give purely
 * real result.
 */
void FFT2torl(complex *x, long int n, long int skip, double scale, complex *ex)
{
long half = n >> 1;
register long m, mm;

	if (!(half >> 1)) return;
	Reals(x,n,skip,-1,ex);
	ConjScale(x, half + 1, 2.0 * scale);
	FFT2raw(x, half, 2, skip, ex);
	reverseDig(x, half, skip);

	for (mm = half - 1; mm >= 0; mm--) {
		m = mm * skip;
		x[m << 1].re = x[m].re;
		x[(m << 1) + skip].re = -x[m].im;		/* need to conjugate result for true ifft */
		x[m << 1].im = x[(m << 1) + skip].im = 0.0;
	}
}

/*
 * Like FFT2torl, but leaves result in x[0].re, x[0].im, x[1].re,...
 * so that if x is considered a double array the result is there.
 */
void FFT2torl_pack(complex *x, long int n, long int skip, double scale, complex *ex)
{
long half = n >> 1;
long i;

	if (!(half >> 1)) return;
	Reals(x,n,skip,-1,ex);
	ConjScale(x, half + 1, 2.0 * scale);
	FFT2raw(x, half, 2, skip, ex);
	reverseDig(x, half, skip);
	/* just negate the imaginary part */
	for (i = 0; i < half; i++)
		x[i].im = -x[i].im;
}

/*
 * Conjugate and scale complex data (e.g. prior to IFFT by FFT)
 */
void ConjScale(complex *x, long int n, double scale)
{
double miscale = -scale;

	while (n-- > 0) {
		x->re *= scale;
		x->im *= miscale;
		x++;
	}
}

/*
 * Perform real FFT, data arrange as {re, 0, re, 0, re, 0...};
 * leaving full complex result in x.
 */
void FFT2real(complex *x, long int n, long int skip, complex *ex)
           
             
            	/* lookup table */
{
long half = n >> 1;
register long m, mm;

	if (!(half >> 1)) return;
	for (mm = 0; mm < half; mm++) {
		m = mm * skip;
		x[m].re = x[m << 1].re;
		x[m].im = x[(m << 1) + skip].re;
	}
	FFT2raw(x, half, 2L, skip, ex);
	reverseDig(x, half, skip);
	Reals(x, n, skip, 1L, ex);
}

/*
 * sign is 1 for FFT2real, -1 for torl.
 */
void Reals(complex *x, long int n, long int skip, int sign, complex *ex)
           
             
         
            	/* lookup table passed in */
{
long half = n >> 1;
long quarter = half >> 1;
register long m, mm;
register double tmp;
complex a, b, s, t;

	half *= skip;
	n *= skip;
	if (sign == 1) x[half]= x[0];	/* only for Yc to Xr */
	for (mm = 0; mm <= quarter; mm++) {
		m = mm * skip;
		s.re = (1.0 + ex[mm].im) / 2;
		s.im = -sign * ex[mm].re / 2;
		t.re = 1.0 - s.re;
		t.im = -s.im;
		a = x[m];
		b = x[half - m];
		b.im = -b.im;

		tmp = a.re;
		a.re = (a.re*s.re - a.im*s.im);
		a.im = (tmp*s.im + a.im*s.re);

		tmp = b.re;
		b.re = (b.re*t.re - b.im*t.im);
		b.im = (tmp*t.im + b.im*t.re);
	
		b.re += a.re;
		b.im += a.im;
	
		a = x[m];
		a.im = -a.im;
		x[m] = b;
		if (m) {
			 b.im = -b.im;
			 x[n - m] = b;
		}
		b = x[half - m];
	
		tmp = a.re;
		a.re = (a.re * t.re + a.im * t.im);
		a.im = (-tmp * t.im + a.im * t.re);
	
		tmp = b.re;
		b.re = (b.re * s.re + b.im * s.im);
		b.im = (-tmp * s.im + b.im * s.re);
	
		b.re += a.re;
		b.im += a.im;
	
		x[half - m] = b;
		if (m) {
			 b.im = -b.im;
			 x[half + m] = b;
		}
	}
}

/*
 * Perform FFT for n = a power of 2.
 * The relevant data are the complex numbers x[0], x[skip], x[2*skip], ...
 */
void FFT2(complex *x, long int n, long int skip, complex *ex)
{
	FFT2raw(x, n, 1, skip, ex);
	reverseDig(x, n, skip);
}

/*
 * Data is x,
 * data size is n,
 * dilate means: library global expn is the (cos, -j sin) array, EXCEPT for
 * effective data size n/dilate,
 * skip is the offset of each successive data term, as in "FFT2" above.
 */
void FFT2raw(complex *x, long int n, long int dilate, long int skip, complex *ex)
           
                     
            	/* lookup table */
{
register long j, m = 1, p, q, i, k, n2 = n, n1;
register double c, s, rtmp, itmp;

	while (m < n) {
		n1 = n2;
		n2 >>= 1;
		for (j = 0, q = 0; j < n2; j++) {
			c = ex[q].re;
			s = ex[q].im;
			q += m*dilate;
			for (k = j; k < n; k += n1) {
				p = (k + n2) * skip;
				i = k * skip;
				rtmp = x[i].re - x[p].re;
				x[i].re += x[p].re;
				itmp = x[i].im - x[p].im;
				x[i].im += x[p].im;
				x[p].re = (c * rtmp - s * itmp);
				x[p].im = (c * itmp + s * rtmp);
			 }
		}
		m <<= 1;
	}
}

/*
 * Perform direct Discrete Fourier Transform.
 */
void DFT(complex *data, complex *result, long int n, complex *ex)
{
long j, k, m;
double s, c;

	for (j = 0; j < n; j++) {
		result[j].re = 0;
		result[j].im = 0;
		for (k = 0; k < n; k++) {
			 m = (j * k) % n;
			 c = ex[m].re;
			 s = ex[m].im;
			 result[j].re += (data[k].re * c - data[k].im * s);
			 result[j].im += (data[k].re * s + data[k].im * c);
		}
	}
}

/*
 * Simplified routines.
 */

/*
 * Real, radix-2, FFT. Data occupies real part of x,
 * imaginary part ignored. Conjugate symmetric result
 * returned in x.
 */
void fft_real(complex *x, long int n)
{
complex *ex;
	ex = AssignBasis(NULL,n);
	FFT2real(x, n, 1L, ex);
}

/*
 * Real, radix-2, IFFT. Conjugate symmetric spectrum in x.
 * Real result returned in real part of x, imaginary part
 * set to 0.
 */
void ifft_real(complex *x, long int n)
{
complex *ex;
	ex = AssignBasis(NULL,n);
	FFT2torl(x, n, 1L, 1.0 / n, ex);
}

/*
 * Internal version of FFT with skip.
 */
void fft_skip(complex *x, long int n, long int skip)
{
complex *ex;
	ex = AssignBasis(NULL,n);
	FFT2(x, n, skip, ex);
}

/*
 * Complex, radix-2, FFT.
 */
void fft(complex *x, long int n)
{
	fft_skip(x,n,1L);
}

/*
 * Internal version of IFFT with skip.
 */
void ifft_skip(complex *x, long int n, long int skip)
{
complex *ex;
complex tmp;
double c, s;
long i;
long j, k;	/* for optimized skip indexing */
	ex = AssignBasis(NULL,n);
	/* reverse direction of x */
	for (i = 0; i < n/2; i++) {
		j = (n - 1 - i) * skip;
		k = i * skip;
		tmp = x[j];
		x[j] = x[k];
		x[k] = tmp;
	}
	FFT2(x, n, skip, ex);
	/* now do final twiddle */
	for (i = 0; i < n; i++) {
		k = i * skip;
		c = ex[i].re;
		s = ex[i].im;
		tmp.re = (x[k].re * c - x[k].im * s) / n;
		tmp.im = (x[k].re * s + x[k].im * c) / n;
		x[k] = tmp;
	}
}

/*
 * Complex, radix-2, IFFT.
 */
void ifft(complex *x, long int n)
{
	ifft_skip(x, n, 1L);
}

/*
 * Complex, radix-2, two dimensional FFT.
 */
void fft_2d(complex *x, long int n1, long int n2)
           
            	/* n1 is x or #cols, n2 is y or #rows */
{
long i;
	/* cols */
	for (i = 0; i < n1; i++)
		fft_skip(x + i, n2, n1);
	/* rows */
	for (i = 0; i < n2; i++)
		fft(x + i * n1, n1);
}

/*
 * Complex, radix-2, two dimensional IFFT.
 */
void ifft_2d(complex *x, long int n1, long int n2)
           
            	/* n1 is x or #cols, n2 is y or #rows */
{
long i;
	/* cols */
	for (i = 0; i < n1; i++)
		ifft_skip(x + i, n2, n1);
	/* rows */
	for (i = 0; i < n2; i++)
		ifft(x + i * n1, n1);
}

/*
 * Performs real, radix-2, FFT on n pt real data packed into
 * x[0].re, x[0].im, x[1].re, x[1].im, etc.
 * Leaves n pt complex conjugate symmetric result in x.
 */
void fft_real_pack(complex *x, long int n)
{
complex *ex;
long half = n >> 1;
	if (!(half >> 1)) return;
	ex = AssignBasis(NULL,n);
	FFT2raw((complex *) x, half, 2L, 1L, ex);
	reverseDig((complex *) x, half, 1L);
	Reals((complex *) x, n, 1L, 1L, ex);
}

/*
 * Performs IFFT on n pt complex x (conjugate symmetric).
 * Real result is left in first n/2 complex pts in x, or first
 * n pts of (double *) x.
 */
void ifft_real_pack(complex *x, long int n)
{
complex *ex;
	ex = AssignBasis(NULL,n);
	FFT2torl_pack(x, n, 1L, 1.0 / n, ex);
}



