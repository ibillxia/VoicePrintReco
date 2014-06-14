/**********************************************************************/
/*                                                                    */
/*             FILENAME:  mem.h                                       */
/*             BY:  Jonathan G. Fiscus                                */
/*                  NATIONAL INSTITUTE OF STANDARDS AND TECHNOLOGY    */
/*                  SPEECH RECOGNITION GROUP                          */
/*                                                                    */
/*           DESC:  This file contains macros for allocating memore   */
/*                                                                    */
/**********************************************************************/

#ifndef MEMORY_HEADER
#define MEMORY_HEADER

#include <util/mtrf.h>

/**********************************************************************/
/*  Allocation macros                                                 */
/**********************************************************************/

void malloc_died PROTO((int len));

#define MEMORY_H_VERSION "V1.0"

#define alloc_singarr(_arg,_len,_type) \
 if ((_arg=(_type *)mtrf_malloc((_len) * sizeof(_type)))==(_type *)NULL) \
     {malloc_died((_len)*sizeof(_type));}

#define alloc_2dimarr(_arg,_num,_len,_type) \
      { register int _mn; \
        alloc_singarr(_arg,_num,_type *); \
        for(_mn=0;_mn<_num;_mn++) \
            alloc_singarr(_arg[_mn],_len,_type); \
      }

#define alloc_singZ(_arg,_len,_type,_val) \
  { register int _ml; \
    alloc_singarr(_arg,_len,_type); \
    for (_ml=0; _ml<_len; _ml++) \
        _arg[_ml] = _val; \
  }

#define alloc_2dimZ(_arg,_num,_len,_type,_val) \
      { register int _mn; \
        alloc_singarr(_arg,_num,_type *); \
        for(_mn=0;_mn<_num;_mn++) \
            alloc_singZ(_arg[_mn],_len,_type,_val) ; \
      }

#define alloc_3dimZ(_arg,_planes,_num,_len,_type,_val) \
      { register int _mp; \
        alloc_singarr(_arg, _planes, _type **); \
        for(_mp=0;_mp<_planes;_mp++) \
            alloc_2dimZ(_arg[_mp],_num,_len,_type,_val); \
      }

#define alloc_char_singarr(_arg,_len)       alloc_singarr(_arg,_len,char)
#define alloc_char_2dimarr(_arg,_num,_len)  alloc_2dimarr(_arg,_num,_len,char)

#define alloc_short_singarr(_arg,_len)      alloc_singarr(_arg,_len,short)
#define alloc_short_singarr_zero(_arg,_len) alloc_singZ(_arg,_len,short,0)
#define alloc_short_2dimarr(_arg,_num,_le)  alloc_2dimarr(_arg,_num,_le,short)
#define alloc_short_2dimarr_zero(_arg,_num,_le) \
                         alloc_2dimZ(_arg,_num,_le,short,0)

#define alloc_int_singarr(_arg,_len)	    alloc_singarr(_arg,_len,int)
#define alloc_int_2dimarr(_arg,_num,_len)   alloc_2dimarr(_arg,_num,_len,int)

#define alloc_int_singarr_zero(_arg,_len)   alloc_singZ(_arg,_len,int,0)


/**********************************************************************/
/*  Clearing memory macros                                            */
/**********************************************************************/

#define clear_sing(_arg,_len,_val) \
  { register int _ml; \
    for (_ml=0; _ml<_len; _ml++) \
        _arg[_ml] = _val; \
  }

#define clear_2dim(_arg,_num,_len,_val) \
      { register int _mn; \
        for(_mn=0;_mn<_num;_mn++) \
            clear_sing(_arg [_mn],_len,_val) ;\
      }

#define add_to_singarr(_arg,_len,_val) \
  { register int _ml; for (_ml=0; _ml<_len; _ml++) _arg[_ml] += _val; }


#define clear_int_2dimarr(_arg,_x,_y)    clear_2dim(_arg,_x,_y,0)
#define clear_char_2dimarr(_arg,_x,_y)   clear_2dim(_arg,_x,_y,'\0')

/**********************************************************************/
/*  printint memory to stdout macros                                  */
/**********************************************************************/

#define dump_singarr(_a,_x,_fmt,_fp) \
  { register int _nx; \
    for (_nx=0; _nx<_x; _nx++) \
       fprintf(_fp,_fmt,_a[_nx]); \
    fprintf(_fp,"\n"); \
  }

#define dump_2dimarr(_d,_y,_x,_fmt,_fp) \
  { register int _ny; \
    for (_ny=0;_ny<_y;_ny++) \
        dump_singarr(_d[_ny],_x,_fmt,_fp); \
  }



/*******************************************************************/
/*   Free memory macros                                            */
/*******************************************************************/

#define free_singarr(_fl,_ty) \
  { mtrf_free((char *) _fl); \
    _fl=(_ty *)0; \
  }

#define free_2dimarr(_fl,_num,_ty)\
  { register int _ny; \
    for (_ny=0;_ny<_num;_ny++) \
	free_singarr(_fl[_ny], _ty); \
    free_singarr(_fl,_ty *); \
  }

#define free_3dimarr(_fl,_pl,_num,_ty)\
  { register int _np; \
    for (_np=0;_np<_pl;_np++) \
	free_2dimarr(_fl[_np], _num, _ty); \
    free_singarr(_fl,_ty **); \
  }


/******************************************************************/
/* Array Expansion macros                                         */
#define expand_singarr(_a,_n,_max,_fact,_type) \
    {   \
        _type *_tp; \
        alloc_singarr(_tp,(int)((_max) * (_fact)),_type); \
        memcpy(_tp,_a,sizeof(_type) * (_n)); \
        free_singarr(_a,_type); \
        _a = _tp; \
        _max = (int)((_max) * (_fact)); \
    }
 
 
#define expand_1dim(_a,_n,_max,_fact,_type,_upd1) \
    {   \
        _type *_tp; \
        /* printf("        In expand 1dim\n"); */ \
        alloc_singarr(_tp,(int)((_max) * (_fact)),_type); \
        memcpy(_tp,_a,sizeof(_type) * (_n)); \
        free_singarr(_a,_type); \
        _a = _tp; \
        if (_upd1) { _max = (int)((_max) * (_fact)); } \
    }
 
#define expand_1dimZ(_a,_n,_max,_fact,_type,_val,_upd1) \
    {   \
        _type *_tp; \
        alloc_singZ(_tp,(int)((_max) * (_fact)),_type,_val); \
        memcpy(_tp,_a,sizeof(_type) * (_n)); \
        free_singarr(_a,_type); \
        _a = _tp; \
        if (_upd1) { _max = (int)((_max) * (_fact)); } \
    }
 
#define expand_2dimZ(_a,_n1,_max1,_fact1,_n2,_max2,_fact2,_type,_val,_upd2) \
    {   register int _x2 = 0;\
        if ((int)((_max1) * (_fact1)) != _max1){ \
            expand_1dim(_a,_n1,_max1,_fact1,_type *, FALSE); \
            for (_x2=_n1; _x2<(int)((_max1) * (_fact1)); _x2++) { \
                alloc_singZ(_a[_x2],_max2,_type,_val); \
            } \
        } \
        if ((int)((_max2) * (_fact2)) != _max2) { \
            for (_x2=0; _x2<(int)((_max1) * (_fact1)); _x2++){  \
                expand_1dimZ(_a[_x2],_n2,_max2,_fact2,_type,_val,FALSE); \
            } \
        } \
        if (_upd2) { _max1 = (int)((_max1) * (_fact1));   \
                     _max2 = (int)((_max2) * (_fact2)); } \
    }

#endif
