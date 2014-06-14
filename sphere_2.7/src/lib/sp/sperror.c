#include <stdio.h>
#define SPHERE_LIBRARY_CODE
#include <sp/sphere.h>
#include <string.h>

/*
 *
 *  sp_error()
 *
 */

int sp_error(SP_FILE *sp)
{
    char *proc_name="sp_error " SPHERE_VERSION_STR;
    SPIFR *spifr;

    if (sp == SPNULL)
	return_err(proc_name,200,200,rsprintf("Null SPFILE pointer"));
    if ((sp->open_mode == SP_mode_read) || (sp->open_mode == SP_mode_update)){
	spifr = sp->read_spifr;
	
	if (spifr->waveform->failed_checksum)
	    return_success(proc_name,0,100,
			   rsprintf("File '%s' had a checksum error",
				    spifr->status->external_filename)); 
	if (spifr->waveform->read_premature_eof)
	    return_success(proc_name,0,101,
			   rsprintf("Premature EOF on file '%s'",
				    spifr->status->external_filename));
	if (spifr->waveform->sp_fp != FPNULL) {
	    if (ferror(spifr->waveform->sp_fp) != 0)
		return_success(proc_name,0,102,
			       rsprintf("File '%s' has an error",
					spifr->status->external_filename));
	} else {
	    if (spifr->waveform->sp_fob == FOBPNULL)
		return_err(proc_name,103,103,
			   rsprintf("Empty File pointer for file '%s'",
				    spifr->status->external_filename));
	    if (fob_ferror(spifr->waveform->sp_fob) != 0)
		return_success(proc_name,0,104,
			       rsprintf("File '%s' has an error",
					spifr->status->external_filename));
	}	
    } 
    if ((sp->open_mode == SP_mode_write) || (sp->open_mode == SP_mode_update)){
	spifr = sp->write_spifr;
	
	if (spifr->waveform->failed_checksum)
	    return_success(proc_name,0,100,
			   rsprintf("File '%s' had a checksum error",
				    spifr->status->external_filename)); 
	if (spifr->waveform->sp_fp != FPNULL) {
	    if (ferror(spifr->waveform->sp_fp) != 0)
		return_success(proc_name,0,102,
			       rsprintf("File '%s' has an error",
					spifr->status->external_filename));
	} else {
	    if (spifr->waveform->sp_fob == FOBPNULL)
		return_err(proc_name,103,103,
			   rsprintf("Empty File pointer for file '%s'",
				    spifr->status->external_filename));
	    if (fob_ferror(spifr->waveform->sp_fob) != 0)
		return_success(proc_name,0,104,
			       rsprintf("File '%s' has an error",
					spifr->status->external_filename));
	}	
    }
    return_success(proc_name,0,0,"There was no file error");
}
