/******************************************************************************
******************************************************************************/

# include <math.h>
# include <stdio.h>
# include <sp/sphere.h>
# include <sp/shpack/shpk_sph.h>

#if defined(NARCH_SUN) || defined(NARCH_HP)	/* cth */
#include <sys/types.h>
#include <netinet/in.h>
#endif


#define		SWAP_BYTES	1

int shortpack_compress(FOB *fpin, FOB *fpout, char *status) {

    return(-1);

}

int shortpack_uncompress(FOB *fpin, FOB *fpout, struct header_t *header)
{
    int waveform_len, write_len;
    short *waveform;
    int swap;
    int filesize;

    waveform_len = read_wav_data(fpin, &waveform, header, 1);
/*    printf(" Read length = %d\n",waveform_len);    */
    if (waveform_len < 0)
	return(-1);

    write_len = fob_fwrite((void *)waveform,2,waveform_len,fpout);
/*    printf(" Write length = %d\n",write_len);*/
    if (write_len != waveform_len)
	return(-1);
    free(waveform);

    return(0);
}

  
  
  
    
  



#ifdef old

    int swap;
    int filesize;
    int pos, num_in_chunk, written;
    short array[1024];
 
    read_shortpack_header(fpin, 0, &filesize, &swap);    /* read the header from input */
    do {
	num_in_chunk =  read_shortpack_chunk_from_file(fpin, array, 0,
						       1024, 0xffffffff);
	printf("  %d read\n",num_in_chunk);
	if (num_in_chunk < 0)
	    return(-1);
	written=fob_fwrite((void *)array,2,num_in_chunk,fpout);
	printf("  %d written\n",num_in_chunk);
	if (written != num_in_chunk)
	    return(-1);
    } while (num_in_chunk > 0);
    return 0;


/*
read_shortpacked_data(array, num_elements, fp)
short *array;
int num_elements;
FILE *fp;
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
 */
/*


    pos=fob_ftell(fpin);
    read_shortpack_header(fpin, 0, &filesize, &swap);    /* read the header from input */
    while (!fob_feof(fpin) && read_shortpack_writing_to_file(fpin, fpout, swap) == 0)
      ;
    return(0);
*/

#endif



/* 
 * read_shortpack_header(): This stuff gets the byte-ordering of the
   compressed file.  Default (unswapped) is IEEE ordering (SPARC,
   Motorola 680X0) Swapped is VAX, DEC RISC, Intel 80X86, etc.  * * arg:
   use in_core rotines if 1, file access if 0
 */

read_shortpack_header(FOB *ifp, int arg, int *pfilesize, int *pswap)
{
  int flags;

  *pfilesize = sp_ntohl(fob_getw(ifp));
  flags = sp_ntohl(fob_getw(ifp));

  *pswap = (flags & SWAP_BYTES);
}

