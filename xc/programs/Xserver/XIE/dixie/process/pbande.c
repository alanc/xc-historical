/* $XConsortium$ */
/**** module pbande.c ****/
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
  
	pbande.c -- DIXIE routines for managing the band combine element
  
	Dean Verheiden -- AGE Logic, Inc. July 1993
  
*****************************************************************************/

#define _XIEC_PBANDE

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
#include <macro.h>
#include <element.h>
#include <difloat.h>


/*
 *  routines referenced by other modules.
 */
peDefPtr	MakeBandExt();

/*
 *  routines internal to this module
 */
static Bool	PrepBandExt();

/*
 * dixie entry points
 */
static diElemVecRec pBandExtVec = {
    PrepBandExt		/* prepare for analysis and execution	*/
    };


/*------------------------------------------------------------------------
----------------------- routine: make a blend element --------------------
------------------------------------------------------------------------*/
peDefPtr MakeBandExt(flo,tag,pe)
     floDefPtr      flo;
     xieTypPhototag tag;
     xieFlo        *pe;
{
  int inputs, index;
  double *coef;
  peDefPtr ped;
  inFloPtr inFlo;
  ELEMENT(xieFloBandExtract);
  ELEMENT_SIZE_MATCH(xieFloBandExtract);
  ELEMENT_NEEDS_1_INPUT(src); 

  if(!(ped = MakePEDef(1,(CARD32)stuff->elemLength<<2,sizeof(pBandExtDefRec))))
    FloAllocError(flo, tag, xieElemBandExtract, return(NULL));

  ped->diVec	     = &pBandExtVec;
  ped->phototag      = tag;
  ped->flags.process = TRUE;
  raw = (xieFloBandExtract *)ped->elemRaw;
  coef = ((pBandExtDefPtr)ped->elemPvt)->coef;
  /*
   * copy the client element parameters (swap if necessary)
   */
  if( flo->client->swapped ) {
    raw->elemType   = stuff->elemType;
    raw->elemLength = stuff->elemLength;
    cpswaps(stuff->src, raw->src);
    coef[0] = ConvertFromIEEE(lswapl(stuff->constant0));
    coef[1] = ConvertFromIEEE(lswapl(stuff->constant1));
    coef[2] = ConvertFromIEEE(lswapl(stuff->constant2));
  } else {
    bcopy((char *)stuff, (char *)raw, sizeof(xieFloBandExtract));
    coef[0] = ConvertFromIEEE(stuff->constant0);
    coef[1] = ConvertFromIEEE(stuff->constant1);
    coef[2] = ConvertFromIEEE(stuff->constant2);
  }
  /* assign phototags to inFlos
   */
  inFlo = ped->inFloLst;
  inFlo[SRCtag].srcTag = raw->src;
  
  return(ped);
}                               /* end MakeBandExt */


/*------------------------------------------------------------------------
---------------- routine: prepare for analysis and execution -------------
------------------------------------------------------------------------*/
static Bool PrepBandExt(flo,ped)
     floDefPtr  flo;
     peDefPtr   ped;
{
  xieFloBandExtract *raw = (xieFloBandExtract *)ped->elemRaw;
  inFloPtr           inf = &ped->inFloLst[SRCtag]; 
  outFloPtr          src = &inf->srcDef->outFlo;
  outFloPtr          dst = &ped->outFlo;
  pBandExtDefPtr    pvt  = (pBandExtDefPtr)ped->elemPvt;
  double           *coef = pvt->coef;
  
  pvt->mix_bands = FALSE;
  
  /* All inputs must be single band */
  if (src->bands != 3)
    MatchError(flo,ped, return(FALSE));
  
  if (coef[0] == 1.0 && coef[1] == 0.0 && coef[2] == 0.0)
    pvt->out_band = 0;
  else if (coef[1] == 1.0 && coef[0] == 0.0 && coef[2] == 0.0)
    pvt->out_band = 1;
  else if (coef[2] == 1.0 && coef[0] == 0.0 && coef[1] == 0.0)
    pvt->out_band = 2;
  else {
    /* If mixing bands, dimensions must match */
    if (src->format[0].width  != src->format[1].width ||
	src->format[1].width  != src->format[2].width ||
	src->format[0].height != src->format[1].height ||
	src->format[1].height != src->format[2].height) {
      MatchError(flo,ped, return(FALSE));
    } else {		
      pvt->mix_bands = TRUE;
      /* Set out_band to the index of input band with hightest coefficient */
      if (coef[0] >= coef[1] && coef[0] >= coef[1])
	pvt->out_band = 0;
      else if (coef[1] >= coef[0] && coef[1] >= coef[2])
	pvt->out_band = 1;
      else
	pvt->out_band = 2;
    }
  }
  inf->bands = 3;
  dst->bands = 1;
  inf->format[0]      = src->format[0];
  inf->format[1]      = src->format[1];
  inf->format[2]      = src->format[2];
  dst->format[0]      = src->format[pvt->out_band];
  dst->format[0].band = 0;
  
  return( TRUE );
}                               /* end PrepBandExt */

/* end module pbande.c */
