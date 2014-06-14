/**********************************************************************/
/*                                                                    */
/*             FILENAME:  str_srch.c                                  */
/*             BY:  Jonathan G. Fiscus                                */
/*                  NATIONAL INSTITUTE OF STANDARDS AND TECHNOLOGY    */
/*                  SPEECH RECOGNITION GROUP                          */
/*                                                                    */
/*           DESC:  This file contains general routines used          */
/*                  throughout the scoring package                    */
/*                                                                    */
/**********************************************************************/
#include <util/utillib.h>

/*********************************************************************/
/*   Return the index into the list of character string matching the */
/*   goal, if non is found, return a value < 0                       */
/*********************************************************************/
int strings_search(char **list, int count, char *goal)
{
    int low, high, mid, eval;

    low = 0, high = count-1;

    do { 
        mid = (low + high)/2;
        eval = strcmp(goal,list[mid]);
/*        printf("%s:  %s (%d) [mid %s (%d)] %s (%d) = res %d\n",
                   goal,list[low],low,list[mid],mid,
                   list[high],high,eval);*/
        if (eval == 0)
            return(mid);
        if (eval < 0)
            high = mid-1;
        else
            low = mid+1;
    } while (low <= high);

    return(-1);
}
