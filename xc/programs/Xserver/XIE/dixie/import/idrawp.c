/* $XConsortium$ */
/**** module idrawp.c ****/
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
  
	idrawp.c -- DIXIE routines for managing the ImportDrawablePlane element
  
	Dean Verheiden -- AGE Logic, Inc. June 1993
  
*****************************************************************************/

#define _XIEC_IDRAWP

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
peDefPtr	MakeIDrawP();

/*
 *  routines internal to this module
 */
static Bool	PrepIDrawP();

/*
 * dixie entry points
 */
static diElemVecRec iDrawPVec = {
    PrepIDrawP		/* prepare for analysis and execution	*/
    };


/*------------------------------------------------------------------------
----------------- routine: make an ImportDrawablePlane element ----------------
------------------------------------------------------------------------*/
peDefPtr MakeIDrawP(flo,tag,pe)
     floDefPtr      flo;
     xieTypPhototag tag;
     xieFlo        *pe;
{
  peDefPtr ped;
  inFloPtr inflo;
  ELEMENT(xieFloImportDrawablePlane);
  ELEMENT_SIZE_MATCH(xieFloImportDrawablePlane);
  
  if(!(ped = MakePEDef(1, (CARD32)stuff->elemLength<<2, sizeof(iDrawDefRec)))) 
    FloAllocError(flo,tag,xieElemImportDrawablePlane, return(NULL));
  
  ped->diVec	    = &iDrawPVec;
  ped->phototag     = tag;
  ped->flags.import = TRUE;
  raw = (xieFloImportDrawablePlane *)ped->elemRaw;
  /*
   * copy the client element parameters (swap if necessary)
   */
  if( flo->client->swapped ) {
    raw->elemType   = stuff->elemType;
    raw->elemLength = stuff->elemLength;
    raw->notify     = stuff->notify; 
    cpswapl(stuff->drawable, raw->drawable);
    cpswaps(stuff->srcX, raw->srcX);
    cpswaps(stuff->srcY, raw->srcY);
    cpswaps(stuff->width, raw->width);
    cpswaps(stuff->height, raw->height);
    cpswapl(stuff->fill, raw->fill);
    cpswapl(stuff->bitPlane, raw->bitPlane);
  }
  else
    bcopy((char *)stuff, (char *)raw, sizeof(xieFloImportDrawablePlane));
  
  return(ped);
}                               /* end MakeIDrawP */


/*------------------------------------------------------------------------
---------------- routine: prepare for analysis and execution -------------
------------------------------------------------------------------------*/
static Bool PrepIDrawP(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  iDrawDefPtr pvt 	    = (iDrawDefPtr) ped->elemPvt;
  inFloPtr    inf 	    = &ped->inFloLst[IMPORT];
  outFloPtr   dst 	    = &ped->outFlo; 
  xieFloImportDrawablePlane *raw = (xieFloImportDrawablePlane *) ped->elemRaw;
  register DrawablePtr pd; 
  CARD32 format, padmask;

  if (!(pd = pvt->pDraw = 
		(DrawablePtr)LookupIDByClass(raw->drawable, RC_DRAWABLE)))
    DrawableError(flo,ped,raw->drawable,return(FALSE));

  /* If window, make sure we are within the screen */
  /* Check one at a time so offending value can be returned */
  if ( pd->type == DRAWABLE_WINDOW ) {
	if (!((WindowPtr)pd)->realized) {
    		DrawableError(flo,ped,raw->drawable,return(FALSE));
	} else if (raw->srcX < 0) {
    		ValueError(flo,ped,raw->srcX,return(FALSE));
	} else if (raw->srcY < 0) {
    		ValueError(flo,ped,raw->srcY,return(FALSE));
	} else if (raw->srcX + raw->width  > pd->pScreen->width) {
    		ValueError(flo,ped,raw->width,return(FALSE));
	} else if (raw->srcY + raw->height > pd->pScreen->height) {
    		ValueError(flo,ped,raw->height,return(FALSE));
	}
  } else if (pd->type != DRAWABLE_PIXMAP) 
    	DrawableError(flo,ped,raw->drawable,return(FALSE));

  if ( !raw->bitPlane || raw->bitPlane & (raw->bitPlane - 1) ||
	raw->bitPlane >= (1<<pd->depth))
	ValueError(flo,ped,raw->bitPlane,return(FALSE));

  inf->bands = 1;
  inf->format[0].band   = 0;
  inf->format[0].interleaved = FALSE;
  inf->format[0].depth	= pd->depth;
  inf->format[0].width  = pd->width;
  inf->format[0].height = pd->height;
  inf->format[0].levels = 1<<pd->depth;
  inf->format[0].params = NULL;

  for( format = 0; format < screenInfo.numPixmapFormats
	&& pd->depth != screenInfo.formats[format].depth; format++ );

  /* XXX oops, if (format >= screenInfo.numPixmapFormats) XXX */

  /* 
   * setting up the inflo format seems bizarre, but dagonizer may require it.
   */
  inf->format[0].stride = screenInfo.formats[format].bitsPerPixel;
  padmask = screenInfo.formats[format].scanlinePad - 1;
  inf->format[0].pitch  = 
		(inf->format[0].stride * pd->width + padmask) & ~padmask;

  /*
   * determine output attributes from input parameters
   */
  dst->bands = inf->bands;
  dst->format[0] = inf->format[0];
  dst->format[0].width = raw->width;
  dst->format[0].height = raw->height;
  dst->format[0].levels = 2;

  if (UpdateFormatfromLevels(ped) == FALSE)
	MatchError(flo, ped, return(FALSE));

  /*
   * still need to deal with following two arguments:
   *    CARD32	fill B32;
   *    BOOL	notify;
   */

  return(TRUE);
}                               /* end PrepIDrawP */
/* end module idrawp.c */
