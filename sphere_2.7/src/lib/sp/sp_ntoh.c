#include <stdio.h>
#include <string.h>
#define SPHERE_LIBRARY_CODE
#include <sp/sphere.h>

long sp_ntohl(long t){
    static enum SP_sample_byte_fmt sbf = SP_sbf_null;

    if (sbf == SP_sbf_null)
	sbf = get_natural_sbf(4);

    if (sbf == SP_sbf_3210)
	 return(t);
    if (sbf == SP_sbf_1032){
	swap_short(t);
	return(t);
    }
    fprintf(stderr,
	    "Error: sp_ntohl(%lx) Failed, Contact the SPHERE maintainer\n",
	    t);
    exit(-1);
}

short sp_htons(short t){
    static enum SP_sample_byte_fmt sbf = SP_sbf_null;

    if (sbf == SP_sbf_null)
	sbf = get_natural_sbf(2);

    if (sbf == SP_sbf_10)
	 return(t);
    swap_bytes(t);
    return(t);
}
