/* speech_det.h -- global stuff for speech detection */

#ifndef _SPEECH_DET_H_
#define _SPEECH_DET_H_

#define SD_INIT_SIL		0
#define SD_START_LOW		1
#define SD_START_LOW_DROP	2
#define SD_START_HIGH		3
#define SD_START_LOW2		4
#define SD_START_DROP		5
#define SD_SPEAKING		6
#define SD_FINISHING		7
#define SD_FIN_SPIKE		8
#define SD_FIN_SP_DROP		9
#define SD_DONE			10


/* speech_det.c */	int do_speech_detect PROTO((double r0, int fr_num));
/* speech_det.c */	void set_speech_params PROTO((double start_low, double start_high, double end_low, double end_high));
/* speech_det.c */	void force_speech_detect_done PROTO((int fr_num));
/* speech_det.c */	int speech_begin PROTO((void)   /* return the frame when speech last began */);
/* speech_det.c */	int speech_end PROTO((void) /* return the frame when speech last ended */);
/* speech_det.c */	void init_speech_detect PROTO((int fr_num));

#endif
