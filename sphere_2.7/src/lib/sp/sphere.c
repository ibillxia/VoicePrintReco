/******************************************************************************

Copyright (C) 1992,1993 Tony Robinson

Permission is granted to use this software for non-commercial purposes.
Explicit permission must be obtained from the author to use this software
for commercial purposes.

This software carries no warranty, expressed or implied.  The user assumes
all risks, known or unknown, direct or indirect, which involve this software
in any way.

Dr Tony Robinson
Cambridge University Engineering Department
Trumpington Street, Cambridge, CB2 1PZ, UK.
ajr@eng.cam.ac.uk     voice: +44-223-332815

******************************************************************************/

# include <math.h>
# include <stdio.h>
# include <stdlib.h>
# include <ctype.h>
# include <string.h>
# include <setjmp.h>
# include <util/fob.h>
# include <sp/shorten/shorten.h>
# include <sp/shorten/shrt_sph.h>

extern jmp_buf	exitenv;
extern char	*exitmessage;

# define MAX_FLAG_SIZE 32
# define MAX_SHORTEN_ARGC 256

int   shorten_argc = 1;
char *shorten_argv[MAX_SHORTEN_ARGC];

int shorten_set_flag(char *flag) {
  int nbyte = (int)strlen(flag) + 1;
  char *new_argv = (char *)malloc(nbyte);
  
  if(new_argv == NULL) {
    fprintf(stderr, "shorten_set_flag: malloc(%d) == NULL\n", nbyte);
    return(100);
  }

  if(shorten_argc >= MAX_SHORTEN_ARGC) {
    fprintf(stderr, "shorten_set_flag: maximum argument count exceeded\n");
    return(100);
  }

  strcpy(new_argv, flag);
  shorten_argv[shorten_argc++] = new_argv;
  return(0);
}

void shorten_reset_flags(void) {
  int i;

  for(i = 1; i < shorten_argc; i++)
    free(shorten_argv[i]);
  shorten_argc = 1;
}

void shorten_init(void) {
    shorten_reset_flags();
    shorten_argv[0] = "embedded-shorten";
}

void shorten_dump_flags(FILE *fpout) {
  int i;
    
  fprintf(fpout,"Shorten Arguements:\n");
  for(i = 0; i < shorten_argc; i++)
      fprintf(fpout,"   Arg %1d: %s\n",i,shorten_argv[i]);
}

int shorten_set_ftype(char *ftype) {
    char flag[MAX_FLAG_SIZE];
    sprintf(flag, "-t%s", ftype);
    return(shorten_set_flag(flag));
}

#define MAX_FLAG_SIZE 32
int shorten_set_channel_count(int nchannel) {
  char flag[MAX_FLAG_SIZE];
  sprintf(flag, "-c%d", nchannel);
  return(shorten_set_flag(flag));
}

int shorten_compress(FOB *fpin, FOB *fpout, char *status) {
    int exitcode;

    exitmessage = status;
    if (shorten_set_flag("-") != 0) return(-1);
    if (shorten_set_flag("-") != 0) return(-1);
    exitcode = shorten(fpin, fpout, shorten_argc, shorten_argv);
    shorten_reset_flags();
    return(exitcode + 1);
}

int shorten_uncompress(FOB *fpin, FOB *fpout, char *status) {
    int exitcode;

    if (shorten_set_flag("-x") != 0) return (-1);
    exitmessage = status;
    if (shorten_set_flag("-") != 0) return(-1);
    if (shorten_set_flag("-") != 0) return(-1);
    exitcode = shorten(fpin, fpout, shorten_argc, shorten_argv);
    shorten_reset_flags();
    return(exitcode + 1);
}
