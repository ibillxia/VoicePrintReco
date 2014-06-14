typedef struct fob_t {
	FILE * fp;		/* if not NULL, use stdio.h functions */
        int read_byte_swap;     /* if true, swap after read */
        int write_byte_swap;    /* if true, swap after write */
	int length;		/* #bytes read or written */
	char * buf;		/* allocated buffer pointer */
	char * buf_swap;        /* if the write operation requries byte*/
	                        /* swapping, it's done here */
	int bufsize;		/* allocated buffer size */
	char * pos;		/* current position in buffer */
} FOB;


#define FOB_BUF_GRAN	8192
#define FOBPNULL	( (FOB *) NULL )

#if defined(__STDC__) || defined(__GNUC__) || defined(sgi)
#define PROTO(ARGS)	ARGS
#else
#define PROTO(ARGS)	()
#endif

FOB * fob_create PROTO((FILE *fp)) ;
int fob_destroy PROTO((FOB *fobp)) ;
int fob_fflush PROTO((FOB *fobp)) ;
int fob_create2 PROTO((FILE *fpin, FILE *fpout, FOB **fobpin, FOB **fobpout)) ;
int fob_destroy2 PROTO((FOB *fobpin, FOB *fobpout)) ;
void fob_read_byte_swap PROTO((FOB *f)) ;
void fob_write_byte_swap PROTO((FOB *f)) ;
void fob_read_byte_natural PROTO((FOB *f));
void fob_write_byte_natural PROTO((FOB *f));
void fob_bufinit PROTO((FOB *f, char *buf, int len)) ;
void fob_rewind PROTO((FOB *f)) ;
int fob_ftell PROTO((FOB *f)) ;
int fob_is_fp PROTO((FOB *f)) ;
int fob_flush_to_fp PROTO((FOB *f, FILE *fp)) ;
int fob_bufcleanup PROTO((FOB *f, char **buf, int *len)) ;
int fob_bufput PROTO((FOB *fobp, char *p, int len)) ;
int fob_bufget PROTO((FOB *fobp, char *p, int len)) ;
void buffer_swap_bytes PROTO((char *mem, int blen)) ;
void copy_buffer_swap_bytes PROTO((char *to, char *from, int blen)) ;
int fob_fread PROTO((char *p, int size, int nitems, FOB *fobp)) ;
int fob_fwrite PROTO((char *p, int size, int nitems, FOB *fobp)) ;
int fob_putc PROTO((char c, FOB *fobp)) ;
int fob_getc PROTO((FOB *fobp)) ;
int fob_getw PROTO((FOB *fobp)) ;
int fob_ferror PROTO((FOB *fobp)) ;
int fob_feof PROTO((FOB *fobp)) ;
int fob_fseek PROTO((FOB *f, long len, int offset));
int fob_fclose PROTO((FOB *fob));
FOB *fob_fopen PROTO((char *name, char *mode));

/*
 * lseek & access args
 *
 * SEEK_* have to track L_* in sys/file.h & SEEK_* in 5include/stdio.h
 * ?_OK have to track ?_OK in sys/file.h
 */
#ifndef SEEK_SET
#define SEEK_SET        0       /* Set file pointer to "offset" */
#define SEEK_CUR        1       /* Set file pointer to current plus "offset" */
#define SEEK_END        2       /* Set file pointer to EOF plus "offset" */
#endif
