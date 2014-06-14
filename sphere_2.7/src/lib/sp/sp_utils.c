
/** File: sp_utils.c **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define SPHERE_LIBRARY_CODE
#include <sp/sphere.h>

extern int farray_fields;
extern struct field_t *farray[];

/***************************************************************/
/* Reads an existing header in from file pointer "fp".         */
/* The file pointer is assumed to be positioned at the         */
/*    beginning of a speech file with a header in NIST SPHERE  */
/*    format.                                                  */
/* On success, "fp" is positioned at the end of the header     */
/*    (ready to read samples) and a pointer to a header        */
/*    structure is returned.                                   */
/* On failure, argument "error" will point to a string         */
/*    describing the problem.                                  */
/* If "parse_flag" is false (zero), the fields in the header   */
/*    will not be parsed and inserted into the header          */
/*    structure; the structure will contain zero fields.       */
/*    This is useful for operations on files when the contents */
/*    of the header are not important, for example when        */
/*    stripping the header.                                    */
/***************************************************************/

struct header_t *sp_open_header(register FILE *fp, int parse_flag,
				char **error)
{
register struct header_t *h;
int header_size, i;
struct field_t **fv;

if (fp == FPNULL)	return HDRNULL;	/* check sanity of arguments */

if (spx_read_header(fp,&header_size,parse_flag,error) < 0)
	return HDRNULL;

if ((! parse_flag) || (farray_fields == 0))
	fv = FVNULL;
else {
	fv = spx_get_field_vector(farray_fields);
	if (fv == FVNULL) {
		for (i=0; i<farray_fields; i++)
			(void) spx_deallocate_field(farray[i]);
		return HDRNULL;
	}
	(void) spx_copy_field_vector(farray, fv, farray_fields);
}

h = spx_allocate_header(farray_fields,fv);
if (h == HDRNULL)
	for (i=0; i<farray_fields; i++)
		(void) spx_deallocate_field(farray[i]);
return h;
}

/*******************************************************************/
/* Deletes all fields from the header pointed to by h.             */
/*******************************************************************/

int sp_clear_fields(register struct header_t *h)
{
register int i, j, errors = 0;

if (h == HDRNULL)	return -1;	/* check sanity of arguments */

for (i=0, j = h->fc; i<j; i++) {
	if (spx_deallocate_field(h->fv[i]) < 0)
		errors++;
	h->fv[i] = FNULL;
}
if (h->fv != FVNULL)
	mtrf_free((char *) h->fv);
h->fv = FVNULL;
h->fc = 0;

return errors ? -1 : 0;
}

/***********************************************************************/
/* Reclaims the space allocated for the header structure pointed to    */
/* by h. First reclaims all space allocated for the header's fields,   */
/* if any exist.                                                       */
/***********************************************************************/

int sp_close_header(register struct header_t *h)
{
    (void) sp_clear_fields(h);
    mtrf_free((char *) h);
    return 0;
}

/**************************************************************************/
/*    make an exact copy of the header pointed to by h, and then return   */
/*    the new header.                                                     */
/**************************************************************************/
struct header_t *sp_dup_header(struct header_t *h)
{
    struct header_t *duph;
    struct field_t *nf, **fv;
    int i;

    if (h == HDRNULL) return(0);
    duph = sp_create_header();
    if ( duph == HDRNULL ) {
	fprintf(spfp,"Error: Unable to dup header, can't allocate mem.\n");
	return(HDRNULL);
    }

    /* just loop through all the names, adding each field */
    for (i=0; i < h->fc ; i++){
        nf = spx_allocate_field_str(h->fv[i]->type,h->fv[i]->name,
				    h->fv[i]->data,h->fv[i]->datalen);
	if (nf == FNULL)
	    return(HDRNULL);
	fv = spx_get_field_vector(duph->fc + 1);
        if (fv == FVNULL)
	    return(HDRNULL);
	if (duph->fc > 0) {
	    (void) spx_copy_field_vector(duph->fv, fv, duph->fc);
	    mtrf_free((char *) duph->fv);
	}
	fv[duph->fc++] = nf;
	duph->fv = fv;
    }
    return(duph);
}

int sp_copy_header(SP_FILE *spin, SP_FILE *spout)
{
    char *proc_name="sp_copy_header " SPHERE_VERSION_STR;
    struct header_t *h;
    int i;
    SP_INTEGER l_int;
    SP_REAL real;

    if (spin->open_mode == SP_mode_read)
	h = spin->read_spifr->header;
    else if (spin->open_mode == SP_mode_write)
	h = spin->write_spifr->header;
    else
	return_err(proc_name,100,100,"Unable to dup header opened for update");

    /* just loop through all the names, adding each field */
    for (i=0; i < h->fc ; i++){
	switch (h->fv[i]->type){
	  case T_STRING:
	    if (sp_h_set_field(spout,h->fv[i]->name,
			       h->fv[i]->type,h->fv[i]->data) != 0){
		sp_print_return_status(spfp);
		return_err(proc_name,200,200,
			   rsprintf("Unable to copy STRING field '%s'",
				    h->fv[i]->name));
	    }
	    break;
	  case T_INTEGER:
	    l_int=atol(h->fv[i]->data);
	    if (sp_h_set_field(spout,h->fv[i]->name,
			       h->fv[i]->type,&l_int) != 0){
		sp_print_return_status(spfp);
		return_err(proc_name,200,200,
			   rsprintf("Unable to copy INTEGER field '%s'",
				    h->fv[i]->name));
	    }
	    break;
	  case T_REAL:
	    real=atof(h->fv[i]->data);
	    if (sp_h_set_field(spout,h->fv[i]->name,h->fv[i]->type,&real) != 0){
		sp_print_return_status(spfp);
		return_err(proc_name,200,200,
			   rsprintf("Unable to copy REAL field '%s'",
				    h->fv[i]->name));
	    }
	    break;
	}
    }
    /* Special Check, if the input file is a pipe and the sample_count  */
    /* field is missing, (this is legal), AND the output is a disk file */
    /* add a dummy sample_count field to the output header              */
    /*   Added June 22, 1994 */
    {   int type, size, is_disk_file, out_is_disk_file;
	is_disk_file = (spin->open_mode == SP_mode_read) ? 
	    spin->read_spifr->status->is_disk_file :
		spin->write_spifr->status->is_disk_file;
	out_is_disk_file = (spout->open_mode == SP_mode_read) ? 
	    spout->read_spifr->status->is_disk_file :
		spout->write_spifr->status->is_disk_file;
	if (! is_disk_file)
	    if (sp_get_field(h,SAMPLE_COUNT_FIELD,&type,&size) < 0){
		/* add the field to the header */
		l_int = 999999999;
		if (sp_h_set_field(spout,SAMPLE_COUNT_FIELD,
				   T_INTEGER,&l_int) != 0){
		    sp_print_return_status(spfp);
		    return_err(proc_name,400,400,
			       rsprintf("Unable to copy INTEGER field '%s'",
				    h->fv[i]->name));
		}
	    }
    }
    if (sp_set_default_operations(spout) != 0)
	return_err(proc_name,300,300,
		   "Unable to set default operations duplicated file");
    return_success(proc_name,0,0,"ok");
}


/*********************************************************************/
/* Returns the number of fields stored in the specified header.      */
/*********************************************************************/

int sp_get_nfields(struct header_t *h)
{
if (h == HDRNULL)	return -1;	/* check sanity of arguments */

return h->fc;
}

/*********************************************************************/
/* Fills in an array of character pointers with addresses of the     */
/* fields in the specified header. No more than n pointers in the    */
/* array will be set.                                                */
/* Returns the number of pointers set.                               */
/*********************************************************************/

int sp_get_fieldnames(struct header_t *h, int n, char **v)
{
register struct field_t **fv;
int i, fc;

if (h == HDRNULL)
	return -1;	/* check sanity of arguments */
if (v == (char **) NULL)
	return -1;

fc = h->fc;
fv = h->fv;
for (i=0; i < fc && i < n; i++)
	v[i] = fv[i]->name;
return i;
}

/***********************************************************************/
/* Returns the type and size (in bytes) of the specified header field. */
/* Types are T_INTEGER, T_REAL, T_STRING (defined in header.h).        */
/* The size of a T_INTEGER field is sizeof(SP_INTEGER).                */
/* The size of a T_REAL field is sizeof(SP_REAL).                      */
/* The size of a string is variable and does not includes a            */
/*    null-terminator byte (null bytes are allowed in a string).       */
/***********************************************************************/

int sp_get_field(struct header_t *h, char *name, int *type, int *size)
{
register int i, fc;
register struct field_t **fv;

if (h == HDRNULL)	return -1;	/* check sanity of arguments */
if (name == CNULL)	return -1;

fc = h->fc;
fv = h->fv;
for (i=0; i < fc ; i++, fv++)
	if (strcmp(name,(*fv)->name) == 0) {
		switch ((*fv)->type) {
			case T_INTEGER:
				*size = sizeof(SP_INTEGER);
				break;
			case T_REAL:
				*size = sizeof(SP_REAL);
				break;
			case T_STRING:
				*size = (*fv)->datalen;
				break;
			default:
				return -1;
		}
		*type = (*fv)->type;
		return 0;
	}
return -1;
}

/*********************************************************************/
/* Returns the type of the specified header field.                   */
/* Types are T_INTEGER, T_REAL, T_STRING (defined in header.h).      */
/*********************************************************************/

int sp_get_type(struct header_t *h, char *name)
{
register int i, fc;
register struct field_t **fv;

if (h == HDRNULL)	return -1;	/* check sanity of arguments */
if (name == CNULL)	return -1;

fc = h->fc;
fv = h->fv;
for (i=0; i < fc ; i++, fv++)
	if (strcmp(name,(*fv)->name) == 0)
		switch ((*fv)->type) {
			case T_INTEGER:
			case T_REAL:
			case T_STRING:
				return (*fv)->type;
			default:
				return -1;
		}
return -1;
}

/*********************************************************************/
/* Returns the size (in bytes) of the specified header field.        */
/* The size of a T_INTEGER field is sizeof(SP_INTEGER).              */
/* The size of a T_REAL field is sizeof(SP_REAL).                    */
/* The size of a string is variable and does not includes a          */
/*    null-terminator byte (null bytes are allowed in a string).     */
/*********************************************************************/

int sp_get_size(struct header_t *h, char *name)
{
register int i, fc;
register struct field_t **fv;

if (h == HDRNULL)	return -1;	/* check sanity of arguments */
if (name == CNULL)	return -1;

fc = h->fc;
fv = h->fv;
for (i=0; i < fc ; i++, fv++)
	if (strcmp(name,(*fv)->name) == 0)
		switch ((*fv)->type) {
			case T_INTEGER:
				return sizeof(SP_INTEGER);
			case T_REAL:
				return sizeof(SP_REAL);
			case T_STRING:
				return (*fv)->datalen;
			default:
				return -1;
		}
return -1;
}

/***********************************************************************/
/* Returns the value of the specifed header field in "buf".            */
/* No more than "len" bytes are copied; "len" must be positive.        */
/* It really doesn't make much sense to ask for part of a SP_INTEGER   */
/*    or SP_REAL, but it's not illegal.                                */
/* Remember that strings are not null-terminated.                      */
/***********************************************************************/

int sp_get_data(struct header_t *h, char *name, char *buf, int *len)
{
register struct field_t **fv;
register int i, fc;
SP_INTEGER n;
SP_REAL x;

if (h == HDRNULL)	return -1;	/* check sanity of arguments */
if (name == CNULL)	return -1;
if (buf == CNULL)	return -1;
if (len == INULL)	return -1;
if (*len <= 0)		return -1;

fc = h->fc;
fv = h->fv;
for (i=0; i<fc; i++, fv++)
	if (strcmp(name,(*fv)->name) == 0) {
		switch ((*fv)->type) {
			case T_INTEGER:
				n = atol((*fv)->data);
				*len = MIN(*len,sizeof(SP_INTEGER));
				(void) memcpy( buf, (char *) &n, *len );
				break;
			case T_REAL:
				x = atof((*fv)->data);
				*len = MIN(*len,sizeof(SP_REAL));
				(void) memcpy( buf, (char *) &x, *len );
				break;
			case T_STRING:
				*len = MIN(*len,(*fv)->datalen);
				(void) memcpy( buf, (*fv)->data, *len );
				break;
			default:
				return -1;
		}
		return 0;
	}
return -1;
}

/*******************************************************************/
/* Adds the field "name" to header specified by "h".               */
/* Argument "type" is T_INTEGER, T_REAL, or T_STRING.              */
/* Argument "p" is a pointer to a SP_INTEGER integer, a SP_REAL, or*/
/*    a character cast if necessary to a character pointer.        */
/* The specified field must not already exist in the header.       */
/*******************************************************************/

int sp_add_field(struct header_t *h, char *name, int type, char *p)
{
    register struct field_t **fv, *nf;
    int size, i, fc;

    if (h == HDRNULL)		return -1;	/* check sanity of arguments */
    if (h->fc < 0)		return -1;
    if (name == CNULL)		return -1;
    if (p == CNULL)		return -1;
    if (spx_tp(type) == '?')	return -1;

    fc = h->fc;
    for (i=0; i < fc; i++)
	if (strcmp(name,h->fv[i]->name) == 0)
	    return -1;

    switch (type) {
      case T_INTEGER:
	size = sizeof(SP_INTEGER);
	break;
      case T_REAL:
	size = sizeof(SP_REAL);
	break;
      default:
	size = strlen(p);
	break;
    }

    nf = spx_allocate_field(type,name,p,size);
    if (nf == FNULL)
	return -1;

    fv = spx_get_field_vector(fc + 1);
    if (fv == FVNULL) {
	(void) spx_deallocate_field(nf);
	return -1;
    }
    
    if (fc > 0) {
	(void) spx_copy_field_vector(h->fv, fv, fc);
	mtrf_free((char *) h->fv);
    }
    fv[h->fc++] = nf;
    h->fv = fv;
    return 0;
}

/***********************************************************/
/* Deletes field "name" from header specified by "h".      */
/* The field must exist in the header.                     */
/***********************************************************/

int sp_delete_field(struct header_t *h, char *name)
{
    struct field_t **newfv, *nf;
    int i, newfc;

    if (h == HDRNULL)	return -1;	/* check sanity of arguments */
    if (h->fc <= 0)	return -1;
    if (name == CNULL)	return -1;

    if (h->fc > 1) {
	/* one less field will be left */
	newfv = spx_get_field_vector(h->fc - 1);
	if (newfv == FVNULL)
	    return -1;
    } else
	newfv = FVNULL;			/* won't have any fields left */

    nf = FNULL;
    for (i=0, newfc=0; i < h->fc; i++)
	if (strcmp(name,h->fv[i]->name) == 0) {
	    if (nf != FNULL) {
		mtrf_free((char *) newfv);
		return -1;
	    }
	    nf = h->fv[i];
	} else {
	    if ((nf == FNULL) && (i == h->fc - 1)) {
		mtrf_free((char *) newfv);
		return -1;
	    }
	    newfv[newfc++] = h->fv[i];
	}

    (void) spx_deallocate_field(nf);
    mtrf_free((char *) h->fv);
    h->fv = newfv;
    --h->fc;
    return 0;
}

/***********************************************************/
/* Changes an existing field to a new type and/or value.   */
/* The field must already exist in the header.             */
/***********************************************************/

int sp_change_field(struct header_t *h, char *name, int type, char *p)
{
register int i, field_index, size;
struct field_t *nf;

if (h == HDRNULL)		return -1;	/* check sanity of arguments */
if (name == CNULL)		return -1;
if (p == CNULL)			return -1;
if (spx_tp(type) == '?')	return -1;

for (i=0, field_index = -1; i< h->fc; i++)
	if (strcmp(h->fv[i]->name,name) == 0) {
		if (field_index >= 0)
			return -1;
		field_index = i;
	}

if (field_index < 0)			/* field not found */
	return -1;

switch (type) {
	case T_INTEGER:
		size = sizeof(SP_INTEGER); break;
	case T_REAL:
		size = sizeof(SP_REAL); break;
	default:
		size = strlen(p); break;
}

nf = spx_allocate_field(type,name,p,size);
if (nf == FNULL) return -1;

if (spx_deallocate_field(h->fv[field_index]) < 0) {
	(void) spx_deallocate_field(nf);
	return -1;
}
h->fv[field_index] = nf;
return 0;
}

/******************************************************************/
/* Returns a pointer to an empty header.                          */
/* Use sp_add_field() to insert fields into it.                   */
/* Use sp_print_header() to print it in readable format.          */
/* Use sp_format_header() to print it to a file in NIST SPHERE    */
/*      format.                                                   */
/******************************************************************/

struct header_t *sp_create_header(void)
{
return spx_allocate_header(0,FVNULL);
}

/*******************************************************************/
/* Returns TRUE if the specified field name is a "standard" field, */
/* FALSE otherwise.                                                */
/* Standard fields are listed in stdfield.c.                       */
/*******************************************************************/

int sp_is_std(register char *name)
{
register char **f;

if (name == CNULL)
	return FALSE;

f = &std_fields[0];
while (*f != CNULL)
	if (strcmp(name,*f++) == 0)
		return TRUE;

return FALSE;
}

/**********************************************************************/
/* Returns the size of the file's sphere header in bytes              */
/**********************************************************************/
SP_INTEGER sp_file_header_size(char *file)
{
    FILE *fp;
    struct fileheader_fixed fh;

    if (file == CNULL)
	return -1;

    fp = fopen(file,"r");
    if (fp == FPNULL)
	return -1;

    if (fread((char *)&fh,1,sizeof(fh),fp) != sizeof(fh)) {
	(void) fclose(fp);
	return -1;
    }

    (void) fclose(fp);

    fh.header_size[HDR_SIZE_SIZE-1] = '\0';
    return atol(fh.header_size);
}

/**********************************************************************/
/* Returns the size of the header if it were to be written to a file  */
/**********************************************************************/
SP_INTEGER sp_header_size(struct header_t *h)
{
    FILE *fp;
    SP_INTEGER header_size, data_size;    

    if (h == HDRNULL)
	return -1;

    fp = fopen(rsprintf("%s/%s.sz",TEMP_DIR,TEMP_BASE_NAME),"w");
    if (fp == FPNULL){
	if (sp_verbose > 15)
	    fprintf(spfp,"Can't open file '%s'\n",
		   rsprintf("%s/%s.sz",TEMP_DIR,TEMP_BASE_NAME));
	return -1;
    }
    if (sp_write_header(fp,h,&header_size,&data_size) < 0){
	fclose(fp);
	unlink(rsprintf("%s/%s.sz",TEMP_DIR,TEMP_BASE_NAME));
	return(-1);
    }
    fclose(fp);
    unlink(rsprintf("%s/%s.sz",TEMP_DIR,TEMP_BASE_NAME));
    return(header_size);
}

