/* $XConsortium: band.c,v 1.1 93/10/26 10:08:17 rws Exp $ */
/**** module band.c ****/
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
  
	band.c -- BandCombine/BandExtract flo element tests 

	Syd Logan -- AGE Logic, Inc. October, 1993 - MIT Beta release
  
*****************************************************************************/
#include "xieperf.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <math.h>

static XiePhotomap XIEPhotomap1;
static XiePhotomap XIEPhotomap2;
static XiePhotomap XIEPhotomap3;
static XieColorList clist;

static XiePhotoElement *flograph;
static int flo_notify;
static XiePhotoflo flo;
static int flo_elements;
static int colorclass;
static int cclass;

static XStandardColormap stdCmap;
static Bool useStdCmap;
extern Bool WMSafe;

int 
InitBandSelectExtract(xp, p, reps)
XParms  xp;
Parms   p;
int     reps;
{
	XieLTriplet levels;
	int cube;
	int which;

	which = (( BandParms * )p->ts)->which;

#if     defined(__cplusplus) || defined(c_plusplus)
    	cclass = xp->vinfo.c_class;
#else
    	cclass = xp->vinfo.class;
#endif
	clist = ( XieColorList ) NULL;
	XIEPhotomap1 = ( XiePhotomap ) NULL;
	XIEPhotomap2 = ( XiePhotomap ) NULL;
	XIEPhotomap3 = ( XiePhotomap ) NULL;
	flograph = ( XiePhotoElement * ) NULL;
	flo = ( XiePhotoflo ) NULL;

	if ( xp->vinfo.depth < 4 || ( IsColorVisual( cclass ) && IsStaticVisual( cclass ) ) )
		return( 0 );
	colorclass = 0;
	if ( reps )
	{
		cube = icbrt( 1 << xp->vinfo.depth );
	
		if ( IsColorVisual( cclass ) )
		{
			colorclass = 1;
			levels[ 0 ] = cube;
			levels[ 1 ] = cube;
			levels[ 2 ] = cube;
		}
		else
		{
			levels[ 0 ] = 1 << xp->vinfo.depth;
			levels[ 1 ] = 1 << xp->vinfo.depth;
			levels[ 2 ] = 1 << xp->vinfo.depth;
		}

		if (!( clist = XieCreateColorList( xp->d ) ) )
			reps = 0;
	}
	if ( reps )
	{
		XIEPhotomap1 = GetXIEDitheredTriplePhotomap( xp, p, 1,
			xieValDitherDefault, 0, levels );
		if ( XIEPhotomap1 == ( XiePhotomap ) NULL )
		{
			reps = 0;
		}	 
	}

	if ( reps && colorclass )
	{
		InstallDefaultColormap( xp->d, xp->p );
		if ( !CreateColorBandSelectExtractFlo( xp, p, cube ) )
		{
			fprintf( stderr, "CreateColorBandSelectFlo failed\n" );
			reps = 0;
		}
	}
	else if ( reps )
	{
		if ( !CreateGrayBandSelectExtractFlo( xp, p, cube ) )
		{
			fprintf( stderr, "CreateGrayBandSelectFlo failed\n" );
			reps = 0;
		}
	}

	if ( !reps )
	{
		if ( colorclass )
			InstallCustomColormap( xp->d, xp->p );
		FreeBandStuff( xp, p );
	}
	return( reps );
}

int
InitBandColormap(xp, p, reps)
XParms  xp;
Parms   p;
int     reps;
{
	XieLTriplet levels;
        Atom atom;

	useStdCmap = ( ( BandParms * )p->ts )->useStdCmap;
	if ( xp->vinfo.depth == 1 && useStdCmap == True )
		return( 0 );

#if     defined(__cplusplus) || defined(c_plusplus)
    	cclass = xp->vinfo.c_class;
#else
    	cclass = xp->vinfo.class;
#endif
	clist = ( XieColorList ) NULL;
	XIEPhotomap1 = ( XiePhotomap ) NULL;
	flograph = ( XiePhotoElement * ) NULL;
	flo = ( XiePhotoflo ) NULL;

	if ( useStdCmap == True )
	{
		atom = ( ( BandParms * )p->ts )->atom;
		if ( GetStandardColormap( xp, &stdCmap, atom ) == False )
		{
			fprintf( stderr, "Couldn't get a standard colormap\n" );
			fflush( stderr );
			reps = 0;
		}
	}

	if ( reps )
	{
		if ( useStdCmap == True )
		{
			levels[ 0 ] = stdCmap.red_max + 1; 
			levels[ 1 ] = stdCmap.green_max + 1; 
			levels[ 2 ] = stdCmap.blue_max + 1; 
		}
		else
		{
			levels[ 0 ] = 1 << xp->vinfo.depth;
			levels[ 1 ] = 1 << xp->vinfo.depth;
			levels[ 2 ] = 1 << xp->vinfo.depth;
		}

		XIEPhotomap1 = GetXIEDitheredTriplePhotomap( xp, p, 1,
			xieValDitherDefault, 0, levels );
		if ( XIEPhotomap1 == ( XiePhotomap ) NULL )
		{
			reps = 0;
		}	 
	}

	if ( reps )
	{
		if ( !CreateColormapFlo( xp, p, useStdCmap ) )
		{
			fprintf( stderr, "CreateColormapFlo failed\n" );
			reps = 0;
		}
	}

	if ( !reps )
	{
		FreeBandStuff( xp, p );
	}
        else if ( useStdCmap == True )
		if ( WMSafe == True )
			InstallThisColormap( xp->d, xp->p, stdCmap.colormap );
		else
			InstallThisColormap( xp->d, xp->w, stdCmap.colormap );

	return( reps );
}

int
CreateColormapFlo( xp, p, useStdCmap )
XParms  xp;
Parms   p;
Bool	useStdCmap;
{
	int idx, decode_notify;
	unsigned int mylevels;
	XieConstant c1;
	float bias;

	idx = 0;
	if ( useStdCmap == True )
	{
		c1[ 0 ] = stdCmap.red_mult;
		c1[ 1 ] = stdCmap.green_mult;
		c1[ 2 ] = stdCmap.blue_mult;
		bias = ( float ) stdCmap.base_pixel;
	}
	else
	{
		c1[ 0 ] = (( BandParms * )p->ts)->c1[ 0 ];
		c1[ 1 ] = (( BandParms * )p->ts)->c1[ 1 ];
		c1[ 2 ] = (( BandParms * )p->ts)->c1[ 2 ];
		bias = 0.0;
	}

	mylevels = ( 1 << xp->vinfo.depth );

	flo_elements = 3;

        flograph = XieAllocatePhotofloGraph( flo_elements );
        if ( flograph == ( XiePhotoElement * ) NULL )
        {
                fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
                return( 0 );
        }

	decode_notify = False;

        XieFloImportPhotomap(&flograph[idx],XIEPhotomap1,decode_notify);
	idx++;

	XieFloBandExtract( &flograph[idx], idx, mylevels, bias, c1 ); idx++;

       	XieFloExportDrawable(&flograph[idx],
		idx,              /* source phototag number */
		xp->w,
		xp->fggc,
		0,       /* x offset in window */
		0        /* y offset in window */
	);

	flo = XieCreatePhotoflo( xp->d, flograph, flo_elements );
	XSync( xp->d, 0 );
	flo_notify = False;
	return( 1 );
}

int 
InitBandCombine(xp, p, reps)
XParms  xp;
Parms   p;
int     reps;
{
#if     defined(__cplusplus) || defined(c_plusplus)
    	cclass = xp->vinfo.c_class;
#else
    	cclass = xp->vinfo.class;
#endif
	if ( xp->vinfo.depth < 4 || IsStaticVisual( cclass ) )
		return( 0 );

	clist = ( XieColorList ) NULL;
	XIEPhotomap1 = ( XiePhotomap ) NULL;
	XIEPhotomap2 = ( XiePhotomap ) NULL;
	XIEPhotomap3 = ( XiePhotomap ) NULL;
	flograph = ( XiePhotoElement * ) NULL;
	flo = ( XiePhotoflo ) NULL;

	if ( reps )
	{
		XIEPhotomap1 = GetXIEPhotomap( xp, p, 1 );
		XIEPhotomap2 = GetXIEPhotomap( xp, p, 2 );
		XIEPhotomap3 = GetXIEPhotomap( xp, p, 3 );

		if ( XIEPhotomap1 == ( XiePhotomap ) NULL ||
		     XIEPhotomap2 == ( XiePhotomap ) NULL ||
		     XIEPhotomap3 == ( XiePhotomap ) NULL )
		{
			reps = 0;
		}	 
	}

	if ( reps )
	{
		if ( !CreateBandCombineFlo( xp, p ) )
		{
			fprintf( stderr, "CreateBandCombineFlo failed\n" );
			reps = 0;
		}
	}

	if ( !reps )
	{
		FreeBandStuff( xp, p );
	}
	return( reps );
}

int
CreateBandCombineFlo( xp, p )
XParms	xp;
Parms	p;
{
	int idx, decode_notify;
	float bias;
	XieConstant c;
	unsigned int mylevels;

	mylevels = 1 << xp->vinfo.depth;

	idx = 0;
	flo_elements = 6;

	c[ 0 ] = ( ( BandParms * )p->ts )->c1[ 0 ];
	c[ 1 ] = ( ( BandParms * )p->ts )->c1[ 1 ];
	c[ 2 ] = ( ( BandParms * )p->ts )->c1[ 2 ];
	bias = ( ( BandParms * )p->ts )->bias;

        flograph = XieAllocatePhotofloGraph( flo_elements );
        if ( flograph == ( XiePhotoElement * ) NULL )
        {
                fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
                return( 0 );
        }

	decode_notify = False;

        XieFloImportPhotomap(&flograph[idx],XIEPhotomap1,decode_notify);
	idx++;

        XieFloImportPhotomap(&flograph[idx],XIEPhotomap2,decode_notify);
	idx++;

        XieFloImportPhotomap(&flograph[idx],XIEPhotomap3,decode_notify);
	idx++;

	XieFloBandCombine( &flograph[ idx ], idx - 2, idx - 1, idx ); idx++;

	XieFloBandExtract( &flograph[idx], idx, mylevels, bias, c ); idx++;

       	XieFloExportDrawable(&flograph[idx],
		idx,              /* source phototag number */
		xp->w,
		xp->fggc,
		0,       /* x offset in window */
		0        /* y offset in window */
	);

	flo = XieCreatePhotoflo( xp->d, flograph, flo_elements );
	XSync( xp->d, 0 );
	flo_notify = False;
	return( 1 );
}

int
CreateColorBandSelectExtractFlo( xp, p, cube )
XParms	xp;
Parms	p;
int 	cube;
{
	int idx, which, decode_notify;
	XieColorAllocAllParam *color_param = NULL;
	unsigned int mylevels;
	XWindowAttributes xwa;
	XieConstant c1, c2, c3;
	float bias;
	XIEimage *image;

	which = (( BandParms * )p->ts)->which;

	image = p->finfo.image1;
	idx = 0;

	if ( which == BandExtract )
	{
		c1[ 0 ] = (( BandParms * )p->ts)->c1[ 0 ];
		c1[ 1 ] = (( BandParms * )p->ts)->c1[ 1 ];
		c1[ 2 ] = (( BandParms * )p->ts)->c1[ 2 ];

		c2[ 0 ] = (( BandParms * )p->ts)->c2[ 0 ];
		c2[ 1 ] = (( BandParms * )p->ts)->c2[ 1 ];
		c2[ 2 ] = (( BandParms * )p->ts)->c2[ 2 ];

		c3[ 0 ] = (( BandParms * )p->ts)->c3[ 0 ];
		c3[ 1 ] = (( BandParms * )p->ts)->c3[ 1 ];
		c3[ 2 ] = (( BandParms * )p->ts)->c3[ 2 ];

		bias = (( BandParms * )p->ts)->bias;
	}
	flo_elements = 7;

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

        XieFloImportPhotomap(&flograph[idx],XIEPhotomap1,decode_notify);
	idx++;

	if ( IsColorVisual( cclass ) )
	{
		XGetWindowAttributes( xp->d, DefaultRootWindow( xp->d ), &xwa );
		XSetWindowColormap( xp->d, xp->w, xwa.colormap );
		XSync( xp->d, 0 );
	}
	else
	{
		XGetWindowAttributes( xp->d, xp->w, &xwa );
	}	

	if ( which == BandExtract )
	{
		mylevels = cube;
		XieFloBandExtract( &flograph[idx], 1, mylevels, bias, c1 ); idx++;
		XieFloBandExtract( &flograph[idx], 1, mylevels, bias, c2 ); idx++;
		XieFloBandExtract( &flograph[idx], 1, mylevels, bias, c3 ); idx++;
	}
	else
	{
		XieFloBandSelect( &flograph[idx], 1, 0 ); idx++;
		XieFloBandSelect( &flograph[idx], 1, 1 ); idx++;
		XieFloBandSelect( &flograph[idx], 1, 2 ); idx++;
	}

	XieFloBandCombine( &flograph[ idx ], idx - 2, idx - 1, idx ); idx++;

	XieFloConvertToIndex(&flograph[idx],
		idx,
		xwa.colormap,	
		clist,	
		False,
		xieValColorAllocAll,
		(char *)color_param
	);
	idx++;

       	XieFloExportDrawable(&flograph[idx],
		idx,              /* source phototag number */
		xp->w,
		xp->fggc,
		0,       /* x offset in window */
		0        /* y offset in window */
	);

	flo = XieCreatePhotoflo( xp->d, flograph, flo_elements );
	XSync( xp->d, 0 );
	if ( color_param )
		free( color_param );
	flo_notify = False;
	return( 1 );
}

int
CreateGrayBandSelectExtractFlo( xp, p, cube )
XParms	xp;
Parms	p;
int cube;
{
	int decode_notify;
	unsigned int mylevels;
	XieLTriplet levels;
	XieConstant c1;
	int idx;
	float bias;
	int which;

	which = (( BandParms * )p->ts)->which;
	idx = 0;

	if ( which == BandExtract )
	{
		bias = (( BandParms * )p->ts)->bias;
		c1[ 0 ] = (( BandParms * )p->ts)->c1[ 0 ];
		c1[ 1 ] = (( BandParms * )p->ts)->c1[ 1 ];
		c1[ 2 ] = (( BandParms * )p->ts)->c1[ 2 ];
	}

	mylevels = 1 << xp->vinfo.depth; 

	flo_elements = 4;

        flograph = XieAllocatePhotofloGraph( flo_elements );
        if ( flograph == ( XiePhotoElement * ) NULL )
        {
                fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
                return( 0 );
        }

	decode_notify = False;

        XieFloImportPhotomap( &flograph[idx],XIEPhotomap1,decode_notify );
	idx++;

	if ( which == BandExtract )
		XieFloBandExtract( &flograph[idx], 1, mylevels, bias, c1 ); 
	else
		XieFloBandSelect( &flograph[idx], 1, 0 );
	idx++;

	levels[ 0 ] = 1 << xp->vinfo.depth; 
	levels[ 1 ] = 0;
	levels[ 2 ] = 0;

	XieFloDither( &flograph[idx], idx, 1, levels, xieValDitherDefault, 
		NULL );
	idx++;

	XieFloExportDrawable(&flograph[idx],
		idx,              /* source phototag number */
		xp->w,
		xp->fggc,
		0,       /* x offset in window */
		0        /* y offset in window */
	);

	flo = XieCreatePhotoflo( xp->d, flograph, flo_elements );
	XSync( xp->d, 0 );
	flo_notify = False;
	return( 1 );
}

void 
DoBand(xp, p, reps)
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

void EndBandCombine( xp, p )
XParms	xp;
Parms	p;
{
	FreeBandStuff( xp, p );
}

void EndBandColormap( xp, p )
XParms	xp;
Parms	p;
{
	if ( useStdCmap == True )
		if ( WMSafe == True )
			InstallCustomColormap( xp->d, xp->p );
		else
			InstallCustomColormap( xp->d, xp->w );
	FreeBandStuff( xp, p );
}

void EndBandSelectExtract(xp, p)
XParms  xp;
Parms   p;
{
	if ( IsColorVisual( cclass ) )
		InstallCustomColormap( xp->d, xp->p );
	FreeBandStuff( xp, p );
}

int
FreeBandStuff( xp, p )
XParms	xp;
Parms	p;
{
	if ( XIEPhotomap1 && IsPhotomapInCache( XIEPhotomap1 ) == False )
	{
		XieDestroyPhotomap(xp->d, XIEPhotomap1);
		XIEPhotomap1 = ( XiePhotomap ) NULL;
	}

	if ( XIEPhotomap2 && IsPhotomapInCache( XIEPhotomap2 ) == False )
	{
		XieDestroyPhotomap(xp->d, XIEPhotomap2);
		XIEPhotomap2 = ( XiePhotomap ) NULL;
	}

	if ( XIEPhotomap3 && IsPhotomapInCache( XIEPhotomap3 ) == False )
	{
		XieDestroyPhotomap(xp->d, XIEPhotomap3);
		XIEPhotomap3 = ( XiePhotomap ) NULL;
	}

	if ( clist )
	{
		XieDestroyColorList( xp->d, clist );
		clist = ( XieColorList ) NULL;
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
