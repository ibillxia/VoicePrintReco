#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>

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
 
#ifdef fputc
#       undef fputc
#endif
#define fputc(a,b)               fob_putc((a),(b))

/**** End of FOB additions ****/

#ifndef TRUE
#define		FALSE		0
#define		TRUE		1
#endif

#define		COMPRESS	1
#define		UNCOMPRESS	2
#define		INFO		3
#define		SWAP_BYTES	1
#define 	ALL_ONES	0xffffffff

static short log2s[] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024,
2048, 4096, 8192, 16384, 32768};

/*keep*/
static char ieee_order = -1; /*-1 means not set yet*/

static char *SccsId = "@(#) %W% %D% MIT/LCS/SLS";

int write_shortpacked_data(short int *, int, FOB *);
int read_shortpacked_data(short int *, int, FOB *);
void write_shortpack_reading_from_file(FOB *, FOB *, int);
void read_shortpack_writing_to_file(FOB *, FOB *, int);
int find_bits_needed(short int );
int find_next_chunk_to_compress(short int *, int, int , int *, int );
int better_to_chop_here_vs_using_more_bits(int, int , int );
int better_to_chop_here_and_use_fewer_bits_for_future(int, int , short int *, short int *, int, int , int , int );
void pack_and_write_chunk_to_file(short int *, int , int, int, FOB *, int );
int read_shortpack_chunk_from_file(FOB *, short int *, int , int, int );
int pack_short_array_into_buffer(short int *, int , int, int , int , short int *);
int unpack_short_array_into_buffer(short int *, int , int , int, int , short int *, int);

/***************************************************************************

	shortpack_io.c

	Peter Daly			pdaly@goldilocks.lcs.mit.edu
	Spoken Language Systems Group
	MIT Laboratory for Computer Science

	Creation Date: 	12-JUL-1990

	This program compresses utterance files made up of short integers.  
	Refer to the manpage for more information.
	This segment comprises the "guts" of the shortpack algorithim.

	Revision History:

	08-OCT-1990	Modified to not compress files which have an odd 
	                number of bytes, since they are obviously not made
		        up of short integers (pdaly).

	28-AUG-1991	Rewrote many routines to make implementation clearer
	                (mp)

	Compression algorithim by Mike Phillips, MIT/LCS/SLS

	Copyright (C) 1990, 1991	Massachusetts Institute of Technology
					All Rights Reserved

**************************************************************************/


/*  

/*ok, here's how shortpack works:

The idea is to pack short integers into as few bits as required for
sequences of speech samples.  For each sequence, a 16 bit header is
written that says "here comes n words using m bits per word".  The
compression code needs to decide where to break up the sequences to
optimize the tradeoff between transmitting fewer bits for the sequence
vs the cost of transmitting another header for another sequence break.
The present algorithm does an OK job, but if you went to more work in
the compression algorithm, you could probably save a percent or two of
the file size.

If you just want to read and write shortpacked data from your
programs, use the following two functions:

read_shortpacked_data(array, num_elements, fp) - use this for reading
  shortpacked data into an array (works like fread())
write_shortpacked_data(array, num_elements, fp) - use this for writing
  shortpacked data from an array (works like fwrite())

If you want to do file-to-file compression (like the shortpack utility
does), use write_shortpack_reading_from_file() and
read_shortpack_writing_to_file()

These routines use a circular buffer to hold data between input and
output of the shortpacked data.

The file-to-file and file-to-memory routines use the same low level
routines (so the low level routines support the use of circular
buffers (they use a mask before indexing the buffer array - the
memory-to-file routines just use ALL_ONES as a mask))


All of these routines should work with machines with either byte order
(the routines load and dump shortpack file format data to and from the
machines short representation).

If you have any problems, let me know - mike (phillips@goldilocks.lcs.mit.edu)

*/
#define MAX_HEADER_FIELD 100
#define MAX_SEQUENCE_SIZE 255

/*this goes through the input array, looks for chunks to pack into so
number of bit, compresses those into a temporary array, and writes a
header followed by the chunk of data*/



int write_shortpacked_data(short int *array, int num_elements, FOB *fp)
{ int i,j,k;
  int input_pointer;
  int num_in_chunk;
  int bits;
  input_pointer = 0;

  while(input_pointer < num_elements)
  { num_in_chunk = find_next_chunk_to_compress(array, input_pointer, 
					       num_elements, &bits, ALL_ONES);
    pack_and_write_chunk_to_file(array, input_pointer, num_in_chunk,
				 bits, fp, ALL_ONES);
    input_pointer += num_in_chunk;
  }
  return input_pointer;
}

int read_shortpacked_data(short int *array, int num_elements, FOB *fp)
{ int i,j,k;
  int input_pointer;
  int num_in_chunk;

  input_pointer = 0;

  while(input_pointer < num_elements)
  { num_in_chunk =  read_shortpack_chunk_from_file(fp, array, input_pointer,
						   num_elements, ALL_ONES);
    input_pointer += num_in_chunk;
  }
  return input_pointer;
}

#define CBUF_SIZE 512
#define CBUF_MASK 511

void write_shortpack_reading_from_file(FOB *ifp, FOB *ofp, int swap)
{ int i,j,k;
  short cbuf[CBUF_SIZE];
  int next_write,next_read; /*pointers_to_cbuf*/
  int num_read;
  short test_short = 1;
  int how_much_to_write;
  char *char_ptr;
  char temp_char;
  char temp_char2;
  int num_to_read;
  int num_in_chunk;
  int bits;

  if(ieee_order == -1) /*set this if not set already*/
  { ieee_order = (sp_htons(test_short) == test_short);
  }

  next_read = next_write = 0;
  while(1)
  { /*first read data from file into cbuf, swapping bytes if needed*/
    if(next_read <= next_write)
    { how_much_to_write = CBUF_SIZE - next_write;
      num_read = fread(&(cbuf[next_write]), sizeof(short), how_much_to_write,
		       ifp);
      if((num_read == 0) && (next_read == next_write)) break;

/*      printf("num_read1 %d\n", num_read);*/
      if(!(swap ^ ieee_order))
      { char_ptr = (char*)&(cbuf[next_write]);
	for(i=0;i<num_read;i++)
	{ temp_char = *char_ptr;
	  temp_char2 = *(char_ptr+1);
	  *(char_ptr++) = temp_char2;
	  *(char_ptr++) = temp_char;
	}
      }
      if(num_read < how_much_to_write)
      { next_write += num_read;
	
      }
      else
      { next_write = 0;
      }
    }
    if(next_read > next_write)
    { how_much_to_write = next_read - next_write;
      num_read = fread(&(cbuf[next_write]), sizeof(short), how_much_to_write, 
		       ifp);
/*      printf("num_read2 %d\n", num_read);*/
      if(!(swap ^ ieee_order))
      { char_ptr = (char*)&(cbuf[next_write]);
	for(i=0;i<num_read;i++)
	{ temp_char = *char_ptr;
	  temp_char2 = *(char_ptr+1);
	  *(char_ptr++) = temp_char2;
	  *(char_ptr++) = temp_char;
	}
      }
      next_write += num_read;
    }
    /*now compress and write next chunk*/
    if(next_write == next_read)
    { num_to_read = CBUF_SIZE;
    }
    else
    { if(next_write > next_read)
      { num_to_read = next_write - next_read;
      }
      else
      { num_to_read = CBUF_SIZE - (next_read - next_write);
      }
    }
    num_in_chunk = find_next_chunk_to_compress(cbuf, next_read, 
					       next_read + num_to_read, &bits,
					       CBUF_MASK);
    pack_and_write_chunk_to_file(cbuf, next_read, num_in_chunk, bits, ofp,
				 CBUF_MASK);
    next_read += num_in_chunk;
    next_read &= CBUF_MASK;
  }
}

void read_shortpack_writing_to_file(FOB *ifp, FOB *ofp, int swap)
{ int i,j,k;
  short cbuf[CBUF_SIZE];
  int next_write,next_read; /*pointers_to_cbuf*/
  short test_short = 1;
  int how_much_to_write;
  int num_in_chunk;
  char *char_ptr;
  char temp_char;
  char temp_char2;
  int num_written;
  int room_in_cbuf;
  if(ieee_order == -1) /*set this if not set already*/
  { ieee_order = (sp_htons(test_short) == test_short);
  }

  next_read = next_write = 0;

  while(1)
  { if(next_write == next_read)
    { room_in_cbuf = CBUF_SIZE;
    }
    else
    { if(next_write > next_read)
      { room_in_cbuf = next_write - next_read;
      }
      else
      { room_in_cbuf = CBUF_SIZE - (next_read - next_write);
      }
    }

    num_in_chunk =  read_shortpack_chunk_from_file(ifp, cbuf, next_write, 
						   next_write + room_in_cbuf, 
						   CBUF_MASK);
    if(num_in_chunk < 0) break;

    next_write += num_in_chunk;
    next_write &= CBUF_MASK;

    /*write data from cbuf to file, swapping bytes if needed*/
    if(next_read >= next_write)
    { how_much_to_write = CBUF_SIZE - next_read;
      if(!(swap ^ ieee_order))
      { char_ptr = (char*)&(cbuf[next_read]);
	for(i=0;i<how_much_to_write;i++)
	{ temp_char = *char_ptr;
	  temp_char2 = *(char_ptr+1);
	  *(char_ptr++) = temp_char2;
	  *(char_ptr++) = temp_char;
	}
      }
      num_written = fwrite(&(cbuf[next_read]), sizeof(short), 
			   how_much_to_write, ofp);
      if(num_written < how_much_to_write)
      { fprintf(stderr,"read_shortpack_writing_to_file: HAD_TROUBLE_WRITING!!\n");
	longjmp(exitenv, -1);
/*	exit (-1); */
      }
      else
      { next_read = 0;
      }
    }
    if(next_read < next_write)
    { how_much_to_write = next_write - next_read;
      if(!(swap ^ ieee_order))
      { char_ptr = (char*)&(cbuf[next_read]);
	for(i=0;i<how_much_to_write;i++)
	{ temp_char = *char_ptr;
	  temp_char2 = *(char_ptr+1);
	  *(char_ptr++) = temp_char2;
	  *(char_ptr++) = temp_char;
	}
      }
      num_written = fwrite(&(cbuf[next_read]), sizeof(short), 
			   how_much_to_write, ofp);
      if(num_written < how_much_to_write)
      { fprintf(stderr,"read_shortpack_writing_to_file: HAD_TROUBLE_WRITING!!\n");
	longjmp(exitenv,-1);
/*	exit (-1); */
      }
      next_read += how_much_to_write;
    }
  }
}

/*
 * find_bits_needed(): deterimine the minimum number of bits needed to to 
 represent a number.
 */

int
find_bits_needed(short int num)
{
  int i;

  num = abs(num);
  for (i = 14; i >= 0; i--)
    if ((num & log2s[i]) != 0)
      return (i + 1);

  return (0);
}

/*
 * find_next_chunk_to_compress: (compression) look ahead in the uncompressed 
 data to determine how many bits 
 will be needed to represent the next chunk of data.
 */
int find_next_chunk_to_compress(short int *buffer, int start, int num_elements, int *pbits, int mask)
{ int i,j,k;
  int end;
  short abs_element;
  int bits_for_element;
  int bits_for_chunk;
  short bits_for_element_cache[MAX_SEQUENCE_SIZE]; /*so we don't have to recompute each time*/
  int num;
  int ind;


  if((num_elements - start) > MAX_SEQUENCE_SIZE)
  { end = start + MAX_SEQUENCE_SIZE;
  }
  else
  { end = num_elements;
  }
  num = end-start;
  for(i=0;i<num;i++) bits_for_element_cache[i] = -1;

  bits_for_chunk = 0;

  for(i=start;i<end;i++)
  { ind = i-start;
    if(bits_for_element_cache[ind] < 0)
    { abs_element = buffer[i & mask];
      if(abs_element < 0) abs_element *= -1;

      bits_for_element_cache[ind] = find_bits_needed(abs_element);
    }
    bits_for_element = bits_for_element_cache[ind];

/*    printf("i %d mask %d abs %d\n", i, mask, abs_element);*/

    if(bits_for_element > bits_for_chunk)
    { if(   (i!= start) 
	 && better_to_chop_here_vs_using_more_bits(bits_for_element, 
						   bits_for_chunk, (i-start)))
      { *pbits = bits_for_chunk;
/*	printf("chopping upscale bits %d inchunk %d i %d\n", bits_for_element, bits_for_chunk, i);*/
	return (i-start);
      }
      else
      { bits_for_chunk = bits_for_element;
      }
    }
    else
    if(bits_for_element < bits_for_chunk)
    { if(better_to_chop_here_and_use_fewer_bits_for_future(bits_for_element, 
							   bits_for_chunk, buffer, 
							   bits_for_element_cache, start, 
							   i, end, mask))
      { *pbits = bits_for_chunk;
/*	printf("chopping downscale bits %d inchunk %d i %d\n", bits_for_element, bits_for_chunk, i);*/
	return (i-start);
      }
    }
  }
/*  printf("no chopping inchunk %d i %d\n", bits_for_chunk, i);*/

  *pbits = bits_for_chunk;
  return (i-start);
}


/*this figures out how many shorts will be needed to represent output if we 
  chop now vs how many will be needed if we increase the bits for the chunk
  from bits_for_chunk to bits_for_element
*/

int better_to_chop_here_vs_using_more_bits(int bits_for_element, int bits_for_chunk, int num_in_chunk)
{ int els_so_far;
  int new_els;

  /* if we were to stop here and section off now.. */

  els_so_far = ((bits_for_chunk * num_in_chunk) % 16 == 0)
    ? bits_for_chunk * num_in_chunk / 16 : bits_for_chunk * num_in_chunk / 16 + 1;

  els_so_far++; /*add one for the header*/

  /* or to upgrade and continue? */

  new_els = ((bits_for_element * num_in_chunk) % 16 == 0)
    ? bits_for_element * num_in_chunk / 16 : bits_for_element * num_in_chunk / 16 + 1;

  if (els_so_far < new_els)
  { return 1;
  }
  else
  { return (0);
  }
}

/*chop if number of bits saved for future is more than the header size.  
  Check though to see if it will want to use less bits for for before these
  bits are saved (for steadily decreasing amp for example)
*/

int better_to_chop_here_and_use_fewer_bits_for_future(int bits_for_element, int bits_for_chunk, short int *buffer, short int *bits_for_element_cache, int cache_start, int start, int end, int mask)
{ int bits_saved_per_element;
  register int i, j, k;
  int bits;
  short abs_element;
  int max_bits;
  int real_end;
  int ind;

  max_bits = bits_for_element;

  if(end > start+16)
  { real_end = start+16;
  }
  else
  { real_end = end;
  }

  for(i=start+1;i<real_end;i++)
  { ind = i-cache_start;
    if(bits_for_element_cache[ind] < 0)
    { abs_element = buffer[i & mask];
      if(abs_element < 0) abs_element *= -1;

      bits_for_element_cache[ind] = find_bits_needed(abs_element);
    }
    bits = bits_for_element_cache[ind];

    if(bits > max_bits)
    { max_bits = bits;
    }

    bits_saved_per_element = (bits_for_chunk - max_bits);

    if(bits_saved_per_element <= 0) break; /*no hope for saving any bits*/

    if((bits_saved_per_element * (i-start)) > 16) /*if the total bits saved is more than header*/
    { /*check if we are going to reduce bits in the near future 
	(before the current savings is realized)*/
      for(j=start+1;j<=i;j++)
      { ind = j-cache_start;
	if(bits_for_element_cache[ind] < 0)
	{ abs_element = buffer[j & mask];
	  if(abs_element < 0) abs_element *= -1;

	  bits_for_element_cache[ind] = find_bits_needed(abs_element);
	}
	bits = bits_for_element_cache[ind];

	if(better_to_chop_here_and_use_fewer_bits_for_future(bits, max_bits,
							     buffer, bits_for_element_cache, 
							 cache_start, 
							     j, end, mask))
	{ /*going to chop in the future anyway, so don't chop here*/
	  return 0;
	}
      }
      return 1;
    }
  }

  return 0;
}


/*
 * write_chunk_to_file(): (compression) writes "shortpacked" data to a file.
 */

void pack_and_write_chunk_to_file(short int *buffer, int start, int num, int bits, FOB *fp, int mask)
        
           
          
              
          /*so the buffer can be a circular buffer 
	    (if ALL_ONES, just use linear buffer)*/
{ int i,j,k;
  char *char_ptr;
  char temp_char;
  char temp_char2;
  short compressed_data[MAX_SEQUENCE_SIZE];
  int els;
  short test_short = 1;

/*  printf("paw start %d num %d mask %d\n", start, num, mask);*/
  els = pack_short_array_into_buffer(buffer, start, num, bits, mask, 
				     compressed_data);


  fputc((unsigned char) num, fp);
  fputc((unsigned char) bits, fp);

  if(ieee_order == -1) /*set this if not set already*/
  { ieee_order = (sp_htons(test_short) == test_short);
  }

  /* if the machine is NOT IEEE order, swap bytes of data before writing.*/
  if (!ieee_order)
  { char_ptr = (char*)compressed_data;
    for(i=0;i<els;i++)
    { temp_char = *char_ptr;
      temp_char2 = *(char_ptr+1);
      *(char_ptr++) = temp_char2;
      *(char_ptr++) = temp_char;
    }
  }
  /* write the "shortpacked" data to file */
  fwrite(compressed_data, sizeof(short), els, fp);
}

/*
 * read_shortpack_from_file(): (uncompression) read "shortpacked" data from a 
 * file
 */
int
read_shortpack_chunk_from_file(FOB *fp, short int *buffer, int start, int max_elements, int mask)
        
              
                        
          /*so the buffer can be a circular buffer (if ALL_ONES, 
	    just use linear buffer)*/
{ int i,j,k;
  int num_read;
  int num, bits_used, els;
  char *char_ptr;
  char temp_char;
  char temp_char2;
  int num_unpacked;
  short compressed_data[MAX_SEQUENCE_SIZE];
  short test_short = 1;

/*   printf("read chunk file position %d\n",fob_ftell(fp)); */
  num = fgetc(fp);
  bits_used = fgetc(fp);
/*  printf("   num = %d,   bits_used = %d\n",num,bits_used);
  printf("   num = %x,   bits_used = %x\n",num,bits_used); */

  if (num < 0 || bits_used < 0){
      return (-1);
  }

  els = (num * (bits_used + 1)) / 16.0;
  if ((num * (bits_used + 1)) % 16 != 0)
    els++;

/*   printf("   els = %d\n",els);  */
  num_read = fread(compressed_data, sizeof(short), els, fp);
/*  printf("num_read %d != els %d\n", num_read,els);
  printf("read_shortpack_chunk_from_file: it's ok\n");
  printf("       num_read %d eld %d! num %d bits_used %d start %d max %d\n",
	   num_read, els, num, bits_used, start, max_elements); */
  if(num_read != els)
  { fprintf(stderr,"read_shortpack_chunk_from_file: HEY! something is seriously wrong!  maybe you\n");
    fprintf(stderr,"       are trying to read a non-shortpacked file with a shortpack reading\n");
    fprintf(stderr,"       routine num_read %d eld %d! num %d bits_used %d start %d max %d\n",
	   num_read, els, num, bits_used, start, max_elements);
    els = num_read;
    longjmp(exitenv,-1);
/*    exit(-1); */
  }
  if(ieee_order == -1) /*set this if not set already*/
  { ieee_order = (sp_htons(test_short) == test_short);
  }

  /*
   * If this machinee does not follow IEEE byte swapping, swap the input bytes here
   */
  if (!ieee_order)
  { char_ptr = (char*)compressed_data;
    for(i=0;i<els;i++)
    { temp_char = *char_ptr;
      temp_char2 = *(char_ptr+1);
      *(char_ptr++) = temp_char2;
      *(char_ptr++) = temp_char;
    }
  }
  num_unpacked = unpack_short_array_into_buffer(buffer, start, max_elements,
						bits_used, mask, 
						compressed_data, num);
  
  return num_unpacked;
}

int pack_short_array_into_buffer(short int *buffer, int start, int num, int bits, int mask, short int *compressed_data)
{ int i,j,k;
  int bit_mark;
  int buffer_ind;
  int els;
  short v;

/*  printf("psa:start %d num %d mask %d\n", start, num, mask);*/
  els = (num * (bits + 1)) / 16.0;
  if ((num * (bits + 1)) % 16 != 0)
    els++;
/*  printf("psa:els %d\n", els);*/
  for(j=0;j<els;j++) compressed_data[j] = 0;

  j = 0;
  bit_mark = 0;

  for (i = 0; i < num; i++)
  { buffer_ind = (start+i) & mask;
    v = buffer[buffer_ind];
    /* set the sign here */
    bit_mark++;
    
    if (v < 0)
    { compressed_data[j] |= log2s[16 - bit_mark];
      v = -v;
    }
    if (bit_mark == 16)
    { bit_mark = 0;
      j++;
    }
    for (k = bits - 1; k >= 0; k--)
    {
      bit_mark++;
      if ((v & log2s[k]) != 0)
	compressed_data[j] |= log2s[16 - bit_mark];
      if (bit_mark == 16)
      {
	bit_mark = 0;
	j++;
      }
    }
  }
  if(bit_mark == 0)return j;
  else return (j+1);
}


int unpack_short_array_into_buffer(short int *buffer, int start, int max_elements, int bits, int mask, short int *compressed_data, int num)
{
  int i, k;
  char negative;
  register short *log2s_ptr;
  register short *log2s_stop_ptr;
  short *log2s_start_ptr;
  register short temp_out;
  register short *logs2_kptr;
  int buffer_ind;
  register short temp_short;
  short *buf_ptr;

  buf_ptr = compressed_data;

  temp_short = *buf_ptr++;
  log2s_ptr = log2s_start_ptr = &(log2s[15]);
  log2s_stop_ptr = log2s;

  for (i = 0; i < num; i++)
  {
    if((start + i) >= max_elements)
    { printf("unpack_short_array_into_buffer:HEY! something seems wrong - ran out of space in buffer!! (just truncating)\n");
      return i;
    }
    buffer_ind = (start+i) & mask;
    temp_out = 0;

    negative = ((temp_short & *(log2s_ptr--)) != 0);
    if (log2s_ptr < log2s_stop_ptr)
    {
      log2s_ptr = log2s_start_ptr;
      temp_short = *(buf_ptr++);

    }
    logs2_kptr = &(log2s[bits - 1]);

    for (k = bits + 1; (--k) > 0;)
    {
      if ((temp_short & *(log2s_ptr--)) != 0)
	temp_out |= *logs2_kptr;
      logs2_kptr--;

      if (log2s_ptr < log2s_stop_ptr)
      {
	log2s_ptr = log2s_start_ptr;
	temp_short = *(buf_ptr++);
      }
    }

    if (negative)
      if (temp_out != 0)
	buffer[buffer_ind] = -temp_out;
      else
	buffer[buffer_ind] = 32768;
    else
      buffer[buffer_ind] = temp_out;

  }
  return num;
}







