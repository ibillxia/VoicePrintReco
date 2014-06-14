/****************************************************************/
/*                                                              */
/*           FILE: stat_dist.c                                  */
/*           WRITTEN BY: Jonathan G. Fiscus                     */
/*           DATE: April 14 1989                                */
/*                 NATIONAL INSTITUTE OF STANDARDS              */
/*                         AND TECHNOLOGY                       */
/*                    SPEECH RECOGNITION GROUP                  */
/*                                                              */
/*           USAGE: This file initializes the distribution      */
/*                  structures for Z1tail, Z2tail, and X2       */
/*                  distributions                               */
/*                                                              */
/****************************************************************/
#include <util/utillib.h>

#define MAX_Z	10
#define MAX_X2	32

Z_STRUCT Z2tail[MAX_Z] = {
   {  2.575,
      "99%",
      "01%",
      0.99
   },
   {  2.329,
      "98%",
      "02%",
      0.98
   },
   {  2.170,
      "97%",
      "03%",
      0.97
   },
   {  2.055,
      "96%",
      "04%",
      0.96
   },
   {  1.960,
      "95%",
      "05%",
      0.95
   },
   {  1.881,
      "94%",
      "06%",
      0.94
   },
   {  1.811,
      "93%",
      "07%",
      0.93
   },                 
   {  1.751,
      "92%",
      "08%",
      0.92
   },
   {  1.695,
      "91%",
      "09%",
      0.91
   },
   {  1.645,
      "90%",
      "10%",
      0.90
   }
};


Z_STRUCT Z1tail[MAX_Z] = {
   {
      2.330,
      "99%",
      "01%",
      0.99
   },
   {
      2.055,
      "98%",
      "02%",
      0.98
   },
   {
      1.881,
      "97%",
      "03%",
      0.97
   },
   {
      1.751,
      "96%",
      "04%",
      0.96
   },
   {
      1.645,
      "95%",
      "05%",
      0.95
   },
   {
      1.555,
      "94%",
      "06%",
      0.94
   },
   {
      1.474,
      "93%",
      "07%",
      0.93
   },
   {
      1.405,
      "92%",
      "08%",
      0.92
   },
   {
      1.340,
      "91%",
      "09%",
      0.91
   },
   {
      1.381,
      "90%",
      "10%",
      0.90
   }
};


X2_STRUCT X2 = {
    { "99%",
      "98%",
      "95%",
      "90%",
      "80%",
      "70%",
      "50%",
      "30%",
      "20%",
      "10%",
      "5%",
      "2%",
      "1%",
      ".1%"
    },
    { "1%",
      "2%",
      "5%",
      "10%",
      "20%",
      "30%",
      "50%",
      "70%",
      "80%",
      "90%",
      "95%",
      "98%",
      "99%",
      "99.9%"
    },
    {{ "df1",
       { 0.00016,
         0.00063,
         0.0039,
         0.016,
         0.064,
         0.15,
         0.46,
         1.07,
         1.64,
         2.71,
         3.84,
         5.41,
         6.64,
         10.83
       }
     },
     { "df2",
       { 0.02,
         0.04,
         0.10,
         0.21,
         0.45,
         0.71,
         1.39,
         2.41,
         3.22,
         4.60,
         5.99,
         7.82,
         9.21,
         13.82
       }
     },
     { "df3",
       { 0.12,
         0.18,
         0.35,
         0.58,
         1.00,
         1.42,
         2.37,
         3.66,
         4.64,
         6.25,
         7.82,
         9.84,
         11.34,
         16.27
       }
     },
     { "df4",
       { 0.30,
         0.43,
         0.71,
         1.06,
         1.65,
         2.20,
         3.36,
         4.88,
         5.99,
         7.78,
         9.49,
         11.67,
         13.28,
         18.46
       }
     },
     { "df5",
       { 0.55,
         0.75,
         1.14,
         1.61,
         2.34,
         3.00,
         4.35,
         6.06,
         7.29,
         9.24,
         11.07,
         13.39,
         15.09,
         20.52
       }
     },
     { "df6",
       { 0.87,
         1.13,
         1.64,
         2.20,
         3.07,
         3.83,
         5.35,
         7.23,
         8.56,
         10.64,
         12.59,
         15.03,
         16.81,
         22.46
       }
     },
     { "df7",
       { 1.24,
         1.56,
         2.17,
         2.83,
         3.82,
         4.67,
         6.35,
         8.38,
         9.80,
         12.02,
         14.07,
         16.62,
         18.48,
         24.32
       }
     },
     { "df8",
       { 1.65,
         2.03,
         2.73,
         3.49,
         4.59,
         5.53,
         7.34,
         9.52,
         11.03,
         13.36,
         15.51,
         18.17,
         20.09,
         26.12
       }
     },
     { "df9",
       { 2.09,
         2.53,
         3.32,
         4.17,
         5.38,
         6.39,
         8.34,
         10.66,
         12.24,
         14.68,
         16.92,
         19.68,
         21.67,
         27.88
       }
     },
     { "df10",
       { 2.56,
         3.06,
         3.94,
         4.86,
         6.18,
         7.27,
         9.34,
         11.78,
         13.44,
         15.99,
         18.31,
         21.16,
         23.21,
         29.59
       }
     },
     { "df11",
       { 3.05,
         3.61,
         4.58,
         5.58,
         6.99,
         8.15,
         10.34,
         12.90,
         14.63,
         17.28,
         19.68,
         22.62,
         24.72,
         31.26
       }
     },
     { "df12",
       { 3.57,
         4.18,
         5.23,
         6.30,
         7.81,
         9.03,
         11.34,
         14.01,
         15.81,
         18.55,
         21.03,
         24.05,
         26.22,
         32.91
       }
     },
     { "df13",
       { 4.11,
         4.76,
         5.89,
         7.04,
         8.63,
         9.93,
         12.34,
         15.12,
         16.98,
         19.81,
         22.36,
         25.47,
         27.69,
         34.53
       }
     },
     { "df14",
       { 4.66,
         5.37,
         6.57,
         7.79,
         9.47,
         10.82,
         13.34,
         16.22,
         18.15,
         21.06,
         23.68,
         26.87,
         29.14,
         36.12
       }
     },
     { "df15",
       { 5.23,
         5.98,
         7.26,
         8.55,
         10.31,
         11.72,
         14.34,
         17.32,
         19.31,
         22.31,
         25.00,
         28.26,
         30.58,
         37.70
       }
     },
     { "df16",
       { 5.81,
         6.61,
         7.96,
         9.31,
         11.15,
         12.62,
         15.34,
         18.42,
         20.46,
         23.54,
         26.30,
         29.63,
         32.00,
         39.29
       }
     },
     { "df17",
       { 6.41,
         7.26,
         8.67,
         10.08,
         12.00,
         13.53,
         16.34,
         19.51,
         21.62,
         24.77,
         27.59,
         31.00,
         33.41,
         40.75
       }
     },
     { "df18",
       { 7.02,
         7.91,
         9.39,
         10.86,
         12.86,
         14.44,
         17.34,
         20.60,
         22.76,
         25.99,
         28.87,
         32.35,
         34.80,
         42.31
       }
     },
     { "df19",
       { 7.63,
         8.57,
         10.12,
         11.65,
         13.72,
         15.35,
         18.34,
         21.69,
         23.90,
         27.20,
         30.14,
         33.69,
         36.19,
         43.82
       }
     },
     { "df20",
       { 8.26,
         9.24,
         10.85,
         12.44,
         14.58,
         16.27,
         19.34,
         22.78,
         25.04,
         28.41,
         31.41,
         35.02,
         37.57,
         45.32
       }
     },
     { "df21",
       { 8.90,
         9.92,
         11.59,
         13.24,
         15.44,
         17.18,
         20.34,
         23.86,
         26.17,
         29.62,
         32.67,
         36.34,
         38.93,
         46.80
       }
     },
     { "df22",
       { 9.54,
         10.60,
         12.34,
         14.04,
         16.31,
         18.10,
         21.24,
         24.94,
         27.30,
         30.81,
         33.92,
         37.66,
         40.29,
         48.27
       }
     },
     { "df23",
       { 10.20,
         11.29,
         13.09,
         14.85,
         17.19,
         19.02,
         22.34,
         26.02,
         28.43,
         32.01,
         35.17,
         38.97,
         41.64,
         49.73
       }
     },
     { "df24",
       { 10.86,
         11.99,
         13.85,
         15.66,
         18.06,
         19.94,
         23.34,
         27.10,
         29.55,
         33.20,
         36.42,
         40.27,
         42.98,
         51.18
       }
     },
     { "df25",
       { 11.52,
         12.70,
         14.61,
         16.47,
         18.94,
         20.87,
         24.34,
         28.17,
         30.68,
         34.38,
         37.65,
         41.57,
         44.31,
         52.62
       }
     },
     { "df26",
       { 12.20,
         13.41,
         15.38,
         17.29,
         19.82,
         21.79,
         25.34,
         29.25,
         31.80,
         35.56,
         38.88,
         42.86,
         45.64,
         54.05
       }
     },
     { "df27",
       { 12.88,
         14.12,
         16.15,
         18.11,
         20.70,
         22.72,
         26.34,
         30.32,
         32.91,
         36.74,
         40.11,
         44.14,
         46.96,
         55.48
       }
     },
     { "df28",
       { 13.56,
         14.85,
         16.93,
         18.94,
         21.59,
         23.65,
         27.34,
         31.39,
         34.03,
         37.92,
         41.34,
         45.42,
         48.28,
         56.89
       }
     },
     { "df29",
       { 14.26,
         15.57,
         17.71,
         19.77,
         22.48,
         24.58,
         28.34,
         32.46,
         35.14,
         39.09,
         42.56,
         46.69,
         49.59,
         58.30
       }
     },
     { "df30",
       { 14.95,
         16.31,
         18.49,
         20.60,
         23.36,
         25.51,
         29.34,
         33.53,
         36.25,
         40.26,
         43.77,
         47.96,
         50.89,
         59.70
       }
     }
    }

};

/**********************************************************/
/*    print to stdout the chi squared distribution table  */
/**********************************************************/
void dump_X2_table(void)
{
    int i,j;
    
    printf("\n\n\t\t\t\tX2 DISTRIBUTION TABLE\n\n");
    printf("\t");
    for (i=MIN_X2_PER;i<MAX_X2_PER+1; i++)
        printf("%s\t",X2.per_str[i]);
    printf("\n\n");
    for (i=MIN_DF; i<MAX_DF+1; i++){
        printf("%s\t",X2.df[i].str);
        for (j=MIN_X2_PER;j<MAX_X2_PER+1; j++)
            printf("%2.3f\t",X2.df[i].level[j]);
        printf("\n");
    }
    printf("\n\n");
}

/**********************************************************************/
/*  given a list of integers, calculate the mean, variance,           */
/*  standard deviation and Z_statistic                                */
/*  Sep 17 1992: changed the variance divisor to be n-1               */
void calc_mean_var_std_dev_Zstat(int *Z_list, int num_Z, double *mean, double *variance, double *std_dev, double *Z_stat)
{
    int i;
    double tmp = 0.0;

    for (i=0;i<num_Z;i++){
       tmp+=Z_list[i];
    }
    *mean = tmp/(double)num_Z;

    tmp = 0.0;
    for (i=0;i<num_Z;i++)
       tmp+= (Z_list[i] - *mean) *  (Z_list[i] - *mean);

    if (tmp != 0.0){
      *variance = tmp / (double)(num_Z - 1);
       *std_dev = sqrt(*variance);
       *Z_stat = *mean / (sqrt(*variance) / sqrt((double)num_Z));
    }
    else
       *variance = *std_dev = *Z_stat = 0.0;
}

/**********************************************************************/
/*  given a list of doubles, calculate the mean, variance,             */
/*  standard deviation and Z_statistic                                */
void calc_mean_var_std_dev_Zstat_double(double *Z_list, int num_Z, double *mean, double *variance, double *std_dev, double *Z_stat)
{
    int i;
    double tmp = 0.0;

    for (i=0;i<num_Z;i++){
       tmp+=Z_list[i];
    }
    *mean = tmp/(double)num_Z;

    tmp = 0.0;
    for (i=0;i<num_Z;i++)
       tmp+= (Z_list[i] - *mean) *  (Z_list[i] - *mean);

    if (tmp != 0.0){
       *variance = tmp / (double)(num_Z - 1);
       *std_dev = sqrt(*variance);
       *Z_stat = *mean / (sqrt(*variance) / sqrt((double)num_Z));
    }
    else
       *variance = *std_dev = *Z_stat = 0.0;
}

/**********************************************************************/
/*     a general analysis routine to test whether or not the the      */
/*     Z_statistic indicates a significant difference                 */
int print_Z_analysis(double Z_stat)
{
    int i;

    printf("%s                                  Reject if\n","");
    printf("%s                         Z > (+%1.3f) or Z < (-%1.3f)\n","",
                                                     Z2tail[GEN_Z_PER].z,
                                                     Z2tail[GEN_Z_PER].z);
    printf("\n");
    printf("%s                                  Z = %1.3f\n","",Z_stat);

    printf("\n\n%s\t\tSUMMARY:\n\t\t-------\n\n","");
    if (fabs(Z_stat) > Z2tail[GEN_Z_PER].z){
     printf("\tThere is a significant difference between the test results\n");
        printf("\tusing a %s confidence level.\n",
                                   Z2tail[GEN_Z_PER].str);
        printf("\n");
        printf("\tFurther, the probablity of there being a difference is\n");
        for (i=GEN_Z_PER;i>0;i--)
            if (fabs(Z_stat) < Z2tail[i-1].z)
                break;
        if (i==MAX_Z_PER)
            printf("\tgreater that %s.\n",Z2tail[0].str);
        else
            printf("\tbetween %s to %s.",Z2tail[i].str,Z2tail[i-1].str);
        return(TEST_DIFF);
    }
    else{
        printf("\tThere is no significant difference between the test\n");
        printf("\tresults using a %s confidence level.\n",
                                            Z2tail[GEN_Z_PER].str);
        printf("\n");
        printf("\tFurther, the probablity of there being a difference is\n");
        for (i=GEN_Z_PER;i<MIN_Z_PER;i++)
            if (fabs(Z_stat) > Z2tail[i+1].z)
                break;
        if (i==MIN_Z_PER)
            printf("\tless than %s.\n",Z2tail[i].str);
        else
            printf("\tbetween %s to %s.",Z2tail[i+1].str,Z2tail[i].str);
        return(NO_DIFF);
    }
}

int Z_pass(double Z_stat)
{
    if (fabs(Z_stat) > Z2tail[GEN_Z_PER].z)
        return(TEST_DIFF);
    else
        return(NO_DIFF);
}

/**********************************************************************/
/* calc_two_sample_z_test_double by Brett 5/10/93                      */ 
/*     a general routine to perform a two-sample z test on two lists  */
/*     of doubles                                                      */
/**********************************************************************/
void calc_two_sample_z_test_double(double *l1, double *l2, int num_l1, int num_l2, double *Z)
{
  double mean_l1,var_l1,sd_l1,Z_stat_l1;
  double mean_l2,var_l2,sd_l2,Z_stat_l2;
  calc_mean_var_std_dev_Zstat_double(l1,num_l1,&mean_l1,&var_l1,&sd_l1,&Z_stat_l1);
  calc_mean_var_std_dev_Zstat_double(l2,num_l2,&mean_l2,&var_l2,&sd_l2,&Z_stat_l2);
  *Z=((mean_l1-mean_l2)/sqrt(((sd_l1*sd_l1)/(double)num_l1)+((sd_l2*sd_l2)/(double)num_l2)));
  /* print_Z_analysis(*Z);*/
}

/******************************************************************/
/*   Compute the accumulated binomial distribution for the given  */
/*   parameters.  The Formula was taken from                      */
/*   Statistics: Probability, Inference and Decision              */
/*   By Robert L. Winkler and William L. Hays,  Page 206          */
/*                                                                */
/*                        / n \      r      n-r                   */
/*     P( R = r | n, p) = |   |  *  p   *  q                      */
/*                        \ r /                                   */
/*                                                                */
/*        n = number of trials                                    */
/*        R = number of successes in 'n' trials                   */
/*        p = Probability of success                              */
/*        q = 1 - p                                               */
/******************************************************************/
double compute_acc_binomial(int R, int n, double p)
{
    int r, dbg=0;
    double sum=0.0;

    /* printf("Binomial 7,5,0.5 = %f\n",(double)compute_acc_binomial(1,1,0.1));*/

    for (r=0; r <= R; r++) {
        if (dbg) printf("Binomial nCr(%d,%d) = %f / ",
                        n,r,n_CHOOSE_r(n,r));
        if (dbg) printf("[pow(%5.3f,%2d) = %f",
                        p,r,pow(p,(double)r));
        if (dbg) printf(" * pow(%5.3f,%2d) = %f",
                        p,n-r,pow(1.0-p,(double)(n-r)));
        sum += n_CHOOSE_r(n,r) * pow(p,(double)r) * pow(1.0-p,(double)(n-r));	
        if (dbg) printf("] = %f\n",sum);
    }
    return(sum);
}


/*******************************************************************/
/*  Return the result of multipying all the numbers in the sequence*/
/*  from f to to                                                   */
/*******************************************************************/
double seq_mult(int f, int t)
{  
    double sum=1.0;
    int i;
    for (i=f; i<=t; i++)
        sum *= i;
    return(sum);
}

/*******************************************************************/
/* return the result for the N choose R counting Function          */
/*******************************************************************/
double n_CHOOSE_r(int n, int r)
{
    return(seq_mult(r+1,n) / seq_mult(1,n-r));
}





