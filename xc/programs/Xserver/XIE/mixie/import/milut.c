/* $XConsortium: milut.c,v 1.2 93/11/06 15:33:42 rws Exp $ */
/**** module milut.c ****/
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
  
	milut.c -- DDXIE prototype import client lut element
  
	Larry Hare -- AGE Logic, Inc. June, 1993
  
*****************************************************************************/

#define _XIEC_ILUT
#define _XIEC_MILUT

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
#include <lut.h>
#include <element.h>
#include <texstr.h>

/*
 *  routines referenced by other DDXIE modules
 */
int	miAnalyzeILUT();

/*
 *  routines used internal to this module
 */
static int CreateILUT();
static int InitializeILUT();
static int ActivateILUT();
static int ResetILUT();
static int DestroyILUT();

/*
 * DDXIE ImportLUT entry points
 */
static ddElemVecRec miLUTVec = {
  CreateILUT,
  InitializeILUT,
  ActivateILUT,
  (xieIntProc)NULL,
  ResetILUT,
  DestroyILUT
  };


/*------------------------------------------------------------------------
------------------- see if we can handle this element --------------------
------------------------------------------------------------------------*/
int miAnalyzeILUT(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  /* for now just stash our entry point vector in the peDef */
  ped->ddVec = miLUTVec;

  return(TRUE);
}                               /* end miAnalyzeILUT */


/*------------------------------------------------------------------------
---------------------------- create peTex . . . --------------------------
------------------------------------------------------------------------*/
static int CreateILUT(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  /* attach an execution context to the import lut element definition */
  return(MakePETex(flo, ped, NO_PRIVATE, NO_SYNC, NO_SYNC));
}                               /* end CreateILUT */


/*------------------------------------------------------------------------
---------------------------- initialize peTex . . . ----------------------
------------------------------------------------------------------------*/
static int InitializeILUT(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  /* note: ImportResource elements don't use their receptors */
  return(InitEmitter(flo, ped, NO_DATAMAP, NO_INPLACE));
}                               /* end InitializeILUT */


/*------------------------------------------------------------------------
----------------------------- crank some data ----------------------------
------------------------------------------------------------------------*/
static int ActivateILUT(flo,ped,pet)
     floDefPtr flo;
     peDefPtr  ped;
     peTexPtr  pet;
{
  lutPtr  lut = ((iLUTDefPtr)ped->elemPvt)->lut;
  bandPtr bnd = pet->emitter;
  CARD32  b;

  for(b = 0; b < lut->lutCnt; ++bnd, ++b)
    if(!bnd->final) {

      if(ListEmpty(&lut->strips[b]))
	AccessError(flo,ped, return(FALSE));

      if(!PassStrip(flo,pet,bnd,lut->strips[b].flink))
	return(FALSE);	/* alloc error */
    }
  return(TRUE);
}                               /* end ActivateILUT */


/*------------------------------------------------------------------------
------------------------ get rid of run-time stuff -----------------------
------------------------------------------------------------------------*/
static int ResetILUT(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  ResetEmitter(ped);
  
  return(TRUE);
}                               /* end ResetILUT */


/*------------------------------------------------------------------------
-------------------------- get rid of this element -----------------------
------------------------------------------------------------------------*/
static int DestroyILUT(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  /* get rid of the peTex structure  */
  ped->peTex = (peTexPtr) XieFree(ped->peTex);

  /* zap this element's entry point vector */
  ped->ddVec.create     = (xieIntProc) NULL;
  ped->ddVec.initialize = (xieIntProc) NULL;
  ped->ddVec.activate   = (xieIntProc) NULL;
  ped->ddVec.flush      = (xieIntProc) NULL;
  ped->ddVec.reset      = (xieIntProc) NULL;
  ped->ddVec.destroy    = (xieIntProc) NULL;

  return(TRUE);
}                               /* end DestroyILUT */

/* end module milut.c */

