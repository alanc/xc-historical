/* $XConsortium: ilut.c,v 1.1 93/10/26 09:59:42 rws Exp $ */
/**** module ilut.c ****/
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
  
	ilut.c -- DIXIE routines for managing the ImportLUT element
  
	Larry Hare -- AGE Logic, Inc. June 1993
  
*****************************************************************************/

#define _XIEC_ILUT

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
peDefPtr	MakeILUT();

/*
 *  routines internal to this module
 */
static Bool	PrepILUT();
static Bool	DebriefILUT();

/*
 * dixie entry points
 */
static diElemVecRec iLUTVec = {
  PrepILUT,
  DebriefILUT
  };


/*------------------------------------------------------------------------
----------------- routine: make an import lut element ---------------
------------------------------------------------------------------------*/
peDefPtr MakeILUT(flo,tag,pe)
     floDefPtr      flo;
     xieTypPhototag tag;
     xieFlo        *pe;
{
  peDefPtr ped;
  ELEMENT(xieFloImportLUT);
  ELEMENT_SIZE_MATCH(xieFloImportLUT);
  
  if(!(ped = MakePEDef(1,(CARD32)stuff->elemLength<<2,sizeof(iLUTDefRec)))) 
    FloAllocError(flo,tag,xieElemImportLUT, return(NULL));

  ped->diVec	    = &iLUTVec;
  ped->phototag     = tag;
  ped->flags.import = TRUE;
  raw = (xieFloImportLUT *)ped->elemRaw;
  
  /*
   * copy the client element parameters (swap if necessary)
   */
  if( flo->reqClient->swapped ) {
    raw->elemType   = stuff->elemType;
    raw->elemLength = stuff->elemLength;
    cpswapl(stuff->lut, raw->lut);
  }
  else
    memcpy((char *)raw, (char *)stuff, sizeof(xieFloImportLUT));

  return(ped);
}                               /* end MakeILUT */


/*------------------------------------------------------------------------
---------------- routine: prepare for analysis and execution -------------
------------------------------------------------------------------------*/
static Bool PrepILUT(flo,ped)
     floDefPtr  flo;
     peDefPtr   ped;
{
  xieFloImportLUT *raw = (xieFloImportLUT *)ped->elemRaw;
  iLUTDefPtr	pvt = (iLUTDefPtr) ped->elemPvt;
  inFloPtr	inf = &ped->inFloLst[IMPORT];
  outFloPtr	dst = &ped->outFlo;
  formatPtr	dfp, ifp;
  lutPtr	lut;
  CARD32	b, nbands;

  /* find the LUT resource and bind it to our flo */
  if( !(lut = (lutPtr) LookupIDByType(raw->lut, RT_LUT)) )
	LUTError(flo,ped,raw->lut, return(FALSE));
  ++lut->refCnt;
  pvt->lut = lut;

  if(!lut->lutCnt)
    AccessError(flo,ped, return(FALSE));

  nbands = lut->lutCnt; 
  if (nbands != 1 && nbands != 3) 
     ImplementationError(flo,ped,return(FALSE));

  /* propagate LUT attributes to our output */
  dst->bands = inf->bands = nbands;
  for(b = 0, dfp = &(dst->format[0]), ifp = &(inf->format[0]);
      b < nbands; b++, dfp++, ifp++) {
    dfp->band	= ifp->band	= b;
    dfp->class	= ifp->class	= LUT_ARRAY;
    dfp->levels	= ifp->levels	= lut->format[b].level;
    dfp->height	= ifp->height	= lut->format[b].length;	/* ugly hack */
    dfp->width	= ifp->width	= lut->format[b].bandOrder;	/* ugly hack */
    dfp->interleaved = ifp->interleaved = FALSE;
    /* width = 1; depth=8?; stride=8; pitch = 8*height; */
  }

  return(TRUE);
}                               /* end PrepILUT */


/*------------------------------------------------------------------------
---------------------- routine: post execution cleanup -------------------
------------------------------------------------------------------------*/
static Bool DebriefILUT(flo,ped,ok)
     floDefPtr  flo;
     peDefPtr   ped;
     Bool	ok;
{
  xieFloImportLUT *raw = (xieFloImportLUT *)ped->elemRaw;
  iLUTDefPtr pvt = (iLUTDefPtr) ped->elemPvt;
  lutPtr lut;

  if(pvt && (lut = pvt->lut))
    if(lut->refCnt > 1)
      --lut->refCnt;
    else if(LookupIDByType(raw->lut, RT_LUT))
      FreeResourceByType(lut->ID, RT_LUT, RT_NONE);
    else
      DeleteLUT(lut, lut->ID);

  return(TRUE);
}                               /* end DebriefILUT */

/* end module ilut.c */
