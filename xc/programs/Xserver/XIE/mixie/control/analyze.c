/* $XConsortium$ */
/**** module analyze.c ****/
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
  
	analyze.c -- DDXIE prototype (simple minded) DAG analyzer
  
	Robert NC Shelley -- AGE Logic, Inc. April, 1993
  
*****************************************************************************/

#define _XIEC_ANALYZE

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
#include <extnsionst.h>
/*
 *  Server XIE Includes
 */
#include <error.h>
#include <macro.h>
#include <flostr.h>
#include <texstr.h>


/*
 *  routines called from DIXIE
 */
int DAGalyze();


/*------------------------------------------------------------------------
----------------------- analyze (sort of) the DAG ------------------------
------------------------------------------------------------------------*/
int DAGalyze(flo)
     floDefPtr flo;
{
  int ok = TRUE;
  peDefPtr ped;
  pedLstPtr lst = ListEmpty(&flo->optDAG) ? &flo->defDAG : &flo->optDAG;
  
  /* establish our default flo manager
   */
  InitFloManager(flo);

  /* choose element handlers
   */
  for(ped = lst->flink; ok && !ListEnd(ped,lst); ped = ped->flink)
    switch(ped->elemRaw->elemType) {
      case xieElemImportClientLUT:   ok = miAnalyzeICLUT(flo,ped);       break;
      case xieElemImportClientPhoto: ok = miAnalyzeICPhoto(flo,ped);     break;
      case xieElemImportClientROI:   ok = miAnalyzeICROI(flo,ped);       break;
      case xieElemImportDrawable:    ok = miAnalyzeIDraw(flo,ped);       break;
      case xieElemImportDrawablePlane:ok = miAnalyzeIDrawP(flo,ped);     break;
      case xieElemImportLUT:         ok = miAnalyzeILUT(flo,ped);        break;
      case xieElemImportPhotomap:    ok = miAnalyzeIPhoto(flo,ped);      break;
      case xieElemImportROI:         ok = miAnalyzeIROI(flo,ped);        break;
      case xieElemBlend:	     ok = miAnalyzeBlend(flo,ped);       break;
      case xieElemConstrain:	     ok = miAnalyzeConstrain(flo,ped);   break;
      case xieElemConvertFromIndex:  ok = miAnalyzeCvtFromInd(flo,ped);  break;
      case xieElemConvertToIndex:    ok = miAnalyzeCvtToInd(flo,ped);    break;
      case xieElemDither:	     ok = miAnalyzeDither(flo,ped);      break;
      case xieElemGeometry:	     ok = miAnalyzeGeometry(flo,ped);    break;
      case xieElemLogical:	     ok = miAnalyzeLogic(flo,ped);       break;
      case xieElemPasteUp:           ok = miAnalyzePasteUp(flo,ped);     break;
      case xieElemPoint:	     ok = miAnalyzePoint(flo,ped);       break;
      case xieElemUnconstrain:       ok = miAnalyzeUnconstrain(flo,ped); break;
      case xieElemExportClientLUT:   ok = miAnalyzeECLUT(flo,ped);       break;
      case xieElemExportClientPhoto: ok = miAnalyzeECPhoto(flo,ped);     break;
      case xieElemExportClientROI:   ok = miAnalyzeECROI(flo,ped);       break;
      case xieElemExportDrawable:    ok = miAnalyzeEDraw(flo,ped);       break;
      case xieElemExportDrawablePlane:ok = miAnalyzeEDrawP(flo,ped);     break;
      case xieElemExportLUT:         ok = miAnalyzeELUT(flo,ped);        break;
      case xieElemExportPhotomap:    ok = miAnalyzeEPhoto(flo,ped);      break;
      case xieElemExportROI:         ok = miAnalyzeEROI(flo,ped);        break;
      default: ImplementationError(flo,ped, return(FALSE));
    }
  return(ok);
}                               /* end DAGalyze */

/* end module analyze.c */
