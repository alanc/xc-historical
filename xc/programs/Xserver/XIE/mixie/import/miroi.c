/* $XConsortium: miroi.c,v 1.2 93/10/31 09:45:14 dpw Exp $ */
/**** module miroi.c ****/
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
  
	miroi.c -- DDXIE prototype import client roi element
  
	Robert NC Shelley -- AGE Logic, Inc. April, 1993
  
*****************************************************************************/

#define _XIEC_MIROI
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
#include <extnsionst.h>
/*
 *  Server XIE Includes
 */
#include <error.h>
#include <macro.h>
#include <roi.h>
#include <element.h>
#include <texstr.h>

/*
 *  routines referenced by other DDXIE modules
 */
int	miAnalyzeIROI();

/*
 *  routines used internal to this module
 */
static int CreateIROI();
static int InitializeIROI();
static int ActivateIROI();
static int FlushIROI();
static int ResetIROI();
static int DestroyIROI();

/*
 * DDXIE ImportROI entry points
 */
static ddElemVecRec miROIVec = {
  CreateIROI,
  InitializeIROI,
  ActivateIROI,
  FlushIROI,
  ResetIROI,
  DestroyIROI
  };

/*
 *  Private stuff. peTex extension for the ImportClientROI element.
 */
typedef struct _miroidef {
  stripPtr    next_strip;
}
miROIDefRec, *miROIDefPtr;

/*------------------------------------------------------------------------
------------------- see if we can handle this element --------------------
------------------------------------------------------------------------*/
int miAnalyzeIROI(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  /* for now just stash our entry point vector in the peDef */
  ped->ddVec = miROIVec;
  return TRUE ;
}                               /* end miAnalyzeIROI */


/*------------------------------------------------------------------------
---------------------------- create peTex . . . --------------------------
------------------------------------------------------------------------*/
static int CreateIROI(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  /* attach an execution context to the roi element definition */
  return MakePETex(flo,ped,sizeof(miROIDefRec),NO_SYNC,NO_SYNC);
}                               /* end CreateIROI */


/*------------------------------------------------------------------------
---------------------------- initialize peTex . . . ----------------------
------------------------------------------------------------------------*/
static int InitializeIROI(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  roiPtr  roi = ((iROIDefPtr)ped->elemPvt)->roi;
  miROIDefPtr  pvt = (miROIDefPtr)ped->peTex->private;
  
  pvt->next_strip = roi->strips.flink;
  
  return InitEmitter(flo,ped,NO_DATAMAP,NO_INPLACE);
}                               /* end InitializeIROI */


/*------------------------------------------------------------------------
----------------------------- crank some data ----------------------------
------------------------------------------------------------------------*/
static int ActivateIROI(flo,ped,pet)
     floDefPtr flo;
     peDefPtr  ped;
     peTexPtr  pet;
{
  miROIDefPtr	pvt = (miROIDefPtr)pet->private;
  bandPtr		bnd = &pet->emitter[0];
  
  /* pass a clone of the current/only strip to our recipients */
  
  if(!PassStrip(flo,pet,bnd,pvt->next_strip))
    return FALSE;	/* alloc error */
  
  return TRUE;
}                               /* end ActivateIROI */


/*------------------------------------------------------------------------
--------------------------- get rid of left overs ------------------------
------------------------------------------------------------------------*/
static int FlushIROI(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  return TRUE;
}                               /* end FlushIROI */


/*------------------------------------------------------------------------
------------------------ get rid of run-time stuff -----------------------
------------------------------------------------------------------------*/
static int ResetIROI(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  ResetEmitter(ped);
  return TRUE;
}                               /* end ResetIROI */


/*------------------------------------------------------------------------
-------------------------- get rid of this element -----------------------
------------------------------------------------------------------------*/
static int DestroyIROI(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  /* get rid of the peTex structure  */
  ped->peTex = (peTexPtr) XieFree(ped->peTex);
  
  /* zap this element's entry point vector */
  ped->ddVec.create = (xieIntProc) NULL;
  ped->ddVec.initialize = (xieIntProc) NULL;
  ped->ddVec.activate = (xieIntProc) NULL;
  ped->ddVec.flush = (xieIntProc) NULL;
  ped->ddVec.reset = (xieIntProc) NULL;
  ped->ddVec.destroy = (xieIntProc) NULL;
  return TRUE;
}                               /* end DestroyIROI */

/* end module miroi.c */

