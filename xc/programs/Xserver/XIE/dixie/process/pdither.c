/* $XConsortium$ */
/**** module pdither.c ****/
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
  
	pdither.c -- DIXIE routines for managing the Dither element
  
	Larry Hare -- AGE Logic, Inc. May 1993
  
*****************************************************************************/

#define _XIEC_PDITHER

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
 *  routines referenced by other modules
 */
peDefPtr	MakeDither();
Bool		CopyPDitherErrorDiffusion();
Bool		PrepPDitherErrorDiffusion();


/*
 *  routines internal to this module
 */
static Bool	PrepPDither();
static Bool	CopyPDitherStandard();
static Bool	PrepPDitherStandard();

/*
 * dixie element entry points
 */
static diElemVecRec pDitherVec = {
  PrepPDither		/* prepare for analysis and execution	*/
  };

/*------------------------------------------------------------------------
-------------------- routine: make a dither element ------------------
------------------------------------------------------------------------*/
peDefPtr MakeDither(flo,tag,pe)
     floDefPtr      flo;
     xieTypPhototag tag;
     xieFlo        *pe;
{
  peDefPtr ped;
  ELEMENT(xieFloDither);
  ELEMENT_AT_LEAST_SIZE(xieFloDither);
  ELEMENT_NEEDS_1_INPUT(src);
  
  if(!(ped = MakePEDef(1, (CARD32)stuff->elemLength<<2, 0)))
    FloAllocError(flo,tag,xieElemDither, return(NULL)) ;

  ped->diVec	     = &pDitherVec;
  ped->phototag      = tag;
  ped->flags.process = TRUE;
  raw = (xieFloDither *)ped->elemRaw;
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
    cpswaps(stuff->dither, raw->dither);
  }
  else
    bcopy((char *)stuff, (char *)raw, sizeof(xieFloDither));
  /*
   * copy technique data (if any)
   */
  if(!(ped->techVec = FindTechnique(xieValDither, raw->dither)) ||
     !(ped->techVec->copyfnc(flo, ped, &stuff[1], &raw[1], raw->lenParams)))
    TechniqueError(flo,ped,raw->dither,raw->lenParams, return(ped));

 /*
   * assign phototag to inFlo
   */
  ped->inFloLst[SRCtag].srcTag = raw->src;


  return(ped);
}                               /* end MakePDither */

/*------------------------------------------------------------------------
---------------- routine: copy routine for no param techniques -------------
------------------------------------------------------------------------*/

static
Bool CopyPDitherStandard(flo, ped, sparms, rparms, tsize) 
     floDefPtr  flo;
     peDefPtr   ped;
     void *sparms, *rparms;
     CARD16	tsize;
{
  return(tsize == 0);
}

/*------------------------------------------------------------------------
---------------- routine: copy routine for no Error Diffusion technique --
------------------------------------------------------------------------*/

Bool CopyPDitherErrorDiffusion(flo, ped, sparms, rparms, tsize) 
     floDefPtr  flo;
     peDefPtr   ped;
     void *sparms, *rparms;
     CARD16	tsize;
{
  return CopyPDitherStandard(flo, ped, sparms, rparms, tsize);
}

/*------------------------------------------------------------------------
---------------- routine: copy routine for Ordered techniques  ---------
------------------------------------------------------------------------*/


/*------------------------------------------------------------------------
---------------- routine: prep routine for no param techniques -------------
------------------------------------------------------------------------*/
static
Bool PrepPDitherStandard(flo, ped, raw, tec) 
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
---------------- routine: prep routine for Error Diffusion techniques ----
------------------------------------------------------------------------*/
Bool PrepPDitherErrorDiffusion(flo, ped, raw, tec) 
     floDefPtr  flo;
     peDefPtr   ped;
     xieFloDither *raw;
     void	*tec;
{
  return PrepPDitherStandard(flo, ped, (void *) raw, tec);
}


/*------------------------------------------------------------------------
---------------- routine: prepare for analysis and execution -------------
------------------------------------------------------------------------*/

static Bool PrepPDither(flo,ped)
     floDefPtr  flo;
     peDefPtr   ped;
{
  inFloPtr inf = &ped->inFloLst[SRCtag];
  outFloPtr src = &inf->srcDef->outFlo;
  outFloPtr dst = &ped->outFlo;
  xieFloDither *raw = (xieFloDither *)ped->elemRaw;
  int b;

  /* grab a copy of the src attributes and propagate them to our input */
  dst->bands = inf->bands = src->bands;
  for(b = 0; b < src->bands; b++) {

	if (src->format[b].class == UNCONSTRAINED)
		MatchError(flo, ped, return(FALSE));

	if (!IsConstrained(src->format[b].class))
		ImplementationError(flo, ped, return(FALSE));

	inf->format[b] = src->format[b];

	/* Copy outFlo values that are unchanged by dither */
	dst->format[b].band 		= b;
  	dst->format[b].interleaved 	= src->format[b].interleaved;
	dst->format[b].width 		= src->format[b].width;
	dst->format[b].height 		= src->format[b].height;
  }

  /* Pull in level information from the element description */ 
  if ((dst->format[0].levels = raw->level0) > src->format[0].levels)
	ValueError(flo,ped,raw->level0,return(FALSE));
  if (dst->bands > 1) {
  	if ((dst->format[1].levels = raw->level1) > src->format[1].levels)
		ValueError(flo,ped,raw->level1,return(FALSE));
	if ((dst->format[2].levels = raw->level2) > src->format[2].levels)
		ValueError(flo,ped,raw->level2,return(FALSE));
  }

  /* Set depth, class, stride, and pitch */
  if (UpdateFormatfromLevels(ped) == FALSE)
	MatchError(flo,ped, return(FALSE));

  /* Take care of any technique parameters */
  if (!(ped->techVec->prepfnc(flo, ped, raw, &raw[1])))
	TechniqueError(flo, ped, raw->dither, raw->lenParams,
		return(FALSE));

  return (TRUE);
}	

/* end module pdither.c */
