/* $XConsortium: geometry.c,v 1.3 93/07/20 20:51:09 rws Exp $ */

/**** module geometry.c ****/
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
  
	geometry.c -- geometry flo element tests 

	Syd Logan -- AGE Logic, Inc. July, 1993 - MIT Alpha release
  
*****************************************************************************/
#include "xieperf.h"
#include <stdio.h>
#include <math.h>
#ifndef M_PI
#define M_PI	3.14159265358979323846
#endif

static XiePhotomap XIEPhotomap;

static XieLTriplet levels;
static XieConstrainTechnique tech = xieValConstrainClipScale;
static XieClipScaleParam *parms;
static XieConstant in_low,in_high;
static XieLTriplet out_low,out_high;
static int constrainflag = 0;
static int drawableplaneflag = 0;

static XieDecodeG42DParam *G42Ddecode_params=NULL;
static XieDecodeG32DParam *G32Ddecode_params=NULL;
static XieDecodeG31DParam *G31Ddecode_params=NULL;
static XiePhotoElement *flograph;
static XiePhotospace photospace;
static int flo_elements;

int InitGeometry(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
	XIEimage	*image;

	image = p->finfo.image1;
	if ( !image )
		return 0;
	p->data = ( char * ) NULL;
	parms = NULL;
	constrainflag = drawableplaneflag = 0;
	if ( xp->vinfo.depth == 1 && !IsFaxImage( image->decode ) )
	{
		constrainflag = 1;
		if ( !SetupMonoClipScale( image, levels, in_low, 
			in_high, out_low, out_high, &parms ) )
		{
			reps = 0;
		}
	}	
	else if ( xp->vinfo.depth != 1 && IsFaxImage( image->decode ) 
		&& ( ( GeometryParms * ) p->ts )->geoTech == xieValGeomAntialias )
	{
		constrainflag = 1;
		if ( !SetupFaxClipScale( xp, p, levels, in_low, 
			in_high, out_low, out_high, &parms ) )
		{
			reps = 0;
		}
	}
	if ( xp->vinfo.depth != 1 && IsFaxImage( image->decode ) 
		&& ( ( GeometryParms * ) p->ts )->geoTech != xieValGeomAntialias )
		drawableplaneflag = 1;

	if ( reps )
	{
		if ( image->decode == xieValDecodeG42D )
			XIEPhotomap = GetXIEFAXPhotomap( xp, p, 1 );
		else if ( image->decode == xieValDecodeG32D )
			XIEPhotomap = GetXIEFAXPhotomap( xp, p, 1 );
		else if ( image->decode == xieValDecodeG31D )
			XIEPhotomap = GetXIEFAXPhotomap( xp, p, 1 );
		else
			XIEPhotomap = GetXIEPhotomap( xp, p, 1 );
		if ( XIEPhotomap == ( XiePhotomap ) NULL )
		{
			reps = 0;
		}
	}
	if ( !reps && parms )
		free( parms );
	return( reps );
}

void DoGeometry(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    	int     i;
	int     band_mask = 1;
	int	flo_notify, flo_id;
	int	idx;
	float	coeffs[ 6 ];
	static XieConstant constant = { 0.0, 0.0, 0.0 };
	XieGeometryTechnique	geo_tech;
	char	*geo_tech_params = NULL;

        photospace = XieCreatePhotospace(xp->d);/* XXX error check */
	flo_id = 1;
	if ( constrainflag )
		flo_elements = 4;
	else
		flo_elements = 3;
	flograph = XieAllocatePhotofloGraph(flo_elements);	
	if ( flograph == ( XiePhotoElement * ) NULL )
	{
		fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
		return;
	}

	idx = 0;
	XieFloImportPhotomap(&flograph[idx], XIEPhotomap, False); idx++;

	geo_tech = ( ( GeometryParms * ) p->ts )->geoTech;

	SetCoefficients( xp, p, ( ( GeometryParms * ) p->ts )->geoType, coeffs );

	XieFloGeometry(&flograph[idx], 
		idx,
		( ( GeometryParms * ) p->ts )->geoWidth,
		( ( GeometryParms * ) p->ts )->geoHeight,
		coeffs,
		constant,
		band_mask,
		geo_tech,
		geo_tech_params
	); idx++;

	if ( constrainflag )
	{
		XieFloConstrain(&flograph[idx],
			idx,
			levels,
			tech,
			(char *)parms
		); idx++;		
	}	

	XieFloExportDrawable(&flograph[idx],
		idx, 		/* source phototag number */
		xp->w,
		xp->fggc,
		0,       /* x offset in window */
		0        /* y offset in window */
	); idx++;

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

int InitGeometryFAX(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
	XIEimage *image;

	image = p->finfo.image1;
	if ( !image )
		return( 0 );
	G42Ddecode_params=NULL;
	G32Ddecode_params=NULL;
	G31Ddecode_params=NULL;
	parms = NULL;
	p->data = ( char * ) NULL;
	constrainflag = drawableplaneflag = 0;
	if ( xp->vinfo.depth == 1 && !IsFaxImage( image->decode ) )
	{
		constrainflag = 1;
		if ( !SetupMonoClipScale( image, levels, in_low, 
			in_high, out_low, out_high, &parms ) )
		{
			reps = 0;
		}
	}	
	else if ( xp->vinfo.depth != 1 && IsFaxImage( image->decode ) 
		&& ( ( GeometryParms * ) p->ts )->geoTech == xieValGeomAntialias )
	{
		constrainflag = 1;
		if ( !SetupFaxClipScale( xp, p, levels, in_low, 
			in_high, out_low, out_high, &parms ) )
		{
			reps = 0;
		}
	}

	if ( xp->vinfo.depth != 1 && IsFaxImage( image->decode ) 
		&& ( ( GeometryParms * ) p->ts )->geoTech != xieValGeomAntialias )
		drawableplaneflag = 1;

	if ( reps )	
        	if ( !GetImageData( xp, p, 1 ) )
			reps = 0;
		else  {
		    if ( image->decode == xieValDecodeG42D )
       			G42Ddecode_params = XieTecDecodeG42D(
                		image->fill_order,
                		True    /* XXX needs config */
			);
		    else if ( image->decode == xieValDecodeG32D )
       			G32Ddecode_params = XieTecDecodeG32D(
                		image->fill_order,
                		True    /* XXX needs config */
			);
		    else if ( image->decode == xieValDecodeG31D )
       			G31Ddecode_params = XieTecDecodeG31D(
                		image->fill_order,
                		True    /* XXX needs config */
			);
		    else {
			fprintf(stderr," bogus decode, %d, expected",
			   image->decode);
			fprintf(stderr," xieValDecodeG42D (%d) \n",
				xieValDecodeG42D);
			fprintf(stderr," or xieValDecodeG32D (%d) \n",
				xieValDecodeG32D);
			fprintf(stderr," or xieValDecodeG31D (%d) \n",
				xieValDecodeG31D);
			exit(1);	/* XXX */
		    }
		}

	if ( reps )
	{
		if ( constrainflag )
			flo_elements = 4;
		else
			flo_elements = 3;
        	photospace = XieCreatePhotospace(xp->d);
       		flograph = XieAllocatePhotofloGraph(flo_elements);
        	if ( flograph == ( XiePhotoElement * ) NULL )
        	{
			fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
			free( p->data );
			reps = 0;
		}
	} 
	if ( !reps && parms )
		free( parms );
	if ( !reps && p->data )
	{
		free( p->data );
		p->data = ( char * ) NULL;
	}
	return( reps );
}

void DoGeometryFAX(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
	int     band_mask = 1;
	int	flo_notify, flo_id;
	int	i, size, idx;
	float	coeffs[ 6 ];
	static XieConstant constant = { 0.0, 0.0, 0.0 };
        XieLTriplet width, height, mylevels;
	XieGeometryTechnique	geo_tech;
	char	*geo_tech_params = NULL;
	XIEimage *image;

	image = p->finfo.image1;
	if ( !image )
		return;

        width[ 0 ] = image->width;
        height[ 0 ] = image->height;
        mylevels[ 0 ] = image->levels;

	idx = 0;
	if ( image->decode == xieValDecodeG42D )
            XieFloImportClientPhoto(&flograph[idx],
                image->data_class,
                width, height, mylevels,
                False,
                image->decode, (char *)G42Ddecode_params
            ); 
	else if ( image->decode == xieValDecodeG32D )
            XieFloImportClientPhoto(&flograph[idx],
                image->data_class,
                width, height, mylevels,
                False,
                image->decode, (char *)G32Ddecode_params
            ); 
	else if ( image->decode == xieValDecodeG31D )
            XieFloImportClientPhoto(&flograph[idx],
                image->data_class,
                width, height, mylevels,
                False,
                image->decode, (char *)G31Ddecode_params
            ); 
	else
	{
		fprintf( stderr, "Invalid image decode\n" );
		return;
	}
	idx++;
 
	geo_tech = ( ( GeometryParms * ) p->ts )->geoTech;

	SetCoefficients( xp, p, ( ( GeometryParms * ) p->ts )->geoType, coeffs );

	if ( constrainflag )
	{
		XieFloConstrain(&flograph[idx],
			idx,
			levels,
			tech,
			(char *)parms
		); idx++;		
	}	

	XieFloGeometry(&flograph[idx], 
		idx,
		( ( GeometryParms * ) p->ts )->geoWidth,
		( ( GeometryParms * ) p->ts )->geoHeight,
		coeffs,
		constant,
		band_mask,
		geo_tech,
		geo_tech_params
	); idx++;

	if ( drawableplaneflag ) {
		XieFloExportDrawablePlane(&flograph[idx],
			idx, 		/* source phototag number */
			xp->w,
			xp->bggc,
			0,       /* x offset in window */
			0        /* y offset in window */
		); idx++;
	}
	else {
		XieFloExportDrawable(&flograph[idx],
			idx, 		/* source phototag number */
			xp->w,
			xp->bggc,
			0,       /* x offset in window */
			0        /* y offset in window */
		); idx++;
	}


	flo_notify = True;	
	flo_id = 1;
	size = image->fsize;

    	for (i = 0; i != reps; i++) {
       		XieExecuteImmediate(xp->d, photospace,
               		flo_id,		
               		flo_notify,     
               		flograph,       /* photoflo specification */
               		flo_elements    /* number of elements */
       		);
		XSync( xp->d, 0 );
	        PumpTheClientData( xp, p, flo_id, photospace, 1, p->data, size );

		WaitForFloToFinish( xp, flo_id );
		flo_id++;
    	}
}

int
SetCoefficients( xp, p, type, coeffs )
XParms	xp;
Parms	p;
int	type;
float	coeffs[];
{
	double	sf, rad;
	XIEimage *image;
	GeometryParms *gp;

	image = p->finfo.image1;
	gp = ( GeometryParms * ) p->ts;
	if ( !image || !gp )
	{
		fprintf( stderr, "SetCoefficients: invalid image or test parameters\n" );
		return;
	}

	switch( type )
	{
	case GEO_TYPE_CROP:
		coeffs[ 0 ] = 1;
		coeffs[ 1 ] = 0;
		coeffs[ 2 ] = 0;
		coeffs[ 3 ] = 1;
		coeffs[ 4 ] = gp->geoXOffset;
		coeffs[ 5 ] = gp->geoYOffset;
		break;
	case GEO_TYPE_SCALE:
		coeffs[ 0 ] = image->width / ( float )gp->geoWidth;
		coeffs[ 1 ] = 0;
		coeffs[ 2 ] = 0;
		coeffs[ 3 ] = image->height / ( float )gp->geoHeight;
		coeffs[ 4 ] = 0;
		coeffs[ 5 ] = 0;
		break;
	case GEO_TYPE_SCALEDROTATE:
		rad = M_PI * gp->geoAngle / 180.0;
		sf = ( image->width / ( float )gp->geoWidth );
		coeffs[ 0 ] = sf * cos( rad );
		coeffs[ 1 ] = sf * sin( rad );
		coeffs[ 2 ] = -sf * sin( rad );
		coeffs[ 3 ] = sf * cos( rad );
		coeffs[ 4 ] = image->width / 2.0 - sf/2 * (cos(rad)*gp->geoWidth + sin(rad) * gp->geoHeight); 
		coeffs[ 5 ] = image->height / 2.0 - sf/2 * (-sin(rad)*gp->geoWidth + cos(rad) * gp->geoHeight); 
		break;
	case GEO_TYPE_MIRRORX:
		coeffs[ 0 ] = -1;
		coeffs[ 1 ] = 0;
		coeffs[ 2 ] = 0;
		coeffs[ 3 ] = 1;
		coeffs[ 4 ] = gp->geoWidth - 1;
		coeffs[ 5 ] = 0;
		break;
	case GEO_TYPE_MIRRORY:
		coeffs[ 0 ] = 1;
		coeffs[ 1 ] = 0;
		coeffs[ 2 ] = 0;
		coeffs[ 3 ] = -1;
		coeffs[ 4 ] = 0;
		coeffs[ 5 ] = gp->geoHeight - 1;
		break;
	case GEO_TYPE_MIRRORXY:
		coeffs[ 0 ] = -1;
		coeffs[ 1 ] = 0;
		coeffs[ 2 ] = 0;
		coeffs[ 3 ] = -1;
		coeffs[ 4 ] = gp->geoWidth - 1;
		coeffs[ 5 ] = gp->geoHeight - 1;
		break;
	case GEO_TYPE_ROTATE:
		rad = M_PI * gp->geoAngle / 180.0;
		coeffs[ 0 ] = cos( rad );
		coeffs[ 1 ] = sin( rad );
		coeffs[ 2 ] = -sin( rad );
		coeffs[ 3 ] = cos( rad );
		coeffs[ 4 ] = gp->geoWidth/2.0 - 0.5 * ( cos( rad ) * image->width + sin( rad ) * image->height );
		coeffs[ 5 ] = gp->geoHeight/2.0 - 0.5 * ( -sin( rad ) * image->width + cos( rad ) * image->height );
		break;
	case GEO_TYPE_DEFAULT:
	default:
		coeffs[ 0 ] = 1;
		coeffs[ 1 ] = 0;
		coeffs[ 2 ] = 0;
		coeffs[ 3 ] = 1;
		coeffs[ 4 ] = 0;
		coeffs[ 5 ] = 0;
		break;
	}
}

int EndGeometry(xp, p)
XParms  xp;
Parms   p;
{
	if ( p->data ) 
	{
		free( p->data );
		p->data = ( char * ) NULL;
	}
	if ( parms )
		free( parms );
	XieDestroyPhotomap( xp->d, XIEPhotomap );
}

int
EndGeometryFAX(xp, p)
XParms  xp;
Parms   p;
{
	if ( p->data )
	{
		free( p->data );
		p->data = ( char * ) NULL;
	}
	if ( parms )
		free( parms );
	XieDestroyPhotospace( xp->d, photospace );
	XieFreePhotofloGraph(flograph,flo_elements);	
	if ( G42Ddecode_params )
		free( G42Ddecode_params );
	else if ( G32Ddecode_params )
		free( G32Ddecode_params );
	else if ( G31Ddecode_params )
		free( G31Ddecode_params );
}
