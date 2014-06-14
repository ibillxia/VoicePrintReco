#include <snr/snrlib.h>

/*****************************************************************
 *
 * speech_det.c -- Speech Detection algorithm
 *
 * The speech detect is a fairly simple two event windowing algorithm,
 * based on the value (usually in db) of the average energy (r0).
 *
 * For start of speech,
 *  if r0 is greater than start_high_water_mark for more than
 *  start_window number of frames, then the beginning of speech
 *  was at start_offset number of frames before when speech went above
 *  start_low_water_mark.
 *
 *   [a click]        [a stop cons.]
 * 
 *       *               **  ************* ...
 * -----*-*-------------*-*-*------------------ start_high
 *      * *            *  * *                            
 * -----*-*----------**----*------------------- start_low
 * *****   **********      *
 *        ^   ^      ^  ^            ^
 *  spike/  start   low high         now
 *   end          cross cross
 *             |<--->|  |<---------->|
 *              start       start
 *             offset      window
 * 
 * 
 * The end of speech is not quite symetric:
 *  If r0 goes under end_low_water_mark, and then stays below
 *  end_high_water_mark for end_window number of frames, then end of
 *  speech is end_offset frames after when speech went below
 *  end_low_water_mark.
 * 
 * 
 *     [a stop]                [a click]
 *
 * ** ***    *****                 *
 * -**---**-*-----*----------------*------------ end_high
 *         *       ***        **** *            
 * --------*----------* ******----* ******------ end_low
 *         *           *
 *                     ^   ^       ^     ^  
 *                    low  end   click  now
 *                   cross       start
 *                     |<--------------->|
 *                     |<->|       end_window
 *                      end
 *                    offset
 * 
 *
 * All this is complicated somewhat by the need to reject brief
 * events (spikes and dropouts) in different ways depending on
 * how close they are to other speech events.  For instance:
 *
 * (when starting)
 *       *    |    ****    |        **  ************* ...
 * -----*-*---|---*----*---|-------*-*-*------------------ start_high
 *      * *   |   *    *   |      *  * *                            
 * -----*-*---|---*----*---|----**----*------------------- start_low
 * *****   ***|***      ***|****      *
 *       A          B              C     D
 *
 * A is short, and far away from other speech, so it is a spike and
 * should be ignored.
 * B is long so even though it is far from other speech, it is not
 * a spike.
 * C is short, but close to D, so it is part of the speech.
 *
 * (when starting)
 *           **  *******|     *        **********....
 * ---------*-*-*-------|----*-*------*---------------- start_high
 *      *  *  * *       |    * * **   *       
 * ----*-*-*---*--------|----*-*-*-*-*----------------- start_low
 * ****   *    *        |****   *   *         
 *      E F  G H  I     |     J K L M  N      
 *
 * E is short and below start_high, the dropout at F is shorter than
 * max_dropout, G is short but above short_high, dropout at H is also
 * shorter than max_dropout, and from the beginning of G to into I is
 * longer than start_window. In this case, speech starts at the beginning
 * of E.
 * JKLMN is like EFGHI, but J is above start_high, and L is not.
 * Here also speech starts at the beginning of J.
 *
 * (when stopping)
 * ** ***    *****   *             *
 * -**---**-*-----*-*-*------------*------------ end_high
 *         *      * * *       **** *            
 * --------*-------**-* ******----* ******------ end_low
 *         *       *   *
 *         O       P Q             R
 *
 * O is short, so it is part of the speech.
 * P is a short dropout, followed by Q, a short spike.  This is also
 * part of speech.  (Alternately Q is a short spike, but "close" to
 * the speech, so it is part of the speech.)
 * R is a spike far from speeh (on both sides), so it is not speech
 * and should not be counted in the long end-of-speech timeout period.
 *
 * (when stopping)
 * ** ***            *    *****
 * -**---**---------*-*--*--------- end_high
 *         *        * * *         
 * --------*--------*-* *---------- end_low
 *          ********   *       
 *                   S   T
 *
 * Also, S is a short spike, far from past speech, but it is followed
 * closely by a second high point at T, so it is part of the speeech.
 * This is like an end of speech event followed by a new beginning
 * of speech event.
 *
 * (when stopping)
 * ** ***            **   **
 * -**---**---------*--*--* *-------- end_high
 *         *        *  * *   *       
 * --------*--------*--* *---*------- end_low
 *          ********    *     ****
 *                    U    V
 * 
 * Worse yet, neither U nor V are large enough to be counted as
 * longer than a pulse spike, but together they are (beg of P to 
 * end of Q is long enough).  They are speech (a short, unstressed
 * word with a stop in the middle).
 *
 *****************************************************************/

/* Parameters */
/* The next four values represent energies in dB */
double start_low_water  = 55.0;
double start_high_water = 58.0;
double end_low_water    = 45.0;
double end_high_water   = 48.0;

int   start_window = 8;
int   start_offset = 20;
int   end_window   = 15;
int   end_offset   = 20;

int   spike_rej    = 6;
int   dropout_rej  = 10;

/* "OUTPUT" Externals */
int   start_cross;	  /* frame when we up-crossed low water */
int start_rejd_spike_end; /* ending frame of last rejected spike */
int   end_cross;	  /* last frame before we crossed low water */
int   end_rejd_spike_beg; /* starting fr. of first rej'd ending spike */

/* internal time storage */
int start_lowdrop_cross;	/* frame when we down-crossed low water */
int start_high_cross;	        /* frame when we up-crossed high water */
int start_dropout_cross;        /* frame when we down-crossed high water */

int end_spike_up_cross;
int end_spike_down_cross;

int sd_state;
int old_sd_state;

int current_frame;   /* save the current frame as local info */

#undef VERBOSE

int speech_begin(void)   /* return the frame when speech last began */
{
  int frame;

  frame = (start_cross - start_offset);
  if ((start_rejd_spike_end > 0)
      && (frame < start_rejd_spike_end))
    frame = start_rejd_spike_end;
  if (frame < 0)
    frame = 0;

  return frame;
}

int speech_end(void) /* return the frame when speech last ended */
{
  int frame;

  frame = (end_cross + end_offset);
  if ((end_rejd_spike_beg > end_cross) &&
      (frame > end_rejd_spike_beg))
    frame = end_rejd_spike_beg;

  /* bounds check against end of data */
  if (frame > current_frame)
    frame = current_frame;

  return frame;
}




/*+****************************************************************
 *
 * do_speech_detect() -- check for all of the above condiditions.
 *
 *****************************************************************/

#define TRANSITION(ns)	{sd_state = (ns); break;}

int do_speech_detect(double r0, int fr_num)
         			/* r0 in db */
             
{
    old_sd_state = sd_state;	/* update previous state */
    current_frame = fr_num;

    switch (sd_state) {
	/*
	 * Initial state (before speech starts)
	 */
      case SD_INIT_SIL:		/* in initial silence region */
	if (r0 >= start_high_water) {
	    start_cross = fr_num;
	    start_high_cross = fr_num;
	    TRANSITION (SD_START_HIGH);
	}
	else if (r0 >= start_low_water) {
	    start_cross = fr_num;
	    TRANSITION (SD_START_LOW);
	}
	break;


	/*
	 * Starting window states including brief dropout rejection.
	 *
	 * States for a series of low peaks before the first high
	 * peak.
	 */
      case SD_START_LOW:	/* have gone above low water, but not high */
	if (r0 < start_low_water) {	/* must have been a noise spike */
	    start_lowdrop_cross = fr_num;
	    TRANSITION (SD_START_LOW_DROP);
	}
	else if (r0 >= start_high_water) {
	    start_high_cross = fr_num;
	    TRANSITION (SD_START_HIGH);
	}
	break;

      case SD_START_LOW_DROP:
	if (r0 >= start_low_water) {
	    /* note that we DO NOT set start_cross here! */
	    TRANSITION (SD_START_LOW);
	}
	else if ((fr_num - start_lowdrop_cross) >= dropout_rej) {
	    start_rejd_spike_end = start_lowdrop_cross + 1;
#ifdef VERBOSE
printf("   *** LOW  start_rejd_spike_end = %d\n", start_rejd_spike_end);
#endif
	    TRANSITION (SD_INIT_SIL);	/* too long a pause */
	}
	break;

	/*
	 * States for a high peak, possibly followed by low or high
	 * peaks, followed by high for more than enough time.
	 * (also, if two adjacent high peaks are close enough
	 * together, start speech.)
	 */
      case SD_START_HIGH:	/* have gone above high water mark */
	if (r0 < start_high_water) {	/* might be a stop dropout */
	    TRANSITION (SD_START_LOW2);
	}
	else if ((fr_num - start_high_cross) >= start_window) {
	    TRANSITION (SD_SPEAKING);	/* we have started!!! */
	    /* notify user */
	    /*
	     * speech started at either (start_cross - start_offset)
	     * or start_rejd_spike_end, whichever is later.
	     */
	}
	break;

      case SD_START_LOW2:	/* have gone above high,  */
	if (r0 < start_low_water) {	/* must have been a noise spike */
	    start_dropout_cross = fr_num;
	    TRANSITION (SD_START_DROP);
	}
	else if (r0 >= start_high_water) {
	    TRANSITION (SD_START_HIGH);
	}
	break;


      case SD_START_DROP:	/* went above high, then droped below high */
	if (r0 >= start_low_water) {
	    TRANSITION (SD_START_LOW2);
	}
	else if ((fr_num - start_dropout_cross) >= dropout_rej) {
	    start_rejd_spike_end = start_dropout_cross + 1;
#ifdef VERBOSE
printf("   *** HIGH start_rejd_spike_end = %d\n", start_rejd_spike_end);
#endif
	    TRANSITION (SD_INIT_SIL);	/* too long a pause */
	}
	break;

	/*
	 * Speaking state
	 */
      case SD_SPEAKING:
	if (r0 < end_low_water) {
	    end_cross = fr_num;
	    end_rejd_spike_beg = fr_num; /* just in case */
	    TRANSITION (SD_FINISHING);
	}
	break;


	/*
	 * Ending states including spike rejection
	 */
      case SD_FINISHING:
	if (r0 >= end_high_water) {
	    /*
	     * If the dropout between speaking and now was less than
	     * the dropout rejection threshold, then it must have been
	     * a closure and is therefore still part of the speech.
	     *
	     * If it happened longer than the dropout rejection threshold
	     * frames ago, we ignore it as a spike (unless it is of
	     * long duration).
	     *
	     * This assumes that (dropout_rej < end_window), which
	     * should be true (the former is about 180ms, the latter 800ms
	     * or more).
	     */
	    if ((fr_num - end_cross) < dropout_rej) {
		TRANSITION (SD_SPEAKING);
	    } else {
		end_spike_up_cross = fr_num;
		TRANSITION (SD_FIN_SPIKE);
	    }
	}
	else if ((fr_num - end_cross) >= end_window) {
	    TRANSITION (SD_DONE);		/* we have finished!! */

	    /* notify user */
	    /* end of speech is at (end_cross + end_offset) */
	}
	break;

      case SD_FIN_SPIKE:
	if (r0 < end_low_water) {
	    /* note we do NOT set end_cross here */
	    end_spike_down_cross = fr_num;
				/* spike over, see if one of a group */
	    TRANSITION (SD_FIN_SP_DROP);
	}
	else if ((fr_num - end_spike_up_cross) >= spike_rej) {
	    /* that was no spike, they are still talking */

	    /* reset pointer to beginning of first ignorable spike */
	    end_rejd_spike_beg = end_cross;
	    TRANSITION (SD_SPEAKING);
	}
	break;

      case SD_FIN_SP_DROP:
	if (r0 >= end_high_water) {
	    /* another spike, pretend dropout between didn't happen */
	    TRANSITION (SD_FIN_SPIKE);
	}
	else if ((fr_num - end_spike_down_cross) >= dropout_rej) {
	    /*
	     * it's been long enough since the spike ended, it was
	     * an ignorable spike.
	     */
	    /*
	     * If we havn't saved the start of the first spike
	     * after end of speech, do so now.
	     */
	    if (end_rejd_spike_beg <= end_cross) {
		end_rejd_spike_beg = end_spike_up_cross - 1;
#ifdef VERBOSE
printf("   *** end_rejd_spike_beg = %d\n", end_rejd_spike_beg);
#endif
	    }

	    TRANSITION (SD_FINISHING);
	}
	break;


	/*
	 * Speech has ended
	 */
      case SD_DONE:
	/* we don't have anything to do here */
	/* jump back in the original state */
	init_speech_detect(fr_num);

	break;
    }

    return sd_state;
}


/*+****************************************************************
 * init_speech_detect() -- reset the state of the speech detector
 *
 * This is usually called with fram_num == 0;
 */

void init_speech_detect(int fr_num)
{
    sd_state = SD_INIT_SIL;
    old_sd_state = SD_INIT_SIL;

    start_cross = fr_num;
    end_cross = fr_num;

    start_rejd_spike_end = fr_num;
    end_rejd_spike_beg = fr_num;

    start_high_cross = fr_num;
    start_dropout_cross = fr_num;

    end_spike_up_cross = fr_num;
    end_spike_down_cross = fr_num;

    current_frame=fr_num;
}



void set_speech_params(double start_low, double start_high, double end_low, double end_high)
{
  start_low_water=start_low;
  start_high_water=start_high;
  end_low_water=end_low;
  end_high_water=end_high;
}



/*+****************************************************************
 * force_speech_detect_done() -- pretend that we got something
 *  (called to handle an interrupt).
 */

void force_speech_detect_done(int fr_num)
{
    sd_state = SD_DONE;
    old_sd_state = SD_FINISHING;

    start_cross = fr_num;
    end_cross = fr_num;

    start_rejd_spike_end = fr_num;
    end_rejd_spike_beg = fr_num;
    
    start_high_cross = fr_num;
    start_dropout_cross = fr_num;

    end_spike_up_cross = fr_num;
    end_spike_down_cross = fr_num;
}






