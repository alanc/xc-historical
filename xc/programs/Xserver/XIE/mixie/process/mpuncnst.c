/* $XConsortium: mpuncnst.c,v 1.1 93/10/26 09:46:28 rws Exp $ */
/**** module mpuncnst.c ****/
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
  
	mpuncnst.c -- DDXIE Unconstrain element
  
	Dean Verheiden && Larry Hare -- AGE Logic, Inc. May, 1993
  
*****************************************************************************/


#define _XIEC_MPUNCNST
#define _XIEC_PUNCNST

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
int	miAnalyzeUnconstrain();

/*
 *  routines used internal to this module
 */
static int CreateUnconstrain();
static int InitializeUnconstrain();
static int ActivateUnconstrain();
static int FlushUnconstrain();
static int ResetUnconstrain();
static int DestroyUnconstrain();

/*
 * DDXIE Unconstrain entry points
 */
static ddElemVecRec UnconstrainVec = {
	CreateUnconstrain,
	InitializeUnconstrain,
	ActivateUnconstrain,
	FlushUnconstrain,
	ResetUnconstrain,
	DestroyUnconstrain
};

/*
 * Local Declarations. 
 */

typedef struct _mpuncnstndef {
	void	(*action) ();
} mpUncnstPvtRec, *mpUncnstPvtPtr;

static void CastQuad(), CastPair(), CastByte(), CastBit(), CastNothing(); 

/*------------------------------------------------------------------------
------------------------  fill in the vector  ---------------------------
------------------------------------------------------------------------*/
int miAnalyzeUnconstrain(flo,ped)
    floDefPtr flo;
    peDefPtr  ped;
{
    ped->ddVec = UnconstrainVec;
    return TRUE;
}

/*------------------------------------------------------------------------
---------------------------- create peTex . . . --------------------------
------------------------------------------------------------------------*/
static int CreateUnconstrain(flo,ped)
    floDefPtr flo;
    peDefPtr  ped;
{
    int auxsize = xieValMaxBands * sizeof(mpUncnstPvtRec);

    return MakePETex(flo,ped,auxsize,NO_SYNC,NO_SYNC);
}

/*------------------------------------------------------------------------
---------------------------- initialize peTex . . . ----------------------
------------------------------------------------------------------------*/
static int InitializeUnconstrain(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
    peTexPtr  pet = ped->peTex;
    mpUncnstPvtPtr pvt = (mpUncnstPvtPtr) pet->private;
    bandPtr iband;
    int band, nbands, status;

    /* ped->flags.modified = FALSE; */

    status = InitReceptors(flo,ped,NO_DATAMAP,1) &&
		InitEmitter(flo,ped,NO_DATAMAP,-1);

    nbands = pet->receptor[SRCtag].inFlo->bands;
    iband = &(pet->receptor[SRCtag].band[0]);

    for(band = 0; band < nbands; band++, pvt++, iband++) {
	switch (iband->format->class) {
	case QUAD_PIXEL:	pvt->action = CastQuad; break;
	case PAIR_PIXEL:	pvt->action = CastPair; break;
	case BYTE_PIXEL:	pvt->action = CastByte; break;
	case BIT_PIXEL:		pvt->action = (iband->format->levels == 1)
						? CastNothing : CastBit;
				break;
	default:		ImplementationError(flo, ped, return(FALSE));
				break;
	}
    }
    return status;
}

/*------------------------------------------------------------------------
----------------------------- crank some data ----------------------------
------------------------------------------------------------------------*/


static int ActivateUnconstrain(flo,ped,pet)
    floDefPtr flo;
    peDefPtr  ped;
    peTexPtr  pet;
{
    mpUncnstPvtPtr pvt = (mpUncnstPvtPtr) pet->private;
    bandPtr oband = &(pet->emitter[0]);
    bandPtr iband = &(pet->receptor[SRCtag].band[0]);
    int band, nbands = pet->receptor[SRCtag].inFlo->bands;

    for(band = 0; band < nbands; band++, iband++, oband++, pvt++) {
	register int bw = iband->format->width;
	register RealPixel *outp;
	register void *voidp;

	if (!(voidp = GetCurrentSrc(void,flo,pet,iband)) || 
	    !(outp  = GetCurrentDst(RealPixel,flo,pet,oband))) continue;

	do {

	    (*(pvt->action)) (voidp, outp, bw);

	    voidp = GetNextSrc(void,flo,pet,iband,TRUE);
	    outp  = GetNextDst(RealPixel,flo,pet,oband,TRUE);

	} while (!ferrCode(flo) && voidp && outp) ;

	FreeData(flo, pet, iband, iband->current);
    }
    return TRUE;
}

/*------------------------------------------------------------------------
--------------------------- get rid of left overs ------------------------
------------------------------------------------------------------------*/
static int FlushUnconstrain(flo,ped)
    floDefPtr flo;
    peDefPtr  ped;
{
    return TRUE;
}

/*------------------------------------------------------------------------
------------------------ get rid of run-time stuff -----------------------
------------------------------------------------------------------------*/
static int ResetUnconstrain(flo,ped)
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
static int DestroyUnconstrain(flo,ped)
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
---------------------  Lotsa Little Action Routines  ---------------------
------------------------------------------------------------------------*/

#define MakeCast(fn_name,itype)					\
static void fn_name(voidp,outp,bw)				\
	void * voidp; RealPixel *outp; int bw;			\
{								\
	register itype *inp = (itype *) voidp;			\
	register int ix;					\
	for (ix = 0; ix < bw; ix++)				\
		*outp++ = (RealPixel) *inp++;			\
}

MakeCast	(CastQuad,QuadPixel)
MakeCast	(CastPair,PairPixel)
MakeCast	(CastByte,BytePixel)

static void CastBit(voidp,outp,bw)
	void * voidp; RealPixel *outp; int bw;
{
	register LogInt *inp	= (LogInt *) voidp;
	register RealPixel One  = (RealPixel) 1.0;
	register RealPixel Zero = (RealPixel) 0.0;
	register LogInt M, inval;
	for ( ; bw >= LOGSIZE ; bw -= 32)
	    for (M=LOGLEFT, inval = *inp++; M; LOGRIGHT(M))
		*outp++ = (inval & M) ? One : Zero;
	if (bw)
	    for (M=LOGLEFT, inval = *inp++; bw; bw--, LOGRIGHT(M))
		*outp++ = (inval & M) ? One : Zero;
}

static void CastNothing(voidp,outp,bw)
	void * voidp; RealPixel *outp; int bw;
{
	register RealPixel Zero = (RealPixel) 0.0;
	register int ix;
	for (ix = 0; ix < bw; ix++)
		outp[ix] = Zero;
}

/* end module mpuncnst.c */
