/* $XConsortium: do_logical.c,v 1.1 93/07/19 13:03:04 rws Exp $ */

/**** module do_logical.c ****/
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
  
	do_logical.c -- logical flo element tests 

	Syd Logan -- AGE Logic, Inc. July, 1993 - MIT Alpha release
  
*****************************************************************************/
#include "xieperf.h"
#include <stdio.h>

static XiePhotomap XIEPhotomap;
static XieRoi XIERoi;
static XiePhotomap src1, src2;

static XieLTriplet levels;
static XieConstrainTechnique tech = xieValConstrainClipScale;
static XieClipScaleParam *parms;
static XieConstant in_low,in_high;
static XieLTriplet out_low,out_high;
static int monoflag = 0;

int InitLogicalMonadic(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
	if ( xp->vinfo.depth == 1 )
	{
		monoflag = 1;
		if ( !SetupMonoClipScale( xp, p, levels, in_low, in_high, out_low, out_high, &parms ) )
		{
			reps = 0;
		}
	}	

	if ( reps )
		if ( ( XIEPhotomap = GetXIEPhotomap( xp, p, 1 ) ) == ( XiePhotomap ) NULL )
			reps = 0;
	return( reps );
}

int InitLogicalDyadic(xp, p, reps)
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

        if ( ( src1 = GetXIEPhotomap( xp, p, 1 ) ) == ( XiePhotomap ) NULL )
                reps = 0;
        else
        {
		free( p->data );
                if ( ( src2 = GetXIEPhotomap( xp, p, 2 ) ) == ( XiePhotomap ) NULL )
		{
			XieDestroyPhotomap( xp->d, src1 );
                        reps = 0;
		}
        }
        return( reps );
}

int InitROILogicalMonadic(xp, p, reps)
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
        rectsSize = 5;
        rects = (XieRectangle *)malloc( rectsSize * sizeof( XieRectangle ) );
        if ( rects == ( XieRectangle * ) NULL )
                reps = 0;
        else
        {
                for ( i = 0; i < rectsSize; i++ )
                {
                        rects[ i ].x = i * 100;
                        rects[ i ].y = i * 100;
                        rects[ i ].width = 100;
                        rects[ i ].height = 100;
                }
        }
        if ( ( XIERoi = GetXIERoi( xp, p, rects, rectsSize ) ) == ( XieRoi ) NULL )
        {
                reps = 0;
        }
        if ( rects )
                free( rects );
        if ( ( XIEPhotomap = GetXIEPhotomap( xp, p, 1 ) ) == ( XiePhotomap ) NULL )
	{
		XieDestroyROI( xp->d, XIERoi );
                reps = 0;
	}
        return( reps );
}

int InitROILogicalDyadic(xp, p, reps)
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
        rectsSize = 5;
        rects = (XieRectangle *)malloc( rectsSize * sizeof( XieRectangle ) );
        if ( rects == ( XieRectangle * ) NULL )
                reps = 0;
        else
        {
                for ( i = 0; i < rectsSize; i++ )
                {
                        rects[ i ].x = i * 100;
                        rects[ i ].y = i * 100;
                        rects[ i ].width = 100;
                        rects[ i ].height = 100;
                }
        }
        if ( ( XIERoi = GetXIERoi( xp, p, rects, rectsSize ) ) == ( XieRoi ) NULL )
        {
                reps = 0;
        }
        if ( rects )
                free( rects );

        if ( ( src1 = GetXIEPhotomap( xp, p, 1 ) ) == ( XiePhotomap ) NULL )
	{
                reps = 0;
		XieDestroyROI( xp->d, XIERoi );
	}
        else
        {
		free( p->data );
                if ( ( src2 = GetXIEPhotomap( xp, p, 2 ) ) == ( XiePhotomap ) NULL )
		{
                        reps = 0;
			XieDestroyPhotomap( xp->d, src1 );
			XieDestroyROI( xp->d, XIERoi );
		}
        }
        return( reps );
}

void DoLogicalMonadicImmediate(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    	int     i;
	int     band_mask = 1;
	char 	*datatmp;
	int	flo_notify, flo_id;
	int	flo_elements;
	XiePhotospace photospace;
	int     band_order = xieValLSFirst;
	XieProcessDomain domain;
	XiePhotoElement *flograph;
	int	decode_notify;

        photospace = XieCreatePhotospace(xp->d);/* XXX error check */
	flo_id = 1;
	if ( monoflag )
		flo_elements = 4;
	else
		flo_elements = 3;
	flograph = XieAllocatePhotofloGraph(flo_elements);	
	if ( flograph == ( XiePhotoElement * ) NULL )
	{
		fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
		return;
	}
	XieFloImportPhotomap(&flograph[0], XIEPhotomap, decode_notify);

	domain.offset_x = 0;
	domain.offset_y = 0;
	domain.phototag = 0;

	XieFloLogical(&flograph[1], 
		1,
		0,
		&domain,
		p->logicalConstant,
		p->logicalOp,
		p->logicalBandMask );
	if ( monoflag )
	{
		XieFloConstrain(&flograph[2],
			2,
			levels,
			tech,
			(char *)parms
		); 		
	}	
	XieFloExportDrawable(&flograph[flo_elements - 1],
		flo_elements - 1, /* source phototag number */
		xp->w,
		xp->fggc,
		p->dst_x,       /* x offset in window */
		p->dst_y        /* y offset in window */
	);
	flo_notify = True;	
    	for (i = 0; i != reps; i++) {


       		XieExecuteImmediate(xp->d, photospace,
               		flo_id,		
               		flo_notify,     
               		flograph,       /* photoflo specification */
               		flo_elements    /* number of elements */
       		);
		XSync( xp->d, 0 );
		WaitForFloToFinish( xp, flo_id );
		flo_id++;
    	}
	XieDestroyPhotospace( xp->d, photospace );
	XieFreePhotofloGraph(flograph,flo_elements);	
}

void DoLogicalDyadicImmediate(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
        int     i, flo_elements;
        int     band_mask = 1;
        char    *datatmp;
        int     flo_notify, flo_id;
        XiePhotospace photospace;
        int     band_order = xieValLSFirst;
        XieProcessDomain domain;
        XiePhotoElement *flograph;
        int     decode_notify;

        photospace = XieCreatePhotospace(xp->d);/* XXX error check */

        flo_id = 1;
        if ( monoflag )
                flo_elements = 5;
        else
                flo_elements = 4;
        flograph = XieAllocatePhotofloGraph(flo_elements);
        if ( flograph == ( XiePhotoElement * ) NULL )
        {
                fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
                return;
        }

	XieFloImportPhotomap(&flograph[0], src1,
		decode_notify);

	XieFloImportPhotomap(&flograph[1], src2,
		decode_notify);

	domain.offset_x = 0;
	domain.offset_y = 0;
	domain.phototag = 0;

	XieFloLogical(&flograph[2],
		1,
		2,
		&domain,
		p->logicalConstant,
		p->logicalOp,
		p->logicalBandMask );
	if ( monoflag )
	{
		XieFloConstrain(&flograph[3],
			3,
			levels,
			tech,
			(char *)parms
		);
	}

       XieFloExportDrawable(&flograph[flo_elements-1],
		flo_elements-1,         /* source phototag number */
		xp->w,
		xp->fggc,
		p->dst_x,       /* x offset in window */
		p->dst_y        /* y offset in window */
	);

	flo_notify = True;
        for (i = 0; i != reps; i++) {

                XieExecuteImmediate(xp->d, photospace,
                        flo_id,
                        flo_notify,
                        flograph,       /* photoflo specification */
                        flo_elements    /* number of elements */
                );
		WaitForFloToFinish( xp, flo_id );
                XSync( xp->d, 0 );

                flo_id++;
        }
        XieDestroyPhotospace( xp->d, photospace );
        XieFreePhotofloGraph(flograph,flo_elements);
}

void DoROILogicalMonadicImmediate(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    	int     i, flo_elements;
	int     band_mask = 1;
	char 	*datatmp;
	int	flo_notify, flo_id;
	XiePhotospace photospace;
	int     band_order = xieValLSFirst;
	XieProcessDomain domain;
	XiePhotoElement *flograph;
	int	decode_notify;

	photospace = XieCreatePhotospace(xp->d);/* XXX error check */

	flo_id = 1;
        if ( monoflag )
                flo_elements = 5;
        else
                flo_elements = 4;
	flograph = XieAllocatePhotofloGraph(flo_elements);	
	if ( flograph == ( XiePhotoElement * ) NULL )
	{
		fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
		return;
	}

	XieFloImportPhotomap(&flograph[0], XIEPhotomap, decode_notify);

	XieFloImportROI(&flograph[1], XIERoi);

	domain.offset_x = 0;
	domain.offset_y = 0;
	domain.phototag = 2;

	XieFloLogical(&flograph[2], 
		1,
		0,
		&domain,
		p->logicalConstant,
		p->logicalOp,
		p->logicalBandMask );

	if ( monoflag )
	{
		XieFloConstrain(&flograph[3],
			3,
			levels,
			tech,
			(char *)parms
		);
	}

       XieFloExportDrawable(&flograph[flo_elements - 1],
		flo_elements - 1,       /* source phototag number */
		xp->w,
		xp->fggc,
		p->dst_x,       /* x offset in window */
		p->dst_y        /* y offset in window */
	);

	flo_notify = True;	
    	for (i = 0; i != reps; i++) {

       		XieExecuteImmediate(xp->d, photospace,
               		flo_id,		
               		flo_notify,     
               		flograph,       /* photoflo specification */
               		flo_elements    /* number of elements */
       		);

		WaitForFloToFinish( xp, flo_id );
		XSync( xp->d, 0 );

		flo_id++;
    	}
	XieDestroyPhotospace( xp->d, photospace );
	XieFreePhotofloGraph(flograph,flo_elements);	
}

void DoROILogicalDyadicImmediate(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
        int     i, flo_elements;
        int     band_mask = 1;
        char    *datatmp;
        int     flo_notify, flo_id;
        XiePhotospace photospace;
        int     band_order = xieValLSFirst;
        XieProcessDomain domain;
        XiePhotoElement *flograph;
        int     decode_notify;

        photospace = XieCreatePhotospace(xp->d);/* XXX error check */

        flo_id = 1;
        if ( monoflag )
                flo_elements = 6;
        else
                flo_elements = 5;
        flograph = XieAllocatePhotofloGraph(flo_elements);
        if ( flograph == ( XiePhotoElement * ) NULL )
        {
                fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
                return;
        }

	XieFloImportPhotomap(&flograph[0], src1,
		decode_notify);

	XieFloImportPhotomap(&flograph[1], src2,
		decode_notify);

	XieFloImportROI(&flograph[2], XIERoi );

	domain.offset_x = 0;
	domain.offset_y = 0;
	domain.phototag = 3;

	XieFloLogical(&flograph[3],
		1,
		2,
		&domain,
		p->logicalConstant,
		p->logicalOp,
		p->logicalBandMask );
	if ( monoflag )
	{
		XieFloConstrain(&flograph[4],
			4,
			levels,
			tech,
			(char *)parms
		);
	}
       XieFloExportDrawable(&flograph[flo_elements - 1],
		flo_elements - 1,   /* source phototag number */
		xp->w,
		xp->fggc,
		p->dst_x,       /* x offset in window */
		p->dst_y        /* y offset in window */
	);

	flo_notify = True;
        for (i = 0; i != reps; i++) {

                XieExecuteImmediate(xp->d, photospace,
                        flo_id,
                        flo_notify,
                        flograph,       /* photoflo specification */
                        flo_elements    /* number of elements */
                );

		WaitForFloToFinish( xp, flo_id );
                XSync( xp->d, 0 );
                flo_id++;
        }
        XieFreePhotofloGraph(flograph,flo_elements);
        XieDestroyPhotospace( xp->d, photospace );
}

int EndLogicalMonadic(xp, p)
    XParms  xp;
    Parms   p;
{
	free( p->data );
	XieDestroyPhotomap( xp->d, XIEPhotomap );
}

int EndLogicalDyadic(xp, p)
    XParms  xp;
    Parms   p;
{
	free( p->data );
	XieDestroyPhotomap( xp->d, src1 );
	XieDestroyPhotomap( xp->d, src2 );
}

int EndROILogicalMonadic(xp, p)
    XParms  xp;
    Parms   p;
{
        free( p->data );
        XieDestroyPhotomap( xp->d, XIEPhotomap );
        XieDestroyROI( xp->d, XIERoi );
}

int EndROILogicalDyadic(xp, p)
    XParms  xp;
    Parms   p;
{
        free( p->data );
        XieDestroyPhotomap( xp->d, src1 );
        XieDestroyPhotomap( xp->d, src2 );
        XieDestroyROI( xp->d, XIERoi );
}

