/* $XConsortium: compare.c,v 1.1 93/10/26 10:07:57 rws Exp $ */

/**** module compare.c ****/
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
  
	compare.c -- compare flo element tests 

	Syd Logan -- AGE Logic, Inc. August, 1993 - MIT Beta release
  
*****************************************************************************/
#include "xieperf.h"
#include <stdio.h>

static XiePhotomap XIEPhotomap1;
static XiePhotomap XIEPhotomap2;
static XieRoi XIERoi;

static XIEimage	*image1, *image2;
static int flo_notify;
static XiePhotoElement *flograph;
static XiePhotoflo flo;
static int flo_elements;

int 
InitCompare(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
	XieProcessDomain domain;
	XieCompareOp	op;
	XieConstant	constant;
	Bool		combine;
	unsigned int	bandMask;
	Bool		useROI;
	int		idx;
	int		src1, src2;
	XieRectangle	rect;

	useROI = ( ( CompareParms * )p->ts )->useROI;
	op = ( ( CompareParms * )p->ts )->op;
	constant[ 0 ] = ( ( CompareParms * )p->ts )->constant[ 0 ];
	constant[ 1 ] = ( ( CompareParms * )p->ts )->constant[ 1 ];
	constant[ 2 ] = ( ( CompareParms * )p->ts )->constant[ 2 ];
	combine = ( ( CompareParms * )p->ts )->combine;
	bandMask = ( ( CompareParms * )p->ts )->bandMask;

        XIERoi = ( XieRoi ) NULL;
        XIEPhotomap1 = ( XiePhotomap ) NULL;
        XIEPhotomap2 = ( XiePhotomap ) NULL;
        flograph = ( XiePhotoElement * ) NULL;
        flo = ( XiePhotoflo ) NULL;

	switch( op )
	{
		case xieValLT:
		case xieValLE:
		case xieValEQ:
		case xieValNE:
		case xieValGE:
		case xieValGT:
			break;
		default:
			fprintf( stderr, "Invalid compare op\n" );
			reps = 0;
			break;
	}

	if ( !reps )
		return( reps );	

	image1 = p->finfo.image1;

	/* we must have image1. image2, however, is optional */
 
	if ( !image1 )
		return ( 0 );
	image2 = p->finfo.image2;

	if ( image2 )
		flo_elements = 4;
	else
		flo_elements = 3;
	if ( useROI == True )
		flo_elements++;

	flograph = XieAllocatePhotofloGraph( flo_elements );	
	if ( flograph == ( XiePhotoElement * ) NULL )
	{
		fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
		reps = 0;
	}
	else if ( ( XIEPhotomap1 = 
		GetXIEPhotomap( xp, p, 1 ) ) == ( XiePhotomap ) NULL )
	{
		reps = 0;
	}
	else if ( image2 ) 
	{
		if ( ( XIEPhotomap2 =
			GetXIEPhotomap( xp, p, 2 ) ) == ( XiePhotomap ) NULL )
		{
			reps = 0;
		}
	}

	if ( useROI == True )
	{
		rect.x = ( ( CompareParms * )p->ts )->x;
		rect.y = ( ( CompareParms * )p->ts )->y;
		rect.width = ( ( CompareParms * )p->ts )->width;
		rect.height = ( ( CompareParms * )p->ts )->height;

		if ( ( XIERoi = GetXIERoi( xp, p, &rect, 1 ) ) == 
			( XieRoi ) NULL )
		{
			reps = 0;
		}
	}

	if ( reps )
	{
		idx = 0;

		domain.offset_x = 0;
		domain.offset_y = 0;


		if ( useROI == True )
		{
	        	XieFloImportROI(&flograph[idx], XIERoi);
			idx++;
			domain.phototag = idx;
		}
		else
		{
			domain.phototag = 0;
		}

		XieFloImportPhotomap(&flograph[idx], XIEPhotomap1, False );
		idx++;
		src1 = idx;

		if ( image2 )
		{
			XieFloImportPhotomap(&flograph[idx], XIEPhotomap2, 
				False );
			idx++;
			src2 = idx;
		}
		else
			src2 = 0;

		XieFloCompare(&flograph[idx], 
			src1,
			src2,
			&domain,
			constant,
			op,
			combine,
			bandMask
		);
		idx++;
			
		XieFloExportDrawablePlane(&flograph[idx],
			idx,     	/* source phototag number */
			xp->w,
			xp->fggc,
			0,       /* x offset in window */
			0        /* y offset in window */
		);
		idx++;
		
		flo = XieCreatePhotoflo( xp->d, flograph, flo_elements );
		flo_notify = True;
	}
	if ( !reps )
		FreeCompareStuff( xp, p );
	return( reps );
}

void 
DoCompare(xp, p, reps)
XParms  xp;
Parms   p;
int     reps;
{
    	int     i;

    	for (i = 0; i != reps; i++) {
		XieExecutePhotoflo( xp->d, flo, flo_notify );
		XSync( xp->d, 0 );
		WaitForXIEEvent( xp, xieEvnNoPhotofloDone, flo, 0, False );	
    	}
}

void 
EndCompare(xp, p)
XParms  xp;
Parms   p;
{
	FreeCompareStuff( xp, p );
}

int
FreeCompareStuff( xp, p )
XParms	xp;
Parms	p;
{
	if ( XIERoi )
        {
                XieDestroyROI( xp->d, XIERoi );
                XIERoi = ( XieRoi ) NULL;
        }

        if ( XIEPhotomap1 != ( XiePhotomap ) NULL && IsPhotomapInCache( XIEPhotomap1 ) == False )
        {
                XieDestroyPhotomap( xp->d, XIEPhotomap1 );
                XIEPhotomap1 = ( XiePhotomap ) NULL;
        }

        if ( XIEPhotomap2 != ( XiePhotomap ) NULL && IsPhotomapInCache( XIEPhotomap2 ) == False )
        {
                XieDestroyPhotomap( xp->d, XIEPhotomap2 );
                XIEPhotomap2 = ( XiePhotomap ) NULL;
        }

        if ( flograph )
        {
                XieFreePhotofloGraph(flograph,flo_elements);
                flograph = ( XiePhotoElement * ) NULL;
        }

        if ( flo )
        {
                XieDestroyPhotoflo( xp->d, flo );
                flo = ( XiePhotoflo ) NULL;
        }
}