/* $XConsortium: geometry.c,v 1.2 93/07/19 14:43:55 rws Exp $ */

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

static XieDecodeG42DParam *G42Ddecode_params=NULL;
static XiePhotoElement *flograph;
static XiePhotospace photospace;
static int flo_elements;

int InitGeometry(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
	if ( xp->vinfo.depth == 1 && !IsFaxImage( p->decode ) )
	{
		constrainflag = 1;
		if ( !SetupMonoClipScale( xp, p, levels, in_low, 
			in_high, out_low, out_high, &parms ) )
		{
			reps = 0;
		}
	}	
	else if ( xp->vinfo.depth != 1 && IsFaxImage( p->decode ) )
	{
		constrainflag = 1;
		if ( !SetupFaxClipScale( xp, p, levels, in_low, 
			in_high, out_low, out_high, &parms ) )
		{
			reps = 0;
		}
	}

	if ( reps )
	{
		if ( p->decode == xieValDecodeG42D )
			XIEPhotomap = GetXIEFAXPhotomap( xp, p, 1 );
		else
			XIEPhotomap = GetXIEPhotomap( xp, p, 1 );
		if ( XIEPhotomap == ( XiePhotomap ) NULL )
		{
			reps = 0;
		}
	}
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

	geo_tech = p->geo.geoTech;

	SetCoefficients( xp, p, p->geo.geoType, coeffs );

	XieFloGeometry(&flograph[idx], 
		idx,
		p->geo.geoWidth,
		p->geo.geoHeight,
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
		p->dst_x,       /* x offset in window */
		p->dst_y        /* y offset in window */
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
	if ( xp->vinfo.depth == 1 && !IsFaxImage( p->decode ) )
	{
		constrainflag = 1;
		if ( !SetupMonoClipScale( xp, p, levels, in_low, 
			in_high, out_low, out_high, &parms ) )
		{
			reps = 0;
		}
	}	
	else if ( xp->vinfo.depth != 1 && IsFaxImage( p->decode ) )
	{
		constrainflag = 1;
		if ( !SetupFaxClipScale( xp, p, levels, in_low, 
			in_high, out_low, out_high, &parms ) )
		{
			reps = 0;
		}
	}

	if ( reps )	
        	if ( !GetImageData( xp, p, 1 ) )
			reps = 0;
		else
       			G42Ddecode_params = XieTecDecodeG42D(
                		xieValLSFirst, /* XXX needs config */
                		True    /* XXX needs config */
			);

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
#if 0
	if ( reps )
	{
		if ( p->decode == xieValDecodeG42D )
			XIEPhotomap = GetXIEFAXPhotomap( xp, p, 1 );
		else
			XIEPhotomap = GetXIEPhotomap( xp, p, 1 );
		if ( XIEPhotomap == ( XiePhotomap ) NULL )
		{
			reps = 0;
		}
	}
#endif
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

        width[ 0 ] = p->width;
        height[ 0 ] = p->height;
        mylevels[ 0 ] = p->levels;

	idx = 0;
        XieFloImportClientPhoto(&flograph[idx],
                p->class,
                width, height, mylevels,
                False,
                p->decode, (char *)G42Ddecode_params
        ); idx++;
 
	geo_tech = p->geo.geoTech;

	SetCoefficients( xp, p, p->geo.geoType, coeffs );


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
		p->geo.geoWidth,
		p->geo.geoHeight,
		coeffs,
		constant,
		band_mask,
		geo_tech,
		geo_tech_params
	); idx++;

	XieFloExportDrawable(&flograph[idx],
		idx, 		/* source phototag number */
		xp->w,
		xp->fggc,
		p->dst_x,       /* x offset in window */
		p->dst_y        /* y offset in window */
	); idx++;

	flo_notify = True;	
	flo_id = 1;
	size = p->finfo.fsize1;

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

	switch( type )
	{
	case GEO_TYPE_CROP:
		coeffs[ 0 ] = 1;
		coeffs[ 1 ] = 0;
		coeffs[ 2 ] = 0;
		coeffs[ 3 ] = 1;
		coeffs[ 4 ] = p->geo.geoXOffset;
		coeffs[ 5 ] = p->geo.geoYOffset;
		break;
	case GEO_TYPE_SCALE:
		coeffs[ 0 ] = p->width / ( float )p->geo.geoWidth;
		coeffs[ 1 ] = 0;
		coeffs[ 2 ] = 0;
		coeffs[ 3 ] = p->height / ( float )p->geo.geoHeight;
		coeffs[ 4 ] = 0;
		coeffs[ 5 ] = 0;
		break;
	case GEO_TYPE_SCALEDROTATE:
		rad = M_PI * p->geo.geoAngle / 180.0;
		sf = ( p->width / ( float )p->geo.geoWidth );
		coeffs[ 0 ] = sf * cos( rad );
		coeffs[ 1 ] = sf * sin( rad );
		coeffs[ 2 ] = -sf * sin( rad );
		coeffs[ 3 ] = sf * cos( rad );
		coeffs[ 4 ] = p->width / 2.0 - sf/2 * (cos(rad)*p->geo.geoWidth + sin(rad) * p->geo.geoHeight); 
		coeffs[ 5 ] = p->height / 2.0 - sf/2 * (-sin(rad)*p->geo.geoWidth + cos(rad) * p->geo.geoHeight); 
#if 0
fprintf( stderr, "iw %d ih %d w %d h %d \n", p->width, p->height, p->geo.geoWidth, p->geo.geoWidth );
fflush( stderr );
fprintf( stderr, "rad %f sf %f 0 %f 1 %f 2 %f 3 %f 4 %f 5 %f\n",
	rad, sf, coeffs[ 0 ], coeffs[ 1 ], coeffs[ 2 ], coeffs[ 3 ], coeffs[ 4 ], coeffs[ 5 ] );
fflush( stderr );
#endif
		break;
	case GEO_TYPE_MIRRORX:
		coeffs[ 0 ] = -1;
		coeffs[ 1 ] = 0;
		coeffs[ 2 ] = 0;
		coeffs[ 3 ] = 1;
		coeffs[ 4 ] = p->geo.geoWidth - 1;
		coeffs[ 5 ] = 0;
		break;
	case GEO_TYPE_MIRRORY:
		coeffs[ 0 ] = 1;
		coeffs[ 1 ] = 0;
		coeffs[ 2 ] = 0;
		coeffs[ 3 ] = -1;
		coeffs[ 4 ] = 0;
		coeffs[ 5 ] = p->geo.geoHeight - 1;
		break;
	case GEO_TYPE_MIRRORXY:
		coeffs[ 0 ] = -1;
		coeffs[ 1 ] = 0;
		coeffs[ 2 ] = 0;
		coeffs[ 3 ] = -1;
		coeffs[ 4 ] = p->geo.geoWidth - 1;
		coeffs[ 5 ] = p->geo.geoHeight - 1;
		break;
	case GEO_TYPE_ROTATE:
		rad = M_PI * p->geo.geoAngle / 180.0;
		coeffs[ 0 ] = cos( rad );
		coeffs[ 1 ] = sin( rad );
		coeffs[ 2 ] = -sin( rad );
		coeffs[ 3 ] = cos( rad );
		coeffs[ 4 ] = p->geo.geoWidth/2.0 - 0.5 * ( cos( rad ) * p->width + sin( rad ) * p->height );
		coeffs[ 5 ] = p->geo.geoHeight/2.0 - 0.5 * ( -sin( rad ) * p->width + cos( rad ) * p->height );
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
	free( p->data );
	XieDestroyPhotomap( xp->d, XIEPhotomap );
}

int
EndGeometryFAX(xp, p)
XParms  xp;
Parms   p;
{
	free( p->data );
	XieDestroyPhotospace( xp->d, photospace );
	XieFreePhotofloGraph(flograph,flo_elements);	
}
