/* $XConsortium$ */

/**** module do_import.c ****/
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
  
	do_import.c -- import flo element tests 

	Syd Logan -- AGE Logic, Inc. July, 1993 - MIT Alpha release
  
*****************************************************************************/
#include "xieperf.h"
#include <stdio.h>

static XiePhotomap XIEPhotomap;
static XieRoi	XIERoi;
static XieLut	XIELut;
extern Window 	drawableWindow;
static Pixmap	myPixmap;

static XieLTriplet levels;
static XieConstrainTechnique tech = xieValConstrainClipScale;
static XieClipScaleParam *parms;
static XieConstant in_low,in_high;
static XieLTriplet out_low,out_high;
static int monoflag = 0;

extern Bool dontClear;

int InitImportDrawablePixmap(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
	if ( xp->vinfo.depth == 1 )
	{
		monoflag = 1;
                if ( !SetupMonoClipScale( xp, p, levels, in_low, in_high, out_low, out_high, &parms ) )
                {
			return 0;
                }
	}
	else
		monoflag = 0;
	myPixmap = XCreatePixmap( xp->d, drawableWindow,
		WIDTH, HEIGHT, xp->vinfo.depth );
	XFillRectangle( xp->d, myPixmap, xp->bggc, 0, 0, 
		WIDTH, HEIGHT );
	XSync( xp->d, 0 );
	GetXIEPixmap( xp, p, myPixmap, 1 );
	return( reps );
}

int InitImportDrawableWindow(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
	if ( xp->vinfo.depth == 1 )
	{
		monoflag = 1;
                if ( !SetupMonoClipScale( xp, p, levels, in_low, in_high, out_low, out_high, &parms ) )
                {
			return 0;
                }
	}
	else
		monoflag = 0;
	GetXIEWindow( xp, p, xp->w, 1 );
	if ( p->testPrivate == Obscuring )
		XMoveWindow( xp->d, drawableWindow, 100, 100 );
	else if ( p->testPrivate == Obscured )
		XMoveWindow( xp->d, drawableWindow, 400, 300 );
	else
		XMoveWindow( xp->d, drawableWindow, WIDTH + 10, 0 );
	if ( p->testPrivate == Obscured )
	{
		XLowerWindow( xp->d, drawableWindow );
		XSync( xp->d, 0 );
		XMapWindow( xp->d, drawableWindow );
	}
	else
		XMapRaised( xp->d, drawableWindow );
	dontClear = True;
	return( reps );
}

int InitImportPhoto(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
	if ( ( XIEPhotomap = GetXIEPhotomap( xp, p, 1 ) ) == ( XiePhotomap ) NULL )
	{
		reps = 0;
	}
	return( reps );
}

int InitImportLUT(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
	unsigned char *lut;
	int lutSize, i;

	lutSize = ( 1 << xp->vinfo.depth ) * sizeof( unsigned char );
	lut = malloc( lutSize );
	if ( lut == ( unsigned char * ) NULL )
		reps = 0;
	else
	{
		for ( i = 0; i < lutSize; i++ )
		{
			if ( i % 5 == 0 )
			{
				lut[ i ] = ( 1 << xp->vinfo.depth ) - i;
			}
			else
			{
				lut[ i ] = i;
			}
		}
	}
	if ( ( XIELut = GetXIELut( xp, p, lut, lutSize ) ) == ( XieLut ) NULL )
	{
		reps = 0;
	}
	if ( lut )
		free( lut );
	return( reps );
}

int InitImportROI(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
	XieRectangle *rects;
	int     rectsSize, i;

	rectsSize = 10;
	rects = malloc( rectsSize * sizeof( XieRectangle ) );
	if ( rects == ( XieRectangle * ) NULL )
		reps = 0;
	else
	{
		/* who cares what the data is */

		for ( i = 0; i < rectsSize; i++ )
		{
			rects[ i ].x = i;
			rects[ i ].y = i;
			rects[ i ].width = i + 10;
			rects[ i ].height = i + 10;
		}
	}
	if ( ( XIERoi = GetXIERoi( xp, p, rects, rectsSize ) ) == ( XieRoi ) NULL )
	{
		reps = 0;
	}
	if ( rects )
		free( rects );
	return( reps );
}

void DoImportPhotoImmediate(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    	int     i;
	int	flo_notify, flo_id;
	XiePhotospace photospace;
        XieEncodeTechnique encode_tech=xieValEncodeServerChoice;
        char *encode_params=NULL;
	XiePhotoElement *flograph;
	int	decode_notify;

	photospace = XieCreatePhotospace(xp->d);/* XXX error check */

	flo_id = 1;
	flo_notify = False;	
	flograph = XieAllocatePhotofloGraph(2);	
	if ( flograph == ( XiePhotoElement * ) NULL )
	{
		fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
		return;
	}

	XieFloImportPhotomap(&flograph[0], XIEPhotomap, decode_notify);

	XieFloExportPhotomap(&flograph[1],
		1,              /* source phototag number */
		XIEPhotomap,
		encode_tech,
		encode_params
	);

    	for (i = 0; i != reps; i++) {
       		XieExecuteImmediate(xp->d, photospace,
               		flo_id,		
               		flo_notify,     
               		flograph,       /* photoflo specification */
               		2               /* number of elements */
       		);

		flo_id++;
		XSync( xp->d, 0 );
    	}
	XieFreePhotofloGraph(flograph,2);	
	XieDestroyPhotospace( xp->d, photospace );
}

void DoImportPhotoStored(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    	int     i;
	int	flo_notify;
	XiePhotoElement *flograph;
	XiePhotoflo flo;
        int     decode_notify;
        XieEncodeTechnique encode_tech=xieValEncodeServerChoice;
        char *encode_params=NULL;

	flograph = XieAllocatePhotofloGraph(2);	
	if ( flograph == ( XiePhotoElement * ) NULL )
	{
		fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
		return;
	}

        XieFloImportPhotomap(&flograph[0], XIEPhotomap,
		decode_notify);

        XieFloExportPhotomap(&flograph[1],
	    	1,              /* source phototag number */
		XIEPhotomap,
		encode_tech,
		encode_params
	);

	flo = XieCreatePhotoflo( xp->d, flograph, 2 );

	/* crank it */

	flo_notify = 0;
    	for (i = 0; i != reps; i++) {
		XieExecutePhotoflo( xp->d, flo, flo_notify );
		XSync( xp->d, 0 );
    	}

	/* destroy the photoflo */

	XieFreePhotofloGraph(flograph,2);	
	XieDestroyPhotoflo( xp->d, flo );
}

void DoImportDrawablePixmapImmediate(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    	int     i, flo_elements;
	int	flo_notify, flo_id;
	XiePhotospace photospace;
        XieEncodeTechnique encode_tech=xieValEncodeServerChoice;
        char *encode_params=NULL;
	XiePhotoElement *flograph;
	int	decode_notify;

	photospace = XieCreatePhotospace(xp->d);/* XXX error check */

	flo_id = 1;
	flo_elements = 2;
	flo_notify = False;	
	flograph = XieAllocatePhotofloGraph(flo_elements);	
	if ( flograph == ( XiePhotoElement * ) NULL )
	{
		fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
		return;
	}
	XieFloImportDrawable(&flograph[0], 
		myPixmap,
		0, 
		0,
		WIDTH,
		HEIGHT,
		0,
		False
	);	

	XieFloExportDrawable(&flograph[flo_elements-1],
		flo_elements-1,       /* source phototag number */
		xp->w,
		xp->fggc,
		p->dst_x,       /* x offset in window */
		p->dst_y        /* y offset in window */
	);

    	for (i = 0; i != reps; i++) {
       		XieExecuteImmediate(xp->d, photospace,
               		flo_id,		
               		flo_notify,     
               		flograph,       /* photoflo specification */
               		flo_elements    /* number of elements */
       		);

		flo_id++;
		XSync( xp->d, 0 );
    	}
	XieFreePhotofloGraph(flograph,flo_elements);	
	XieDestroyPhotospace( xp->d, photospace );
}

void DoImportDrawableWindowImmediate(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    	int     i, flo_elements;
	int	flo_notify, flo_id;
	XiePhotospace photospace;
        XieEncodeTechnique encode_tech=xieValEncodeServerChoice;
        char *encode_params=NULL;
	XiePhotoElement *flograph;
	int	decode_notify;

	photospace = XieCreatePhotospace(xp->d);/* XXX error check */

	flo_id = 1;
        if ( monoflag )
                flo_elements = 3;
        else
                flo_elements = 2;
	flo_notify = False;	
	flograph = XieAllocatePhotofloGraph(flo_elements);	
	if ( flograph == ( XiePhotoElement * ) NULL )
	{
		fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
		return;
	}
	XieFloImportDrawable(&flograph[0], 
		xp->w,
		0, 
		0,
		WIDTH,
		HEIGHT,
		0,
		False
	);	

	XieFloExportDrawable(&flograph[1],
		1,              /* source phototag number */
		drawableWindow,
		xp->fggc,
		p->dst_x,       /* x offset in window */
		p->dst_y        /* y offset in window */
	);

    	for (i = 0; i != reps; i++) {

       		XieExecuteImmediate(xp->d, photospace,
               		flo_id,		
               		flo_notify,     
               		flograph,       /* photoflo specification */
               		2               /* number of elements */
       		);

		flo_id++;
		XSync( xp->d, 0 );
    	}
	XClearWindow( xp->d, drawableWindow );
	XieFreePhotofloGraph(flograph,2);	
	XieDestroyPhotospace( xp->d, photospace );
}

void DoImportDrawablePlanePixmapImmediate(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    	int     i;
	int	flo_notify, flo_id;
	XiePhotospace photospace;
        XieEncodeTechnique encode_tech=xieValEncodeServerChoice;
        char *encode_params=NULL;
	XiePhotoElement *flograph;
	int	decode_notify;
	int	planes;

	photospace = XieCreatePhotospace(xp->d);/* XXX error check */

	if ( monoflag )
		planes = 1;
	else
		planes = 1 << 4;
	flo_id = 1;
	flo_notify = False;	
	flograph = XieAllocatePhotofloGraph(2);	
	if ( flograph == ( XiePhotoElement * ) NULL )
	{
		fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
		return;
	}

	XieFloImportDrawablePlane(&flograph[0], 
		myPixmap,
		0, 
		0,
		WIDTH,
		HEIGHT,
		0,
		planes,	
		False
	);	

	XieFloExportDrawablePlane(&flograph[1],
		1,              /* source phototag number */
		xp->w,
		xp->fggc,
		p->dst_x,       /* x offset in window */
		p->dst_y        /* y offset in window */
	);

	for ( i = 0; i < reps; i++ ) {
       		XieExecuteImmediate(xp->d, photospace,
               		flo_id,		
               		flo_notify,     
               		flograph,       /* photoflo specification */
               		2               /* number of elements */
       		);

		flo_id++;
		XSync( xp->d, 0 );
    	}
	XieFreePhotofloGraph(flograph,2);	
	XieDestroyPhotospace( xp->d, photospace );
}

void DoImportDrawablePlaneWindowImmediate(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    	int     i;
	int	flo_notify, flo_id;
	XiePhotospace photospace;
        XieEncodeTechnique encode_tech=xieValEncodeServerChoice;
        char *encode_params=NULL;
	XiePhotoElement *flograph;
	int	decode_notify;
	int	planes;

	photospace = XieCreatePhotospace(xp->d);/* XXX error check */

	flo_id = 1;
	if ( monoflag )
		planes = 1;
	else
		planes = 1 << 4;

	flo_notify = False;	
	flograph = XieAllocatePhotofloGraph(2);	
	if ( flograph == ( XiePhotoElement * ) NULL )
	{
		fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
		return;
	}

	XieFloImportDrawablePlane(&flograph[0], 
		xp->w,
		0, 
		0,
		WIDTH,
		HEIGHT,
		0,
		planes,
		False
	);	

	XieFloExportDrawablePlane(&flograph[1],
		1,              /* source phototag number */
		drawableWindow,
		xp->fggc,
		p->dst_x,       /* x offset in window */
		p->dst_y        /* y offset in window */
	);

    	for (i = 0; i != reps; i++) {

       		XieExecuteImmediate(xp->d, photospace,
               		flo_id,		
               		flo_notify,     
               		flograph,       /* photoflo specification */
               		2               /* number of elements */
       		);

		flo_id++;
		XSync( xp->d, 0 );
    	}
	XClearWindow( xp->d, drawableWindow );
	XieDestroyPhotospace( xp->d, photospace );
	XieFreePhotofloGraph(flograph,2);	
}

void DoImportLUTImmediate(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    	int     i;
	int	flo_notify, flo_id;
	XiePhotospace photospace;
	XieOrientation band_order = xieValLSFirst;
        Bool    merge;
        XieLTriplet start;
	XiePhotoElement *flograph;

	photospace = XieCreatePhotospace(xp->d);/* XXX error check */

	flo_id = 1;
	flo_notify = False;	

	flograph = XieAllocatePhotofloGraph(2);	
	if ( flograph == ( XiePhotoElement * ) NULL )
	{
		fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
		return;
	}

	XieFloImportLUT(&flograph[0], XIELut );

	merge = False;
	start[ 0 ] = 0;
	start[ 1 ] = 0;
	start[ 2 ] = 0;

	XieFloExportLUT(&flograph[1],
		1,              /* source phototag number */
		XIELut,
		merge,
		start
	);


    	for (i = 0; i != reps; i++) {
       		XieExecuteImmediate(xp->d, photospace,
               		flo_id,		
               		flo_notify,     
               		flograph,       /* photoflo specification */
               		2               /* number of elements */
       		);

		flo_id++;
		XSync( xp->d, 0 );
    	}
	XieFreePhotofloGraph(flograph,2);	
	XieDestroyPhotospace( xp->d, photospace );
}

void DoImportLUTStored(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    	int     i;
	int	flo_notify;
	XiePhotoElement *flograph;
	XiePhotoflo flo;
        int     decode_notify;
        Bool    merge;
        XieLTriplet start;

	flograph = XieAllocatePhotofloGraph(2);	
	if ( flograph == ( XiePhotoElement * ) NULL )
	{
		fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
		return;
	}

	XieFloImportLUT(&flograph[0], XIELut );

	merge = False;
	start[ 0 ] = 0;
	start[ 1 ] = 0;
	start[ 2 ] = 0;

	XieFloExportLUT(&flograph[1],
		1,              /* source phototag number */
		XIELut,
		merge,
		start
	);

	flo = XieCreatePhotoflo( xp->d, flograph, 2 );

	/* crank it */

	flo_notify = 0;
    	for (i = 0; i != reps; i++) {
		XieExecutePhotoflo( xp->d, flo, flo_notify );
		XSync( xp->d, 0 );
    	}

	/* destroy the photoflo */

	XieFreePhotofloGraph(flograph,2);	
	XieDestroyPhotoflo( xp->d, flo );
}

void DoImportROIImmediate(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    	int     i;
	int	flo_notify, flo_id;
	XiePhotospace photospace;
	XiePhotoElement *flograph;

	photospace = XieCreatePhotospace(xp->d);/* XXX error check */

	flo_id = 1;
	flo_notify = False;	
	flograph = XieAllocatePhotofloGraph(2);	
	if ( flograph == ( XiePhotoElement * ) NULL )
	{
		fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
		return;
	}

	XieFloImportROI(&flograph[0], XIERoi );

	XieFloExportROI(&flograph[1],
		1,              /* source phototag number */
		XIERoi	
	);

    	for (i = 0; i != reps; i++) {

       		XieExecuteImmediate(xp->d, photospace,
               		flo_id,		
               		flo_notify,     
               		flograph,       /* photoflo specification */
               		2               /* number of elements */
       		);

		flo_id++;
		XSync( xp->d, 0 );
    	}
	XieDestroyPhotospace( xp->d, photospace );
	XieFreePhotofloGraph(flograph,2);	
}

void DoImportROIStored(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    	int     i;
	int	flo_notify;
	XiePhotoElement *flograph;
	XiePhotoflo flo;
        int     decode_notify;

	flograph = XieAllocatePhotofloGraph(2);	
	if ( flograph == ( XiePhotoElement * ) NULL )
	{
		fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
		return;
	}

	XieFloImportROI(&flograph[0], XIERoi );

	XieFloExportROI(&flograph[1],
		1,              /* source phototag number */
		XIERoi
	);

	flo = XieCreatePhotoflo( xp->d, flograph, 2 );

	/* crank it */

	flo_notify = 0;
    	for (i = 0; i != reps; i++) {
		XieExecutePhotoflo( xp->d, flo, flo_notify );
		XSync( xp->d, 0 );
    	}

	/* destroy the photoflo */

	XieFreePhotofloGraph(flograph,2);	
	XieDestroyPhotoflo( xp->d, flo );
}

void EndImportLUT(xp, p)
    XParms  xp;
    Parms   p;
{
        /* deallocate the data buffer */

	CloseXIELut( xp, p, XIELut );
}

void EndImportPhoto(xp, p)
    XParms  xp;
    Parms   p;
{
        /* deallocate the data buffer */

	CloseXIEPhotomap( xp, p, XIEPhotomap );
}

void EndImportROI(xp, p)
    XParms  xp;
    Parms   p;
{
        /* deallocate the data buffer */

	CloseXIERoi( xp, p, XIERoi );
}

void EndImportDrawableWindow(xp, p)
    XParms  xp;
    Parms   p;
{
	XUnmapWindow( xp->d, drawableWindow );
	dontClear = False;
}

void EndImportDrawablePixmap(xp, p)
    XParms  xp;
    Parms   p;
{
	XFreePixmap( xp->d, myPixmap );
}

