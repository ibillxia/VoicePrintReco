
#include <snr/snrlib.h>

#define MILI_SEC 20.0
#ifndef PI
#define PI 3.14159265358979
#endif
#define BLOCKSIZE 2048


int concatenate_speech(SP_FILE *in_file, int **list, int segments, SP_FILE *out_file)
{
    char *proc="concatenate_speech";
    short *buffer;
    int half_window,window,frame,current_segment;
    SP_INTEGER srate;
    
    if (sp_rewind(in_file) > 0)
	fprintf(spfp,"Warning: sp_rewind failed\n");
    if (sp_h_get_field(in_file, "sample_rate", T_INTEGER,(void *) &srate)){
	fprintf(spfp,"Error: unable to get sample_rate field\n");
	sp_print_return_status(spfp);  return(-1);
    }

    if (sp_copy_header(in_file,out_file) != 0){
	fprintf(stderr,"%s: Unable to copy header from original file\n",proc);
	return(-1);
    }
    if (sp_h_delete_field(out_file,"sample_count") > 100 ||
	sp_h_delete_field(out_file,"sample_checksum") > 100){
	fprintf(stderr,
		"%s: Unable to delete sample_count or sample_checksum\n",proc);
	return(-1);
    }
	       	
    /* calculate buffer size we need according to sample rate */
    window = ROUND((double)srate*MILI_SEC/1000.0);
    if (MOD(window,2)==1) window +=1;
    half_window = window/2;

    if ((buffer=(short *)sp_data_alloc(in_file,window)) == (short *)0){
	fprintf(spfp,"Unable to malloc memory for wav buffer\n");
	exit(-1);
    }
    
    frame=0;
    current_segment=0;

    while (sp_read_data(buffer,half_window,in_file) == half_window &&
	   (current_segment < segments)) {
	if ((frame >= list[current_segment][FROM]) &&
	    (frame <= list[current_segment][TO])) 
	    /* if we're in the speech zone, */
	    /* write it to disk. */
	    sp_write_data((char *)buffer,half_window,out_file);
	if (frame > list[current_segment][TO]) current_segment++;
	frame++;
    }

    if (sp_rewind(in_file) > 0)
	fprintf(spfp,"Warning: sp_rewind failed\n");
    sp_data_free(in_file,buffer);
    return(0);
}
    
int segment_speech(SP_FILE *sp, double speech, double noise, int *max_segment, 
		   int ***list)  /* returns the number of segments. */
{
    char *proc = "segment_speech";
    short *buffer;
    int window,half_window,start,frame,is_speaking,segment;
    int i;
    double start_low,start_high,end_low,end_high,r0;
    double offset;
    int state,old_state;
    SP_INTEGER srate;

    if (sp_rewind(sp) > 0)
	fprintf(spfp,"Warning: sp_rewind failed\n");
    if (sp_h_get_field(sp, "sample_rate", T_INTEGER,(void *) &srate)){
	fprintf(spfp,"%s: unable to get sample_rate field\n",proc);
	sp_print_return_status(spfp);  return(-1);
    }
    
    /* calculate buffer size we need according to sample rate */
    window = window_size(srate,MILI_SEC);
    half_window = window/2;
    
    /* allocate appropriate buffer */
    buffer = sp_data_alloc(sp,window*sizeof(short));
    for (i=0; i<window; i++) buffer[i]=0;
    
    offset = dc_offset(sp);
    
    start=0;
    frame=0;
    is_speaking=0;
    segment=0;
    
    if ((speech-noise) > 30.0)
	start_low=start_high=end_low=end_high=(speech-noise)*0.4 + noise;
    else if ((speech-noise) > 20)
	start_low=start_high=end_low=end_high=(speech-noise)*0.2 + noise;
    else if ((speech-noise) > 10)
	start_low=start_high=end_low=end_high=(speech-noise)*0.1 + noise;
    else 
	start_low=start_high=end_low=end_high=(speech-noise)*0.0 + noise;

    old_state=SD_INIT_SIL; /* assume we start in silence */
    set_speech_params(start_low,start_high,end_low,end_high);
    init_speech_detect(0); /* initialize at state 0 */
    
    while (sp_read_data(buffer+start,half_window,sp) == half_window){
	r0=rms_power(buffer,start,window,offset);
	r0= (r0==0) ? -9999.9 : 10.0*log10(r0); /* convert to dB */
	old_state=state;
	state=do_speech_detect(r0,frame);
	if (old_state != state) {
	    switch (state) {
	      case SD_SPEAKING:
		if (!is_speaking) {
		    (*list)[segment][FROM]=speech_begin();
		    is_speaking=1;
		}
		break;
	      case SD_DONE:
		(*list)[segment++][TO]=speech_end();
		if (segment >= *max_segment){
		    int foo1 = 2, foo2 = 2;
		    int **tlist = *list;		    
		    expand_2dimZ(tlist,segment,*max_segment,1.5,
				 foo1,foo2,1.0,int,0,1) ;
		    *list = tlist;
		}
		is_speaking=0;
		break;
	    }
	}
	start = (start==0) ? half_window : 0;
	frame++;
    }
    
    if (is_speaking) (*list)[segment++][TO] = frame-1;
    
    if (sp_rewind(sp) > 0)
	fprintf(spfp,"Warning: sp_rewind failed\n");
    
    sp_data_free(sp,buffer);
    return segment;
}


void hamming_window(short int *data, int size, int start)
{
  int i,index;

  for (i=start; i<start+size; i++) {

    index = MOD(i,size); /* circular buffer */
    data[index] = (short) ((double) data[index]) *
      (0.54 - 0.46*cos(2.0*PI*((double)index)/((double)size - 1.0)));
  }
}



double rms_power (short int *buffer, int start, int size, double offset)  /* includes hamming window */
{
  int i,index;
  double sum=0.0,datum;

  for (i=start; i<start+size; i++) {
    index = MOD(i,size); /* circular buffer */
    datum = (double) buffer[index] - offset;
    datum *= (0.54 - 0.46*cos(2.0*PI*((double)index)/((double)size - 1.0)));
    sum += datum*datum;
  }

  return sum / (double) size;
}

int window_size(int sample_rate, double miliseconds)
/* returns an even-sized window length corresponding */
/* to the number of msec desired. */
{
  int window;

  window = ROUND((double)sample_rate*miliseconds/1000.0);
  if (MOD(window,2)==1) window +=1;
  return window;
}
 

double dc_offset (SP_FILE *sp)
{
    int read,total_read=0,i;
    short buffer[BLOCKSIZE];
    double sum=0.0;
    
    if (sp_rewind(sp) > 0)
	fprintf(spfp,"Warning: sp_rewind failed\n");
    
    while (sp_read_data(buffer,BLOCKSIZE,sp) > BLOCKSIZE){
	total_read += read;
	for (i=0; i< BLOCKSIZE; i++) 
	    sum += (double) buffer[i];
    }
    
    if (sp_rewind(sp) > 0)
	fprintf(spfp,"Warning: sp_rewind failed\n");
    
    return (total_read==0) ? 0.0 : sum / (double) total_read;
}
