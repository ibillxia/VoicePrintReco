#include <stdio.h>
/*  this the direct_search algorithm from Robert Hook and T. A. Reeves
    "Direct Search" Solution of Numerical and Statistical Problems
    (Journal ACM 1961 (p212-229)

    The search uses an input vector to calculate a value from a function
    S and then modifies the vector to minimize the function.  Input
    parameters are:

        phi:	the current base point
        K:	The number of coordinate points
        DELTA:	The current step size
        delta:	The "minimum" step size	
        rho:	The reduction factor for the step size (rho < 1)
        S:	The function used for the minimization

    OTHER VARIABLES:

        theta:	the previous base point
        psi:	the base point resulting from the current move
        Spsi:	The functional value of S(psi)
        Sphi:	The functional value of S(phi)
        SS:	?

    Last change date: Nov 27 1990
    cleaned up slightly, verbose option removed summer 1992.
    Jan 24, 1996 JGF, Added the Environement flag DS_DEBUG to diagnose problems
           - Added the full_search variable to search the entire space around a
	     test point before making a move. 
	   - added the cache functionality to reduce re-computation of S
*/
#define DS_C_VERSION "V1.0"
#include <util/utillib.h>

static double SS, Spsi, Sphi, theta, *DELTA, *delta, (*S)(int *), rho;
static int phi[30], phi_base[30], phi_cm[30], phi_map[30], K, k;
static int *psi, DELTA_change;
static int debug, x;
static int full_search = 0;
static int num_cache=400, **cache, last_cache = (-1), cache_count = 0;
static double *Scache;

static void E_proc(void);
static void add_to_cache(double Snew, int *phi_new);
static int in_cache(double *Snew, int *phi_new);

static void add_to_cache(double Snew, int *phi_new){
    int ack;
    int new_pos = (last_cache + 1) % (num_cache);

    for (ack=0; ack<K; ack++)
	cache[new_pos][ack] = phi_new[ack];
    Scache[new_pos] = Snew;
    last_cache = new_pos;
    if (cache_count < num_cache)
	cache_count ++;
/* 
    for (x=0; x<cache_count; x++){
	printf("Cached %d:  S()= %.2f %2s ",x,Scache[x],(x == last_cache) ? "x" : " ");
	for (ack=0; ack<K; ack++)
	    printf("%d ",cache[x][ack]);
	printf("\n");
    }
*/
}

static int in_cache(double *Snew, int *phi_new){
    int ick;

    for (x=0; x<cache_count; x++){
	for (ick=0; ick<K && cache[x][ick] == phi_new[ick]; ick++)
	    ;
	if (ick == K){
	    if (debug) printf("DS:  CACHE HIT Distance = %d\n",
			      (x <= last_cache) ? last_cache-x : num_cache - (x-last_cache));
	    *Snew = Scache[x];
	    return(1);
	}
    }
    return(0);
}

static void E_proc(void){
    int epk;

    if (!full_search){
	for (epk=0; epk<K; epk++) { phi_base[epk] = phi[epk]; }

	for (epk=0; epk<K; epk++){
	    phi[epk] = phi_base[epk] + DELTA[epk];
	    if (!in_cache(&Sphi, phi)){
		Sphi = (*S)(phi);
		add_to_cache(Sphi, phi);
	    }
	    if (Sphi < SS)
		SS = Sphi;
	    else{
		phi[epk] = phi_base[epk] - DELTA[epk];
		if (!in_cache(&Sphi, phi)){
		    Sphi = (*S)(phi);
		    add_to_cache(Sphi, phi);
		}
		if (Sphi < SS)
		    SS = Sphi;
		else
		    phi[epk] = phi_base[epk];
	    } 
	}
    } else {
	int iter = 0, maxiter = 1 << K, done=0, carry;
	if (debug) printf("DS: E:  Full Search: Maxiter= %d\n",maxiter);

	for (epk=0; epk<K; epk++) { phi_cm[epk] = phi_base[epk] = phi[epk]; phi_map[epk] = 0; }

	while (! done){
	    /* load the new phi */
	    if (debug) printf("DS: E:  iter: %d  Mapping: ",iter);
	    for (epk=0; epk<K; epk++) 
		if (phi_map[epk] == 0) {
		    phi[epk] = phi_base[epk];            if (debug) printf(" 0");
		} else if (phi_map[epk] == 1) {
		    phi[epk] = phi_base[epk] + DELTA[epk]; if (debug) printf(" +");
		} else {
		    phi[epk] = phi_base[epk] - DELTA[epk]; if (debug) printf(" -");
		}
	    if (debug) printf("\n");

	    /* compute the cost function */
	    if (!in_cache(&Sphi, phi)){
		Sphi = (*S)(phi);
		add_to_cache(Sphi, phi);
	    }
	    if (Sphi < SS) {
		SS = Sphi;
		for (epk=0; epk<K; epk++) phi_cm[epk] = phi[epk];
	    }

	    /* increment the phi_map */
	    for (epk=0, carry=1; epk<K && carry; epk++){
		carry = 0;
		if (phi_map[epk] < 2)
		    phi_map[epk] ++;
		else{
		    phi_map[epk] = 0;
		    carry = 1;
		    if (epk == K-1) done = 1;
		}
	    }
		    
	}
	/* copy in the selected minimum */
	for (epk=0; epk<K; epk++) phi[epk] = phi_cm[epk];
    }
    /* copy in the selected minimum */
    if (debug) {
	printf("DS: E: Minimized Sphi=%.2f ",SS);
	for (epk=0; epk<K; epk++) printf(" %d",phi[epk]);
	printf("\n");
    }
}

void direct_search(int *IN_psi, int IN_K, double *IN_DELTA, double IN_rho, double *IN_delta, double (*IN_S) ( int *), int IN_full_search)
{
    debug = getenv("DS_DEBUG") != (char *)0;


    alloc_2dimarr(cache,num_cache,IN_K,int);
    alloc_singarr(Scache,num_cache,double);

    psi = IN_psi;
    K = IN_K;
    DELTA = IN_DELTA;
    rho = IN_rho;
    S = IN_S;
    delta = IN_delta;
    last_cache = (-1), cache_count = 0;
    full_search = IN_full_search;

    if (debug) {
	printf("DS: IN_K=%3d  IN_rho=%.4f\n",IN_K,IN_rho);
	printf("DS:     Vector=");
	for (x=0; x<IN_K; x++) printf(" %d",IN_psi[x]);	printf("\n");
	printf("DS:     DELTA= ");
	for (x=0; x<IN_K; x++) printf(" %.3f",IN_DELTA[x]);	printf("\n");
	printf("DS:     delta= ");
	for (x=0; x<IN_K; x++) printf(" %.3f",IN_delta[x]); 	printf("\n");
    }

    Spsi = (*S)(psi);
    add_to_cache(Spsi, psi);
    if (debug) printf("DS: Initial measurement= %f\n",Spsi);

L1: SS = Spsi;
    if (debug) printf("DS: label L1 executed\n");

    for (k=0; k<K; k++)
        phi[k] = psi[k];

    E_proc();

    if (SS < Spsi){
L2:
	if (debug) printf("DS: label L2 executed\n");
	for (k=0; k<K; k++){
	    theta = psi[k];
	    psi[k] = phi[k];
	    phi[k] = 2*phi[k] - theta;
	}
	Spsi = SS;
	if (!in_cache(&Sphi, phi)){
	    Sphi = (*S)(phi);
	    add_to_cache(Sphi, phi);
	}
	SS = Sphi;
	
	E_proc();
	if (SS < Spsi) goto L2; else goto L1;
    }
    DELTA_change=0;
    for (k=0; k<K; k++)
        if (DELTA[k] >= delta[k]){
            DELTA[k] = rho * DELTA[k];
            DELTA_change=1;
        }
    if (DELTA_change==1) goto L1;

    free_2dimarr(cache,num_cache,int);
    free_singarr(Scache,double);
}







