/* $XConsortium$ */

/**** module rgb.c ****/
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
  
	rgb.c -- ConvertToRGB/ConvertFromRGB flo element tests 

	Syd Logan -- AGE Logic, Inc. September, 1993 - MIT Beta release
  
*****************************************************************************/
#include "xieperf.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <math.h>

static XiePhotomap XIEPhotomap;
static XieColorList clist;

static XiePhotoElement *flograph;
static int flo_notify;
static XiePhotoflo flo;
static int flo_elements;

int 
InitRGB(xp, p, reps)
XParms  xp;
Parms   p;
int     reps;
{
	XieLTriplet levels;
	int cclass;

#if     defined(__cplusplus) || defined(c_plusplus)
    	cclass = xp->vinfo.c_class;
#else
    	cclass = xp->vinfo.class;
#endif
	if ( !IsColorVisual( cclass ) )
		return( 0 );
	clist = ( XieColorList ) NULL;
	XIEPhotomap = ( XiePhotomap ) NULL;
	flograph = ( XiePhotoElement * ) NULL;
	flo = ( XiePhotoflo ) NULL;

	InstallDefaultColormap( xp->d, xp->p );
	
	if ( !( clist = XieCreateColorList( xp->d ) ) )
		reps = 0;

	if ( reps )
	{
		XIEPhotomap = GetXIETriplePhotomap( xp, p, 1 ); 

		if ( XIEPhotomap == ( XiePhotomap ) NULL )
		{
			reps = 0;
		}	 
	}
	if ( reps )
	{
		if ( !CreateRGBFlo( xp, p ) ) 
		{
			fprintf( stderr, "Could not create RGB flo\n" );
			reps = 0;
		}
	}

	if ( !reps )
	{
		InstallCustomColormap( xp->d, xp->p );
		FreeRGBStuff( xp, p );
	}
	return( reps );
}

int
CreateRGBFlo( xp, p )
XParms	xp;
Parms	p;
{
	RGBParms *rgb = ( RGBParms * )p->ts;
	int idx, decode_notify;
	double cube;
	char *techParms, *colorParm1, *colorParm2;
	char *whiteAdjustParm, *gamutParm;
	XieRGBToCIELabParam *RGBToCIELabParm;
	XieRGBToCIEXYZParam *RGBToCIEXYZParm;
	XieRGBToYCbCrParam *RGBToYCbCrParm;
	XieRGBToYCCParam *RGBToYCCParm;
	XieCIELabToRGBParam *CIELabToRGBParm;
	XieCIEXYZToRGBParam *CIEXYZToRGBParm;
	XieYCbCrToRGBParam *YCbCrToRGBParm;
	XieYCCToRGBParam *YCCToRGBParm;
	XieColorAllocAllParam *convertToIndexParms;
	XieColorspace colorSpace;
	XWindowAttributes xwa;
	Bool colorAllocNotify;
	XieMatrix toMatrix, fromMatrix;
	XieWhiteAdjustTechnique whiteAdjust;
	double ycc_scale = 1.402;
	XieGamutTechnique gamut;
        XieLTriplet levels, hclevels, rgblevels;
        XieConstant in_low,in_high;
        XieLTriplet out_low,out_high;
	char *ditherTech;
	int retval;

	retval = 1;
	idx = 0;
	levels[ 0 ] = ( 1 << xp->vinfo.depth );
	levels[ 1 ] = ( 1 << xp->vinfo.depth );
	levels[ 2 ] = ( 1 << xp->vinfo.depth );
	colorSpace = rgb->colorspace;
	memcpy( toMatrix, rgb->toMatrix, sizeof( XieMatrix ) );
	memcpy( fromMatrix, rgb->fromMatrix, sizeof( XieMatrix ) );
	whiteAdjust = rgb->whiteAdjust;
	gamut = rgb->gamut;
	cube = floor( cbrt( ( double ) ( 1 << xp->vinfo.depth ) ) );

	whiteAdjustParm = ( char * ) NULL;
	gamutParm = ( char * ) NULL;
	convertToIndexParms = ( XieColorAllocAllParam * ) NULL;
	colorParm1 = ( char * ) NULL;
	colorParm2 = ( char * ) NULL;
	techParms = ( char * ) NULL;
	ditherTech = ( char * ) NULL;

	switch (rgb->which) {
	case RGB_FF:	flo_elements = 8; break; /* + unconstrain, hardclip */
	case RGB_IF:	flo_elements = 7; break; /* + clipscale */
	case RGB_II:	flo_elements = 6; break; /* baseline */
	}

        flograph = XieAllocatePhotofloGraph( flo_elements );
        if ( flograph == ( XiePhotoElement * ) NULL )
        {
                fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
                return( 0 );
        }
	if ( whiteAdjust )
	{
		whiteAdjustParm = (char *)
			XieTecWhiteAdjustCIELabShift (rgb->whitePoint);
	}

	switch ( colorSpace )
	{
	case xieValCIELab:
		RGBToCIELabParm = XieTecRGBToCIELab( 
			toMatrix,
			whiteAdjust,
			whiteAdjustParm );
		colorParm1 = ( char * ) RGBToCIELabParm;
		break;
	case xieValCIEXYZ:
		RGBToCIEXYZParm = XieTecRGBToCIEXYZ( 
			toMatrix,
			whiteAdjust,
			whiteAdjustParm );
		colorParm1 = ( char * ) RGBToCIEXYZParm;
		break;
	case xieValYCbCr:
		rgblevels[0] = rgb->RGBLevels[0];
		rgblevels[1] = rgb->RGBLevels[1];
		rgblevels[2] = rgb->RGBLevels[2];
		RGBToYCbCrParm = XieTecRGBToYCbCr( 
			/* rgb->RGBLevels */ rgblevels,
			(double) rgb->luma[ 0 ],
			(double) rgb->luma[ 1 ],
			(double) rgb->luma[ 2 ],
			rgb->bias ); 
		colorParm1 = ( char * )  RGBToYCbCrParm;
		break;
	case xieValYCC:
		rgblevels[0] = rgb->RGBLevels[0];
		rgblevels[1] = rgb->RGBLevels[1];
		rgblevels[2] = rgb->RGBLevels[2];
		RGBToYCCParm = XieTecRGBToYCC( 
			/* rgb->RGBLevels */ rgblevels,
			(double) rgb->luma[ 0 ],
			(double) rgb->luma[ 1 ],
			(double) rgb->luma[ 2 ],
			(double) rgb->scale); 
		colorParm1 = ( char * ) RGBToYCCParm;
		break; 
	default:
		fprintf( stderr, "Unknown colorspace\n" );	
		fflush( stderr );
		return( 0 );
	}
	colorAllocNotify = False;

	XGetWindowAttributes( xp->d, DefaultRootWindow( xp->d ), &xwa );
	XSetWindowColormap( xp->d, xp->w, xwa.colormap );
	XSync( xp->d, 0 );

	decode_notify = False;

        XieFloImportPhotomap(&flograph[idx],XIEPhotomap,decode_notify);
	idx++;
		
	if (rgb->which == RGB_FF)
	{
		XieFloUnconstrain( &flograph[idx], idx );
		idx++;
	}

	XieFloConvertFromRGB( &flograph[idx],
		idx,
		colorSpace,
		colorParm1
	);
	idx++;

	switch( colorSpace )
	{
	case xieValCIELab:
		CIELabToRGBParm = XieTecCIELabToRGB( 
			fromMatrix,
			whiteAdjust,
			whiteAdjustParm,
			gamut,
			gamutParm );
		colorParm2 = ( char * ) CIELabToRGBParm;
		break;
	case xieValCIEXYZ:
		CIEXYZToRGBParm = XieTecCIEXYZToRGB( 
			fromMatrix,
			whiteAdjust,
			whiteAdjustParm,
			gamut,
			gamutParm );
		colorParm2 = ( char * ) CIEXYZToRGBParm;
		break;
	case xieValYCbCr:
		YCbCrToRGBParm = XieTecYCbCrToRGB( 
			levels,
			(double) rgb->luma[ 0 ],
			(double) rgb->luma[ 1 ],
			(double) rgb->luma[ 2 ],
			rgb->bias,
			gamut,
			gamutParm );
		colorParm2 = ( char * ) YCbCrToRGBParm;
		break;
	case xieValYCC:
		YCCToRGBParm = XieTecYCCToRGB( 
			levels,
			(double) rgb->luma[ 0 ],
			(double) rgb->luma[ 1 ],
			(double) rgb->luma[ 2 ],
			ycc_scale,
			gamut,
			gamutParm );
		colorParm2 = ( char * ) YCCToRGBParm;
		break; 
	default:
		fprintf( stderr, "Unknown colorspace\n" );
		fflush( stderr );
		retval = 0;
		goto out;
	}

	XieFloConvertToRGB( &flograph[idx],
		idx,
		colorSpace,
		colorParm2
	);
	idx++;

	techParms = ( char * ) NULL;
	if (rgb->which == RGB_FF)
	{
		/* or perhaps clipscale with in_high == 255.0 */
		hclevels[ 0 ] = ( 1 << xp->vinfo.depth );
		hclevels[ 1 ] = ( 1 << xp->vinfo.depth );
		hclevels[ 2 ] = ( 1 << xp->vinfo.depth );
		XieFloConstrain( &flograph[idx],
			idx,
			hclevels,
			xieValConstrainHardClip,
			techParms
		);
		idx++;
	}
	else if (rgb->which == RGB_IF)
	{
		in_low[ 0 ] = in_low[ 1 ] = in_low[ 2 ] = 0.0;
		in_high[ 0 ] = in_high[ 1 ] = in_high[ 2 ] = 1.0;
		out_low[ 0 ] = out_low[ 1 ] = out_low[ 2 ] = 0;

		out_high[ 0 ] = ( 1 << xp->vinfo.depth ) - 1;
		out_high[ 1 ] = ( 1 << xp->vinfo.depth ) - 1;
		out_high[ 2 ] = ( 1 << xp->vinfo.depth ) - 1;
		techParms = (char *) ( XieClipScaleParam * ) 
			XieTecClipScale( in_low, in_high, out_low, out_high);

		hclevels[ 0 ] = ( 1 << xp->vinfo.depth );
		hclevels[ 1 ] = ( 1 << xp->vinfo.depth );
		hclevels[ 2 ] = ( 1 << xp->vinfo.depth );
		XieFloConstrain( &flograph[idx],
			idx,
			hclevels,
			xieValConstrainClipScale,
			techParms
		);
		idx++;
	}

	levels[ 0 ] = ( long ) cube;
	levels[ 1 ] = ( long ) cube;
	levels[ 2 ] = ( long ) cube;
	ditherTech = ( char * ) NULL;
	XieFloDither( &flograph[ idx ],
		idx,
		0x7,
		levels,
		xieValDitherErrorDiffusion,
		( char * ) ditherTech
	);
	idx++;

	convertToIndexParms = XieTecColorAllocAll( 123 );

        XieFloConvertToIndex(&flograph[idx],
                idx,
                xwa.colormap,
                clist,
                False,
                xieValColorAllocAll,
                (char *)convertToIndexParms
        );
        idx++;

        XieFloExportDrawable(&flograph[idx],
                idx,              /* source phototag number */
                xp->w,
                xp->fggc,
                0,       /* x offset in window */
                0        /* y offset in window */
        );
	idx++;

	flo = XieCreatePhotoflo( xp->d, flograph, flo_elements );
	XSync( xp->d, 0 );
out:
	if ( whiteAdjustParm )
		free( whiteAdjustParm );
	if ( colorParm1 )
		free( colorParm1 );
	if ( colorParm2 )
		free( colorParm2 );
	if ( techParms )
		free( techParms );
	if ( convertToIndexParms )
		free( convertToIndexParms );
	if ( ditherTech )
		free( ditherTech );
	flo_notify = False;
	return( retval );
}

void 
DoRGB(xp, p, reps)
XParms  xp;
Parms   p;
int     reps;
{
	int	i;
	int	flo_id;

	flo_id = 1;
	for ( i = 0; i < reps; i++ )
	{
                XieExecutePhotoflo(xp->d, flo, flo_notify );
        }
}

void EndRGB(xp, p)
XParms  xp;
Parms   p;
{
	InstallCustomColormap( xp->d, xp->p );
	FreeRGBStuff( xp, p );
}

int
FreeRGBStuff( xp, p )
XParms	xp;
Parms	p;
{
	if ( XIEPhotomap && IsPhotomapInCache( XIEPhotomap ) == False )
	{
		XieDestroyPhotomap(xp->d, XIEPhotomap);
		XIEPhotomap = ( XiePhotomap ) NULL;
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
