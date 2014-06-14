/**********************************************************************/
/*                                                                    */
/*             FILENAME:  t_or_f.c                                    */
/*             BY:  Jonathan G. Fiscus                                */
/*                  NATIONAL INSTITUTE OF STANDARDS AND TECHNOLOGY    */
/*                  SPEECH RECOGNITION GROUP                          */
/*                                                                    */
/*           DESC:  This file contains general routines used          */
/*                  throughout the scoring package                    */
/*                                                                    */
/**********************************************************************/
#include <util/utillib.h>

/**********************************************************************/
/*  return the strings depending on the value passed down             */
/**********************************************************************/
char *TRUE_STRING="TRUE", *FALSE_STRING="FALSE";
char *t_or_f(int val)
{
    if (val == TRUE)
        return(TRUE_STRING);
    else
        return(FALSE_STRING);
}
