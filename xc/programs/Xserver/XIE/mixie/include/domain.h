/* $XConsortium: domain.h,v 1.2 93/11/06 15:34:46 rws Exp $ */
/**** module domain.h ****/
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
     
     "Copyright 1993, 1994 by AGE Logic, Inc. and the Massachusetts
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
  
	domain.h -- DDXIE MI Process Domain definitions
  
	Dean Verheiden  -- AGE Logic, Inc. August 1993
  
*****************************************************************************/


#ifndef _XIEH_DOMAIN
#define _XIEH_DOMAIN

#if !defined(_XIEC_DOMAIN)

#define	SyncDomain	(*pet->roiinit)
#define	GetRun		(*pet->roiget)

extern Bool  InitProcDomain();
extern void  ResetProcDomain();

#endif /* ! defined _XIEC_DOMAIN */

/*
 * Rectangles of Interest structure definitions 
 */

#define RUNPTR(idx)	(&((runPtr)&lp[1])[idx])
#define LEND(rband)	((linePtr)&rband->strip->data[rband->strip->length])

typedef struct _runRec {
	CARD32 dstart; 		/* delta to start of run */
	CARD32 length;		/* length of run */
} runRec, *runPtr;

typedef struct _linerec {
	INT32	y;		/* starting y coodinate of this y-x band */
	CARD32	nline;		/* height of y-x band */
	CARD32  nrun;		/* number of runRecs for this y-x band */
/*      runRec  run[nrun]; */
} lineRec, *linePtr;

typedef struct _ROIrec {
	INT32   x, y;		/* min x and y coordinates of bounding box */
	CARD32  width;		/* width  of bounding box 		   */
	CARD32  height;		/* height of bounding box 		   */
	CARD32  nrects;		/* number of rectangles in this ROI 	   */
	linePtr lend;		/* first address past end of ROI table	   */
} ROIRec, *ROIPtr;

#endif /* module _XIEH_DOMAIN */
