/* $XConsortium: math.c,v 1.1 93/10/26 10:08:10 rws Exp $ */

/**** module math.c ****/
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
  
	math.c -- math flo element tests 

	Syd Logan -- AGE Logic, Inc. August, 1993 - MIT Beta release
  
*****************************************************************************/
#include "xieperf.h"
#include <stdio.h>

static XiePhotomap XIEPhotomap;
static XieRoi XIERoi;

static int flo_notify;
static XiePhotoElement *flograph;
static XiePhotoflo flo;
static int flo_elements;

static XieClipScaleParam *parms;

int 
InitMath(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
	XieProcessDomain domain;
	XieArithmeticOp	*ops;
	int		nops;
	unsigned int	bandMask;
	Bool		useROI;
	Bool		constrain;
	XieConstrainTechnique constrainTech;
	int		idx;
	XieRectangle	rect;
	int		i;
        XieLTriplet levels;
        XieConstant in_low,in_high;
        XieLTriplet out_low,out_high;
	XIEimage *image;

	useROI = ( ( MathParms * )p->ts )->useROI;
	constrain = ( ( MathParms * )p->ts )->constrain;
	constrainTech = ( ( MathParms * )p->ts )->constrainTech;
	ops = ( ( MathParms * )p->ts )->ops;
	nops = ( ( MathParms * )p->ts )->nops;
	bandMask = ( ( MathParms * )p->ts )->bandMask;
	in_low[ 0 ] = ( ( MathParms * )p->ts )->inLow;
	in_high[ 0 ] = ( ( MathParms * )p->ts )->inHigh;  
	in_low[ 1 ] = in_low[ 2 ] = 0.0;
	in_high[ 1 ] = in_high[ 2 ] = 0.0;

	XIERoi = ( XieRoi ) NULL;
	XIEPhotomap = ( XiePhotomap ) NULL;
	flo = ( XiePhotoflo ) NULL;
	flograph = ( XiePhotoElement * ) NULL;
	parms = ( XieClipScaleParam * ) NULL;

	for ( i = 0; i < nops; i++ )
	{
		switch( ops[ i ] )
		{
			case xieValExp:
			case xieValLn:
			case xieValLog2:
			case xieValLog10:
			case xieValSquare:
			case xieValSqrt:
				break;
			default:
				fprintf( stderr, "Invalid math op\n" );
				reps = 0;
				break;
		}
	}

	if ( !reps )
		return( reps );	

	image = p->finfo.image1;

	if ( !image )
		return ( 0 );

	if ( ( constrain == False && xp->vinfo.depth != image->depth[ 0 ] ) || 
	     ( constrain == True && 
	       constrainTech == xieValConstrainHardClip && 
	       xp->vinfo.depth != image->depth[ 0 ] ) )
	{
		in_low[ 0 ] = 0.0;
		in_high[ 0 ] = ( 1 << image->depth[ 0 ] ) - 1.0;  
		constrainTech = xieValConstrainClipScale;
		constrain = True;
	}

	flo_elements = 2 + nops;
	if ( useROI == True )
		flo_elements++;
	if ( constrain == True )
		flo_elements += 2;

	flograph = XieAllocatePhotofloGraph( flo_elements );	
	if ( flograph == ( XiePhotoElement * ) NULL )
	{
		fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
		reps = 0;
	}
	else if ( ( XIEPhotomap = 
		GetXIEPhotomap( xp, p, 1 ) ) == ( XiePhotomap ) NULL )
	{
		reps = 0;
	}
	else if ( useROI == True )
	{
		rect.x = ( ( MathParms * )p->ts )->x;
		rect.y = ( ( MathParms * )p->ts )->y;
		rect.width = ( ( MathParms * )p->ts )->width;
		rect.height = ( ( MathParms * )p->ts )->height;

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

		XieFloImportPhotomap(&flograph[idx], XIEPhotomap, False );
		idx++;

		if ( constrain == True )
		{
			XieFloUnconstrain( &flograph[idx], idx );
			idx++;
		}

		for ( i = 0; i < nops; i++ )
		{ 
			XieFloMath(&flograph[idx], 
				idx,
				&domain,
				ops[ i ],
				bandMask
			);
			idx++;
		}

		if ( constrain == True )
		{
		        levels[ 0 ] = 1 << xp->vinfo.depth;
			levels[ 1 ] = 0;
			levels[ 2 ] = 0;

/* XXX */		if ( constrainTech == xieValConstrainHardClip )
				parms = ( XieClipScaleParam * ) NULL;
			else if ( constrainTech == xieValConstrainClipScale )
			{
				out_low[ 0 ] = 0;
				out_high[ 0 ] = ( 1 << xp->vinfo.depth ) - 1;

				parms = ( XieClipScaleParam * ) 
					XieTecClipScale( in_low, 
					in_high, out_low, out_high);
				if ( parms == ( XieClipScaleParam * ) NULL )
				{
	                                fprintf( stderr, "Trouble loading clipscale technique parameters\n" );
					reps = 0;
				}
			} 
			else
			{
				fprintf( stderr, 
					"Invalid technique for constrain\n" );
				reps = 0;
			}	
			if ( reps )
			{
				XieFloConstrain( &flograph[idx],
					idx,
					levels,
					constrainTech,
					( char * ) parms
				);
				idx++;
			}
		}
		
		if ( reps )	
		{
			XieFloExportDrawable(&flograph[idx],
				idx,     	/* source phototag number */
				xp->w,
				xp->fggc,
				0,       /* x offset in window */
				0        /* y offset in window */
			);
			idx++;
	
			flo = XieCreatePhotoflo( xp->d, flograph, 
				flo_elements );
			flo_notify = True;
		}
	}
	if ( !reps )
		FreeMathStuff( xp, p );
	return( reps );
}

void 
DoMath(xp, p, reps)
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
EndMath(xp, p)
XParms  xp;
Parms   p;
{
	FreeMathStuff( xp, p );
}

int
FreeMathStuff( xp, p )
XParms	xp;
Parms	p;
{
	if ( parms )
	{
		free( parms );
		parms = ( XieClipScaleParam * ) NULL;
	}
	if ( XIERoi )
	{
                XieDestroyROI( xp->d, XIERoi );
                XIERoi = ( XieRoi ) NULL;
	}
	if ( XIEPhotomap && IsPhotomapInCache( XIEPhotomap ) == False )
	{
		XieDestroyPhotomap(xp->d, XIEPhotomap);
		XIEPhotomap = ( XiePhotomap ) NULL;
	}
	if ( flo )
	{
		XieDestroyPhotoflo( xp->d, flo );
		flo = ( XiePhotoflo ) NULL;
	}
	if ( flograph )
	{
		XieFreePhotofloGraph(flograph,flo_elements);	
                flograph = ( XiePhotoElement * ) NULL;
	}
}
