/***********************************************************************/
/*    File: shpk_sph.h                                                 */
/*    Desc: Sphere to Shorten interface header file                    */
/*    History:                                                         */
/*        Mar 29, 1993:  Creation                                      */
/***********************************************************************/

#ifndef _SHPK_SPH_
#define _SHPK_SPH_


#if defined(__STDC__) || defined(__GNUC__) || defined(sgi)
#define PROTO(ARGS)	ARGS
#else
#define PROTO(ARGS)	()
#endif

int shortpack_compress PROTO((FOB *fpin, FOB *fpout, char *status)) ;
int shortpack_uncompress PROTO((FOB *fpin, FOB *fpout, struct header_t *header)) ;

#endif
