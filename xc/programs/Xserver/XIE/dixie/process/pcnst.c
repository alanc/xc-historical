/* $XConsortium$ */
/**** module pcnst.c ****/
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
  
	pcnst..c -- DIXIE routines for managing the Constrain element
  
	Dean Verheiden -- AGE Logic, Inc. May 1993
  
*****************************************************************************/

#define _XIEC_PCNST

/*
 *  Include files
 */
#include <stdio.h>

#ifndef XoftWare
#define XoftWare
#endif
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
 *  routines referenced by other modules
 */
peDefPtr	MakeConstrain();
Bool		CopyPConstrainStandard();
Bool		CopyPConstrainClipScale();
Bool		PrepPConstrainStandard();
Bool		PrepPConstrainClipScale();

/*
 *  routines internal to this module
 */
static Bool	PrepPConstrain();

/*
 * dixie element entry points
 */
static diElemVecRec pConstrainVec = {
  PrepPConstrain		/* prepare for analysis and execution	*/
  };

/*------------------------------------------------------------------------
-------------------- routine: make a constrain element ------------------
------------------------------------------------------------------------*/
peDefPtr MakeConstrain(flo,tag,pe)
     floDefPtr      flo;
     xieTypPhototag tag;
     xieFlo        *pe;
{
  peDefPtr ped;
  ELEMENT(xieFloConstrain);
  ELEMENT_AT_LEAST_SIZE(xieFloConstrain);
  ELEMENT_NEEDS_1_INPUT(src);
  
  if(!(ped = MakePEDef(1, (CARD32)stuff->elemLength<<2, 0)))
    FloAllocError(flo,tag,xieElemConstrain, return(NULL)) ;

  ped->diVec	     = &pConstrainVec;
  ped->phototag      = tag;
  ped->flags.process = TRUE;
  raw = (xieFloConstrain *)ped->elemRaw;
  /*
   * copy the client element parameters (swap if necessary)
   */
  if( flo->client->swapped ) {
    raw->elemType   = stuff->elemType;
    raw->elemLength = stuff->elemLength;
    cpswaps(stuff->src, raw->src);
    cpswapl(stuff->level0,  raw->level0);
    cpswapl(stuff->level1,  raw->level1);
    cpswapl(stuff->level2,  raw->level2);
    cpswaps(stuff->lenParams, raw->lenParams);
    cpswaps(stuff->constrain, raw->constrain);
  }
  else
    bcopy((char *)stuff, (char *)raw, sizeof(xieFloConstrain));
  /*
   * copy technique data (if any)
   */
  if(!(ped->techVec = FindTechnique(xieValConstrain, raw->constrain)) ||
     !(ped->techVec->copyfnc(flo, ped, &stuff[1], &raw[1], raw->lenParams)))
    TechniqueError(flo,ped,raw->constrain,raw->lenParams, return(ped));

 /*
   * assign phototag to inFlo
   */
  ped->inFloLst[SRCtag].srcTag = raw->src;


  return(ped);
}                               /* end MakePConstrain */

/*------------------------------------------------------------------------
---------------- routine: copy routine for no param techniques -------------
------------------------------------------------------------------------*/

Bool CopyPConstrainStandard(flo, ped, sparms, rparms, tsize) 
     floDefPtr  flo;
     peDefPtr   ped;
     void *sparms, *rparms;
     CARD16	tsize;
{
  return(tsize == 0);
}

/*------------------------------------------------------------------------
---------------- routine: copy routine for Clip-Scale technique  ---------
------------------------------------------------------------------------*/

Bool CopyPConstrainClipScale(flo, ped, sparms, rparms, tsize) 
     floDefPtr  flo;
     peDefPtr   ped;
     xieTecClipScale *sparms, *rparms;
     CARD16	tsize;
{
     pCnstDefPtr pvt;

     if (tsize != sizeof(xieTecClipScale) >> 2) return(FALSE);

     if (!(ped->techPvt = (void *)XieMalloc(sizeof(pCnstDefRec))))
	     FloAllocError(flo, ped->phototag,xieElemConstrain, return(TRUE));

     pvt = (pCnstDefPtr)ped->techPvt;

     if( flo->client->swapped ) {
	     pvt->input_low[0] = ConvertFromIEEE(lswapl(sparms->input_low0));
	     pvt->input_low[1] = ConvertFromIEEE(lswapl(sparms->input_low1));
	     pvt->input_low[2] = ConvertFromIEEE(lswapl(sparms->input_low2));
	     pvt->input_high[0] = ConvertFromIEEE(lswapl(sparms->input_high0));
	     pvt->input_high[1] = ConvertFromIEEE(lswapl(sparms->input_high1));
	     pvt->input_high[2] = ConvertFromIEEE(lswapl(sparms->input_high2));
	     cpswapl(sparms->output_low0,  pvt->output_low[0]);
	     cpswapl(sparms->output_low1,  pvt->output_low[1]);
	     cpswapl(sparms->output_low2,  pvt->output_low[2]);
	     cpswapl(sparms->output_high0, pvt->output_high[0]);
	     cpswapl(sparms->output_high1, pvt->output_high[1]);
	     cpswapl(sparms->output_high2, pvt->output_high[2]);
      } else {
	     pvt->input_low[0] = ConvertFromIEEE(sparms->input_low0);
	     pvt->input_low[1] = ConvertFromIEEE(sparms->input_low1);
	     pvt->input_low[2] = ConvertFromIEEE(sparms->input_low2);
	     pvt->input_high[0] = ConvertFromIEEE(sparms->input_high0);
	     pvt->input_high[1] = ConvertFromIEEE(sparms->input_high1);
	     pvt->input_high[2] = ConvertFromIEEE(sparms->input_high2);
	     pvt->output_low[0] = sparms->output_low0;
	     pvt->output_low[1] = sparms->output_low1;
	     pvt->output_low[2] = sparms->output_low2;
	     pvt->output_high[0] = sparms->output_high0;
	     pvt->output_high[1] = sparms->output_high1;
	     pvt->output_high[2] = sparms->output_high2;
      }

     return (TRUE);
}
/*------------------------------------------------------------------------
---------------- routine: prep routine for no param techniques -------------
------------------------------------------------------------------------*/
Bool PrepPConstrainStandard(flo, ped, raw, tec) 
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
/*------------------------------------------------------------------------
---------------- routine: prep routine for Clip Scale technique ----------
------------------------------------------------------------------------*/
Bool PrepPConstrainClipScale(flo, ped, raw, tec) 
     floDefPtr  flo;
     peDefPtr   ped;
     xieTecClipScale *raw, *tec;
{
  pCnstDefPtr pvt = ped->techPvt;

  ped->outFlo.format[0].params 	= (void *)NULL;
  if (pvt->input_low[0] == pvt->input_high[0] ||
      pvt->output_low[0] > ped->outFlo.format[0].levels - 1 ||
      pvt->output_high[0] > ped->outFlo.format[0].levels - 1)
		return(FALSE);
  if (ped->outFlo.bands > 1) {
  	if (  pvt->input_low[1] == pvt->input_high[1] ||
	      pvt->output_low[1] > ped->outFlo.format[1].levels - 1 ||
	      pvt->output_high[1] > ped->outFlo.format[1].levels - 1 ||
  	      pvt->input_low[2] == pvt->input_high[2] ||
	      pvt->output_low[2] > ped->outFlo.format[2].levels - 1 ||
	      pvt->output_high[2] > ped->outFlo.format[2].levels - 1)
		return(FALSE);
	  ped->outFlo.format[1].params 	= (void *)NULL;
	  ped->outFlo.format[2].params 	= (void *)NULL;
  }

  return(TRUE);
}

/*------------------------------------------------------------------------
---------------- routine: prepare for analysis and execution -------------
------------------------------------------------------------------------*/

static Bool PrepPConstrain(flo,ped)
     floDefPtr  flo;
     peDefPtr   ped;
{
  inFloPtr inf = &ped->inFloLst[SRCtag];
  outFloPtr src = &inf->srcDef->outFlo;
  outFloPtr dst = &ped->outFlo;
  xieFloConstrain *raw = (xieFloConstrain *)ped->elemRaw;
  int b;

  /* grab a copy of the src attributes and propagate them to our input */
  dst->bands = inf->bands = src->bands;
  for(b = 0; b < src->bands; b++) {

	/* This should be impossible */
	if (!IsCanonic(src->format[b].class))
		ImplementationError(flo, ped, return(FALSE));

	inf->format[b] = src->format[b];

	/* Copy outFlo values that are unchanged by constrain */
	dst->format[b].band 		= b;
  	dst->format[b].interleaved 	= src->format[b].interleaved;
	dst->format[b].width 		= src->format[b].width;
	dst->format[b].height 		= src->format[b].height;
  }

  /* Pull in level information from the element description */ 
  if ((dst->format[0].levels = raw->level0) > MAX_LEVELS(src->bands))
	ValueError(flo,ped,raw->level0,return(FALSE));
  if (dst->bands > 1) {
  	if ((dst->format[1].levels = raw->level1) > MAX_LEVELS(src->bands))
		ValueError(flo,ped,raw->level1,return(FALSE));
	if ((dst->format[2].levels = raw->level2) > MAX_LEVELS(src->bands))
		ValueError(flo,ped,raw->level2,return(FALSE));
  }

  /* Set depth, class, stride, and pitch */
  UpdateFormatfromLevels(ped);

  /* Take care of any technique parameters */
  if (!(ped->techVec->prepfnc(flo, ped, raw, &raw[1])))
	TechniqueError(flo, ped, raw->constrain, raw->lenParams,
		return(FALSE));


  return (TRUE);
}	

/* end module pcnst.c */
