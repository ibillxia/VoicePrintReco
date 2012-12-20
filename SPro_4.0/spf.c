/******************************************************************************/
/*                                                                            */
/*                                   spf.c                                    */
/*                                                                            */
/*                               SPro Library                                 */
/*                                                                            */
/* Guig                                                             Sep. 2002 */
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
 * $Date: 2003/08/22 12:44:10 $
 * $Revision: 1.4 $
 *
 */

/*
 * Feature related routines.
 *
 * SPro features are stored in buffers (spfbuf_t) where a buffer is a
 * collection of feature vectors of given dimension. Buffers are
 * allocated for a maximum dimension (adim) and a maximum number of
 * vectors (m) to provide control of the amount of memory dedicated to
 * feature I/Os. Buffer memory is allocated with spf_buf_alloc() with
 * a given size (in bytes) and released with spf_buf_free(). The
 * amount of memory allocated to a buffer can be extended using
 * spf_buf_resize(). Appending a feature vector to a buffer is done
 * using spf_buf_append(). If the last argument of the function is not
 * null, the buffer is extended if necessary. Otherwise,
 * spf_buf_append() returns an error when the buffer is full.
 * Finally, buffer level I/Os are provided with spf_buf_read() and
 * spf_buf_write().
 *
 * Features are read from and written to SPro feature streams
 * (spfstream_t). Every feature stream has a Unix stream associated,
 * along with the mandatory header information, an I/O feature buffer
 * and, eventually, information on the convertion operation to be
 * performed on the features after reading them or before writing
 * them, depending on the stream type. 

 * The stream header has an optionnal variable length part (see
 * header.c for details) and some mandatory information, namely
 * feature dimension, frame rate and feature description flag. The
 * convertion flag (cflag) can be used to specify *additionnal*
 * descriptors with respect to the original feature descriptors
 * (iflag, idim), which means you cannot remove elements from a
 * feature vector using this method (e.g. convertion from EDA to ED is
 * *not possible* simply by setting the convertion flag to ED). After
 * convertion, the feature flag is oflag and the dimension odim. In
 * read mode, input is what you read from file and output what you get
 * with get_next_spf_frame(). In write mode, input is the argument to
 * spf_stream_write() and output is what is on file. By definition,
 * the stream dimension is the output dimension.
 *
 * To enable seeks in feature streams, we keep track of the absolute
 * frame indices via the start and idx variables. The first one
 * indicates the absolute frame number of the first frame in the
 * buffer while the second one indicates the relative index of the
 * feature in the buffer. The current absolute position in the stream
 * is therefore given by t = start + idx. Frame indices, whether
 * relative or absolute starts at 0.
 *
 * The function spf_input_stream_open() opens a feature stream for
 * reading while spf_output_stream_open() is available for output
 * streams. In addition to the convertion flag specified when opening
 * the stream, one can set additional convertion using
 * set_stream_energy_scale() and set_stream_seg_length(). The first
 * one specifies the energy scale factor while the second one
 * specifies the segment length (in number of frames) on which global
 * transforms such as energy normalization or mean normalization are
 * performed. Input and output streams are closed using
 * spf_stream_close(). The function spf_stream_flush() can be used to
 * flush the buffer of an output stream to the associated Unix
 * stream. The function spf_stream_write() may be used to write
 * feature vectors to a stream. On the contrary, spf_stream_read()
 * reads in new data into the SPro feature stream buffer (which is
 * somehow confusing since it is not the dual operation of
 * spf_stream_write()). The function spf_stream_seek() can be used to
 * seek to a particular frame index in a stream. If the stream is an
 * input stream, the next call to get_next_spf_frame() will return the
 * target frame (you can alternately access the frame directly using
 * s->idx but I do *not* recomend it since successive calls to
 * get_next_spf_frame() will be screwed up). If the stream is an
 * output stream, the next call to spf_stream_write() will write the
 * frame at the specified index. The current position in the stream is
 * obtained using spf_stream_tell() and spf_stream_rewind() rewinds
 * back to t=0.  
 */

#define _spf_c_

#include <spro.h>

     /* -------------------------------------------- */
     /* ----- feature buffer related functions ----- */ 
     /* -------------------------------------------- */

/* ------------------------------------------------------------ */
/* ----- spfbuf_t *spf_buf_alloc(unsigned short, size_t) ------ */
/* ------------------------------------------------------------ */
/*
 * Allocate feature buffer with a maximum of nbytes. Return allocated
 * address or NULL in case of error.
 */
spfbuf_t *spf_buf_alloc(unsigned short dim, size_t nbytes)
{
  spfbuf_t *p;

  if ((p = (spfbuf_t *)malloc(sizeof(spfbuf_t))) != NULL) {

    p->n = p->m = 0;
    p->dim = p->adim = 0;
    p->s = (spf_t *)NULL;

    if (nbytes) {
      if ((p->s = (spf_t *)malloc(nbytes)) == NULL) {
	free(p);
	return(NULL);
      }
      else {
	p->adim = p->dim = dim;
	p->m = nbytes / (dim * sizeof(spf_t));
      }
    }
  }

  return(p);
}

/* ----------------------------------------- */
/* ----- void spf_buf_free(spfbuf_t *) ----- */
/* ----------------------------------------- */
/*
 * Free memory allocated to a feature buffer.
 */
void spf_buf_free(spfbuf_t *p)
{
  if (p) {
    if (p->s)
      free(p->s);
    free(p);
  }
}

/* ---------------------------------------------------------------- */
/* ----- feature_t *spf_buf_resize(spfbuf_t *, unsigned long) ----- */
/* ---------------------------------------------------------------- */
/*
 * Resize feature buffer and return a pointer to the newly allocated 
 * buffer or NULL in case of error. The new size is given as the new
 * number of samples in the buffer.
 */
spf_t *spf_buf_resize(spfbuf_t *p, unsigned long n)
{
  if ((p->s = realloc(p->s, n * p->adim * sizeof(spf_t))) != NULL)
    p->m = n;

  return(p->s);
}

/* ------------------------------------------------------------------------------------- */
/* ----- spf_t *spf_buf_append(spfbuf_t *, spf_t *, unsigned short, unsigned long) ----- */
/* ------------------------------------------------------------------------------------- */
/*
 * Append feature vector to buffer -- if buffer is full and resize block 
 * size (nmore) is not null, increment buffer size by nmore, otherwise 
 * returns an error. Returns a pointer to the appended vector in the 
 * buffer or NULL in case of error.
 */
spf_t *spf_buf_append(spfbuf_t *buf, spf_t *s, unsigned short dim, unsigned long nmore)
{
  spf_t *p = NULL;
  unsigned short j;
  unsigned long t;

  if (buf->n == 0) {
    if (dim <= buf->adim)
      buf->dim = dim;
    else {
      fprintf(stderr, "spf_buf_append(): invalid input dimension %d (max=%d)\n", dim, buf->adim);
      return(NULL);
    }
  }
  else if (dim != buf->dim) {
    fprintf(stderr, "spf_buf_append(): invalid input dimension %d (dim=%d)\n", dim, buf->dim);
    return(NULL);
  }

  if (buf->n < buf->m) {
    t = buf->n;
    buf->n++;
    p = get_spf_buf_vec(buf, t);
    for (j = 0; j < dim; j++)
      *(p+j) = *(s+j);
  }
  else if (nmore) {
    if (spf_buf_resize(buf, buf->m + nmore) == NULL) {
      fprintf(stderr, "spf_buf_append(): cannot extend buffer\n");
      return(NULL);
    }
    t = buf->n;
    buf->n++;
    p = get_spf_buf_vec(buf, t);

    for (j = 0; j < dim; j++)
      *(p+j) = *(s+j);
  }
  
  return(p);
}

/* ---------------------------------------------------------- */
/* ----- unsigned long spf_buf_read(spfbuf_t *, FILE *) ----- */
/* ---------------------------------------------------------- */
/*
 * Read buffer data from stream. Return the number of vectors read.
 * To deal with the possible difference between the allocated 
 * dimension and the actual dimension in the buffer, vectors are 
 * read one by one which maybe slow but I hope not too much...
 */
unsigned long spf_buf_read(spfbuf_t *buf, FILE *f)
{
  spf_t *p;
  unsigned long nread = 0;
#ifdef WORDS_BIGENDIAN
  unsigned long i;
  unsigned short j;
#endif /* WORDS_BIGENDIAN */

  p = buf->s;

  while (nread < buf->m) {
    if (fread(p, sizeof(spf_t), buf->dim, f) != buf->dim)
      break;

#ifdef WORDS_BIGENDIAN
    for (j = 0 ; j < buf->dim; j++)
      sp_swap(p+j, sizeof(spf_t));
#endif /* WORDS_BIGENDIAN */

    nread++;
    p += buf->adim;
  }

  buf->n = nread;
  
  return(nread);
}

/* ----------------------------------------------------------- */
/* ----- unsigned long spf_buf_write(spfbuf_t *, FILE *) ----- */
/* ----------------------------------------------------------- */
/*
 * Write buffer data to stream. Return the number of vectors written.
 * To deal with the possible difference between the allocated 
 * dimension and the actual dimension in the buffer, vectors are 
 * written one by one which maybe slow but I hope not too much...
 */
unsigned long spf_buf_write(spfbuf_t *buf, FILE *f)
{
  spf_t *p;
  unsigned long nwritten = 0;
#ifdef WORDS_BIGENDIAN
  unsigned long i;
  unsigned short j;
#endif

  p = buf->s;
  while (nwritten < buf->n) {
#ifdef WORDS_BIGENDIAN
    for (j = 0 ; j < buf->dim; j++)
      sp_swap(p+j, sizeof(spf_t));
#endif /* WORDS_BIGENDIAN */

    if (fwrite(p, sizeof(spf_t), buf->dim, f) != buf->dim)
      break;

    nwritten++;
    p += buf->adim;
  }

  return(nwritten);
}

/* ------------------------------------------------------------- */
/* ----- spf_t *get_spf_buf_vec(spfbuf_t *, unsigned long) ----- */
/* ------------------------------------------------------------- */
/*
 * Return a pointer to the specified vector in the buffer or NULL 
 * in case of error. Frame indexes start at t = 0.
 */
spf_t *get_spf_buf_vec(spfbuf_t *buf, unsigned long t)
{
  if (t >= buf->n)
    return(NULL);

  return(buf->s + t * buf->adim);
}

     /* -------------------------------------------- */
     /* ----- feature stream related functions ----- */ 
     /* -------------------------------------------- */

/* -------------------------------------------------------------------------- */
/* ----- spfstream_t *spf_input_stream_open(const char *, long, size_t) ----- */
/* -------------------------------------------------------------------------- */
/*
 * Open input feature stream. Return a pointer to the stream or NULL
 * in case of error.
 */
spfstream_t *spf_input_stream_open(const char *fn, long flag, size_t nbytes)
{
  spfstream_t *s;
  unsigned short dim, idx[9];

  if ((s = (spfstream_t *)malloc(sizeof(spfstream_t))) == NULL) {
    fprintf(stderr, "spf_input_stream_open(): cannot allocate memory\n");
    return(NULL);
  }

  s->name = NULL;
  s->f = NULL;
  s->iomode = SPRO_STREAM_READ_MODE;
  s->header = NULL;
  s->cflag = flag;
  s->escale = 0.0;
  s->winlen = 0;
  s->buf = NULL;
  s->start = 0;
  s->idx = 0;

  /* set stream filename */
  if (fn && strcmp(fn, "-") != 0) {
    if ((s->name = strdup(fn)) == NULL) {
      fprintf(stderr, "spf_input_stream_open(): cannot set stream name %s\n", fn);
      spf_stream_close(s);
      return(NULL);
    }
    if ((s->f = fopen(fn, "r")) == NULL) {
      fprintf(stderr, "spf_input_stream_open(): cannot open input file %s\n", fn);
      spf_stream_close(s);
      return(NULL);
    }
  }
  else
    s->f = stdin;

  /* read header */
  if ((s->header = spf_header_read(s->f)) == NULL) {
    fprintf(stderr, "spf_input_stream_open(): cannot initialize header from %s\n", (fn) ? (fn) : "stdin"); 
    spf_stream_close(s);
    return(NULL);
  }
  
  if (fread(&(s->idim), SIZEOF_SHORT, 1, s->f) != 1 || 
      fread(&(s->iflag), SIZEOF_LONG, 1, s->f) != 1 || 
      fread(&(s->Fs), sizeof(float), 1, s->f) != 1) {
    fprintf(stderr, "spf_header_read(): cannot read fixed header\n");
    spf_stream_close(s);
    return(NULL);
  }
#ifdef WORDS_BIGENDIAN
  sp_swap(&(s->idim), SIZEOF_SHORT);
  sp_swap(&(s->iflag), SIZEOF_LONG);
  sp_swap(&(s->Fs), sizeof(float));
#endif

  /* initialize output dimension */
  s->oflag = s->iflag  | flag; /* flag is the flag we want *added* */
  if (s->oflag != s->iflag) {
    spf_indexes(idx, s->idim, s->iflag);
    s->odim = spf_tot_dim(idx[1] + 1, s->oflag);
  }
  else
    s->odim = s->idim;

  /* allocate buffer */
  /*
    if a convertion is necessary, allocate a buffer for the highest
    of the input and output dimensions so that we can use this buffer
    for both feature set.  
  */
  dim = s->idim;
  if (s->odim > dim)
    dim = s->odim;

  if ((s->buf = spf_buf_alloc(dim, nbytes)) == NULL) {
    fprintf(stderr, "spf_input_stream_open(): cannot create input buffer for %s\n", (fn) ? (fn) : "stdin"); 
    spf_stream_close(s);
    return(NULL);
  }

  return(s);
}

/* --------------------------------------------------------------------------------------------------------------------------- */
/* ----- spfstream_t *spf_output_stream_open(const char *, unsigned short, long, long, float, const spfield_t *, size_t) ----- */
/* --------------------------------------------------------------------------------------------------------------------------- */
/*
 * Open output feature stream. Return a pointer to the stream or NULL
 * in case of error.
 */
spfstream_t *spf_output_stream_open(const char *fn, unsigned short idim, long iflag, 
				    long cflag, float frate, const spfield_t *vh, size_t nbytes)
{
  spfstream_t *s;
  unsigned short dim, idx[9];
  long flag;
  float rate;

  if ((s = (spfstream_t *)malloc(sizeof(spfstream_t))) == NULL) {
    fprintf(stderr, "spf_output_stream_open(): cannot allocate memory\n");
    return(NULL);
  }
  
  s->name = NULL;
  s->f = NULL;
  s->iomode = SPRO_STREAM_WRITE_MODE;
  s->Fs = frate;

  s->idim = idim;
  s->iflag = iflag;
  s->cflag = cflag;
  s->escale = 0.0;
  s->winlen = 0;

  s->header = NULL;
  s->buf = NULL;
  s->start = 0;
  s->idx = 0;

  /* determine output flag and dim */
  s->oflag = iflag | cflag;
  dim = s->idim;

  if (s->oflag != s->iflag) {
    spf_indexes(idx, s->idim, s->iflag);
    s->odim = spf_tot_dim(idx[1] + 1, s->oflag);
    if (s->odim > dim)
      dim = s->odim;
  }
  else
    s->odim = s->idim;
  
  /* allocate buffer */
  if ((s->buf = spf_buf_alloc(dim, nbytes)) == NULL) {
    fprintf(stderr, "spf_output_stream_open(): cannot create output buffer for %s\n", (fn) ? (fn) : "stdout"); 
    spf_stream_close(s);
    return(NULL);
  }
  s->buf->dim = s->idim; /* set the actual size */

  /* create stream header */
  if ((s->header = spf_header_init(vh)) == NULL) {
    fprintf(stderr, "spf_output_stream_open(): cannot initialize variable length header\n");
    spf_stream_close(s);
    return(NULL);
  }

  /* set stream filename */
  if (fn && strcmp(fn, "-") != 0) {
    if ((s->name = strdup(fn)) == NULL) {
      fprintf(stderr, "spf_output_stream_open(): cannot set stream name %s\n", fn);
      spf_stream_close(s);
      return(NULL);
    }
    if ((s->f = fopen(fn, "w")) == NULL) {
      fprintf(stderr, "spf_output_stream_open(): cannot open input file %s\n", fn);
      spf_stream_close(s);
      return(NULL);
    }
  }
  else
    s->f = stdout;
  
  /* write header to stream */
  if (spf_header_write(s->header, s->f)) {
    fprintf(stderr, "spf_output_stream_open(): cannot write header to %s\n", (fn) ? (fn) : "stdout"); 
    spf_stream_close(s);
    return(NULL);
  }
  
  dim = s->odim;
  flag = s->oflag;
  rate = s->Fs;
  
#ifdef WORDS_BIGENDIAN
  sp_swap(&dim, SIZEOF_SHORT);
  sp_swap(&flag, SIZEOF_LONG);
  sp_swap(&rate, sizeof(float));
#endif
  
  if (fwrite(&dim, SIZEOF_SHORT, 1, s->f) != 1 || 
      fwrite(&flag, SIZEOF_LONG, 1, s->f) != 1 || 
      fwrite(&rate, sizeof(float), 1, s->f) != 1) {
    fprintf(stderr, "spf_output_stream_open() -- cannot write fixed header to %s\n", (fn) ? (fn) : "stdout"); 
    return(NULL);
  }
  
  return(s);
}

/* ------------------------------------------------ */
/* ----- void spf_stream_close(spfstream_t *) ----- */
/* ------------------------------------------------ */
/*
 * Close feature stream. Close opened files, free memory and, 
 * if stream is an output stream, flush buffer. No error code
 * returned, even if flushing generates an error.
 */
void spf_stream_close(spfstream_t *s)
{
  if (s) {

    spf_stream_flush(s);
    
    if (s->name) {
      free(s->name);
      if (s->f)
	fclose(s->f);
    }

    spf_header_free(s->header);
    spf_buf_free(s->buf);
    
    free(s);
  }
}

/* ----------------------------------------------- */
/* ------ int spf_stream_read(spfstream_t *) ----- */
/* ----------------------------------------------- */
/*
 * Read new data from stream into the buffer. Return the
 * number of feature vectors transfered to the buffer.
 */
unsigned long spf_stream_read(spfstream_t *s)
{
  unsigned long nread;
  unsigned short idx[9];

  if (s->iomode != SPRO_STREAM_READ_MODE) {
    fprintf(stderr, "spf_stream_read(): cannot read from an output stream!\n");
    return(0);
  }

  s->buf->dim = s->idim;        /* reset actual dimension to input stream dimension */
  s->start += s->buf->n;        /* increment start from buffer content */
  nread = spf_buf_read(s->buf, s->f);
  s->idx = 0;                   /* reset relative buffer index */
  
  if ((s->iflag & WITHE) && s->escale != 0.0) {
    spf_indexes(idx, s->idim, s->iflag);
    scale_energy(s->buf, idx[2], s->escale, s->winlen);
  }

  if (s->oflag != s->iflag)
    if (spf_buf_convert(s->buf, s->iflag, s->oflag, s->winlen, SPRO_CONV_UPDATE) == NULL) {
      fprintf(stderr, "spf_stream_read(): cannot convert input data\n");
      return(0);
    }

  return(nread);
}

/* ---------------------------------------------------------------------------------- */
/* ------ unsigned long spf_stream_write(spfstream_t *, spf_t *, unsigned long) ----- */
/* ---------------------------------------------------------------------------------- */
/*
 * Write data into the stream buffer, flushing the buffer
 * to file if necessary. Return the number of feature vectors 
 * transfered to the buffer.
 */
unsigned long spf_stream_write(spfstream_t *s, spf_t *p, unsigned long n)
{
  unsigned long i, nwritten = 0;
  spf_t *pp = p;

  if (s->iomode != SPRO_STREAM_WRITE_MODE) {
    fprintf(stderr, "spf_stream_write(): cannot write to an inut stream!\n");
    return(0);
  }

  for (i = 0; i < n; i++) {
    if (spf_buf_append(s->buf, pp, s->idim, 0) == NULL) {
      /* 
	 If cannot append, flush stream and retry. 
	 If still cannot append, then it's an error...
      */
      spf_stream_flush(s);

      if (spf_buf_append(s->buf, pp, s->idim, 0) == NULL)
	return(nwritten);
    }
    nwritten++;
    s->idx++;
    pp += s->idim;
  }

  return(nwritten);
}

/* --------------------------------------------------------- */
/* ----- unsigned long spf_stream_flush(spfstream_t *) ----- */
/* --------------------------------------------------------- */
/*
 * Flush content of (output) buffer. Long term operations on features,
 * such as cepstral mean removal and other stuff is done here.
 *
 * Return the number of elements transfered to file or 0 in case of error.
 */
unsigned long spf_stream_flush(spfstream_t *s)
{
  unsigned long nwritten;
  unsigned short idx[9];

  if (s->iomode != SPRO_STREAM_WRITE_MODE)
    return(0);

  if ((s->iflag & WITHE) && s->escale != 0.0) {
    spf_indexes(idx, s->idim, s->iflag);
    scale_energy(s->buf, idx[2], s->escale, s->winlen);
  }

  if (s->iflag != s->oflag)
    if (spf_buf_convert(s->buf, s->iflag, s->oflag, s->winlen, SPRO_CONV_UPDATE) == NULL) {
      fprintf(stderr, "spf_stream_flush(): cannot convert buffer\n");
      return(0);
    }

  /* write to output stream */
  nwritten = spf_buf_write(s->buf, s->f);
  s->start += s->buf->n;
  s->idx = 0;
  s->buf->dim = s->idim;
  s->buf->n = 0;

  return(nwritten);
}

/* ------------------------------------------------------------------ */
/* ----- int spf_stream_seek(spfstream_t *, unsigned long, int) ----- */
/* ------------------------------------------------------------------ */
/*
 * Seek into the stream so that the next call to get_next_spf_stream()
 * returns a pointer to the (absolute) specified frame number. Return
 * 0 if ok.
 */
int spf_stream_seek(spfstream_t *s, long offset, int whence)
{
  unsigned long st, en, t;
  long n;

  st = s->start;
  en = st + s->buf->n - 1;

  /* first, compute the absolute target frame number ... */
  if (whence == SEEK_SET)
    t = offset;
  else if (whence == SEEK_CUR)
    t = spf_stream_tell(s) + offset;
  else
    return(SPRO_STREAM_SEEK_ERR);

  /* ... then, compute the target absolute start index and relative
     buffer index: if the target vector is already in the buffer,
     simply change the current index. Otherwise, the buffer must be
     modified so that its first element is the target vector. This
     means that, in read mode, seeking backward is painfully slow
     since the buffer has to be reloaded all the time but,
     fortunately, we usually read forward ... */
  
  if (t >= s->start && t < s->start + s->buf->n)
    s->idx = t - st;
  else if (s->iomode == SPRO_STREAM_READ_MODE) {
    /* we first have to seek in the file before reading the new
    buffer. We are targeting frame t while the current position in the
    file s->f corresponds to s->start+s->buf->n, so we'll have to seek
    n=t-s->start-s->buf->n frames w.r.t. the current file
    position. Note that, because of the variable length header, it is
    *not* possible to seek in the Unix stream according to the
    beginning of the file. */

    n = t - s->start - s->buf->n;
    if (fseek(s->f, n * s->idim * sizeof(spf_t), SEEK_CUR) != 0)
      return(SPRO_STREAM_SEEK_ERR);

    /* now, we can (virtually) empty he buffer and read new data into
       the buffer */
    s->start = t;
    s->idx = 0;
    s->buf->n = 0;

    if (spf_stream_read(s) == 0)
      return(SPRO_STREAM_SEEK_ERR);

  } else if (s->iomode == SPRO_STREAM_WRITE_MODE) {
    spf_stream_flush(s);
    /* We have to seek in the stream's file to get to the right
    position. We are targeting frame t while the current position in
    the file corresponds to frame s->start, and we therefore have to
    seek n=t-s->start frames from the current position. Note that,
    because of the variable length header, it is *not* possible to
    seek in the Unix stream according to the beginning of the
    file.  */
    n = t - s->start;

    if (fseek(s->f, n * s->odim * sizeof(spf_t), SEEK_CUR) != 0)
      return(SPRO_STREAM_SEEK_ERR);

    s->start = t;
    s->idx = 0;
  }
  else
    return(SPRO_STREAM_SEEK_ERR);

  return(0);
}

/* ---------------------------------------------------- */
/* ----- spf_t *get_next_spf_frame(spfstream_t *) ----- */
/* ---------------------------------------------------- */
/*
 * Return pointer to the next frame from stream or NULL if end of
 * stream.
 */
spf_t *get_next_spf_frame(spfstream_t *s)
{
  spf_t *p;

  /* if no more data available in the buffer we have to read some new
     ones or flush the current ones, depending on the stream I/O
     mode. */
  if (s->idx == s->buf->n)
    switch (s->iomode) {

    case SPRO_STREAM_READ_MODE: 
      spf_stream_read(s); 
      break;

    case SPRO_STREAM_WRITE_MODE: 
      spf_stream_flush(s); 
      break;

    default:     
      fprintf(stderr, "get_next_spf_stream(): invalid feature stream mode %d\n", s->iomode);
      return(NULL); 
      break;
    }
    
  if ((p = get_spf_buf_vec(s->buf, s->idx)) != NULL)
    s->idx++;
  
  return(p);
}

/* --------------------------------------------- */
/* ----- long sp_str_to_flag(const char *) ----- */
/* --------------------------------------------- */
/*
 * Convert a data description string to binary flag, ignoring unkown
 * convertion letters.
 */
long sp_str_to_flag(const char *str)
{
  long flag = 0;
  const char *p = str;

  if (str)
    while (*p) { 
      switch (*p) {
      case 'E': flag |= WITHE; break;
      case 'D': flag |= WITHD; break;
      case 'A': flag |= WITHA; break;
      case 'Z': flag |= WITHZ; break;
      case 'R': flag |= WITHR; break;
      case 'N': flag |= WITHN; break;
      default: break;
      }
      p++;
    }

  return(flag);
}

/* ------------------------------------------------ */
/* ----- char *sp_flag_to_str(long, char [7]) ----- */
/* ------------------------------------------------ */
/*
 * Transform flag to stream description string.
 */
char *sp_flag_to_str(long flag, char str[7])
{
  char *p = str;

  if (flag & WITHZ) {
    *p = 'Z'; 
    p++;
  }
  if (flag & WITHR) {
    *p = 'R'; 
    p++;
  }
  if (flag & WITHE) {
    *p = 'E'; 
    p++;
  }
  if (flag & WITHD) {
    *p = 'D'; 
    p++;
  }
  if (flag & WITHA) {
    *p = 'A'; 
    p++;
  }
  if (flag & WITHN) {
    *p = 'N'; 
    p++;
  }
  *p = 0x00;  
  
  return(str);
}

#undef _spf_c_
