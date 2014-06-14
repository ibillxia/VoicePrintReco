#include <util/utillib.h>


/**********************************************************************/
/*  function to just say malloc died                                  */
/**********************************************************************/
void malloc_died(int len)
{
    fprintf(stderr,"Malloc could not allocate memory of size %d\n",len);
    exit(-1);
}
