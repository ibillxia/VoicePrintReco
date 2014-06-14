/**********************************************************************/
/*                                                                    */
/*             FILENAME:  pad.c                                       */
/*             BY:  Jonathan G. Fiscus                                */
/*                  NATIONAL INSTITUTE OF STANDARDS AND TECHNOLOGY    */
/*                  SPEECH RECOGNITION GROUP                          */
/*                                                                    */
/*           DESC:  This is file contains routines to set 'pad' to    */
/*                  a certain length.  `pad` is a character array used*/
/*                  for centering output,  when it's set to a length, */
/*                  n spaces are written into the array and terminated*/
/*                  with a NULL_CHAR so when written to output, there */
/*                  will be n spaces before the next formatted item   */
/*                                                                    */
/*              **  These functions make use of a static variable to  */
/*                  tell the routines how long a single output line   */
/*                  can be.                                           */
/*                                                                    */
/**********************************************************************/
#include <util/utillib.h>

static int pad_print_out_width=SCREEN_WIDTH;

/*************************************************************/
/*    init the utilities to the proper printout dimensions   */
/*************************************************************/
void init_pad_util(int pr_width)
{
   pad_print_out_width = pr_width;
}

/*************************************************************/
/*    return the value of the print_out_width                */
/*************************************************************/
int pad_pr_width(void)
{
    return(pad_print_out_width);
}

/*************************************************************/
/*     pad manipulating routines                             */
/*************************************************************/
/*  set the pad to center the passed in string               */
void set_pad(char *pad, char *str)
{
    int i, len;
    
    len = (pad_print_out_width - strlen(str) )/2;
    for (i=0; i<len;i++)
        pad[i] = SPACE;
    if (i>0)
       pad[i-1] = NULL_CHAR;
    else
       pad[0] = NULL_CHAR;
}

/*************************************************************/
/*  set the pad to n spaces                                  */
void set_pad_n(char *pad, int n)
{
    int i;

    for (i=0;i<n;i++)
        pad[i]=SPACE;
    pad[i] = NULL_CHAR;
}

/*************************************************************/
/*  set the pad to center a string of length `len`           */
void set_pad_cent_n(char *pad, int len)
{
    int i;

    for (i=0;i<((pad_print_out_width - len) / 2);i++)
        pad[i]=SPACE; 
    if (i>0)
       pad[i-1] = NULL_CHAR;
    else
       pad[0] = NULL_CHAR;
}

/************************************************************/
/* Return a pointer to a string with a centered version of  */
/* str to len characters                                    */	
/************************************************************/
char *center(char *str, int len)
{
    static char desc[400], *ptr;
    int il, ft, bk, i;
    if (len >= 400){
        fprintf(stderr,"Error: center utility in gen.c failed, buffer should be manually expanded to >%d characters\n",
                       len);
        exit(-1);
    }

    il = strlen(str);
    ft = (len - il) / 2;
    bk = len - (il+ft);
    ptr=desc;
    for (i=0; i<ft; i++)
        *ptr++ = ' ';
    for (i=0; i<il; i++)
        *ptr++ = str[i];
    for (i=0; i<bk; i++)
        *ptr++ = ' ';
    *ptr = '\0';
    return(desc);

}
