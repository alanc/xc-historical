/* $XConsortium$ */
/**** module miphoto.c ****/
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
  
	miphoto.c -- DDXIE prototype import client photomap element
  
	Robert NC Shelley -- AGE Logic, Inc. April, 1993
  
*****************************************************************************/

#define _XIEC_MIPHOTO
#define _XIEC_IPHOTO

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
#include <photomap.h>
#include <element.h>
#include <texstr.h>
#include <miphoto.h>


/*
 *  routines referenced by other DDXIE modules
 */
int	miAnalyzeIPhoto();

/*
 *  routines used internal to this module
 */
static int CreateIPhoto();
static int InitializeIPhoto();
static int ActivateIPhoto();
static int ResetIPhoto();
static int DestroyIPhoto();

/*
 * DDXIE ImportPhotomap entry points
 */
static ddElemVecRec miPhotoVec = {
  CreateIPhoto,
  InitializeIPhoto,
  ActivateIPhoto,
  (xieIntProc)NULL,
  ResetIPhoto,
  DestroyIPhoto
  };


/*------------------------------------------------------------------------
------------------- see if we can handle this element --------------------
------------------------------------------------------------------------*/
int miAnalyzeIPhoto(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  /* for now just stash our entry point vector in the peDef */
  ped->ddVec = miPhotoVec;

  return(TRUE);
}                               /* end miAnalyzeIPhoto */


/*------------------------------------------------------------------------
---------------------------- create peTex . . . --------------------------
------------------------------------------------------------------------*/
static int CreateIPhoto(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  /* attach an execution context to the photo element definition */
  return(MakePETex(flo, ped, sizeof(miPhotoDefRec), NO_SYNC, NO_SYNC));
}                               /* end CreateIPhoto */


/*------------------------------------------------------------------------
---------------------------- initialize peTex . . . ----------------------
------------------------------------------------------------------------*/
static int InitializeIPhoto(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  photomapPtr   map = ((iPhotoDefPtr)ped->elemPvt)->map;
  miPhotoDefPtr pvt = ped->peTex->private;
  CARD32 b;

  for(b = 0; b < map->bands; ++b)
    pvt->next_strip[b] = map->strips[b].flink;

  /* note: ImportResource elements don't use their receptors */
  return(InitEmitter(flo, ped, NO_DATAMAP, NO_INPLACE));
}                               /* end InitializeIPhoto */


/*------------------------------------------------------------------------
----------------------------- crank some data ----------------------------
------------------------------------------------------------------------*/
static int ActivateIPhoto(flo,ped,pet)
     floDefPtr flo;
     peDefPtr  ped;
     peTexPtr  pet;
{
  miPhotoDefPtr pvt = pet->private;
  photomapPtr   map = ((iPhotoDefPtr)ped->elemPvt)->map;
  bandPtr       bnd = &pet->emitter[0];
  CARD32 b;

  for(b = 0; b < map->bands; ++bnd, ++b)
    if(!bnd->final) {
      stripPtr strip = pvt->next_strip[b];
      pvt->next_strip[b] = strip->flink;

      if(ListEnd(strip,&map->strips[b]))
	AccessError(flo,ped, return(FALSE));

      /* pass a clone of the current strip to our recipients
       */
      if(!PassStrip(flo,pet,bnd,strip))
	return(FALSE);	/* alloc error */
    }
  return(TRUE);
}                               /* end ActivateIPhoto */


/*------------------------------------------------------------------------
------------------------ get rid of run-time stuff -----------------------
------------------------------------------------------------------------*/
static int ResetIPhoto(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  ResetEmitter(ped);
  
  return(TRUE);
}                               /* end ResetIPhoto */


/*------------------------------------------------------------------------
-------------------------- get rid of this element -----------------------
------------------------------------------------------------------------*/
static int DestroyIPhoto(flo,ped)
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
}                               /* end DestroyIPhoto */

/* end module miphoto.c */

