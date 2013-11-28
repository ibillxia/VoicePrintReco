/*
   Copyright (c) 2010 Guillaume Gravier

   Permission is hereby granted, free of charge, to any person
   obtaining a copy of this software and associated documentation
   files (the "Software"), to deal in the Software without
   restriction, including without limitation the rights to use, copy,
   modify, merge, publish, distribute, sublicense, and/or sell copies
   of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
   BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/

/*

   This is a non GPL implementation of GNU getopt() and getopt_long(),
   provided as a non contaminating replacement for GNU functions in
   proprietary software. 

   For a detailed description of GNU getopt functions, see related man
   pages, a copy/paste of which are provided below (though this
   probably violates the GPL license).
   
   Note that *not* all features of GNU getopt are implemented in
   gggetopt, in particular the following things differs:

     - only the POSIXLY_CORRECT implementation is provided by
     gggetopt, which means that arguments are not permutted and the
     optional initial '+' sign in optstring is not supported ('+' will
     be treated as the declaration of a regular short option);

     - the optional initial '-' sign in optstring, which tells getopt
     to handle each non-option argument as the argument of the special
     option with character 1, is not supported;

     - getopt_long_only() features are not supported, thus not
     permitting to reference long options with a single dash;

     - the special short option switch W is not supported;

   Return codes in gggetopt are as for GNU getopt, i.e.:

     For short options: If a valid (correctly formed and properly
     declared in optstring) short option is found, returns the
     option's character. The value '?' is returned when an undeclared
     option is found. In case of missing argument, '?' is returned
     unless optstring starts with ':' in which case ':' is returned.

     For long options: return the value declared in the longopts table
     if flag is NULL; otherwise return 0. In case of an ambiguous
     abbreviation, return '?'. 
*/

/*
   Extract of the GNU getopt manpage, describing features which apply
   to gggetopt:

   ===============
       #include <unistd.h>

       int getopt(int argc, char * const argv[],
                  const char *optstring);

       extern char *optarg;
       extern int optind, opterr, optopt;

       #define _GNU_SOURCE
       #include <getopt.h>

       int getopt_long(int argc, char * const argv[],
                  const char *optstring,
                  const struct option *longopts, int *longindex);
       
       [...]
		  
       The getopt() function parses the command-line arguments.  Its
       arguments argc and argv are the argument count and array as
       passed to the main() function on program invocation.  An
       element of argv that starts with '-' (and is not exactly "-" or
       "--") is an option element.  The charac- ters of this element
       (aside from the initial '-') are option charac- ters.  If
       getopt() is called repeatedly, it returns successively each of
       the option characters from each of the option elements.

       If getopt() finds another option character, it returns that
       character, updating the external variable optind and a static
       variable nextchar so that the next call to getopt() can resume
       the scan with the following option character or argv-element.

       If there are no more option characters, getopt() returns -1.
       Then optind is the index in argv of the first argv-element that
       is not an option.

       optstring is a string containing the legitimate option
       characters.  If such a character is followed by a colon, the
       option requires an argu- ment, so getopt() places a pointer to
       the following text in the same argv-element, or the text of the
       following argv-element, in optarg.  Two colons mean an option
       takes an optional arg; if there is text in the current
       argv-element (i.e., in the same word as the option name itself,
       for example, "-oarg"), then it is returned in optarg, otherwise
       optarg is set to zero.

       [...]

       If getopt() does not recognize an option character, it prints
       an error message to stderr, stores the character in optopt, and
       returns '?'.  The calling program may prevent the error message
       by setting opterr to 0.

       If getopt() finds an option character in argv that was not
       included in optstring, or if it detects a missing option
       argument, it returns '?'  and sets the external variable optopt
       to the actual option character.  If the first character [...] 
       of optstring is a colon (':'), then getopt() returns ':'
       instead of '?'  to indicate a missing option argument.  If an
       error was detected, and the first character of optstring is not
       a colon, and the external variable opterr is non-zero (which is
       the default), getopt() prints an error message.

       The getopt_long() function works like getopt() except that it
       also accepts long options, started out by two dashes.  (If the
       program accepts only long options, then optstring should be
       specified as an empty string (""), not NULL.)  Long option
       names may be abbreviated if the abbreviation is unique or is an
       exact match for some defined option.  A long option may take a
       parameter, of the form --arg=param or --arg param.

       longopts is a pointer to the first element of an array of
       struct option declared in <getopt.h> as

          struct option {
              const char *name;
              int has_arg;
              int *flag;
              int val;
          };

       The meanings of the different fields are:

       name is the name of the long option.

       has_arg is: no_argument (or 0) if the option does not take an
       argument; required_argument (or 1) if the option requires an
       argument; or optional_argument (or 2) if the option takes an
       optional argu- ment.

       flag specifies how results are returned for a long option.  If
       flag is NULL, then getopt_long() returns val.  (For example,
       the calling program may set val to the equivalent short option
       char- acter.)  Otherwise, getopt_long() returns 0, and flag
       points to a variable which is set to val if the option is
       found, but left unchanged if the option is not found.

       val is the value to return, or to load into the variable
       pointed to by flag.

       The last element of the array has to be filled with zeroes.

       If longindex is not NULL, it points to a variable which is set
       to the index of the long option relative to longopts.

       [...]
   ===============
*/


#ifndef	__GNU_LIBRARY__ /* or should it be __GNU_SOURCE */

# ifdef HAVE_CONFIG_H
#  include <config.h>
# endif

# include <stdio.h>
# include "gggetopt.h"

char * optarg = NULL;
int optind = 1;
int opterr = 1;
int optopt = 0;

/*
   current index in the current argv entry
*/
char * _nextchar = NULL;


/* ---------------------------------------------------------- */
/* ----- int getopt (int, char * const *, const char *) ----- */
/* ---------------------------------------------------------- */
int getopt (int argc, char * const * argv, const char * options)
{
  int _getopt_internal (int, char * const *, const char *, const struct option *, int *, int);

  return _getopt_internal(argc, argv, options, NULL, NULL, 0);
}

/* --------------------------------------------------------------------------------------------- */
/* ----- int getopt_long(int, char * const *,  const char *, const struct option *, int *) ----- */
/* --------------------------------------------------------------------------------------------- */
int getopt_long(int argc, char * const * argv,  const char * options, const struct option * longopts, int * longidx)
{
  int _getopt_internal (int, char * const *, const char *, const struct option *, int *, int);

  return _getopt_internal(argc, argv, options, longopts, longidx, 0);
}

/* ----------------------------------------------------------------------------------------------------- */
/* ----- int _getopt_internal (int, char * const, const char *, const struct option *, int *, int) ----- */
/* ----------------------------------------------------------------------------------------------------- */
/* 
   This is the generic getopt function.
*/
int _getopt_internal (int argc, char * const * argv, const char * options, const struct option * longopts, int * longidx, int long_only)
{
  const char * p;
  int c;

  int _getopt_long(int, char * const *, const struct option *, int *, const char *);

  optarg = NULL;

  /* 
     Check we're not in the middle of scanning bundled short
     options. If we're not, then we shift optind and process.
  */
  
  if ( _nextchar == NULL || *_nextchar == 0 ) {

    if ( optind == argc)
      return -1;

    p = argv[optind];
    
    /* 
       check we have an option
    */
    if ( ( *p != '-' ) || ( *p == '-' && p[1] == 0 ) )
      return (-1);

    /* 
       we do have an option! now, we must check it's not '--' which is
       not truly an option, before deciding to process that as a long
       option or as a short option.
    */
    if ( p[1] == '-' ) {

      if ( p[2] == 0 ) { /* found  '--' special option */
	optind++;
	return -1;
      }
    
      if ( longopts ) { /* found long option (--x*) */
	optind++;
	return _getopt_long(argc, argv, longopts, longidx, p + 2);
      }
    }
    
    /* 
       so now it is a short option: simply initialize _nextchar to the
       first option switch and proceed as if we were following up on
       previously seen bundled option switches.
    */
    _nextchar = (char *) (p + 1);
  }
  
  /* 
     we're here because we are searching for a short option pointed to by _nextchar
  */
  c = *_nextchar++;

  if ( *_nextchar == 0 ) /* end of current argv entry -> shift to next one */
    optind++;

  /* search if switch is declared as an option */
  p = options;

  while ( *p ) {
    if ( *p == c )
      break;
    p++;
  }

  /* if not found, then we're out */
  if ( *p == 0 ) {

    if ( opterr )
      fprintf(stderr, "%s: illegal option -- %c\n", argv[0], c);
    
    optopt = c;
    return '?';
  }

  if ( p[1] == ':' ) {
    
    if ( p[2] == ':' ) { /* optional argument */
      if ( *_nextchar ) {
	optarg = _nextchar;
	_nextchar = NULL;
	optind++;
      }
      else
	optarg = NULL;
    }
    else { /* required argument */
      if ( *_nextchar ) {
	optarg = _nextchar;
	_nextchar = NULL;
	optind++;
      }
      else {
	
	if ( optind == argc ) {
	  
	  if ( opterr )
	    fprintf(stderr, "%s: option requires an argument -- %c\n", argv[0], c);

	  optopt = c;

	  return ( *options == ':' ) ? ':' : '?'; // might also return ':'
	}

	optarg = argv[optind++];
	_nextchar = NULL;
      }
    }
  }
  /* else no argument */

  return (c);
}


/* --------------------------------------------------------------------------------------------- */
/* ----- int _getopt_long(int, char * const *, const struct option *, int *, const char *) ----- */
/* --------------------------------------------------------------------------------------------- */
/*

*/
int _getopt_long(int argc, char * const * argv, const struct option * longopts, int * longidx, const char * optstr)
{
  int i = -1;
  int ifound = -1;
  int nfounds = 0;
  int len = -1;
  const struct option * pfound = NULL;

  *longidx = -1;

  /*
     search for the entry in longops which (unambiguously) corresponds to optstr
  */
  while ( longopts[++i].name ) {
    int k = 0;
    char * p = longopts[i].name;
   
    while ( *p && optstr[k] ) { /* find longest common prefix between longopts[i].name and optstr */

      if ( *p != optstr[k] )
	break;
 
      k++;
      p++;
    }

    if ( optstr[k] != 0 && optstr[k] != '=' ) /* optstr is not a prefix of longopts[i].name -> not a match */
      continue;

    len = k;

    if ( *p == 0 ) { /* exact match */
      nfounds = 1;
      ifound = i;
      break;
    }

    /* if we get to that line, it's because optstr is a prefix of
       longopts[i].name: but we have to keep on scanning longopts to
       make sure that (i) optstr is a unambiguous prefix and that (ii)
       there is no exact match for optstr somewhere after. */
    nfounds++;
    ifound = i;
  }

  /*
     if optstr was not found or if it is ambiguous, print error message and return -1
  */
  if ( ! nfounds ) {

    if ( opterr )
      fprintf(stderr, "unknown long option name '--%s'\n", optstr); // should remove the =val (if any) to be real clean!

    return '?';
  }

  if ( nfounds > 1 ) {

    if ( opterr )
      fprintf(stderr, "%s: option `%s' is ambiguous\n", argv[0], optstr);

    _nextchar = NULL;
    // optind++;
    optopt = 0;

    return '?';
  }

  /*
     ok, here we go, we found it! process argument of any and wrap it
     up nicely.
  */
  pfound = longopts + ifound;
  // optind++;
  _nextchar = NULL;

  // fprintf (stderr, "_getopt_long(): optstr=%s matches %s (%d:%x:%d)\n", optstr, pfound->name, pfound->type, pfound->address, pfound->val);

  switch ( pfound->type ) {

  case optional_argument:
    optarg = ( optstr[len] == '=' ) ? (char *) ( optstr + len + 1 ) : (char *) 0;
    break;

  case required_argument:
    if ( optstr[len] == '=' )
      optarg = (char *) (optstr + len + 1);
    else if ( optind != argc ) {
      optarg = argv[optind++];
      
      /* check however that next arg is not '--' */
      if ( strcmp(optarg, "--") == 0 ) {

	if ( opterr )
	  fprintf(stderr, "%s: option `%s' requires an argument\n", argv[0], pfound->name);

	optopt = pfound->val;

	return ( *optstr == ':' ) ? ':' : '?';	
      }
    }
    else {

      if ( opterr )
	fprintf(stderr, "%s: option `%s' requires an argument\n", argv[0], pfound->name);

      optopt = pfound->val;

      return ( *optstr == ':' ) ? ':' : '?';
    }
    break;
    
  case no_argument:
    if ( optstr[len] == '=' ) { /* extraneous argument */

      if ( opterr )
	fprintf(stderr, "%s: option `--%s' doesn't allow an argument", argv[0], pfound->name);

      optopt = pfound->val;
      return '?';
    }
    break;

  default: /* to please some nasty compilers */
    ;
  }

  if ( pfound->address )
    *(pfound->address) = pfound->val;
 
  if ( longidx )
    *longidx = ifound;

  return pfound->val;
}

#endif /* __GNU_LIBRARY__ */

#ifdef TEST

# include <stdio.h>
# include <stdlib.h>

int main (int argc, char ** argv)
{
  int c;
  int digit_optind = 0;

  opterr = 1;

  if ( argc == 1 ) 
    {
      fprintf(stderr, "valid short options: abc:d:0123456789\n");
      fprintf(stderr, "valid long options with no arguments: append, create\n");
      fprintf(stderr, "valid long options with optional arguments: verbose\n");
      fprintf(stderr, "valid long options with required arguments: add, file\n");
    }

  while (1)
    {
      int this_option_optind = optind ? optind : 1;
      int option_index = 0;
      static struct option long_options[] =
      {
	{"add", required_argument, 0, 0},
	{"append", no_argument, 0, 0},
	{"delete", required_argument, 0, 0},
	{"verbose", optional_argument, 0, 0},
	{"create", no_argument, 0, 0},
	{"file", required_argument, 0, 0},
	{0, 0, 0, 0}
      };

      c = getopt_long (argc, argv, ":abc:d:v::0123456789", long_options, &option_index);

      fprintf(stderr, "getopt_long returned %d  (optind=%d optopt=%d)\n", c, optind, optopt);

      if (c == -1)
	break;

      switch (c)
	{
	case 0:
	  printf ("option %s", long_options[option_index].name);
	  if (optarg)
	    printf (" with arg %s", optarg);
	  printf ("\n");
	  break;

	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	  if (digit_optind != 0 && digit_optind != this_option_optind)
	    printf ("digits occur in two different argv-elements.\n");
	  digit_optind = this_option_optind;
	  printf ("option %c\n", c);
	  break;

	case 'a':
	  printf ("option a\n");
	  break;

	case 'b':
	  printf ("option b\n");
	  break;

	case 'c':
	  printf ("option c with value `%s'\n", optarg);
	  break;

	case 'd':
	  printf ("option d with value `%s'\n", optarg);
	  break;

	case 'v':
	  printf ("option with value `%s'\n", optarg ? optarg : "(null)");
	  break;

	case '?':
	  break;

	default:
	  printf ("?? getopt returned character code 0%o (%c) ??\n", c, c);
	}
    }

  if (optind < argc)
    {
      printf ("non-option ARGV-elements: ");
      while (optind < argc)
	printf ("%s ", argv[optind++]);
      printf ("\n");
    }

  exit (0);
}

#endif /* TEST */

