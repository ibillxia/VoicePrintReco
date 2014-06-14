/******************************************************************************
*                                                                             *
*       Copyright (C) 1992-1995 Tony Robinson                                 *
*                                                                             *
*       See the file LICENSE for conditions on distribution and usage         *
*                                                                             *
******************************************************************************/

#ifndef _SHORTEN_H_
#define _SHORTEN_H_

/* Added by Jon Fiscus */
#ifdef __STDC__
#ifndef HAVE_STDARG_H
#define HAVE_STDARG_H
#endif
#endif

# define MAGIC			"ajkg"
# define FORMAT_VERSION		2
# define BUGFIX_RELEASE         "00"
# define MIN_SUPPORTED_VERSION	1
# define MAX_SUPPORTED_VERSION	2
# define MAX_VERSION		7
# define UNDEFINED_UINT		-1
# define DEFAULT_BLOCK_SIZE	256
# define DEFAULT_V0NMEAN	0
# define DEFAULT_V2NMEAN	4
# define DEFAULT_MAXNLPC	0
# define DEFAULT_NCHAN		1
# define DEFAULT_NSKIP		0
# define DEFAULT_NDISCARD	0
# define NBITPERLONG		32
# define DEFAULT_MINSNR         256
# define DEFAULT_MAXRESNSTR	"32.0"
# define DEFAULT_QUANTERROR	0
# define MINBITRATE		2.5

# define MAX_LPC_ORDER	64
# define CHANSIZE	0
# define ENERGYSIZE	3
# define BITSHIFTSIZE	2
# define NWRAP		3

# define FNSIZE		2
# define FN_DIFF0	0
# define FN_DIFF1	1
# define FN_DIFF2	2
# define FN_DIFF3	3
# define FN_QUIT	4
# define FN_BLOCKSIZE	5
# define FN_BITSHIFT	6
# define FN_QLPC	7
# define FN_ZERO	8

# define ULONGSIZE	2
# define NSKIPSIZE	1
# define LPCQSIZE	2
# define LPCQUANT	5
# define XBYTESIZE	7

# define TYPESIZE	4
# define TYPE_AU1	0
# define TYPE_S8	1
# define TYPE_U8	2
# define TYPE_S16HL	3
# define TYPE_U16HL	4
# define TYPE_S16LH	5
# define TYPE_U16LH	6
# define TYPE_ULAW	7
# define TYPE_AU2	8
# define TYPE_EOF	9
# define TYPE_GENERIC_ULAW 128

# define POSITIVE_ULAW_ZERO 0xff
# define NEGATIVE_ULAW_ZERO 0x7f

#ifndef	MIN
# define MIN(a,b) (((a)<(b))?(a):(b))
#endif

#ifndef	MAX
# define MAX(a,b) (((a)>(b))?(a):(b))
#endif

#if defined(unix) && !defined(linux)
# define labs abs
#endif

# define ROUNDEDSHIFTDOWN(x, n) (((n) == 0) ? (x) : ((x) >> ((n) - 1)) >> 1)

#ifndef M_LN2
#define	M_LN2	0.69314718055994530942
#endif

/* BUFSIZ must be a multiple of four to contain a whole number of words */
#ifndef BUFSIZ
# define BUFSIZ 1024
#endif

#define putc_exit(val, stream)\
{ char rval;\
  if((rval = putc((val), (stream))) != (char) (val))\
    update_exit(1, "write failed: putc returns EOF\n");\
}

extern int getc_exit_val;
#define getc_exit(stream)\
(((getc_exit_val = getc(stream)) == EOF) ? \
  update_exit(1, "read failed: getc returns EOF\n"), 0: getc_exit_val)

#undef	uchar
#define uchar	unsigned char
#undef	ushort
#define ushort	unsigned short
#undef	ulong
#define ulong	unsigned long

#if defined(__STDC__) || defined(__GNUC__) || defined(sgi) || !defined(unix)
typedef signed char	schar;
#define PROTO(ARGS)	ARGS
#else
typedef char		schar;
#define PROTO(ARGS)	()
#endif

#ifdef NEED_OLD_PROTOTYPES
 /*******************************************/
/* this should be in string.h or strings.h */
extern int	strcmp		PROTO((const char*, const char*));
extern char*	strcpy		PROTO((char*, const char*));
extern char*	strcat		PROTO((char*, const char*));
extern int	strlen		PROTO((const char*));

 /**************************************/
/* defined in stdlib.h if you have it */
extern void*	malloc		PROTO((unsigned long));
extern void	free		PROTO((void*));
extern int	atoi		PROTO((const char*));
extern void	swab		PROTO((char*, char*, int));
extern int	fseek		PROTO((FILE*, long, int));

 /***************************/
/* other misc system calls */
extern int	unlink		PROTO((const char*));
extern void	exit		PROTO((int));
#endif

 /************************/
/* defined in shorten.c */
extern void	init_offset     PROTO((long**, int, int, int));
extern int	shorten		PROTO((FOB*, FOB*, int, char**));

 /*********************/
/* defined in ulaw.c */
extern uchar    linear2ulaw     PROTO((long));
extern int      ulaw2linear     PROTO((uchar));

 /**********************/
/* defined in fixio.c */
extern void     init_sizeof_sample PROTO((void));
extern void	fread_type_init	PROTO((void));
extern int	fread_type	PROTO((long**, int, int, int, FOB*));
extern void	fread_type_quit	PROTO((void));
extern void	fwrite_type_init PROTO((void));
extern void	fwrite_type	PROTO((long**, int, int, int, FOB*));
extern void	fwrite_type_quit PROTO((void));
extern int	find_bitshift	PROTO((long*, int, int));
extern void	fix_bitshift	PROTO((long*, int, int, int));

 /**********************/
/* defined in vario.c */
extern void	var_put_init	PROTO((void));
extern void	uvar_put	PROTO((ulong, int, FOB*));
extern void	var_put		PROTO((long, int, FOB*));
extern void	ulong_put	PROTO((ulong, FOB*));
extern void	var_put_quit	PROTO((FOB*));

extern void	var_get_init	PROTO((void));
extern long	uvar_get	PROTO((int, FOB*));
extern long	var_get		PROTO((int, FOB*));
extern ulong	ulong_get	PROTO((FOB*));
extern void	var_get_quit	PROTO((void));

extern int	sizeof_uvar	PROTO((ulong, int));
extern int	sizeof_var	PROTO((long, int));

extern void     mkmasktab       PROTO((void));
extern void     word_put        PROTO((ulong, FOB*));
extern ulong    word_get        PROTO((FOB*));

 /********************/
/* defined in lpc.c */
extern int	wav2lpc		PROTO((long*,int,long,int*,int,int,float*,float*));

 /*********************/
/* defined in poly.c */
extern int	wav2poly	PROTO((long*, int, long, int, float*, float*));

 /*********************/
/* defined in exit.c */
extern void	basic_exit	PROTO((int));
#ifdef HAVE_STDARG_H
extern void	perror_exit	PROTO((char*, ...));
extern void	usage_exit	PROTO((int, char*, ...));
extern void	update_exit	PROTO((int, char*, ...));
# else
extern void	perror_exit	PROTO(());
extern void	usage_exit	PROTO(());
extern void	update_exit	PROTO(());
# endif

 /***********************/
/* defined in hsgetopt.c */
extern void	hs_resetopt	PROTO((void));
extern int	hs_getopt	PROTO((int, char**, char*));
extern int	hs_optind;
extern char    *hs_optarg;

 /**********************/
/* defined in array.c */
extern void	*pmalloc	PROTO((ulong));
extern long	**long2d	PROTO((ulong, ulong));

 /****************************/
/* defined in dupfileinfo.c */
extern int	dupfileinfo	PROTO((char*, char*));

#endif
