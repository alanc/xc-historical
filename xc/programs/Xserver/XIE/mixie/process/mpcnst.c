/* $XConsortium$ */
/**** module mpcnst.c ****/
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
  
	mpcnst.c -- DDXIE constrain element
  
	Dean Verheiden && Larry Hare -- AGE Logic, Inc. May, 1993
  
*****************************************************************************/


#define _XIEC_MPCNST
#define _XIEC_PCNST

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
int	miAnalyzeConstrain();

/*
 *  routines used internal to this module
 */
static int CreateConstrain();
static int InitializeConstrain();
static int ActivateConstrain();
static int FlushConstrain();
static int ResetConstrain();
static int DestroyConstrain();

/*
 * DDXIE ImportClientPhoto entry points
 */
static ddElemVecRec ConstrainVec = {
  CreateConstrain,
  InitializeConstrain,
  ActivateConstrain,
  FlushConstrain,
  ResetConstrain,
  DestroyConstrain
  };


/*------------------------------------------------------------------------
------------------------  Local declares and defines  --------------------
------------------------------------------------------------------------*/

typedef struct _mpconstraindef {
	void	(*action) ();
	void	*lut;
	double	pad[7];
} mpCnstPvtRec, *mpCnstPvtPtr;


static void (*hc_nop())();
static void (*HCp_Rb())(), (*HCp_bb())(), (*HCp_Bb())(),
			   (*HCp_Pb())(), (*HCp_Qb())();
static void (*HCp_RB())(), (*HCp_bB())(), (*HCp_BB())(),
			   (*HCp_PB())(), (*HCp_QB())();
static void (*HCp_RP())(), (*HCp_bP())(), (*HCp_BP())(),
			   (*HCp_PP())(), (*HCp_QP())();
static void (*HCp_RQ())(), (*HCp_bQ())(), (*HCp_BQ())(),
			   (*HCp_PQ())(), (*HCp_QQ())();

static void (*(*prep_hc[5][5])())() = {
	hc_nop, hc_nop, hc_nop, hc_nop, hc_nop, /* [out=0][inp=0...4] */
	HCp_Rb, HCp_bb, HCp_Bb, HCp_Pb, HCp_Qb,	/* [out=1][inp=0...4] */
	HCp_RB, HCp_bB, HCp_BB, HCp_PB, HCp_QB,	/* [out=2][inp=0...4] */
	HCp_RP, HCp_bP, HCp_BP, HCp_PP, HCp_QP,	/* [out=3][inp=0...4] */
	HCp_RQ, HCp_bQ, HCp_BQ, HCp_PQ, HCp_QQ,	/* [out=4][inp=0...4] */
};

static void (*cs_nop())();
static void (*CSp_Rb())(), (*CSp_bb())(), (*CSp_Bb())(),
			   (*CSp_Pb())(), (*CSp_Qb())();
static void (*CSp_RB())(), (*CSp_bB())(), (*CSp_BB())(),
			   (*CSp_PB())(), (*CSp_QB())();
static void (*CSp_RP())(), (*CSp_bP())(), (*CSp_BP())(),
			   (*CSp_PP())(), (*CSp_QP())();
static void (*CSp_RQ())(), (*CSp_bQ())(), (*CSp_BQ())(),
			   (*CSp_PQ())(), (*CSp_QQ())();

static void (*(*prep_cs[5][5])())() = {
	cs_nop, cs_nop, cs_nop, cs_nop, cs_nop, /* [0][0...4] */
	CSp_Rb, CSp_bb, CSp_Bb, CSp_Pb, CSp_Qb,	/* [1][0...4] */
	CSp_RB, CSp_bB, CSp_BB, CSp_PB, CSp_QB,	/* [2][0...4] */
	CSp_RP, CSp_bP, CSp_BP, CSp_PP, CSp_QP,	/* [3][0...4] */
	CSp_RQ, CSp_bQ, CSp_BQ, CSp_PQ, CSp_QQ,	/* [4][0...4] */
};

/*------------------------------------------------------------------------
------------------------  fill in the vector  ---------------------------
------------------------------------------------------------------------*/
int miAnalyzeConstrain(flo,ped)
    floDefPtr flo;
    peDefPtr  ped;
{

    ped->ddVec = ConstrainVec;

    /* based on the technique, fill in the appropriate entry point vector */

    /* XXX may have to defer some of this when modify is done */
    switch(((xieFloConstrain *)ped->elemRaw)->constrain) {
	case	xieValConstrainClipScale:
	case	xieValConstrainHardClip:
		break;
	default:
		ImplementationError(flo,ped, return(FALSE));
    }

    return TRUE;
}


/*------------------------------------------------------------------------
---------------------------- create peTex . . . --------------------------
------------------------------------------------------------------------*/
static int CreateConstrain(flo,ped)
    floDefPtr flo;
    peDefPtr  ped;
{
    return MakePETex(flo,ped,
		     xieValMaxBands * sizeof(mpCnstPvtRec),
		     FALSE,FALSE);
} 


/*------------------------------------------------------------------------
---------------------------- initialize peTex . . . ----------------------
------------------------------------------------------------------------*/

static int InitializeConstrain(flo,ped)
    floDefPtr flo;
    peDefPtr  ped;
{
    peTexPtr  pet = ped->peTex;
    pCnstDefPtr techpvt = ped->techPvt;
    mpCnstPvtPtr pvt = (mpCnstPvtPtr) pet->private;
    bandPtr oband;
    bandPtr iband;
    int band, nbands, status;

    status =  InitReceptors(flo,ped,0,1) && InitEmitter(flo,ped,0,-1);

    nbands = pet->receptor[SRCtag].inFlo->bands;
    iband = &(pet->receptor[SRCtag].band[0]);
    oband = &(pet->emitter[0]);

    for(band = 0; band < nbands; band++, pvt++, iband++, oband++) {

	int oo = IndexClass(oband->format->class);
	int ii = IndexClass(iband->format->class);

	switch(((xieFloConstrain *)ped->elemRaw)->constrain) {

	case	xieValConstrainClipScale:
    		pvt->action =
		    ((*(prep_cs[oo][ii])) (iband, oband, pvt, techpvt, band)); 
		break;
	case	xieValConstrainHardClip:
    		pvt->action =
		    ((*(prep_hc[oo][ii])) (iband, oband, pvt, techpvt, band)); 
		break;
	}
    }
    return status;
}

/*------------------------------------------------------------------------
----------------------------- crank some data ----------------------------
------------------------------------------------------------------------*/
static int ActivateConstrain(flo,ped,pet)
     floDefPtr flo;
     peDefPtr  ped;
     peTexPtr  pet;
{
    mpCnstPvtPtr pvt = (mpCnstPvtPtr) pet->private;
    int band, nbands = pet->receptor[SRCtag].inFlo->bands;
    bandPtr iband = &(pet->receptor[SRCtag].band[0]);
    bandPtr oband = &(pet->emitter[0]);

    for(band = 0; band < nbands; band++, pvt++, iband++, oband++) {
	register int bw = iband->format->width;
	void *ivoid, *ovoid;

	if (!(ivoid = GetCurrentSrc(void,flo,pet,iband)))
		continue;

	if (!pvt->action) {
	    do { /* pass a clone of the current src strip downstream */
		if(!PassStrip(flo,pet,oband,iband->strip))
		    return(FALSE);
	    	ivoid = GetSrc(void,flo,pet,iband,iband->maxLocal,TRUE);
	    } while (!ferrCode(flo) && ivoid) ;
	    FreeData(void, flo, pet, iband, iband->current);
	    continue;
	}

	if (!(ovoid = GetCurrentDst(void,flo,pet,oband)))
		continue;

	do {
	    (*(pvt->action)) (ivoid, ovoid, pvt, bw);
	    ivoid = GetNextSrc(void,flo,pet,iband,TRUE);
	    ovoid = GetNextDst(void,flo,pet,oband,TRUE);
	} while (!ferrCode(flo) && ivoid && ovoid) ;

	FreeData(void, flo, pet, iband, iband->current);
    }
    return TRUE;
}

/*------------------------------------------------------------------------
--------------------------- get rid of left overs ------------------------
------------------------------------------------------------------------*/
static int FlushConstrain(flo,ped)
    floDefPtr flo;
    peDefPtr  ped;
{
    return TRUE;
}               

/*------------------------------------------------------------------------
------------------------ get rid of run-time stuff -----------------------
------------------------------------------------------------------------*/
static int ResetConstrain(flo,ped)
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
static int DestroyConstrain(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
    if (ped->peTex) {
	mpCnstPvtPtr pvt = (mpCnstPvtPtr) (ped->peTex->private);
	if (pvt->lut) pvt->lut = (void *) XieFree(pvt->lut);
    }

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
------------------------  This guy must be nuts  -------------------------
------------------------------------------------------------------------*/

/*----------------------------- HardClip  --------------------------------*/

/*
**  DO_HCb	- consume bits, produce bits.
**  DO_HCc	- consume bits, produce otype.
**  DO_HCp	- consume itype, produce bits..
**  DO_HCfp	- consume floats, produce bits.
**  DO_HClt	- consume itype, produce otype (itype < otype).
**  DO_HCeq	- consume itype, produce otype (itype == otype).
**  DO_HCgt	- consume itype, produce otype (itype > otype)
**  DO_HCf	- consume floats, produce otype.
*/

static void constrain_nop(INP,OUTP,pvt,bw)
	void *INP; void *OUTP; mpCnstPvtPtr pvt; int bw;
{
	return;
}
static void (*hc_nop(iband,oband,pvt,techpvt,band))()
	bandPtr iband, oband; mpCnstPvtPtr pvt; pCnstDefPtr techpvt; int band;
{
	return constrain_nop;
}
static void (*cs_nop(iband,oband,pvt,techpvt,band))()
	bandPtr iband, oband; mpCnstPvtPtr pvt; pCnstDefPtr techpvt; int band;
{
	return constrain_nop;
}

static void clearbitline(INP,OUTP,pvt,bw)
	void *INP; void *OUTP; mpCnstPvtPtr pvt; int bw;
{
	bzero((char *)OUTP, (bw+7)>>3);
}

static void setbitline(INP,OUTP,pvt,bw)
	void *INP; void *OUTP; mpCnstPvtPtr pvt; int bw;
{
	memset((char *)OUTP, ~0, (bw+7)>>3);
}

#define LEVELSM1(T) *((T *) &(pvt->pad[0]))

#define DO_HCb(fn_prep,fn_do_a, fn_do_b,itype,otype) 			\
static void 								\
fn_do_a(INP,OUTP,pvt,bw) 						\
	void *INP; void *OUTP; mpCnstPvtPtr pvt; int bw; 		\
{ 									\
	bcopy((char *) INP, (char *)OUTP, (bw+7)>>3); 			\
} 									\
static void 								\
(*fn_prep (iband,oband,pvt,techpvt,band))() 				\
	bandPtr iband, oband; mpCnstPvtPtr pvt; pCnstDefPtr techpvt; int band; \
{ 									\
	if (iband->format->levels == 1) 				\
		return clearbitline; 					\
	if (oband->format->levels == 1) 				\
		return clearbitline; 					\
	return fn_do_a; 						\
} 

#define DO_HCc(fn_prep,fn_do_a, fn_do_b,itype,otype) 			\
static void 								\
fn_do_a(INP,OUTP,pvt,bw) 						\
	void *INP; void *OUTP; mpCnstPvtPtr pvt; int bw; 		\
{ 									\
	LogInt inval, M, *inp = (LogInt *) INP; 			\
	otype *outp = (otype *) OUTP; 					\
	for ( ; bw >= LOGSIZE ; bw -= LOGSIZE) 				\
		for (M=LOGLEFT, inval = *inp++; M; LOGRIGHT(M)) 	\
			*outp++ =  (inval & M) ? (otype) 1 : (otype) 0; \
	if (bw > 0) 							\
		for (M=LOGLEFT, inval = *inp++; bw; bw--, LOGRIGHT(M)) 	\
			*outp++ =  (inval & M) ? (otype) 1 : (otype) 0; \
} 									\
static void 								\
fn_do_b(INP,OUTP,pvt,bw) 						\
	void *INP; void *OUTP; mpCnstPvtPtr pvt; int bw; 		\
{ 									\
	bzero((char *)OUTP, bw * sizeof(otype)); 			\
} 									\
static void 								\
(*fn_prep (iband,oband,pvt,techpvt,band))() 				\
	bandPtr iband, oband; mpCnstPvtPtr pvt; pCnstDefPtr techpvt; int band; \
{ 									\
	if (iband->format->levels == 1) 				\
		return fn_do_b; 					\
	return fn_do_a; 						\
} 

#define DO_HCp(fn_prep,fn_do_a, fn_do_b,itype,otype) 			\
static void 								\
fn_do_a(INP,OUTP,pvt,bw) 						\
	void *INP; void *OUTP; mpCnstPvtPtr pvt; int bw; 		\
{ 									\
	itype *inp = (itype *) INP; 					\
	LogInt outval, M, *outp = (LogInt *) OUTP; 			\
	for ( ; bw >= LOGSIZE ; *outp++ = outval, bw -= LOGSIZE) 	\
		for (M=LOGLEFT, outval = 0; M; LOGRIGHT(M)) 		\
			if (*inp++) 					\
				outval |= M; 				\
	if (bw > 0) { 							\
		for (M=LOGLEFT, outval = 0; bw; bw--, LOGRIGHT(M)) 	\
			if (*inp++) 					\
				outval |= M; 				\
		*outp = outval; 					\
	} 								\
} 									\
static void 								\
(*fn_prep (iband,oband,pvt,techpvt,band))() 				\
	bandPtr iband, oband; mpCnstPvtPtr pvt; pCnstDefPtr techpvt; int band; \
{ 									\
	if (oband->format->levels == 1) 				\
		return clearbitline; 					\
	return fn_do_a; 						\
} 

#define DO_HCfp(fn_prep,fn_do_a, fn_do_b,itype,otype) 			\
static void 								\
fn_do_a(INP,OUTP,pvt,bw) 						\
	void *INP; void *OUTP; mpCnstPvtPtr pvt; int bw; 		\
{ 									\
	itype *inp = (itype *) INP; 					\
	LogInt outval, M, *outp = (LogInt *) OUTP; 			\
	for ( ; bw >= LOGSIZE ; *outp++ = outval, bw -= LOGSIZE) 	\
		for (M=LOGLEFT, outval = 0; M; LOGRIGHT(M)) 		\
			if (*inp++ >= (itype) 1.0) 			\
				outval |= M; 				\
	if (bw > 0) { 							\
		for (M=LOGLEFT, outval = 0; bw; bw--, LOGRIGHT(M)) 	\
			if (*inp++ >= (itype) 1.0) 			\
				outval |= M; 				\
		*outp = outval; 					\
	} 								\
} 									\
static void 								\
(*fn_prep (iband,oband,pvt,techpvt,band))() 				\
	bandPtr iband, oband; mpCnstPvtPtr pvt; pCnstDefPtr techpvt; int band; \
{ 									\
	if (oband->format->levels == 1) 				\
		return clearbitline; 					\
	return fn_do_a; 						\
} 

#define DO_HClt(fn_prep,fn_do_a, fn_do_b,itype,otype) 			\
static void 								\
fn_do_b(INP,OUTP,pvt,bw) 						\
	void *INP; void *OUTP; mpCnstPvtPtr pvt; int bw; 		\
{ 									\
	itype *inp = (itype *) INP; 					\
	otype *outp = (otype *) OUTP; 					\
	while (bw-- > 0 ) { 						\
		*outp++ = (otype) *inp++; 				\
	} 								\
} 									\
static void 								\
(*fn_prep (iband,oband,pvt,techpvt,band))() 				\
	bandPtr iband, oband; mpCnstPvtPtr pvt; pCnstDefPtr techpvt; int band; \
{ 									\
	return fn_do_b; 						\
} 

#define DO_HCeq(fn_prep,fn_do_a, fn_do_b,itype,otype) 			\
static void 								\
fn_do_a(INP,OUTP,pvt,bw) 						\
	void *INP; void *OUTP; mpCnstPvtPtr pvt; int bw; 		\
{ 									\
	itype *inp = (itype *) INP; 					\
	otype *outp = (otype *) OUTP; 					\
	itype inval, lm1 = LEVELSM1(itype); 				\
	while (bw-- > 0 ) { 						\
		if ((inval = *inp++) > lm1) inval = lm1; 		\
		*outp++ = (otype) inval; 				\
	} 								\
} 									\
static void 								\
fn_do_b(INP,OUTP,pvt,bw) 						\
	void *INP; void *OUTP; mpCnstPvtPtr pvt; int bw; 		\
{ 									\
	bcopy((char *) INP, (char *)OUTP, bw * sizeof(otype)); 		\
} 									\
static void 								\
(*fn_prep (iband,oband,pvt,techpvt,band))() 				\
	bandPtr iband, oband; mpCnstPvtPtr pvt; pCnstDefPtr techpvt; int band; \
{ 									\
	itype ilm1 = (itype)( (iband->format->levels) 			\
				? iband->format->levels - 1 : ~0); 	\
	itype olm1 = (itype)( (oband->format->levels) 			\
				? oband->format->levels - 1 : ~0); 	\
	if (ilm1 < olm1) { 						\
		/*special hint to PassStrip */				\
		return 	(void (*) ()) 0; 				\
	} 								\
	LEVELSM1(itype) = (itype) olm1; 				\
	return fn_do_a; 						\
} 

#define DO_HCgt(fn_prep,fn_do_a, fn_do_b,itype,otype) 			\
static void 								\
fn_do_a(INP,OUTP,pvt,bw) 						\
	void *INP; void *OUTP; mpCnstPvtPtr pvt; int bw; 		\
{ 									\
	itype *inp = (itype *) INP; 					\
	otype *outp = (otype *) OUTP; 					\
	itype inval, lm1 = LEVELSM1(itype); 				\
	while (bw-- > 0 ) { 						\
		if ((inval = *inp++) > lm1) inval = lm1; 		\
		*outp++ = (otype) inval; 				\
	} 								\
} 									\
static void 								\
(*fn_prep (iband,oband,pvt,techpvt,band))() 				\
	bandPtr iband, oband; mpCnstPvtPtr pvt; pCnstDefPtr techpvt; int band; \
{ 									\
	otype olm1 = (otype)( (oband->format->levels) 			\
				? oband->format->levels - 1 : ~0); 	\
	LEVELSM1(itype) = (itype) olm1; 				\
	return fn_do_a; 						\
} 


#define DO_HCf(fn_prep,fn_do_a,fn_do_b,itype,otype) 			\
static void 								\
fn_do_a(INP,OUTP,pvt,bw) 						\
	void *INP; void *OUTP; mpCnstPvtPtr pvt; int bw; 		\
{ 									\
	itype *inp = (itype *) INP; 					\
	otype *outp = (otype *) OUTP; 					\
	itype inval, olm1 = *((itype *) &(pvt->pad[0])); 		\
	while (bw-- > 0 ) { 						\
		if ((inval = *inp++) > olm1) inval = olm1; 		\
		else if (inval < 0) inval = (itype) 0; 			\
		*outp++ = (otype) inval; 				\
	} 								\
} 									\
static void 								\
(*fn_prep (iband,oband,pvt,techpvt,band))() 				\
    bandPtr iband, oband; mpCnstPvtPtr pvt; pCnstDefPtr techpvt; int band; \
{ 									\
	otype olm1 = (otype)( (oband->format->levels) 			\
				? oband->format->levels - 1 : ~0); 	\
	*((itype *) &(pvt->pad[0])) = olm1; 				\
	return fn_do_a; 						\
}

DO_HCb	(HCp_bb, HCa_bb, HCb_bb, BitPixel,  BitPixel)
DO_HCp	(HCp_Bb, HCa_Bb, HCb_Bb, BytePixel, BitPixel)
DO_HCp	(HCp_Pb, HCa_Pb, HCb_Pb, PairPixel, BitPixel)
DO_HCp	(HCp_Qb, HCa_Qb, HCb_Qb, QuadPixel, BitPixel)
DO_HCfp	(HCp_Rb, HCa_Rb, HCb_Rb, RealPixel, BitPixel)

DO_HCc	(HCp_bB, HCa_bB, HCb_bB, BitPixel,  BytePixel)
DO_HCeq	(HCp_BB, HCa_BB, HCb_BB, BytePixel, BytePixel)
DO_HCgt	(HCp_PB, HCa_PB, HCb_PB, PairPixel, BytePixel)
DO_HCgt	(HCp_QB, HCa_QB, HCb_QB, QuadPixel, BytePixel)
DO_HCf	(HCp_RB, HCa_RB, HCb_RB, RealPixel, BytePixel)

DO_HCc	(HCp_bP, HCa_bP, HCb_bP, BitPixel,  PairPixel)
DO_HClt	(HCp_BP, HCa_BP, HCb_BP, BytePixel, PairPixel)
DO_HCeq	(HCp_PP, HCa_PP, HCb_PP, PairPixel, PairPixel)
DO_HCgt	(HCp_QP, HCa_QP, HCb_QP, QuadPixel, PairPixel)
DO_HCf	(HCp_RP, HCa_RP, HCb_RP, RealPixel, PairPixel)

DO_HCc	(HCp_bQ, HCa_bQ, HCb_bQ, BitPixel,  QuadPixel)
DO_HClt	(HCp_BQ, HCa_BQ, HCb_BQ, BytePixel, QuadPixel)
DO_HClt	(HCp_PQ, HCa_PQ, HCb_PQ, PairPixel, QuadPixel)
DO_HCeq	(HCp_QQ, HCa_QQ, HCb_QQ, QuadPixel, QuadPixel)
DO_HCf	(HCp_RQ, HCa_RQ, HCb_RQ, RealPixel, QuadPixel)

#undef  LEVELSM1

/*----------------------------- ClipScale --------------------------------*/

typedef RealPixel ConstrainFloat; /* XXX ??? */
#define HALF	((RealPixel) 0.5)

#define	LBOUND(T)	*((T *) &(pvt->pad[0]))
#define	UBOUND(T)	*((T *) &(pvt->pad[1]))
#define	OLOW(T)		*((T *) &(pvt->pad[2]))
#define	OHIGH(T)	*((T *) &(pvt->pad[3]))
#define	FLT_SF		*((ConstrainFloat *) &(pvt->pad[4]))
#define	FLT_OFF		*((ConstrainFloat *) &(pvt->pad[5]))
#define	INT_SF		*((INT32 *) &(pvt->pad[4]))
#define	INT_OFF		*((INT32 *) &(pvt->pad[5]))

#define DO_CSb(fn_prep,fn_do,fn_do_b,itype,otype) 			\
static void 								\
fn_do(INP,OUTP,pvt,bw) 							\
	void *INP; void *OUTP; mpCnstPvtPtr pvt; int bw; 		\
{ 									\
	bcopy((char *)INP, (char *) OUTP, (bw+7)>>3); 			\
} 									\
static void 								\
fn_do_b(INP,OUTP,pvt,bw) 						\
	void *INP; void *OUTP; mpCnstPvtPtr pvt; int bw; 		\
{ 									\
	LogInt *inp = (LogInt *)INP; LogInt *outp = (LogInt *) OUTP; 	\
	bw = (bw + LOGMASK) / LOGSIZE; 					\
	while (bw-- > 0) *outp++ = *inp++ ^ (LogInt) ~0; 		\
} 									\
static void 								\
(*fn_prep (iband,oband,pvt,techpvt,band))()				\
    bandPtr iband, oband; mpCnstPvtPtr pvt; pCnstDefPtr techpvt; int band; \
{									\
	int ordered = (techpvt->input_low[band] < techpvt->input_high[band]); \
	if (iband->format->levels == 1)					\
		return clearbitline;					\
	if (oband->format->levels == 1)					\
		return clearbitline;					\
	/* XXX: Solve equation. (ihigh+ilow)/2>0 --> all zeros, etc */	\
	return ordered ? fn_do : fn_do_b;				\
} 


#define DO_CSc(fn_prep,fn_do,fn_do_b,itype,otype) 			\
static void 								\
fn_do(INP,OUTP,pvt,bw) 							\
	void *INP; void *OUTP; mpCnstPvtPtr pvt; int bw; 		\
{ 									\
	LogInt inval, M, *inp = (LogInt *) INP; 			\
	otype *outp = (otype *) OUTP; 					\
        otype olow = OLOW(otype), ohigh = OHIGH(otype); 		\
	for ( ; bw >= LOGSIZE ; bw -= LOGSIZE) 				\
		for (M=LOGLEFT, inval = *inp++; M; LOGRIGHT(M)) 	\
			*outp++ =  (inval & M) ? ohigh : olow ; 	\
	if (bw > 0) 							\
		for (M=LOGLEFT, inval = *inp++; bw; bw--, LOGRIGHT(M)) 	\
			*outp++ =  (inval & M) ? ohigh : olow ; 	\
} 									\
static void 								\
(*fn_prep (iband,oband,pvt,techpvt,band))() 				\
    bandPtr iband, oband; mpCnstPvtPtr pvt; pCnstDefPtr techpvt; int band; \
{ 									\
	int ordered = (techpvt->input_low[band] < techpvt->input_high[band]); \
	/* TODO NOTE: Solve equation.  OHIGH is for when bit is 1. */ 	\
	/*  Prep guarantees ilow != ihigh, but what if ilow > ihigh */ 	\
	if (ordered) { /* 1 bit selects output_high */ 			\
		OLOW(otype) =  techpvt->output_low[band]; 		\
		OHIGH(otype) = techpvt->output_high[band]; 		\
	} else { /* 1 bit selects output_low */ 			\
		OHIGH(otype) =  techpvt->output_low[band]; 		\
		OLOW(otype) = techpvt->output_high[band]; 		\
	} 								\
	return fn_do; 							\
} 


	/* uses 4way bit expansion utility */
#define DO_CScx(fn_prep,fn_do,fn_do_b,itype,otype)                      \
static void 								\
fn_do(INP,OUTP,pvt,bw) 							\
	void *INP; void *OUTP; mpCnstPvtPtr pvt; int bw; 		\
{ 									\
	void  bitexpand();						\
        otype olow = OLOW(otype), ohigh = OHIGH(otype); 		\
	bitexpand(INP, OUTP, bw, olow, ohigh);				\
} 									\
static void 								\
(*fn_prep (iband,oband,pvt,techpvt,band))() 				\
    bandPtr iband, oband; mpCnstPvtPtr pvt; pCnstDefPtr techpvt; int band; \
{ 									\
	int ordered = (techpvt->input_low[band] < techpvt->input_high[band]); \
	/* TODO NOTE: Solve equation.  OHIGH is for when bit is 1. */ 	\
	/*  Prep guarantees ilow != ihigh, but what if ilow > ihigh */ 	\
	if (ordered) { /* 1 bit selects output_high */ 			\
		OLOW(otype) =  techpvt->output_low[band]; 		\
		OHIGH(otype) = techpvt->output_high[band]; 		\
	} else { /* 1 bit selects output_low */ 			\
		OHIGH(otype) =  techpvt->output_low[band]; 		\
		OLOW(otype) = techpvt->output_high[band]; 		\
	} 								\
	return fn_do;							\
} 


#define DO_CSp(fn_prep,fn_do,fn_do_b,itype,otype) 			\
static void 								\
fn_do(INP,OUTP,pvt,bw) 							\
	void *INP; void *OUTP; mpCnstPvtPtr pvt; int bw; 		\
{ 									\
	itype *inp = (itype *) INP; 					\
	LogInt outval, M, *outp = (LogInt *) OUTP; 			\
        itype imedian = UBOUND(itype); 					\
	for ( ; bw >= LOGSIZE ; *outp++ = outval, bw -= LOGSIZE) 	\
		for (M=LOGLEFT, outval = 0; M; LOGRIGHT(M)) 		\
			if (*inp++ > imedian) 				\
				outval |= M; 				\
	if (bw > 0) { 							\
		for (M=LOGLEFT, outval = 0; bw; bw--, LOGRIGHT(M)) 	\
			if (*inp++ > imedian) 				\
				outval |= M; 				\
		*outp = outval; 					\
	} 								\
} 									\
static void 								\
fn_do_b(INP,OUTP,pvt,bw) 						\
	void *INP; void *OUTP; mpCnstPvtPtr pvt; int bw; 		\
{ 									\
	itype *inp = (itype *) INP; 					\
	LogInt outval, M, *outp = (LogInt *) OUTP; 			\
        itype imedian = UBOUND(itype); 					\
	for ( ; bw >= LOGSIZE ; *outp++ = outval, bw -= LOGSIZE) 	\
		for (M=LOGLEFT, outval = 0; M; LOGRIGHT(M)) 		\
			if (*inp++ <= imedian) 				\
				outval |= M; 				\
	if (bw > 0) { 							\
		for (M=LOGLEFT, outval = 0; bw; bw--, LOGRIGHT(M)) 	\
			if (*inp++ <= imedian) 				\
				outval |= M; 				\
		*outp = outval; 					\
	} 								\
} 									\
static void 								\
(*fn_prep (iband,oband,pvt,techpvt,band))() 				\
    bandPtr iband, oband; mpCnstPvtPtr pvt; pCnstDefPtr techpvt; int band; \
{ 									\
	itype i1, i2; 							\
	otype olow = techpvt->output_low[band]; 			\
	otype ohigh = techpvt->output_high[band]; 			\
	int ordered = (techpvt->input_low[band] < techpvt->input_high[band]); \
	if (olow == ohigh) 						\
		return (olow == 0) ? clearbitline : setbitline; 	\
	/* round to ints first ?? */ 					\
	i2 = (itype) (techpvt->input_high[band] + HALF); 		\
	i1 = (itype) (techpvt->input_low[band] + HALF); 		\
	UBOUND(itype) = (i1 + i2) / 2; 					\
	return ordered ? fn_do : fn_do_b; 				\
} 

/* NOTE: Same as DO_CSp except median calculatio does not round */
#define DO_CSfp(fn_prep,fn_do,fn_do_b,itype,otype) 			\
static void 								\
fn_do(INP,OUTP,pvt,bw) 							\
	void *INP; void *OUTP; mpCnstPvtPtr pvt; int bw; 		\
{ 									\
	itype *inp = (itype *) INP; 					\
	LogInt outval, M, *outp = (LogInt *) OUTP; 			\
        itype imedian = UBOUND(itype); 					\
	for ( ; bw >= LOGSIZE ; *outp++ = outval, bw -= LOGSIZE) 	\
		for (M=LOGLEFT, outval = 0; M; LOGRIGHT(M)) 		\
			if (*inp++ > imedian) 				\
				outval |= M; 				\
	if (bw > 0) { 							\
		for (M=LOGLEFT, outval = 0; bw; bw--, LOGRIGHT(M)) 	\
			if (*inp++ > imedian) 				\
				outval |= M; 				\
		*outp = outval; 					\
	} 								\
} 									\
static void 								\
fn_do_b(INP,OUTP,pvt,bw) 						\
	void *INP; void *OUTP; mpCnstPvtPtr pvt; int bw; 		\
{ 									\
	itype *inp = (itype *) INP; 					\
	LogInt outval, M, *outp = (LogInt *) OUTP; 			\
        itype imedian = UBOUND(itype); 					\
	for ( ; bw >= LOGSIZE ; *outp++ = outval, bw -= LOGSIZE) 	\
		for (M=LOGLEFT, outval = 0; M; LOGRIGHT(M)) 		\
			if (*inp++ <= imedian) 				\
				outval |= M; 				\
	if (bw > 0) { 							\
		for (M=LOGLEFT, outval = 0; bw; bw--, LOGRIGHT(M)) 	\
			if (*inp++ <= imedian) 				\
				outval |= M; 				\
		*outp = outval; 					\
	} 								\
} 									\
static void 								\
(*fn_prep (iband,oband,pvt,techpvt,band))() 				\
    bandPtr iband, oband; mpCnstPvtPtr pvt; pCnstDefPtr techpvt; int band; \
{ 									\
	otype olow = techpvt->output_low[band]; 			\
	otype ohigh = techpvt->output_high[band]; 			\
	int ordered = (techpvt->input_low[band] < techpvt->input_high[band]); \
	if (olow == ohigh) 						\
		return (olow == 0) ? clearbitline : setbitline; 	\
	/* round to ints first ?? */ 					\
	UBOUND(itype) = ( (techpvt->input_high[band]) + 		\
			  (techpvt->input_low[band]) ) * HALF; 		\
	return ordered ? fn_do : fn_do_b; 				\
} 

/* XXX need integer or LUT based approach */
/* XXX need to insure that rounded delta input != 0. */
/* XXX verify negative ramp capability, probable rounding error */
#define DO_CS(fn_prep,fn_do,fn_do_b,itype,otype) 			\
static void 								\
fn_do(INP,OUTP,pvt,bw) 							\
	void *INP; void *OUTP; mpCnstPvtPtr pvt; int bw; 		\
{ 									\
	itype *inp = (itype *) INP; otype *outp = (otype *) OUTP; 	\
	itype inv, lbound = LBOUND(itype), ubound = UBOUND(itype); 	\
        otype out, olow = OLOW(otype), ohigh = OHIGH(otype); 		\
	ConstrainFloat sf = FLT_SF, offset = FLT_OFF; 			\
	while (bw-- > 0) { 						\
		inv = *inp++; 						\
		out = olow; 						\
		if (inv >= ubound) 					\
			out = ohigh; 					\
		else if (inv > lbound)	 				\
			out = (otype)(sf * inv + offset); 		\
		*outp++ = out; 						\
	} 								\
} 									\
static void 								\
(*fn_prep (iband,oband,pvt,techpvt,band))() 				\
    bandPtr iband, oband; mpCnstPvtPtr pvt; pCnstDefPtr techpvt; int band; \
{ 									\
	otype olow; ConstrainFloat sf; 					\
	OLOW(otype) = olow = techpvt->output_low[band]; 		\
	OHIGH(otype) = techpvt->output_high[band]; 			\
	LBOUND(itype) = (itype) (techpvt->input_low[band] + HALF); 	\
	UBOUND(itype) = (itype) (techpvt->input_high[band] + HALF); 	\
	FLT_SF = sf = (OHIGH(otype) - OLOW(otype) /			\
		  (UBOUND(itype) - LBOUND(itype));			\
	FLT_OFF = HALF + olow - sf *  techpvt->input_low[band];		\
	return fn_do; 							\
}

#if defined(USE_FLOATS)
#define DO_CSi(fn_prep,fn_do,fn_do_b,itype,otype,shift)			\
	DO_CSf(fn_prep,fn_do,fn_do_b,itype,otype) 
#else
/* XXX: A lookup table would be better, at least for short things */
#define DO_CSi(fn_prep,fn_do,fn_do_b,itype,otype,shift)			\
static void 								\
fn_do(INP,OUTP,pvt,bw) 							\
	void *INP; void *OUTP; mpCnstPvtPtr pvt; int bw; 		\
{ 									\
	itype *inp = (itype *) INP; otype *outp = (otype *) OUTP; 	\
        otype out, olow = OLOW(otype), ohigh = OHIGH(otype); 		\
	INT32 lbound = (INT32) LBOUND(itype);				\
	INT32 ubound = (INT32) UBOUND(itype); 				\
	INT32 int_sf = INT_SF, int_off = INT_OFF;	 		\
	CARD32 inv;							\
	while (bw > 0) { 						\
	    inv = *inp++; 						\
	    out = ohigh; 						\
	    bw--;							\
	    if (inv < ubound) { 					\
		out = (otype)((int_sf * inv + int_off)>>shift);		\
		if (inv <= lbound)					\
			out = olow; 					\
	    }								\
	    *outp++ = out;						\
	} 								\
} 									\
static void 								\
(*fn_prep (iband,oband,pvt,techpvt,band))() 				\
    bandPtr iband, oband; mpCnstPvtPtr pvt; pCnstDefPtr techpvt; int band; \
{ 									\
	CARD32 olow, ohigh; INT32 int_sf, int_off;			\
	olow = techpvt->output_low[band];  				\
	ohigh = techpvt->output_high[band]; 				\
	LBOUND(itype) = (itype) (techpvt->input_low[band] + HALF); 	\
	UBOUND(itype) = (itype) (techpvt->input_high[band] + HALF); 	\
	OLOW(otype) = olow;	olow <<= shift;				\
	OHIGH(otype) = ohigh;	ohigh <<= shift;			\
	int_sf = (ohigh - olow) / (UBOUND(itype) - LBOUND(itype));	\
	int_off = (1<<(shift-1)) + olow - int_sf * (INT32) LBOUND(itype);\
	INT_SF = int_sf; 						\
	INT_OFF = int_off;						\
	return fn_do; 							\
}
#endif

/* XXX verify negative ramp capability */
#define DO_CSf(fn_prep,fn_do,fn_do_b,itype,otype) 			\
static void 								\
fn_do(INP,OUTP,pvt,bw) 							\
	void *INP; void *OUTP; mpCnstPvtPtr pvt; int bw; 		\
{ 									\
	itype *inp = (itype *) INP; otype *outp = (otype *) OUTP; 	\
	itype inv, lbound = LBOUND(itype), ubound = UBOUND(itype); 	\
        otype out, olow = OLOW(otype), ohigh = OHIGH(otype); 		\
	ConstrainFloat sf = FLT_SF, offset = FLT_SF; 			\
	while (bw-- > 0) { 						\
		inv = *inp++; 						\
		out = olow; 						\
		if (inv > ubound) 					\
			out = ohigh; 					\
		else if (inv >= lbound) 				\
			out = (otype)(sf * inv + offset); 		\
		*outp++ = out; 						\
	} 								\
} 									\
static void 								\
(*fn_prep (iband,oband,pvt,techpvt,band))() 				\
    bandPtr iband, oband; mpCnstPvtPtr pvt; pCnstDefPtr techpvt; int band; \
{ 									\
	otype olow, ohigh; ConstrainFloat sf; 				\
	OLOW(otype) = olow = techpvt->output_low[band]; 		\
	OHIGH(otype) = ohigh = techpvt->output_high[band];		\
	LBOUND(itype) = (itype) techpvt->input_low[band]; 		\
	UBOUND(itype) = (itype) techpvt->input_high[band]; 		\
	FLT_SF = sf = (ohigh - olow) /					\
			  (UBOUND(itype) - LBOUND(itype));		\
	FLT_OFF =  olow - sf *  LBOUND(itype); 				\
	return fn_do; 							\
}

DO_CSb	(CSp_bb, CSa_bb, CSb_bb, BitPixel,  BitPixel)
DO_CSp	(CSp_Bb, CSa_Bb, CSb_Bb, BytePixel, BitPixel)
DO_CSp	(CSp_Pb, CSa_Pb, CSb_Pb, PairPixel, BitPixel)
DO_CSp	(CSp_Qb, CSa_Qb, CSb_Qb, QuadPixel, BitPixel)
DO_CSfp	(CSp_Rb, CSa_Rb, CSb_Rb, RealPixel, BitPixel)

DO_CScx	(CSp_bB, CSa_bB, CSb_bB, BitPixel,  BytePixel)
DO_CSi	(CSp_BB, CSa_BB, CSb_BB, BytePixel, BytePixel, 22)
DO_CSi	(CSp_PB, CSa_PB, CSb_PB, PairPixel, BytePixel, 22)
DO_CSi	(CSp_QB, CSa_QB, CSb_QB, QuadPixel, BytePixel, 22)
DO_CSf	(CSp_RB, CSa_RB, CSb_RB, RealPixel, BytePixel)

DO_CSc	(CSp_bP, CSa_bP, CSb_bP, BitPixel,  PairPixel)
DO_CSi	(CSp_BP, CSa_BP, CSb_BP, BytePixel, PairPixel, 14)
DO_CSi	(CSp_PP, CSa_PP, CSb_PP, PairPixel, PairPixel, 14)
DO_CSi	(CSp_QP, CSa_QP, CSb_QP, QuadPixel, PairPixel, 14)
DO_CSf	(CSp_RP, CSa_RP, CSb_RP, RealPixel, PairPixel)

DO_CSc	(CSp_bQ, CSa_bQ, CSb_bQ, BitPixel,  QuadPixel)
DO_CSi	(CSp_BQ, CSa_BQ, CSb_BQ, BytePixel, QuadPixel, 6)
DO_CSi	(CSp_PQ, CSa_PQ, CSb_PQ, PairPixel, QuadPixel, 6)
DO_CSi	(CSp_QQ, CSa_QQ, CSb_QQ, QuadPixel, QuadPixel, 6)
DO_CSf	(CSp_RQ, CSa_RQ, CSb_RQ, RealPixel, QuadPixel)

#undef LBOUND
#undef HBOUND
#undef OLOW
#undef OHIGH
#undef FLT_SF
#undef FLT_OFF
#undef INT_SF
#undef INT_OFF

/* end module mpcnst.c */
