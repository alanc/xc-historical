/* $XConsortium: eclut.c,v 1.2 93/11/06 15:48:53 rws Exp $ */
/**** module eclut.c ****/
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
  
	eclut.c -- DIXIE routines for managing the ExportClientLUT element
  
	Dean Verheiden -- AGE Logic, Inc. July 1993
  
*****************************************************************************/

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
  /*
   *  Server XIE Includes
   */
#include <corex.h>
#include <error.h>
#include <macro.h>
#include <element.h>
#include <lut.h>


/*
 *  routines referenced by other modules
 */
peDefPtr	MakeECLUT();

/*
 *  routines internal to this module
 */
static Bool	PrepECLUT();

/*
 * dixie element entry points
 */
static diElemVecRec eCLUTVec = {
  PrepECLUT			/* prepare for analysis and execution	*/
  };


/*------------------------------------------------------------------------
--------------- routine: make an export client lut element -------------
------------------------------------------------------------------------*/
peDefPtr MakeECLUT(flo,tag,pe)
     floDefPtr      flo;
     xieTypPhototag tag;
     xieFlo        *pe;
{
  peDefPtr ped;
  ELEMENT(xieFloExportClientLUT);
  ELEMENT_SIZE_MATCH(xieFloExportClientLUT);
  ELEMENT_NEEDS_1_INPUT(src);
  
  if(!(ped = MakePEDef(1, (CARD32)stuff->elemLength<<2, 0)))
    FloAllocError(flo,tag,xieElemExportClientLUT, return(NULL)) ;

  ped->diVec	     = &eCLUTVec;
  ped->phototag      = tag;
  ped->flags.getData = TRUE;
  ped->flags.export  = TRUE;
  raw = (xieFloExportClientLUT *)ped->elemRaw;
  /*
   * copy the standard client element parameters (swap if necessary)
   */
  if( flo->reqClient->swapped ) {
    raw->elemType   = stuff->elemType;
    raw->elemLength = stuff->elemLength;
    cpswaps(stuff->src,  raw->src);
    raw->notify     = stuff->notify;
    raw->bandOrder  = stuff->bandOrder;
    cpswapl(stuff->start0,  raw->start0);
    cpswapl(stuff->start1,  raw->start1);
    cpswapl(stuff->start2,  raw->start2);
    cpswapl(stuff->length0, raw->length0);
    cpswapl(stuff->length1, raw->length1);
    cpswapl(stuff->length2, raw->length2);
  }
  else
    memcpy((char *)raw, (char *)stuff, sizeof(xieFloExportClientLUT));

  /* assign phototag to inFlo
   */
  ped->inFloLst[SRCtag].srcTag = raw->src;

  return(ped);
}                               /* end MakeECLUT */

/*------------------------------------------------------------------------
---------------- routine: prepare for analysis and execution -------------
------------------------------------------------------------------------*/
static Bool PrepECLUT(flo,ped)
     floDefPtr  flo;
     peDefPtr   ped;
{
  xieFloExportClientLUT *raw = (xieFloExportClientLUT *)ped->elemRaw;
  inFloPtr inf   = &ped->inFloLst[SRCtag];
  outFloPtr src  = &inf->srcDef->outFlo;
  outFloPtr dst  = &ped->outFlo;
  CARD32 *start  = &(raw->start0);
  CARD32 *length = &(raw->length0);
  int b;
  
  if(raw->notify != xieValDisable   &&		   /* check notify */
     raw->notify != xieValFirstData &&
     raw->notify != xieValNewData)
    ValueError(flo,ped,raw->notify, return(FALSE));

  if(raw->bandOrder  != xieValLSFirst &&	    /* check band-order     */
     raw->bandOrder  != xieValMSFirst)
    ValueError(flo,ped,raw->bandOrder, return(FALSE));
		
  /* Validate and Propagate input attributes to our output */
  dst->bands = inf->bands = src->bands;
  for(b = 0; b < src->bands; ++b) {
    if (IsntLut(src->format[b].class))
       FloSourceError(flo,raw->src,raw->elemType, return(FALSE));
    if ( start[b] + length[b] > src->format[b].height )
       MatchError(flo,ped, return(FALSE));

    dst->format[b] = inf->format[b] = src->format[b];
    ped->swapUnits[b] = LutPitch(dst->format[b].levels);
  }
  return(TRUE);
}                               /* end PrepECLUT */


/* end module eclut.c */
