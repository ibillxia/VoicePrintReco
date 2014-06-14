
/*
 *
 * The Error return utility accessor
 *
 *
 */

#define RETURN_TYPE_ERROR    1
#define RETURN_TYPE_WARNING  2
#define RETURN_TYPE_SUCCESS  3
#define RETURN_TYPE_CHILD    4

#if defined(__STDC__) || defined(__GNUC__) || defined(sgi)
#define PROTO(ARGS)	ARGS
#else
#define PROTO(ARGS)	()
#endif

void set_error_util_debug PROTO((int n)) ;
void print_return_status PROTO((FILE *fp)) ;
int return_status PROTO((void)) ;
void set_return_util PROTO((char *proc_name, int return_code, char *mesg, int type)) ;
char *get_return_status_message PROTO((void));
int return_type PROTO((void));
char *get_subordinated_message PROTO((void));

#define return_err(_proc,_code,_value,_mesg) \
        { set_return_util(_proc,_code,_mesg,RETURN_TYPE_ERROR) ; return(_value); }

#define return_success(_proc,_code,_value,_mesg) \
        { set_return_util(_proc,_code,_mesg,RETURN_TYPE_SUCCESS) ; return(_value); }

#define return_warn(_proc,_code,_value,_mesg) \
        { set_return_util(_proc,_code,_mesg,RETURN_TYPE_WARNING) ; \
	  return(_value); }

#define return_child(_proc,_type, _func) \
        { _type _code; _code = (_type)_func; \
	  set_return_util(_proc,-1,"",RETURN_TYPE_CHILD) ; \
	  return(_code); }

