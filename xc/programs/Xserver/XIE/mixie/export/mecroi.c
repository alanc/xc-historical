/* $XConsortium$ */
/**** module mecroi.c ****/
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
  
	mecroi.c -- DDXIE export client roi element
  
	James H Weida -- AGE Logic, Inc. June, 1993
  
*****************************************************************************/

#define _XIEC_MECROI
#define _XIEC_ECROI

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


/* routines referenced by other DDXIE modules
 */
int	miAnalyzeECROI();

/* routines used internal to this module
 */
static int CreateECROI();
static int InitializeECROI();
static int ActivateECROI();
static int ResetECROI();
static int DestroyECROI();
static CARD32 ConvertRunLengthROI();

/* DDXIE ExportClientROI entry points
 */
static ddElemVecRec ECROIVec =
{
  CreateECROI,
  InitializeECROI,
  ActivateECROI,
  (xieIntProc)NULL,
  ResetECROI,
  DestroyECROI
};

/*  Local things.
 */
static CARD32 ConvertRunLengthROI();

/*------------------------------------------------------------------------
------------------- see if we can handle this element --------------------
------------------------------------------------------------------------*/
int miAnalyzeECROI(flo,ped)
	floDefPtr flo;
	peDefPtr  ped;
{
	/* for now just stash our entry point vector in the peDef */
	ped->ddVec = ECROIVec;
	return TRUE;
}                               /* end miAnalyzeECROI */

/*------------------------------------------------------------------------
---------------------------- create peTex . . . --------------------------
------------------------------------------------------------------------*/
static int CreateECROI(flo,ped)
	floDefPtr flo;
	peDefPtr  ped;
{
	/* attach an execution context to the roi element definition */
	return MakePETex(flo, ped, NO_PRIVATE, NO_SYNC, NO_SYNC);
}                               /* end CreateECROI */

/*------------------------------------------------------------------------
---------------------------- initialize peTex . . . ----------------------
------------------------------------------------------------------------*/
static int InitializeECROI(flo,ped)
	floDefPtr flo;
	peDefPtr  ped;
{
	return InitReceptors(flo,ped,NO_DATAMAP,1) &&
	         InitEmitter(flo,ped,NO_DATAMAP,NO_INPLACE);
}                               /* end InitializeECROI */

/*------------------------------------------------------------------------
----------------------------- crank some data ----------------------------
------------------------------------------------------------------------*/
static int ActivateECROI(flo,ped,pet)
     floDefPtr flo;
     peDefPtr  ped;
     peTexPtr  pet;
{
	xieFloExportClientROI *raw = (xieFloExportClientROI*)ped->elemRaw;
	receptorPtr	rcp = pet->receptor;
	bandPtr		sbnd = &rcp->band[0];
	bandPtr		dbnd = &pet->emitter[0];
	CARD8	 	*src, *dst;
	CARD32		szrects;

	if (!(src = GetCurrentSrc(CARD8,flo,pet,sbnd)))
	    return FALSE;

	/* One pass to compute transformed size of rectangle list */
	szrects = sizeof(xieTypRectangle) * ConvertRunLengthROI(src,NULL,FALSE);
	if (!(dst = GetDstBytes(CARD8,flo,pet,dbnd,0,szrects,FALSE)))
		return FALSE;

	/* Now actually crank the data */
	(void) ConvertRunLengthROI(src,dst,TRUE);

	SetBandFinal(dbnd);
	PutData(flo,pet,dbnd,szrects);
	FreeData(void,flo,pet,sbnd,sbnd->maxLocal);

	switch(raw->notify) {
	  case xieValFirstData:	/* fall thru */
	  case xieValNewData:	SendExportAvailableEvent(flo,ped,0,0,0,0);
	  default:		break;
	}
	return TRUE;
}                               /* end ActivateECROI */

/*------------------------------------------------------------------------
------------------------ get rid of run-time stuff -----------------------
------------------------------------------------------------------------*/
static int ResetECROI(flo,ped)
	floDefPtr flo;
	peDefPtr  ped;
{
	ResetReceptors(ped);
	ResetEmitter(ped);
	
	return TRUE;
}                               /* end ResetECROI */

/*------------------------------------------------------------------------
-------------------------- get rid of this element -----------------------
------------------------------------------------------------------------*/
static int DestroyECROI(flo,ped)
	floDefPtr flo;
	peDefPtr  ped;
{
	/* get rid of the peTex structure  */
	ped->peTex = (peTexPtr) XieFree(ped->peTex);

	/* zap this element's entry point vector */
	ped->ddVec.create     = (xieIntProc) NULL;
	ped->ddVec.initialize = (xieIntProc) NULL;
	ped->ddVec.activate   = (xieIntProc) NULL;
	ped->ddVec.flush      = (xieIntProc) NULL;
	ped->ddVec.reset      = (xieIntProc) NULL;
	ped->ddVec.destroy    = (xieIntProc) NULL;

	return TRUE;
}                               /* end DestroyECROI */


/*------------------------------------------------------------------------
--------------- convert from internal RL to external ROI  ----------------
------------------------------------------------------------------------*/

static CARD32
ConvertRunLengthROI(frame,roi,doit)
RunLengthPtr	frame;
xieTypRectangle *roi;
BOOL		doit;
{
	register int i, j;
	register CARD32 ndx;
	register RunLengthTbl *tbl;
	register INT32 x;
	INT32 xmin;

	tbl = (RunLengthTbl*)&frame[1];

	xmin = tbl->hdr.x;
	for (ndx = 0, i = 0; i < frame->nentry; i++) {
	    x = xmin;
	    for (j = 0; j < tbl->hdr.npair; j++) {
		if (doit) {
		    x += tbl->pair[j].count;
		    roi[ndx].x      = x;
		    roi[ndx].y      = tbl->hdr.y;
		    roi[ndx].width  = tbl->pair[j].length;
		    roi[ndx].height = tbl->hdr.nline;
		    x += tbl->pair[j].length;
		}
		ndx++;
	    }
	    tbl = (RunLengthTbl *)(&tbl->pair[tbl->hdr.npair]);
	}

	return ndx;
}					/* end ConvertRunLengthROI */

/* end module mecroi.c */
