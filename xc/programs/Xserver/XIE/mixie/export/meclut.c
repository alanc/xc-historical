/* $XConsortium: meclut.c,v 1.1 93/10/26 09:49:37 rws Exp $ */
/**** module meclut.c ****/
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
  
	meclut.c -- DDXIE export client LUT element
  
	Larry Hare -- AGE Logic, Inc. July, 1993
  
*****************************************************************************/

#define _XIEC_MECLUT
#define _XIEC_ECLUT

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
#include <lut.h>


/* routines referenced by other DDXIE modules
 */
int	miAnalyzeECLUT();

/* routines used internal to this module
 */
static int CreateECLUT();
static int InitializeECLUT();
static int ActivateECLUT();
static int ResetECLUT();
static int DestroyECLUT();

/* DDXIE ExportClientLUT entry points
 */
static ddElemVecRec ECLUTVec = {
  CreateECLUT,
  InitializeECLUT,
  ActivateECLUT,
  (xieIntProc)NULL,
  ResetECLUT,
  DestroyECLUT
  };

/*------------------------------------------------------------------------
------------------- see if we can handle this element --------------------
------------------------------------------------------------------------*/
int miAnalyzeECLUT(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  /* for now just stash our entry point vector in the peDef */
  ped->ddVec = ECLUTVec;

  return(TRUE);
}                               /* end miAnalyzeECLUT */

/*------------------------------------------------------------------------
---------------------------- create peTex . . . --------------------------
------------------------------------------------------------------------*/
static int CreateECLUT(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  /* attach an execution context to the photo element definition */
  return MakePETex(flo, ped, NO_PRIVATE, NO_SYNC, NO_SYNC);
}                               /* end CreateECLUT */

/*------------------------------------------------------------------------
---------------------------- initialize peTex . . . ----------------------
------------------------------------------------------------------------*/
static int InitializeECLUT(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  return InitReceptors(flo,ped,NO_DATAMAP,1) &&
	 InitEmitter(flo,ped,NO_DATAMAP,NO_INPLACE);
}                               /* end InitializeECLUT */

/*------------------------------------------------------------------------
----------------------------- crank some data ----------------------------
------------------------------------------------------------------------*/
static int ActivateECLUT(flo,ped,pet)
     floDefPtr flo;
     peDefPtr  ped;
     peTexPtr  pet;
{
  xieFloExportClientLUT *raw = (xieFloExportClientLUT *)ped->elemRaw;
  receptorPtr	rcp = pet->receptor;
  CARD32	bands = rcp->inFlo->bands;
  bandPtr	dbnd, sbnd = &rcp->band[0];
  CARD32	*start  = &(raw->start0);
  CARD32	*length = &(raw->length0);
  CARD8	 	*src, *dst;
  CARD32	b, pitch, nentry, dlen;
  Bool		swizzle;

  swizzle = (bands == 3) && (raw->bandOrder != xieValLSFirst);
  
  for(b = 0; b < bands; ++sbnd, ++b) {
    
    if (!(src = GetCurrentSrc(CARD8,flo,pet,sbnd))) continue; 

    dbnd = &pet->emitter[swizzle ? xieValMaxBands - b - 1 : b];
    
    nentry = length[b] ? length[b] : sbnd->format->height;
    pitch = LutPitch(sbnd->format->levels);
    dlen = nentry * pitch;
    
    if (!(dst = GetDstBytes(CARD8,flo,pet,dbnd,dbnd->current,dlen,KEEP)))
      return FALSE;

    if (start[b])
      src += start[b] * pitch;       /* Adjust via start[b] && length[b] */

    memcpy(dst,src,dlen);

    SetBandFinal(dbnd);
    PutData(flo,pet,dbnd,dbnd->maxGlobal);
    
    FreeData(flo,pet,sbnd,sbnd->maxLocal);

    switch(raw->notify) {
    case xieValFirstData:	/* fall thru */
    case xieValNewData:		SendExportAvailableEvent(flo,ped,b,nentry,0,0);
    default:			break;
    }
  }
  return(TRUE);
}                               /* end ActivateECLUT */

/*------------------------------------------------------------------------
------------------------ get rid of run-time stuff -----------------------
------------------------------------------------------------------------*/
static int ResetECLUT(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  ResetReceptors(ped);
  ResetEmitter(ped);
  
  return(TRUE);
}                               /* end ResetECLUT */

/*------------------------------------------------------------------------
-------------------------- get rid of this element -----------------------
------------------------------------------------------------------------*/
static int DestroyECLUT(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  /* get rid of the peTex structure  */
  ped->peTex = (peTexPtr) XieFree(ped->peTex);

  /* zap this element's entry point vector */
  ped->ddVec.create     = (xieIntProc) NULL;
  ped->ddVec.initialize = (xieIntProc) NULL;
  ped->ddVec.activate   = (xieIntProc) NULL;
  ped->ddVec.reset      = (xieIntProc) NULL;
  ped->ddVec.destroy    = (xieIntProc) NULL;

  return(TRUE);
}                               /* end DestroyECLUT */

/* end module meclut.c */
