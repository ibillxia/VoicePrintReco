/* File: sp.h */

#ifndef _SP_H_
#define _SP_H_

/* Global variables */

extern int sp_verbose;

extern char *std_fields[];

#if defined(__STDC__) || defined(__GNUC__) || defined(sgi)
#define PROTO(ARGS)	ARGS
#else
#define PROTO(ARGS)	()
#endif

/* User library functions */

/* dif_wavf.c */ int diff_data PROTO((char *, char *, int , FILE *)) ;
/* dif_wavf.c */ int diff_header PROTO((char *, char *, int *, int *,
					int *, int , FILE *)) ;
/* dif_wavf.c */ int diff_waveforms PROTO((char *, char *, char *, char *,
					   int , FILE *)) ;
/* dif_wavf.c */ int diff_SP_FILE_waveforms PROTO((SP_FILE *, SP_FILE *,
						   char *, char *,int,FILE *));
/* dif_wavf.c */ int diff_files PROTO((char *, char *)) ;

/* h_fix.c */    int correct_out_of_date_headers PROTO((SP_FILE *sp));

/* nat_bord.c */ enum SP_sample_byte_fmt get_natural_sbf PROTO((int)) ;

/* ntoh.c */     short sp_htons PROTO((short t));
/* ntoh.c */     long sp_ntohl PROTO((long t));

/* sdatamod.c */ int sp_set_data_mode PROTO((SP_FILE *, char *)) ;

/* sp_utils.c */ int sp_add_field PROTO((struct header_t *, char *, int ,
					 char *)) ;
/* sp_utils.c */ int sp_change_field PROTO((struct header_t *, char *, int,
					    char *)) ;
/* sp_utils.c */ int sp_clear_fields PROTO((register struct header_t *)) ;
/* sp_utils.c */ int sp_close_header PROTO((register struct header_t *)) ;
/* sp_utils.c */ int sp_delete_field PROTO((struct header_t *, char *)) ;
/* sp_utils.c */ int sp_get_data PROTO((struct header_t *, char *, char *,
					int *)) ;
/* sp_utils.c */ int sp_get_field PROTO((struct header_t *, char *, int *,
					 int *)) ;
/* sp_utils.c */ int sp_get_fieldnames PROTO((struct header_t *, int ,
					      char **)) ;
/* sp_utils.c */ int sp_get_nfields PROTO((struct header_t *)) ;
/* sp_utils.c */ int sp_get_size PROTO((struct header_t *, char *)) ;
/* sp_utils.c */ int sp_get_type PROTO((struct header_t *, char *)) ;
/* sp_utils.c */ int sp_is_std PROTO((register char *)) ;
/* sp_utils.c */ struct header_t *sp_create_header PROTO((void)) ;
/* sp_utils.c */ struct header_t *sp_open_header PROTO((register FILE *, int,
							char **)) ;
/* sp_utils.c */ int sp_copy_header PROTO((SP_FILE *, SP_FILE *)) ;
/* sp_utils.c */ SP_INTEGER sp_file_header_size PROTO((char *file)) ;
/* sp_utils.c */ SP_INTEGER sp_header_size PROTO((struct header_t *)) ;
/* sp_utils.c */ struct header_t *sp_dup_header PROTO((struct header_t *h)) ;

/* spalloc.c */  SP_FILE *sp_alloc_and_init_sphere_t PROTO((void)) ;
/* spalloc.c */  SPIFR *alloc_SPIFR PROTO((void)) ;
/* spalloc.c */  CHANNELS *alloc_CHANNELS PROTO((int, int));
/* spalloc.c */  int free_CHANNELS PROTO((SPIFR *)) ;
/* spalloc.c */  int free_SPIFR PROTO((SPIFR *spifr)) ;
/* spalloc.c */  int free_sphere_t PROTO((SP_FILE *sp)) ;
/* spalloc.c */  void free_SPIFR_waveform_buffers PROTO((SPIFR *spifr));

/* spchksum.c */ SP_CHECKSUM sp_add_checksum PROTO((SP_CHECKSUM,SP_CHECKSUM)) ;
/* spchksum.c */ SP_CHECKSUM sp_compute_char_checksum PROTO((char *, size_t ));
/* spchksum.c */ SP_CHECKSUM sp_compute_short_checksum PROTO((short int *, 
							      size_t , int)) ;
/* spchksum.c */ int sp_compute_checksum PROTO((SP_FILE *sp, SP_CHECKSUM *chks));

/* spclose.c */  int sp_close PROTO((SP_FILE *)) ;

/* speof.c */    int sp_eof PROTO((SP_FILE *)) ;

/* sperror.c */  int sp_error PROTO((SP_FILE *)) ;

/* sphio.c */    int h_delete_field PROTO((struct header_t *, char *)) ;
/* sphio.c */    int h_get_field PROTO((struct header_t *, char *, int ,
					void **)) ;
/* sphio.c */    int h_set_field PROTO((struct header_t *, char *, int ,
					void *)) ;
/* sphio.c */    int sp_h_delete_field PROTO((SP_FILE *, char *)) ;
/* sphio.c */    int sp_h_get_field PROTO((SP_FILE *, char *, int , void **)) ;
/* sphio.c */    int sp_h_set_field PROTO((SP_FILE *, char *, int , void *)) ;

/* sploadf.c */  int sp_load_file PROTO((char *file, char *sdm, SP_INTEGER *nsamp, SP_INTEGER *nchan, SP_INTEGER *nsnb, void **data));

/* spopen.c */   SP_FILE *sp_open PROTO((char *, char *)) ;
/* spopen.c */   void sp_file_dump PROTO((SP_FILE *sp, FILE *fp)) ;
/* spopen.c */   void spifr_dump PROTO((SPIFR *spifr, FILE *fp));
/* spopen.c */   int sp_set_default_operations PROTO((SP_FILE *sp));
/* spopen.c */   int parse_sample_byte_format PROTO((char *str, 
					       enum SP_sample_byte_fmt *sbf));
/* spopen.c */   int parse_sample_coding PROTO((char *str, int sample_n_bytes,
		        enum SP_sample_encoding *sample_encoding,
			enum SP_waveform_comp *wav_compress));

/* spoutput.c */ int sp_format_lines PROTO((struct header_t *,
					    register FILE *)) ;
/* spoutput.c */ int sp_fpcopy PROTO((register FILE *, register FILE *)) ;
/* spoutput.c */ int sp_overwrite_header PROTO((char *, char *, int )) ;
/* spoutput.c */ int sp_print_lines PROTO((struct header_t *, 
					   register FILE *)) ;
/* spoutput.c */ int sp_write_header PROTO((register FILE *, struct header_t *,
					    SP_INTEGER *, SP_INTEGER *)) ;

/* sprewind.c */ int sp_rewind PROTO((SP_FILE *sp));

/* spread.c */   int sp_mc_read_data PROTO((void *, size_t num_, SP_FILE *)) ;

/* sprstat.c */  int sp_get_return_type PROTO((void));
/* sprstat.c */  int sp_get_return_status PROTO((void)) ;
/* sprstat.c */  int sp_print_return_status PROTO((FILE *)) ;

/* sptemp.c */   char * sptemp PROTO((char *)) ;
/* sptemp.c */   char * sptemp_dirfile PROTO((void)) ;

/* sputils2.c */ char *enum_str_SP_data_format PROTO((enum SP_data_format )) ;
/* sputils2.c */ char *enum_str_SP_file_open_mode PROTO((
					       enum SP_file_open_mode )) ;
/* sputils2.c */ char *enum_str_SP_sample_byte_fmt PROTO((
					       enum SP_sample_byte_fmt )) ;
/* sputils2.c */ char *enum_str_SP_sample_encoding PROTO((
					       enum SP_sample_encoding )) ;
/* sputils2.c */ char *enum_str_SP_waveform_comp PROTO((
					       enum SP_waveform_comp )) ;
/* sputils2.c */ char *get_natural_byte_order PROTO((int bps));
/* sputils2.c */ char *header_str_SP_sample_byte_fmt PROTO((
					       enum SP_sample_byte_fmt )) ;
/* sputils2.c */ int convert_file PROTO((char *, char *, char *, char *)) ;
/* sputils2.c */ int do_update PROTO((char *, char *, char *)) ;
/* sputils2.c */ void *sp_data_alloc PROTO((SP_FILE *, int )) ;
/* sputils2.c */ int sp_data_free PROTO((SP_FILE *, void *)) ;
/* sputils2.c */ int strdiff PROTO((char *, char *)) ;
/* sputils2.c */ int strsame PROTO((char *, char *)) ;

/* spwrite.c */  int sp_mc_write_data PROTO((void *, size_t, SP_FILE *)) ;

/* verbose.c */  void sp_set_verbose PROTO((int )) ;

/* sp/shpacki2.c*/ int shortpack_uncompress PROTO((FOB *, FOB *,
						   struct header_t *));

/* sp/spvers.c */ char *sp_get_version PROTO((void));

/* sp/wavedit.c*/ int wav_edit(int, char **,char *, char *);

/* sp/spseek.c */ int sp_seek(SP_FILE *sp, int offset, int origin);

/* sp/sptell.c */ int sp_tell(SP_FILE *sp);

#endif
