
#ifndef _SEGMENT_H_
#define _SEGMENT_H_

#define FROM 0
#define TO 1

#define MAX_SEGMENT 500

/* segment.c */	int concatenate_speech PROTO((SP_FILE *in_file, int **list, int segments, SP_FILE *out_file));
/* segment.c */	void hamming_window PROTO((short int *data, int size, int start));
/* segment.c */	int window_size PROTO((int sample_rate, double miliseconds));
/* segment.c */	double dc_offset  PROTO((SP_FILE *sp));
/* segment.c */	double rms_power  PROTO((short int *buffer, int start, int size, double offset));
/* segment.c */ int segment_speech(SP_FILE *sp, double speech, double noise, int *max_segment, int ***list) ;
#endif
