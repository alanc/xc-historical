/* $XConsortium: midraw.c,v 1.2 93/07/20 11:35:14 rws Exp $ */
/**** module midraw.c ****/
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
  
	midraw.c -- DDXIE prototype import drawable element
  
	Larry Hare -- AGE Logic, Inc. June, 1993
  
*****************************************************************************/

#define _XIEC_IDRAW
#define _XIEC_MIDRAW

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
#include <scrnintstr.h>
#include <pixmapstr.h>
/*
 *  Server XIE Includes
 */
#include <error.h>
#include <macro.h>
#include <photomap.h>
#include <element.h>
#include <texstr.h>

DrawablePtr ValDrawable();

/*
 *  routines referenced by other DDXIE modules
 */
int	miAnalyzeIDraw();

/*
 *  routines used internal to this module
 */
static int CreateIDraw();
static int InitializeIDraw();
static int ActivateIDraw();
static int FlushIDraw();
static int ResetIDraw();
static int DestroyIDraw();

static int ActivateIDrawP();

/*
 * DDXIE ImportDrawable && ImportDrawablePlane entry points
 */
static ddElemVecRec miDrawVec = {
  CreateIDraw,
  InitializeIDraw,
  ActivateIDraw,
  FlushIDraw,
  ResetIDraw,
  DestroyIDraw
  };

static ddElemVecRec miDrawPVec = {
  CreateIDraw,
  InitializeIDraw,
  ActivateIDrawP,
  FlushIDraw,
  ResetIDraw,
  DestroyIDraw
  };

/*------------------------------------------------------------------------
------------------- see if we can handle this element --------------------
------------------------------------------------------------------------*/
int miAnalyzeIDraw(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  /* for now just stash our entry point vector in the peDef */
  ped->ddVec = miDrawVec;
  return(TRUE);
}                               /* end miAnalyzeIDraw */

int miAnalyzeIDrawP(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  /* for now just stash our entry point vector in the peDef */
  ped->ddVec = miDrawPVec;
  return(TRUE);
}                               /* end miAnalyzeIDrawP */


/*------------------------------------------------------------------------
---------------------------- create peTex . . . --------------------------
------------------------------------------------------------------------*/
static int CreateIDraw(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  /* attach an execution context to the photo element definition */
  return(MakePETex(flo, ped, 0, FALSE, FALSE));
}                               /* end CreateIDraw */


/*------------------------------------------------------------------------
---------------------------- initialize peTex . . . ----------------------
------------------------------------------------------------------------*/
static int InitializeIDraw(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{

  /* note: ImportResource elements don't use their receptors */
  return(InitEmitter(flo,ped,0,-1));
}                               /* end InitializeIDraw */


/*------------------------------------------------------------------------
----------------------------- crank some data ----------------------------
------------------------------------------------------------------------*/
static int ActivateIDraw(flo,ped,pet)
     floDefPtr flo;
     peDefPtr  ped;
     peTexPtr  pet;
{
  xieFloImportDrawable	*raw = (xieFloImportDrawable *) ped->elemRaw;
  bandPtr		bnd = &pet->emitter[0];
  BytePixel		*dstLine;
  DrawablePtr		pDraw;
  CARD32		pixtype = ZPixmap;

  if (!(pDraw = ValDrawable(flo,ped,raw->drawable)))
    return FALSE;

  if (!(dstLine = GetCurrentDst(BytePixel,flo,pet,bnd)))
    return FALSE;

  (*pDraw->pScreen->GetImage)(
			pDraw,			  /* drawable	 */
			raw->srcX,		  /* drawable-x	 */
			raw->srcY+bnd->minLocal,  /* drawable-y	 */
			bnd->format->width,	  /* width	 */
			bnd->strip->length,	  /* height	 */
			pixtype,		  /* data format */
			~0,			  /* plane mask	 */
			(char *)dstLine		  /* data buffer */
			);

  PutData(flo,pet,bnd,bnd->maxLocal);

  return TRUE;
}                               /* end ActivateIDraw */

static int ActivateIDrawP(flo,ped,pet)
     floDefPtr flo;
     peDefPtr  ped;
     peTexPtr  pet;
{
  xieFloImportDrawablePlane *raw = (xieFloImportDrawablePlane *) ped->elemRaw;
  bandPtr	bnd = &pet->emitter[0];
  BytePixel	*dstLine;
  DrawablePtr	pDraw;
  CARD32	pixtype;

  if (!(pDraw = ValDrawable(flo,ped,raw->drawable)))
    return FALSE;

  if (!(dstLine = GetCurrentDst(BytePixel,flo,pet,bnd)))
    return FALSE;

  /* ??? Is this the correct thing when depth > 1 ??? */
  /* pixtype = (pDraw->type == DRAWABLE_PIXMAP) ? XYPixmap : XYBitmap; */
  pixtype = XYPixmap;

  (*pDraw->pScreen->GetImage)(
			pDraw,			  /* drawable	 */
			raw->srcX,		  /* drawable-x	 */
			raw->srcY+bnd->minLocal,  /* drawable-y	 */
			bnd->format->width,	  /* width	 */
			bnd->strip->length,	  /* height	 */
			pixtype,		  /* data format */
			raw->bitPlane,		  /* plane mask	 */
			(char *)dstLine		  /* data buffer */
			);

  PutData(flo,pet,bnd,bnd->maxLocal);

  return TRUE;
}                               /* end ActivateIDrawP */

/*------------------------------------------------------------------------
--------------------------- get rid of left overs ------------------------
------------------------------------------------------------------------*/
static int FlushIDraw(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  /* since Activate was suppose to do the whole image, there's nothing to do */
  return(TRUE);
}                               /* end FlushIDraw */


/*------------------------------------------------------------------------
------------------------ get rid of run-time stuff -----------------------
------------------------------------------------------------------------*/
static int ResetIDraw(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  ResetEmitter(ped);
  
  return(TRUE);
}                               /* end ResetIDraw */


/*------------------------------------------------------------------------
-------------------------- get rid of this element -----------------------
------------------------------------------------------------------------*/
static int DestroyIDraw(flo,ped)
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

  return(TRUE);
}                               /* end DestroyIDraw */

/*------------------------------------------------------------------------
--- similar to GetGCAndDrawableAndValidate (from extension.h)          ---
--- made callable because the macro version returned standard X errors ---
------------------------------------------------------------------------*/
DrawablePtr ValDrawable(flo,ped,draw_id)
     floDefPtr    flo;
     peDefPtr     ped;
     Drawable	  draw_id;
{
  register ClientPtr	client = flo->client;
  register DrawablePtr	draw;
  
  if (client->lastDrawableID != draw_id) {
    draw = (DrawablePtr)LookupIDByClass(draw_id, RC_DRAWABLE);
    if (draw->type  == UNDRAWABLE_WINDOW)
	MatchError(flo,ped, return(NULL));
      
    client->lastDrawable = draw;
    client->lastDrawableID = draw_id;
  } else {
    draw = client->lastDrawable;
  }
  if(!draw) {
    client->errorValue = draw_id;
    DrawableError(flo,ped,draw_id, return(NULL));
  }

  return draw;
}                               /* end ValDrawable */

/* end module midraw.c */

