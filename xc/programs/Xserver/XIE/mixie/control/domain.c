/* $XConsortium$ */
/**** module domain.c ****/
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
  
	domain.c -- DDXIE Process Domain routines for flo manager 
  
	Dean Verheiden -- AGE Logic, Inc. July, 1993
  
*****************************************************************************/

#define _XIEC_DOMAIN
#define _XIEC_DOMAIN

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
Bool  InitProcDomain();
void  ResetProcDomain();


static Bool  RunLengthSyncDomain();
static Bool  ControlPlaneSyncDomain();
static Bool  NoDomainSyncDomain();

static INT32 RunLengthGetRun();
static INT32 ControlPlaneGetRun();
static INT32 NoDomainGetRun();


/* XXX, when we add replication of RUN_LENGTH and Control Plane bands,
** then the places marked with RBAND may need changes.
*/
#define RBAND 0

Bool InitProcDomain(flo,ped,dtag,offX,offY)
floDefPtr flo;
peDefPtr ped;
xieTypPhototag dtag;
INT32 offX,offY;
{
peTexPtr pet     = ped->peTex;
receptorPtr rcp  = &pet->receptor[ped->inCnt-1];

	if (!dtag) {	/* No process domain */
	    pet->roiinit = NoDomainSyncDomain;
	    pet->roiget  = NoDomainGetRun;

	    return TRUE;
	}

	if (!InitReceptor(flo,ped,rcp,0,1,ALL_BANDS,NO_BANDS))
		return FALSE;

	pet->domXoff = offX;
	pet->domYoff = offY;

	if (rcp->band[RBAND].format->class == RUN_LENGTH) {
	    pet->roiinit = RunLengthSyncDomain;
	    pet->roiget  = RunLengthGetRun;
	} else {
	    bandPtr rband = &rcp->band[RBAND];
	    bandPtr band  = &pet->emitter[0];
	    CARD32 b;

	    pet->roiinit = ControlPlaneSyncDomain;
	    pet->roiget  = ControlPlaneGetRun;

	    /* See if there is some intersection between the ROI and image*/
  	    for(b = 0; b < ped->outFlo.bands; b++, band++)  
	    	if ( pet->domXoff +  (INT32)rband->format->width  <= 0 ||
	             pet->domXoff >= (INT32)band->format->width        ||
	             pet->domYoff +  (INT32)rband->format->height <= 0 || 
	             pet->domYoff >= (INT32)band->format->height) {
			 SetBandThreshold(rband,~0);
			 IgnoreBand(rband);
	                 band->allpass = TRUE; /* No regions any line */
		     }
	}

	return (TRUE);
}

/* Called when elements do not have an optional process domain */
static Bool NoDomainSyncDomain(flo,ped,bnd,purge)
floDefPtr flo;
peDefPtr  ped;
bandPtr   bnd;
Bool	  purge;	
{
	    bnd->xcount = 0;
	    return (TRUE);
}

static INT32 NoDomainGetRun(flo,pet,bnd)
floDefPtr flo;
peTexPtr  pet;
bandPtr   bnd;
{
    if (bnd->xcount)
	return(0);
    else 
	return(bnd->xcount = (INT32)bnd->format->width);
}

/* Initialize run length structures for desired line */
static Bool RunLengthSyncDomain(flo,ped,bnd,purge)
floDefPtr flo;
peDefPtr  ped;
bandPtr   bnd;
Bool	  purge;
{
peTexPtr  	pet	= ped->peTex;
bandPtr 	rband	= &pet->receptor[ped->inCnt-1].band[RBAND];
RunLengthTbl	*tp, *table_end;
RunLengthPtr	rlp;
INT32		ytrans;

	/* Grab table if necessary */
	if (!pet->roi) 
	    if (!(pet->roi = GetSrcBytes(void,flo,pet,rband,0,1,KEEP)))
		return (FALSE);

	rlp = (RunLengthPtr)pet->roi;

	/* Make sure that there is some intersection between the ROI and image*/
	if (bnd->allpass ||
	    rlp->x + pet->domXoff + rlp->width  <= 0           ||
	    rlp->x + pet->domXoff >= (INT32)bnd->format->width ||
	    rlp->y + pet->domYoff + rlp->height <= 0           || 
	    rlp->y + pet->domYoff >= (INT32)bnd->format->height) {
	    bnd->allpass = TRUE; /* No regions any line */
	    bnd->xcount = 0;
	    return (TRUE);
	}

	/* If no table or current y is past y desired, start at the beginning */
	tp = (RunLengthTbl *)bnd->pcroi;
	ytrans = bnd->current - pet->domYoff;
	if (!tp || tp->hdr.y > ytrans) {
	    tp = (RunLengthTbl *)&(rlp[1]);
	    if (tp->hdr.y > ytrans) {	/* Make sure we are after first entry */
		bnd->ypass  = TRUE;	/* No regions this line */
		bnd->xcount = 0;
		return (TRUE);
	    }
	} 

        /* 
          step through structures until corresponding y is found or until
 	  table is exhausted
	*/
	table_end = (RunLengthTbl *)((CARD32)&(rlp[1]) + rlp->size);
	while (tp < table_end && ytrans >= tp->hdr.y + tp->hdr.nline) 
	  tp = (RunLengthTbl *)(&tp->pair[tp->hdr.npair]);

	/* If some domains for this line, set up for processing */
	if (!(bnd->ypass = tp >= table_end || ytrans < tp->hdr.y)) {
	  bnd->pcroi   = (void *)tp;
	  bnd->xcount = (tp->hdr.x + pet->domXoff) < 0 ? 
					 tp->hdr.x + pet->domXoff : 0;
  	  bnd->xindex = 0;	
	  bnd->inside = tp->hdr.npair && !tp->pair[0].count && 
					tp->hdr.x + pet->domXoff <= 0;
	} else {
	  bnd->xcount = 0;
	  if (tp >= table_end) /* Tidy up garbage pointer */
	    bnd->pcroi = (void *)NULL;  
	}
	
	return (TRUE);
} /* SyncDomain */

static INT32 RunLengthGetRun(flo,pet,bnd)
floDefPtr flo;
peTexPtr  pet;
bandPtr   bnd;
{
RunLengthTbl	*tp = (RunLengthTbl *)bnd->pcroi;
RunLengthPair	*pair;
INT32		width	= (INT32)bnd->format->width;
INT32		xcount	= bnd->xcount;
INT32		startx, nextx, length;
CARD32		xindex, npair;
Bool		inside;

	/* Make sure that SyncDomain was called first */
	if (!pet->roi) ImplementationError(flo,pet->peDef,return(0));

	/* See if there are any processing domains for the current line */
	if (bnd->allpass || bnd->ypass || xcount >= width) {
	    if (xcount)
		return(0);	/* off edge of image */
	    else 
		return (-(bnd->xcount = width));
	}

	pair  = tp->pair;
	npair = tp->hdr.npair;
	inside = bnd->inside;
	xindex = bnd->xindex;

	/* Handle left clip */
	if (xcount < 0) {
	    while (xindex < npair && 
		  (!inside && xcount + pair[xindex].count  < 0 ||
		    inside && xcount + pair[xindex].length < 0)) {
		if (inside)
			xcount += pair[xindex++].length;
		else
			xcount += pair[xindex].count;
		inside = !inside;
	    }

	    /* This should have already been caught in init */
	    if (xindex >= npair) { /* no intersection */
		bnd->ypass = TRUE;	
		return (-(bnd->xcount = width));
	    }

	    startx = 0;
	} else 
	    startx = xcount;

	/* Handle right clip */
	if (xindex >= npair ||
	     inside && xcount + pair[xindex].length > width ||
	    !inside && xcount + pair[xindex].count  > width) {
	    bnd->ypass = TRUE;	
	    nextx = width;
	} else if (inside) 
	    nextx = xcount + pair[xindex++].length;
	else {
	    nextx = xcount + pair[xindex].count;
	    /* Handle case where first domain starts past start of image */
	    if (!xindex && tp->hdr.x + pet->domXoff > 0)
		nextx += tp->hdr.x + pet->domXoff;
	}

	length = (inside) ? nextx - startx : startx - nextx;
	
	/* toggle inside/outside */
	bnd->inside = !inside;
	bnd->xcount = nextx;
	bnd->xindex = xindex;
		
	return (length);
} /* GetRun */

/* Initialize control plane structures for desired line */
static Bool ControlPlaneSyncDomain(flo,ped,bnd,purge)
floDefPtr flo;
peDefPtr  ped;
bandPtr   bnd;
Bool	  purge;
{
peTexPtr pet  = ped->peTex;
bandPtr	rband;
INT32	ytrans;

	bnd->xcount = 0;

	/*  If init routine determined no intersection, pass line */
	if (bnd->allpass)
	    return (TRUE);

	rband = &pet->receptor[ped->inCnt-1].band[RBAND];
	ytrans = bnd->current - pet->domYoff;

	if (ytrans < 0 || ytrans >= (INT32)rband->format->height) {
	    bnd->ypass = TRUE;
	    return (TRUE);
	} else
	    bnd->ypass = FALSE;

	/* Grab control plane line */
        if (!(pet->roi = bnd->pcroi = GetSrc(void,flo,pet,rband,ytrans,purge))){
	    if (purge)
		FreeData(void,flo,pet,rband,rband->current);
	    else
		SetBandThreshold(rband,rband->available + 1);
	    return (FALSE);
	}

	if ( pet->domXoff > 0 ) {
		bnd->inside = FALSE;
		bnd->xindex = 0;
	} else {
		bnd->xindex = -pet->domXoff;
		bnd->inside = LOG_tstbit((LogInt *)bnd->pcroi,bnd->xindex) != 0;
	}

	return (TRUE);
}

static INT32 countZeros(line,offset,iedge)
LogInt *line;
INT32 offset,iedge;
{
INT32	count = 0;
CARD32  index = (CARD32)offset, edge = (CARD32)iedge;

    while (index < edge && !LOG_tstbit(line,index)) {
	count++;
	index++;
    }

    return(count);
}

INT32 countOnes(line,offset,iedge)
LogInt *line;
INT32 offset,iedge;
{
INT32	count = 0;
CARD32  index = (CARD32)offset, edge = (CARD32)iedge;

    while (index < edge && LOG_tstbit(line,index)) {
		count++;
		index++;
    }

    return(count);
}

static INT32 ControlPlaneGetRun(flo,pet,bnd)
floDefPtr flo;
peTexPtr  pet;
bandPtr   bnd;
{
peDefPtr  ped = pet->peDef;
bandPtr rband = &pet->receptor[ped->inCnt-1].band[RBAND];
INT32  xcount = bnd->xcount;
INT32   width = (INT32)bnd->format->width;

LogInt   *cpl;
Bool   inside;
INT32  xindex, dwidth, Xoff, length, edge;
	
	/* See if there is every anything to do with this band */
	if (bnd->allpass || bnd->ypass) {
	    if (xcount)
		return(0);
	    else
		return(-(bnd->xcount = width));
	}

	/* Make sure that there is something left to look at */
	if (xcount >= width) 
	    return (0);

	/* defer sucking these out til necessary */
	cpl = (LogInt *)bnd->pcroi;
	inside = bnd->inside;
	xindex = bnd->xindex;
	dwidth = (INT32)rband->format->width;
	Xoff = pet->domXoff;

	/* Figure out the maximum bound to check */
    	edge = (Xoff + dwidth > width) ? width - Xoff : dwidth;

	/* See if we are off the side of the control plane */
	if (!xcount && pet->domXoff > 0) {
	    xcount = length = Xoff + (xindex = countZeros(cpl, 0, edge));
	} else if (inside) {
	    xcount += (length = countOnes(cpl, xindex, edge));
	    xindex += length;
	} else {
	    xcount += (length = countZeros(cpl, xindex, edge));
	    xindex += length;
	    /* If control plane ends before image, push to edge */
	    if (xindex >= edge && xcount < width) {
	        length += width - xcount;
	        xcount = width;
	    } 
	}

	bnd->inside = !inside;
	bnd->xcount = xcount;
	bnd->xindex = xindex;

	return ((inside) ? length : -length);
}

void ResetProcDomain(ped)
peDefPtr ped;
{
peTexPtr pet     = ped->peTex;
bandPtr	 band	 = &pet->emitter[0];
int b;

  pet->roi     = (void *) NULL;
  pet->roiinit = (Bool  (*)()) NULL;
  pet->roiget  = (INT32 (*)()) NULL;
  pet->domXoff = 0;
  pet->domYoff = 0;

  for(b = 0; b < ped->outFlo.bands; b++, band++) {
      band->pcroi    = (void *)NULL;
      band->xindex  = 0;	
      band->xcount  = 0;	
      band->ypass   = FALSE;	
      band->inside  = FALSE;	
      band->allpass = FALSE;	
  }
}

/* end domain.c */
