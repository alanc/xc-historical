/* $XConsortium$ */
/**** module medraw.c ****/
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
  
	medraw.c -- DDXIE prototype export drawable element
  
	Robert NC Shelley -- AGE Logic, Inc. April, 1993
  
*****************************************************************************/

#define _XIEC_MEDRAW
#define _XIEC_EDRAW

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
#include <pixmapstr.h>
#include <gcstruct.h>
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
int	miAnalyzeEDraw();

/*
 *  routines used internal to this module
 */
static int CreateEDraw();
static int InitializeEDraw();
static int ActivateEDraw();
static int FlushEDraw();
static int ResetEDraw();
static int DestroyEDraw();


extern Bool	DrawableAndGC();

/*
 * DDXIE ExportDrawable entry points
 */
static ddElemVecRec EDrawVec = {
  CreateEDraw,
  InitializeEDraw,
  ActivateEDraw,
  FlushEDraw,
  ResetEDraw,
  DestroyEDraw
  };


/*------------------------------------------------------------------------
------------------- see if we can handle this element --------------------
------------------------------------------------------------------------*/
int miAnalyzeEDraw(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  /* for now just stash our entry point vector in the peDef */
  ped->ddVec = EDrawVec;

  return(TRUE);
}                               /* end miAnalyzeEDraw */

/*------------------------------------------------------------------------
---------------------------- create peTex . . . --------------------------
------------------------------------------------------------------------*/
static int CreateEDraw(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  /* attach an execution context to the photo element definition */
  return( MakePETex(flo,ped,0,FALSE,FALSE) );
}                               /* end CreateEDraw */

/*------------------------------------------------------------------------
---------------------------- initialize peTex . . . ----------------------
------------------------------------------------------------------------*/
static int InitializeEDraw(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  return( InitReceptors(flo,ped,0,1) );
}                               /* end InitializeEDraw */


/*------------------------------------------------------------------------
----------------------------- crank some data ----------------------------
------------------------------------------------------------------------*/
static int ActivateEDraw(flo,ped,pet)
     floDefPtr flo;
     peDefPtr  ped;
     peTexPtr  pet;
{
  xieFloExportDrawable *raw = (xieFloExportDrawable *) ped->elemRaw;
  eDrawDefPtr pvt = (eDrawDefPtr) ped->elemPvt;
  bandPtr     bnd = &pet->receptor[SRCtag].band[0];
  BytePixel  *src = GetSrc(BytePixel,flo,pet,bnd,bnd->minGlobal,FALSE);
  
  if(src) {
    if (!DrawableAndGC(flo,ped,raw->drawable,raw->gc,&pvt->pDraw,&pvt->pGC))
      return FALSE;
    do    
      (*pvt->pGC->ops->PutImage)(pvt->pDraw,		  /* drawable	 */
				 pvt->pGC,		  /* gc		 */
				 pvt->pDraw->depth,	  /* depth	 */
				 raw->dstX,		  /* drawable-x	 */
				 raw->dstY+bnd->minLocal, /* drawable-y	 */
				 bnd->format->width,	  /* width	 */
				 bnd->strip->length,	  /* height	 */
				 bnd->strip->bitOff,	  /* padding? 	 */
				 ZPixmap,		  /* data format */
				 src			  /* data buffer */
				 );
    while(src = GetSrc(BytePixel,flo,pet,bnd,bnd->maxLocal,FALSE));
  }
  /* make sure the scheduler knows how much src we used */
  FreeData(BytePixel,flo,pet,bnd,bnd->current);

  return(TRUE);
}                               /* end ActivateEDraw */


/*------------------------------------------------------------------------
--------------------------- get rid of left overs ------------------------
------------------------------------------------------------------------*/
static int FlushEDraw(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  /* Activate was suppose to do the whole image -- there's nothing to do */
  return(TRUE);
}                               /* end FlushEDraw */


/*------------------------------------------------------------------------
------------------------ get rid of run-time stuff -----------------------
------------------------------------------------------------------------*/
static int ResetEDraw(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  ResetReceptors(ped);
  
  return(TRUE);
}                               /* end ResetEDraw */


/*------------------------------------------------------------------------
-------------------------- get rid of this element -----------------------
------------------------------------------------------------------------*/
static int DestroyEDraw(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  /* get rid of the peTex structure  */
  if(ped->peTex)
    ped->peTex = (peTexPtr) XieFree(ped->peTex);

  /* zap this element's entry point vector */
  ped->ddVec.create     = (xieIntProc) NULL;
  ped->ddVec.initialize = (xieIntProc) NULL;
  ped->ddVec.activate   = (xieIntProc) NULL;
  ped->ddVec.flush      = (xieIntProc) NULL;
  ped->ddVec.reset      = (xieIntProc) NULL;
  ped->ddVec.destroy    = (xieIntProc) NULL;

  return(TRUE);
}                               /* end DestroyEDraw */

/* end module medraw.c */
