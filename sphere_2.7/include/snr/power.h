
#ifndef _POWER_H_
#define _POWER_H_

#define NEGATIVE_INFINITY -20.0

/* power.c */	void pwr_compute PROTO((short int *samples, double **pwr, int sample_count, int *pwr_cnt, int win_size, int win_step));
/* power.c */	double pwr1 PROTO((short int *win, int len));

#endif
