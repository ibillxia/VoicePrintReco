/*   File: spfile.h
     Created: 05/06/93
  */

#ifndef _SPFILE_H_
#define _SPFILE_H_

# define the data declaration types used in the SPHERE functions
typedef long   SP_INTEGER;
typedef double SP_REAL;
typedef char * SP_STRING;


enum SP_file_open_mode  { SP_mode_null=100, SP_mode_read, SP_mode_write, 
			      SP_mode_update };
enum SP_sample_byte_fmt { SP_sbf_null=200, SP_sbf_01, SP_sbf_10, SP_sbf_1,
			      SP_sbf_0123, SP_sbf_1032, SP_sbf_2301, SP_sbf_3210, 
			      SP_sbf_N, SP_sbf_orig};
enum SP_sample_encoding { SP_se_null=300, SP_se_pcm2, SP_se_pcm1,
			      SP_se_ulaw, SP_se_alaw, SP_se_pculaw, SP_se_raw};
enum SP_waveform_comp   { SP_wc_null=400, SP_wc_shorten, SP_wc_wavpack,
			      SP_wc_shortpack, SP_wc_none };
enum SP_data_format     { SP_df_null=500, SP_df_raw, SP_df_array };

typedef struct waveform_t {
    FILE *sp_fp;
    FOB *sp_fob;
    int samples_read;
    int samples_written;
    SP_CHECKSUM checksum;    /* cumulative checksum of read/written data */
    long header_data_size;
    int read_premature_eof;
    int failed_checksum;
    int waveform_setup;
    int file_data_buffer_len;
    void *file_data_buffer;
    int converted_buffer_len;
    void *converted_buffer;
    int interleave_buffer_len;
    void *interleave_buffer;
    int byteswap_buffer_len;
    void *byteswap_buffer;
} SPWAVEFORM;

typedef struct origination_channel{
    int num_origin;
    int *orig_channel;
} ORIGINATION_CHAN;

typedef struct channel_struct {
    SP_INTEGER num_chan;
    SP_INTEGER max_num_chan;
    SP_INTEGER max_chan_add;
    ORIGINATION_CHAN *ochan;
} CHANNELS;

typedef struct spfile_status_t {
    char *external_filename;
    struct header_t *file_header;
    int extra_checksum_verify;
    int is_disk_file;
    int is_temp_file;
    char *temp_filename;

    int user_channel_count, file_channel_count;
    int user_sample_count, file_sample_count;
    int user_sample_rate, file_sample_rate;
    int user_sample_n_bytes, file_sample_n_bytes;
    SP_CHECKSUM file_checksum;     /* original file checksums, set for a file*/
                                   /* that when read in, contained a checksum*/
    int ignore_checksum;           /* TRUE iff a checksum is in the header*/

    enum SP_sample_encoding    user_encoding, file_encoding;
    enum SP_waveform_comp      user_compress, file_compress;
    enum SP_sample_byte_fmt    user_sbf, file_sbf;
    enum SP_sample_byte_fmt    natural_sbf;
    enum SP_data_format        user_data_fmt;

    CHANNELS *channels;

    int write_occured_flag;
    int read_occured_flag;
    int field_set_occured_flag;
    int set_data_mode_occured_flag;
} SPSTATUS;

typedef struct sphere_internal_file_rep_t {
    struct header_t *header;
    struct waveform_t *waveform;
    struct spfile_status_t *status;
} SPIFR;

typedef struct sphere_t {
    enum SP_file_open_mode open_mode;
    SPIFR *read_spifr;
    SPIFR *write_spifr;
} SP_FILE;

#define SPNULL		((SP_FILE *) NULL)
#define SPIFRNULL	((SPIFR *) NULL)
#define CHANNELSNULL	((CHANNELS *)NULL)
#define TRANSLATION_LEN 1024

#endif
