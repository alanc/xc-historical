/* $XConsortium$ */
/**** module pconv.c ****/
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
  
	pconv.c -- DIXIE routines for managing the Convolution element
  
	Dean Verheiden -- AGE Logic, Inc. June 1993
  
*****************************************************************************/

#define _XIEC_PCONV

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
#include <element.h>
#include <technq.h>
#include <difloat.h>



/*
 *  routines referenced by other modules.
 */
peDefPtr	MakeConvolve();
Bool		CopyConvolveConstant();
Bool		CopyConvolveReplicate();
Bool		PrepConvolveStandard();

/*
 *  routines internal to this module
 */
static Bool	PrepConvolve();

/*
 * dixie entry points
 */
static diElemVecRec pConvolveVec = {
    PrepConvolve		/* prepare for analysis and execution	*/
    };


/*------------------------------------------------------------------------
----------------------- routine: make a convolution element --------------
------------------------------------------------------------------------*/
peDefPtr MakeConvolve(flo,tag,pe)
     floDefPtr      flo;
     xieTypPhototag tag;
     xieFlo        *pe;
{
  int inputs;
  peDefPtr ped;
  inFloPtr inFlo;
  double *pvt;
  xieTypFloat *kptr;
  int i, numke;
  ELEMENT(xieFloConvolve);
  ELEMENT_AT_LEAST_SIZE(xieFloConvolve);
  ELEMENT_NEEDS_1_INPUT(src);
  inputs = 1 + (stuff->domainPhototag ? 1 :0);


  numke = stuff->kernelSize * stuff->kernelSize;

  if(!(ped = MakePEDef(inputs, (CARD32)stuff->elemLength<<2,
			       numke * sizeof(double))))
    FloAllocError(flo, tag, xieElemConvolve, return(NULL));

  ped->diVec	     = &pConvolveVec;
  ped->phototag      = tag;
  ped->flags.process = TRUE;
  raw = (xieFloConvolve *)ped->elemRaw;
  /*
   * copy the client element parameters (swap if necessary)
   */
  if( flo->client->swapped ) {
    raw->elemType   = stuff->elemType;
    raw->elemLength = stuff->elemLength;
    cpswaps(stuff->src, raw->src);
    cpswapl(stuff->domainOffsetX, raw->domainOffsetX);
    cpswapl(stuff->domainOffsetY, raw->domainOffsetY);
    cpswaps(stuff->domainPhototag,raw->domainPhototag);
    raw->bandMask = stuff->bandMask;
    raw->kernelSize = stuff->kernelSize;
    cpswaps(stuff->convolve, raw->convolve);
    cpswaps(stuff->lenParams, raw->lenParams);
  }
  else
    bcopy((char *)stuff, (char *)raw, sizeof(xieFloConvolve));

  /* Copy over and convert the kernel */
  kptr = (xieTypFloat *)&stuff[1];
  pvt = (double *)ped->elemPvt;
  if (flo->client->swapped)
	  for (i = 0; i < numke; i++) {
		/* can't use *pvt++ = ConvertFromIEEE(lswapl(*kptr++)); */
		/* because lswapl is a macro, and overincrements kptr   */
		*pvt++ = ConvertFromIEEE(lswapl(*kptr));
		++kptr;
	  }
  else
	  for (i = 0; i < numke; i++) 
		*pvt++ = ConvertFromIEEE(*kptr++);
  /* 
   * Ensure that the kernel size is odd
   */
  if (!(stuff->kernelSize & 1))
    ValueError(flo,ped,(CARD32)raw->kernelSize,return(ped));
	
  /*
   * copy technique data (if any) 
   * Note that we must skip past the convolution kernel to get there
   */
  if(!(ped->techVec = FindTechnique(xieValConvolve, raw->convolve)) ||
     !(ped->techVec->copyfnc(flo, ped,  &stuff[1] + numke * 4,
					&raw[1] + numke * 4, 
					raw->lenParams))) 
    TechniqueError(flo,ped,raw->convolve,raw->lenParams, return(ped));

  /*
   * assign phototags to inFlos
   */
  inFlo = ped->inFloLst;
  inFlo[SRCtag].srcTag = raw->src;
  if(raw->domainPhototag) inFlo[ped->inCnt-1].srcTag = raw->domainPhototag;
  
  return(ped);
}                               /* end MakeConv */

/*------------------------------------------------------------------------
---------------- routine: copy routine for Constant technique  ---------
------------------------------------------------------------------------*/

Bool CopyConvolveConstant(flo, ped, sparms, rparms, tsize) 
     floDefPtr  flo;
     peDefPtr   ped;
     xieTecConvolveConstant *sparms, *rparms;
     CARD16	tsize;
{
     pTecConvolveConstantDefPtr pvt;

     if (tsize && tsize != sizeof(xieTecConvolveConstant) >> 2) return(FALSE);

     if (!(ped->techPvt=(void *)XieMalloc(sizeof(pTecConvolveConstantDefRec))))
	     FloAllocError(flo, ped->phototag, xieElemConvolve, return(TRUE));

     pvt = (pTecConvolveConstantDefPtr)ped->techPvt;

    /*
     *	Convolve Constant can be called with no parameters
     */
     if (!tsize) {
	     pvt->constant[0] = pvt->constant[1] = pvt->constant[2] = 0;
	     return(TRUE);
     }

     if( flo->client->swapped ) {
	     pvt->constant[0] = ConvertFromIEEE(lswapl(sparms->constant0));
	     pvt->constant[1] = ConvertFromIEEE(lswapl(sparms->constant1));
	     pvt->constant[2] = ConvertFromIEEE(lswapl(sparms->constant2));
      } else {
	     pvt->constant[0] = ConvertFromIEEE(sparms->constant0);
	     pvt->constant[1] = ConvertFromIEEE(sparms->constant1);
	     pvt->constant[2] = ConvertFromIEEE(sparms->constant2);
      }

     return (TRUE);
}

/*------------------------------------------------------------------------
---------------- routine: copy routine for no param techniques -------------
------------------------------------------------------------------------*/

Bool CopyConvolveReplicate(flo, ped, sparms, rparms, tsize) 
     floDefPtr  flo;
     peDefPtr   ped;
     void *sparms, *rparms;
     CARD16	tsize;
{
  return(tsize == 0);
}


/*------------------------------------------------------------------------
---------------- routine: prepare for analysis and execution -------------
------------------------------------------------------------------------*/
static Bool PrepConvolve(flo,ped)
     floDefPtr  flo;
     peDefPtr   ped;
{
  xieFloConvolve *raw = (xieFloConvolve *)ped->elemRaw;
  inFloPtr  ind, in = &ped->inFloLst[SRCtag];
  outFloPtr dom, src = &in->srcDef->outFlo;
  outFloPtr dst = &ped->outFlo;
  int b;

  /* check out our process domain */
  if(raw->domainPhototag) {
    ind = &ped->inFloLst[ped->inCnt-1];
    dom = &ind->srcDef->outFlo;
    if((ind->bands = dom->bands) != 1)
      DomainError(flo,ped,raw->domainPhototag, return(FALSE));
    ind->format[0] = dom->format[0];
  } else
    dom = NULL;

  /* grab a copy of the input attributes and propagate them to our output */
  dst->bands = in->bands = src->bands;

  for(b = 0; b < dst->bands; b++) {
	if (!IsCanonic(src->format[b].class))
		MatchError(flo, ped, return(FALSE));
	dst->format[b] = in->format[b] = src->format[b];
  }

  if(!(ped->techVec->prepfnc(flo, ped, raw, &raw[1] + 
		raw->kernelSize * raw->kernelSize * 4)))
    TechniqueError(flo, ped, raw->convolve, raw->lenParams,
		   return(FALSE));

  return( TRUE );
}                               /* end PrepConvolve */

/*------------------------------------------------------------------------
---------------- routine: prep routine for no param techniques -----------
------------------------------------------------------------------------*/
Bool PrepConvolveStandard(flo, ped, raw, tec) 
     floDefPtr  flo;
     peDefPtr   ped;
     void *raw, *tec;
{
  ped->outFlo.format[0].params 	= (void *)NULL;
  if (ped->outFlo.bands > 1) {
	  ped->outFlo.format[1].params 	= (void *)NULL;
	  ped->outFlo.format[2].params 	= (void *)NULL;
  }
  return(TRUE);
}
/* end module pconv.c */
