/* $XConsortium: mecphoto.c,v 1.2 93/07/19 18:31:29 rws Exp $ */
/**** module mecphoto.c ****/
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
  
	mecphoto.c -- DDXIE export client photo element
  
	Robert NC Shelley && Dean Verheiden -- AGE Logic, Inc. June, 1993
  
*****************************************************************************/

/**** Note: Triple Band Interleaved by Pixel Code is untested . . . *********/

#define _XIEC_MECPHOTO
#define _XIEC_ECPHOTO

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

typedef struct _eactionparms {
  void   (*action)();
  CARD32 Bstride;
  CARD8	 dstoffset;	/* Number of bytes offset to this pixel's band 	*/
  CARD8  mask;		/* For obtaining subbyte pixels 	 	*/
  CARD8  shift;		/* Necessary shift after obtaining pixel 	*/
  BOOL	 clear_dst;     /* Clear destination before packing		*/
} miCPhotoActionRec, *miCPhotoActionPtr;

typedef struct _micphotoun {
  miCPhotoActionRec aarray[xieValMaxBands];
  CARD8		    bandMap[xieValMaxBands];
} miCPhotoUnRec, *miCPhotoUnPtr;


/* routines referenced by other DDXIE modules
 */
int	miAnalyzeECPhoto();

/* routines used internal to this module
 */
static int ResetECPhoto();

static int CreateECPhotoUncomByPlane();
static int CreateECPhotoUncomByPixel();
static int InitializeECPhotoUn();
static int ActivateECPhotoUncomByPlane();
static int ActivateECPhotoUncomByPixel();
static int DestroyECPhotoUn();

/* DDXIE ExportClientPhoto entry points
 */
static ddElemVecRec ECPhotoUncomByPlaneVec = {
  CreateECPhotoUncomByPlane,
  InitializeECPhotoUn,
  ActivateECPhotoUncomByPlane,
  (xieIntProc)NULL,
  ResetECPhoto,
  DestroyECPhotoUn
  };

static ddElemVecRec ECPhotoUncomByPixelVec = {
  CreateECPhotoUncomByPixel,
  InitializeECPhotoUn,
  ActivateECPhotoUncomByPixel,
  (xieIntProc)NULL,
  ResetECPhoto,
  DestroyECPhotoUn
  };


/* Action routines used by Triple band by pixel decoding */
static void BtoS(), PtoS(), sPtoS(), BtoSb(), btoSb();

/*------------------------------------------------------------------------
------------------- see if we can handle this element --------------------
------------------------------------------------------------------------*/
int miAnalyzeECPhoto(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  xieFloExportClientPhoto *raw = (xieFloExportClientPhoto *) ped->elemRaw;
  int b;

  /* temporary restriction: pixel strides must match */
  for(b = 0; b < ped->outFlo.bands; ++b)
    if(ped->inFloLst->format[b].stride != ped->outFlo.format[b].stride)
      ImplementationError(flo,ped, return(FALSE));

  switch(raw->encodeTechnique) {
  case xieValEncodeUncompressedSingle:
    ped->ddVec = ECPhotoUncomByPlaneVec;
    break;
    
  case xieValEncodeUncompressedTriple:

    switch(((xieTecEncodeUncompressedTriple *) &raw[1])->interleave) {
    case xieValBandByPlane:
      ped->ddVec = ECPhotoUncomByPlaneVec;
      break;

    case xieValBandByPixel:
      if(ped->inFloLst[SRCtag].format[0].stride !=  8  &&
	 ped->inFloLst[SRCtag].format[0].stride != 16  &&
	 ped->inFloLst[SRCtag].format[0].stride != 24  &&
	 ped->inFloLst[SRCtag].format[0].stride != 32 )
	ImplementationError(flo,ped, return(FALSE));
      ped->ddVec = ECPhotoUncomByPixelVec;
      break;
    }
    break;
  default:
    ImplementationError(flo,ped, return(FALSE));
  }

  return(TRUE);
}                               /* end meAnalyzeECPhoto */

/*------------------------------------------------------------------------
---------------------------- create peTex . . . --------------------------
------------------------------------------------------------------------*/
static int CreateECPhotoUncomByPlane(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  /* attach an execution context to the photo element definition */
  return(MakePETex(flo, ped, sizeof(miCPhotoUnRec), NO_SYNC, NO_SYNC));
}                               /* end CreateECPhoto */

/*------------------------------------------------------------------------
---------------------------- create peTex . . . --------------------------
------------------------------------------------------------------------*/
static int CreateECPhotoUncomByPixel(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  /* attach an execution context to the photo element definition */
  return(MakePETex(flo, ped, sizeof(miCPhotoUnRec), NO_SYNC, SYNC));
}                               /* end CreateECPhoto */
/*------------------------------------------------------------------------
---------------------------- initialize peTex . . . ----------------------
------------------------------------------------------------------------*/
static int InitializeECPhotoUn(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  peTexPtr                 pet = ped->peTex;
  xieFloExportClientPhoto *raw = (xieFloExportClientPhoto *)ped->elemRaw;
  miCPhotoUnPtr            pvt = (miCPhotoUnPtr)(pet->private);
  xieTecEncodeUncompressedTriple *tec = 
  				(xieTecEncodeUncompressedTriple *)&raw[1];

  if(pet->receptor->inFlo->bands == 1)
    pvt->bandMap[0] = 0;
  else {
    int s, d;
    /* XXX	should this match the order of the R, G, and B masks for True
     *		and Direct (and some Static) Color Visauls?  (probably not)
     */
    if(((xieTecEncodeUncompressedTriple*)&raw[1])->bandOrder == xieValLSFirst)
      for(d = 0; d < xieValMaxBands; ++d)
	pvt->bandMap[d] = d;
    else 
      for(s = 0, d = xieValMaxBands; d--; ++s)
	pvt->bandMap[s] = d;

    /* If interleaved data going out, set up action structure */
    if(tec->interleave == xieValBandByPixel) {
	miCPhotoActionPtr aa = &(pvt->aarray[0]);
  	bandPtr        sbnd1 = &pet->receptor[SRCtag].band[pvt->bandMap[0]];
  	bandPtr        sbnd2 = &pet->receptor[SRCtag].band[pvt->bandMap[1]];
  	bandPtr        sbnd3 = &pet->receptor[SRCtag].band[pvt->bandMap[2]];
  	CARD32        depth1 = sbnd1->format->depth;
  	CARD32        depth2 = sbnd2->format->depth;
  	CARD32        depth3 = sbnd3->format->depth;
  	CARD32       dstride = tec->pixelStride[0]>>3;
	
	if (depth1 == 16 && depth2 == 16 && depth3 == 16) {
#if (BITMAP_BIT_ORDER == MSBFirst)
	    void (*pa)() = (tec->pixelOrder == xieValMSFirst) ? PtoS : sPtoS;
#else
	    void (*pa)() = (tec->pixelOrder == xieValMSFirst) ? sPtoS : PtoS;
#endif
            for(s = 0; s < xieValMaxBands; s++, aa++) {
	        aa->action    = pa;
  	        aa->Bstride   = dstride;
	        aa->dstoffset = s;
	        aa->mask      = 0; /* Unused */
	        aa->shift     = 0; /* Unused */
		aa->clear_dst = FALSE;
	    }
	} else if (depth1 == 8 && depth2 == 8 && depth3 == 8) {
            for(s = 0; s < xieValMaxBands; s++, aa++) {
	        aa->action    = BtoS;
  	        aa->Bstride   = dstride;
	        aa->dstoffset = s;
	        aa->mask      = 0; /* Unused */
	        aa->shift     = 0; /* Unused */
		aa->clear_dst = FALSE;
	    }
	} else if (depth1 == 4 && depth2 == 4 && depth3 == 4) {
	    if (tec->fillOrder == xieValMSFirst) {
	        aa->action    = BtoSb;
  	        aa->Bstride   = dstride;
	        aa->dstoffset = 0;
	        aa->mask      = 0xf0; 
		aa->clear_dst = FALSE;
	        (aa++)->shift = 4; 
	        aa->action    = BtoSb;
  	        aa->Bstride   = dstride;
	        aa->dstoffset = 0;
	        aa->mask      = 0x0f; 
		aa->clear_dst = FALSE;
	        (aa++)->shift = 0; 
	        aa->action    = BtoSb;
  	        aa->Bstride   = dstride;
	        aa->dstoffset = 1;
	        aa->mask      = 0xf0; 
		aa->clear_dst = FALSE;
	        aa->shift     = 4; 
	    } else { /* xieValLSFirst */
	        aa->action    = BtoSb;
  	        aa->Bstride   = dstride;
	        aa->dstoffset = 0;
		aa->clear_dst = FALSE;
	        aa->mask      = 0x0f; 
	        (aa++)->shift = 0; 
	        aa->action    = BtoSb;
  	        aa->Bstride   = dstride;
	        aa->dstoffset = 0;
	        aa->mask      = 0xf0; 
		aa->clear_dst = FALSE;
	        (aa++)->shift = 4; 
	        aa->action    = BtoSb;
  	        aa->Bstride   = dstride;
	        aa->dstoffset = 1;
	        aa->mask      = 0x0f; 
		aa->clear_dst = FALSE;
	        aa->shift     = 0; 
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
	    aa->action    = (depth1 > 1) ? BtoSb : btoSb;
  	    aa->Bstride   = dstride;
	    aa->dstoffset = 0;
	    aa->mask      = smask1;
	    aa->clear_dst = TRUE;
	    (aa++)->shift = shift1;
	    aa->action    = (depth2 > 1) ? BtoSb : btoSb;
  	    aa->Bstride   = dstride;
	    aa->dstoffset = 0;
	    aa->mask      = smask2;
	    aa->clear_dst = TRUE;
	    (aa++)->shift = shift2; 
	    aa->action    = (depth3 > 1) ? BtoSb : btoSb;
  	    aa->Bstride   = dstride;
	    aa->dstoffset = 0;
	    aa->mask      = smask3;
	    aa->clear_dst = TRUE;
	    aa->shift     = shift3; 
	} else 
      	    ImplementationError(flo,ped, return(FALSE));
    }
  }
  return(InitReceptors(flo, ped, NO_DATAMAP, 1) && 
	 InitEmitter(flo, ped, NO_DATAMAP, NO_INPLACE));
}                               /* end InitializeECPhoto */

/*------------------------------------------------------------------------
----------------------------- crank some data ----------------------------
------------------------------------------------------------------------*/
static int ActivateECPhotoUncomByPlane(flo,ped,pet)
     floDefPtr flo;
     peDefPtr  ped;
     peTexPtr  pet;
{
  xieFloExportClientPhoto *raw = (xieFloExportClientPhoto*)ped->elemRaw;
  receptorPtr  rcp = pet->receptor;
  CARD32     bands = rcp->inFlo->bands;
  bandPtr     sbnd = rcp->band;
  bandPtr     dbnd = pet->emitter;
  CARD8 ready, *src, *dst;
  CARD32 b, dlen, start;
  
  for(b = 0; b < bands; ++sbnd, ++dbnd, ++b) {
    /* get pointer to the initial src data (i.e. beginning of strip) */
    ready = ped->outFlo.ready & 1<<b;
    start = dbnd->current;
    dlen  = dbnd->format->pitch+7>>3;
    src   = GetCurrentSrc(CARD8,flo,pet,sbnd);
    dst   = GetDstBytes(CARD8,flo,pet,dbnd,start,dlen,FALSE);
    
    while(src && dst) {
      bcopy(src,dst,dlen);

      if((src = GetNextSrc(CARD8,flo,pet,sbnd,TRUE)) || !sbnd->final)
	dst = GetDstBytes(CARD8,flo,pet,dbnd,dbnd->current+dlen,dlen,TRUE);
    }
    FreeData(CARD8,flo,pet,sbnd,sbnd->current);
    if(!src && sbnd->final) {
      SetBandFinal(dbnd);
      PutData(flo,pet,dbnd,dbnd->maxGlobal);   /* write the remaining data */
    }
    if(!ready && (ped->outFlo.ready & (1<<b)) &&
       ((raw->notify == xieValFirstData && start == 0) ||
	(raw->notify == xieValNewData)))
      SendExportAvailableEvent(flo,ped,b,0,0,0);
      /* XXX we could use 0 fields to pass extra info?? */
  }

  return(TRUE);
}                               /* end ActivateECPhoto */

/*------------------------------------------------------------------------
----------------------------- crank some data ----------------------------
------------------------------------------------------------------------*/
static int ActivateECPhotoUncomByPixel(flo,ped,pet)
     floDefPtr flo;
     peDefPtr  ped;
     peTexPtr  pet;
{
  xieFloExportClientPhoto *raw = (xieFloExportClientPhoto*)ped->elemRaw;
  miCPhotoUnPtr pvt = (miCPhotoUnPtr)pet->private;
  bandPtr   sb0 = &pet->receptor[SRCtag].band[pvt->bandMap[0]];
  bandPtr   sb1 = &pet->receptor[SRCtag].band[pvt->bandMap[1]];
  bandPtr   sb2 = &pet->receptor[SRCtag].band[pvt->bandMap[2]];
  bandPtr  dbnd = &pet->emitter[0];
  CARD32   dlen = dbnd->format->pitch+7>>3;
  CARD32  width = sb0->format->width;
  miCPhotoActionPtr aap0 = &(pvt->aarray[0]);
  miCPhotoActionPtr aap1 = &(pvt->aarray[1]);
  miCPhotoActionPtr aap2 = &(pvt->aarray[2]);
  BytePixel *sp0, *sp1, *sp2, *dst;
  Bool clear_dst;
  CARD32 start;
  CARD8 ready;

  if((dst = GetSrcBytes(BytePixel,flo,pet,dbnd,dbnd->current,dlen,KEEP)) && 
     (sp0 = GetCurrentDst(BytePixel,flo,pet,sb0)) &&
     (sp1 = GetCurrentDst(BytePixel,flo,pet,sb1)) && 
     (sp2 = GetCurrentDst(BytePixel,flo,pet,sb2)))
    do {
      ready = ped->outFlo.ready & 1;
      start = dbnd->current;

      if (aap0->clear_dst) bzero(dst,(int)dlen);

      (*aap0->action)(sp0,dst,width,aap0);
      (*aap1->action)(sp1,dst,width,aap1);
      (*aap2->action)(sp2,dst,width,aap2);

      dst = GetSrcBytes(BytePixel,flo,pet,dbnd,dbnd->current+dlen,dlen,FLUSH);
      sp0 = GetNextDst(BytePixel,flo,pet,sb0,FLUSH);
      sp1 = GetNextDst(BytePixel,flo,pet,sb1,FLUSH);
      sp2 = GetNextDst(BytePixel,flo,pet,sb2,FLUSH);
    } while(dst && sp0 && sp1 && sp2);

    FreeData(CARD8,flo,pet,sb0,sb0->current);
    FreeData(CARD8,flo,pet,sb1,sb1->current);
    FreeData(CARD8,flo,pet,sb2,sb2->current);
    if(!sp0 && sb0->final && !sp1 && sb1->final && !sp2 && sb2->final) {
      SetBandFinal(dbnd);
      PutData(flo,pet,dbnd,dbnd->maxGlobal);   /* write the remaining data */
    }

    if(!ready && (ped->outFlo.ready & 1) &&
       ((raw->notify == xieValFirstData && start == 0) ||
	(raw->notify == xieValNewData)))
      SendExportAvailableEvent(flo,ped,0,0,0,0);
      /* XXX we could use 0 fields to pass extra info?? */
  return(TRUE);
}                               /* end ActivateECPhotoUncomByPixel */


/*------------------------------------------------------------------------
------------------------ get rid of run-time stuff -----------------------
------------------------------------------------------------------------*/
static int ResetECPhoto(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  ResetReceptors(ped);
  ResetEmitter(ped);
  
  return(TRUE);
}                               /* end ResetECPhoto */

/*------------------------------------------------------------------------
-------------------------- get rid of this element -----------------------
------------------------------------------------------------------------*/
static int DestroyECPhotoUn(flo,ped)
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
}                               /* end DestroyECPhotoUn */

/* Triple Band Byte by Pixel Action routines */

/* Stream bits to bits */
static void btoSb(isrc,dst,count,aap)
BytePixel *isrc, *dst;
CARD32 count;
miCPhotoActionPtr aap;
{
CARD32 Bstride = aap->Bstride;
CARD8     mask = aap->mask;
LogInt	  *src = (LogInt *)isrc;
int i;

	dst += aap->dstoffset;
	for (i = 0; i < count; i++, dst+=Bstride) 
	    if (LOG_tstbit(src,count) != 0)
		*dst |= mask;
}

/* BytePixels to Stream in bits */
static void BtoSb(src,dst,count,aap)
BytePixel *src, *dst;
CARD32 count;
miCPhotoActionPtr aap;
{
CARD32 Bstride = aap->Bstride;
CARD8     mask = aap->mask;
CARD8    shift = aap->shift;
int i;

	dst += aap->dstoffset;
	for (i = 0; i < count; i++, dst+=Bstride) 
		*dst |= *src++ << shift & mask;
}

/* BytePixel to Stream */
static void BtoS(src,dst,count,aap)
BytePixel *src, *dst;
CARD32 count;
miCPhotoActionPtr aap;
{
CARD32 Bstride = aap->Bstride;
int i;

	dst += aap->dstoffset;
	for (i = 0; i < count; i++, dst+=Bstride) 
		*dst = *src++;
}

/* PairPixel (unswapped) to Stream */
static void PtoS(src,idst,count,aap)
PairPixel *src, *idst;
CARD32 count;
miCPhotoActionPtr aap;
{
CARD32 Bstride = aap->Bstride;
CARD8 *dst = ((CARD8 *)idst) + aap->dstoffset;
int i;
	for (i = 0; i < count; i++, dst+=Bstride) 
		*((PairPixel *)dst) = *src++;
}

/* PairPixel (swapped) to Stream */
static void sPtoS(src,idst,count,aap)
PairPixel *src, *idst;
CARD32 count;
miCPhotoActionPtr aap;
{
CARD32 Bstride = aap->Bstride;
CARD8 *dst = ((CARD8 *)idst) + aap->dstoffset;
int i;
	for (i = 0; i < count; i++, dst+=Bstride) {
		PairPixel sval = *src++;
		*((PairPixel *)dst) = (sval >> 8) | (sval << 8);
	}
}
/* end module mecphoto.c */
