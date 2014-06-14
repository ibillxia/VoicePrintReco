/*********************************************************************/
/*   File: sprewind.c                                                */
/*   Desc: sp_rewind() goes back to the beginning of the waveform    */
/*         to permit multiple re-reads of the same waveform file     */
/*   Created: March 8, 1995                                          */
/*********************************************************************/


#include <sp/sphere.h>


int sp_rewind(SP_FILE *sp)
{
    char *proc="sp_rewind " SPHERE_VERSION_STR;

    if (sp_verbose > 10) fprintf(spfp,"Proc %s: Entering\n",proc);

    if (sp == SPNULL)
	return_err(proc,101,101,"Null SPFILE structure");

    if (sp->open_mode != SP_mode_read)
	return_err(proc,102,102,
		   "Unable rewind file not opened for reading");

    if (! sp->read_spifr->status->is_disk_file)
	return_err(proc,103,103, 
		   "Unable rewind file if opened as a pipe");

    if (sp->read_spifr->status->read_occured_flag == FALSE) {
	/* THERE IS NO NEED TO MOVE THE FILE POINTER */
	return_success(proc,0,0,"ok");
    }
	
    fob_rewind(sp->read_spifr->waveform->sp_fob);
    
    /* if the FOB is pointing to a file, then the header must be */
    /* skipped over */
    if (sp->read_spifr->waveform->sp_fob->fp != FPNULL){
	if (! sp->read_spifr->status->is_temp_file){
	    if (sp_verbose > 10) 
		fprintf(spfp,"Proc %s: Skipping header in file\n",proc);
	    if (fob_fseek(sp->read_spifr->waveform->sp_fob,
			  (long)sp->read_spifr->waveform->header_data_size,0) > 0)
		return_err(proc,104,104, 
			   "Rewind failed");
	} else {
	    if (sp_verbose > 10) 
		fprintf(spfp,"Proc %s: Rewinding temp file to beginning\n",proc);
	    if (fob_fseek(sp->read_spifr->waveform->sp_fob,0,0) > 0)
		return_err(proc,104,104, 
			   "Rewind failed");
	}
    }

    /* Reset to SP_FILE to pre-read status */
    sp->read_spifr->status->read_occured_flag = FALSE;
    free_SPIFR_waveform_buffers(sp->read_spifr);
    sp->read_spifr->waveform->checksum = 0;
    sp->read_spifr->waveform->samples_read = 0;
    sp->read_spifr->waveform->read_premature_eof = FALSE;
    sp->read_spifr->waveform->failed_checksum = FALSE;


    return_success(proc,0,0,"ok");
}

