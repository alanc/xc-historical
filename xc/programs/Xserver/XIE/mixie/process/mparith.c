/* $XConsortium$ */
/**** module mparith.c ****/
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
  
	mparith.c -- DDXIE prototype Arithmetic element
  
	Robert NC Shelley -- AGE Logic, Inc. May, 1993
  
*****************************************************************************/

#define _XIEC_MPARITH
#define _XIEC_PARITH

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

#include <difloat.h> 	/*   T E M P O R A R Y    H A C K  for blend stuff */

/*
 *  routines referenced by other DDXIE modules
 */
int	miAnalyzeArith();

/*
 *  routines used internal to this module
 */
static int CreateArith();
static int InitializeArith();
static int MonoArith();
static int SplitArith();
static int DyadArith();
static int ResetArith();
static int DestroyArith();

/*
 * DDXIE Arithmetic entry points
 */
static ddElemVecRec ArithVec = {
  CreateArith,
  InitializeArith,
  (xieIntProc)NULL,
  (xieIntProc)NULL,
  ResetArith,
  DestroyArith
  };


/*------------------------------------------------------------------------
------------------- see if we can handle this element --------------------
------------------------------------------------------------------------*/
int miAnalyzeArith(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  xieFloArithmetic *raw = (xieFloArithmetic *)ped->elemRaw;
  inFloPtr inf;
  int b, i, inputs = raw->src2 ? 2 : 1;
  
  /*
   * sorry, but we only handle BYTE_PIXEL class images for now
   */
  for(i = 0; i < inputs; i++)
    for(inf = &ped->inFloLst[i], b = 0; b < inf->bands; b++)
      if(inf->format[b].class != BYTE_PIXEL)
	ImplementationError(flo,ped, return(FALSE));
  
  /* for now just stash our entry point vector in the peDef */
  ped->ddVec = ArithVec;
  ped->ddVec.activate = (inputs == 1 ? inf->bands == 1
			 ? MonoArith : SplitArith : DyadArith);
  /* NOTE: no support for process domain input yet */

  return(TRUE);
}                               /* end miAnalyzeArith */

/*------------------------------------------------------------------------
---------------------------- create peTex . . . --------------------------
------------------------------------------------------------------------*/
static int CreateArith(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  xieFloArithmetic *raw = (xieFloArithmetic *)ped->elemRaw;

  /* attach an execution context to the photo element definition */
  return( MakePETex(flo, ped, NO_PRIVATE, raw->src2 != 0, raw->src2 == 0) );
}                               /* end CreateArith */


/*------------------------------------------------------------------------
---------------------------- initialize peTex . . . ----------------------
------------------------------------------------------------------------*/
static int InitializeArith(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  xieFloArithmetic *raw = (xieFloArithmetic *)ped->elemRaw;
  receptorPtr  rcp = ped->peTex->receptor;
  bandMsk      msk = (bandMsk)raw->bandMask;
  pArithDefPtr pvt = (pArithDefPtr) ped->elemPvt;
  int b;


  if(raw->src2)
    {  /*   T E M P O R A R Y    H A C K  (for blending) */
      pvt->constant[0] = ConvertFromIEEE(raw->constant0);
      pvt->constant[1] = ConvertFromIEEE(raw->constant1);
      pvt->constant[2] = ConvertFromIEEE(raw->constant2);
    }
  for(b = 0; b < ped->inFloLst->bands; ++b) {
    if(pvt->constant[0] < 0.0 || pvt->constant[0] > 1.0)
      ImplementationError(flo,ped, return(FALSE));
  }
  InitReceptor(flo, ped, &rcp[SRCt1], NO_DATAMAP, 1, msk, ~msk);
  if(msk && raw->src2)
    InitReceptor(flo, ped, &rcp[SRCt2], NO_DATAMAP, 1, msk, NO_BANDS);
  if(msk)
    InitEmitter(flo, ped, NO_DATAMAP,
		ped->ddVec.activate == SplitArith ? NO_INPLACE : SRCt1);

  return(!ferrCode(flo));
}                               /* end InitializeArith */


/*------------------------------------------------------------------------
------------------------ crank some single input data --------------------
------------------------------------------------------------------------*/
static int MonoArith(flo,ped,pet)
     floDefPtr flo;
     peDefPtr  ped;
     peTexPtr  pet;
{
  pArithDefPtr pvt = (pArithDefPtr) ped->elemPvt;
  receptorPtr  rcp = pet->receptor;
  bandPtr      sb1 = rcp[SRCt1].band;
  bandPtr      bnd = pet->emitter;
  CARD32     bands = rcp[SRCt1].inFlo->bands;
  BytePixel *sr1, *dst;
  CARD32 i, b;
  
  for(b = 0; b < bands; ++sb1, ++dst, b++) {
    /* get pointers to the initial src and dst scanlines */
    if(!(sr1 = GetCurrentSrc(BytePixel,flo,pet,sb1)) ||
       !(dst = GetCurrentDst(BytePixel,flo,pet,bnd))) continue;

    /* continue while all is well and we have pointers */
    do {
      /* THIS IS WHERE WE ARE SUPPOSE TO DO THE ARITHMETIC OPERATION */
      if(sr1 != dst)
	bcopy(sr1, dst, bnd->pitch);

      /* get pointers to the next src and dst scanlines */
      sr1 = GetNextSrc(BytePixel,flo,pet,sb1,FLUSH);
      dst = GetNextDst(BytePixel,flo,pet,bnd,FLUSH);
    } while(sr1 && dst);
    
    /* make sure the scheduler knows how much src we used */
    FreeData(BytePixel,flo,pet,sb1,sb1->current);
  }
  return(TRUE);
}                               /* end MonoArith */


/*------------------------------------------------------------------------
------------------------ crank some single input data --------------------
------------------------------------------------------------------------*/
static int SplitArith(flo,ped,pet)
     floDefPtr flo;
     peDefPtr  ped;
     peTexPtr  pet;
{
  BytePixel *s0, *s1, *s2, *d0, *d1, *d2;
  pArithDefPtr pvt = (pArithDefPtr) ped->elemPvt;
  receptorPtr  rcp = pet->receptor;
  bandPtr      sb0 = &rcp[SRCt1].band[0];
  bandPtr      sb1 = &rcp[SRCt1].band[1];
  bandPtr      sb2 = &rcp[SRCt1].band[2];
  bandPtr      db0 = &pet->emitter[0];
  bandPtr      db1 = &pet->emitter[1];
  bandPtr      db2 = &pet->emitter[2];
  CARD32     bands = rcp[SRCt1].inFlo->bands;
  CARD32 w0 = pvt->constant[0] * db0->format->width;
  CARD32 w1 = pvt->constant[1] * db1->format->width;
  CARD32 w2 = pvt->constant[2] * db2->format->width;
  
  /* get scanlines pointers to all three input bands and a dst band */
  if(!(s0 = GetCurrentSrc(BytePixel,flo,pet,sb0)) ||
     !(s1 = GetCurrentSrc(BytePixel,flo,pet,sb1)) ||
     !(s2 = GetCurrentSrc(BytePixel,flo,pet,sb2)) ||
     !(d0 = GetCurrentDst(BytePixel,flo,pet,db0)) ||
     !(d1 = GetCurrentDst(BytePixel,flo,pet,db1)) ||
     !(d2 = GetCurrentDst(BytePixel,flo,pet,db2))) return(TRUE);
  
  /* continue while all is well and we have pointers */
  do {
    /* THIS IS WHERE WE ARE SUPPOSE TO DO THE ARITHMETIC OPERATION
     * instead we're gona fool around
     * note: don't try this with bands of differing dimensions!
     */
    bcopy(s0, d0, db0->pitch);
    bcopy(s1, d1, db1->pitch);
    bcopy(s2, d2, db2->pitch);
    bcopy(s0, d1, w0);
    bcopy(s1, d2, w1);
    bcopy(s2, d0, w2);
    
    /* get pointers to the next src and dst scanlines */
    s0 = GetNextSrc(BytePixel,flo,pet,sb0,FLUSH);
    s1 = GetNextSrc(BytePixel,flo,pet,sb1,FLUSH);
    s2 = GetNextSrc(BytePixel,flo,pet,sb2,FLUSH);
    d0 = GetNextDst(BytePixel,flo,pet,db0,FLUSH);
    d1 = GetNextDst(BytePixel,flo,pet,db1,FLUSH);
    d2 = GetNextDst(BytePixel,flo,pet,db2,FLUSH);
  } while(s0 && s1 && s2 && d0 && d1 && d2);
  
  /* make sure the scheduler knows how much src we used */
  FreeData(BytePixel,flo,pet,sb0,sb0->current);
  FreeData(BytePixel,flo,pet,sb1,sb1->current);
  FreeData(BytePixel,flo,pet,sb2,sb2->current);
  
  return(TRUE);
}                               /* end SplitArith */


/*------------------------------------------------------------------------
------------------------- crank some dual input data ---------------------
------------------------------------------------------------------------*/
static int DyadArith(flo,ped,pet)
     floDefPtr flo;
     peDefPtr  ped;
     peTexPtr  pet;
{
  pArithDefPtr pvt = (pArithDefPtr) ped->elemPvt;
  receptorPtr  rcp =  pet->receptor;
  bandPtr      sb1 = &rcp[SRCt1].band[0];
  bandPtr      sb2 = &rcp[SRCt2].band[0];
  bandPtr      bnd = &pet->emitter[0];
  CARD32     bands =  rcp[SRCt1].inFlo->bands;
  CARD32 b, p, r, w;
  Bool  do_right;
  BytePixel *sr1, *sr2, *dst;
  double     con;
  
  for(b = 0; b < bands; ++sb1, ++sb2, ++bnd, b++) {
    /* get pointers to the initial src-1, src-2, and dst scanlines */
    if(!(sr1 = GetCurrentSrc(BytePixel,flo,pet,sb1)) ||
       !(sr2 = GetCurrentSrc(BytePixel,flo,pet,sb2)) ||
       !(dst = GetCurrentDst(BytePixel,flo,pet,bnd))) continue;
    con = pvt->constant[b];
    if(do_right = sb1->format->width > sb2->format->width) {
      w = sb2->format->width;
      r = sizeof(BytePixel) * (sb1->format->width - w);
    } else {
      w = sb1->format->width;
    }
    do {
      /* THIS IS WHERE WE ARE SUPPOSE TO DO THE ARITHMETIC OPERATION
       * we'll do a little blending instead 
       */
      for(p = 0; p < w; ++p)
	*dst++ = (*sr1++ * con + *sr2++ * (1.0 - con));

      if(do_right && sr1 != dst)
        bcopy(sr1, dst, r);

      /* get pointers to the next src-1, src-2, and dst scanlines */
      sr1 = GetNextSrc(BytePixel,flo,pet,sb1,FLUSH);
      sr2 = GetNextSrc(BytePixel,flo,pet,sb2,FLUSH);
      dst = GetNextDst(BytePixel,flo,pet,bnd,FLUSH);
    } while(sr1 && sr2 && dst);

    if(!sr1 && sb1->final)		/* when sr1 runs out, kill sr2 too  */
      DisableSrc(flo,pet,sb2,FLUSH);
    else if(!sr2 && sb2->final)		/* when sr2 runs out, pass-thru sr1 */
      BypassSrc(flo,pet,sb1);
    else { 	/* both inputs still active, keep the scheduler up to date  */
      FreeData(BytePixel,flo,pet,sb1,sb1->current);
      FreeData(BytePixel,flo,pet,sb2,sb2->current);
    }
  }
  return(TRUE);
}                               /* end DyadArith */


/*------------------------------------------------------------------------
------------------------ get rid of run-time stuff -----------------------
------------------------------------------------------------------------*/
static int ResetArith(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  ResetReceptors(ped);
  ResetEmitter(ped);
  
  return(TRUE);
}                               /* end ResetArith */


/*------------------------------------------------------------------------
-------------------------- get rid of this element -----------------------
------------------------------------------------------------------------*/
static int DestroyArith(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  /* get rid of the peTex structure  */
  ped->peTex = (peTexPtr) XieFree(ped->peTex);
  
  /* zap this element's entry point vector */
  ped->ddVec.create     = (xieIntProc)NULL;
  ped->ddVec.initialize = (xieIntProc)NULL;
  ped->ddVec.activate   = (xieIntProc)NULL;
  ped->ddVec.flush      = (xieIntProc)NULL;
  ped->ddVec.reset      = (xieIntProc)NULL;
  ped->ddVec.destroy    = (xieIntProc)NULL;
  
  return(TRUE);
}                               /* end DestroyArith */

/* end module mparith.c */
