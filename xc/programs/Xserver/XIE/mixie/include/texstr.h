/* $XConsortium: texstr.h,v 1.2 93/10/31 09:46:19 dpw Exp $ */
/**** module texstr.h ****/
/****************************************************************************
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
******************************************************************************
  
	texstr.h -- DDXIE generic photoflo execution context structures
  
	Dean Verheiden, Robert NC Shelley -- AGE Logic, Inc. April 1993
  
*****************************************************************************/

#ifndef _XIEH_TEXSTR
#define _XIEH_TEXSTR

#include <tex.h>
#include <floman.h>
#include <sched.h>
#include <strip.h>

#include <domain.h>

/*
 *  link-pair for managing a circular doubly linked list of peTexRecs
 */
typedef struct _petlst {
  struct _petex *flink;
  struct _petex *blink; 
} petLstRec, *petLstPtr;

/*
 *  photoflo execution context structure
 */
typedef struct _flotex {
  CARD32         imports;	/* number of bands receiving client data  */
  CARD32         exports;	/* number of bands supplying client data  */
  CARD32	 exitCnt;	/* incremented each time scheduler exits  */
  petLstRec	 schedHead;	/* list of scheduled elements	 	  */
  stripLstRec	 stripHead;	/* list of reusable strips		  */
  CARD32	 stripSize;	/* size of standard cachable strips	  */
  CARD32	 putCnt;	/* incremented each time a strip is "Put" */
  pointer	 yieldPtr;	/* yielding info ptr			  */
} floTexRec, *floTexPtr;

/* 
 * receptor data structure
 */
typedef struct _receptor {
				  /* set of per-band Boolean state flags: */
  bandMsk   admit;		  /*    bands that will admit more data   */
  bandMsk   ready;		  /*    bands with available >= threshold */
  bandMsk   active;		  /*    bands that will produce more data */
  bandMsk   attend;		  /*    bands to consider when scheduling */
  bandMsk   bypass;		  /*    bands that bypass this element    */
  CARD8	    pad[3];
  inFloPtr  inFlo;		  /* pointer to receptor's inFlo	  */
  bandRec   band[xieValMaxBands]; /* per-band input context		  */
} receptorRec, *receptorPtr;

/*
 *  photo element execution context structure
 */
typedef struct _petex {
  struct _petex *flink;		  /* foreword link for scheduler list	  */
  struct _petex *blink;		  /* backword link for scheduler list	  */
  peDefPtr       peDef;		  /* pointer to the DIXIE element def     */
  outFloPtr      outFlo;	  /* pointer to element's outFlo	  */
  receptorPtr    receptor;	  /* pointer to in-line receptors	  */
  pointer        private;	  /* pointer to in-line private info	  */
  CARD32	 schedCnt;	  /* to detect that scheduler has resumed */
  CARD32         admissionCnt;	  /* number of receptors expecting data	  */
  BOOL	         inSync;	  /* true if inputs must be kept in sync  */
  BOOL		 bandSync;	  /* true if bands must be kept in sync   */
  bandMsk        scheduled;	  /* bits set if element is on ready-list */
  bandMsk	 emitting;	  /* mask of bands that are emitting data */
  bandRec        emitter[xieValMaxBands]; /* per-band output context	  */
  pointer   	 roi;		  /* pointer to run length/control plane  */
  Bool		 (*roiinit)();	  /* process domain line setup function   */
  INT32		 (*roiget)();	  /* process domain run length function   */
  INT32		 domXoff;	  /* processing domain x offset		  */
  INT32		 domYoff;	  /* processing domain y offset		  */
/*receptorRec    receptor[inCnt]; /* array of in-line input receptors	  */
/*void		 private_info;	  /* in-line element-private data	  */
} peTexRec, *peTexPtr;

#endif /* end _XIEH_TEXSTR */
