/* $XConsortium: iphoto.c,v 1.1 93/10/26 09:59:29 rws Exp $ */
/**** module iphoto.c ****/
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
  
	iphoto.c -- DIXIE routines for managing the ImportPhotomap element
  
	Robert NC Shelley -- AGE Logic, Inc. April 1993
  
*****************************************************************************/

#define _XIEC_IPHOTO

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
#include <photomap.h>
#include <element.h>


/*
 *  routines referenced by other modules.
 */
peDefPtr	MakeIPhoto();



/*
 *  routines internal to this module
 */
static Bool	PrepIPhoto();
static Bool	PrepIPhotoUnPlane();
static Bool	PrepIPhotoG31D();
static Bool	PrepIPhotoG32D();
static Bool	PrepIPhotoG42D();
static Bool	PrepIPhotoTIFF2();
static Bool	PrepIPhotoTIFFPackBits();
static Bool	DebriefIPhoto();

#if XIE_FULL
static Bool	PrepIPhotoUnTripleInterleaved();
static Bool	PrepIPhotoJPEGBaseline();
static Bool	PrepIPhotoJPEGLossless();
#endif

/*
 * dixie entry points
 */
static diElemVecRec iPhotoVec = {
  PrepIPhoto,
  DebriefIPhoto
  };


/*------------------------------------------------------------------------
----------------- routine: make an import photomap element ---------------
------------------------------------------------------------------------*/
peDefPtr MakeIPhoto(flo,tag,pe)
     floDefPtr      flo;
     xieTypPhototag tag;
     xieFlo        *pe;
{
  peDefPtr ped;
  ELEMENT(xieFloImportPhotomap);
  ELEMENT_SIZE_MATCH(xieFloImportPhotomap);
  
  if(!(ped = MakePEDef(1,(CARD32)stuff->elemLength<<2,sizeof(iPhotoDefRec)))) 
    FloAllocError(flo,tag,xieElemImportPhotomap, return(NULL));

  ped->diVec	    = &iPhotoVec;
  ped->phototag     = tag;
  ped->flags.import = TRUE;
  raw = (xieFloImportPhotomap *)ped->elemRaw;
  
  /*
   * copy the client element parameters (swap if necessary)
   */
  if( flo->reqClient->swapped ) {
    raw->elemType   = stuff->elemType;
    raw->elemLength = stuff->elemLength;
    cpswapl(stuff->photomap, raw->photomap);
    raw->notify = stuff->notify;
  }
  else
    memcpy((char *)raw, (char *)stuff, sizeof(xieFloImportPhotomap));

  return(ped);
}                               /* end MakeIPhoto */


/*------------------------------------------------------------------------
---------------- routine: prepare for analysis and execution -------------
------------------------------------------------------------------------*/
static Bool PrepIPhoto(flo,ped)
     floDefPtr  flo;
     peDefPtr   ped;
{
  xieFloImportPhotomap *raw = (xieFloImportPhotomap *)ped->elemRaw;
  iPhotoDefPtr pvt = (iPhotoDefPtr) ped->elemPvt;
  inFloPtr     inf = &ped->inFloLst[IMPORT];
  outFloPtr    dst = &ped->outFlo;
  photomapPtr  map;
  CARD32 b;

  /* find the photomap resource and bind it to our flo */
  if( !(map = (photomapPtr) LookupIDByType(raw->photomap, RT_PHOTOMAP)) )
	PhotomapError(flo,ped,raw->photomap, return(FALSE));
  ++map->refCnt;

  /* Load up a generic structure for importing photos from map and client */
  pvt->map = (void *)map;

  if(!map->bands)
    AccessError(flo,ped, return(FALSE));

  /* grab a copy of the input attributes and propagate them to our output */
  inf->bands = map->bands;

  /* copy map formats to inflo format */
  for(b = 0; b < inf->bands; b++) 
    inf->format[b] = map->format[b];

  /* also copy them to the outflo format, handling interleave if necessary  */
  dst->bands =  (map->dataClass == xieValTripleBand)? 3 : 
		(map->dataClass == xieValSingleBand)? 1 : 0;

  for(b = 0; b < dst->bands; b++)  {
    dst->format[b] = map->format[b];
    dst->format[b].interleaved = FALSE;
  }
  /* NOTE: the loop is over dst->bands,  not map->bands. This is because
   * dst->bands can be 3 when map->bands is 1. The in ephoto.c saves all 
   * the formats of the inflo in the photomap, so they are available now 
   * when we need them. interleaved is FALSE by definition because only
   * ExportPhotomap elements can *produce* interleaved data.
   */
  
  switch(map->technique) {
    case xieValDecodeUncompressedSingle:
	return(PrepIPhotoUnPlane(flo, ped));
	break;
    case xieValDecodeG31D:
        return(PrepIPhotoG31D(flo, ped));
	break;
    case xieValDecodeG32D:
        return(PrepIPhotoG32D(flo, ped));
	break;
    case xieValDecodeG42D:
        return(PrepIPhotoG42D(flo, ped));
	break;
    case xieValDecodeTIFF2:
        return(PrepIPhotoTIFF2(flo, ped));
	break;
    case xieValDecodeTIFFPackBits:
        return (PrepIPhotoTIFFPackBits(flo, ped));
	break;
#if XIE_FULL
    case xieValDecodeUncompressedTriple:
	if (inf->format[0].interleaved)
	    return(PrepIPhotoUnTripleInterleaved(flo, ped));
	else
	    return(PrepIPhotoUnPlane(flo, ped));
	break;
    case xieValDecodeJPEGBaseline:
        return(PrepIPhotoJPEGBaseline(flo, ped));
	break;
    case xieValDecodeJPEGLossless:
        return(PrepIPhotoJPEGLossless(flo, ped));
	break;
#endif
    default:
        ImplementationError(flo, ped, return(FALSE));
  }
  return(TRUE);
}                               /* end PrepIPhoto */

/*------------------------------------------------------------------------
---------- routines: prepare for analysis and execution ------------------
------------------------------------------------------------------------*/

/* Note: parameters are "guaranteed" to be correct because they were 
   introduced into the photomap structure by ExportPhotomap.
*/

/* Prep routine for uncompressed plane data */
static Bool PrepIPhotoUnPlane(flo, ped) 
     floDefPtr flo;
     peDefPtr  ped;
{
  inFloPtr   inf =  &ped->inFloLst[IMPORT];
  int i;

  ped->outFlo.bands = inf->bands;

  /* Copy input to output, setting differing output parameters when necessary */
  for (i = 0; i < ped->outFlo.bands; i++) {
     ped->outFlo.format[i] = inf->format[i];
     ped->outFlo.format[i].interleaved = FALSE;
  }

   /* Fill in other format parameters based on the number of output levels */
  if (UpdateFormatfromLevels(ped) == FALSE)
     ImplementationError(flo, ped, return(FALSE));

  return(TRUE);
} /* PrepIPhotoUnPlane */

static Bool PrepIPhotoG31D(flo, ped) 
     floDefPtr flo;
     peDefPtr  ped;
{
  inFloPtr   inf =  &ped->inFloLst[IMPORT];
  int i;
  /*
   * determine output attributes from input parameters
   */
  ped->outFlo.bands = inf->bands;
  for (i = 0; i < inf->bands; i++) {
    ped->outFlo.format[i] = inf->format[i];
    ped->outFlo.format[i].interleaved = FALSE;
  }

  if (UpdateFormatfromLevels(ped) == FALSE)
	ImplementationError(flo, ped, return(FALSE));

  return(TRUE);

} /* PrepIPhotoG31D */

static Bool PrepIPhotoG32D(flo, ped) 
     floDefPtr flo;
     peDefPtr  ped;
{
  inFloPtr   inf =  &ped->inFloLst[IMPORT];
  int i;

  /*
   * determine output attributes from input parameters
   */
  ped->outFlo.bands = inf->bands;
  for (i = 0; i < inf->bands; i++) {
    ped->outFlo.format[i] = inf->format[i];
    ped->outFlo.format[i].interleaved = FALSE;
  }

  if (UpdateFormatfromLevels(ped) == FALSE)
	ImplementationError(flo, ped, return(FALSE));

  return(TRUE);

} /* PrepIPhotoG32D */

static Bool PrepIPhotoG42D(flo, ped) 
     floDefPtr flo;
     peDefPtr  ped;
{
  inFloPtr   inf =  &ped->inFloLst[IMPORT];
  int i;

  /*
   * determine output attributes from input parameters
   */
  ped->outFlo.bands = inf->bands;
  for (i = 0; i < inf->bands; i++) {
    ped->outFlo.format[i] = inf->format[i];
    ped->outFlo.format[i].interleaved = FALSE;
  }

  if (UpdateFormatfromLevels(ped) == FALSE)
	ImplementationError(flo, ped, return(FALSE));

  return(TRUE);

} /* PrepIPhotoG42D */

static Bool PrepIPhotoTIFF2(flo, ped) 
     floDefPtr flo;
     peDefPtr  ped;
{
  inFloPtr   inf =  &ped->inFloLst[IMPORT];
  int i;

  /*
   * determine output attributes from input parameters
   */
  ped->outFlo.bands = inf->bands;
  for (i = 0; i < inf->bands; i++) {
    ped->outFlo.format[i] = inf->format[i];
    ped->outFlo.format[i].interleaved = FALSE;
  }

  if (UpdateFormatfromLevels(ped) == FALSE)
	ImplementationError(flo, ped, return(FALSE));

  return(TRUE);

} /* PrepIPhotoTIFF2 */

static Bool PrepIPhotoTIFFPackBits(flo, ped) 
     floDefPtr flo;
     peDefPtr  ped;
{
  inFloPtr   inf =  &ped->inFloLst[IMPORT];
  int i;

  /*
   * determine output attributes from input parameters
   */
  ped->outFlo.bands = inf->bands;
  for (i = 0; i < inf->bands; i++) {
    ped->outFlo.format[i] = inf->format[i];
    ped->outFlo.format[i].interleaved = FALSE;
  }

  if (UpdateFormatfromLevels(ped) == FALSE)
	ImplementationError(flo, ped, return(FALSE));

  return(TRUE);

} /* PrepIPhotoTIFFPackBits */


#if XIE_FULL
/* Prep routine for uncompressed triple band data interleaved by pixel */
static Bool PrepIPhotoUnTripleInterleaved(flo, ped) 
     floDefPtr flo;
     peDefPtr  ped;
{
  inFloPtr   inf =  &ped->inFloLst[IMPORT];

  /* Set up 3 outflows for the one interleaved inflo */
  ped->outFlo.bands = 3;

   /* Fill in other format parameters based on the number of output levels */
  if (UpdateFormatfromLevels(ped) == FALSE)
     ImplementationError(flo, ped, return(FALSE));

  return(TRUE);
} /* PrepIPhotoUnTripleInterleaved */

static Bool PrepIPhotoJPEGBaseline(flo, ped) 
     floDefPtr flo;
     peDefPtr  ped;
{
  /*
   * determine output attributes from input parameters
   */
  if(!UpdateFormatfromLevels(ped))
    MatchError(flo, ped, return(FALSE));

  return(TRUE);

} /* PrepIPhotoJPEGBaseline */

static Bool PrepIPhotoJPEGLossless(flo, ped) 
     floDefPtr flo;
     peDefPtr  ped;
{
  inFloPtr   inf =  &ped->inFloLst[IMPORT];
  int i;

  /*
   * determine output attributes from input parameters
   */
  ped->outFlo.bands = inf->bands;
  for (i = 0; i < inf->bands; i++) {
    ped->outFlo.format[i] = inf->format[i];
    ped->outFlo.format[i].interleaved = FALSE;
  }

  if (UpdateFormatfromLevels(ped) == FALSE)
	ImplementationError(flo, ped, return(FALSE));

  return(TRUE);

} /* PrepIPhotoJPEGLossless */
#endif


/*------------------------------------------------------------------------
---------------------- routine: post execution cleanup -------------------
------------------------------------------------------------------------*/
static Bool DebriefIPhoto(flo,ped,ok)
     floDefPtr  flo;
     peDefPtr   ped;
     Bool	ok;
{
  xieFloImportPhotomap *raw = (xieFloImportPhotomap *)ped->elemRaw;
  iPhotoDefPtr pvt = (iPhotoDefPtr) ped->elemPvt;
  photomapPtr map;

  if(pvt && (map = (photomapPtr)pvt->map))
    if(map->refCnt > 1)
      --map->refCnt;
    else if(LookupIDByType(raw->photomap, RT_PHOTOMAP))
      FreeResourceByType(map->ID, RT_PHOTOMAP, RT_NONE);
    else
      DeletePhotomap(map, map->ID);

  return(TRUE);
}                               /* end DebriefIPhoto */

/* end module iphoto.c */
