/* $XConsortium$ */
/**** module mpdither.c ****/
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
  
	mpdither.c -- DDXIE dither element
  
	Larry Hare -- AGE Logic, Inc. May, 1993
  
*****************************************************************************/


#define _XIEC_MPDITHER
#define _XIEC_PDITHER


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

typedef RealPixel DitherFloat; /* XXX ??? Double or Float, need servermd.h */

typedef struct _mpditherEDdef {
  void		(*action) ();
  DitherFloat	*previous;
  DitherFloat	*current;
  DitherFloat	range;
  DitherFloat	round;
  CARD32	levels;

} mpDitherEDDefRec, *mpDitherEDDefPtr;

/*
 *  routines referenced by other DDXIE modules
 */
int	miAnalyzeDither();

/*
 *  routines used internal to this module
 */
static int CreateDitherErrorDiffusion(), CreateDitherOrdered();
static int InitializeDitherErrorDiffusion(), InitializeDitherOrdered();
static int ActivateDitherErrorDiffusion(), ActivateDitherOrdered();
static int FlushDither();
static int ResetDitherErrorDiffusion(), ResetDitherOrdered();
static int DestroyDither();

static void DoDitherQb(),DoDitherPb(), DoDitherBb(), DoDitherbb();
static void DoDitherQB(),DoDitherPB(), DoDitherBB();
static void DoDitherQP(),DoDitherPP();
static void DoDitherQQ();

/*
 * DDXIE Dither entry points
 */
/* static		Testing Hack.  See mpcnst.c */
ddElemVecRec DitherErrorDiffusion = {
	CreateDitherErrorDiffusion,
	InitializeDitherErrorDiffusion,
	ActivateDitherErrorDiffusion,
	FlushDither,
	ResetDitherErrorDiffusion,
	DestroyDither
};

static ddElemVecRec DitherOrdered = {
	CreateDitherOrdered,
	InitializeDitherOrdered,
	ActivateDitherOrdered,
	FlushDither,
	ResetDitherOrdered,
	DestroyDither
};

/*------------------------------------------------------------------------
------------------------  fill in the vector  ---------------------------
------------------------------------------------------------------------*/
int miAnalyzeDither(flo,ped)
    floDefPtr flo;
    peDefPtr  ped;
{

    switch(((xieFloDither *)ped->elemRaw)->dither) {
	default: /* Fall Thru - XXX will probably change though. */
	case	xieValDitherErrorDiffusion:
		ped->ddVec = DitherErrorDiffusion;
		break;
	case	xieValDitherOrdered:
		ped->ddVec = DitherOrdered;
		break;
    }

    return TRUE;
}

/*------------------------------------------------------------------------
---------------------------- create peTex . . . --------------------------
------------------------------------------------------------------------*/
static int CreateDitherErrorDiffusion(flo,ped)
    floDefPtr flo;
    peDefPtr  ped;
{
    int auxsize = xieValMaxBands * sizeof(mpDitherEDDefRec);

    return MakePETex(flo,ped,auxsize,FALSE,FALSE);
}

static int CreateDitherOrdered(flo,ped)
    floDefPtr flo;
    peDefPtr  ped;
{
    return MakePETex(flo,ped,0,FALSE,FALSE);
}

/*------------------------------------------------------------------------
---------------------------- initialize peTex . . . ----------------------
------------------------------------------------------------------------*/
static int InitializeDitherErrorDiffusion(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
    peTexPtr  pet = ped->peTex;
    mpDitherEDDefPtr pvt = (mpDitherEDDefPtr) (ped->peTex->private);
    xieFloDither *elem = (xieFloDither *) (ped->elemRaw);
    bandPtr oband;
    bandPtr iband;
    int band, nbands, status;

    status = InitReceptors(flo,ped,0,1) && InitEmitter(flo,ped,0,-1);

    /* ped->flags.modified = FALSE; */
    bzero (pvt, xieValMaxBands * sizeof(mpDitherEDDefRec));

    (pvt+0)->levels = elem->level0;
    (pvt+1)->levels = elem->level1;
    (pvt+2)->levels = elem->level2;

    oband = &(pet->emitter[0]);
    iband = &(pet->receptor[SRCtag].band[0]);
    nbands = pet->receptor[SRCtag].inFlo->bands;
    for (band = 0 ; band < nbands ; band++, pvt++, iband++, oband++) {
	int auxsize = (iband->format->width + 2) * sizeof(DitherFloat);

	switch (oband->format->class) {
	case	QUAD_PIXEL:
		switch (iband->format->class) {
		case	QUAD_PIXEL:  pvt->action = DoDitherQQ; break;
		default: ImplementationError(flo, ped, return(FALSE));
		}
		break;
	case	PAIR_PIXEL:
		switch (iband->format->class) {
		case	QUAD_PIXEL:  pvt->action = DoDitherQP; break;
		case	PAIR_PIXEL:  pvt->action = DoDitherPP; break;
		default: ImplementationError(flo, ped, return(FALSE));
		}
		break;
	case	BYTE_PIXEL:
		switch (iband->format->class) {
		case	QUAD_PIXEL:  pvt->action = DoDitherQB; break;
		case	PAIR_PIXEL:  pvt->action = DoDitherPB; break;
		case	BYTE_PIXEL:  pvt->action = DoDitherBB; break;
		default: ImplementationError(flo, ped, return(FALSE));
		}
		break;
	case	BIT_PIXEL:
		switch (iband->format->class) {
		case	QUAD_PIXEL:  pvt->action = DoDitherQb; break;
		case	PAIR_PIXEL:  pvt->action = DoDitherPb; break;
		case	BYTE_PIXEL:  pvt->action = DoDitherBb; break;
		case	BIT_PIXEL:   pvt->action = DoDitherbb; break;
		default: ImplementationError(flo, ped, return(FALSE));
		}
		break;
	default:
		ImplementationError(flo, ped, return(FALSE));
		break;
	}
	pvt->range = (iband->format->levels - 1) / (pvt->levels - 1);
	pvt->round = pvt->range/((DitherFloat) 2.);

	/* Use XieCalloc to force DitherFloat's to 0.0 */
	pvt->previous = (DitherFloat *) XieCalloc(auxsize);
	pvt->current  = (DitherFloat *) XieCalloc(auxsize);
	if ((!pvt->previous) || (!pvt->current)) {
	    for ( ; band >= 0 ; band--, pvt--) {
		pvt->previous = (DitherFloat *) XieFree(pvt->previous);
		pvt->current  = (DitherFloat *) XieFree(pvt->current);
	    }
	    AllocError(flo, ped, return(FALSE));
	}
    }
    return status;
}

static int InitializeDitherOrdered(flo,ped)
    floDefPtr flo;
    peDefPtr  ped;
{
    /* ped->flags.modified = FALSE; XXX */
    return InitReceptors(flo,ped,0,1) && InitEmitter(flo,ped,0,-1);
}

/*------------------------------------------------------------------------
----------------------------- crank some data ----------------------------
------------------------------------------------------------------------*/


static int ActivateDitherErrorDiffusion(flo,ped,pet)
    floDefPtr flo;
    peDefPtr  ped;
    peTexPtr  pet;
{
    mpDitherEDDefPtr pvt = (mpDitherEDDefPtr) pet->private;
    bandPtr oband = &(pet->emitter[0]);
    bandPtr iband = &(pet->receptor[SRCtag].band[0]);
    int band, nbands = pet->receptor[SRCtag].inFlo->bands;

    for(band = 0; band < nbands; band++, pvt++, iband++, oband++) {
	register int bw = iband->format->width;
	register void *inp, *outp;

	if (!(inp  = GetCurrentSrc(void,flo,pet,iband)) ||
	    !(outp = GetCurrentDst(void,flo,pet,oband))) continue;

	do {
		(*(pvt->action)) (inp, outp, bw, pvt);

		{   /* Swap error buffers */
		    DitherFloat *curr = pvt->current;
		    curr = pvt->current;
		    pvt->current  = pvt->previous;
		    pvt->previous = curr;
		}
		inp  = GetNextSrc(void,flo,pet,iband,TRUE);
		outp = GetNextDst(void,flo,pet,oband,TRUE);

	} while (!ferrCode(flo) && inp && outp) ;

	FreeData(void, flo, pet, iband, iband->current);
    }
    return TRUE;
}

/*------------------------------------------------------------------------
----------------------------- crank some data ----------------------------
------------------------------------------------------------------------*/
static int ActivateDitherOrdered(flo,ped,pet)
    floDefPtr flo;
    peDefPtr  ped;
    peTexPtr  pet;
{
    return FALSE;	/* XXX ??? STUB CITY ??? XXX */
}


/*------------------------------------------------------------------------
--------------------------- get rid of left overs ------------------------
------------------------------------------------------------------------*/
static int FlushDither(flo,ped)
    floDefPtr flo;
    peDefPtr  ped;
{
    return TRUE;
}

/*------------------------------------------------------------------------
------------------------ get rid of run-time stuff -----------------------
------------------------------------------------------------------------*/
static int ResetDitherErrorDiffusion(flo,ped)
    floDefPtr flo;
    peDefPtr  ped;
{
    mpDitherEDDefPtr pvt = (mpDitherEDDefPtr) ped->peTex->private;
    int band;

    /* free any dynamic private data */
    for (band = 0 ; band < xieValMaxBands ; band++, pvt++) {
	pvt->previous = (DitherFloat *) XieFree(pvt->previous);
	pvt->current  = (DitherFloat *) XieFree(pvt->current);
    }

    ResetReceptors(ped);
    ResetEmitter(ped);
    return TRUE;
}

static int ResetDitherOrdered(flo,ped)
    floDefPtr flo;
    peDefPtr  ped;
{
    ResetReceptors(ped);
    ResetEmitter(ped);
    return TRUE;
}

/*------------------------------------------------------------------------
-------------------------- get rid of this element -----------------------
------------------------------------------------------------------------*/
static int DestroyDither(flo,ped)
    floDefPtr flo;
    peDefPtr  ped;
{
    if(ped->peTex)
	ped->peTex = (peTexPtr) XieFree(ped->peTex);

    /* zap this element's entry point vector */
    ped->ddVec.create = (xieIntProc)NULL;
    ped->ddVec.initialize = (xieIntProc)NULL;
    ped->ddVec.activate = (xieIntProc)NULL;
    ped->ddVec.flush = (xieIntProc)NULL;
    ped->ddVec.reset = (xieIntProc)NULL;
    ped->ddVec.destroy = (xieIntProc)NULL;

    return TRUE;
}

/*------------------------------------------------------------------------
------------------------------- Macro Mania ------------------------------
------------------------------------------------------------------------*/

/*
** The only legal bit to bit dither is 2 levels to 1 level.
*/

static void DoDitherbb(inp,outp,bw,pvt)
	void *inp; void *outp; int bw; mpDitherEDDefPtr pvt;
{
	bzero((char *)outp, (bw+7)>>3);
}

/*
** Quads, Pairs, and Bytes can be dithered to bits.  Since this code is
** so slow anyway, don't get too excited about optimizing the bit output.
*/

#define MakeDitherBit(fn_name,itype,otype)				\
static void fn_name(INP,OUTP,bw,pvt) 					\
	void *INP; void *OUTP; int bw; mpDitherEDDefPtr pvt;		\
{									\
	register itype *inp = (itype *) INP;				\
	register LogInt *outp = (LogInt *) OUTP;			\
	register unsigned int actual;					\
	register DitherFloat range = pvt->range;			\
	register DitherFloat round = pvt->round;			\
	register DitherFloat *prev = pvt->previous;			\
	register DitherFloat *curr = pvt->current;			\
	register DitherFloat range1over = 1./range;			\
	register DitherFloat current = *curr, desire;			\
	register int ix;						\
	bzero((char *)outp, (bw+7)>>3);					\
	for (ix = 0; ix < bw; ix++) {					\
	    desire = inp[ix] + ((DitherFloat) .4375 * current) +	\
			    ((DitherFloat) .0625 * *(prev+0)) +		\
			    ((DitherFloat) .3125 * *(prev+1)) +		\
			    ((DitherFloat) .1875 * *(prev+2)) ;		\
	    prev++;							\
	    actual = (desire+round)*range1over;				\
	    if (actual >= 1) LOG_setbit(outp,ix);			\
	    *++curr = current = (desire - actual*range);		\
	}								\
}

/*
** All the other multitudes of combinations.
*/

#define MakeDither(fn_name,itype,otype)					\
static void fn_name(INP,OUTP,bw,pvt) 					\
	void *INP; void *OUTP; int bw; mpDitherEDDefPtr pvt;		\
{									\
	register itype *inp = (itype *) INP;				\
	register otype *outp = (otype *) OUTP, actual;			\
	register DitherFloat range = pvt->range;			\
	register DitherFloat round = pvt->round;			\
	register DitherFloat *prev = pvt->previous;			\
	register DitherFloat *curr = pvt->current;			\
	register DitherFloat current = *curr, desire;			\
	register DitherFloat range1over = 1./range;			\
	register int ix;						\
	for (ix = 0; ix < bw; ix++) {					\
	    desire = inp[ix] + ((DitherFloat) .4375 * current) +	\
			    ((DitherFloat) .0625 * *(prev+0)) +		\
			    ((DitherFloat) .3125 * *(prev+1)) +		\
			    ((DitherFloat) .1875 * *(prev+2)) ;		\
	    prev++;							\
	    *outp++ = actual = (desire+round)*range1over;		\
	    *++curr = current = (desire - actual*range);		\
	}								\
}

MakeDitherBit	(DoDitherQb,QuadPixel,BitPixel)
MakeDitherBit	(DoDitherPb,PairPixel,BitPixel)
MakeDitherBit	(DoDitherBb,BytePixel,BitPixel)

MakeDither	(DoDitherQB,QuadPixel,BytePixel)
MakeDither	(DoDitherPB,PairPixel,BytePixel)
MakeDither	(DoDitherBB,BytePixel,BytePixel)

MakeDither	(DoDitherQP,QuadPixel,PairPixel)
MakeDither	(DoDitherPP,PairPixel,PairPixel)

MakeDither	(DoDitherQQ,QuadPixel,QuadPixel)

/* end module mpdither.c */
