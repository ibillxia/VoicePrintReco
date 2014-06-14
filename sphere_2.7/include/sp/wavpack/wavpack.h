#ifndef _WAVPACK_H_
#define _WAVPACK_H_

#define WAVPACK_MAGIC	"wavpack1.0"

#define RUNL 256		/* max run length <=256 */
				/* compression format flags */
#define BYTE 0			/* 8 bit data format */
#define SHORT 0x20		/* 16 bit data format (else 8 bit) */
#define REV 0x10		/* byte reversed (SHORT only) */
#define OFFSET 0x8		/* 1 byte data offset */
#define DIFF 0x4		/* differential pcm */
#define STEREO 0x2		/* 2 channel */
#define CHAN1 0x1		/* channel nr (0 or 1) */

#define WAV 1			/* modes */
#define UNWAV 2
#define WCAT 3

#if defined(__STDC__) || defined(__GNUC__) || defined(sgi)
#define PROTO(ARGS)	ARGS
#else
#define PROTO(ARGS)	()
#endif

void wavpack_set_monoflg PROTO((int val));
int wavpack_get_monoflg PROTO((void));
void wavpack_set_byteflg PROTO((int val));
int wavpack_get_byteflg PROTO((void));
int wavpack_set_vflg PROTO((int value));
int wavpack_set_oname PROTO((char *s));
void wavpack_set_oflg PROTO((int value));
int wavpack_set_progname PROTO((char *s));
int wavpack_free_progname PROTO((void));
void wavpack_dump_interface PROTO((FILE *fp));
int wavpack_pack PROTO((FOB *ifile, FOB *ofile));
int wavpack_unpack PROTO((FOB *ifile, FOB *ofile));

#endif
