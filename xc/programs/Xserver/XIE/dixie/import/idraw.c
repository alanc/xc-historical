/* $XConsortium: idraw.c,v 1.1 93/10/26 09:59:54 rws Exp $ */
/**** module idraw.c ****/
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
  
	idraw.c -- DIXIE routines for managing the ImportDrawable element
  
	Dean Verheiden -- AGE Logic, Inc. June 1993
  
*****************************************************************************/

#define _XIEC_IDRAW

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
#include <extnsionst.h>
#include <dixstruct.h>
#include <pixmapstr.h>
#include <scrnintstr.h>
#include <windowstr.h>
#include <window.h>
  /*
   *  Server XIE Includes
   */
#include <corex.h>
#include <error.h>
#include <macro.h>
#include <element.h>


/*
 *  routines referenced by other modules.
 */
peDefPtr	MakeIDraw();

/*
 *  routines internal to this module
 */
static Bool	PrepIDraw();

/*
 * dixie entry points
 */
static diElemVecRec iDrawVec = {
    PrepIDraw		/* prepare for analysis and execution	*/
    };


/*------------------------------------------------------------------------
----------------- routine: make an ImportDrawable element ----------------
------------------------------------------------------------------------*/
peDefPtr MakeIDraw(flo,tag,pe)
     floDefPtr      flo;
     xieTypPhototag tag;
     xieFlo        *pe;
{
  peDefPtr ped;
  ELEMENT(xieFloImportDrawable);
  ELEMENT_SIZE_MATCH(xieFloImportDrawable);
  
  if(!(ped = MakePEDef(1, (CARD32)stuff->elemLength<<2, sizeof(iDrawDefRec)))) 
    FloAllocError(flo,tag,xieElemImportDrawable, return(NULL));
  
  ped->diVec	    = &iDrawVec;
  ped->phototag     = tag;
  ped->flags.import = TRUE;
  raw = (xieFloImportDrawable *)ped->elemRaw;
  /*
   * copy the client element parameters (swap if necessary)
   */
  if( flo->reqClient->swapped ) {
    raw->elemType   = stuff->elemType;
    raw->elemLength = stuff->elemLength;
    raw->notify     = stuff->notify; 
    cpswapl(stuff->drawable, raw->drawable);
    cpswaps(stuff->srcX, raw->srcX);
    cpswaps(stuff->srcY, raw->srcY);
    cpswaps(stuff->width, raw->width);
    cpswaps(stuff->height, raw->height);
    cpswapl(stuff->fill, raw->fill);
  }
  else
    memcpy((char *)raw, (char *)stuff, sizeof(xieFloImportDrawable));
  
  return(ped);
}                               /* end MakeIDraw */


/*------------------------------------------------------------------------
---------------- routine: prepare for analysis and execution -------------
------------------------------------------------------------------------*/
static Bool PrepIDraw(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  xieFloImportDrawable *raw = (xieFloImportDrawable *) ped->elemRaw;
  iDrawDefPtr pvt 	    = (iDrawDefPtr) ped->elemPvt;
  inFloPtr    inf 	    = &ped->inFloLst[IMPORT];
  outFloPtr   dst 	    = &ped->outFlo;
  formatPtr   fmt           = &inf->format[0]; 
  DrawablePtr pd; 
  CARD32   f, padmask;

  if(!(pd = pvt->pDraw = ((DrawablePtr)
			  LookupIDByClass(raw->drawable, RC_DRAWABLE))))
    DrawableError(flo,ped,raw->drawable, return(FALSE));

  if(!(pd->type == DRAWABLE_WINDOW && ((WindowPtr)pd)->realized ||
       pd->type == DRAWABLE_PIXMAP))
    DrawableError(flo,ped,raw->drawable, return(FALSE));

  if(raw->srcX < 0) {
    ValueError(flo,ped,raw->srcX, return(FALSE));
  } else if (raw->srcY < 0) {
    ValueError(flo,ped,raw->srcY, return(FALSE));
  } else if (raw->srcX + raw->width  > pd->width) {
    ValueError(flo,ped,raw->width, return(FALSE));
  } else if (raw->srcY + raw->height > pd->height) {
    ValueError(flo,ped,raw->height, return(FALSE));
  }
  /* find the screen format that matches this drawable and fill in the format
   */
  for(f = 0; f < screenInfo.numPixmapFormats
      && pd->depth != screenInfo.formats[f].depth; ++f);
  if(f == screenInfo.numPixmapFormats)
    DrawableError(flo,ped,raw->drawable,return(FALSE));
  padmask = screenInfo.formats[f].scanlinePad - 1;
  fmt->interleaved = FALSE;
  fmt->band   = 0;
  fmt->depth  = pd->depth;
  fmt->width  = raw->width;
  fmt->height = raw->height;
  fmt->levels = 1<<pd->depth;
  fmt->stride = screenInfo.formats[f].bitsPerPixel;
  fmt->pitch  = fmt->stride * raw->width + padmask & ~padmask;
  /*
   * set output attributes from input format (stride and pitch may differ)
   */
  dst->bands     = inf->bands = 1;
  dst->format[0] = inf->format[0];
  if(!UpdateFormatfromLevels(ped))
    MatchError(flo,ped, return(FALSE));

  return(TRUE);
}                               /* end PrepIDraw */
/* end module idraw.c */
