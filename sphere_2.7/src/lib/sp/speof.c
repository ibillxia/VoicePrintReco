#include <stdio.h>
#define SPHERE_LIBRARY_CODE
#include <sp/sphere.h>
#include <string.h>

/*
 *
 *  sp_eof()
 *
 */

int sp_eof(SP_FILE *sp)
{
    char *proc_name="sp_eof " SPHERE_VERSION_STR;

    if (sp == SPNULL)
	return_err(proc_name,100,0,rsprintf("Null SPFILE pointer"));
    if (sp->open_mode == SP_mode_write)
	return_err(proc_name,101,0,
		   rsprintf("File '%s' not opened for read",
			    sp->write_spifr->status->external_filename));

    if (sp->read_spifr->waveform->sp_fob != FOBPNULL) {
	if (fob_feof(sp->read_spifr->waveform->sp_fob) != 0)
	    return_success(proc_name,0,100,
			   rsprintf("File '%s' is at EOF",
				    sp->read_spifr->status->external_filename));
	return_success(proc_name,0,0,
		       rsprintf("File '%s' is NOT at EOF",
				sp->read_spifr->status->external_filename));
    } else {
	if (sp->read_spifr->waveform->sp_fp == FPNULL)
	    return_err(proc_name,102,0,
		       rsprintf("Empty File pointer for file '%s'",
				sp->read_spifr->status->external_filename));
	if (feof(sp->read_spifr->waveform->sp_fp) != 0)
	    return_success(proc_name,0,100,
			   rsprintf("File '%s' is at EOF",
				    sp->read_spifr->status->external_filename));
	return_success(proc_name,0,0,
		       rsprintf("File '%s' is NOT at EOF",
				sp->read_spifr->status->external_filename));
    }	

}


