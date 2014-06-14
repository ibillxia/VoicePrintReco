#include <stdio.h>
#define SPHERE_LIBRARY_CODE
#include <sp/sphere.h>

/*
 *  memory allocation/deallocation routines for the sphere_t 
 *  structure.
 *
 *  Returns:  a SP_FILE pointer upon success
 *            SPNULL upon failure
 */
SP_FILE *sp_alloc_and_init_sphere_t(void)
{
    SP_FILE *tsp;
    SPIFR *alloc_SPIFR(void);

    if (sp_verbose > 10) fprintf(spfp,"Proc sp_alloc_and_init_sphere_t:\n");
    /* alloc sphere_t structure */
    if ((tsp=(SP_FILE *)mtrf_malloc(sizeof(SP_FILE))) == SPNULL)
	return(SPNULL);

    tsp->open_mode = SP_mode_null;

    if ((tsp->read_spifr = alloc_SPIFR()) == SPIFRNULL){
	mtrf_free((char *)tsp);
	return(SPNULL);
    }
    if ((tsp->write_spifr = alloc_SPIFR()) == SPIFRNULL){
	free_SPIFR(tsp->read_spifr);
	mtrf_free((char *)tsp);
	return(SPNULL);
    }
    return(tsp);
}


SPIFR *alloc_SPIFR(void)
{
    SPIFR *tspifr;

    if ((tspifr=(SPIFR *)mtrf_malloc(sizeof(SPIFR))) == SPIFRNULL)
	return(SPIFRNULL);
    
    /* init the sphere_t structure */
    if ((tspifr->status=(struct spfile_status_t *)
	                mtrf_malloc(sizeof(struct spfile_status_t))) == 
        (struct spfile_status_t *)0){
	mtrf_free((char *)tspifr);
	return(SPIFRNULL);
    }
    if ((tspifr->waveform=(struct waveform_t *)
	                mtrf_malloc(sizeof(struct waveform_t))) == 
	(struct waveform_t *)0){
	mtrf_free((char *)tspifr->status);
	mtrf_free((char *)tspifr);
	return(SPIFRNULL);
    }
    tspifr->header = HDRNULL;

    /* init the waveform structure */
    tspifr->waveform->sp_fp = (FILE *)0;
    tspifr->waveform->sp_fob = FOBPNULL;
    tspifr->waveform->samples_read = 0;
    tspifr->waveform->samples_written = 0;
    tspifr->waveform->checksum = 0;
    tspifr->waveform->header_data_size = -1;
    tspifr->waveform->read_premature_eof = FALSE;
    tspifr->waveform->failed_checksum = FALSE;
    tspifr->waveform->waveform_setup = FALSE;
    tspifr->waveform->file_data_buffer_len = 0;
    tspifr->waveform->file_data_buffer = (void *)0;
    tspifr->waveform->converted_buffer_len = 0;
    tspifr->waveform->converted_buffer = (void *)0;
    tspifr->waveform->interleave_buffer_len = 0;
    tspifr->waveform->interleave_buffer = (void *)0;
    tspifr->waveform->byteswap_buffer_len = 0;
    tspifr->waveform->byteswap_buffer = (void *)0;

    /* init the status structure */
    tspifr->status->external_filename = CNULL;
    tspifr->status->file_header = HDRNULL;
    tspifr->status->extra_checksum_verify = FALSE;
    tspifr->status->is_disk_file = FALSE;
    tspifr->status->is_temp_file = FALSE;
    tspifr->status->temp_filename = CNULL;

    tspifr->status->user_channel_count = 0;
    tspifr->status->user_sample_count = 0;
    tspifr->status->user_sample_rate = 0; 
    tspifr->status->user_sample_n_bytes = 0;

    tspifr->status->file_channel_count = 0;
    tspifr->status->file_sample_count = 0;
    tspifr->status->file_sample_rate = 0; 
    tspifr->status->file_sample_n_bytes = 0;

    tspifr->status->file_checksum = (-1);
    tspifr->status->ignore_checksum = FALSE;

    tspifr->status->file_header = HDRNULL;
    tspifr->status->user_encoding = tspifr->status->file_encoding = SP_se_pcm2;
    tspifr->status->user_compress = tspifr->status->file_compress = SP_wc_none;
    tspifr->status->user_sbf = tspifr->status->file_sbf = SP_sbf_null;
    tspifr->status->natural_sbf = get_natural_sbf(2);
    tspifr->status->user_data_fmt = SP_df_raw;
    tspifr->status->channels = CHANNELSNULL;
    tspifr->status->write_occured_flag = FALSE;
    tspifr->status->read_occured_flag = FALSE;
    tspifr->status->field_set_occured_flag = FALSE;
    tspifr->status->set_data_mode_occured_flag = FALSE;

    return(tspifr);    
}

CHANNELS *alloc_CHANNELS(int num_chan, int max_chan_add){
    CHANNELS *tchannels;
    int c;

    if ((tchannels=(CHANNELS *)mtrf_malloc(sizeof(CHANNELS))) == CHANNELSNULL)
	return(CHANNELSNULL);
    tchannels->num_chan = 0;
    tchannels->max_num_chan = num_chan;
    tchannels->max_chan_add = max_chan_add;
    if ((tchannels->ochan=
	 (ORIGINATION_CHAN *)mtrf_malloc(sizeof(ORIGINATION_CHAN) * num_chan))
	== (ORIGINATION_CHAN *)0){
	mtrf_free((char *)tchannels);
	return(CHANNELSNULL);
    }
    for (c=0; c<num_chan; c++){
	if ((tchannels->ochan[c].orig_channel=
	     (int *)mtrf_malloc(sizeof(int) * max_chan_add)) == (int *)0){
	    mtrf_free((char *)tchannels->ochan);
	    mtrf_free((char *)tchannels);
	    return(CHANNELSNULL);
	}
	tchannels->ochan[c].num_origin = 0;
    }
    return(tchannels);
}

int free_CHANNELS(SPIFR *spifr){
    int c;

    if (spifr->status->channels == CHANNELSNULL)
	return(0);

    for (c=0; c<spifr->status->channels->max_num_chan; c++)
	mtrf_free((char *)spifr->status->channels->ochan[c].orig_channel);
    mtrf_free((char *)spifr->status->channels->ochan);
    mtrf_free((char *)spifr->status->channels);
    spifr->status->channels = CHANNELSNULL;
    return(0);
}

void free_SPIFR_waveform_buffers(SPIFR *spifr)
{
    if (spifr->waveform->file_data_buffer != (void *)0)
	mtrf_free((char *)spifr->waveform->file_data_buffer);
    spifr->waveform->file_data_buffer = (void *)0;
    spifr->waveform->file_data_buffer_len = 0;

    if (spifr->waveform->converted_buffer != (void *)0)
	mtrf_free((char *) spifr->waveform->converted_buffer);
    spifr->waveform->converted_buffer = (void *)0;
    spifr->waveform->converted_buffer = 0;
    
    if (spifr->waveform->interleave_buffer != (void *)0)
	mtrf_free((char *)spifr->waveform->interleave_buffer);
    spifr->waveform->interleave_buffer = (void *)0;
    spifr->waveform->interleave_buffer = 0;

    if (spifr->waveform->byteswap_buffer != (void *)0)
	mtrf_free((char *)spifr->waveform->byteswap_buffer);
    spifr->waveform->byteswap_buffer = (void *)0;
    spifr->waveform->byteswap_buffer = 0;
}

int free_SPIFR(SPIFR *spifr)
{
    FILE *fp=FPNULL;

    /* free the waveform structure */
    if (spifr->waveform->sp_fob != FOBPNULL){
	if (spifr->waveform->sp_fob->fp != FPNULL){
	    fp = spifr->waveform->sp_fob->fp;
	    fflush(fp);
	    if ((fp != stdin) && (fp != stdout)) fclose(fp);
	}
	fob_destroy(spifr->waveform->sp_fob);
    }

    if ((fp != FPNULL) && (fp == spifr->waveform->sp_fp))
	;
    else
	if (spifr->waveform->sp_fp != FPNULL){
	    fp = spifr->waveform->sp_fp;
	    fflush(fp);
	    if ((fp != stdin) && (fp != stdout)) fclose(fp);
	}

    if (spifr->status->is_temp_file && (spifr->status->temp_filename != CNULL))
	unlink(spifr->status->temp_filename);

    free_SPIFR_waveform_buffers(spifr);

    /* free the status structure */
    if (spifr->status->external_filename != CNULL)
	mtrf_free((char *)spifr->status->external_filename);
    if (spifr->status->file_header != HDRNULL)
	sp_close_header(spifr->status->file_header);
    if (spifr->status->temp_filename != CNULL)
	mtrf_free((char *)spifr->status->temp_filename);

    free_CHANNELS(spifr);

    /* the sphere_t data */
    if (spifr->header != HDRNULL)
	sp_close_header(spifr->header);
    if (spifr->waveform != (struct waveform_t *)0)
	mtrf_free((char *)spifr->waveform);
    if (spifr->status != (struct spfile_status_t *)0)
	mtrf_free((char *)spifr->status);
    mtrf_free((char *)spifr);
    return(0);
}


/*
 * Free all memory associated with a SP_FILE
 * Returns: -1 on failure
 *           0 on success
 */
int free_sphere_t(SP_FILE *sp)
{
    if (sp->read_spifr != SPIFRNULL)
        free_SPIFR(sp->read_spifr);
    if (sp->write_spifr != SPIFRNULL)
        free_SPIFR(sp->write_spifr);
    mtrf_free((char *)sp);	    
    return(0);
}
