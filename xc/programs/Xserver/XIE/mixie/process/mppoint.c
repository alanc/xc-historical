/* $XConsortium$ */
/**** module mppoint.c ****/
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
  
	mppoint.c -- DDXIE point element
  
	Larry Hare -- AGE Logic, Inc. May, 1993
  
*****************************************************************************/


#define _XIEC_MPPOINT
#define _XIEC_PPOINT

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
 *  routines referenced by other DDXIE modules
 */
int	miAnalyzePoint();

/*
 *  routines used internal to this module
 */
static int CreatePoint();
static int InitializePoint();
static int FlushPoint();
static int ResetPoint();
static int DestroyPoint();

static int ActivatePointExplode();
static int ActivatePointCombine();
static int ActivatePointROI();
static int ActivatePoint();

/*
 * DDXIE Point entry points
 */
static ddElemVecRec PointVec = {
  CreatePoint,
  InitializePoint,
  ActivatePoint,
  FlushPoint,
  ResetPoint,
  DestroyPoint
  };

/*
* Local Declarations.
*/

typedef struct _mppointdef {
	void	(*action) ();
	void	(*action2) ();
	CARD32	constant;
} mpPointPvtRec, *mpPointPvtPtr;

static void P11_bb0(), P11_bb1();
static void P11_bb(), P11_Bb(), P11_Pb(), P11_Qb();
static void P11_bB(), P11_BB(), P11_PB(), P11_QB();
static void P11_bP(), P11_BP(), P11_PP(), P11_QP();
static void P11_bQ(), P11_BQ(), P11_PQ(), P11_QQ();

static void (*action_pnt11[4][4])() = {
	P11_bb, P11_Bb, P11_Pb, P11_Qb,	/* [out=1][inp=1...4] */
	P11_bB, P11_BB, P11_PB, P11_QB,	/* [out=2][inp=1...4] */
	P11_bP, P11_BP, P11_PP, P11_QP,	/* [out=3][inp=1...4] */
	P11_bQ, P11_BQ, P11_PQ, P11_QQ,	/* [out=4][inp=1...4] */
};

static void Proi11_bb(), Proi11_BB(), Proi11_PP(), Proi11_QQ();
static void (*action_pntroi11[4])() = {
	Proi11_bb, Proi11_BB, Proi11_PP, Proi11_QQ /* [out==inp=1...4] */
};

static void CrazyPixels();


/*------------------------------------------------------------------------
------------------------  fill in the vector  ---------------------------
------------------------------------------------------------------------*/
int miAnalyzePoint(flo,ped)
    floDefPtr flo;
    peDefPtr  ped;
{
    ped->ddVec = PointVec;
    return TRUE;
}


/*------------------------------------------------------------------------
---------------------------- create peTex . . . --------------------------
------------------------------------------------------------------------*/
static int CreatePoint(flo,ped)
    floDefPtr flo;
    peDefPtr  ped;
{
    return MakePETex(flo,ped,
		     xieValMaxBands * sizeof(mpPointPvtRec),
		     TRUE,	/* InSync: Make sure Lut exists first */
		     FALSE	/* bandSync: see InitializePoint */
		     );
} 

/*------------------------------------------------------------------------
---------------------------- initialize peTex . . . ----------------------
------------------------------------------------------------------------*/

static int InitializePoint(flo,ped)
    floDefPtr flo;
    peDefPtr  ped;
{
    xieFloPoint     *raw = (xieFloPoint *)ped->elemRaw;
    peTexPtr 	     pet = ped->peTex;
    mpPointPvtPtr    pvt = (mpPointPvtPtr) pet->private;
    receptorPtr      rcp = pet->receptor;
    CARD32	  nbands = pet->receptor[SRCtag].inFlo->bands;
    CARD32	  lbands = pet->receptor[LUTtag].inFlo->bands;
    bandPtr	   iband = &(pet->receptor[SRCtag].band[0]);
    bandPtr	   oband = &(pet->emitter[0]);
    CARD8	     msk = raw->bandMask;
    BOOL        bandsync = FALSE;
    CARD8	  passmsk, lutmask;
    CARD32	  band;

    if (nbands == 3 && lbands == 1) {
	int oo = oband->format->class;
	int ii = iband->format->class;
	/* Create CRAZY pixels by multiply/accumulate */
        if (pet->receptor[LUTtag].band[0].format->width == xieValLSFirst) {
	    (pvt+0)->constant = 1;
	    (pvt+1)->constant = (pvt+0)->constant * (iband+0)->format->levels;
	    (pvt+2)->constant = (pvt+1)->constant * (iband+1)->format->levels;
        } else { /* swizzle band-order MSFirst */
	    (pvt+2)->constant = 1;
	    (pvt+1)->constant = (pvt+2)->constant * (iband+2)->format->levels;
	    (pvt+0)->constant = (pvt+1)->constant * (iband+1)->format->levels;
	}
	/* XXX
	**  Only does bytes to bytes now.  Need to generate temporary
	**  storage rather than usurping destination space, or build in
	**  lookup into the multiplication phase.
	*/
	pvt->action2 = CrazyPixels;

	if (oband->format->levels == 1)
	    pvt->action = P11_bb0;
	else
	    pvt->action = action_pnt11[oo-1][ii-1];

	/* protocol requires msk == 7 */
	msk = 7; passmsk = 0, lutmask = 1; bandsync = TRUE;
    	ped->ddVec.activate = ActivatePointCombine;

    } else if (nbands == 1 && lbands == 3) {
	
	/* XXX: Jam in replicate mask here, then treat like 3x3 */

	for(band = 0; band < lbands; band++, pvt++, oband++) {
	    int oo = oband->format->class;
	    int ii = iband->format->class;

	    if (oband->format->levels == 1)
		pvt->action = P11_bb0;
	    else if ((iband->format->levels == 1) && (oo == BIT_PIXEL))
		pvt->action = P11_bb1;
	    else
		pvt->action = action_pnt11[oo-1][ii-1];
	}
	/*
	** XXX, might argue that a band mask of 5 would apply luts
	** on band 0 and 2, while passing thru band 1 untouched. If 
	** so we need to add support in dixie and here.  Would rather
	** disable this from protocol.
	*/
	msk = 1; passmsk = 0; lutmask = 7; bandsync = TRUE;
    	ped->ddVec.activate = ActivatePointExplode;

    } else if (nbands == lbands) {
	  /* Standard Case.  Map each band thru its own LUT */

	if (raw->domainPhototag) {
	    for(band = 0; band < nbands; band++, pvt++, iband++, oband++) {
	      int oo = oband->format->class;

	      if (oband->format->levels == 1)
		  pvt->action = P11_bb0; /* XXX could passthrough source here*/
	      else
		  pvt->action = action_pntroi11[oo-1];
	    }
	    passmsk = ~msk; lutmask = msk;
	    ped->ddVec.activate = ActivatePointROI;
	    pet->receptor[ped->inCnt-1].band[0].replicate = msk;
        } else {
	    for(band = 0; band < nbands; band++, pvt++, iband++, oband++) {
	      int oo = oband->format->class;
	      int ii = iband->format->class;

	      if (oband->format->levels == 1)
		  pvt->action = P11_bb0;
	      else if ((iband->format->levels == 1) && (oo == BIT_PIXEL))
		  pvt->action = P11_bb1;
	      else
		  pvt->action = action_pnt11[oo-1][ii-1];
	    }
	    passmsk = ~msk; lutmask = msk;
	    ped->ddVec.activate = ActivatePoint;
	  }
  
    } else 
	ImplementationError(flo,ped, return(FALSE));

    pet->bandSync = bandsync;
    return  InitReceptor(flo, ped, &rcp[SRCtag], 0, 1, msk, passmsk) &&
	    InitReceptor(flo, ped, &rcp[LUTtag], 0, 1, lutmask, (CARD8)0) &&
	    (!raw->domainPhototag || InitProcDomain(flo, ped, 
			raw->domainPhototag, raw->domainOffsetX, 
					raw->domainOffsetY)) &&
	    InitEmitter(flo,ped,0,(raw->domainPhototag) ? SRCtag : NO_INPLACE);

}

/*------------------------------------------------------------------------
----------------------------- crank some data ----------------------------
------------------------------------------------------------------------*/

/*------------------  (1 Band, 3 LUTS) --> (3 Bands) -------------------*/

static int ActivatePointExplode(flo,ped,pet)
     floDefPtr flo;
     peDefPtr  ped;
     peTexPtr  pet;
{
    mpPointPvtPtr pvt = (mpPointPvtPtr) pet->private;
    int band, nbands = pet->receptor[SRCtag].inFlo->bands;
    bandPtr iband = &(pet->receptor[SRCtag].band[0]);
    bandPtr lband = &(pet->receptor[LUTtag].band[0]);
    bandPtr oband = &(pet->emitter[0]);
    void    *lvoid0, *lvoid1, *lvoid2;
    void    *ovoid0, *ovoid1, *ovoid2;
    void    *ivoid;
    int     bw = iband->format->width;

    /* asking for 1 should fetch entire lut strip */
    lvoid0 = GetSrcBytes(void,flo,pet,lband,0,1,FALSE); lband++;
    lvoid1 = GetSrcBytes(void,flo,pet,lband,0,1,FALSE); lband++;
    lvoid2 = GetSrcBytes(void,flo,pet,lband,0,1,FALSE);

    if (!lvoid0 || !lvoid1 || !lvoid2)
	ImplementationError(flo,ped, return(FALSE));

    ivoid  = GetCurrentSrc(void,flo,pet,iband);
    ovoid0 = GetCurrentDst(void,flo,pet,oband); oband++;
    ovoid1 = GetCurrentDst(void,flo,pet,oband); oband++;
    ovoid2 = GetCurrentDst(void,flo,pet,oband); oband -= 2;
    while (!ferrCode(flo) && ivoid && ovoid0 && ovoid1 && ovoid2) {

        (*((pvt+0)->action)) (ivoid, ovoid0, lvoid0, bw);
        (*((pvt+1)->action)) (ivoid, ovoid1, lvoid1, bw);
        (*((pvt+2)->action)) (ivoid, ovoid2, lvoid2, bw);

	ivoid  = GetNextSrc(void,flo,pet,iband,TRUE);
	ovoid0 = GetNextDst(void,flo,pet,oband,TRUE); oband++;
	ovoid1 = GetNextDst(void,flo,pet,oband,TRUE); oband++;
	ovoid2 = GetNextDst(void,flo,pet,oband,TRUE); oband -= 2;
    }
    FreeData(void, flo, pet, iband, iband->current);
    if (iband->final) {
	FreeData(void, flo, pet, lband ,lband->current); lband--;
	FreeData(void, flo, pet, lband ,lband->current); lband--;
	FreeData(void, flo, pet, lband ,lband->current); 
    }
    return TRUE;
}

/*------------------  (3 Band, 1 LUTS) --> (1 Bands) -------------------*/

static int ActivatePointCombine(flo,ped,pet)
     floDefPtr flo;
     peDefPtr  ped;
     peTexPtr  pet;
{
    mpPointPvtPtr pvt = (mpPointPvtPtr) pet->private;
    bandPtr iband = &(pet->receptor[SRCtag].band[0]);
    bandPtr lband = &(pet->receptor[LUTtag].band[0]);
    bandPtr oband = &(pet->emitter[0]);
    void    *ivoid0, *ivoid1, *ivoid2, *lvoid, *ovoid;
    int     bw = iband->format->width;

    /* asking for 1 should fetch entire lut strip */
    if (!(lvoid = GetSrcBytes(void,flo,pet,lband,0,1,FALSE)))
	return FALSE;

    ovoid  = GetCurrentDst(void,flo,pet,oband);
    ivoid0 = GetCurrentSrc(void,flo,pet,iband); iband++;
    ivoid1 = GetCurrentSrc(void,flo,pet,iband); iband++;
    ivoid2 = GetCurrentSrc(void,flo,pet,iband); iband -= 2;

    while (!ferrCode(flo) && ovoid && ivoid0 && ivoid1 && ivoid2) {

	/* XXX Currently operates in place (B*B*B==>B==>any) */
        (*(pvt->action2)) (ivoid0, (pvt)->constant,
			   ivoid1, (pvt+1)->constant,
			   ivoid2, (pvt+2)->constant,
			   ovoid, bw);

        (*(pvt->action)) (ovoid, ovoid, lvoid, bw);

	ovoid  = GetNextDst(void,flo,pet,oband,TRUE);
	ivoid0 = GetNextSrc(void,flo,pet,iband,TRUE); iband++;
	ivoid1 = GetNextSrc(void,flo,pet,iband,TRUE); iband++;
	ivoid2 = GetNextSrc(void,flo,pet,iband,TRUE); iband -= 2;
    }

    FreeData(void, flo, pet, iband, iband->current); iband++;
    FreeData(void, flo, pet, iband, iband->current); iband++;
    FreeData(void, flo, pet, iband, iband->current);

    if (iband->final)
	FreeData(void, flo, pet, lband, lband->current);

    return TRUE;
}

/*------------------  (3 Band, 3 LUTS, ROI) --> (3 Bands) -------------------*/
/*------------------  (1 Band, 1 LUTS, ROI) --> (1 Bands) -------------------*/

static int ActivatePointROI(flo,ped,pet)
     floDefPtr flo;
     peDefPtr  ped;
     peTexPtr  pet;
{
    
    mpPointPvtPtr pvt = (mpPointPvtPtr) pet->private;
    int band, nbands  = pet->receptor[SRCtag].inFlo->bands;
    bandPtr iband     = &(pet->receptor[SRCtag].band[0]);
    bandPtr lband     = &(pet->receptor[LUTtag].band[0]);
    bandPtr rband     = &(pet->receptor[ped->inCnt-1].band[0]);
    bandPtr oband     = &(pet->emitter[0]);
    INT32 Xoffset     = ((xieFloPoint *)ped->elemRaw)->domainOffsetX;
    void * lvoid;


    for(band=0; band < nbands; band++,pvt++,iband++,oband++,lband++,rband++) {
	register int bw   = iband->format->width;
	void *ivoid, *ovoid;

	/* 1 should fetch entire lut strip */
    	if (!(lvoid = GetSrcBytes(void,flo,pet,lband,0,1,FALSE)) ||
	    !(ivoid = GetCurrentSrc(void,flo,pet,iband)) ||
	    !(ovoid = GetCurrentDst(void,flo,pet,oband))) continue;

	while (!ferrCode(flo) && ivoid && ovoid && 
				SyncDomain(flo,ped,oband,FLUSH)) {
	    INT32 run, currentx = 0;
	   
    	    if(ivoid != ovoid) bcopy(ivoid, ovoid, oband->pitch);

	    while (run = GetRun(flo,pet,oband)) {
		if (run > 0) {
	    	    (*(pvt->action)) (ovoid, lvoid, run, currentx);
		    currentx += run;
		} else
		    currentx -= run;
	    }

	    ivoid = GetNextSrc(void,flo,pet,iband,TRUE);
	    ovoid = GetNextDst(void,flo,pet,oband,TRUE);
	}

	FreeData(void, flo, pet, iband, iband->current);
	if (iband->final)
	    FreeData(void, flo, pet, lband, lband->current);
    }
    return TRUE;
}


/*------------------  (3 Band, 3 LUTS) --> (3 Bands) -------------------*/
/*------------------  (1 Band, 1 LUTS) --> (1 Bands) -------------------*/

static int ActivatePoint(flo,ped,pet)
     floDefPtr flo;
     peDefPtr  ped;
     peTexPtr  pet;
{
    mpPointPvtPtr pvt = (mpPointPvtPtr) pet->private;
    int band, nbands = pet->receptor[SRCtag].inFlo->bands;
    bandPtr iband = &(pet->receptor[SRCtag].band[0]);
    bandPtr lband = &(pet->receptor[LUTtag].band[0]);
    bandPtr oband = &(pet->emitter[0]);
    void * lvoid;

    for(band = 0; band < nbands; band++, pvt++, iband++, oband++, lband++) {
	register int bw = iband->format->width;
	void *ivoid, *ovoid;

	/* 1 should fetch entire lut strip */
    	if (!(lvoid = GetSrcBytes(void,flo,pet,lband,0,1,FALSE)) ||
	    !(ivoid = GetCurrentSrc(void,flo,pet,iband)) ||
	    !(ovoid = GetCurrentDst(void,flo,pet,oband))) continue;

	do {
	    (*(pvt->action)) (ivoid, ovoid, lvoid, bw);
	    ivoid = GetNextSrc(void,flo,pet,iband,TRUE);
	    ovoid = GetNextDst(void,flo,pet,oband,TRUE);
	} while (!ferrCode(flo) && ivoid && ovoid) ;

	FreeData(void, flo, pet, iband, iband->current);
	if (iband->final)
	    FreeData(void, flo, pet, lband, lband->current);
    }
    return TRUE;
}

/*------------------------------------------------------------------------
--------------------------- get rid of left overs ------------------------
------------------------------------------------------------------------*/
static int FlushPoint(flo,ped)
    floDefPtr flo;
    peDefPtr  ped;
{
    return TRUE;
}               

/*------------------------------------------------------------------------
------------------------ get rid of run-time stuff -----------------------
------------------------------------------------------------------------*/
static int ResetPoint(flo,ped)
    floDefPtr flo;
    peDefPtr  ped;
{
    ResetReceptors(ped);
    ResetProcDomain(ped);
    ResetEmitter(ped);
    return TRUE;
}

/*------------------------------------------------------------------------
-------------------------- get rid of this element -----------------------
------------------------------------------------------------------------*/
static int DestroyPoint(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
    /* get rid of the peTex structure  */
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
---------------------  Lotsa Little Action Routines  ---------------------
------------------------------------------------------------------------*/

static void
P11_bb(INP,OUTP,LUTP,bw)
	void *INP; void *OUTP; void *LUTP; int bw;
{
	unsigned char *inp = (unsigned char *) INP;
	unsigned char *outp = (unsigned char *) OUTP;
	unsigned char *lutp = (unsigned char *) LUTP;
	/*
	** bitonal ---> LUT ---> bitonal
	**
	** 1) We could try to infer the lut relationships once and
	**    jam a new action routine, but this is not convenient.
	**    P11_bb1 below is used when input levels ==1 ;
	**
	** 2) following code may modify bit pixels in pad area. an
	**    alternative would be to work hard and not molest these
	**    pixels.
	**
	** 3) should we use bytes and thus bzero, bcopy, and perhaps memset
	**	to pick up cpu optimized routines automatically, OR, should
	**	we use unsigned longs (or perhaps LogInts which might be
	**	eg 64 bit ints on R4000?) and do cpu loops which may need
	**	to be unrolled?  Have fun out there?
	*/
	bw = (bw + 7) >> 3;
	if (lutp[0] == 0) {
	    if (lutp[1] == 0) 
		bzero(outp, bw);
	    else
		bcopy(inp, outp, bw);
	} else {
	    if (lutp[1] == 0)
		while (bw-- > 0) { *outp++ = *inp++ ^ (unsigned char) ~0; }
	    else
		memset(outp, ~0, bw);
	}
}

static void
P11_bb1(INP,OUTP,LUTP,bw)
	void *INP; void *OUTP; void *LUTP; int bw;
{
	unsigned char *inp = (unsigned char *) INP;
	unsigned char *outp = (unsigned char *) OUTP;
	unsigned char *lutp = (unsigned char *) LUTP;

	/*
	** bitonal ---> LUT ---> bitonal (input is single level aka 0)
	*/
	bw = (bw + 7) >> 3;
	if (lutp[0] == 0) 
	    bzero(outp, bw);
	else
	    memset(outp, ~0, bw);
}

static void
P11_bb0(INP,OUTP,LUTP,bw)
	void *INP; void *OUTP; void *LUTP; int bw;
{
	unsigned char *outp = (unsigned char *) OUTP;
	/*
	** anything ---> LUT ---> bitonal (output is single level aka 0)
	*/
	bw = (bw + 7) >> 3;
	bzero(outp, bw);
}

/*
** DO_P11b - single band, single lut, bit to bit
** DO_P11c - single band, single lut, consume bits
** DO_P11p - single band, single lut, produce bits
** DO_P11  - single band, single lut, non-bit versions
*/

#define DO_P11b(fd_do,itype,otype)	/* see P11_bb, P11_bb1, P11_bb0 above */

/*
** Consume bits. (DO_P11c)
**  1) Could be implemented to use small lookup table as used in typical
**	color expansion code, in which case we would need to precalculate
**	the table somewhere and pass in a pointer.  Would need more endian
**	specific macros.
**  2) Could look for runs of zero's and one's and try to use faster ways
**	of setting blocks of pixels.  ffs() type instructions could help.
**
** Produce bits. (DO_P11p)
**  1) Probably about as good as it will get. Perhaps add some unrolling.
*/

#if defined(LOG_XXXbit)
	/* This is the really slow way.  You don't want to use this */
#define DO_P11c(fn_do,itype,otype)					\
static void								\
fn_do(INP,OUTP,LUTP,bw)							\
	void *INP; void *OUTP; void *LUTP; int bw;			\
{									\
	LogInt *inp = (LogInt *) INP;					\
	otype *outp = (otype *) OUTP;					\
	otype *lutp = (otype *) LUTP;					\
	otype outval, loval = lutp[0], hival = lutp[1];			\
	int ix;								\
	for (ix=0; ix < bw; ix++)					\
		outp[ix] = LOG_tstbit(inp,ix) ? hival : loval;		\
}
#define DO_P11p(fn_do,itype,otype)					\
static void								\
fn_do(INP,OUTP,LUTP,bw)							\
	void *INP; void *OUTP; void *LUTP; int bw;			\
{									\
	itype *inp = (itype *) INP;					\
	LogInt *outp = (LogInt *) OUTP;					\
	unsigned char *lutp = (otype *) LUTP;				\
	int ix;								\
	bzero((unsigned char *) OUTP, (bw+7)>>3);			\
	for (ix=0; ix < bw; ix++)					\
	    if (lutp[inp[ix]])						\
		LOG_setbit(outp,ix);					\
}
#else

#define DO_P11c(fn_do,itype,otype)					\
static void								\
fn_do(INP,OUTP,LUTP,bw)							\
	void *INP; void *OUTP; void *LUTP; int bw;			\
{									\
	LogInt *inp = (LogInt *) INP;					\
	otype *outp = (otype *) OUTP;					\
	otype *lutp = (otype *) LUTP;					\
	otype loval = lutp[0], hival = lutp[1];				\
	LogInt M, inval;						\
	for ( ; bw >= LOGSIZE ; bw -= LOGSIZE)				\
	    for (M=LOGLEFT, inval = *inp++; M; LOGRIGHT(M))		\
		*outp++ =  (inval & M) ? hival : loval;			\
	if (bw > 0)							\
	    for (M=LOGLEFT, inval = *inp++; bw; bw--, LOGRIGHT(M))	\
		*outp++ =  (inval & M) ? hival : loval;			\
}
#define DO_P11p(fn_do,itype,otype)					\
static void								\
fn_do(INP,OUTP,LUTP,bw)							\
	void *INP; void *OUTP; void *LUTP; int bw;			\
{									\
	itype *inp = (itype *) INP;					\
	LogInt *outp = (LogInt *) OUTP, M, outval;			\
	unsigned char *lutp = (unsigned char *) LUTP;			\
	for ( ; bw >= LOGSIZE ; *outp++ = outval, bw -= LOGSIZE)	\
	    for (M=LOGLEFT, outval = 0; M; LOGRIGHT(M))			\
		if (lutp[*inp++])					\
		    outval |= M;					\
	if (bw > 0) {							\
	    for (M=LOGLEFT, outval = 0; bw; bw--, LOGRIGHT(M))		\
		if (lutp[*inp++])					\
		    outval |= M;					\
	    *outp++ = outval;						\
	}								\
}
#endif

	/*
	** This is the optimal way to convert bits to bytes.  You may
	** be able to improve on the bitexpand subroutine yourself ...
	*/

#define DO_P11x(fn_do,itype,otype)					\
static void								\
fn_do(INP,OUTP,LUTP,bw)							\
	void *INP; void *OUTP; void *LUTP; int bw;			\
{ 									\
	void  bitexpand();						\
	otype *lutp = (otype *) LUTP;					\
	bitexpand(INP,OUTP,bw, lutp[0], lutp[1]);			\
}


#define DO_P11(fn_do,itype,otype)					\
static void								\
fn_do(INP,OUTP,LUTP,bw)							\
	void *INP; void *OUTP; void *LUTP; int bw;			\
{									\
	itype *inp = (itype *) INP;					\
	otype *outp = (otype *) OUTP;					\
	otype *lutp = (otype *) LUTP;					\
	while (bw-- > 0) *outp++ = lutp[*inp++];			\
}

/*
** DO_P11b - bit to bit
** DO_P11c - consume bits (also DO_P11x)
** DO_P11x - consume bits (faster bit eXpander)
** DO_P11p - produce bits
** DO_P11  - non-bit versions
*/

DO_P11b	(P11_bb, BitPixel,  BitPixel)
DO_P11p	(P11_Bb, BytePixel, BitPixel)
DO_P11p	(P11_Pb, PairPixel, BitPixel)
DO_P11p	(P11_Qb, QuadPixel, BitPixel)

DO_P11x	(P11_bB, BitPixel,  BytePixel)
DO_P11	(P11_BB, BytePixel, BytePixel)
DO_P11	(P11_PB, PairPixel, BytePixel)
DO_P11	(P11_QB, QuadPixel, BytePixel)

DO_P11c	(P11_bP, BitPixel,  PairPixel)
DO_P11	(P11_BP, BytePixel, PairPixel)
DO_P11	(P11_PP, PairPixel, PairPixel)
DO_P11	(P11_QP, QuadPixel, PairPixel)

DO_P11c	(P11_bQ, BitPixel,  QuadPixel)
DO_P11	(P11_BQ, BytePixel, QuadPixel)
DO_P11	(P11_PQ, PairPixel, QuadPixel)
DO_P11	(P11_QQ, QuadPixel, QuadPixel)

/*------------------------------------------------------------------------
---------------------  Crazy Pixels Action Routines  ---------------------
------------------------------------------------------------------------*/

/* Only one right now takes (bytes*bytes*bytes) ==> bytes */

static void
CrazyPixels(ivoid0, c0, ivoid1, c1, ivoid2, c2, ovoid, bw)
    void  *ivoid0, *ivoid1, *ivoid2, *ovoid;
    int   c0, c1, c2, bw;
{
    BytePixel *inp0 = (BytePixel *) ivoid0;
    BytePixel *inp1 = (BytePixel *) ivoid1;
    BytePixel *inp2 = (BytePixel *) ivoid2;
    BytePixel *outp = (BytePixel *) ovoid;
    while (bw-- > 0) {
	/* We could do lookups here too, but variations will kill us */
	*outp++ = (c0 * *inp0++) + (c1 * *inp1++) + (c2 * *inp2++);
    }
}

/*------------------------------------------------------------------------
---------------------  ROI operations work on subranges ------------------
------------------------------------------------------------------------*/

/*
**  The ROI routines only map within a single type.  It would
**  have been possible to call the regular P11_XX routines above,
**  by clever adjusting of the input arguments to offset to the
**  correct pixel.  However, we decided to use new routines here,
**  which take a subscript as an input.  This will allow the P11_XX
**  routines to be optimized on the assumption the arrays start 
**  on a word boundary (for instance the P11_BB routine could load
**  and store 4 pixels at a time and reduce the number of memory
**  operations by a factor of two.)
*/

/* XXX keep it simple this time.  see mplogic.c for code to process
** a word at a time with masking at the edges.
*/

static void
Proi11_bb(INP,LUTP,bw,ix)
	void *INP; void *LUTP; int bw; int ix;
{
	LogInt *inp  = ((LogInt *) INP);
	CARD8 *lutp = (CARD8 *) LUTP;

	/* Too bad we don't know the LUT at CreatePoint time ... */

	if (*lutp == 0) {
		if (*(lutp+1) == 1)
			return;  /* identity mapping */
	} else if (*(lutp+1) == 0)
		while ( bw-- ) { LOG_xorbit (inp, ix); ix++; }
	else if (*(lutp+1) & 1) 
		while ( bw-- ) { LOG_setbit (inp, ix); ix++; }
	else 
		while ( bw-- ) { LOG_clrbit (inp, ix); ix++; }

	/* Should do RROP type stuff with start/end mask .... */
}

#define ROI_P11b(fn_do,iotype)	/* see Proi11_bb above */

#define ROI_P11(fn_do,iotype)					\
static void 							\
fn_do(INP,LUTP,bw,ix) 						\
	void *INP; void *LUTP; INT32 bw; INT32 ix;		\
{ 								\
	iotype *inp  = ((iotype *) INP) + ix;			\
	iotype *lutp = (iotype *) LUTP; 			\
	while (bw-- > 0) { *inp = lutp[*inp]; inp++; }		\
}

ROI_P11b	(Proi11_bb, BitPixel)
ROI_P11		(Proi11_BB, BytePixel)
ROI_P11		(Proi11_PP, PairPixel)
ROI_P11		(Proi11_QQ, QuadPixel)

/* end module mppoint.c */
