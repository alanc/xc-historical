/* $XConsortium: micphoto.c,v 1.3 93/10/31 09:45:05 dpw Exp $ */
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
  
	micphoto.c -- DDXIE import client photo element
  
	Robert NC Shelley && Dean Verheiden -- AGE Logic, Inc. April, 1993

	Fax stuff added by Ben Fahy
  
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
#include <miuncomp.h>

/*
 * Local Declarations. 
 */

/*
 *  routines referenced by other DDXIE modules
 */
int	miAnalyzeICPhoto();

/*
 *  routines used internal to this module
 */
static int CreateICPhotoUn();
static int InitializeICPhotoUncomByPlane();
static int ActivateICPhotoUncomByPlane();
static int ResetICPhoto();
static int DestroyICPhotoUn();

#if XIE_FULL
static int InitializeICPhotoUncomByPixel();
static int ActivateICPhotoUncomByPixel();
#endif

#ifdef optional
static int CreateICPhotoJPEGLoss();
static int InitializeICPhotoJPEGLoss();
static int ActivateICPhotoJPEGLoss();
static int DestroyICPhotoJPEGLoss();
#endif

/*
 * routines we need from somewhere else
 */

extern int CreateICPhotoFax();
extern int InitializeICPhotoFax();
extern int ActivateICPhotoFax();
extern int ResetICPhotoFax();
extern int DestroyICPhotoFax();

#if XIE_FULL
extern int CreateIPhotoJpegBase();
extern int InitializeICPhotoJpegBase();
extern int ActivateIPhotoJpegBase();
extern int ResetIPhotoJpegBase();
extern int DestroyIPhotoJpegBase();
#endif /* XIE_FULL */

/*
 * DDXIE ImportClientPhoto entry points
 */
static ddElemVecRec ICPhotoUncomByPlaneVec = {
  CreateICPhotoUn,
  InitializeICPhotoUncomByPlane,
  ActivateICPhotoUncomByPlane,
  (xieIntProc)NULL,
  ResetICPhoto,
  DestroyICPhotoUn
  };

#if XIE_FULL
static ddElemVecRec ICPhotoUncomByPixelVec = {
  CreateICPhotoUn,
  InitializeICPhotoUncomByPixel,
  ActivateICPhotoUncomByPixel,
  (xieIntProc)NULL,
  ResetICPhoto,
  DestroyICPhotoUn
  };
#endif /* XIE_FULL */

static ddElemVecRec ICPhotoFaxVec = {
  CreateICPhotoFax,
  InitializeICPhotoFax,
  ActivateICPhotoFax,
  (xieIntProc)NULL,
  ResetICPhotoFax,
  DestroyICPhotoFax
  };

#if XIE_FULL
static ddElemVecRec ICPhotoJpegBaseVec = {
  CreateIPhotoJpegBase,
  InitializeICPhotoJpegBase,	/* only thing not shared with ImportPmap */
  ActivateIPhotoJpegBase,
  (xieIntProc)NULL,
  ResetIPhotoJpegBase,
  DestroyIPhotoJpegBase
  };
#endif /* XIE_FULL */

/*------------------------------------------------------------------------
------------------- see if we can handle this element --------------------
------------------------------------------------------------------------*/
int miAnalyzeICPhoto(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  xieFloImportClientPhoto *raw = (xieFloImportClientPhoto *)ped->elemRaw;
  
  switch(raw->decodeTechnique) {
  case xieValDecodeUncompressedSingle:
    ped->ddVec = ICPhotoUncomByPlaneVec;
    break;
    
#if XIE_FULL
  case xieValDecodeUncompressedTriple:

    if ((((xieTecDecodeUncompressedTriple *)&raw[1])->interleave) == 
							xieValBandByPlane) {
	ped->ddVec = ICPhotoUncomByPlaneVec;
    } else {  /* xieValBandByPixel */
      ped->ddVec = ICPhotoUncomByPixelVec;
      break;
    }
    break;

#endif /* XIE_FULL */

  case xieValDecodeG31D:
  case xieValDecodeG32D:
  case xieValDecodeG42D:
  case xieValDecodeTIFF2:
  case xieValDecodeTIFFPackBits:
    ped->ddVec = ICPhotoFaxVec;
    break;

#if XIE_FULL
  case xieValDecodeJPEGBaseline:
    ped->ddVec = ICPhotoJpegBaseVec;
    break;
#endif /* XIE_FULL */

  default:
    ImplementationError(flo,ped, return(FALSE));
  }
  return(TRUE);
}                               /* end miAnalyzeICPhoto */


/*------------------------------------------------------------------------
---------------------------- create peTex . . . --------------------------
------------------------------------------------------------------------*/
static int CreateICPhotoUn(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  /* attach an execution context to the photo element definition */
  return( MakePETex(flo, ped, xieValMaxBands * sizeof(miUncompRec), 
			      NO_SYNC, NO_SYNC) );
}                               /* end CreateICPhotoUn */

/*------------------------------------------------------------------------
---------------------------- initialize peTex . . . ----------------------
------------------------------------------------------------------------*/
static int InitializeICPhotoUncomByPlane(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  xieFloImportClientPhoto *raw = (xieFloImportClientPhoto *)ped->elemRaw;
  peTexPtr                 pet = ped->peTex;
  miUncompPtr              pvt = (miUncompPtr)pet->private;
  CARD32	        nbands = ped->outFlo.bands,b,s;
  formatPtr                inf = ped->inFloLst[IMPORT].format;
  xieTypOrientation pixelOrder, fillOrder;
  CARD8 *ppad;
  
  if (nbands == 1) {
    xieTecDecodeUncompressedSingle *tec = ((xieTecDecodeUncompressedSingle*)
					   &raw[1]);
    pixelOrder   = tec->pixelOrder;
    fillOrder    = tec->fillOrder;
    pvt->bandMap = 0;
    ppad         = &tec->leftPad;
  } else {
    xieTecDecodeUncompressedTriple *tec = ((xieTecDecodeUncompressedTriple*)
					   &raw[1]);
    pixelOrder = tec->pixelOrder;
    fillOrder  = tec->fillOrder;
    ppad       = tec->leftPad;
    if (tec->bandOrder == xieValLSFirst) 
      for(b = 0; b < xieValMaxBands; ++b) 
	pvt[b].bandMap = b;
    else 
      for(s = 0, b = xieValMaxBands; b--; ++s)
	pvt[s].bandMap = b;
  }
  
  for (b = 0; b < nbands; b++, pvt++, ppad++, inf++) {
    pvt->bitOff =  pvt->leftPad = *ppad;
    if (inf->depth == 1) {  
#if (IMAGE_BYTE_ORDER == MSBFirst)
      if (pvt->leftPad & 7 || inf->stride != 1) {
	   pvt->action = (fillOrder == xieValMSFirst) ? CPextractstreambits:
						   CPextractswappedstreambits;
      } else {
      	   pvt->action = (fillOrder == xieValMSFirst) ? CPpass_bits : 
						        CPreverse_bits;
      }
#else
      if (pvt->leftPad & 7 || inf->stride != 1) {
	   pvt->action = (fillOrder == xieValLSFirst) ? CPextractstreambits:
						   CPextractswappedstreambits;
      } else {
      	   pvt->action = (fillOrder == xieValLSFirst) ? CPpass_bits : 
						        CPreverse_bits;
      }
#endif
    } else if (inf->depth <= 8) {
      if (pvt->leftPad & 7 || inf->stride & 7) {
	/* They chose . . . poorly */
	if (pixelOrder == xieValMSFirst) {
	  if(fillOrder == xieValMSFirst)
	    pvt->action = MMUBtoB;
	  else
	    pvt->action = MLUBtoB;
	} else {
	  if(fillOrder == xieValMSFirst)
	    pvt->action = LMUBtoB;
	  else
	    pvt->action = LLUBtoB;
	}
      } else {
	/* They chose wisely */
	pvt->action = CPpass_bytes; 
      }
    } else if (inf->depth <= 16) {
      if (pvt->leftPad & 15 || inf->stride & 15) {
	/* They chose . . . poorly */
	if (pixelOrder == xieValMSFirst) {
	  if(fillOrder == xieValMSFirst)
	    pvt->action = MMUPtoP;
	  else
	    pvt->action = MLUPtoP;
	} else {
	  if(fillOrder == xieValMSFirst)
	    pvt->action = LMUPtoP;
	  else
	    pvt->action = LLUPtoP;
	}
      } else {
	/* They chose wisely */
#if (IMAGE_BYTE_ORDER == MSBFirst)
	pvt->action = (fillOrder==xieValMSFirst) ? CPpass_pairs : CPswap_pairs;
#else
	pvt->action = (fillOrder==xieValLSFirst) ? CPpass_pairs : CPswap_pairs;
#endif
      }
    } else if (inf->depth <= 24) {
      if (pvt->leftPad & 31 || inf->stride & 31) {
	/* They chose . . . poorly */
	if (pixelOrder == xieValMSFirst) {
	  if(fillOrder == xieValMSFirst)
	    pvt->action = MMUQtoQ;
	  else
	    pvt->action = MLUQtoQ;
	} else {
	  if(fillOrder == xieValMSFirst)
	    pvt->action = LMUQtoQ;
	  else
	    pvt->action = LLUQtoQ;
	}
      } else {
	/* They chose wisely */
#if (IMAGE_BYTE_ORDER == MSBFirst)
	pvt->action = (fillOrder==xieValMSFirst) ? CPpass_quads : CPswap_quads;
#else
	pvt->action = (fillOrder==xieValLSFirst) ? CPpass_quads : CPswap_quads;
#endif
      }
    } else {
      ImplementationError(flo,ped, return(FALSE));
    }
  }
  return(InitReceptors(flo, ped, NO_DATAMAP, 1) &&
	 InitEmitter(flo, ped, NO_DATAMAP, NO_INPLACE));
}                               /* end InitializeICPhotoUnSingle */


/*------------------------------------------------------------------------
----------------------------- crank some data ----------------------------
------------------------------------------------------------------------*/
static int ActivateICPhotoUncomByPlane(flo,ped,pet)
     floDefPtr flo;
     peDefPtr  ped;
     peTexPtr  pet;
{
  xieFloImportClientPhoto *raw = (xieFloImportClientPhoto *)ped->elemRaw;
  miUncompPtr              pvt = (miUncompPtr)pet->private;
  CARD32 	        nbands = ped->inFloLst[IMPORT].bands;
  bandPtr                 sbnd = pet->receptor[IMPORT].band, dbnd;
  CARD32 oldslen, nextslen, b;
  pointer src = (pointer)NULL, dst = (pointer)NULL;

  for(b = 0; b < nbands; ++sbnd, ++b, pvt++) {
    void (*action)() = pvt->action;

    nextslen = pvt->bitOff + sbnd->format->pitch + 7 >> 3;
    dbnd = &pet->emitter[pvt->bandMap];
    if(pet->scheduled & 1<<b &&
       (src = GetSrcBytes(pointer,flo,pet,sbnd,sbnd->current,nextslen,KEEP)) &&
       (dst = GetCurrentDst(pointer,flo,pet,dbnd))) {
      do {

	(*action)(src, dst, sbnd->format->width, pvt->bitOff,
					  (CARD32)sbnd->format->depth,
					  sbnd->format->stride);

        pvt->bitOff = pvt->bitOff + sbnd->format->pitch & 7;	/* Set next */
	oldslen = (pvt->bitOff) ? nextslen - 1 : nextslen;
	nextslen = pvt->bitOff + sbnd->format->pitch + 7 >> 3;
	src = GetSrcBytes(pointer,flo,pet,sbnd,sbnd->current+oldslen, nextslen,
									KEEP);
	dst = GetNextDst(pointer,flo,pet,dbnd,FLUSH);
      } while(src && dst);
    }
    
    if(!src && sbnd->final && dbnd->current < dbnd->format->height) {
      /*
       * the client lied about the image size!
       */
      if(raw->notify)
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
    if (!src || dbnd->final) {
      /* free whatever we've used so far and
       * set the threshold to one byte more than whatever is left over
       */
      FreeData(flo, pet, sbnd, sbnd->final ? sbnd->maxGlobal : sbnd->current);
      SetBandThreshold(sbnd, sbnd->available + 1);
    }
  }
  return(TRUE);
}                               /* end ActivateICPhotoUncomByPlane */


#if XIE_FULL
/*------------------------------------------------------------------------
---------------------------- initialize peTex . . . ----------------------
------------------------------------------------------------------------*/
static int InitializeICPhotoUncomByPixel(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  xieFloImportClientPhoto *raw = (xieFloImportClientPhoto *)ped->elemRaw;
  peTexPtr                 pet = ped->peTex;
  miUncompPtr              pvt = (miUncompPtr)pet->private;
  bandPtr                 sbnd = &ped->peTex->receptor[IMPORT].band[0];
  CARD32               sstride = sbnd->format->stride>>3;
  xieTecDecodeUncompressedTriple *tec = ((xieTecDecodeUncompressedTriple*)
					 &raw[1]);
  CARD8                leftPad =  tec->leftPad[0]>>3;
  CARD32 depth1, depth2, depth3;
  int s, d;
  
  pvt->unaligned = FALSE;	/* Hope for the best */
  
  if(tec->bandOrder == xieValLSFirst)
    for(d = 0; d < xieValMaxBands; ++d)
      pvt[d].bandMap = d;
  else 
    for(s = 0, d = xieValMaxBands; d--; ++s)
      pvt[s].bandMap = d;
  
  depth1 = pet->emitter[pvt[0].bandMap].format->depth;
  depth2 = pet->emitter[pvt[1].bandMap].format->depth;
  depth3 = pet->emitter[pvt[2].bandMap].format->depth;
  
  pvt->bitOff = pvt->leftPad = tec->leftPad[0];
  
  /* See if data is nicely aligned */
  if (!(tec->leftPad[0] & 7) && !(sbnd->format->stride & 7)) {
    if (depth1 == 16 && depth2 == 16 && depth3 == 16) {
#if (IMAGE_BYTE_ORDER == MSBFirst)
      void (*pa)() = (tec->pixelOrder == xieValMSFirst) ? StoP  : StosP;
#else
      void (*pa)() = (tec->pixelOrder == xieValMSFirst) ? StosP : StoP;
#endif
      for(s = 0; s < xieValMaxBands; s++, pvt++) {
	pvt->action    = pa;
	pvt->Bstride   = sstride;
	pvt->srcoffset = s + leftPad;
	pvt->mask      = 0; /* Unused */
	pvt->shift     = 0; /* Unused */
      }
    } else if (depth1 == 8 && depth2 == 8 && depth3 == 8) {
      for(s = 0; s < xieValMaxBands; s++, pvt++) {
	pvt->action    = StoB;
	pvt->Bstride   = sstride;
	pvt->srcoffset = s + leftPad;
	pvt->mask      = 0; /* Unused */
	pvt->shift     = 0; /* Unused */
      }
    } else if (depth1 == 4 && depth2 == 4 && depth3 == 4) {
      if (tec->fillOrder == xieValMSFirst) {
	pvt->action    = SbtoB;
	pvt->Bstride   = sstride;
	pvt->srcoffset = leftPad;
	pvt->mask      = 0xf0; 
	(pvt++)->shift = 4; 
	pvt->action    = SbtoB;
	pvt->Bstride   = sstride;
	pvt->srcoffset = leftPad;
	pvt->mask      = 0x0f; 
	(pvt++)->shift = 0; 
	pvt->action    = SbtoB;
	pvt->Bstride   = sstride;
	pvt->srcoffset = 1 + leftPad;
	pvt->mask      = 0xf0; 
	pvt->shift     = 4; 
      } else { /* xieValLSFirst */
	pvt->action    = SbtoB;
	pvt->Bstride   = sstride;
	pvt->srcoffset = leftPad;
	pvt->mask      = 0x0f; 
	(pvt++)->shift = 0; 
	pvt->action    = SbtoB;
	pvt->Bstride   = sstride;
	pvt->srcoffset = leftPad;
	pvt->mask      = 0xf0; 
	(pvt++)->shift = 4; 
	pvt->action    = SbtoB;
	pvt->Bstride   = sstride;
	pvt->srcoffset = 1 + leftPad;
	pvt->mask      = 0x0f; 
	pvt->shift     = 0; 
      }
    } else if (depth1 + depth2 + depth3 <= 8) {
      CARD8 ones = 0xff,smask1,smask2,smask3,shift1,shift2,shift3;
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
      pvt->action    = (depth1 > 1) ? SbtoB : Sbtob;
      pvt->Bstride   = sstride;
      pvt->srcoffset = leftPad;
      pvt->mask      = smask1;
      (pvt++)->shift = shift1;
      pvt->action    = (depth2 > 1) ? SbtoB : Sbtob;
      pvt->Bstride   = sstride;
      pvt->srcoffset = leftPad;
      pvt->mask      = smask2;
      (pvt++)->shift = shift2; 
      pvt->action    = (depth3 > 1) ? SbtoB : Sbtob;
      pvt->Bstride   = sstride;
      pvt->srcoffset = leftPad;
      pvt->mask      = smask3;
      pvt->shift     = shift3; 
    } else {
      pvt->unaligned = TRUE;
    }
  } else {
    pvt->unaligned = TRUE;
  }
  
  /* Wasn't nicely aligned, do it the hard way */
  if (((miUncompPtr)pet->private)->unaligned) {
    pvt->action = ExtractTripleFuncs
      [(tec->pixelOrder == xieValLSFirst) ? 0 : 1]
	[(tec->fillOrder  == xieValLSFirst) ? 0 : 1]
	  [(depth1 <= 8) ? 0 : 1] 
	    [(depth2 <= 8) ? 0 : 1] 
	      [(depth3 <= 8) ? 0 : 1];
  }
  return(InitReceptors(flo, ped, NO_DATAMAP, 1) &&
	 InitEmitter(flo, ped, NO_DATAMAP, NO_INPLACE));
}                               /* end InitializeICPhotoUnTriple */

/*------------------------------------------------------------------------
----------------------------- crank some data ----------------------------
------------------------------------------------------------------------*/
static int ActivateICPhotoUncomByPixel(flo,ped,pet)
     floDefPtr flo;
     peDefPtr  ped;
     peTexPtr  pet;
{
  xieFloImportClientPhoto *raw = (xieFloImportClientPhoto *)ped->elemRaw;
  miUncompPtr pvt = (miUncompPtr) (pet->private);
  bandPtr    sbnd = &pet->receptor[IMPORT].band[0];
  bandPtr     db0 = &pet->emitter[pvt[0].bandMap];
  bandPtr     db1 = &pet->emitter[pvt[1].bandMap];
  bandPtr     db2 = &pet->emitter[pvt[2].bandMap];
  CARD32    width = db0->format->width;
  pointer   dp0 = NULL, dp1 = NULL, dp2 = NULL;
  BytePixel  *src;
  
  if (pvt->unaligned) {
    CARD32 oldslen, nextslen;
    void (*action)() = pvt->action;
    CARD32 depth0 = db0->format->depth;
    CARD32 depth1 = db1->format->depth;
    CARD32 depth2 = db2->format->depth;
    CARD32 stride = sbnd->format->stride;
    
    nextslen = pvt->bitOff + sbnd->format->pitch + 7 >> 3;
    if((src = GetSrcBytes(BytePixel *,flo,pet,sbnd,sbnd->current,nextslen,KEEP))
       && (dp0 = GetCurrentDst(pointer,flo,pet,db0)) &&
       (dp1 = GetCurrentDst(pointer,flo,pet,db1)) && 
       (dp2 = GetCurrentDst(pointer,flo,pet,db2)))
      do {
	
	(*action)(src,dp0,dp1,dp2,width,pvt->bitOff,depth0,depth1,depth2, 
		  stride);
	
	pvt->bitOff = pvt->bitOff + sbnd->format->pitch & 7;	/* Set next */
	oldslen = (pvt->bitOff) ? nextslen - 1 : nextslen;
	nextslen = pvt->bitOff + sbnd->format->pitch + 7 >> 3;
	src = GetSrcBytes(BytePixel *,flo,pet,sbnd,sbnd->current+oldslen,
			  nextslen,KEEP);
	dp0 = GetNextDst(pointer,flo,pet,db0,FLUSH);
	dp1 = GetNextDst(pointer,flo,pet,db1,FLUSH);
	dp2 = GetNextDst(pointer,flo,pet,db2,FLUSH);
      } while(src && dp0 && dp1 && dp2);
  } else {
    CARD32   slen = sbnd->format->pitch+7>>3;
    if((src = GetSrcBytes(BytePixel *,flo,pet,sbnd,sbnd->current,slen,KEEP)) && 
       (dp0 = GetCurrentDst(pointer,flo,pet,db0)) &&
       (dp1 = GetCurrentDst(pointer,flo,pet,db1)) && 
       (dp2 = GetCurrentDst(pointer,flo,pet,db2)))
      do {
	
	(*pvt[0].action)(src,dp0,width,&pvt[0]);
	(*pvt[1].action)(src,dp1,width,&pvt[1]);
	(*pvt[2].action)(src,dp2,width,&pvt[2]);
	
	src =GetSrcBytes(BytePixel *,flo,pet,sbnd,sbnd->current+slen,slen,KEEP);
	dp0 = GetNextDst(pointer,flo,pet,db0,FLUSH);
	dp1 = GetNextDst(pointer,flo,pet,db1,FLUSH);
	dp2 = GetNextDst(pointer,flo,pet,db2,FLUSH);
      } while(src && dp0 && dp1 && dp2);
  }
  if(!src && sbnd->final && db0->current < db0->format->height) {
    /*
     * the client lied about the image size!
     */
    if(raw->notify)
      SendDecodeNotifyEvent(flo, ped, 0, xieValDecodeUncompressedTriple,
			    db0->format->width, db0->current, TRUE);
    /* 
     * If the client didn't send enough data, we could zero-fill the
     * remaining lines.  Since we sent the "aborted" status, we won't
     * bother (the protocol offers both choices).
     */
    ValueError(flo,ped,db0->format->height, return(FALSE));
  }
  if (!src || db0->final) {
    /* free whatever we've used so far and
     * set the threshold to one byte more than whatever is left over
     */
    FreeData(flo, pet, sbnd, sbnd->final ? sbnd->maxGlobal : sbnd->current);
    SetBandThreshold(sbnd, sbnd->available + 1);
  }
  return(TRUE);
}                               /* end ActivateICPhotoUncomByPixel */
#endif /* XIE_FULL */


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

/* end module micphoto.c */
