/* $XConsortium$ */
/**** module pbandc.c ****/
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
  
	pbandc.c -- DIXIE routines for managing the band combine element
  
	Dean Verheiden -- AGE Logic, Inc. July 1993
  
*****************************************************************************/

#define _XIEC_PBANDC

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


/*
 *  routines referenced by other modules.
 */
peDefPtr	MakeBandCom();

/*
 *  routines internal to this module
 */
static Bool	PrepBandCom();

/*
 * dixie entry points
 */
static diElemVecRec pBandComVec = {
    PrepBandCom		/* prepare for analysis and execution	*/
    };


/*------------------------------------------------------------------------
----------------------- routine: make a blend element --------------------
------------------------------------------------------------------------*/
peDefPtr MakeBandCom(flo,tag,pe)
     floDefPtr      flo;
     xieTypPhototag tag;
     xieFlo        *pe;
{
  int inputs, index;
  peDefPtr ped;
  inFloPtr inFlo;
  ELEMENT(xieFloBandCombine);
  ELEMENT_SIZE_MATCH(xieFloBandCombine);
  ELEMENT_NEEDS_3_INPUTS(src1,src2,src3); 

  if(!(ped = MakePEDef(3, (CARD32)stuff->elemLength<<2, 0)))
    FloAllocError(flo, tag, xieElemBandCombine, return(NULL));

  ped->diVec	     = &pBandComVec;
  ped->phototag      = tag;
  ped->flags.process = TRUE;
  raw = (xieFloBandCombine *)ped->elemRaw;
  /*
   * copy the client element parameters (swap if necessary)
   */
  if( flo->client->swapped ) {
    raw->elemType   = stuff->elemType;
    raw->elemLength = stuff->elemLength;
    cpswaps(stuff->src1, raw->src1);
    cpswaps(stuff->src2, raw->src2);
    cpswaps(stuff->src3, raw->src3);
  }
  else
    bcopy((char *)stuff, (char *)raw, sizeof(xieFloBandCombine));

  
  /*
   * assign phototags to inFlos
   */
  inFlo = ped->inFloLst;
  inFlo[SRCt1].srcTag = raw->src1;
  inFlo[SRCt2].srcTag = raw->src2;
  inFlo[SRCt3].srcTag = raw->src3;
  
  return(ped);
}                               /* end MakeBandCom */


/*------------------------------------------------------------------------
---------------- routine: prepare for analysis and execution -------------
------------------------------------------------------------------------*/
static Bool PrepBandCom(flo,ped)
     floDefPtr  flo;
     peDefPtr   ped;
{
  xieFloBandCombine *raw = (xieFloBandCombine *)ped->elemRaw;
  inFloPtr  in1 = &ped->inFloLst[SRCt1]; 
  inFloPtr  in2 = &ped->inFloLst[SRCt2]; 
  inFloPtr  in3 = &ped->inFloLst[SRCt3]; 
  inFloPtr  inf = in1;
  outFloPtr sr1 = &in1->srcDef->outFlo;
  outFloPtr sr2 = &in2->srcDef->outFlo;
  outFloPtr sr3 = &in3->srcDef->outFlo;
  outFloPtr dst = &ped->outFlo;
  int b;

  /* All inputs must be single band */
  if (sr1->bands != 1 || sr2->bands != 1 || sr3->bands != 1)
    MatchError(flo,ped, return(FALSE));
  
  /* All sources must be either constrained or unconstrained */
  if (IsConstrained(sr1->format[0].class)) {
    if (!(IsConstrained(sr2->format[0].class) || 
	!(IsConstrained(sr3->format[0].class))))
      MatchError(flo,ped, return(FALSE));
  } else if (sr1->format[0].class == LUT_ARRAY) {
    if ((sr2->format[0].class != LUT_ARRAY) ||
	(sr3->format[0].class != LUT_ARRAY) ||
	(sr2->format[0].width != sr1->format[0].width) ||
	(sr3->format[0].width != sr1->format[0].width))
      MatchError(flo,ped, return(FALSE));
  } else {
    if (sr1->format[0].class != UNCONSTRAINED ||
	sr2->format[0].class != UNCONSTRAINED ||
	sr3->format[0].class != UNCONSTRAINED)
      MatchError(flo,ped, return(FALSE));
  }
  /* grab a copy of the input attributes and propagate them to our output */
  in1->bands = in2->bands = in3->bands = 1;
  dst->bands = xieValMaxBands;
  for(b = 0; b < xieValMaxBands; ++inf, ++b) {
    dst->format[b] = inf->format[0] = inf->srcDef->outFlo.format[0];
    dst->format[b].band = b;
  }
  return( TRUE );
}                               /* end PrepBandCom */

/* end module pbandc.c */
