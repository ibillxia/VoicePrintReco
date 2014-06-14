/* File: sysparams.h */

#ifndef _SYSPARAM_H_
#define _SYSPARAM_H_

#define FUNCTION

#define LINESIZE		16384
#define MAX_INTERNAL_WAVFORM    300000

#define NISTLABEL		"NIST_1A"
#define ENDSTR			"end_head"
#define HDR_ID_SIZE		8
#define HDR_SIZE_SIZE		8
#define PAD_NEWLINES		24
#define PAD_CHAR		' '
#define PAD_MULT		1024

#define TMPEXT			"-"
#define TEMP_DIR		"/tmp"
#define TEMP_BASE_NAME		"spt"


#define SAMPLE_CODING_FIELD	"sample_coding"
#define SAMPLE_COUNT_FIELD	"sample_count"
#define SAMPLE_RATE_FIELD	"sample_rate"
#define SAMPLE_CHECKSUM_FIELD	"sample_checksum"
#define SAMPLE_SIG_BITS_FIELD	"sample_sig_bits"
#define SAMPLE_BF_FIELD		"sample_byte_format"
#define SAMPLE_N_BYTES_FIELD	"sample_n_bytes"
#define CHANNEL_COUNT_FIELD	"channel_count"

#define READMODE "r"
#define UPDATEMODE "r+"
#define WRITEMODE "w"
#define TRUNCATE_UPDATEMODE "w+"

struct fileheader_fixed {
	char header_id[HDR_ID_SIZE];
	char header_size[HDR_SIZE_SIZE];
};

#endif
