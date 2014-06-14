/* File: version.h */

#ifndef _VERSION_H_
#define _VERSION_H_

#define SPHERE_VERSION_STR "V2.6"

#ifdef SPHERE_DECLARE_VERSION
char *sp_version_str = "SPHERE Lib " SPHERE_VERSION_STR;
#else
extern char *sp_version_str;
#endif

#endif

