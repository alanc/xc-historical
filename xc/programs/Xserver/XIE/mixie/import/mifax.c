/* $XConsortium$ */
/**** module mig4.c ****/
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
  
	miactg4.c --    DDXIE prototype import client photo element,
			portions specific to G4 decompression
  
	Ben Fahy -- AGE Logic, Inc. July, 1993
  
*****************************************************************************/

#define _XIEC_MICPHOTO
#define _XIEC_ICPHOTO

/*
 *  Include files
 */
#include <stdio.h>
/*
 *  Core X Includes
 */
#include <X.h>
#include <Xproto.h>
/*
 *  XIE Includes
 */
#include <XIE.h>
#include <XIEproto.h>
/*
 *  more X server includes.
 */
#include <misc.h>
#include <dixstruct.h>
#include <extnsionst.h>
/*
 *  Server XIE Includes
 */
#include <error.h>
#include <macro.h>
#include <element.h>
#include <texstr.h>
#include <fax.h>
/* XXX - this should be cleaned up. */
#include "../fax/faxint.h"

/*
 *  routines referenced by other DDXIE modules
/*
int CreateICPhotoG42D();
int InitializeICPhotoG42D();
int ActivateICPhotoG42D();
int DestroyICPhotoG42D();

/*
 * Local Declarations
 */

#define MAX_STRIPS_INC	20
typedef struct _g4pvt {
  FaxState state;
  int max_strips;	/* how many we've allocated space for	*/
  int n_strips;		/* how many strips we've seen so far 	*/
  int next_byte;	/* next input byte we are looking for	*/
  int max_lines;	/* maximum number of lines/output strip	*/
  char **o_lines;	/* array of pointers to output lines	*/
  int 	(*decodptr)();	/* function used to decode the data	*/
  xieTypOrientation encodedOrder;
} g3or4PvtRec, *g3or4PvtPtr;


/*------------------------------------------------------------------------
---------------------------- create peTex . . . --------------------------
------------------------------------------------------------------------*/
int CreateICPhotoG42D(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  /* attach an execution context to the photo element definition */
  return MakePETex(flo,ped, sizeof(g3or4PvtRec), NO_SYNC, NO_SYNC); 
}                               /* end CreateICPhotoG42D */

/*------------------------------------------------------------------------
---------------------------- initialize peTex . . . ----------------------
------------------------------------------------------------------------*/
int InitializeICPhotoG42D(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  xieFloImportClientPhoto *raw = (xieFloImportClientPhoto *)ped->elemRaw;
  xieTecDecodeG42D *tecG42D=(xieTecDecodeG42D *) &raw[1];
  xieTecDecodeG42D *tecG32D=(xieTecDecodeG32D *) &raw[1];
  xieTecDecodeG42D *tecG31D=(xieTecDecodeG31D *) &raw[1];
  peTexPtr  pet=ped->peTex;
  g3or4PvtPtr  texpvt = (g3or4PvtPtr) pet->private;
  bandPtr   dbnd = &pet->emitter[0];
  int	    pbytes;

  switch(raw->decodeTechnique) {
  case xieValDecodeG31D: 
	texpvt->decodptr = decode_g31d;   
	texpvt->encodedOrder = tecG31D->encodedOrder;
  	texpvt->state.goal     = FAX_GOAL_SkipPastAnyToEOL;
	break;
  case xieValDecodeG32D:
	texpvt->decodptr = decode_g32d;   
	texpvt->encodedOrder = tecG32D->encodedOrder;
  	texpvt->state.goal     = FAX_GOAL_SeekEOLandTag;
	break;
  case xieValDecodeG42D: 
	texpvt->decodptr = decode_g4;   
	texpvt->encodedOrder = tecG42D->encodedOrder;
  	texpvt->state.goal     = FAX_GOAL_StartNewLine;
	break;
  }


/* the following will be freed automatically */
  texpvt->state.width    = raw->width0;
  texpvt->state.a0_color = WHITE;
  texpvt->state.a0_pos   = (-1);

/* things that are zero don't really need to be initialized - consider
 * these comments.
 */
  texpvt->state.magic_needs = 0;
  texpvt->state.strip_state = StripStateNone;
  texpvt->state.strip	    = 0;
  texpvt->state.strip_size  = 0;
  texpvt->state.final  	    = 0;
  texpvt->state.o_line	    = 0;

/*
 * Things that do need to be initialized
 */
  texpvt->state.write_data = 1;
  texpvt->max_strips = MAX_STRIPS_INC;


/* the following must be freed explicitly */

  texpvt->state.old_trans = (int *) XieMalloc(raw->width0 * sizeof(int));
  if (!texpvt->state.old_trans) AllocError(flo,ped, return(FALSE));

  texpvt->state.new_trans = (int *) XieMalloc(raw->width0 * sizeof(int));
  if (!texpvt->state.new_trans) AllocError(flo,ped, return(FALSE));

/* 
 * I suppose this will make Bob mad at me, but... how else do I
 * figure out the output map size? 
 */

  pbytes = (ped->outFlo.format[0].pitch + 7) >> 3;

  texpvt->max_lines = flo->floTex->stripSize / pbytes;

  if (!texpvt->max_lines)
	texpvt->max_lines++;


/* set output emitter to map texpvt->max_lines lines of data */
  return( InitReceptors(flo, ped, NO_DATAMAP, 1) && 
	  InitEmitter(flo, ped, texpvt->max_lines, NO_INPLACE) );
}                               /* end InitializeICPhotoG4 */


/*------------------------------------------------------------------------
----------------------------- crank some data ----------------------------
------------------------------------------------------------------------*/
int ActivateICPhotoG42D(flo,ped,pet)
     floDefPtr flo;
     peDefPtr  ped;
     peTexPtr  pet;
{
  xieFloImportClientPhoto *raw = (xieFloImportClientPhoto *)ped->elemRaw;
  xieTecDecodeG42D *tecG42D=(xieTecDecodeG42D *) &raw[1];
  xieTecDecodeG32D *tecG32D=(xieTecDecodeG32D *) &raw[1];
  xieTecDecodeG31D *tecG31D=(xieTecDecodeG31D *) &raw[1];
  bandPtr   sbnd = &pet->receptor[IMPORT].band[0];
  bandPtr   dbnd = &pet->emitter[0];
  CARD32    olen = dbnd->format->pitch+7>>3;
  g3or4PvtPtr  texpvt = (g3or4PvtPtr) ped->peTex->private;
  FaxState  *state = &(texpvt->state);
  CARD32   min_len = max(MIN_BYTES_NEEDED, sbnd->available);
  BytePixel *src, *dst;
  int	    lines_found;
  Bool ok;
  int 	(*decodptr)() = texpvt->decodptr;

/*
 *  get current input and output strips
 */
 /* 
  * Important!  As long as there is source data available, we'd better
  * deal with it, because it's probably owned by Core X and will die
  * if we let the scheduler go away.  It's ok to exit with TRUE to let
  * an output strip be written (we'll be called back). It is death to
  * exit with FALSE unless we are absolutely sure we are done with all
  * current input strips.
  */
  if(src = GetSrcBytes(BytePixel,flo,pet,sbnd,sbnd->current,min_len,KEEP)) {
     /* 
      * Ok, so if we are here, there is src available. We will ignore it
      * though if we already have a strip scrolled away in state->strip.
      */
      if (!state->strip) {
	 state->strip 	    = (unsigned char *) src;
	 state->strip_state = StripStateNew;
  	 state->strip_size  = sbnd->maxLocal - sbnd->minLocal;
  	 state->final  	    = sbnd->final;
	 if (texpvt->encodedOrder == xieValLSFirst) {
	   register int i,size=state->strip_size;
	   register unsigned char *ucp = state->strip;
	   for (i=0; i<size; ++i)
		*ucp++ = _ByteReverseTable[*ucp];
	 }

#ifdef foo
         texpvt->next_byte += state->strip_size;
#endif
      }
     /*
      * We have some data to decode, anything to write to?
      */
      while (dst = GetDst(BytePixel,flo,pet,dbnd,state->o_line,KEEP)) {


	/*
 	 *  Now, as much as I'd like to use our nifty line-oriented macros,
 	 *  it would kill performance.  So instead we ask the decoder to 
 	 *  decode as much as remains in the output line as it can get.
 	 *  We ask the data manager to map all the lines of data in the 
	 *  current output strip to a convenient array.
 	 */

  	state->nl_sought = dbnd->maxLocal - state->o_line; 

  	ok = MapData(flo,pet,dbnd,0,dbnd->current,state->nl_sought,KEEP);
		/* map desired lines into an array starting at 0 */
  	/* printf(" tring to map dst, got %d\n",ok); */
  	if (!ok)
      	    ImplementationError(flo,ped, return(FALSE));

  	state->o_lines = (char **) dbnd->dataMap;

  	lines_found = (*decodptr)(&(texpvt->state));
  	if (lines_found < 0) {
     		ImplementationError(flo,ped, return(FALSE));
	} else {
		state->o_line += lines_found;
		PutData(flo,pet,dbnd,state->o_line);
        }
	if (state->decoder_done) {
		/* decoders sometimes return errors when they run
		   out of data. ignore them if we got all the lines
		   we wanted.
		*/
		if (state->o_line < raw->height0 &&
		    state->decoder_done > FAX_DECODE_DONE_OK) {
		   printf(" error code returned is %d\n",
			state->decoder_done);
     	           ImplementationError(flo,ped, return(FALSE));
		}
  	      	FreeData(BytePixel,flo,pet,sbnd,sbnd->maxGlobal);
	       	break;
	}

	if (state->magic_needs) {
	    /* decoder needs a new strip */
	    if (state->strip_state != StripStateDone)  {
     	       ImplementationError(flo,ped, return(FALSE));
	    }
  	    FreeData(BytePixel,flo,pet,sbnd,sbnd->maxLocal);

  	    texpvt->state.strip_state = StripStateNone;
  	    texpvt->state.strip	      = 0;
  	    texpvt->state.strip_size  = 0;

	    if (!state->final) 
	        break;
	}
      } /* end of while(dst = ...) */
     /*
      *  No more dst or need another src strip? three possible reasons:
      *
      *  1:  we're done with output image.  Just shut down and be happy
      *  2:  scheduler has noticed a downstream element can run. In this
      *      case, we should be a good guy and return TRUE so the scheduler
      *      gives us back control again later (without going back to Core
      *	     X, which would cause our input strip to vanish).
      *	 3:  we ran out of src.
      */
      if (!dst && dbnd->final) {
  	 FreeData(BytePixel,flo,pet,sbnd,sbnd->maxGlobal);
      }
      SetBandThreshold(sbnd, !dst ? 1 : sbnd->available + 1);
  }  /* end of if (src = ...) */

/* 
 * if here, we ran out of src.  Scheduler will wake us up again
 * when a PutClientData request comes along.
 */
  return TRUE;	/* Hmmm. Well, I think I did my part ok... */
}                               /* end ActivateICPhotoG4 */


/*------------------------------------------------------------------------
-------------------------- get rid of this element -----------------------
------------------------------------------------------------------------*/
int DestroyICPhotoG42D(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  /* get rid of the peTex structure  */
  if(ped->peTex) {
     g3or4PvtPtr texpvt = (g3or4PvtPtr) ped->peTex->private;

    /* only have to parts of private structure which were malloc'd */
    if (texpvt->state.old_trans)
	texpvt->state.old_trans = (int *)XieFree(texpvt->state.old_trans);
    if (texpvt->state.new_trans)
	texpvt->state.new_trans = (int *)XieFree(texpvt->state.new_trans);

    /* this frees everything else */
    ped->peTex = (peTexPtr) XieFree(ped->peTex);
  }

  /* zap this element's entry point vector */
  ped->ddVec.create     = (xieIntProc) NULL;
  ped->ddVec.initialize = (xieIntProc) NULL;
  ped->ddVec.activate   = (xieIntProc) NULL;
  ped->ddVec.flush      = (xieIntProc) NULL;
  ped->ddVec.reset      = (xieIntProc) NULL;
  ped->ddVec.destroy    = (xieIntProc) NULL;

  return(TRUE);
}                               /* end DestroyICPhoto */

/* end module mig4.c */
