#include <stdio.h>
#define SPHERE_LIBRARY_CODE
#include <sp/sphere.h>
#include <string.h>
#include <setjmp.h>

extern jmp_buf	exitenv;

int verify_file_checksum(char *filename) ;

int sp_close(SP_FILE *sp)
{
    char *proc="sp_close " SPHERE_VERSION_STR;
    char *write_name;
    char *read_name;
    SP_INTEGER lint=0, header_size=0, data_size=0;
    int header_changed=FALSE;
    SPIFR *spifr;
    SPSTATUS *w_spstat, *r_spstat;
    int ret, verify_checksum=FALSE;

    if (sp_verbose > 10) fprintf(spfp,"Proc %s:\n",proc);
    if (sp == SPNULL)
	return_err(proc,100,100,"Null SPFILE pointer");

    w_spstat = sp->write_spifr->status;
    r_spstat = sp->read_spifr->status;
    write_name = (w_spstat->external_filename == CNULL) ? CNULL :
	mtrf_strdup(sp->write_spifr->status->external_filename);
    read_name =  (r_spstat->external_filename == CNULL) ? CNULL : 
	mtrf_strdup(r_spstat->external_filename);
    if (sp->open_mode == SP_mode_update) {
	if (sp_verbose > 10) fprintf(spfp,"Proc %s:  Mode SP_update\n",proc);
	if (w_spstat->write_occured_flag) {
	    /* if there has been a spwrite, then the waveform is written     */
	    /* as if it were a file opened for write */
	    /* Step 1: recursively call sp_close, changing the mode to write */
	    /* Step 2: delete the previous file                              */
	    /* Step 3: rename the temporary file                             */

	    if (sp_verbose > 15)
		fprintf(spfp,"Proc %s: Overwriting the original waveform\n",
		       proc);
	    sp->open_mode = SP_mode_write;
	    if ((ret=sp_close(sp)) != 0){
		unlink(write_name);
		if (write_name != CNULL) mtrf_free(write_name);
		if (read_name != CNULL) mtrf_free(read_name);
		return_child(proc,int,ret);
	    }
	    unlink(read_name);
	    rename(write_name,read_name);
	    if (write_name != CNULL) mtrf_free(write_name);
	    if (read_name != CNULL) mtrf_free(read_name);
	    return_success(proc,0,0,"ok");
	} else {
	    /* the header has been changed and the data mode of the waveform */
	    /* COULD BE CHANGED the waveform has not been modified in any    */
	    /* way, only the header has changed */

	    /* Step 1: write the header into the temporary file           */
	    /* Step 2: If the header changed in size OR the waveform      */
            /*         format has changed                                 */
	    /*            A: copy the waveform into the temp file         */
	    /*            B: close the files                              */
	    /*            C: delete the old file in favor of the new file */
	    /*         Else the header has not changed in size.           */
	    /*            A: write the header into the original file      */
	    /*            B: Close both files                             */
	    /*            C: delete the temporary file                    */
	    FILE *fp;
	    int samples_read, samples_written;

	    /* Step 1: */
	    spifr = sp->write_spifr;
	    fp = ((spifr->waveform->sp_fp != FPNULL) ?
		  (spifr->waveform->sp_fp) : 
		  ((spifr->waveform->sp_fob->fp != FPNULL) ?
		   (spifr->waveform->sp_fob->fp) : FPNULL));
	    if (sp_verbose > 15)
		fprintf(spfp,
			"Proc %s: Writing header to temp file. position %ld\n",
		       proc,ftell(fp));
	    if (fp == FPNULL){
		free_sphere_t(sp);
		unlink(write_name);
		if (write_name != CNULL) mtrf_free(write_name);
		if (read_name != CNULL) mtrf_free(read_name);
		return_err(proc,3000,3000,"Internal Error");
	    }
	    /* Write the header into the temporary file to compute the size  */
	    /* of the header and then rewind back over the just written header*/
	    rewind(fp);
	    if (sp_write_header(fp,spifr->status->file_header,
				&header_size,&data_size) < 0){
		free_sphere_t(sp);
		unlink(write_name);
		if (write_name != CNULL) mtrf_free(write_name);
		if (read_name != CNULL) mtrf_free(read_name);
		return_err(proc,3001,3001,
			   "Unable to update header in file");
	    }
	    rewind(fp);
	    /* Step 2   -  -  if the header size or waveform has changed */
	    if ((sp->read_spifr->waveform->header_data_size != header_size) || 
		(w_spstat->file_encoding != r_spstat->file_encoding) ||
		(w_spstat->file_compress != r_spstat->file_compress) ||
		(w_spstat->file_sbf != r_spstat->file_sbf) ||
		(w_spstat->channels != CHANNELSNULL)){
		char *buff;
		int ns, nc, in_nspb, out_nspb;
		if (sp_verbose > 15) {
		    fprintf(spfp,"Proc %s:   output header and/or",proc);
		    fprintf(spfp,"data has changed, copying file.\n");
		    fprintf(spfp,"Proc %s:       from %ld to %ld\n",proc, 
			   sp->read_spifr->waveform->header_data_size,
			   header_size);
		}
		ns = r_spstat->user_sample_count;
		nc = r_spstat->user_channel_count;
		in_nspb  = r_spstat->user_sample_n_bytes * 
		    r_spstat->user_channel_count;
		out_nspb = w_spstat->user_sample_n_bytes *
		     w_spstat->user_channel_count;
		if ((buff=mtrf_malloc(nc * in_nspb * 4096)) == CNULL) {
		    free_sphere_t(sp);
		    unlink(write_name);

		    if (write_name != CNULL) mtrf_free(write_name);
		    if (read_name != CNULL) mtrf_free(read_name);
		    return_err(proc,3010,3010,
			       "Unable to malloc transfer buffer space");
		}
		/* A: */
		do {
		    sp->open_mode = SP_mode_read;	
		    samples_read = sp_read_data(buff,4096,sp);
		    if (samples_read > 0) {
			sp->open_mode = SP_mode_write;
			samples_written = sp_write_data(buff,samples_read,sp);
			if (samples_written != samples_read){
			    free_sphere_t(sp);
			    unlink(write_name);
			    if (write_name != CNULL) mtrf_free(write_name);
			    if (read_name != CNULL) mtrf_free(read_name);
			    return_err(proc,3012,3012,
				       "Copy of waveform data failed");
			}
		    } else { 
			if (sp_eof(sp) == 0) {
			    free_sphere_t(sp);
			    unlink(write_name);
			    if (write_name != CNULL) mtrf_free(write_name);
			    if (read_name != CNULL) mtrf_free(read_name);
			    return_err(proc,3013,3013,
				  "Error: Zero samples read while not at EOF");
			}
			if (sp_error(sp) >= 100) {
			    /* a checksum error occured, close the sp and */
			    /* delete the temp file */
			    sp->open_mode = SP_mode_update;
			    sp_print_return_status(spfp);
			    free_sphere_t(sp);
			    unlink(write_name);
			    if (write_name != CNULL) mtrf_free(write_name);
			    if (read_name != CNULL) mtrf_free(read_name);
			    mtrf_free(buff);
			    return_err(proc,3011,3011,
				       "Error copying waveform data");
			}
		    }
		    sp->open_mode = SP_mode_update;
		} while (samples_read > 0);
		mtrf_free(buff);
		/* make sure the file is at eof (as if it were opened for   */
		/* read) */
		sp->open_mode = SP_mode_read;
		if (! sp_eof(sp)){
		    sp->open_mode = SP_mode_update;
		    free_sphere_t(sp);
		    unlink(write_name);
		    if (write_name != CNULL) mtrf_free(write_name);
		    if (read_name != CNULL) mtrf_free(read_name);
		    return_err(proc,3012,3012,"Error copying waveform data");
		}
		sp->open_mode = SP_mode_write;
		if ((ret=sp_close(sp)) != 0){
		    unlink(write_name);
		    if (write_name != CNULL) mtrf_free(write_name);
		    if (read_name != CNULL) mtrf_free(read_name);
		    return_child(proc,int,ret);
		}
		/* C: */
		unlink(read_name);
		rename(write_name,read_name);
		if (write_name != CNULL) mtrf_free(write_name);
		if (read_name != CNULL) mtrf_free(read_name);
		return_success(proc,0,0,"ok");
	    } else { 
		/* A: */
		spifr = sp->read_spifr;
		fp = ((spifr->waveform->sp_fp != FPNULL) ? 
		      (spifr->waveform->sp_fp) : 
		      ((spifr->waveform->sp_fob->fp != FPNULL) ?
		       (spifr->waveform->sp_fob->fp) : FPNULL));
		if (fp == FPNULL)
		    return_err(proc,3002,3002,"Internal Error");
		rewind(fp);
		if (sp_verbose > 15)
		    fprintf(spfp,
			 "Proc %s:   header size not changed.  position %ld\n",
			   proc,ftell(fp));
		if (sp_write_header(fp,w_spstat->file_header,
				    &header_size,&data_size) < 0){
		    free_sphere_t(sp);
		    unlink(write_name);
		    if (write_name != CNULL) mtrf_free(write_name);
		    if (read_name != CNULL) mtrf_free(read_name);
		    return_err(proc,3003,3003,
			       "Unable to update header in file");
		}
		/* B: */
		free_sphere_t(sp);
		/* C: */
		unlink(write_name);
		if (write_name != CNULL) mtrf_free(write_name);
		if (read_name != CNULL) mtrf_free(read_name);
		return_success(proc,0,0,"ok");
	    }
	}
    }

    /*  END of update mode file */

    if (sp->open_mode == SP_mode_write) {
	if (sp_verbose > 10) fprintf(spfp,
				     "Proc %s:  Mode SP_mode_write\n",proc);
	spifr = sp->write_spifr;
	/* flush the data to the file */
	if (spifr->status->is_disk_file)
	    fob_fflush(spifr->waveform->sp_fob);

	/* if the mode is write, update the sample_count and checksum */
	/* field if needed.  If the checksum field exists, verify it, */
	/* and warn if it's not the same */
	/************ ONLY UPDATE FIELDS IF THE FILE IS NOT A STREAM *********/
	if (spifr->status->is_disk_file){
	    h_get_field(w_spstat->file_header, SAMPLE_COUNT_FIELD,
			T_INTEGER, (void *)&lint);
	    if (spifr->waveform->samples_written != lint) {
		/* then update the field */
		lint = (SP_INTEGER) spifr->waveform->samples_written;
		spifr->status->file_sample_count = lint;
		/* temporarily reset the write occured flag to allow header  */
		/* modifications */
		w_spstat->write_occured_flag = FALSE;
		if (sp_h_set_field(sp,SAMPLE_COUNT_FIELD,T_INTEGER,&lint) !=0){
		    if (write_name != CNULL) mtrf_free(write_name);
		    if (read_name != CNULL) mtrf_free(read_name);
		    free_sphere_t(sp);
		    return_err(proc,200,200,"Unable to update sample_count");
		}
		/* Reset the write occured flag */
		w_spstat->write_occured_flag = TRUE;
		header_changed = TRUE;
	    }
	    if (h_get_field(spifr->status->file_header,SAMPLE_CHECKSUM_FIELD,
			    T_INTEGER,(void *)&lint) != 0){
		if (write_name != CNULL) mtrf_free(write_name);
		if (read_name != CNULL) mtrf_free(read_name);
		free_sphere_t(sp);
		return_err(proc,201,201,
			   "Unable to get sample_checksum from file header");
	    }
	    if (lint != spifr->status->file_checksum) {
		/* then the checksum was just computed, so install it */
		lint = (SP_INTEGER)spifr->waveform->checksum;
		/* temporarily reset the write occured flag to allow header  */
		/* modifications */
		w_spstat->write_occured_flag = FALSE;
		if (sp_h_set_field(sp,SAMPLE_CHECKSUM_FIELD,
				   T_INTEGER,&lint) >= 100){
		    if (write_name != CNULL) mtrf_free(write_name);
		    if (read_name != CNULL) mtrf_free(read_name);
		    free_sphere_t(sp);
		    return_err(proc,202,202,"Unable to update checksum");
		}
		/* Reset the write occured flag */
		w_spstat->write_occured_flag = TRUE;
		header_changed = TRUE;
	    } else 
		if (lint != spifr->waveform->checksum) {
		    spifr->waveform->failed_checksum = TRUE;	    
		    if (write_name != CNULL) mtrf_free(write_name);
		    if (read_name != CNULL) mtrf_free(read_name);
		    free_sphere_t(sp);
		    return_err(proc,203,203,
			       "Write verification of checksum failed");
		}
	}

	/* flush the updated header to the file  */
	if (header_changed) {
	    FILE *fp;
	    if (! spifr->status->is_disk_file) {
		if (write_name != CNULL) mtrf_free(write_name);
		if (read_name != CNULL) mtrf_free(read_name);
		free_sphere_t(sp);
		return_err(proc,301,301,
		   "Internal Error, header changed size on write to stdout");
	    }
	    fp = ((spifr->waveform->sp_fp != FPNULL) ? 
		  (spifr->waveform->sp_fp) : 
		  ((spifr->waveform->sp_fob->fp != FPNULL) ?
		   (spifr->waveform->sp_fob->fp) : FPNULL));
	    if (fp == FPNULL) {
		if (write_name != CNULL) mtrf_free(write_name);
		if (read_name != CNULL) mtrf_free(read_name);
		free_sphere_t(sp);
		return_err(proc,300,300,"Internal Error");
	    }
	    rewind(fp);
	    if (sp_write_header(fp,spifr->status->file_header,
				&header_size,&data_size) < 0) {
		if (write_name != CNULL) mtrf_free(write_name);
		if (read_name != CNULL) mtrf_free(read_name);		
		free_sphere_t(sp);
		return_err(proc,204,204,"Unable to update header in file");
	    }
        }
	
	if ((spifr->status->is_temp_file == FALSE) &&
	    fob_is_fp(spifr->waveform->sp_fob)) { 
	    /* check to make sure the blocking has not changed */
	    if (header_changed)
		/* check to see if we jumped header sizes */
		/* Modified by JGF 060496 */
		if (header_size > spifr->waveform->header_data_size){
		    if (write_name != CNULL) mtrf_free(write_name);
		    if (read_name != CNULL) mtrf_free(read_name);
		    free_sphere_t(sp);
		    return_err(proc,205,205,
			       "Header size has changed on update");    
		}
	} else {
	    if (spifr->status->user_compress == spifr->status->file_compress){
		if (fob_flush_to_fp(spifr->waveform->sp_fob,
				    spifr->waveform->sp_fp) != 0){
		    if (write_name != CNULL) mtrf_free(write_name);
		    if (read_name != CNULL) mtrf_free(read_name);
		    free_sphere_t(sp);
		    return_err(proc,206,206,"Unable to flush data to disk");
		}
	    } else { /* do some compression */
		FOB *comp_fob;
		/* 1. rewind the data */
		/* 2. alloc FOB to compress into */
		/* 3. compress the file */
		/* 4. free the allocated FOB */
		fob_rewind(spifr->waveform->sp_fob);
		if  ((comp_fob = fob_create(spifr->waveform->sp_fp)) ==
		     FOBPNULL) {
		    if (write_name != CNULL) mtrf_free(write_name);
		    if (read_name != CNULL) mtrf_free(read_name);
		    free_sphere_t(sp);
		    return_err(proc,207,207,"Unable to setup for compression");
		}
		spifr->waveform->sp_fp = FPNULL;
		switch (spifr->status->file_compress){
                  char message[70];
		  case SP_wc_shorten:
		    /* optimize the compression */
  	 	   shorten_reset_flags();
		   shorten_set_channel_count(spifr->status->file_channel_count);
		    if (spifr->status->file_encoding == SP_se_ulaw)
			shorten_set_ftype("au");
		    else if (spifr->status->file_encoding == SP_se_alaw)
			shorten_set_ftype("alaw");
		    else if (spifr->status->file_encoding == SP_se_pcm1)
			shorten_set_ftype("s8");
		    else if (spifr->status->file_encoding == SP_se_pcm2)
			if (spifr->status->file_sbf == SP_sbf_01)
			    shorten_set_ftype("s16lh");
			else
			    shorten_set_ftype("s16hl");
		    if (sp_verbose > 15) shorten_dump_flags(spfp);
  
	            if(setjmp(exitenv) == 0){
			if (shorten_compress(spifr->waveform->sp_fob, 
					     comp_fob, message) < 0){
			    fob_destroy(comp_fob);
			    if (write_name != CNULL) mtrf_free(write_name);
			    if (read_name != CNULL) mtrf_free(read_name);
			    free_sphere_t(sp);
			    return_err(proc,208,208,
				  rsprintf("Shorten Compression Failed - %s",
					   message));
			}
		    } else
			return_err(proc,213,213,"Shorten Compression Aborted");
		  
		    fob_fflush(comp_fob);
		    break;
		  case SP_wc_wavpack:
		    if(setjmp(exitenv) == 0){
			/* optimize the compression */
			wavpack_set_progname( "wavpack" );
			if (spifr->status->file_channel_count == 1)
			    wavpack_set_monoflg(TRUE);
			else
			    wavpack_set_monoflg(FALSE);
			wavpack_set_byteflg(spifr->status->file_sbf ==SP_sbf_1);
			if (sp_verbose > 15) wavpack_dump_interface(spfp);
			if (wavpack_pack(spifr->waveform->sp_fob, comp_fob)<0){
			    fob_destroy(comp_fob);
			    if (write_name != CNULL) mtrf_free(write_name);
			    if (read_name != CNULL) mtrf_free(read_name);
			    free_sphere_t(sp);
			    return_err(proc,209,209,
				       "Wavpack Compression Failed");
			}
			wavpack_free_progname();
			fob_fflush(comp_fob);
		    } else {
			return_err(proc,212,212,"Wavpack Compression Aborted");
		    }
		    break;
		  case SP_wc_shortpack:
		    return_err(proc,211,211,
			       "Unable to Compress using shortpack\n");
		  default:
		    if (write_name != CNULL) mtrf_free(write_name);
		    if (read_name != CNULL) mtrf_free(read_name);
		    free_sphere_t(sp);
		    return_err(proc,210,210,
			       "Unable to Compress the requested format\n");
		}
		spifr->waveform->sp_fp = comp_fob->fp;
		fob_destroy(comp_fob);
	    }
	}
	if ((sp->open_mode == SP_mode_write) ||
	    (sp->open_mode == SP_mode_update))
	    if (w_spstat->extra_checksum_verify)
		verify_checksum = TRUE;

    }


    free_sphere_t(sp);
    /*************************************************/
    /* The file is now completely written and closed */
    /*************************************************/

    /**************************************************/
    /*  If the write verification is requested, do it */
    if (verify_checksum) {
	if (strsame(write_name,"-")) {
	    if (write_name != CNULL) mtrf_free(write_name);
	    if (read_name != CNULL) mtrf_free(read_name);
	    return_warn(proc,1,1,
			"Unable to verify checksum, file went to STDOUT");
	}
	if (verify_file_checksum(write_name) != 0){
	    sp_print_return_status(spfp);
	    if (write_name != CNULL) mtrf_free(write_name);
	    if (read_name != CNULL) mtrf_free(read_name);
	    return_err(proc,1000,1000,
		       "Read Verification of written file failed");
	}
    }

    if (write_name != CNULL) mtrf_free(write_name);
    if (read_name != CNULL) mtrf_free(read_name);
    return_success(proc,0,0,"ok");
}

int verify_file_checksum(char *filename)
{
    char *proc="verify_file_checksum " SPHERE_VERSION_STR;
    SP_FILE *sp;
    char buf[1024];

    if (sp_verbose > 10) fprintf(spfp,"Proc %s:\n",proc);
    if (filename == CNULL) return_err(proc,100,100,"Null filename");
    
    if ((sp = sp_open(filename,"rv")) == SPNULL)
        return_err(proc,101,101,
		   rsprintf("Unable to open SPHERE file '%s'",filename));
    if (sp->read_spifr->status->user_sample_count > 0){
	if (sp_read_data(buf,1,sp) != 1){
	    sp_close(sp);
	    return_err(proc,200,200,"Verification of checksum failed");
	}
    } else {
	sp_close(sp);
	return_err(proc,300,300,"No data in file to check");
    }
	
    sp_close(sp);
    if (sp_verbose > 11) fprintf(spfp,"Proc %s: Returning 0\n",proc);
    return_success(proc,0,0,"Checksum verification passed");
}
