/* $XConsortium$ */
/**** module micphoto.c ****/
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
  
	micphoto.c -- DDXIE prototype import client photo element
  
	Robert NC Shelley -- AGE Logic, Inc. April, 1993
  
*****************************************************************************/

#define _XIEC_MICPHOTO
#define _XIEC_ICPHOTO

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
#include <xiemd.h>

/*
 * Local Declarations. 
 */

typedef struct _actionparms {
  void   (*action)();
  CARD32 Bstride;
  CARD8	 srcoffset;	/* Number of bytes offset to this pixel's band 	*/
  CARD8  mask;		/* For obtaining subbyte pixels 	 	*/
  CARD8  shift;		/* Necessary shift after obtaining pixel 	*/
  CARD8	 pad;
} miCPhotoActionRec, *miCPhotoActionPtr;

typedef struct _micphotoun {
  miCPhotoActionRec aarray[xieValMaxBands];
  CARD8		    bandMap[xieValMaxBands];
} miCPhotoUnRec, *miCPhotoUnPtr;


/*
 *  routines referenced by other DDXIE modules
 */
int	miAnalyzeICPhoto();

/*
 *  routines used internal to this module
 */
static int ResetICPhoto();

static int CreateICPhotoUn();
static int InitializeICPhotoUn();
static int ActivateICPhotoUncomByPlane();
static int ActivateICPhotoUncomByPixel();
static int DestroyICPhotoUn();

static int CreateICPhotoG31D();
static int InitializeICPhotoG31D();
static int ActivateICPhotoG31D();
static int DestroyICPhotoG31D();

static int CreateICPhotoG32D();
static int InitializeICPhotoG32D();
static int ActivateICPhotoG32D();
static int DestroyICPhotoG32D();


/*
 * routines we need from somewhere else
 */

extern int CreateICPhotoG42D();
extern int InitializeICPhotoG42D();
extern int ActivateICPhotoG42D();
extern int DestroyICPhotoG42D();

/*
 * DDXIE ImportClientPhoto entry points
 */
static ddElemVecRec ICPhotoUncomByPlaneVec = {
  CreateICPhotoUn,
  InitializeICPhotoUn,
  ActivateICPhotoUncomByPlane,
  (xieIntProc)NULL,
  ResetICPhoto,
  DestroyICPhotoUn
  };

static ddElemVecRec ICPhotoUncomByPixelVec = {
  CreateICPhotoUn,
  InitializeICPhotoUn,
  ActivateICPhotoUncomByPixel,
  (xieIntProc)NULL,
  ResetICPhoto,
  DestroyICPhotoUn
  };

static ddElemVecRec ICPhotoG31DVec = {
  CreateICPhotoG31D,
  InitializeICPhotoG31D,
  ActivateICPhotoG31D,
  (xieIntProc)NULL,
  ResetICPhoto,
  DestroyICPhotoG31D
  };

static ddElemVecRec ICPhotoG32DVec = {
  CreateICPhotoG32D,
  InitializeICPhotoG32D,
  ActivateICPhotoG32D,
  (xieIntProc)NULL,
  ResetICPhoto,
  DestroyICPhotoG32D
  };

static ddElemVecRec ICPhotoG42DVec = {
  CreateICPhotoG42D,
  InitializeICPhotoG42D,
  ActivateICPhotoG42D,
  (xieIntProc)NULL,
  ResetICPhoto,
  DestroyICPhotoG42D
  };

/* Bit reversal routine for single band uncompressed bitonal images */
static void reverse_bits(), pass_bits();

/* Action routines used by Triple band by pixel decoding */
static void StoB(), StoP(), StosP(), SbtoB(), Sbtob();

/*------------------------------------------------------------------------
------------------- see if we can handle this element --------------------
------------------------------------------------------------------------*/
int miAnalyzeICPhoto(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  xieFloImportClientPhoto *raw = (xieFloImportClientPhoto *) ped->elemRaw;
  
  switch(raw->decodeTechnique) {
  case xieValDecodeUncompressedSingle:
    if(ped->inFloLst[IMPORT].format[0].stride !=  1 &&
       ped->inFloLst[IMPORT].format[0].stride !=  8 &&
       ped->inFloLst[IMPORT].format[0].stride != 16)
      ImplementationError(flo,ped, return(FALSE));
    ped->ddVec = ICPhotoUncomByPlaneVec;
    break;
    
  case xieValDecodeUncompressedTriple:

    switch(((xieTecDecodeUncompressedTriple *) &raw[1])->interleave) {
    case xieValBandByPlane:
      {	int b;
	
	for(b = 0; b < xieValMaxBands; ++b)
	  if(ped->inFloLst[IMPORT].format[b].stride !=  1 &&
	     ped->inFloLst[IMPORT].format[b].stride !=  8 &&
	     ped->inFloLst[IMPORT].format[b].stride != 16 )
	    ImplementationError(flo,ped, return(FALSE));
	ped->ddVec = ICPhotoUncomByPlaneVec;
      }
    break;

    case xieValBandByPixel:
      if(ped->inFloLst[IMPORT].format[0].stride !=  8  &&
	 ped->inFloLst[IMPORT].format[0].stride != 16  &&
	 ped->inFloLst[IMPORT].format[0].stride != 24  &&
	 ped->inFloLst[IMPORT].format[0].stride != 32 )
	ImplementationError(flo,ped, return(FALSE));
      ped->ddVec = ICPhotoUncomByPixelVec;
      break;
    }
    break;
  case xieValDecodeG31D:
  case xieValDecodeG32D:
  case xieValDecodeG42D:
    ped->ddVec = ICPhotoG42DVec;
    break;
    
  default:
    ImplementationError(flo,ped, return(FALSE));
  }
  return(TRUE);
}                               /* end miAnalyzeICPhoto */

/*------------------------------------------------------------------------
------------------------ get rid of run-time stuff -----------------------
------------------------------------------------------------------------*/
static int ResetICPhoto(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  ResetReceptors(ped);
  ResetEmitter(ped);
  
  return(TRUE);
}                               /* end ResetICPhoto */


/*------------------------------------------------------------------------
---------------------------- create peTex . . . --------------------------
------------------------------------------------------------------------*/
static int CreateICPhotoUn(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  /* attach an execution context to the photo element definition */
  return( MakePETex(flo, ped, sizeof(miCPhotoUnRec), NO_SYNC, NO_SYNC) );
}                               /* end CreateICPhotoUn */

/*------------------------------------------------------------------------
---------------------------- initialize peTex . . . ----------------------
------------------------------------------------------------------------*/
static int InitializeICPhotoUn(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  peTexPtr pet = ped->peTex;
  xieFloImportClientPhoto *raw = (xieFloImportClientPhoto *)ped->elemRaw;
  miCPhotoUnPtr pvt = (miCPhotoUnPtr) (ped->peTex->private);

  if(raw->class == xieValSingleBand) {
    xieTecDecodeUncompressedSingle *tec = 
  			        (xieTecDecodeUncompressedSingle *)&raw[1];
    pvt->bandMap[0] = 0;
#if (BITMAP_BIT_ORDER == MSBFirst)
    if (tec->fillOrder == xieValLSFirst && 
			pet->emitter[pvt->bandMap[0]].format->depth == 1)
    	pvt->aarray[0].action = reverse_bits;
    else
    	pvt->aarray[0].action = pass_bits;
#else
    if (tec->fillOrder == xieValMSFirst && 
			pet->emitter[pvt->bandMap[0]].format->depth == 1)
    	pvt->aarray[0].action = reverse_bits;
    else
    	pvt->aarray[0].action = pass_bits;
#endif
  } else {
    int s, d;
    xieTecDecodeUncompressedTriple *tec = 
  			        (xieTecDecodeUncompressedTriple *)&raw[1];
    /* XXX	should this match the order of the R, G, and B masks for True
     *		and Direct (and some Static) Color Visauls?  (probably not)
     */
    if(((xieTecDecodeUncompressedTriple*)&raw[1])->bandOrder == xieValLSFirst)
      for(d = 0; d < xieValMaxBands; ++d)
	pvt->bandMap[d] = d;
    else 
      for(s = 0, d = xieValMaxBands; d--; ++s)
	pvt->bandMap[s] = d;

    /* If interleaved data coming in, set up action structure */
    if(tec->interleave == xieValBandByPixel) {
	miCPhotoActionPtr aa = &(pvt->aarray[0]);
  	bandPtr         sbnd = &ped->peTex->receptor[IMPORT].band[0];
  	CARD32       sstride = sbnd->format->stride>>3;
	CARD8        leftPad = tec->leftPad[0]>>3;
  	CARD32 depth1 = pet->emitter[pvt->bandMap[0]].format->depth;
  	CARD32 depth2 = pet->emitter[pvt->bandMap[1]].format->depth;
  	CARD32 depth3 = pet->emitter[pvt->bandMap[2]].format->depth;
	
	if (depth1 == 16 && depth2 == 16 && depth3 == 16) {
#if (BITMAP_BIT_ORDER == MSBFirst)
	    void (*pa)() = (tec->pixelOrder == xieValMSFirst) ? StoP : StosP;
#else
	    void (*pa)() = (tec->pixelOrder == xieValMSFirst) ? StosP : StoP;
#endif
            for(s = 0; s < xieValMaxBands; s++, aa++) {
	        aa->action    = pa;
  	        aa->Bstride   = sstride;
	        aa->srcoffset = s + leftPad;
	        aa->mask      = 0; /* Unused */
	        aa->shift     = 0; /* Unused */
	    }
	} else if (depth1 == 8 && depth2 == 8 && depth3 == 8) {
            for(s = 0; s < xieValMaxBands; s++, aa++) {
	        aa->action    = StoB;
  	        aa->Bstride   = sstride;
	        aa->srcoffset = s + leftPad;
	        aa->mask      = 0; /* Unused */
	        aa->shift     = 0; /* Unused */
	    }
	} else if (depth1 == 4 && depth2 == 4 && depth3 == 4) {
	    if (tec->fillOrder == xieValMSFirst) {
	        aa->action    = SbtoB;
  	        aa->Bstride   = sstride;
	        aa->srcoffset = leftPad;
	        aa->mask      = 0xf0; 
	        (aa++)->shift = 4; 
	        aa->action    = SbtoB;
  	        aa->Bstride   = sstride;
	        aa->srcoffset = leftPad;
	        aa->mask      = 0x0f; 
	        (aa++)->shift = 0; 
	        aa->action    = SbtoB;
  	        aa->Bstride   = sstride;
	        aa->srcoffset = 1 + leftPad;
	        aa->mask      = 0xf0; 
	        aa->shift     = 4; 
	    } else { /* xieValLSFirst */
	        aa->action    = SbtoB;
  	        aa->Bstride   = sstride;
	        aa->srcoffset = leftPad;
	        aa->mask      = 0x0f; 
	        (aa++)->shift = 0; 
	        aa->action    = SbtoB;
  	        aa->Bstride   = sstride;
	        aa->srcoffset = leftPad;
	        aa->mask      = 0xf0; 
	        (aa++)->shift = 4; 
	        aa->action    = SbtoB;
  	        aa->Bstride   = sstride;
	        aa->srcoffset = 1 + leftPad;
	        aa->mask      = 0x0f; 
	        aa->shift     = 0; 
	    }
	} else if (depth1 + depth2 + depth3 <= 8) {
	    CARD8 ones = ~0,smask1,smask2,smask3,shift1,shift2,shift3;
	    if (tec->fillOrder == xieValMSFirst) {
	        smask1 = ~(ones>>depth1);
	        smask2 = ~(ones>>(depth1 + depth2) | smask1);
	        smask3 = ~(ones>>(depth1 + depth2 + depth3) | smask1 | smask2);
	        shift1 = 8 - depth1;
	    	shift2 = 8 - (depth1 + depth2);
	    	shift3 = 8 - (depth1 + depth2 + depth3);
	    } else { /* fillOrder == xieValLSFirst */
		smask3 = ~(ones<<depth3);
		smask2 = ~(ones<<(depth2 + depth3) | smask3);
		smask1 = ~(ones<<(depth1 + depth2 + depth3) | smask2 | smask3);
		shift3 = 0;
		shift2 = depth3;
		shift1 = depth2 + depth3;
	    }
	    aa->action    = (depth1 > 1) ? SbtoB : Sbtob;
  	    aa->Bstride   = sstride;
	    aa->srcoffset = leftPad;
	    aa->mask      = smask1;
	    (aa++)->shift = shift1;
	    aa->action    = (depth2 > 1) ? SbtoB : Sbtob;
  	    aa->Bstride   = sstride;
	    aa->srcoffset = leftPad;
	    aa->mask      = smask2;
	    (aa++)->shift = shift2; 
	    aa->action    = (depth3 > 1) ? SbtoB : Sbtob;
  	    aa->Bstride   = sstride;
	    aa->srcoffset = leftPad;
	    aa->mask      = smask3;
	    aa->shift     = shift3; 
	} else 
      	    ImplementationError(flo,ped, return(FALSE));
    }
  }
  return(InitReceptors(flo, ped, NO_DATAMAP, 1) &&
	 InitEmitter(flo, ped, NO_DATAMAP, NO_INPLACE));
}                               /* end InitializeICPhotoUn */

/*------------------------------------------------------------------------
-------------------------- get rid of this element -----------------------
------------------------------------------------------------------------*/
static int DestroyICPhotoUn(flo,ped)
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
}                               /* end DestroyICPhotoUn */


/*------------------------------------------------------------------------
----------------------------- crank some data ----------------------------
------------------------------------------------------------------------*/
static int ActivateICPhotoUncomByPlane(flo,ped,pet)
     floDefPtr flo;
     peDefPtr  ped;
     peTexPtr  pet;
{
  miCPhotoUnPtr pvt = (miCPhotoUnPtr) (pet->private);
  bandPtr  dbnd;
  bandPtr  sbnd = pet->receptor[IMPORT].band;
  CARD32 nbands = ped->inFloLst[IMPORT].bands;
  CARD32  slen, b;
  void *src, *dst;

  for(b = 0; b < nbands; ++sbnd, ++b) {
    void (*action)() = pvt->aarray[0].action;

    slen = sbnd->format->pitch+7>>3;
    dbnd = &pet->emitter[pvt->bandMap[b]];
    if(pet->scheduled & 1<<b &&
       (src = GetSrcBytes(void,flo,pet,sbnd,sbnd->current,slen,KEEP)) &&
       (dst = GetCurrentDst(void,flo,pet,dbnd)))
      do {
/*
 * XXX for > 8-bit: swapping should happen here (if needed)
 * XXX for < 8-bit: "pixel order within byte" should get fixed here (if needed)
 */
	(*action)(src, dst, dbnd->pitch);

	src = GetSrcBytes(void,flo,pet,sbnd,sbnd->current+slen,slen,KEEP);
	dst = GetNextDst(void,flo,pet,dbnd,FLUSH);
      } while(src && dst);
    
    if(!src && sbnd->final || !dst && dbnd->final) {
      if(dbnd->current != dbnd->format->height) {
	/*
	 * the client lied about the image size!
	 */
	if(((xieFloImportClientPhoto*)ped->elemRaw)->notify)
	  SendDecodeNotifyEvent(flo, ped, dbnd->band,
				xieValDecodeUncompressedSingle,
				dbnd->format->width, dbnd->current, TRUE);
	/* 
	 * If the client didn't send enough data, we could zero-fill the
	 * remaining lines.  Since we sent the "aborted" status, we won't
	 * bother (the protocol offers both choices).
	 */
	ValueError(flo,ped,dbnd->format->height, return(FALSE));
      }
      FreeData(void,flo,pet,sbnd,sbnd->maxGlobal);
    } else if (!src) {
      /* free whatever we've used so far and
       * set the threshold to one byte more than whatever is left over
       */
      FreeData(void,flo,pet,sbnd,sbnd->current);
      SetBandThreshold(sbnd, sbnd->available + 1);
    }
  }
  return(TRUE);
}                               /* end ActivateICPhotoUncomByPlane */

/*------------------------------------------------------------------------
----------------------------- crank some data ----------------------------
------------------------------------------------------------------------*/
static int ActivateICPhotoUncomByPixel(flo,ped,pet)
     floDefPtr flo;
     peDefPtr  ped;
     peTexPtr  pet;
{
/* XXX   the following should work for a RGB:332 BytePixels in and 3 separate
 *	 BytePixels out. --- have fun doing all the cases ---
 */
  miCPhotoUnPtr pvt = (miCPhotoUnPtr) (pet->private);
  bandPtr  sbnd = &pet->receptor[IMPORT].band[0];
  bandPtr   db0 = &pet->emitter[pvt->bandMap[0]];
  bandPtr   db1 = &pet->emitter[pvt->bandMap[1]];
  bandPtr   db2 = &pet->emitter[pvt->bandMap[2]];
  CARD32   slen = sbnd->format->pitch+7>>3;
  CARD32  width = db0->format->width;
  miCPhotoActionPtr aap1 = &(pvt->aarray[0]);
  miCPhotoActionPtr aap2 = &(pvt->aarray[1]);
  miCPhotoActionPtr aap3 = &(pvt->aarray[2]);
  BytePixel *src, *dp0, *dp1, *dp2;

  if((src = GetSrcBytes(BytePixel,flo,pet,sbnd,sbnd->current,slen,KEEP)) && 
     (dp0 = GetCurrentDst(BytePixel,flo,pet,db0)) &&
     (dp1 = GetCurrentDst(BytePixel,flo,pet,db1)) && 
     (dp2 = GetCurrentDst(BytePixel,flo,pet,db2)))
    do {
      /* XXX	this is where you'll need a for loop that walks across the
       *	line and copies the three sub-parts of each src pixel into
       *	their respective dst bands
       */
      (*aap1->action)(src,dp0,width,aap1);
      (*aap2->action)(src,dp1,width,aap2);
      (*aap3->action)(src,dp2,width,aap3);

      src = GetSrcBytes(BytePixel,flo,pet,sbnd,sbnd->current+slen,slen,KEEP);
      dp0 = GetNextDst(BytePixel,flo,pet,db0,FLUSH);
      dp1 = GetNextDst(BytePixel,flo,pet,db1,FLUSH);
      dp2 = GetNextDst(BytePixel,flo,pet,db2,FLUSH);
    } while(src && dp0 && dp1 && dp2);

  if(!src && sbnd->final || !dp0 && db0->final) {
    if(db0->current != db0->format->height) {
      /*
       * the client lied about the image size!
       */
      if(((xieFloImportClientPhoto*)ped->elemRaw)->notify)
        SendDecodeNotifyEvent(flo, ped, 0, xieValDecodeUncompressedTriple,
                              db0->format->width, db0->current, TRUE);
      /* 
       * If the client didn't send enough data, we could zero-fill the
       * remaining lines.  Since we sent the "aborted" status, we won't
       * bother (the protocol offers both choices).
       */
      ValueError(flo,ped,db0->format->height, return(FALSE));
    }
    FreeData(void,flo,pet,sbnd,sbnd->maxGlobal);
  } else if (!src) {
    /* free whatever we've used so far and
     * set the threshold to one byte more than whatever is left over
     */
    FreeData(void,flo,pet,sbnd,sbnd->current);
    SetBandThreshold(sbnd, sbnd->available + 1);
  }
  return(TRUE);
}                               /* end ActivateICPhotoUncomByPixel */

/*------------------------------------------------------------------------
---------------------------- create peTex . . . --------------------------
------------------------------------------------------------------------*/
static int CreateICPhotoG31D(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  /* attach an execution context to the photo element definition */
  return( MakePETex(flo,ped,0,FALSE,FALSE) );
}                               /* end CreateICPhoto */


/*------------------------------------------------------------------------
---------------------------- initialize peTex . . . ----------------------
------------------------------------------------------------------------*/
static int InitializeICPhotoG31D(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  return( InitReceptors(flo,ped,0,1) && InitEmitter(flo,ped,0,-1) );
}                               /* end InitializeICPhoto */


/*------------------------------------------------------------------------
----------------------------- crank some data ----------------------------
------------------------------------------------------------------------*/
static int ActivateICPhotoG31D(flo,ped,pet)
     floDefPtr flo;
     peDefPtr  ped;
     peTexPtr  pet;
{
  bandPtr   sbnd = &pet->receptor[IMPORT].band[0];
  bandPtr   dbnd = &pet->emitter[0];
  CARD32    slen = sbnd->format->pitch+7>>3;
  BytePixel *src, *dst;

  for(src = GetSrcBytes(BytePixel,flo,pet,sbnd,sbnd->current,slen,FALSE),
      dst = GetCurrentDst(BytePixel,flo,pet,dbnd);
      src && dst;
      src = GetSrcBytes(BytePixel,flo,pet,sbnd,sbnd->current+slen,slen,TRUE),
      dst = GetNextDst(BytePixel,flo,pet,dbnd,TRUE)) {

    bcopy(src, dst, dbnd->pitch);
  }
  /* free what we've used so far */
  FreeData(BytePixel,flo,pet,sbnd,sbnd->current);

  /* ImportClient elements (i.e. STREAM data) have to tell the scheduler when
   * they can't go any farther (left over bytes are buffered in the receptor)
   */
  return(src != NULL);
}                               /* end ActivateICPhoto */



/*------------------------------------------------------------------------
-------------------------- get rid of this element -----------------------
------------------------------------------------------------------------*/
static int DestroyICPhotoG31D(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  /* get rid of the peTex structure  */
  if(ped->peTex)
    ped->peTex = (peTexPtr) XieFree(ped->peTex);

  /* zap this element's entry point vector */
  ped->ddVec.create     = (xieIntProc) NULL;
  ped->ddVec.initialize = (xieIntProc) NULL;
  ped->ddVec.activate   = (xieIntProc) NULL;
  ped->ddVec.flush      = (xieIntProc) NULL;
  ped->ddVec.reset      = (xieIntProc) NULL;
  ped->ddVec.destroy    = (xieIntProc) NULL;

  return(TRUE);
}                               /* end DestroyICPhoto */

/*------------------------------------------------------------------------
---------------------------- create peTex . . . --------------------------
------------------------------------------------------------------------*/
static int CreateICPhotoG32D(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  /* attach an execution context to the photo element definition */
  return( MakePETex(flo,ped,0,FALSE,FALSE) );
}                               /* end CreateICPhoto */


/*------------------------------------------------------------------------
---------------------------- initialize peTex . . . ----------------------
------------------------------------------------------------------------*/
static int InitializeICPhotoG32D(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  return( InitReceptors(flo,ped,0,1) && InitEmitter(flo,ped,0,-1) );
}                               /* end InitializeICPhoto */


/*------------------------------------------------------------------------
----------------------------- crank some data ----------------------------
------------------------------------------------------------------------*/
static int ActivateICPhotoG32D(flo,ped,pet)
     floDefPtr flo;
     peDefPtr  ped;
     peTexPtr  pet;
{
  bandPtr   sbnd = &pet->receptor[IMPORT].band[0];
  bandPtr   dbnd = &pet->emitter[0];
  CARD32    slen = sbnd->format->pitch+7>>3;
  BytePixel *src, *dst;

  for(src = GetSrcBytes(BytePixel,flo,pet,sbnd,sbnd->current,slen,FALSE),
      dst = GetCurrentDst(BytePixel,flo,pet,dbnd);
      src && dst;
      src = GetSrcBytes(BytePixel,flo,pet,sbnd,sbnd->current+slen,slen,TRUE),
      dst = GetNextDst(BytePixel,flo,pet,dbnd,TRUE)) {

    bcopy(src, dst, dbnd->pitch);
  }
  /* free what we've used so far */
  FreeData(BytePixel,flo,pet,sbnd,sbnd->current);

  /* ImportClient elements (i.e. STREAM data) have to tell the scheduler when
   * they can't go any farther (left over bytes are buffered in the receptor)
   */
  return(src != NULL);
}                               /* end ActivateICPhoto */



/*------------------------------------------------------------------------
-------------------------- get rid of this element -----------------------
------------------------------------------------------------------------*/
static int DestroyICPhotoG32D(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  /* get rid of the peTex structure  */
  if(ped->peTex)
    ped->peTex = (peTexPtr) XieFree(ped->peTex);

  /* zap this element's entry point vector */
  ped->ddVec.create     = (xieIntProc) NULL;
  ped->ddVec.initialize = (xieIntProc) NULL;
  ped->ddVec.activate   = (xieIntProc) NULL;
  ped->ddVec.flush      = (xieIntProc) NULL;
  ped->ddVec.reset      = (xieIntProc) NULL;
  ped->ddVec.destroy    = (xieIntProc) NULL;

  return(TRUE);
}                               /* end DestroyICPhoto */

/*------------------------------------------------------------------------
---------------------------- create peTex . . . --------------------------
------------------------------------------------------------------------*/
static int CreateICPhotoTIFF2(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  /* attach an execution context to the photo element definition */
  return( MakePETex(flo,ped,0,FALSE,FALSE) );
}                               /* end CreateICPhoto */


/*------------------------------------------------------------------------
---------------------------- initialize peTex . . . ----------------------
------------------------------------------------------------------------*/
static int InitializeICPhotoTIFF2(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  return( InitReceptors(flo,ped,0,1) && InitEmitter(flo,ped,0,-1) );
}                               /* end InitializeICPhoto */


/*------------------------------------------------------------------------
----------------------------- crank some data ----------------------------
------------------------------------------------------------------------*/
static int ActivateICPhotoTIFF2(flo,ped,pet)
     floDefPtr flo;
     peDefPtr  ped;
     peTexPtr  pet;
{
  bandPtr   sbnd = &pet->receptor[IMPORT].band[0];
  bandPtr   dbnd = &pet->emitter[0];
  CARD32    slen = sbnd->format->pitch+7>>3;
  BytePixel *src, *dst;

  for(src = GetSrcBytes(BytePixel,flo,pet,sbnd,sbnd->current,slen,FALSE),
      dst = GetCurrentDst(BytePixel,flo,pet,dbnd);
      src && dst;
      src = GetSrcBytes(BytePixel,flo,pet,sbnd,sbnd->current+slen,slen,TRUE),
      dst = GetNextDst(BytePixel,flo,pet,dbnd,TRUE)) {

    bcopy(src, dst, dbnd->pitch);
  }
  /* free what we've used so far */
  FreeData(BytePixel,flo,pet,sbnd,sbnd->current);

  /* ImportClient elements (i.e. STREAM data) have to tell the scheduler when
   * they can't go any farther (left over bytes are buffered in the receptor)
   */
  return(src != NULL);
}                               /* end ActivateICPhoto */



/*------------------------------------------------------------------------
-------------------------- get rid of this element -----------------------
------------------------------------------------------------------------*/
static int DestroyICPhotoTIFF2(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  /* get rid of the peTex structure  */
  if(ped->peTex)
    ped->peTex = (peTexPtr) XieFree(ped->peTex);

  /* zap this element's entry point vector */
  ped->ddVec.create     = (xieIntProc) NULL;
  ped->ddVec.initialize = (xieIntProc) NULL;
  ped->ddVec.activate   = (xieIntProc) NULL;
  ped->ddVec.flush      = (xieIntProc) NULL;
  ped->ddVec.reset      = (xieIntProc) NULL;
  ped->ddVec.destroy    = (xieIntProc) NULL;

  return(TRUE);
}                               /* end DestroyICPhoto */

/* Triple Band Byte by Pixel Action routines */

/* Stream bits to bits */
static void Sbtob(src,idst,count,aap)
BytePixel *src, *idst;
CARD32 count;
miCPhotoActionPtr aap;
{
CARD32 Bstride = aap->Bstride;
CARD8     mask = aap->mask;
LogInt	  *dst = (LogInt *)idst;
int i;

	bzero(dst,count+7>>3);	/* zero out the output, only set ones */
	src += aap->srcoffset;
	for (i = 0; i < count; i++, src+=Bstride) 
	    if ((*src & mask) != 0)
		LOG_setbit(dst,count);
}

/* Stream in nibbles to BytePixels */
static void SbtoB(src,dst,count,aap)
BytePixel *src, *dst;
CARD32 count;
miCPhotoActionPtr aap;
{
CARD32 Bstride = aap->Bstride;
CARD8     mask = aap->mask;
CARD8    shift = aap->shift;
int i;

	src += aap->srcoffset;
	for (i = 0; i < count; i++, src+=Bstride) 
		*dst++ = (*src & mask) >> shift;
}

/* Stream to BytePixel */
static void StoB(src,dst,count,aap)
BytePixel *src, *dst;
CARD32 count;
miCPhotoActionPtr aap;
{
CARD32 Bstride = aap->Bstride;
int i;

	src += aap->srcoffset;
	for (i = 0; i < count; i++, src+=Bstride) 
		*dst++ = *src;
}

/* Stream to PairPixel (unswapped) */
static void StoP(isrc,dst,count,aap)
PairPixel *isrc, *dst;
CARD32 count;
miCPhotoActionPtr aap;
{
CARD32 Bstride = aap->Bstride;
CARD8 *src = ((CARD8 *)isrc) + aap->srcoffset;
int i;
	for (i = 0; i < count; i++, src+=Bstride) 
		*dst++ = *(PairPixel *)src;
}

/* Stream to PairPixel (swapped) */
static void StosP(isrc,dst,count,aap)
PairPixel *isrc, *dst;
CARD32 count;
miCPhotoActionPtr aap;
{
CARD32 Bstride = aap->Bstride;
CARD8 *src = ((CARD8 *)isrc) + aap->srcoffset;
int i;
	for (i = 0; i < count; i++, src+=Bstride) {
		PairPixel sval = *(PairPixel *)src;
		*dst++ = (sval >> 8) | (sval << 8);
	}
}

extern unsigned char _ByteReverseTable[];

static void reverse_bits(src,dst,count)
BytePixel *src, *dst;
CARD32 count;
{
int i;
	for (i = 0; i < count; i++) *dst++ = _ByteReverseTable[*src++];
}

static void pass_bits(src,dst,count)
BytePixel *src, *dst;
CARD32 count;
{
	bcopy((char *)src,(char *)dst,(int)count);
}
/* end module micphoto.c */
