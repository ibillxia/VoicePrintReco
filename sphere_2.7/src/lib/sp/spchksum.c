/* File: sp_compute_short_checksum.c, Updated 10/31/91.

This function for computing a speech waveform checksum was adapted
from a function contributed by Mike Phillips at M.I.T.  It differs
from his function in that it computes an unsigned (vs. signed) integer
checksum.  It takes as arguments a pointer to an array of shorts
containing the waveform samples (wav) and the number of shorts/samples
in the array (len).  Please note that it works on 16-bit data ONLY and
it is not guaranteed to work on machines with less than 32-bit longs */

/*

Modified by Charles Hemphill at TI to provide a better functional
interface and increased portability.

*/


/* Modifications: Jon Fiscus  June 20, 1993
    This function did not take into account arrays which do not
    have the same byte format orientation as the host machine.  If
    the flag, swap_bytes is true, a byte swap is performed before the
    sample is added to the checksum.

*/

#include <stddef.h>		/* for size_t */
#include <stdio.h>
#define SPHERE_LIBRARY_CODE
#include <sp/sphere.h>

SP_CHECKSUM sp_compute_short_checksum(short int *wav, size_t len,
				      int do_swap)
{ 
  unsigned short *p;
  unsigned short *end;
  unsigned long checksum;
  short sample;
  char *s_ptr, *p_ptr;

  checksum = 0;

  p = (unsigned short *) wav;
  end = p + len;

  if (! do_swap){
      while (p < end) {
	  checksum = (checksum + (*p++)) & 0xffff;
      }
  } else {
      s_ptr = (char *)&sample;
      p_ptr = (char *)p;
      while (p_ptr < (char *)end) {
	  *s_ptr = *(p_ptr+1);
	  *(s_ptr+1) = *p_ptr;
	  checksum = (checksum + sample) & 0xffff;
	  p_ptr += 2;
      }
  }
  return (SP_CHECKSUM) checksum;
}

SP_CHECKSUM sp_compute_char_checksum(char *wav, size_t len)
{ 
  unsigned char *p;
  unsigned char *end;
  unsigned long checksum;

  checksum = 0;

  p = (unsigned char *) wav;
  end = p + len;

  while (p < end) 
      checksum = (checksum + (*p++)) & 0xffff;
  
  return (SP_CHECKSUM) checksum;
}

SP_CHECKSUM sp_add_checksum(SP_CHECKSUM csum1, SP_CHECKSUM csum2)
{
    long checksum;
    checksum = (csum1 + csum2) & 0xffff;
    return((SP_CHECKSUM) checksum);
}

int sp_compute_checksum(SP_FILE *sp, SP_CHECKSUM *comp_chksum){
    char *proc = "sp_compute_checksum";
    void *buff;
    SP_INTEGER cur_pos;

    if (sp == SPNULL)
	return_err(proc,101,101,"Null SPFILE structure");
    if (sp->open_mode != SP_mode_read) 
	return_err(proc,102,102,"File must be opened for read");
    if (! sp->read_spifr->status->is_disk_file)
	return_err(proc,103,103,"File must be a disk file");
    if (sp_error(sp) != 0)
	return_err(proc,104,104,"Sphere file already has an error");

    /* save the current position so we can go back to it */
    if ((cur_pos = sp_tell(sp)) < 0)
	return_err(proc,110,110,
		   rsprintf("sp_tell() failed, returning: %s",
			    get_return_status_message()));
    /* rewind the file */
    if (sp_seek(sp,0,0) != 0)
	return_err(proc,111,111,
		   rsprintf("sp_seek() to sample 0 failed, returning: %s",
			    get_return_status_message()));
    
    /* allocate some memory */
    if ((buff = (void *)sp_data_alloc(sp,4096)) == (void *)0)
	return_err(proc,112,112,
		   rsprintf("can not alloc waveform buffer, returning: %s",
			    get_return_status_message()));

    /* read in the data */
    while (sp_read_data(buff,4096,sp) > 0)
	;

    /* dealloc memory */
    sp_data_free(sp,buff);

    if (sp_error(sp) != 0)
	return_err(proc,120,120,
		   rsprintf("sp_error() indicates an error, returning: %s",
			    get_return_status_message()));

    *comp_chksum = sp->read_spifr->waveform->checksum;

    if (sp_seek(sp,cur_pos,0) != 0)
	return_err(proc,130,130,
		   rsprintf("sp_seek() to return the file to it's initial"
			    "state failed, returning: %s",
			    get_return_status_message()));

    return_success(proc,0,0,"ok");
}
