/*CopyRight 1992 Massachusetts Institute of Technology*/
 
#include <stdio.h>

/**** Added byt JGF  ****/
#if defined(NARCH_SUN) || defined(NARCH_HP)	/* cth */
#include <sys/types.h>
#include <netinet/in.h>
#endif

#include <sp/sphere.h>
#include <util/min.h>
#include <setjmp.h>

extern jmp_buf	exitenv;

#ifdef fread
#       undef fread
#endif
#define fread(a,b,c,d)          fob_fread((void *)(a),(b),(c),(d))
 
#ifdef fwrite
#       undef fwrite
#endif
#define fwrite(a,b,c,d)         fob_fwrite((void *)(a),(b),(c),(d))
 
#ifdef putc
#       undef putc
#endif
#define putc(a,b)               fob_putc((a),(b))

#ifdef getc
#       undef getc
#endif
#define getc(a)                 fob_getc((a))

#ifdef fgetc
#       undef fgetc
#endif
#define fgetc(a)                fob_getc((a))

#ifdef getw
#       undef getw
#endif
#define getw(a)                 fob_getw((a))

/**** End of FOB additions ****/

typedef struct header_t header_t;

#define MAX_HEADER_FIELD 100

/*this is an example of how to use the shortpack functions in conjunction with
NIST's shpere header reading code to read and write shortpacked .wav data.

The sample_byte_format (which used to be either "01" or "10") now has an additional
option - "shortpack-v0" (the v0 means version 0).

This program reads a .wav file (shortpacked or not), then writes it with the specified
sample_byte_format.  It was just meant as an example of the use of the shortpack routines,
but you can use this program to compress (or uncompress) existing .wav files.
*/


/*uses htons to check byte ordering of machine*/

/*if check_checksum = 1 the checksum will be checked if = 0 it won't*/
read_wav_data(FOB *fp, short int **pwaveform, header_t *header, int check_checksum)
{ int i;
  char sample_byte_format[MAX_HEADER_FIELD];
  short *waveform;
  int len;
  int string_len;
  short test_short = 1;
  char *char_ptr;
  char temp_char1, temp_char2;
  int checksum;

  string_len = MAX_HEADER_FIELD;
  sp_get_data(header,"sample_byte_format",sample_byte_format, &string_len);
  sample_byte_format[string_len] = '\0';

  string_len = sizeof(int);
  sp_get_data(header,"sample_count",(char *)&len,&string_len);

/*  printf("read byte_format %s ", sample_byte_format);  */
  waveform = *pwaveform = (short*) calloc(len, sizeof(short));

/*  if((strcmp(sample_byte_format, "01") == 0) || (strcmp(sample_byte_format, "10") == 0))*/
  if (0)
  { fread(waveform, sizeof(short), len, fp);
    
    /*if byte ordering of file was different from machine, swapem*/

    
    if ((sp_htons(test_short) == test_short && strcmp("01", sample_byte_format) == 0) ||
	(sp_htons(test_short) != test_short && strcmp("10", sample_byte_format) == 0))
    { printf("swapping");
      char_ptr = (char*)waveform;
      for(i=0;i<len;i++)
      { temp_char1 = *char_ptr;
	temp_char2 = *(char_ptr+1);
	*(char_ptr++) = temp_char2;
	*(char_ptr++) = temp_char1;
      }
    }
  }
  else
  { if (1) /* if(strcmp(sample_byte_format, "shortpack-v0") == 0)*/
    { read_shortpacked_data(waveform, len, fp);  /*reads into machines byte-order*/
/*      printf("reading shortpack");*/
    }
    else
    { fprintf(stderr,"SORRY! don't know about sample_byte_format %s\n", sample_byte_format);
      longjmp(exitenv,-1);
/*      exit(-1); */
    }
  }
  if(check_checksum)
  { string_len = sizeof(int);
    if(sp_get_data(header,"sample_checksum",(char *)&checksum,&string_len) < 0)
    { printf("read_wav_data: HEY! can't read checksum in header!!\n");
    }
    else
    { if(checksum != compute_checksum(waveform, len))
      { printf("read_wav_data: CHECKSUM ERROR: in header %d computed %d\n", checksum, compute_checksum(waveform, len));
      }
    }
  }
  return len;
}

write_wav_data(FOB *fp, short int *waveform, header_t *header)
{ int i;
  char sample_byte_format[MAX_HEADER_FIELD];
  int swap; 
  int string_len;
  int len;
  short test_short = 1;
  char *char_ptr;
  char temp_char1, temp_char2;

  string_len = MAX_HEADER_FIELD;
  sp_get_data(header,"sample_byte_format",sample_byte_format, &string_len);
  sample_byte_format[string_len] = '\0';

  printf("write sample_byte_format %s ", sample_byte_format);
  string_len = sizeof(int);
  sp_get_data(header,"sample_count",(char *)&len,&string_len);

  if((strcmp(sample_byte_format, "01") == 0) || (strcmp(sample_byte_format, "10") == 0))
  {     /*if byte ordering of file was different from machine, swapem*/

    if ((sp_htons(test_short) == test_short && strcmp("01", sample_byte_format) == 0) ||
	(sp_htons(test_short) != test_short && strcmp("10", sample_byte_format) == 0))
    { char_ptr = (char*)waveform;
      printf("swapping ");
      for(i=0;i<len;i++)
      { temp_char1 = *char_ptr;
	temp_char2 = *(char_ptr+1);
	*(char_ptr++) = temp_char2;
	*(char_ptr++) = temp_char1;
      }
      swap = 1;
    }
    else
    { swap = 0;
    }
    fwrite(waveform, sizeof(short), len, fp);
    if(swap) /*remember to swap back!*/
    { char_ptr = (char*)waveform;
      for(i=0;i<len;i++)
      { temp_char1 = *char_ptr;
	temp_char2 = *(char_ptr+1);
	*(char_ptr++) = temp_char2;
	*(char_ptr++) = temp_char1;
      }
    }
  }
  else
  { if(strcmp(sample_byte_format, "shortpack-v0") == 0)
    { printf("writing with shortpack");
      write_shortpacked_data(waveform, len, fp);  /*writes from machines byte-order*/
      
    }
    else
    { fprintf(stderr,"SORRY! don't know about sample_byte_format %s\n", sample_byte_format);
      longjmp(exitenv,-1);
      /* exit(-1); */
    }
  }
  printf("\n");
}


update_checksum_unless_its_already_there(short int *waveform, header_t *header)
{ int field_len;
  int checksum;
  int wav_len;

  field_len = sizeof(int);
  if(sp_get_data(header,"sample_checksum",(char *)&checksum,&field_len) < 0)
  { field_len = sizeof(int);
    if(sp_get_data(header,"sample_count",(char *)&wav_len,&field_len) < 0)
    { fprintf(stderr,"HEY! couldn't read sample count from header!!\n");
      longjmp(exitenv,-1);
      /* exit(-1); */
    }
    checksum = compute_checksum(waveform, wav_len);
    sp_add_field(header,"sample_checksum",T_INTEGER,(void *)&checksum);
  }
}

update_checksum_even_if_its_already_there(short int *waveform, header_t *header)
{ int field_len;
  int checksum;
  int wav_len;

  field_len = sizeof(int);
  if(sp_get_data(header,"sample_count",(char *)&wav_len,&field_len) < 0)
  { fprintf(stderr,"HEY! couldn't read sample count from header!!\n");
    longjmp(exitenv,-1);
    /* exit(-1); */
  }
  checksum = compute_checksum(waveform, wav_len);
  printf("adding checksum %d\n", checksum);
  field_len = sizeof(int);
  if(sp_get_data(header,"sample_checksum",(char *)&checksum,&field_len) < 0)
  { sp_change_field(header,"sample_checksum",T_INTEGER,(void *)&checksum);
  }
  else
  { sp_add_field(header,"sample_checksum",T_INTEGER,(void *)&checksum);
  }
}



compute_checksum(short int *wav, int len)
{ int i;
  short *p;
  short *end;
  int checksum;

  checksum = 0;

  p = wav;
  end = wav + len;

  while (p < end)
  { checksum = (checksum + (*p++)) & 0xffff;
  }

  return checksum;
}

  
  
  
  
    
  
