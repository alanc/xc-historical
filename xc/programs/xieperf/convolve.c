/* $XConsortium$ */

/**** module convolve.c ****/
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
  
	convolve.c -- convolve flo element tests 

	Syd Logan -- AGE Logic, Inc. August, 1993 - MIT Beta release
  
*****************************************************************************/
#include "xieperf.h"
#include <stdio.h>

static XiePhotomap XIEPhotomap;
static XiePhotomap XIEPhotomap2;
static XieRoi XIERoi;

static XIEimage	*image;
static int flo_notify;
static XiePhotoElement *flograph;
static XiePhotoflo flo;
static int flo_elements;
static float *data;
static XieClipScaleParam *parms;

#define	DATA( n, i, j ) ( float * ) ( *data + ( i * n ) ) + j

int 
InitConvolve(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
	XieProcessDomain domain;
	unsigned int bandMask;
	Bool useROI;
	int idx, n, src;
	XieRectangle rect;
	XieConvolveTechnique tech;
	char *tech_parm;
	XieConstant constant;
	int ( * kfunc )();
        XieLTriplet levels;
        XieConstant in_low,in_high;
        XieLTriplet out_low,out_high;
	int monoflag = 0;
	Bool photoDest;
        XieEncodeTechnique encode_tech=xieValEncodeServerChoice;
        char *encode_params=NULL;


	photoDest = ( ( ConvolveParms * )p->ts )->photoDest;
	useROI = ( ( ConvolveParms * )p->ts )->useROI;
	tech = ( ( ConvolveParms * )p->ts )->tech;
	bandMask = ( ( ConvolveParms * )p->ts )->bandMask;
	kfunc = ( ( ConvolveParms * )p->ts )->kfunc;
	constant[ 0 ] = ( ( ConvolveParms * )p->ts )->constant[ 0 ]; 
	constant[ 1 ] = ( ( ConvolveParms * )p->ts )->constant[ 1 ]; 
	constant[ 2 ] = ( ( ConvolveParms * )p->ts )->constant[ 2 ]; 

	if ( useROI == True )
		flo_elements = 4;
	else
		flo_elements = 3; 

	data = ( float * ) NULL;
        XIEPhotomap = ( XiePhotomap ) NULL;
        XIEPhotomap2 = ( XiePhotomap ) NULL;
        XIERoi = ( XieRoi ) NULL;
        flograph = ( XiePhotoElement * ) NULL;
        flo = ( XiePhotoflo ) NULL;
	parms = ( XieClipScaleParam * ) NULL;

	if ( !kfunc )
		return( 0 );
	else if ( ( n = kfunc( &data ) ) < 3 )
	{
		fprintf( stderr, "Error getting kernel\n" );
		reps = 0;
	}

	if ( reps )
	{
		image = p->finfo.image1;
		if ( !image )
		{
			reps = 0;
		}
	}

	if ( reps )
	{
		if ( xp->vinfo.depth != 8 && photoDest == False )
		{
			monoflag = 1;
			flo_elements++;
		        if ( SetupClipScale( xp, p, image, levels, in_low,
                        	in_high, out_low, out_high, &parms ) == 0 )
			{
				reps = 0;
			}
		}
	}
	if ( reps )
	{
		flograph = XieAllocatePhotofloGraph( flo_elements );	
		if ( flograph == ( XiePhotoElement * ) NULL )
		{
			reps = 0;
		}
		else if ( ( XIEPhotomap = 
			GetXIEPhotomap( xp, p, 1 ) ) == ( XiePhotomap ) NULL )
		{
			reps = 0;
		}
	}
	if ( reps )
	{
		XIEPhotomap2 = XieCreatePhotomap( xp->d );
		if ( useROI == True )
		{
			rect.x = ( ( ConvolveParms * )p->ts )->x;
			rect.y = ( ( ConvolveParms * )p->ts )->y;
			rect.width = ( ( ConvolveParms * )p->ts )->width;
			rect.height = ( ( ConvolveParms * )p->ts )->height;

			if ( ( XIERoi = GetXIERoi( xp, p, rect, 1 ) ) == 
				( XieRoi ) NULL )
			{
				reps = 0;
			}
		}
	}

	if ( reps )
	{
		switch ( tech )
		{
		case xieValConvolveDefault:	
			tech_parm = ( char * ) NULL;
			break;
		case xieValConvolveConstant:	
			tech_parm = ( char * ) 
				XieTecConvolveConstant( constant );
			if ( tech_parm == ( char * ) NULL )
			{
				fprintf( stderr, 
					"XieTecConvolveConstant failed\n" );
				reps = 0;	
			}
			break;
		case xieValConvolveReplicate:	/* XXX Not supported in SI */
		default:
			fprintf( stderr, "Invalid convolve technique\n" );
			reps = 0;
			break;
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

		XieFloImportPhotomap(&flograph[idx], XIEPhotomap, False );
		idx++;
		src = idx;

		XieFloConvolve(&flograph[idx], 
			src,
			&domain,
			data,
			n,
			bandMask,
			tech,
			tech_parm
		);
		idx++;

		if ( photoDest == False )
		{	
			if ( monoflag )
			{
				XieFloConstrain( &flograph[ idx ],
					idx,
					levels,
					xieValConstrainClipScale,
					( char * ) parms
				);
				idx++;
			}
			
			XieFloExportDrawable(&flograph[idx],
				idx,     	/* source phototag number */
				xp->w,
				xp->fggc,
				0,       /* x offset in window */
				0        /* y offset in window */
			);
			idx++;
		}
		else
		{
                        XieFloExportPhotomap(&flograph[idx],
                                idx,              /* source phototag number */
                                XIEPhotomap2,
                                encode_tech,
                                encode_params
                        );
			idx++;
		}
		
		flo = XieCreatePhotoflo( xp->d, flograph, flo_elements );
		flo_notify = True;
	}
	if ( tech_parm )
		free( tech_parm );
	if ( !reps )
		FreeConvolveStuff( xp, p );
	return( reps );
}

void 
DoConvolve(xp, p, reps)
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
EndConvolve(xp, p)
XParms  xp;
Parms   p;
{
	FreeConvolveStuff( xp, p );
}

int
FreeConvolveStuff( xp, p )
XParms	xp;
Parms	p;
{
        if ( parms )
        {
                free( parms );
                parms = ( XieClipScaleParam * ) NULL;
        }

	if ( data )
	{
		free( data );
		data = ( float * ) NULL;
	}

        if ( XIERoi )
        {
                XieDestroyROI( xp->d, XIERoi );
                XIERoi = ( XieRoi ) NULL;
        }

        if ( XIEPhotomap != ( XiePhotomap ) NULL && IsPhotomapInCache( XIEPhotomap ) == False )
        {
                XieDestroyPhotomap( xp->d, XIEPhotomap );
                XIEPhotomap = ( XiePhotomap ) NULL;
        }

        if ( XIEPhotomap2 != ( XiePhotomap ) NULL ) 
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

/* convolve kernel functions. add as many as you like. all must correspond
   to the following prototype:

	int	foo( float **data );

	The return value is the dimension of the kernel, which must be odd, 
	and greater than 3 in value. The kernel function must malloc an 
	area which is sizeof( float ) * dimension * dimension in size.
	The caller is responsible for freeing the data after use. And it
  	will do so therefore dynamically allocate the matrix using malloc,
	please.

	The kernel function is responsible for initializing the kernel.

	See the declaration of ConvolveParmVec[] in tests.c.

	A convention for naming the function is to concatenate the name
	of the transformation represented by the kernel data with the 
	dimension of the kernel. For example, if the kernel is LaPlacian,
	and the matrix size is 5x5, then the function would be named 
	LaPlacian5().
*/

int	Boxcar3( data )
float	**data;
{
	int	i, j;

	if ( ( *data = ( float * ) malloc( sizeof( float ) * 9 ) ) == 
		( float * ) NULL )
	{
		return( -1 );
	}
	for ( i = 0; i < 3; i++ )
	{
		for ( j = 0; j < 3; j++ )
			*(DATA( 3, i, j )) = 0.11111111; 
	}
	return( 3 );
}

int	Boxcar5( data )
float	**data;
{
	int	i, j;

	if ( ( *data = ( float * ) malloc( sizeof( float ) * 25 ) ) == 
		( float * ) NULL )
	{
		return( -1 );
	}
	for ( i = 0; i < 5; i++ )
	{
		for ( j = 0; j < 5; j++ )
			*(DATA( 5, i, j )) = 0.04; 
	}
	return( 5 );
}

int	LaPlacian3( data )
float	**data;
{
	int	i, j;

	if ( ( *data = ( float * ) malloc( sizeof( float ) * 3 * 3 ) ) == 
		( float * ) NULL )
	{
		return( -1 );
	}
	for ( i = 0; i < 3; i++ )
	{
		for ( j = 0; j < 3; j++ )
			*(DATA( 3, i, j )) = -1.0; 
	}
	*(DATA( 3, 2, 2 )) = 8.0;
	return( 3 );
}

int	LaPlacian5( data )
float	**data;
{
	int	i, j;

	if ( ( *data = ( float * ) malloc( sizeof( float ) * 5 * 5 ) ) == 
		( float * ) NULL )
	{
		return( -1 );
	}
	for ( i = 0; i < 5; i++ )
	{
		for ( j = 0; j < 5; j++ )
			*(DATA( 5, i, j )) = -1.0; 
	}
	*(DATA( 5, 4, 4 )) = 24.0;
	return( 5 );
}


