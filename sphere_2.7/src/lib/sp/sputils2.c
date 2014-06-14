
/* LINTLIBRARY */

/** File: sputils2.c **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define SPHERE_LIBRARY_CODE
#include <sp/sphere.h>

extern int farray_fields;
extern struct field_t *farray[];


int strsame(char *s1, char *s2)
{
    return ( strcmp( s1, s2 ) == 0 );
}

int strdiff(char *s1, char *s2)
{
    return ! strsame( s1, s2 );
}

void *sp_data_alloc(SP_FILE *sp, int nsamp)
{
    SPIFR *spifr;
    void *tptr, **tarr;
    int c, size;
    char *proc = "sp_data_alloc " SPHERE_VERSION_STR;

    if (nsamp == 0)
	return_err(proc,100,0,
		   rsprintf("Illegal value 0 for 'nsamp' arguement"));
    if (nsamp < (-1))
	return_err(proc,101,0,
		   rsprintf("Illegal value '%d' for 'nsamp' arguement",nsamp));
    if (sp == SPNULL)
	return_err(proc,102,0,"Null SP_FILE pointer");

    if (sp->open_mode == SP_mode_read)
	spifr = sp->read_spifr;
    else {
	if (nsamp == (-1))
	    return_err(proc,103,0,
	       "Illegal number samples, -1, for a file opened for write");
	spifr = sp->write_spifr; 
    }

    if (spifr->status->user_data_fmt == SP_df_raw){
	size = spifr->status->user_channel_count * 
	    spifr->status->user_sample_n_bytes;
	if (nsamp > 0){
	    size *= nsamp;
	} else
	    size *= spifr->status->user_sample_count;
	if ((tptr = mtrf_malloc(size)) == CNULL)
	    return_err(proc,200,0,
		       rsprintf("Unable to alloc linear array %d bytes long",
				size));
	return_success(proc,0,tptr,"ok");
    } else if (spifr->status->user_data_fmt == SP_df_array){
	/* first make an array of pointers for each channel */
	if ((tarr = (void **) mtrf_malloc(spifr->status->user_channel_count * 
					  sizeof(void *))) == (void **)0)
	    return_err(proc,300,0,
		       rsprintf("Unable to alloc %d-channel pointer array",
				spifr->status->user_channel_count));

	/* compute the size of each channel */
	size = spifr->status->user_sample_n_bytes * ((nsamp > 0) ? nsamp :
					     spifr->status->user_sample_count);
	/* alloc space for each waveform */ 
	for (c=0; c<spifr->status->user_channel_count; c++)	
	    if ((tarr[c] = mtrf_malloc(size)) == (void *)0)
		return_err(proc,301,0,
			rsprintf("Unable to channel array of %d bytes",size));
    
	return_success(proc,0,(void *)tarr,"ok");
    } else 
	return_err(proc,400,0,rsprintf("Unknown data format '%d'",
					    spifr->status->user_data_fmt));
}

int sp_data_free(SP_FILE *sp, void *tptr)
{
    SPIFR *spifr;
    void **tarr;
    int c;
    char *proc = "sp_data_free " SPHERE_VERSION_STR;

    if (tptr == (void *)0)
	return_err(proc,100,0,
		   rsprintf("Illegal value '%d' for second arguement",tptr));
    if (sp == SPNULL)
	return_err(proc,101,0,"Null SP_FILE pointer");

    if (sp->open_mode == SP_mode_read)
	spifr = sp->read_spifr;
    else
	spifr = sp->write_spifr; 

    if (spifr->status->user_data_fmt == SP_df_raw)
	mtrf_free(tptr);
    else {
	tarr  = (void **)tptr;
	for (c=0; c<spifr->status->user_channel_count; c++)	
	    mtrf_free((char *)tarr[c]);
	mtrf_free((char *)tarr);
    }    
    return_success(proc,0,0,"ok");
}

int convert_file(char *filein, char *fileout, char *format_conversion, 
		 char *prog_name)
{
    SP_FILE *sp_in, *sp_out;
    SP_INTEGER sp_in_snb, sp_out_snb;
    
    if ((sp_in=sp_open(filein,"r")) == SPNULL){
	fprintf(spfp,"%s: Unable to open file '%s' to update\n",
		prog_name,(strsame(filein,"-") ? "stdin" : filein ));
	sp_print_return_status(spfp);
	return(100);
    }
    if ((sp_out=sp_open(fileout,"w")) == SPNULL){
	fprintf(spfp,"%s: Unable to open file '%s' to update\n",
		prog_name,(strsame(fileout,"-") ? "stdout" : fileout ));
	sp_print_return_status(spfp);
	sp_close(sp_in);
	if (! strsame(fileout,"-")) unlink(fileout);
	return(100);
    }

    /* special provision for keeping the sample byte format */
    if (strstr(format_conversion,"SBF-ORIG") != (char *)0){
	/* set the sbf of file 1 to sbf_ORIG */
	if (sp_set_data_mode(sp_in,"SBF-ORIG") != 0){
	    fprintf(spfp,"%s: Unable to set data mode to '%s' on original"
		    " file to maintain the sample_byte_format\n",
		    prog_name,format_conversion);
	    sp_print_return_status(spfp);
	    sp_close(sp_in); sp_close(sp_out);
	    if (! strsame(fileout,"-")) unlink(fileout);
	    return(100);
	}
    }

    if (sp_copy_header(sp_in,sp_out) != 0){
	fprintf(spfp,"%s: Unable to duplicate the input file\n",prog_name);
	sp_print_return_status(spfp);
	sp_close(sp_in); sp_close(sp_out);
	if (! strsame(fileout,"-")) unlink(fileout);
	return(100);
    }

    if (sp_set_data_mode(sp_out,format_conversion) != 0){
	fprintf(spfp,"%s: Unable to set data mode to '%s'\n",
		prog_name,format_conversion);
	sp_print_return_status(spfp);
	sp_close(sp_in); sp_close(sp_out);
	if (! strsame(fileout,"-")) unlink(fileout);
	return(100);
    }

    if (sp_h_get_field(sp_in,SAMPLE_N_BYTES_FIELD,
		       T_INTEGER,(void *)&sp_in_snb) != 0){
	fprintf(spfp,"Unable to retieve %s field from file '%s'\n",
		SAMPLE_N_BYTES_FIELD,filein);
	sp_print_return_status(spfp);
	sp_close(sp_in); sp_close(sp_out);
	if (! strsame(fileout,"-")) unlink(fileout);
	return(100);
    }
    if (sp_h_get_field(sp_out,SAMPLE_N_BYTES_FIELD,
		       T_INTEGER,(void *)&sp_out_snb) != 0){
	fprintf(spfp,"Unable to retieve %s field from file '%s'\n",
		SAMPLE_N_BYTES_FIELD,fileout);
	sp_print_return_status(spfp);
	sp_close(sp_in); sp_close(sp_out);
	if (! strsame(fileout,"-")) unlink(fileout);
	return(100);
    }

    /**************************************************/
    /*   If certain format conversion have happened   */
    /*   delete the now invalid checksum              */
    if (sp_out->write_spifr->status->file_encoding !=
	sp_out->write_spifr->status->user_encoding)
	sp_h_delete_field(sp_out,"sample_checksum");
    
    { char *buff;
      int ns, nc, nspb;
      int samples_read, samples_written, tot_samples_read;

      ns = sp_in->read_spifr->status->user_sample_count;
      nc = sp_in->read_spifr->status->user_channel_count;      
      nspb = sp_in->read_spifr->status->user_sample_n_bytes;
      tot_samples_read=0;
      if ((buff=mtrf_malloc(nc * nspb * 4096)) == CNULL) {
	  sp_close(sp_in); sp_close(sp_out);
	  if (! strsame(fileout,"-")) unlink(fileout);
	  return(100);
      }
      do {
	  samples_read = sp_read_data(buff,4096,sp_in);
	  tot_samples_read += samples_read;
	  if (samples_read > 0) {
	      samples_written = sp_write_data(buff,samples_read,sp_out);
	      if (samples_written != samples_read){
		  sp_print_return_status(spfp);
		  fprintf(spfp,"%s: Samples written %d != Samples Read %d\n",
			  prog_name,samples_written, samples_read);
		  sp_close(sp_in); sp_close(sp_out);
		  if (! strsame(fileout,"-")) unlink(fileout);
		  mtrf_free(buff);
		  return(100);
	      }
	  } else { 
	      /*  Change to allow for a file read in from a pipe */
	      /*  June 22, 1994                                  */
	      if (ns != 999999999 && ns != tot_samples_read)
		  sp_print_return_status(spfp);
	      if (sp_eof(sp_in) == 0) {
		  fprintf(spfp,"%s: Zero samples read while not at EOF\n",
			  prog_name);
		  sp_print_return_status(spfp);
		  sp_close(sp_in); sp_close(sp_out);
		  if (! strsame(fileout,"-")) unlink(fileout);
		  mtrf_free(buff);
		  return(100);
	      }
	      if ((!sp_eof(sp_in)) || (sp_error(sp_in) >= 100)) {
		  /* a checksum error occured, close the sp and */
		  fprintf(spfp,"%s: Error reading input file '%s'\n",
			  prog_name,filein);
		  sp_print_return_status(spfp);
		  sp_close(sp_in); sp_close(sp_out);
		  if (! strsame(fileout,"-")) unlink(fileout);
		  mtrf_free(buff);
		  return(100);
	      }
	  }
      } while (samples_read > 0);
      mtrf_free(buff);
      sp_close(sp_in);
      if (sp_close(sp_out) != 0) {
	  fprintf(spfp,"%s: In-place update of file '%s' FAILED\n",
		  prog_name,filein);
	  sp_print_return_status(spfp);
	  if (! strsame(fileout,"-")) unlink(fileout);
	  return(100);
      }
  }
    return(0);
}

int do_update(char *filein, char *format_conversion, char *prog_name)
{
    SP_FILE *sp;

    if ((sp=sp_open(filein,"u")) == SPNULL){
	fprintf(spfp,"%s: Unable to open file '%s' to update\n",
		prog_name,filein);
	sp_print_return_status(spfp);
	return(100);
    }

    if (sp_set_data_mode(sp,format_conversion) != 0){
	fprintf(spfp,"%s: Unable to set data mode of file '%s' to '%s'\n",
		prog_name,filein,format_conversion);
	sp_print_return_status(spfp);
	sp_close(sp);
	return(100);
    }

    /**************************************************/
    /*   If certain format conversion have happened   */
    /*   delete the now invalid checksum              */
    if ((sp->write_spifr->status->file_encoding != 
	 sp->write_spifr->status->user_encoding) ||
	(sp->write_spifr->status->channels != CHANNELSNULL))
	sp_h_delete_field(sp,"sample_checksum");
    
    if (sp_close(sp) != 0){
	fprintf(spfp,"%s: In-place update of file '%s' FAILED\n",
		prog_name,filein);
	sp_print_return_status(spfp);
	return(100);
    }
    return(0);
}


char *enum_str_SP_file_open_mode(enum SP_file_open_mode id)
{
    switch (id){
	  case SP_mode_read : return("SP_mode_read");
	  case SP_mode_write : return("SP_mode_write");
	  case SP_mode_update : return("SP_mode_update");
	  case SP_mode_null : return("SP_mode_null");
	  default: return("UNKNOWN");
    }
}

char *enum_str_SP_sample_encoding(enum SP_sample_encoding id)
{
    switch (id){
	case SP_se_pcm2: return("SP_se_pcm2");
	case SP_se_pcm1: return("SP_se_pcm1");
	case SP_se_ulaw: return("SP_se_ulaw");
	case SP_se_pculaw: return("SP_se_pculaw");
	case SP_se_alaw: return("SP_se_alaw");
	case SP_se_raw: return("SP_se_raw");
	case SP_se_null: return("SP_se_null");
	default: return("UNKNOWN");
    }
}

char *enum_str_SP_waveform_comp(enum SP_waveform_comp id)
{
    switch (id){
	case SP_wc_shorten: return("SP_wc_shorten");
	case SP_wc_wavpack: return("SP_wc_wavpack");
	case SP_wc_shortpack: return("SP_wc_shortpack");
	case SP_wc_none: return("SP_wc_none");
	case SP_wc_null: return("SP_wc_null");
	default: return("UNKNOWN");
    }
}
char *enum_str_SP_sample_byte_fmt(enum SP_sample_byte_fmt id)
{
    switch (id){
	  case SP_sbf_01: return("SP_sbf_01");
	  case SP_sbf_10: return("SP_sbf_10");
	  case SP_sbf_1: return("SP_sbf_1");
	  case SP_sbf_N: return("SP_sbf_N");
	  case SP_sbf_3210: return("SP_sbf_3210");
	  case SP_sbf_2301: return("SP_sbf_2301");
	  case SP_sbf_1032: return("SP_sbf_1032");
	  case SP_sbf_0123: return("SP_sbf_0123");
	  case SP_sbf_orig: return("SP_sbf_orig");
	  case SP_sbf_null: return("SP_sbf_null");
	  default: return("UNKNOWN");
    }
}


char *get_natural_byte_order(int bps)
{
    return(header_str_SP_sample_byte_fmt(get_natural_sbf(bps)));
}

char *header_str_SP_sample_byte_fmt(enum SP_sample_byte_fmt id)
{
    switch (id){
	  case SP_sbf_01: return("01");
	  case SP_sbf_10: return("10");
	  case SP_sbf_1: return("1");
	  case SP_sbf_N: return("N");
	  case SP_sbf_3210: return("3210");
	  case SP_sbf_2301: return("2301");
	  case SP_sbf_1032: return("1032");
	  case SP_sbf_0123: return("0123");
	  default: return("UNKNOWN");
    }
}
char *enum_str_SP_data_format(enum SP_data_format id)
{
    switch (id){
	  case SP_df_null: return("SP_df_null");
	  case SP_df_raw: return("SP_df_raw");
	  case SP_df_array: return("SP_df_array");
	  default: return("UNKNOWN");
    }
}
