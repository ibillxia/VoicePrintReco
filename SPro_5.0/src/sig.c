/******************************************************************************/
/*                                                                            */
/*                                   sig.c                                    */
/*                                                                            */
/*                               SPro Library                                 */
/*                                                                            */
/* Guig                                                             Apr. 1997 */
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
 * Signal I/O routines.
 *
 * Signals are input via a signal stream (sigstream_t) mechanisms in
 * order to be able to deal with very large files (I'm talking about
 * several hours of 44.1 kHz stereo data). As we are mostly interested
 * in getting successive frames of signal, signal streams are accessed
 * sequentially and solely input functions are provided. Several input
 * format are supported (currently raw PCM, A-LAW, MU-LAW, WAVE and
 * SPHERE).  Small chunks of signals, such as frames, are stored as an
 * array of float samples in a spsig_t structures.
 *
 * A signal stream consists of some stream information (format, sample
 * rate, number of channels, ...) and an input signal buffer
 * (sigbuf_t). Signal input streams must be opened with the
 * sig_stream_open() function. For streams whose format does not
 * permit to retrieve the sample rate, the latter is arbitrarily set
 * to 0 and must be changed by the user in higher level
 * functions. Some stream formats let's you know the number of samples
 * some don't. In the last case, the number of samples field of the
 * stream is arbitrarily set to zero (but this field is not used
 * anyway).  Stream samples are read by the sig_stream_read() function
 * which returns the number of samples per channel read.
 *
 * Note that at the stream level, the number of samples is counted as
 * the number of samples per channel, meaning that, for stereo files,
 * a sample is two actual samples. Therefore, the sig_stream_read()
 * functions returns the number of samples per channel. On the
 * contrary, at the buffer level, the number of samples is the actual
 * number of samples in the buffer as the number of channels is not
 * known. I admit this is somehow confusing but that's the way it is.
 *
 * Many thanks to Sacha Krstulovic for implementing the support for
 * A-law and Mu-law input formats.
 */

#define _sig_c_

#include <string.h>

#include "spro.h"

#ifdef SPHERE
# include <sp/sphere.h>
#endif
#include "sptables.h"  /* -> for Alaw and ulaw conversions */

/* --------------------------------------------- */
/* ----- spsig_t *sig_alloc(unsigned long) ----- */
/* --------------------------------------------- */
/*
 * Allocate memory for a n point signal. Return allocated address or NULL 
 * in case of error.
 */
spsig_t *sig_alloc(unsigned long n)
{
  spsig_t *p;

  if ((p = (spsig_t *)malloc(sizeof(spsig_t))) == NULL)
    return(NULL);
  
  if ((p->s = (sample_t *)malloc(n * sizeof(sample_t))) == NULL) {
    free(p);
    return(NULL);
  }
  
  p->n = n;

  return(p);
}

/* ------------------------------------ */
/* ----- void sig_free(spsig_t *) ----- */
/* ------------------------------------ */
/*
 * Free allocated signal
 */
void sig_free(spsig_t *p)
{
  if (p) {
    if (p->s)
      free(p->s);
    free(p);
  }
}

/* --------------------------------------------------------------------------- */
/* ----- sigbuf_t *sig_buf_alloc(size_t, unsigned short, unsigned short) ----- */
/* --------------------------------------------------------------------------- */
/*
 * Allocate memory for a signal I/O buffer for a stream with
 * nchannels, with a maximum size of nbytes bytes.
 */
sigbuf_t *sig_buf_alloc(size_t nbytes, unsigned short nbps, unsigned short nchannels)
{
  sigbuf_t *p;

  if ((p = (sigbuf_t *)malloc(sizeof(sigbuf_t))) == NULL)
    return(NULL);
  
  if ((p->s = (short *)malloc(nbytes)) == NULL) {
    free(p);
    return(NULL);
  }

  p->m = nbytes / nbps;

  /* The following lines assumed that nchannels \in [1,2]. So let's
     make it more general. Thanks to Mathieu Ben for pointing this
     out. */
  /*
    if (p->m % 2)
    (p->m)--;
  */
  while (p->m % nchannels)
    (p->m)--;

  p->n = 0;
  
  return(p);
}

/* ----------------------------------------- */
/* ----- void sig_buf_free(sigbuf_t *) ----- */
/* ----------------------------------------- */
/*
 * Free memory allocated to a signal I/O buffer.
 */
void sig_buf_free(sigbuf_t *p)
{
  if (p) {
    if (p->s)
      free(p->s);
    free(p);
  }
}

/* ------------------------------------------------------------------------------- */
/* ----- sigstream_t *sig_stream_open(const char *, int, float, size_t, int) ----- */
/* ------------------------------------------------------------------------------- */
/* Open a signal input stream in a given format. Currently, the
 * following format are supported:
 *
 * SPRO_SIG_PCM16_FORMAT -- single channel raw 16 bits/sample PCM
 * (sample rate must be specified in this case)
 *
 * SPRO_SIG_ALAW_FORMAT -- single channel raw 8 bits/sample A-law PCM
 * (sample rate must be specified in this case)
 *
 * SPRO_SIG_ULAW_FORMAT -- single channel raw 8 bits/sample Mu-law PCM
 * (sample rate must be specified in this case)
 *
 * SPRO_SIG_WAVE_FORMAT -- WAVE COMP 360 file format (so popular in
 * the PC world) is supported
 *
 * SPRO_SIG_SPHERE_FORMAT -- NIST SPHERE file format (if compiled with
 * -DSPHERE) 
 */
sigstream_t *sig_stream_open(const char *fn, int format, float Fs, size_t nbytes, int swap)
{
  sigstream_t *p;
  char *name;                    /* just a buffer pointer for nice output   */
  int status;

  /* allocate output structure */
  if ((p = (sigstream_t *)malloc(sizeof(sigstream_t))) == NULL) {
    fprintf(stderr, "sig_stream_open(): cannot allocate memory\n");
    return(NULL);
  }

  p->name = NULL;
  p->f = NULL;
  p->buf = NULL;
  p->format = format;
  p->nsamples = 0;
  p->nread = 0;
  p->Fs = Fs;
  p->nchannels = 0;
  p->nbps = 0;
  p->swap = swap;
  
  /* set stream filename */
  if (fn && strcmp(fn, "-") != 0)
    name = (char *)fn;
  else
    name = NULL;

  /* initialize stream - open input file and read header info */
  switch (format) {

  case SPRO_SIG_PCM16_FORMAT:    
    status = sig_pcm16_stream_init(p, name);
    break;
    
  case SPRO_SIG_ALAW_FORMAT:    
    status = sig_alaw_stream_init(p, name);
    break;
    
  case SPRO_SIG_ULAW_FORMAT:    
    status = sig_ulaw_stream_init(p, name);
    break;
    
  case SPRO_SIG_WAVE_FORMAT:
    status = sig_wave_stream_init(p, name);
    break;
    
#ifdef SPHERE
  case SPRO_SIG_SPHERE_FORMAT:
    status = sig_sphere_stream_init(p, name);
    break;
    
#endif /* SPHERE */
    
  default:
    fprintf(stderr, "sig_stream_open(): unknown stream format\n");
    sig_stream_close(p);
    return(NULL);
  }
  
  if (status) {
    fprintf(stderr, "sig_stream_open(): cannot open input signal stream %s\n", (name) ? (name) : "stdin");
    sig_stream_close(p);
    return(NULL);
  }

  /* sanity check for malformed wave files */
  if (p->nchannels == 0 || p->nbps == 0 || p->Fs == 0.0) {
    fprintf(stderr, "sig_stream_open(): invalid header values in input signal stream %s (wrong format?)\n", (name) ? (name) : "stdin");
    sig_stream_close(p);
    return(NULL);
  }
  
  /* allocate buffer */
  if ((p->buf = sig_buf_alloc(nbytes, p->nbps, p->nchannels)) == NULL) {
    sig_stream_close(p);
    return(NULL);
  }

  return(p);
}

/* ------------------------------------------------ */
/* ----- void sig_stream_close(sigstream_t *) ----- */
/* ------------------------------------------------ */
/*
 * Close signal input stream.
 */
void sig_stream_close(sigstream_t *p)
{
  if (p) {

    switch(p->format) {
    case SPRO_SIG_PCM16_FORMAT:  
    case SPRO_SIG_ALAW_FORMAT:
    case SPRO_SIG_ULAW_FORMAT:
    case SPRO_SIG_WAVE_FORMAT:
      if (p->f && p->name) /* if p->name == NULL, input is from stdin and we don't wanna close stdin! */
	fclose(p->f);
      break;

#ifdef SPHERE
    case SPRO_SIG_SPHERE_FORMAT:
      sp_close(p->f);
      break;    
#endif /* SPHERE */
    }

    if (p->name)
      free(p->name);

    if (p->buf)
      sig_buf_free(p->buf);

    free(p);
  }
}

/* --------------------------------------------------------- */
/* ----- unsigned long sig_stream_read(sigstream_t *) ------ */
/* --------------------------------------------------------- */
/* 
 * Read samples from signal stream and transfer them to the
 * buffer. Return the number of samples per channel transfered into
 * the buffer.  
 */
unsigned long sig_stream_read(sigstream_t *f)
{
  unsigned long nread = 0;

  switch(f->format) {
    case SPRO_SIG_PCM16_FORMAT:  
      nread = sig_pcm16_stream_read(f);
      break;

    case SPRO_SIG_ALAW_FORMAT:  
      nread = sig_alaw_stream_read(f);
      break;

    case SPRO_SIG_ULAW_FORMAT:  
      nread = sig_ulaw_stream_read(f);
      break;

    case SPRO_SIG_WAVE_FORMAT:
      nread = sig_wave_stream_read(f);
      break;

#ifdef SPHERE
    case SPRO_SIG_SPHERE_FORMAT:
      nread = sig_sphere_stream_read(f);
      break;    
#endif /* SPHERE */
      
  default:
    f->buf->n = 0;
  }
  
  f->nread += nread;

  /* fprintf(stderr, "read %ld samples from stream (%ld samples / channel), total samples / channel = %ld\n", f->buf->n, nread, f->nread); */
  
  return(nread);
}

/* ------------------------------------------------------------------ */
/* ----- int sig_pcm16_stream_init(sigstream_t *, const char *) ----- */
/* ------------------------------------------------------------------ */
/*
 * Initialize stream for RAW PCM16 format (do *not* allocate the buffer)
 */
int sig_pcm16_stream_init(sigstream_t *f, const char *fn)
{
  if (fn) {
    if ((f->name = strdup(fn)) == NULL) {
      fprintf(stderr, "sig_pcm16_stream_init(): cannot set stream name %s\n", fn);
      return(SPRO_ALLOC_ERR);
    }
    
    if ((f->f = fopen(fn, "rb")) == NULL) {
      fprintf(stderr, "sig_pcm16_stream_init(): cannot open file %s\n", fn);
      return(SPRO_SIG_READ_ERR);
    }
  }
  else
    f->f = stdin;
  
  f->nchannels = 1;
  f->nbps = 2;

  return(0);
}

/* --------------------------------------------------------------- */
/* ----- unsigned long sig_pcm16_stream_read(sigstream_t *) ------ */
/* --------------------------------------------------------------- */
/* 
 * Read samples into the buffer from a raw 16 bits/sample signal
 * stream.  Return the number of samples per channel transfered into
 * the buffer.  
 */
unsigned long sig_pcm16_stream_read(sigstream_t *f)
{
  short *p = f->buf->s;
  unsigned long nread, i;

  nread = fread(f->buf->s, f->nbps, f->buf->m, f->f);
  
  if (f->swap)
    for (i = 0; i < nread; i++, p += f->nbps)
      sp_swap(p, f->nbps);

  f->buf->n = nread;

  return(nread /* / f->nchannels*/);
}

/* ------------------------------------------------------------------ */
/* ----- int sig_alaw_stream_init(sigstream_t *, const char *) ----- */
/* ------------------------------------------------------------------ */
/*
 * Initialize stream for RAW 8-bits ALAW format (do *not* allocate the buffer)
 */
int sig_alaw_stream_init(sigstream_t *f, const char *fn)
{
  if (fn) {
    if ((f->name = strdup(fn)) == NULL) {
      fprintf(stderr, "sig_alaw_stream_init(): cannot set stream name %s\n", fn);
      return(SPRO_ALLOC_ERR);
    }
    
    if ((f->f = fopen(fn, "rb")) == NULL) {
      fprintf(stderr, "sig_alaw_stream_init(): cannot open file %s\n", fn);
      return(SPRO_SIG_READ_ERR);
    }
  }
  else
    f->f = stdin;
  
  f->nchannels = 1;
  f->nbps = 2;

  return(0);
}

/* --------------------------------------------------------------- */
/* ----- unsigned long sig_alaw_stream_read(sigstream_t *) ------ */
/* --------------------------------------------------------------- */
/* 
 * Read and decodes samples into the buffer (of shorts) from a raw 8 bits/sample
 * signal stream.  Return the number of samples per channel transfered into
 * the buffer.  
 */
unsigned long sig_alaw_stream_read(sigstream_t *f)
{
  short *p = f->buf->s;
  unsigned long nread, i;
  unsigned char *cbuff, *cp;
  extern short sp_alaw_exp_table[256];

  /* Allocate a temporary buffer where we can read samples coded on one byte */
  if ((cbuff = malloc( f->buf->m * sizeof(char) )) == NULL) {
    fprintf(stderr, "sig_alaw_stream_read(): cannot allocate temporary buffer of %lu chars.\n", f->buf->m);
    return(0);
  }
  
  /* Read 1 byte samples from the file stream */
  nread = fread(cbuff, sizeof(char), f->buf->m, f->f);
  
  /* Decode and copy the bytes in the buffer of shorts */
  for (i = 0, cp = cbuff; i < nread; i++, p++, cp++)
    *p = sp_alaw_exp_table[(*cp)];

  /* if (f->swap): no swapping needed. */

  f->buf->n = nread;

  free(cbuff);

  return(nread /* / f->nchannels */);
}

/* ------------------------------------------------------------------ */
/* ----- int sig_ulaw_stream_init(sigstream_t *, const char *) ----- */
/* ------------------------------------------------------------------ */
/*
 * Initialize stream for RAW 8-bits ULAW format (do *not* allocate the buffer)
 */
int sig_ulaw_stream_init(sigstream_t *f, const char *fn)
{
  if (fn) {
    if ((f->name = strdup(fn)) == NULL) {
      fprintf(stderr, "sig_ulaw_stream_init(): cannot set stream name %s\n", fn);
      return(SPRO_ALLOC_ERR);
    }
    
    if ((f->f = fopen(fn, "rb")) == NULL) {
      fprintf(stderr, "sig_ulaw_stream_init(): cannot open file %s\n", fn);
      return(SPRO_SIG_READ_ERR);
    }
  }
  else
    f->f = stdin;
  
  f->nchannels = 1;
  f->nbps = 2;

  return(0);
}

/* --------------------------------------------------------------- */
/* ----- unsigned long sig_ulaw_stream_read(sigstream_t *) ------ */
/* --------------------------------------------------------------- */
/* 
 * Read and decodes samples into the buffer (of shorts) from a raw 8 bits/sample
 * signal stream.  Return the number of samples per channel transfered into
 * the buffer.  
 */
unsigned long sig_ulaw_stream_read(sigstream_t *f)
{
  short *p = f->buf->s;
  unsigned long nread, i;
  unsigned char *cbuff, *cp;
  extern short sp_ulaw_exp_table[256];

  /* Allocate a temporary buffer where we can read samples coded on one byte */
  if ((cbuff = malloc( f->buf->m * sizeof(char) )) == NULL) {
    fprintf(stderr, "sig_ulaw_stream_read(): cannot allocate temporary buffer of %lu chars.\n", f->buf->m);
    return(0);
  }
  
  /* Read 1 byte samples from the file stream */
  nread = fread(cbuff, sizeof(char), f->buf->m, f->f);
  
  /* Decode and copy the bytes in the buffer of shorts */
  for (i = 0, cp = cbuff; i < nread; i++, p++, cp++)
    *p = sp_ulaw_exp_table[(*cp)];
  
  /* if (f->swap): no swapping needed. */

  f->buf->n = nread;

  free(cbuff);

  return(nread /*/ f->nchannels */);
}

/* ----------------------------------------------------------------- */
/* ----- int sig_wave_stream_init(sigstream_t *, const char *) ----- */
/* ----------------------------------------------------------------- */
/*
 * Initialize stream for WAVE COMP 360 format. 
 */
int sig_wave_stream_init(sigstream_t *f, const char *fn)
{
  struct {
    char riff[4];                 /* String 'RIFF' without \0                */
    long totsize;                 /* Total file size - 8                     */
    char wave[4];                 /* String 'WAVE' without \0                */
    char fmtstr[4];               /* String 'fmt_' without \0                */
    long dum1;                    /* Length of format chunk (0x10)           */
    short dum2;                   /* Always 0x01                             */
    short numchans;               /* Number of channels                      */
    long Fs;                      /* Sample rate (in Hz)                     */
    long nbytespersec;            /* number of bytes/seconde                 */
    short nbytespersample;        /* number of bytes/sample                  */
    /* (1=8 bit mono, 2=8 bit stereo or 16 bit mono, 4= 16 bit stereo)   */
    short nbitspersample;         /* number of bits/samples                  */
	short cbsize;     /* length of extra header information. Added by Bill.*/
    char data[4];                 /* string 'data' without \0                */
    unsigned long datsize;        /* number of data bytes (not samples)      */
  } hdr;

  /* open input file */
  if (fn) {
    if ((f->name = strdup(fn)) == NULL) {
      fprintf(stderr, "sig_wave_stream_init(): cannot set stream name %s\n", fn);
      return(SPRO_ALLOC_ERR);
    }
    
    if ((f->f = fopen(fn, "rb")) == NULL) {
      fprintf(stderr, "sig_wave_stream_init(): cannot open file %s\n", fn);
      return(SPRO_SIG_READ_ERR);
    }
  }
  else
    f->f = stdin;
  
  /* read WAVE header */
  if (fread(&hdr, sizeof(hdr), 1, f->f) != 1) {
    fprintf(stderr, "sig_wave_stream_init(): cannot read WAVE header from %s\n", (fn) ? (fn) : "stdin");
    return(SPRO_SIG_READ_ERR);
  }
  
  if (f->swap) {
    sp_swap(&(hdr.numchans), sizeof(short));
    sp_swap(&(hdr.Fs), sizeof(long));
    sp_swap(&(hdr.nbytespersample), sizeof(short));
    sp_swap(&(hdr.datsize), sizeof(long));
  }

  if (strncmp(hdr.riff, "RIFF", 4) != 0 || strncmp(hdr.wave, "WAVE", 4)) {
    fprintf(stderr, "sig_wave_stream_init(): stream %s not in WAVE format\n", (fn) ? (fn) : "stdin");
    return(SPRO_SIG_READ_ERR);
  }

  if (hdr.numchans == 0 || hdr.Fs == 0 || hdr.nbitspersample == 0 || hdr.nbytespersample == 0) {
    fprintf(stderr, "sig_wave_stream_init(): stream %s not in WAVE format\n", (fn) ? (fn) : "stdin");
    return(SPRO_SIG_READ_ERR);
  }

  /* set the value we are looking for */
  f->nchannels = (unsigned short)hdr.numchans;
  f->nbps = hdr.nbytespersample / hdr.numchans;
  f->Fs = (float)hdr.Fs;
  f->nsamples = (unsigned long)(hdr.datsize / hdr.nbytespersample); /* number of samples / channel */
  
  /* fprintf(stderr, "nchannels=%d  nbytespersample=%d  datasize=%ld\n", hdr.numchans, hdr.nbytespersample, hdr.datsize); */

  return(0);
}

/* -------------------------------------------------------------- */
/* ----- unsigned long sig_wave_stream_read(sigstream_t *) ------ */
/* -------------------------------------------------------------- */
/* 
 * Read samples into the buffer from a wave signal stream. Return the
 * number of samples per channel transfered into the buffer.  
 */
unsigned long sig_wave_stream_read(sigstream_t *f)
{
  short *p = f->buf->s;
  unsigned long nread, n, i;

  /* in wave format, the number of samples to read must be computed
     since there are some trailer (as opposed to header) data after
     the samples, so that we can't simply wait for the end of the
     input file! (though I suspect in most cases eof corresponds to
     the end of the input signal. */
  n = (f->nsamples - f->nread) * f->nchannels;
  if (n > f->buf->m)
    n = f->buf->m;

  /* Hi there! If you're reading this because you're trying to
     understand the warning below, here's an explanation! The function
     was supposed to read a total of nasked samples and actually did
     read less. This is probably because of a weird total number of
     samples read from the WAVE header and stored in
     f->nsamples. Check that! */
  if ((nread = fread(f->buf->s, f->nbps, n, f->f)) != n)
    fprintf(stderr, "[SPro warning] end of wave stream unexpected!\n");

  if (f->swap && f->nbps > 1)
    for (i = 0; i < nread; i++, p += f->nbps)
      sp_swap(p, f->nbps);

  f->buf->n = nread;;

  return(nread / f->nchannels);
}

#ifdef SPHERE
/* ------------------------------------------------------------------- */
/* ----- int sig_sphere_stream_init(sigstream_t *, const char *) ----- */
/* ------------------------------------------------------------------- */
/*
 * Initialize stream for WAVE COMP 360 format (do *not* allocate the buffer)
 */
int sig_sphere_stream_init(sigstream_t *f, const char *fn)
{
  long n, rate;
  
  /* open file */
  if (fn) {
    if ((f->name = strdup(fn)) == NULL) {
      fprintf(stderr, "sig_wave_stream_init(): cannot set stream name %s\n", fn);
      return(SPRO_ALLOC_ERR);
    }
  }
  
  if ((f->f = sp_open((fn) ? (char *)(fn) : "-", "r")) == NULL) {
    fprintf(stderr, "sig_wave_stream_init(): cannot open SPHERE stream %s\n", (fn) ? (fn) : "stdin");
    return(SPRO_SIG_READ_ERR);
  }
  
  /* set convertion to PCM16 */
  if (sp_set_data_mode(f->f, "SE-PCM-2:DF-RAW") != 0) {
    fprintf(stderr, "sig_sphere_stream_init(): cannot set convertion mode to 16 bits/sample PCM\n");
    return(SPRO_SIG_READ_ERR);
  }

  /* read header fields */
  if (sp_h_get_field(f->f, "channel_count", T_INTEGER, (void *)&n) != 0
      || sp_h_get_field(f->f, "sample_rate", T_INTEGER, (void *)&rate) != 0) {
    fprintf(stderr, "sig_sphere_stream_init(): cannot read SPHERE header from stream %s \n", (fn) ? (fn) : "stdin");    
    return(SPRO_SIG_READ_ERR);
  }

  f->Fs = (float)rate;
  f->nchannels = (unsigned short)n;
  f->nbps = 2;

  return(0);
}

/* ---------------------------------------------------------------- */
/* ----- unsigned long sig_sphere_stream_read(sigstream_t *) ------ */
/* ---------------------------------------------------------------- */
/*
 * Read samples into the buffer from a wave signal stream. Return the
 * number of samples per channel transfered into the buffer.  
 */
unsigned long sig_sphere_stream_read(sigstream_t *f)
{
  void *p = f->buf->s;
  unsigned long nread, i;

  nread = sp_read_data(p, f->buf->m / f->nchannels, f->f);

  if (f->swap) 
    for (i = 0; i < nread; i++, p += f->nbps)
      sp_swap(p, f->nbps);

  f->buf->n = nread * f->nchannels;
  
  return(nread);
}
#endif

/* ----------------------------------------------------------------------------------- */
/* ----- int get_next_sig_frame(sigstream_t *, int, int, int, float, sample_t *) ----- */
/* ----------------------------------------------------------------------------------- */
/*
 * Return next pre-emphasized signal frame from input stream. 
 * Return 0 if no frames can be read and 1 otherwise.
 *
 * NOTE: the frame sample buffer s *must* be kept untouched between two successive calls.
 */
int get_next_sig_frame(sigstream_t *f, int ch, int l, int d, float a, sample_t *s)
{
  static double prev = 0.0;     /* pre-emphasis filter memory               */
  static unsigned long bp = 0;  /* current position in the stream buffer    */
  unsigned long nread;          /* number of samples read in buffer         */
  unsigned short i, j;
  void *p = NULL;
  double v;

  /* channel sanity check */
  if (ch < 1 || ch > f->nchannels)
    return(0);

  if (f->nread == 0) { /* first call ==> we have to read completely the first frame */
    bp = 0;
    prev = 0.0;
    j = 0;
  }
  else /* next calls ==> reuse l-d samples and add d new samples */
    for (i = d, j = 0; i < l; i++, j++)
      *(s+j) = *(s+i); /* memmove should be much faster */

  nread = 1; /* ugly trick to get into the while loop ;) */
  p = (short*)f->buf->s + (ch - 1) * f->nbps;

  while (j < l && nread) {

    if (bp == f->buf->n) { /* this means buffer is empty! */
      nread = sig_stream_read(f);
      bp = 0;
    }
    
    if (nread)
      while (j < l && bp < f->buf->n) {
	v = getsample(p, bp, f->nbps);
	*(s+j) = (sample_t)(v - a * prev);
	prev = v;
	j++;
	bp += f->nchannels;
      }
    else /* we failed to read additionnal samples */
      return(0);
  }

  return(1);
}

/* ------------------------------------------------------------------- */
/* ----- double getsample(void *, unsigned long, unsigned short) ----- */
/* ------------------------------------------------------------------- */
/*
 * Return n'th sample value assuming m bytes samples.
 */
double getsample(void *p, unsigned long n, unsigned short m)
{
  double v;

  switch(m) {
  case 1:
    v = (double)(*((char *)p+n));
    break;
  case 2:
    v = (double)*((short *)p+n);
    break;
  case 4:
    v = (double)*((long *)p+n);
  default:
    v = 0.0;
  }

  return(v);
}

/* ---------------------------------------- */
/* ----- void sp_swap(void *, size_t) ----- */
/* ---------------------------------------- */
/*
 * Swap n bytes.
 */
void sp_swap(void *x, size_t n)
{
  char c, *p = (char *)x;
  size_t j, nn = n >> 1;
  
  for (j = 0; j < nn; j++) {
    c = *(p+j);
    *(p+j) = *(p+n-j-1);
    *(p+n-j-1) = c;
  }
}

#undef _sig_c_
