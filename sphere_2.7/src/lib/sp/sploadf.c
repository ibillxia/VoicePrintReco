#include <sp/sphere.h>

int sp_load_file(char *file, char *sdm, SP_INTEGER *nsamp, SP_INTEGER *nchan, SP_INTEGER *nsnb, void **data){
    SP_FILE *sp;
    char  *proc = "sp_load_file";
    int ret;

    if (file == (char *)0)
	return_err(proc,101,1,"Null filename");
    if (sdm == (char *)0) sdm = "";

    if ((sp=sp_open(file,"r")) == SPNULL) 
	return_err(proc,110,1,
		   rsprintf("sp_open failed to open file '%s'.  Message return"
			    "ed:\n%s\n",file,get_return_status_message()));

    if (sp_set_data_mode(sp,sdm) != 0)
	return_err(proc,111,1,
		   rsprintf("sp_set_data_mode failed.  Message return"
			    "ed:\n%s\n",file,get_return_status_message()));

    if (sp_h_get_field(sp,CHANNEL_COUNT_FIELD,T_INTEGER,(void *)nchan)!=0)
	return_err(proc,112,1,rsprintf("unable to get channel count from "
				       "file '%s'.  Message returned: %s\n",
				       file,get_return_status_message()));
    if (sp_h_get_field(sp,SAMPLE_COUNT_FIELD,T_INTEGER,(void *)nsamp)!=0)
	return_err(proc,113,1,rsprintf("unable to get sample count from "
				       "file '%s'.  Message returned: %s\n",
				       file,get_return_status_message()));
    if (sp_h_get_field(sp,SAMPLE_N_BYTES_FIELD,T_INTEGER,(void *)nsnb)!=0)
	return_err(proc,113,1,rsprintf("unable to get sample_n_bytes from "
				       "file '%s'.  Message returned: %s\n",
				       file,get_return_status_message()));
    if ((*data = (void *)sp_data_alloc(sp,-1))== (void *)0)
	return_err(proc,114,1,rsprintf("unable to alloc waveform memory."
				       "  Message returned: %s\n",
				       get_return_status_message()));
    
    if ((ret=sp_read_data(*data,*nsamp,sp)) != *nsamp)
	return_err(proc,115,1,rsprintf("unable to read waveform, exp samples "
				       "%d != %d.  Message returned: %s\n",
				       *nsamp,ret,
				       get_return_status_message()));
    if (sp_close(sp) != 0)
	return_err(proc,116,1,rsprintf("sp_close failed.  Message "
				       "returned: %s\n",
				       get_return_status_message()));
	
    return_success(proc,0,0,"ok");
}
