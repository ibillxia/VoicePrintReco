/* File: h_edit.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sp/sphere.h>

			/* for temporary files */
#define EXTENSION	".BAK"

static void wav_edit_usage(char *, char *);
static int wav_edit_headers(char *, char *, int, char **, char *,
			    char *,int, int, int *, int);
static int add_checksum_to_file(char *, char *);

int wav_edit(int argc, char **argv, char *ops, char *opspec)
{
    int exit_status = 0;
    int ignore_failure = 0, ov, in_place, verbose = 0;
    int add_checksum = 0;
    char *prog;
    
    char *field[MAXFIELDS];
    char op[MAXFIELDS];
    int nfields = 0;
    
    int i, c, uflag = 0;
    char *outfile = CNULL, *dir = CNULL;
    
    prog = strrchr(argv[0],'/');
    prog = (prog == CNULL) ? argv[0] : (prog + 1);
    
    while ( (c = hs_getopt(argc,argv,opspec)) != -1 )
	
	switch (c) {
	  case 'D':
	    if (hs_optarg == CNULL)
		wav_edit_usage(prog,ops);
	    dir = hs_optarg;
	    break;
	    
	  case 'f':
	    ignore_failure = 1;
	    break;
	    
	  case 'o':
	    if (hs_optarg == CNULL)
		wav_edit_usage(prog,ops);
	    outfile = hs_optarg;
	    break;
	    
	  case 'u':
	    uflag = 1;
	    break;
	    
	  case 'v':
	    verbose = 1;
	    break;
 
	  case 'c':
	    add_checksum = 1;
	    break;

	  default:
	    if (hs_optarg == CNULL)
		wav_edit_usage(prog,ops);
	    if (strchr(ops,c) == CNULL)
		wav_edit_usage(prog,ops);
	    field[nfields] = hs_optarg;
	    op[nfields] = (char) c;
	    nfields++;
	    break;
	}
    
    if (verbose > 0) fprintf(spfp,"%s: %s\n",prog,sp_get_version());
    
    if (nfields == 0 && !add_checksum) {
	(void) fprintf(stderr,"%s: Error -- no fields specified\n",prog);
	exit(1);
    }
    
    if (outfile != CNULL) {
	if (dir != CNULL)
	    wav_edit_usage(prog,ops);
	if (hs_optind + 1 != argc)
	    wav_edit_usage(prog,ops);
    }
    
    if (hs_optind >= argc) {
	(void) fprintf(stderr,"%s: Error -- no files specified\n",prog);
	exit(ERROR_EXIT_STATUS);
    }
    
    in_place = (outfile == CNULL) && (dir == CNULL);
    if (uflag && in_place) {
	(void) fprintf(stderr,
		       "%s: Error -- cannot unlink if editing in-place\n",
		       prog);
	exit(ERROR_EXIT_STATUS);
    }
    if (verbose && in_place)
	(void) printf("Editing in-place\n");
    
    for (i=hs_optind; i < argc; i++) {
	
	static char ofile[MAXPATHLEN];
	
	if (outfile != CNULL)
	    (void) strcpy(ofile,outfile);
	else {
	    if (dir == CNULL) {
		(void) strcpy(ofile,argv[i]);
		(void) strcat(ofile,EXTENSION);
	    } else {
		char *base;
		
		(void) strcpy(ofile,dir);
		(void) strcat(ofile,"/");
		base = strrchr(argv[i],'/');
		base = (base == CNULL) ? argv[i] : (base + 1);
		(void) strcat(ofile,base);
	    }
	}
	
	if (verbose)
	    (void) printf("\nEditing %s\n",argv[i]);
	if (wav_edit_headers(argv[i],ofile,nfields,field,op,prog,in_place,
		     ignore_failure,&ov,add_checksum) < 0) {
	    exit_status = ERROR_EXIT_STATUS;
	    if (verbose)
		(void) printf("\tContinuing\n");
	    continue;
	}
	
	if (in_place) {
	    if (ov) {
		if (verbose)
		    (void) printf("\tOverwriting header for %s\n",
				  argv[i]);
		if (sp_overwrite_header(ofile,argv[i],ov) < 0) {
		    (void) fprintf(stderr,
				   "%s: %s: Error -- cannot overwrite header\n",
				   prog,argv[i]);
		    exit_status = ERROR_EXIT_STATUS;
		} else if (unlink(ofile) < 0) {
		    perror(ofile);
		    exit_status = ERROR_EXIT_STATUS;
		}

		/* added by JGF 060696, add the checksum if requested */
		if (add_checksum)
		    if (add_checksum_to_file(argv[i], prog) == -1){
			perror(argv[i]);
			exit_status = ERROR_EXIT_STATUS;
		    }		
	    } else {
		if (verbose)
		    (void) printf("\tRenaming %s to %s\n",
				  ofile,argv[i]);
		if (rename(ofile,argv[i]) < 0) {
		    perror(argv[i]);
		    exit_status = ERROR_EXIT_STATUS;
		}

		/* added by JGF 060696, add the checksum if requested */
		if (add_checksum)
		    if (add_checksum_to_file(argv[i], prog) == -1){
			perror(argv[i]);
			exit_status = ERROR_EXIT_STATUS;
		    }		
	    }
	    continue;
	}
	
	if (verbose)
	    (void) printf("\tCreated %s\n",ofile);
	/* added by JGF 060696, add the checksum if requested */
	if (add_checksum)
	    if (add_checksum_to_file(ofile, prog) == -1){
		perror(ofile);
		exit_status = ERROR_EXIT_STATUS;
	    }		
	if (uflag && (unlink(argv[i]) < 0)) {
	    perror(argv[i]);
	    exit_status = ERROR_EXIT_STATUS;
	}
    }
    
    exit(exit_status);
}

/*****************************************************************************/

static void wav_edit_usage(char *prog, char *ops)
{
    int multi;
    static char fn[]   = "fieldname";
    static char fnv[]  = "fieldname=value";
    static char use1[] = "Usage: %s [-%suvf] [-D dir] -%s %s ... file ...\n";
    static char use2[] = "   or: %s [-%suvf] [-o outfile] -%s %s ... file\n";
    
    multi = strlen(ops) > 1;
    (void) fprintf(stderr, use1, prog,(strcmp(prog,"h_edit")==0)?"c":"",
		   (!multi)?ops:"opchar", multi?fnv:fn );
    (void) fprintf(stderr, use2, prog,(strcmp(prog,"h_edit")==0)?"c":"",
		   (!multi)?ops:"opchar", multi?fnv:fn );
    if (multi)
	(void) fprintf(stderr, "Opchar is any of %s\n", ops);
    exit(ERROR_EXIT_STATUS);
}


/* ARGSUSED */
static int wav_edit_headers(char *f1, char *f2,
			     int num_fld, char **fld_names, char *fop,
			     char *prog,
			     int in_place, int ignore_failure, int *ov,
			    int add_checksum)
{
    struct header_t *h;
    char *errmsg;
    register FILE *fp1, *fp2;
    int i, failed, remove_newlines;
    long ohs, hs, dummy;
    

    if (in_place) {
	ohs = sp_file_header_size(f1);
	if (ohs <= 0) {
	    (void) fprintf(stderr,"%s: %s: Bad header\n",prog,f1);
	    return -1;
	}
    }

    fp1 = fopen(f1,"r");
    if (fp1 == FPNULL) {
	(void) fprintf(stderr,"%s: %s: Error opening for reading\n",prog,f1);
	return -1;
    }

    fp2 = fopen(f2,"w");
    if (fp2 == FPNULL) {
	(void) fprintf(stderr,"%s: %s: Error opening %s for writing\n",
		       prog,f1,f2);
	(void) fclose(fp1);
	return -1;
    }

    h = sp_open_header(fp1,1,&errmsg);
    if (h == HDRNULL) {
	(void) fprintf(stderr,"%s: %s: Error reading header -- %s\n",
		       prog,f1,errmsg);
	(void) fclose(fp1);
	(void) fclose(fp2);
	return -1;
    }

    failed = 0;
    for (i=0; i < num_fld; i++) {
	char *eq, *p;
	long lbuf;
	double dbuf;
	int dummy, t, type;


	/* Adding 'F' allows h_delete to function */
	if (fop[i] == 'K' || fop[i] == 'F') {
	    /* delete the field */
	    if (sp_delete_field(h,fld_names[i]) < 0) {
		(void) fprintf(stderr,"%s: %s: Error deleting field %s\n",
			       prog,f1,fld_names[i]);
		failed++;
	    }
	} else {
	    eq = p = strchr(fld_names[i],'=');
	    if (p == (char *) NULL) {
		(void) fprintf(stderr,"%s: %s: Error in edit specifier %s -- \"=\" required\n",
			       prog,f1,fld_names[i]);
		(void) sp_close_header(h);
		(void) fclose(fp1);
		(void) fclose(fp2);
		return -1;
	    }
	    *p++ = '\0';
	    remove_newlines=FALSE;
	    switch (fop[i]) {
	    
	      case 'S':	type = T_STRING;
		        break;
		
	      case 'I':	type = T_INTEGER;
			lbuf = atol(p);
			p = (char *) &lbuf;
			break;

	      case 'R':	type = T_REAL;
			dbuf = atof(p);
			p = (char *) &dbuf;
			break;

	      default:	(void) fprintf(stderr,
				"%s: %s: Error in edit specification -%s\n",
				prog,f1,fop);
			*eq = '=';
			(void) sp_close_header(h);
			(void) fclose(fp1);
			(void) fclose(fp2);
			return -1;
	    }

	    /* change a field */
	    if (sp_get_field(h,fld_names[i],&t,&dummy) < 0) {
		if (sp_add_field(h,fld_names[i],type,p) < 0) {
		    (void) fprintf(stderr,"%s: %s: Error adding field %s\n",
				   prog,f1,fld_names[i]);
		    failed++;
		}
		*eq = '=';
		continue;
	    }
	    
	    if (type != t) {  /* Delete the field instead of reporting an
				 error */
		if (sp_delete_field(h,fld_names[i]) < 0) {
		    (void) fprintf(stderr,
				   "%s: %s: Error deleting field %s before changing\n",
				   prog,f1,fld_names[i]);
		    failed++;
		    continue;
		}
	    }

	    if (sp_change_field(h,fld_names[i],type,p) < 0) {
		(void) fprintf(stderr,"%s: %s: Error changing field %s\n",
			       prog,f1,fld_names[i]);
		failed++;
	    }
	    *eq = '=';
	}
    }

    if (failed && ! ignore_failure) {
	(void) unlink(f2);
	(void) fclose(fp1);
	(void) fclose(fp2);
	(void) sp_close_header(h);
	return -1;
    }

    if ((sp_write_header(fp2,h,&hs,&dummy) < 0) ||
	(!(*ov = (in_place && (ohs == hs))) && (sp_fpcopy(fp1,fp2) < 0))) {
	(void) fprintf(stderr,"%s: %s: Error writing to %s\n",
		       prog,f1,f2);
	(void) unlink(f2);
	(void) fclose(fp1);
	(void) fclose(fp2);
	(void) sp_close_header(h);
	return -1;
    }

    (void) fclose(fp1);
    (void) fclose(fp2);

    if (sp_close_header(h) < 0) {
	(void) fprintf(stderr,"%s: %s: Warning -- close_header failed\n",
		       prog,f1);
	return -1;
    }
    
    if (*ov)
	*ov = hs;

    return 0;
}

int add_checksum_to_file(char *f2, char *prog){ 
    SP_FILE *sp;
    SP_CHECKSUM cchksum, fchksum;   
    SP_INTEGER lint;
    int add_it = 0;
    
    /* open the file and compute the checksum */
    if ((sp=sp_open(f2,"r")) == SPNULL) {
	(void) fprintf(stderr,"%s: %s: sp_open failed.\n",prog,f2);
	sp_print_return_status(stderr);
	return -1; 
    }
    if (sp_compute_checksum(sp, &cchksum) != 0){
	(void) fprintf(stderr,"%s: %s: sp_compute_checksum failed.\n",
		       prog,f2);
	sp_print_return_status(stderr);
	sp_close(sp);
	return -1; 
    }
    /* check to see if a checksum exists, if it does, there's nothing */
    /* else to do.  */
    if (sp_h_get_field(sp,"sample_checksum",
		       T_INTEGER,(void *)&fchksum)!=0){
	add_it=1;
    }    
    if (sp_close(sp) != 0){
	(void) fprintf(stderr,"%s: %s: initial sp_close failed.\n",
		       prog,f2);
	return -1; 
    }

    /* effect the change */
    if (add_it){
	if ((sp=sp_open(f2,"u")) == SPNULL) {
	    (void) fprintf(stderr,"%s: %s: update sp_open failed.\n",prog,f2);
	    sp_print_return_status(stderr);
	    return -1; 
	}
	lint = (SP_INTEGER)cchksum;
	if (sp_h_set_field(sp,"sample_checksum",
			   T_INTEGER,(void *)&lint) >= 100){
	    (void) fprintf(stderr,"%s: %s: sp_h_set_field failed.\n",
			   prog,f2);
	    sp_print_return_status(stderr);
	    sp_close(sp);
	    return -1; 
	}    
	if (sp_close(sp) != 0){
	    (void) fprintf(stderr,"%s: %s: update sp_close failed.\n",
			   prog,f2);
	    sp_print_return_status(stderr);
	    return -1; 
	}
    }
    return 0;
}
