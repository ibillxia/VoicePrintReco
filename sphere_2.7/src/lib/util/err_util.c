#include <util/utillib.h>

#define ERROR_UTIL_MESSAGE_LEN 200
static char static_error_util_proc_name[ERROR_UTIL_MESSAGE_LEN];
static char static_error_util_message[ERROR_UTIL_MESSAGE_LEN];
static char static_error_util_message1[ERROR_UTIL_MESSAGE_LEN];
static char static_error_util_buffer[ERROR_UTIL_MESSAGE_LEN];
static static_error_util_return_code;
static static_error_util_return_type;

static int err_util_dbg = 0;

void set_error_util_debug(int n)
{
    err_util_dbg=n;
}

void print_return_status(FILE *fp)
{  
    char *offset="";
    fprintf(fp,"Procedure: %s\n",static_error_util_proc_name);
    fprintf(fp,"%s   Status Code:     %d\n",offset,static_error_util_return_code);
    fprintf(fp,"%s   Status Type:     ",offset);
    switch(static_error_util_return_type){
      case RETURN_TYPE_WARNING: fprintf(fp,"Warning\n"); break;
      case RETURN_TYPE_SUCCESS: fprintf(fp,"Success\n"); break;
      case RETURN_TYPE_ERROR:   fprintf(fp,"Error\n"); break;
      default:  fprintf(fp,"UNKNOWN\n"); break;
    }
    fprintf(fp,"%s   Message:         %s\n",offset,static_error_util_message);
}

int return_status(void)
{
    return(static_error_util_return_code);
}

int return_type(void)
{
    return(static_error_util_return_type);
}

char *get_subordinated_message(void){
    int l=0; 
    char *type, num[1024];
    strncpy(static_error_util_buffer,static_error_util_proc_name,
	    ERROR_UTIL_MESSAGE_LEN);
    l = strlen(static_error_util_buffer);

    if (l < ERROR_UTIL_MESSAGE_LEN)
	switch(static_error_util_return_type){
	  case RETURN_TYPE_WARNING: type = " Warning "; break;
	  case RETURN_TYPE_SUCCESS: type = " Success "; break;
	  case RETURN_TYPE_ERROR:   type = " Error "; break;
	  default:                  type = " UNKNOWN "; break;
	}
	strncat(static_error_util_buffer, type,ERROR_UTIL_MESSAGE_LEN-l);
    l = strlen(static_error_util_buffer);
    
    if (l < ERROR_UTIL_MESSAGE_LEN){
	sprintf(num,"Code: %d Message: ",static_error_util_return_code);
	strncat(static_error_util_buffer, num,ERROR_UTIL_MESSAGE_LEN-l);
	l = strlen(static_error_util_buffer);
    }

    if (l < ERROR_UTIL_MESSAGE_LEN)
	strncat(static_error_util_buffer,static_error_util_message,
		ERROR_UTIL_MESSAGE_LEN);
    if (*(static_error_util_buffer + strlen(static_error_util_buffer)-1)=='\n')
	*(static_error_util_buffer + strlen(static_error_util_buffer)-1)= '\0';
    return(static_error_util_buffer);
}

char *get_return_status_message(void){
    strncpy(static_error_util_buffer,static_error_util_message,
	    ERROR_UTIL_MESSAGE_LEN);
    if (*(static_error_util_buffer + strlen(static_error_util_buffer)-1)=='\n')
	*(static_error_util_buffer + strlen(static_error_util_buffer)-1)= '\0';
    return(static_error_util_buffer);
}

void set_return_util(char *proc_name, int return_code, char *mesg, int type)
{
    if (type != RETURN_TYPE_CHILD){
	strncpy(static_error_util_proc_name,proc_name,ERROR_UTIL_MESSAGE_LEN);
	strncpy(static_error_util_message,mesg,ERROR_UTIL_MESSAGE_LEN);
	static_error_util_return_code = return_code;
	static_error_util_return_type = type;
    } else {
	sprintf(static_error_util_message1,"Child '%s' returns message '%s'",
		static_error_util_proc_name,static_error_util_message);
	strncpy(static_error_util_message,static_error_util_message1,ERROR_UTIL_MESSAGE_LEN);
	strncpy(static_error_util_proc_name,proc_name,ERROR_UTIL_MESSAGE_LEN);
    }
    
    if (err_util_dbg) {
	fprintf(stderr,"set_return_util: proc_name %s code %d message %s type %d\n",
		static_error_util_proc_name,
		static_error_util_return_code,
		static_error_util_message,
		static_error_util_return_type);
    }
}
    
