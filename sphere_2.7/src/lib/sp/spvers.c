#include <stdio.h>
#define SPHERE_LIBRARY_CODE
#define SPHERE_DECLARE_VERSION
#include <sp/sphere.h>

char *sp_get_version(void){
    return(sp_version_str);
}
    

