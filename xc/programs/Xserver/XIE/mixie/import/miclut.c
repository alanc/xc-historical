/* $XConsortium$ */
/**** module miclut.c ****/
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
  
	miclut.c -- DDXIE prototype import client lut element
  
	Ben Fahy -- AGE Logic, Inc. May, 1993
  
*****************************************************************************/

#define _XIEC_MICLUT
#define _XIEC_ICLUT

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
#include <element.h>
#include <texstr.h>
#include <lut.h>


/*
 *  routines referenced by other DDXIE modules
 */
int	miAnalyzeICLUT();

/*
 *  routines used internal to this module
 */

static int CreateICLUT();
static int InitializeICLUT();
static int ActivateICLUT();
static int FlushICLUT();
static int ResetICLUT();
static int DestroyICLUT();

/*
 * DDXIE ImportClientLUT entry points
 */

static ddElemVecRec ICLUTVec = {
  CreateICLUT,
  InitializeICLUT,
  ActivateICLUT,
  FlushICLUT,
  ResetICLUT,
  DestroyICLUT
  };

/*
 * Local Declarations
 */

typedef struct _lutpvt {
  int byteptr;		/* current number of bytes received */
  int bytelength;	/* selects total size of array */
  int bytetype;		/* selects array element size */
  int bandnum;		/* for interleave, to swizzle bands */
} lutPvtRec, *lutPvtPtr;

/*------------------------------------------------------------------------
------------------- see if we can handle this element --------------------
------------------------------------------------------------------------*/
int miAnalyzeICLUT(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  xieFloImportClientLUT *raw = (xieFloImportClientLUT *) ped->elemRaw;

  /* for now just copy our entry point vector into the peDef */
  ped->ddVec = ICLUTVec;

  return(TRUE);
}

/*------------------------------------------------------------------------
---------------------------- create peTex . . . --------------------------
------------------------------------------------------------------------*/
static int CreateICLUT(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  int status;
  lutPvtPtr ext;
  
  return MakePETex(flo,ped, xieValMaxBands * sizeof(lutPvtRec), FALSE,FALSE); 
}

/*------------------------------------------------------------------------
---------------------------- initialize peTex . . . ----------------------
------------------------------------------------------------------------*/
static int InitializeICLUT(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  xieFloImportClientLUT *raw = (xieFloImportClientLUT *)ped->elemRaw;
  lutPvtPtr ext = (lutPvtPtr) ped->peTex->private;
  int band, nbands = ped->peTex->receptor[SRCtag].inFlo->bands;
  CARD32 *lengths = &(raw->length0);
  CARD32 *levels = &(raw->level0);

  for (band = 0; band < nbands; band++, ext++, lengths++, levels++) {
	ext->byteptr = 0;
	ext->bytetype = LutPitch(*levels);
	ext->bytelength = *lengths * ext->bytetype;
	ext->bandnum = (raw->class == xieValSingleBand ||
			raw->bandOrder == xieValLSFirst)
			? band : xieValMaxBands - band - 1;
        if (flo->client->swapped && (ext->bytetype == 4))
	     ImplementationError(flo,ped, return(FALSE));
  }
  return InitReceptors(flo,ped,0,1) && InitEmitter(flo,ped,0,-1);
}

/*------------------------------------------------------------------------
----------------------------- crank some data ----------------------------
------------------------------------------------------------------------*/
static int ActivateICLUT(flo,ped,pet)
     floDefPtr flo;
     peDefPtr  ped;
     peTexPtr  pet;
{
  int band, nbands = pet->receptor[SRCtag].inFlo->bands;
  bandPtr iband = &(pet->receptor[SRCtag].band[0]);
  bandPtr oband;
  lutPvtPtr ext = (lutPvtPtr) pet->private;
  
  for(band = 0; band < nbands; band++, ext++, iband++) {
    void * ivoid, *ovoid;
    int ilen, icopy;

    oband = &(pet->emitter[ext->bandnum]);	/* Band Swap ?? */
    
    if(!(ovoid = GetDstBytes(void,flo,pet,oband,0,ext->bytelength,FALSE)))
      return(FALSE);
    
    /*
    **  We have no guarantee to get all the output in one packet.  In 
    **  fact we have no guarantee we will ever get all the output, or
    **  that perhaps we might get too much.  If we do get too much we
    **  would like to notify the user but the protocol does provide
    **  something similar to a decodeNotify.
    */
    for (ilen = 0;
	 ivoid = GetSrcBytes(void,flo,pet,iband,iband->current+ilen,1,FALSE);
	 ) {
      icopy = ilen = iband->strip->length;
      
      if ((ext->byteptr + ilen) > ext->bytelength)
	icopy = ext->bytelength - ext->byteptr;
      
      /* XXX (levels > 2**16) will require padding of 3 bytes to 4 */
      
      if (icopy) {
	bcopy((unsigned char *) ivoid,
	      ((unsigned char *) ovoid) + ext->byteptr, icopy);
	ext->byteptr += icopy;
      }
    }
    FreeData(void, flo, pet, iband, iband->maxLocal);
    
    /* if final, process the data received */
    if (iband->final) {
      if (flo->client->swapped) switch(ext->bytetype) {
      case 1: break;
      case 2: SwapShorts(ovoid, ext->bytelength>>1);
	break;
      case 4:	/* XXX, GACK */
      default: break;
      }
      SetBandFinal(oband);
      PutData(flo,pet,oband,ext->bytelength);
    }
  }
  return TRUE;
}              

/*------------------------------------------------------------------------
--------------------------- get rid of left overs ------------------------
------------------------------------------------------------------------*/
static int FlushICLUT(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  return(TRUE);
}

/*------------------------------------------------------------------------
------------------------ get rid of run-time stuff -----------------------
------------------------------------------------------------------------*/
static int ResetICLUT(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  ResetReceptors(ped);
  ResetEmitter(ped);
  
  return(TRUE);
}

/*------------------------------------------------------------------------
-------------------------- get rid of this element -----------------------
------------------------------------------------------------------------*/
static int DestroyICLUT(flo,ped)
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
}

/* end module miclut.c */

