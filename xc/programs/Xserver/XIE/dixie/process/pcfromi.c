/* $XConsortium$ */
/**** module pcfromi.c ****/
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
  
	pcfromi.c -- DIXIE routines for managing the ConvertFromIndex element
  
	Dean Verheiden -- AGE Logic, Inc. June 1993
  
*****************************************************************************/

#define _XIEC_PCFROMI
#define _XIEC_PCI

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
#include <scrnintstr.h>
#include <colormapst.h>
  /*
   *  Server XIE Includes
   */
#include <corex.h>
#include <error.h>
#include <macro.h>
#include <element.h>
#include <technq.h>


/* routines referenced by other modules.
 */
peDefPtr	MakeConvertFromIndex();

/* routines internal to this module
 */
static Bool	PrepConvertFromIndex();

/* dixie entry points
 */
static diElemVecRec pCfromIVec = {
    PrepConvertFromIndex
    };


/*------------------------------------------------------------------------
----------------- routine: make an ConvertFromIndex element ----------------
------------------------------------------------------------------------*/
peDefPtr MakeConvertFromIndex(flo,tag,pe)
     floDefPtr      flo;
     xieTypPhototag tag;
     xieFlo        *pe;
{
  peDefPtr ped;
  inFloPtr inFlo;
  ELEMENT(xieFloConvertFromIndex);
  ELEMENT_AT_LEAST_SIZE(xieFloConvertFromIndex);
  ELEMENT_NEEDS_1_INPUT(src);
  
  if(!(ped=MakePEDef(1,(CARD32)stuff->elemLength<<2,sizeof(pCfromIDefRec))))
    FloAllocError(flo,tag,xieElemConvertFromIndex, return(NULL));

  ped->diVec	     = &pCfromIVec;
  ped->phototag      = tag;
  ped->flags.process = TRUE;
  raw = (xieFloConvertFromIndex *)ped->elemRaw;
  /*
   * copy the standard client element parameters (swap if necessary)
   */
  if( flo->client->swapped ) {
    raw->elemType   = stuff->elemType;
    raw->elemLength = stuff->elemLength;
    cpswaps(stuff->src, raw->src);
    raw->class      = stuff->class;
    raw->precision  = stuff->precision;
    cpswapl(stuff->colormap, raw->colormap);
  }
  else  
    bcopy((char *)stuff, (char *)raw, sizeof(xieFloConvertFromIndex));
  /*
   * assign phototags to inFlos
   */
  inFlo = ped->inFloLst;
  inFlo[SRCtag].srcTag = raw->src;

  return(ped);
}                               /* end MakeConvertFromIndex */


/*------------------------------------------------------------------------
---------------- routine: prepare for analysis and execution -------------
------------------------------------------------------------------------*/
static Bool PrepConvertFromIndex(flo,ped)
     floDefPtr  flo;
     peDefPtr   ped;
{
  xieFloConvertFromIndex *raw = (xieFloConvertFromIndex *)ped->elemRaw;
  pCfromIDefPtr pvt = (pCfromIDefPtr) ped->elemPvt;
  inFloPtr      inf = &ped->inFloLst[SRCt1];
  outFloPtr     src = &inf->srcDef->outFlo;
  outFloPtr     dst = &ped->outFlo;
  CARD32        mapDepth, b;

  /* verify that args, colormap, and src image are compatible
   */
  dst->bands = (raw->class == xieValSingleBand) ? 1 : 3;
  if(raw->class != xieValSingleBand &&
     raw->class != xieValTripleBand ||
     raw->precision < 1 || raw->precision > 16)
    ValueError(flo,ped,raw->precision, return(FALSE));

  if(!(pvt->cmap = (ColormapPtr) LookupIDByType(raw->colormap, RT_COLORMAP)))
    ColormapError(flo,ped,raw->colormap, return(FALSE));
  SetDepthFromLevels(pvt->cmap->pVisual->ColormapEntries, mapDepth);

  if(!IsConstrained(src->format[0].class)
    || src->bands > 1 || src->format[0].depth != mapDepth)
    MatchError(flo,ped,return(FALSE));

  /* grab attributes from colormap, visual, ...
   */
  pvt->cells     = 1<<mapDepth;
  pvt->class     = pvt->cmap->class;
  pvt->visual    = pvt->cmap->pVisual;
  pvt->pixMsk[0] = pvt->visual->redMask;
  pvt->pixMsk[1] = pvt->visual->greenMask;
  pvt->pixMsk[2] = pvt->visual->blueMask;
  pvt->pixPos[0] = pvt->visual->offsetRed;
  pvt->pixPos[1] = pvt->visual->offsetGreen;
  pvt->pixPos[2] = pvt->visual->offsetBlue;
  pvt->precShift = 16 - raw->precision;

  /* generate output attributes from input attributes and precision arg
   */
  inf->bands = src->bands;
  for(b = 0; b < dst->bands; b++) {
    dst->format[b] = inf->format[0] = src->format[0];
    dst->format[b].band = b;
    dst->format[b].levels = (1<<raw->precision);
  }
  if(!UpdateFormatfromLevels(ped))
    MatchError(flo,ped,return(FALSE));

  return(TRUE);
}                               /* end PrepConvertFromIndex */

/* end module pcfromi.c */
