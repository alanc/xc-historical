/* $XConsortium$ */
/**** module pblend.c ****/
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
  
	pblend.c -- DIXIE routines for managing the Blend element
  
	Dean Verheiden -- AGE Logic, Inc. June 1993
  
*****************************************************************************/

#define _XIEC_PBLEND

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
peDefPtr	MakeBlend();

/*
 *  routines internal to this module
 */
static Bool	PrepBlend();

/*
 * dixie entry points
 */
static diElemVecRec pBlendVec = {
    PrepBlend			/* prepare for analysis and execution	*/
    };


/*------------------------------------------------------------------------
----------------------- routine: make a blend element --------------------
------------------------------------------------------------------------*/
peDefPtr MakeBlend(flo,tag,pe)
     floDefPtr      flo;
     xieTypPhototag tag;
     xieFlo        *pe;
{
  int inputs, index;
  peDefPtr ped;
  inFloPtr inFlo;
  pBlendDefPtr pvt;
  ELEMENT(xieFloBlend);
  ELEMENT_SIZE_MATCH(xieFloBlend);
  ELEMENT_NEEDS_1_INPUT(src1); /* Other input either src2 or constant */
  inputs = 1 + (stuff->src2 ? 1 : 0)
             + (stuff->alpha ? 1 : 0)
             + (stuff->domainPhototag ? 1 : 0);

  if(!(ped = MakePEDef(inputs, (CARD32)stuff->elemLength<<2,
		       sizeof(pBlendDefRec))))
    FloAllocError(flo, tag, xieElemBlend, return(NULL));

  ped->diVec	     = &pBlendVec;
  ped->phototag      = tag;
  ped->flags.process = TRUE;
  raw = (xieFloBlend *)ped->elemRaw;
  /*
   * copy the client element parameters (swap if necessary)
   */
  if( flo->client->swapped ) {
    raw->elemType   = stuff->elemType;
    raw->elemLength = stuff->elemLength;
    cpswaps(stuff->src1, raw->src1);
    cpswaps(stuff->src2, raw->src2);
    cpswaps(stuff->alpha, raw->alpha);
    cpswapl(stuff->domainOffsetX, raw->domainOffsetX);
    cpswapl(stuff->domainOffsetY, raw->domainOffsetY);
    cpswaps(stuff->domainPhototag,raw->domainPhototag);
    raw->bandMask = stuff->bandMask;
    cpswapl(stuff->constant0, raw->constant0);
    cpswapl(stuff->constant1, raw->constant1);
    cpswapl(stuff->constant2, raw->constant2);
    cpswapl(stuff->alphaConst, raw->alphaConst);
  }
  else
    bcopy((char *)stuff, (char *)raw, sizeof(xieFloBlend));

  /*
   * convert constants
   */
  pvt = (pBlendDefPtr)ped->elemPvt;
  if(!raw->src2) {
	pvt->constant[0] = ConvertFromIEEE(raw->constant0);
	pvt->constant[1] = ConvertFromIEEE(raw->constant1);
	pvt->constant[2] = ConvertFromIEEE(raw->constant2);
	index = SRCt1;
  } else
	index = SRCt2;

  pvt->alphaConst = ConvertFromIEEE(raw->alphaConst);

  pvt->aindex = (raw->alpha) ? ++index : 0;
  pvt->dindex = (raw->domainPhototag) ? ++index : 0;

 if ( raw->alpha && pvt->alphaConst == 0.0 || 
     !raw->alpha && (pvt->alphaConst < 0.0 || pvt->alphaConst > 1.0))
    		ValueError(flo, ped, raw->alphaConst, return(NULL));
  
  /*
   * assign phototags to inFlos
   */
  inFlo = ped->inFloLst;
  inFlo[SRCt1].srcTag = raw->src1;
  if(raw->src2) inFlo[SRCt2].srcTag = raw->src2;
  if(raw->alpha) inFlo[pvt->aindex].srcTag = raw->alpha;
  if(raw->domainPhototag) inFlo[pvt->dindex].srcTag = raw->domainPhototag;
  
  return(ped);
}                               /* end MakeBlend */


/*------------------------------------------------------------------------
---------------- routine: prepare for analysis and execution -------------
------------------------------------------------------------------------*/
static Bool PrepBlend(flo,ped)
     floDefPtr  flo;
     peDefPtr   ped;
{
  xieFloBlend *raw = (xieFloBlend *)ped->elemRaw;
  pBlendDefPtr pvt = (pBlendDefPtr)ped->elemPvt;
  inFloPtr  ind, ina, in2, in1 = &ped->inFloLst[SRCt1];
  outFloPtr dom, sra, sr2, sr1 = &in1->srcDef->outFlo;
  outFloPtr dst = &ped->outFlo;
  int b;
  int aindex = pvt->aindex, dindex = pvt->dindex;

  /* check out our second source */
  if(raw->src2) {
    in2 = &ped->inFloLst[SRCt2];
    sr2 = &in2->srcDef->outFlo;
    if(sr1->bands != sr2->bands) {
      MatchError(flo,ped, return(FALSE));
    }
    in2->bands = sr2->bands;
  } else 
    sr2 = NULL;


  /* check out our alpha plane */
  if(aindex) {
    ina = &ped->inFloLst[aindex];
    sra = &ina->srcDef->outFlo;
    if((ina->bands = sra->bands) != 1 || sra->format[0].class == UNCONSTRAINED)
      MatchError(flo,ped, return(FALSE));
    ina->format[0] = sra->format[0];
  } 

  /* check out our process domain */
  if(dindex) {
    ind = &ped->inFloLst[dindex];
    dom = &ind->srcDef->outFlo;
    if((ind->bands = dom->bands) != 1)
      DomainError(flo,ped,raw->domainPhototag, return(FALSE));
    ind->format[0] = dom->format[0];
  } 

  /* grab a copy of the input attributes and propagate them to our output */
  dst->bands = in1->bands = sr1->bands;
  for(b = 0; b < dst->bands; b++) {
    dst->format[b] = in1->format[b] = sr1->format[b];
    if(sr2)
      in2->format[b] = sr2->format[b];
  }

  return( TRUE );
}                               /* end PrepBlend */

/* end module pblend.c */
