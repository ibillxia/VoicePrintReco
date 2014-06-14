#include <stdio.h>
#define SPHERE_LIBRARY_CODE
#include <sp/sphere.h>

int sp_print_return_status(FILE *fp)
{
    char *proc_name="sp_print_return_status " SPHERE_VERSION_STR;
    
    if (fp == FPNULL)
	return_err(proc_name,100,100,"Null File pointer");
    print_return_status(fp);
    return(0);
}

int sp_get_return_status(void)
{
    return(return_status());
}

int sp_get_return_type(void)
{
    return(return_type());
}
