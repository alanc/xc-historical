/* $XConsortium: cvttoindex.c,v 1.2 93/10/26 10:05:52 rws Exp $ */

/**** module cvttoindex.c ****/
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
  
	cvttoindex.c -- converttoindex flo element tests 

	Syd Logan -- AGE Logic, Inc. July, 1993 - MIT Alpha release
  
*****************************************************************************/
#include "xieperf.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <math.h>

static XiePhotomap ditheredPhotomap;
static XieColorList clist1;
static XieColorList clist2;

static XiePhotoElement *flograph;
static int flo_notify;
static XiePhotoflo flo;
static int flo_elements;

extern Bool showErrors;
extern Bool dontClear;
extern Window drawableWindow;

int 
InitConvertToIndex(xp, p, reps)
XParms  xp;
Parms   p;
int     reps;
{
	XieLTriplet levels;
	double cube;
	int cclass;

#if     defined(__cplusplus) || defined(c_plusplus)
    	cclass = xp->vinfo.c_class;
#else
    	cclass = xp->vinfo.class;
#endif
	if ( !IsColorVisual( cclass ) || IsStaticVisual( cclass ) )
		return( 0 );
	cube = floor( cbrt( ( double ) ( 1 << xp->vinfo.depth ) ) );
	levels[ 0 ] = ( long ) cube;
	levels[ 1 ] = ( long ) cube;
	levels[ 2 ] = ( long ) cube;
	clist1 = ( XieColorList ) NULL;
	clist2 = ( XieColorList ) NULL;
	ditheredPhotomap = ( XiePhotomap ) NULL;
	flograph = ( XiePhotoElement * ) NULL;
	flo = ( XiePhotoflo ) NULL;
	
	if ( !( clist1 = XieCreateColorList( xp->d ) ) )
		reps = 0;
	if ( ( ( CvtToIndexParms * ) p->ts )->addCvtFromIndex == True )
		if ( !( clist2 = XieCreateColorList( xp->d ) ) )
			reps = 0;

	if ( reps )
	{
		if ( ( ( CvtToIndexParms * ) p->ts )->addCvtFromIndex == False ) 		{
			ditheredPhotomap = 
				GetXIEDitheredTriplePhotomap( xp, p, 1, 
					( ( CvtToIndexParms * ) p->ts )->dither,
					0, levels );
			if ( ditheredPhotomap == ( XiePhotomap ) NULL )
			{
				reps = 0;
			}	 
		}
		else
		{
			dontClear = True;
			if ( !GetXIEDitheredTripleWindow( xp, p, xp->w, 1, 
				( ( CvtToIndexParms * ) p->ts )->dither, 
				0, levels ) )
			{
				reps = 0;
			}
			else
			{
	                       	XMoveWindow( xp->d, drawableWindow, WIDTH + 10, 
					0 );
				XMapRaised( xp->d, drawableWindow );
			}
		}
	}
	if ( reps )
	{
		if ( ( ( CvtToIndexParms * ) p->ts )->useDefaultCmap == True )
			InstallDefaultColormap( xp->d, xp->p ); 
		if ( !CreateCvtToIndexFlo( xp, p ) ) 
		{
			fprintf( stderr, 
				"Could not create ConvertToIndex flo\n" );
			reps = 0;
		}
	}

	if ( !reps )
	{
		if ( ( ( CvtToIndexParms * ) p->ts )->useDefaultCmap == True )
			InstallCustomColormap( xp->d, xp->p );
		FreeCvtToIndexStuff( xp, p );
	}
	return( reps );
}

int
CreateCvtToIndexFlo( xp, p )
XParms	xp;
Parms	p;
{
	int decode_notify;
	int idx;
	XieColorAllocAllParam *color_param = NULL;
	XWindowAttributes xwa;
	Bool colorAllocNotify;
	Bool addCvtFromIndex;

	addCvtFromIndex = ( ( CvtToIndexParms * ) p->ts )->addCvtFromIndex;

	if ( addCvtFromIndex == False )
		flo_elements = 3;
	else
		flo_elements = 4;
        flograph = XieAllocatePhotofloGraph( flo_elements );
        if ( flograph == ( XiePhotoElement * ) NULL )
        {
                fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
                return( 0 );
        }

	color_param = XieTecColorAllocAll( 123 );
        if ( color_param == ( XieColorAllocAllParam * ) NULL )
        {
                fprintf( stderr, "XieTecColorAllocAll failed\n" );
        	XieFreePhotofloGraph(flograph,3);
                return( 0 );
        }

	decode_notify = False;
	idx = 0;

	if ( addCvtFromIndex == True )
	{
                XieFloImportDrawable(&flograph[idx],
                        xp->w,
                        0,
                        0,
                        WIDTH,
                        HEIGHT,
                        0,
                        False
                );
		idx++;
	}
	else
	{
		XieFloImportPhotomap(&flograph[idx],ditheredPhotomap,
			decode_notify);
		idx++;
	}

	colorAllocNotify = True;
	if ( ( ( CvtToIndexParms * ) p->ts )->useDefaultCmap == True )
	{
		colorAllocNotify = False;
		XGetWindowAttributes( xp->d, DefaultRootWindow( xp->d ), &xwa );
		XSetWindowColormap( xp->d, xp->w, xwa.colormap );
		XSync( xp->d, 0 );
	}

	XGetWindowAttributes( xp->d, xp->w, &xwa );

	if ( addCvtFromIndex == True )
	{
		XieFloConvertFromIndex(&flograph[idx],
			idx,
			xwa.colormap,	
			xieValTripleBand,
			xp->vinfo.bits_per_rgb
		);
		idx++;
	}

	XieFloConvertToIndex(&flograph[idx],
		idx,
		xwa.colormap,	
		clist1,	
		colorAllocNotify,
		xieValColorAllocAll,
		(char *)color_param
	);
	idx++;

        XieFloExportDrawable(&flograph[idx],
                idx,     /* source phototag number */
		( addCvtFromIndex == False ? xp->w : drawableWindow ),
                xp->fggc,
                0,       /* x offset in window */
                0        /* y offset in window */
        );
	idx++;

	flo = XieCreatePhotoflo( xp->d, flograph, flo_elements );
	XSync( xp->d, 0 );
	free( color_param );
	flo_notify = ( ( CvtToIndexParms * ) p->ts )->flo_notify;
	return( 1 );
}

void 
DoConvertToIndex(xp, p, reps)
XParms  xp;
Parms   p;
int     reps;
{
	int	i;

	for ( i = 0; i < reps; i++ )
	{
                XieExecutePhotoflo(xp->d, flo, flo_notify );
        }
}

/* this is for the ColorAlloc event test ( see events.c ) */

void
DoColorAllocEvent( xp, p, reps )
XParms  xp;
Parms   p;
int     reps;
{
        int     i;

        for (i = 0; i != reps; i++) {
                XieExecutePhotoflo( xp->d, flo, flo_notify );
                WaitForXIEEvent( xp, ( ( EventParms * ) p->ts )->event, flo, 0,
showErrors );
                WaitForXIEEvent( xp, xieEvnNoPhotofloDone, flo, 0, False );
        }
}

void EndConvertToIndex(xp, p)
XParms  xp;
Parms   p;
{
	if ( ( ( CvtToIndexParms * ) p->ts )->useDefaultCmap == True )
		InstallCustomColormap( xp->d, xp->p );
	if ( ( ( CvtToIndexParms * ) p->ts )->addCvtFromIndex == True )
	{
	        XUnmapWindow( xp->d, drawableWindow );
        	dontClear = False;
	}
	FreeCvtToIndexStuff( xp, p );
}

int
FreeCvtToIndexStuff( xp, p )
XParms	xp;
Parms	p;
{
	if ( ditheredPhotomap )
	{
		XieDestroyPhotomap(xp->d, ditheredPhotomap);
		ditheredPhotomap = ( XiePhotomap ) NULL;
	}

	if ( clist1 )
	{
		XieDestroyColorList( xp->d, clist1 );
		clist1 = ( XieColorList ) NULL;
	}

	if ( clist2 )
	{
		XieDestroyColorList( xp->d, clist2 );
		clist2 = ( XieColorList ) NULL;
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
