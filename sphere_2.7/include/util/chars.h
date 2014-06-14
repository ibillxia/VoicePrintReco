/**********************************************************************/
/*                                                                    */
/*             FILENAME:  chars.h                                     */
/*             BY:  Jonathan G. Fiscus                                */
/*                  NATIONAL INSTITUTE OF STANDARDS AND TECHNOLOGY    */
/*                  SPEECH RECOGNITION GROUP                          */
/*                                                                    */
/*           DESC:  This file contains general character defines      */
/*                                                                    */
/**********************************************************************/
#define CHARS_H_VERSION "V1.0"

#define HYPHEN			'-'
#define UND_SCR			'_'
#define NEWLINE			'\n'
#define NULL_CHAR		'\0'
#define SPACE			' '
#define TAB			'\t'
#define LEFT_PAREN		'('
#define RIGHT_PAREN		')'
#define ASTERISK		'*'
#define PERIOD			'.'
#define SLASH			'/'
#define SEMI_COLON		';'
#define DOUBLE_QUOTE		'"'
#define EQUALS			'='

#define COMMENT_CHAR		SEMI_COLON
#define COMMENT_INFO_CHAR	ASTERISK

#define TRUE			1
#define FALSE			0

#define MAX_BUFF_LEN		1024

#define FULL_SCREEN		132
#define SCREEN_WIDTH		79

#define FPNULL			((FILE *) NULL)
#define CNULL			((char *) NULL)
#define INULL			((int *) NULL)
#define LNULL			((long *) NULL)

/* set this to 1 if PCIND_T is a short */
#define PCIND_SHORT 0


#ifdef PEDANTIC
#include <util/proto.h>
#endif
