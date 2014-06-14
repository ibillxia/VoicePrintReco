
/* LINTLIBRARY */

/** File: spinput.c **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>

#define SPHERE_LIBRARY_CODE
#include <sp/sphere.h>
#include <util/chars.h>

int farray_fields;
struct field_t *farray[MAXFIELDS];
char *header = CNULL;

static int parse_header(register char *p, int hsize, int *fields,
			char **error);
static char *parse_line(char *h, char *t, char *v, char **error);

/*******************************************************************/
/* Reads a NIST header from file pointer "fp" into a buffer, then  */
/*    calls parse_header() to parse the buffer if "parse_flag" is  */
/*    set.                                                         */
/* On failure, "*error" is set to a string describing the error.   */
/*******************************************************************/

int spx_read_header(FILE *fp, int *header_size, int parse_flag, char **error)
{
char *p;
struct fileheader_fixed fh;
int hsize, hfields;

if (fp == FPNULL) {
	*error = "File pointer is null";
	goto errexit;
}
if (fread((char *) &fh, sizeof fh, 1, fp) != 1) {
	*error = "Fread for fixed part of header failed";
	goto errexit;
}
if (fh.header_id[sizeof(fh.header_id) - 1] != '\n') {
	*error = "Bad header label line";
	goto errexit;
}
if (strncmp(fh.header_id,NISTLABEL,sizeof(NISTLABEL)-1) != 0) {
	*error = "Bad header label";
	goto errexit;
}
if (fh.header_size[sizeof(fh.header_size) - 1] != '\n') {
	*error = "Bad header size line";
	goto errexit;
}
p = fh.header_size;
while ((p < &fh.header_size[sizeof(fh.header_size)-1]) && (*p == ' '))
	p++;
if (! isdigit(*p)) {
	*error = "Bad header size specifier";
	goto errexit;
}
hsize = atoi(p);
if (hsize < sizeof fh) {
	*error = "Specified header size is too small";
	goto errexit;
}
header = mtrf_malloc((u_int) (hsize - sizeof fh + 1));
if (header == CNULL) {
	*error = "Malloc for header failed";
	goto errexit;
}
if (fread(header,hsize-sizeof fh,1,fp) != 1) {
	*error = "Can't read entire header into memory";
	goto errexit;
}
if (parse_flag && (parse_header(header,hsize,&hfields,error) < 0))
	goto errexit;

mtrf_free(header);
header = CNULL;
if (header_size != INULL)
	*header_size = hsize;
return 0;

errexit:
	if (header != CNULL) {
		mtrf_free(header);
		header = CNULL;
	}
	return -1;
}

/************************************************************/
/* Parses the bytes read from a speech file and inserts the */
/*    fields into "farray".                                 */
/* If the parsing finishes with success, the fields should  */
/*    then be copied into a header structure for the user.  */
/************************************************************/

static int parse_header(register char *p, int hsize, int *fields,
			char **error)
{
register char *lim = p + (hsize - sizeof(struct fileheader_fixed));
int i, remaining;

farray_fields = 0;
for (i = 0; i < MAXFIELDS; i++)
	farray[i] = FNULL;

*lim = '\0';		/* by setting last character in buffer to NULL,   */
*fields = 0;		/* strchr() can be used at any point in buffer w/o */
			/* accessing potentially-illegal addresses        */

while (p < lim) {
	remaining = lim - p;
	if (remaining < sizeof(ENDSTR)-1) {
		*error = "Bad header end";
		return -1;
	}
	if (*p == COMMENT_CHAR) {
		while ((p < lim) && (*p != '\n'))
			p++;
		if (p < lim) p++;
	} else if (isalpha(*p)) {
		register char *t, *v;

		if ((strncmp(p,ENDSTR,sizeof(ENDSTR)-1) == 0) &&
			((remaining == sizeof(ENDSTR)-1) ||
			(*(p+sizeof(ENDSTR)-1) == ' ') ||
			(*(p+sizeof(ENDSTR)-1) == '\n')))
				return 0;
		t = strchr(p,' ');
		if (t == CNULL) {
			*error = "space expected after field name";
			return -1;
		}
		v = strchr(t+1,' ');
		if (v == CNULL) {
			*error = "space expected after type specifier";
			return -1;
		}
		p = parse_line(p,t,v,error);
		if (p == CNULL)
			return -1;
		++*fields;
	} else {
		*error = "Bad character at beginning of line";
		return -1;
	}
}
return 0;
}

/*********************************************************************/
/* Parses a line from a speech file.                                 */
/* The arguments to parse_line() point into a line in the header     */
/*    buffer as follows:                                             */
/*                                                                   */
/*	field type value[;.....]\n                                   */
/* 	^    ^    ^                                                  */
/*	h    t    v                                                  */
/*********************************************************************/

static char *parse_line(char *h, char *t, char *v, char **error)
{
struct field_t *f;
int vtype, vlen;
char *endofvalue = v + 1, *endoffieldname = h, *ptr;

if (farray_fields >= MAXFIELDS) {
	*error = "too many fields";
	return CNULL;
}
*t = '\0';
while (isalnum(*endoffieldname) || (*endoffieldname == '_'))
	endoffieldname++;
if (endoffieldname != t) {
	*error = "space expected after field name";
	return CNULL;
}
if (*(t+1) != '-') {
	*error = "dash expected in type specifier";
	return CNULL;
}
switch (*(t+2)) {
	case 'i':
		vtype = T_INTEGER;
		while (isdigit(*endofvalue) || (*endofvalue == '-'))
			++endofvalue;
		vlen = endofvalue - (v + 1);
		break;
	case 'r':
		vtype = T_REAL;
		while (isdigit(*endofvalue) ||
			(*endofvalue == '.') ||
			(*endofvalue == '-'))
				++endofvalue;
		vlen = endofvalue - (v + 1);
		break;
	case 's':
		vtype = T_STRING;
		vlen = 0;
		ptr = t + 3;
		while (isdigit(*ptr))
			vlen = 10 * vlen + (*ptr++ - '0');
		if (! vlen) {
			*error = "bad string length";
			return CNULL;
		}
		if (ptr != v) {
			*error = "space expected after type specifier";
			return CNULL;
		}
		endofvalue = v + vlen + 1;
		break;
	default:
		*error = "unknown type specifier";
		return CNULL;
}

{
/* Really only need the function call, but by null-terminating the     */
/* string at (v+1), looking at a stack trace in "dbx" (a BSD Unix      */
/* debugger) is easier. Otherwise, "dbx" will expect a null-terminator */
/* and print the rest of the header block.                             */

int ch = *(v + 1 + vlen);

*(v + 1 + vlen) = '\0';
f = spx_allocate_field_str(vtype,h,v+1,vlen);
*(v + 1 + vlen) = ch;
}

if (f == FNULL) {
	*error = "Malloc for triple failed";
	return CNULL;
}
farray[farray_fields++] = f;

switch (*endofvalue) {
	case COMMENT_CHAR:
	case '\n':
		return endofvalue + 1;
	case ' ':
		while (*endofvalue == ' ')
			++endofvalue;
		if (*endofvalue == '\n')
			return endofvalue + 1;
		if (*endofvalue == COMMENT_CHAR) {
			char *eoln = strchr(endofvalue,'\n');
			if (eoln != CNULL)
				return eoln + 1;
		}
		*error = "bad character after triple and space(s)";
		return CNULL;
}
*error = "bad character after triple";
return CNULL;
}

/********************************************************/

struct header_t *spx_allocate_header(int fc, struct field_t **fv)
{
register struct header_t *h;

h = (struct header_t *) mtrf_malloc((u_int) sizeof(struct header_t));
if (h != HDRNULL) {
	h->fc = fc;
	h->fv = fv;
}
return h;
}

/**********************************************************/
/* Deallocates a header by freeing the structure that     */
/*    represents it.                                      */
/**********************************************************/

int spx_deallocate_header(struct header_t *h)
{
if (h == HDRNULL)	return -1;	/* check sanity of arguments */

mtrf_free((char *) h);
return 0;
}

/**********************************************************/
/* Allocates room for a field with name "fieldname", type */
/*    "type" represented at address "v" and comprising    */
/*    "vlen" bytes.                                       */
/**********************************************************/

struct field_t *spx_allocate_field(int type, char *fieldname, char *v, int vlen)
{
char buffer[1024];
	/* plenty big enough for storing ascii form of numbers */

if (fieldname == CNULL) return FNULL;
if (v == CNULL) return FNULL;

switch (type) {
	case T_INTEGER:
		sprintf(buffer,"%ld",*(SP_INTEGER *)v);
		break;
	case T_REAL:
		sprintf(buffer,"%f",*(SP_REAL *)v);
		break;
	case T_STRING:
		if (vlen <= 0)
			return FNULL;
		return spx_allocate_field_str(type,fieldname,v,vlen);
		break;
	default:
		return FNULL;
}
return spx_allocate_field_str(type,fieldname,buffer,strlen(buffer));
}

/***************************************************************/

struct field_t *spx_allocate_field_str(int type, char *fieldname,
				       char *value, int vlen)
{
register struct field_t *f;
char *p1, *p2;

if (vlen <= 0)
	return FNULL;

f = (struct field_t *) mtrf_malloc((u_int) sizeof(struct field_t));
if (f == FNULL)
	return FNULL;

p1 = mtrf_malloc((u_int) (strlen(fieldname) + 1));
if (p1 == CNULL) {
	mtrf_free((char *) f);
	return FNULL;
}

p2 = mtrf_malloc((u_int) (vlen + 1));
if (p2 == CNULL) {
	mtrf_free((char *) f);
	mtrf_free(p1);
	return FNULL;
}

f->type = type;
f->name = p1;
f->data = p2;
f->datalen = vlen;
(void) strcpy(p1,fieldname);
(void) memcpy(p2,value,vlen);
p2[vlen] = '\0';

return f;
}

/*******************************************************************/
/* Deallocates a field by freeing bytes used to store the field    */
/*    name and value, then freeing bytes that were allocated for   */
/*    the structure.                                               */
/*******************************************************************/

int spx_deallocate_field(struct field_t *fv)
{
    if (fv == FNULL)	return -1;	/* check sanity of arguments */

    mtrf_free(fv->name);
    mtrf_free(fv->data);
    mtrf_free((char *) fv);
    return 0;
}

/******************************************************************/
/* Returns a pointer to a vector of field structures for the      */
/*    specified number of elements.                               */
/******************************************************************/

struct field_t **spx_get_field_vector(int elements)
{
    int vbytes;

    if (elements <= 0)
	return FVNULL;
    vbytes = elements * sizeof(struct field_t *);
    return (struct field_t **) mtrf_malloc((u_int) vbytes);
}

/**********************************************************/
/* Copies field vector "src_fv" to field vector "dst_fv". */
/* The number of fields in the source vector must be      */
/*    specified by "elements", a positive number.         */
/**********************************************************/

int spx_copy_field_vector(struct field_t **src_fv, struct field_t **dst_fv, 
			  int elements)
{
    int vbytes;

    if (elements <= 0)
	return -1;
    vbytes = elements * sizeof(struct field_t *);
    (void) memcpy( (char *) dst_fv, (char *) src_fv, vbytes );
    return 0;
}
