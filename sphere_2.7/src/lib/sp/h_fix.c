#include <stdio.h>
#define SPHERE_LIBRARY_CODE
#include <sp/sphere.h>
#include <string.h>
#include <sys/stat.h>

int correct_out_of_date_headers(SP_FILE *sp)
{
    char *proc="correct_out_of_date_headers " SPHERE_VERSION_STR;
    struct header_t *header;
    struct stat fileinfo;
    SP_INTEGER samp_cnt, chan_cnt, samp_nb_cnt, new_samp_cnt;
    int changes_have_occured = FALSE;
    
    if (sp == SPNULL)
	return_err(proc,100,0,"Null SPFILE structure");

    /******* These corrections  only applies to files opened for read *******/
    if (sp->open_mode != SP_mode_read)
	return_err(proc,200,0,
		   "Tried to correct a header in a file opened for write");

    header = sp->read_spifr->header;

    /* check the sample count field to make sure it's the number of samples */
    /* per channel.  If not, correct the header now IF THE FILE is NOT a    */
    /* pipe.  Pipes are assumed NOT to have this characteristic.            */

    if (sp->read_spifr->status->is_disk_file){
	if (stat(sp->read_spifr->status->external_filename,&fileinfo) != 0)
	    return_err(proc,300,300,
		       rsprintf("Unable to stat file '%s'\n",
				sp->read_spifr->status->external_filename));
	/* get the sample_count, channel_count, sample_n_bytes fields from  */
	/* the header */
	if (h_get_field(header, SAMPLE_COUNT_FIELD,
			T_INTEGER, (void *)&samp_cnt) != 0)
	    return_err(proc,301,301,
	       rsprintf("Unable to extract %s field from header of file '%s'\n",
			SAMPLE_COUNT_FIELD,
			sp->read_spifr->status->external_filename));
	if (h_get_field(header, SAMPLE_N_BYTES_FIELD,
			T_INTEGER, (void *)&samp_nb_cnt) != 0)
	    return_err(proc,302,302,
	       rsprintf("Unable to extract %s field from header of file '%s'\n",
			SAMPLE_N_BYTES_FIELD,
			sp->read_spifr->status->external_filename));
	if (h_get_field(header, CHANNEL_COUNT_FIELD,
			T_INTEGER, (void *)&chan_cnt) != 0)
	    chan_cnt = 1;
	if (chan_cnt != 1){
	    if (samp_cnt * samp_nb_cnt == 
		fileinfo.st_size - sp->read_spifr->waveform->header_data_size) {
		/****** WE HAVE AN OUT-OF-DATE Header ******/
		/* change the sample count to be the number of samples per    */
		/* channel */
		new_samp_cnt = samp_cnt / chan_cnt;
		/* update the header IN memory! */
		if (h_set_field(header, SAMPLE_COUNT_FIELD,
				T_INTEGER, &new_samp_cnt) != 0)
		    return_err(proc,303,303,
			       rsprintf("%s field in file '%s'\n",
					"Unable to correct the sample_count",
  				    sp->read_spifr->status->external_filename));
		changes_have_occured = TRUE;
	    }
	}
    }

    if (changes_have_occured)
	if ((sp->read_spifr->waveform->header_data_size = 
	     sp_header_size(sp->read_spifr->header)) < 0){
	    return_err(proc,1000,1000,
		       rsprintf("Unable to get SPHERE header size%s",
				"of corrected header"));
	}
	
    return_success(proc,0,0,"ok");
}
