/* $XConsortium$ */
/**** module pctoi.c ****/
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
  
	pctoi.c -- DIXIE routines for managing the ConvertToIndex element
  
	Dean Verheiden -- AGE Logic, Inc. June 1993
  
*****************************************************************************/

#define _XIEC_PCTOI
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
#include <colorlst.h>
#include <element.h>
#include <technq.h>
#include <difloat.h>


/* routines referenced by other modules.
 */
peDefPtr	MakeConvertToIndex();
Bool 		CopyCtoIAllocAll();
Bool 		CopyCtoIAllocMatch();
Bool 		CopyCtoIAllocRequantize();
Bool 		PrepCtoIAllocAll();
Bool 		PrepCtoIAllocMatch();

/* routines internal to this module
 */
static Bool	PrepConvertToIndex();
static Bool	DebriefConvertToIndex();

/* dixie entry points
 */
static diElemVecRec pCtoIVec = {
    PrepConvertToIndex,
    DebriefConvertToIndex
    };


/*------------------------------------------------------------------------
----------------- routine: make an ExportPhotomap element ----------------
------------------------------------------------------------------------*/
peDefPtr MakeConvertToIndex(flo,tag,pe)
     floDefPtr      flo;
     xieTypPhototag tag;
     xieFlo        *pe;
{
  peDefPtr ped;
  inFloPtr inFlo;
  ELEMENT(xieFloConvertToIndex);
  ELEMENT_AT_LEAST_SIZE(xieFloConvertToIndex);
  ELEMENT_NEEDS_1_INPUT(src);
  
  if(!(ped = MakePEDef(1,(CARD32)stuff->elemLength<<2,sizeof(pCtoIDefRec)))) 
    FloAllocError(flo,tag,xieElemConvertToIndex, return(NULL));

  ped->diVec	     = &pCtoIVec;
  ped->phototag      = tag;
  ped->flags.process = TRUE;
  raw = (xieFloConvertToIndex *)ped->elemRaw;
  /*
   * copy the standard client element parameters (swap if necessary)
   */
  if( flo->client->swapped ) {
    raw->elemType   = stuff->elemType;
    raw->elemLength = stuff->elemLength;
    cpswaps(stuff->src, raw->src);
    raw->notify     = stuff->notify;
    cpswapl(stuff->colormap, raw->colormap);
    cpswapl(stuff->colorList, raw->colorList);
    cpswaps(stuff->colorAlloc, raw->colorAlloc);
    cpswaps(stuff->lenParams, raw->lenParams);
  }
  else  
    bcopy((char *)stuff, (char *)raw, sizeof(xieFloConvertToIndex));
  /*
   * assign phototags to inFlos
   */
  inFlo = ped->inFloLst;
  inFlo[SRCtag].srcTag = raw->src;

  /*
   * copy technique data (if any)
   */
  if(!(ped->techVec = FindTechnique(xieValColorAlloc,raw->colorAlloc)) ||
     !(ped->techVec->copyfnc(flo, ped, &stuff[1], &raw[1], raw->lenParams)))
    TechniqueError(flo,ped,raw->colorAlloc,raw->lenParams, return(ped));

  return(ped);
}                               /* end MakeConvertToIndex */

/*------------------------------------------------------------------------
-----------------------  copy routines for techniques  -------------------
------------------------------------------------------------------------*/

Bool CopyCtoIAllocAll(flo, ped, sparms, rparms, tsize) 
     floDefPtr  flo;
     peDefPtr   ped;
     xieTecColorAllocAll *sparms, *rparms;
     CARD16	tsize;
{
  TECHNIQUE_SIZE_MATCH(xieTecColorAllocAll,tsize);
  
  if( flo->client->swapped ) {
    cpswapl(sparms->fill, rparms->fill);
  } else
    rparms->fill = sparms->fill;
  
  return(TRUE);
}

Bool CopyCtoIAllocMatch(flo, ped, sparms, rparms, tsize) 
     floDefPtr  flo;
     peDefPtr   ped;
     xieTecColorAllocMatch *sparms, *rparms;
     CARD16	tsize;
{
  pConvertToIndexMatchDefPtr pvt;
  TECHNIQUE_SIZE_MATCH(xieTecColorAllocMatch,tsize);
  
  if (!(ped->techPvt=(void *)XieMalloc(sizeof(pTecConvertToIndexMatchDefRec))))
    AllocError(flo,ped, return(TRUE));
  
  pvt = (pConvertToIndexMatchDefPtr)ped->techPvt;
  
  if( flo->client->swapped ) {
    pvt->matchLimit = ConvertFromIEEE(lswapl(sparms->matchLimit));
    pvt->grayLimit  = ConvertFromIEEE(lswapl(sparms->grayLimit));
  } else {
    pvt->matchLimit = ConvertFromIEEE(sparms->matchLimit);
    pvt->grayLimit  = ConvertFromIEEE(sparms->grayLimit);
  }
  return(TRUE);
}

Bool CopyCtoIAllocRequantize(flo, ped, sparms, rparms, tsize) 
     floDefPtr  flo;
     peDefPtr   ped;
     xieTecColorAllocRequantize *sparms, *rparms;
     CARD16	tsize;
{
  TECHNIQUE_SIZE_MATCH(xieTecColorAllocRequantize,tsize);
  
  if( flo->client->swapped ) {
    cpswapl(sparms->maxCells, rparms->maxCells);
  } else
    rparms->maxCells = sparms->maxCells;
  
  return(TRUE);
}

/*------------------------------------------------------------------------
---------------- routine: prepare for analysis and execution -------------
------------------------------------------------------------------------*/
static Bool PrepConvertToIndex(flo,ped)
     floDefPtr  flo;
     peDefPtr   ped;
{
  xieFloConvertToIndex *raw = (xieFloConvertToIndex *)ped->elemRaw;
  pCtoIDefPtr pvt = (pCtoIDefPtr) ped->elemPvt;
  inFloPtr    inf = &ped->inFloLst[SRCt1];
  outFloPtr   src = &inf->srcDef->outFlo;
  outFloPtr   dst = &ped->outFlo;
  formatPtr   sf  = &src->format[0];
  formatPtr   df  = &dst->format[0];
  CARD32 b, f, right_padm1;

  /* must be constrained and inter-band dimensions must match */
  if(!IsConstrained(sf[0].class) ||
     src->bands == 3 && (!IsConstrained(sf[1].class)  ||
			 !IsConstrained(sf[2].class)  ||
			 sf[0].width  != sf[1].width  ||
			 sf[1].width  != sf[2].width  ||
			 sf[0].height != sf[1].height ||
			 sf[1].height != sf[2].height))
    MatchError(flo,ped, return(FALSE));

  /* find the ColorList and Colormap resources */
  if(raw->colorList) {
    if(!(pvt->list = LookupColorList(raw->colorList)))
      ColorListError(flo,ped,raw->colorList, return(FALSE));
    if(pvt->list->refCnt != 1)
      AccessError(flo,ped, return(FALSE));
    ++pvt->list->refCnt;
  } else {
    pvt->list = NULL;
  }
  if(!(pvt->cmap = (ColormapPtr) LookupIDByType(raw->colormap, RT_COLORMAP)))
    ColormapError(flo,ped,raw->colormap, return(FALSE));

  /* grab attributes from colormap, visual, ...
   */
  pvt->class   = pvt->cmap->class;
  pvt->visual  = pvt->cmap->pVisual;
  pvt->stride  = pvt->visual->bitsPerRGBValue;
  pvt->mask[0] = pvt->visual->redMask;
  pvt->mask[1] = pvt->visual->greenMask;
  pvt->mask[2] = pvt->visual->blueMask;
  pvt->shft[0] = pvt->visual->offsetRed;
  pvt->shft[1] = pvt->visual->offsetGreen;
  pvt->shft[2] = pvt->visual->offsetBlue;
  SetDepthFromLevels(pvt->visual->ColormapEntries, pvt->depth);
  pvt->cells   = 1<<pvt->depth;

  pvt->preFmt = pvt->doHist = FALSE;
  switch(pvt->class) {
  case DirectColor :
    pvt->doHist = TRUE;           	  /* used by Match technique */
  case TrueColor   :
  case StaticColor :
    for(b = 0; b < 3; ++b)
      pvt->levels[b] = pvt->mask[b] >> pvt->shft[b];

    /* see if we have a full set of masks while turning them into levels
     */
    if(pvt->levels[0]++ & pvt->levels[1]++ & pvt->levels[2]++) {
      /*
       * if doing grayscale with a static Colormap, all pvt->levels must match
       */
      if(src->bands == 1 && !(pvt->cmap->class & DynamicClass) &&
	 (pvt->levels[0] != pvt->levels[1] ||
	  pvt->levels[1] != pvt->levels[2]))
	MatchError(flo,ped, return(FALSE));
      break;
    }                     /* for StaticColor with no masks, we'll fall thru */
  case PseudoColor :
    pvt->levels[1] = pvt->levels[2] = 1;  /* for df levels calc (see below) */
  case GrayScale   :
    pvt->doHist    = TRUE;                /* used by Match technique */
  case StaticGray  :
    pvt->preFmt    = src->bands > 1;      /* used by Match technique */
    pvt->levels[0] = pvt->cells;
  }
  /* determine our output attributes from the input and Colormap levels
   */
  for(b = 0; b < src->bands; ++b)
    inf->format[b] = sf[b];
  df[0] = sf[0];
  inf->bands = src->bands;
  dst->bands = 1;
  if(!(df[0].levels = pvt->levels[0] * pvt->levels[1] * pvt->levels[2]))
    MatchError(flo,ped, return(FALSE)); /* can't do RGB using gray Colormap */

  /* search for the stride and pitch requirements that match our depth
   */
  UpdateFormatfromLevels(ped);
  for(f = 0; (f < screenInfo.numPixmapFormats &&
	      df[0].depth != screenInfo.formats[f].depth); ++f);
  if(df[0].stride != screenInfo.formats[f].bitsPerPixel)
    ImplementationError(flo,ped, return(FALSE)); /* XXX what to do ? */

  right_padm1 = screenInfo.formats[f].scanlinePad - 1;
  df[0].pitch = df[0].width * df[0].stride + right_padm1 & ~right_padm1;
  
  /* go do technique-specific stuff
   */
  if(!(ped->techVec->prepfnc(flo, ped, raw, &raw[1])))
    TechniqueError(flo, ped, raw->colorAlloc, raw->lenParams, return(FALSE));

  /* init the colorlist resource
   */
  if(pvt->list) {
    ResetColorList(pvt->list, pvt->list->mapPtr);
    pvt->list->mapID  = raw->colormap;
    pvt->list->mapPtr = pvt->cmap;
  }
  return(TRUE);
}                               /* end PrepConvertToIndex */

/*------------------------------------------------------------------------
------------------------ technique prep routines  ------------------------
------------------------------------------------------------------------*/
Bool PrepCtoIAllocAll(flo, ped, raw, tec) 
     floDefPtr flo;
     peDefPtr  ped;
     xieFloConvertToIndex  *raw;
     xieTecColorAllocMatch *tec;
{
  pCtoIDefPtr pvt = (pCtoIDefPtr) ped->elemPvt;
  inFloPtr     inf = &ped->inFloLst[SRCtag];
  formatPtr    fmt = &inf->format[0];
  
  if(!(pvt->class & DynamicClass) || !pvt->list || pvt->class == DirectColor)
    return(FALSE);	/* AllocAll needs a dynamic colormap and a colorlist
                         * (also, alpha release doesn't support direct color)
                         */
  
  /* XXX check levels for each band to make sure they're within reason...
   *     I think it's unreasonable to expect AllocAll to handle a potential
   *     of more than 2^16 gray shades or 2^8 colors per band (RNCS).
   *     (ddx only supports BYTE_PIXEL 3-band images so far)
   */
  if(inf->bands == 1 &&  fmt[0].levels > 1<<16 ||
     inf->bands == 3 && (fmt[0].class != BYTE_PIXEL   ||
			 fmt[0].class != fmt[1].class ||
			 fmt[1].class != fmt[2].class ))
    return(FALSE);
  
  return(TRUE);
}				/* end PrepCtoIAllocAll */

/*------------------------------------------------------------------------
------------------------ technique prep routines  ------------------------
------------------------------------------------------------------------*/
Bool PrepCtoIAllocMatch(flo, ped, raw, tec) 
     floDefPtr flo;
     peDefPtr  ped;
     xieFloConvertToIndex  *raw;
     xieTecColorAllocMatch *tec;
{
  pConvertToIndexMatchDefPtr tp = (pConvertToIndexMatchDefPtr)ped->techPvt;
  pCtoIDefPtr dix = (pCtoIDefPtr) ped->elemPvt;
  inFloPtr     inf = &ped->inFloLst[SRCtag];
  formatPtr    fmt = &inf->format[0];

  if(dix->class & DynamicClass && !dix->list)
    return(FALSE);	/* Match needs a colorlist if colormap is dynamic */
  
  /* Limits must be between 0.0 and 1.0 */
  if(tp->matchLimit < 0.0 || tp->matchLimit > 1.0 ||
     tp->grayLimit  < 0.0 || tp->grayLimit  > 1.0)
    return(FALSE);
  
  /* XXX check levels for each band to make sure they're within "reason" . . .
   *     When I conceived the AllocMatch technique (1989), it was intended
   *     to be a good citizen and make a "best attempt" at fitting an
   *     image's gray/color requirements into a shared colormap (e.g. the
   *     default colormap).  The number of levels in the input image should
   *     not exceed the total number of cells in the colormap.  AllocMatch
   *     can handle images with more levels than the colormap size, but it is
   *     computationally intense (n^2 or worse situation) and also requires
   *     sufficient memory to perform histogram and lookup table operations
   *     on (red.levels * green.levels * blue.levels) different pixel values.
   *     Therefore the SI will punt if the product of the levels is much
   *     greater than the colormap size (4x suggested in the protocol doc),
   *     or if levels for a single band image exceeds 2^16.  After all, XIE
   *     offers dither, constrain, arithmetic, and other wonderful ways to
   *     be "reasonable" (RNCS).
   */
  if(inf->bands == 1 &&  fmt[0].levels > 1<<16 ||
     inf->bands == 3 &&
     fmt[0].levels * fmt[1].levels * fmt[2].levels > dix->cells * 4)
    return(FALSE);

  /* NOTE: due to the above restrictions the AllocAll technique is being
   *       substituted for the Match technique for the alpha release at
   *       MIT's request.
   */
  return(FALSE);
}				/* end PrepCtoIAllocMatch */

/*------------------------------------------------------------------------
---------------------- routine: post execution cleanup -------------------
------------------------------------------------------------------------*/
static Bool DebriefConvertToIndex(flo,ped,ok)
     floDefPtr  flo;
     peDefPtr   ped;
     Bool	ok;
{
  xieFloConvertToIndex *raw = (xieFloConvertToIndex *)ped->elemRaw;
  pCtoIDefPtr pvt = (pCtoIDefPtr) ped->elemPvt;
  colorListPtr lst;
  
  if(pvt && (lst = pvt->list))
    if(lst->refCnt == 1) {
      FreeResourceByType(lst->ID, RT_COLORLIST, RT_NONE);
    } else {
      if(!ok || !lst->cellCnt)
	ResetColorList(lst, lst->mapPtr);

      --lst->refCnt;	/* nice doing business with you */
    }
  return(TRUE);
}                               /* end DebriefConvertToIndex */

/* end module ictoi.c */
