/* $XConsortium: edrawp.c,v 1.2 93/11/06 15:49:23 rws Exp $ */
/**** module edrawp.c ****/
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
     
     "Copyright 1993, 1994 by AGE Logic, Inc. and the Massachusetts
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
  
	edrawp.c -- DIXIE routines for managing the ExportDrawablePlane element
  
	Robert NC Shelley -- AGE Logic, Inc. April 1993
  
*****************************************************************************/

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
#include <pixmapstr.h>
#include <gcstruct.h>
  /*
   *  Server XIE Includes
   */
#include <corex.h>
#include <macro.h>
#include <element.h>
#include <error.h>


/*
 *  routines referenced by other modules.
 */
peDefPtr	MakeEDrawPlane();

/*
 *  routines internal to this module
 */
static Bool	PrepEDrawPlane();

/*
 * dixie entry points
 */
static diElemVecRec eDrawPlaneVec = {
    PrepEDrawPlane		/* prepare for analysis and execution	*/
    };


/*------------------------------------------------------------------------
--------------- routine: make an ExportDrawablePlane element -------------
------------------------------------------------------------------------*/
peDefPtr MakeEDrawPlane(flo,tag,pe)
     floDefPtr      flo;
     xieTypPhototag tag;
     xieFlo        *pe;
{
  peDefPtr ped;
  inFloPtr inFlo;
  ELEMENT(xieFloExportDrawablePlane);
  ELEMENT_SIZE_MATCH(xieFloExportDrawablePlane);
  ELEMENT_NEEDS_1_INPUT(src);
  
  if(!(ped = MakePEDef(1, (CARD32)stuff->elemLength<<2,
        	       sizeof(eDrawPDefRec)))) 
    FloAllocError(flo,tag,xieElemExportDrawablePlane, return(NULL));

  ped->diVec	    = &eDrawPlaneVec;
  ped->phototag     = tag;
  ped->flags.export = TRUE;
  raw = (xieFloExportDrawablePlane *)ped->elemRaw;
  /*
   * copy the client element parameters (swap if necessary)
   */
  if( flo->reqClient->swapped ) {
    raw->elemType   = stuff->elemType;
    raw->elemLength = stuff->elemLength;
    cpswaps(stuff->src, raw->src);
    cpswaps(stuff->dstX, raw->dstX);
    cpswaps(stuff->dstY, raw->dstY);
    cpswapl(stuff->drawable, raw->drawable);
    cpswapl(stuff->gc, raw->gc);
  }
  else
    memcpy((char *)raw, (char *)stuff, sizeof(xieFloExportDrawablePlane));
  /*
   * assign phototags to inFlos
   */
  inFlo = ped->inFloLst;
  inFlo[SRCtag].srcTag = raw->src;

  return(ped);
}                               /* end MakeEDrawPlane */


/*------------------------------------------------------------------------
---------------- routine: prepare for analysis and execution -------------
------------------------------------------------------------------------*/
static Bool PrepEDrawPlane(flo,ped)
     floDefPtr  flo;
     peDefPtr   ped;
{
  xieFloExportDrawablePlane *raw = (xieFloExportDrawablePlane *)ped->elemRaw;
  eDrawPDefPtr pvt = (eDrawPDefPtr) ped->elemPvt;
  inFloPtr    inf = &ped->inFloLst[SRCtag];
  outFloPtr   src = &inf->srcDef->outFlo;
  outFloPtr   dst = &ped->outFlo;
  /*
   * check out drawable and gc
   */
  if(!DrawableAndGC(flo,ped,raw->drawable,raw->gc,&(pvt->pDraw),&(pvt->pGC)))
    return(FALSE);
  /*
   * check for: constrained, single-band, bitonal image
   */
  if( (src->format[0].class != BIT_PIXEL) || (src->bands != 1))
    MatchError(flo,ped, return(FALSE));
  /*
   * grab a copy of the input attributes and propagate them to our output
   */
  dst->bands     = inf->bands     = src->bands;
  dst->format[0] = inf->format[0] = src->format[0];
  
  return(TRUE);
}                               /* end PrepEDrawplane */

/* end module edrawp.c */
