
/* LINTLIBRARY */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define SPHERE_LIBRARY_CODE
#include <sp/sphere.h>

char * sptemp(char *file)
{
    int len;
    char * s;


    if ( file == CNULL )
	return CNULL;
    len = strlen( file ) + strlen( TMPEXT ) + 1;
    s = mtrf_malloc( len );
    if ( s == CNULL )
	return CNULL;

    (void) strcpy( s, file );
    (void) strcat( s, TMPEXT );
    return s;
}

char * sptemp_dirfile(void)
{
    int max_attempt=999, attempt=0;
    char * s, *n;
    static int call=0;
    struct stat fileinfo;


    do {
	s = rsprintf("%s/%s%d.sph",TEMP_DIR,TEMP_BASE_NAME,call++);
	if (attempt++ >= max_attempt)
	    return(CNULL);
	if (call > 9999)
	    call=0;
    }  while (stat(s,&fileinfo) == 0);
    if ((n = mtrf_malloc(strlen(s) + 1)) == CNULL)
	return(CNULL);
    strcpy(n,s);
    return(n);
}

