/************************************************************************/
/*    file: hist.c							*/
/*    Desc: General purpose histogram manipulating routines.		*/
/*    Date: Nov 27, 1990						*/
/*									*/
/************************************************************************/
/*
DOC:filename: hist.c
DOC:include:  ../include/hist.h
DOC:package:  numeric histogram package
DOC:purpose:  create, manipulate and de-create histograms
DOC:
*/
#include <util/utillib.h>

#define HIST_C_VERSION "V1.0"

#ifndef M_PI
#define M_PI    3.14159265358979323846
#endif
/***********************************************************/
/* DOC-P:init_hist()*/
/* DOC-PS:allocate and init a HIST structure*/
/* Based on the number of bins, and the range of values to */
/* be within the histograms values, build and return a     */
/* histogram.                                              */
void init_hist(HIST ***hist, int num_bins, double from, double to)
{
    HIST **th;
    int i;
    double dist;

    /* what's the span of possible values */
    dist = (double) (to - from);

    alloc_2dimarr(th,num_bins,1,HIST);
    /* initialize the count, and set up the ranges */
    for (i=0; i<num_bins; i++){
        th[i]->count=0;
        th[i]->from=from+(dist*((double)i/(double)num_bins));
        th[i]->to=from+(dist*((double)(i+1)/(double)num_bins));
    }
    *hist=th;
}

/***********************************************************/
/* DOC-P: free_hist()       */
/* DOC-PS:Free the memory associated with a histogram      */
void free_hist(HIST ***hist, int num_bins)
{
    HIST **th;

    th = *hist;
    free_2dimarr(th,num_bins,HIST);
    *hist = (HIST **)0;
}

/************************************************************/
/* DOC-P:   diff_hist()       */
/* DOC-PS:   subtract the counts of hist 2 from hist 1. */
/* subtract the counts of hist 2 from hist 1.  This may     */
/* create negative count values                             */
void diff_hist(HIST **h1, HIST **h2, HIST **hd, int num_bins)
{
    int i;

    for (i=0; i<num_bins; i++)
        hd[i]->count = h1[i]->count - h2[i]->count;
}

/************************************************************/
/* DOC-P:  subtract_hist()       */
/* DOC-PS: subtract h1 from h2, all negative values are set to zero */
void subtract_hist(HIST **h1, HIST **h2, HIST **hs, int num_bins)
{
    int i;

    for (i=0; i<num_bins; i++){
        hs[i]->count = h2[i]->count - h1[i]->count;
        if (hs[i]->count < 0)
            hs[i]->count = 0;
    }
}

/************************************************************/
/* DOC-P:   double percentile_hist()       */
/* DOC-PS: return the mid-point of the bin with the Nth percentile */
/* return the mid-point of the histogram bin containing the */
/* percentile specified.                                    */
double percentile_hist(HIST **hist, int num_bins, double percentile)
{
    int i, pct_area, area=0;

    pct_area = (int)((double)hist_area(hist,num_bins) * percentile);

    for (i=0; (i<num_bins) && (area+hist[i]->count < pct_area); i++){
        area+=hist[i]->count;
    }
    return(hist[i]->from+((hist[i]->to-hist[i]->from)/2.0));
}


/************************************************************/
/* DOC-P:   double db_RMS_hist()                             */
/* DOC-PS:  return the estimated RMS value of a histogram   */
/*          whose bins have been spaced in Decibels.        */
double db_RMS_hist(HIST **hist, int num_bins)
{
    int i, area=0;
    double sum=0.0;

    for (i=0; i<num_bins; i++){
        area+=hist[i]->count;
        sum+=hist[i]->count * pow(10.0,(((hist[i]->to+hist[i]->from)/2.0) / 10.0));
    }
    return(10.0 * log(sum/area) * M_LOG10E);
}

/************************************************************/
/* DOC-P:   percentage_left_hist()       */
/* DOC-PS:  return the % of HIST to the left of the bin */
/* return the percentage of the histogram to the left of    */
/* specified bin value, including that bin                  */
double percentage_left_hist(HIST **hist, int num_bins, double value)
{
    int i, area=0, left_area=0;

    area = (double)hist_area(hist,num_bins);
   
    for (i=0; (i<num_bins) &&((hist[i]->to+hist[i]->from)/2.0) <= value;i++){
        left_area+=hist[i]->count;
    }

    return((double)left_area/(double)area*100.0);
}

/***************************************************************/
/* DOC-P:   double do_least_squares()                           */
/* DOC-PS:  returns the square-distance between two histograms */
/*          Used as a distance metric in the direct search.    */
/***************************************************************/

double do_least_squares(HIST **noise, HIST **normal, int num_bins)
{
    double sqr_sum=0.0, sqr, extend_db=5.0;
    int i,end=0;

    i=0;
    while ((i<num_bins) && (normal[i]->count <= 0))
        i++;
    end=i;
    if ((i-=num_bins)<0)
        i=0; 

    for (; end<num_bins && (normal[end]->count > 0); end++)
        ;
    if (end>=num_bins) end=num_bins-1;
    end += ((double)(double)num_bins/(normal[num_bins-1]->to-normal[0]->from)) * extend_db;
    if (end>=num_bins) end=num_bins-1;
    for (; i<end; i++){
        sqr      = (double)(noise[i]->count - normal[i]->count) *
                   (double)(noise[i]->count - normal[i]->count) ;
        if (noise[i]->count == 0)
            sqr_sum += sqr*sqr;
        else
            sqr_sum += sqr;
    }
    return(sqr_sum);
}


/*************************************************************/
/* DOC-P:   hist_copy()                                      */
/* DOC-PS:  copy one histogram to another.                   */
void hist_copy(HIST **from, HIST **to, int num_bins, int start, int end)
{
    int i;
 
    for (i=start; (i<num_bins) && (i<=end); i++)
         to[i]->count = from[i]->count;
}

/*************************************************************/
/* DOC-P:   erase_hist()                                     */
/* DOC-PS:  reset all bin counts to zero.                    */
void erase_hist(HIST **hist, int num_bins)
{
    int i;

    for (i=0; i<num_bins; i++)
        hist[i]->count = 0;
}

/**********************************************************************/
/* DOC-P:   dump_hist()                                               */
/* DOC-PS:  print the contents of the histogram to the file pointer   */
void dump_hist(HIST **hist, int num_bins, FILE *fp)
{
    int i;

    fprintf(fp,"Dump of a histogram\n");
    for (i=0; i<num_bins; i++)
        fprintf(fp,"%5.3f : %5.3f -> %d\n",
                hist[i]->from,hist[i]->to,hist[i]->count);
}

/*************************************************************/
/* DOC-P:   dump_esps_hist()                                 */
/* DOC-PS:  dump a HIST in ESPS format for aplot             */
/* Print the histogram to the file "fname" in a format       */
/* readable by the entropic 'aplot' program                  */
void dump_esps_hist(HIST **hist, int num_bins, char *fname)
{
    int i,height;
    FILE *fp;

/*  printf("Dump of a histogram in ESPS format to %s\n",fname); */
    if ((fp=fopen(fname,"w")) == NULL){
        fprintf(stderr,"Warning: unable to open %s for writing\n",fname);
        return;
    }

    fprintf(fp,"%d\n",num_bins*3); /*each bin has three points */
    fprintf(fp,"1\n");             /*I'm not sure why */
    /* the X axis - beg, end, increment*/
    fprintf(fp,"%5.1f %5.1f 10.0\n",hist[0]->from,hist[num_bins-1]->to);
     /* the Y axis - beg, end, increment*/
    height= (int)(max_hist(hist,num_bins)+10)/10*10;
    fprintf(fp,"%d %d %d\n",0,height,height/10);

    for (i=0; i<num_bins; i++){
        fprintf(fp,"%5.1f %d\n", hist[i]->from,hist[i]->count);
        fprintf(fp,"%5.1f %d\n", hist[i]->to,hist[i]->count);
        fprintf(fp,"%5.1f 0\n", hist[i]->to);
    }
    fflush(fp);
    fclose(fp);
}

/**************************************************************/
/* DOC-P:   read_esps_hist()                                  */
/* DOC-PS:  read an ESPS format histogram                     */
/* Read an entropic histogram file, then create a histogram to*/
/* hold it's information and return that histogram            */
void read_esps_hist(HIST ***hist, int *num_bins, char *fname)
{
    int i;
    FILE *fp;
    HIST **t_hist;
    char buff[400];
    float hist_beg, hist_end, hist_incr;

    printf("Reading of a histogram in ESPS format from %s\n",fname);
    if ((fp=fopen(fname,"r")) == NULL){
        fprintf(stderr,"Warning: unable to open %s for reading\n",fname);
        return;
    }

    fscanf(fp,"%d\n",&i);
    *num_bins = i/3;
    printf("number of bins to read %d\n",*num_bins);

    fgets(buff,400,fp);  /* skip a line */
    
    fscanf(fp,"%f %f %f\n",&hist_beg, &hist_end, &hist_incr);
    printf("hist beg %f,  end %f  incr %f\n",hist_beg,hist_end,hist_incr);
    init_hist(&t_hist,*num_bins,hist_beg,hist_end);

    fgets(buff,400,fp); /* skip a line */

    for (i=0; i<*num_bins; i++){
        fscanf(fp,"%*f %d\n",&(t_hist[i]->count));
        fgets(buff,400,fp);  /* skip a line */
        fgets(buff,400,fp);  /* skip a line */
    }

    fclose(fp);
    *hist = t_hist;
}

/*************************************************************/
/* DOC-P:   dump_gnuplot_hist()                              */
/* DOC-PS:  dump a HIST in gnuplot format                    */
/* Write to the file 'fname' the histogram in a form usable  */
/* by gnuplot                                                */
void dump_gnuplot_hist(HIST **hist, int num_bins, char *fname)
{
    int i;
    FILE *fp;

/*  printf("Dump of a histogram in GNUPLOT format to %s\n",fname); */
    if ((fp=fopen(fname,"w")) == NULL){
        fprintf(stderr,"Warning: unable to open %s for writing\n",fname);
        return;
    }

    for (i=0; i<num_bins; i++){
        fprintf(fp,"%5.3f %d\n", (hist[i]->to-hist[i]->from)/2.0+hist[i]->from,
                hist[i]->count);
    }

    fflush(fp);
    fclose(fp);
}

/*************************************************************/
/* DOC-P:   dump_gnuplot_2hist()                             */
/* DOC-PS:  dump a 2 HIST's in gnuplot format                */
/* Write to the file 'fname' the histogram in a form usable  */
/* by gnuplot                                                */
void dump_gnuplot_2hist(HIST **hist1, HIST **hist2, int num_bins, char *fname)
{
    int i;
    FILE *fp;

/*  printf("Dump of a histogram in GNUPLOT format to %s\n",fname); */
    if ((fp=fopen(fname,"w")) == NULL){
        fprintf(stderr,"Warning: unable to open %s for writing\n",fname);
        return;
    }

    for (i=0; i<num_bins; i++){
        fprintf(fp,"%5.3f %d %d\n",
		(hist1[i]->to-hist1[i]->from)/2.0+hist1[i]->from,
                hist1[i]->count, hist2[i]->count);
    }

    fflush(fp);
    fclose(fp);
}

/*****************************************************************/
/* DOC-P:   half_cosine_hist()                                   */
/* DOC-PS:  store the values of cos(3pi/2) to cos(5PI/2) in hist */
void half_cosine_hist(HIST **hist, int num_bins, int begin_bin, int end_bin, int height)
{
    int i;
    double factor, heightby2;

    factor = 1.0 / (double)(end_bin-begin_bin+1);
    heightby2 = (double)height / 2.0;
    for (i=begin_bin;(i<num_bins) && (i <= end_bin);i++){
        hist[i]->count = heightby2 * (1.0+cos((double)(i-begin_bin)*factor*M_PI+M_PI));
    }
}

/****************************************************************/
/* DOC-P:   full_cosine_hist()                                  */
/* DOC-PS:  store the values of cos(pi/2) to cos(5pi/2) in hist */
void full_cosine_hist(HIST **hist, int num_bins, int begin_bin, int end_bin, int height)
{
    int i;
    double factor, heightby2;

    factor = 1.0 / (double)(end_bin-begin_bin+1);
    heightby2 = (double)height / 2.0;
    for (i=begin_bin;(i<num_bins) && (i <= end_bin+(end_bin-begin_bin));i++){
        hist[i]->count = heightby2 * (1.0+cos((double)(i-begin_bin)*factor*M_PI+M_PI));
    }
}

/*************************************************************/
/* DOC-P:   hist_area()                                      */
/* DOC-PS:  return the area of the HIST                      */
int hist_area(HIST **hist, int num_bins)
{
    int i, sum=0;

    for (i=0; i<num_bins; i++)
        sum+=hist[i]->count;
    return(sum);
}

/*************************************************************/
/* DOC-P:   hist_character()                                 */
/* DOC-PS:  compute the mean, variance and area of a HIST    */
void hist_character(HIST **hist, int num_bins, double *mean, double *vari, int *area)
{
    int i;
    double sum=0.0, ave;

    *area = hist_area(hist,num_bins);
    for (i=0; i<num_bins; i++)
        sum+=(hist[i]->from+(hist[i]->to - hist[i]->from)/2.0) * hist[i]->count;
    *mean = sum/(double)*area;
    sum=0.0;
    for (i=0; i<num_bins; i++){
        if (hist[i]->count > 0){
            ave = hist[i]->from + (hist[i]->to - hist[i]->from)/2.0 ;
            sum+= (ave - *mean) * (ave - *mean) * hist[i]->count;
            printf(" var %f ave %f mean %f\n",sum,ave,*mean);
        }
    }
    *vari = sum / (double)*area;
}

/*************************************************************/
/* DOC-P:   max_hist()                                       */
/* DOC-PS:  return the maximum value in a HIST               */
int max_hist(HIST **hist, int num_bins)
{
    int i, max=0;

    for (i=0; i<num_bins; i++)
        if (max<hist[i]->count)
            max=hist[i]->count;
    return(max);
}

/*********************************************************************/
/* DOC-P:   average_hieght_hist()                                    */
/* DOC-PS:  return tge average hieght in a the center of this window */
int average_hieght_hist(HIST **hist, int num_bins, int center, int window)
{
    int i, sum=0;

    for (i=center-window; i<=center+window; i++)
        if ((i>=0) && (i<num_bins))
	  sum+=hist[i]->count;
    return(sum/(window*2+1));
}

/*************************************************************/
/* DOC-P:   smooth_hist()                                    */
/* DOC-PS:  compute the average hieghts of the histogram     */
void smooth_hist(HIST **from, HIST **to, int num_bins, int window)
{
    int i, value=0,window2=window*2;

    for (value=0,i=(-window); i<(num_bins+window); i++){
        if (i-window>=0)
           value -= from[i-window]->count;
        if (i+window<num_bins)
           value += from[i+window]->count;
        if ((i>=0) && (i<num_bins))
           to[i]->count = value/window2;
    }
}

int comp(const void *a, const void *b)
{
    int *ai=(int *)a, *bi=(int *)b;
    return (*ai-*bi);
}

void median_filter(HIST **h, HIST **out, int num_bins, int size)  /* size must be ODD */
                                    /* h and out may be the same */
                  
{
  int bin,*out_vals,*temp,i,half_size,index,median;
  
  half_size=size/2;

  out_vals = (int *) malloc (num_bins * sizeof (int));
  temp = (int *) malloc (size * sizeof (int));

  for (bin=0; bin <= num_bins; bin++) { /* loop through all the bins */

    /* update analysis window */
    for (i=0; i<size; i++) {
      index=bin-half_size+i;
      temp[i]=((index >= 0) && (index < num_bins)) ? h[index]->count : 0;
    }
      
    /* find median of window */
    qsort(temp,size,sizeof(int),comp);
    median = temp[half_size];

    out_vals[bin]=median;
  }

  /* write filtered values back into output histogram */
  for (i=0; i<num_bins; i++)
    out[i]->count = out_vals[i];

  free(out_vals);
  free(temp);
}

    
int locate_extremum(HIST **h, int from, int to, int type)
{
  int i,j,extremum,swing_loc,k,next_swing_loc,diff1,diff2,pre_swing,post_swing;

  for (i=from+PEAK_WIDTH; i<to-PEAK_WIDTH; i++) {
    if (h[i]->count==0) continue; /* not interested in extrema at 0 */
    extremum=1; /* assume it's an extremum to begin with */
    pre_swing=post_swing=0;
    swing_loc=i-PEAK_WIDTH;
    for (j=i-PEAK_WIDTH; j<i; j++) /* check the preceding samples */
      if (type==PEAK) {
	if (h[j]->count > h[j+1]->count) {
	  extremum=0;
	  break;
	}
	if (h[j]->count != h[j+1]->count) {
	  pre_swing=1;
	}
      }	else { /* type == TROUGH */
	if (h[j]->count < h[j+1]->count) {
	  extremum=0;
	  break;
	}
	if (h[j]->count != h[j+1]->count) {
	  pre_swing=1;
	}
      }

    if (!extremum) continue;

    for (j=i; j<i+PEAK_WIDTH; j++) /* check the subsequent samples */
      if (type==PEAK) {
	if (h[j]->count < h[j+1]->count) {
	  extremum=0;
	  break;
	}
	if (h[j]->count != h[j+1]->count) {
	  post_swing=1;
	}
      }	else { /* type == TROUGH */
	if (h[j]->count > h[j+1]->count) {
	  extremum=0;
	  break;
	}
	if (h[j]->count != h[j+1]->count) {
	  post_swing=1;
	}
      }

    /* check to make sure it isn't a step function */
    /* this kind of check is necessary if the peak is wider than the window */
    if (((pre_swing+post_swing)<=1)&&(extremum)) {
      for (k=i; k>from; k--)
	if ((diff1 = (h[k-1]->count - h[k]->count)) != 0)
	  break;
      swing_loc=k;
      for (k=i; (k<to-1); k++)  /* find next swing */
	if ((diff2 = (h[k]->count - h[k+1]->count)) != 0)
	  break;
      next_swing_loc=k;
      if ((type==PEAK)&&((diff1>0)||(diff2<0))) continue;   /* no dice */
      if ((type==TROUGH)&&((diff1<0)||(diff2>0))) continue; /* ditto   */

      /* otherwise, the peak is at the mid-point of this plateau */
      return (int) (swing_loc+next_swing_loc)/2;
    }

    if (extremum) return i;
  }

  return to;
}

/*************************************************************/
/* DOC-P:   build_normal_hist()                              */
/* DOC-PS:  store the the HIST a normal distribution         */
void build_normal_hist(HIST **hist, int num_bins, double mean, double variance, double total_area)
{
    int i;
    double x, value, con, e;

    printf("Building a histogram for a normal distribution\n");
    printf("       mean %f,  variance %f, area %f\n",mean,variance,total_area);

    con = (1.0 / sqrt(2.0*M_PI*variance));
    /* first generate a N(mean,variance) dist scaled to 10000*/
    for (i=0; i<num_bins; i++){
        x = hist[i]->from + (hist[i]->to - hist[i]->from)/2.0;
        e = exp(-( (x-mean)*(x-mean) / (2*variance)));
        value = 10000.0 * con * e;
        hist[i]->count = value;
    }
    /* now normalize it to the total_area */
    { double scale; int area;
      area = hist_area(hist,num_bins);
      scale = (double)total_area/(double)area;
      for (i=0; i<num_bins; i++)
          hist[i]->count *= scale;
    }
}

/*************************************************************/
/* DOC-P:   hist_slope()       */
/* DOC-PS:  return the slope of histogram the center  */
int hist_slope(HIST **hist, int num_bins, int center, int factor)
{
    int ind, cnt;

    for (ind=0,cnt=0; ind < factor; ind++)
        if (center-ind < 0) 
            cnt -= hist[center+ind]->count;
        else if (ind+center>=num_bins)
            cnt += hist[center-ind]->count;
        else
            cnt += hist[center-ind]->count - hist[center+ind]->count;
    return((int)-(((double)cnt/(double)factor)*1000.0));
}

/*************************************************************/
/* DOC-P:   do_hist()       */
/* DOC-PS:  using an input array, compute a historgram */
void do_hist(HIST **hist, int num_bins, double *arr, int arr_cnt)
{
    int i, index, out_of_range=0;
    double dist, from;

    from = hist[0]->from;
    dist = hist[num_bins-1]->to - hist[0]->from;
    for (i=0; i<arr_cnt; i++){
        index = (int)((double)num_bins * (*(arr+i) - from) / dist);
        if ((index>=0) && (index<num_bins)) hist[index]->count++;
        else
            out_of_range++;
    }
}
