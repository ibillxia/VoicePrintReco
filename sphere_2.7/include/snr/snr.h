
#ifndef _SNR_H_
#define _SNR_H_

#include <snr/filter.h>
#include <snr/power.h>

#define MILI_SEC 20.0
#define PEAK_LEVEL 0.95

#define BINS 500
#define SMOOTH_BINS 7
#define CODEC_SMOOTH_BINS 15
#define LOW -28.125
#define HIGH 96.875

#define BLOCKSIZE 2048


/* snr.c */	double compute_dc_bias PROTO((SP_FILE *fp, SPFILE_CHARS *sp_char, int channel));
/* snr.c */	int snr_all_channels PROTO((SP_FILE *fp, double ***signal, double ***noise, double ***snrs, double ***cross, Filterbank *filters, int codec, int graph, char *filename, double pct_speech_level));
/* snr.c */	int quick_snr_all_channels PROTO((SP_FILE *fp, double ***signal, double ***noise, double ***snrs, Filterbank *filters, int graph, char *filename));
/* snr.c */	int snr_single_channel PROTO((SP_FILE *sp, int channel, double *noise_level, double *speech_level, double *sig_to_noise_ratio, double *cross_level, int do_quick_snr, double *coeff, int taps, int codec, int graph, char *filename, double pct_speech_level));
/* snr.c */	int segsnr_single_channel PROTO((SP_FILE *sp, int channel, double *noise_level, double *speech_level, double *sig_to_noise_ratio, int **list, int segments, double *coeff, int taps));
/* snr.c */	int compute_pwr_hist_sd PROTO((SP_FILE *fp, HIST **pwr_hist, int num_bins, SPFILE_CHARS *sp_char, int channel, int frame_width, int frame_adv, double *coeff, int taps));
/* snr.c */	void snr PROTO((HIST **full_hist, int num_bins, double cutoff_percentile, double *noise_lvl, double *speech_lvl, double *cross_lvl, int codec, int graph, char *filename));
/* snr.c */	void  pick_peaks PROTO((HIST **full_hist, double *noise_lvl, double *cross_lvl, double *speech_lvl));
/* snr.c */	void build_raised_cos_hist PROTO((HIST **ref_hist, HIST **ret_hist, int num_bins, double *noise_peak));
/* snr.c */	double comp1 PROTO((int *vector));
/* snr.c */	void do_init_comp1 PROTO((HIST **ref, HIST **hyp, int num_bins));
/* snr.c */	double pick_center PROTO((HIST **h, int bin));
/* snr.c */	void special_cosine_hist PROTO((HIST **hist, int num_bins, int middle, int height, int width));


#endif
