/**********************************************************************/
/*                                                                    */
/*             FILENAME:  cfg.c                                       */
/*             BY:  Jonathan G. Fiscus                                */
/*                  NATIONAL INSTITUTE OF STANDARDS AND TECHNOLOGY    */
/*                  SPEECH RECOGNITION GROUP                          */
/*                                                                    */
/*           DESC:  This file contains utilities to load and          */
/*                  manipulate a config struct, It handles command    */
/*                  line arguments and configuration files            */
/**********************************************************************/

#include <util/utillib.h>

#define CFG_VERSION "V1.0"

static int cfg_dbg=FALSE;

/****************************************************************/
/*   Enable or disable debugging of the cfg utility             */
/****************************************************************/
void set_cfg_debug(int value)
{
    cfg_dbg=value;
}

/****************************************************************/
/*   Allocate memory requirements for the CONFIG struct.  When  */
/*   the struct is created (by the main routine) maximum        */
/*   element counts are defined.                                */
/****************************************************************/
void alloc_CONFIG(CONFIG *cfg)
{
    int cfg_ind;
    char **t_arr;

    for (cfg_ind=0 ;*(cfg->rec_list[cfg_ind].intern_id) != NULL_CHAR; cfg_ind++)
        /* based on the type, allocate memory for them */
        switch (cfg->rec_list[cfg_ind].value_type){
            case CFG_C:
            case CFG_STR:
                 alloc_singZ(cfg->rec_list[cfg_ind].value,
                               MAX_CONFIG_LENGTH,char,NULL_CHAR);
                 break;
            case CFG_C2:
                 alloc_2dimZ(t_arr, cfg->rec_list[cfg_ind].num_elem,
                                  MAX_CONFIG_LENGTH,char,NULL_CHAR);
                 cfg->rec_list[cfg_ind].value = (char *)t_arr;
                 break;
            case CFG_TGL:
		 cfg->rec_list[cfg_ind].value = TOGGLE_OFF_CHR;
                 break;
            default:
                 fprintf(stderr,"Error: Unable to allocate CONFIG %d\n",
                        cfg->rec_list[cfg_ind].value_type);
                 exit(-1);
                 break;
	}
}

/*************************************************************/
/*  general routine to load the command line and a config    */
/*  file                                                     */
void cfg_get_args(CONFIG *cfg, int argc, char **argv, char *setup_config)
{
    
    /* load the initialization file common to all */
    if (cfg_dbg) fprintf(stdout,"Loading setup config file %s\n",setup_config);
    if (*setup_config != NULL_CHAR){
        if (cfg_dbg) fprintf(stdout,"Not loading NULL setup config file\n");
	load_CONFIG(cfg,setup_config,/*silent=*/TRUE && !cfg_dbg);
    }
    if (cfg_dbg) dump_CONFIG(cfg);

    /* load the command line arguments from the command line */
    if (cfg_dbg) fprintf(stdout,"Loading command line arguments\n");
    load_com_line_CONFIG(cfg,argc,argv,/* silent = */TRUE && !cfg_dbg);
    if (cfg_dbg) dump_CONFIG(cfg);

    /* if there was a config file on the command line */
    { char *cfg_file;
      cfg_file = CONFIG_elem_C(cfg,CFG_FILE);
      if (*cfg_file != NULL_CHAR){
          load_CONFIG(cfg,cfg_file,/*silent=*/TRUE);
          /* make sure the command line takes precedence */
          load_com_line_CONFIG(cfg,argc,argv,/*silent=*/FALSE);
      } else /* this makes the error messages visible if there is no */
             /* new configuration file */
          load_com_line_CONFIG(cfg,argc,argv,/*silent=*/FALSE);
    }
}

/****************************************************************/
/*   Given argv and argc, load the arguments into the CONFIG    */
/*   structure.                                                 */
/****************************************************************/
void load_com_line_CONFIG(CONFIG *cfg, int argc, char **argv, int silent)
{
    int arg, cfg_ind;

    for (arg=1; arg < argc;){
        if ((cfg_ind = is_CONFIG_id(cfg,argv[arg]+1)) != NOT_CONFIG_ID){
            switch (cfg->rec_list[cfg_ind].value_type){
                case CFG_TGL:
                    cfg->rec_list[cfg_ind].value = TOGGLE_ON_CHR;
                    arg++;
                    break;
                case CFG_C:
                case CFG_STR:
                    arg++;
                    if (arg >= argc){
                        char *de;
                        de = "FILE" ; if (cfg->rec_list[cfg_ind].value_type==CFG_STR)
                                          de = "STR";
                        fprintf(stderr,"Error: %s type for %s ",de,(argv[arg-1]+1));
                        fprintf(stderr,"requires an assigned value\n");
                        exit(-1);
                    }
                    strcpy(cfg->rec_list[cfg_ind].value,argv[arg]);
                    arg++;
                    break;
                case CFG_C2:{
                    char **t_arr;
                    int arg_count = 0, stop = 0;
 
                    t_arr = (char **)cfg->rec_list[cfg_ind].value;
                    arg++;
                    while ((arg < argc) && (stop == 0)){
                        if ((arg_count < cfg->rec_list[cfg_ind].num_elem) &&
                            (*argv[arg] != HYPHEN))
                            strcpy(t_arr[arg_count++],argv[arg++]);
                        else
                           stop = 1;
                    }
                    if (arg < argc){
                      if ((arg_count == cfg->rec_list[cfg_ind].num_elem) &&
                        (*argv[arg] != HYPHEN))
                      if (!silent){
                       fprintf(stderr,"Warning: Too many Com Line elements");
                       fprintf(stderr," for %s\n",
                                      cfg->rec_list[cfg_ind].intern_id);
                      }
                    }
                    break;
		}
                default:
                    fprintf(stderr,"Error: unable to store CONFIG value");
                    fprintf(stderr," of type %d\n",
                           cfg->rec_list[cfg_ind].value_type);
                    exit(-1);
                    break;
	    }
            reset_group(cfg,cfg_ind);
	}
        else{
            if (!silent)
                fprintf(stderr,"Warning: Unknown command line argument %s\n",
                           argv[arg]);
            arg++;
	}
    }
}

/****************************************************************/
/*   Given a file (of configuration arguments, load them into   */
/*   the config structure                                       */
/****************************************************************/
void load_CONFIG(CONFIG *cfg, char *file, int silent)
{
   FILE *fp;
   char buff[MAX_BUFF_LEN], id[MAX_BUFF_LEN], *buff_ptr;
   int cfg_ind;

   if ((*(file) == NULL_CHAR) || ((fp=fopen(file,"r")) == NULL)){
       fprintf(stderr,"Warning: Config file %s is unreadable\n",file);
       return;
   }

   while (safe_fgets(buff,MAX_BUFF_LEN,fp) != CNULL)
       if (!is_comment(buff) && !is_comment_info(buff)){
           buff_ptr = buff;
           strip_newline(buff_ptr);
           wrdcpy(id,buff_ptr);
           if ((cfg_ind = is_CONFIG_id(cfg,id)) != NOT_CONFIG_ID){
               find_next_word(&buff_ptr);
               switch (cfg->rec_list[cfg_ind].value_type){
                   case CFG_TGL:
                       cfg->rec_list[cfg_ind].value = (char *)TOGGLE_ON_CHR;
                       break;
                   case CFG_C:
                       wrdcpy(cfg->rec_list[cfg_ind].value,buff_ptr);
                       break;
                   case CFG_C2:{
                       char **t_arr;
                       int arg_count = 0;

                       t_arr = (char **)cfg->rec_list[cfg_ind].value;

                       while ((*buff_ptr != NULL_CHAR) &&
                              (arg_count < cfg->rec_list[cfg_ind].num_elem)){
                           wrdcpy(t_arr[arg_count++],buff_ptr);
                           find_next_word(&buff_ptr);
                       }
                       if ((arg_count == cfg->rec_list[cfg_ind].num_elem) &&
                           (*buff_ptr != NULL_CHAR))
                        if (!silent){
                         fprintf(stderr,"Warning: Ignored Config elements\n");
                         fprintf(stderr,"         %s of\n         %s\n",
                                         buff_ptr,buff);
			}
                       break;
		   }
                   case CFG_STR:
                       strcpy(cfg->rec_list[cfg_ind].value,buff_ptr);
                       break;
                   default:
                       fprintf(stderr,"Error: Unknown CONFIG type %d\n",
                             cfg->rec_list[cfg_ind].value_type);
                       exit(-1);
                       break;
	       }
               reset_group(cfg,cfg_ind);
	   }
           else if (!silent)
              fprintf(stderr,"Warning: Configuration not usable - %s\n",buff);
       }
   fclose(fp);
}

/****************************************************************/
/*   If the indexed config element is part of a group, erase    */
/*   all other arguments in that group                          */
/****************************************************************/
void reset_group(CONFIG *cfg, int cfg_ind)
{
    int i=0, ind;
    char **t_arr;

    if (cfg_dbg) printf("CFG: reset_group id: %d for flag %s\n",
                        cfg->rec_list[cfg_ind].flag_group,
                        cfg->rec_list[cfg_ind].intern_id);

    /* check to see if this element is part of a grouping */
    if (cfg->rec_list[cfg_ind].flag_group == NO_GRP)
        return;

    /* otherwise, erase all the other values for it's grouping */
    while (*(cfg->rec_list[i].intern_id) != NULL_CHAR){
        if ((i != cfg_ind) && (cfg->rec_list[cfg_ind].flag_group ==
                               cfg->rec_list[i].flag_group))
            switch (cfg->rec_list[i].value_type){
                   case CFG_TGL:
                       cfg->rec_list[i].value = (char *)TOGGLE_OFF_CHR;
                       break;
                   case CFG_C:
                   case CFG_STR:
                       *(cfg->rec_list[i].value) = NULL_CHAR;
                       break;
                   case CFG_C2:
                       t_arr = (char **)cfg->rec_list[i].value;
                       for (ind=0; ind < cfg->rec_list[i].num_elem; ind++)
                           *(t_arr[ind]) = NULL_CHAR;
                       break;
                   default:
                       fprintf(stderr,"Error: Unknown CONFIG type %d\n",
                             cfg->rec_list[i].value_type);
                       exit(-1);
                       break;
	    }
        i++;
    }

}

/****************************************************************/
/*  Return the CFG_C value based on the id string               */
/****************************************************************/
char * CONFIG_elem_C(CONFIG *cfg, char *str)
{
    int cfg_ind;

    if ((cfg_ind = is_CONFIG_id(cfg,str)) != NOT_CONFIG_ID)
        return (cfg->rec_list[cfg_ind].value);
    fprintf(stderr,"Error: Unknown Config Paramater Requested %s\n",str);
    exit(-1);
}

/****************************************************************/
/*  Return the CFG_STR value based on the id string             */
/****************************************************************/
char * CONFIG_elem_STR(CONFIG *cfg, char *str)
{
    return(CONFIG_elem_C(cfg,str));
}

/****************************************************************/
/*  Return the CFG_TGL value based on the id string             */
/****************************************************************/
int CONFIG_elem_TGL(CONFIG *cfg, char *str)
{
    int cfg_ind;

    if ((cfg_ind = is_CONFIG_id(cfg,str)) != NOT_CONFIG_ID)
        return (strcmp(cfg->rec_list[cfg_ind].value,TOGGLE_ON_CHR) == 0
		? TOGGLE_ON : TOGGLE_OFF);
    fprintf(stderr,"Error: Unknown Config Paramater Requested %s\n",str);
    exit(-1);
}

/****************************************************************/
/*  Return the CFG_C2 value based on the id string              */
/****************************************************************/
char ** CONFIG_elem_C2(CONFIG *cfg, char *str)
{
    int cfg_ind;

    if ((cfg_ind = is_CONFIG_id(cfg,str)) != NOT_CONFIG_ID)
        return ((char **)cfg->rec_list[cfg_ind].value);
    fprintf(stderr,"Error: Unknown Config Paramater Requested %s\n",str);
    exit(-1);
}

/****************************************************************/
/*  Set the value for the CFG_TGL value based on the id string  */
/****************************************************************/
void set_CONFIG_elem_TGL(CONFIG *cfg, char *str, int value)
{
    int cfg_ind;

    if ((cfg_ind = is_CONFIG_id(cfg,str)) == NOT_CONFIG_ID){
        fprintf(stderr,"Error: Unknown Config Paramater Set Requested %s\n",
                       str);
        exit(-1);
    }
    cfg->rec_list[cfg_ind].value = (value == TOGGLE_ON) ? TOGGLE_ON_CHR : TOGGLE_OFF_CHR;
}

/****************************************************************/
/*  Set the value for the CFG_C value based on the id string    */
/****************************************************************/
void set_CONFIG_elem_C(CONFIG *cfg, char *str, char *value)
{
    set_CONFIG_elem_STR(cfg,str,value);
}

/****************************************************************/
/*  Set the value for the CFG_STR value based on the id string  */
/****************************************************************/
void set_CONFIG_elem_STR(CONFIG *cfg, char *str, char *value)
{
    int cfg_ind;

    if ((cfg_ind = is_CONFIG_id(cfg,str)) == NOT_CONFIG_ID){
        fprintf(stderr,"Error: Unknown Config Paramater Set Requested %s\n",
                       str);
        exit(-1);
    }
    strcpy(cfg->rec_list[cfg_ind].value,value);
}

/****************************************************************/
/*  Return the element count for the id string                  */
/****************************************************************/
int CONFIG_elem_count(CONFIG *cfg, char *str)
{
    int ind;

    if ((ind = is_CONFIG_id(cfg,str)) == NOT_CONFIG_ID){
        fprintf(stderr,"Error: Invalid id string to find CONFIG ");
        fprintf(stderr,"element count\n");
        exit(-1);
    }
    return(cfg->rec_list[ind].num_elem);
}

/****************************************************************/
/*  Return the element count for the id string                  */
/****************************************************************/
int CONFIG_elem_present_C2(CONFIG *cfg, char *str)
{
    int ind, i;
    char **t_arr;

    if ((ind = is_CONFIG_id(cfg,str)) == NOT_CONFIG_ID){
        fprintf(stderr,"Error: Invalid id string to find CONFIG ");
        fprintf(stderr,"element present count\n");
        exit(-1);
    }
    t_arr = (char **)cfg->rec_list[ind].value;

    for (i=0; i<cfg->rec_list[ind].num_elem; i++)
        if (t_arr[i][0] == NULL_CHAR)
           return(i);
    return(cfg->rec_list[ind].num_elem);
}

/****************************************************************/
/*  Return the element index for the id string                  */
/****************************************************************/
int is_CONFIG_id(CONFIG *cfg, char *id)
{
    int i=0;

    while (*(cfg->rec_list[i].intern_id) != NULL_CHAR){
        if (strcasecmp(id,cfg->rec_list[i].intern_id) == 0)
            return(i);
        i++;
    }
    return(NOT_CONFIG_ID);
}

/****************************************************************/
/*  Dump the config structure to stdout                         */
/****************************************************************/
void dump_CONFIG(CONFIG *cfg)
{
    int i=0;

    printf("Dump of the Configuration arguments and their values\n\n");
    printf("      Name       # elem  type    value\n");
    while (*(cfg->rec_list[i].intern_id) != NULL_CHAR){
       printf(" %15s  %3d    ",cfg->rec_list[i].intern_id,
             cfg->rec_list[i].num_elem);
       switch (cfg->rec_list[i].value_type){
           case CFG_TGL:
              printf("TGL    ");
              if (strcmp(cfg->rec_list[i].value,TOGGLE_ON_CHR) != 0)
                  printf(" OFF\n");
              else
                  printf(" ON\n");
              break;
           case CFG_STR:
              printf("STR    ");
              printf(" %s\n",(char *)cfg->rec_list[i].value);
              break;
           case CFG_C:
              printf("FILE   ");
              printf(" %s\n",(char *)cfg->rec_list[i].value);
              break;
           case CFG_C2:{
              char **chr;
              int ind = 0;
              printf("N_STR  ");
              chr = (char **)cfg->rec_list[i].value;
              if (chr == NULL){
                  printf("\n");
                  break;
	      }
              printf(" %s\n",chr[ind]);
              for (ind = 1;ind < cfg->rec_list[i].num_elem; ind++)
                  if (*(chr[ind]) != NULL_CHAR)
                      printf("%30s   %s\n","",chr[ind]);
              break;
	   }
           default:
              fprintf(stderr,"Error: Unknown CONFIG type %d\n",
                             cfg->rec_list[i].value_type);
              exit(-1);
              break;
       }
       i++;
    }

}

/****************************************************************/
/*  Dump the config structure to stdout (this is as a usage     */
/*  statement)                                                  */
/****************************************************************/
void print_usage_CONFIG(CONFIG *cfg, char *prog_name)
{
    int i=0, j, grp_ids[10], first;

    for (i=0; i<10; i++) grp_ids[i]=FALSE;

    printf("Usage: %s\n",prog_name);
    for (i=0; *(cfg->rec_list[i].intern_id) != NULL_CHAR; i++){
	if (cfg->rec_list[i].flag_group != NO_GRP){
	    if (grp_ids[cfg->rec_list[i].flag_group] == FALSE){
		/* dump the group */
		printf("      [ ");
		first = 0;
		for (j=0; *(cfg->rec_list[j].intern_id) != NULL_CHAR; j++){
		    if (cfg->rec_list[j].flag_group == 
			cfg->rec_list[i].flag_group){
			if (first++ != 0)
			    printf("| ");
			switch (cfg->rec_list[j].value_type){
			  case CFG_TGL:
			    printf("-%s ", cfg->rec_list[j].intern_id);
			    break;
			  case CFG_STR:
			    printf("-%s string ",cfg->rec_list[j].intern_id);
			    break;
			  case CFG_C:
			    printf("-%s file_name ",cfg->rec_list[j].intern_id);
			    break;
			  case CFG_C2:{
			      printf("-%s ",cfg->rec_list[j].intern_id);
			      printf("(MAX %d) file_name ",
				     cfg->rec_list[j].num_elem);
			      break;
			  }
			  default:
			    fprintf(stderr,"Error: Unknown CONFIG type %d\n",
				    cfg->rec_list[i].value_type);
			    exit(-1);
			    break;
			}
		    }
		}
		printf("]\n");
		grp_ids[cfg->rec_list[i].flag_group] = TRUE;
	    }
	} else {
	    switch (cfg->rec_list[i].value_type){
	      case CFG_TGL:
		printf("      [ -%s ]\n",cfg->rec_list[i].intern_id);
		break;
	      case CFG_STR:
		printf("      [ -%s string ]\n",cfg->rec_list[i].intern_id);
		break;
	      case CFG_C:
		printf("      [ -%s file_name ]\n",cfg->rec_list[i].intern_id);
		break;
	      case CFG_C2:{
		  printf("      [ -%s ",cfg->rec_list[i].intern_id);
		  printf("(MAX %d) file_name ]\n",cfg->rec_list[i].num_elem);
		  break;
	      }
	      default:
		fprintf(stderr,"Error: Unknown CONFIG type %d\n",
			cfg->rec_list[i].value_type);
		exit(-1);
		break;
	    }
	}
    }
}

/****************************************************************/
/*  Check the configuration id 'ind' to see if it has been set  */
/****************************************************************/
int is_CONFIG_ind_set(CONFIG *cfg, int ind, int verbose)
{
    char **t_arr;
    int i;

    switch (cfg->rec_list[ind].value_type){
      case CFG_TGL:
	if (strcmp(cfg->rec_list[ind].value,TOGGLE_ON_CHR) == 0)
	    return(TRUE);
	break;
      case CFG_C:
	if (*(cfg->rec_list[ind].value) != NULL_CHAR){
	    if (file_readable(cfg->rec_list[ind].value))
		return(TRUE);
	    fprintf(stderr,
		    "Error: File '%s' for argument '%s' is un-readable\n",
		    cfg->rec_list[ind].value,cfg->rec_list[ind].intern_id);
	    return(FALSE);
	}
	break;
      case CFG_STR:
	if (*(cfg->rec_list[ind].value) != NULL_CHAR)
	    return(TRUE);
	break;
      case CFG_C2:
	t_arr = (char **)cfg->rec_list[ind].value;
	for (i=0; i < cfg->rec_list[ind].num_elem; i++)
	    if (*(t_arr[i]) != NULL_CHAR)
		return(TRUE);
	break;
      default:
	fprintf(stderr,"Error: Unknown CONFIG type %d\n",
		cfg->rec_list[ind].value_type);
	exit(-1);
	break;
    }
    if (verbose)
	fprintf(stderr,"Error: Argument '%s' was not used\n",
		cfg->rec_list[ind].intern_id);
    return(FALSE);
}

/****************************************************************/
/*  verify that at least one of a group set has been set        */
/****************************************************************/
int is_CONFIG_group_set(CONFIG *cfg, int grp_ind, int verbose)
{
    int i=0;

    if (cfg_dbg) printf("CFG: cfg_group_is_active: %d\n",grp_ind);

    /* otherwise, erase all the other values for it's grouping */
    for (i=0; *(cfg->rec_list[i].intern_id) != NULL_CHAR; i++){
        if (cfg->rec_list[i].flag_group == grp_ind){
	    if (is_CONFIG_ind_set(cfg, i, FALSE))
		return(TRUE);
	}
    }
    if (verbose) {
	fprintf(stderr,"Error: One of these arguments should be used: ");
	for (i=0; *(cfg->rec_list[i].intern_id) != NULL_CHAR; i++)
	    if (cfg->rec_list[i].flag_group == grp_ind)
		fprintf(stderr,"%s ",cfg->rec_list[i].intern_id);
	fprintf(stderr,"\n");
    }
    return(FALSE);
}

/****************************************************************/
/*  verify that a cfg string has been set.                      */
/****************************************************************/
int is_CONFIG_set(CONFIG *cfg, char *str, int verbose)
{
    return(is_CONFIG_ind_set(cfg,is_CONFIG_id(cfg,str),verbose));
}

