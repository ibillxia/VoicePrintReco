/**********************************************************************/
/*                                                                    */
/*             FILENAME:  macros.h                                    */
/*             BY:  Jonathan G. Fiscus                                */
/*                  NATIONAL INSTITUTE OF STANDARDS AND TECHNOLOGY    */
/*                  SPEECH RECOGNITION GROUP                          */
/*                                                                    */
/*           DESC:  This file contains general macros used            */
/*                  throughout the scoring package                    */
/*                                                                    */
/**********************************************************************/
#define strip_newline(_str) \
      { register int _pos; \
        if (_str[_pos=(strlen(_str)-1)] == '\n') \
           _str[_pos] = (char)0; \
      } \

#define skip_white_space(_ptr) \
    while ((*_ptr == ' ') || (*_ptr == '\t')) \
        _ptr++; \

#define char2char(_ptr,_from,_to) \
    { char *_tp; _tp = _ptr; \
      while (*_tp != '\0') {\
         if (*_tp == _from) \
            *_tp = _to; \
         _tp++; \
      } \
    }

/***************************************************************/
/*  move the pointer to the end of the word                    */
/***************************************************************/
#define find_end_of_word(_ptr) \
    while ((*_ptr != ' ') && (*_ptr != '\t') && (*_ptr != (char)0) && \
           (*_ptr != '\n')) \
        _ptr++; 

#define pct(num,dem)	(((dem)==0) ? 0 : (double)(num)/(double)(dem) * 100.0)
#define ROUND(_x) ((int) ((_x) + 0.5))
#define INT(_x) ((int) floor((double) (_x)))
#define TRUNC(_x) (floor((double) (_x)))
#define MOD(_x,_y) ((int) ((_x) - TRUNC((double)(_x)/(double)(_y))*(_y)))
#define DIV(_x,_y) ((int) (_x)/(_y))

#ifndef MAX
#define MAX(_a,_b) (((_a) > (_b)) ? (_a) : (_b))
#endif

#include <util/min.h>

#define swap_short(_x) \
{short *_p=(short *)&_x,_t; _t=*(_p+1); *(_p+1)=*(_p); *(_p)=_t;}
#define swap_bytes(_x) \
{char *_p=(char *)&_x,_t; _t=*(_p+1); *(_p+1)=*(_p); *(_p)=_t;}


#define move_to_null(to) \
    while (*to != NULL_CHAR) \
        to++; 

#define form_feed()	printf("");

/* cause a core dump in a nice manner                          */
#define die() {fflush(stderr); fflush(stdout); abort(); }

/***************************************************************/
/*  make a usable filename from a directory and a filename     */
/***************************************************************/
#define mk_fname(str,dir,name)	sprintf(str,"%s/%s",dir,name);

#ifndef M_LOG10E
#define	M_LOG10E	0.43429448190325182765
#endif

/***************************************************************/
/*    misc external function definitions                       */
/***************************************************************/
#if defined(__STDC__) || defined(__GNUC__) || defined(sgi)
#define PROTO(ARGS)	ARGS
#else
#define PROTO(ARGS)	()
#endif

/* pad.c */    void init_pad_util PROTO((int pr_width)) ;
/* pad.c */    int pad_pr_width PROTO((void)) ;
/* pad.c */    void set_pad PROTO((char *pad, char *str)) ;
/* pad.c */    void set_pad_n PROTO((char *pad, int n)) ;
/* pad.c */    void set_pad_cent_n PROTO((char *pad, int len)) ;
/* pad.c */    char *center PROTO((char *str, int len)) ;

/* string.c */ void search_for_char PROTO((char **ptr, char chr)) ;
/* string.c */ void search_back_for_char PROTO((char *beg_ptr, char **ptr, char chr)) ;
/* string.c */ void strcpy_to_char PROTO((char *to, char *from, char chr)) ;
/* string.c */ void strcpy_to_before_char PROTO((char *to, char *from, char chr)) ;
/* string.c */ void strncpy_to_before_char PROTO((char *to, char *from, char chr, int len)) ;
/* string.c */ void strcpy_lc PROTO((char *to, char *from)) ;
/* string.c */ void strncpy_pad PROTO((char *to, char *from, int len, char chr)) ;
/* string.c */ char *str2up PROTO((char *str)) ;
/* string.c */ char *str2low PROTO((char *str)) ;
/* string.c */ int is_number PROTO((char *str)) ;
/* string.c */ int is_integer PROTO((char *str)) ;
/* string.c */ int strcmp_nhu PROTO((char *s, char *t)) ;

/* sfgets.c */   char *safe_fgets PROTO((char *arr, int len, FILE *fp)) ;

/* comments.c */ int is_comment PROTO((char *str)) ;
/* comments.c */ int is_comment_info PROTO((char *str)) ;

/* word.c */     int wrdlen PROTO((char *ptr)) ;
/* word.c */     void wrdcpy PROTO((char *to, char *from)) ;
/* word.c */     void find_next_word PROTO((char **ptr)) ;

/* itoa.c */     void nist_itoa PROTO((int val, char *buff, int max_len));

/* str_srch.c */ int strings_search PROTO((char **list, int count, char *goal)) ;

/* ds.c       */ void direct_search PROTO((int *IN_psi, int IN_K, double *IN_DELTA, double IN_rho, double *IN_delta, double (*IN_S) (int *), int full_search));
#ifdef __STDC__
/* rsprintf.c */ char *rsprintf PROTO((char *format , ...));
#else
/* rsprintf.c */ char *rsprintf PROTO((va_alist));
#endif

/* dir.c */  int dir_exists PROTO((char *dir)) ;
/* dir.c */  int file_readable PROTO((char *fname)) ;
/* dir.c */  int file_size PROTO((char *fname));

/* string.c */  int is_empty PROTO((char *str)) ;
/* string.c */  void search_for_char PROTO((char **ptr, char chr)) ;
/* string.c */  void search_back_for_char PROTO((char *beg_ptr, char **ptr, char chr)) ;
/* string.c */  void strcpy_to_char PROTO((char *to, char *from, char chr)) ;
/* string.c */  void strcpy_to_before_char PROTO((char *to, char *from, char chr)) ;
/* string.c */  void strncpy_to_before_char PROTO((char *to, char *from, char chr, int len)) ;
/* string.c */  void strcpy_lc PROTO((char *to, char *from)) ;
/* string.c */  void strncpy_pad PROTO((char *to, char *from, int len, char chr)) ;
/* string.c */  char *str2up PROTO((char *str)) ;
/* string.c */  char *str2low PROTO((char *str)) ;
/* string.c */  int is_number PROTO((char *str)) ;
/* string.c */  int is_integer PROTO((char *str)) ;
/* string.c */  char *strstr1_i PROTO((char *ps1, char *ps2)) ;
/* string.c */  char *strstr1 PROTO((char *ps1, char *ps2)) ;
/* t_or_f.c */  char *t_or_f PROTO((int val));



