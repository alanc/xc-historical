/* $XConsortium$ */
/**** module mpctoi.c ****/
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
  
	mpctoi.c -- DDXIE ConvertToIndex element
  
	Robert NC Shelley -- AGE Logic, Inc. July, 1993
  
*****************************************************************************/

#define _XIEC_MPCTOI
#define _XIEC_PCTOI

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
#include <extnsionst.h>
#include <dixstruct.h>
#include <scrnintstr.h>
#include <colormapst.h>
/*
 *  Server XIE Includes
 */
#include <error.h>
#include <macro.h>
#include <colorlst.h>
#include <element.h>
#include <texstr.h>

extern int AllocColor();  /* in ...server/dix/colormap.c */


/* routines referenced by other DDXIE modules
 */
int	miAnalyzeCvtToInd();

/* routines used internal to this module
 */
static int CreateCtoIAll();

static int InitSingleCtoIAll();
static int InitTripleCtoIAll();

static int DoSingleCtoIAll();
static int DoTripleCtoIAll();

static int ResetCtoIAll();

static int DestroyCtoI();



/* DDXIE ConvertToIndex entry points
 */
static ddElemVecRec mpSingleCtoIAllVec = {
  CreateCtoIAll,
  InitSingleCtoIAll,
  DoSingleCtoIAll,
  (xieIntProc)NULL,
  ResetCtoIAll,
  DestroyCtoI
  };

static ddElemVecRec mpTripleCtoIAllVec = {
  CreateCtoIAll,
  InitTripleCtoIAll,
  DoTripleCtoIAll,
  (xieIntProc)NULL,
  ResetCtoIAll,
  DestroyCtoI
  };


/* Local Declarations.
 */
#define HR  19
#define HG  23
#define HB  17

typedef struct _ctihash {
  CARD32	 rgbVal;
  Pixel		 pixdex;
} ctiHashRec, *ctiHashPtr;

typedef struct _mpctiall {
  xieVoidProc	 action;
  ColormapPtr    cmap;
  int		 clindex;
  Pixel		*pixLst;
  CARD32	 pixCnt;
  CARD32	 pixBad;
  CARD32	 width;
  CARD32	 fill;
  CARD32	 mask[xieValMaxBands];
  CARD32         mult[xieValMaxBands];
  float		 coef[xieValMaxBands];
  void          *hitLst[xieValMaxBands];
  CARD8 	 truncate[xieValMaxBands];
  BOOL		 singleMap;
  Bool		 hashing;
  CARD32         slop;
} ctiAllRec, *ctiAllPtr;


/* AllocAll action routines
 */
#define     CtoI_1bb     (xieVoidProc)NULL
#define     CtoI_1bB     (xieVoidProc)NULL
#define     CtoI_1bP     (xieVoidProc)NULL
#define     CtoI_1Bb     (xieVoidProc)NULL
static void CtoI_1BB();
static void CtoI_1BP();
#define     CtoI_1Pb     (xieVoidProc)NULL
static void CtoI_1PB();
static void CtoI_1PP();

static void (*single_action[3][3])() = {
  CtoI_1bb, CtoI_1Bb, CtoI_1Pb,         /* out=1, in=1..3 */
  CtoI_1bB, CtoI_1BB, CtoI_1PB,         /* out=2, in=1..3 */
  CtoI_1bP, CtoI_1BP, CtoI_1PP,         /* out=3, in=1..3 */
};

static void CtoI_31BBB();
static void CtoI_31BHB();
#define     CtoI_33BBB   (xieVoidProc)NULL
#define     CtoI_33BHB   (xieVoidProc)NULL

static void (*triple_action[2][2])() = {
  CtoI_31BHB, CtoI_31BBB,   /* singleMap (bytes in/out), hashing..!hashing */
  CtoI_33BHB, CtoI_33BBB,   /* tripleMap (bytes in/out), hashing..!hashing */
};

/*------------------------------------------------------------------------
------------------- see if we can handle this element --------------------
------------------------------------------------------------------------*/
int miAnalyzeCvtToInd(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  xieFloConvertToIndex *raw = (xieFloConvertToIndex *)ped->elemRaw;
  pCtoIDefPtr dix = (pCtoIDefPtr) ped->elemPvt;
  formatPtr   fmt = &ped->outFlo.format[0];
  int      nbands = ped->inFloLst[SRCtag].bands;

  /* alpha release of SI only required to support 1 and 8 bit drawables :-)
   */
  if(fmt->depth != 1 && fmt->depth != 8)
    ImplementationError(flo,ped, return(FALSE));

  /* stash the appropriate entry point vector in the peDef
   */
  switch(raw->colorAlloc) {
  case xieValColorAllocAll :
    ped->ddVec = nbands == 1 ? mpSingleCtoIAllVec   : mpTripleCtoIAllVec;
    break;
    
  case xieValColorAllocMatch : /* delayed until beta */
  case xieValColorAllocRequantize :	/* beyond SI */
  default : ImplementationError(flo,ped, return(FALSE));
  }
  return(TRUE);
}                               /* end miAnalyzeCvtToInd */

/*------------------------------------------------------------------------
---------------------------- create peTex . . . --------------------------
------------------------------------------------------------------------*/
static int CreateCtoIAll(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  int nbands = ped->inFloLst[SRCtag].bands;

  return(MakePETex(flo, ped, sizeof(ctiAllRec), NO_SYNC, nbands == 3));
}                               /* end CreateCtoIAll */


/*------------------------------------------------------------------------
---------------------------- initialize peTex . . . ----------------------
------------------------------------------------------------------------*/
static int InitSingleCtoIAll(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  xieFloConvertToIndex *raw = (xieFloConvertToIndex *)ped->elemRaw;
  xieTecColorAllocAll  *tec = (xieTecColorAllocAll *) &raw[1];
  peTexPtr    pet = ped->peTex;
  formatPtr   ift = &ped->inFloLst[SRCtag].format[0];
  formatPtr   oft = &ped->outFlo.format[0];
  pCtoIDefPtr dix = (pCtoIDefPtr) ped->elemPvt;
  ctiAllPtr   ddx = (ctiAllPtr)   pet->private;
  CARD32 length, ic = ift->class, oc = oft->class;

  /* init color allocation params
   */
  ddx->cmap        = dix->cmap;
  ddx->clindex     = dix->list->clindex;
  ddx->fill	   = tec->fill;
  ddx->width       = oft->width;
  ddx->singleMap   = dix->class <= PseudoColor;
  ddx->pixBad      = 0;
  ddx->pixCnt      = 0;
  if(!(ddx->pixLst = (Pixel*) XieMalloc(dix->cells * sizeof(Pixel))))
    AllocError(flo,ped, return(FALSE));

  /* init pixel to color cell conversion parameters
   */
  ddx->truncate[0] = (ift[0].depth > dix->stride ?
		      ift[0].depth - dix->stride : 0);
  ddx->mask[0]     = (1 << ift->depth - ddx->truncate[0]) - 1;
  ddx->coef[0]     = 65535.0 / ((ift->levels >> ddx->truncate[0]) - 1);
  ddx->action      = single_action[oc-1][ic-1];
  if(!ddx->action)
    ImplementationError(flo,ped, return(FALSE));

  /* initialize a pixel "hit list"
   */
  length = (ddx->mask[0] + 1) * sizeof(Pixel);
  if(!(ddx->hitLst[0] = (void *)XieMalloc(length)))
    AllocError(flo,ped, return(FALSE));
  memset((char *)ddx->hitLst[0], ~0, length);

  return(InitReceptors(flo, ped, NO_DATAMAP, 1) &&
         InitEmitter(flo, ped, NO_DATAMAP, NO_INPLACE));
}                               /* end InitSingleCtoIAll */

static int InitTripleCtoIAll(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  xieFloConvertToIndex *raw = (xieFloConvertToIndex *)ped->elemRaw;
  xieTecColorAllocAll  *tec = (xieTecColorAllocAll *) &raw[1];
  peTexPtr    pet =  ped->peTex;
  formatPtr   ift = &ped->inFloLst[SRCtag].format[0];
  formatPtr   oft = &ped->outFlo.format[0];
  pCtoIDefPtr dix = (pCtoIDefPtr) ped->elemPvt;
  ctiAllPtr   ddx = (ctiAllPtr)   pet->private;
  CARD32  b, levels, length;

  /* init color allocation params
   */
  ddx->cmap        = dix->cmap;
  ddx->clindex     = dix->list->clindex;
  ddx->fill	   = tec->fill;
  ddx->width       = oft->width;
  ddx->singleMap   = dix->class <= PseudoColor;
  ddx->pixBad      = 0;
  ddx->pixCnt      = 0;
  if(!(ddx->pixLst = (Pixel*) XieMalloc(dix->cells * sizeof(Pixel))))
    AllocError(flo,ped, return(FALSE));

  /* init per-band pixel to color cell conversion parameters
   */
  for(levels = 1, b = 0; b < xieValMaxBands; ++b) {
    ddx->truncate[b] = (ift[b].depth > dix->stride ?
			ift[b].depth - dix->stride : 0);
    ddx->mask[b] = (1<<ift[b].depth - ddx->truncate[b]) - 1;
    ddx->coef[b] = 65535.0 / ((ift[b].levels>>ddx->truncate[b])-1);
    ddx->mult[b] = b == 0 ? 1 : ddx->mult[b-1] * ift[b-1].levels;
    if(ddx->singleMap)
      levels *= ift[b].levels;
    else
      levels  = max(levels, ift[b].levels);
  }
  /* init a "hit list"; if levels from above is too big, we will be hashing
   */
  if(ddx->hashing = levels > 256) {
    ddx->slop = dix->cells;
    length = ((HR+HG+HB) * dix->cells + ddx->slop) * sizeof(ctiHashRec);
  } else {
    length = min(1<<dix->stride,levels) * sizeof(Pixel);
  }
  for(b = 0; b < (ddx->singleMap ? 1 : 3); ++b) {
    if(!(ddx->hitLst[b] = (void*)XieMalloc(length)))
      AllocError(flo,ped, return(FALSE));
    memset((char*)ddx->hitLst[b], ddx->hashing ? 0 : ~0, length);
  }
  /* let's hope we have an action routine that can do what we need
   */
  ddx->action = triple_action[ddx->singleMap ? 0 : 1][ddx->hashing ? 0 : 1];
  if(!ddx->action)
    ImplementationError(flo,ped, return(FALSE));

  return(InitReceptors(flo, ped, NO_DATAMAP, 1) &&
         InitEmitter(flo, ped, NO_DATAMAP, NO_INPLACE));
}                               /* end InitTripleCtoIAll */


/*------------------------------------------------------------------------
------------------------- crank some input data --------------------------
------------------------------------------------------------------------*/
static int DoSingleCtoIAll(flo,ped,pet)
     floDefPtr flo;
     peDefPtr  ped;
     peTexPtr  pet;
{
  xieFloConvertToIndex *raw = (xieFloConvertToIndex *)ped->elemRaw;
  ctiAllPtr ddx = (ctiAllPtr)   pet->private;
  bandPtr iband = &pet->receptor[SRCtag].band[0];
  bandPtr oband = &pet->emitter[0];
  void   *ivoid, *ovoid;
  
  if(Resumed(pet) &&
     ddx->cmap != (ColormapPtr) LookupIDByType(raw->colormap, RT_COLORMAP))
    ColormapError(flo,ped,raw->colormap, return(FALSE));

  if((ivoid = GetCurrentSrc(void,flo,pet,iband)) &&
     (ovoid = GetCurrentDst(void,flo,pet,oband)))
  do {
    (*ddx->action)(ddx, ovoid, ivoid);

    ivoid = GetNextSrc(void,flo,pet,iband,FLUSH);
    ovoid = GetNextDst(void,flo,pet,oband,FLUSH);
  } while(ivoid && ovoid);

  FreeData(void,flo,pet,iband,iband->current);

  return(TRUE);
}                               /* end DoSingleCtoIAll */


/*------------------------------------------------------------------------
------------------------- crank some input data --------------------------
------------------------------------------------------------------------*/
static int DoTripleCtoIAll(flo,ped,pet)
     floDefPtr flo;
     peDefPtr  ped;
     peTexPtr  pet;
{
  xieFloConvertToIndex *raw = (xieFloConvertToIndex *)ped->elemRaw;
  ctiAllPtr ddx = (ctiAllPtr)   pet->private;
  bandPtr iband = &pet->receptor[SRCtag].band[0];
  bandPtr oband = &pet->emitter[0];
  void *ovoid, *ivoid0, *ivoid1, *ivoid2;
  
  if(Resumed(pet) &&
     ddx->cmap != (ColormapPtr) LookupIDByType(raw->colormap, RT_COLORMAP))
    ColormapError(flo,ped,raw->colormap, return(FALSE));

  ovoid  = GetCurrentDst(void,flo,pet,oband);
  ivoid0 = GetCurrentSrc(void,flo,pet,iband); iband++;
  ivoid1 = GetCurrentSrc(void,flo,pet,iband); iband++;
  ivoid2 = GetCurrentSrc(void,flo,pet,iband); iband -= 2;
  
  while(ovoid && ivoid0 && ivoid1 && ivoid2) {
    
    (*ddx->action)(ddx, ovoid, ivoid0, ivoid1, ivoid2);

    ovoid  = GetNextDst(void,flo,pet,oband,FLUSH);
    ivoid0 = GetNextSrc(void,flo,pet,iband,FLUSH); iband++;
    ivoid1 = GetNextSrc(void,flo,pet,iband,FLUSH); iband++;
    ivoid2 = GetNextSrc(void,flo,pet,iband,FLUSH); iband -= 2;
  }
  FreeData(void,flo,pet,iband,iband->current); iband++;
  FreeData(void,flo,pet,iband,iband->current); iband++;
  FreeData(void,flo,pet,iband,iband->current);
  
  return(TRUE);
}                               /* end DoTripleCtoIAll */


/*------------------------------------------------------------------------
------------------------ get rid of run-time stuff -----------------------
------------------------------------------------------------------------*/
static int ResetCtoIAll(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  xieFloConvertToIndex *raw = (xieFloConvertToIndex *)ped->elemRaw;
  pCtoIDefPtr  dix = (pCtoIDefPtr) ped->elemPvt;
  ctiAllPtr    ddx = (ctiAllPtr)   ped->peTex->private;
  colorListPtr lst = dix->list;
  CARD32       b, i;
  /*
   * give the list to the ColorList resource
   * if we've got pixels, they might need to be transfered into the list
   */
  if((lst->cellPtr = ddx->pixLst) && (lst->cellCnt = ddx->pixCnt)) {

    if(ddx->hashing) {
      ctiHashPtr hash = (ctiHashPtr) ddx->hitLst[0];

      for(i = 0; i < ddx->pixCnt; ++hash)
        if(hash->rgbVal)
          lst->cellPtr[i++] = hash->pixdex;

    } else if(ddx->singleMap) {
      Pixel p, *ppix = (Pixel*) ddx->hitLst[0];

      for(i = 0; i < ddx->pixCnt; ++ppix)
        if((long int)(p = *ppix) >= 0)
          lst->cellPtr[i++] = p;

    }/* else tripleMap and pixels are already in place (if there was code) */
  }
  if(ddx->pixBad && raw->notify && !ferrCode(flo) && !flo->flags.aborted)
    SendColorAllocEvent(flo, ped, lst->mapID, raw->colorAlloc, ddx->pixBad);
  
  for(b = 0; b < (ddx->singleMap ? 1 : 3); ++b)
    if(ddx->hitLst[b])
       ddx->hitLst[b] = (void *) XieFree(ddx->hitLst[b]);

  ddx->pixLst = NULL;  
  ddx->pixCnt = 0;
  ddx->pixBad = 0;
  ResetReceptors(ped);
  ResetEmitter(ped);
  
  return(TRUE);
}                               /* end ResetCtoIAll */


/*------------------------------------------------------------------------
-------------------------- get rid of this element -----------------------
------------------------------------------------------------------------*/
static int DestroyCtoI(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  /* get rid of the peTex structure  */
  ped->peTex = (peTexPtr) XieFree(ped->peTex);
  
  /* zap this element's entry point vector */
  ped->ddVec.create     = (xieIntProc)NULL;
  ped->ddVec.initialize = (xieIntProc)NULL;
  ped->ddVec.activate   = (xieIntProc)NULL;
  ped->ddVec.reset      = (xieIntProc)NULL;
  ped->ddVec.destroy    = (xieIntProc)NULL;
  
  return(TRUE);
}                               /* end DestroyCtoI */


/*------------------------------------------------------------------------
------------- action routines: macro-ize for BB, BP, PB, PP  -------------
------------- b?, ?b not yet implemented; Q?, ?Q not allowed -------------
------------------------------------------------------------------------*/
#define DO_SINGLE_CTOI_ALL(fn_do,itype,otype)				      \
static void fn_do(ddx, DST, SRC)  					      \
  ctiAllPtr ddx; void *DST,*SRC; 					      \
{									      \
  itype *src = (itype *)SRC;						      \
  otype *dst = (otype *)DST;						      \
  Pixel  pv, *pp, *lst = ddx->hitLst[0];				      \
  CARD32 w, mask = ddx->mask[0], trim = ddx->truncate[0];		      \
  CARD16 r, g, b, val;							      \
  for(w = ddx->width; w--; *dst++ = pv) {				      \
    if((long int)(pv = *(pp = &lst[(val = *src++ >> trim & mask)])) < 0) {    \
      r = g = b = (unsigned short)((float)val * ddx->coef[0]);		      \
      if(!ddx->pixBad) {				        	      \
	if(AllocColor(ddx->cmap, &r, &g, &b, pp, ddx->clindex) == Success) {  \
	  ++ddx->pixCnt;						      \
	  pv = *pp;							      \
	  continue;							      \
	}								      \
	++ddx->pixBad;							      \
      }									      \
      pv = ddx->fill;							      \
    }									      \
  }									      \
}

DO_SINGLE_CTOI_ALL(CtoI_1BB, BytePixel, BytePixel)
DO_SINGLE_CTOI_ALL(CtoI_1PB, PairPixel, BytePixel)
DO_SINGLE_CTOI_ALL(CtoI_1BP, BytePixel, PairPixel)
DO_SINGLE_CTOI_ALL(CtoI_1PP, PairPixel, PairPixel)


static void CtoI_31BBB(ddx, DST, SRCR, SRCG, SRCB)
       ctiAllPtr ddx; void *DST,*SRCR,*SRCG,*SRCB;
{
  BytePixel *srcR = (BytePixel *)SRCR;
  BytePixel *srcG = (BytePixel *)SRCG;
  BytePixel *srcB = (BytePixel *)SRCB;
  BytePixel *dst  = (BytePixel *)DST;
  CARD32    Rmask = ddx->mask[0], Rtrim = ddx->truncate[0];
  CARD32    Gmask = ddx->mask[1], Gtrim = ddx->truncate[1];
  CARD32    Bmask = ddx->mask[2], Btrim = ddx->truncate[2];
  CARD32    Gmult = ddx->mult[1], Bmult = ddx->mult[2];
  Pixel     pv, *pp, *lst = (Pixel*)ddx->hitLst[0];
  CARD32    val, rv, gv, bv, w;
  CARD16    r, g, b;

  for(w = ddx->width; w--; *dst++ = pv) {
    rv  = *srcR++ >> Rtrim & Rmask;
    gv  = *srcG++ >> Gtrim & Gmask;
    bv  = *srcB++ >> Btrim & Bmask;
    /*
     * first see if we already have what we're looking for
     */
    if((long int)(pv = *(pp = &lst[val = rv + gv * Gmult + bv * Bmult])) < 0) {
      /*
       * good grief, we have to go begging
       */
      if(!ddx->pixBad) {
	r = (unsigned short)((float)rv * ddx->coef[0]);
	g = (unsigned short)((float)gv * ddx->coef[1]);
	b = (unsigned short)((float)bv * ddx->coef[2]);
	if(AllocColor(ddx->cmap, &r, &g, &b, pp, ddx->clindex) == Success) {
	  ++ddx->pixCnt;
	  pv = *pp;
	  continue;
	}
	++ddx->pixBad;
      }
      pv = ddx->fill;
    }
  }
}
static void CtoI_31BHB(ddx, DST, SRCR, SRCG, SRCB)
       ctiAllPtr ddx; void *DST,*SRCR,*SRCG,*SRCB;
{
  BytePixel *srcR = (BytePixel *)SRCR;
  BytePixel *srcG = (BytePixel *)SRCG;
  BytePixel *srcB = (BytePixel *)SRCB;
  BytePixel *dst  = (BytePixel *)DST;
  CARD32    Rmask = ddx->mask[0], Rtrim = ddx->truncate[0];
  CARD32    Gmask = ddx->mask[1], Gtrim = ddx->truncate[1];
  CARD32    Bmask = ddx->mask[2], Btrim = ddx->truncate[2];
  ctiHashPtr lst  = (ctiHashPtr)ddx->hitLst[0];
  ctiHashPtr hit;
  CARD32    w = ddx->width, slop = ddx->slop;
  CARD16    r, g, b;
  union{ CARD32 rgbGlob; CARD8 rgbVals[4]; } rgb;

  rgb.rgbVals[3] = 1;
  while(w--) {
    hit = &lst[HR * (rgb.rgbVals[0] = *srcR++ >> Rtrim & Rmask) +
	       HG * (rgb.rgbVals[1] = *srcG++ >> Gtrim & Gmask) +
	       HB * (rgb.rgbVals[2] = *srcB++ >> Btrim & Bmask) + slop];

    while(hit->rgbVal && hit->rgbVal != rgb.rgbGlob) --hit;

    if(hit->rgbVal) {
      *dst++ = hit->pixdex;
      continue;
    }
    if(!ddx->pixBad) {
      r = (unsigned short)((float)rgb.rgbVals[0] * ddx->coef[0]);
      g = (unsigned short)((float)rgb.rgbVals[1] * ddx->coef[1]);
      b = (unsigned short)((float)rgb.rgbVals[2] * ddx->coef[2]);

      if(AllocColor(ddx->cmap,&r,&g,&b,&hit->pixdex,ddx->clindex) == Success) {
        ++ddx->pixCnt;
        *dst++ = hit->pixdex;
	hit->rgbVal = rgb.rgbGlob;
        continue;
      }
      ++ddx->pixBad;
    }
    *dst++ = ddx->fill;
  }
}

/* end module mpctoi.c */
