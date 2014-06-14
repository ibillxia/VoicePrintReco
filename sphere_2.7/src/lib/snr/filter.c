/* filter.c */

#include	<snr/snrlib.h>

static complex *X;     /* real and imaginary parts of DFT(filter) */
static complex *x;     /* real and imaginary parts of DFT(input) */
static double *overlap;  /* for overlap add; the part that we carry over */
static int fft_size,log_fft_size;


void fft_filter_init(double *coeff, int size,int taps)
{
  int i;

  log_fft_size=ceil(log10((double)size)/log10(2.0))+1;
  fft_size = (int) pow(2.0,(double)log_fft_size);

  X = (complex *) malloc (fft_size*sizeof(complex));

  x = (complex *) malloc (fft_size*sizeof(complex));

  overlap = (double *) malloc ((taps-1)*sizeof(double));

  for (i=0; i<taps-1; i++) overlap[i]=0.0;

  for (i=0; i<fft_size; i++) {
    X[i].re = (double) (i < taps) ? coeff[i] : 0.0;
    X[i].im = 0.0;
  }

  fft_real(X,fft_size);
  /* now X holds transform of filter */
}


short fir_filter (short int sample, double *coeff, int taps, double *memory)

/* slow but simple way to do this one sample at a time */
{
  int i;
  double y=0.0;

  for (i=0; i<taps; i++) memory[taps-i] = memory[taps-i-1];
  memory[0]=(double) sample;

  for (i=0; i<taps; i++) 
    y += coeff[i]*memory[i];

  if (y < -32767) y=-32767;   /* be sure to clip */
  if (y > 32767) y=32767;

  return (short) y;
}

void fir_filter_fft (short int *input, int size, double *coeff, int taps)
{
  int i;
  double re1,re2,im1,im2;

  for (i=0; i<fft_size; i++) {
    x[i].re = (i < size) ? (double) input[i] : 0.0;
    x[i].im = 0.0;
  }

  fft_real(x,fft_size);

  for (i=0; i<fft_size; i++) {   /* complex multiply */
    re1=x[i].re; im1=x[i].im;
    re2=X[i].re; im2=X[i].im;
    
    x[i].re = re1*re2 - im1*im2;
    x[i].im = re1*im2 + im1*re2;
  }

  ifft_real(x,fft_size);  /* now x holds the convolution result */

  for (i=0; i<size; i++)
    input[i] = (short) (i < taps-1) ? (x[i].re+overlap[i]) : x[i].re;

  for (i=size; i<size+taps-1; i++) overlap[i-size]=x[i].re;
}
  






  



  
  

