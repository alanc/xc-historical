/* $XConsortium$ */
/**** module melut.c ****/
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
  
	melut.c -- DDXIE prototype export lut element
  
	Larry Hare -- AGE Logic, Inc. June, 1993
  
*****************************************************************************/

#define _XIEC_MELUT
#define _XIEC_ELUT

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
#include <pixmapstr.h>
#include <gcstruct.h>
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
int	miAnalyzeELUT();

/*
 *  routines used internal to this module
 */
static int CreateELUT();
static int InitializeELUT();
static int ActivateELUT();
static int FlushELUT();
static int ResetELUT();
static int DestroyELUT();

/*
 * DDXIE ExportLUT entry points
 */
static ddElemVecRec ELUTVec = {
  CreateELUT,
  InitializeELUT,
  ActivateELUT,
  FlushELUT,
  ResetELUT,
  DestroyELUT
  };


/*------------------------------------------------------------------------
------------------- see if we can handle this element --------------------
------------------------------------------------------------------------*/
int miAnalyzeELUT(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  /* for now just stash our entry point vector in the peDef */
  ped->ddVec = ELUTVec;

  return(TRUE);
}                               /* end miAnalyzeELUT */

/*------------------------------------------------------------------------
---------------------------- create peTex . . . --------------------------
------------------------------------------------------------------------*/
static int CreateELUT(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  /* attach an execution context to the lut element definition */
  return(MakePETex(flo, ped, 0, FALSE, FALSE));
}                               /* end CreateELUT */

/*------------------------------------------------------------------------
---------------------------- initialize peTex . . . ----------------------
------------------------------------------------------------------------*/
static int InitializeELUT(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  return(InitReceptors(flo,ped,0,1) && InitEmitter(flo,ped,0,-1));
}                               /* end InitializeELUT */

/*------------------------------------------------------------------------
----------------------------- crank some data ----------------------------
------------------------------------------------------------------------*/
static int ActivateELUT(flo,ped,pet)
     floDefPtr flo;
     peDefPtr  ped;
     peTexPtr  pet;
{
  lutPtr  lut = ((eLUTDefPtr)ped->elemPvt)->lut;
  receptorPtr  rcp =  pet->receptor;
  CARD32     bands =  rcp->inFlo->bands;
  bandPtr     sbnd = &rcp->band[0];
  bandPtr     dbnd = &pet->emitter[0];
  CARD8       *src;
  CARD32 b;
  
  for(b = 0; b < bands; ++sbnd, ++dbnd, ++b) {

    /* get pointer to the initial src data (i.e. beginning of strip) */
    src = GetCurrentSrc(CARD8,flo,pet,sbnd);

    /* LUT only has 1 strip per band */
    if(!PassStrip(flo,pet,dbnd,sbnd->strip))
      return(FALSE);

    /* free the amount of src strip(s) we've used */
    FreeData(CARD8,flo,pet,sbnd,sbnd->maxLocal);
  }
  return(TRUE);
}                               /* end ActivateELUT */

/*------------------------------------------------------------------------
--------------------------- get rid of left overs ------------------------
------------------------------------------------------------------------*/
static int FlushELUT(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  /* since Activate was suppose to do the whole image, there's nothing to do */
  return(TRUE);
}                               /* end FlushELUT */

/*------------------------------------------------------------------------
------------------------ get rid of run-time stuff -----------------------
------------------------------------------------------------------------*/
static int ResetELUT(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  ResetReceptors(ped);
  ResetEmitter(ped);
  
  return(TRUE);
}                               /* end ResetELUT */

/*------------------------------------------------------------------------
-------------------------- get rid of this element -----------------------
------------------------------------------------------------------------*/
static int DestroyELUT(flo,ped)
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
}                               /* end DestroyELUT */

/* end module melut.c */
