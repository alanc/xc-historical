/* $XConsortium$ */
/**** module mplogic.c ****/
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
  
	mplogic.c -- DDXIE logic element
  
	Larry Hare -- AGE Logic, Inc. July, 1993
  
*****************************************************************************/


#define _XIEC_MPLOGIC
#define _XIEC_PLOGIC

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
int	miAnalyzeLogic();

/*
 *  routines used internal to this module
 */
static int CreateLogic();
static int InitializeLogic();
static int FlushLogic();
static int ResetLogic();
static int DestroyLogic();

static int ActivateLogicM();
static int ActivateLogicD();
static int ActivateLogicMROI();
static int ActivateLogicDROI();

/*
 * DDXIE Logic entry points
 */
static ddElemVecRec LogicVec = {
  CreateLogic,
  InitializeLogic,
  ActivateLogicM,
  FlushLogic,
  ResetLogic,
  DestroyLogic
  };

/*
* Local Declarations.
*/

typedef struct _mplogicdef {
	void	(*action) ();
	void	(*action2) ();
	CARD32	constant;
	CARD32	endrun;
	CARD32	endix;
} mpLogicPvtRec, *mpLogicPvtPtr;

void action_tail();
extern void (*action_mono[16])();
extern void (*action_dyad[16])();
extern void (*action_monoROI[16])();
extern void (*action_dyadROI[16])();

static CARD32 replicate_const();

#define SHIFT_FROM_LEVELS(shift,levels) \
{ CARD32 _lev = levels; \
  shift = _lev <= 256 ? (_lev <= 2 ? 0 : 3 ) : (_lev <=65536 ? 4 : 5); \
}

/*
** NOTE:  might change over to use mergerops (see mfb/mergerop.h)
** NOTE:  might change constant operations to use dyads with prefilled
**		constant strip;
** XXX:	  does not worry about upper bits when levls not stock power of two
** XXX:   assumes Pixel Size same for Dyad operations.
*/

/*------------------------------------------------------------------------
------------------------  fill in the vector  ---------------------------
------------------------------------------------------------------------*/
int miAnalyzeLogic(flo,ped)
    floDefPtr flo;
    peDefPtr  ped;
{
    ped->ddVec = LogicVec;
    return TRUE;
}


/*------------------------------------------------------------------------
---------------------------- create peTex . . . --------------------------
------------------------------------------------------------------------*/
static int CreateLogic(flo,ped)
    floDefPtr flo;
    peDefPtr  ped;
{
    /* always force syncing between inputs (is nop if only one input) */
    return MakePETex(flo,ped,
		     xieValMaxBands * sizeof(mpLogicPvtRec),
		     SYNC,	/* InSync: Make sure ROI exists first */
		     NO_SYNC	/* bandSync: see CreateLogic */
		     );
} 

/*------------------------------------------------------------------------
---------------------------- initialize peTex . . . ----------------------
------------------------------------------------------------------------*/

static int InitializeLogic(flo,ped)
    floDefPtr flo;
    peDefPtr  ped;
{
    peTexPtr 	     pet = ped->peTex;
    xieFloLogical   *raw = (xieFloLogical *) ped->elemRaw;
    pLogicDefPtr    epvt = (pLogicDefPtr)  ped->elemPvt;
    mpLogicPvtPtr    pvt = (mpLogicPvtPtr) pet->private;
    receptorPtr      rcp = pet->receptor;
    CARD32	  nbands = pet->receptor[SRCt1].inFlo->bands;
    bandPtr	   dband = &(pet->emitter[0]);
    CARD8	     msk = raw->bandMask;
    BOOL	  hasROI = raw->domainPhototag != 0;
    CARD32	    band;
    void	(*act)();

    if (hasROI) {
	if (raw->src2) {
	    ped->ddVec.activate = ActivateLogicDROI;
	    act = action_dyadROI[raw->operator];
	} else {
	    ped->ddVec.activate = ActivateLogicMROI;
	    act = action_monoROI[raw->operator];
	}
    } else { /* no ROI */
	if (raw->src2) {
	    ped->ddVec.activate = ActivateLogicD;
	    act = action_dyad[raw->operator];
	} else {
	    ped->ddVec.activate = ActivateLogicM;
	    act = action_mono[raw->operator];
	}
    }

    for (band=0; band<nbands; band++, pvt++, dband++) {
	pvt->action = act;
	if (!raw->src2) {
	    pvt->constant = replicate_const(dband->format->levels,
					(CARD32)(epvt->constant[band] + 0.5));
	} else if (!hasROI) {
	    /* gack, an alternative is to use INPLACE for SRCt1 */
    	    bandPtr tband = &(pet->receptor[SRCt2].band[band]);
	    if (dband->format->pitch < tband->format->pitch) {
		pvt->action2 = (void (*)()) NULL;
		pvt->endix = dband->format->pitch; /* bits */
	    } else {
	        pvt->action2 = action_tail;
		pvt->endix = tband->format->pitch;
		pvt->endrun = dband->format->pitch - pvt->endix;
	    }
	}
    }

    InitReceptor(flo, ped, &rcp[SRCt1], NO_DATAMAP, 1, msk, ~msk);
    if (msk && raw->src2)
	InitReceptor(flo, ped, &rcp[SRCt2], NO_DATAMAP, 1, msk, NO_BANDS);
    if (hasROI)
	InitProcDomain(flo, ped, raw->domainPhototag, raw->domainOffsetX, 
							raw->domainOffsetY);
    if (msk)
	InitEmitter(flo, ped, NO_DATAMAP, hasROI ? SRCt1 : NO_INPLACE);

    /* If processing domain, allow replication */
    if (hasROI)
	pet->receptor[ped->inCnt-1].band[0].replicate = msk;

    return !ferrCode(flo);
}

/*------------------------------------------------------------------------
----------------------------- crank some data ----------------------------
------------------------------------------------------------------------*/

static int ActivateLogicM(flo,ped,pet)
     floDefPtr flo;
     peDefPtr  ped;
     peTexPtr  pet;
{
    mpLogicPvtPtr pvt = (mpLogicPvtPtr) pet->private;
    int band, nbands = pet->receptor[SRCt1].inFlo->bands;
    bandPtr sband = &(pet->receptor[SRCt1].band[0]);
    bandPtr dband = &(pet->emitter[0]);

    for(band = 0; band < nbands; band++, pvt++, sband++, dband++) {
	int pitch = sband->format->pitch; /* bits */
	LogInt *svoid, *dvoid;

    	if (!(svoid = GetCurrentSrc(void,flo,pet,sband)) ||
	    !(dvoid = GetCurrentDst(void,flo,pet,dband))) continue;

	do {
	    (*(pvt->action)) (dvoid, svoid, pvt->constant, pitch);
	    svoid = GetNextSrc(void,flo,pet,sband,FLUSH);
	    dvoid = GetNextDst(void,flo,pet,dband,FLUSH);
	} while (!ferrCode(flo) && svoid && dvoid) ;

	FreeData(void, flo, pet, sband, sband->current);
    }
    return TRUE;
}

static int ActivateLogicD(flo,ped,pet)
     floDefPtr flo;
     peDefPtr  ped;
     peTexPtr  pet;
{
    mpLogicPvtPtr pvt = (mpLogicPvtPtr) pet->private;
    int band, nbands = pet->receptor[SRCt1].inFlo->bands;
    bandPtr sband = &(pet->receptor[SRCt1].band[0]);
    bandPtr tband = &(pet->receptor[SRCt2].band[0]);
    bandPtr dband = &(pet->emitter[0]);

    for(band = 0; band < nbands; band++, pvt++, sband++, tband++, dband++) {
	LogInt *svoid, *tvoid, *dvoid;

    	if (!(svoid = GetCurrentSrc(void,flo,pet,sband)) ||
	    !(tvoid = GetCurrentSrc(void,flo,pet,tband)) ||
	    !(dvoid = GetCurrentDst(void,flo,pet,dband))   ) continue;

	do {
	    /* This is the code that might rather utilize INPLACE */
	    (*(pvt->action)) (dvoid, svoid, tvoid, pvt->endix);
	    if (pvt->action2)
		(*(pvt->action2)) (dvoid, svoid, pvt->endrun, pvt->endix);
	    svoid = GetNextSrc(void,flo,pet,sband,FLUSH);
	    tvoid = GetNextSrc(void,flo,pet,tband,FLUSH);
	    dvoid = GetNextDst(void,flo,pet,dband,FLUSH);
	} while (!ferrCode(flo) && svoid && tvoid && dvoid) ;

	if(!svoid && sband->final)	/* when sr1 runs out, kill sr2 too  */
	    DisableSrc(flo,pet,tband,FLUSH);
	else if(!tvoid && tband->final)	/* when sr2 runs out, pass-thru sr1 */
	    BypassSrc(flo,pet,sband);
	else { 	/* both inputs still active, keep the scheduler up to date  */
	    FreeData(void,flo,pet,sband,sband->current);
	    FreeData(void,flo,pet,tband,tband->current);
	}
    }
    return TRUE;
}

static int ActivateLogicMROI(flo,ped,pet)
     floDefPtr flo;
     peDefPtr  ped;
     peTexPtr  pet;
{
    mpLogicPvtPtr pvt = (mpLogicPvtPtr) pet->private;
    int band, nbands  = pet->receptor[SRCt1].inFlo->bands;
    bandPtr sband     = &(pet->receptor[SRCt1].band[0]);
    bandPtr rband     = &(pet->receptor[ped->inCnt-1].band[0]);
    bandPtr dband     = &(pet->emitter[0]);
    INT32 Xoffset     = ((xieFloLogical *)ped->elemRaw)->domainOffsetX;

    for(band = 0; band < nbands; band++, pvt++, sband++, dband++) {
	void *svoid, *dvoid;
	CARD32 shift;

    	if (!(svoid = GetCurrentSrc(void,flo,pet,sband)) ||
	    !(dvoid = GetCurrentDst(void,flo,pet,dband))) continue;

	SHIFT_FROM_LEVELS(shift, dband->format->levels)

	while (!ferrCode(flo) && svoid && dvoid && 
				SyncDomain(flo,ped,dband,FLUSH)) {
	    INT32 run, ix = 0;
	   
    	    if (svoid != dvoid) bcopy(svoid, dvoid, dband->pitch);

	    while (run = GetRun(flo,pet,dband)) {
		if (run > 0) {
	    	    (*(pvt->action)) (dvoid, pvt->constant,
					run << shift, ix << shift);
		    ix += run;
		} else
		    ix -= run;
	    }
	    svoid = GetNextSrc(void,flo,pet,sband,FLUSH);
	    dvoid = GetNextDst(void,flo,pet,dband,FLUSH);
	}

	FreeData(void, flo, pet, sband, sband->current);
    }
    return TRUE;
}

static int ActivateLogicDROI(flo,ped,pet)
     floDefPtr flo;
     peDefPtr  ped;
     peTexPtr  pet;
{
    mpLogicPvtPtr pvt = (mpLogicPvtPtr) pet->private;
    int band, nbands  = pet->receptor[SRCt1].inFlo->bands;
    bandPtr sband     = &(pet->receptor[SRCt1].band[0]);
    bandPtr tband     = &(pet->receptor[SRCt2].band[0]);
    bandPtr rband     = &(pet->receptor[ped->inCnt-1].band[0]);
    bandPtr dband     = &(pet->emitter[0]);
    INT32 Xoffset     = ((xieFloLogical *)ped->elemRaw)->domainOffsetX;

    for(band = 0; band < nbands; band++, pvt++, sband++, tband++, dband++) {
	void *svoid, *tvoid, *dvoid;
	CARD32 shift, w;

	w = sband->format->width;
	if (w > tband->format->width) w = tband->format->width;

    	if (!(svoid = GetCurrentSrc(void,flo,pet,sband)) ||
    	    !(tvoid = GetCurrentSrc(void,flo,pet,tband)) ||
	    !(dvoid = GetCurrentDst(void,flo,pet,dband))) continue;

	SHIFT_FROM_LEVELS(shift, dband->format->levels)

	while (!ferrCode(flo) && svoid && tvoid && dvoid && 
				SyncDomain(flo,ped,dband,FLUSH)) {
	    INT32 run, ix = 0;
	   
    	    if (svoid != dvoid) bcopy(svoid, dvoid, dband->pitch);

	    while (run = GetRun(flo,pet,dband)) {
		if (run > 0) {
		    /* needs to clip to second source, yuck */
		    if ((ix + run) > w) {
			if (ix < w) (*(pvt->action)) (dvoid, tvoid,
						(w-ix) << shift, ix << shift);
			break;
		    }
	    	    (*(pvt->action)) (dvoid, tvoid, run << shift, ix << shift);
		    ix += run;
		} else
		    ix -= run;
	    }
	    svoid = GetNextSrc(void,flo,pet,sband,FLUSH);
	    tvoid = GetNextSrc(void,flo,pet,tband,FLUSH);
	    dvoid = GetNextDst(void,flo,pet,dband,FLUSH);
	}

	if(!svoid && sband->final)	/* when sr1 runs out, kill sr2 too  */
	    DisableSrc(flo,pet,tband,FLUSH);
	else if(!tvoid && tband->final)	/* when sr2 runs out, pass-thru sr1 */
	    BypassSrc(flo,pet,sband);
	else { 	/* both inputs still active, keep the scheduler up to date  */
	    FreeData(void,flo,pet,sband,sband->current);
	    FreeData(void,flo,pet,tband,tband->current);
	}
    }
    return TRUE;
}
/*------------------------------------------------------------------------
--------------------------- get rid of left overs ------------------------
------------------------------------------------------------------------*/
static int FlushLogic(flo,ped)
    floDefPtr flo;
    peDefPtr  ped;
{
    return TRUE;
}               

/*------------------------------------------------------------------------
------------------------ get rid of run-time stuff -----------------------
------------------------------------------------------------------------*/
static int ResetLogic(flo,ped)
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
static int DestroyLogic(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
    /* get rid of the peTex structure  */
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

/* NOTE: would be nice to use mfb/mergerop.h. beware though. the roles
** of dst, and source are somewhat reversed. This will be almost 
** necessary if we choose to support mismatched levels, and should make
** it simpler to make fine tuned assembly or C code.  
*/

/* M:	(*(pvt->action)) (dvoid, svoid, pvt->constant, bw); */
/* D:	(*(pvt->action)) (dvoid, svoid, tvoid, bw); */

#define MakeM1(name, rev, op)						\
static void name(d,src1,con,bits)					\
    LogInt *d, *src1, con;						\
    CARD32 bits;							\
{									\
    CARD32 w = (bits + LOGSIZE - 1) / LOGSIZE;				\
    while (w >= 4) { *d++ = op; *d++ = op;				\
		     *d++ = op; *d++ = op; w -= 4; }			\
    switch (w) {							\
	case 3:  *d++ = op;						\
	case 2:  *d++ = op;						\
	case 1:  *d   = op;						\
	default: break;							\
    }									\
}

#define MakeM2(name, rev, op) 						\
static void name(d,src1,con,bits)					\
    LogInt *d, *src1, con;						\
    CARD32 bits;							\
{									\
    CARD32 w = (bits + LOGSIZE - 1) / LOGSIZE;				\
    LogInt A, B, C, D;							\
    while (w >= 4) {							\
	A = *src1++; B = *src1++; C = *src1++; D = *src1++;		\
	*d++ = (rev A) op; *d++ = (rev B) op;				\
	*d++ = (rev C) op; *d++ = (rev D) op; w -= 4;			\
    }									\
    switch (w) {							\
	case 3:  A = *src1++; *d++ = (rev A) op;			\
	case 2:  A = *src1++; *d++ = (rev A) op;			\
	case 1:  A = *src1;   *d   = (rev A) op;			\
	default: break;							\
    }									\
}

#define MakeD1(dname, mname) 						\
static void dname(d,src1,src2,bits)					\
    LogInt *d, *src1, *src2;						\
    CARD32 bits;							\
{									\
    mname (d, src2, 0, bits);						\
}

#define MakeD2(name, rev, op) 						\
static void name(d,src1,src2,bits)					\
    LogInt *d, *src1, *src2;						\
    CARD32 bits;							\
{									\
    CARD32 w = (bits + LOGSIZE - 1) / LOGSIZE;				\
    LogInt A, B, C, D, E, F, G, H;					\
    while (w >= 4) {							\
	A = src1[0]; B = src1[1]; C = src1[2]; D = src1[3]; src1 += 4;	\
	E = src2[0]; F = src2[1]; G = src2[2]; H = src2[3]; src2 += 4;	\
	d[0] = (rev A) op E; d[1] = (rev B) op F;			\
	d[2] = (rev C) op G; d[3] = (rev D) op H; w -= 4; d += 4;	\
    }									\
    switch (w) {							\
	case 3:  A = *src1++; E = *src2++; *d++ = (rev A) op E;		\
	case 2:  A = *src1++; E = *src2++; *d++ = (rev A) op E;		\
	case 1:  A = *src1;   E = *src2++; *d   = (rev A) op E;		\
	default: break;							\
    }									\
}

MakeM1	(mono_clear, 	,  (LogInt) 0)
MakeM2	(mono_and,	, &  con)
MakeM2	(mono_andrev,	, & ~con)
MakeM2	(mono_copy,	,       )
MakeM2	(mono_andinv, ~	, &  con)
MakeM1	(mono_noop,	,    con)
MakeM2	(mono_xor,	, ^  con)
MakeM2	(mono_or,	, |  con)
MakeM2	(mono_nor,    ~	, & ~con)
MakeM2	(mono_equiv,  ~	, ^  con)
MakeM1	(mono_invert,	,   ~con)
MakeM2 	(mono_orrev,	, | ~con)
MakeM2	(mono_copyinv,~	,       )
MakeM2	(mono_orinv,  ~	, |  con)
MakeM2	(mono_nand,   ~	, | ~con)
MakeM1 	(mono_set,	,   ~(LogInt) 0)

#define dyad_clear	mono_clear
MakeD2	(dyad_and,	, &  )
MakeD2	(dyad_andrev,	, & ~)
#define dyad_copy	mono_copy
MakeD2	(dyad_andinv, ~	, &  )
MakeD1	(dyad_noop,	mono_copy)
MakeD2	(dyad_xor,	, ^  )
MakeD2	(dyad_or,	, |  )
MakeD2	(dyad_nor,    ~	, & ~)
MakeD2	(dyad_equiv,  ~	, ^  )
MakeD1	(dyad_invert,	mono_copyinv)
MakeD2 	(dyad_orrev,	, | ~)
#define dyad_copyinv	mono_copyinv
MakeD2	(dyad_orinv,  ~	, |  )
MakeD2	(dyad_nand,   ~	, | ~)
#define dyad_set	mono_set

static void (*action_mono[16])() = {
	mono_clear,	mono_and,	mono_andrev,	mono_copy,
	mono_andinv,	mono_noop,	mono_xor,	mono_or,
	mono_nor,	mono_equiv,	mono_invert,	mono_orrev,
	mono_copyinv,	mono_orinv,	mono_nand,	mono_set
};
static void (*action_dyad[16])() = {
	dyad_clear,	dyad_and,	dyad_andrev,	dyad_copy,
	dyad_andinv,	dyad_noop,	dyad_xor,	dyad_or,
	dyad_nor,	dyad_equiv,	dyad_invert,	dyad_orrev,
	dyad_copyinv,	dyad_orinv,	dyad_nand,	dyad_set
};

/*------------------------------------------------------------------------
---------------------  ROI operations work on subranges ------------------
------------------------------------------------------------------------*/

/* MROI: (*(pvt->action)) (dvoid, pvt->constant, run, ix); */
/* DROI: (*(pvt->action)) (dvoid, src2, run, ix); */

#define ONES ~((LogInt)0)

#define MakeROIM1(name, rev, op)					\
static void name(d,con,run,ix)						\
    LogInt *d, con;							\
    CARD32 run, ix;							\
{									\
    LogInt D, M;							\
    CARD32 sbit = ix & LOGMASK;						\
    d += (ix >>= LOGSHIFT); 						\
    if (sbit + run > LOGSIZE) {						\
	if (sbit) {							\
	    M = BitRight(ONES,sbit); run -= (LOGSIZE - sbit);		\
	    D = *d; *d = (D & ~M) | (op & M); d++;			\
	}								\
	for (sbit = run >> LOGSHIFT; sbit > 0; sbit--, d++ ) {		\
	    *d = op;							\
	}								\
	if (run &= LOGMASK) {						\
	    M = ~BitRight(ONES,run);					\
	    D = *d; *d = (D & ~M) | (op & M);				\
	}								\
    } else {								\
	M = BitRight(ONES,sbit) & ~(BitRight(ONES,sbit+run));		\
	D = *d; *d = (D & ~M) | (op & M);				\
    }									\
}

#define MakeROIM2(name, rev, op)					\
static void name(d,con,run,ix)						\
    LogInt *d, con;							\
    CARD32 run, ix;							\
{									\
    LogInt D, M;							\
    CARD32 sbit = ix & LOGMASK;						\
    d += (ix >>= LOGSHIFT); 						\
    if (sbit + run > LOGSIZE) {						\
	if (sbit) {							\
	    M = BitRight(ONES,sbit); run -= (LOGSIZE - sbit);		\
	    D = *d; *d = (D & ~M) | (((rev D) op) & M); d++;		\
	}								\
	for (sbit = run >> LOGSHIFT; sbit > 0; sbit--, d++ ) {		\
	    *d = (rev *d) op;						\
	}								\
	if (run &= LOGMASK) {						\
	    M = ~BitRight(ONES,run);					\
	    D = *d; *d = (D & ~M) | (((rev D) op) & M);			\
	}								\
    } else {								\
	M = BitRight(ONES,sbit) & ~(BitRight(ONES,sbit+run));		\
	D = *d; *d = (D & ~M) | (((rev D) op) & M); d++;		\
    }									\
}

#define MakeROIM3(name)		 					\
static void name(d,con,run,ix)						\
    LogInt *d, con;							\
    CARD32 run, ix;							\
{									\
    return;	/* NoOp */						\
}

#define MakeROID1(name, rev, op) 					\
static void name(d,src2,run,ix)						\
    LogInt *d, *src2;							\
    CARD32 run, ix;							\
{									\
    LogInt D, M;							\
    CARD32 sbit = ix & LOGMASK;						\
    ix >>= LOGSHIFT; d += ix; src2 += ix;				\
    if (sbit + run > LOGSIZE) {						\
	if (sbit) {							\
	    M = BitRight(ONES,sbit); run -= (LOGSIZE - sbit);		\
	    D = *d; *d = (D & ~M) | ((op *src2++) & M); d++;		\
	}								\
	for (sbit = run >> LOGSHIFT; sbit > 0; sbit--, d++, src2++ ) {	\
	    *d = op *src2;						\
	}								\
	if (run &= LOGMASK) {						\
	    M = ~BitRight(ONES,run);					\
	    D = *d; *d = (D & ~M) | ((op *src2) & M); 			\
	}								\
    } else {								\
	M = BitRight(ONES,sbit) & ~(BitRight(ONES,sbit+run));		\
	D = *d; *d = (D & ~M) | ((op *src2) & M);			\
    }									\
}

#define MakeROID2(name, rev, op) 					\
static void name(d,src2,run,ix)						\
    LogInt *d, *src2;							\
    CARD32 run, ix;							\
{									\
    LogInt D, M;							\
    CARD32 sbit = ix & LOGMASK;						\
    ix >>= LOGSHIFT; d += ix; src2 += ix;				\
    if (sbit + run > LOGSIZE) {						\
	if (sbit) {							\
	    M = BitRight(ONES,sbit); run -= (LOGSIZE - sbit);		\
	    D = *d; *d = (D & ~M) | (((rev D) op *src2++) & M); d++;	\
	}								\
	for (sbit = run >> LOGSHIFT; sbit > 0; sbit--, d++, src2++ ) {	\
	    *d = (rev *d) op *src2;					\
	}								\
	if (run &= LOGMASK) {						\
	    M = ~BitRight(ONES,run);					\
	    D = *d; *d = (D & ~M) | (((rev D) op *src2) & M);		\
	}								\
    } else {								\
	M = BitRight(ONES,sbit) & ~(BitRight(ONES,sbit+run));		\
	D = *d; *d = (D & ~M) | (((rev D) op *src2) & M);		\
    }									\
}

MakeROIM1	(mroi_clear, 	,  (LogInt) 0)
MakeROIM2	(mroi_and,	, &  con)
MakeROIM2	(mroi_andrev,	, & ~con)
MakeROIM3	(mroi_copy		)
MakeROIM2	(mroi_andinv, ~	, &  con)
MakeROIM1	(mroi_noop,	,    con)
MakeROIM2	(mroi_xor,	, ^  con)
MakeROIM2	(mroi_or,	, |  con)
MakeROIM2	(mroi_nor,    ~	, & ~con)
MakeROIM2	(mroi_equiv,  ~	, ^  con)
MakeROIM1	(mroi_invert,	,   ~con)
MakeROIM2 	(mroi_orrev,	, | ~con)
MakeROIM2	(mroi_copyinv,~	,       )
MakeROIM2	(mroi_orinv,  ~	, |  con)
MakeROIM2	(mroi_nand,   ~	, | ~con)
MakeROIM1 	(mroi_set,	,   ~(LogInt) 0)

#define		droi_clear	mroi_clear
MakeROID2	(droi_and,	, &  )
MakeROID2	(droi_andrev,	, & ~)
#define		droi_copy	mroi_copy
MakeROID2	(droi_andinv, ~	, &  )
MakeROID1	(droi_noop,	,    )
MakeROID2	(droi_xor,	, ^  )
MakeROID2	(droi_or,	, |  )
MakeROID2	(droi_nor,    ~	, & ~)
MakeROID2	(droi_equiv,  ~	, ^  )
MakeROID1	(droi_invert,	, ~  )
MakeROID2 	(droi_orrev,	, | ~)
#define		droi_copyinv	mroi_copyinv
MakeROID2	(droi_orinv,  ~	, |  )
MakeROID2	(droi_nand,   ~	, | ~)
#define		droi_set	mroi_set

static void (*action_monoROI[16])() = {
	mroi_clear,	mroi_and,	mroi_andrev,	mroi_copy,
	mroi_andinv,	mroi_noop,	mroi_xor,	mroi_or,
	mroi_nor,	mroi_equiv,	mroi_invert,	mroi_orrev,
	mroi_copyinv,	mroi_orinv,	mroi_nand,	mroi_set
};
static void (*action_dyadROI[16])() = {
	droi_clear,	droi_and,	droi_andrev,	droi_copy,
	droi_andinv,	droi_noop,	droi_xor,	droi_or,
	droi_nor,	droi_equiv,	droi_invert,	droi_orrev,
	droi_copyinv,	droi_orinv,	droi_nand,	droi_set
};

/*------------------------------------------------------------------------
----------------------------  some other goodies -------------------------
------------------------------------------------------------------------*/

static void action_tail(d, src, run, ix)
    LogInt *d, *src;
    CARD32 run, ix;
{
    LogInt D, M;
    CARD32 sbit = ix & LOGMASK;
    ix >>= LOGSHIFT; d += ix; src += ix;
    if (sbit + run > LOGSIZE) {
	if (sbit) {
	    M = BitRight(ONES,sbit); run -= (LOGSIZE - sbit);
	    D = *d; *d = (D & ~M) | (*src++ & M); d++;
	}
	for (sbit = run >> LOGSHIFT; sbit > 0; sbit--) {
	    *d++ = *src++;
	}
	if (run &= LOGMASK) {	/* may be unnecessary due to padding */
	    M = ~BitRight(ONES,run);
	    D = *d; *d = (D & ~M) | (*src & M);
	}
    } else {
	M = BitRight(ONES,sbit) & ~(BitRight(ONES,sbit+run));
	D = *d; *d = (D & ~M) | (*src & M);
    }
}

static CARD32 replicate_const(levels, con)
    CARD32 levels;
    CARD32 con;
{
    /* XXX needs to look for partial powers and clear bits.  Not
    ** necessary until we also add masking operations in logical
    ** operations.  not a pretty thing. sigh.
    */
    if (levels <= 0x100) {
	if ( levels > 2) {
	    con &= 0xff; con |= con << 8; return con | (con << 16); 
	} 
	if (levels == 2) {
	    return con & 1 ? ~0 : 0;
	}
	return 0;
    } else if (levels < 0x10000) {
	con &= 0xffff; return con | (con << 16);
    }
    return con & 0xffffff;
}
	
/* end module mpqlogic.c */
