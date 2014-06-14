/* File: header.h */
#ifndef _HEADER_H
#define _HEADER_H

#define MAXFIELDS		8000

#define FNULL			((struct field_t *) NULL)
#define FVNULL			((struct field_t **) NULL)
#define HDRNULL			((struct header_t *) NULL)

#define DEFAULT_SAMPLE_CODING   "pcm"
#define DEFAULT_CHANNEL_COUNT   1
#define DEFAULT_SAMPLE_RATE     16000
#define DEFAULT_SAMPLE_BYTE_COUNT     2

#define T_INTEGER		0
#define T_REAL			1
#define T_STRING		2

#define N_STDFIELDS		11

#define ERROR_EXIT_STATUS	1

struct field_t {
	int type;
	char *name;
	char *data;
	int datalen;
	char *comment;
};

struct header_t {
	int fc;
	struct field_t **fv;
};
 
#endif


