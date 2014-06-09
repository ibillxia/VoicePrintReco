/******************************************************************************/
/*                                                                            */
/*                                  header.c                                  */
/*                                                                            */
/*                               SPro Library                                 */
/*                                                                            */
/* Guig                                                             Nov. 2002 */
/* -------------------------------------------------------------------------- */
/*
   $Author: guig $
   $Date: 2010-11-09 16:57:22 +0100 (Tue, 09 Nov 2010) $
   $Revision: 151 $
*/
/*  
   Copyright (C) 1997-2010 Guillaume Gravier (guig@irisa.fr)

   Permission is hereby granted, free of charge, to any person
   obtaining a copy of this software and associated documentation
   files (the "Software"), to deal in the Software without
   restriction, including without limitation the rights to use, copy,
   modify, merge, publish, distribute, sublicense, and/or sell copies
   of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
   BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/

/*
 * SPro feature header.
 *
 * The feature stream header is made of an optionnal variable length
 * header followed by a fixed one. The functions in this file deals
 * with variable length header. The variable length header is a text
 * header which defines fields as name=value pairs. The variable
 * length header syntax is
 *
 * <header> 
 * name = value; # some comments if you want to 
 * </header> 
 *
 * Note that there *must* be one, and only one, carriage return (\n)
 * between the <\header> tag and the first byte of the fixed binary
 * header. 
 *
 * The function spf_header_init() can be used to allocate and
 * initialize a header which is de-allocated with
 * spf_header_free(). Fields are added to the variable length header
 * using spf_header_add(), and the field value is retrieved using
 * spf_header_field_get(). The functions spf_header_read() and
 * spf_header_write() are provided to respectively read and write a
 * header to a Unix stream.
 *
 * Both spf_header_init() and spf_header_add() takes as input a
 * (possibly NULL) NULL terminated array of fields. No control over
 * duplicate field names is made anywhere so that if two fields have
 * the same name, only the first one will be considered.  
 */

#define _header_c_

#include "spro.h"

/* ------------------------------------------------------------------------- */
/* ----- spfheader_t *spf_header_init(const struct spf_header_field *) ----- */
/* ------------------------------------------------------------------------- */
/*
 * Allocate memory and initialize the header.
 */
spfheader_t *spf_header_init(const struct spf_header_field *fld)
{
  spfheader_t *p;
  int i, n = 0;

  if ((p = (spfheader_t *)malloc(sizeof(spfheader_t))) == NULL) {
    fprintf(stderr, "spf_header_init() -- cannot allocate memory\n");
    return(NULL);
  }

  p->nfields = 0;
  p->field = NULL;
  
  if (fld) {
    while (fld[n].name && fld[n].value)
      n++;
    
    if (n) {
      
      /* allocate memory */
      if ((p->field = (struct spf_header_field *)malloc(n * sizeof(struct spf_header_field))) == NULL) {
	fprintf(stderr, "spf_header_add() -- cannot allocate memory\n");
	return(NULL);
      }
      
      for (i = 0; i < n; i++)
	p->field[i].name = p->field[i].value = (char *)NULL;
    
      p->nfields = n;
      
      /* copy fields name and value */
      for (i = 0; i < n; i++)
	if ((p->field[i].name = strdup(fld[i].name)) == NULL || (p->field[i].value = strdup(fld[i].value)) == NULL) {
	fprintf(stderr, "spf_header_add() -- cannot allocate memory\n");
	return(NULL);
	}
    }
  }

  return(p);
}

/* ----------------------------------------------- */
/* ----- void spf_header_free(spfheader_t *) ----- */
/* ----------------------------------------------- */
/*
 * Free memory allocated to the header.
 */
void spf_header_free(spfheader_t *p)
{
  int i;

  if (p) {
    if (p->field) {
      for (i = 0; i < p->nfields; i++) {
	if (p->field[i].name)
	  free(p->field[i].name);
	if (p->field[i].value)
	  free(p->field[i].value);
      }
      free(p->field);
    }
    free(p);
  }
}

/* ------------------------------------------------ */
/* ----- spfheader_t *spf_header_read(FILE *) ----- */
/* ------------------------------------------------ */
/* 
 * Read header from file. Return a pointer to the allocated (possibly
 * empty) header or NULL in case of error.  
 */
spfheader_t *spf_header_read(FILE *f)
{
  spfheader_t *hp;
  int lino = 1;
  char c, line[5120], *p;
  char *name, *value;

  /* create empty header */
  if ((hp = spf_header_init(NULL)) == NULL) {
    fprintf(stderr, "spf_header_read(): cannot allocate memory\n");
    return(NULL);
  }
  
  c = getc(f);
  ungetc(c, f);

  if (c == '<') { /* variable length header */

    if (fgets(line, 5120, f) == NULL) {
      fprintf(stderr, "spf_header_read(): cannot read line %d\n", lino);
      spf_header_free(hp);
      return(NULL);
    }

    if (strcasecmp(line, "<header>\n") != 0) {
      fprintf(stderr, "spf_header_read(): expecting <header> tag at line %d\n", lino);
      spf_header_free(hp);
      return(NULL);
    }

    while (1) {

      lino++;
      if (fgets(line, 5120, f) == NULL) {
	fprintf(stderr, "spf_header_read(): cannot read line %d (maybe a missing </header> tag)\n", lino);
	spf_header_free(hp);
	return(NULL);
      }
      
      if (strcasecmp(line, "</header>\n") == 0)
	break;
      
      /* find out field name and value in line */
      p = line; 
      name = value = NULL;
      while (*p && strchr(" \t\n", *p)) /* skip heading blanks */
	p++;
      
      if (! *p) /* it's an empty line ==> skip it! */
	continue;
      
      /* find out name */
      name = p;
      p++;
      while (*p && strchr(" \t=", *p) == NULL)
	p++;
      
      if (! *p) {
	fprintf(stderr, "spf_header_read(): no separator in variable header at line %d\n", lino);
	spf_header_free(hp);
	return(NULL);
      }
    
      *p = 0;
      p++;
      while (*p && strchr(" \t=", *p))
	p++;
      
      if (! *p) {
	fprintf(stderr, "spf_header_read(): no value for attribute %s in variable header at line %d\n", name, lino);
	spf_header_free(hp);
	return(NULL);
      }
      
      value = p;
      p++;
      while (*p && *p != ';')
	p++;
      
      if (! *p) {
	fprintf(stderr, "spf_header_read(): no end delimiter for attribute %s in variable header at line %d\n", name, lino);
	spf_header_free(hp);
	return(NULL);
      }
      
      *p = 0;


      if (spf_header_field_add(hp, name, value) < 0) {
	fprintf(stderr, "spf_header_read(): cannot set header field\n");
	spf_header_free(hp);
	return(NULL);	
      }
      
    }
  }

  return(hp);
}

/* -------------------------------------------------------- */
/* ------ int spf_header_write(spfheader_t *, FILE *) ----- */
/* -------------------------------------------------------- */
/*
 * Write variable header to file. Return 0 if ok.
 */
int spf_header_write(spfheader_t *hp, FILE *f)
{
  unsigned short i;

  if (hp) {
    if (hp->nfields) {

      if (fprintf(f, "<header>\n") == 0)
	return(SPRO_FEATURE_WRITE_ERR);
      
      for (i = 0; i < hp->nfields; i++)
	if (fprintf(f, "%s = %s;\n", hp->field[i].name, hp->field[i].value) == 0)
	  return(SPRO_FEATURE_WRITE_ERR);
      
      if (fprintf(f, "</header>\n") == 0)
	return(SPRO_FEATURE_WRITE_ERR);
    }
  }

  return(0);
}

/* ------------------------------------------------------------------- */
/* ----- int spf_header_field_index(spfheader_t *, const char *) ----- */
/* ------------------------------------------------------------------- */
/*
 * Return field index or -1 if field is not defined.
 */
int spf_header_field_index(spfheader_t *hp, const char *name)
{
  int i;
  
  for (i = 0; i < hp->nfields; i++)
    if (strcmp(hp->field[i].name, name) == 0)
      return(i);

  return(-1);
}

/* ------------------------------------------------------------------- */
/* ----- char *spf_header_field_get(spfheader_t *, const char *) ----- */
/* ------------------------------------------------------------------- */
/*
 * Return the header field value for specified name or NULL.
 */
char *spf_header_field_get(spfheader_t *hp, const char *name)
{
  int i = spf_header_field_index(hp, name);

  if (i < 0)
    return(NULL);
  
  return(hp->field[i].value);
}

/* ---------------------------------------------------------------------------------------- */
/* ----- int spf_header_str_field_set(spfheader_t *, const char *, const char *, int) ----- */
/* ---------------------------------------------------------------------------------------- */
/*
 * Set a field in the header table. Return the index of the new field
 * or -1 in case of error.
 *
 * If a field already exists, its value is replaced by the new
 * one. Otherwise, a new entry is added unless add is set to 0, in
 * which case -1 is returned.
 */
int spf_header_field_set(spfheader_t *hp, const char *name, const char *value, int add)
{
  int i = spf_header_field_index(hp, name);

  if (i < 0) {

    if (! add)
      return(-1);

    if ((hp->field = (struct spf_header_field *)realloc(hp->field, (hp->nfields + 1) * sizeof(struct spf_header_field))) == NULL) {
      fprintf(stderr, "spf_header_field_set(): cannot allocate memory\n");
      return(-1);
    }
    
    i = hp->nfields;
    hp->field[i].name = NULL;
  }

  /* make new entry */
  if (! hp->field[i].name) {
    if ((hp->field[i].name = strdup(name)) == NULL) {
      fprintf(stderr, "spf_header_field_set(): cannot allocate memory\n");
      return(-1);
    }

    hp->nfields += 1;
  }

  if ((hp->field[i].value = strdup(value)) == NULL) {
    fprintf(stderr, "spf_header_field_set(): cannot allocate memory\n");
    return(-1);
  }

  return(i);
}

#undef _header_c_
