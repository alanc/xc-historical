/* $XConsortium$ */
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
  
	Robert NC Shelley -- AGE Logic, Inc. April 1993
  
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
  if( flo->client->swapped ) {
    raw->elemType   = stuff->elemType;
    raw->elemLength = stuff->elemLength;
    cpswaps(stuff->src, raw->src);
    cpswapl(stuff->photomap, raw->photomap);
    cpswaps(stuff->encodeTechnique, raw->encodeTechnique);
    cpswaps(stuff->lenParams, raw->lenParams);
  }
  else  
    bcopy((char *)stuff, (char *)raw, sizeof(xieFloExportPhotomap));
  /*
   * assign phototags to inFlos
   */
  inFlo = ped->inFloLst;
  inFlo[SRCtag].srcTag = raw->src;

#ifdef phooblat
Ignore for the sake of Xhibition quickie -- Bob...
  /*
   * copy technique data (if any)
   */
  if(!(ped->techVec = FindTechnique(xieValEncode, raw->encodeTechnique)) ||
     !(ped->techVec->copyfnc(flo, ped, &stuff[1], &raw[1], raw->lenParams)))
    TechniqueError(flo,ped,raw->encodeTechnique,raw->lenParams, return(ped));
#endif
  return(ped);
}                               /* end MakeEPhoto */


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

  /* grab a copy of the input attributes and propagate them to our output
   */
  dst->bands = inf->bands = src->bands;
  for(b = 0; b < src->bands; ++b)
    dst->format[b] = inf->format[b] = src->format[b];

  return(TRUE);
}                               /* end PrepEPhoto */


/*------------------------------------------------------------------------
---------------------- routine: post execution cleanup -------------------
------------------------------------------------------------------------*/
static Bool DebriefEPhoto(flo,ped,ok)
     floDefPtr  flo;
     peDefPtr   ped;
     Bool	ok;
{
  ePhotoDefPtr pvt = (ePhotoDefPtr) ped->elemPvt;
  photomapPtr  map;
  CARD32   b;
  
  if(!(pvt && (map = pvt->map))) return(FALSE);

  if(ok && map->refCnt > 1) {
    /* free old compression parameters and image data
     */
    for(b = 0; b < map->bands; b++) {
      if(map->format[b].params)
	XieFree(map->format[b].params);
      FreeStrips(&map->strips[b]);
    }
    /* stash our new attributes and data into the photomap
     */
    map->bands = ped->outFlo.bands;
    map->technique = xieValEncodeServerChoice;      /* heh, heh, heh :-) */
    map->dataType = (map->format[0].class & UNCONSTRAINED
		     ? xieValUnconstrained : xieValConstrained);
    for(b = 0; b < map->bands; ++b) {
      map->format[b] = ped->outFlo.format[b];
      DebriefStrips(&ped->outFlo.export[b],&map->strips[b]);
    }
  }
  /* free image data that's left over on our outFlo
   */
  for(b = 0; b < ped->outFlo.bands; b++)
    FreeStrips(&ped->outFlo.export[b]);
  
  /* unbind ourself from the photomap
   */
  if(pvt->map->refCnt == 1)
    FreeResourceByType(pvt->map->ID, RT_PHOTOMAP, RT_NONE);
  else
    --pvt->map->refCnt;
  
  return(TRUE);
}                               /* end DebriefEPhoto */

/* end module ephoto.c */
