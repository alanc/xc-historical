/* $XConsortium$ */
/**** module microi.c ****/
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
  
	microi.c -- DDXIE prototype import client roi element
  
	James H Weida  -- AGE Logic, Inc. April, 1993
  
*****************************************************************************/

#define _XIEC_MICROI

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
#include <microi.h>
#include <strip.h>

/*
 *  routines referenced by other DDXIE modules
 */
int	miAnalyzeICROI();

/*
 *  routines used internal to this module
 */
static int CreateICROI();
static int InitializeICROI();
static int ActivateICROI();
static int FlushICROI();
static int ResetICROI();
static int DestroyICROI();
static void QuickSortROI();
static xieTypRectangle *BandROI();
static CARD32 RunLengthSizeROI();
static void RunLengthROI();

/*
 * DDXIE ImportClientROI entry points
 */
static ddElemVecRec ICROIVec =
{
	CreateICROI,
	InitializeICROI,
	ActivateICROI,
	FlushICROI,
	ResetICROI,
	DestroyICROI
};

/*------------------------------------------------------------------------
------------------- see if we can handle this element --------------------
------------------------------------------------------------------------*/
int miAnalyzeICROI(flo,ped)
	floDefPtr flo;
	peDefPtr  ped;
{
	/* for now just stash our entry point vector in the peDef */
	ped->ddVec = ICROIVec;
	return(TRUE);
}                               /* end miAnalyzeICROI */


/*------------------------------------------------------------------------
---------------------------- create peTex . . . --------------------------
------------------------------------------------------------------------*/
static int CreateICROI(flo,ped)
	floDefPtr flo;
	peDefPtr  ped;
{
	/* attach an execution context to the roi element definition */
	return MakePETex(flo,ped,sizeof(miCROIDefRec),FALSE,FALSE);
}                               /* end CreateICROI */


/*------------------------------------------------------------------------
---------------------------- initialize peTex . . . ----------------------
------------------------------------------------------------------------*/
static int InitializeICROI(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
	xieFloImportClientROI *raw = (xieFloImportClientROI *)ped->elemRaw;
	miCROIDefPtr ext = (miCROIDefPtr) ped->peTex->private;

	/* init icroi private data */
	ext->nroi = raw->rectangles;
	ext->size = raw->rectangles * sizeof(xieTypRectangle);
	ext->dataLast = 0;

	/* XXX what if size == 0 ?? */
	/* XXX possible need to free or Realloc on modify flow ?? */

	/* make storage space for expected roi */
	ext->u.data = (CARD8 *)XieCalloc(ext->size);
	if(!ext->u.data)
	{
		AllocError(flo,ped, return(FALSE));
	}
 	return InitReceptors(flo,ped,0,1) && InitEmitter(flo,ped,0,-1);
}                               /* end InitializeICROI */


/*------------------------------------------------------------------------
----------------------------- crank some data ----------------------------
------------------------------------------------------------------------*/
static int ActivateICROI(flo,ped,pet)
     floDefPtr flo;
     peDefPtr  ped;
	peTexPtr pet;
{
	CARD32 ncopy;
	miCROIDefPtr ext = (miCROIDefPtr) pet->private;
	bandPtr   sbnd = &pet->receptor[IMPORT].band[0];
	CARD32    slen;
	BytePixel *src;

	src = GetSrcBytes(BytePixel,flo,pet,sbnd,sbnd->current,1,FALSE);
	while (src)
	{
		slen = sbnd->strip->length;
		/* check for too much data */
		if (ext->dataLast >= ext->size)
			ncopy = 0;
		else
		/* check for too much incomming data */
		if (slen + ext->dataLast > ext->size)
			ncopy = ext->size - ext->dataLast;
		else
			ncopy = slen;
		if (ncopy)
		{
			bcopy(src, &ext->u.data[ext->dataLast], ncopy);
			ext->dataLast += ncopy;
		}
		src = GetSrcBytes(BytePixel,flo,pet,sbnd,sbnd->current+slen,1,FALSE);
	}
	/* if final, process the data received */
	if (sbnd->final)
	{
		bandPtr   dbnd;
		RunLengthPtr dst;
		RunLengthRec Frame;
		RunLengthPtr frame;
		RunLengthTbl *tbl;
		CARD32 size;
		INT32 xmin;
		register int i, tmp;

		if (ext->dataLast != ext->size)
			ValueError(flo,ped,ext->dataLast/sizeof(xieTypRectangle), return(FALSE));

		/* XXX SwapLongs... */
		if (flo->client->swapped) for (i = 0; i < ext->nroi; i++)
		{
			swapl(&ext->u.roi[i].x, tmp);
			swapl(&ext->u.roi[i].y, tmp);
			swapl(&ext->u.roi[i].width, tmp);
			swapl(&ext->u.roi[i].height, tmp);
		}


#if defined(VERBOSE)
ErrorF("\nXIE: ImportClientROI nROI=%d\n", ext->nroi);
{
register int f;
for (f = 0; f < ext->nroi; f++)
ErrorF("  %3dROI [%4d,%4d,%4d,%4d]\n",f, ext->u.roi[f].x, ext->u.roi[f].y, ext->u.roi[f].width, ext->u.roi[f].height);
}
#endif
		/* sort rectangles */
		QuickSortROI(ext->u.roi, ext->nroi);

#if defined(VERBOSE)
ErrorF("XIE: ImportClientROI - after Quicksort nROI=%d\n", ext->nroi);
{
register int f;
for (f = 0; f < ext->nroi; f++)
ErrorF("  %3dROI [%4d,%4d,%4d,%4d]\n",f, ext->u.roi[f].x, ext->u.roi[f].y, ext->u.roi[f].width, ext->u.roi[f].height);
}
#endif
    		/* band rectangles */
		ext->u.roi = BandROI(ext->u.roi, &ext->nroi, &xmin);
		if (! ext->u.roi)
			AllocError(flo,ped, return(FALSE));
		ext->size = ext->nroi * sizeof(xieTypRectangle);

#if defined(VERBOSE)
ErrorF("XIE: ImportClientROI - after BandROI nROI=%d\n", ext->nroi);
{
register int f;
for (f = 0; f < ext->nroi; f++)
ErrorF("  %3dROI [%4d,%4d,%4d,%4d]\n",f, ext->u.roi[f].x, ext->u.roi[f].y, ext->u.roi[f].width, ext->u.roi[f].height);
}
#endif

		/* make strip */
		size = RunLengthSizeROI(&Frame,ext->u.roi,ext->nroi, xmin);

#if defined(VERBOSE)
ErrorF("XIE ImportClientROI: after RunLengthSizeROI size=%d\n", size);
#endif
		size += sizeof(Frame);
		dbnd = &pet->emitter[0];
		dst = GetDstBytes(RunLengthRec,flo,pet,dbnd,0,size,FALSE);
		if (!dst)
			AllocError(flo,ped, return(FALSE));
		*dst = Frame;
      		/* write/generate run-length tables */
		RunLengthROI(&dst[1], ext->u.roi, ext->nroi, xmin);

#if defined(VERBOSE)
ErrorF("XIE ImportClientROI: after RunLengthROI\n");
{
register RunLengthPtr frame = dst;
register RunLengthTbl *tbl = (RunLengthTbl *)&frame[1];
register int f,g,s;
ErrorF("XIE FRAME x=%d y=%d n=%d size=%d\n", 
frame->x,frame->y,frame->nentry,frame->size);
for (f = 0; f < frame->nentry; f++)
{
ErrorF(" %4dLIST [%d,%d,%d,%d]",f,
tbl->hdr.x,tbl->hdr.y,tbl->hdr.nline,tbl->hdr.npair); 
for (g = 0; g < tbl->hdr.npair; g++)
ErrorF(" [%d,%d]", tbl->pair[g].count, tbl->pair[g].length); ErrorF("\n");
tbl = (RunLengthTbl *)(&tbl->pair[g]);
}
}
#endif
		SetBandFinal(dbnd);
		PutData(flo,pet,dbnd,size);
	}
	/* make sure the scheduler knows how much src we used */
	FreeData(BytePixel,flo,pet,sbnd,sbnd->maxGlobal);
	return TRUE;
}                               /* end ActivateICROI */

/*------------------------------------------------------------------------
--------------------------- get rid of left overs ------------------------
------------------------------------------------------------------------*/
static int FlushICROI(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
  /* since Activate was suppose to do the whole image, there's nothing to do */

	return TRUE;
}                               /* end FlushICROI */


/*------------------------------------------------------------------------
------------------------ get rid of run-time stuff -----------------------
------------------------------------------------------------------------*/
static int ResetICROI(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
	miCROIDefPtr ext = (miCROIDefPtr) ped->peTex->private;
	if (ext && ext->u.data)
		ext->u.data = (CARD8 *)XieFree(ext->u.data);
	ResetReceptors(ped);
	ResetEmitter(ped);
	return TRUE;
}                               /* end ResetICROI */


/*------------------------------------------------------------------------
-------------------------- get rid of this element -----------------------
------------------------------------------------------------------------*/
static int DestroyICROI(flo,ped)
     floDefPtr flo;
     peDefPtr  ped;
{
	/* get rid of the peTex structure  */
	ped->peTex = (peTexPtr) XieFree(ped->peTex);

	/* zap this element's entry point vector */
	ped->ddVec.create = (xieIntProc) NULL;
	ped->ddVec.initialize = (xieIntProc) NULL;
	ped->ddVec.activate = (xieIntProc) NULL;
	ped->ddVec.flush = (xieIntProc) NULL;
	ped->ddVec.reset = (xieIntProc) NULL;
	ped->ddVec.destroy = (xieIntProc) NULL;

	return TRUE;
}                               /* end DestroyICROI */


#define SwitchROI(roi1,roi2)	\
{\
    xieTypRectangle tmp;\
    tmp = roi1; roi1 = roi2; roi2 = tmp;\
}

static void QuickSortROI(roi, nroi)
    register xieTypRectangle *roi;
    register CARD32 nroi;
{
	register INT32 y;
	register int i, j, k;
	register xieTypRectangle *r;

	if (nroi < 2)
		return;
	/* sort upwards using y-coordinate */
	i = 0;
	do /*while (i != nroi)*/
	{
		j = i+1;
		while (j != nroi)
		{
			if (roi[i].y > roi[j].y)
			{
				/* roi[i] out of order */
				SwitchROI(roi[i],roi[j]);
			}
			/* sort by x-coordinate */
			else if ((roi[i].y == roi[j].y)
			&&       (roi[j].x < roi[i].x))
			{
				SwitchROI(roi[i],roi[j]);
			}
			j++;
		}
		i++;
	}
	while (i != nroi); return;
}
/* QuickSortROI */


#define iy1	roi[i].y
#define ix1	roi[i].x
#define iht	roi[i].height
#define iwd	roi[i].width
#define jy1	roi[j].y
#define jx1	roi[j].x
#define jht	roi[j].height
#define jwd	roi[j].width

#define RemoveROI(roi,ndx,nr)	\
{\
	register int k;\
	(nr)--; surplus++;\
	for (k = (ndx); k < (nr); k++)\
		(roi)[k] = (roi)[k+1];\
}

#define ROI_SURPLUS 10

#define InsertSortROI(XC,YC,WC,HC,roi,nr) \
	if (!surplus)\
	{\
		surplus = ROI_SURPLUS;\
		(roi) = (xieTypRectangle *)\
		XieRealloc((roi),sizeof(xieTypRectangle)*((nr)+ROI_SURPLUS));\
		if (! (roi))\
		{\
			return (xieTypRectangle *)roi ;\
		}\
	}\
	(roi)[(nr)].x = (XC); (roi)[(nr)].y = (YC);\
	(roi)[(nr)].width = (WC); (roi)[(nr)].height = (HC);\
	nr++; surplus--;\
	QuickSortROI(roi,nr)

#define InsertROI(XC,YC,WC,HC,roi,nr) \
	if (!surplus)\
	{\
		surplus = ROI_SURPLUS;\
		(roi) = (xieTypRectangle *)\
		XieRealloc((roi),sizeof(xieTypRectangle)*((nr)+ROI_SURPLUS));\
		if (! (roi))\
		{\
			return (xieTypRectangle *)roi;\
		}\
	}\
	(roi)[(nr)].x = (XC); (roi)[(nr)].y = (YC);\
	(roi)[(nr)].width = (WC); (roi)[(nr)].height = (HC);\
	(nr)++; surplus--

static xieTypRectangle * BandROI(roi, nroi, xmin)
    register xieTypRectangle *roi;
    CARD32 *nroi;
    INT32 *xmin;
{
	register int i, j;
	register int nr, surplus;
	register INT32 iy2, ix2, jy2, jx2;

	surplus = 0;
	nr = *nroi;
	*xmin = 0x7fffffff;

	/* purge impossible to band roi */
	for (i = 0; ; i++)
	{
		if (i >= nr) break; /* nr is varible */
		if ((iht == 0) || (iwd == 0))
			RemoveROI(roi,i,nr)
		else if (roi[i].x < *xmin)
			*xmin = roi[i].x;
	}
	/* prerequisite: y then x sorted */
	i = 0;
	if (nr) for (;;)
	{
		iy2 = iy1+(iht-1);
		ix2 = ix1+(iwd-1);

		j = i+1;
		while (iy2 >= jy1)
		{
			if (j >= nr) break; /* nr is varible */

			jy2 = jy1+(jht-1);
			jx2 = jx1+(jwd-1);

			/* merge/remove redundant roi's */
			if (iy1 == jy1)
			{
				if (iht > jht)
				{
					/* new bottom band roi[i] */
					InsertSortROI(ix1,jy2+1,iwd,iht-jht,roi,nr);
					iy2 = jy2;
					iht = jht;
				}
				else if (iht < jht)
				{
					/* new bottom band roi[j] */
					InsertSortROI(jx1,iy2+1,jwd,jht-iht,roi,nr);
					jy2 = iy2;
					jht = iht;
				}
				if (iht == jht) /* always */
				{
					/* x adjacent roi */
					if ((ix1 <= jx1) && (ix2+1 >= jx1))
					{
						/* merge (possible redundancy)  */
						if (ix2 < jx2)
						{
							ix2 = jx2;
							if (ix1 < 0)
								if (jx2 < 0)
									iwd = ix1-jx2+1;
								else
									iwd = jx2+abs(ix1)+1;
							else
								iwd = jx2-ix1+1;
						}
						RemoveROI(roi,j,nr);
					}
					/* should never happen if x sorted */
					else if ((jx1 < ix1) && (jx2+1 >= ix1))
					{
						/* merge (possible redundancy) */
						if (jx2 <= ix2)
						{
							ix1 = jx1;
							if (jx2 < 0)
								if (ix2 < 0)
									iwd = jx2-ix2+1;
								else
									iwd = ix2+abs(jx2)+1;
							else
								iwd = ix2-jx1+1;
						}
						RemoveROI(roi,j,nr);
					}
					else j++;
				}
			}
			else if (iy2 < jy2)
			{
				/* new bottom band roi[j] */
				InsertROI(jx1,iy2+1,jwd,jy2-iy2,roi,nr);
				jy2 = iy2;
				if (iy2 < 0)
					if (jy2 < 0)
						jht = iy2-jy2+1;
					else
						jht = abs(iy2)+jy2+1;
				else
					jht = iy2-jy1+1;

				/* new bottom band roi[i] */
				InsertSortROI(ix1,jy1,iwd,jht,roi,nr);
				iy2 = jy1-1;
				if (iy1 < 0)
					if (jy1 < 0)
						iht = iy1-jy1;
					else
						iht = abs(iy1)+jy1;
				else
					iht = jy1-iy1;
				j++;
			}
			else if (iy2 > jy2)
			{
				/* redundant roi[j] */
				if ((ix1 <= jx1) && (ix2 >= jx2))
				{
					RemoveROI(roi,j,nr);
				}
				else
				{
					/* new bottom band roi[i] */
					InsertROI(ix1,jy2+1,iwd,iy2-jy2,roi,nr);
					iy2 = jy2;
					if (iy1 < 0)
						if (jy1 < 0)
							iht = iy1-jy2+1;
						else
							iht = abs(iy1)+jy2+1;
					else
						iht = jy2-iy1+1;

					/* new bottom band roi[i] */
					InsertSortROI(ix1,jy1,iwd,jht,roi,nr);
					iy2 = jy1-1;
					if (iy1 < 0)
						if (jy1 < 0)
							iht = iy1-jy1;
						else
							iht = abs(iy1)+jy1;
					else
						iht = jy1-iy1;
					j++;
				}
			}
			else /* if (iy2 == jy2) */
			{
				/* redundant roi[j] */
				if ((ix1 <= jx1) && (ix2 >= jx2))
					RemoveROI(roi,j,nr)
				else
				{
					/* new bottom band roi[i] */
					iy2 = jy1-1;
					if (iy1 < 0)
						if (jy1 < 0)
							iht = iy1-jy1;
						else
							iht = abs(iy1)+jy1;
					else
						iht = jy1-iy1;
					InsertSortROI(ix1,jy1,iwd,jht,roi,nr);
					j++;
				}
			}
		}
		i++;
		if (i >= nr) break;
	}
	*nroi = nr;
	return roi;
}
/* BandROI */

#define RUNLEN_SIZE	(RUNLEN)0x7fffffff

static CARD32 RunLengthSizeROI(frame, roi, nroi, xmin)
	register RunLengthPtr frame;
	xieTypRectangle *roi;
	CARD32 nroi;
	INT32 xmin;
{
	register CARD32 i, j, k, cnt, len;
	register CARD32 size;
	CARD32 count;

	frame->x = xmin;
	frame->y = roi[0].y;
	frame->width = 0;
	frame->height = 0;
	frame->nentry = 0;
	/* compute table size */
	size = 0;
	/* compute size of run-length table */
	for (i=0; i < nroi; i=j) {
		INT32 y = roi[i].y;
		size += sizeof(RunLengthHdr);
		for (j = i; (j < nroi) && (roi[j].y <= y); j++)
			size += sizeof(RunLengthPair);
	
		frame->nentry++;
		if (roi[i].x + roi[i].width > frame->width)
			frame->width = roi[i].x + roi[i].width; 
		if (roi[i].y + roi[i].height > frame->height)
			frame->height = roi[i].y + roi[i].height; 
	}
	frame->width  -= frame->x;
	frame->height -= frame->y;
	frame->size = size;
	return size;
}

static void RunLengthROI(tbl, roi, nroi, xmin)
	RunLengthTbl *tbl;
	xieTypRectangle *roi;
	CARD32 nroi;
	INT32 xmin;
{
	CARD32 i, j;

	/* compute size of run-length table */
	for (i=0; i < nroi; i=j) {
		INT32 ix = xmin, iy = roi[i].y, npair = 0;

		tbl->hdr.x = ix;
		tbl->hdr.y = iy;
		tbl->hdr.nline = roi[i].height;

		for (j = i; (j < nroi) && (roi[j].y <= iy); j++) {
			tbl->pair[npair].count  = roi[j].x - ix;
			tbl->pair[npair].length = roi[j].width;
			ix += (roi[j].x - ix) + roi[j].width;
			npair++;
		}
		tbl->hdr.npair = npair;
		tbl = (RunLengthTbl *)(&tbl->pair[npair]);
	}
}
/* RunLengthROI */

/* end module microi.c */
