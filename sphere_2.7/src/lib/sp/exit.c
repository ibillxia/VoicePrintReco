/******************************************************************************
*                                                                             *
*       Copyright (C) 1992-1995 Tony Robinson                                 *
*                                                                             *
*       See the file LICENSE for conditions on distribution and usage         *
*                                                                             *
******************************************************************************/

/* Added by Jon Fiscus */
#ifdef __STDC__
#ifndef HAVE_STDARG_H
#define HAVE_STDARG_H
#endif
#endif
 
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <setjmp.h>
# ifdef HAVE_STDARG_H
#   include <stdarg.h>
# else
#   include <varargs.h>
# endif
/* Changed by Jon Fiscus */
# include <util/fob.h>
# include <sp/shorten/shorten.h>

/* From: fred@masterd.fdn.org (Frederic SAVOIR) */
#ifdef __WATCOMC__
extern volatile int errno;
#else
/* Added by jaube@nist.gov - Smartspace group */
/* extern int errno; */
#include <errno.h>
#endif

extern const char *const sys_errlist[];
/* extern char *sys_errlist[]; */
extern char *argv0;
extern char *filenameo;
extern FILE *fileo;

jmp_buf	exitenv;
char	*exitmessage;

void basic_exit(exitcode) int exitcode; {

/* only do this if it is standalone code */
#ifdef STANDALONE_CODE
    
  /* try to delete the output file on all abnormal exit conditions */
  if(exitcode != 0 && fileo != NULL && fileo != stdout) {
    fclose(fileo);
    unlink(filenameo);
  }
#endif

  if(exitmessage == NULL)
    exit(exitcode < 0 ? 0 : exitcode);
  else
    longjmp(exitenv, exitcode);
}

# ifdef HAVE_STDARG_H
void perror_exit(char* fmt, ...) {
  va_list args;

  va_start(args, fmt);
# else
void perror_exit(va_alist) va_dcl {
  va_list args;
  char    *fmt;

  va_start(args);
  fmt = va_arg(args, char*);
# endif

  if(exitmessage == NULL) {
    fprintf(stderr, "%s: ", argv0);
    (void) vfprintf(stderr, fmt, args);
    (void) fprintf(stderr, ": ");
    perror("\0");
  }
  else {
    (void) vsprintf(exitmessage, fmt, args);
    strcat(exitmessage, ": ");
    strcat(exitmessage, strerror(errno));
    strcat(exitmessage, "\n");
  }

  va_end(args);

  basic_exit(errno);
}

# ifdef HAVE_STDARG_H
void usage_exit(int exitcode, char* fmt, ...) {
  va_list args;

  va_start(args, fmt);
# else
void usage_exit(va_alist) va_dcl {
  va_list args;
  int	  exitcode;
  char    *fmt;

  va_start(args);
  exitcode = va_arg(args, int);
  fmt      = va_arg(args, char*);
# endif

  if(exitmessage == NULL) {
    if(fmt != NULL) {
      fprintf(stderr, "%s: ", argv0);
      (void) vfprintf(stderr, fmt, args);
    }
    fprintf(stderr, "%s: for more information use: %s -h\n", argv0, argv0);
  }
  va_end(args);

  basic_exit(exitcode);
}


# ifdef HAVE_STDARG_H
void update_exit(int exitcode, char* fmt, ...) {
  va_list args;

  va_start(args, fmt);
# else
void update_exit(va_alist) va_dcl {
  va_list args;
  int	  exitcode;
  char    *fmt;

  va_start(args);
  exitcode = va_arg(args, int);
  fmt      = va_arg(args, char*);
# endif

  if(exitmessage == NULL) {
    if(fmt != NULL) {
      fprintf(stderr, "%s: ", argv0);
      (void) vfprintf(stderr, fmt, args);
    }
    fprintf(stderr, "%s: version %d.%s\n",argv0,FORMAT_VERSION,BUGFIX_RELEASE);
    fprintf(stderr, "%s: please report this problem to ajr4@cam.ac.uk\n", argv0);
  }
  va_end(args);

  basic_exit(exitcode);
}
