/* $XConsortium: iroi.c,v 1.3 94/01/12 19:49:34 rws Exp $ */
/**** module iroi.c ****/
/******************************************************************************

Copyright (c) 1993, 1994  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.


				NOTICE
                              
This software is being provided by AGE Logic, Inc. under the
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
     
     "Copyright 1993, 1994 by AGE Logic, Inc."
     
     THIS SOFTWARE IS PROVIDED "AS IS".  AGE LOGIC MAKES NO
     REPRESENTATIONS OR WARRANTIES, EXPRESS OR IMPLIED.  By way of
     example, but not limitation, AGE LOGIC MAKE NO
     REPRESENTATIONS OR WARRANTIES OF MERCHANTABILITY OR FITNESS
     FOR ANY PARTICULAR PURPOSE OR THAT THE SOFTWARE DOES NOT
     INFRINGE THIRD-PARTY PROPRIETARY RIGHTS.  AGE LOGIC 
     SHALL BEAR NO LIABILITY FOR ANY USE OF THIS SOFTWARE.  IN NO
     EVENT SHALL EITHER PARTY BE LIABLE FOR ANY INDIRECT,
     INCIDENTAL, SPECIAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOSS
     OF PROFITS, REVENUE, DATA OR USE, INCURRED BY EITHER PARTY OR
     ANY THIRD PARTY, WHETHER IN AN ACTION IN CONTRACT OR TORT OR
     BASED ON A WARRANTY, EVEN IF AGE LOGIC LICENSEES
     HEREUNDER HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH
     DAMAGES.
    
     The name of AGE Logic, Inc. may not be used in
     advertising or publicity pertaining to this software without
     specific, written prior permission from AGE Logic.

     Title to this software shall at all times remain with AGE
     Logic, Inc.
*****************************************************************************
  
	iroi.c -- DIXIE routines for managing the ImportROI element
  
	Robert NC Shelley  -- AGE Logic, Inc. April 1993
  
*****************************************************************************/

#define _XIEC_IROI

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
#include <flodata.h>
#include <roi.h>
#include <element.h>
#include <error.h>


/*
 *  routines referenced by other modules.
 */
peDefPtr	MakeIROI();

/*
 *  routines internal to this module
 */
static Bool	PrepIROI();
static Bool    DebriefIROI();

/*
 * dixie entry points
 */
static diElemVecRec iROIVec =
{
	PrepIROI,		/* prepare for analysis and execution	*/
	DebriefIROI		/* debrief */
};

/*------------------------------------------------------------------------
----------------- routine: make an ImportROI element ----------------
------------------------------------------------------------------------*/
peDefPtr MakeIROI(flo,tag,pe)
     floDefPtr      flo;
     xieTypPhototag tag;
     xieFlo        *pe;
{
  peDefPtr ped;
  inFloPtr inFlo;
  ELEMENT(xieFloImportROI);
  ELEMENT_SIZE_MATCH(xieFloImportROI);
  
  if(!(ped = MakePEDef(1, (CARD32)stuff->elemLength<<2, sizeof(iROIDefRec))))
      FloAllocError(flo,tag,xieElemImportROI, return(NULL));
  
  ped->diVec	    = &iROIVec;
  ped->phototag     = tag;
  ped->flags.import = TRUE;
  raw = (xieFloImportROI *)ped->elemRaw;
  /*
   * copy the standard client element parameters (swap if necessary)
   */
  if (flo->reqClient->swapped) {
      raw->elemType   = stuff->elemType;
      raw->elemLength = stuff->elemLength;
      cpswapl(stuff->roi, raw->roi);
  } else  
      memcpy((char *)raw, (char *)stuff, sizeof(xieFloImportROI));
  /*
   * assign phototags to inFlos
   */
  inFlo = ped->inFloLst;

  return ped;
}                               /* end MakeIROI */


/*------------------------------------------------------------------------
---------------- routine: prepare for analysis and execution -------------
------------------------------------------------------------------------*/
static Bool PrepIROI(flo,ped)
     floDefPtr  flo;
     peDefPtr   ped;
{
  xieFloImportROI *raw = (xieFloImportROI *)ped->elemRaw;
  iROIDefPtr pvt = (iROIDefPtr) ped->elemPvt;
  inFloPtr inf   = &ped->inFloLst[IMPORT];
  outFloPtr dst  = &ped->outFlo;
  roiPtr roi;
  
  /* grab roi resource */
  if (!(roi = (roiPtr)LookupIDByType(raw->roi, RT_ROI)))
      ROIError(flo,ped,raw->roi,return(FALSE));
 
  /* Make sure the roi has been populated */
  if (ListEmpty(&roi->strips))
      AccessError(flo,ped, return(FALSE));

  roi->refCnt++;
  pvt->roi = roi;
  
  dst->bands = inf->bands = 1;
  dst->format[0].class = inf->format[0].class = RUN_LENGTH;
  
  return TRUE;
}                               /* end PrepIROI */

/*------------------------------------------------------------------------
---------------------- routine: post execution cleanup -------------------
------------------------------------------------------------------------*/
static Bool DebriefIROI(flo,ped,ok)
     floDefPtr  flo;
     peDefPtr   ped;
     Bool	ok;
{
  xieFloImportROI *raw = (xieFloImportROI *)ped->elemRaw;
  iROIDefPtr pvt = (iROIDefPtr)ped->elemPvt;
  roiPtr roi;

  if(pvt && (roi = pvt->roi))
    if(pvt->roi->refCnt > 1)
      --roi->refCnt;
    else if(LookupIDByType(raw->roi, RT_ROI))
      FreeResourceByType(pvt->roi->ID, RT_ROI, RT_NONE);
    else
      DeleteROI(roi, roi->ID);

  return TRUE;
}			                         /* end DebriefIROI */

/* end module iroi.c */
