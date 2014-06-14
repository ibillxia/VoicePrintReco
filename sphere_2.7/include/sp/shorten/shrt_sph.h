/***********************************************************************/
/*    File: shrt_sph.h                                                 */
/*    Desc: Sphere to Shorten interface header file                    */
/*    History:                                                         */
/*        Mar 29, 1993:  Creation                                      */
/*        May 5, 1994:   Converted function dec's to ANSI              */
/***********************************************************************/

#ifndef _SHRT_SPH_H_
#define _SHRT_SPH_H_

#if defined(__STDC__) || defined(__GNUC__) || defined(sgi)
#define PROTO(ARGS)	ARGS
#else
#define PROTO(ARGS)	()
#endif

int shorten_set_flag PROTO((char *flag)) ;
void shorten_reset_flags PROTO((void)) ;
void shorten_init PROTO((void)) ;
void shorten_dump_flags PROTO((FILE *fpout)) ;
int shorten_set_ftype PROTO((char *ftype)) ;
int shorten_set_channel_count PROTO((int nchannel)) ;
int shorten_compress PROTO((FOB *fpin, FOB *fpout, char *status)) ;
int shorten_uncompress PROTO((FOB *fpin, FOB *fpout, char *status)) ;

int alaw2linear PROTO((unsigned char alawbyte));
unsigned char linear2alaw PROTO((int sample));

#endif
