/* $XConsortium$ */
/**** module fax/g4.c ****/
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
  
	fax/g4.c -- DDXIE G4 fax decode technique/element
  
	Ben Fahy -- AGE Logic, Inc. May, 1993
  
*****************************************************************************/

#define RETURN_after_anything 0

#define use_verbosity_groups_not
#define vbdecoder 0

#include "fax.h"
#include "faxint.h"
#include "bits.h"
#include "verbosity.h"

#include <servermd.h>
	/* pick up the BITMAP_BIT_ORDER from Core X*/


/**********************************************************************/
int decode_g4(state)
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

int 	lines_found=0;
int	n_old_trans;
int	n_new_trans;
int	*old_trans;
int	*new_trans;
int 	a0a1,a1a2;
int 	length_acc=0;
int 	last_b1_idx=0;
int 	b1_pos,b2_pos;
int 	rl;
int	magic_blab = 0;


	if (state == (FaxState *)NULL)
		return(-1);

	/* set up initial bitstream for the very first strip */
	if (!state->bits.started) {
	    if (state->strip_state != StripStateNew) {
	       if (vbdecoder || be_verbose(decoder)) 
	 	 printf(" %s%d,program error, unexpected strip state\n",
			__FILE__,__LINE__,state->strip_state);
	       return(-1);
	    }
	    state->bits.byteptr = (unsigned char *)state->strip;
	    state->bits.endptr  = state->bits.byteptr + state->strip_size-4;
	       /* we will panic with four bytes to go */

	    if (vbdecoder || be_verbose(decoder)) {
		printf(" started a bitstream for strip 0x%x, len %d\n",
			state->strip,
			state->strip_size);
	    }
	    state->bits.bitpos = 0;
	    state->bits.started = 1;
	}

	localize_state(state);

	if (state->magic_needs) 
	    finish_magic(state->final);
		/* a magic strip was waiting for 1st word of next strip */

	if (vbdecoder || be_verbose(decoder)) {
	    printf(" seeking to decode %d lines\n",state->nl_sought);
	    printf(" bitstream is at 0x%x + %d bits, limit 0x%x\n",
		byteptr,bitpos,endptr);
	    printf(" four bytes starting at 0x%x are %x %x %x %x\n",
		byteptr,*byteptr,*(byteptr+1),*(byteptr+2),*(byteptr+3));
	    fflush(stdout);
	}

/***	Main Decoding Loop	***/
	while(1) {
	  set_verbosity_group_level(decoder,3);
	  if (vbdecoder || be_verbose(decoder)) {		
	    printf(" bitstream is at 0x%x + %d bits, limit 0x%x\n",
		byteptr,bitpos,endptr);
	    printf(" four bytes starting at 0x%x are %x %x %x %x\n",
		byteptr,*byteptr,*(byteptr+1),*(byteptr+2),*(byteptr+3));
	    fflush(stdout);
	  }
	  set_verbosity_group_level(decoder,2);
	  if (vbdecoder || be_verbose(decoder))
	  	printf(" goal is %d\n",goal);
	  switch(goal) {
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
	    goal     = FAX_GOAL_DetermineMode;
	    reset_transitions();
	    set_verbosity_group_level(decoder,3);
	    if (vbdecoder || be_verbose(decoder)) {
		printf(" new goal is determine mode\n");
		fflush(stdout);
	    }
	    set_verbosity_group_level(decoder,2);
	    break;
	  case FAX_GOAL_DetermineMode:
	    get_mode_and_length(mode,length,byteptr,bitpos,endptr); 
	    goal = mode;
		/* our goal is now to handle whatever mode we're in! */

	    if (mode != FAX_MODE_Unknown) 
	         adjust_bitstream_8(length,byteptr,bitpos,endptr);
	    break;

	  case  FAX_GOAL_HandleHoriz: 
	    if (vbdecoder || be_verbose(decoder)) {
		printf(" wow! I'm in horizontal mode!\n");
	    }
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
	    if (vbdecoder || be_verbose(decoder)) {
	      printf(" a0a1 is %d!\n",a0a1);
	      fflush(stdout);
	    }
	    if (vbdecoder || be_verbose(decoder)) {
	      printf(" will try to finish horizontal stuff\n");
	      fflush(stdout);
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
	    if (vbdecoder || be_verbose(decoder)) {
	      printf(" a1a2 is %d!\n",a1a2);
	      fflush(stdout);
	    }
	    if (a0_pos < 0) {
		/* at start of line, a0a1 is the number of white pixels,  */
		/* which is also the index on the line where white->black */
		new_trans[n_new_trans++] = a0a1;
		new_trans[n_new_trans] = a0_pos = a0a1 + a1a2;
		++n_new_trans;
	    } else {
		/* in middle of the line, a0a1 is run-length, so
		 * a1_pos = a0_pos + a0a1, a2_pos = a1_pos + a1a2
		 *
		 *  0  1  2  3  4  5  6  7  8  9  A  B  C  D 
		 *    |w |w |w |b |b |b |b |b |w |  |  |  |  |
		 *     a0       a1             a2
		 */ 
		new_trans[n_new_trans++] = a0_pos + a0a1;
		new_trans[n_new_trans]   = a0_pos = a0_pos + a0a1 + a1a2;
		++n_new_trans;
	    }
	    if (vbdecoder || be_verbose(decoder))
	        printf("  a0_pos is now %d\n",a0_pos);
	    if (a0_pos >= width) {
		if (a0_pos > width) {
		   if (vbdecoder || be_verbose(decoder))
		     printf(" error, a0 (%d) spilled past width (%d)\n",
			a0_pos,width);
		   return(-1);
		}
		if (vbdecoder || be_verbose(decoder))
		     printf(" found end of line with Horizontal!\n");
	        ++lines_found;
		goal = FAX_GOAL_StartNewLine;
		break;
	    }
	    else 
	        goal = FAX_GOAL_DetermineMode;
	    break;

	  case FAX_MODE_Pass:
	    if (vbdecoder || be_verbose(decoder)) {
	      printf(" Yoiks! I'm in pass mode!\n");
	      fflush(stdout);
	    }
	    if (!n_old_trans) {	/* line above all white */
	      if (vbdecoder || be_verbose(decoder)) {
		printf(" line above pass mode has no transitions - ");
		printf(" is that legal?\n");
		fflush(stdout);
	      }
	      return(-1);
	    }
	    find_b2pos(a0_pos,a0_color,n_old_trans,old_trans);
	    if (vbdecoder || be_verbose(decoder))
	        printf(" %s-%d: b2 pos returned is %d\n",
		     __FILE__,__LINE__,b2_pos);

	    a0_pos = b2_pos;
	    if (vbdecoder || be_verbose(decoder))
	        printf("  after passing, a0_pos is now %d\n",a0_pos);
	    if (a0_pos < 0) {
	       if (vbdecoder || be_verbose(decoder))
	         printf(" Yoiks! pass mode yielded b2_pos of %d!\n",b2_pos);
	       ++lines_found;
	       goal = FAX_GOAL_StartNewLine;
	       break;
	    }
	    if (a0_pos >= width) {
		printf(" %s(%d) Egads! a0_pos is %d\n",
			__FILE__,__LINE__,a0_pos);
		return(-1);
	    }
	    goal = FAX_GOAL_DetermineMode;
	    break;

	  case FAX_GOAL_HandleVertL3:
	  case FAX_GOAL_HandleVertL2:
	  case FAX_GOAL_HandleVertL1:
	  case FAX_GOAL_HandleVert0:
	  case FAX_GOAL_HandleVertR1:
	  case FAX_GOAL_HandleVertR2:
	  case FAX_GOAL_HandleVertR3:
	    if (vbdecoder || be_verbose(decoder)) {
	      printf(" about to try some vertical stuff\n");
	      fflush(stdout);
	    }
	    if (n_old_trans) {	/* line above not all white */
		if (vbdecoder || be_verbose(decoder)) {
		    printf(" egads! line above us (at %d) not all white!\n",
			a0_pos);
		    printf(" number of transitions was %d, last idx %d was %d\n",
			n_old_trans,last_b1_idx,old_trans[last_b1_idx]);
		}
		find_b1pos(a0_pos,a0_color,n_old_trans,old_trans);
		if (vbdecoder || be_verbose(decoder))
		   printf(" %s-%d: b1 pos returned is %d\n",
		     __FILE__,__LINE__,b1_pos);

		if (b1_pos < 0) {
		   if (goal > FAX_GOAL_HandleVert0) {
		     if (vbdecoder || be_verbose(decoder)) {
			printf(" got b1pos negative, vertical RIGHT mode\n");
		        printf(" %s-%d: b1 pos is %d, a0_pos %d\n",
		          __FILE__,__LINE__,b1_pos,a0_pos);
		     }
		     return(-1);	/* error! */
		   }
		   b1_pos = width;
		}
		/* ok, we are guaranteed that 0 <= b1_pos <= width */
		/* set a0_pos = a1_pos,  which is relative to b1_pos */
		a0_pos   = b1_pos + (goal-FAX_GOAL_HandleVert0);
		a0_color = 1-a0_color;
	        if (a0_pos >= width) {
	            ++lines_found;
		    goal = FAX_GOAL_StartNewLine;
		}
		else  {
		    /* not at eol yet */
		    goal = FAX_GOAL_DetermineMode;
		    new_trans[n_new_trans++] = a0_pos;
		}
	    }
	    else {		
	       /* line above was all white. Since b1 was first non-white */
	       /* b1 is imaginary transition off right edge.  In other   */
	       /* words, there are no more transitions! Done with line	 */

	        ++lines_found;
		if (vbdecoder || be_verbose(decoder))
		   printf(" no more vertical transitions: found %d lines!!\n",
			lines_found);
		goal = FAX_GOAL_StartNewLine;
	    }
	    break;

	  default:
	  break;
	  }  /* end of switch */
	}
/***	End, Main Decoding Loop	***/
}
/**** module fax/g4.c ****/
