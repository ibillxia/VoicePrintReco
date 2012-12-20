/******************************************************************************/
/*                                                                            */
/*                                  header.c                                  */
/*                                                                            */
/*                               SPro Library                                 */
/*                                                                            */
/* Guig                                                             Nov. 2002 */
/* -------------------------------------------------------------------------- */
/*  Copyright (C) 2002 Guillaume Gravier (ggravier@irisa.fr)                  */
/*                                                                            */
/*  This program is free software; you can redistribute it and/or             */
/*  modify it under the terms of the GNU General Public License               */
/*  as published by the Free Software Foundation; either version 2            */
/*  of the License, or (at your option) any later version.                    */
/*                                                                            */
/*  This program is distributed in the hope that it will be useful,           */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/*  GNU General Public License for more details.                              */
/*                                                                            */
/*  You should have received a copy of the GNU General Public License         */
/*  along with this program; if not, write to the Free Software               */
/*  Foundation, Inc., 59 Temple Place - Suite 330,                            */
/*  Boston, MA  02111-1307, USA.                                              */
/*                                                                            */
/******************************************************************************/
/*
 * CVS log:
 *
 * $Author: ggravier $
 * $Date: 2003/07/25 15:47:07 $
 * $Revision: 1.2 $
 *
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

#include <spro.h>

/* ----------------------------------------------------------- */
/* ----- spfheader_t *spf_header_init(const spfield_t *) ----- */
/* ----------------------------------------------------------- */
/*
 * Allocate memory and initialize the header.
 */
spfheader_t *spf_header_init(const spfield_t *fld)
{
  spfheader_t *p;

  if ((p = (spfheader_t *)malloc(sizeof(spfheader_t))) == NULL) {
    fprintf(stderr, "spf_header_init() -- cannot allocate memory\n");
    return(NULL);
  }

  p->nfields = 0;
  p->field = NULL;
  
  if (fld)
    if (spf_header_add(p, fld) == 0) {
      fprintf(stderr, "spf_header_init() -- cannot add header field\n");
      spf_header_free(p);
      return(NULL);
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

/* ---------------------------------------------------------------- */
/* ----- int spf_header_add(spfheader_t *, const spfield_t *) ----- */
/* ---------------------------------------------------------------- */
/*
 * Add some fields to the header table and return the number of fields
 * added. Return number of fields added or -1 in case of error.
 *
 * WARNING: this function does *not* check for duplicate field names!!!!!  
 */
int spf_header_add(spfheader_t *p, const spfield_t *fld)
{
  spfield_t *fp;
  int i, in, n = 0;

  /* count number of fields */
  if (fld)
    while (fld[n].name && fld[n].value)
      n++;

  
  if (n) {
    in = p->nfields;
    
    /* reallocate memory */
    if ((fp = (spfield_t *)realloc(p->field, (in + n) * sizeof(spfield_t))) == NULL) {
      fprintf(stderr, "spf_header_add() -- cannot allocate memory\n");
      return(-1);
    }

    for (i = 0; i < n; i++)
      fp[in+i].name = fp[in+i].value = (char *)NULL;

    p->field = fp;
    p->nfields += n;
  
    /* copy fields name and value */
    for (i = 0; i < n; i++)
      if ((fp[in+i].name = strdup(fld[i].name)) == NULL || (p->field[in+i].value = strdup(fld[i].value)) == NULL) {
	fprintf(stderr, "spf_header_add() -- cannot allocate memory\n");
	return(-1);
      }
  }

  return(n);
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
  int n = 0;

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
      n++;
      
      /* reallocate memory */
      if ((hp->field = (spfield_t *)realloc(hp->field, n * sizeof(spfield_t))) == NULL) {
	fprintf(stderr, "spf_header_read(): cannot allocate memory\n");
	spf_header_free(hp);
	return(NULL);
      }
      hp->nfields = n;
      if ((hp->field[n-1].name = strdup(name)) == NULL || (hp->field[n-1].value = strdup(value)) == NULL) {
	fprintf(stderr, "spf_header_read(): cannot allocate memory\n");
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

  if (hp->nfields) {

    if (fprintf(f, "<header>\n") == 0)
      return(SPRO_FEATURE_WRITE_ERR);

    for (i = 0; i < hp->nfields; i++)
      if (fprintf(f, "%s = %s\n", hp->field[i].name, hp->field[i].value) == 0)
	return(SPRO_FEATURE_WRITE_ERR);
	
    if (fprintf(f, "</header>\n") == 0)
      return(SPRO_FEATURE_WRITE_ERR);
  }

  return(0);
}

/* ------------------------------------------------------------- */
/* ----- char *spf_header_get(spfheader_t *, const char *) ----- */
/* ------------------------------------------------------------- */
/*
 * Return the header field value for specified name or NULL.
 */
char *spf_header_get(spfheader_t *hp, const char *name)
{
  unsigned short i;
  spfield_t *p = hp->field;

  for (i = 0; i < hp->nfields; i++)
    if (strcmp((p+i)->name, name) == 0)
      return((p+i)->value);

  return(NULL);
}

#undef _header_c_
