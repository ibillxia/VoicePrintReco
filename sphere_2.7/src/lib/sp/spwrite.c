#include <stdio.h>
#define SPHERE_LIBRARY_CODE
#include <sp/sphere.h>
#include <sp/ulaw.h>
#include <sp/alaw.h>
#include <sp/shorten/shorten.h>
#include <string.h>

static int spw_file_init(SP_FILE *sp, char *call_proc);
static int spw_process_data(SP_FILE *sp, void *buffer, size_t num_sample, char *call_proc);
static void array2interleaved(char **arr_buff, SP_INTEGER arr_offset, char *inter_buff, SP_INTEGER chcnt, SP_INTEGER snb, SP_INTEGER samples);

void ulaw2pcm2(unsigned char *ulaw_data, short *pcm_data, enum SP_sample_encoding pcm_sbf, SP_INTEGER samples);
void alaw2pcm2(unsigned char *alaw_data, short *pcm_data, enum SP_sample_encoding pcm_sbf, SP_INTEGER samples);
void pculaw2pcm2(unsigned char *pculaw_data, short *pcm_data, enum SP_sample_encoding pcm_sbf, SP_INTEGER samples);


void pcm22ulaw(short *pcm_data, enum SP_sample_encoding pcm_sbf, unsigned char *ulaw_data, SP_INTEGER samples);
void pcm22alaw(short *pcm_data, enum SP_sample_encoding pcm_sbf, unsigned char *alaw_data, SP_INTEGER samples);
void pcm22pculaw(short *pcm_data, enum SP_sample_encoding pcm_sbf, unsigned char *pculaw_data, SP_INTEGER samples);

/*
 *
 *  sp_mc_write_data()
 *
 */

int sp_mc_write_data(void *buffer, size_t num_sample, SP_FILE *sp)
{
    char *proc="sp_write_data " SPHERE_VERSION_STR;

    if (sp_verbose > 10)
	fprintf(spfp,
	       "Proc %s: file %s, %d bytes/sample, %d channels, %ld samples\n",
	       proc,sp->write_spifr->status->external_filename,
	       sp->write_spifr->status->user_sample_n_bytes,
	       sp->write_spifr->status->user_channel_count,
		(long int) num_sample);
    if (buffer == (void *)0) 
	return_err(proc,100,0,"Null memory buffer"); 
    if (sp == SPNULL)
	return_err(proc,101,0,"Null SPFILE structure");
    if (sp->open_mode == SP_mode_read)
	return_err(proc,104,0,
		   "Unable to write data to a file opened for reading");
#ifdef isnotansi
    if (num_sample < 0)
	return_err(proc,103,0,
		   rsprintf("Negative sample count %d",num_sample));
#endif

    /*************************************************************************/
    /*     Perform all initializations to the sphere file to begin reading   */

    /* set up the FoB Structure for reading */
    if (sp->write_spifr->status->write_occured_flag == FALSE) 
	if (spw_file_init(sp,proc) != 0)
	    return_err(proc,100,0,get_subordinated_message());

    return_child(proc,int,spw_process_data(sp,buffer,num_sample,proc));
}

/* do any sample conversions, and write the data to the FOB */
static int spw_process_data(SP_FILE *sp, void *buffer, size_t num_sample, char *call_proc){
    char *proc = "spw_process_data";
    int samples_written = 0, ret;
    int out_samples=0;
    int block=0;
    void *out_block = (void *)0, *end_of_input_block=(void *)0;
    void *out_inter_block = (void *)0;
    SP_CHECKSUM checksum=0;
    SPIFR *spifr; 
    SPSTATUS *spstat; 
    SPWAVEFORM *spwav;

    spifr = sp->write_spifr;
    spstat = spifr->status;
    spwav = spifr->waveform;
    
    while (samples_written < num_sample){
	if (sp_verbose > 16)
	    fprintf(spfp,"Proc %s: Beginning Block %d\n",proc,block);
	
	/* if conversions are neccessary, restrict the size of the */
	/* data to that of the translation buffers   */
	if ((spstat->user_encoding != spstat->file_encoding) || 
	    (spstat->channels != CHANNELSNULL) ||
	    (spstat->user_sbf != spstat->file_sbf) ||
	    spstat->user_data_fmt == SP_df_array){
	    if ((out_samples = (num_sample - samples_written)) >
		TRANSLATION_LEN)
		out_samples = TRANSLATION_LEN;
	} else
	    out_samples = num_sample;
	
	/*  First, un-interleave the data if neccessary */
	if (spstat->user_data_fmt == SP_df_array){
	    /* convert the data from it's array form */
	    /* to an interleaved format              */
	    array2interleaved((char **)buffer,samples_written,
			      (char *)spwav->interleave_buffer,
			      spifr->status->user_channel_count,
			      spifr->status->user_sample_n_bytes,
			      out_samples);

	    out_inter_block = (void *)spwav->interleave_buffer;
	} else { /* use the buffer passed to the function */
	    if (out_inter_block == (void *)0)
		out_inter_block = (void *)buffer;
	    else
		out_inter_block = end_of_input_block;
	    end_of_input_block = 
		(void *)((char *)out_inter_block + 
			 (out_samples * spifr->status->user_sample_n_bytes *
			  spifr->status->user_channel_count));
	}
	
	/*  Next do the sample encoding conversions if necessary */
	if ((spstat->user_encoding == SP_se_ulaw) &&
	    (spstat->file_encoding == SP_se_pcm2)) {

	    if (sp_verbose > 15)
		fprintf(spfp,"Proc %s: converting ULAW to PCM\n",proc);

	    ulaw2pcm2((unsigned char *)out_inter_block,
		     (short *)spwav->converted_buffer,
		     spstat->file_sbf,
		     out_samples*spstat->user_channel_count);

	    out_block = (void *)spwav->converted_buffer;
	} else if ((spstat->user_encoding == SP_se_pcm2) &&
		   (spstat->file_encoding == SP_se_ulaw)) {
	    if (sp_verbose > 15)
		fprintf(spfp,"Proc %s: converting PCM to ULAW\n",proc);       

	    pcm22ulaw((short *)out_inter_block,
		      spstat->user_sbf,
		      (unsigned char *)spwav->converted_buffer,
		      out_samples*spstat->user_channel_count);
	
	    out_block = (void *)spwav->converted_buffer;
	} else if ((spstat->user_encoding == SP_se_pculaw) &&
		   (spstat->file_encoding == SP_se_pcm2)) {
	    if (sp_verbose > 15)
		fprintf(spfp,"Proc %s: converting PCULAW to PCM\n",proc);
	    
	    pculaw2pcm2((unsigned char *)out_inter_block,
			(short *)spwav->converted_buffer,
			spstat->file_sbf,
			out_samples*spstat->user_channel_count);
	    
	    out_block = (void *)spwav->converted_buffer;
	} else if ((spstat->user_encoding == SP_se_pcm2) &&
		   (spstat->file_encoding == SP_se_pculaw)) {
	    if (sp_verbose > 15)
		fprintf(spfp,"Proc %s: converting PCULAW to PCM\n",proc);
	    
	    pcm22pculaw((short *)out_inter_block,
			spstat->user_sbf,
			(unsigned char *)spwav->converted_buffer,
			out_samples*spstat->user_channel_count);
	    
	    out_block = (void *)spwav->converted_buffer;
	} else if ((spstat->user_encoding == SP_se_alaw) &&
		   (spstat->file_encoding == SP_se_pcm2)) {
	    if (sp_verbose > 15)
		fprintf(spfp,"Proc %s: converting ALAW to PCM\n",proc);
	    
	    alaw2pcm2((unsigned char *)out_inter_block,
		     (short *)spwav->converted_buffer,
		     spstat->file_sbf,
		     out_samples*spstat->user_channel_count);

	    out_block = (void *)spwav->converted_buffer;
	} else if ((spstat->user_encoding == SP_se_pcm2) &&
		   (spstat->file_encoding == SP_se_alaw)) {
	    if (sp_verbose > 15)
		fprintf(spfp,"Proc %s: converting ALAW to PCM\n",proc);

	    pcm22alaw((short *)out_inter_block,
		      spstat->user_sbf,
		      (unsigned char *)spwav->converted_buffer,
		      out_samples*spstat->user_channel_count);
	    
	    out_block = (void *)spwav->converted_buffer;
	} else if (((spstat->user_encoding == SP_se_pcm2) &&
		    (spstat->file_encoding == SP_se_pcm2)) &&
		   (spstat->user_sbf != spstat->file_sbf)){
	    /* swap them bytes now */
	    char *tp = (char *)spwav->byteswap_buffer;
	    char *p=(char *)out_inter_block;

	    int nn = out_samples*spstat->user_channel_count, n;
	    for (n = 0; n < nn; n++, p+=2, tp+=2){
		*tp= *(p+1); *(tp+1) = *p;
	    }
	    out_block = spwav->byteswap_buffer;
	} else { /* use the buffer passed to the function */
	    out_block = out_inter_block;
	}
	
	/*****************************************************/
	/****             INVARIANT ASSERTION:            ****/
	/****  out_block points to the file_encoded data  ****/
	/****  and file_sbf, but the same channel count   ****/
	/****  as the user's input.                       ****/
	/*****************************************************/
	
	/* do the channel modifications */
	if (spstat->channels != CHANNELSNULL){	
	    int in_recsize, out_recsize, in_chancnt, out_chancnt, out_snb;
	    int oc, s;
	    void *eod;
	    
	    if (sp_verbose > 15)
		fprintf(spfp,"Proc %s: Converting channels\n",proc);
	    
	    in_chancnt = spstat->user_channel_count;
	    in_recsize = spstat->file_sample_n_bytes *  in_chancnt;
	    out_chancnt = spstat->file_channel_count;
	    out_snb = spstat->file_sample_n_bytes;
	    out_recsize = spstat->file_sample_n_bytes * out_chancnt;
	    
	    /*  process each requested channel */
	    for (oc=0; oc<spifr->status->channels->num_chan; oc++)
		if (spifr->status->channels->ochan[oc].num_origin == 1){
		    char *in_ptr, *out_ptr;
		    in_ptr = (char *)out_block + 
			((spstat->channels->ochan[oc].orig_channel[0]-1)*
			 spstat->file_sample_n_bytes);
		    out_ptr = (char *)spwav->file_data_buffer + 
			(oc * spifr->status->file_sample_n_bytes);
		    /*
		       fprintf(spfp,
		       " %x  %x  %x  %x\n",in_ptr,in_conversion_block,
		       out_ptr , out_block);
		       fprintf(spfp,"  %d   %d\n",in_recsize, out_recsize);
		       */
		    
		    for (s=0; s<out_samples; s++){
			memcpy(out_ptr,in_ptr,out_snb);
			in_ptr += in_recsize;
			out_ptr += out_recsize;
		    }
		    in_ptr-=((spstat->channels->ochan[oc].orig_channel[0]-1)*
			     spstat->file_sample_n_bytes);
		    eod = (void *)in_ptr;
		} else { /* Channels need to be added together */
		    if (spstat->file_encoding == SP_se_pcm2) {
			/* do a straight add */
			ORIGINATION_CHAN *ochn;
			short *in_ptr, *out_ptr;
			int sum, ch;
			if (sp_verbose > 15)
			    fprintf(spfp,
				    "Proc %s: Adding PCM data %s %d\n",
				    proc, "channel",oc);
			in_ptr = (short *)out_block;
			out_ptr = (short *)spwav->file_data_buffer;
			ochn = &(spstat->channels->ochan[oc]);
			
			if (spstat->file_sbf== get_natural_sbf(2)){
			    for (s=0; s<out_samples; s++){
				for (ch=0, sum=0; ch <ochn->num_origin;ch++){
				    sum +=*(in_ptr+ochn->orig_channel[ch]-1);
				}
				if (sum > 32767) sum = 32767;
				else if (sum < -32768) sum = -32768;
				*(out_ptr + oc) = (short)sum;
				in_ptr += in_chancnt;
				out_ptr += out_chancnt;
			    }
			    eod = in_ptr;
			} else {
			    short x;
			    if (sp_verbose > 15)
				fprintf(spfp,"Proc %s: %s %d\n", proc,
					"Adding Swapped PCM data channel",oc);
			    for (s=0; s<out_samples; s++){
				/*				   fprintf(spfp,"sample %d\n",s);  */
				for (ch=0, sum=0; ch <ochn->num_origin;ch++){
				    x = *(in_ptr+ochn->orig_channel[ch]-1);
				    swap_bytes(x);
				    sum += x;
				    /*				       fprintf(spfp,"   %d  =  %d\n",x,sum); */
				}
				if (sum > 32767) x = 32767;
				else if (sum < -32768) x = -32768;
				else x = sum;
				swap_bytes(x);
				*(out_ptr + oc) = (short)x;
				in_ptr += in_chancnt;
				out_ptr += out_chancnt;
			    }
			    eod = in_ptr;
			}
		    } else if (spifr->status->file_encoding == SP_se_ulaw){
			/* do an add on ulaw data */
			ORIGINATION_CHAN *ochn;
			unsigned char *in_ptr, *out_ptr;
			int sum, ch;
			if (sp_verbose > 15)
			    fprintf(spfp,
				    "Proc %s: Adding ULAW data %s %d\n",
				    proc, "channel",oc);
			in_ptr = (unsigned char *)out_block;
			out_ptr = (unsigned char *)spwav->file_data_buffer;
			ochn = &(spifr->status->channels->ochan[oc]);
			
			for (s=0; s<out_samples; s++){
			    for (ch=0, sum=0; ch <ochn->num_origin;ch++){
				sum +=
				    ulaw2linear_10_sbf_lut[
							   *(in_ptr+ochn->orig_channel[ch]-1)];
			    }
			    if (sum > 32767) sum = 32767;
			    else if (sum < -32768) sum = -32768;
			    *(out_ptr + oc) =  linear2ulaw(sum);
			    in_ptr += in_chancnt;
			    out_ptr += out_chancnt;
			}
			eod = in_ptr;
		    } else if (spifr->status->file_encoding == SP_se_pculaw){
			/* do an add on ulaw data */
			ORIGINATION_CHAN *ochn;
			unsigned char *in_ptr, *out_ptr;
			int sum, ch;
			if (sp_verbose > 15)
			    fprintf(spfp,
				    "Proc %s: Adding PCULAW data %s %d\n",
				    proc, "channel",oc);
			in_ptr = (unsigned char *)out_block;
			out_ptr = (unsigned char *)spwav->file_data_buffer;
			ochn = &(spifr->status->channels->ochan[oc]);
			
			for (s=0; s<out_samples; s++){
			    for (ch=0, sum=0; ch <ochn->num_origin;ch++){
				sum +=
				    ulaw2linear_10_sbf_lut[
							   uchar_bitreverse_lut[
										*(in_ptr+ochn->orig_channel[ch]-1)]];
			    }
			    if (sum > 32767) sum = 32767;
			    else if (sum < -32768) sum = -32768;
			    *(out_ptr + oc) = uchar_bitreverse_lut[
								   linear2ulaw(sum)];
			    in_ptr += in_chancnt;
			    out_ptr += out_chancnt;
			}
			eod = in_ptr;
		    } else if (spifr->status->file_encoding == SP_se_alaw){
			/* do an add on alaw data */
			ORIGINATION_CHAN *ochn;
			unsigned char *in_ptr, *out_ptr;
			int sum, ch;
			if (sp_verbose > 15)
			    fprintf(spfp,
				    "Proc %s: Adding ALAW data %s %d\n",
				    proc, "channel",oc);
			in_ptr = (unsigned char *)out_block;
			out_ptr = (unsigned char *)spwav->file_data_buffer;
			ochn = &(spifr->status->channels->ochan[oc]);
			
			for (s=0; s<out_samples; s++){
			    for (ch=0, sum=0; ch <ochn->num_origin;ch++){
				sum +=
				    alaw2linear_10_sbf_lut[
							   *(in_ptr+ochn->orig_channel[ch]-1)];
			    }
			    if (sum > 32767) sum = 32767;
			    else if (sum < -32768) sum = -32768;
			    *(out_ptr + oc) =  linear2alaw(sum);
			    in_ptr += in_chancnt;
			    out_ptr += out_chancnt;
			}
			eod = in_ptr;
		    }
		}
	    /**** in_ptr MUST point to the next input block of data ***/
	    
	    out_block = spwav->file_data_buffer;
	}
	
	ret = fob_fwrite(out_block, spstat->file_sample_n_bytes*
			 spstat->file_channel_count,
			 out_samples, spwav->sp_fob);
	if (ret != out_samples){
	    return_err(proc,306,0,
		       rsprintf("Unable to write data %d written, not %d",
				ret,out_samples));
	}	   
	samples_written += ret;
	
	/* Perform the checksum computation */
	switch (spstat->file_encoding){
	  case SP_se_pcm2:
	    if (spstat->file_sbf != get_natural_sbf(2)){
		if (sp_verbose > 16)
		    fprintf(spfp,"Proc %s: Swapping for checksum\n",proc);
		checksum = sp_compute_short_checksum(out_block, out_samples*
						     spstat->file_channel_count, TRUE);
	    } else {
		if (sp_verbose > 16)
		    fprintf(spfp,
			    "Proc %s: Not Swapping for checksum\n",proc);
		checksum = sp_compute_short_checksum(out_block, out_samples*
						     spstat->file_channel_count, FALSE);
	    }
	    
	    if (sp_verbose > 16)
		fprintf(spfp,"Proc %s: short checksum = %d\n",proc,checksum);
	    spwav->checksum =
		sp_add_checksum(spwav->checksum,checksum);
	    spwav->samples_written += ret;
	    break;
	  default: {
	      int n;
	      n =  out_samples * spstat->file_channel_count *
		  spstat->file_sample_n_bytes;
	      if (sp_verbose > 16)
		  fprintf(spfp,"Proc %s: Computing char checksum %d bytes\n",
			  proc,n);
	      checksum = sp_compute_char_checksum(out_block,n);
	      spwav->checksum = 
		  sp_add_checksum(spwav->checksum,checksum);
	      spwav->samples_written += ret;
	  }
	}
	
	if (sp_verbose > 11) 
	    fprintf(spfp,"Proc %s: Block %d: Requested %ld samples, written %d total %d, Checksum %d (BC %d)\n",
		    proc,block++, (long int) num_sample,ret,
		    spwav->samples_written,
		    checksum,spwav->checksum);
    }
    return_success(proc,0,samples_written,"ok");
}


static int spw_file_init(SP_FILE *sp, char *call_proc){
    char proc[40], *str;
    SP_INTEGER l_int, h_size;
    int ret;
    SPIFR *spifr; 
    SPSTATUS *spstat; 
    SPWAVEFORM *spwav;

    sprintf(proc,"%s:%s",call_proc,"spw_file_init");

    spifr = sp->write_spifr;
    spstat = spifr->status;
    spwav = spifr->waveform;

    /******************************************************/
    /*             Check for required fields              */
    
    if (sp_verbose > 15) fprintf(spfp,"Proc %s: initializing FOB\n",proc);
    
    /* the sample_n_bytes is required, if it is not set, there is an    */
    /* error */
    if (h_get_field(spstat->file_header,
		    SAMPLE_N_BYTES_FIELD, T_INTEGER, (void *)&l_int) != 0)
	return_err(proc,150,100,
		   rsprintf("Header field '%s' is missing",
			    SAMPLE_N_BYTES_FIELD));
    
    /* if 'channel_count' is not in the header, there is an error */
    if (h_get_field(spstat->file_header,
		    CHANNEL_COUNT_FIELD, T_INTEGER, (void *)&l_int) != 0)
	return_err(proc,151,100,
		   rsprintf("Header field '%s' is missing",
			    CHANNEL_COUNT_FIELD));
    
    /*   This constraint was relaxed to allow for streams to not */
    /*   have a sample_count field.  Added June 22, 1994, JGF    */
    /* if (! spstat->is_disk_file) 
       if (h_get_field(spstat->file_header,
       SAMPLE_COUNT_FIELD, T_INTEGER, (void *)&l_int) !=0)
       return_err(proc,151,100,
       rsprintf("Header field '%s' is missing%s",
       SAMPLE_COUNT_FIELD," for stream file"));*/
    
    /* if the sample coding field is not 'raw' the sample_rate fielda   */
    /* must exist if the field is missing, it is assumed to be a pcm file*/
    ret=h_get_field(spstat->file_header,
		    SAMPLE_CODING_FIELD, T_STRING, (void *)&str);
    if (((ret == 0) && (!strsame(str,"raw"))) || (ret != 0))
	if (h_get_field(spstat->file_header,
			SAMPLE_RATE_FIELD, T_INTEGER, (void *)&l_int) !=0){
	    if (ret == 0) mtrf_free(str);
	    return_err(proc,151,100,
		       rsprintf("Header field '%s' is missing %s",
				SAMPLE_RATE_FIELD,"from wave type file"));
	}
    if (ret == 0) mtrf_free(str);
    
    /* if the following fields are missing from the header, default      */
    /* values need to be assumed in the SP_FILE status structure         */
    if (h_get_field(spstat->file_header,
		    SAMPLE_BF_FIELD, T_STRING, (void *)&str) == 0)
	mtrf_free(str);
    else {
	spstat->user_sbf =
	    spstat->file_sbf =
		get_natural_sbf(spstat->file_sample_n_bytes);
    }
    
    
    /* only add data to the header if the file is not a stream */
    if (spstat->is_disk_file) {
	/* if 'sample_count' is not in the header, add it to take up    */
	/* space for later correction                                   */
	if (h_get_field(spstat->file_header, 
			SAMPLE_COUNT_FIELD, T_INTEGER, (void *)&l_int)!=0){
	    l_int=(-1);
	    
	    spstat->file_sample_count=l_int;
	    sp_h_set_field(sp, SAMPLE_COUNT_FIELD, T_INTEGER, &l_int);
	}
	
	/* if 'sample_checksum' is not in the header, add it with a fake */
	/* checksum */
	if (h_get_field(spstat->file_header,SAMPLE_CHECKSUM_FIELD,
			T_INTEGER,(void *)&l_int)!=0){
	    l_int=999999999;
	    spstat->file_checksum=l_int;
	    sp_h_set_field(sp, SAMPLE_CHECKSUM_FIELD, T_INTEGER, &l_int);
	}
    }
    
    /* bug fixed JGF 060496, make sure the header size is saved */
    /* Flush the header to the file pointer */
    if (sp_write_header(spwav->sp_fp, spstat->file_header,
			&(spwav->header_data_size),
			&h_size) < 0)
	return_err(proc,200,100,"Unable to write header to file");
    
    if ((spstat->user_compress == SP_wc_none) &&
	(spstat->file_compress != SP_wc_none)){
	char *buff;
	int blen;
	
	/* The file needs to be written into a temporary place, and then */
	/* compressed.  If The expected waveform size is bigger that     */
	/* MAX_INTERNAL_WAVEFORM, use a temporary file. otherwise:       */
	/* 1. make an MEMORY FOB struct for the uncompressed file.       */
	/* 2. allocate the memory for the FOB struct,                    */
	/* 3. install the memory into the FOB struct                     */
	
	if (spstat->file_sample_count < 0)
	    /* allocate a minimal size for the waveform */
	    blen = MAX_INTERNAL_WAVFORM;
	else
	    blen = spstat->file_channel_count *spstat->file_sample_count * 
		spstat->file_sample_n_bytes;
	
	if (blen < MAX_INTERNAL_WAVFORM){
	    if ((spwav->sp_fob = 
		 fob_create(FPNULL)) == FOBPNULL)
		return_err(proc,300,100,
			   "Unable to allocate a FOB structure");
	    if ((buff = mtrf_malloc(blen)) == CNULL){
		fob_destroy(spwav->sp_fob);
		return_err(proc,301,100,
			   "Unable to malloc buffer for waveform data");
	    }
	    fob_bufinit(spwav->sp_fob, buff, blen);
	} else {
	    FILE *temp_fp;
	    spstat->temp_filename = sptemp_dirfile();
	    if (spstat->temp_filename == CNULL)
		return_err(proc,301,100,
			   "Unable to create usable temporary file");
	    if (sp_verbose > 15)
		fprintf(spfp,
			"Proc %s: Attempting to write a big file %d%s%s\n",
			proc,blen," bytes long, using temp file ",
			spstat->temp_filename);
	    if ((temp_fp=fopen(spstat->temp_filename,
			       TRUNCATE_UPDATEMODE)) == FPNULL) 
		return_err(proc,302,100,
			   rsprintf("Unable to open temporary file %s",
				    spstat->temp_filename));
	    if ((spwav->sp_fob = fob_create(temp_fp)) == FOBPNULL)
		return_err(proc,303,100,
			   "Unable to allocate a FOB structure");
	    spstat->is_temp_file = TRUE;
	}
    }
    else {
	/* This assumes that no pre-buffering is required */
	/*	    fprintf(spfp,"Setting up for un-buffered IO \n");*/
	if ((spwav->sp_fob = fob_create(spwav->sp_fp)) == FOBPNULL)
	    return_err(proc,300,100,
		       "Unable to allocate a FOB structure");
	spwav->sp_fp = FPNULL;
    }

    /* allocate a translation buffer for the sample encodings */
    if (spstat->user_encoding != spstat->file_encoding) {
	if (((spstat->user_encoding == SP_se_ulaw ||
	      spstat->user_encoding == SP_se_pculaw ||
	      spstat->user_encoding == SP_se_alaw) &&
	     (spstat->file_encoding == SP_se_pcm2)) ||
	    ((spstat->user_encoding == SP_se_pcm2) &&
	     (spstat->file_encoding == SP_se_ulaw || 
	      spstat->file_encoding == SP_se_pculaw || 
	      spstat->file_encoding == SP_se_alaw))) {
	    
	    /* allocate the memory for the translation buffer */
	    /*  IF it's a legal transformation                */
	    spwav->converted_buffer_len = TRANSLATION_LEN *
		spstat->file_sample_n_bytes *
		    spstat->user_channel_count;
	    if (sp_verbose > 15)
		fprintf(spfp,"Proc %s: Alloc %d (%d*%d*%d) bytes for%s\n",
			proc, spwav->converted_buffer_len,
			TRANSLATION_LEN, spstat->file_sample_n_bytes,
			spstat->user_channel_count, " encoding buffer");
	    if ((spwav->converted_buffer = 
		 (void *)mtrf_malloc(spwav->converted_buffer_len)) ==
		(void *)0)
		return_err(proc,500,100,
			   "Unable to alloc memory for the encoding buffer");
	    
	} else {
	    return_err(proc,400,100,
		       "Unable to convert sample types ... for now\n");
	}
    }
    /* allocate a buffer for the channel conversions */
    if (spstat->channels != CHANNELSNULL){
	/* allocate the memory for the translation buffer */
	spwav->file_data_buffer_len = TRANSLATION_LEN *
	    spstat->file_sample_n_bytes * spstat->file_channel_count;
	if (sp_verbose > 15)
	    fprintf(spfp,"Proc %s: Alloc %d (%d*%d*%d) bytes for%s\n",
		    proc, spwav->file_data_buffer_len,
		    TRANSLATION_LEN, spstat->file_sample_n_bytes,
		    spstat->file_channel_count, " channel mod buffer");
	if ((spwav->file_data_buffer = 
	     (void *)mtrf_malloc(spwav->file_data_buffer_len)) ==(void *)0)
	    return_err(proc,501,100,
		       "Unable to alloc memory for the channel mod buffer");
    }
    /* allocate a buffer for the interleaved version of incomming data */
    if (spstat->user_data_fmt == SP_df_array){
	/* allocate the memory for the interleaved buffer */
	spwav->interleave_buffer_len = TRANSLATION_LEN *
	    spstat->user_sample_n_bytes * spstat->user_channel_count;
	if (sp_verbose > 15)
	    fprintf(spfp,"Proc %s: Alloc %d (%d*%d*%d) bytes for%s\n",
		    proc, spwav->interleave_buffer_len,
		    TRANSLATION_LEN, spstat->user_sample_n_bytes,
		    spstat->user_channel_count, " interleave buffer");
	if ((spwav->interleave_buffer = 
	     (void *)mtrf_malloc(spwav->interleave_buffer_len)) ==(void *)0)
	    return_err(proc,501,100,
		       "Unable to alloc memory for the interleave buffer");
    }
    /* allocate a buffer for byte swapping pcm data */
    if (((spstat->user_encoding == SP_se_pcm2) &&
	 (spstat->file_encoding == SP_se_pcm2)) &&
	(spstat->user_sbf != spstat->file_sbf)){
	/* allocate the memory for the byte swapping buffer */
	spwav->byteswap_buffer_len = TRANSLATION_LEN *
	    spstat->user_sample_n_bytes * spstat->user_channel_count;
	if (sp_verbose > 15)
	    fprintf(spfp,"Proc %s: Alloc %d (%d*%d*%d) bytes for%s\n",
		    proc, spwav->byteswap_buffer_len,
		    TRANSLATION_LEN, spstat->user_sample_n_bytes,
		    spstat->user_channel_count, " byte swapping buffer");
	if ((spwav->byteswap_buffer = 
	     (void *)mtrf_malloc(spwav->byteswap_buffer_len)) ==(void *)0)
	    return_err(proc,501,100,
		       "Unable to alloc memory for the byteswap buffer");
    }
    
    spstat->write_occured_flag = TRUE;
    if (sp_verbose > 15) fprintf(spfp,"Proc %s: initialization complete\n",
				 proc);

    return_success(proc,0,0,"ok");
}

static void array2interleaved(char **arr_buff, SP_INTEGER arr_offset, char *inter_buff, SP_INTEGER chcnt, SP_INTEGER snb, SP_INTEGER samples){
    char *proc = "array2interleaved";
    int recsize = snb * chcnt, c, s;
    char *out_dat;
    char *arr;
	    
    if (sp_verbose > 15)
	fprintf(spfp,"Proc %s: UN-Interleaving the data\n",proc);
    
    for (c=0; c<chcnt; c++){
	out_dat = inter_buff + (snb * c) ;
	arr = arr_buff[c] + (arr_offset * snb);
	
	for (s=0; s<samples; s++,arr+=snb,out_dat+=recsize){
	    memcpy(out_dat,arr,snb);
	}
    }
}

void ulaw2pcm2(unsigned char *ulaw_data, short *pcm_data, enum SP_sample_encoding pcm_sbf, SP_INTEGER samples){
    int o;

    if (pcm_sbf != get_natural_sbf(2))
	for (o=0; o < samples; o++) {
	    *pcm_data = ulaw2linear_01_sbf_lut[*ulaw_data];
	    pcm_data ++; ulaw_data ++;
	}
    else
	for (o=0; o < samples; o++) {
	    *pcm_data = ulaw2linear_10_sbf_lut[*ulaw_data];
	    pcm_data ++; ulaw_data ++;
	}
}    

void alaw2pcm2(unsigned char *alaw_data, short *pcm_data, enum SP_sample_encoding pcm_sbf, SP_INTEGER samples){
    int o;

    if (pcm_sbf != get_natural_sbf(2))
	for (o=0; o < samples; o++) {
	    *pcm_data = alaw2linear_01_sbf_lut[*alaw_data];
	    pcm_data ++; alaw_data ++;
	}
    else
	for (o=0; o < samples; o++) {
	    *pcm_data = alaw2linear_10_sbf_lut[*alaw_data];
	    pcm_data ++; alaw_data ++;
	}
}    

void pculaw2pcm2(unsigned char *pculaw_data, short *pcm_data, enum SP_sample_encoding pcm_sbf, SP_INTEGER samples){
    int o;

    if (pcm_sbf != get_natural_sbf(2))
	for (o=0; o < samples; o++) {
	    *pcm_data = ulaw2linear_01_sbf_lut[uchar_bitreverse_lut[*pculaw_data]];
	    pcm_data ++; pculaw_data ++;
	}
    else
	for (o=0; o < samples; o++) {
	    *pcm_data = ulaw2linear_10_sbf_lut[uchar_bitreverse_lut[*pculaw_data]];
	    pcm_data ++; pculaw_data ++;
	}
}    

void pcm22ulaw(short *pcm_data, enum SP_sample_encoding pcm_sbf, unsigned char *ulaw_data, SP_INTEGER samples){
    int o;

    if (pcm_sbf != get_natural_sbf(2)){
	for (o=0; o < samples; o++) {
	    /* Pre swap the bytes */
	    char *p , temp; short odata;
	    
	    odata = *pcm_data;
	    p = (char *) &odata;
	    temp = *p;
	    *p = *(p + 1);
	    *(p + 1) = temp;
	    
	    *ulaw_data = linear2ulaw(odata);
	    pcm_data ++; ulaw_data ++;
	}
    } else
	for (o=0; o < samples; o++) {
	    *ulaw_data = linear2ulaw(*pcm_data);
	    /*		       if(o<30){
			       fprintf(spfp,"  conv %6d-%4x    %4d-%2x",
			       *pcm_data,*pcm_data,
			       *ulaw_data, *ulaw_data);
			       if ((o+1) % 4 == 0) fprintf(spfp,"\n");
			       }*/
	    pcm_data ++; ulaw_data ++;
	}
}

void pcm22alaw(short *pcm_data, enum SP_sample_encoding pcm_sbf, unsigned char *alaw_data, SP_INTEGER samples){
    int o;

    if (pcm_sbf != get_natural_sbf(2)){
	for (o=0; o < samples; o++) {
	    /* Pre swap the bytes */
	    char *p , temp; short odata;
	    
	    odata = *pcm_data;
	    p = (char *) &odata;
	    temp = *p;
	    *p = *(p + 1);
	    *(p + 1) = temp;
	    
	    *alaw_data = linear2alaw(odata);
	    pcm_data ++; alaw_data ++;
	}
    } else
	for (o=0; o < samples; o++) {
	    *alaw_data = linear2alaw(*pcm_data);
	    pcm_data ++; alaw_data ++;
	}
}

void pcm22pculaw(short *pcm_data, enum SP_sample_encoding pcm_sbf, unsigned char *pculaw_data, SP_INTEGER samples){
    int o;

    if (pcm_sbf != get_natural_sbf(2)){
	for (o=0; o < samples; o++) {
	    /* Pre swap the bytes */
	    char *p , temp; short odata;
	    
	    odata = *pcm_data;
	    p = (char *) &odata;
	    temp = *p;
	    *p = *(p + 1);
	    *(p + 1) = temp;
	    
	    *pculaw_data = uchar_bitreverse_lut[linear2ulaw(odata)];
	    pcm_data ++; pculaw_data ++;
	}
    } else
	for (o=0; o < samples; o++) {
	    *pculaw_data = uchar_bitreverse_lut[linear2ulaw(*pcm_data)];
	    pcm_data ++; pculaw_data ++;
	}
}
