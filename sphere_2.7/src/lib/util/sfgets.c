/**********************************************************************/
/*                                                                    */
/*             FILENAME:  safe_fgets.c                                */
/*             BY:  Jonathan G. Fiscus                                */
/*                  NATIONAL INSTITUTE OF STANDARDS AND TECHNOLOGY    */
/*                  SPEECH RECOGNITION GROUP                          */
/*                                                                    */
/*           DESC:  This file contains general routines used          */
/*                  throughout the scoring package                    */
/*                                                                    */
/**********************************************************************/
#include <util/utillib.h>

#define safe_fgets_C_VERSION "V1.1"

/**********************************************/
/*   A safe version of fgets(), it will check */
/*   to make sure that if len characters where*/
/*   read, the last character before the NULL */
/*   is a '\n'.                               */
/**********************************************/
char *safe_fgets(char *arr, int len, FILE *fp)
{
   char *tc, ans;

   if (fgets(arr,len,fp) == NULL)
       return(NULL);
   tc = arr;
   while (*tc != '\0') tc++;
   if ((tc - arr) == len-1)
       if (*(tc-1) != '\n'){
           fprintf(stderr,"Warning: safe_fgets could not read");
           fprintf(stderr," and entire line\nDo you want");
           fprintf(stderr," to (d) dump core or (c) continue? [d]  ");
           ans = getchar();
           if ((ans == 'c') || (ans == 'C'))
               ;
           else
               abort();
        }
    return(arr);
}
