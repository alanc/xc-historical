/* $XConsortium$ */
/**** module meroi.c ****/
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
  
	meroi.c -- DDXIE prototype export roi element
  
	James H Weida -- AGE Logic, Inc. April, 1993
  
*****************************************************************************/

#define _XIEC_MEROI

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


/*
 *  routines referenced by other DDXIE modules
 */
int	miAnalyzeEROI();

/*
 *  routines used internal to this module
 */
static int CreateEROI();
static int InitializeEROI();
static int ActivateEROI();
static int FlushEROI();
static int ResetEROI();
static int DestroyEROI();

/*
 * DDXIE ImportROI entry points
 */
static ddElemVecRec meROIVec =
{
	CreateEROI,
	InitializeEROI,
	ActivateEROI,
	FlushEROI,
	ResetEROI,
	DestroyEROI
};


/*------------------------------------------------------------------------
------------------- see if we can handle this element --------------------
------------------------------------------------------------------------*/
int miAnalyzeEROI(flo,ped)
	floDefPtr flo;
	peDefPtr  ped;
{
	/* for now just stash our entry point vector in the peDef */
	ped->ddVec = meROIVec;
	return TRUE ;
}                               /* end miAnalyzeEROI */


/*------------------------------------------------------------------------
---------------------------- create peTex . . . --------------------------
------------------------------------------------------------------------*/
static int CreateEROI(flo,ped)
	floDefPtr flo;
	peDefPtr  ped;
{
	xieFloImportROI *raw;
  
	/* attach an execution context to the roi element definition */
	return MakePETex(flo,ped,NO_PRIVATE,SYNC,NO_SYNC);
}                               /* end CreateEROI */


/*------------------------------------------------------------------------
---------------------------- initialize peTex . . . ----------------------
------------------------------------------------------------------------*/
static int InitializeEROI(flo,ped)
	floDefPtr flo;
	peDefPtr  ped;
{
	return InitReceptors(flo,ped,NO_DATAMAP,1) &&
	       InitEmitter(flo,ped,NO_DATAMAP,NO_INPLACE);
}                               /* end InitializeEROI */


/*------------------------------------------------------------------------
----------------------------- crank some data ----------------------------
------------------------------------------------------------------------*/
static int ActivateEROI(flo,ped,pet)
     floDefPtr flo;
     peDefPtr  ped;
	peTexPtr  pet;
{
	receptorPtr  rcp =  pet->receptor;
	CARD32     bands =  rcp->inFlo->bands;
	bandPtr     sbnd = &rcp->band[0];
	bandPtr     dbnd = &pet->emitter[0];
	CARD8       *src;
	CARD32 b;

	/* get pointer to the initial src data (i.e. beginning of strip) */
	src = GetCurrentSrc(CARD8,flo,pet,sbnd);

	/* pass a clone of the current src strip to DIXIE */
	if(!PassStrip(flo,pet,dbnd,sbnd->strip))
		return FALSE;

	/* free the src strip(s) we've used */
	FreeData(CARD8,flo,pet,sbnd,sbnd->maxLocal);

	return TRUE;
}                               /* end ActivateEROI */


/*------------------------------------------------------------------------
--------------------------- get rid of left overs ------------------------
------------------------------------------------------------------------*/
static int FlushEROI(flo,ped)
	floDefPtr flo;
	peDefPtr  ped;
{
	return TRUE;
}                               /* end FlushEROI */


/*------------------------------------------------------------------------
------------------------ get rid of run-time stuff -----------------------
------------------------------------------------------------------------*/
static int ResetEROI(flo,ped)
	floDefPtr flo;
	peDefPtr  ped;
{
	ResetReceptors(ped);
	ResetEmitter(ped);
	return TRUE;
}                               /* end ResetEROI */


/*------------------------------------------------------------------------
-------------------------- get rid of this element -----------------------
------------------------------------------------------------------------*/
static int DestroyEROI(flo,ped)
	floDefPtr flo;
	peDefPtr  ped;
{
	/* get rid of the peTex structure  */
	ped->peTex = (peTexPtr) XieFree(ped->peTex);

	/* zap this element's entry point vector */
	ped->ddVec.create = (xieIntProc) NULL;
	ped->ddVec.initialize = (xieIntProc) NULL;
	ped->ddVec.activate = (xieIntProc) NULL;
	ped->ddVec.flush = (xieIntProc) NULL;
	ped->ddVec.reset = (xieIntProc) NULL;
	ped->ddVec.destroy = (xieIntProc) NULL;
	return TRUE;
}                               /* end DestroyEROI */

/* end module meroi.c */
