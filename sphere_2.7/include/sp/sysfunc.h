/* File: sysfunct.h */

#ifndef _SYSFUNC_H_
#define _SYSFUNC_H_

#ifndef __STDC__
/* C Library functions */

#ifndef NARCH_AIX 
extern char     *index(), *rindex();
#endif
extern double   atof();
extern long     atol(), ftell();
extern int      atoi();
#else
#include <stdlib.h>
#endif

#if defined(__STDC__) || defined(__GNUC__) || defined(sgi)
#define PROTO(ARGS)	ARGS
#else
#define PROTO(ARGS)	()
#endif

/* Support library functions */

/* spinput */    int spx_read_header PROTO((FILE *, int *, int, char **)) ;
/* spinput */    struct field_t **spx_get_field_vector PROTO((int)) ;
/* spinput */    struct field_t *spx_allocate_field PROTO((int, char *,
							  char *v, int )) ;
/* spinput */    struct field_t *spx_allocate_field_str PROTO((int, char *, 
							  char *value, int )) ;
/* spinput */    struct header_t *spx_allocate_header PROTO((int, 
							  struct field_t **)) ;

/* spinput.c */  int spx_copy_field_vector PROTO((struct field_t **, struct field_t **, int )) ;
/* spinput.c */  int spx_deallocate_field PROTO((struct field_t *)) ;
/* spinput.c */  int spx_deallocate_header PROTO((struct header_t *)) ;
/* spinput.c */  int spx_read_header PROTO((FILE *, int *, int, char **)) ;
/* spoutput.c */ int spx_tp PROTO((int )) ;
/* spoutput.c */ int spx_write_header PROTO((struct header_t *, 
       struct fileheader_fixed *, FILE *, int , SP_INTEGER *, SP_INTEGER *)) ;

#endif
