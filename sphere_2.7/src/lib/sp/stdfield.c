
/* LINTLIBRARY */

/* File: stdfield.c */

#include <stdio.h>

#define SPHERE_LIBRARY_CODE
#include <sp/sphere.h>

char *std_fields[] = {
	"database_id",
	"database_version",
	"utterance_id",
	"channel_count",
	"sample_count",
	"sample_rate",
	"sample_min",
	"sample_max",
	"sample_n_bytes",
	"sample_byte_format",
	"sample_sig_bits",
	CNULL
};
