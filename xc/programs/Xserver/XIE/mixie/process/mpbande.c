/* $XConsortium$ */
/**** module mpbande.c ****/
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
  
	mpbande.c -- DDXIE BandExtract element
  
	Robert NC Shelley -- AGE Logic, Inc. June, 1993
  
*****************************************************************************/

#define _XIEC_MPBANDE
#define _XIEC_PBANDE

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
int	miAnalyzeBandExt();

/* routines used internal to this module
 */
static int CreateBandExt();
static int InitializeBandExt();
static int SingleBandExt();
static int TripleBandExt();
static int ResetBandExt();
static int DestroyBandExt();

/* DDXIE BandExtract entry points
 */
static ddElemVecRec BandExtVec = {
  CreateBandExt,
  InitializeBandExt,
  (xieIntProc)NULL,
  (xieIntProc)NULL,
  ResetBandExt,
  DestroyBandExt
  };


/*------------------------------------------------------------------------
------------------- see if we can handle this element --------------------
------------------------------------------------------------------------*/
int miAnalyzeBandExt(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  pBandExtDefPtr pvt = (pBandExtDefPtr)ped->elemPvt;

  if(pvt->mix_bands)
    ImplementationError(flo,ped, return(FALSE));

  /* for now just stash our entry point vector in the peDef */
  ped->ddVec = BandExtVec;
  ped->ddVec.activate = pvt->mix_bands ? TripleBandExt : SingleBandExt;

  return(TRUE);
}                               /* end miAnalyzeBandExt */

/*------------------------------------------------------------------------
---------------------------- create peTex . . . --------------------------
------------------------------------------------------------------------*/
static int CreateBandExt(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  pBandExtDefPtr pvt = (pBandExtDefPtr)ped->elemPvt;

  /* attach an execution context to the photo element definition */
  return( MakePETex(flo, ped, 0, FALSE, pvt->mix_bands) );
}                               /* end CreateBandExt */


/*------------------------------------------------------------------------
---------------------------- initialize peTex . . . ----------------------
------------------------------------------------------------------------*/
static int InitializeBandExt(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  pBandExtDefPtr pvt = (pBandExtDefPtr)ped->elemPvt;

  return((pvt->mix_bands
	  ? InitReceptors(flo, ped, 0, 1)
	  : InitReceptor( flo, ped, &ped->peTex->receptor[SRCtag], 0, 1,
			 (CARD8)1<<pvt->out_band, (CARD8)0))
	 && InitEmitter(flo, ped, 0, -1));
}                               /* end InitializeBandExt */


/*------------------------------------------------------------------------
------------------------ crank some single input data --------------------
------------------------------------------------------------------------*/
static int SingleBandExt(flo,ped,pet)
     floDefPtr flo;
     peDefPtr  ped;
     peTexPtr  pet;
{
  pBandExtDefPtr pvt = (pBandExtDefPtr)ped->elemPvt;
  CARD32           b =  pvt->out_band;
  receptorPtr    rcp =  pet->receptor;
  bandPtr       sbnd = &rcp->band[b];
  bandPtr       dbnd =  pet->emitter;
  
  /* pass the chosen receptor band to our output band
   */
  if(GetCurrentSrc(void,flo,pet,sbnd)) {
    do {
      /* pass a clone of the current src strip downstream
       */
      if(!PassStrip(flo,pet,dbnd,sbnd->strip))
	return(FALSE);
    } while(GetSrc(void,flo,pet,sbnd,sbnd->maxLocal,TRUE));

    FreeData(void,flo,pet,sbnd,sbnd->maxLocal);
  }
  return(TRUE);
}                               /* end SingleBandExt */


/*------------------------------------------------------------------------
------------------------ crank some single input data --------------------
------------------------------------------------------------------------*/
static int TripleBandExt(flo,ped,pet)
     floDefPtr flo;
     peDefPtr  ped;
     peTexPtr  pet;
{
  pBandExtDefPtr pvt = (pBandExtDefPtr)ped->elemPvt;

  ImplementationError(flo,ped, return(FALSE));
}                               /* end TripleBandExt */


/*------------------------------------------------------------------------
------------------------ get rid of run-time stuff -----------------------
------------------------------------------------------------------------*/
static int ResetBandExt(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  ResetReceptors(ped);
  ResetEmitter(ped);
  
  return(TRUE);
}                               /* end ResetBandExt */


/*------------------------------------------------------------------------
-------------------------- get rid of this element -----------------------
------------------------------------------------------------------------*/
static int DestroyBandExt(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  /* get rid of the peTex structure  */
  ped->peTex = (peTexPtr) XieFree(ped->peTex);
  
  /* zap this element's entry point vector */
  ped->ddVec.create     = (xieIntProc)NULL;
  ped->ddVec.initialize = (xieIntProc)NULL;
  ped->ddVec.activate   = (xieIntProc)NULL;
  ped->ddVec.reset      = (xieIntProc)NULL;
  ped->ddVec.destroy    = (xieIntProc)NULL;
  
  return(TRUE);
}                               /* end DestroyBandExt */

/* end module mpbande.c */
