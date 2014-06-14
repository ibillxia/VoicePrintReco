
#ifndef DISKARR_HEADER
#define DISKARR_HEADER

typedef short DISK_T;

#define init_val (DISK_T)0

/* diskarr.c */ void init_disk_array PROTO((char *filename, int x, int y, int dbg));
/* diskarr.c */ void close_disk_array PROTO((void));
/* diskarr.c */ void check_arg PROTO((int x, int y));
/* diskarr.c */ void set_disk_element PROTO((int x, int y, DISK_T value));
/* diskarr.c */ void get_disk_element PROTO((int x, int y, DISK_T *value));
/* diskarr.c */ void print_disk_array PROTO((FILE *fp));
/* diskarr.c */ void disk_compute_entropy PROTO((int dbg, double *entropy_out));


#endif
