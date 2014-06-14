/* 

    file: pmmlcg.h
    Desc: Include file for the random number generator

*/
#include <math.h>
#if defined(__STDC__) || defined(__GNUC__) || defined(sgi)
#define PROTO(ARGS)	ARGS
#else
#define PROTO(ARGS)	()
#endif

/* pmmlcg.c */  void init_seed PROTO((int new_seed, int verbose)) ;
/* pmmlcg.c */  double pmmlcg PROTO((void)) ;
/* pmmlcg.c */  int pmmlcg_newseed PROTO((void)) ;
/* pmmlcg.c */  int pmmlcg_seed PROTO((void)) ;
/* pmmlcg.c */  double exp_dist PROTO((double beta)) ;
/* pmmlcg.c */  double M_erlang PROTO((double beta, int M)) ;
/* pmmlcg.c */  int arbitrary_discrete_rv PROTO((double *prob_list, int num_prob)) ;
