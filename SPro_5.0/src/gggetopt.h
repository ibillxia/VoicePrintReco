/*
   This is genuinely free replacement of the popular GNU getopt.

   Not all features of GNU getopt are supported though. In particular,
   this code was desgined to deal with long options as a replacement
   of getopt_long(), not getopt(). Hence, getopt() is *not*
   implemented (and will probably never be). Short options with
   arguments should be immediatly followed by the argument's value
   (glued or not).

   Invalid syntax (that are valid with GNU getopt):

   './test input.txt -v -o output.txt'               

   './test -ov output.txt input.txt' should be './test -v -f output.txt input.txt'
*/
#ifndef _GETOPT_H
# define _GETOPT_H 1

# ifdef  __cplusplus
extern "C" {
# endif

  // # define GG_GETOPT_IMPLEMENTATION 

extern char * optarg; /* value of the argument */
extern int optind; /* index of current option in argv */
extern int opterr; /* whether or not to print error message */
extern int optopt; /* returned bad option character */

struct option {
  char * name;
  int type; // has_arg;
  int * address; // flag
  int val;
};

# define no_argument 0
# define required_argument 1
# define optional_argument 2

extern int getopt (int, char * const *, const char *);
extern int getopt_long (int, char * const *, const char *, const struct option *, int *);

# ifdef  __cplusplus
}
# endif

# endif /* _GETOPT_H */
