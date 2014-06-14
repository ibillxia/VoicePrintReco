/**********************************************************************/
/*                                                                    */
/*             FILENAME:  strings.c                                   */
/*             BY:  Jonathan G. Fiscus                                */
/*                  NATIONAL INSTITUTE OF STANDARDS AND TECHNOLOGY    */
/*                  SPEECH RECOGNITION GROUP                          */
/*                                                                    */
/*           DESC:  This file contains general routines used          */
/*                  throughout the scoring package                    */
/*                                                                    */
/**********************************************************************/
#include <util/utillib.h>

/***************************************************************/
/*  Return TRUE if the string is empty, i.e. containing all    */
/*  spaces, or tabs.                                           */
/***************************************************************/
int is_empty(char *str)
{
    if (str == NULL) return(FALSE);
    while (isspace(*str))
        str++;
    if (*str == '\0')
        return(TRUE);
    return(FALSE);
}
 
/***************************************************************/
/*  move the character pointer to the character                */
/***************************************************************/
void search_for_char(char **ptr, char chr)
{
    while ((**ptr != chr) && (**ptr != NULL_CHAR))
        (*ptr)++;
}

/***************************************************************/
/*  move the character pointer BACKWARDS to the character      */
/***************************************************************/
void search_back_for_char(char *beg_ptr, char **ptr, char chr)
{
    while ((**ptr != chr) && (*ptr != beg_ptr))
        (*ptr)--;
}
 
/***************************************************************/
/*  copy the strings until the character of the NULL is found  */
/***************************************************************/
void strcpy_to_char(char *to, char *from, char chr)
{
    int i=(-1);

    do{
        i++;
        to[i] = from[i];
    }while ((from[i] != chr) && (from[i] != NULL_CHAR));
    to[i+1] = NULL_CHAR;
}

/***************************************************************/
/*  copy the strings until the character of the NULL is found  */
/*  except dont copy the search character if it exists         */
/***************************************************************/
void strcpy_to_before_char(char *to, char *from, char chr)
{
    int i=(-1);

    do{
        i++;
        to[i] = from[i];
    }while ((from[i] != chr) && (from[i] != NULL_CHAR));
    if (from[i] == chr)
       to[i] = NULL_CHAR;
}

/***************************************************************/
/*  copy the strings until the character of the NULL is found  */
/*  except dont copy the search character if it exists         */
/*  only copy a maximum of len characters                      */
/***************************************************************/
void strncpy_to_before_char(char *to, char *from, char chr, int len)
{
    int i=(-1);

    do{
        i++;
        to[i] = from[i];
    }while ((from[i] != chr) && (from[i] != NULL_CHAR) && (i<len-2));
                                 /* -2 allows appending a null character*/
    if (i == len-2)
       to[i+1] = NULL_CHAR;
    if (from[i] == chr)
       to[i] = NULL_CHAR;
}

/***************************************************************/
/*  copy the strings but if a character is upper case convert  */
/*  it to lower case                                           */
/***************************************************************/
void strcpy_lc(char *to, char *from)
{
    while (*from != NULL_CHAR){
        if ((*from >= 'A') && (*from <= 'Z'))
            *(to++) = *(from++) + 32;
        else
            *(to++) = *(from++);
    }
    *to = NULL_CHAR;
}

/***************************************************************/
/*  copy the string if the len of from<len, the pad with char  */
/***************************************************************/
void strncpy_pad(char *to, char *from, int len, char chr)
{
    int i;
    for (i=0; i<len; i++){
       if (*from != NULL_CHAR)
          *(to++) = *(from++);
       else
          *(to++) = chr;
    }
    *to = NULL_CHAR;
}

 /***************************************/
 /* some general-purpose functions      */
 /* found in Turbo C but not BCD 4.2    */
 /* source code:                        */
 /* strstr(ps1,ps2)  K&R 2nd ed p. 250  */
 /***************************************/

 
 /********************************************************/
 /*  strstr1(ps1,ps2)                                     */
 /*  Scans string ps1 for the first occurrence of ps2.   */
 /*  Returns a pointer to the element is ps1 where ps2   */
 /* begins.  If ps2 does not occur in ps1, returns null. */
 /********************************************************/
 char *strstr1(char *ps1, char *ps2)
{
  char *px, *plast; int matched; int lps2;
  matched = 0;
  lps2 = strlen(ps2);
  plast = ps1 + strlen(ps1) - lps2;
  for (px = ps1; ((!matched) && (px <= plast)); px++)
    if (strncmp(px,ps2,lps2) == 0) matched = 1;
  if (!matched) px = NULL;
  return px;
 }

 
 /********************************************************/
 /*  strstr1(ps1,ps2)                                     */
 /*  Scans string ps1 for the first occurrence of ps2.   */
 /*  Returns a pointer to the element is ps1 where ps2   */
 /* begins.  If ps2 does not occur in ps1, returns null. */
 /********************************************************/
 char *strstr1_i(char *ps1, char *ps2)
{
  char *px, *plast; int matched; int lps2;
  matched = 0;
  lps2 = strlen(ps2);
  plast = ps1 + strlen(ps1) - lps2;
  for (px = ps1; ((!matched) && (px <= plast)); px++)
    if (strncasecmp(px,ps2,lps2) == 0) matched = 1;
  if (!matched) px = NULL;
  return px;
}
 

/**********************************************/
/* convert a string to upper case             */
/**********************************************/
char *str2up(char *str)
{
    static char mem[100], len=100;
    char *t=mem, *f;
    for (f=str; (*f != NULL_CHAR) && (t<mem+len-1); f++,t++)
         *t = (islower(*f) ? *f - 32 : *f);
    *t=NULL_CHAR;
    if (t == mem+len-1)
         fprintf(stderr,
                 "Warning: Upper case string may be truncated\n From: %s\n To:   %s\n",
                 str,mem);
    return(mem);
  }
 
/**********************************************/
/* convert a string to lower case             */
/**********************************************/
char *str2low(char *str)
{
    static char mem[100], len=100;
    char *t=mem, *f;
    for (f=str; (*f != NULL_CHAR) && (t<mem+len-1); f++,t++){
         *t = (isupper(*f) ? (*f) + 32 : *f);
/*         printf(" char %c isupper %d \n",*f,isupper(*f));*/
	}
    *t=NULL_CHAR;
    if (t == mem+len-1)
         fprintf(stderr,
                 "Warning: Upper case string may be truncated\n From: %s\n To:   %s\n",
                 str,mem);
    return(mem);
}

/**********************************************/
/* return 1 if a string is a number           */
/**********************************************/
int is_number(char *str)
{
    /* Skip a leading minus sign */
    if (*str == '-') str++;
    for (;*str != '\0'; str++)
	if (strchr("0123456789.",*str) == NULL)
	    return(0);
    return(1);
}

/**********************************************/
/* return 1 if a string is an integer number  */
/**********************************************/
int is_integer(char *str)
{
    /* Skip a leading minus sign */
    if (*str == '-') str++;
    for (;*str != '\0'; str++)
	if (strchr("0123456789",*str) == NULL)
	    return(0);
    return(1);
}

/***************************************************************/
/*  Return TRUE if the strings are equal ignoring hyphens and  */
/*  underscores, (NHU)                                         */
/***************************************************************/
int strcmp_nhu(char *s, char *t)
{
    if (s == (char*)0 || t == (char *)0){
	fprintf(stderr,"Error: strcmp_nhu comparing null pointer,");
	fprintf(stderr,"s=%ld t=%ld\n",(long)s,(long)t);
	return(0);
    }
    while (*s != '\0' && (strchr("_- ",*s) != NULL))
	s++;
    while (*t != '\0' && (strchr("_- ",*t) != NULL))
	t++;
    do {
	if (*s != *t) {
	    return(*s - *t);
	}
	for (s++; *s != '\0' && (strchr("_- ",*s) != NULL); s++)
	    ;
	for (t++; *t != '\0' && (strchr("_- ",*t) != NULL); t++)
	    ;
    } while (*s != '\0' && *t != '\0');
    return(*s - *t);
}
