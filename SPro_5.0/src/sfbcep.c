/******************************************************************************/
/*                                                                            */
/*                                 sfbcep.c                                   */
/*                                                                            */
/*                               SPro Sources                                 */
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
 * Convert speech input file to cepstral coefficients output feature file.
 */

#define _sfbcep_c_

#include <stdlib.h>
#include <string.h>
#include "spro.h"
#include "gggetopt.h"

static char *cvsid = "$Id: sfbcep.c 151 2010-11-09 15:57:22Z guig $";

/* ----------------------------------------------- */
/* ----- global variables set by read_args() ----- */
/* ----------------------------------------------- */
int format = SPRO_SIG_PCM16_FORMAT; /* signal file format                     */
float Fs = 8000.0;                /* input signal sample rate                 */
int channel = 1;                  /* channel to process                       */
int swap = 0;                     /* change input sample byte order           */
size_t ibs = 10000000;            /* input buffer size                        */
size_t obs = 10000000;            /* output buffer size (in bytes)            */
int with_header = 0;              /* output variable length header            */

float emphco = 0.95;              /* pre-emphasis coefficient                 */
float fm_l = 20.0;                /* frame length in ms                       */
float fm_d = 10.0;                /* frame shift in ms                        */
int win = SPRO_HAMMING_WINDOW;    /* weighting window                         */

unsigned short nfilters = 24;     /* number of filters                        */
float alpha = 0.0;                /* frequency deformation parameter          */
int usemel = 0;                   /* use Usemel scale?                        */
float f_min = 0.0;                /* lower srateuency bound                   */
float f_max = 0.0;                /* higher srateuency bound                  */
int fftnpts = 512;                /* FFT length                               */

unsigned short numceps = 12;      /* number of cepstral coefficients          */
int lifter = 0;                   /* liftering value                          */

int flag = 0;                     /* output stream description                */
unsigned long winlen = 0;         /* length in frames of the CMS window       */
float escale = 0.0;               /* energy scale factor                      */

int trace = 0;                    /* trace level                              */

/* ---------------------------- */
/* ----- void usage(void) ----- */
/* ---------------------------- */
void usage(void) 
{
  fprintf(stdout, "\nUsage:\n"); 
  fprintf(stdout, "    sfbcep [options] ifile ofile\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "Synopsis:\n");
  fprintf(stdout, "    Filter-bank based cepstral analysis of the input signal.\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "Options:\n");
  fprintf(stdout, "  -F, --format=s            input signal file format (pcm16)\n");
  fprintf(stdout, "  -f, --sample-rate=n       signal sample rate for pcm16 input (%.1f Hz)\n", Fs);
  fprintf(stdout, "  -x, --channel=n           channel number (%d)\n", channel);
  fprintf(stdout, "  -B, --swap                swap sample byte order (don't)\n");
  fprintf(stdout, "  -I, --input-bufsize=n     input buffer size in kbytes (10000)\n");
  fprintf(stdout, "  -O, --output-bufsize=n    output buffer size in kbytes (10000)\n");
  fprintf(stdout, "  -H, --header              output variable length header (don't)\n");  
  fprintf(stdout, "\n");
  fprintf(stdout, "  -k, --pre-emphasis=f      pre-emphasis coefficient (%.2f)\n", emphco);
  fprintf(stdout, "  -l, --length=f            frame length in ms (%.1f ms)\n", fm_l);
  fprintf(stdout, "  -d, --shift=f             frame shift in ms (%.1f ms)\n", fm_d);
  fprintf(stdout, "  -w, --window=s            weighting window (HAMMING)\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "  -n, --num-filter=n        number of filters in the filter-bank (%u)\n", nfilters);
  fprintf(stdout, "  -a, --alpha=f             frequency warping parameter (%.1f)\n", alpha);
  fprintf(stdout, "  -m, --mel                 use MEL frequency scale (off)\n");
  fprintf(stdout, "  -i, --freq-min=n          lower frequency bound (0 Hz)\n");
  fprintf(stdout, "  -u, --freq-max=n          higher frequency bound (Fs/2)\n");
  fprintf(stdout, "  -b, --fft-length=n        FFT length (%d)\n", fftnpts);
  fprintf(stdout, "\n");
  fprintf(stdout, "  -p, --num-ceps=n          number of cepstral coefficients (%u)\n", numceps);
  fprintf(stdout, "  -r, --lifter=n            liftering value (%d)\n", lifter);
  fprintf(stdout, "\n");
  fprintf(stdout, "  -e, --energy              add log-energy (off)\n");
  fprintf(stdout, "  -s, --scale-energy=f      scale and normalize log-energy (off)\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "  -Z, --cms                 cepstral mean normalization\n");
  fprintf(stdout, "  -R, --normalize           variance normalization\n");
  fprintf(stdout, "  -L, --segment-length=n    segment length in frames for normalization (whole data)\n");
  fprintf(stdout, "  -D, --delta               add first order derivatives\n");
  fprintf(stdout, "  -A, --acceleration        add second order derivatives\n");
  fprintf(stdout, "  -N, --no-static-energy    remove static energy\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "  -v, --verbose             verbose mode\n");
  fprintf(stdout, "  -V, --version             print version number and exit\n");
  fprintf(stdout, "  -h, --help                this help message\n");
  fprintf(stdout, "\n");
}

/* ---------------------------------- */
/* ----- int main(int, char **) ----- */
/* ---------------------------------- */
int main(int argc, char **argv)
{
  char *ifn, *ofn;                /* I/O filenames                            */
  sigstream_t *is;
  spfstream_t *os;
  float frate; /* real frame rate */
  unsigned short dim;
  struct spf_header_field * vh = NULL;
  int status;                     /* error status                             */

  int read_args(int, char **);
  int cepstral_analysis(sigstream_t *, spfstream_t *);

  struct spf_header_field * set_header(const char *, float);
  void free_header(struct spf_header_field *);

  /* ----- process command line ----- */
  if (read_args(argc, argv))
    return(1);

  if (optind < argc)
    ifn = argv[optind++];
  else {
    fprintf(stderr, "sfbcep error -- no input filename specified (use --help to get usage)\n");
    return(1);
  }

  if (optind < argc)
    ofn = argv[optind++];
  else {
    fprintf(stderr, "sfbcep error -- no input filename specified (use --help to get usage)\n");
    return(1);
  }
  
  if (optind < argc) {
    fprintf(stderr, "sfbcep error -- invalid number of arguments (use --help to get usage)\n");
    return(1);
  }
  
  /* ----- initialize necessary stuff ----- */
  if ((status = fft_init(fftnpts)) != 0) {
    fprintf(stderr, "sfbcep error -- cannot initialize FFT with %d points\n", fftnpts);
    return(status);
  }

  if ((status = dct_init(nfilters, numceps)) != 0) {
    fprintf(stderr, "sfbcep error -- cannot initialize %u x %u DCT kernel\n", nfilters, numceps);
    fft_reset(); 
    return(status);
  }
  
  /* ----- show what was asked to do ----- */
  if (trace) {
    fprintf(stdout, "%s --> %s\n", ifn, ofn);
    fflush(stdout);
  }

  /* ----- open input stream ----- */
  if ((is = sig_stream_open(ifn, format, Fs, ibs, swap)) == NULL) {
    fprintf(stderr, "sfbcep error -- cannot open input stream %s\n", ifn);
    fft_reset(); dct_reset();
    return(SPRO_STREAM_OPEN_ERR);
  }
  
  /* ----- open output stream ----- */
  frate = (unsigned long)(fm_d * is->Fs / 1000.0) / is->Fs; /* real frame period */
  dim = (flag & WITHE) ? numceps + 1 : numceps;

  if (with_header)
    if ((vh = set_header(ifn, is->Fs)) == NULL) {
      fprintf(stderr, "sfbank error -- cannot allocate memory\n");
      fft_reset(); sig_stream_close(is);
      return(SPRO_ALLOC_ERR);
    }

  if ((os = spf_output_stream_open(ofn, dim, flag & WITHE, flag, 1.0 / frate, vh, obs)) == NULL) {
    fprintf(stderr, "sfbcep error -- cannot open output stream %s\n", ofn);
    fft_reset(); dct_reset(); sig_stream_close(is);
    return(SPRO_STREAM_OPEN_ERR);
  }

  free_header(vh);

  if (winlen)
    set_stream_seg_length(os, winlen);

  if (escale != 0.0)
    set_stream_energy_scale(os, escale);

  /* ----- run cepstral analysis ----- */
  if ((status = cepstral_analysis(is, os)) != 0) {
    fprintf(stderr, "sfbcep error -- error processing stream %s\n", ifn);
    fft_reset(); dct_reset(); sig_stream_close(is); spf_stream_close(os);
    return(status);
  }

  /* ----- clean ----- */
  sig_stream_close(is);
  spf_stream_close(os);

  fft_reset();
  dct_reset();

  return(0);
}

/* --------------------------------------------------------------- */
/* ----- int cepstral_analysis(sigstream_t *, spfstream_t *) ----- */
/* --------------------------------------------------------------- */
/*
 * Main standard loop for filter bank analysis.
 */
int cepstral_analysis(sigstream_t *is, spfstream_t *os)
{
  unsigned short *idx;
  unsigned long l, d, j;
  float *w = NULL, *r = NULL;
  sample_t *buf;
  spsig_t *s;
  spf_t *e, *c;
  double energy;
  int status;
  // added for debug by Bill, 2013-07-06
  int i;
  char fn[256];
  FILE *fp;
  sprintf(fn,"%s.mfcc",os->name);
  fp = fopen(fn,"w");
  /* ----- initialize some more stuff ----- */
  l = (unsigned long)(fm_l * is->Fs / 1000.0); /* frame length in samples */
  d = (unsigned long)(fm_d * is->Fs / 1000.0); /* frame shift in samples */

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
  else
    buf = s->s;

  if ((e = (spf_t *)malloc(nfilters * sizeof(spf_t))) == NULL) { /* filter-bank output */
    if (win) free(buf); sig_free(s); if (win) free(w);
    return(SPRO_ALLOC_ERR);
  }

  if ((c = (spf_t *)malloc((numceps+1) * sizeof(spf_t))) == NULL) {
    if (win) free(buf); sig_free(s); free(e); if (win) free(w);
    return(SPRO_ALLOC_ERR);
  }

  if (lifter)
    if ((r = set_lifter(lifter, numceps)) == NULL) {
      if (win) free(buf); sig_free(s); free(e); free(c); if (win) free(w);
      return(SPRO_ALLOC_ERR);      
    }

  if (usemel) {
    if ((idx = set_mel_idx(nfilters, f_min / is->Fs, f_max / is->Fs, is->Fs)) == NULL) {
      if (win) free(buf); sig_free(s); free(e); free(c); if (win) free(w); if (r) free(r);
      return(SPRO_ALLOC_ERR);
    }
  }
  else if ((idx = set_alpha_idx(nfilters, alpha, f_min / is->Fs, f_max / is->Fs)) == NULL) {
    if (win) free(buf); sig_free(s); free(e); free(c); if (win) free(w); if (r) free(r);
    return(SPRO_ALLOC_ERR);
  }
  
  /* ----- loop on each frame ----- */
  while (get_next_sig_frame(is, channel, l, d, emphco, buf)) {

    /* weight signal */
    if (win)
      sig_weight(s, buf, w);

    /* apply the filter bank */
    if ((status = log_filter_bank(s, nfilters, idx, e)) != 0) {
      if (win) free(buf); sig_free(s); free(e); free(c); 
      if (w) free(w); if (r) free(r); free(idx);
      return(status);
    }

    /* DCT */
    if ((status = dct(e, c)) != 0) {
      if (win) free(buf); sig_free(s); free(e); free(c); 
      if (w) free(w); if (r) free(r); free(idx);
      return(status);
    }

    /* liftering */
    if (lifter)
      for (j = 0; j < numceps; j++)
	*(c+j) *= *(r+j);

    /* energy */
    if (flag & WITHE) {
      if ((energy = sig_normalize(s, 0)) < SPRO_ENERGY_FLOOR)
	energy = SPRO_ENERGY_FLOOR;

      *(c+numceps) = (spf_t)(2.0 * log(energy));
    }

    /* write vector to stream */
    if (spf_stream_write(os, c, 1) != 1) { 
      if (win) free(buf); sig_free(s); free(e); free(c); 
      if (w) free(w); if (r) free(r); free(idx);
      return(SPRO_FEATURE_WRITE_ERR);
    }

	// added for debug by Bill, 2013-07-06
	for(i=0;i<os->idim;i++)
		fprintf(fp,"%f ",c[i]);
	fprintf(fp,"\n");
  }

  /* ----- reset memory ----- */
  if (win) {
    free(buf); 
    free(w); 
  }
  sig_free(s); 
  free(e); 
  free(c); 
  if (r) 
    free(r);
  free(idx);

  return(0);
}

/* --------------------------------------------------------------------- */
/* ----- struct spf_header_field * set_header(const char *, float) ----- */
/* --------------------------------------------------------------------- */
/*
 * Create variable length header.
 */
#define MAX_NUM_HEADER_FIELDS 12

struct spf_header_field * set_header(const char *ifn, float Fs)
{
  char str[2048];
  struct spf_header_field * p;
  int i = 0;

  void free_header(struct spf_header_field *);

  if ((p = (struct spf_header_field *)malloc((MAX_NUM_HEADER_FIELDS + 1) * sizeof(struct spf_header_field))) == NULL)
    return(NULL);

  for (i = 0; i <= MAX_NUM_HEADER_FIELDS; i++)
    p[i].name = p[i].value = NULL;

  i = 0;

  /* source file */
  sprintf(str, "%s:%1d", ifn, channel);
  p[i].name = strdup("source_filename"); p[i++].value = strdup(str);

  /* frame stuff:_length, shift, rate and pre-emphasis */
  sprintf(str, "%f", fm_l / 1000.0);
  p[i].name = strdup("frame_length"); p[i++].value = strdup(str);

  sprintf(str, "%f", fm_d / 1000.0);
  p[i].name = strdup("frame_shift"); p[i++].value = strdup(str);

  sprintf(str, "%f", emphco);
  p[i].name = strdup("pre_emphasis"); p[i++].value = strdup(str);

  /* analysis type */
  sprintf(str, "fbank(%d)+cep(%d)", nfilters, numceps);
  p[i].name = strdup("feature_type"); p[i++].value = strdup(str);

  /* miscellaneous parameters */
  sprintf(str, "%d", fftnpts);
  p[i].name = strdup("fft_length"); p[i++].value = strdup(str);

  if (usemel) {
    p[i].name = strdup("frequency_warping"); p[i++].value = strdup("mel");
  }
  else if (alpha != 0.0) {
    sprintf(str, "%f", alpha);
    p[i].name = strdup("frequency_warping"); p[i++].value = strdup(str);
  }
  else {
    p[i].name = strdup("frequency_warping"); p[i++].value = strdup("linear");
  }

  sprintf(str, "%.1f", f_min);
  p[i].name = strdup("lower_frequency"); p[i++].value = strdup(str);

  sprintf(str, "%.1f", (f_max == 0) ? Fs / 2 : f_max);
  p[i].name = strdup("higher_frequency"); p[i++].value = strdup(str);
  
  if ((flag & WITHE) && escale != 0.0) {
    sprintf(str, "%f", escale);
    p[i].name = strdup("energy_scaling"); p[i++].value = strdup(str);
  }

  if (((flag & WITHE) || (flag & WITHZ)) && winlen) {
    sprintf(str, "%lu", winlen);
    p[i].name = strdup("segment_length"); p[i++].value = strdup(str);
  }

  if (lifter) {
    sprintf(str, "%d", lifter);
    p[i].name = strdup("liftering"); p[i++].value = strdup(str);
  }

  /* check all fields were set */
  while (i) {

    i--;

    if (p[i].name == NULL || p[i].value == NULL) {
      free_header(p);
      return(NULL);
    }
  }
  
  return(p);
}

/* ------------------------------------------------------- */
/* ----- void free_header(struct spf_header_field *) ----- */
/* ------------------------------------------------------- */
void free_header(struct spf_header_field *p)
{
  int i;

  if (p) {

    for (i = 0; i < MAX_NUM_HEADER_FIELDS; i++) {
      if (p[i].name)
	free(p[i].name);

      if (p[i].value)
	free(p[i].value);
    }

    free(p);
  }
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
 *  -p, --num-ceps=n          number of cepstral coefficients (16)
 *  -r, --lifter=n            liftering value (0)
 *  -e, --energy              add log-energy (off)
 *  -s, --scale-energy=f      scale and normalize log-energy (off)
 *  -L, --segment-length=n    segment length in frames for normalization (whole data)
 *  -Z, --cms                 mean normalization
 *  -R, --var-norm            variance normalization
 *  -D, --delta               add first order derivatives
 *  -A, --acceleration        add second order derivatives
 *  -N, --no-static-energy    remove static energy
 *  -v, --verbose             verbose mode
 *  -V, --version             print version number
 *  -h, --help
 */
int read_args(int argc, char **argv)
{
  int c, i;
  char *p;

  struct option longopts[] = {
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
    {"num-cep", required_argument, NULL, 'p'},
    {"lifter", required_argument, NULL, 'r'},
    {"energy", no_argument, NULL, 'e'},
    {"scale-energy", required_argument, NULL, 's'},
    {"segment-length", required_argument, NULL, 'L'},
    {"cms", no_argument, NULL, 'Z'},
    {"normalize", no_argument, NULL, 'R'},
    {"delta", no_argument, NULL, 'D'},
    {"acceleration", no_argument, NULL, 'A'},
    {"no-static-energy", no_argument, NULL, 'N'},
    {"verbose", no_argument, NULL, 'v'},
    {"version", no_argument, NULL, 'V'},
    {"help", no_argument, NULL, 'h'},
    {0, 0, 0, 0}
  };

  opterr = 0;
  optopt = '!';

  while ((c = getopt_long(argc, argv, "F:f:x:BI:O:k:l:d:w:n:a:mi:u:b:p:r:es:ZRL:DANvVh", longopts, &i)) != EOF)

    switch (c) {

    case 'F':
      if (strcasecmp(optarg, "pcm16") == 0)
	format = SPRO_SIG_PCM16_FORMAT;
      else if(strcasecmp(optarg, "alaw") == 0)
	format = SPRO_SIG_ALAW_FORMAT;
      else if(strcasecmp(optarg, "ulaw") == 0)
	format = SPRO_SIG_ULAW_FORMAT;
      else if(strcasecmp(optarg, "wave") == 0)
	format = SPRO_SIG_WAVE_FORMAT;
      else {
#ifdef SPHERE
	if(strcasecmp(optarg, "sphere") == 0)
	  format = SPRO_SIG_SPHERE_FORMAT;
	else {
#endif
	  fprintf(stderr, "sfbcep error -- unknown input signal format %s with --format\n", optarg);
	  fprintf(stderr, "                must be one of PCM16, ALAW, ULAW, WAVE or SPHERE.\n");
	  return(1);
#ifdef SPHERE
	}
#endif
      }
      break;

    case 'f':
      Fs = (float)strtod(optarg, &p);
      if (p == optarg || *p != 0) {
	fprintf(stderr, "sfbcep error -- invalid argument %s to --sample-rate (use --help to get usage)\n", optarg);
	return(1);
      }
      break;

    case 'x':
      channel = (int)strtol(optarg, &p, 10);
      if (p == optarg || *p != 0) {
	fprintf(stderr, "sfbcep error -- invalid argument %s to --channel (use --help to get usage)\n", optarg);
	return(1);
      }
      if (channel < 1 || channel > 2) {
	fprintf(stderr, "sfbcep error -- invalid channel number %d (not in [1,2])\n", channel);
	return(1);
      }
      break;

    case 'B':
      swap = 1;
      break;

    case 'I':
      ibs = (size_t)strtol(optarg, &p, 10) * 1000;
      if (p == optarg || *p != 0) {
	fprintf(stderr, "sfbcep error -- invalid argument %s to --input-bufsize (use --help to get usage)\n", optarg);
	return(1);
      }
      break;      

    case 'O':
      obs = (size_t)strtol(optarg, &p, 10) * 1000;
      if (p == optarg || *p != 0) {
	fprintf(stderr, "sfbcep error -- invalid argument %s to --output-bufsize (use --help to get usage)\n", optarg);
	return(1);
      }
      break;      

    case 'k':
      emphco = (float)strtod(optarg, &p);
      if (p == optarg || *p != 0) {
	fprintf(stderr, "sfbcep error -- invalid argument %s to --pre-emphasis (use --help to get usage)\n", optarg);
	return(1);
      }
      if (emphco < 0 || emphco >= 1.0) {
	fprintf(stderr, "sfbcep error -- invalid pre-emphasis coefficient %f (not in [0,1[)\n", emphco);
	return(1);
      }
      break;

    case 'l':
      fm_l = (float)strtod(optarg, &p);
      if (p == optarg || *p != 0) {
	fprintf(stderr, "sfbcep error -- invalid argument %s to --length (use --help to get usage)\n", optarg);
	return(1);
      }
      break;

    case 'd':
      fm_d = (float)strtod(optarg, &p);
      if (p == optarg || *p != 0) {
	fprintf(stderr, "sfbcep error -- invalid argument %s to --shift (use --help to get usage)\n", optarg);
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
	fprintf(stderr, "sfbcep error -- unknown window type %s with --window\n", optarg);
	fprintf(stderr, "                must be one of NONE, HAMMING, HANNING or BLACKMAN.\n");
	return(1);
      }
      break;

    case 'n':
      nfilters = (unsigned short)strtol(optarg, &p, 10);
      if (p == optarg || *p != 0) {
	fprintf(stderr, "sfbcep error -- invalid argument %s to --num-filter (use --help to get usage)\n", optarg);
	return(1);
      }
      if (nfilters < SPRO_MIN_FILTERS || nfilters > SPRO_MAX_FILTERS) {
	fprintf(stderr, "sfbcep error -- invalid number of filters %d (not in [%d,%d])\n", 
		nfilters, SPRO_MIN_FILTERS, SPRO_MAX_FILTERS);
	return(1);    
      }
      break;

    case 'a':
      alpha = (float)strtod(optarg, &p);
      if (p == optarg || *p != 0) {
	fprintf(stderr, "sfbcep error -- invalid argument %s to --alpha (use --help to get usage)\n", optarg);
	return(1);
      }
      if (alpha <= -1 || alpha >= 1) {
	fprintf(stderr, "sfbcep error -- invalid spectral resolution coefficient %f (not in ]-1,1[)\n", alpha);
	return(1);
      }
      break;

    case 'm':
      usemel = 1;
      break;

    case 'i':
      f_min = (float)strtod(optarg, &p);
      if (p == optarg || *p != 0) {
	fprintf(stderr, "sfbcep error -- invalid argument %s to --freq-min (use --help to get usage)\n", optarg);
	return(1);
      }
      break;

    case 'u':
      f_max = (float)strtod(optarg, &p);
      if (p == optarg || *p != 0) {
	fprintf(stderr, "sfbcep error -- invalid argument %s to --freq-max (use --help to get usage)\n", optarg);
	return(1);
      }
      break;

    case 'b':
      fftnpts = (int)strtol(optarg, &p, 10);
      if (p == optarg || *p != 0) {
	fprintf(stderr, "sfbcep error -- invalid argument %s to --fft-length (use --help to get usage)\n", optarg);
	return(1);
      }
      if (fftnpts < SPRO_MIN_FFT_SIZE || fftnpts > SPRO_MAX_FFT_SIZE) {
	fprintf(stderr, "sfbcep error -- invalid FFT length %d (not in [%d,%d])\n", 
		fftnpts, SPRO_MIN_FFT_SIZE, SPRO_MAX_FFT_SIZE);
	return(1);
      }
      break;

    case 'p':
      numceps = (unsigned short)strtol(optarg, &p, 10);
      if (p == optarg || *p != 0) {
	fprintf(stderr, "sfbcep error -- invalid argument %s to --num-ceps (use --help to get usage)\n", optarg);
	return(1);
      }
      if (numceps < 1) {
	fprintf(stderr, "sfbcep error -- null number of cepstral coefficients\n");
	return(1);
      }
      break;

    case 'r':
      lifter = (int)strtol(optarg, &p, 10);
      if (p == optarg || *p != 0) {
	fprintf(stderr, "sfbcep error -- invalid argument %s to --lifter (use --help to get usage)\n", optarg);
	return(1);
      }
      break;

    case 'e':
      flag |= WITHE;
      break;

    case 's':
      escale = (float)strtod(optarg, &p);
      if (p == optarg || *p != 0) {
	fprintf(stderr, "sfbcep error -- invalid argument %s to --scale-energy (use --help to get usage)\n", optarg);
	return(1);
      }
      if (escale <= 0.0) {
	fprintf(stderr, "sfbcep error -- invalid energy scale factor %.2f (not > 0)\n", escale);
	return(1);
      }
      break;

    case 'L':
      winlen = (unsigned long)strtol(optarg, &p, 10);
      if (p == optarg || *p != 0) {
	fprintf(stderr, "sfbcep error -- invalid argument %s to --segment-length (use --help to get usage)\n", optarg);
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
      fprintf(stderr, "sfbcep error -- unrecognized option %s (use --help to get usage)\n", argv[optind-1]);
      return(1);
    }

  /* ----- basic argument sanity check ----- */
  if (usemel && alpha != 0.0)
    fprintf(stderr, "sfbcep warning -- ignoring --alpha option with --mel specified\n");

  if (f_max != 0.0 && f_min != 0.0 && f_max <= f_min) {
    fprintf(stderr, "sfbcep error -- invalid frequency range specification\n");
    return(1);
  }
  
  if (numceps > nfilters) {
    fprintf(stderr, 
	    "sfbcep error -- number of cepstral coefficients %d > number of filters %d\n", 
	    numceps, nfilters);
    return(1);
  }

  if ((flag & WITHN) && !(flag & WITHE)) {
    fprintf(stderr, "sfbcep error -- cannot suppress static energy without energy\n");
    return(1);
  }

  if ((flag & WITHA) && !(flag & WITHD)) {
    fprintf(stderr, "sfbcep error -- cannot have accelerations without delta\n");
    return(1);
  }

  if ((flag & WITHN) && ! (flag & WITHD)) {
    fprintf(stderr, "sfbcep error -- cannot suppress static log-energy if delta not set\n");
    return(1);
  }

  if ((flag & WITHR) && !(flag & WITHZ)) {
    fprintf(stderr, "sfbcep error -- cannot have variance normalization without mean removal\n");
    return(1);
  }

  if (fm_l < fm_d)
    fprintf(stderr, "sfbcep warning -- using non overlapping frames\n");

  return(0);
}

#undef _sfbcep_c_
