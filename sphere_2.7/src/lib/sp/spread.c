#include <stdio.h>
#include <sp/sphere.h>
#include <sp/ulaw.h>
#include <sp/alaw.h>
#include <sp/shorten/shorten.h>
#include <string.h>
#include <setjmp.h>

extern jmp_buf	exitenv;

static int fob_short_checksum(FOB *f, SP_CHECKSUM *checksum, int do_byte_swap, 
		      SP_CHECKSUM (*add_checksum) (SP_CHECKSUM,SP_CHECKSUM));
static int fob_char_checksum(FOB *, SP_CHECKSUM *,
	      SP_CHECKSUM (*add_checksum) (SP_CHECKSUM , SP_CHECKSUM ));
static int decompress_waveform(SP_FILE *);
static int pre_verify_checksum(SP_FILE *);
static int read_data_in(void *, size_t, SP_FILE *);

/*
 *
 *  sp_read_data()
 *
 */
int sp_mc_read_data(void *buffer, size_t num_sample, SP_FILE *sp)
{
    char *proc="sp_read_data " SPHERE_VERSION_STR;
    SPIFR *spifr;
    int ret;
    int do_conversions, do_channel_selections;

    if (buffer == (void *)0) 
	return_err(proc,100,0,"Null memory buffer"); 
    if (sp == SPNULL)
	return_err(proc,101,0,"Null SPFILE structure");
    if (sp->open_mode != SP_mode_read) 
	return_err(proc,104,104,"Read on a file not opened for read");
#ifdef isnotansi
    if (num_sample < 0)
	return_err(proc,103,0,
		   rsprintf("Negative sample count %d",num_sample));
#endif
    spifr = sp->read_spifr;
    /* dissallow reads if the file is still compressed. J.Thompson */
    if (spifr->status->user_compress != SP_wc_none) 
 	return_err(proc,110,110,"Unable to Read file in compressed mode");
 
    if (sp_verbose > 10) 
       fprintf(spfp,
	       "Proc %s: file %s, %d bytes/sample, %d channels, %ld samples\n",
	       proc,spifr->status->external_filename,
	       spifr->status->user_sample_n_bytes,
	       spifr->status->user_channel_count,
	       (long int) num_sample);

    if (spifr->waveform->failed_checksum)
	return_err(proc,1001,0,"Pre-Read Checksum Test Failed");


    /***********************************************************************/
    /*     Perform all initializations to the sphere file to begin reading */

    if (spifr->status->read_occured_flag == FALSE) {
	spifr->status->read_occured_flag = TRUE;

	if (sp_verbose > 15) fprintf(spfp,
				     "Proc %s: Initializing read of data\n",
				     proc);
	if (! spifr->waveform->waveform_setup){
	    if ((spifr->status->user_compress == SP_wc_none) &&
		(spifr->status->file_compress != SP_wc_none)){
		decompress_waveform(sp) ; 
		if ((ret = sp_get_return_status()) != 0)
		    return_err(proc,ret,0,
			       rsprintf("decompress_waveform failed, %s",
					get_return_status_message()));
	    }
	    else {
		/* The following code assumes that no pre-reading of the   */
		/* waveform is needed                                      */
		
		if ((spifr->waveform->sp_fob = 
		     fob_create(spifr->waveform->sp_fp)) == FOBPNULL)
		    return_err(proc,300,0,
			  "Unable to allocate a FOB 'File or Buffer' struct.");
		spifr->waveform->sp_fp = FPNULL;
	    }
	    spifr->waveform->waveform_setup = TRUE;
	}
	
    	/****************************************************/
	/**            INVARIANT ASSERTION:                **/
	/** The data is now in it's natural (decomp) form  **/
	/****************************************************/

	/************ Set up the file conversions ***********/
	/****************************************************/
	/*        Set up byte format the conversions        */
	if (spifr->status->user_sbf != spifr->status->file_sbf) 
	    if (((spifr->status->user_sbf == SP_sbf_01) &&
		 (spifr->status->file_sbf == SP_sbf_10)) ||
		((spifr->status->user_sbf == SP_sbf_10) &&
		 (spifr->status->file_sbf == SP_sbf_01)))
		fob_read_byte_swap(spifr->waveform->sp_fob);
	    else
		fob_read_byte_natural(spifr->waveform->sp_fob);

	/********************************************************/
	/*  set up a translation buffer, for sample conversions */
	/*  and channel selections                              */

	/* are sample encodings necessary ????? */
	do_conversions = FALSE;
	if (spifr->status->user_encoding != spifr->status->file_encoding) {
	    if (((spifr->status->file_encoding == SP_se_ulaw) &&
		 (spifr->status->user_encoding == SP_se_pcm2)) || 
		((spifr->status->file_encoding == SP_se_pcm2) &&
		 (spifr->status->user_encoding == SP_se_ulaw))) 
		do_conversions = TRUE;
	    if (((spifr->status->file_encoding == SP_se_pculaw) &&
		 (spifr->status->user_encoding == SP_se_pcm2)) || 
		((spifr->status->file_encoding == SP_se_pcm2) &&
		 (spifr->status->user_encoding == SP_se_pculaw))) 
		do_conversions = TRUE;
	    if (((spifr->status->file_encoding == SP_se_alaw) &&
		 (spifr->status->user_encoding == SP_se_pcm2)) || 
		((spifr->status->file_encoding == SP_se_pcm2) &&
		 (spifr->status->user_encoding == SP_se_alaw))) 
		do_conversions = TRUE;
	    if (do_conversions == FALSE)
		return_err(proc,400,0,
			   "Unable to convert sample types ... for now\n");
	}

	/* are channel selection requested ????? */
	do_channel_selections = FALSE;
	if (spifr->status->channels != CHANNELSNULL) 
	    do_channel_selections = TRUE;

	if (do_conversions || do_channel_selections || 
	    (spifr->status->user_data_fmt == SP_df_array)){
	    /* allocate the memory for the file data buffer   */
	    /*  IF it's a legal transformation                */
	    spifr->waveform->file_data_buffer_len = TRANSLATION_LEN *
		spifr->status->file_sample_n_bytes *
		    spifr->status->file_channel_count;
	    if (sp_verbose > 15)
		fprintf(spfp,"Proc %s: Alloc %d (%d*%d*%d) bytes%s\n",
		       proc,	
		       spifr->waveform->file_data_buffer_len,TRANSLATION_LEN,
		       spifr->status->file_sample_n_bytes,
		       spifr->status->file_channel_count,
		       " for file data buffer");
	    if ((spifr->waveform->file_data_buffer = 
		 (void *)
		       mtrf_malloc(spifr->waveform->file_data_buffer_len)) ==
	        (void *)0)
		return_err(proc,500,0,
			"Unable to alloc memory for the translation buffer");
	} 
	if (do_conversions && 
	    (do_channel_selections ||
	     (spifr->status->user_data_fmt == SP_df_array))){
	    spifr->waveform->converted_buffer_len = TRANSLATION_LEN *
		spifr->status->user_sample_n_bytes * 
		    spifr->status->file_channel_count;
	    if (sp_verbose > 15)
		fprintf(spfp,"Proc %s: Alloc %d (%d*%d*%d) bytes %s\n",
		       proc,	
		       spifr->waveform->converted_buffer_len,TRANSLATION_LEN,
		       spifr->status->user_sample_n_bytes,
		       spifr->status->file_channel_count,
		       "for converted data buffer");
	    if ((spifr->waveform->converted_buffer = 
		 (void *)
		       mtrf_malloc(spifr->waveform->converted_buffer_len)) ==
		(void *)0)
		return_err(proc,550,0,
			"Unable to alloc memory for the translation buffer");
	}
	if (spifr->status->user_data_fmt == SP_df_array){
	    spifr->waveform->interleave_buffer_len = TRANSLATION_LEN *
		spifr->status->user_sample_n_bytes * 
		    spifr->status->user_channel_count;
	    if (sp_verbose > 15)
		fprintf(spfp,"Proc %s: Alloc %d (%d*%d*%d) bytes %s\n",
			proc,	
			spifr->waveform->interleave_buffer_len,TRANSLATION_LEN,
			spifr->status->user_sample_n_bytes,
			spifr->status->user_channel_count,
			"for interleave data buffer");
	    if ((spifr->waveform->interleave_buffer = 
		 (void *)
		       mtrf_malloc(spifr->waveform->interleave_buffer_len)) ==
		(void *)0)
		return_err(proc,600,0,
			"Unable to alloc memory for the interleave buffer");
	}
	    

	/* pre-verify the waveform data */
	if (spifr->status->extra_checksum_verify){
	    pre_verify_checksum(sp);
	    if ((ret = sp_get_return_status()) != 0)
		return_err(proc,ret,0,
			   rsprintf("pre_verify_checksum failed, %s",
				    get_return_status_message()));
	} else {
	    /* ONLY Pre-verify the waveform once, subsequent rewind operations */
	    /* should not have to do this */
	    spifr->status->extra_checksum_verify = FALSE;
	}
    }

    if (sp_verbose > 15) fprintf(spfp,
				 "Proc %s: current file position %d\n",proc,
				fob_ftell(spifr->waveform->sp_fob));

    ret = read_data_in(buffer, num_sample, sp);
    if (sp_get_return_type() == RETURN_TYPE_ERROR)
	return_err(proc,sp_get_return_status(),0,
		   rsprintf("read_data_in failed, %s",
			    get_return_status_message()));
    return_success(proc,0,ret,"ok");
}

static int read_data_in(void *buffer, size_t num_sample, SP_FILE *sp){ 
    SPIFR *spifr;
    char *proc="    read_data_in " SPHERE_VERSION_STR;
    int samples_read = 0;
    int in_samples=0, o, oc, s, c, ret;
    int block=0;
    char *in_block=(char *)0, *out_block=(char *)0;
    char *current_data_location = (char *)0;
    char *out_conversion_block=(char *)0,*in_conversion_block=(char *)0;
    char *next_out_block=(char *)0;
    short *sh_data=(short *)0;
    unsigned char *ulaw_data = (unsigned char *)0;
    unsigned char *pculaw_data = (unsigned char *)0;
    unsigned char *alaw_data = (unsigned char *)0;
    SP_CHECKSUM checksum=0, block_checksum=0;
    int file_record_size;
    char **arr_buff = (char **)0, *arr;
       
    spifr = sp->read_spifr;

    file_record_size = spifr->status->file_sample_n_bytes *
	spifr->status->file_channel_count;
    
    while ((samples_read < num_sample) &&
	   (! fob_feof(spifr->waveform->sp_fob))){
	if (sp_verbose > 16)
	    fprintf(spfp,"Proc %s:  Beginning block %d\n",proc,block);
	
	/* read in either a block or the whole chunk if data */
	if (spifr->waveform->file_data_buffer != CNULL) {
	    if (sp_verbose > 15) 
		fprintf(spfp,
			"Proc %s: reading a block into temporary storage\n",
			proc);
	    in_block = (void *)spifr->waveform->file_data_buffer;
	    current_data_location = in_block;
	    if ((in_samples = (num_sample - samples_read))>
		TRANSLATION_LEN)
		in_samples = TRANSLATION_LEN;
	    ret = fob_fread(in_block,  file_record_size,
			    in_samples, spifr->waveform->sp_fob);
	    if (ret < 0)
		return_err(proc,105,0,"Unable to read data");
	} else { 
	    /* there was no change in the sample coding so just read    */
	    /* in the data */
	    if (sp_verbose > 15)
		fprintf(spfp,
			"Proc %s: read a block WITHOUT coding conversions\n",
			proc);
	    ret = fob_fread(buffer, file_record_size,
			    num_sample, spifr->waveform->sp_fob);
	    if (ret < 0)
		return_err(proc,107,0,"Unable to read data");
	    in_block = (void *)buffer;
	    current_data_location = in_block;
	}

	if (sp_verbose > 16)
	    fprintf(spfp,
		    "Proc %s: block read of %d Samples, expected %ld\n",
		    proc,ret, (long int) num_sample);
	in_samples = ret;
	/**** ret is the number of samples per time period read in from */
	/**** the file */
	
	/**** Do the checksum computation before format changes occur ***/
	switch (spifr->status->file_encoding){
	  case SP_se_pcm2:
	    
	    /* DON'T SWAP if the coding type hasn't changed, and the    */
	    /* output SBF == natural_sbf                                */
	    /*    OR         if the coding changed and the file SBF     */
	    /*               is == natural SBF */
	    if (((spifr->status->user_encoding == SP_se_pcm2) &&
		 (spifr->status->natural_sbf == spifr->status->user_sbf))
		||
		((spifr->status->user_encoding != SP_se_pcm2) &&
		 (spifr->status->natural_sbf == spifr->status->file_sbf))){
		if (sp_verbose > 16)
		    fprintf(spfp,
			    "Proc %s: Not Swapping for checksum\n",proc);
		checksum =
		    sp_compute_short_checksum((void *)in_block,in_samples*
				      spifr->status->file_channel_count, 
					      FALSE);
	    } else {
		if (sp_verbose > 16)
		    fprintf(spfp,"Proc %s: Swapping for checksum\n",proc);
		checksum = 
		    sp_compute_short_checksum((void *)in_block,
		       in_samples*spifr->status->file_channel_count, TRUE);
	    }
	    break;
	  default: {
	      int n;
	      n = in_samples * spifr->status->file_channel_count *
		  spifr->status->file_sample_n_bytes;
	      if (sp_verbose > 16)
		  fprintf(spfp,"Proc %s: Computing char checksum %d bytes\n",
			  proc,n);
	      checksum = sp_compute_char_checksum(in_block, n);
	      break;
	  }
	}
	
	spifr->waveform->checksum = 
	    sp_add_checksum(spifr->waveform->checksum,checksum);
	block_checksum = checksum;
	
	/***  FINISHED WITH THE CHECKSUMS ***/
	samples_read += in_samples;
	spifr->waveform->samples_read += in_samples;
	
	if (sp_eof(sp))
	    /* only check this if the user_sample count is real */
	    /* Added June 22, 1994 by JGF                       */
	    if (spifr->status->user_sample_count != 999999999){
		if (spifr->waveform->samples_read !=
		    spifr->status->user_sample_count){
		    spifr->waveform->read_premature_eof = TRUE;
		    return_err(proc,500,0,
			       rsprintf("Premature End-of-File %d read != %d",
					spifr->waveform->samples_read,
					spifr->status->user_sample_count));

		}
	    }
	if (spifr->waveform->samples_read ==
	    spifr->status->user_sample_count){
	    if ((! spifr->status->ignore_checksum) &&
		(spifr->waveform->checksum !=
		 spifr->status->file_checksum)){
		spifr->waveform->failed_checksum = TRUE;	    
		return_err(proc,1000,0,
			   rsprintf("%sComputed %d != Expected %d",
				    "Checksum Test Failed ",
				    spifr->waveform->checksum,
				    spifr->status->file_checksum));
	    }
	}
	
	/****   Do the sample coding conversions  ****/
	if (((spifr->status->file_encoding == SP_se_ulaw) &&
	     (spifr->status->user_encoding == SP_se_pcm2)) || 
	    ((spifr->status->file_encoding == SP_se_pcm2) &&
	     (spifr->status->user_encoding == SP_se_ulaw))) {
	    int samples_to_change = 
		in_samples*spifr->status->file_channel_count;

	    if ((spifr->status->channels != CHANNELSNULL) ||
		(spifr->status->user_data_fmt == SP_df_array)){
		out_conversion_block = next_out_block = 
		    (char *)spifr->waveform->converted_buffer;
		if (sp_verbose > 16)
		    fprintf(spfp,"Proc %s: using converted buffer output\n",
			    proc);
	    }
	    else {
		out_conversion_block = (char *)buffer;
		if (sp_verbose > 16)
			fprintf(spfp,"Proc %s: using buffer output argument\n",
				proc);
		if (next_out_block == (char *)0)
		    next_out_block = out_conversion_block;
	    }
	    
	    if (sp_verbose > 16)
		fprintf(spfp,
			"Proc %s: converting %d (%d*%d) samples\n",proc,
			samples_to_change,in_samples,
			spifr->status->file_channel_count);
	    if ((spifr->status->file_encoding == SP_se_ulaw) &&
		(spifr->status->user_encoding == SP_se_pcm2)) {
		/* now convert the block into the buffer */
		if (sp_verbose > 16)
		    fprintf(spfp,"Proc %s: Converting ULAW to PCM", proc);
		
		/* set up some pointers */
		ulaw_data =
		    (unsigned char *)spifr->waveform->file_data_buffer;
		sh_data = (short *)next_out_block ;
		
	       /*** This was a bug, it used to compare to sp_sbf_10 ***/
		if (spifr->status->user_sbf != get_natural_sbf(2)){
		    if (sp_verbose > 16)
			fprintf(spfp," outputting 01 format bytes\n");
		    for (o=0; o < samples_to_change; o++) {
			*sh_data = ulaw2linear_01_sbf_lut[*ulaw_data];
			sh_data ++; ulaw_data ++;
		    }
		} else {
		    if (sp_verbose > 16)
			fprintf(spfp," outputting 10 format bytes\n");
		    for (o=0; o < samples_to_change; o++) {
			*sh_data = ulaw2linear_10_sbf_lut[*ulaw_data];
			sh_data ++; ulaw_data ++;
		    }
		}
		next_out_block = (char *)sh_data;
	    } else if ((spifr->status->file_encoding == SP_se_pcm2) &&
		       (spifr->status->user_encoding == SP_se_ulaw)) {
		if (sp_verbose > 16)
		    fprintf(spfp,"Proc %s: Performing read translation%s\n",
			    proc," from PCM2 to ULAW");
		/* set up some pointers */
		sh_data = (short *)spifr->waveform->file_data_buffer;
		ulaw_data = (unsigned char *)next_out_block;
		

		if (spifr->status->file_sbf != spifr->status->natural_sbf){
		    char *p , temp; short odata;
		    if (sp_verbose > 16)
			fprintf(spfp,
				"Proc %s: Swapping input shorts\n", proc);
		    for (o=0; o < samples_to_change; o++) {
			/* Pre swap the bytes */
			
			odata = *sh_data;
			p = (char *) &odata;
			temp = *p;
			*p = *(p + 1);
			*(p + 1) = temp;
			/* fprintf(spfp,
			   "  %d -> %x -> %x\n",o,*sh_data,odata); */
			
			*ulaw_data = linear2ulaw(odata);
			sh_data ++; ulaw_data ++;
		    }
		}else
		    for (o=0; o < samples_to_change; o++) {
			*ulaw_data =linear2ulaw(*sh_data);
			sh_data ++; ulaw_data ++;
		    }
		next_out_block = (char *)ulaw_data;
	    } else 
		return_err(proc,109,0,"Internal Error");
	    
	    current_data_location = out_conversion_block;
	} else if (((spifr->status->file_encoding == SP_se_pculaw) &&
	     (spifr->status->user_encoding == SP_se_pcm2)) || 
	    ((spifr->status->file_encoding == SP_se_pcm2) &&
	     (spifr->status->user_encoding == SP_se_pculaw))) {
	    int samples_to_change = 
		in_samples*spifr->status->file_channel_count;

	    if ((spifr->status->channels != CHANNELSNULL) ||
		(spifr->status->user_data_fmt == SP_df_array)){
		out_conversion_block = next_out_block = 
		    (char *)spifr->waveform->converted_buffer;
		if (sp_verbose > 16)
		    fprintf(spfp,"Proc %s: using converted buffer output\n",
			    proc);
	    }
	    else {
		out_conversion_block = (char *)buffer;
		if (sp_verbose > 16)
			fprintf(spfp,"Proc %s: using buffer output argument\n",
				proc);
		if (next_out_block == (char *)0)
		    next_out_block = out_conversion_block;
	    }
	    
	    if (sp_verbose > 16)
		fprintf(spfp,
			"Proc %s: converting %d (%d*%d) samples\n",proc,
			samples_to_change,in_samples,
			spifr->status->file_channel_count);
	    if ((spifr->status->file_encoding == SP_se_pculaw) &&
		(spifr->status->user_encoding == SP_se_pcm2)) {
		/* now convert the block into the buffer */
		if (sp_verbose > 16)
		    fprintf(spfp,"Proc %s: Converting PCULAW to PCM", proc);
		
		/* set up some pointers */
		pculaw_data =
		    (unsigned char *)spifr->waveform->file_data_buffer;
		sh_data = (short *)next_out_block ;
		
	       /*** This was a bug, it used to compare to sp_sbf_10 ***/
		if (spifr->status->user_sbf != get_natural_sbf(2)){
		    if (sp_verbose > 16)
			fprintf(spfp," outputting 01 format bytes\n");
		    for (o=0; o < samples_to_change; o++) {
			*sh_data = ulaw2linear_01_sbf_lut[
				       uchar_bitreverse_lut[*pculaw_data]];
			sh_data ++; pculaw_data ++;
		    }
		} else {
		    if (sp_verbose > 16)
			fprintf(spfp," outputting 10 format bytes\n");
		    for (o=0; o < samples_to_change; o++) {
			*sh_data = ulaw2linear_10_sbf_lut[
					uchar_bitreverse_lut[*pculaw_data]];
			sh_data ++; pculaw_data ++;
		    }
		}
		next_out_block = (char *)sh_data;
	    } else if ((spifr->status->file_encoding == SP_se_pcm2) &&
		       (spifr->status->user_encoding == SP_se_pculaw)) {
		if (sp_verbose > 16)
		    fprintf(spfp,"Proc %s: Performing read translation%s\n",
			    proc," from PCM2 to PCULAW");
		/* set up some pointers */
		sh_data = (short *)spifr->waveform->file_data_buffer;
		pculaw_data = (unsigned char *)next_out_block;
		

		if (spifr->status->file_sbf != spifr->status->natural_sbf){
		    char *p , temp; short odata;
		    if (sp_verbose > 16)
			fprintf(spfp,
				"Proc %s: Swapping input shorts\n", proc);
		    for (o=0; o < samples_to_change; o++) {
			/* Pre swap the bytes */
			
			odata = *sh_data;
			p = (char *) &odata;
			temp = *p;
			*p = *(p + 1);
			*(p + 1) = temp;
			/* fprintf(spfp,
			   "  %d -> %x -> %x\n",o,*sh_data,odata); */
			
			*pculaw_data = uchar_bitreverse_lut[linear2ulaw(odata)];
			sh_data ++; pculaw_data ++;
		    }
		}else
		    for (o=0; o < samples_to_change; o++) {
			*pculaw_data = uchar_bitreverse_lut[
				                       linear2ulaw(*sh_data)];
			sh_data ++; pculaw_data ++;
		    }
		next_out_block = (char *)pculaw_data;
	    } else 
		return_err(proc,109,0,"Internal Error");
	    
	    current_data_location = out_conversion_block;
	} else 	if (((spifr->status->file_encoding == SP_se_alaw) &&
		     (spifr->status->user_encoding == SP_se_pcm2)) || 
		    ((spifr->status->file_encoding == SP_se_pcm2) &&
		     (spifr->status->user_encoding == SP_se_alaw))) {
	    int samples_to_change = 
		in_samples*spifr->status->file_channel_count;

	    if ((spifr->status->channels != CHANNELSNULL) ||
		(spifr->status->user_data_fmt == SP_df_array)){
		out_conversion_block = next_out_block = 
		    (char *)spifr->waveform->converted_buffer;
		if (sp_verbose > 16)
		    fprintf(spfp,"Proc %s: using converted buffer output\n",
			    proc);
	    }
	    else {
		out_conversion_block = (char *)buffer;
		if (sp_verbose > 16)
			fprintf(spfp,"Proc %s: using buffer output argument\n",
				proc);
		if (next_out_block == (char *)0)
		    next_out_block = out_conversion_block;
	    }
	    
	    if (sp_verbose > 16)
		fprintf(spfp,
			"Proc %s: converting %d (%d*%d) samples\n",proc,
			samples_to_change,in_samples,
			spifr->status->file_channel_count);
	    if ((spifr->status->file_encoding == SP_se_alaw) &&
		(spifr->status->user_encoding == SP_se_pcm2)) {
		/* now convert the block into the buffer */
		if (sp_verbose > 16)
		    fprintf(spfp,"Proc %s: Converting ALAW to PCM", proc);
		
		/* set up some pointers */
		alaw_data =
		    (unsigned char *)spifr->waveform->file_data_buffer;
		sh_data = (short *)next_out_block ;
		
	       /*** This was a bug, it used to compare to sp_sbf_10 ***/
		if (spifr->status->user_sbf != get_natural_sbf(2)){
		    if (sp_verbose > 16)
			fprintf(spfp," outputting 01 format bytes\n");
		    for (o=0; o < samples_to_change; o++) {
			*sh_data = alaw2linear_01_sbf_lut[*alaw_data];
			sh_data ++; alaw_data ++;
		    }
		} else {
		    if (sp_verbose > 16)
			fprintf(spfp," outputting 10 format bytes\n");
		    for (o=0; o < samples_to_change; o++) {
			*sh_data = alaw2linear_10_sbf_lut[*alaw_data];
			sh_data ++; alaw_data ++;
		    }
		}
		next_out_block = (char *)sh_data;
	    } else if ((spifr->status->file_encoding == SP_se_pcm2) &&
		       (spifr->status->user_encoding == SP_se_alaw)) {
		if (sp_verbose > 16)
		    fprintf(spfp,"Proc %s: Performing read translation%s\n",
			    proc," from PCM2 to ALAW");
		/* set up some pointers */
		sh_data = (short *)spifr->waveform->file_data_buffer;
		alaw_data = (unsigned char *)next_out_block;
		

		if (spifr->status->file_sbf != spifr->status->natural_sbf){
		    char *p , temp; short odata;
		    if (sp_verbose > 16)
			fprintf(spfp,
				"Proc %s: Swapping input shorts\n", proc);
		    for (o=0; o < samples_to_change; o++) {
			/* Pre swap the bytes */
			
			odata = *sh_data;
			p = (char *) &odata;
			temp = *p;
			*p = *(p + 1);
			*(p + 1) = temp;
			/* fprintf(spfp,
			   "  %d -> %x -> %x\n",o,*sh_data,odata); */
			
			*alaw_data = linear2alaw(odata);
			sh_data ++; alaw_data ++;
		    }
		}else
		    for (o=0; o < samples_to_change; o++) {
			*alaw_data =linear2alaw(*sh_data);
			sh_data ++; alaw_data ++;
		    }
		next_out_block = (char *)alaw_data;
	    } else 
		return_err(proc,109,0,"Internal Error");
	    
	    current_data_location = out_conversion_block;
	}

	/* Do the channel conversions */
	if (spifr->status->channels != CHANNELSNULL){
	    int in_recsize, out_recsize, in_chancnt, out_chancnt;
	    
	    if (sp_verbose > 15)
		fprintf(spfp,"Proc %s: Converting channels\n",proc);
	    if (spifr->status->user_data_fmt == SP_df_array)
		out_block = spifr->waveform->interleave_buffer;
	    else
		if (out_block == (void *)0)
		    out_block = (void *)buffer;
	    current_data_location = out_block;
		
	    if (spifr->status->file_encoding !=	spifr->status->user_encoding)
		in_conversion_block = 
		    (void *)spifr->waveform->converted_buffer;
	    else 
		if (in_conversion_block == (void *)0) 
		    in_conversion_block =
			(void *)spifr->waveform->file_data_buffer;

	    
	    in_chancnt = spifr->status->file_channel_count;
	    in_recsize = spifr->status->user_sample_n_bytes *  in_chancnt;
	    out_chancnt = spifr->status->user_channel_count;
	    out_recsize = spifr->status->user_sample_n_bytes * out_chancnt;
	    
	    for (oc=0; oc<spifr->status->channels->num_chan; oc++)
		if (spifr->status->channels->ochan[oc].num_origin == 1){
		    char *in_ptr, *out_ptr;
		    in_ptr = in_conversion_block + 
                        ((spifr->status->channels->ochan[oc].orig_channel[0]-1)
			 * spifr->status->user_sample_n_bytes);
		    out_ptr = out_block + 
			(oc * spifr->status->user_sample_n_bytes);
		    
		    for (s=0; s<in_samples; s++){
			memcpy(out_ptr,in_ptr,
			      spifr->status->user_sample_n_bytes);
			in_ptr += in_recsize;
			out_ptr += out_recsize;
		    }
		} else {
		    if (spifr->status->user_encoding == SP_se_pcm2) {
			/* do a straight add */
			ORIGINATION_CHAN *ochn;
			short *in_ptr, *out_ptr;
			int sum, ch;
			if (sp_verbose > 15)
			    fprintf(spfp,
				    "Proc %s: Adding PCM data %s %d\n",
				    proc, "channel",oc);
			in_ptr = (short *)in_conversion_block;
			out_ptr = (short *)out_block;
			ochn = &(spifr->status->channels->ochan[oc]);
			
			if (spifr->status->user_sbf== get_natural_sbf(2)){
			    for (s=0; s<in_samples; s++){
				for (ch=0, sum=0; ch < ochn->num_origin; 
				     ch++){
				    sum +=*(in_ptr+ochn->orig_channel[ch]-1);
				}
				if (sum > 32767) sum = 32767;
				else if (sum < -32768) sum = -32768;
				*(out_ptr + oc) = (short)sum;
				in_ptr += in_chancnt;
				out_ptr += out_chancnt;
			    }
			} else {
			    short x;
			    if (sp_verbose > 15)
				fprintf(spfp,"Proc %s: %s %d\n", proc,
					"Adding Swapped PCM data channel",oc);
			    for (s=0; s<in_samples; s++){
				/*    fprintf(spfp,"sample %d\n",s);  */
				for (ch=0, sum=0; ch < ochn->num_origin; 
				     ch++){
				    x = *(in_ptr+ochn->orig_channel[ch]-1);
				    swap_bytes(x);
				    sum += x;
				    /* fprintf(spfp,"   %d  =  %d\n",x,sum); */
				}
				if (sum > 32767) x = 32767;
				else if (sum < -32768) x = -32768;
				else x = sum;
				swap_bytes(x);
				*(out_ptr + oc) = (short)x;
				in_ptr += in_chancnt;
				out_ptr += out_chancnt;
			    }
			    
			}			   
		    } else if (spifr->status->user_encoding == SP_se_ulaw){
			/* do an add on ulaw data */
			ORIGINATION_CHAN *ochn;
			unsigned char *in_ptr, *out_ptr;
			int sum, ch;
			if (sp_verbose > 15)
			    fprintf(spfp,
				    "Proc %s: Adding ULAW data %s %d\n",
				    proc, "channel",oc);
			in_ptr = (unsigned char *)in_conversion_block;
			out_ptr = (unsigned char *)out_block;
			ochn = &(spifr->status->channels->ochan[oc]);
			
			for (s=0; s<in_samples; s++){
			    for (ch=0, sum=0; ch < ochn->num_origin;
				 ch++){
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
		    } else if (spifr->status->user_encoding == SP_se_pculaw){
			/* do an add on ulaw data */
			ORIGINATION_CHAN *ochn;
			unsigned char *in_ptr, *out_ptr;
			int sum, ch;
			if (sp_verbose > 15)
			    fprintf(spfp,
				    "Proc %s: Adding PCULAW data %s %d\n",
				    proc, "channel",oc);
			in_ptr = (unsigned char *)in_conversion_block;
			out_ptr = (unsigned char *)out_block;
			ochn = &(spifr->status->channels->ochan[oc]);
			
			for (s=0; s<in_samples; s++){
			    for (ch=0, sum=0; ch < ochn->num_origin;
				 ch++){
				sum +=
				    ulaw2linear_10_sbf_lut[
				      uchar_bitreverse_lut[
					*(in_ptr+ochn->orig_channel[ch]-1)]];
			    }
			    if (sum > 32767) sum = 32767;
			    else if (sum < -32768) sum = -32768;
			    *(out_ptr + oc) =  uchar_bitreverse_lut[
						    linear2ulaw(sum)];
			    in_ptr += in_chancnt;
			    out_ptr += out_chancnt;
			}
		    } else if (spifr->status->user_encoding == SP_se_alaw){
			/* do an add on alaw data */
			ORIGINATION_CHAN *ochn;
			unsigned char *in_ptr, *out_ptr;
			int sum, ch;
			if (sp_verbose > 15)
			    fprintf(spfp,
				    "Proc %s: Adding ALAW data %s %d\n",
				    proc, "channel",oc);
			in_ptr = (unsigned char *)in_conversion_block;
			out_ptr = (unsigned char *)out_block;
			ochn = &(spifr->status->channels->ochan[oc]);
			
			for (s=0; s<in_samples; s++){
			    for (ch=0, sum=0; ch < ochn->num_origin;
				 ch++){
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
		    }
		}
	    out_block += out_recsize * in_samples;
	}
	
	if (spifr->status->user_data_fmt == SP_df_array){
	    /* convert the data from it's interleaved form */
	    /* to an array format                          */
	    int snb = spifr->status->user_sample_n_bytes;
	    int chcnt = spifr->status->user_channel_count;
	    int recsize = snb * chcnt;
	    char *in_dat;
	    
	    if (sp_verbose > 15)
		fprintf(spfp,"Proc %s: Converting data to an array\n",
			proc);
	    
	    if (arr_buff == (char  **)0)
		arr_buff = (char **)buffer;
	    for (c=0; c<chcnt; c++){
		in_dat =  current_data_location + (snb * c) ;
		arr = arr_buff[c] + ((samples_read-in_samples) * snb);
		
		for (s=0; s<in_samples; s++,arr+=snb,in_dat+=recsize)
		    memcpy(arr,in_dat,snb);
	    }
	}
	if (sp_verbose > 11){
	    fprintf(spfp,"Proc %s: Block %d: Requested %ld samples",
		    proc,block++, (long int) num_sample);
	    fprintf(spfp," read %d total %d, Checksum %d (BC %d)\n",
		    ret,spifr->waveform->samples_read,
		    block_checksum,
		    spifr->waveform->checksum);
	}
    }
    return_success(proc,0,samples_read,"ok");
}

static int decompress_waveform(SP_FILE *sp)
{
    int decomp_into_memory=TRUE;
    int wav_bytes=0;
    SPIFR *spifr = sp->read_spifr;
    FOB *fob_in, *fob_out;
    char *buff;
    int blen;
    char *proc="decompress_waveform " SPHERE_VERSION_STR;
    
    wav_bytes = spifr->status->user_sample_count * 
	spifr->status->user_channel_count * 
	    spifr->status->user_sample_n_bytes;
    
    /* the file must be decompressed, Question: Should it be done in memory? */
    if (wav_bytes > MAX_INTERNAL_WAVFORM)
	decomp_into_memory = FALSE;
    
    /* The file needs to be de_compressed into memory !!!! */
    /* 1. make an FOB struct for the uncompressed waveform to be read into d */
    /*    and the original file */
    /* 2. allocate memory for the entire waveform */
    /* 3. decompress the file */
    /* 4. Clean up the FOB struct for the file, moving the fp from the FOB */
    /* 5. reset the uncompressed FOB struct to the beginning of the memory */
		

    if (decomp_into_memory) {
	if (sp_verbose > 15)
	    fprintf(spfp,"Proc %s: Pre-buffering compressed data into memory\n",
		   proc);
	if (fob_create2(spifr->waveform->sp_fp, FPNULL, &fob_in, &fob_out) < 0)
	    return_err(proc,200,1,"Unable to setup for decompression");
	blen = spifr->status->file_channel_count * 
	    spifr->status->file_sample_count * 
		spifr->status->file_sample_n_bytes;
	if ((buff=mtrf_malloc(blen)) == CNULL){
	    fob_destroy(fob_in);
	    fob_destroy(fob_out);
	    return_err(proc,201,1,"Unable to malloc memory to decompress into");
	}
	fob_bufinit(fob_out, buff, blen);
    } else { /* decompress into a disk file */
	FILE *temp_fp;
	if (sp_verbose > 15)
	    fprintf(spfp,"Proc %s: Pre-buffering compressed data %s\n",
		   proc,"into a temporary file");
	spifr->status->temp_filename = sptemp_dirfile();
	if (spifr->status->temp_filename == CNULL)
	    return_err(proc,400,1,"Unable to create usable temporary file");
	if (sp_verbose > 15)
	    fprintf(spfp,"Proc %s: %s%d bytes long, using temp file %s\n",
		   proc, "Attempting to read a big file ",wav_bytes,
		   spifr->status->temp_filename);
	if ((temp_fp=fopen(spifr->status->temp_filename,TRUNCATE_UPDATEMODE)) 
	    == FPNULL) 
	    return_err(proc,401,1,
		       rsprintf("Unable to open temporary file %s",
				spifr->status->temp_filename));

	if (fob_create2(spifr->waveform->sp_fp, temp_fp, &fob_in, &fob_out) <0)
	    return_err(proc,402,1,"Unable to setup for decompression");
	
	/* the FILE pointer will be closed after completion of the           */
	/* decompression directly from the fob_in FOB pointer.  this is the  */
	/* bug which Francis caught.                                         */
	
	/* Note:  Do NOT set the waveform file to FPNULL here (see bugfix    */
	/* below.)  ***PSI***  24-Sep-1993                                   */
	/* spifr->waveform->sp_fp = FPNULL; */
	spifr->status->is_temp_file = TRUE;
    }
    if (sp_verbose > 15)
	fprintf(spfp,
		"Proc %s: before decompression, input file at position %d\n",
	       proc,fob_ftell(fob_in));
    switch (spifr->status->file_compress){
	char message[80];
      case SP_wc_shorten:
	if (sp_verbose > 15)
	    fprintf(spfp,"Proc %s: Executing Shorten Decompression\n",proc);
	if (setjmp(exitenv) == 0){
	    shorten_reset_flags();
	    if (shorten_uncompress(fob_in, fob_out, message) < 0){
		fob_destroy(fob_in);
		fob_destroy(fob_out);
		return_err(proc,202,1,
			   rsprintf("Shorten Decompression Failed",message));
	    }
	} else {
	    fob_destroy(fob_in);
	    fob_destroy(fob_out);
	    return_err(proc,207,1,"Shorten Decompression aborted");
	}
	break;
      case SP_wc_wavpack:
	if(setjmp(exitenv) == 0){
	    if (sp_verbose > 15) fprintf(spfp,"before set progname\n");
	    wavpack_set_progname( "wavpack" );
	    if (sp_verbose > 15)
		fprintf(spfp,
			"Proc %s: Executing Wavpack Decompression\n",proc);
	    if (sp_verbose > 15)
		wavpack_dump_interface(spfp);
	    if (wavpack_unpack(fob_in, fob_out) < 0){
		fob_destroy(fob_in);
		fob_destroy(fob_out);
		return_err(proc,203,1,"Wavpack Decompression Failed");
	    }
	    wavpack_free_progname();
	} else {
	    fob_destroy(fob_in);
	    fob_destroy(fob_out);
	    return_err(proc,206,1,"Wavpack Decompression aborted");
	}
	break;
      case SP_wc_shortpack:
	if(setjmp(exitenv) == 0){
	    if (sp_verbose > 15)
		fprintf(spfp,
			"Proc %s: Executing Shortpack Decompression\n",proc);
	    if (shortpack_uncompress(fob_in, fob_out,
				     spifr->status->file_header) < 0){
		fob_destroy(fob_in);
		fob_destroy(fob_out);
		return_err(proc,203,1,"Shortpack Decompression Failed");
	    }
	} else {
	    fob_destroy(fob_in);
	    fob_destroy(fob_out);
	    return_err(proc,208,1,"Shortpack Decompression aborted");
	}		    
	break;
      default:
	return_err(proc,209,1,"Unable to decompress the requested format\n");
    }
    fob_rewind(fob_out);
    
    
    /**** Begin SRI bugfix. ****/
    
    /* If a temporary file is being used, close the waveform file BEFORE    */
    /* setting it to FPNULL.  ***PSI***  24-Sep-1993                        */
    
    if (spifr->status->is_temp_file) {
	if (sp_verbose > 15)
	    fprintf(spfp,"Proc %s: Closing waveform file \"%s\" (%d)\n",
		   proc, spifr->status->external_filename,
		   fileno(spifr->waveform->sp_fp));
	
	if (fclose(spifr->waveform->sp_fp))
	    return_err(proc, 403, 0,
		       rsprintf("Unable to close waveform file \"%s\" (%d)",
				spifr->status->external_filename,
				fileno(spifr->waveform->sp_fp)));
	spifr->waveform->sp_fp = FPNULL;
	/* MM add: 'fob_in' was created from 'spifr->waveform->sp_fp',
	   so it is important to set it FPNULL */
	fob_in->fp = FPNULL;
    }
    
    /**** End SRI bugfix. ****/
    
    if (! decomp_into_memory) { 
	/* Close the original file pointer to the SPHERE file */
      if (fob_in->fp != FPNULL)
	fclose(fob_in->fp);
    }
    fob_destroy(fob_in);	    
    
    spifr->waveform->sp_fob = fob_out;
    return_success(proc,0,0,"ok");
}


/*
 *  This function just computes the checksum of the file pointed to by 
 *  an FOB structure.
 */
static int fob_short_checksum(FOB *f, SP_CHECKSUM *checksum, int do_byte_swap, 
	       SP_CHECKSUM (*add_checksum) (SP_CHECKSUM , SP_CHECKSUM ))
{
    long cur_fp_pos;
    short buff[1024];
    int len, ss;
    SP_CHECKSUM chks;

    chks = *checksum = 0;
    ss = sizeof(short);
    if (fob_is_fp(f)){
	cur_fp_pos = ftell(f->fp);
	while ((len=fread(buff,ss,1024,f->fp)) > 0){
	    chks = sp_compute_short_checksum((short *)buff, len, do_byte_swap);
	    *checksum = (*add_checksum)(*checksum,chks);
	}
	if (ferror(f->fp)){
	    fseek(f->fp,cur_fp_pos,SEEK_SET);
	    return(-1);
	}
	fseek(f->fp,cur_fp_pos,SEEK_SET);
	clearerr(f->fp);
	return(0);
    } else {
        *checksum = sp_compute_short_checksum((short *)(f->buf),
					      f->length/2, do_byte_swap);
	return(0);
    }
}

/*
 *  This function just computes the checksum of the file pointed to by 
 *  an FOB structure.
 */
static int fob_char_checksum(FOB *f, SP_CHECKSUM *checksum,  
	       SP_CHECKSUM (*add_checksum) (SP_CHECKSUM , SP_CHECKSUM ))
{
    long cur_fp_pos;
    char buff[1024];
    int len, sc;
    SP_CHECKSUM chks;

    chks = *checksum = 0;
    sc = sizeof(char);
    if (fob_is_fp(f)){
	cur_fp_pos = ftell(f->fp);
	while ((len=fread(buff,sc,1024,f->fp)) > 0){
	    chks = sp_compute_char_checksum((char *)buff, len);
	    *checksum = (*add_checksum)(*checksum,chks);
	}
	if (ferror(f->fp)){
	    fseek(f->fp,cur_fp_pos,SEEK_SET);
	    return(-1);
	}
	fseek(f->fp,cur_fp_pos,SEEK_SET);
	clearerr(f->fp);
	return(0);
    } else {
        *checksum = sp_compute_char_checksum((char *)(f->buf),f->length);
	return(0);
    }
}

static int pre_verify_checksum(SP_FILE *sp){
    char *proc="pre_verify_checksum " SPHERE_VERSION_STR;
    SPIFR *spifr=sp->read_spifr;
    SP_CHECKSUM checksum;
    
    /* Check to see if the stored checksum is valid.  If not, report that the checksum was correct */
    if (spifr->status->ignore_checksum == TRUE){
      return_success(proc,0,0,"ok, no checksum to test");
    }

    /* if the samples are in memory, compute the checksum from there. */
    /* if not, read in the file, a block at a time and compute the    */
    /* checksum.                                                      */
    switch (spifr->status->file_encoding){
      case SP_se_pcm2:
	if (fob_short_checksum(spifr->waveform->sp_fob, &checksum,
			       spifr->status->file_sbf != 
			       spifr->status->natural_sbf,
			       sp_add_checksum) < 0)
	    return_err(proc,501,0,"Unable to Pre-Verify Checksum");
	if (checksum != spifr->status->file_checksum){
	    spifr->waveform->failed_checksum = TRUE;
	    return_err(proc,1001,0,
		       "Pre-Read Short Checksum Test Failed");
	}
	break;
      default:
	if (fob_char_checksum(spifr->waveform->sp_fob,
			      &checksum,sp_add_checksum) < 0)
	    return_err(proc,502,0,"Unable to Pre-Verify Checksum");
	if (checksum != spifr->status->file_checksum){
	    spifr->waveform->failed_checksum = TRUE;
	    return_err(proc,1001,0,
		       "Pre-Read Char Checksum Test Failed");
	}
	break;
    }
    return_success(proc,0,0,"ok");
}

