#include <snr/snrlib.h>

#define GSPFCHAR_C_VERSION "V1.1"

#define NEEDED     1

int read_hdr(SP_FILE *sp, char *id, char *value, int type, int importance)
{ 
    int ret=0.0;
    SP_STRING *chr;

    if (type == T_STRING) {
	if ((ret = sp_h_get_field(sp,id,type,(void *)&chr)) == 0){
	    strcpy((char *)value,(char *)chr);
	    mtrf_free((char *)chr);
	}
    } else 
	ret = sp_h_get_field(sp,id,type,(void *)value);
	
    if (ret > 0){
        if (importance == NEEDED){
            fprintf(stderr,"Error: no %s field when expected\n",id);
            exit(-1);
        }
	*value = '\0';
	/*  fprintf(stderr,"Error: no %s field, returning NULL\n",id);  */
        return(-1);
    }
    return(0);
}

void load_sp_char(SP_FILE *sp, SPFILE_CHARS *sp_char)
{
    read_hdr(sp,"sample_count",(char *)&sp_char->sample_count,T_INTEGER,NEEDED);
    read_hdr(sp,"sample_rate",(char *)&sp_char->sample_rate,T_INTEGER,NEEDED);
    if (read_hdr(sp,"sample_n_bytes",
                 (char *)&sp_char->sample_n_bytes,T_STRING,!NEEDED) == -1)
        sp_char->sample_n_bytes = 2;
    if (read_hdr(sp,"sample_coding",
                 (char *)sp_char->sample_coding,T_STRING,!NEEDED)==-1)
        strcpy(sp_char->sample_coding,"linear");
    if (read_hdr(sp,"channel_count",
                 (char *)&sp_char->channel_count,T_INTEGER,!NEEDED)==-1)
        sp_char->channel_count=1;
    { char inter[50];
      inter[0]='\0';
      if (read_hdr(sp,"channels_interleaved",(char *)inter,
		   T_STRING,!NEEDED) == -1)
          sp_char->channels_interleaved = 0;
      else
          sp_char->channels_interleaved = strcmp(inter,"TRUE");
    }
    sp_char->dc_bias = 0.0;
}

/* print the fields in the structure */
void print_sp_chars(SPFILE_CHARS *sp_char)
{
    printf("Channels:               %ld\n",sp_char->channel_count);
    printf("Sample Count:           %ld\n",sp_char->sample_count);
    printf("Sample Rate:            %ld\n",sp_char->sample_rate);
    printf("Sample Num Bytes:       %ld\n",sp_char->sample_n_bytes);
    printf("Sample Coding:          %s\n",sp_char->sample_coding);
    printf("Channels Interleaved:   %d\n",sp_char->channels_interleaved);
    printf("Swap Bytes (BOOL):      %d\n",sp_char->swap_bytes);
    printf("DC_bias:                %f\n",sp_char->dc_bias);
}


