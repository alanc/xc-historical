/* $XConsortium: meroi.c,v 1.2 93/11/06 15:27:28 rws Exp $ */
/**** module meroi.c ****/
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
  
	meroi.c -- DDXIE export roi element
  
	Dean Verheiden -- AGE Logic, Inc. August, 1993
  
*****************************************************************************/

#define _XIEC_MEROI

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
#include <error.h>
#include <macro.h>
#include <element.h>
#include <texstr.h>


/*
 *  routines referenced by other DDXIE modules
 */
int	miAnalyzeEROI();

/*
 *  routines used internal to this module
 */
static int CreateEROI();
static int InitializeEROI();
static int ResetEROI();
static int DestroyEROI();

/*
 * DDXIE ImportROI entry points
 */
static ddElemVecRec meROIVec =
{
	CreateEROI,
	InitializeEROI,
	(xieIntProc)NULL,
	(xieIntProc)NULL,
	ResetEROI,
	DestroyEROI
};


/*------------------------------------------------------------------------
------------------- see if we can handle this element --------------------
------------------------------------------------------------------------*/
int miAnalyzeEROI(flo,ped)
	floDefPtr flo;
	peDefPtr  ped;
{
	/* for now just stash our entry point vector in the peDef */
	ped->ddVec = meROIVec;
	return TRUE ;
}                               /* end miAnalyzeEROI */


/*------------------------------------------------------------------------
---------------------------- create peTex . . . --------------------------
------------------------------------------------------------------------*/
static int CreateEROI(flo,ped)
	floDefPtr flo;
	peDefPtr  ped;
{
	/* attach an execution context to the roi element definition */
	return MakePETex(flo,ped,NO_PRIVATE,NO_SYNC,NO_SYNC);
}                               /* end CreateEROI */


/*------------------------------------------------------------------------
---------------------------- initialize peTex . . . ----------------------
------------------------------------------------------------------------*/
static int InitializeEROI(flo,ped)
	floDefPtr flo;
	peDefPtr  ped;
{
 /* Allows data manager to bypass element entirely */
  return(InitReceptor(flo, ped, ped->peTex->receptor, NO_DATAMAP, 1, NO_BANDS, 
								ALL_BANDS));

}                               /* end InitializeEROI */


/*------------------------------------------------------------------------
------------------------ get rid of run-time stuff -----------------------
------------------------------------------------------------------------*/
static int ResetEROI(flo,ped)
	floDefPtr flo;
	peDefPtr  ped;
{
	ResetReceptors(ped);
	return TRUE;
}                               /* end ResetEROI */


/*------------------------------------------------------------------------
-------------------------- get rid of this element -----------------------
------------------------------------------------------------------------*/
static int DestroyEROI(flo,ped)
	floDefPtr flo;
	peDefPtr  ped;
{
	/* get rid of the peTex structure  */
	ped->peTex = (peTexPtr) XieFree(ped->peTex);

	/* zap this element's entry point vector */
	ped->ddVec.create = (xieIntProc) NULL;
	ped->ddVec.initialize = (xieIntProc) NULL;
	ped->ddVec.reset = (xieIntProc) NULL;
	ped->ddVec.destroy = (xieIntProc) NULL;
	return TRUE;
}                               /* end DestroyEROI */

/* end module meroi.c */
