
#ifndef _WAVIO_H_
#define  _WAVIO_H_
/* wav_io.c */	void clear_waveform_read PROTO((void));
/* wav_io.c */	void init_waveform_read PROTO((SP_FILE *fp, int buff_len, SPFILE_CHARS *sp_char));
/* wav_io.c */	int wave_read_1channel PROTO((char *out_arr, int channel));
/* wav_io.c */	int wave_read_2channel PROTO((char *out0, char *out1));
/* wav_io.c */	void shift_short_up PROTO((short int *to, short int *from, int num));

#endif
