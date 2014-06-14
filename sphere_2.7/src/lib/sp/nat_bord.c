#include <stdio.h>
#include <string.h>
#define SPHERE_LIBRARY_CODE
#include <sp/sphere.h>

enum SP_sample_byte_fmt get_natural_sbf(int sample_size)
{
    short s;
    int sl;
    unsigned char *cp;
    char sbf_str[10],n;

    if (sample_size == 1)
	return(SP_sbf_1);

    s = 1;
    sl = 0x03020100;
    cp = (unsigned char *)&s;

    if (sample_size == 2) {
	if (*cp == 0)
	    return(SP_sbf_10);
	else if (*cp != 0)
	    return(SP_sbf_01);
	return(SP_sbf_null);    
    }
    if (sample_size == 4) {
	cp = (unsigned char *)&sl;
	/* byte 1 */ memcpy(&n,cp,1);    sbf_str[0] = '0' + n;	
	/* byte 2 */ memcpy(&n,cp+1,1);  sbf_str[1] = '0' + n;
	/* byte 3 */ memcpy(&n,cp+2,1);  sbf_str[2] = '0' + n;
	/* byte 4 */ memcpy(&n,cp+3,1);  sbf_str[3] = '0' + n;
	sbf_str[4] = '\0';
	if (strsame(sbf_str,"3210")) return(SP_sbf_3210);
	else if (strsame(sbf_str,"2301")) return(SP_sbf_2301);
	else if (strsame(sbf_str,"1032")) return(SP_sbf_1032);
	else if (strsame(sbf_str,"0123")) return(SP_sbf_0123);
    }
    return(SP_sbf_N);    
}
