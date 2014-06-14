
/* LINTLIBRARY */

/** File: spoutput.c **/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>

#define SPHERE_LIBRARY_CODE
#include <sp/sphere.h>

#define NAMEWIDTH	30
/** Field names are printed to this width, left-justified, in */
/** sp_print_lines().                                         */

/****  This function is exempt from the required use of spfp ****/
int spx_write_header(struct header_t *h, struct fileheader_fixed *fh, 
		     FILE *fp, int set, SP_INTEGER *hpos, SP_INTEGER *d)
{
    SP_INTEGER p, pbytes;
    int c;
    
    clearerr(fp);
    (void) fwrite((char *)fh,sizeof(struct fileheader_fixed),1,fp);
    if (sp_format_lines(h,fp) < 0)
	return -1;
    (void) fprintf(fp,"%s\n",ENDSTR);
    
    if (set)
	*d = ftell(fp);
    
    for (c=0; c < PAD_NEWLINES; c++)	/* pad with newlines always */
	(void) putc('\n',fp);
    pbytes = PAD_MULT - ((*d + PAD_NEWLINES) % PAD_MULT);
    for (p=0; p < pbytes; p++)	/* pad to multiple of PAD_MULT */
	(void) putc(PAD_CHAR,fp);
    
    if (set) {
	*hpos = ftell(fp);
	if (*hpos % PAD_MULT != 0)
	    return -1;
    }
    
    (void) fflush(fp);
    if (ferror(fp)) return -1;
    
    return 0;
}

/******************************************************************/
/* Prints the specified header to stream fp in the standard       */
/* format that will allow "sp_open_header() to read it back       */
/* in later.                                                      */
/* More specifically:                                             */
/* writes "header_end" after the last field, then adds some       */
/* newlines (so pagers can be used to look at the file a          */
/* screenful at a time without seeing the samples themselves),    */
/* and then pads the header to a multiple of PAD_MULT bytes.      */
/* That constant is #define'd in sysparam.h, currently at 1024.   */
/* Padding is accomplished via ftell(), so it won't work on       */
/* systems where ftell() doesn't return byte offsets.             */
/******************************************************************/

FUNCTION int sp_write_header(register FILE *fp, struct header_t *h, 
			     SP_INTEGER *hbytes, SP_INTEGER *databytes)
{
    register FILE *tfp;
    char *tfile;
    int n;
    SP_INTEGER d, hpos;
    struct fileheader_fixed fh;
    
    if (fp == FPNULL)	return -1;	/* check sanity of arguments */
    if (h == HDRNULL)	return -1;
    if (hbytes == LNULL)	return -1;
    if (databytes == LNULL)	return -1;
    
    tfile = sptemp_dirfile();		/* create temp filename to format */
    if (tfile == (char *) NULL) return -1;  /* header (need to know size) */
    
    tfp = fopen(tfile,"w+");		/* open the temp file */
    if (tfp == (FILE *) NULL)
	return -1;
    
    memset( (char *)&fh, 0, sizeof fh );
    n = spx_write_header(h,&fh,tfp,1,&hpos,&d);
    (void) fclose(tfp);			/* close temp file */
    (void) unlink(tfile);			/* delete temp file */
    if (n < 0)
	return -1;
    /* the filename must not be free'd until after the file is unlinked */
    mtrf_free(tfile);
    
/**** Bug: JGF, Sept 12.  this sprintf causes some machines/compilers
      to royally get messed up.  The bug report, by Paul F. Werkowski
      (pw@snoopy.mv.com), found that it wrote an extra '\0' character
      onto the stack, thus causing a core dump.  

    sprintf((char *) &fh,"%*s\n%*ld\n",
	    sizeof(fh.header_id)-1,		NISTLABEL,
	    sizeof(fh.header_size)-1,	hpos);
****/
     { char tmpstr[sizeof fh +1];
       sprintf((char *) tmpstr,"%*s\n%*ld\n",
	       (int)sizeof(fh.header_id)-1,NISTLABEL,
	       (int)sizeof(fh.header_size)-1,hpos);
       memcpy((char*)&fh,tmpstr,sizeof fh);
     }

    n = spx_write_header(h,&fh,fp,0,&hpos,&d);
    if (n < 0)
	return -1;
    
    *hbytes = hpos;		/* on success, return #bytes in header */
    *databytes = d;		/* and #bytes before padding           */
    
    return 0;
}

/*********************************************************************/
/* Dumps header fields to stream fp in a readable form:              */
/*        fieldnumber: fieldtype fieldname "field_value"             */
/*   where fieldtype is either 's', 'i' or 'r'.                      */
/* Field numbers start at 0.                                         */
/* Beware printing headers with unprintable characters in data       */
/*   fields using this function.                                     */
/*********************************************************************/

/****  This function is exempt from the required use of spfp ****/
FUNCTION int sp_print_lines(struct header_t *h, register FILE *fp)
{
    register int i, fc;
    register struct field_t **fv;
    int len, j;
    char *p;
    
    if (h == HDRNULL)	return -1;	/* check sanity of arguments */
    if (fp == FPNULL)	return -1;
    
    clearerr(fp);
    fv = h->fv;
    fc = h->fc;
    for (i=0; i < fc; i++) {
	len = strlen(fv[i]->name);
	if (len <= NAMEWIDTH)
	    (void) fprintf(fp,"%5d: %c %-*s ",i,spx_tp(fv[i]->type),
			   NAMEWIDTH,fv[i]->name);
	else
	    (void) fprintf(fp,"%5d: %c %s ",i,spx_tp(fv[i]->type),
			   fv[i]->name);
	for (j=0, p=fv[i]->data; j < fv[i]->datalen; j++, p++)
	    if (isprint(*p) || (*p == '\n') || (*p == '\t'))
		(void) putc(*p,fp);
	    else
		(void) fprintf(fp,"%c%o",'\0',*p); /* changed by JGF to make */
	                                           /* the null byte be an    */
                                                   /* arguement              */
	(void) fprintf(fp,"\n");
    }
    if (ferror(fp)) {
	clearerr(fp);
	return -1;
    }
    return 0;
}

/********************************************************************/
/* Write header fields to stream fp in NIST SPHERE format.          */
/* Format is basically: name -type value.                           */
/* See documentation for the complete header grammar.               */
/********************************************************************/

/****  This function is exempt from the required use of spfp ****/
FUNCTION int sp_format_lines(struct header_t *h, register FILE *fp)
{
    int i, j, fc;
    char *p;
    register struct field_t **fv;
    
    if (h == HDRNULL)	return -1;	/* check sanity of arguments */
    if (fp == FPNULL)	return -1;
    
    clearerr(fp);
    fv = h->fv;
    fc = h->fc;
    for (i=0; i < fc; i++) {
	(void) fprintf(fp,"%s -%c",fv[i]->name,spx_tp(fv[i]->type));
	if (fv[i]->type == T_STRING)
	  (void) fprintf(fp,"%ld",(long int) strlen(fv[i]->data));
	(void) fprintf(fp," ");
	for (j=0, p=fv[i]->data; j < fv[i]->datalen; j++, p++)
	    (void) putc(*p,fp);
	(void) fprintf(fp,"\n");
    }
    if (ferror(fp)) {
	clearerr(fp);
	return -1;
    }
    return 0;
}

/************************************************************/
/** Returns a character that represents the type associated */
/** with the integer parameter, '?' for bad type.           */
/************************************************************/

int spx_tp(register int ftype)
{
    register int result;
    
    switch (ftype) {
      case T_INTEGER:
	result = 'i'; break;
      case T_REAL:
	result = 'r'; break;
      case T_STRING:
	result = 's'; break;
      default:
	result = '?'; break;
    }

    return result;
}

/**************************************************************/
/* Copies stream fp to stream outfp through EOF.              */
/* Returns -1 if an error occurs (either reading or writing). */
/**************************************************************/
/*  Updated to use fread and fwrite, not getc and putc !!!    */
FUNCTION int sp_fpcopy(register FILE *fp, register FILE *outfp)
{
    char buf[2048];
    int n;

    while ((n = fread(buf,1,2048,fp)) > 0){
        if (fwrite(buf,1,n,outfp) != n)
            return -1;
    }
    return (ferror(fp) || ferror(outfp)) ? -1 : 0;
}

/**************************************************************/
/* Copies stream fp over the specified file.                  */
/* The file pointer must be positioned at the beginning of    */
/*     file it refers to.                                     */
/* Returns -1 if an error occurs (either reading or writing). */
/**************************************************************/

FUNCTION int sp_overwrite_header(char *src, char *dst, int s)
{
    int n;
    char *p;
    FILE *srcfp, *dstfp;

    if ((src == CNULL) || (dst == CNULL) || (s <= 0))
	return -1;

    p = mtrf_malloc((u_int) (s+1));
    if (p == CNULL)
	return -1;

    srcfp = fopen(src,"r");
    if (srcfp == FPNULL) {
	mtrf_free(p);
	return -1;
    }

    n = fread(p,1,s+1,srcfp);
    if (n != s) {
	mtrf_free(p);
	(void) fclose(srcfp);
	return -1;
    }

    dstfp = fopen(dst,"r+");
    if (dstfp == FPNULL) {
	mtrf_free(p);
	(void) fclose(srcfp);
	return -1;
    }

    n = fwrite(p,1,s,dstfp);

    mtrf_free(p);
    (void) fclose(srcfp);
    (void) fclose(dstfp);

    return (n != s) ? -1 : 0;
}
