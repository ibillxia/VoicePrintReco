#include <stdio.h>
#define SPHERE_LIBRARY_CODE
#include <sp/sphere.h>
#include <string.h>

#define READMODE "r"
#define UPDATEMODE "r+"
#define WRITEMODE "w"


/*
 *  sp_open
 *
 */
SP_FILE *sp_open(char *filename, char *mode)
{
    SP_FILE *tsp;
    char *errmsg, *proc="sp_open " SPHERE_VERSION_STR, *fopen_mode;
    enum SP_file_open_mode current_mode;

    if (sp_verbose > 10) fprintf(spfp,"Proc %s:\n",proc);
    if (filename == CNULL) 
	return_err(proc,101,SPNULL,"Null filename string");
    if (mode == CNULL) 
	return_err(proc,101,SPNULL,"Null file mode string");

    if (sp_verbose > 10) fprintf(spfp,"Proc %s: file '%s' mode '%s'\n",
				 proc,filename,mode);

    if ((tsp=sp_alloc_and_init_sphere_t()) == SPNULL)
        return_err(proc,102,SPNULL,"Unable to malloc SPFILE memory");

    /* set the file open mode in the status structure */
    if (strsame(mode,"r"))
	current_mode = tsp->open_mode = SP_mode_read;
    else if (strsame(mode,"w"))
	current_mode = tsp->open_mode = SP_mode_write;
    else if (strsame(mode,"rv")){
	current_mode = tsp->open_mode = SP_mode_read;
	tsp->read_spifr->status->extra_checksum_verify = TRUE;
    }
    else if (strsame(mode,"wv")) {
	current_mode = tsp->open_mode = SP_mode_write;
	tsp->write_spifr->status->extra_checksum_verify = TRUE;
    }
    else if (strsame(mode,"u")) {
	tsp->open_mode = SP_mode_read;
	current_mode = SP_mode_update;
    }
    else {
	free_sphere_t(tsp);
        return_err(proc,103,SPNULL,
		   rsprintf("Illegal SPFILE open mode '%s'",mode));
    }

    /* just open the file, either for reading or writing.  If the      */
    /* mode was SP_mode_writing, and the file exists, change the mode  */ 
    /* to SP_mode_update.                                              */
    switch (tsp->open_mode) {
	case (SP_mode_read): {
	    if (! strsame(filename,"-")) {
		fopen_mode = (current_mode == SP_mode_read) ? READMODE : 
		    UPDATEMODE;
		if ((tsp->read_spifr->waveform->sp_fp = 
		     fopen(filename,fopen_mode)) == (FILE *)0){
		    free_sphere_t(tsp);
		    return_err(proc,111,SPNULL,
			       rsprintf("%s'%s' for reading fopen mode %s",
					"Unable to open SPHERE file ",
					filename,fopen_mode));
		}
		tsp->read_spifr->status->is_disk_file = TRUE;
	    } else {
		tsp->read_spifr->waveform->sp_fp = stdin;
		tsp->read_spifr->status->is_disk_file = FALSE;
	    }
	    tsp->read_spifr->status->external_filename =
		mtrf_strdup(filename);
	    break;
	}
	case (SP_mode_write):{ 
	    if (! strsame(filename,"-")) {
		/* open the file, truncating if the file exists */
		fopen_mode = (current_mode == SP_mode_write) ? WRITEMODE : 
		    UPDATEMODE;
		if ((tsp->write_spifr->waveform->sp_fp =
		     fopen(filename,fopen_mode)) == (FILE *)0){
		    free_sphere_t(tsp);
		    return_err(proc,105,SPNULL,
			rsprintf("Unable to open SPHERE file '%s' for %s %s",
				 filename,"writing, fopen mode",fopen_mode));
		}
		tsp->write_spifr->status->is_disk_file = TRUE;
	    } else {
		tsp->write_spifr->waveform->sp_fp = stdout;
		tsp->write_spifr->status->is_disk_file = FALSE;
	    }
	    tsp->write_spifr->status->external_filename = 
		mtrf_strdup(filename);
	    break;
	}
	default: {
	    return_err(proc,200,SPNULL,"Internal error");
	}
    }

    /* now that the file is opened, load the header if it exist, */
    /* otherwise alloc an empty header for the user              */
    switch (tsp->open_mode) {
	case (SP_mode_read): {
	    /* read the header */
	    tsp->read_spifr->header =
	       sp_open_header(tsp->read_spifr->waveform->sp_fp,TRUE,&errmsg);
	    if ( tsp->read_spifr->header == HDRNULL ) {
		free_sphere_t(tsp);				
		return_err(proc,104,SPNULL,
		   rsprintf("Unable to open SPHERE header of file '%s', %s",
			    filename,errmsg));
	    }
	    /* get the size of the header */
	    if (! strsame(filename,"-")) {
		if ((tsp->read_spifr->waveform->header_data_size = 
		     sp_file_header_size(filename)) < 0){
		    free_sphere_t(tsp);
		    return_err(proc,110,SPNULL,
		    rsprintf("Unable to get SPHERE header size of file '%s'",
				filename));
		}
	    } else {
		if ((tsp->read_spifr->waveform->header_data_size =
		     sp_header_size(tsp->read_spifr->header)) < 0){
		    free_sphere_t(tsp);
		    return_err(proc,111,SPNULL,
		    rsprintf("Unable to get SPHERE header size of file '%s'",
			filename));
		}
	    }
	    /****** Remove the sample_count field if it's       ******/
	    /****** value is 999999999 and the file is a stream ******/
	    /****** Added by JGF June 22, 1994                  ******/
	    if (! tsp->read_spifr->status->is_disk_file){
		int type, size;
		SP_INTEGER l_int;
		if (sp_get_field(tsp->read_spifr->header,
				 SAMPLE_COUNT_FIELD,&type,&size) == 0){
		  if (sp_get_data(tsp->read_spifr->header,SAMPLE_COUNT_FIELD,
				  (char *)&l_int,&size) == 0){
		    if (l_int == 999999999){
		      if (sp_verbose > 10)
			fprintf(spfp,
				"Proc %s: file '%s' deleting %s field\n",
				proc,filename,SAMPLE_COUNT_FIELD);
		      if (sp_delete_field(tsp->read_spifr->header,
					  SAMPLE_COUNT_FIELD) < 0)
			return_err(proc,112,SPNULL,
				   rsprintf("Unable to delete fake '%s' field",
					    SAMPLE_COUNT_FIELD));
			
		    }
		  }
	      }
	    }	    

	    /****** Correct any out-of-date headers right NOW ******/
	    if (correct_out_of_date_headers(tsp) != 0){
		fprintf(spfp,"Warning: correction of ");
		fprintf(spfp,"out-of-date headers failed\n");
		sp_print_return_status(spfp);
	    }

            /* duplicate the header for the file interface */
            if ((tsp->read_spifr->status->file_header = 
		 sp_dup_header(tsp->read_spifr->header)) == HDRNULL){
		fprintf(spfp,"Error: sp_open_header unable ");
		fprintf(spfp,"to dup header for file '%s'\n",filename);
		free_sphere_t(tsp);		
		return_err(proc,106,SPNULL,
		   rsprintf("Unable to duplicate the SPHERE header of file",
			    filename));
	    }
            /* set the default operation settings */
	    if (sp_set_default_operations(tsp) != 0){
	      print_return_status(spfp);
	      return_err(proc,107,SPNULL,
	       rsprintf("Unable to interpret the SPHERE header of file '%s'",
			filename));
	    }
	    break;	
	}
	case (SP_mode_write):{ 
	    tsp->write_spifr->header = sp_create_header();
	    if ( tsp->write_spifr->header == HDRNULL ) {
		free_sphere_t(tsp);		
		return_err(proc,108,SPNULL,
		   rsprintf("Unable to allocate SPHERE header for file '%s'",
			    filename));
	    }
	    tsp->write_spifr->status->file_header = sp_create_header();
	    if ( tsp->write_spifr->status->file_header == HDRNULL ) {
	     free_sphere_t(tsp);		
	     return_err(proc,109,SPNULL,
 	     rsprintf("Unable to allocate hidden SPHE. header for file '%s'",
				    filename));
	    }
	}
	default: 
	  ;
    }

    /* the file was actually opened for update, so make a temp file, and */
    /* duplicate the read in file. */
    if (current_mode == SP_mode_update){ 
	SP_FILE *tsp2;
	SPIFR *tspifr;
	char *temp_file;
	char data_mode[100];
	temp_file = sptemp(tsp->read_spifr->status->external_filename);
	if (temp_file == CNULL)
	   return_err(proc,300,SPNULL,"Unable to create temporary filename");
	if ((tsp2 = sp_open(temp_file,WRITEMODE)) == SPNULL) {
	    free_sphere_t(tsp);		
	    mtrf_free(temp_file);
	    return_err(proc,301,SPNULL,
		       rsprintf("Unable to open temporary file",temp_file));
	}
	/* NOT NECESSARY TO BE DONE J.Thompson */
	/* sp_set_data_mode(tsp,"SE-ORIG:SBF-ORIG"); */
	/* now copy the header into the update file */
	if (sp_copy_header(tsp,tsp2) != 0){
	    free_sphere_t(tsp);		
	    free_sphere_t(tsp2);		
	    unlink(temp_file);
	    mtrf_free(temp_file);
	    return_err(proc,302,SPNULL,"Unable to duplicate output header");
	}
	*data_mode = '\0';
	/*now set the data mode to match the output format of the read file*/
	switch (tsp->read_spifr->status->file_compress){
	  case SP_wc_shorten: strcat(data_mode,"SE-SHORTEN:"); break;
	  case SP_wc_wavpack: strcat(data_mode,"SE-WAVPACK:"); break;
	  case SP_wc_shortpack: strcat(data_mode,"SE-SHORTPACK:"); break;
	  default: ;
	}
	switch (tsp->read_spifr->status->file_sbf){
	  case SP_sbf_01: strcat(data_mode,"SBF-01"); break;
	  case SP_sbf_10: strcat(data_mode,"SBF-10"); break;
	  case SP_sbf_1: strcat(data_mode,"SBF-1"); break;
	  case SP_sbf_N: strcat(data_mode,"SBF-N"); break;
	  default: ;
	}
	if (sp_set_data_mode(tsp2,data_mode) >= 100){
	    free_sphere_t(tsp);		
	    free_sphere_t(tsp2);		
	    unlink(temp_file);
	    mtrf_free(temp_file);
	    return_err(proc,303,SPNULL,
	       rsprintf("Unable to set_data_mode '%s' for update file",
			data_mode));
	}
	/* now merge the two SPFILE pointers into a single structure */
	/* and free the residual */
	tspifr = tsp->write_spifr;
	tsp->write_spifr = tsp2->write_spifr;
	tsp2->write_spifr = tspifr;
	free_sphere_t(tsp2);
	mtrf_free(temp_file);	
	tsp->open_mode = current_mode;
    }

/*    sp_file_dump(tsp,spfp);*/
    if (sp_verbose > 17) sp_file_dump(tsp,spfp);
    if (sp_verbose > 11)
	fprintf(spfp,"Proc %s: Returning Sphere-file pointer\n",proc);
    return_success(proc,0,tsp,"ok");
}

/**** This function is exempt from the requirement of using spfp ****/
void sp_file_dump(SP_FILE *sp, FILE *fp)
{
    fprintf(fp,"|==========================================");
    fprintf(fp,"========================\n");
    fprintf(fp,"File open mode:      %s\n",
	    enum_str_SP_file_open_mode(sp->open_mode));
    if ((sp->open_mode == SP_mode_read) || 
	(sp->open_mode == SP_mode_update)){
	fprintf(fp,"Read SPIFR:\n");
	spifr_dump(sp->read_spifr,fp);
    }
    if ((sp->open_mode == SP_mode_write) ||
	(sp->open_mode == SP_mode_update)){
	fprintf(fp,"Write SPIFR:\n");
	spifr_dump(sp->write_spifr,fp);
    }
    fprintf(fp,"|==========================================");
    fprintf(fp,"========================\n");
}

/**** This function is exempt from the requirement of using spfp ****/
void spifr_dump(SPIFR *spifr, FILE *fp)
{
    fprintf(fp,"|------------------------------------------");
    fprintf(fp,"-----------------------------\n|\n");
    fprintf(fp,"Dump of an SP_FILE structure\n");
    fprintf(fp,"Users file header\n");
    sp_print_lines(spifr->header,fp);
    fprintf(fp,"\n");
    fprintf(fp,"Wave Sructure\n");
    fprintf(fp,"File pointer:     %p\n",spifr->waveform->sp_fp);
    fprintf(fp,"FOB pointer:      %p\n",spifr->waveform->sp_fob);
    fprintf(fp,"Samples Read:     %d\n",spifr->waveform->samples_read);
    fprintf(fp,"Samples written:  %d\n",spifr->waveform->samples_written);
    fprintf(fp,"Checksum:         %d\n",spifr->waveform->checksum);
    fprintf(fp,"Header Data Size: %ld\n",spifr->waveform->header_data_size);
    fprintf(fp,"Read Pre-Mat. EOF %d\n",spifr->waveform->read_premature_eof);
    fprintf(fp,"Failed Checksum   %d\n",spifr->waveform->failed_checksum);
    fprintf(fp,"Waveform Setup    %d\n",spifr->waveform->waveform_setup);
    fprintf(fp,"File Tran. Len    %d\n",
	    spifr->waveform->file_data_buffer_len);
    fprintf(fp,"File Tran. Buf.   %p\n",
	    spifr->waveform->file_data_buffer);
    fprintf(fp,"Code Tran. Len    %d\n",
	    spifr->waveform->converted_buffer_len);
    fprintf(fp,"Code Tran. Buf.   %p\n",
	    spifr->waveform->converted_buffer);
    fprintf(fp,"Interkleave Len   %d\n",
	    spifr->waveform->interleave_buffer_len);
    fprintf(fp,"Interleave Buf.   %p\n",
	    spifr->waveform->interleave_buffer);
    
    fprintf(fp,"\n");

    fprintf(fp,"Status Structure\n");
    fprintf(fp,"External file name:  %s\n",
	    spifr->status->external_filename);
    fprintf(fp,"The File header\n");
    sp_print_lines(spifr->status->file_header,fp);
    fprintf(fp,"Write Occured Flag:  %d\n",
	    spifr->status->write_occured_flag);
    fprintf(fp,"Read Occured Flag:   %d\n",
	    spifr->status->read_occured_flag);
    fprintf(fp,"Field Set Occ. Flag: %d\n",
	    spifr->status->field_set_occured_flag);
    fprintf(fp,"S_D_MODE Occ. Flg:   %d\n",
	    spifr->status->set_data_mode_occured_flag);
    fprintf(fp,"File checksum:       %d\n",
	    spifr->status->file_checksum);   
    fprintf(fp,"Ignore checksum:     %d\n",
	    spifr->status->ignore_checksum);   
    fprintf(fp,"Nat Sample Byte Fmt: %s\n",
	    enum_str_SP_sample_byte_fmt(spifr->status->natural_sbf));
    fprintf(fp,"Extra Checksum Check %d\n",
	    spifr->status->extra_checksum_verify);
    fprintf(fp,"Is Disk File         %d\n",spifr->status->is_disk_file);
    fprintf(fp,"Is Temp File         %d\n",spifr->status->is_temp_file);
    fprintf(fp,"Temp File Name       %s\n",spifr->status->temp_filename);

    if (spifr->status->channels != CHANNELSNULL) {
	int outc, orgc;
	CHANNELS *Channels=spifr->status->channels;
	fprintf(spfp,"Channel Structure:\n");
	for (outc=0; outc<Channels->num_chan; outc++){
	    fprintf(fp,"     Channel %d: #sources=%d ",
		    outc,Channels->ochan[outc].num_origin);
	    for (orgc=0; orgc < Channels->ochan[outc].num_origin; orgc++)
		fprintf(fp," %s%d",(orgc >= 1) ? "+ " : "",
		       Channels->ochan[outc].orig_channel[orgc]);
	    fprintf(fp,"\n");
	}
	fprintf(fp,"\n");
    }

    fprintf(fp,"                                   USER");
    fprintf(fp,"                    FILE\n");
    fprintf(fp,"Channel count:    %22d  %22d\n",
	    spifr->status->user_channel_count,
	    spifr->status->file_channel_count);
    fprintf(fp,"Sample Count:     %22d  %22d\n",
	    spifr->status->user_sample_count,
	    spifr->status->file_sample_count);
    fprintf(fp,"Sample Rate:      %22d  %22d\n",
	    spifr->status->user_sample_rate, 
	    spifr->status->file_sample_rate); 
    fprintf(fp,"Sample N bytes:   %22d  %22d\n",
           spifr->status->user_sample_n_bytes,
	    spifr->status->file_sample_n_bytes);
    fprintf(fp,"Sample Byte Fmt:  %22s  %22s\n",
	    enum_str_SP_sample_byte_fmt(spifr->status->user_sbf),
	    enum_str_SP_sample_byte_fmt(spifr->status->file_sbf));
    fprintf(fp,"File Coding:      %22s  %22s\n",
	    enum_str_SP_sample_encoding(spifr->status->user_encoding),
	    enum_str_SP_sample_encoding(spifr->status->file_encoding));
    fprintf(fp,"File Compress:    %22s  %22s\n",
	    enum_str_SP_waveform_comp(spifr->status->user_compress),
	    enum_str_SP_waveform_comp(spifr->status->file_compress));
    fprintf(fp,"Data Format:      %22s\n",
	    enum_str_SP_data_format(spifr->status->user_data_fmt));
    fprintf(fp,"|\n|--------------------------------------");
    fprintf(fp,"---------------------------------\n");

}

int sp_set_default_operations(SP_FILE *sp)
{
    SP_INTEGER l_int;
    char *str, *proc="sp_set_default_operations " SPHERE_VERSION_STR;
    SPIFR *spifr;
    int ret;

    if (sp_verbose > 10) fprintf(spfp,"Proc %s:\n",proc);
    if (sp == SPNULL) return_err(proc,100,100,"Null SPFILE pointer");
    if ((sp->open_mode == SP_mode_read) || (sp->open_mode == SP_mode_update))
        spifr = sp->read_spifr;
    else if (sp->open_mode == SP_mode_write)
        spifr = sp->write_spifr;
    else
        return_err(proc,100,100,"Unknown File Mode");

    /**********************************************************************/
    /*    The following fields are REQUIRED for Read operations,          */
    /*                      NO Exceptions                                 */

    if (sp_h_get_field(sp,SAMPLE_COUNT_FIELD,T_INTEGER,(void *)&l_int) != 0){
	/* ----------  OLD CODE  ---------------------
	       return_err(proc,101,101,rsprintf("Missing '%s' header field",
	                                        SAMPLE_COUNT_FIELD));
	    spifr->status->user_sample_count =
		spifr->status->file_sample_count = (int)l_int;*/
        /*  added a condition to permit a pipe to NOT have a sample_count */
	/*  field.  ADDED June 22, 1993   */ 
	if (spifr->status->is_disk_file){
	    return_err(proc,101,101,rsprintf("Missing '%s' header field",
					     SAMPLE_COUNT_FIELD));
	} else { /* Sample counts may be missing from piped files */
	    spifr->status->user_sample_count =
		spifr->status->file_sample_count = 999999999;	    
	} 
    } else
	spifr->status->user_sample_count =
	    spifr->status->file_sample_count = (int)l_int;
    if (l_int <= 0)
	return_err(proc,108,108,
		   rsprintf("Field '%s' value out of range, %d <= 0",
			    SAMPLE_COUNT_FIELD,l_int));

    if (sp_h_get_field(sp,SAMPLE_N_BYTES_FIELD,
		       T_INTEGER,(void *)&l_int) != 0)
	return_err(proc,104,104,
		   rsprintf("Missing '%s' header field",
			    SAMPLE_N_BYTES_FIELD));
    spifr->status->user_sample_n_bytes = 
	spifr->status->file_sample_n_bytes = (int)l_int;
    if (l_int <= 0)
	return_err(proc,108,108,
		   rsprintf("Field '%s' value out of range, %d <= 0",
			    SAMPLE_N_BYTES_FIELD,l_int));

    if (sp_h_get_field(sp,CHANNEL_COUNT_FIELD,T_INTEGER,(void *)&l_int) != 0)
	return_err(proc,105,105,
		   rsprintf("Missing '%s' header field",
			    CHANNEL_COUNT_FIELD));
    spifr->status->user_channel_count = spifr->status->file_channel_count =
	(int)l_int;
    if (l_int <= 0)
	return_err(proc,108,108,
		   rsprintf("Field '%s' value out of range, > 0",
			    CHANNEL_COUNT_FIELD,l_int));

    /**********************************************************************/
    /* The following fields may exist, if they do not, there is a default */

    /***** NOTE:  only set the file_sbf, Sp_set_data_mode is called to    */
    /***** set the user_sbf                                               */
    if ((ret=sp_h_get_field(sp,SAMPLE_BF_FIELD,T_STRING,(void *)&str)) != 0){
	if ((spifr->status->file_sbf = 
	     get_natural_sbf(spifr->status->user_sample_n_bytes)) == 
	    SP_sbf_null)
	    return_err(proc,107,107,
		       rsprintf("Unable to read sample sizes of %d bytes",
				spifr->status->user_sample_n_bytes));
    } else {  /* str holds the sample_byte_format_value */
	ret = parse_sample_byte_format(str,&(spifr->status->file_sbf));
	if (ret == 1000) { 
	    /* then the file was compressed using change_wav_format */
	    spifr->status->file_sbf =
		get_natural_sbf(spifr->status->user_sample_n_bytes);
	    spifr->status->file_compress = SP_wc_shortpack;

	    if ((h_set_field(spifr->header,SAMPLE_BF_FIELD,
			     T_STRING,get_natural_byte_order(2)) != 0) ||
		(h_set_field(spifr->status->file_header,SAMPLE_BF_FIELD,
			     T_STRING,get_natural_byte_order(2)) != 0)){
		sp_print_return_status(spfp);
		mtrf_free(str);
		return_err(proc,110,110,
			   rsprintf("Unable to re-set sample byte format%s",
				    "field for a shortpacked file"));
	    }
	    if ((h_set_field(spifr->header,SAMPLE_CODING_FIELD,T_STRING,
			     rsprintf("pcm,embedded-%s",str)) != 0) ||
		(h_set_field(spifr->status->file_header,SAMPLE_CODING_FIELD,
			     T_STRING,rsprintf("pcm,embedded-%s",str)) !=0)){
		sp_print_return_status(spfp);
		mtrf_free(str);
		return_err(proc,111,111,
			   rsprintf("Unable to re-set sample coding field%s",
				    "for a shortpacked file"));
	    }
	} else if (ret != 0) { /* there really was an error */
	    mtrf_free(str);
	    return_err(proc,106,106,
		       "Unable to parse the 'sample_byte_format' field");
	}
	mtrf_free(str);
    }
              /***** field break *****/	    
    if (sp_h_get_field(sp,SAMPLE_CODING_FIELD,T_STRING,(void *)&str) != 0)
	if (spifr->status->user_sample_n_bytes == 1)
	    str = mtrf_strdup("ulaw");
	else
	    /* the default, since old Corpora are missing this field */
	    str = mtrf_strdup("pcm"); 
    
    if (parse_sample_coding(str,spifr->status->file_sample_n_bytes,
			    &(spifr->status->file_encoding),
			    &(spifr->status->file_compress)) != 0){
	mtrf_free(str);
	print_return_status(spfp);
	return_err(proc,107,107,
	   rsprintf("Unable to parse sample_coding value '%s' header field",
			    str));
    }
    mtrf_free(str);

    /*********************************************************************/
    /*    The following fields are conditionally required.               */
    ret = sp_h_get_field(sp,SAMPLE_RATE_FIELD,T_INTEGER,(void *)&l_int);
    switch (spifr->status->file_encoding) {
      case SP_se_pcm1:
      case SP_se_pcm2:
      case SP_se_ulaw:
	if (ret != 0)
	    return_err(proc,102,102,
		       rsprintf("Header field '%s' missing, but required%s",
				SAMPLE_RATE_FIELD," for waveform data"));
	spifr->status->user_sample_rate = spifr->status->file_sample_rate =
	    (int)l_int;
	break;
      case SP_se_pculaw:
	if (ret != 0)
	    return_err(proc,1022,1022,
		       rsprintf("Header field '%s' missing, but required%s",
				SAMPLE_RATE_FIELD," for waveform data"));
	spifr->status->user_sample_rate = spifr->status->file_sample_rate =
	    (int)l_int;
	break;
      case SP_se_alaw:
	if (ret != 0)
	    return_err(proc,1021,1021,
		       rsprintf("Header field '%s' missing, but required%s",
				SAMPLE_RATE_FIELD," for waveform data"));
	spifr->status->user_sample_rate = spifr->status->file_sample_rate =
	    (int)l_int;
	break;
      case SP_se_raw:
      default:
	spifr->status->user_sample_rate = spifr->status->file_sample_rate =0;
    }
	  
    /***********************************************************************/
    /*    The following fields are OPTIONAL, but if they exist, there is a */
    /*    special purpose for them                                         */

    if (sp_h_get_field(sp,SAMPLE_CHECKSUM_FIELD,T_INTEGER,(void *)&l_int)==0)
	spifr->status->file_checksum = l_int;
    else {
	spifr->status->ignore_checksum = TRUE;
    }

    /*********************/
    /* Consitency checks */
    /*********************/

    if (spifr->status->file_encoding == SP_se_ulaw &&
	spifr->status->file_sample_n_bytes != 1) 
	return_err(proc,120,120,
		   rsprintf("Ulaw encoding requires a 1 byte sample,%s %d",
			    " however the header value is",
			    spifr->status->file_sample_n_bytes));
    if (spifr->status->file_encoding == SP_se_pculaw &&
	spifr->status->file_sample_n_bytes != 1) 
	return_err(proc,120,120,
		   rsprintf("Pculaw encoding requires a 1 byte sample,%s %d",
			    " however the header value is",
			    spifr->status->file_sample_n_bytes));
    if (spifr->status->file_encoding == SP_se_alaw &&
	spifr->status->file_sample_n_bytes != 1) 
	return_err(proc,1201,1201,
		   rsprintf("Alaw encoding requires a 1 byte sample,%s %d",
			    " however the header value is",
			    spifr->status->file_sample_n_bytes));
    if (spifr->status->file_encoding == SP_se_pcm1 &&
	spifr->status->file_sample_n_bytes != 1) 
	return_err(proc,120,120,
		   rsprintf("PCM1 encoding requires a 1 byte sample, %s %d",
			    "however the header value is",
			    spifr->status->file_sample_n_bytes));
    if (spifr->status->file_encoding == SP_se_pcm1 &&
	spifr->status->file_sample_n_bytes != 2) 
	return_err(proc,120,120,
		   rsprintf("PCM2 encoding requires a 2 byte sample, %s %d",
			    "however the header value is",
			    spifr->status->file_sample_n_bytes));

    /********************************/
    /* set up the default decodings */

    if (sp->open_mode == SP_mode_read)
	if (sp_set_data_mode(sp,"") != 0){
	    print_return_status(spfp);
	    return_err(proc,110,110,
		       rsprintf("Unable to set up default encodings %s",
				"on file opened for read"));
	}
    else if (sp->open_mode == SP_mode_update)
	if (sp_set_data_mode(sp,"SE_ORIG:SBF_ORIG") != 0){
	    print_return_status(spfp);
	    return_err(proc,111,111,
	     "Unable to set up default encodings on file opened for update");
	}

    if (sp_verbose > 11) fprintf(spfp,"Proc %s: Returning 0\n",proc);
    return_success(proc,0,0,"ok");
}

int parse_sample_byte_format(char *str, enum SP_sample_byte_fmt *sbf)
{
    char *proc="parse_sample_byte_format " SPHERE_VERSION_STR;

    if (sp_verbose > 10) fprintf(spfp,"Proc %s:\n",proc);
    if (str == CNULL) 
	return_err(proc,100,100,"Null sample_byte_format_string");
    if (sbf == (enum SP_sample_byte_fmt *)0)
	return_err(proc,101,101,"Null sbf pointer");

    if (strsame(str,"01")) *sbf = SP_sbf_01;
    else if (strsame(str,"10")) *sbf = SP_sbf_10;
    else if (strsame(str,"1")) *sbf = SP_sbf_1;
    else if (strsame(str,"0123")) *sbf = SP_sbf_0123;
    else if (strsame(str,"1032")) *sbf = SP_sbf_1032;
    else if (strsame(str,"2301")) *sbf = SP_sbf_2301;
    else if (strsame(str,"3210")) *sbf = SP_sbf_3210;
    else if (strsame(str,"N")) *sbf = SP_sbf_N;
    if (strstr(str,"shortpack") != CNULL) {
	/* this return value must remain 1000, other functions depend on it*/
	return_err(proc,1000,1000,
	   rsprintf("Unknown sample_byte_format value '%s' in header",str));
    }
    if (sp_verbose > 11) fprintf(spfp,"Proc %s: Returning 0\n",proc);
    return_success(proc,0,0,"ok");
}    

int parse_sample_coding(char *str, int sample_n_bytes,
			enum SP_sample_encoding *sample_encoding,
			enum SP_waveform_comp *wav_compress)
{
    int enc_set=FALSE, comp_set=FALSE;
    char *pstr, *str_mem;
    char *proc="parse_sample_coding " SPHERE_VERSION_STR;
    
    if (sp_verbose > 10) fprintf(spfp,"Proc %s:\n",proc);

    if (str == CNULL)
	return_err(proc,101,101,"Null coding string");
    if (sample_n_bytes < 1 || sample_n_bytes > 2) {
	/*then we are assuming the field wasn't set yet, so use the default*/
	sample_n_bytes = 0;
    }
    if (sample_encoding == (enum SP_sample_encoding *)0)
	return_err(proc,103,103,"Null sample encoding pointer");
    if (wav_compress == (enum SP_waveform_comp *)0)
	return_err(proc,104,104,"Null waveform compress pointer");

    *wav_compress = SP_wc_null;
    *sample_encoding = SP_se_null;

    if (sp_verbose > 16) fprintf(spfp,"%s: string IS %s\n",proc,str);

    /* the algorithm to parse the sample encoding field is : */
    /*    1: get a token before a ',' or NULL */
    /*    2: set a flag to what it matches    */
    /*    3: move past the token              */
    /*    4: loop to (1)                      */
      
    /* make a duplicate copy because strtok is destructive */
    str_mem = mtrf_strdup(str);
    pstr = strtok(str_mem,",");
    while (pstr != CNULL){
	if (sp_verbose > 16)
	    fprintf(spfp,"%s: token found = %s\n",proc,pstr);
	if (strsame(pstr,"pcm")){
	    if (enc_set){
		mtrf_free(str_mem);
		return_err(proc,105,105,
			   "Multiple sample encodings in header field");
	    }
	    if (sample_n_bytes == 1)
		*sample_encoding = SP_se_pcm1;
	    else
		*sample_encoding = SP_se_pcm2;
	    enc_set = TRUE;
	}
	else if (strsame(pstr,"ulaw") || strsame(pstr,"mu-law")) {
	    if (enc_set){
		mtrf_free(str_mem);
		return_err(proc,105,105,
			   "Multiple sample encodings in header field");
	    }
	    *sample_encoding = SP_se_ulaw;
	    enc_set = TRUE;
	}
	else if (strsame(pstr,"pculaw")) {
	    if (enc_set){
		mtrf_free(str_mem);
		return_err(proc,105,105,
			   "Multiple sample encodings in header field");
	    }
	    *sample_encoding = SP_se_pculaw;
	    enc_set = TRUE;
	}
	else if (strsame(pstr,"alaw")){
	    if (enc_set){
		mtrf_free(str_mem);
		return_err(proc,1051,1051,
			   "Multiple sample encodings in header field");
	    }
	    *sample_encoding = SP_se_alaw;
	    enc_set = TRUE;
	}
	else if (strsame(pstr,"raw")){
	    if (enc_set){
		mtrf_free(str_mem);
		return_err(proc,105,105,
			   "Multiple sample encodings in header field");
	    }
	    *sample_encoding = SP_se_raw;
	    enc_set = TRUE;
	}
	else if (strstr(pstr,"embedded-shorten-v") != CNULL) {
	    if (comp_set) {
		mtrf_free(str_mem);
		return_err(proc,106,106,
			   "Multiple waveform compressions in header field");
	    }
	    *wav_compress = SP_wc_shorten;
	    comp_set = TRUE;
	}
	else if (strstr(pstr,"embedded-wavpack") != CNULL) {
	    if (comp_set){
		mtrf_free(str_mem);
		return_err(proc,106,106,
			   "Multiple waveform compressions in header field");
	    }
	    *wav_compress = SP_wc_wavpack;
	    comp_set = TRUE;
	}
	else if (strstr(pstr,"embedded-shortpack-v") != CNULL) {
	    if (comp_set){
		mtrf_free(str_mem);
		return_err(proc,106,106,
			   "Multiple waveform compressions in header field");
	    }
	    *wav_compress = SP_wc_shortpack;
	    comp_set = TRUE;
	}
	else {
	    mtrf_free(str_mem);
	    return_err(proc,107,107,"Unknown token in sample coding field");
	}
	pstr = strtok(CNULL,",");
    }
    if (*wav_compress == SP_wc_null)
	*wav_compress = SP_wc_none;
    if (*sample_encoding == SP_se_null)
	*sample_encoding = SP_se_pcm2;
    mtrf_free(str_mem);
    if (sp_verbose > 11) fprintf(spfp,"Proc %s: Returning 0\n",proc);
    return_success(proc,0,0,"ok");
}
