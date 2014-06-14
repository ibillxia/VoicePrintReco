#include <stdio.h>
#define SPHERE_LIBRARY_CODE
#include <sp/sphere.h>
#include <sp/shorten/shorten.h>
#include <string.h>

int parse_channel_selection(char *, SP_FILE *sp);

struct header_t *global_header;
/*
 *
 *  set_data_mode
 *
 *
 */

int sp_set_data_mode(SP_FILE *sp, char *mode)
{
    char *mode_str, *mstr, *proc="sp_set_data_mode " SPHERE_VERSION_STR;
    int se_set=0, sbf_set=0, df_set=0;
    enum SP_sample_encoding     new_encoding,    old_encoding = SP_se_null;
    enum SP_waveform_comp       new_compress,    old_compress = SP_wc_null;
    enum SP_sample_byte_fmt     new_sbf     ,    old_sbf      = SP_sbf_null;
    enum SP_data_format         new_data_fmt,    old_data_fmt = SP_df_null;

    if (sp_verbose > 10) fprintf(spfp,"Proc %s: \n",proc);
    if (sp_verbose > 15)
	fprintf(spfp,"Proc %s: mode string '%s'\n",proc,mode);

    if (sp == SPNULL) return_err(proc,100,100,"Null SPFILE");
    if (mode == CNULL) return_err(proc,101,101,"Null mode string");

    /* Set up the old file status.  */
    switch (sp->open_mode){
      case SP_mode_read:
	if (sp_verbose > 15)
	    fprintf(spfp,"Proc %s: read filename %s\n",proc,
		    sp->read_spifr->status->external_filename);
	if (sp->read_spifr->status->read_occured_flag)
	    return_err(proc,1000,1000,
		       "Call executed after READ occured\n");
	old_encoding    = sp->read_spifr->status->file_encoding ;
	old_compress    = sp->read_spifr->status->file_compress ;
	old_sbf         = sp->read_spifr->status->file_sbf ;
	old_data_fmt = sp->read_spifr->status->user_data_fmt ;

	/* set up the default operation modes */
	new_compress = SP_wc_none;
	new_sbf = SP_sbf_N;
	new_encoding = old_encoding;
	new_data_fmt = old_data_fmt;

	break;
      case SP_mode_update:
      case SP_mode_write:
	if (sp_verbose > 15)
	    fprintf(spfp,"Proc %s: write/update filename %s\n",proc,
		    sp->write_spifr->status->external_filename);
	if (sp->write_spifr->status->write_occured_flag)
	    return_err(proc,1001,1001,"Call executed after WRITE occured\n");
	old_encoding = sp->write_spifr->status->user_encoding ;
	old_compress = sp->write_spifr->status->user_compress ;
	old_sbf      = sp->write_spifr->status->user_sbf ;
	old_data_fmt = sp->write_spifr->status->user_data_fmt ;

	/* set up the default operation modes */
	new_compress = old_compress;
	new_sbf = SP_sbf_N;
	new_encoding = old_encoding;
	new_data_fmt = old_data_fmt;

	break;
      default:
	return_err(proc,106,106,
		   "Unknown file open mode in SPFILE structure");
    }

    if (sp_verbose > 15){
	fprintf(spfp,"Proc %s: Before mode parsing:    ",proc);
	fprintf(spfp,"old_encoding %10s   new_encoding %10s\n",
		enum_str_SP_sample_encoding(old_encoding),
		enum_str_SP_sample_encoding(new_encoding));
	fprintf(spfp,"Proc %s: Before mode parsing:    ",proc);
	fprintf(spfp,"old_compress %10s   new_compress %10s\n",
		enum_str_SP_waveform_comp(old_compress),
		enum_str_SP_waveform_comp(new_compress));
	fprintf(spfp,"Proc %s: Before mode parsing:    ",proc);
	fprintf(spfp,"old_sbf      %10s   new_sbf      %10s\n",
		enum_str_SP_sample_byte_fmt(old_sbf),
		enum_str_SP_sample_byte_fmt(new_sbf));
	fprintf(spfp,"Proc %s: Before mode parsing:    ",proc);
	fprintf(spfp,"old_data_fmt %10s   new_data_fmt %10s\n",
		enum_str_SP_data_format(old_data_fmt),
		enum_str_SP_data_format(new_data_fmt));
    }

    /* the tokenization method is data destructive , so */
    /* make a local copy */
    mode_str=mtrf_strdup(mode);

    /* begin parsing each token */
    mstr = strtok(mode_str,":");
    while (mstr != CNULL){
	if (sp_verbose > 15)
	    fprintf(spfp,"Proc %s: token found = %s\n",proc,mstr);
	if ((strsame(mstr,"SE-PCM")) || (strsame(mstr,"SE-PCM-2"))) {
	    new_encoding = SP_se_pcm2;
	    new_compress = SP_wc_none; 
	    se_set++;
	} else if (strsame(mstr,"SE-PCM-1")) {
	    new_encoding = SP_se_pcm1;
	    new_compress = SP_wc_none; 
	    se_set++;
	} else if (strsame(mstr,"SE-RAW")) {
	    new_encoding = SP_se_raw;
	    new_compress = SP_wc_none; 
	    se_set++;
	} else if (strsame(mstr,"SE-ULAW")) {
	    new_encoding = SP_se_ulaw;
	    new_compress = SP_wc_none; 
	    se_set++;
	} else if (strsame(mstr,"SE-PCULAW")) {
	    new_encoding = SP_se_pculaw;
	    new_compress = SP_wc_none; 
	    se_set++;
	} else if (strsame(mstr,"SE-ALAW")) {
	    new_encoding = SP_se_alaw;
	    new_compress = SP_wc_none; 
	    se_set++;
	} else if (strsame(mstr,"SE-SHORTEN")){
	    new_encoding = old_encoding;
	    new_compress = SP_wc_shorten;
	    se_set++;
	}
	else if (strsame(mstr,"SE-WAVPACK")){
	    new_encoding = old_encoding;
	    new_compress = SP_wc_wavpack;
	    se_set++;
	}
	else if (strsame(mstr,"SE-SHORTPACK")){
	    new_encoding = old_encoding;
	    new_compress = SP_wc_shortpack;
	    se_set++;
	}
	else if (strsame(mstr,"SE-ORIG")){
	    new_encoding = old_encoding;
	    new_compress = old_compress;
	    se_set++;
	}
	else if (strsame(mstr,"SBF-01")) {
	    new_sbf = SP_sbf_01;
	    sbf_set++;
	}
	else if (strsame(mstr,"SBF-10")) {
	    new_sbf = SP_sbf_10;
	    sbf_set++;
	}

	else if (strsame(mstr,"SBF-1")) {
	    new_sbf = SP_sbf_1;
	    sbf_set++;
	}
	else if (strsame(mstr,"SBF-N")) {
	    new_sbf = SP_sbf_N;
	    sbf_set++;
	}
	else if (strsame(mstr,"SBF-ORIG"))   {
	    new_sbf = old_sbf;
	    sbf_set++;
	}
	else if (strsame(mstr,"DF-RAW"))   {
	    new_data_fmt = SP_df_raw;
	    df_set++;
	}
	else if (strsame(mstr,"DF-ARRAY"))   {
	    new_data_fmt = SP_df_array;
	    df_set++;
	}
	else if (strncmp(mstr,"CH-",3) == 0) {
	    if (parse_channel_selection(mstr+3,sp) != 0)
		return_err(proc,105,105,
			   rsprintf("Parse channel selection string '%s'",
				    get_return_status_message()));
	}    
	else {
	    mtrf_free(mode_str);
	    return_err(proc,102,102,
		       rsprintf("Illegal token '%s' in mode string '%s'",
				mstr,mode));
	}
	mstr = strtok(CNULL,":");
    }
    mtrf_free(mode_str);
    if (se_set > 1)
	return_err(proc,103,103,
		   rsprintf("Too many sample_encoding options%s'%s'",
			    " used in mode string ",mode));
    if (sbf_set > 1)
	return_err(proc,104,104,
		   rsprintf("Too many sample_byte_format options%s'%s'",
			    " used in mode string ",mode));

    /* convert the natural byte order to the proper value */
    if (new_sbf == SP_sbf_N){
	int conv_size;
	if (new_encoding == SP_se_pcm2) conv_size = 2;
	else if ((new_encoding == SP_se_ulaw) ||
		 (new_encoding == SP_se_pculaw) ||
		 (new_encoding == SP_se_alaw) ||
		 (new_encoding == SP_se_pcm1)) conv_size = 1;
	else conv_size = (sp->open_mode == SP_mode_read) ?
	    sp->read_spifr->status->file_sample_n_bytes :
		sp->write_spifr->status->file_sample_n_bytes;

	new_sbf = get_natural_sbf(conv_size);
    }

    if (sp_verbose > 15){
	fprintf(spfp,"Proc %s: After mode parsing    ",proc);
	fprintf(spfp,"old_encoding %10s   new_encoding %10s\n",
		enum_str_SP_sample_encoding(old_encoding),
		enum_str_SP_sample_encoding(new_encoding));
	fprintf(spfp,"Proc %s: After mode parsing    ",proc);
	fprintf(spfp,"old_compress %10s   new_compress %10s\n",
		enum_str_SP_waveform_comp(old_compress),
		enum_str_SP_waveform_comp(new_compress));
	fprintf(spfp,"Proc %s: After mode parsing    ",proc);
	fprintf(spfp,"old_sbf      %10s   new_sbf      %10s\n",
		enum_str_SP_sample_byte_fmt(old_sbf),
		enum_str_SP_sample_byte_fmt(new_sbf));
	fprintf(spfp,"Proc %s: After mode parsing    ",proc);
	fprintf(spfp,"old_data_fmt %10s   new_data_fmt %10s\n",
		enum_str_SP_data_format(old_data_fmt),
		enum_str_SP_data_format(new_data_fmt));
    }

    /* Test the supported file conversions dependent on the file's open mode */
    switch (sp->open_mode){
      case SP_mode_read:
	switch (old_encoding) {
	  case SP_se_pcm2:
	    if ((new_encoding == SP_se_ulaw) || (new_encoding == SP_se_pculaw)||
		(new_encoding == SP_se_raw) ||
		(new_encoding == SP_se_alaw) || (new_encoding == SP_se_pcm2))
		;
	    else
		return_err(proc,108,108,
			   "Illegal read transformation from PCM2");
	    break;
	  case SP_se_pcm1:
	    if ((new_encoding == SP_se_ulaw) || (new_encoding == SP_se_raw) ||
		(new_encoding == SP_se_pcm2) || (new_encoding == SP_se_pcm1) ||
		(new_encoding == SP_se_alaw) || (new_encoding == SP_se_pculaw))
		;
	    else
		return_err(proc,109,109,
			   "Illegal read transformation from PCM1");
	    break;
	  case SP_se_ulaw:
	    if ((new_encoding == SP_se_ulaw) || (new_encoding == SP_se_raw) ||
		(new_encoding == SP_se_pcm2) ||	(new_encoding == SP_se_pcm1))
		;
	    else
		return_err(proc,110,110,
			   "Illegal read transformation from ULAW");
	    break;
	  case SP_se_pculaw:
	    if ((new_encoding == SP_se_pculaw) || (new_encoding == SP_se_raw) ||
		(new_encoding == SP_se_pcm2)  || (new_encoding == SP_se_pcm1))
		;
	    else
		return_err(proc,110,110,
			   "Illegal read transformation from PCULAW");
	    break;
	  case SP_se_alaw:
	    if ((new_encoding == SP_se_alaw) || (new_encoding == SP_se_raw) ||
		(new_encoding == SP_se_pcm2) ||	(new_encoding == SP_se_pcm1))
		;
	    else
		return_err(proc,130,130,
			   "Illegal read transformation from ALAW");
	    break;
	  case SP_se_raw:
	    if (new_encoding == SP_se_raw)
		;
	    else
		return_err(proc,111,111,
			   "Illegal read transformation from RAW");
	    break;
	  default:
	    if (old_encoding != new_encoding)
		return_err(proc,112,112,
			"Internal function error for file opened for reading");
	}
	switch (old_compress){
	  case SP_wc_shortpack:
	  case SP_wc_wavpack:
	  case SP_wc_shorten:
	    /* permit a compressed format iff it's the same. J.Thompson */
	    if (new_compress != SP_wc_none && new_compress != old_compress)
		return_err(proc,113,113,
			   rsprintf("Illegal read transformation to %s",
				    "compress a compressed file"));
	    if (new_encoding == SP_se_null)
		return_err(proc,114,114,
			   "Internal function error for file opened for read");
	    break;
	  case SP_wc_none:
	    if (new_compress != SP_wc_none)
		return_err(proc,115,115,
			  "Unable to read an uncompressed file as compressed");
	    break;
	  default:
	    return_err(proc,115,115,
		       "Internal function error for file opened for read");
	}
	switch (old_sbf){
	  case SP_sbf_10:
	  case SP_sbf_01:
	  case SP_sbf_N:
	    if (new_sbf == SP_sbf_1)
		if ((new_encoding == SP_se_pcm1) ||
		    (new_encoding == SP_se_ulaw) ||
		    (new_encoding == SP_se_pculaw) ||
		    (new_encoding == SP_se_alaw))
		    ;
		else
		    return_err(proc,116,116,
			  "Unable to convert to a one-byte per sample format");
	    break;
	  case SP_sbf_1:
	    if (new_sbf != SP_sbf_1)
		if (new_encoding == SP_se_pcm2)
		    ;
		else
		    return_err(proc,117,117,
			  "Unable to convert to a 2-byte per sample format");
	    break;
	  default:
	    if (new_sbf != old_sbf)
		return_err(proc,118,118,
			   rsprintf("%s for read sbf converted from %s to %s",
				    "Internal function error for file opened",
				    enum_str_SP_sample_byte_fmt(old_sbf),
				    enum_str_SP_sample_byte_fmt(new_sbf)));
	    break;
	}
	break;
	/*********************************************************************/
	/**              Write and Update Mode Checking                      */
      case SP_mode_update:
      case SP_mode_write:
	switch (old_encoding) {
	  case SP_se_pcm2:
	    if ((new_encoding == SP_se_ulaw) || (new_encoding == SP_se_pculaw)||
		(new_encoding == SP_se_pcm1) ||
		(new_encoding == SP_se_pcm2) || (new_encoding == SP_se_alaw))
		;
	    else
		return_err(proc,119,119,
			   "Illegal write transformation from PCM2");
	    break;
	  case SP_se_pcm1:
	    if ((new_encoding == SP_se_ulaw) || (new_encoding == SP_se_pcm1) ||
		(new_encoding == SP_se_pcm2) ||	(new_encoding == SP_se_pcm1) ||
		(new_encoding == SP_se_alaw) || (new_encoding == SP_se_alaw))
		;
	    else
		return_err(proc,120,120,
			   "Illegal write transformation from PCM1");
	    break;
	  case SP_se_ulaw:
	    if ((new_encoding == SP_se_ulaw) || (new_encoding == SP_se_pcm1) ||
	        (new_encoding == SP_se_pcm2))
		;
	    else
		return_err(proc,121,121,
			   "Illegal write transformation from ULAW");
	    break;
	  case SP_se_pculaw:
	    if ((new_encoding == SP_se_pculaw)|| (new_encoding == SP_se_pcm1) ||
	        (new_encoding == SP_se_pcm2))
		;
	    else
		return_err(proc,121,121,
			   "Illegal write transformation from PCULAW");
	    break;
	  case SP_se_alaw:
	    if ((new_encoding == SP_se_alaw) || (new_encoding == SP_se_pcm1) ||
	        (new_encoding == SP_se_pcm2))
		;
	    else
		return_err(proc,131,131,
			   "Illegal write transformation from ALAW");
	    break;
	  case SP_se_raw:
	    if (new_encoding == SP_se_raw)
		;
	    else
		return_err(proc,122,122,
			   "Illegal write transformation from RAW");
	    break;
	  default:
	    return_err(proc,123,123,
		       "Internal function error: old sample_encoding not set");
	}
	switch (old_compress){
	  case SP_wc_shortpack:
	  case SP_wc_wavpack:
	  case SP_wc_shorten:
	    /* permit a compressed format iff it's the same. J.Thompson */
	    if (new_compress != SP_wc_none && new_compress != old_compress)
		return_err(proc,124,124,
			   rsprintf("Illegal write transformation to %s",
				    "compress a compressed file"));
	    break;
	  case SP_wc_none:
	    break;
	  default:
	    return_err(proc,125,125,
		       "Internal function error: old compression not set");
	}
	switch (old_sbf){
	  case SP_sbf_10:
	  case SP_sbf_01:
	  case SP_sbf_N:
	    if (new_sbf == SP_sbf_1)
		if ((new_encoding == SP_se_pcm1) ||
		    (new_encoding == SP_se_ulaw) ||
		    (new_encoding == SP_se_pculaw) ||
		    (new_encoding == SP_se_alaw))
		    ;
		else
		    return_err(proc,126,126,
			  "Unable to convert to a one-byte per sample format");
	    break;
	  case SP_sbf_1:
	    if (new_sbf != SP_sbf_1)
		if (new_encoding == SP_se_pcm2)
		    ;
		else
		    return_err(proc,127,127,
			   "Unable to convert to a 2-byte per sample format");
	    break;
	  default:
	    if (new_sbf != old_sbf)
		return_err(proc,128,128,
			   "Internal function error for file opened for write");
	}
	break;
      default:
	return_err(proc,129,129,
		   "Unknown file open mode in SPFILE structure");
    }

    /* now update the hidden header: file_header for write, user_header for */
    /* read and update the status structure variables                       */
    {	struct header_t *hdr_to_modify;
	SP_INTEGER lint;
	char new_sample_coding[100];
	int channels_changed = FALSE;
       
	if (sp->open_mode == SP_mode_read){
	    if (sp_verbose > 15)
		fprintf(spfp,"Proc %s: modifying the user's header \n",
			proc);
	    hdr_to_modify = sp->read_spifr->header;
	}
	else {
	    if (sp_verbose > 15)
		fprintf(spfp,"Proc %s: modifying the file's header \n",
			proc);
	    hdr_to_modify = sp->write_spifr->status->file_header;
	}

	/* set the sample_n_bytes field */
	if (new_encoding == SP_se_pcm2){
	    lint = 2;
	    if (sp_verbose > 15) 
		fprintf(spfp,"Proc %s: new %s value %ld\n",
			proc,SAMPLE_N_BYTES_FIELD,lint);
	    if (h_set_field(hdr_to_modify, SAMPLE_N_BYTES_FIELD, 
			    T_INTEGER, &lint))
		return_err(proc,150,150,
			   "Unable to update sample_n_bytes field\n");
	} else if ((new_encoding == SP_se_ulaw) ||
		   (new_encoding == SP_se_pculaw) ||
		   (new_encoding == SP_se_alaw) ||
		   (new_encoding == SP_se_pcm1)) {
	    lint = 1;
	    if (sp_verbose > 15)
		fprintf(spfp,"Proc %s: new %s value %ld\n",
			proc,SAMPLE_N_BYTES_FIELD,lint);
	    if (h_set_field(hdr_to_modify, SAMPLE_N_BYTES_FIELD,
			    T_INTEGER, &lint))
		return_err(proc,151,151,
			   "Unable to update sample_n_bytes field\n"); 	    
	} else { /* nothing has changed, used the file_sample_n_bytes */
	    if (sp->open_mode == SP_mode_read)
		lint = sp->read_spifr->status->file_sample_n_bytes;
	    else
		lint = sp->write_spifr->status->file_sample_n_bytes;
	}
	
	/* setup the new sample_n_bytes values */
	if (sp->open_mode == SP_mode_read)
	    sp->read_spifr->status->user_sample_n_bytes = lint ;
	else
	    sp->write_spifr->status->file_sample_n_bytes = lint ;

	/* set the sample byte format */ 
	{   char *str = header_str_SP_sample_byte_fmt(new_sbf);
	    if (new_sbf == SP_sbf_null)
		return_err(proc,157,157,"Internal function error");
	    if (sp_verbose > 15) 
		fprintf(spfp,"Proc %s: new sample_byte_format %s\n",
			proc,str);
	    if (h_set_field(hdr_to_modify, SAMPLE_BF_FIELD, T_STRING, str))
	      return_err(proc,153,153,
		rsprintf("Unable to update sample_byte_format field to '%s'\n",
			 str));
	}

	/* set the new channel count */ 
	lint = -1;
	if (sp->open_mode == SP_mode_read) {
	    if (sp->read_spifr->status->channels != CHANNELSNULL)
		lint = sp->read_spifr->status->user_channel_count = 
		    sp->read_spifr->status->channels->num_chan;
	} else {
	    if (sp->write_spifr->status->channels != CHANNELSNULL)
		lint = sp->write_spifr->status->file_channel_count = 
		    sp->write_spifr->status->channels->num_chan;
	} 
	if (lint != (-1)){
	    if (sp_verbose > 15) 
		fprintf(spfp,"Proc %s: new channel count %ld\n",
			proc,lint);
	    if (h_set_field(hdr_to_modify, CHANNEL_COUNT_FIELD, T_INTEGER, 
			    (void *)&lint))
		return_err(proc,154,154,
			   rsprintf("%schannel count field to %d\n",
				    "Unable to update ",lint));
	    channels_changed = TRUE;
	}
		    
	/* set the sample encoding field */
	*new_sample_coding = '\0';
	switch (new_encoding){
	  case SP_se_pcm1:
	  case SP_se_pcm2: strcat(new_sample_coding,"pcm"); break;
	  case SP_se_ulaw: strcat(new_sample_coding,"ulaw"); break;
	  case SP_se_pculaw: strcat(new_sample_coding,"pculaw"); break;
	  case SP_se_alaw: strcat(new_sample_coding,"alaw"); break;
	  case SP_se_raw: strcat(new_sample_coding,"raw"); break;
	  default: return_err(proc,158,158,"Internal Function Error");
	}
	switch (new_compress){
	  case SP_wc_shorten:
	    strcat(new_sample_coding,
		   rsprintf(",embedded-shorten-v%1d.%s",FORMAT_VERSION,
			    BUGFIX_RELEASE));
	    break;
	  case SP_wc_wavpack:
	    strcat(new_sample_coding,rsprintf(",embedded-%s", WAVPACK_MAGIC));
	    break;
	  case SP_wc_shortpack:
	    strcat(new_sample_coding,",embedded-shortpack-vIDONTKNOW");
	    break;
	  default: ;
	}
	if (h_set_field(hdr_to_modify, SAMPLE_CODING_FIELD,
			T_STRING, new_sample_coding))
	    return_err(proc,160,160,
		   rsprintf("Unable to update sample_coding field to '%s'\n",
			    new_sample_coding));
	if (sp_verbose > 15)
	    fprintf(spfp,"Proc %s: new sample coding %s\n",
		    proc,new_sample_coding);

	/* if the mode is write and theses conditions have been met, delete */
	/* the channel count */
	if ((sp->open_mode == SP_mode_write) ||
	    (sp->open_mode == SP_mode_update))
     	    if ((old_encoding !=  new_encoding) || channels_changed){
		if (h_delete_field(hdr_to_modify, SAMPLE_CHECKSUM_FIELD) > 100)
		    return_err(proc,170,170,
			       "Unable to delete sample checksum field");
		if (sp_verbose > 15)
		    fprintf(spfp,"Proc %s: deleting sample checksum\n",proc);
	    }

	/* encoding has changed, re-set the sample_sig_bits */
	if (old_encoding !=  new_encoding){
	    if (new_encoding == SP_se_pcm2)
		lint = 16;
	    else if ((new_encoding == SP_se_ulaw) || 
		     (new_encoding == SP_se_pculaw) || 
		     (new_encoding == SP_se_alaw) || 
		     (new_encoding == SP_se_pcm1))
		lint = 8;
	    
	    if (h_set_field(hdr_to_modify, SAMPLE_SIG_BITS_FIELD,
			    T_INTEGER, (void *)&lint)){		
		if (sp_verbose > 15)
		    fprintf(spfp,"Proc %s: new sample_sig_bits to %ld\n",
			    proc,lint);
		return_err(proc,180,180,
			   rsprintf("Unable to update sample_sig_bits %s %d\n",
				    "field to ",lint));
	    }
	}
	/* setup the new values */
	if (sp->open_mode == SP_mode_read){
	    sp->read_spifr->status->user_encoding = new_encoding ;
	    sp->read_spifr->status->user_compress = new_compress ;
	    sp->read_spifr->status->user_sbf      = new_sbf      ;
	    sp->read_spifr->status->user_data_fmt = new_data_fmt ;
	} else {
	    sp->write_spifr->status->file_encoding = new_encoding ;
	    sp->write_spifr->status->file_compress = new_compress ;
	    sp->write_spifr->status->file_sbf      = new_sbf      ;
	    sp->write_spifr->status->user_data_fmt = new_data_fmt ;
	}
    }

/* make a warning if the default mode is used */
    
    if (sp->open_mode == SP_mode_read)
	sp->read_spifr->status->set_data_mode_occured_flag = TRUE;
    else
	sp->write_spifr->status->set_data_mode_occured_flag = TRUE;

    /* Return a warning if we are in the original data mode, which has
       compression J.Thompson */
    switch (old_compress){
      case SP_wc_shortpack:
      case SP_wc_wavpack:
      case SP_wc_shorten:
	if (new_compress != SP_wc_none)
 	    return_warn(proc,1,1,
			"Can not read or write data in this compressed mode");
      case SP_wc_none:
      case SP_wc_null:
	;
    }
    
    if (sp_verbose > 16) sp_file_dump(sp,spfp);
    if (sp_verbose > 11) fprintf(spfp,"Proc %s: Exit\n",proc);

    return_success(proc,0,0,"ok");

}

int parse_channel_selection(char *desc, SP_FILE *sp){
    char *tstr, *tstr2, tmp[10], *temp_description, *next_str;
    int out_channels = 0, c;
    int first, last, len, pass, max_chan_add=1, chan_add;
    CHANNELS *Channels;
    int sp_nchan;
    char *proc="parse_channel_selection " SPHERE_VERSION_STR;
    SPIFR *spifr;

    if (sp_verbose > 10) fprintf(spfp,"Proc %s:\n",proc);
    if (sp_verbose > 15)
	fprintf(spfp,"Proc %s: channel description string '%s'\n",proc,desc);

    if (sp == SPNULL) return_err(proc,100,100,"Null SPFILE");
    if (desc == CNULL)
	return_err(proc,101,101,"NULL channel descriptor passed");

    if (sp->open_mode == SP_mode_read){
	spifr = sp->read_spifr;
	sp_nchan = spifr->status->file_channel_count;
    } else {
	spifr = sp->write_spifr;
	sp_nchan = spifr->status->user_channel_count;
    }

    if (strspn(desc,"0123456789,.+") != strlen(desc))
	return_err(proc,102,102,
		   rsprintf("Error: Illegal format for descriptor '%s'\n",
			    desc));
    /* make sure .'s occure in pairs */
    tstr = desc;
    while ((tstr2 = (char *)strchr(tstr,'.')) != CNULL){
	if (*(tstr2+1) != '.')
	    return_err(proc,103,103,
		 rsprintf("Illegal channel range format for descriptor '%s'\n",
			  desc));
	tstr = tstr2+2;
    }

    /* Constency checks have passed, free the channel structure if it */
    /* already exists */
    if (spifr->status->channels != CHANNELSNULL)
	free_CHANNELS(spifr);

    for (pass=0; pass < 2; pass++){
	temp_description = (char *)mtrf_strdup(desc);
	
	if (pass == 1) {
	    if (spifr->status->channels != CHANNELSNULL)
		free_CHANNELS(spifr);
	    if ((Channels = alloc_CHANNELS(out_channels,max_chan_add)) ==
		CHANNELSNULL)
		return_err(proc,102,102,"Unable to alloc Channel structure");
	    spifr->status->channels = Channels;
	}
	/* count the channels in the tokens */
        /* instead of using strtok, use strchr() and do it by hand */
        tstr = temp_description;
        if ((tstr2 = (char *)strchr(tstr,',')) != CNULL){
            next_str = tstr2+1;
            *tstr2 = '\0';
        } else
            next_str = CNULL;

	while (tstr != CNULL){
	    if (pass == 0 && sp_verbose > 15)
		fprintf(spfp,"Proc %s: token '%s'\n",proc,tstr);
	    if (strspn(tstr,"0123456789") == strlen(tstr)){
		first = atoi(tstr);
		if ((first > sp_nchan) || (first <= 0))
		    return_err(proc,1000,1000,
			   rsprintf("Error: channel %s out of range\n",tstr));
		if (pass == 0)
		    out_channels ++;
		else {
		    Channels->ochan[Channels->num_chan].num_origin=1;
		    Channels->ochan[Channels->num_chan].orig_channel[0]=first;
		    Channels->num_chan++;
		}
	    } else if ((char *)strchr(tstr,'+') != CNULL){
		chan_add=0;
		if ((spifr->status->user_encoding != SP_se_pcm2) &&
		    (spifr->status->user_encoding != SP_se_ulaw) &&
		    (spifr->status->user_encoding != SP_se_pculaw) &&
		    (spifr->status->user_encoding != SP_se_alaw))
		    return_err(proc,2000,2000,
	         "Error: Can't add channels for types other that pcm, alaw, pculaw or ulaw");
		while ((len = strspn(tstr,"0123456789")) > 0) {
		    strncpy(tmp,tstr,len);
		    *(tmp+len) = '\0';
		    first = atoi(tmp);
		    if ((first > sp_nchan) || (first <= 0))
			return_err(proc,1000,1000,
			    rsprintf("Error: channel %s out of range\n",tmp));
		    if (*(tstr+=len) != '\0')
			tstr++;
		    chan_add++;
		    if (pass == 1){
			c=Channels->ochan[Channels->num_chan].num_origin;
			Channels->ochan[Channels->num_chan].orig_channel[c]=
			     first;
			Channels->ochan[Channels->num_chan].num_origin++;
		    }
		}
		if (pass == 0){
		    if (chan_add > max_chan_add)
			max_chan_add = chan_add;
		    out_channels ++;
		} else {
		    Channels->num_chan++;
		}
	    } else if ((tstr2 = (char *)strchr(tstr,'.')) != CNULL) {
		/* get off the first number */
		strncpy(tmp,tstr,tstr2-tstr) ;
		*(tmp + (tstr2-tstr)) = '\0';
		first = atoi(tmp);
		if ((first > sp_nchan) || (first <= 0))
		    return_err(proc,1000,1000,
			   rsprintf("Error: channel %s out of range\n",tmp));
		tstr2+=2;
		if ((char *)strchr(tstr2,'.') != CNULL)
		    return_err(proc,1001,1001,
			 rsprintf("Illegal format of channel desc %s\n",tstr));
		strcpy(tmp,tstr2);
		last=atoi(tmp);
		if ((last > sp_nchan) || (last <= 0))
		    return_err(proc,1000,1000,
			     rsprintf("Error: channel %s out of range\n",tmp));
		if (pass == 0)
		    out_channels += last-first + 1;
		else {
		    for (c=first; c<=last; c++){
			Channels->ochan[Channels->num_chan].num_origin=1;
			Channels->ochan[Channels->num_chan].orig_channel[0]=c;
			Channels->num_chan++;
		    }
		}
	    }
            if (next_str == CNULL)
                tstr = next_str;
            else {
                tstr = next_str;
                if ((tstr2 = (char *)strchr(next_str,',')) != CNULL){
                    next_str = tstr2+1;
                    *tstr2 = '\0';
                } else
                    next_str = CNULL;
            }
	}
	mtrf_free(temp_description);
    }
    return_success(proc,0,0,"ok");
}
