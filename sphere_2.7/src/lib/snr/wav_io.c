
#include <snr/snrlib.h>
#include <snr/linear.c>

#define wav_io_C_VERSION "V1.1"

static FILE *loc_fp; 
static int loc_buff_len;
static SPFILE_CHARS *loc_sp_char;
static unsigned char *chr_buff;
static short *sht_buff;
static double loc_dc_bias;

void clear_waveform_read(void)
{
  loc_fp = (FILE *)0; 
  loc_buff_len = 0;
  loc_sp_char = (SPFILE_CHARS *)8;
  if (chr_buff!=(unsigned char *)0) free(chr_buff);
  if (sht_buff!=(short *)0) free(sht_buff);
  loc_dc_bias = 0;
}

void init_waveform_read(FILE *fp, int buff_len, SPFILE_CHARS *sp_char)
{
  loc_fp = fp; 
  loc_buff_len = buff_len;
  loc_sp_char = sp_char;
  loc_dc_bias = sp_char->dc_bias;

  rewind(loc_fp);
  fseek(loc_fp,1024,0);  /* skip header */

  if (strcmp(sp_char->sample_coding,"mu-law") == 0){
    alloc_singarr(chr_buff,(int)(buff_len*sp_char->channel_count),unsigned char);
    sht_buff=(short *)0; 
 }
  else {
    alloc_singarr(sht_buff,(int)(buff_len*sp_char->channel_count),short);
    chr_buff=(unsigned char *)0;
  }
}

int wave_read_1channel(char *out_arr, int channel)
{
    int red, i;

    if (strcmp(loc_sp_char->sample_coding,"mu-law") == 0)
        red = fread(chr_buff,1,loc_sp_char->channel_count*loc_buff_len,
                    loc_fp);
    else
        red = fread((char *)sht_buff,2,loc_sp_char->channel_count*loc_buff_len,
                    loc_fp);

    if (strcmp(loc_sp_char->sample_coding,"mu-law") == 0){
        unsigned char *t_buff;
        short *sam;

        red /= loc_sp_char->channel_count;
        sam = (short *)out_arr;
        t_buff = chr_buff+channel;
        for (i=0; i< red; i++){
	  *sam = linear[*t_buff] - loc_dc_bias;
           sam++;
           t_buff+=loc_sp_char->channel_count;
        }
    }
    else {
        short *t_buff;
        short *sam;

        sam = (short *)out_arr;
        t_buff = sht_buff+channel;
        for (i=0; i< red; i++){
	  *sam = *t_buff;
           sam++;
           t_buff+=loc_sp_char->channel_count;
        }
        if (loc_sp_char->swap_bytes){
            char *c, t;
            sam = (short *)out_arr;
            for (i=0; i< red; i++){
                c = (char *)sam;
                t = *(c+1); *(c+1) = *c; *c = t;
                sam++;
	    }
        }
        for (i=0,sam = (short *)out_arr; i< red; i++)
	  *(sam++) -= loc_dc_bias;
    }
    return(red);
}

int wave_read_2channel(char *out0, char *out1)
{
    int red, i;

    if (strcmp(loc_sp_char->sample_coding,"mu-law") == 0)
        red = fread(chr_buff,1,loc_sp_char->channel_count*loc_buff_len,
                    loc_fp);
    else
        red = fread(sht_buff,2,loc_sp_char->channel_count*loc_buff_len,
                    loc_fp);

    if (strcmp(loc_sp_char->sample_coding,"mu-law") == 0){
        unsigned char *t_buff;
        short *sam0, *sam1;

        red /= loc_sp_char->channel_count;
        sam0 = (short *)out0;
        sam1 = (short *)out1;
        t_buff = chr_buff;
        for (i=0; i< red; i++){
           *(sam0++) = linear[*t_buff];
           *(sam1++) = linear[*(t_buff+1)];
           t_buff+=loc_sp_char->channel_count;
        }
    }
    else {
        fprintf(stderr,"Error: unable to read 2 channel PCM files\n");
        exit(-1);
    }
    return(red);
}


void shift_short_up(short int *to, short int *from, int num)
{
    int i;
    for (i=0; i<num;i++)
        *(to++) = *(from++);
}

