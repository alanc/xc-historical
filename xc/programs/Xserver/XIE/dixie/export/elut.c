/* $XConsortium: elut.c,v 1.1 93/10/26 10:02:19 rws Exp $ */
/**** module elut.c ****/
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
  
	elut.c -- DIXIE routines for managing the ExportLUT element
  
	Larry Hare  -- AGE Logic, Inc. June 1993
  
*****************************************************************************/

#define _XIEC_ELUT

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
  /*
   *  Server XIE Includes
   */
#include <corex.h>
#include <error.h>
#include <macro.h>
#include <lut.h>
#include <element.h>

/*
 *  routines referenced by other modules.
 */
peDefPtr	MakeELUT();

/*
 *  routines internal to this module
 */
static Bool	PrepELUT();
static Bool	DebriefELUT();

/*
 * dixie entry points
 */
static diElemVecRec eLUTVec = {
    PrepELUT,
    DebriefELUT
    };


/*------------------------------------------------------------------------
----------------- routine: make an ExportLUT element ----------------
------------------------------------------------------------------------*/
peDefPtr MakeELUT(flo,tag,pe)
     floDefPtr      flo;
     xieTypPhototag tag;
     xieFlo        *pe;
{
  peDefPtr ped;
  inFloPtr inFlo;
  ELEMENT(xieFloExportLUT);
  ELEMENT_SIZE_MATCH(xieFloExportLUT);
  ELEMENT_NEEDS_1_INPUT(src);
  
  if(!(ped = MakePEDef(1,(CARD32)stuff->elemLength<<2,sizeof(eLUTDefRec)))) 
    FloAllocError(flo,tag,xieElemExportLUT, return(NULL));

  ped->diVec	    = &eLUTVec;
  ped->phototag     = tag;
  ped->flags.export = TRUE;
  raw = (xieFloExportLUT *)ped->elemRaw;
  /*
   * copy the standard client element parameters (swap if necessary)
   */
  if( flo->reqClient->swapped ) {
    raw->elemType   = stuff->elemType;
    raw->elemLength = stuff->elemLength;
    raw->merge	    = stuff->merge;
    cpswaps(stuff->src,    raw->src);
    cpswapl(stuff->lut,    raw->lut);
    cpswapl(stuff->start0, raw->start0);
    cpswapl(stuff->start1, raw->start1);
    cpswapl(stuff->start2, raw->start2);
  }
  else  
    memcpy((char *)raw, (char *)stuff, sizeof(xieFloExportLUT));
  /*
   * assign phototags to inFlos
   */
  inFlo = ped->inFloLst;
  inFlo[SRCtag].srcTag = raw->src;

  return(ped);
}                               /* end MakeELUT */


/*------------------------------------------------------------------------
---------------- routine: prepare for analysis and execution -------------
------------------------------------------------------------------------*/
static Bool PrepELUT(flo,ped)
     floDefPtr  flo;
     peDefPtr   ped;
{
  xieFloExportLUT *raw = (xieFloExportLUT *)ped->elemRaw;
  eLUTDefPtr   pvt = (eLUTDefPtr) ped->elemPvt;
  inFloPtr     inf = &ped->inFloLst[IMPORT];
  outFloPtr    src = &inf->srcDef->outFlo;
  outFloPtr    dst = &ped->outFlo;
  CARD32    *start = &(raw->start0);
  lutPtr	lut;
  CARD32 b;

  /* find the LUT resource and bind it to our flo */
  if(!(lut = pvt->lut = (lutPtr) LookupIDByType(raw->lut, RT_LUT)))
    LUTError(flo,ped,raw->lut, return(FALSE));

  ++pvt->lut->refCnt;
  if ( !raw->merge && (start[0] || start[1] || start[2]) )
	MatchError(flo, ped, return(FALSE));

  /* Validate and Propagate input attributes to our output */
  dst->bands = inf->bands = src->bands;
  for(b = 0; b < src->bands; ++b) {
    if (IsntLut(src->format[b].class))
	FloSourceError(flo,raw->src,raw->elemType, return(FALSE));
    if (raw->merge) {
	if ( (src->format[b].levels != lut->format[b].level) ||
	     (start[b] + src->format[b].height > lut->format[b].length) )
	    MatchError(flo, ped, return(FALSE));
    }
    dst->format[b] = inf->format[b] = src->format[b];
  }
  return(TRUE);
}                               /* end PrepELUT */


/*------------------------------------------------------------------------
---------------------- routine: post execution cleanup -------------------
------------------------------------------------------------------------*/
static Bool DebriefELUT(flo,ped,ok)
     floDefPtr  flo;
     peDefPtr   ped;
     Bool	ok;
{
  xieFloExportLUT *raw = (xieFloExportLUT *)ped->elemRaw;
  eLUTDefPtr      pvt = (eLUTDefPtr) ped->elemPvt;
  lutPtr          lut;
  CARD32          b;
  
  if(!(pvt && (lut = pvt->lut))) return(FALSE);

  if(ok && lut->refCnt > 1) {

    if (raw->merge) {
  	CARD32 *start = &(raw->start0);
	for(b = 0; b < lut->lutCnt; b++) {
	   int nbytes = LutPitch(lut->format[b].level);
	   memcpy(lut->strips[b].flink->data + start[b] * nbytes,
	   	 ped->outFlo.export[b].flink->data,
		 ped->outFlo.export[b].flink->length * nbytes);
	} /* transient strips freed below */
    } else { 
	/* free old LUT data */
	for(b = 0; b < lut->lutCnt; b++)
	    FreeStrips(&lut->strips[b]);

	/* stash our new attributes and data into the LUT */
	lut->lutCnt = ped->outFlo.bands;
	for(b = 0; b < lut->lutCnt; ++b) {
	    lut->format[b].bandOrder = ped->outFlo.format[b].width;/*ugly hack*/
	    lut->format[b].length = ped->outFlo.format[b].height;/* ugly hack */
	    lut->format[b].level  = ped->outFlo.format[b].levels;
	    DebriefStrips(&ped->outFlo.export[b],&lut->strips[b]);
	}
    }
  }
  /* free image data that's left over on our outFlo
   */
  for(b = 0; b < ped->outFlo.bands; b++)
    FreeStrips(&ped->outFlo.export[b]);
  
  /* unbind ourself from the LUT
   */
  if(lut->refCnt > 1)
    --lut->refCnt;
  else if(LookupIDByType(raw->lut, RT_LUT))
    FreeResourceByType(lut->ID, RT_LUT, RT_NONE);
  else
    DeleteLUT(lut, lut->ID);

  return(TRUE);
}                               /* end DebriefELUT */

/* end module elut.c */
