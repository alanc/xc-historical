/* $XConsortium$ */

/**** module do_modify.c ****/
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
  
	do_modify.c -- modify flo element tests 

	Syd Logan -- AGE Logic, Inc. July, 1993 - MIT Alpha release
  
*****************************************************************************/
#include "xieperf.h"
#include <stdio.h>

static XiePhotomap XIEPhotomap;
static XieRoi XIERoi1, XIERoi2;
static XieLut XIELut1, XIELut2;
static XiePhotoElement *flograph;
static XiePhotoflo	flo;
static int	flo_elements;

static XieLTriplet levels;
static XieConstrainTechnique tech = xieValConstrainClipScale;
static XieClipScaleParam *parms;
static XieConstant in_low,in_high;
static XieLTriplet out_low,out_high;
static int monoflag = 0;

int InitModifyROI(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
        XieRectangle *rects;
        int     rectsSize, i;

        if ( xp->vinfo.depth == 1 )
        {
		monoflag = 1;
		if ( SetupMonoClipScale( xp, p, levels, in_low, in_high, out_low, out_high, &parms ) == 0 )
		{
			return( 0 );
		}
        }
        rectsSize = 4;
        rects = malloc( rectsSize * sizeof( XieRectangle ) );
        if ( rects == ( XieRectangle * ) NULL )
	{
                reps = 0;
	}
        else
        {
                for ( i = 0; i < rectsSize; i++ )
                {
                        rects[ i ].x = i * 100;
                        rects[ i ].y = i * 100;
                        rects[ i ].width = 50;
                        rects[ i ].height = 50;
                }
        	if ( ( XIERoi1 = GetXIERoi( xp, p, rects, rectsSize ) ) == ( XieRoi ) NULL )
        	{
                	reps = 0;
        	}
		else
		{
			for ( i = 0; i < rectsSize; i++ )
			{
				rects[ i ].x = i * 100 + 50;
				rects[ i ].y = i * 100 + 50;
				rects[ i ].width = 50;
				rects[ i ].height = 50;
			}
			if ( ( XIERoi2 = GetXIERoi( xp, p, rects, rectsSize ) ) == ( XieRoi ) NULL )
			{
				XieDestroyROI( xp->d, XIERoi1 );
				reps = 0;
			}
			else
			{
				if ( ( XIEPhotomap = GetXIEPhotomap( xp, p, 1 ) ) == ( XiePhotomap ) NULL )
				{
					XieDestroyROI( xp->d, XIERoi1 );
					XieDestroyROI( xp->d, XIERoi2 );
					free( p->data );
					reps = 0;
				}
			}
		}
	}
	if ( reps )
		if ( !BuildModifyROIFlograph( xp, p, &flograph ) )
		{
			XieDestroyROI( xp->d, XIERoi1 );
			XieDestroyROI( xp->d, XIERoi2 );
			XieDestroyPhotomap( xp->d, XIEPhotomap );
			free( p->data );
			reps = 0;
		}
        if ( rects )
                free( rects );
        return( reps );
}

static int
BuildModifyROIFlograph( xp, p, flograph )
XParms	xp;
Parms	p;
XiePhotoElement **flograph;
{
	XieProcessDomain domain;

        if ( monoflag )
                flo_elements = 5;
        else
                flo_elements = 4;
	*flograph = XieAllocatePhotofloGraph(flo_elements);	
	if ( *flograph == ( XiePhotoElement * ) NULL )
	{
		fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
		return( 0 );
	}

	XieFloImportPhotomap(&(*flograph)[0], XIEPhotomap,
		False);

	XieFloImportROI(&(*flograph)[1], XIERoi1); /* as good a ROI as any */

	domain.offset_x = 0;
	domain.offset_y = 0;
	domain.phototag = 2;

	XieFloLogical(&(*flograph)[2], 
		1,
		0,
		&domain,
		p->logicalConstant,
		p->logicalOp,
		p->logicalBandMask );

        if ( monoflag )
        {
		XieFloConstrain(&(*flograph)[3],
			3,
			levels,
			tech,
			parms
		);
	}

	XieFloExportDrawable(&(*flograph)[flo_elements - 1],
		flo_elements - 1,       /* source phototag number */
		xp->w,
		xp->fggc,
		p->dst_x,       /* x offset in window */
		p->dst_y        /* y offset in window */
	);

	flo = XieCreatePhotoflo( xp->d, *flograph, flo_elements );
	return( 1 );
}

void DoModifyROI(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    	int     i;
	int	flo_notify;
	int	toggle;

	flo_notify = False;	
	toggle = 0;

/* We shouldn't really execute the photoflo if we are interested in really 
   timing the operation, but hey, it's an alpha release and right now 
   showing it works is important ( no other test currently uses it ) */

	for ( i = 0; i != reps; i++ )
	{
                XieExecutePhotoflo( xp->d, flo, flo_notify );
		XSync( xp->d, 0 );
		if ( !toggle )
		{
			toggle = 1;
			XieFloImportROI(&flograph[1], XIERoi2);
		}
		else
		{
			toggle = 0;
			XieFloImportROI(&flograph[1], XIERoi1);
		}
		XieModifyPhotoflo( xp->d, flo, 2, &flograph[1], 1 );
		XSync( xp->d, 0 );
    	}
}

int EndModifyROI(xp, p)
    XParms  xp;
    Parms   p;
{
	free( p->data );
	XieDestroyPhotomap( xp->d, XIEPhotomap );
	XieDestroyROI( xp->d, XIERoi1 );
	XieDestroyROI( xp->d, XIERoi2 );
        XieFreePhotofloGraph(flograph,flo_elements);
        XieDestroyPhotoflo( xp->d, flo );
}

int InitModifyPoint(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    	unsigned char *lut; 
        int     lutSize, i;

        lutSize = 1 << xp->vinfo.depth;
        lut = malloc( lutSize * sizeof( unsigned char ) );
        if ( lut == ( unsigned char * ) NULL )
		return( 0 );
	for ( i = 0; i < lutSize; i++ )
		if ( i % 2 )
			lut[ i ] = 1;
		else
			lut[ i ] = 0;
        if ( ( XIELut1 = GetXIELut( xp, p, lut, lutSize ) ) == ( XieLut ) NULL )
	{
		free( lut );
		return( 0 );
	}
	free( lut );
        lut = malloc( lutSize * sizeof( unsigned char ) );
        if ( lut == ( unsigned char * ) NULL )
	{
		XieDestroyLUT( xp->d, XIELut1 );
		return( 0 );
	}
	for ( i = 0; i < lutSize; i++ )
		if ( i % 2 )
			lut[ i ] = 0;
		else
			lut[ i ] = 1;
        if ( ( XIELut2 = GetXIELut( xp, p, lut, lutSize ) ) == ( XieLut ) NULL )
	{
		free( lut );
		XieDestroyLUT( xp->d, XIELut1 );
		return( 0 );
	}
	free( lut );

	if ( ( XIEPhotomap = GetXIEBitonalPhotomap( xp, p, 1 ) ) == ( XiePhotomap ) NULL )
	{
		XieDestroyLUT( xp->d, XIELut1 );
		XieDestroyLUT( xp->d, XIELut2 );
		free( p->data );
		reps = 0;
	}

	if ( reps )
		if ( !BuildModifyPointFlograph( xp, p, &flograph ) )
		{
			XieDestroyLUT( xp->d, XIELut1 );
			XieDestroyLUT( xp->d, XIELut2 );
			XieDestroyPhotomap( xp->d, XIEPhotomap );
			free( p->data );
			reps = 0;
		}
        return( reps );
}

static int
BuildModifyPointFlograph( xp, p, flograph )
XParms	xp;
Parms	p;
XiePhotoElement **flograph;
{
	XieProcessDomain domain;
        int     band_mask = 1;

        flo_elements = 4;
	*flograph = XieAllocatePhotofloGraph(flo_elements);	
	if ( *flograph == ( XiePhotoElement * ) NULL )
	{
		fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
		return( 0 );
	}

	XieFloImportPhotomap(&(*flograph)[0], XIEPhotomap, False);

	XieFloImportLUT(&(*flograph)[1], XIELut1); /* as good a LUT as any */

	domain.offset_x = 0;
	domain.offset_y = 0;
	domain.phototag = 0;

	XieFloPoint(&(*flograph)[2], 1, &domain, 2, band_mask ); 

	XieFloExportDrawablePlane(&(*flograph)[3],
		3,       /* source phototag number */
		xp->w,
		xp->fggc,
		p->dst_x,       /* x offset in window */
		p->dst_y        /* y offset in window */
	);

	flo = XieCreatePhotoflo( xp->d, *flograph, flo_elements );
	return( 1 );
}

void DoModifyPoint(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    	int     i;
	int	flo_notify;
	int	toggle;

	flo_notify = False;	
	toggle = 0;

/* We shouldn't really execute the photoflo if we are interested in really 
   timing the operation, but hey, it's an alpha release and right now 
   showing it works is important ( no other test currently uses it ) */

	for ( i = 0; i != reps; i++ )
	{
                XieExecutePhotoflo( xp->d, flo, flo_notify );
		XSync( xp->d, 0 );
		if ( !toggle )
		{
			toggle = 1;
			XieFloImportLUT(&flograph[1], XIELut2);
		}
		else
		{
			toggle = 0;
			XieFloImportLUT(&flograph[1], XIELut1);
		}
		XieModifyPhotoflo( xp->d, flo, 2, &flograph[1], 1 );
		XSync( xp->d, 0 );
    	}
}

int EndModifyPoint(xp, p)
    XParms  xp;
    Parms   p;
{
	free( p->data );
	XieDestroyPhotomap( xp->d, XIEPhotomap );
	XieDestroyLUT( xp->d, XIELut1 );
	XieDestroyLUT( xp->d, XIELut2 );
        XieFreePhotofloGraph(flograph,flo_elements);
        XieDestroyPhotoflo( xp->d, flo );
}

int InitModifySimple(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
        if ( xp->vinfo.depth == 1 )
        {
		monoflag = 1;
		if ( SetupMonoClipScale( xp, p, levels, in_low, in_high, out_low, out_high, &parms ) == 0 )
		{
			return( 0 );
		}
        }

	if ( ( XIEPhotomap = GetXIEPhotomap( xp, p, 1 ) ) == ( XiePhotomap ) NULL )
	{
		reps = 0;
	}

	else if ( !BuildModifySimpleFlograph( xp, p, &flograph ) )
	{
		XieDestroyPhotomap( xp->d, XIEPhotomap );
		free( p->data );
		reps = 0;
	}
        return( reps );
}

static int
BuildModifySimpleFlograph( xp, p, flograph )
XParms	xp;
Parms	p;
XiePhotoElement **flograph;
{
	if ( monoflag )
		flo_elements = 3;
	else
        	flo_elements = 2;
	*flograph = XieAllocatePhotofloGraph(flo_elements);	
	if ( *flograph == ( XiePhotoElement * ) NULL )
	{
		fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
		return( 0 );
	}

	XieFloImportPhotomap(&(*flograph)[0], XIEPhotomap, False);

        if ( monoflag )
        {
                XieFloConstrain(&(*flograph)[1],
                        1,
                        levels,
                        tech,
                        parms
                );
        }

	XieFloExportDrawable(&(*flograph)[flo_elements - 1],
		flo_elements - 1,       /* source phototag number */
		xp->w,
		xp->fggc,
		p->dst_x,       /* x offset in window */
		p->dst_y        /* y offset in window */
	);

	flo = XieCreatePhotoflo( xp->d, *flograph, flo_elements );
	return( 1 );
}

void DoModifySimple(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    	int     i;
	int	flo_notify;
	int	toggle;

	flo_notify = False;	
	toggle = 0;

/* We shouldn't really execute the photoflo if we are interested in really 
   timing the operation, but hey, it's an alpha release and right now 
   showing it works is important ( no other test currently uses it ) */

	for ( i = 0; i != reps; i++ )
	{
                XieExecutePhotoflo( xp->d, flo, flo_notify );
		XSync( xp->d, 0 );
		if ( !toggle )
		{
			toggle = 1;
			XieFloExportDrawable(&flograph[ flo_elements - 1],
				flo_elements - 1,  /* source phototag number */
				xp->w,
				xp->fggc,
				p->dst_x,       /* x offset in window */
				p->dst_y        /* y offset in window */
			);
		}
		else
		{
			toggle = 0;
			XieFloExportDrawable(&flograph[ flo_elements - 1],
				flo_elements - 1,  /* source phototag number */
				xp->w,
				xp->fggc,
				p->dst_x + 100,       /* x offset in window */
				p->dst_y + 100        /* y offset in window */
			);
		}
		XClearWindow( xp->d, xp->w );
		XieModifyPhotoflo( xp->d, flo, flo_elements, &flograph[flo_elements - 1], 1 );
		XSync( xp->d, 0 );
    	}
}

int EndModifySimple(xp, p)
    XParms  xp;
    Parms   p;
{
	free( p->data );
	XieDestroyPhotomap( xp->d, XIEPhotomap );
        XieFreePhotofloGraph(flograph,flo_elements);
        XieDestroyPhotoflo( xp->d, flo );
}

