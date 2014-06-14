/**********************************************************************/
/*  File: cfg.h                                                       */
/*  Desc: The include file for definition of the configuration        */
/*        structure.                                                  */
/*  Date: Sept, 1990                                                  */
/**********************************************************************/
#define CFG_H_VERSION "V1.0"

#define NOT_CONFIG_ID	(-1)

/* filename definitions */
#define CFG_ROOT	"ROOT_DIR"
#define CFG_CODESET	"CODESET"
#define CFG_LEX		"LEX"
#define CFG_HOMO	"HOMO"
#define CFG_REF		"REF"
#define CFG_SPLTMRG	"SPLTMRG"
#define CFG_MONO	"MONOSYL"
#define CFG_LSUBS	"LSUBS"
#define CFG_ALP_NUM	"ALP_NUM"
#define CFG_FILE	"CFG"
#define CFG_WWL_FILE    "WWL"

/* print filetype defintions */
#define CFG_SHLEX	"SHLEX"
#define CFG_SHLSUBS	"SHLSUBS"
#define CFG_SHCODESET	"SHCODE"
#define CFG_SHHOMO	"SHHOMO"
#define CFG_SHALP_NUM	"SHALPHA"
#define CFG_SHMONO	"SHMONO"
#define CFG_SHSPLT_MRG	"SHSPLT_MRG"

/* alignment program definitions */
#define CFG_HOMO_ERROR	"HOMO_ERROR"
#define CFG_STR_SIL	"SILENCE"
#define CFG_NO_GR	"NO_GR"
#define CFG_ATIS_FILES	"ATIS"
#define CFG_ATIS3_FILES	"ATIS3"
#define CFG_RM_FILES	"RM"
#define CFG_WSJ_FILES	"WSJ"
#define CFG_SWB_FILES	"SWB"
#define CFG_TIMIT_FILES	"TIMIT"
#define CFG_NTYPE_FILES	"NOTYPE"
#define CFG_SPUID_FILES	"SPU_ID"
#define CFG_NO_ID_FILES	"NO_ID"
#define CFG_USE_PHONE 	"PHONE"
#define CFG_PHDIST_FILE	"DIST_TBL"
#define CFG_OUT_FILE	"OUTFILE"
#define CFG_SYS_NAME	"NAME"
#define CFG_SYS_DESC	"DESC"
#define CFG_ONE_TO_ONE	"ONE2ONE"
#define CFG_ONE_TO_MANY	"ONE2MANY"
#define CFG_SM_RESCORE  "SM"
#define CFG_SM_FOM      "SM_FOM"
#define CFG_FRAG_RESCORE "FRAG"
#define CFG_MRK_FILES    "TMK"
#define CFG_CMRK_FILES   "CTM"
#define CFG_QSCR         "SUM"
#define CFG_QSCR_RAW     "RSUM"
#define CFG_PERMIT_OOCE  "POOCE"

/* statistics program defintions */

#define CFG_TESTNAME	"TEST_NAME"
#define CFG_LPR		"LP"
#define CFG_HDR		"HDR"
#define CFG_MTCHPR	"MTCH_PR"
#define CFG_SENT_MCN	"SENT_MCN"
#define CFG_COUNT	"DUMP_COUNT"
#define CFG_LONG_SEG	"SEG_LONG"
#define CFG_AVE_SEG	"SEG_AVE"
#define CFG_ANOVAR	"ANOVAR"
#define CFG_SIGNTEST    "SIGNTEST"
#define CFG_WILCOXON    "WILCOXON"
#define CFG_2_SAMP      "SAMP_Z"
#define CFG_SIG         "SIG"
#define CFG_RANGE	"RANGE"
#define CFG_TROFF	"TROFF"
#define CFG_MINGOOD	"MIN_GOOD"
#define CFG_PCT_FORMULA	"PCT_FORM"
#define CFG_VERBOSE     "V"


/* unique scoring defintions */
#define CFG_OVR_SCR	"OVRALL"
#define CFG_OVR_RAW	"OVRRAW"
#define CFG_OVR_DTL	"OVRDTL"
#define CFG_SPKR_SCR	"SPKR"
#define CFG_SENT_SCR	"SENT"
#define CFG_OUT_DIR	"OUT_DIR"

/* general difinitions */
#define CFG_DUMPARG	"DUMP"
#define CFG_ALI_FILE	"ALIGN"
#define CFG_HYP_FILE	"HYP"
#define CFG_ALL_SENT	"ALL"
#define CFG_DIFF_SENT	"DIFF"
#define CFG_DIFF_SCORE	"SDIFF"
#define CFG_ERRORS	"ERRORS"

#define MAX_CONFIG_REC	40
#define MAX_CONFIG_LENGTH	160
#define TOGGLE_ON	1
#define TOGGLE_OFF	0
#define TOGGLE_ON_CHR   "on"
#define TOGGLE_OFF_CHR  "off"


#define CFG_TGL	0
#define CFG_C	1
#define CFG_C2	2
#define CFG_STR	3

#define NO_GRP	0

typedef struct config_record_struct{
    char *intern_id;
    int num_elem;
    int value_type;
    int flag_group;
    char *value;
} CONFIG_REC;

typedef struct config_struct{
    CONFIG_REC rec_list[MAX_CONFIG_REC];
} CONFIG;

#if defined(__STDC__) || defined(__GNUC__) || defined(sgi)
#define PROTO(ARGS)	ARGS
#else
#define PROTO(ARGS)	()
#endif

void set_cfg_debug PROTO((int)) ;
void alloc_CONFIG PROTO((CONFIG *)) ;
void cfg_get_args PROTO((CONFIG *, int, char **, char *)) ;
void load_com_line_CONFIG PROTO((CONFIG *cfg, int argc, char **argv, 
				 int silent)) ;
void load_CONFIG PROTO((CONFIG *cfg, char *file, int silent)) ;
void reset_group PROTO((CONFIG *cfg, int cfg_ind)) ;
char * CONFIG_elem_C PROTO((CONFIG *cfg, char *str)) ;
int CONFIG_elem_TGL PROTO((CONFIG *cfg, char *str)) ;
char ** CONFIG_elem_C2 PROTO((CONFIG *cfg, char *str)) ;
void set_CONFIG_elem_TGL PROTO((CONFIG *cfg, char *str, int value)) ;
void set_CONFIG_elem_C PROTO((CONFIG *cfg, char *str, char *value)) ;
void set_CONFIG_elem_STR PROTO((CONFIG *cfg, char *str, char *value)) ;
int CONFIG_elem_count PROTO((CONFIG *cfg, char *str)) ;
int CONFIG_elem_present_C2 PROTO((CONFIG *cfg, char *str)) ;
int is_CONFIG_id PROTO((CONFIG *cfg, char *id)) ;
void dump_CONFIG PROTO((CONFIG *cfg)) ;
void print_usage_CONFIG PROTO((CONFIG *cfg, char *prog_name)) ;
char * CONFIG_elem_STR PROTO((CONFIG *cfg, char *str)) ;
int is_CONFIG_set PROTO((CONFIG *cfg, char *str, int verbose)) ;
int is_CONFIG_group_set PROTO((CONFIG *cfg, int grp_ind, int verbose)) ;
int is_CONFIG_ind_set PROTO((CONFIG *cfg, int ind, int verbose)) ;
