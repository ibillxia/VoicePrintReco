/******************************************************************************/
/*                                                                            */
/*                                  scopy.c                                   */
/*                                                                            */
/*                               SPro Sources                                 */
/*                                                                            */
/* Guig                                                             Apr. 1997 */
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
 * $Revision: 1.17 $
 */

/*
 * Copy a data file doing the specified convertions. Also do file format
 * convertion to export datafiles to HTK or Sirocco.
 *
 * Possible convertions are, in their order of processing:
 *
 *   - delta computation, mean normalization (ZRDAN coercion)
 *   - multiplicative scaling
 *   - linear transformation
 *   - bin extraction (i.e. masking)
 *
 * The matrix for linear transformations are specified in a text file
 * according to the following format:
 *
 *   nrows ncolumns nsplice
 *   A[0][0]        A[0][1]   .........   A[0][ncolumns-1]
 *                            .........
 *   A[nrows-1][0]            .........   A[nrows-1][ncolumns-1]
 *
 * The --info option makes it possible to only see the content of
 * the header *after* the convertions.
 */

#define _scopy_c_

#include <spro.h>
#include <getopt.h>

static char *cvsid = "$Id: scopy.c,v 1.17 2003/07/25 15:47:07 ggravier Exp $";

/* ----------------------------------------------- */
/* ----- global variables set by read_args() ----- */
/* ----------------------------------------------- */
long st = 0;                      /* start at sample st (default: first)      */
long en = -1;                     /* end at sample en (default: last)         */
char *xptn = NULL;                /* extraction pattern                       */
char *ofmt = NULL;                /* output format                            */
char *transfn = NULL;             /* transformation matrix filename           */
float scalef = 0.0;               /* scale coefficient                        */
int info = 0;                     /* show file info                           */
int showdata = 1;                 /* show data                                */
int swap = 0;                     /* swap byte order                          */
size_t bufsize = 10000000;        /* I/O buffer size (in bytes)               */
unsigned long winlen = 0;         /* normalization window length              */
long flag = 0;                    /* feature additionnal streams              */
int compat = 0;                   /* import from SPro 3.x                     */
int trace = 0;                    /* trace level                              */

/* ---------------------------- */
/* ----- void usage(void) ----- */
/* ---------------------------- */
void usage(void)
{
  fprintf(stdout, "\nUsage:\n"); 
  fprintf(stdout, "    scopy [options] ifn ofn\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "Synopsis:\n");
  fprintf(stdout, "    copy input file to output file making necessary convertions.\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "Options:\n");
  fprintf(stdout, "  -c, --compatibility       input is in SPro 3.x file format (no)\n");
  fprintf(stdout, "  -I, --bufsize=n           I/O buffer size in kbytes (10000)\n");
  fprintf(stdout, "  -i, --info                print stream information (off)\n");
  fprintf(stdout, "  -z, --suppress            suppress printing data (off)\n"); 
  fprintf(stdout, "  -B, --swap                swap byte order before writing new file (no)\n");
  fprintf(stdout, "  -o, --output-format=s     set output format (spro)\n");
  fprintf(stdout, "                            valid formats are: ascii, htk, sirocco\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "  -Z, --cms                 cepstral mean normalization\n");
  fprintf(stdout, "  -R, --normalize           variance normalization\n");
  fprintf(stdout, "  -L, --segment-length=n    segment length in frames for normalization (whole data)\n");
  fprintf(stdout, "  -D, --delta               add first order derivatives\n");
  fprintf(stdout, "  -A, --acceleration        add second order derivatives\n");
  fprintf(stdout, "  -N, --no-static-energy    remove static energy\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "  -m, --scale=f             scale features (no)\n");
  fprintf(stdout, "  -t, --transform=fn        apply transformation matrix in fn (none)\n");
  fprintf(stdout, "  -x, --extract=s           bin extraction pattern (none)\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "  -s, --start=n             starting at sample number (1)\n");
  fprintf(stdout, "  -e, --end=n               ending at sample number (last)\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "  -v, --verbose             verbose mode\n");
  fprintf(stdout, "  -V, --version             print version number\n");
  fprintf(stdout, "  -h, --help                this help message\n");
  fprintf(stdout, "\n");
}

/* ----------------------------- */
/* ----- long option array ----- */
/* ----------------------------- */
static struct option longopts[] =
{
  {"compatibility", no_argument, NULL, 'c'},
  {"segment-length", required_argument, NULL, 'L'},
  {"cms", no_argument, NULL, 'Z'},
  {"normalize", no_argument, NULL, 'R'},
  {"delta", no_argument, NULL, 'D'},
  {"acceleration", no_argument, NULL, 'A'},
  {"no-static-energy", no_argument, NULL, 'N'},
  {"scale", required_argument, NULL, 'm'},
  {"transform", required_argument, NULL, 't'},
  {"extract", required_argument, NULL, 'x'},
  {"info", no_argument, NULL, 'i'},
  {"supress", no_argument, NULL, 'z'},
  {"start", required_argument, NULL, 's'},
  {"end", required_argument, NULL, 'e'},
  {"swap", no_argument, NULL, 'B'},
  {"output-format", required_argument, NULL, 'o'},
  {"bufsize", no_argument, NULL, 'I'},
  {"verbose", no_argument, NULL, 'v'},
  {"version", no_argument, NULL, 'V'},
  {"help", no_argument, NULL, 'h'},
  {0, 0, 0, 0}
};

/* ---------------------------------- */
/* ----- local type definitions ----- */
/* ---------------------------------- */
typedef struct {
  unsigned short n, m, splice;
  double **a;
} transmat_t;

/* ---------------------------------------------------------- */
/* ----- void swap_bytes(void *, unsigned long, size_t) ----- */
/* ---------------------------------------------------------- */
/*
 * Swap bytes
 */
void swap_bytes(void *p, unsigned long n, size_t m)
{
  char c, *cp = (char *)p;
  unsigned long i;
  size_t j;
  
  for (i = 0; i < n; i++) {

    for (j = 0; j < m / 2; j++) {
      c = cp[j];
      cp[j] = cp[m-j-1];
      cp[m-j-1] = c;
    }

    cp += m;
  }
}

/* --------------------------------- */
/* ----- int main(int,char **) ----- */
/* --------------------------------- */
int main(int argc,char **argv)
{
  char *ifn, *ofn;                /* I/O filenames                            */
  FILE *f;                        /* I/O file descriptor                      */
  spfstream_t *is = NULL;         /* I/O feature streams                      */
  spfbuf_t *ibuf;                 /* input stream buffer                      */
  spfbuf_t *obuf;                 /* output feature buffer                    */
  spf_t *pmem;
  transmat_t *A = NULL;           /* transformation matrix                    */
  long t1 = -1, t2 = -1, from, to;
  unsigned short odim;
  unsigned long nsamples = 0;

  int read_args(int, char **);

  transmat_t *read_trans_mat(const char *);
  void free_trans_mat(transmat_t *);

  spfbuf_t *import_data(char *);
  void show_input_info(spfstream_t *);
  unsigned long get_n_samples(spfstream_t *);
  unsigned short get_output_dim(unsigned short, transmat_t *);
  FILE *init_output(const char *, unsigned long, unsigned short, long, float);
  unsigned long output(spfbuf_t *, FILE *);

  void scale(spfbuf_t *, float);
  spfbuf_t *transform(spfbuf_t *, transmat_t *);
  int extract(spfbuf_t *, char *);
  
  /* ----- process command line ----- */
  if (read_args(argc, argv))
    return(1);

  if (optind < argc)
    ifn = argv[optind++];
  else {
    fprintf(stderr, "scopy error -- no input filename specified (use --help to get usage)\n");
    return(1);
  }

  if (optind < argc) {
    ofn = argv[optind++];
  }
  else {
    fprintf(stderr, "scopy error -- no output filename specified (use --help to get usage)\n");
    return(1);
  }

  if (optind < argc) {
    fprintf(stderr, "scopy error -- invalid number of arguments (use --help to get usage)\n");
    return(1);
  }

  /* ----- show what was asked to do ----- */
  if (trace) {
    fprintf(stdout, "%s --> %s", ifn, ofn);
    if (ofmt)
      fprintf(stdout, " [%s]", ofmt);
    fprintf(stdout, "\n");
    fflush(stdout);
  }

  /* ----- load transform matrix if any ----- */
  if (transfn)
    if ((A = read_trans_mat(transfn)) == NULL) {
      fprintf(stderr, "scopy error -- cannot open transform file %s\n", transfn);
      return(1);
    }
  
  /* ----- open input stream or read input buffer ----- */
  if (compat) {

    if ((ibuf = import_data(ifn)) == NULL) {
      fprintf(stderr, "scopy error -- cannot open import SPro 3 data from stream %s\n", ifn);
      free_trans_mat(A);
      return(SPRO_STREAM_OPEN_ERR);
    }
    odim = get_output_dim(ibuf->dim, A);
    nsamples = ibuf->n;
  }
  else {

    if ((is = spf_input_stream_open(ifn, flag, bufsize)) == NULL) {
      fprintf(stderr, "scopy error -- cannot open input stream %s\n", ifn);
      free_trans_mat(A);
      return(SPRO_STREAM_OPEN_ERR);
    }

    if (info)
      show_input_info(is);

    if (winlen)
      set_stream_seg_length(is, winlen);
    
    ibuf = is->buf;
    odim = get_output_dim(is->odim, A);

    if (ofmt && (strcasecmp(ofmt, "htk") == 0 || strcasecmp(ofmt, "sirocco") == 0)) {
      if ((nsamples = get_n_samples(is)) == 0) {
	fprintf(stderr, "scopy error -- cannot determine number of frames (probably not using a seekable stream)\n");
	fprintf(stderr, "               export to HTK and Sirocco can only be done on seekable streams.\n");
	free_trans_mat(A); spf_stream_close(is);
      }
    }
    else
      nsamples = 0;
  }

  /* ----- print out data ----- */
  if (showdata) {
    
    if ((f = init_output(ofn, nsamples, odim, (compat) ? flag : is->oflag, (compat) ? 100.0 : is->Fs)) == NULL) {
      fprintf(stderr, "scopy error -- cannot initialize output stream %s\n", ofn);
      if (compat) spf_buf_free(ibuf); else spf_stream_close(is); free_trans_mat(A);
      return(SPRO_STREAM_OPEN_ERR);
    }
      
    /* while input, run convertions and write */
    while (1) {
      
      if (! compat)
	if (spf_stream_read(is) == 0)
	  break;
      
      /* 
	 check time boundaries with specified boundaries: 
	 we want frames from st to en (en = -1 means en = \infty)
	 current buffer holds frames t1 to t2
	 skip current buffer if t2 < st or t1 > en
	 else start at frame max(st, t1) and end at frame min(en, t2).
      */
      t1 = t2 + 1;
      t2 = t1 + ibuf->n - 1;
      
      
      if (t2 < st || (en >= 0 && t1 > en)) {
	if (compat)
	  break;
	else
	  continue;
      }

      from = (st > t1) ? st : t1;
      if (en >= 0)
	to = (en > t2) ? t2 : en;
      else
	to = t2;
      
      if (scalef != 0.0)                            /* scaling   */
	scale(ibuf, scalef);
      
      if (A) {                                     /* transform */
	if ((obuf = transform(ibuf, A)) == NULL) {
	  fprintf(stderr, "scopy error -- cannot allocate memory\n");
	  spf_stream_close(is); free_trans_mat(A); if (f != stdout) fclose(f);
	  return(SPRO_ALLOC_ERR);
	}
      }
      else
	obuf = ibuf;
      
      if (xptn)                                     /* extract   */
	if (extract(obuf, xptn) != 0) {
	  fprintf(stderr, "scopy error -- cannot allocate memory\n");
	  spf_stream_close(is); free_trans_mat(A); if (f != stdout) fclose(f);
	  if (A) spf_buf_free(obuf);
	  return(SPRO_ALLOC_ERR);
	}

      /* write to output stream */
      pmem = obuf->s;
      obuf->s = pmem + (from - t1) * obuf->adim;
      obuf->n = to - from + 1;
      
      if (output(obuf, f) != obuf->n) {
	fprintf(stderr, "scopy error -- cannot write buffer to output stream %s\n", ofn);
	spf_stream_close(is); free_trans_mat(A); if (f != stdout) fclose(f); spf_buf_free(obuf);
	return(SPRO_FEATURE_WRITE_ERR);
      }
      
      obuf->s = pmem;
      if (A)
	spf_buf_free(obuf);

      if (compat)
	break;
    }
    
    if (f != stdout)
      fclose(f);
  }

  /* that's it man! */
  if (! compat)
    spf_stream_close(is);
  else
    spf_buf_free(ibuf);

  free_trans_mat(A);

  return(0);
}

/* ----------------------------------------------- */
/* ----- void show_input_info(spfstream_t *) ----- */
/* ----------------------------------------------- */
void show_input_info(spfstream_t *p) 
{
  char s[7];
  unsigned short dim, i, idx[9];

  for (i = 0; i < p->header->nfields; i++)
    fprintf(stdout, "%s = %s\n", p->header->field[i].name, p->header->field[i].value);
  
  fprintf(stdout, "sample_rate = %f\n", p->Fs);
  sp_flag_to_str(p->iflag, s);
  fprintf(stdout, "input:     dim=%-3hu (%s)\n", p->idim, (*s) ? s : "<nil>");

  if (p->iflag != p->oflag)
    fprintf(stdout, "convert:   dim=%-3hu (%s)\n", p->odim, sp_flag_to_str(p->oflag, s));
}

/* ------------------------------------------------------ */
/* ----- unsigned long get_n_samples(spfstream_t *) ----- */
/* ------------------------------------------------------ */
/*
 * This is a rather dirty function to get the number of frames of a
 * (seekable) input feature stream.
 */
unsigned long get_n_samples(spfstream_t *f)
{
  size_t spos, epos;

  spos = ftell(f->f);

  if (fseek(f->f, 0, SEEK_END) != 0)
    return(0);
  
  epos = ftell(f->f);

  if (fseek(f->f, spos, SEEK_SET) != 0)
    return(0);

  return((epos-spos) / (spf_stream_dim(f) * sizeof(spf_t)));
}

/* --------------------------------------------------------------------------------------- */
/* ----- FILE *init_output(const char *, unsigned long, unsigned short, long, float) ----- */
/* --------------------------------------------------------------------------------------- */
FILE *init_output(const char *ofn, unsigned long nsamples, unsigned short dim, long oflag, float frate)
{
  FILE *f;
  
  if (strcmp(ofn, "-")) {
    if ((f = fopen(ofn, "w")) == NULL)
      return(NULL);
  }
  else
    f = stdout;

  if (! ofmt) {
    unsigned short _dim = dim;
    long _flag = oflag;
    float _rate = frate;
    
    if (transfn || xptn)
      _flag = 0;

#ifdef WORDS_BIGENDIAN
    swap_bytes(&_dim, 1, SIZEOF_SHORT);
    swap_bytes(&_flag, 1, SIZEOF_LONG);
    swap_bytes(&_rate, 1, sizeof(float));
#endif
  
    if (fwrite(&_dim, SIZEOF_SHORT, 1, f) != 1 || 
	fwrite(&_flag, SIZEOF_LONG, 1, f) != 1 || 
	fwrite(&_rate, sizeof(float), 1, f) != 1) {
      if (f != stdout) fclose(f);
      fprintf(stderr, "scopy error -- cannot write header to file %s\n", ofn); 
      return(NULL);
    }
  }
  else if (strcasecmp(ofmt, "htk") == 0) {
    struct {
      long nsamples, period;
      short size, kind;
    } htkheader;

    if (en >= 0)
      nsamples = en - st + 1;
    else
      nsamples -= st;

    fprintf(stderr, "nsamples=%lu\n", nsamples);

    htkheader.nsamples = (long)nsamples;
    htkheader.size = (short)(dim * sizeof(float));
    htkheader.period = 1e7 / (long)frate;
    htkheader.kind = 9;

    if (oflag & WITHE) htkheader.kind |= 0x0040;
    if (oflag & WITHD) htkheader.kind |= 0x0100;
    if (oflag & WITHA) htkheader.kind |= 0x0200;
    if (oflag & WITHN) htkheader.kind |= 0x0080;
    if (oflag & WITHZ) htkheader.kind |= 0x0800;
    
    if (swap) {
      swap_bytes(&(htkheader.period), 1, sizeof(long));
      swap_bytes(&(htkheader.size), 1, sizeof(short));
      swap_bytes(&(htkheader.kind), 1, sizeof(short));    
    }

    if (fwrite(&htkheader, sizeof(htkheader), 1, f) != 1) {
      if (f != stdout) fclose(f);
      return(NULL);
    }
  }
  
  else if (strcasecmp(ofmt, "sirocco") == 0) {
    short int nframes;
    unsigned char c;

    if (en >= 0)
      nsamples = en - st + 1;
    else
      nsamples -= st;

    nframes = (short int)nsamples;
    c = (unsigned char)dim;
    
    if (swap)
      swap_bytes(&nframes, 1, sizeof(short int));

    if (fwrite(&nframes, sizeof(short int), 1, f) != 1 || fwrite(&c, sizeof(unsigned char), 1, f) != 1) {
      if (f != stdout) fclose(f);
      return(NULL);
    }
  }

  else if (strcasecmp(ofmt, "ascii") != 0) {
    fprintf(stderr, "scopy error -- unkown output format %s\n", ofmt);
    if (f != stdout) fclose(f);
    return(NULL);
  }

  return(f);
}

/* ----------------------------------------------------------------------------- */
/* ----- unsigned short get_output_dim(unsigned short, long, transmat_t *) ----- */
/* ----------------------------------------------------------------------------- */
unsigned short get_output_dim(unsigned short d, transmat_t *A)
{
  char *p, *pp;
  unsigned short dim = d;
  unsigned long a, b;

  if (A) {
    dim = A->n;
    if (info)
      fprintf(stdout, "transform: dim=%-3hu (%s)\n", dim, transfn);
  }

  if (xptn) { /* extraction pattern governs the output dimension */
    p = strtok(xptn, ",");
    dim = 0;

    while (p) {
      a = b = 0;
      a = strtoul(p, &pp, 10);
      if (*pp == '-')
	b = strtoul(pp + 1, &pp, 10);
      else
	b = a;

      if (b - a >= 0)
	dim += b - a + 1;

      if (*xptn != ',')
	*(p-1) = ',';

      p = strtok(NULL, ",");
    }
      
    if (info)
      fprintf(stdout, "extract:   dim=%-3hu (%s)\n", dim, xptn);
  }
  /* let's hope xptn is back to normal */
  
  if (info)
    fflush(stdout);

  return(dim);
}

/* ----------------------------------------- */
/* ----- spfbuf_t *import_data(char *) ----- */
/* ----------------------------------------- */
spfbuf_t *import_data(char *fn)
{
  char s[7];
  enum {OTHER, FBANK, FBCEPSTRA, LPCEPSTRA, LPCOEFF, PARCOR, LAR} dum;
  unsigned long dim, n;
  unsigned int lflag;
  long iflag = 0;
  spfbuf_t *buf;
  FILE *f;

  if ((f = fopen(fn, "rb")) == 0) {
    fprintf(stderr, "scopy error -- cannot open file %s\n", fn);
    return(NULL);
  }
  
  if (fread(&dum, sizeof(dum), 1, f) != 1 || fread(&dim, sizeof(unsigned long), 1, f) != 1 ||
      fread(&n, sizeof(unsigned long), 1, f) != 1 || fread(&lflag, sizeof(unsigned int), 1, f) != 1) {
    fprintf(stderr, "scopy error -- cannot read SPro 3.x data header from file %s\n", fn);
    fclose(f);
    return(NULL);
  }

  if (lflag & 0x01) {
    iflag |= WITHE;
    dim++;
  }
  if (lflag & 0x08) {
    iflag |= WITHD;
    if (lflag & 0xA0) {
      iflag |= WITHA; 
      dim *= 3;
    }
    else
      dim += dim;
  }
  if (lflag & 0x04) {
    iflag |= WITHN;
    dim--;
  }

  if (info) {
    fprintf(stdout, "sample_rate = 100\n");
    sp_flag_to_str(iflag, s);
    fprintf(stdout, "input:    dim=%-3lu (%s)\n", dim, (*s) ? s : "<nil>");
  }

  flag |= iflag;

  if ((buf = spf_buf_alloc(dim, dim * n * sizeof(float))) == NULL) {
    fprintf(stderr, "scopy error -- cannot allocate buffer\n");
    fclose(f);
    return(NULL);
  }

  if (fread(buf->s, dim * sizeof(float), n, f) != n) {
    fprintf(stderr, "scopy error -- cannot read SPro 3.x data from file %s\n", fn);
    fclose(f); spf_buf_free(buf);
    return(NULL);
  }
  buf->n = n;

  fclose(f);

  /* now convert buffer to target stream description */
  if ((buf = spf_buf_convert(buf, iflag, flag, winlen, SPRO_CONV_REPLACE)) == NULL) {
    fprintf(stderr, "scopy error -- cannot read SPro 3.x data from file %s\n", fn);
    return(NULL);
  }

  if (info) {
    sp_flag_to_str(flag, s);
    fprintf(stdout, "input:     dim=%-3hu (%s)\n", buf->dim, (*s) ? s : "<nil>");
    fflush(stdout);
  }

  return(buf);
}

/* ---------------------------------------------------- */
/* ----- unsigned long output(spfbuf_t *, FILE *) ----- */
/* ---------------------------------------------------- */
unsigned long output(spfbuf_t *buf, FILE *f)
{
  float *tmp = NULL;
  spf_t *p;
  unsigned long i, nwritten = 0;
  unsigned short j;
  int convert = (sizeof(spf_t) != sizeof(float));

  if (! ofmt)
    nwritten = spf_buf_write(buf, f);

  else if (strcasecmp(ofmt, "htk") == 0 || strcasecmp(ofmt, "sirocco") == 0) {

    if (convert) {
      if ((tmp = (float *)malloc(buf->dim * sizeof(float))) == NULL) {
	fprintf(stderr, "scopy error -- cannot allocate memory\n");
	return(0);
      }
    }
    p = buf->s;

    for (i = 0; i < buf->n; i++) {

      if (convert)
	for (j = 0; j < buf->dim; j++)
	  *(tmp+j) = *(p+j);
      else
	tmp = p;

      if (swap)
	swap_bytes(tmp, buf->dim, sizeof(float));

      if (fwrite(tmp, sizeof(float), buf->dim, f) != buf->dim) {
	if (convert) free(tmp);
	return(nwritten);
      }
      nwritten++;
      p += buf->adim;
    }

    if (convert)
      free(tmp);
  }

  else {
    p = buf->s;
    for (i = 0; i < buf->n; i++) {
      for (j = 0; j < buf->dim; j++)
	fprintf(f, (j) ? " %f" : "%f", *(p+j));
      fprintf(f,"\n");
      p += buf->adim;
    }
    nwritten = buf->n;
  }

  return(nwritten);
}

/* ----------------------------------------- */
/* ----- void scale(spfbuf_t *, float) ----- */
/* ----------------------------------------- */
void scale(spfbuf_t *buf, float s)
{
  unsigned long i;
  unsigned short j;
  spf_t *p = buf->s;

  for (i = 0; i < buf->n; i++) {
    for (j = 0; j < buf->dim; j++)
      *(p+j) *= s;
    p += buf->adim;
  }
}

/* --------------------------------------------------------- */
/* ----- spfbuf_t *transform(spfbuf_t *, transmat_t *) ----- */
/* --------------------------------------------------------- */
spfbuf_t *transform(spfbuf_t *ibuf, transmat_t *A)
{
  int k, tt;
  unsigned long t;
  double v, *ap;
  spf_t *op, *ip;
  spfbuf_t *obuf;
  unsigned short i, j, sd;
  double *ybuf;

  sd = (2 * A->splice + 1) * ibuf->dim;
  if (A->m  != sd) {
    fprintf(stderr, "scopy error -- incompatible input data (%d) and transformation (%d) dimensions (splice=%d)\n", 
	    ibuf->dim, A->m, A->splice);
    return(NULL);
  }

  /* allocate temporary buffer */
  if ((ybuf = (double *)malloc(sd * sizeof(double))) == NULL) {
    fprintf(stderr, "scopy error -- cannot allocate memory for transformed features\n");
    return(NULL);
  }

  /* create new feature buffer */
  if ((obuf = spf_buf_alloc(A->n, ibuf->n * A->n *sizeof(spf_t))) == NULL) {
    fprintf(stderr, "scopy error -- cannot allocate memory for transformed features\n");
    free(ybuf);
    return(NULL);
  }

  /* run the transformation */
  op = obuf->s;

  for (t = 0; t < ibuf->n; t++) {

    /* put data in buffer */
    i = 0;
    for (k = -A->splice; k <= A->splice; k++) {
      tt = t + k;
      if ((tt = t + k) < 0)
	tt = 0;
      else if (tt >= ibuf->n)
	tt = ibuf->n;

      ip = ibuf->s + tt * ibuf->adim;

      for (j = 0; j < ibuf->dim; j++, i++)
	*(ybuf+i) = (double)*(ip+j);
    }

    /* apply transformation to the buffer */
    for (i = 0; i < A->n; i++) {
      v = 0.0;
      ap = A->a[i];
      for (j = 0; j < sd; j++) {
	v += (*(ap+j) * *(ybuf+j));
      }
      *(op+i) = (spf_t)v;
    }

    (obuf->n)++;
    op += obuf->adim;
  }

  free(ybuf);

  return(obuf);
}

/* ------------------------------------------- */
/* ----- int extract(spfbuf_t *, char *) ----- */
/* ------------------------------------------- */
int extract(spfbuf_t *buf, char *ptn)
{
  unsigned long t, i, j;
  unsigned long a, b;
  int *bins, nbins;
  char *p, *pp;
  spf_t *ip;

  /* create and fill in bins array */
  if ((bins = (int *)malloc(buf->dim * sizeof(int))) == NULL) {
    fprintf(stderr, "scopy error -- cannot allocate memory\n");
    return(SPRO_ALLOC_ERR);
  }
  for (i = 0; i < buf->dim; i++)
    *(bins+i) = 0;

  p = strtok(ptn, ",");
  nbins = 0;

  while (p) {
    a = b = 0;
    a = strtoul(p, &pp, 10);
    if (*pp == '-')
      b = strtoul(pp+1, &pp, 10);
    else
      b = a;
    
    if (a < 1 || a > buf->dim || b < 1 || b > buf->dim) {
      fprintf(stderr, "scopy error -- invalid extraction specification [%ld,%ld] in pattern %s\n", a, b, ptn);
      free(bins);
      return(SPRO_BAD_PARAM_ERR);
    }

    for (i = a; i <=b; i++) {
      bins[i-1] = 1;
      nbins++;
    }
      
    p = strtok(NULL, ",");
  }

  /*   fprintf(stderr, "nbins=%d   (%d", nbins, bins[0]); */
  /*   for (i = 1; i < buf->dim; i++) */
  /*     fprintf(stderr, " %d", bins[i]); */
  /*   fprintf(stderr, ")\n"); */

  if (nbins == 0) {
    fprintf(stderr, "scopy error -- empty extraction pattern %s\n", ptn);
    free(bins);
    return(SPRO_BAD_PARAM_ERR);
  }

  /* run the extraction */  
  ip = buf->s;
  for (t = 0; t < buf->n; t++) {
    j = 0;

    for (i = 0; i < buf->dim; i++)
      if (*(bins+i)) {
	*(ip+j) = *(ip+i);
	j++;
      }

    ip += buf->adim;
  }

  buf->dim = nbins;

  return(0);
}

/* ---------------------------------------------------- */
/* ----- transmat_t *read_trans_mat(const char *) ----- */
/* ---------------------------------------------------- */
transmat_t *read_trans_mat(const char *fn)
{
  double *p;
  unsigned short i, j, n, m;
  transmat_t *A;
  FILE *f;
  void free_trans_mat(transmat_t *);

  if ((f = fopen(fn, "r")) == NULL) {
    fprintf(stderr, "scopy error -- cannot open transformation file %s\n", fn);
    return(NULL);
  }
  
  if (fscanf(f, "%hu %hu %hu", &n, &m, &i) != 3) {
    fprintf(stderr, "scopy error -- cannot read matrix dimensions from file %s\n", fn);
    return(NULL);
  }

  /* allocate memory */
  if ((A = (transmat_t *)malloc(sizeof(transmat_t))) == NULL) {
    fclose(f);
    fprintf(stderr, "scopy error -- cannot allocate memory for %d x %d transformation\n", n, m);
    return(NULL);
  }
  A->n = n;
  A->m = m;  
  A->splice = i;

  if ((A->a = (double **)malloc(n * sizeof(double *))) == NULL) {
    fclose(f); free_trans_mat(A);
    fprintf(stderr, "scopy error -- cannot allocate memory for %d x %d transformation\n", n, m);
    return(NULL);
  }
  for (i = 0; i < n; i++)
    A->a[i] = NULL;
  
  /* read the matrix */
  for (i = 0; i < n; i++) {
    if ((p = (double *)malloc(m * sizeof(double))) == NULL) {
      fclose(f); free_trans_mat(A);
      fprintf(stderr, "scopy error -- cannot allocate memory for %d x %d transformation\n", n, m);
      return(NULL);
    }
    for (j = 0; j < m; j++) {
      if (fscanf(f, "%le", p+j) != 1) {
	fclose(f); free_trans_mat(A);
	fprintf(stderr, "scopy error -- cannot read matrix dimensions from file %s\n", fn);
	return(NULL);
      }
    }
    A->a[i] = p;
  }
  fclose(f);
  
  return(A);
}

/* --------------------------------------------- */
/* ----- void free_trans_mat(transmat_t *) ----- */
/* --------------------------------------------- */
void free_trans_mat(transmat_t *A)
{
  unsigned short i;

  if (A) {
    if (A->a) {
      for (i = 0; i < A->n; i++)
	if (A->a[i])
	  free(A->a[i]);
      free(A->a);
    }
    free(A);
  }
}

/* --------------------------------------- */
/* ----- int read_args(int, char **) ----- */
/* --------------------------------------- */
/*
 *  -c, --compatibility       input is in SPro 3.x file format (no)       
 *  -s, --scale-energy=f      scale and normalize log-energy (off)
 *  -W, --segment-length=n    segment length in frames for normalization (whole data)
 *  -Z, --cms                 mean normalization
 *  -R, --var-norm            variance normalization
 *  -D, --delta               add first order derivatives
 *  -A, --acceleration        add second order derivatives
 *  -N, --no-static-energy    remove static energy
 *  -m, --scale=f             scaling factor (1.0)
 *  -t, --transform=fn        linear transform matrix (none)
 *  -x, --extract=s           bin extraction pattern (none) 
 *  -s, --start=n             starting at sample number (1)
 *  -e, --end=n               ending at sample number (last)
 *  -o, --output-format=s     set output format (spro)
 *  -i, --info                print input file header information (off)
 *  -v, --verbose             verbose mode
 *  -V, --version             print version number
 *  -h, --help                this help message
 */
int read_args(int argc, char **argv)
{
  int c, i;
  char *p;

  opterr = 0;
  optopt = '!';

  while ((c = getopt_long(argc, argv, "cZRL:DANm:t:x:s:e:BI:o:izvVh", longopts, &i)) != EOF)
    switch (c) {
      
    case 'c':
      compat = 1;
      break;

    case 'L':
      winlen = (unsigned long)strtol(optarg, &p, 10);
      if (p == optarg || *p != 0) {
	fprintf(stderr, "scopy error -- invalid argument %s to --cms (use --help to get usage)\n", optarg);
	return(1);
      }
      break;

    case 'Z':
      flag |= WITHZ;
      break;

    case 'R':
      flag |= WITHR;
      break;

    case 'D':
      flag |= WITHD;
      break;

    case 'A':
      flag |= WITHA;
      break;

    case 'N':
      flag |= WITHN;
      break;

    case 'm':
      scalef = (float)strtod(optarg, &p);
      if (p == optarg || *p != 0 || scalef == 0.0) {
	fprintf(stderr, "scopy error -- invalid argument %s to --scale (use --help to get usage)\n", optarg);
	return(1);
      }
      break;

    case 't':
      transfn = optarg;
      break;

    case 'x':
      xptn = optarg;
      break;

    case 'e': 
      en = strtol(optarg, &p, 10);
      if (p == optarg || *p != 0) {
	fprintf(stderr, "scopy error -- invalid argument %s to --end (use --help to get usage)\n", optarg);
	return(1);
      }
      break;

    case 's':
      st = strtol(optarg, &p, 10);
      if (p == optarg || *p != 0) {
	fprintf(stderr, "scopy error -- invalid argument %s to --start (use --help to get usage)\n", optarg);
	return(1);
      }
      break;

    case 'B':
      swap = 1;
      break;

    case 'I':
      bufsize = (size_t)strtol(optarg, &p, 10) * 1000;
      if (p == optarg || *p != 0) {
	fprintf(stderr, "scopy error -- invalid argument %s to --bufsize (use --help to get usage)\n", optarg);
	return(1);
      }
      break;

    case 'o':
      ofmt = optarg;
      break;

    case 'i':
      info = 1;
      break;

    case 'z':
      showdata = 0;
      break;

    case 'v':
      trace = 1;
      break;

    case 'V':
      fprintf(stdout, "\nSPro %s -- %s\n\n", VERSION, cvsid);
      exit(0);
      break;

    case 'h':
      usage();
      exit(0);
      break;

    default:
      fprintf(stderr, "scopy error -- unrecognized option %s (use --help to get usage)\n", argv[optind-1]);
      return(1);
    }

  /* ----- basic argument sanity check ----- */
  if ((flag & WITHA) && !(flag & WITHD)) {
    fprintf(stderr, "scopy error -- cannot have accelerations without delta\n");
    return(1);
  }

  if ((flag & WITHR) && !(flag & WITHZ)) {
    fprintf(stderr, "scopy error -- cannot have variance normalization without mean removal\n");
    return(1);
  }

  if (st < 0 || (en >= 0 && st > en)) {
    fprintf(stderr, "scopy error -- invalid start and/or end sample (st=%ld, en=%ld)", st, en);
    return(1);
  }

  return(0);
}
      
#undef _scopy_c_
