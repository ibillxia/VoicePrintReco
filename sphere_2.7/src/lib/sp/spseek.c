#include <sp/sphere.h>
#define BUF_SAMPLES 2048


int sp_seek(SP_FILE *sp, int offset, int origin)
{
    char *proc="sp_seek " SPHERE_VERSION_STR;
    char tb;
    int woff, n1, samples_read, sample;
    char *read_buf=CNULL;

    if (sp == SPNULL)
	return_err(proc,101,1,"Null SPFILE structure");
    if (sp->open_mode != SP_mode_read) 
	return_err(proc,102,1,"file not opened for read");

    /* do an intitial read to set up the buffers & such */
    if (!sp->read_spifr->status->read_occured_flag){
	sp_read_data(&tb,0, sp);
	if (sp_get_return_status() != 0)
	    return_err(proc,1000,1,
		       rsprintf("Initial read failed, returning: %s",
				get_return_status_message()));
    }

    switch (origin){
      case 0: 
	if (offset < 0)
	    return_err(proc,111,1,"Illegal negative offset for origin '0'");
	sample = offset; break;
      case 1:
	sample = offset + sp->read_spifr->waveform->samples_read;
	break;
      case 2:
	if (offset > 0)
	    return_err(proc,112,1,"Illegal positive offset for origin '2'");
	sample = offset + sp->read_spifr->status->file_sample_count;
	break;
      default:
	return_err(proc,113,1,rsprintf("Undefined origin value '%d'",origin));
    }
    if (sample < 0)
	sample = 0;

    if (sample > sp->read_spifr->status->file_sample_count)
	sample = sp->read_spifr->status->file_sample_count;

    if (sp->read_spifr->status->is_disk_file){
	/* compute the byte offset for the waveform data */
	woff = sample * sp->read_spifr->status->file_channel_count *
	    sp->read_spifr->status->file_sample_n_bytes;
	/* adjust for the header size */
	if (sp->read_spifr->waveform->sp_fob->fp != FPNULL)
	    woff += sp->read_spifr->waveform->header_data_size;
	
	/* Execute the fseek command */
	if (fob_fseek(sp->read_spifr->waveform->sp_fob,woff,0) != 0){
	    return_err(proc,103,1,"physical seek failed");
	}
	sp->read_spifr->waveform->samples_read = sample;
	if (sample > 0){
	    /* only disable the checksums if we move somewhere other than
	       sample 0 */
	    sp->read_spifr->status->ignore_checksum = TRUE;
	} else {
	    sp->read_spifr->waveform->checksum = 0;
	    sp->read_spifr->waveform->failed_checksum = FALSE;
	}

	return_success(proc,0,0,"ok");
    } else {
	/* It's a pipe, therefore actually read through the data */

	samples_read = sp->read_spifr->waveform->samples_read;
	if (sample < samples_read)
	    return_err(proc,110,1,"pipe'd-file beyond current position");

	/* allocate memory for the reading buffer */
	if ((read_buf=(char *)sp_data_alloc(sp,BUF_SAMPLES)) == CNULL)
	    return_err(proc,111,1,"buffer alloc failed");
	
	/* First skip the number of samples requested */
	while (samples_read < sample){
	    n1 = sp_read_data((char *)read_buf,
			      (samples_read + BUF_SAMPLES < sample) ? 
			      BUF_SAMPLES :
			      sample - samples_read,sp);
	    if (n1 == 0 && sp_eof(sp) == 0){
		sp_data_free(sp,read_buf);
		return_err(proc,112,1,"pre-mature EOF");
	    }
	    samples_read += n1;
	}
	sp->read_spifr->waveform->samples_read = sample;
	sp_data_free(sp,read_buf);
	return_success(proc,0,0,"ok");
    }
}
