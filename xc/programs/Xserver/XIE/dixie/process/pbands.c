/* $XConsortium: pbands.c,v 1.1 93/10/26 10:02:12 rws Exp $ */
/**** module pbands.c ****/
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
     
     "Copyright 1993, 1994 by AGE Logic, Inc. and the Massachusetts
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
  
	pbands.c -- DIXIE routines for managing the band select element
  
	Robert NC Shelley -- AGE Logic, Inc. September 1993
  
*****************************************************************************/

#define _XIEC_PBANDS

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
peDefPtr	MakeBandSel();

/*
 *  routines internal to this module
 */
static Bool	PrepBandSel();

/*
 * dixie entry points
 */
static diElemVecRec pBandSelVec = {
    PrepBandSel		/* prepare for analysis and execution	*/
    };


/*------------------------------------------------------------------------
----------------------- routine: make a blend element --------------------
------------------------------------------------------------------------*/
peDefPtr MakeBandSel(flo,tag,pe)
     floDefPtr      flo;
     xieTypPhototag tag;
     xieFlo        *pe;
{
  peDefPtr  ped;
  inFloPtr  inFlo;
  ELEMENT(xieFloBandSelect);
  ELEMENT_SIZE_MATCH(xieFloBandSelect);
  ELEMENT_NEEDS_1_INPUT(src); 

  if(!(ped = MakePEDef(1, (CARD32)stuff->elemLength<<2, 0)))
    FloAllocError(flo, tag, xieElemBandSelect, return(NULL));

  ped->diVec	     = &pBandSelVec;
  ped->phototag      = tag;
  ped->flags.process = TRUE;
  raw = (xieFloBandSelect *)ped->elemRaw;
  /*
   * copy the client element parameters (swap if necessary)
   */
  if( flo->reqClient->swapped ) {
    raw->elemType   = stuff->elemType;
    raw->elemLength = stuff->elemLength;
    cpswaps(stuff->src, raw->src);
    raw->bandNumber = stuff->bandNumber;
  } else {
    memcpy((char *)raw, (char *)stuff, sizeof(xieFloBandSelect));
  }
  /* assign phototags to inFlos
   */
  inFlo = ped->inFloLst;
  inFlo[SRCtag].srcTag = raw->src;
  
  return(ped);
}                               /* end MakeBandSel */


/*------------------------------------------------------------------------
---------------- routine: prepare for analysis and execution -------------
------------------------------------------------------------------------*/
static Bool PrepBandSel(flo,ped)
     floDefPtr  flo;
     peDefPtr   ped;
{
  xieFloBandSelect  *raw = (xieFloBandSelect *)ped->elemRaw;
  inFloPtr           inf = &ped->inFloLst[SRCtag]; 
  outFloPtr          src = &inf->srcDef->outFlo;
  outFloPtr          dst = &ped->outFlo;
  
  if(src->bands != 3 || IsntCanonic(src->format[0].class))
    MatchError(flo,ped, return(FALSE));
  if(raw->bandNumber >= xieValMaxBands)
    ValueError(flo,ped,(CARD32)raw->bandNumber, return(FALSE));
  
  inf->bands = 3;
  dst->bands = 1;
  inf->format[0]      = src->format[0];
  inf->format[1]      = src->format[1];
  inf->format[2]      = src->format[2];
  dst->format[0]      = src->format[raw->bandNumber];
  dst->format[0].band = 0;
  
  return( TRUE );
}                               /* end PrepBandSel */

/* end module pbands.c */
