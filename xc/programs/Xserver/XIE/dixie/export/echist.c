/* $XConsortium$ */
/**** module echist.c ****/
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
  
	echist.c -- DIXIE routines for managing the ExportClientHistogram elem.
  
	Dean Verheiden -- AGE Logic, Inc. July 1993
  
*****************************************************************************/

#define _XIEC_ECHIST

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
#include <error.h>


/*
 *  routines referenced by other modules.
 */
peDefPtr	MakeECHistogram();

/*
 *  routines internal to this module
 */
static Bool	PrepECHistogram();

/*
 * dixie entry points
 */
static diElemVecRec eHistogramVec = {
    PrepECHistogram			/* prepare for analysis and execution	*/
    };


/*------------------------------------------------------------------------
-------------- routine: make an ExportClientHistogram element ------------
------------------------------------------------------------------------*/
peDefPtr MakeECHistogram(flo,tag,pe)
floDefPtr      flo;
xieTypPhototag tag;
xieFlo        *pe;
{
    int inputs;
    peDefPtr ped;
    inFloPtr inFlo;
    ELEMENT(xieFloExportClientHistogram);
    ELEMENT_SIZE_MATCH(xieFloExportClientHistogram);
    ELEMENT_NEEDS_1_INPUT(src);

    inputs = 1 + (stuff->domainPhototag ? 1 : 0);

    if(!(ped = MakePEDef(inputs, (CARD32)stuff->elemLength<<2, 0))) 
	FloAllocError(flo,tag,xieElemExportClientHistogram, return(NULL));

    ped->diVec	       = &eHistogramVec;
    ped->phototag      = tag;
    ped->flags.export  = TRUE;
    ped->flags.getData = TRUE;
    raw = (xieFloExportClientHistogram *)ped->elemRaw;
    /*
     * copy the standard client element parameters (swap if necessary)
     */
    if (flo->reqClient->swapped) {
    	raw->elemType   = stuff->elemType;
    	raw->elemLength = stuff->elemLength;
    	cpswaps(stuff->src, raw->src);
	raw->notify = stuff->notify;
	cpswapl(stuff->domainOffsetX, raw->domainOffsetX);
	cpswapl(stuff->domainOffsetY, raw->domainOffsetY);
	cpswaps(stuff->domainPhototag,raw->domainPhototag);
    } else	
    	memcpy((char *)raw, (char *)stuff, sizeof(xieFloExportClientHistogram));
    /*
     * assign phototags to inFlos
     */
    inFlo = ped->inFloLst;
    inFlo[SRCtag].srcTag = raw->src;
    if(raw->domainPhototag)
	inFlo[ped->inCnt-1].srcTag = raw->domainPhototag;

    return(ped);
}                                  /* end MakeECHistogram */


/*------------------------------------------------------------------------
---------------- routine: prepare for analysis and execution -------------
------------------------------------------------------------------------*/
static Bool PrepECHistogram(flo,ped)
     floDefPtr  flo;
     peDefPtr   ped;
{
  xieFloExportClientHistogram *raw = 
    			(xieFloExportClientHistogram *)ped->elemRaw;
  inFloPtr ind, inf = &ped->inFloLst[SRCtag];
  outFloPtr     src = &inf->srcDef->outFlo;
  outFloPtr  outflo = &ped->outFlo;

  /* check notify */
  if(raw->notify != xieValDisable   &&		  
     raw->notify != xieValFirstData &&
     raw->notify != xieValNewData)
    ValueError(flo,ped,raw->notify, return(FALSE));

  /* check out process domain */
  if(raw->domainPhototag) {
    outFloPtr dom;

    ind = &ped->inFloLst[ped->inCnt-1];
    dom = &ind->srcDef->outFlo;
    if((ind->bands = dom->bands) != 1 || IsntDomain(dom->format[0].class)) 
	DomainError(flo,ped,raw->domainPhototag, return(FALSE));
    ind->format[0] = dom->format[0];
  }

  /* check and propagate src */
  if (src->bands != 1 || IsntConstrained(src->format[0].class))
	MatchError(flo,ped,return(FALSE));
  inf->bands = 1;
  inf->format[0] = src->format[0];
  
  /* Don't really have an outflo */
  outflo->bands = 1;
  outflo->format[0] = inf->format[0];
  outflo->format[0].class  = STREAM;
  ped->swapUnits[0] = sizeof(xieTypHistogramData);

  return(TRUE);
}                                  /* end PrepECHistogram */

/* end module echist.c */
