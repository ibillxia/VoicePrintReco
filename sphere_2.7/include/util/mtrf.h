
#ifndef MTRF_HEADER
#define MTRF_HEADER

/* Header file: mtfr.h */
#if defined(__STDC__) || defined(__GNUC__) || defined(sgi)
#define PROTO(ARGS)	ARGS
#else
#define PROTO(ARGS)	()
#endif

char *mtrf_malloc PROTO((int bytes)) ;
char *mtrf_realloc PROTO((char *ptr, int bytes)) ;
int mtrf_free PROTO((char *p)) ;
char *mtrf_strdup PROTO((char *p)) ;
void mtrf_set_dealloc PROTO((int n)) ;
void mtrf_set_verbose PROTO((int n)) ;
int mtrf_get_dealloc PROTO((void)) ;
int mtrf_get_verbose PROTO((void)) ;

#ifdef REDEFINE_ALLOCATION
#ifdef malloc
#       undef malloc
#endif
#define malloc(_a)          mtrf_malloc((_a))

#ifdef realloc
#       undef realloc
#endif
#define realloc(_a,_b)      mtrf_realloc ((_a), (_b))

#ifdef free
#       undef free
#endif
#define free(_a)            mtrf_free ((_a))

#ifdef strdup
#       undef strdup
#endif
#define strdup(_a)          mtrf_strdup ((_a))
#endif




#endif
