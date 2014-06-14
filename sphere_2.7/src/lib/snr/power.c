/************************************************************************/
/*    file: power.c							*/
/*    Desc: General functions for computing the RMS power with		*/
/*	    specific window sizes and step sizes			*/
/*    Date: Nov 27, 1990						*/
/*          cleaned up Summer 1992                                      */
/*									*/
/************************************************************************/
#include <snr/snrlib.h>

#define POWER_C_VERSION "V1.0"

#define db(_n) (double)(10.0*log10((double)_n))

void pwr_compute(short int *samples, double **pwr, int sample_count, int *pwr_cnt, int win_size, int win_step)
{
    double *tpwr, pwr1(short int *win, int len) ;
    int i, pwri;
 
    *pwr_cnt = (sample_count / win_step) - 1 ;
    alloc_singarr(tpwr,*pwr_cnt,double);
    for (i=0, pwri=0; i<sample_count && pwri<*pwr_cnt; i+=win_step, pwri++)
        *(tpwr+pwri) = pwr1(samples+i,win_size);
    *pwr = tpwr;
}


double pwr1(short int *win, int len)
{
    int i;
    double sum;
    int same=1;  /* is this sample value equal to the previous ? */

    sum=0.0;
    for (i=0; i<len; i++) {
      sum += (double)(win[i]*win[i]);
      if (i>0) same = (same && (win[i]==win[i-1]));
    }

    /* watch out for log(zero) errors */
    /* also watch out for constant values */
    if ((sum <= 0.0) || same)
      return (double)NEGATIVE_INFINITY;

    if (len==0) return (double)NEGATIVE_INFINITY;

    return(db(sum/(double)len));
}

