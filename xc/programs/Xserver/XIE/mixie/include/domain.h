/* $XConsortium$ */
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
  
	domain.h -- DDXIE MI prototype Process Domain definitions
  
	James H Weida  -- AGE Logic, Inc. July 1993
  
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
 * run-length table definition
 */

#define NPAIR 1
#define NUM_OF_BYTE_BITS  8

typedef CARD32 RUNLEN;	/* sizeof run-length entries */
typedef CARD8  CTLLEN;	/* sizeof control-plane entries */

typedef struct _runlengthhdr
{
	INT32 x, y; 		/* x & y coodinates */
	CARD32 nline;		/* number of repeats for this line */
	CARD32 npair;		/* number of run-length pairs */
}
RunLengthHdr;

typedef struct _runlengthpair
{
	RUNLEN count; 
	RUNLEN length;		/* length of process domain */
}
RunLengthPair;

typedef struct _runlengthtbl
{
	RunLengthHdr  hdr;			/* run-length header */
	RunLengthPair pair[NPAIR];	/* run-length pairs */
}
RunLengthTbl;

typedef struct _runlengthframe
{
	INT32 x, y;		/* min x and y coordinates */
	CARD32 width;		/* max width, i.e. width of table */

	CARD32 height;		/* height of table */
	CARD32 nentry;		/* number of run-length entries in table */
	CARD32 size;		/* size of table in bytes */
}
RunLengthRec, *RunLengthPtr, RLFrameRec, *RLFramePtr;

#endif /* module _XIEH_DOMAIN */
