/* $XConsortium$ */
/**** module mppaste.c ****/
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
  
	mppaste.c -- DDXIE paste up element
  
	Dean Verheiden && Larry Hare -- AGE Logic, Inc. July, 1993
  
*****************************************************************************/

#define _XIEC_MPPASTE
#define _XIEC_PPASTE

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


typedef RealPixel PasteUpFloat;
/*
 *  routines referenced by other DDXIE modules
 */
int	miAnalyzePasteUp();

/*
 *  routines used internal to this module
 */
static int CreatePasteUp();
static int InitializePasteUp();
static int ActivatePasteUp();
static int ResetPasteUp();
static int DestroyPasteUp();

/* Fill functions */
static void FillReal();
static void FillQuad();
static void FillPair(); 
static void FillByte();
static void FillBit();

/* Action functions */
static void PasteReal();
static void PasteQuad();
static void PastePair(); 
static void PasteByte();
static void PasteBit();

/*
 * DDXIE Paste Up entry points
 */
static ddElemVecRec PasteUpVec = {
  CreatePasteUp,
  InitializePasteUp,
  ActivatePasteUp,
  (xieIntProc)NULL,
  ResetPasteUp,
  DestroyPasteUp
  };

/*
 * Local Declarations. 
 */

typedef struct _mppasteupdef {
	void	(*fill) ();
	void	(*action) ();
	CARD32	nextline;
} mpPasteUpPvtRec, *mpPasteUpPvtPtr;


/*------------------------------------------------------------------------
----------------------------- Analyze ------------------------------------
------------------------------------------------------------------------*/
int miAnalyzePasteUp(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  /* for now just stash our entry point vector in the peDef */
  ped->ddVec = PasteUpVec;

  return(TRUE);
}                               /* end miAnalyzePasteUp */

/*------------------------------------------------------------------------
---------------------------- create peTex . . . --------------------------
------------------------------------------------------------------------*/
static int CreatePasteUp(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  int auxsize = xieValMaxBands * sizeof(mpPasteUpPvtRec);

  /* Force syncing between sources, but not bands */
  return (MakePETex(flo, ped, auxsize, SYNC, NO_SYNC) );
}                               /* end CreatePasteUp */


/*------------------------------------------------------------------------
---------------------------- initialize peTex . . . ----------------------
------------------------------------------------------------------------*/
static int InitializePasteUp(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  xieFloPasteUp *raw  = (xieFloPasteUp *)ped->elemRaw;
  xieTypTile *tp      = (xieTypTile *)&(raw[1]); 
  peTexPtr pet	      = ped->peTex;
  mpPasteUpPvtPtr pvt = (mpPasteUpPvtPtr)pet->private;
  CARD32 nbands	      = pet->receptor[SRCt1].inFlo->bands;
  bandPtr iband;		
  INT32 miny; 
  CARD32 t, band;

  if (!(InitReceptors(flo, ped, NO_DATAMAP, 1) && 
	InitEmitter(flo, ped, NO_DATAMAP, NO_INPLACE)))
    return (FALSE);

  miny = pet->emitter[0].format->height;
  for (t = 0; t < raw->numTiles; t++)  {
	if (tp[t].dstY < miny) 
		miny = tp[t].dstY;
  }

  if (miny < 0)
      miny = 0;

  /* XXX Should consider building derived structures which reflect
  ** totally clipped tiles.  Currently Y- clipped tiles are disabled 
  ** Y+ clipped tiles are simply ignored till shutdown, and X clipped
  ** tiles are processed anyway.
  */
  /* Set inactivity mask of all images without minimum y */
  for (t = 0; t < raw->numTiles; t++) { 
	if (tp[t].dstY != miny) {
            iband = &(pet->receptor[t].band[0]);
  	    for(band = 0; band < nbands; band++, iband++) {
		if (tp[t].dstY > miny) {
		    IgnoreBand(iband);
		} else if ((tp[t].dstY + (INT32)iband->format->height) < 0) {
		    DisableSrc(flo,pet,iband,FLUSH);
		} else {
		    SetBandThreshold(iband,-tp[t].dstY+1);
		}
	    }
	}
  }

 
  /* Figure out the appropriate action vector */
  /* Use first source for the band types since they all must be the same */
  iband = &(pet->receptor[SRCt1].band[0]);
  for(band = 0; band < nbands; band++, pvt++, iband++) {
      switch (iband->format->class) {
      case UNCONSTRAINED:     
		pvt->fill   = FillReal; 
		pvt->action = PasteReal; 
		break;
      case QUAD_PIXEL:        
		pvt->fill   = FillQuad; 
		pvt->action = PasteQuad; 
		break;
      case PAIR_PIXEL:        
		pvt->fill   = FillPair; 
		pvt->action = PastePair; 
		break;
      case BYTE_PIXEL:        
		pvt->fill   = FillByte; 
		pvt->action = PasteByte; 
		break;
      case BIT_PIXEL:         
		pvt->fill   = FillBit; 
		pvt->action = PasteBit; 
		break;
      default:  
		ImplementationError(flo, ped, return(FALSE));
                break;
      }
      pvt->nextline = miny;
  }

  return( TRUE );
}                               /* end InitializePasteUp */

/*------------------------------------------------------------------------
------------------------ crank some single input data --------------------
------------------------------------------------------------------------*/
static int ActivatePasteUp(flo,ped,pet)
     floDefPtr flo;
     peDefPtr  ped;
     peTexPtr  pet;
{
  pPasteUpDefPtr pvt   = (pPasteUpDefPtr) ped->elemPvt;
  xieFloPasteUp *raw   = (xieFloPasteUp *)ped->elemRaw;
  xieTypTile *tp       = (xieTypTile *)&(raw[1]); 
  double *fconst       = pvt->constant;
  receptorPtr rcp      = pet->receptor;
  CARD32 bands         = rcp[SRCt1].inFlo->bands;
  bandPtr dbnd         = &pet->emitter[0];
  mpPasteUpPvtPtr mpvt = (mpPasteUpPvtPtr) pet->private;
  void *src, *dst;
  CARD32 t, b;
  
    /* XXX add .5 for non floats ??? */
    /* XXX should do fconst conversion once in Initialize */

    for(b = 0; b < bands; b++, dbnd++, mpvt++, fconst++) {
	INT32 dst_width = dbnd->format->width;

	/* Get pointer for dst scanline, Fill with constant */
	if (!(dst = GetCurrentDst(void,flo,pet,dbnd)))
	    break;
	(*(mpvt->fill)) (dst,*fconst,dst_width);

	/* Skip any constant lines */
	if (dbnd->current < mpvt->nextline) {
	    while (dbnd->current < mpvt->nextline)  {
		if (dst = GetNextDst(void,flo,pet,dbnd,KEEP)) {
		    (*(mpvt->fill)) (dst,*fconst,dst_width);
		} else {
		    PutData(flo,pet,dbnd,dbnd->current);
		    return TRUE;
		}
	    }
	}

	mpvt->nextline = dbnd->format->height;
	for (t = 0; t < raw->numTiles; t++) {
	    bandPtr sbnd = &rcp[t].band[b];
	    INT32 soff, doff, twidth;
	    INT32 tdy = tp[t].dstY;
	    INT32 tdend = tdy + sbnd->format->height;
	    if (tdend <= (INT32) dbnd->current)
		continue;
	    if (((INT32)dbnd->current >= tdy) &&
		((INT32)dbnd->current < (tdend))) {

	        if (sbnd->threshold > 1) {
		    src = GetSrc(void,flo,pet,sbnd,sbnd->threshold - 1,KEEP);
		    SetBandThreshold(sbnd,1);
		} else
		    src = GetCurrentSrc(void,flo,pet,sbnd);
		if (!src) {
		    /* mpvt->nextline = dbnd->current; causes infinite loop */
		    return TRUE;
		}
		soff = 0;
		doff = tp[t].dstX;
		twidth = sbnd->format->width;

		if (doff < 0) {				/* clip on left side */
		    soff -= doff;
		    twidth += doff;
		    doff  = 0;
		}
		if ((doff + twidth) > dst_width)	/* clip on right */
		    twidth = dst_width - doff;

		if (twidth > 0)
		    (*(mpvt->action)) (src, soff, dst, doff, twidth);

		FreeData(void,flo,pet,sbnd,sbnd->current+1);
		if ((dbnd->current + 1) < tdend)
		    mpvt->nextline = dbnd->current + 1;
	    } else if (tdy == (dbnd->current + 1)) {
		/* TILE missed. Maybe we need it next time? */
		AttendBand(sbnd);
		mpvt->nextline = dbnd->current + 1;
	    } else if (tdy < mpvt->nextline) {
		mpvt->nextline = tdy;
	    }
	}
		
	if (mpvt->nextline < dbnd->format->height) {
	    /* ... still more tiles to copy */
	    (void) GetNextDst(void,flo,pet,dbnd,FLUSH);
	    if (mpvt->nextline != dbnd->current) {
	        /* ... find the Next bunch of tiles */
		for (t = 0; t < raw->numTiles; t++)
		    if (tp[t].dstY == mpvt->nextline) {
		        AttendBand(&rcp[t].band[b]);
		    }
	    }
	} else {
	    /* ... no more tiles to copy; kill remainder */
	    for (t = 0; t < raw->numTiles; t++) {
		bandPtr sbnd = &rcp[t].band[b];
	        if (tp[t].dstY + sbnd->format->height > mpvt->nextline)
      		    DisableSrc(flo,pet,sbnd,FLUSH);
	    }
	    /* ... fill in remaining destination with constant */
	    while((dst = GetNextDst(void,flo,pet,dbnd,KEEP)))
	        (*(mpvt->fill)) (dst,*fconst,dst_width);
	    PutData(flo,pet,dbnd,dbnd->current);
	} 

	/* XXX we never kill of any tiles which end before line 0 */

    }
    return(TRUE);
}                               /* end ActivatePasteUp */

/*------------------------------------------------------------------------
------------------------ get rid of run-time stuff -----------------------
------------------------------------------------------------------------*/
static int ResetPasteUp(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  ResetReceptors(ped);
  ResetEmitter(ped);
  
  return(TRUE);
}                               /* end ResetPasteUp */

/*------------------------------------------------------------------------
-------------------------- get rid of this element -----------------------
------------------------------------------------------------------------*/
static int DestroyPasteUp(flo,ped)
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
}                               /* end DestroyPasteUp */

/*------------------------------------------------------------------------
--------------------------PasteUp fill routines  ------------------------
------------------------------------------------------------------------*/

#define PasteFill(fname,stype)					\
static void fname(din,ifill,width)	 			\
void *din;							\
double ifill;							\
CARD32 width;							\
{								\
stype *dst = (stype *)din, fill = (stype)ifill;			\
	while (width-- > 0) *dst++ = fill;			\
}

PasteFill(FillReal,RealPixel)
PasteFill(FillQuad,QuadPixel)
PasteFill(FillPair,PairPixel)
PasteFill(FillByte,BytePixel)

static void FillBit(dst,ifill,width) 
	double  ifill;
	BitPixel *dst;
	CARD32	width;
{
	int fill =  (ifill > .5) ? ~0 : 1;
	memset((char *)dst,fill,(width+7)>>3);
}

/*------------------------------------------------------------------------
--------------------------PasteUp action routines  ----------------------
------------------------------------------------------------------------*/

#define PasteAction(fname,stype)				\
static void fname(sin,s_off,din,d_off,width) 			\
CARD32 width, s_off, d_off;					\
void *sin, *din;						\
{								\
stype *src = (stype *)sin, *dst = (stype *)din;			\
	src += s_off;						\
	dst += d_off;						\
	while (width-- > 0) *dst++ = *src++;			\
}

PasteAction(PasteReal,RealPixel)
PasteAction(PasteQuad,QuadPixel)
PasteAction(PastePair,PairPixel)
PasteAction(PasteByte,BytePixel)

static void PasteBit(sin,s_off,din,d_off,width)
	CARD32 width, s_off, d_off;
	void *sin, *din;
{
	LogInt * src = (LogInt *) sin;
	LogInt * dst = (LogInt *) din;
	for ( ; width-- > 0 ; s_off++, d_off++ ) {
	    if (LOG_tstbit(src,s_off))
		LOG_setbit(dst,d_off);
	    else
		LOG_clrbit(dst,d_off);
	}
}

/* end module mppaste.c */
