/* $XConsortium: ephoto.c,v 1.1 93/10/26 10:02:37 rws Exp $ */
/**** module ephoto.c ****/
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
  
	ephoto.c -- DIXIE routines for managing the ExportPhotomap element
  
	Robert NC Shelley && Dean Verheiden -- AGE Logic, Inc. April 1993
  
*****************************************************************************/

#define _XIEC_EPHOTO

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
#include <servermd.h>
  /*
   *  Server XIE Includes
   */
#include <corex.h>
#include <error.h>
#include <macro.h>
#include <photomap.h>
#include <element.h>
#include <technq.h>


/*
 *  routines referenced by other modules.
 */
peDefPtr	MakeEPhoto();

/*
 *  routines internal to this module
 */
static Bool	PrepEPhoto();
static Bool	DebriefEPhoto();

/*
 * dixie entry points
 */
static diElemVecRec ePhotoVec = {
    PrepEPhoto,
    DebriefEPhoto
    };


/*------------------------------------------------------------------------
----------------- routine: make an ExportPhotomap element ----------------
------------------------------------------------------------------------*/
peDefPtr MakeEPhoto(flo,tag,pe)
     floDefPtr      flo;
     xieTypPhototag tag;
     xieFlo        *pe;
{
  peDefPtr ped;
  inFloPtr inFlo;
  ELEMENT(xieFloExportPhotomap);
  ELEMENT_AT_LEAST_SIZE(xieFloExportPhotomap);
  ELEMENT_NEEDS_1_INPUT(src);
  
  if(!(ped = MakePEDef(1,(CARD32)stuff->elemLength<<2,sizeof(ePhotoDefRec)))) 
    FloAllocError(flo,tag,xieElemExportPhotomap, return(NULL));

  ped->diVec	    = &ePhotoVec;
  ped->phototag     = tag;
  ped->flags.export = TRUE;
  raw = (xieFloExportPhotomap *)ped->elemRaw;
  /*
   * copy the standard client element parameters (swap if necessary)
   */
  if( flo->reqClient->swapped ) {
    raw->elemType   = stuff->elemType;
    raw->elemLength = stuff->elemLength;
    cpswaps(stuff->src, raw->src);
    cpswapl(stuff->photomap, raw->photomap);
    cpswaps(stuff->encodeTechnique, raw->encodeTechnique);
    cpswaps(stuff->lenParams, raw->lenParams);
  }
  else  
    memcpy((char *)raw, (char *)stuff, sizeof(xieFloExportPhotomap));
  /*
   * assign phototags to inFlos
   */
  inFlo = ped->inFloLst;
  inFlo[SRCtag].srcTag = raw->src;

  /* 
	copy technique data (if any)
   */
  if(raw->encodeTechnique == xieValEncodeServerChoice) {
    if(raw->lenParams)
      TechniqueError(flo,ped,xieValEncode,raw->encodeTechnique,raw->lenParams,
		     return(ped));
  } else if(!(ped->techVec = FindTechnique(xieValEncode,raw->encodeTechnique)) 
    || !(ped->techVec->copyfnc(flo, ped, &stuff[1], &raw[1], raw->lenParams)))
    TechniqueError(flo,ped,xieValEncode,raw->encodeTechnique,raw->lenParams,
		   return(ped));
  return(ped);
}                               /* end MakeEPhoto */

/* All technique copy routines are defined in ecphoto.c */

/*------------------------------------------------------------------------
---------------- routine: prepare for analysis and execution -------------
------------------------------------------------------------------------*/
static Bool PrepEPhoto(flo,ped)
     floDefPtr  flo;
     peDefPtr   ped;
{
  xieFloExportPhotomap *raw = (xieFloExportPhotomap *)ped->elemRaw;
  ePhotoDefPtr pvt = (ePhotoDefPtr) ped->elemPvt;
  inFloPtr     inf = &ped->inFloLst[IMPORT];
  outFloPtr    src = &inf->srcDef->outFlo;
  outFloPtr    dst = &ped->outFlo;
  CARD32 b;
/*
 * protocol parameter verification not yet implemented,
 * and the encodeTechnique is being ignored.   Bob...
 */
  /* find the photomap resource and bind it to our flo */
  if(!(pvt->map = (photomapPtr) LookupIDByType(raw->photomap, RT_PHOTOMAP)))
    PhotomapError(flo,ped,raw->photomap, return(FALSE));
  ++pvt->map->refCnt;

  /* 
     grab a copy of the input attributes and propagate them to our output
   */
  dst->bands = inf->bands = src->bands;
	/*  note: dst->bands may change later if we discover we are
	 *  encoding TripleBand data interleaved BandByPixel 
	 */
  for(b = 0; b < src->bands; ++b) {
    if (IsntCanonic(src->format[b].class))
      MatchError(flo, ped, return(FALSE));
    dst->format[b] = inf->format[b] = src->format[b];
  }

  if(raw->encodeTechnique != xieValEncodeServerChoice &&
     !(ped->techVec->prepfnc(flo, ped, &raw[1])))
    TechniqueError(flo,ped,xieValEncode,raw->encodeTechnique,raw->lenParams,
                   return(FALSE));

  return(TRUE);
}                               /* end PrepEPhoto */

/* All technique prep routines are defined in ecphoto.c */

/*------------------------------------------------------------------------
---------------------- routine: post execution cleanup -------------------
------------------------------------------------------------------------*/
static Bool DebriefEPhoto(flo,ped,ok)
     floDefPtr  flo;
     peDefPtr   ped;
     Bool	ok;
{
  xieFloExportPhotomap *raw = (xieFloExportPhotomap *)ped->elemRaw;
  ePhotoDefPtr pvt = (ePhotoDefPtr) ped->elemPvt;
  inFloPtr     inf = &ped->inFloLst[SRCtag];
  outFloPtr    src = &inf->srcDef->outFlo;
  photomapPtr  map;
  CARD32   b;
  
  if(!(pvt && (map = pvt->map))) return(FALSE);

  if(ok && map->refCnt > 1) {
    /* 
        free old compression parameters and image data
     */
    if (map->tecParms)
	    map->tecParms = (void *)XieFree(map->tecParms);
    if (map->pvtParms)
	    map->pvtParms = (void *)XieFree(map->pvtParms);
    map->pvtParms = pvt->pvtParms;
    map->dataClass = (src->bands == 3)? xieValTripleBand : xieValSingleBand;

    for(b = 0; b < map->bands; b++) 
      FreeStrips(&map->strips[b]);
    /* 
	stash our new attributes and data into the photomap
     */
    map->bands = ped->outFlo.bands;
    map->dataType = (map->format[0].class & UNCONSTRAINED
		     ? xieValUnconstrained : xieValConstrained);
    for(b = 0; b < map->bands; ++b) {
      map->format[b] = ped->outFlo.format[b];
      DebriefStrips(&ped->outFlo.export[b],&map->strips[b]);
    }
    if (src->bands == 3  &&  map->bands == 1) 	
      for(b = 1; b < src->bands; ++b) 
        map->format[b] = src->format[b];
	  /* save these, because we'll need them when we decode */

    /* Create technique parameter record for decoding the photomap */
    switch(raw->encodeTechnique) {
	/* Encode server choice requests as uncompressed data */
        case xieValEncodeServerChoice:
	    if (map->bands == 1) {
	        xieTecDecodeUncompressedSingle *dtec;

	        if (!(dtec = (xieTecDecodeUncompressedSingle *)
			    XieMalloc(sizeof(xieTecDecodeUncompressedSingle))))
	            FloAllocError(flo,ped->phototag,xieElemExportPhotomap, 
		    					return(FALSE));
	        map->technique    = xieValDecodeUncompressedSingle;
	        map->tecParms     = (void *)dtec;
#if (IMAGE_BYTE_ORDER == MSBFirst) /* Conform to server's "native" format */
	        dtec->fillOrder   = xieValMSFirst;
	        dtec->pixelOrder  = xieValMSFirst; 
#else
	        dtec->fillOrder   = xieValLSFirst;
	        dtec->pixelOrder  = xieValLSFirst; 
#endif
	        dtec->pixelStride = map->format[0].stride;
	        dtec->leftPad     = 0;
	        dtec->scanlinePad = PITCH_MOD >> 3;
	    } else {
	        xieTecDecodeUncompressedTriple *dtec;
	        int i;

	        if (!(dtec = (xieTecDecodeUncompressedTriple *)
			    XieMalloc(sizeof(xieTecDecodeUncompressedTriple))))
	            FloAllocError(flo,ped->phototag,xieElemExportPhotomap, 
		    				    return(FALSE));
	        map->technique    = xieValDecodeUncompressedTriple;
	        map->tecParms     = (void *)dtec;
#if (IMAGE_BYTE_ORDER == MSBFirst) /* Conform to server's "native" format */
	        dtec->fillOrder   = xieValMSFirst;
	        dtec->pixelOrder  = xieValMSFirst; 
#else
	        dtec->fillOrder   = xieValLSFirst;
	        dtec->pixelOrder  = xieValLSFirst; 
#endif
	        dtec->bandOrder   = xieValLSFirst;
	        dtec->interleave  = xieValBandByPlane; 
	        for (i = 0; i < 3; i++) {
	 	    dtec->leftPad[i] = 0;
		    dtec->pixelStride[i] = map->format[i].stride; 
		    dtec->scanlinePad[i] = PITCH_MOD >> 3;
	        }
	    }
	    break;
        case xieValEncodeUncompressedSingle:
	    {
	    xieTecEncodeUncompressedSingle *etec = 
				(xieTecEncodeUncompressedSingle *)&raw[1];
	    xieTecDecodeUncompressedSingle *dtec;

	    if (!(dtec = (xieTecDecodeUncompressedSingle *)
			XieMalloc(sizeof(xieTecDecodeUncompressedSingle))))
	        FloAllocError(flo,ped->phototag,xieElemExportPhotomap, 
		    				return(FALSE));
	    map->technique    = xieValDecodeUncompressedSingle;
	    map->tecParms     = (void *)dtec;
	    dtec->fillOrder   = etec->fillOrder;
	    dtec->pixelOrder  = etec->pixelOrder;
	    dtec->pixelStride = etec->pixelStride;
	    dtec->leftPad     = 0;
	    dtec->scanlinePad = etec->scanlinePad;
	    }
	    break;
        case xieValEncodeUncompressedTriple: 
	    {
	    xieTecEncodeUncompressedTriple *etec = 
				(xieTecEncodeUncompressedTriple *)&raw[1];
	    xieTecDecodeUncompressedTriple *dtec;
	    int i;

	    if (!(dtec = (xieTecDecodeUncompressedTriple *)
			XieMalloc(sizeof(xieTecDecodeUncompressedTriple))))
	        FloAllocError(flo,ped->phototag,xieElemExportPhotomap, 
		    				return(FALSE));
	    map->technique    = xieValDecodeUncompressedTriple;
	    map->tecParms     = (void *)dtec;
	    dtec->fillOrder   = etec->fillOrder;
	    dtec->pixelOrder  = etec->pixelOrder;
	    dtec->bandOrder   = etec->bandOrder;
	    dtec->interleave  = etec->interleave;
	    for (i = 0; i < 3; i++) {
		dtec->leftPad[i] = 0;
		dtec->pixelStride[i] = etec->pixelStride[i];
		dtec->scanlinePad[i] = etec->scanlinePad[i];
	    }
	    }
	    break;
	case xieValEncodeG31D:
	    {
	    xieTecEncodeG31D *etec = (xieTecEncodeG31D *)&raw[1];
	    xieTecDecodeG31D *dtec;

	    if (!(dtec = (xieTecDecodeG31D *)
					XieMalloc(sizeof(xieTecDecodeG31D))))
	        FloAllocError(flo,ped->phototag,xieElemExportPhotomap, 
		    				return(FALSE));
	    map->technique     = xieValDecodeG31D;
	    map->tecParms      = (void *)dtec;
	    dtec->normal       = TRUE;
	    dtec->radiometric  = etec->radiometric;
	    dtec->encodedOrder = etec->encodedOrder;
	    }
	    break;
	case xieValEncodeG32D:
	    {
	    xieTecEncodeG32D *etec = (xieTecEncodeG32D *)&raw[1];
	    xieTecDecodeG32D *dtec;

	    if (!(dtec = (xieTecDecodeG32D *)
					XieMalloc(sizeof(xieTecDecodeG32D))))
    		FloAllocError(flo,ped->phototag,xieElemExportPhotomap, 
						return(FALSE));
	    map->technique     = xieValDecodeG32D;
	    map->tecParms      = (void *)dtec;
	    dtec->normal       = TRUE;
	    dtec->radiometric  = etec->radiometric;
	    dtec->encodedOrder = etec->encodedOrder;
	    }
	    break;
	case xieValEncodeG42D:
	    {
	    xieTecEncodeG42D *etec = (xieTecEncodeG42D *)&raw[1];
	    xieTecDecodeG42D *dtec;

	    if (!(dtec = (xieTecDecodeG42D *)
					XieMalloc(sizeof(xieTecDecodeG42D))))
    		FloAllocError(flo,ped->phototag,xieElemExportPhotomap, 
						return(FALSE));
	    map->technique     = xieValDecodeG42D;
	    map->tecParms      = (void *)dtec;
	    dtec->normal       = TRUE;
	    dtec->radiometric  = etec->radiometric;
	    dtec->encodedOrder = etec->encodedOrder;
	    }
	    break;
	case xieValEncodeJPEGBaseline:
	    {
	    xieTecEncodeJPEGBaseline *etec = 
					(xieTecEncodeJPEGBaseline *)&raw[1];
	    xieTecDecodeJPEGBaseline *dtec;

	    if (!(dtec = (xieTecDecodeJPEGBaseline *)
				XieMalloc(sizeof(xieTecDecodeJPEGBaseline))))
    		FloAllocError(flo,ped->phototag,xieElemExportPhotomap, 
						return(FALSE));
	    map->technique     = xieValDecodeJPEGBaseline;
	    map->tecParms      = (void *)dtec;
	    dtec->interleave   = etec->interleave;
	    dtec->bandOrder    = etec->bandOrder;
	    }
	    break;
   	case xieValEncodeTIFF2: 
	    {
	    xieTecEncodeTIFF2 *etec = (xieTecEncodeTIFF2 *)&raw[1];
	    xieTecDecodeTIFF2 *dtec;

	    if (!(dtec = (xieTecDecodeTIFF2 *)
				XieMalloc(sizeof(xieTecDecodeTIFF2))))
    		FloAllocError(flo,ped->phototag,xieElemExportPhotomap, 
						return(FALSE));
	    map->technique     = xieValDecodeTIFF2;
	    map->tecParms      = (void *)dtec;
	    dtec->normal       = TRUE;
	    dtec->radiometric  = etec->radiometric;
	    dtec->encodedOrder = etec->encodedOrder;
	    }
	    break;
	case xieValEncodeTIFFPackBits:
	    {
	    xieTecEncodeTIFFPackBits *etec = 
					(xieTecEncodeTIFFPackBits *)&raw[1];
	    xieTecDecodeTIFFPackBits *dtec;

	    if (!(dtec = (xieTecDecodeTIFFPackBits *)
				XieMalloc(sizeof(xieTecDecodeTIFFPackBits))))
    		FloAllocError(flo,ped->phototag,xieElemExportPhotomap, 
						return(FALSE));
	    map->technique     = xieValDecodeTIFFPackBits;
	    map->tecParms      = (void *)dtec;
	    dtec->normal       = TRUE;
	    dtec->encodedOrder = etec->encodedOrder;
	    }
	    break;
	case xieValEncodeJPEGLossless:
	default:
	    ImplementationError(flo,ped, return(FALSE));
    }
  }
  /* free image data that's left over on our outFlo
   */
  for(b = 0; b < ped->outFlo.bands; b++)
    FreeStrips(&ped->outFlo.export[b]);
  
  /* 
       unbind ourself from the photomap
   */
  if(map->refCnt > 1)
    --map->refCnt;
  else if(LookupIDByType(raw->photomap, RT_PHOTOMAP))
    FreeResourceByType(map->ID, RT_PHOTOMAP, RT_NONE);
  else
    DeletePhotomap(map, map->ID);

  return(TRUE);
}                               /* end DebriefEPhoto */
/* end module ephoto.c */
