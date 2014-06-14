#include <stdio.h>
#include <stdlib.h>
#define SPHERE_LIBRARY_CODE
#include <sp/sphere.h>
#include <math.h>


int sp_h_get_field(SP_FILE *sp_file, char *field, int ftype, void **value)
{
    char *proc="sp_h_get_field " SPHERE_VERSION_STR;

    if (sp_verbose > 10) fprintf(spfp,"Proc %s:\n",proc);
    if ( sp_file == SPNULL )
	return_err(proc,100,100,"Null SPFILE");
    if ( field == CNULL )	
	return_err(proc,102,102,"Null header field requested");
    if ( value == (void **)NULL )
	return_err(proc,103,103,"Null value pointer");
    
    if (sp_file->open_mode == SP_mode_read){
	return_child(proc,int,h_get_field(sp_file->read_spifr->header,
					       field,ftype,value));
    }
    else  /* write or update */
	return_child(proc,int,h_get_field(sp_file->write_spifr->header,
					       field,ftype,value));
}

int h_get_field(struct header_t *header, char *field, int ftype,
		void **value)
{
    char *proc="h_get_field " SPHERE_VERSION_STR;
    int type, size;
    int n;

    if (sp_verbose > 10) fprintf(spfp,"Proc %s:\n",proc);
    if ( header == HDRNULL )
	return_err(proc,101,101,"Null header");
    if ( field == CNULL )	
	return_err(proc,102,102,"Null header field requested");
    if ( value == (void **)NULL )
	return_err(proc,103,103,"Null value pointer");
    if ( ftype != T_STRING && ftype != T_REAL && ftype != T_INTEGER)
	return_err(proc,104,104,"Illegal field type");

    switch (ftype){ 
      case T_INTEGER:{
	  SP_INTEGER *ivalue;
	  /* Bug fixed, this value was mistakenly a int */
	  SP_INTEGER v;
	  ivalue = (SP_INTEGER *)value;
	  n = sp_get_field( header, field, &type, &size );
	  if ( n < 0 )
	      return_err(proc,105,105,"Non-Existing INTEGER field");
	  switch ( type ) {
	    case T_INTEGER:
	      n = sp_get_data( header, field, (char *) &v, &size );
	      if ( n < 0 )
		  return_err(proc,107,107,
			     "Unable to get INTEGER Field");
	      *ivalue = v;
	      return_success(proc,0,0,"ok");
	    case T_STRING:
	      return_err(proc,108,108,
			 "Illegal INTEGER access of a STRING Field");
	    case T_REAL:
	      return_err(proc,109,109,
			 "Illegal INTEGER access of a REAL Field");
	  }
	  return_success(proc,0,0,"ok");
      }
      case T_STRING: {
	  char *buf;

	  n = sp_get_field( header, field, &type, &size );
	  if ( n < 0 )
	      return_err(proc,110,110,
			 "Non-Existing STRING field");
	  switch ( type ) {
	    case T_INTEGER:
	      return_err(proc,111,111,
			 "Illegal STRING access of an INTEGER Field");
	    case T_REAL:
	      return_err(proc,112,112,
			 "Illegal STRING access of a REAL Field");
	    case T_STRING:
	      buf = (char *)mtrf_malloc(    size + 1 );
	      if ( buf == CNULL )
		  return_err(proc,113,113,"Unable to malloc char buffer");
	      n = sp_get_data( header, field, buf, &size );
	      buf[size] = (char )0;
	      if ( n < 0 ) {
		  mtrf_free( buf );
		  return_err(proc,114,114,"Unable to get STRING Field");
	      }
	      *value = buf ;
	      return_success(proc,0,0,"ok");
	  }
      }
      case T_REAL:{
	  SP_REAL fv, *fvalue;
	  fvalue = (SP_REAL *)value;
	  n = sp_get_field( header, field, &type, &size );
	  if ( n < 0 )
	      return_err(proc,115,115,"Non-Existing REAL field");
	  switch ( type ) {
	    case T_INTEGER:
	      return_err(proc,116,116,
			 "Illegal REAL access of an INTEGER Field");
	    case T_REAL:
	      n = sp_get_data( header, field, (char *) &fv, &size );
	      if ( n < 0 )
		  return 0;
	      *fvalue = (SP_REAL)fv;
	      return_success(proc,0,0,"ok");
	    case T_STRING:
	      return_err(proc,117,117,
			 "Illegal INTEGER access of a STRING Field");
	  }
	  return_success(proc,0,0,"ok");
      }
      default:
	return_err(proc,120,120,"Unknown header field type");
    }
}

int sp_h_set_field(SP_FILE *sp_file, char *field, int ftype, void *value)
{
    char *proc="sp_h_set_field " SPHERE_VERSION_STR;
    SPIFR *spifr;
    enum SP_sample_byte_fmt new_sbf = SP_sbf_null;
    enum SP_waveform_comp new_compress = SP_wc_null;
    enum SP_sample_encoding new_encoding = SP_se_null;

    if (sp_verbose > 10) fprintf(spfp,"Proc %s:\n",proc);
    if ( sp_file == SPNULL )
	return_err(proc,100,100,"Null SPFILE");
    if ( field == CNULL )	
	return_err(proc,102,102,"Null header field requested");
    if ( value == (char *)NULL )
	return_err(proc,103,103,"Null value pointer");
    if ( ftype != T_STRING && ftype != T_REAL && ftype != T_INTEGER)
	return_err(proc,104,104,"Illegal field type");

    /**********************************************************/
    /*  Pre-check the field types of standard fields          */
    
    if (strsame(field,SAMPLE_BF_FIELD)) {
	if (ftype != T_STRING)
	    return_err(proc,112,112,
	      rsprintf("Illegal field type for the '%s' field not T_STRING",
		       field));
	if (parse_sample_byte_format((char *)value,&new_sbf) != 0)
	    return_err(proc,105,105,
		       rsprintf("Illegal value '%s' for '%s' field",
				value,field));
    }
    if (strsame(field,SAMPLE_N_BYTES_FIELD)){
	if (ftype != T_INTEGER)
	    return_err(proc,113,113,
		       rsprintf("Illegal field type for the '%s' %s",
				field,"field not T_INTEGER"));
	if (*((SP_INTEGER *)value) < 1)
	    return_err(proc,106,106,
		       rsprintf("Illegal value %d for '%s' field",
				*((SP_INTEGER *)value),field));
    }
    if (strsame(field,SAMPLE_CODING_FIELD)){
	if (ftype != T_STRING)
	    return_err(proc,114,114,
		       rsprintf("Illegal field type for the '%s' %s",
				field,"field not T_STRING"));
	if ((sp_file->open_mode == SP_mode_write) ||
	    (sp_file->open_mode == SP_mode_update))
	    spifr = sp_file->write_spifr;
	else
	    spifr = sp_file->read_spifr;
	if (parse_sample_coding((char *)value,
				spifr->status->user_sample_n_bytes,
				&new_encoding, &new_compress) != 0){
	    sp_print_return_status(spfp);
	    return_err(proc,107,107,
		       rsprintf("Illegal value '%s' for '%s' field",
				value,field));
	}
    }
    if (strsame(field,SAMPLE_COUNT_FIELD) || 
	strsame(field,CHANNEL_COUNT_FIELD) ||
	strsame(field,SAMPLE_RATE_FIELD) || 
	strsame(field,SAMPLE_CHECKSUM_FIELD)) {
	if (ftype != T_INTEGER)
	    return_err(proc,115,115,
		       rsprintf("Illegal field type for the '%s' %s",
				field,"field not T_INTEGER"));
    }

   
    if ((sp_file->open_mode == SP_mode_write) ||
	(sp_file->open_mode == SP_mode_update)){
	
	spifr = sp_file->write_spifr;

	/* do some consitency checking on X fields to be able to catch */
	/* errors                                                      */
	if (new_sbf != SP_sbf_null) {
	    if (((new_sbf == SP_sbf_01) || (new_sbf == SP_sbf_10)) &&
		((spifr->status->user_sample_n_bytes != 2) &&
		 (spifr->status->user_sample_n_bytes != 0)))
		return_err(proc,200,200,
			   rsprintf("Illegal sample_n_bytes field %s",
				    "for a 2-byte sample_byte_format"))
	    else if ((new_sbf == SP_sbf_1) &&
		     ((spifr->status->user_sample_n_bytes != 1) &&
		      (spifr->status->user_sample_n_bytes != 0)))
		return_err(proc,201,201,
			   rsprintf("Illegal sample_n_bytes field %s",
				    "for a 1-byte sample_byte_format"))
	}

	if (h_set_field(spifr->header,field,ftype,value) >= 100){
	    print_return_status(spfp);
	    return_err(proc,108,108,
		       rsprintf("Unable to set field '%s' %s\n",
				field,"in the user's header"));
	}
	if (h_set_field(spifr->status->file_header,field,ftype,value) >= 100)
	    return_err(proc,109,109,
	       rsprintf("Unable to set field '%s' in the files's header\n",
			field));
	/* check for special fields which control the waveform definitions */
	
	if (strsame(field,"sample_n_bytes")) {
	    spifr->status->user_sample_n_bytes = (int)*((SP_INTEGER *)value);
	    spifr->status->file_sample_n_bytes = (int)*((SP_INTEGER *)value);
	    if (spifr->status->set_data_mode_occured_flag) 
	      return_warn(proc,1,1,
		"Field 'sample_n_bytes' set after set_data_mode occured\n");
	}
	if (strsame(field,"sample_byte_format")){
	    spifr->status->user_sbf = new_sbf;
	    spifr->status->file_sbf = new_sbf;
	    if (spifr->status->set_data_mode_occured_flag) 
	     return_warn(proc,2,2,
	     "Field 'sample_byte_format' set after set_data_mode occured\n");
	}
	if (strsame(field,"sample_checksum")){
	    spifr->status->file_checksum = (int)*((SP_INTEGER *)value);
	    if (spifr->status->set_data_mode_occured_flag) 
		return_warn(proc,2,2,
		"Field 'sample_checksum' set after set_data_mode occured\n");
	}
	if (strsame(field,SAMPLE_CODING_FIELD)){
	    spifr->status->user_compress = spifr->status->file_compress = 
		new_compress;
	    spifr->status->user_encoding = spifr->status->file_encoding =
		new_encoding;
	    if (spifr->status->set_data_mode_occured_flag) 
		return_warn(proc,3,3,
		  "Field 'sample_coding' set after set_data_mode occured\n");
	}
	if (strsame(field,"sample_count")){
	    spifr->status->user_sample_count = *((SP_INTEGER *)value);
	    spifr->status->file_sample_count = *((SP_INTEGER *)value);
	}
	if (strsame(field,"channel_count")){
	    spifr->status->user_channel_count = *((SP_INTEGER *)value);
	    spifr->status->file_channel_count = *((SP_INTEGER *)value);
	}
	if (strsame(field,"sample_rate")){
	    spifr->status->user_sample_rate = *((SP_INTEGER *)value);
	    spifr->status->file_sample_rate = *((SP_INTEGER *)value);
	}
	if (spifr->status->write_occured_flag)
	    return_warn(proc,4,4,"Call executed after WRITE occured\n");
    }
    else{
	spifr = sp_file->read_spifr;
	if ((strsame(field,"sample_n_bytes")) ||
	    (strsame(field,"sample_byte_format")) ||
	    (strsame(field,SAMPLE_CODING_FIELD)))
	    return_err(proc,111,111,
		       rsprintf("On READ Field '%s' %s function",field,
				"should be set using the 'set_data_mode'"));
	if ((strsame(field,"sample_count")) ||
	    (strsame(field,"channel_count")) ||
	    (strsame(field,"sample_rate")))
	    return_err(proc,112,112,
		       rsprintf("Field '%s' should not be set on a %s",
				field,"file opened for reading"));
	    
	if (h_set_field(spifr->header,field,ftype,value) >= 100)
	    return_err(proc,110,110,
		rsprintf("Unable to set field '%s' in the SPFILE's header\n",
			 field));
	if (spifr->status->read_occured_flag)
	    return_warn(proc,5,5,"Call executed after READ occured\n");
    }
    return_success(proc,0,0,"ok");
}

int h_set_field(struct header_t *header, char *field, int ftype, void *value)
{
    char *proc="h_set_field " SPHERE_VERSION_STR;
    int type, size, n;
   
    if (sp_verbose > 10) fprintf(spfp,"Proc %s:\n",proc);
    if (sp_verbose > 30) {
	fprintf(spfp,"Proc %s: before set\n",proc);
	sp_print_lines(header,spfp); 
    }
    if ( header == HDRNULL )
	return_err(proc,101,101,"Null header in SPFILE");
    if ( field == CNULL )	
	return_err(proc,102,102,"Null header field requested");
    if ( value == (char *)NULL )
	return_err(proc,103,103,"Null value pointer");
    if ( ftype != T_STRING && ftype != T_REAL && ftype != T_INTEGER)
	return_err(proc,104,104,"Illegal field type");

    n = sp_get_field( header, field, &type, &size );
    
    if ( n < 0 ){ /* add the field to the header */
	if (sp_add_field(header,field,ftype,value) < 0)
	    return_err(proc,105,105,"Unable to add field");
    }
    else 
        if (sp_change_field(header,field,ftype,value) < 0)
	    return_err(proc,106,106,"Unable to change existing field");
    if (sp_verbose > 30) { 
	fprintf(spfp,"Proc %s: After set\n",proc); 
	sp_print_lines(header,spfp); 
    }
    return_success(proc,0,0,"ok");
}

/*
 *  sp_h_delete_field
 *
 */
int sp_h_delete_field(SP_FILE *sp_file, char *field)
{
    char *proc="sp_h_delete_field " SPHERE_VERSION_STR;
    SPIFR *spifr;
    
    if (sp_verbose > 10) fprintf(spfp,"Proc %s:\n",proc);
    if ( sp_file == SPNULL )
	return_err(proc,100,100,"Null SPFILE");
    if ( field == CNULL )	
	return_err(proc,102,102,"Null header field requested");

    if ((sp_file->open_mode == SP_mode_write) ||
	(sp_file->open_mode == SP_mode_update))
	spifr = sp_file->write_spifr;
    else
	spifr = sp_file->read_spifr;
       
    if ((sp_file->open_mode == SP_mode_write) || 
	(sp_file->open_mode == SP_mode_update)) {
	if (h_delete_field(sp_file->write_spifr->header,field) < 0)
	    return_err(proc,105,105,
	       rsprintf("Deletion of field '%s' in the user's header failed",
			field));
	if (h_delete_field(sp_file->write_spifr->status->file_header,
			   field) < 0)
	    return_err(proc,106,106,
	       rsprintf("Deletion of field '%s' in the hidden header failed",
			field));
    }
    if ((sp_file->open_mode == SP_mode_read) ||
	(sp_file->open_mode == SP_mode_update)) {
	if (h_delete_field(sp_file->read_spifr->header,field) < 0)
	    return_err(proc,107,107,
	       rsprintf("Deletion of field '%s' in the user's header failed",
			field));
	if (h_delete_field(sp_file->read_spifr->status->file_header,
			   field) < 0)
	    return_err(proc,108,108,
	       rsprintf("Deletion of field '%s' in the hidden header failed",
			field));
    }

    return_success(proc,0,0,"ok");
}

int h_delete_field(struct header_t *header, char *field)
{
    int type, size;
    int n;
    char *proc="h_delete_field " SPHERE_VERSION_STR;
    
    if (sp_verbose > 10) fprintf(spfp,"Proc %s:\n",proc);
    if ( header == HDRNULL )
	return_err(proc,101,101,"Null header in SPFILE");
    if ( field == CNULL )	
	return_err(proc,102,102,"Null header field requested");
    
    n = sp_get_field( header, field, &type, &size );
    if ( n < 0 )
	return_warn(proc,1,1,rsprintf("Header field '%s' does not exist",
				      field));
    if (sp_delete_field(header,field) < 0)
	return_err(proc,104,104,rsprintf("Deletion of field '%s' failed",
					 field));
    return_success(proc,0,0,"ok");	
}

