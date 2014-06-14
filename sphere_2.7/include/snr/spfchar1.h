
#ifndef _SPFCHAR1_H_
#define _SPFCHAR1_H_

#define SPFCHAR_H_VERSION "V1.0"

/* file spfchar1.c */
/* structure for important speech file characteristics: */
typedef struct spfchars
  {SP_INTEGER channel_count;
   SP_INTEGER sample_count;
   SP_INTEGER sample_rate;
   SP_INTEGER sample_n_bytes;
   char sample_coding[50];
   int channels_interleaved;
   int swap_bytes;
   double dc_bias;
  } SPFILE_CHARS;

/* gspfchar.c */	int read_hdr PROTO((SP_FILE *, char *, char *, int, int));
/* gspfchar.c */	void load_sp_char PROTO((SP_FILE *h, SPFILE_CHARS *sp_char));
/* gspfchar.c */	void print_sp_chars PROTO((SPFILE_CHARS *sp_char));

#endif
