/******************************************************************************/
/*                                                                            */
/*                                 sfbank.c                                   */
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
 * $Date: 2003/08/22 12:44:10 $
 * $Revision: 1.12 $
 *
 */

/*
 * Convert speech input file to filter bank output feature file.
 */

#define _sfbank_c_

#include <spro.h>
#include <getopt.h>

static char *cvsid = "$Id: sfbank.c,v 1.12 2003/08/22 12:44:10 ggravier Exp $";

/* ----------------------------------------------- */
/* ----- global variables set by read_args() ----- */
/* ----------------------------------------------- */
int format = SPRO_SIG_PCM16_FORMAT; /* signal file format                     */
float Fs = 8000.0;                /* input signal sample rate                 */
int channel = 1;                  /* channel to process                       */
int swap = 0;                     /* change input sample byte order           */
size_t ibs = 10000000;            /* input buffer size (in bytes)             */
size_t obs = 10000000;            /* output buffer size (in bytes)            */
float emphco = 0.95;              /* pre-emphasis coefficient                 */
float fm_l = 20.0;                /* frame length in ms                       */
float fm_d = 10.0;                /* frame shift in ms                        */
int win = SPRO_HAMMING_WINDOW;    /* weighting window                         */
unsigned short nfilters = 24;     /* number of filters                        */
float alpha = 0.0;                /* frequency deformation parameter          */
int usemel = 0;                   /* use Mel scale?                           */
float f_min = 0.0;                /* lower frequency bound (in Hz)            */
float f_max = 0.0;                /* higher frequency bound (in Hz)           */
int fftnpts = 512;                /* FFT length                               */
int flag = 0;                     /* output feature stream description        */
int trace = 0;                    /* trace level                              */

/* ---------------------------- */
/* ----- void usage(void) ----- */
/* ---------------------------- */
void usage(void)
{
  fprintf(stdout, "Usage:\n"); 
  fprintf(stdout, "    sfbank [options] ifile ofile\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "Synopsis:\n");
  fprintf(stdout, "    Filter bank analysis of the input signal.\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "Options:\n");
  fprintf(stdout, "  -F, --format=s            input signal file format (pcm16)\n");
  fprintf(stdout, "  -f, --sample-rate=n       signal sample rate for pcm16 input (%.1f Hz)\n", Fs);
  fprintf(stdout, "  -x, --channel=n           channel number (%d)\n", channel);
  fprintf(stdout, "  -B, --swap                swap sample byte order (don't)\n");
  fprintf(stdout, "  -I, --input-bufsize=n     input buffer size in kbytes (10000)\n");
  fprintf(stdout, "  -O, --output-bufsize=n    output buffer size in kbytes (10000)\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "  -k, --pre-emphasis=n      pre-emphasis coefficient (%.2f)\n", emphco);
  fprintf(stdout, "  -l, --length=n            frame length in ms (%.1f ms)\n", fm_l);
  fprintf(stdout, "  -d, --shift=n             frame shift in ms (%.1f ms)\n", fm_d);
  fprintf(stdout, "  -w, --window=s            weighting window (HAMMING)\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "  -n, --num-filter=n        number of filters in the filter-bank (%u)\n", nfilters);
  fprintf(stdout, "  -a, --alpha=n             frequency warping parameter (%.1f)\n", alpha);
  fprintf(stdout, "  -m, --mel                 use MEL frequency scale (off)\n");
  fprintf(stdout, "  -i, --freq-min=n          lower frequency bound (0 Hz)\n");
  fprintf(stdout, "  -u, --freq-max=n          higher frequency bound (Fs/2)\n");
  fprintf(stdout, "  -b, --fft-length=n        FFT length (%d)\n", fftnpts);
  fprintf(stdout, "\n");
  fprintf(stdout, "  -D, --delta               add first order derivatives (no)\n");
  fprintf(stdout, "  -A, --acceleration        add second order derivatives (no)\n");
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
  {"format", required_argument, NULL, 'F'},
  {"sample-rate", required_argument, NULL, 'f'},
  {"channel", required_argument, NULL, 'x'},
  {"swap", no_argument, NULL, 'B'},
  {"input-bufsize", required_argument, NULL, 'I'},
  {"output-bufsize", required_argument, NULL, 'O'},
  {"pre-emphasis", required_argument, NULL, 'k'},
  {"length", required_argument, NULL, 'l'},
  {"shift", required_argument, NULL, 'd'},
  {"window", required_argument, NULL, 'w'},
  {"num-filter", required_argument, NULL, 'n'},
  {"alpha", required_argument, NULL, 'a'},
  {"mel", no_argument, NULL, 'm'},
  {"freq-min", required_argument, NULL, 'i'},
  {"freq-max", required_argument, NULL, 'u'},
  {"fft-length", required_argument, NULL, 'b'},
  {"delta", no_argument, NULL, 'D'},
  {"acceleration", no_argument, NULL, 'A'},
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
  char *ifn, *ofn;                /* I/O filenames                            */
  int status;                     /* error status                             */

  int read_args(int, char **);
  int process(char *, char *);

  /* ----- process command line ----- */
  if (read_args(argc, argv))
    return(1);

  if (optind < argc)
    ifn = argv[optind++];
  else {
    fprintf(stderr, "sfbank error -- no input filename specified (use --help to get usage)\n");
    return(1);
  }

  if (optind < argc)
    ofn = argv[optind++];
  else {
    fprintf(stderr, "sfbank error -- no output filename specified (use --help to get usage)\n");
    return(1);
  }
  
  if (optind < argc) {
    fprintf(stderr, "sfbank error -- invalid number of arguments (use --help to get usage)\n");
    return(1);
  }
  
  /* ----- initialize necessary stuff ----- */
  if ((status = fft_init(fftnpts)) != 0) {
    fprintf(stderr, "sfbank error -- cannot initialize FFT with %d points\n", fftnpts);
    return(status);
  }
  
  /* ----- run filter bank analysis ----- */
  if ((status = process(ifn, ofn)) != 0) {
    fft_reset();
    return(status);
  }

  /* ------ reset memory ----- */
  fft_reset();

  return(0);
}

/* --------------------------------------- */
/* ----- int process(char *, char *) ----- */
/* --------------------------------------- */
/*
 * process input file.
 */
int process(char *ifn, char *ofn)
{
  sigstream_t *is;
  spfstream_t *os;
  float frate; /* real frame rate */
  int status;

  int filter_bank_analysis(sigstream_t *, spfstream_t *);
  
  /* ----- show what was asked to do ----- */
  if (trace) {
    fprintf(stdout, "%s --> %s\n", ifn, ofn);
    fflush(stdout);
  }
  
  /* ----- open input stream ----- */
  if ((is = sig_stream_open(ifn, format, Fs, ibs, swap)) == NULL) {
    fprintf(stderr, "sfbank error -- cannot open input stream %s\n", ifn);
    return(SPRO_STREAM_OPEN_ERR);
  }

  /* ----- open output stream ----- */
  frate = (unsigned long)(fm_d * is->Fs / 1000.0) / is->Fs; /* real frame period */

  if ((os = spf_output_stream_open(ofn, nfilters, 0, flag, 1.0 / frate, NULL, obs)) == NULL) {
    fprintf(stderr, "sfbank error -- cannot open output stream %s\n", ofn);
    sig_stream_close(is);
    return(SPRO_STREAM_OPEN_ERR);
  }

  /* ----- run filter bank analysis ----- */
  if ((status = filter_bank_analysis(is, os)) != 0) {
    fprintf(stderr, "sfbank error -- error processing input stream %s\n", ifn);
    sig_stream_close(is); spf_stream_close(os);
    return(status);
  }
  
  /* ----- clean ----- */
  sig_stream_close(is);
  spf_stream_close(os);

  return(0);
}

/* ------------------------------------------------------------------ */
/* ----- int filter_bank_analysis(sigstream_t *, spfstream_t *) ----- */
/* ------------------------------------------------------------------ */
/*
 * Main standard loop for filter bank analysis.
 */
int filter_bank_analysis(sigstream_t *is, spfstream_t *os)
{
  unsigned short *idx;
  unsigned long l, d, j;
  float *w;
  sample_t *buf, *p;
  spsig_t *s;
  spf_t *e;
  int status;

  l = (unsigned long)(fm_l * is->Fs / 1000.0); /* frame length in samples */
  d = (unsigned long)(fm_d * is->Fs / 1000.0); /* frame shift in samples */

  /* ----- initialize some more stuff ----- */ 
  if ((s = sig_alloc(l)) == NULL) /* frame signal */
    return(SPRO_ALLOC_ERR);

  if (win) {
    if ((buf = (sample_t *)malloc(l * sizeof(sample_t))) == NULL) /* frame buffer */
      return(SPRO_ALLOC_ERR);
    if ((w = set_sig_win(l, win)) == NULL) {
      free(buf); sig_free(s);
      return(SPRO_ALLOC_ERR);    
    }
  }
  else {
    buf = s->s;
    w = (float *)NULL;
  }

  if ((e = (spf_t *)malloc(nfilters * sizeof(spf_t))) == NULL) {
    if (win) free(buf); sig_free(s);
    return(SPRO_ALLOC_ERR);
  }

  if (usemel) {
    if ((idx = set_mel_idx(nfilters, f_min / is->Fs, f_max / is->Fs, is->Fs)) == NULL) {
      if (win) free(buf); free(e); sig_free(s); if (win) free(w);
      return(SPRO_ALLOC_ERR);
    }
  }
  else if ((idx = set_alpha_idx(nfilters, alpha, f_min / is->Fs, f_max / is->Fs)) == NULL) {
    if (win) free(buf); free(e); sig_free(s); if (win) free(w);
    return(SPRO_ALLOC_ERR);
  }

  /* ----- loop on each frame ----- */
  while (get_next_sig_frame(is, channel, l, d, emphco, buf)) {

    /* weight signal */
    if (win)
      sig_weight(s, buf, w);

    /* apply the filter bank */
    if ((status = log_filter_bank(s, nfilters, idx, e)) != 0) {
      if (win) free(buf); if (win) free(w); free(e); sig_free(s); free(idx);
      return(status);
    }

    /* add vector to data */
    if (spf_stream_write(os, e, 1) != 1) { 
      if (win) free(buf); free(e); sig_free(s); if (win) free(w);
      return(SPRO_FEATURE_WRITE_ERR);
    }
  }
  
  /* ----- reset memory ----- */
  if (win) {
    free(buf); 
    free(w);
  }  
  free(e); 
  sig_free(s); 
  free(idx);

  return(0);
}

/* --------------------------------------- */
/* ----- int read_args(int, char **) ----- */
/* --------------------------------------- */
/*
 *  -F, --format=s            signal file format (pcm)
 *  -f, --sample-rate=f       signal sample rate (8000 Hz)
 *  -x, --channel=n           use channel number (n) (1)
 *  -B, --swap                swap input stream samples
 *  -M, --input-bufsize=n     input buffer size (in bytes)
 *  -S, --output-bufsize=n    output buffer size (in bytes)
 *  -k, --pre-emphasis=f      pre-emphasis coefficient (0.95)
 *  -l, --length=f            frame length in ms (20 ms)
 *  -d, --shift=f             frame shift in ms (10 ms)
 *  -w, --window=s            weighting window (hamming)
 *  -n, --num-filter=n        number of filters in the filter-bank (24)
 *  -a, --alpha=n             frequency warping parameter (0.0)
 *  -m, --mel                 use MEL frequency scale (off)
 *  -i, --freq-min=f          lower frequency bound (0 Hz)
 *  -u, --freq-max=f          higher frequency bound (Fs/2)
 *  -b, --fft-length=n        FFT length (512)
 *  -D, --delta               
 *  -A, --acceleration
 *  -v, --verbose             verbose mode
 *  -V, --version             print version number
 *  -h, --help
 */
int read_args(int argc, char **argv)
{
  int c, i;
  char *p;
  char *argstr = "F:f:x:BI:O:k:l:d:w:n:a:mi:u:b:DAvVh";

  opterr = 0;
  optopt = '!';

  while ((c = getopt_long(argc, argv, argstr, longopts, &i)) != EOF)

    switch (c) {

    case 'F':
      if (strcasecmp(optarg, "pcm16") == 0)
	format = SPRO_SIG_PCM16_FORMAT;
      else if(strcasecmp(optarg, "wave") == 0)
	format = SPRO_SIG_WAVE_FORMAT;
#ifdef SPHERE
      else if(strcasecmp(optarg, "sphere") == 0)
	format = SPRO_SIG_SPHERE_FORMAT;
#endif
      else {
	fprintf(stderr, "sfbank error -- unknown input signal format %s with --format\n", optarg);
	fprintf(stderr, "                must be one of PCM16, WAVE or SPHERE.\n");
	return(1);
      }
      break;

    case 'f':
      Fs = (float)strtod(optarg, &p);
      if (p == optarg || *p != 0) {
	fprintf(stderr, "sfbank error -- invalid argument %s to --sample-rate (use --help to get usage)\n", optarg);
	return(1);
      }
      break;

    case 'x':
      channel = (int)strtol(optarg, &p, 10);
      if (p == optarg || *p != 0) {
	fprintf(stderr, "sfbank error -- invalid argument %s to --channel (use --help to get usage)\n", optarg);
	return(1);
      }
      if (channel < 1 || channel > 2) {
	fprintf(stderr, "sfbank error -- invalid channel number %d (not in [1,2])\n", channel);
	return(1);
      }
      break;

    case 'B':
      swap = 1;
      break;

    case 'I':
      ibs = (size_t)strtol(optarg, &p, 10) * 1000;
      if (p == optarg || *p != 0) {
	fprintf(stderr, "sfbank error -- invalid argument %s to --input-bufsize (use --help to get usage)\n", optarg);
	return(1);
      }
      break;

    case 'O':
      obs = (size_t)strtol(optarg, &p, 10) * 1000;
      if (p == optarg || *p != 0) {
	fprintf(stderr, "sfbank error -- invalid argument %s to --output-bufsize (use --help to get usage)\n", optarg);
	return(1);
      }
      break;

    case 'k':
      emphco = (float)strtod(optarg, &p);
      if (p == optarg || *p != 0) {
	fprintf(stderr, "sfbank error -- invalid argument %s to --pre-emphasis (use --help to get usage)\n", optarg);
	return(1);
      }
      if (emphco < 0 || emphco >= 1.0) {
	fprintf(stderr, "sfbank error -- invalid pre-emphasis coefficient %f (not in [0,1[)\n", emphco);
	return(1);
      }
      break;

    case 'l':
      fm_l = (float)strtod(optarg, &p);
      if (p == optarg || *p != 0) {
	fprintf(stderr, "sfbank error -- invalid argument %s to --length (use --help to get usage)\n", optarg);
	return(1);
      }
      break;

    case 'd':
      fm_d = (float)strtod(optarg, &p);
      if (p == optarg || *p != 0) {
	fprintf(stderr, "sfbank error -- invalid argument %s to --shift (use --help to get usage)\n", optarg);
	return(1);
      }
      break;

    case 'w':
      if (strcasecmp(optarg, "none") == 0)
	win = SPRO_NULL_WINDOW;
      else if (strcasecmp(optarg, "hamming") == 0)
	win = SPRO_HAMMING_WINDOW;
      else if (strcasecmp(optarg, "hanning") == 0)
	win = SPRO_HANNING_WINDOW;
      else if (strcasecmp(optarg, "blackman") == 0)
	win = SPRO_BLACKMAN_WINDOW;
      else {
	fprintf(stderr, "sfbank error -- unknown window type %s with --window\n", optarg);
	fprintf(stderr, "                must be one of NONE, HAMMING, HANNING or BLACKMAN.\n");
	return(1);
      }
      break;

    case 'n':
      nfilters = (unsigned short)strtol(optarg, &p, 10);
      if (p == optarg || *p != 0) {
	fprintf(stderr, "sfbank error -- invalid argument %s to --num-filter (use --help to get usage)\n", optarg);
	return(1);
      }
      if (nfilters < SPRO_MIN_FILTERS || nfilters > SPRO_MAX_FILTERS) {
	fprintf(stderr, "sfbank error -- invalid number of filters %d (not in [%d,%d])\n", 
		nfilters, SPRO_MIN_FILTERS, SPRO_MAX_FILTERS);
	return(1);    
      }
      break;

    case 'a':
      alpha = (float)strtod(optarg, &p);
      if (p == optarg || *p != 0) {
	fprintf(stderr, "sfbank error -- invalid argument %s to --alpha (use --help to get usage)\n", optarg);
	return(1);
      }
      if (alpha <= -1 || alpha >= 1) {
	fprintf(stderr, "sfbank error -- invalid spectral resolution coefficient %f (not in ]-1,1[)\n", alpha);
	return(1);
      }
      break;

    case 'm':
      usemel = 1;
      break;

    case 'i':
      f_min = (float)strtod(optarg, &p);
      if (p == optarg || *p != 0) {
	fprintf(stderr, "sfbank error -- invalid argument %s to --freq-min (use --help to get usage)\n", optarg);
	return(1);
      }
      break;

    case 'u':
      f_max = (float)strtod(optarg, &p);
      if (p == optarg || *p != 0) {
	fprintf(stderr, "sfbank error -- invalid argument %s to --freq-max (use --help to get usage)\n", optarg);
	return(1);
      }
      break;

    case 'b':
      fftnpts = (int)strtol(optarg, &p, 10);
      if (p == optarg || *p != 0) {
	fprintf(stderr, "sfbank error -- invalid argument %s to --fft-length (use --help to get usage)\n", optarg);
	return(1);
      }
      if (fftnpts < SPRO_MIN_FFT_SIZE || fftnpts > SPRO_MAX_FFT_SIZE) {
	fprintf(stderr, "sfbank error -- invalid FFT length %d (not in [%d,%d])\n", 
		fftnpts, SPRO_MIN_FFT_SIZE, SPRO_MAX_FFT_SIZE);
	return(1);
      }
      break;

    case 'D':
      flag |= WITHD;
      break;

    case 'A':
      flag |= WITHA;
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
      fprintf(stderr, "sfbank error -- unrecognized option %s (use --help to get usage)\n", argv[optind-1]);
      return(1);
    }

  /* ----- basic argument sanity check ----- */
  if (usemel && alpha != 0.0)
    fprintf(stderr, "sfbank warning -- ignoring --alpha option with --mel specified\n");

  if (f_max != 0.0 && f_min != 0.0 && f_max <= f_min) {
    fprintf(stderr, "sfbank error -- invalid frequency range specification\n");
    return(1);
  }

  if ((flag | WITHA) && !(flag | WITHD)) {
    fprintf(stderr, "sfbank error -- cannot specify --acceleration without --delta\n");
    return(1);
  }

  if (fm_l < fm_d)
    fprintf(stderr, "sfbank warning -- using non overlapping frames\n");
  return(0);
}

#undef _sfbank_c_
