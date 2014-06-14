/*  File: sphere.c
 *
 *  Description:
 *  This is the main sphere include file.  It should be included in any modules 
 *  that use the sphere libraries.
 *
 *  Revision History:
 *     - Created: November 1993
 *
 *     - May 5, 1994: Added the redefinition for the sp_read_data and 
 *       sp_write_data so as to properly handle old function argument syntax
 */

#ifndef _SPHERE_H_
#define _SPHERE_H_

#ifdef __cplusplus
extern "C" {
#endif

#define MTRF_ON
#include <util/utillib.h>

#include <sp/header.h>
#include <sp/sysparam.h>
#include <sp/spchksum.h>
#include <sp/spfile.h>
#include <sp/sysfunc.h>
#include <sp/sp.h>

#include <sp/shorten/shrt_sph.h>
#include <sp/wavpack/wavpack.h>
#include <sp/version.h>

/**************************************************************/
/**** Uncomment this line if you'd like to use the pre_2.2 ****/
/**** argument lists for sp_read_data and sp_write_data    ****/
/* #define SPHERE_PRE_2_2 */

/********************************************************************/
/****  IF you intend to define SPHERE_PRE_2_2, so that all you   ****/
/****  existing applications can use the old SPHERE sp_read_data ****/
/****  and sp_write_data function calls, DO SO ABOVE THIS POINT! ****/
#ifdef SPHERE_LIBRARY_CODE
#ifdef SPHERE_PRE_2_2
#undef SPHERE_PRE_2_2
#endif
#endif

/*  If the source file uses the PRE SPHERE 2.2 syntax, redefine the */
/*  data functions to only have 3 arguments.                        */
#ifdef SPHERE_PRE_2_2
#define sp_read_data(_b,_snb,_ns,_sp) sp_mc_read_data(_b,_ns,_sp)
#define sp_write_data(_b,_snb,_ns,_sp) sp_mc_write_data(_b,_ns,_sp)
#else
#define sp_read_data(_b,_ns,_sp) sp_mc_read_data(_b,_ns,_sp)
#define sp_write_data(_b,_ns,_sp) sp_mc_write_data(_b,_ns,_sp)
#endif

/*** define the output filepointer for all diagnostic messages from the ***/
/*** library                                                            ***/
#define spfp stderr


#ifdef __cplusplus
}
#endif

#endif

