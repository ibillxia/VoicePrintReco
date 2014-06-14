#include <stdio.h>
#define SPHERE_LIBRARY_CODE
#include <sp/sphere.h>

/*** This file's functions write to a passed in file pointer, therefore ***/
/*** the messages created here do NOT go the FILE * spfp                ***/

int diff_waveforms(char *file1, char *file2,
		   char *conv_str1, char *conv_str2, int verbose, FILE *fp)
{
    SP_FILE *sp1, *sp2;
    int fail=0;
    int return_value = 0;

    if ((sp1 = sp_open(file1, "r")) == SPNULL){
	fail=1;
	if (verbose)
	    fprintf(fp,"DIFFERENT WAVEFORM: files %s and %s\n",file1,file2);
	if (verbose)
	    fprintf(fp,"sp_open failed on file %s\n",file1);
	sp_print_return_status(fp);
    }
    if ((sp2 = sp_open(file2, "r")) == SPNULL){
	if (fail == 0)
	    if (verbose)
	       fprintf(fp,"DIFFERENT WAVEFORM: files %s and %s\n",
		       file1,file2);
	fail=1;

	if (verbose)
	    fprintf(fp,"sp_open failed on file %s\n",file2);
	sp_print_return_status(fp);
	goto FATAL_QUIT;
    }    

    if (fail)
	goto FATAL_QUIT;

    if (conv_str1 != CNULL)
	if (sp_set_data_mode(sp1,conv_str1) != 0){
	    fail=1;
	    if (verbose)
	       fprintf(fp,"DIFFERENT WAVEFORM: files %s and %s\n",
		       file1,file2);
	    if (verbose)
		fprintf(fp,"sp_set_data_mode failed on file %s\n",file1);
	    sp_print_return_status(fp);
	}	    
    if (conv_str2 != CNULL)
	if (sp_set_data_mode(sp2,conv_str2) != 0){
	    if (fail == 0)
		if (verbose)
		    fprintf(fp,"DIFFERENT WAVEFORM: files %s and %s\n",
			    file1,file2);
	    fail=1;
	    if (verbose) 
		fprintf(fp,"sp_set_data_mode failed on file %s\n",file2);
	    sp_print_return_status(fp);
	}
    if (fail)
	goto FATAL_QUIT;

    return_value = diff_SP_FILE_waveforms(sp1, sp2, file1, file2, verbose, fp);
    goto CLEAN_UP;

  FATAL_QUIT:  /* Failed routine */
    return_value = 100;

  CLEAN_UP:
    if (sp1 != SPNULL) sp_close(sp1);
    if (sp2 != SPNULL) sp_close(sp2);

    return(return_value);
}


int diff_SP_FILE_waveforms(SP_FILE *sp1, SP_FILE *sp2, char *file1, char *file2, int verbose, FILE *fp)
{
    char *buff1=CNULL, *buff2=CNULL;
    int n1, n2, i, blocksize=512;
    int return_value = 0;
    SP_INTEGER samples_processed=0, file1_snb, file2_snb, file1_chcnt;
    SP_INTEGER file2_chcnt;

    if (sp_h_get_field(sp1,SAMPLE_N_BYTES_FIELD,
		       T_INTEGER,(void *)&file1_snb) != 0){
	fprintf(spfp,"Unable to retieve %s field from file '%s'\n",
		SAMPLE_N_BYTES_FIELD,file1);
	goto FATAL_QUIT;
    }
    if (sp_h_get_field(sp2,SAMPLE_N_BYTES_FIELD,
		       T_INTEGER,(void *)&file2_snb) != 0){
	fprintf(spfp,"Unable to retieve %s field from file '%s'\n",
		SAMPLE_N_BYTES_FIELD,file2);
	goto FATAL_QUIT;
    }
    if (sp_h_get_field(sp1,CHANNEL_COUNT_FIELD,
		       T_INTEGER,(void *)&file1_chcnt) != 0){
	fprintf(spfp,"Unable to retieve %s field from file '%s'\n",
		CHANNEL_COUNT_FIELD,file1);
	goto FATAL_QUIT;
    }
    if (sp_h_get_field(sp2,CHANNEL_COUNT_FIELD,
		       T_INTEGER,(void *)&file2_chcnt) != 0){
	fprintf(spfp,"Unable to retieve %s field from file '%s'\n",
		CHANNEL_COUNT_FIELD,file2);
	goto FATAL_QUIT;
    }


    if ((buff1 = (char *)sp_data_alloc(sp1,blocksize)) == CNULL){
	if (verbose) fprintf(fp,"Unable to malloc memory for buffer 1\n");
	goto FATAL_QUIT;
    }
    if ((buff2 = (char *)sp_data_alloc(sp2,blocksize)) == CNULL){
	if (verbose) fprintf(fp,"Unable to malloc memory for buffer 2\n");
	goto FATAL_QUIT;
    }

    if (file1_snb != file2_snb){
	if (verbose){
	    fprintf(fp,"Files '%s' and '%s' do not have the same",
		    file1,file2);
	    fprintf(fp," sample_n_bytes value, %ld and %ld respectively\n",
		    file1_snb,file2_snb);
	}
	goto FATAL_QUIT;
    }	

    if (file1_chcnt != file2_chcnt){
	if (verbose){
	    fprintf(fp,"Files '%s' and '%s' do not have the same",
		    file1,file2);
	    fprintf(fp," channel_count value, %ld and %ld respectively\n",
		    file1_chcnt,file2_chcnt);
	}
	goto FATAL_QUIT;
    }	

    do {
	int record_size = file1_snb * file1_chcnt;
	n1 = sp_read_data((char *)buff1,blocksize,sp1);
	if ((n1 == 0) && (sp1->read_spifr->waveform->samples_read !=
			  sp1->read_spifr->status->user_sample_count)){
	    sp_print_return_status(fp);
	    goto FATAL_QUIT;
	}
	n2 = sp_read_data((char *)buff2,blocksize,sp2);
	if ((n2 == 0) && (sp2->read_spifr->waveform->samples_read !=
			  sp2->read_spifr->status->user_sample_count)){
	    sp_print_return_status(fp);
	    goto FATAL_QUIT;
	}
	if (n1 != n2){
	    if (verbose)
		fprintf(fp,"DIFFERENT WAVEFORM: files %s and %s\n",
			file1,file2);
	    if (verbose) 
		fprintf(fp,"   %d samples read from '%s'\n",n1,file1);
	    if (verbose)
		fprintf(fp,"   %d samples read from '%s'\n",n2,file2);
	    goto FATAL_QUIT;
	}
	for (i=0; i<n1; i++) {
	    if (memcmp(buff1 + i*record_size,
		     buff2 + i*record_size,record_size)){
		int bn;
		if (verbose){
		 fprintf(fp,"DIFFERENT WAVEFORM: files %s and %s sample %ld\n",
			  file1,file2,samples_processed+i);
		    fprintf(fp,"    File 1:  ");
		    for (bn=0; bn < record_size; bn++) {
			fprintf(fp,"%2x ",
			     *((unsigned char *)(buff1 + i*record_size + bn)));
			if ((bn+1) % file1_snb == 0)
			    fprintf(fp,"| ");
		    }
		    fprintf(fp,"\n    File 2:  ");
		    for (bn=0; bn < record_size; bn++) {
			fprintf(fp,"%2x ",
			     *((unsigned char *)(buff2 + i*record_size + bn)));
			if ((bn+1) % file1_snb == 0)
			    fprintf(fp,"| ");
		    }
		    fprintf(fp,"\n");
		}
		goto FATAL_QUIT;
	    }
	}
	samples_processed += n1;
    } while (!sp_eof(sp1));
    if (!sp_eof(sp2)){
	if (verbose)
	    fprintf(fp,"DIFFERENT WAVEFORM: files %s and %s\n",file1,file2);
	if (verbose)
	    fprintf(fp,"   file %s not at eof\n",file2);
	goto FATAL_QUIT;
    }
    return_value = 0;
    goto CLEAN_UP;

  FATAL_QUIT:  /* Failed routine */
    return_value = 100;

  CLEAN_UP:
    if (sp1 != SPNULL) {
	if (buff1 != (char *)NULL) sp_data_free(sp1,buff1);
    }
    if (sp2 != SPNULL) {
	if (buff2 != (char *)NULL) sp_data_free(sp2,buff2);
    }
    return(return_value);
}

int diff_data(char *file1, char *file2, int verbose, FILE *fp)
{

    FILE *fp1, *fp2;
    struct header_t *h1, *h2;
    char c1, c2, *errmsg;
    int chr=0;

    if (strsame(file1,"-") && strsame(file2,"-")) {
	fprintf(spfp,"diff_data: Unable to compare Stdin to itself\n");
	return(100);
    }
    if ((fp1 = (strsame(file1,"-") ? stdin : fopen(file1, "r"))) == FPNULL){
	fprintf(spfp,"diff_data: Unable to open file '%s'\n",file1);
	return(100);
    }

    if ((fp2 = (strsame(file2,"-") ? stdin : fopen(file2, "r"))) == FPNULL){
	fprintf(spfp,"diff_data: Unable to open file '%s'\n",file2);
	return(100);
    }

    if ((h1 = sp_open_header(fp1,TRUE,&errmsg)) == HDRNULL){
	if (verbose)
	    fprintf(fp,"diff_data: Unable to open SPHERE header for file %s\n"
		    ,file1);
	return(100);
    }
    if ((h2 = sp_open_header(fp2,TRUE,&errmsg)) == HDRNULL){
	if (verbose)
	    fprintf(fp,"diff_data: Unable to open SPHERE header for file %s\n"
		    ,file2);
	return(100);
    }

    /* make Sure the sample byte_formats are the same before proceeding */
    { char *h1_sbf, *h2_sbf;
      int n, type, size;

      n = sp_get_field( h1, "sample_byte_format", &type, &size );
      if ( n < 0 ) {
	  fprintf(fp,"diff_data: Can't get sample_byte_format for file %s\n",
		  file1);
	  return(100);
      }
      if ((h1_sbf = (char *)mtrf_malloc(    size + 1 )) == CNULL ){
	  fprintf(fp,"diff_data: malloc failed for sbf string\n");
	  return(100);
      }
      if ((n = sp_get_data( h1, "sample_byte_format", h1_sbf, &size )) != 0){
	  fprintf(fp,"diff_data: Can't Load sample_byte_format for file %s\n",
		  file1);
	  mtrf_free(h1_sbf);
	  return(100);
      }
      h1_sbf[size] = '\0';


      n = sp_get_field( h2, "sample_byte_format", &type, &size );
      if ( n < 0 ) {
	  fprintf(fp,"diff_data: Can't get sample_byte_format for file %s\n",
		  file2);
	  mtrf_free(h1_sbf);
	  return(100);
      }
      if ((h2_sbf = (char *)mtrf_malloc(    size + 1 )) == CNULL ){
	  fprintf(fp,"diff_data: malloc failed for sbf string\n");
	  mtrf_free(h1_sbf);
	  return(100);
      }
      if ((n = sp_get_data( h2, "sample_byte_format", h2_sbf, &size )) != 0){
	  fprintf(fp,"diff_data: Can't Load sample_byte_format for file %s\n",
		  file2);
	  mtrf_free(h1_sbf); mtrf_free(h2_sbf);
	  return(100);
      }
      h2_sbf[size] = '\0';
      
      if (! strsame(h1_sbf,h2_sbf)){
	  mtrf_free(h1_sbf); mtrf_free(h2_sbf);
	  if (verbose)
	     fprintf(fp,"diff_data: Sphere files %s and %s differ in their sample_byte_format\n",
		     file1,file2);
	  fclose(fp1);
	  fclose(fp2);
	  return(1000);
      }
      /* The check passed, continue the test */
      mtrf_free(h1_sbf); mtrf_free(h2_sbf);
    }


    do {
	c1 = fgetc(fp1);
	c2 = fgetc(fp2);
	if (c1 != c2){
	    if (verbose)
		fprintf(fp,"diff_data: Sphere files %s and %s differ at character %d\n",
			file1,file2,chr);
	    fclose(fp1);
	    fclose(fp2);
	    return(100);
	}
	chr ++;
    } while (!feof(fp1));
    if (!feof(fp2)){
	sp_close_header(h1);
	sp_close_header(h2);
	fclose(fp1);
	fclose(fp2);
	return(100);
    }
    sp_close_header(h1);
    sp_close_header(h2);
    if (fp1 != stdin) fclose(fp1);
    if (fp2 != stdin) fclose(fp2);
    return(0);
}

int diff_header(char *file1, char *file2, int *chg, int *ins, int *del, 
		int verbose, FILE *fp)
{
    FILE *fp1, *fp2;
    struct header_t *h1, *h2;
    char *errmsg;
    int i1, i2, found;

    *chg = *ins = *del = 0;

    if (strsame(file1,"-") && strsame(file2,"-")) {
	fprintf(spfp,"diff_header: Unable to compare Stdin to itself\n");
	return(100);
    }
    if ((fp1 = (strsame(file1,"-") ? stdin : fopen(file1, "r"))) == FPNULL){
	fprintf(spfp,"diff_header: Unable to open file '%s'\n",file1);
	return(100);
    }
	
    if ((fp2 = (strsame(file2,"-") ? stdin : fopen(file2, "r"))) == FPNULL){
	fprintf(spfp,"diff_header: Unable to open file '%s'\n",file2);
	return(100);
    }

    if ((h1 = sp_open_header(fp1,TRUE,&errmsg)) == HDRNULL){
	if (verbose) 
	   fprintf(fp,"diff_header: Unable to open header for file '%s' - %s\n"
		    ,file1,errmsg);
	fclose(fp1);
	fclose(fp2);
	*chg = *ins = *del = 0x7fff;
	return(100);
    }
    if ((h2 = sp_open_header(fp2,TRUE,&errmsg)) == HDRNULL){
	if (verbose)
	   fprintf(fp,"diff_header: Unable to open header for file '%s' - %s\n"
		    ,file2,errmsg);
	sp_close_header(h1);
	fclose(fp1);
	fclose(fp2);
	*chg = *ins = *del = 0x7fff;
	return(100);
    }
    for (i1=0 ;i1 < h1->fc; i1++) {
	found=0;
	for (i2=0 ;i2 < h2->fc; i2++) {
	    if (strsame(h1->fv[i1]->name,h2->fv[i2]->name)) {
		found=1;
		if (h1->fv[i1]->type != h2->fv[i2]->type){
		    if (verbose)
			fprintf(fp,"    Changed field '%s' type %d -> %d\n",
				h1->fv[i1]->name,h1->fv[i1]->type,
				h2->fv[i2]->type);
		    *chg += 1;
		    continue;
		} else {
		    if (! strsame(h1->fv[i1]->data,h2->fv[i2]->data)){
			if(strsame(h1->fv[i1]->name,SAMPLE_CODING_FIELD) && 
                          (strncmp(h1->fv[i1]->data,h2->fv[i2]->data,20)==0))
			    ;
			else {
			    if (verbose){
				fprintf(fp,"    Changed field '%s' ",
					h1->fv[i1]->name);
				fprintf(fp,"value %s -> %s\n",
					h1->fv[i1]->data,h2->fv[i2]->data);
			    }
			    *chg += 1;
			}
		    }
		    continue;
		}
	    }
	}
	if (found == 0){
	    if (verbose) fprintf(fp,"    Deleted field %s\n",
				 h1->fv[i1]->name);
	    *del += 1;
	}
    }		

    for (i2=0 ;i2 < h2->fc; i2++) {
	found=0;
	for (i1=0 ;found==0 && i1 < h1->fc; i1++) {
	    if (strsame(h1->fv[i1]->name,h2->fv[i2]->name)) 
		found=1;
	}
	if (found == 0){
	    if (verbose) fprintf(fp,"    Inserted field %s\n",
				 h2->fv[i2]->name);
	    *ins += 1;
	}
    }		

    sp_close_header(h1);
    sp_close_header(h2);

    if (fp1 != stdin) fclose(fp1);
    if (fp2 != stdin) fclose(fp2);
    return(0);
}


int diff_files(char *file1, char *file2)
{

    FILE *fp1, *fp2;
    char c1, c2;
    fp1 = fopen(file1, "r");
    fp2 = fopen(file2, "r");
    do {
	c1 = fgetc(fp1);
	c2 = fgetc(fp2);
	if (c1 != c2){
	    fclose(fp1);
	    fclose(fp2);
	    return(100);
	}
    } while (!feof(fp1));
    if (!feof(fp2)){
	fclose(fp1);
	fclose(fp2);
	return(100);
    }
    if (fp1 != stdin) fclose(fp1);
    if (fp2 != stdin) fclose(fp2);
    return(0);
}








