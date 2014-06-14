/************************************************************************/
/*    file: snr.c							*/
/*    Desc: The main control functions for calculation of signal to	*/
/*          noise ratio.						*/
/*    Date: Nov 27, 1990						*/
/*          modified Summer 1992 cas                                    */
/*	    modified July 95, better estimate of noise level map	*/
/*									*/
/************************************************************************/
/*
DOC:filename: snr.c
DOC:include:  ../include/hist.h
DOC:package:  signal to noise ratio computation utility
DOC:purpose:  to compute the signal to noise ratio of a digitized signal
DOC:
*/
#include <snr/snrlib.h>

#define SNR_C_VERSION "V1.0"

#ifndef M_PI
#define M_PI    3.14159265358979323846
#endif

double comp2 PROTO((int *vector));
void do_init_comp2 PROTO((HIST **ref, HIST **hyp, int num_bins, int peak_bin));


/* #define DUMP_HISTOGRAMS 1 */

/* Definitions for comp1, a distance function for comparing histograms */
static HIST **st_ref, **st_hyp;
static int st_num_bins, st_peak_bins;
static char name[40];

double pick_center(HIST **h, int bin);

/***********************************************************************/
/* DOC-P:compute_dc_bias()                                             */
/* DOC-PS:Compute the dc-bias of a .wav channel                        */
/* Arguments:                                                          */
/*     fp -> SP_FILE* to an opened .wav file                           */ 
/*     sp_char -> a simple structure of speech file characteristics    */
/*     channel -> channel to compute the dc-bias on                    */
/*                                                                     */
/* Reads file header to see if DC bias has already been computed. If   */
/* it has, it returns that value; otherwise it computes the DC value   */
/* from scratch. Returns DC_BIAS_ERROR if the number of samples it     */
/* reads in the .wav file does not match the number of samples         */
/* expected in the header file.                                        */
/*                                                                     */
/***********************************************************************/

double compute_dc_bias(register SP_FILE *sp, SPFILE_CHARS *sp_char, int channel)
{
  short *tsamp, *sam, *end_of_samp;
  int red, tot_red=0, i;
  int num_samples, diff=0;
  long sum=0;

  if (sp_rewind(sp) > 0)
      fprintf(spfp,"Warning: sp_rewind failed\n");
  if (sp_set_data_mode(sp,rsprintf("CH-%d:SE-PCM",channel+1)) != 0){
      fprintf(spfp,"compute_dc_bias: Error, sp_set_data_mode failed\n");
      sp_print_return_status(spfp);
      exit(-1);
  }
  tsamp = (short *)sp_data_alloc(sp,BLOCKSIZE);

  num_samples = sp_char->sample_count;
  sam = tsamp;
  end_of_samp = tsamp;
  do {
      red = sp_read_data(end_of_samp,BLOCKSIZE,sp);
      /* de-bias the signal */
      add_to_singarr(end_of_samp,red,(-(int)sp_char->dc_bias));

      end_of_samp += red;
      sam=tsamp;

      for (i=0; i<red; i++) sum+= *(sam++);
      
      /* if there is data left in the buffer, move it to the head*/
      /* of each buffer                                          */
      if (!(diff=end_of_samp - sam)) {
	  memcpy(tsamp,sam,diff*2);
	  end_of_samp = tsamp+diff;
	  sam=tsamp;
      } else {
	  sam = end_of_samp = tsamp;
      }
      tot_red += red;
  } while (red == BLOCKSIZE);
  
  if (tot_red != num_samples)
    printf("Warning: I just read %d samples whereas the fileheader promised %d samples.\n",
	   tot_red, num_samples);

  sp_data_free(sp,tsamp);
  if (sp_rewind(sp) > 0)
      fprintf(spfp,"Warning: sp_rewind failed\n");

  return (double) sum / (double) tot_red;
}



/***********************************************************************/
/* DOC-P:snr_all_channels()                                            */
/* DOC-PS:High level call to the SNR utility                           */
/* A high level accessor to the snr utility.  It uses an already opened*/
/* .wav file to call snr. Fills the array snrs[] of SNRs spanning all  */
/* the channels. Returns the number of channels for a normal exit,     */
/* and -1 to indicate an error.                                        */
/* Allocates space for the snrs array. That means the user should pass */
/* a pointer to an array, and not simply an array, as the snrs arg.    */
/***********************************************************************/

int snr_all_channels(SP_FILE *sp, double ***signal, double ***noise, double ***snrs, double ***cross, Filterbank *filters, int codec, int graph, char *filename, double pct_speech_level)
         
                          /* pointer to 2d array: signal to noise ratios */
                          /* ditto: signal powers, noise powers.         */
                          /* ditto: crosstalk powers.                    */
{
    double noise_level, speech_level, sig_to_noise_ratio, cross_level;
    SPFILE_CHARS sp_char;
    char fname[200];
    int channel,band,bands,i;    
    
    if (sp_rewind(sp) > 0)
	fprintf(spfp,"Warning: sp_rewind failed\n");
    
    load_sp_char(sp,&sp_char);
    if ((strcmp(sp_char.sample_coding,"mu-law") == 0) && !codec){
	fprintf(stderr,"Error: Codec is input, but codec flag is false\n");
	fprintf(stderr,"       coding assumption failed\n");
	return(-1);
    }
    
    /* allocate output arrays */
    bands = (filters == DONT_FILTER) ? 1 : filters->bands;
    *snrs = (double **) malloc (sp_char.channel_count*sizeof(double *));
    *signal = (double **) malloc (sp_char.channel_count*sizeof(double *));
    *noise = (double **) malloc (sp_char.channel_count*sizeof(double *));
    if (codec) 
	*cross = (double **) malloc (sp_char.channel_count*sizeof(double *));
    for (i=0; i< sp_char.channel_count; i++)
	(*snrs)[i] = (double *) malloc (bands*sizeof(double));
    for (i=0; i< sp_char.channel_count; i++)
	(*signal)[i] = (double *) malloc (bands*sizeof(double));
    for (i=0; i< sp_char.channel_count; i++)
	(*noise)[i] = (double *) malloc (bands*sizeof(double));
    if (codec)
	for (i=0; i< sp_char.channel_count; i++)
	    (*cross)[i] = (double *) malloc (bands*sizeof(double));
    
    for (channel=0; channel<sp_char.channel_count; channel++) {
	/* loop through all the channels */
	
	if (filters != DONT_FILTER) {
	    for (band=0; band < filters->bands; band++) {
		/* loop through all the subbands */
		printf("Processing band %d\n",band);
		sprintf(fname,"%s-chan%d-band%d",filename,channel,band);
		snr_single_channel(sp,channel,&noise_level,&speech_level,
				   &sig_to_noise_ratio,&cross_level,
				   /*QUICK*/FALSE,filters->coeff[band],
				   filters->taps[band],codec,graph,fname,
				   pct_speech_level);
		(*signal)[channel][band] = (double)speech_level;
		(*noise)[channel][band] = (double)noise_level;
		(*snrs)[channel][band] = (double)sig_to_noise_ratio;
		if (codec) (*cross)[channel][band] = (double)cross_level;
		if (filters == DONT_FILTER) printf("NULL FILTERS.\n");
	    }
	} else {
	    sprintf(fname,"%s-chan%d",filename,channel);
	    snr_single_channel(sp,channel,&noise_level,&speech_level,
			       &sig_to_noise_ratio,&cross_level,
			       /*QUICK*/FALSE,NULL_FILTER,0,codec,graph,fname,
			       pct_speech_level);
	    (*signal)[channel][0] = (double)speech_level;
	    (*noise)[channel][0] = (double)noise_level;
	    (*snrs)[channel][0] = (double)sig_to_noise_ratio;
	    if (codec) (*cross)[channel][0] = (double)cross_level;
	}
    }
    return(sp_char.channel_count);
}

int quick_snr_all_channels(SP_FILE *sp, double ***signal, double ***noise, double ***snrs, Filterbank *filters, int graph, char *filename)
                          /* pointer to 2d array: signal to noise ratios */
                          /* ditto: signal powers, noise powers.         */
{
    double noise_level, speech_level, sig_to_noise_ratio;
    SPFILE_CHARS sp_char;
    char fname[200];
    int channel,band,bands,i;    
    double cross_level;
    
    load_sp_char(sp,&sp_char);
    
    /* allocate output arrays */
    bands = (filters == DONT_FILTER) ? 1 : filters->bands;
    *snrs = (double **) malloc (sp_char.channel_count*sizeof(double *));
    *signal = (double **) malloc (sp_char.channel_count*sizeof(double *));
    *noise = (double **) malloc (sp_char.channel_count*sizeof(double *));
    for (i=0; i< sp_char.channel_count; i++)
	(*snrs)[i] = (double *) malloc (bands*sizeof(double));
    for (i=0; i< sp_char.channel_count; i++)
	(*signal)[i] = (double *) malloc (bands*sizeof(double));
    for (i=0; i< sp_char.channel_count; i++)
	(*noise)[i] = (double *) malloc (bands*sizeof(double));
    
    for (channel=0; channel<sp_char.channel_count; channel++) {
	/* loop through all the channels */
	
	if (sp_rewind(sp) > 0)
	    fprintf(spfp,"Warning: sp_rewind failed\n");
	
	if (filters != DONT_FILTER) {
	    for (band=0; band < filters->bands; band++) {
		/* loop through all the subbands */
		printf("Processing band %d\n",band);
		
		sprintf(fname,"%s-chan%d-band%d",filename,channel,band);
		snr_single_channel(sp,channel,&noise_level,&speech_level,
				   &sig_to_noise_ratio, &cross_level,
				   /*QUICK*/TRUE,filters->coeff[band],
				   filters->taps[band],FALSE,graph,fname,
				   0.0);
		(*signal)[channel][band] = (double)speech_level;
		(*noise)[channel][band] = (double)noise_level;
		(*snrs)[channel][band] = (double)sig_to_noise_ratio;
		if (filters == DONT_FILTER) printf("NULL FILTERS.\n");
	    }
	}
	else {
	    sprintf(fname,"%s-chan%d",filename,channel);
	    snr_single_channel(sp,channel,&noise_level,&speech_level,
			       &sig_to_noise_ratio,&cross_level,
			       /*QUICK*/TRUE,NULL_FILTER,0,FALSE,graph,fname,
			       0.0);
	    (*signal)[channel][0] = (double)speech_level;
	    (*noise)[channel][0] = (double)noise_level;
	    (*snrs)[channel][0] = (double)sig_to_noise_ratio;
	}
    }
    return(sp_char.channel_count);
}



/***********************************************************************/
/* DOC-P:snr_single_channel()                                          */
/* DOC-PS:SNR utility, takes a .wav file pointer and a channel number  */
/* DOC-PS:and returns the signal-to-noise ratio. It also generates the */
/* DOC-PS:power histogram and if needed calls the estimation algorithm */
/* Arguments:                                                          */
/*     fp -> SP_FILE * to an opened .wav file                          */
/*     h -> The sphere header for that .wav file                       */
/*     channel -> which channel, of multi-channel files to use         */
/*     noise_level -> double * to return the db noise level             */
/*     speech_level -> double * to return the db speech level           */
/*     sig_to_noise_ratio -> double * to return the SNR                 */
/*     do_quick_snr -> just use the 15% and 85% as the Noise L and     */
/*                     Speech Level for computing SNR                  */
/***********************************************************************/
int snr_single_channel(SP_FILE *sp, int channel, double *noise_level, double *speech_level, double *sig_to_noise_ratio, double *cross_level, int do_quick_snr, double *coeff, int taps, int codec, int graph, char *filename, double pct_speech_level)
{
  SPFILE_CHARS sp_char;
  HIST **power_hist;
  int num_samples, frame_width, frame_adv;
  
  if (sp_rewind(sp) > 0)
      fprintf(spfp,"Warning: sp_rewind failed\n");
  if (sp_set_data_mode(sp,rsprintf("CH-%d:SE-PCM",channel+1)) != 0){
      fprintf(spfp,"snr_single_channel: Error, sp_set_data_mode failed\n");
      sp_print_return_status(spfp);
      exit(-1);
  }
  
  load_sp_char(sp,&sp_char);
  
  num_samples=sp_char.sample_count;
  frame_width = sp_char.sample_rate / 1000 * 20; /* 20 Milliseconds */
  frame_adv = frame_width / 2;
  while (((num_samples / frame_adv) < 2000) && (frame_adv > 1))
    frame_adv--;
  
  sp_char.dc_bias = compute_dc_bias(sp,&sp_char,channel);
  
  init_hist(&power_hist,BINS,LOW,HIGH);
  if (compute_pwr_hist_sd(sp,power_hist,BINS,&sp_char,channel,frame_width,frame_adv,
			  coeff,taps) < 0) {
    (void) fprintf(stderr,"Error: couldn't compute a power histogram\n");
    return(-1);
  }
  
  if (do_quick_snr) {
      *noise_level  = percentile_hist(power_hist,BINS,0.15);
      *speech_level = percentile_hist(power_hist,BINS,0.85);
  } else { /* perform the long computation */
      snr(power_hist, BINS, pct_speech_level,noise_level,speech_level,cross_level,
	  codec,graph,filename);
  }
  
  *sig_to_noise_ratio =  *speech_level - *noise_level;

  free_hist(&power_hist,BINS);
  return(0);
}

/* finds the snr given the extra information of a segmentation list */
int segsnr_single_channel(SP_FILE *sp, int channel, double *noise_level, double *speech_level, double *sig_to_noise_ratio, int **list, int segments, double *coeff, int taps)
{
  SPFILE_CHARS *sp_char;
  HIST **speech_hist;
  short buffer[BLOCKSIZE];
  double noise_sum,power;
  int current_segment,window,half_window,noise_frames,frame;

  init_hist(&speech_hist,BINS,LOW,HIGH);   /* set up the speech histogram */

  if (sp_rewind(sp) > 0)
      fprintf(spfp,"Warning: sp_rewind failed\n");
  if (sp_set_data_mode(sp,rsprintf("CH-%d:SE-PCM",channel+1)) != 0){
      fprintf(spfp,"segsnr_single_channel: Error, sp_set_data_mode failed\n");
      sp_print_return_status(spfp);
      exit(-1);
  }
  
  sp_char = (SPFILE_CHARS *) malloc (sizeof(SPFILE_CHARS));
  load_sp_char(sp,sp_char);

  /* calculate buffer size we need according to sample rate */
  window = ROUND((double)sp_char->sample_rate*MILI_SEC/1000.0);
  if (MOD(window,2)==1) window +=1;
  half_window = window/2;

  sp_char->dc_bias = compute_dc_bias(sp,sp_char,channel);

  frame=0;
  noise_frames=0;
  noise_sum=0.0;
  current_segment=0;

  head_insert_hist(speech_hist,BINS)

    while ((sp_read_data((char *)buffer,half_window,sp)==half_window)&&
	   (current_segment < segments)) {
	/* de-bias the signal */
	add_to_singarr(buffer,BLOCKSIZE,(-(int)sp_char->dc_bias));

	power = pwr1(buffer,half_window);
	if ((frame >= list[current_segment][FROM]) &&
	    (frame <= list[current_segment][TO])) {
	    /* if we're in the speech zone, */
	    body_insert_hist(speech_hist,BINS,power);
	} else {
	    /* we're in the noise zone */
	    noise_sum += power;
	    noise_frames++;
	}
	if (frame > list[current_segment][TO]) current_segment++;
	frame++;
    }
  } /* closes head_insert_hist block */

  *noise_level = noise_sum / (double) noise_frames;
  *speech_level = percentile_hist (speech_hist,BINS,PEAK_LEVEL);
  *sig_to_noise_ratio = *speech_level - *noise_level;

#ifdef DUMP_HISTOGRAMS
    dump_gnuplot_hist(speech_hist,BINS,"speech.hist");
#endif    

  free_hist(&speech_hist,BINS);
  if (sp_rewind(sp) > 0)
      fprintf(spfp,"Warning: sp_rewind failed\n");
  free(sp_char);
  return(0);
}




/***********************************************************************/
/* DOC-P:compute_pwr_hist_sd()                                         */
/* DOC-PS:Compute a power histogram of a .wav channel, without loading */
/* DOC-PS:the entire file.                                             */
/* Arguments:                                                          */
/*     sp ->SP_FILE * to an opened .wav file                           */ 
/*     pwr_hist -> The output histogram                                */
/*     num_bins -> The number of bins contained for the histograms     */
/*     sp_char -> a simple structure of speech file characteristics    */
/*     channel -> the channel to compute the histogram for             */
/*     frame_width -> width of the computation frame in milliseconds   */
/*     frame_adv -> the frame advance rate                             */
/* Also filters the input for sub-band SNR.                            */
/* Bug fixes:                                                          */
/*     10/11/92  Removed reference for unused FILE *out                */
/***********************************************************************/
int compute_pwr_hist_sd(SP_FILE *sp, HIST **pwr_hist, int num_bins, SPFILE_CHARS *sp_char, int channel, int frame_width, int frame_adv, double *coeff, int taps)
{
    short *tsamp, *sam, *end_of_samp;
    int red, tot_red=0;
    int num_samples, diff=0;
    double pwr;
    int first_frame;

    /*  DON'T NEED IT 
	if (coeff != NULL_FILTER) {
	memory = (double *) malloc (taps*sizeof(double));
	for (i=0; i < taps; i++) memory[i]=0.0;
	}
	*/
    num_samples = sp_char->sample_count;
    
    tsamp = sp_data_alloc(sp,BLOCKSIZE+frame_width);
    sam = tsamp;
    end_of_samp = tsamp;

    first_frame=1;

    do {
	red = sp_read_data(end_of_samp,BLOCKSIZE,sp);
	if (red == 0 && (sp_error(sp) != 0)){
	    sp_print_return_status(stderr);
	    sp_file_dump(sp,stderr);
	    exit(1);
	}
	/* de-bias the signal */
	add_to_singarr(end_of_samp,red,(-(int)sp_char->dc_bias));

	/* filter input if necessary */
	if (coeff != NULL_FILTER) {
	    if (first_frame) {
		first_frame=0;
		fft_filter_init(coeff,BLOCKSIZE,taps);
	    }
	    fir_filter_fft (sam,red,coeff,taps);
	}

	end_of_samp += red;
	head_insert_hist(pwr_hist,num_bins)
	    while (end_of_samp - sam > frame_width){
		pwr = pwr1(sam,frame_width); /* compute log magnitude of (filtered) speech vector */
		if (pwr != NEGATIVE_INFINITY)
		    body_insert_hist(pwr_hist,num_bins,pwr);  /* insert that value in the histogram */
		sam+=frame_adv;
	    }
	end_insert_hist(pwr_hist,num_bins)

	/* if there is data left in the buffer, move it to the head*/
	/* of each buffer                                          */
	if ((diff=end_of_samp-sam) != 0){
	    memcpy(tsamp,sam,diff*2);
	    end_of_samp = tsamp+diff;
	    sam=tsamp;
	} else {
	    printf("Input depleted\n");
	    sam = end_of_samp = tsamp;
	}
	tot_red += red;
    } while (red == BLOCKSIZE);
    
    /*  free(memory); */
    sp_data_free(sp,tsamp);
    return(1);      /*  1 flags OK */
}

    
/***********************************************************************/
/* DOC-P:snr()                                                         */
/* DOC-PS:Make fitted curve to the noise portion peak and locate the   */
/* DOC-PS:Nth percentile of the file                                   */
/* Arguments:                                                          */
/*     full_hist -> The input power histogram                          */
/*     num_bins -> The number of bins the the histograms               */
/*     cutoff_percentile -> percentile of the peak speech level        */
/*     noise_lvl -> double * to return the noise db level               */
/*     speech_lvl -> double * to return the speech db level             */
/* This procedure computes the signal to noise ratio for a speech      */
/* data file by:                                                       */
/*       1. fit a raised-cosine curve to only the left side of the     */
/*          power histogram                                            */
/*       2. subtract the raised-cosine function from the power        */
/*          histogram                                                  */
/*       3. Find the Nth percentile of the result of 2.                */
/*       4. Return the result of 3. minus the mean of the midpoint of  */
/*          the histogram.                                             */
/***********************************************************************/
void snr(HIST **full_hist, int num_bins, double cutoff_percentile, double *noise_lvl, double *speech_lvl, double *cross_lvl, int codec, int graph, char *filename)
{
  HIST **cos_hist, **work_hist;

  char name[100];
 
#ifdef DUMP_HISTOGRAMS
  sprintf(name,"%s/complete.his",getenv("HOME"));
  dump_esps_hist(full_hist,num_bins,name);
#endif

  if (codec) { /* if it's switchboard data, forget about */
               /* curve-fitting and pick peaks instead.  */
    pick_peaks(full_hist,noise_lvl,cross_lvl,speech_lvl);
    printf("S = %5.2f N = %5.2f C = %5.2f\n",*speech_lvl,*noise_lvl,*cross_lvl);
    return;
  }


  init_hist(&cos_hist,num_bins,full_hist[0]->from,
	    full_hist[num_bins-1]->to);
  init_hist(&work_hist,num_bins,full_hist[0]->from,
	    full_hist[num_bins-1]->to);
  
  build_raised_cos_hist(full_hist,cos_hist,num_bins,noise_lvl);

#ifdef DUMP_HISTOGRAMS
  sprintf(name,"%s/cos.his",getenv("HOME"));
  dump_esps_hist(cos_hist,num_bins,name);
#endif

  erase_hist(work_hist,num_bins);
  subtract_hist(cos_hist,full_hist,work_hist,num_bins);
  if (getenv("STNR_RMS_SPEECH") == (char *)0)
      *speech_lvl = percentile_hist(work_hist,num_bins,cutoff_percentile);
  else
      *speech_lvl = db_RMS_hist(work_hist, num_bins);


#ifdef DUMP_HISTOGRAMS
  sprintf(name,"%s/subtract.his",getenv("HOME"));
  dump_esps_hist(work_hist,num_bins,name);
#endif

  if (graph) {
      char outname[200];
      FILE *fp;
      sprintf(outname,"%s.dat",filename);
      dump_gnuplot_2hist(full_hist,cos_hist,BINS,outname);
      if (0){
	  sprintf(outname,"%s.dat.sub",filename);
	  dump_gnuplot_hist(work_hist,num_bins,outname);
      }

      sprintf(outname,"%s.plt",filename);
      if ((fp=fopen(outname,"w")) == (FILE *)0){
	  printf("Warning: Can not open gnuplot file %s\n",outname);
      } else {
	  fprintf(fp,"#set terminal postscript\n");
	  fprintf(fp,"set samples 1000\n");
	  fprintf(fp,"set nolabel\n");
	  fprintf(fp,"set ylabel %cCounts%c\n",'"','"');
	  fprintf(fp,"set xlabel %cDecibels%c\n",'"','"');
	  fprintf(fp,"set xtics -100,10\n");
	  fprintf(fp,"set arrow 1 from %5.2f,%d to %5.2f,%5.2f\n",
		  *noise_lvl,max_hist(full_hist,BINS)/4,*noise_lvl,0.0);
	  fprintf(fp,"set arrow 2 from %5.2f,%d to %5.2f,%5.2f\n",
		  *speech_lvl,max_hist(full_hist,BINS)/4,*speech_lvl,0.0);

	  fprintf(fp,"set label \"Noise Level  = %5.2f\" at -20,%d\n",
		  *noise_lvl,max_hist(full_hist,BINS)/2);
	  fprintf(fp,"set label \"Speech Level = %5.2f\" at -20,%d\n",
		  *speech_lvl,max_hist(full_hist,BINS)/3);

	  fprintf(fp,"set title \"SNR %5.2f power histograms file '%s'\"\n",
		  *speech_lvl-*noise_lvl,outname);
	  sprintf(outname,"%s.dat",filename);
	  fprintf(fp,"plot '%s' using 1:2 \"%%f%%f\" title 'Complete' with lines, '%s' using 1:2 \"%%f%%*s%%f\" title \"Noise\" with lines\n",
		  outname,outname);
	  if (0)
	      fprintf(fp,"plot '%s.sub' using 1:2 \"%%f%%f\" title 'Subtracted hist' with lines\n",
		      outname);


	  fclose(fp);
      }
  }

  free_hist(&work_hist,num_bins);
  free_hist(&cos_hist,num_bins);
}


/* trashes full_hist! */
void  pick_peaks(HIST **full_hist, double *noise_lvl, double *cross_lvl, double *speech_lvl)
{
  HIST **smoothed_hist,**unspiked_hist,**presmooth_hist;
  char name[50];
  int first_peak,first_trough,second_peak,second_trough,i,max_val,starting_point;

  init_hist(&unspiked_hist,BINS,full_hist[0]->from,full_hist[BINS-1]->to);
  init_hist(&presmooth_hist,BINS,full_hist[0]->from,full_hist[BINS-1]->to);
  init_hist(&smoothed_hist,BINS,full_hist[0]->from,full_hist[BINS-1]->to);
  
  median_filter(full_hist,unspiked_hist,BINS,3);
  smooth_hist(unspiked_hist,presmooth_hist,BINS,CODEC_SMOOTH_BINS);
  smooth_hist(presmooth_hist,smoothed_hist,BINS,SMOOTH_BINS);

#ifdef DUMP_HISTOGRAMS
  sprintf(name,"%s/smooth.his",getenv("HOME"));
  dump_esps_hist(smoothed_hist,BINS,name);
#endif

  /* assume to begin with that we don't find any extrema */
  first_peak=first_trough=second_peak=second_trough=BINS;

  max_val = max_hist(smoothed_hist,BINS);

  /* now look for the extrema, sequentially */

  /* find the noise peak; it should be reasonably big */
  starting_point=0;
  do {
    first_peak = locate_extremum(smoothed_hist,starting_point,BINS,PEAK);
    starting_point = first_peak+1;
  } while (10*smoothed_hist[first_peak]->count < max_val);

  /* now find the rest */
  first_trough = locate_extremum(smoothed_hist,first_peak+1,BINS,TROUGH);
  second_peak = locate_extremum(smoothed_hist,first_trough+1,BINS,PEAK);
  second_trough = locate_extremum(smoothed_hist,second_peak+1,BINS,TROUGH);

  printf("peak=%d (%5.2f) trough=%d (%5.2f) peak=%d (%5.2f) trough=%d (%5.2f)\n",
	 first_peak,
	 pick_center(smoothed_hist,first_peak),
	 first_trough,
	 pick_center(smoothed_hist,first_trough),
	 second_peak,
	 pick_center(smoothed_hist,second_peak),
	 second_trough,
	 pick_center(smoothed_hist,second_trough));

  if (first_peak==BINS) {
    printf("I can't find the first peak of the power distribution. Is this a null file?\n");
    exit(-1);
  }

  *noise_lvl = ((double)smoothed_hist[first_peak]->from +
		smoothed_hist[first_peak]->to)/2.0;
  
  if (first_trough==BINS) {
    printf("Can't find first trough. I'll do my best from here...\n");
    for (i=0; i<first_peak; i++) full_hist[i]->count = 0;
    *cross_lvl = NEGATIVE_INFINITY;
    *speech_lvl = percentile_hist(unspiked_hist,BINS,PEAK_LEVEL);
    free_hist(&unspiked_hist,BINS);
    free_hist(&smoothed_hist,BINS);
    free_hist(&presmooth_hist,BINS);
    return;
  }

  for (i=0; i<first_trough; i++) unspiked_hist[i]->count=0;

  if (second_peak==BINS) {
    printf("Can't find second peak.");
    *cross_lvl = NEGATIVE_INFINITY;
    *speech_lvl = percentile_hist(unspiked_hist,BINS,PEAK_LEVEL);
    free_hist(&unspiked_hist,BINS);
    free_hist(&smoothed_hist,BINS);
    free_hist(&presmooth_hist,BINS);
    return;
  }

  /* check for bogus hump */
  if (60*(smoothed_hist[second_peak]->count-smoothed_hist[first_trough]->count) <
      smoothed_hist[first_peak]->count)
    *cross_lvl = NEGATIVE_INFINITY;
  else
    *cross_lvl = ((double)smoothed_hist[second_peak]->from +
		  smoothed_hist[second_peak]->to)/2.0;

  for (i = 0;
       i < ((second_trough==BINS) ? second_peak : second_trough);
       i++)
    unspiked_hist[i]->count=0;

  *speech_lvl = percentile_hist(unspiked_hist,BINS,PEAK_LEVEL);

  free_hist(&unspiked_hist,BINS);
  free_hist(&smoothed_hist,BINS);
  free_hist(&presmooth_hist,BINS);
  return;
}

/***********************************************************************/
/* DOC-P:build_raised_cos_hist()                                       */
/* DOC-PS:Given a power histogram, fit a COS curve to the leftmost     */
/* DOC-PS:peak                                                         */
/* Arguments:                                                          */
/*     ref_hist -> The input power histogram                           */
/*     ret_hist -> The output hist of a COS function                   */
/*     num_bins -> The number of bins the the histograms               */
/*     noise_peak -> double * to the peak of the COS function           */
/***********************************************************************/
void build_raised_cos_hist(HIST **ref_hist, HIST **ret_hist, int num_bins, double *noise_peak)
{
  HIST **work_hist;
  int begin_val=1, begin_bin, peak_bin=0, tmp;
  int max_height;
  int half_peak_height, begin_top, end_top, i;
  int vector[3];
  double comp1(int *vector), chg_fact[3], chg_limit[3];
  double comp2(int *vector);
  double noise_lvl;
  char ch_noise_level[10];
  char snr_type[20],snr_data[20];

  int noisy_data_flag=0;


/*** map 7/10/95 ***/
  strcpy(snr_type,"SNR_DATA_TYP");
  strcpy(snr_data,"SNR_NOISE_VAL");

  if (getenv(snr_type) != NULL)
      {
	  noisy_data_flag=1;
	  strcpy(ch_noise_level,getenv(snr_data));
	  noise_lvl=atof(ch_noise_level);
      }


  init_hist(&work_hist,num_bins,ref_hist[0]->from,
	    ref_hist[num_bins-1]->to);
  smooth_hist(ref_hist,work_hist,num_bins,SMOOTH_BINS);

#ifdef DUMP_HISTOGRAMS
  sprintf(name,"%s/smooth.his",getenv("HOME"));
  dump_esps_hist(work_hist,num_bins,name);
#endif


/*** map 07/10/95 ***/
  if (noisy_data_flag == 1) 
      {
	  for (i=0;(i<num_bins);i++)
	      if (noise_lvl <= ref_hist[i]->to &&
                  noise_lvl >= ref_hist[i]->from)
		  peak_bin=i;
      }


  /* set the threshold for the beginning of the histogram */
  begin_val = (int)((double)max_hist(work_hist,num_bins) * 0.05);

  /* find the beginning of the hist and the first peak */
  for (i=0;(i<num_bins) && (work_hist[i]->count <= begin_val);i++);

  begin_bin = i;
  /* calculate where we think the peak bin should be */

  /* first the slope method */
  for (i=begin_bin; (i<num_bins) &&
       ((tmp=hist_slope(work_hist,num_bins,i,2))>=0); i++);

/*** map 07/10/95 ***/
  if (noisy_data_flag == 0) 
      peak_bin = i;

  if (getenv("SHOW_SEARCH") != (char *)0){
      printf("SHOW_SEARCH peak_bin set to %d (%.2f-%.2f DB)\n",
	     peak_bin,work_hist[peak_bin]->from,work_hist[peak_bin]->to);
  }

  /* find the maximum height on the original histogram within +|- */
  /* 5 bins */
  max_height = 0;
  for (i=peak_bin-5; i<=peak_bin+5; i++)
    if ((i>=0) && (i<num_bins))
      if (ref_hist[i]->count > max_height)
	max_height =ref_hist[i]->count;

  half_peak_height = work_hist[peak_bin]->count / 2;
  begin_top=end_top=peak_bin; 
  for (i=peak_bin;(i>=0) && (work_hist[i]->count > half_peak_height);i--)
    begin_top=i; 
  for (i=peak_bin;(i<num_bins) && (work_hist[i]->count > half_peak_height);
       i++)
      end_top=i; 
 
  erase_hist(work_hist,num_bins);
  hist_copy(ref_hist,work_hist,num_bins,0,num_bins);

  /* set up a vector for doing the direct search */
  /* then generate a full cosine wave for the using the best fit */
  
  vector[0] = peak_bin;          /* middle */
  vector[1] = max_height; /*half_peak_height*4; */ /* height */
  if (noisy_data_flag == 1)
      vector[2] = (peak_bin-begin_bin)*2;  /* width */
  else
      vector[2] = (peak_bin-begin_top)*4;  /* width */

  if (getenv("SHOW_SEARCH") != (char *)0){
      printf("SHOW_SEARCH Initial search vector   P1(mid): %d  P2(height): %d  P3(width(bins)): %d\n",
	     vector[0],vector[1],vector[2]);
      printf("SHOW_SEARCH Initial search vector   P1(db): %.2f,%.2f   P3(db): %.2f,%.2f\n",
	     work_hist[vector[0]]->from,work_hist[vector[0]]->to,
	     work_hist[vector[0]-(vector[2]/2)]->from,work_hist[vector[0]+(vector[2]/2)]->to);      
  }
  

  if (noisy_data_flag == 1)
        chg_fact[0] = num_bins * 0.00;
  else
      chg_fact[0] = num_bins * 0.01;
 
  chg_fact[1] = max_height * 0.05;
  chg_fact[2] = num_bins * 0.01;  

  
  if (chg_fact[0]<2.0 && noisy_data_flag==0) chg_fact[0]=2.0;
  if (chg_fact[1]<2.0) chg_fact[1]=2.0;
  if (chg_fact[2]<2.0) chg_fact[2]=2.0;

  chg_limit[0] = chg_fact[0] / 2;
  chg_limit[1] = chg_fact[1] / 2;
  chg_limit[2] = chg_fact[2] / 2;

  if (noisy_data_flag == 1) {
      do_init_comp2(work_hist,ret_hist,num_bins,peak_bin);
      direct_search(vector+1,2,chg_fact+1,0.7,chg_limit+1,comp2,
		    getenv("STRN_FULL_SRCH") != (char *)0);
      special_cosine_hist(ret_hist,num_bins,
		      peak_bin,vector[1],vector[2]);
  
      *noise_peak = (ret_hist[peak_bin]->from+ret_hist[peak_bin]->to)/2.0;

  }
  else {
      do_init_comp1(work_hist,ret_hist,num_bins);
      direct_search(vector,3,chg_fact,0.7,chg_limit,comp1,
		    getenv("STRN_FULL_SRCH") != (char *)0);
      special_cosine_hist(ret_hist,num_bins,
		      vector[0],vector[1],vector[2]);
  
      *noise_peak = (ret_hist[vector[0]]->from+ret_hist[vector[0]]->to)/2.0;

  }

  free_hist(&work_hist,num_bins);
} 


/***********************************************************************/
/* DOC-P:comp2()                                                       */
/* DOC-PS:compare two histograms based on an input vector,  The vector */
/* DOC-PS:which is intended to be modified by the direct search algo.  */
/* DOC-PS:"ds.c" contains parameters for modifying the COS hist        */
/* Arguments:                                                          */
/***********************************************************************/

double comp2(int *vector)
{
    double result, do_least_squares(HIST **, HIST **, int);

    if ((vector[0] < 10) || (vector[1] < 4)){
	result = 99999999.99; /* a really large double */
	if (getenv("SHOW_SEARCH") != (char *)0){
	    printf("SHOW_SEARCH  P1: %d  P2: %d  UNDER THRESHHOLDS\n",
		   vector[0],vector[1]);
	}
    } else {
	erase_hist(st_hyp,st_num_bins);
	/* at least 4 bins wide, height at least 10 */

	special_cosine_hist(st_hyp,st_num_bins,
			    st_peak_bins,vector[0],vector[1]);
	result=do_least_squares(st_ref,st_hyp,st_num_bins);
	if (getenv("SHOW_SEARCH") != (char *)0){
	    static int sid=1;
	    char *fle=rsprintf("search_hist.%04d.hist",sid++);
	    printf("SHOW_SEARCH file: %s  P1: %d  P2: %d  LS: %f\n",
		   fle,vector[0],vector[1],result);
	    dump_gnuplot_hist(st_hyp,st_num_bins,fle);
	}
    }
    return(result);
}


/***********************************************************************/
/* DOC-P:comp1()                                                       */
/* DOC-PS:compare two histograms based on an input vector,  The vector */
/* DOC-PS:which is intended to be modified by the direct search algo.  */
/* DOC-PS:"ds.c" contains parameters for modifying the COS hist        */
/* Arguments:                                                          */
/***********************************************************************/

double comp1(int *vector)
{
    double result, do_least_squares(HIST **, HIST **, int);
    if ((vector[0] <= 0) || (vector[1] < 10) || (vector[2] < 4)){	
	if (getenv("SHOW_SEARCH") != (char *)0){
	    printf("SHOW_SEARCH file: P1: %d  P2: %d  P3: %d  UNDER THRESHHOLDS\n",
		   vector[0],vector[1],vector[2]);
	}
	result = 99999999.99; /* a really large double */
    } else {

	erase_hist(st_hyp,st_num_bins);
	/* at least 4 bins wide, height at least 10 */
	
	special_cosine_hist(st_hyp,st_num_bins,
			    vector[0],vector[1],vector[2]);
	result=do_least_squares(st_ref,st_hyp,st_num_bins);
	if (getenv("SHOW_SEARCH") != (char *)0){
	    static int sid=1;
	    char *fle=rsprintf("search_hist.%04d.hist",sid++);
	    printf("SHOW_SEARCH file: %s  P1: %d  P2: %d  P3: %d  LS: %f\n",
		   fle,vector[0],vector[1],vector[2],result);
	    dump_gnuplot_hist(st_hyp,st_num_bins,fle);
	}
    }
    return(result);
}



/***********************************************************************/
/* DOC-P:do_init_comp1()                                               */
/* DOC-PS:Initialize the comp1 histogram comparison routine            */
/* Arguments:                                                          */
/*     ref -> The reference histogram to compare to                    */
/*     hyp -> The new histogram                                        */
/*     num_bins -> the number of bins in the histograms                */
/***********************************************************************/

void do_init_comp1(HIST **ref, HIST **hyp, int num_bins)
{
  st_ref = ref;
  st_hyp = hyp;
  st_num_bins = num_bins;
}


void do_init_comp2(HIST **ref, HIST **hyp, int num_bins, int peak_bin)
{
  st_ref = ref;
  st_hyp = hyp;
  st_num_bins = num_bins;
  st_peak_bins = peak_bin;
}


/***********************************************************************/
/* DOC-P:special_cosine_hist()                                         */
/* DOC-PS:Build a cosine curve cos(pi/2) - cos(5pi/2) inside a histgram*/
/* DOC-PS:structure                                                    */
/* Arguments:                                                          */
/*     hist -> The histogram to write the curve into                   */
/*     num_bins -> the number of bins in the histogram                 */
/*     middle -> the midpoint of the histogram                         */
/*     height -> the height of the histogram                           */
/*     width -> the widht of the histogram                             */
/***********************************************************************/
void special_cosine_hist(HIST **hist, int num_bins, int middle, int height, int width)
{
    int i;
    double factor, heightby2,c_fact=0.0,M_PI2=M_PI*2.0;

    factor = 1.0 / (double)(width);
    heightby2=height/2;
    for (i=middle-(width/2);i<=(middle+(width/2));i++){
        if ((i>=0) && (i<num_bins))
	    hist[i]->count = heightby2 +
                heightby2 * cos((double)(c_fact*M_PI2 - M_PI));
        c_fact+=factor;
    }
}


double pick_center(HIST **h, int bin)
{
  if ((bin < 0) || (bin >= BINS)) return 0.0;

  return ((double) (h[bin]->from+h[bin]->to)) / 2.0;
}
