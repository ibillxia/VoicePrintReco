#include <sp/sphere.h>


int sp_tell(SP_FILE *sp)
{
    char *proc="sp_tell " SPHERE_VERSION_STR;

    if (sp == SPNULL)
	return_err(proc,101,-1,"Null SPFILE structure");

    if (sp->open_mode == SP_mode_read){ 
	return_success(proc,0,sp->read_spifr->waveform->samples_read,"ok");
    } else if (sp->open_mode == SP_mode_write) {
	return_success(proc,0,sp->write_spifr->waveform->samples_written,"ok");
    } else
	return_err(proc,101,-1,"file not opened for read or write");
}
