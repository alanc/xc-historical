/* $XConsortium$ */
/**** module medrawp.c ****/
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
  
	medrawp.c -- DDXIE prototype export drawable plane element
  
	Robert NC Shelley && Larry Hare -- AGE Logic, Inc. June, 1993
  
*****************************************************************************/

#define _XIEC_MEDRAWP
#define _XIEC_EDRAWP

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
#include <gcstruct.h>
/*
 *  Server XIE Includes
 */
#include <error.h>
#include <macro.h>
#include <element.h>
#include <texstr.h>

extern Bool	DrawableAndGC();

/*
 *  routines referenced by other DDXIE modules
 */
int	miAnalyzeEDrawP();

/*
 *  routines used internal to this module
 */
static int CreateEDrawP();
static int InitializeEDrawP();
static int ActivateEDrawP(), ActivateEDrawPTrans();
static int FlushEDrawP();
static int ResetEDrawP();
static int DestroyEDrawP();

/*
 * DDXIE ExportDrawable entry points
 */
static ddElemVecRec EDrawPVec = {
  CreateEDrawP,
  InitializeEDrawP,
  ActivateEDrawP,
  FlushEDrawP,
  ResetEDrawP,
  DestroyEDrawP
  };

/*------------------------------------------------------------------------
------------------- see if we can handle this element --------------------
------------------------------------------------------------------------*/
int miAnalyzeEDrawP(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  /* for now just stash our entry point vector in the peDef */
  ped->ddVec = EDrawPVec;

  return(TRUE);
}                               /* end miAnalyzeEDrawP */

/*------------------------------------------------------------------------
---------------------------- create peTex . . . --------------------------
------------------------------------------------------------------------*/
static int CreateEDrawP(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  /* attach an execution context to the photo element definition */
  return( MakePETex(flo,ped,0,FALSE,FALSE) );
}                               /* end CreateEDrawP */

/*------------------------------------------------------------------------
---------------------------- initialize peTex . . . ----------------------
------------------------------------------------------------------------*/
static int InitializeEDrawP(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  xieFloExportDrawablePlane *raw = (xieFloExportDrawablePlane *) ped->elemRaw;
  eDrawPDefPtr	pvt = (eDrawPDefPtr) ped->elemPvt;

  if (!DrawableAndGC(flo,ped,raw->drawable,raw->gc,&pvt->pDraw,&pvt->pGC))
    return FALSE;

  if (pvt->pGC->fillStyle == FillStippled) {

    	ped->ddVec.activate = ActivateEDrawPTrans;

  } else { /* normal case: FillSolid || FillTiled || FillOpaqueStippled */

    	ped->ddVec.activate = ActivateEDrawP;
  }
  return( InitReceptors(flo,ped,0,1) );
}                               /* end InitializeEDrawP */


/*------------------------------------------------------------------------
----------------------------- crank some data ----------------------------
------------------------------------------------------------------------*/
static int ActivateEDrawP(flo,ped,pet)
     floDefPtr flo;
     peDefPtr  ped;
     peTexPtr  pet;
{
  xieFloExportDrawablePlane *raw = (xieFloExportDrawablePlane *) ped->elemRaw;
  eDrawPDefPtr	pvt = (eDrawPDefPtr) ped->elemPvt;
  bandPtr	bnd = &pet->receptor[SRCtag].band[0];
  DrawablePtr	draw;
  BytePixel	*src;
  CARD32	pixtype;
  
  src = GetSrc(BytePixel,flo,pet,bnd,bnd->minGlobal,FALSE);
  if(src) {
    if (!DrawableAndGC(flo,ped,raw->drawable,raw->gc,&pvt->pDraw,&pvt->pGC))
      return FALSE;
    draw = pvt->pDraw;
    pixtype = (draw->type == DRAWABLE_PIXMAP) ? XYPixmap : XYBitmap;
    do    
      (*pvt->pGC->ops->PutImage)(draw,			  /* drawable	 */
				 pvt->pGC,		  /* gc		 */
				 draw->depth,		  /* depth	 */
				 raw->dstX,		  /* drawable-x	 */
				 raw->dstY+bnd->minLocal, /* drawable-y	 */
				 bnd->format->width,	  /* width	 */
				 bnd->strip->length,	  /* height	 */
				 bnd->strip->bitOff,	  /* padding? 	 */
				 pixtype,		  /* data format */
				 src			  /* data buffer */
				 );
    while(src = GetSrc(BytePixel,flo,pet,bnd,bnd->maxLocal,FALSE));
    /* make sure the scheduler knows how much src we used */
    FreeData(BytePixel,flo,pet,bnd,bnd->current);
  }

  return(TRUE);
}                               /* end ActivateEDrawP */

static int ActivateEDrawPTrans(flo,ped,pet)
     floDefPtr flo;
     peDefPtr  ped;
     peTexPtr  pet;
{
  xieFloExportDrawablePlane *raw = (xieFloExportDrawablePlane *) ped->elemRaw;
  eDrawPDefPtr	pvt = (eDrawPDefPtr) ped->elemPvt;
  bandPtr	bnd = &pet->receptor[SRCtag].band[0];
  DrawablePtr	draw;
  GCPtr		gc, scratch;
  PixmapPtr	bitmap;
  BytePixel	*src;
  int		oldstyle, newstyle = FillSolid;
  XID		gcvals[3];
  
  src = GetSrc(BytePixel,flo,pet,bnd,bnd->minGlobal,FALSE);
  if(src) {
    if (!DrawableAndGC(flo,ped,raw->drawable,raw->gc,&pvt->pDraw,&pvt->pGC))
      return FALSE;

    draw = pvt->pDraw;
    gc = pvt->pGC;

    /*
    ** We use PushPixels with a solid fill to move the one bits onto the
    ** screen. Alternatives include:
    **	a) treat the bitmap as a stipple, set it up in a GC, and do a
    **     simple FillRect with FillStippled.  This has all the same
    **	   problems, plus some more.  Notably its more difficult to
    **	   use ChangeGC to transiently save/restore stipples, and to
    **	   compute the stipple offsets.
    **  b) another alternative is to change the incoming bitonal 
    **	   image to run length and do a FillSpans.  This also requires
    **	   either a scratch GC or transiently using the incoming GC.
    **	   It saves the extra bitmap creation and PutImage, but might
    ** 	   become grotesque when a dithered image comes rolling thru.
    */

    /*
    ** Core X does not seem to provide an official interface to create
    ** a pixmap header, or even to replace the data pointer.  We cant
    ** simply bcopy our data to it either, it might be on a separate
    ** cpu/memory system, or even upside down.  So we just use PutImage
    ** to prepare our data. Sigh.  On most cpu's, just explicitly
    ** create a pixmap header yourself and call PushPixels directly.
    ** Another optimization would be to use more lines at once. One 
    ** model for all this is miglblt.c
    **
    ** Since we expect people may want to redo this routine, we have
    ** left the pixmap and GC games local to this function rather than
    ** spreading stuff out into private structures and initialize
    ** and destroy routines.  For instance, the bitmap and scratchGC
    ** could be allocated once for the entire flow.
    **
    ** For now we do 64 lines of bits at a time.  We might come up
    ** with a better estimate based on actual scanline length. We 
    ** need a function to call though to make this unkludgey. Using
    ** NLINES as 64 may be bad if we get a very wide image.
    **
    ** The current performance seems very bad, even doing a large 
    ** number of lines at a time.  Haven't had time to investigate this.
    */

#define NLINES 64

    if (!(scratch =  GetScratchGC(1, draw->pScreen)))
	AllocError(flo,ped,return(FALSE));


    if (!(bitmap = (*draw->pScreen->CreatePixmap) (draw->pScreen,
			bnd->format->width, NLINES, 1))) {
	FreeScratchGC(scratch);
	AllocError(flo,ped,return(FALSE));
    }
    gcvals[0] = 1;
    gcvals[1] = 0;
    ChangeGC(scratch, GCForeground|GCBackground, gcvals);

    oldstyle = gc->fillStyle; 
    ChangeGC(gc, GCFillStyle, (XID *)&newstyle);

    do {
	int iy, ny;
	for (iy = 0 ; iy < bnd->strip->length; iy += ny) {
	    ny = bnd->strip->length - iy;
	    if (ny > NLINES)
		ny = NLINES;
	    if ((scratch->serialNumber) != (bitmap->drawable.serialNumber))
		ValidateGC(bitmap, scratch);
	    (*scratch->ops->PutImage) (
		bitmap,				/* drawable bitmap */
		scratch,			/* gc		*/
		1,				/* depth	*/
		0,				/* drawable-x	*/
		0,				/* drawable-y	*/
		bnd->format->width,		/* width	*/
		ny,				/* height	*/
		bnd->strip->bitOff,		/* padding? 	*/
		XYPixmap,			/* data format	*/
		src				/* data buffer	*/
		);
	    if ((gc->serialNumber) != (draw->serialNumber))
		ValidateGC(draw, gc);
	    (*gc->ops->PushPixels) (
		gc,					/* gc		*/
		bitmap,					/* bitmap	*/
		draw,					/* drawable	*/
		bnd->format->width,			/* width	*/
		ny,					/* height	*/
		raw->dstX+(gc->miTranslate?draw->x:0),	/* dst X/Y */
		raw->dstY+(gc->miTranslate?draw->y:0)+bnd->current+iy
		);
	    src += bnd->pitch * ny; /* gack */
	}
    } while(src = GetSrc(BytePixel,flo,pet,bnd,bnd->maxLocal,FALSE)) ;

    /* make sure the scheduler knows how much src we used */
    FreeData(BytePixel,flo,pet,bnd,bnd->current);

    ChangeGC(gc, GCFillStyle, (XID *)&oldstyle);
    ValidateGC(draw, gc);

    (*draw->pScreen->DestroyPixmap) (bitmap);
    FreeScratchGC(scratch);
  }

  return(TRUE);
}                               /* end ActivateEDrawPTrans */

/*------------------------------------------------------------------------
--------------------------- get rid of left overs ------------------------
------------------------------------------------------------------------*/
static int FlushEDrawP(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  /* Activate was suppose to do the whole image -- there's nothing to do */
  return(TRUE);
}                               /* end FlushEDrawP */


/*------------------------------------------------------------------------
------------------------ get rid of run-time stuff -----------------------
------------------------------------------------------------------------*/
static int ResetEDrawP(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  ResetReceptors(ped);
  
  return(TRUE);
}                               /* end ResetEDrawP */


/*------------------------------------------------------------------------
-------------------------- get rid of this element -----------------------
------------------------------------------------------------------------*/
static int DestroyEDrawP(flo,ped)
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
}                               /* end DestroyEDrawP */

/* end module medrawp.c */
