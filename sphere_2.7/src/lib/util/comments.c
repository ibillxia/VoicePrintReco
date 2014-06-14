#include <util/utillib.h>


/*******************************************************************/
/*   check the character pointer to see if it points to the        */
/*   comment character                                             */
/*******************************************************************/
int is_comment(char *str)
{
   if ((*str == COMMENT_CHAR) && (*(str+1) != COMMENT_CHAR)){
      fprintf(stderr,"Warning: The comment designation is now %c%c, the line below\n",
                     COMMENT_CHAR,COMMENT_CHAR);
      fprintf(stderr,"         has only one comment character, this may");
     fprintf(stderr," be an error\n         %s\n",str);
   }

   if ((*str == COMMENT_CHAR) && (*(str+1) == COMMENT_CHAR))
       return(TRUE);
   else
       return(FALSE);
}

/*******************************************************************/
/*   check the character pointer to see if it points to the        */
/*   comment_info character                                        */
/*******************************************************************/
int is_comment_info(char *str)
{
   if ((*str == COMMENT_INFO_CHAR) && (*(str+1) != COMMENT_INFO_CHAR)){
      fprintf(stderr,"Warning: The comment designation is now %c%c, the line below\n",
                     COMMENT_INFO_CHAR,COMMENT_INFO_CHAR);
      fprintf(stderr,"         has only one comment info character, this may");
     fprintf(stderr," be an error\n         %s\n",str);
   }
   if ((*str == COMMENT_INFO_CHAR) && (*(str+1) == COMMENT_INFO_CHAR))
       return(TRUE);
   else
       return(FALSE);
}
