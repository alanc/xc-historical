/* $XConsortium$ */
/**** module fax/g31d.c ****/
/******************************************************************************
				NOTICE
                              
This software is being provided by AGE Logic, Inc. and MIT under the
following license.  By obtaining, using and/or copying this software,
you agree that you have read, understood, and will comply with these
terms and conditions:

     Permission to use, copy, modify, distribute and sell this
     software and its documentation for any purpose and without
     fee or royalty and to grant others any or all rights granted
     herein is hereby granted, provided that you agree to comply
     with the following copyright notice and statements, including
     the disclaimer, and that the same appears on all copies and
     derivative works of the software and documentation you make.
     
     "Copyright 1993 by AGE Logic, Inc. and the Massachusetts
     Institute of Technology"
     
     THIS SOFTWARE IS PROVIDED "AS IS".  AGE LOGIC AND MIT MAKE NO
     REPRESENTATIONS OR WARRANTIES, EXPRESS OR IMPLIED.  By way of
     example, but not limitation, AGE LOGIC AND MIT MAKE NO
     REPRESENTATIONS OR WARRANTIES OF MERCHANTABILITY OR FITNESS
     FOR ANY PARTICULAR PURPOSE OR THAT THE SOFTWARE DOES NOT
     INFRINGE THIRD-PARTY PROPRIETARY RIGHTS.  AGE LOGIC AND MIT
     SHALL BEAR NO LIABILITY FOR ANY USE OF THIS SOFTWARE.  IN NO
     EVENT SHALL EITHER PARTY BE LIABLE FOR ANY INDIRECT,
     INCIDENTAL, SPECIAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOSS
     OF PROFITS, REVENUE, DATA OR USE, INCURRED BY EITHER PARTY OR
     ANY THIRD PARTY, WHETHER IN AN ACTION IN CONTRACT OR TORT OR
     BASED ON A WARRANTY, EVEN IF AGE LOGIC OR MIT OR LICENSEES
     HEREUNDER HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH
     DAMAGES.
    
     The names of AGE Logic, Inc. and MIT may not be used in
     advertising or publicity pertaining to this software without
     specific, written prior permission from AGE Logic and MIT.

     Title to this software shall at all times remain with AGE
     Logic, Inc.
*****************************************************************************
  
	fax/g31d.c -- DDXIE G31D fax decode technique/element
  
	Ben Fahy -- AGE Logic, Inc. July, 1993
  
*****************************************************************************/


#define xoft_verbose_not


/* the folling define causes extra stuff to be saved in state recorder */
#define _G31D
#include "fax.h"
#include "faxint.h"
#include "bits.h"

#include <servermd.h>
	/* pick up the BITMAP_BIT_ORDER from Core X*/


/**********************************************************************/
int decode_g31d(state)
FaxState *state;
{
register int bitpos;
register unsigned char *byteptr;
register unsigned char *endptr;
register int a0_color;
register int a0_pos;
register int goal;
register int mode;	/* could overload goal, but will resist */
register int length;
register int width;
register int g31d_horiz;
register int rl;

int 	lines_found=0;
int	n_old_trans;
int	n_new_trans;
int	*old_trans;
int	*new_trans;
int 	a0a1,a1a2;
int 	length_acc=0;
int 	last_b1_idx=0;
int 	b1_pos,b2_pos;
int 	code,nbits;
int	magic_blab = 0;


	if (state == (FaxState *)NULL)
		return(-1);

	/* set up initial bitstream for the very first strip */
	if (!state->bits.started) {
	    if (state->strip_state != StripStateNew) {
	       state->decoder_done = FAX_DECODE_DONE_ErrorBadStripper;
	       return(lines_found);
	    }
	    state->bits.byteptr = (unsigned char *)state->strip;
	    state->bits.endptr  = state->bits.byteptr + state->strip_size-4;
	       /* we will panic with four bytes to go */

	    state->bits.bitpos = 0;
	    state->bits.started = 1;
	}

	localize_state(state);

	if (state->magic_needs) 
	    finish_magic(state->final);
		/* a magic strip was waiting for 1st word of next strip */


/***	Main Decoding Loop	***/
	while(1) {

	  switch(goal) {
	  case FAX_GOAL_SkipPastAnyToEOL:
		while (byteptr < endptr) {
	    	  /* look for EOL code */
	    	  code = get_wcode(byteptr,bitpos,endptr);
	    	  rl     = _WhiteFaxTable[code].run_length;
	      	  if (rl == EOL_RUN_LENGTH) 
			break;
		  else
			/* move bitstream one bit further and try again */
 		 	adjust_1bit(byteptr,bitpos,endptr);
		}
		if (byteptr == endptr) {
			state->decoder_done = FAX_DECODE_DONE_ErrorSkipPast;
			exit(1);
		}
	    	nbits  = _WhiteFaxTable[code].n_bits;
	    	goal = FAX_GOAL_StartNewLine;
	    	adjust_bitstream(nbits,byteptr,bitpos,endptr);
		break;

	  case FAX_GOAL_SeekFillAndEOL:
	    /* look for EOL code */
		while (byteptr < endptr) {
	    	  /* look for EOL code */
	    	  code = get_wcode(byteptr,bitpos,endptr);
	    	  rl     = _WhiteFaxTable[code].run_length;
	      	  if (rl == EOL_RUN_LENGTH) 
			break;
		  else if (code) {
		     state->decoder_done =  FAX_DECODE_DONE_ErrorBadFill;
		     return(lines_found);
		  }
		  else
			/* move bitstream one bit further and try again */
 		 	adjust_1bit(byteptr,bitpos,endptr);
		}
		if (byteptr == endptr) {
			printf(" program error, byteptr 0x%x = endptr 0x%x\n",
				byteptr,endptr);
			exit(1);
		}
	    	nbits  = _WhiteFaxTable[code].n_bits;
	    	goal = FAX_GOAL_StartNewLine;
		/* set goal before adjusting in case we run out of data */
	    	adjust_bitstream(nbits,byteptr,bitpos,endptr);
		break;

	  case FAX_GOAL_StartNewLine:

	    /* if any data produced by previous states, write it out */
	    if (lines_found) {
		if (state->write_data) {
		   char *olp = state->o_lines[lines_found-1];

		   /* White will be ones, blacks zeros */
		   zero_even(olp,new_trans,n_new_trans,width,1);
		}
	        if (lines_found >= state->nl_sought)
		    save_state_and_return(state);
	    }
	    /* normal line initialization stuff */
	    a0_pos   = -1;
	    a0_color = WHITE;
	    goal = FAX_GOAL_HandleHoriz;

	    reset_transitions();
	    break;

	  case  FAX_GOAL_RecoverZero:

#ifndef old_yucko_code
	    goal = FAX_GOAL_SeekFillAndEOL;
	    break;
#else
	    /* have to set new goal in case adjusting throws us	*/
	    /* back to the caller (to get more data) 		*/
	    goal = FAX_GOAL_AdjustedButStillRecovering;
	    adjust_bitstream(nbits,byteptr,bitpos,endptr);

	  case FAX_GOAL_AdjustedButStillRecovering:
	    {
	    int next_goal = FAX_GOAL_FoundOneEOL;

	    while (1) {
	       if (*byteptr) {
		register int mask;
	        /* important assumption: we only get here if we have */
	        /* found at least twelve bits of zero leading up to  */
	        /* the current bitposition.  Therefore, there can't  */
		/* be any bits in the byte before the one we want    */

		mask = 1 << (7-bitpos);
		while (bitpos < 8) {
		   if (mask & (*byteptr))
			break;
		   mask >>= 1;
		   ++bitpos;
		 }	     /* can't really reach bitpos = 8 above */
		 goal = next_goal;
 		 adjust_1bit(byteptr,bitpos,endptr);
		 break;
			/* break out of while (1) */
	       }
	       else {
		 bitpos = 0;
		 if (++byteptr >= endptr)
	           do_magic(byteptr,bitpos,endptr);
		 /* loop around to search some more */
	       }
	    } /* end of while */

	    }
	    break;

	  case  FAX_GOAL_FoundOneEOL:
	    /* found an EOL.  If this is RTC, there will be five more EOL's */
	    /* one the way.  If this is a normal EOL,  next code will not   */
	    /* be an EOL,  and we should interpret the next bit as tag bit  */


	    /* look for second EOL code */
	    code = get_wcode(byteptr,bitpos,endptr);
	    rl = _WhiteFaxTable[code].run_length;
	    if (rl != EOL_RUN_LENGTH) {
	        goal =  FAX_GOAL_SeekTagBit;
		break;
	    }

	    /* Got a second EOL code */
	    printf(" Got a second EOL code!!!\n");
	    exit(0);
	    break;
#endif /* old_yucko_code */


	  case  FAX_GOAL_HandleHoriz: 
	    length_acc=0;
	    goal = FAX_GOAL_AccumulateA0A1;
		/* in case I run out of data while getting a0-a1 distance */

	  case FAX_GOAL_AccumulateA0A1:
	    get_a0a1(FAX_GOAL_RecordA0A1);
		/* If we have to return for data before getting the 	*/
		/* whole a0a1 distance, we want to return to the a0a1	*/
		/* accumulate state when we reenter the decoder. If	*/
		/* adjusting the bitstream forces us to return after	*/
		/* getting the last part of the a0a1 distance, then	*/
		/* we want to jump to state FAX_GOAL_RecordA0A1 when	*/
		/* we reenter the decoder.				*/

	  case  FAX_GOAL_RecordA0A1:
	    a0a1 = length_acc;
	    if (a0a1 < 0) {
		printf(" bad a0a1, %d\n",a0a1);
	        printf(" byteptr %x, endptr %x, bitpos %x %x%x%x\n",
	           byteptr,endptr,bitpos,*byteptr,*(byteptr+1),*(byteptr+2));
		return(-1);
	    }
	    if (a0_pos < 0) {
		/* at start of line, a0a1 is the number of white pixels,  */
		/* which is also the index on the line where white->black */
		new_trans[n_new_trans++] = a0a1;
	    } else {
		/* in middle of the line, a0a1 is run-length, so
		 * a1_pos = a0_pos + a0a1, a2_pos = a1_pos + a1a2
		 *
		 *  0  1  2  3  4  5  6  7  8  9  A  B  C  D 
		 *    |w |w |w |b |b |b |b |b |w |  |  |  |  |
		 *     a0       a1             a2
		 */ 
		new_trans[n_new_trans++] = a0_pos + a0a1;
	    }
	    if (new_trans[n_new_trans-1] < 0){
	       printf(" program error, bad transition position\n");
	       printf(" byteptr %x, endptr %x, bitpos %x %x %x %x %x\n",
	         byteptr,endptr,bitpos,*byteptr,*(byteptr+1),
		 *(byteptr+2),*(byteptr+3));
	       printf(" n_new= %d, firstis %d, lastis%d\n",
		n_new_trans,new_trans[0],new_trans[n_new_trans-1]);
	       return(-1);
	    }


	    if (goal == FAX_GOAL_RecoverZero) {

	        /* it's possible we got here by reading a zero in get_a0a1, */
	        /* in which case 'goal' was set to FAX_GOAL_RecoverZero. We */
	        /* must assume that the zero is an EOL pre-padded with a    */
	        /* variable number of zeros, which is legal according to    */
	        /* the coding spec.  So we will record the increment in     */
	        /* line number and then attempt to recover.		    */
	        goal = FAX_GOAL_SeekFillAndEOL;
	        ++lines_found;
	        break;
			/* break out of the switch, loop with the while */
	    }

	    if (rl == EOL_RUN_LENGTH) {
	        ++lines_found;
	        goal =  FAX_GOAL_StartNewLine;
		/* if we got a non-zero length, remember the transition */
		/* in case the next line is coded in vertical mode	*/
		if (a0a1)
		   new_trans[n_new_trans++] = a0a1;

		/* notice we don't have to worry about a0_pos - 
		   it will be reset when we start the new line
		*/
		break;
			/* break out of the switch, loop with the while */
	    }
	    length_acc=0;
	    goal = FAX_GOAL_AccumulateA1A2;
		/* in case I run out of data while getting a1-a2 distance */

	  case  FAX_GOAL_AccumulateA1A2:
	    get_a1a2(FAX_GOAL_FinishHoriz);
		/* If we have to return for data before getting the 	*/
		/* whole a1a2 distance, we want to return to the a1a2	*/
		/* accumulate state when we reenter the decoder. If	*/
		/* adjusting the bitstream forces us to return after	*/
		/* getting the last part of the a1a2 distance, then	*/
		/* we want to jump to state FAX_GOAL_FinishHoriz when	*/
		/* we reenter the decoder.				*/

	  case  FAX_GOAL_FinishHoriz:
	    a1a2 = length_acc;
	    /* XXX - I may regret not checking for a1a2 > 0 later... */
	    if (a1a2 < 0) {
		printf(" bad a1a2, %d\n",a1a2);
	        printf(" byteptr %x, endptr %x, bitpos %x %x%x%x\n",
	           byteptr,endptr,bitpos,*byteptr,*(byteptr+1),*(byteptr+2));
		return(-1);
	    }




	    new_trans[n_new_trans] = a0_pos = new_trans[n_new_trans-1]+a1a2;
	    ++n_new_trans;
	    if (a0_pos < 0) {
		printf(" decoding error, a0_pos is %d, a1a2 is %d\n",
			a0_pos,a1a2);
		printf(" number of transitions is %d, first %d, last %d\n",
			n_new_trans,new_trans[0],new_trans[n_new_trans-1]);
	        printf(" byteptr %x, endptr %x, bitpos %x %x%x%x\n",
	           byteptr,endptr,bitpos,*byteptr,*(byteptr+1),*(byteptr+2));
		return(-1);
	    }



	    /* it's possible we got here by reading a zero in get_a1a2, */
	    /* in which case 'goal' was set to FAX_GOAL_RecoverZero. We */
	    /* must assume that the zero is an EOL pre-padded with a 	*/
	    /* variable number of zeros, which is legal according to	*/
	    /* the coding spec.  So we will update the line count and	*/
	    /* then attempt to recover.					*/
	    if (goal == FAX_GOAL_RecoverZero) {
	        /* assume we have hit the begining of an EOL */
	        goal = FAX_GOAL_SeekFillAndEOL;
	        ++lines_found;
		printf(" hit zero, assuming it starts an EOL, %dlines\n",
			lines_found);
	        break;
	    }
	    if (rl == EOL_RUN_LENGTH) {
	        ++lines_found;
	        goal =  FAX_GOAL_StartNewLine;
		if (state->decoder_done)
	    		save_state_and_return(state);
		break;
	    }
	    if (a0_pos >= width) {
		if (a0_pos > width) {
		   state->decoder_done = FAX_DECODE_DONE_ErrorPastWidth;
		   return(lines_found);
		}
	        ++lines_found;
		if (state->decoder_done)
	    		save_state_and_return(state);
		goal = FAX_GOAL_SeekFillAndEOL;
		break;
	    }
	    else {
	        goal = FAX_GOAL_HandleHoriz;
	    }
	    break;


	  default:
	    state->decoder_done = FAX_DECODE_DONE_ErrorBadGoal;
	    return(lines_found);
	  break;
	  }  /* end of switch */
	}
/***	End, Main Decoding Loop	***/
}
/**** module fax/g31d.c ****/
