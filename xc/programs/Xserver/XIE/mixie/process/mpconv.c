/* $XConsortium$ */
/**** module mpconv.c ****/
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
  
	mpconv.c -- DDXIE convolve element
  
	Dean Verheiden -- AGE Logic, Inc. June, 1993
  
*****************************************************************************/


#define _XIEC_MPCONV
#define _XIEC_PCONV

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
#include <flostr.h>
#include <texstr.h>


/*
 *  routines referenced by other DDXIE modules
 */
int	miAnalyzeConvolve();

/*
 *  routines used internal to this module
 */
static int CreateConvolve();
static int InitializeConvolveConstant();
static int InitializeConvolveReplicate();
static int ActivateConvolveConstant();
static int ActivateConvolveReplicate();
static int FlushConvolve();
static int ResetConvolveConstant();
static int ResetConvolveReplicate();
static int DestroyConvolve();

/*
 * DDXIE Convolve entry points
 */
static ddElemVecRec ConvolveConstantVec = {
  CreateConvolve,
  InitializeConvolveConstant,
  ActivateConvolveConstant,
  FlushConvolve,
  ResetConvolveConstant,
  DestroyConvolve
  };

static ddElemVecRec ConvolveReplicateVec = {
  CreateConvolve,
  InitializeConvolveReplicate,
  ActivateConvolveReplicate,
  FlushConvolve,
  InitializeConvolveReplicate,
  DestroyConvolve
  };

/*------------------------------------------------------------------------
------------------------  fill in the vector  ---------------------------
------------------------------------------------------------------------*/
int miAnalyzeConvolve(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  /* based on the technique, fill in the appropriate entry point vector */
  switch(((xieFloConvolve *)ped->elemRaw)->convolve) {
	case	xieValConvolveDefault:
	case	xieValConvolveConstant:
		ped->ddVec = ConvolveConstantVec;
		break;
	case	xieValConvolveReplicate:
		ped->ddVec = ConvolveReplicateVec;
		break;
  }
  return(TRUE);
}                               /* end miAnalyzeConvolve */


/*------------------------------------------------------------------------
---------------------------- create peTex . . . --------------------------
------------------------------------------------------------------------*/
static int CreateConvolve(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  /* Allocate space for an array of pointers to constant convolution lines */
  return(MakePETex(flo, ped,
                   ped->inFloLst[SRCtag].bands * sizeof(void *),
		   FALSE, FALSE));
}                               /* end CreateConvolve */


/*------------------------------------------------------------------------
---------------------------- initialize peTex . . . ----------------------
------------------------------------------------------------------------*/
static int InitializeConvolveConstant(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
 CARD32   ksize = ((xieFloConvolve *)ped->elemRaw)->kernelSize;
 double *tconst = (double *)ped->techPvt;
 peTexPtr   pet = ped->peTex;
 CARD32  b, ks2 = ksize/2;
 bandPtr    bnd;
 
    if(!(InitReceptors(flo,ped,ksize,ks2+1) &&
         InitEmitter(flo,ped,0,-1)))
      return(FALSE);

    /* Create constant lines for each band */
    for(b = 0; b < ped->inFloLst[SRCtag].bands; b++) {
        bnd = &pet->receptor[SRCtag].band[b];
	 switch (bnd->format->class) {
	 case	UNCONSTRAINED:
		{
 		CARD32 iwidth = bnd->format->width;
		register RealPixel *cline;
		register RealPixel cconst = tconst[b];
		register int i;
		RealPixel **carray = (RealPixel **)pet->private;

		if (!(carray[b] = cline = 
			(RealPixel *)XieMalloc(sizeof(RealPixel) * iwidth)))
			AllocError(flo,ped,return(FALSE));
		
		for (i = 0; i < iwidth; i++) *cline++ = cconst;
		}
	 	break;
	 case	QUAD_PIXEL:
		{
 		CARD32 iwidth = bnd->format->width;
		register QuadPixel *cline;
		register QuadPixel cconst = tconst[b];
		register int i;
		QuadPixel **carray = (QuadPixel **)pet->private;

		if (!(carray[b] = cline = 
			(QuadPixel *)XieMalloc(sizeof(QuadPixel) * iwidth)))
			AllocError(flo,ped,return(FALSE));
		
		for (i = 0; i < iwidth; i++) *cline++ = cconst;
		}
		break;
	 case	PAIR_PIXEL:
		{
 		CARD32 iwidth = bnd->format->width;
		register PairPixel *cline;
		register PairPixel cconst = tconst[b];
		register int i;
		PairPixel **carray = (PairPixel **)pet->private;

		if (!(carray[b] = cline = 
			(PairPixel *)XieMalloc(sizeof(PairPixel) * iwidth)))
			AllocError(flo,ped,return(FALSE));
		
		for (i = 0; i < iwidth; i++) *cline++ = cconst;
		}
		break;
	 case	BYTE_PIXEL:
		{
 		int iwidth = bnd->format->width;
		BytePixel *cline;
		char cconst = tconst[b];
		register int i;
		BytePixel **carray = (BytePixel **)pet->private;

		if (!(carray[b] = cline = 
			(BytePixel *)XieMalloc(sizeof(BytePixel) * iwidth)))
			AllocError(flo,ped,return(FALSE));
		
		memset((char *)cline, cconst, iwidth);
		for (i = 0; i < ks2; i++) bnd->dataMap[i] = carray[b];
		}
		break;
	 case	BIT_PIXEL:
		return(FALSE); /* Hey, fix this */
		break;
	 default:
		return(FALSE);
	 }
    }  
 return(TRUE);
}                               /* end InitializeConvolveConstant */

/*------------------------------------------------------------------------
---------------------------- initialize peTex . . . ----------------------
------------------------------------------------------------------------*/
static int InitializeConvolveReplicate(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
return (FALSE);	/* Not implemented in the SI */
}                               /* end InitializeConvolveReplicate */

/*------------------------------------------------------------------------
----------------------------- crank some data ----------------------------
------------------------------------------------------------------------*/
static int ActivateConvolveConstant(flo,ped,pet)
     floDefPtr flo;
     peDefPtr  ped;
     peTexPtr  pet;
{
  xieFloConvolve *raw = (xieFloConvolve *)ped->elemRaw;
  CARD32 end, ks = raw->kernelSize;
  CARD8    bmask = raw->bandMask;
  double *kernel = (double *)ped->elemPvt;			
  void  **carray = (void **)pet->private;
  bandPtr iband, oband; 
  Bool ok;
  int map,sline,dline,len,b,w,i,j,k, k2 = ks/2;

    for(b = 0; b < ped->inFloLst[SRCtag].bands; b++) 
      if(bmask & 1<<b) {

	iband = &pet->receptor[SRCtag].band[b];
	oband = &pet->emitter[b];
        end   = iband->format->height - ks;
        w     = oband->format->width  - k2;      /* is this a hack Dean? */
        map   = 0;
        sline = iband->current;
        dline = oband->current;
        len   = ks;

	switch (iband->format->class) {
	case	UNCONSTRAINED:
	 	break;
	case	QUAD_PIXEL:
		break;
	case	PAIR_PIXEL:
		break;
	case	BYTE_PIXEL:
		{
		BytePixel *dst, **br = (BytePixel **)iband->dataMap;

		while(!ferrCode(flo)) {
		    /* do special handling at top and bottom of image */
		    if(dline <= k2) {
			len   = iband->threshold;
                        map   = ks - len;
			sline = 0;
			if(dline < k2)
       		            SetBandThreshold(iband, len + 1);
		    }
		    if(dline > end) {
			len = iband->threshold - 1;
			br[len] = carray[b];
       		        SetBandThreshold(iband, len);
 		    }
		    ok  = MapData(flo,pet,iband,map,sline++,len,TRUE);
		    dst = GetDst(BytePixel,flo,pet,oband,dline++,TRUE);
		    if(!ok || !dst) break;

                    for(i = k2; i < w; i++) {
			double count = 0.0;
			for(j = 0 ; j < ks; j++)
			    for(k = -k2; k <= k2; k++)
				count += br[j][i+k] * kernel[j * ks + k + k2];
			if(count < 0.0)	       *dst++ = (BytePixel) 0;
			else if(count > 255.0) *dst++ = (BytePixel) 255;
			else	               *dst++ = (BytePixel) count;
		    }
		}
		FreeData(BytePixel,flo,pet,iband,iband->current); 
		}
		break;
	case	BIT_PIXEL:
		break;
	}
   }
  return(TRUE);
}                               /* end ActivateUnconstrain */


/* Stub for now */
static int ActivateConvolveReplicate(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  return(FALSE);
}

/*------------------------------------------------------------------------
--------------------------- get rid of left overs ------------------------
------------------------------------------------------------------------*/
static int FlushConvolve(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  /* Activate was suppose to do the whole image -- there's nothing to do */
  return(TRUE);
}                               /* end FlushConvolve */


/*------------------------------------------------------------------------
------------------------ get rid of run-time stuff -----------------------
------------------------------------------------------------------------*/
static int ResetConvolveConstant(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  void **carray = (void **)ped->peTex->private;
  int b;

  for(b = 0; b < ped->inFloLst[SRCtag].bands; b++) 
  	if(carray[b]) carray[b] = (void *) XieFree(carray[b]);
  
  ResetReceptors(ped);
  ResetEmitter(ped);

  return(TRUE);
}                               /* end ResetConvolveConstant */

/*------------------------------------------------------------------------
------------------------ get rid of run-time stuff -----------------------
------------------------------------------------------------------------*/
static int ResetConvolveReplicate(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  return(FALSE); /* Not implemented in the SI */
}                               /* end ResetConvolveReplicate */

/*------------------------------------------------------------------------
-------------------------- get rid of this element -----------------------
------------------------------------------------------------------------*/
static int DestroyConvolve(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  /* get rid of the peTex structure  */
  if(ped->peTex)
    ped->peTex = (peTexPtr) XieFree(ped->peTex);

  /* zap this element's entry point vector */
  ped->ddVec.create     = (xieIntProc)NULL;
  ped->ddVec.initialize = (xieIntProc)NULL;
  ped->ddVec.activate   = (xieIntProc)NULL;
  ped->ddVec.flush      = (xieIntProc)NULL;
  ped->ddVec.reset      = (xieIntProc)NULL;
  ped->ddVec.destroy    = (xieIntProc)NULL;

  return(TRUE);
}                               /* end DestroyConvolve */

/* end module mpconv.c */

