/* 
 *   The function definitions for Henry Spencer's getopt code
 */
#if defined(__STDC__) || defined(__GNUC__) || defined(sgi)
#define PROTO(ARGS)	ARGS
#else
#define PROTO(ARGS)	()
#endif

void hs_resetopt PROTO((void)) ;
int hs_getopt PROTO((register int argc, register char **argv, char *optstring)) ;
extern int hs_optind;
extern char * hs_optarg;
