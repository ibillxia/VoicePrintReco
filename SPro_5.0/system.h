/******************************************************************************/
/*                                                                            */
/*                                system.h                                    */
/*                                                                            */
/*                               SPro Library                                 */
/*                                                                            */
/* Guig                                                             Sep. 2002 */
/* -------------------------------------------------------------------------- */
/*
   $Author: guig $
   $Date: 2010-11-09 16:57:22 +0100 (Tue, 09 Nov 2010) $
   $Revision: 151 $
*/
/*  
   Copyright (C) 1997-2010 Guillaume Gravier (guig@irisa.fr)

   Permission is hereby granted, free of charge, to any person
   obtaining a copy of this software and associated documentation
   files (the "Software"), to deal in the Software without
   restriction, including without limitation the rights to use, copy,
   modify, merge, publish, distribute, sublicense, and/or sell copies
   of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
   BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/

/*
 * System dependent includes and defines.
 */

#ifndef _system_h_
# define _system_h_

# ifdef HAVE_CONFIG_H
#  include <config.h>
# endif

# include <stdio.h>

#define SIZEOF_SHORT sizeof(short int)
#define SIZEOF_LONG sizeof(long int)

/* stdlib stuff */
# if STDC_HEADERS
#  include <stdlib.h>
#  include <stddef.h>
#  include <stdarg.h>
# else
#  if HAVE_STDLIB_H
#   include <stdlib.h>
#  endif
# endif

# if defined STDC_HEADERS || defined _LIBC
#   include <stdlib.h>
# elif defined HAVE_MALLOC_H
#   include <malloc.h>
# endif

# if HAVE_UNISTD_H
#  include <unistd.h>
# endif

/* string stuff */
# if HAVE_STRING_H
#  if !STDC_HEADERS && HAVE_MEMORY_H
#   include <memory.h>
#  endif
#  include <string.h>
# endif

/* mathematics */
//# if HAVE_MATH_H
#  include <math.h>
//# endif

/* limits */
# if HAVE_LIMITS_H
#  include <limits.h>
# endif
/* some systems (at least mine) do not define properly the following limits.... */
# ifndef FLT_MIN
#  define FLT_MIN 1.17549435E-38F
# endif
# ifndef FLT_MAX
#  define FLT_MAX 3.40282347e+38F
# endif
# ifndef DBL_MIN
#  define DBL_MIN 2.2250738585072014E-308
# endif
# ifndef DBL_MAX
#  define DBL_MAX 1.7976931348623157E+308
# endif

# if HAVE_SYS_TYPES_H
#  include <sys/types.h>
# endif

#endif /* _system_h_ */




