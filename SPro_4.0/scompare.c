/******************************************************************************/
/*                                                                            */
/*                                  compare.c                                 */
/*                                                                            */
/*                               SPro utilities                               */
/*                                                                            */
/* Guig                                                       Fri Jul 25 2003 */
/* -------------------------------------------------------------------------- */
/*  Copyright (C) 2003 Guillaume Gravier (ggravier@irisa.fr)                  */
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
 *   $Author: ggravier $
 *   $Date: 2003/07/25 15:42:11 $
 *   $Revision: 1.1 $
 *
 */

/* Compares the content of two feature streams. Return 0 if they are
 * the same or an error code otherwise.  
 */

#define _compare_c_

static char *cvsid = "$Header: /udd/ggravier/dev/repository/spro/scompare.c,v 1.1 2003/07/25 15:42:11 ggravier Exp $";

#include <spro.h>
#include <getopt.h>

#define BUFSIZE 10000

/* ----------------------------------------------- */
/* ----- global variables set by read_args() ----- */
/* ----------------------------------------------- */
float epsilon = 0.0001;           /* tolerance                                */
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
  fprintf(stdout, "    Compare input streams.\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "Options:\n");
  fprintf(stdout, "  -e, --epsilon=f           set tolerance to f (%f)\n", epsilon);
  fprintf(stdout, "  -q, --quiet               be real quiet!\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "  -v, --verbose             verbose mode\n");
  fprintf(stdout, "  -V, --version             print version number\n");
  fprintf(stdout, "  -h, --help                this help message\n");
  fprintf(stdout, "\n");
}

/* ----------------------------- */
/* ----- long option array ----- */
/* ----------------------------- */
static struct option longopts[] = {
  {"epsilon", required_argument, NULL, 'e'},
  {"quiet", no_argument, NULL, 'q'},
  {"verbose", no_argument, NULL, 'v'},
  {"version", no_argument, NULL, 'V'},
  {"help", no_argument, NULL, 'h'},
  {0, 0, 0, 0}
};

/* --------------------------------- */
/* ----- int main(int,char **) ----- */
/* --------------------------------- */
int main(int argc,char **argv)
{
  char *fn1, *fn2;
  spfstream_t *f1, *f2;
  spf_t *yt1, *yt2;
  unsigned short dim, j;
  unsigned long t = 0;
  int status = 0;

  int read_args(int, char **);

  /* ----- process command line ----- */
  if (read_args(argc, argv))
    return(1);

  if (optind < argc)
    fn1 = argv[optind++];
  else {
    fprintf(stderr, "compare error -- no input filename specified (use --help to get usage)\n");
    return(1);
  }

  if (optind < argc)
    fn2 = argv[optind++];
  else {
    fprintf(stderr, "compare error -- no output filename specified (use --help to get usage)\n");
    return(1);
  }

  if (optind < argc) {
    fprintf(stderr, "compare error -- invalid number of arguments (use --help to get usage)\n");
    return(1);
  }

  /* ----- open input streams ----- */
  if ((f1 = spf_input_stream_open(fn1, 0, BUFSIZE)) == NULL) {
    fprintf(stderr, "compare error -- cannot open input stream %s\n", fn1);
    return(SPRO_STREAM_OPEN_ERR);
  }

  if ((f2 = spf_input_stream_open(fn2, 0, BUFSIZE)) == NULL) {
    fprintf(stderr, "compare error -- cannot open input stream %s\n", fn2);
    spf_stream_close(f1);
    return(SPRO_STREAM_OPEN_ERR);
  }

  if (trace > 0) {
    fprintf(stdout, "comparing %s and %s:\n", fn1, fn2);
    fflush(stdout);
  }

  /* ----- check headers ----- */
  if (spf_stream_dim(f1) != spf_stream_dim(f2)) {
    if (trace >= 0)
      fprintf(stdout, "stream dimensions differ!\n");
    spf_stream_close(f1); spf_stream_close(f2);
    return(1);
  }
  dim = spf_stream_dim(f1);

  if (spf_stream_rate(f1) != spf_stream_rate(f2)) {
    if (trace >= 0)
      fprintf(stdout, "stream rates differ!\n");
    spf_stream_close(f1); spf_stream_close(f2);
    return(1);
  }
  
  if (spf_stream_flag(f1) != spf_stream_flag(f2)) {
    if (trace >= 0)
      fprintf(stdout, "stream descriptions differ!\n");
    spf_stream_close(f1); spf_stream_close(f2);
    return(1);
  }

  /* ----- check features ----- */
  yt1 = get_next_spf_frame(f1);
  yt2 = get_next_spf_frame(f2);
  
  while (yt1 != NULL && yt2 != NULL) {
    
    for (j = 0; j < dim; j++) {
      double x = *(yt1+j) - *(yt2+j);
      
      if (x < -epsilon || x > epsilon) {
	if (trace > 0)
	  fprintf(stdout, "vectors differ at time %lu", t);
	status = 3;
	break;
      }
    }
    
    if (status && trace <= 0)
      break;
    
    yt1 = get_next_spf_frame(f1);
    yt2 = get_next_spf_frame(f2);
    t += 1;
  }
  
  if (t == 0)
    fprintf(stderr, ">>>>> warning >>>>> empty streams!\n");
  
  if (status && trace != -1)
    fprintf(stdout, "stream differs!\n");
  else if ((yt1 == NULL && yt2 != NULL) || (yt2 == NULL && yt1 != NULL)) {
    if (trace != -1)
      fprintf(stdout, "stream lengths differ\n");
    status = 2;
  }

  spf_stream_close(f1); 
  spf_stream_close(f2);
  

  return(status);
}

/* --------------------------------------- */
/* ----- int read_args(int, char **) ----- */
/* --------------------------------------- */
/*
 *  -e, --epsilon=f           set tolerance
 *  -q, --quiet               be real quiet!
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

  while ((c = getopt_long(argc, argv, "e:qvVh", longopts, &i)) != EOF)
    switch (c) {

    case 'e':
      epsilon = (float)strtod(optarg, &p);
      if (p == optarg || *p != 0) {
	fprintf(stderr, "compare error -- invalid argument %s to --epsilon (use --help to get usage)\n", optarg);
	return(1);
      }
      break;

    case 'q':
      if (trace > 0)
	fprintf(stderr, "compare error -- verbose option ignored (--quiet specified)\n");
      trace = -1;
      break;

    case 'v':
      if (trace < 0)
	fprintf(stderr, "compare error -- verbose option ignored (--quiet specified)\n");
      else
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
      fprintf(stderr, "compare error -- unrecognized option %s (use --help to get usage)\n", argv[optind-1]);
      return(1);
    }

  return(0);
}

#undef _compare_c_

