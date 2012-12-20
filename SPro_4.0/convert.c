/******************************************************************************/
/*                                                                            */
/*                                 convert.c                                  */
/*                                                                            */
/*                               SPro Library                                 */
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
 * $Date: 2003/04/09 16:11:49 $
 * $Revision: 1.1 $
 *
 */

/*
 * Convertion routine.
 *
 * Perform convertion of data to bring them in accordance with flag. This is 
 * mainly for purpose of adding delta and delta-delta on the fly, as done in 
 * HTK (why is it that I'm always trying to mimick HTK, hum?). At least, it 
 * saves disk space and sometimes avoid reparameterizing for new experiments...
 *
 * The convertion function can operate under three different modes:
 *
 *   - duplicate: creates and fill in a new buffer whose address is returned,
 *     leaving the input buffer unchanged (mode = SPRO_CONV_DUPLICATE)
 *
 *   - replace: same as the duplicate mode except that the input buffer is
 *     de-allocated (mode = SPRO_CONV_REPLACE)
 *
 *   - update: convert the input buffer in place (mode = SPRO_CONV_UPDATE)
 */

#define _convert_c_

#include <spro.h>

/* ----------------------------------------------------------------------------------------- */
/* ----- int spf_add_delta(spfbuf_t *, unsigned short, unsigned short, unsigned short) ----- */
/* ----------------------------------------------------------------------------------------- */
/*
 * Add delta coefficients for [a, a + n - 1] bins at offset at. 
 * Return 0 if ok or an error code otherwise.
 */
int spf_add_delta(spfbuf_t *buf, unsigned short a, unsigned short n, unsigned short at)
{
  unsigned short b = a + n - 1;

  /* sanity check */
  if (b > buf->dim) {
    fprintf(stderr, "spf_add_delta(): invalid bounds [%u,%u] (dim=%u)\n", a, b, buf->dim);
    return(SPRO_BAD_PARAM_ERR);
  }
  if (at + n - 1> buf->dim) {
    fprintf(stderr, "spf_add_delta(): cannot write %u delta coefficients at offset %u (dim=%u)\n", n, at, buf->dim);
    return(SPRO_BAD_PARAM_ERR);
  }
  if (at <= b) {
    fprintf(stderr, "spf_add_delta(): overlapping delta and static coefficients\n");
    return(SPRO_BAD_PARAM_ERR);
  }

  spf_delta_set(buf, a, n, buf, at);

  return(0);
}

/* --------------------------------------------------------------------------------- */
/* ----- spfbuf_t *spf_buf_convert(spfbuf_t *, long, long, unsigned long, int) ----- */
/* --------------------------------------------------------------------------------- */
/*
 * Convert the input buffer so that the output stream description matches
 * the target stream description. This somewhat complex function has three
 * different modes:
 */
spfbuf_t *spf_buf_convert(spfbuf_t *ibuf, long iflag, long oflag, unsigned long wl, int mode)
{
  unsigned long i;
  unsigned short sdim, odim, ibins[9], obins[9];
  spfbuf_t *obuf;
  spf_t *ip, *op;

  /* sanity checks */
  if ((oflag & WITHE) && !(iflag & WITHE)) {
    fprintf(stderr, "spf_buf_convert(): cannot invent missing energy\n");
    return(NULL);
  }
  if ((oflag & WITHN) && !(oflag & WITHE)) {
    fprintf(stderr, "spf_buf_convert(): cannot suppress static energy without energy\n");
    return(NULL);
  }
  if ((oflag & WITHA) && !(oflag & WITHD)) {
    fprintf(stderr, "spf_buf_convert(): cannot have accelerations without delta\n");
    return(NULL);
  }
  if ((oflag & WITHR) && !(oflag & WITHZ)) {
    fprintf(stderr, "spf_buf_convert(): cannot have variance normalization without mean removal\n");
    return(NULL);
  }
    
  /* find out an output buffer */
  spf_indexes(ibins, ibuf->dim, iflag);
  sdim = ibins[1] + 1; /* base static feature dimension (regardless of energy) */
  odim = spf_tot_dim(sdim, oflag);
  spf_indexes(obins, odim, oflag);

  if (mode == SPRO_CONV_UPDATE) {
    if (odim > ibuf->adim) {
      fprintf(stderr, "spf_buf_convert(): buffer max dimension (%d) too small (odim=%d)\n", ibuf->adim, odim);
      return(NULL);
    }
    obuf = ibuf;
    obuf->dim = odim;
  }
  else {
    if ((obuf = spf_buf_alloc(odim, ibuf->n * odim * sizeof(spf_t))) == NULL) {
      fprintf(stderr, "spf_buf_convert(): cannot allocate memory\n");
      return(NULL);
    }
    obuf->n = ibuf->n;

    /* copy static coefficients */
    ip = ibuf->s;
    op = obuf->s;
    for (i = 0; i < ibuf->n; i++) {
      memcpy(op, ip, sdim * sizeof(spf_t));
      
      if ((oflag & WITHE) && ! (oflag & WITHN))
	*(op+obins[2]) = *(ip+ibins[2]);

      ip += ibuf->adim;
      op += obuf->adim;
    }
  }
  
  /* global normalizations */
  if (((oflag & WITHZ) && ! (iflag & WITHZ)) || ((oflag & WITHR) && ! (iflag & WITHR)))
    if (spf_buf_normalize(obuf, 0, sdim - 1, wl, oflag & WITHR) != 0) {
      fprintf(stderr, "spf_buf_convert(): cannot normalize output buffer\n");
      if (mode != SPRO_CONV_UPDATE) spf_buf_free(obuf);
      return(NULL);
    }
  
  /* add deltas */
  if (oflag & WITHD) {
    /*
      delta energy *must* be computed first since, in update mode, the static 
      energy can be overwritten by the deltas -- check out if deltas already
      exist in which case they may not be recomputed.
    */
    if (oflag & WITHE)
      spf_delta_set(ibuf, ibins[2], 1, obuf, obins[5]);
    spf_delta_set(obuf, 0, sdim, obuf, obins[3]);
  }

  if (oflag & WITHA)
    spf_delta_set(obuf, obins[3], (oflag & WITHE) ? (sdim + 1): (sdim), obuf, obins[6]);

  if (mode == SPRO_CONV_REPLACE)
    spf_buf_free(ibuf);

  return(obuf);
}

/* ------------------------------------------------------------------------------------------------------ */
/* ----- void spf_delta_set(spfbuf_t *, unsigned short, unsigned short, spfbuf_t *, unsigned short) ----- */
/* ------------------------------------------------------------------------------------------------------ */
/*
 * Compute deltas for dim features of the input buffer, starting at bin ik, storing
 * the resulting delta features in the output buffer starting at bin ok.
 *
 * WARNING: this function is highly unsafe memorywise !!!!!!
 */
void spf_delta_set(spfbuf_t *ibuf, unsigned short ik, unsigned short dim, spfbuf_t *obuf, unsigned short ok)
{
  unsigned long i;
  unsigned short j;
  spf_t *op, *next1, *prev1, *next2, *prev2;
  
  prev1 = ibuf->s + ik;
  next2 = ibuf->s + ibuf->adim + ik;
  op = obuf->s + ok;

  for (i = 0; i < ibuf->n; i++) {
    prev2 = prev1;
    next1 = next2;
    if (i > 1)
      prev1 = prev2 + ibuf->adim;
    if (i + 2 < ibuf->n)
      next2 = next1 + ibuf->adim;
    
    for (j = 0; j < dim; j++)
      *(op+j) = (*(next1+j) - *(prev1+j) + 2.0 * (*(next2+j) - *(prev2+j))) * 0.1;

    /* fprintf(stderr, "i=%ld prev2=%f prev1=%f next1=%f next2=%f  delta=%f\n", i, *prev2, *prev1, *next1, *next2, *op); */

    op += obuf->adim;
  }
}

#undef _convert_c_
