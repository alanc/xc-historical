/* $XConsortium: arith.c,v 1.1 93/10/26 10:07:51 rws Exp $ */

/**** module arith.c ****/
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
  
	arith.c -- arithmetic flo element tests 

	Syd Logan -- AGE Logic, Inc. August, 1993 - MIT Beta release
  
*****************************************************************************/
#include "xieperf.h"
#include <stdio.h>

static XiePhotomap XIEPhotomap1;
static XiePhotomap XIEPhotomap2;
static XieRoi XIERoi;

static XIEimage	*image1, *image2;
static int flo_notify;
static XiePhotoElement *flograph;
static XiePhotoflo flo;
static int flo_elements;
static int monoflag;

static XieClipScaleParam *parms;

int 
InitArithmetic(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
	XieProcessDomain domain;
	XieArithmeticOp	op;
	XieConstant	constant;
	unsigned int	bandMask;
	Bool		useROI, constrain;
	int		idx;
	int		src1, src2;
	XieRectangle	rect;
        XieConstrainTechnique constrainTech;
        XieLTriplet levels;
        XieConstant in_low,in_high;
        XieLTriplet out_low,out_high;

	monoflag = 0;
	useROI = ( ( ArithmeticParms * )p->ts )->useROI;
	op = ( ( ArithmeticParms * )p->ts )->op;
	constant[ 0 ] = ( ( ArithmeticParms * )p->ts )->constant[ 0 ];
	constant[ 1 ] = ( ( ArithmeticParms * )p->ts )->constant[ 1 ];
	constant[ 2 ] = ( ( ArithmeticParms * )p->ts )->constant[ 2 ];
	bandMask = ( ( ArithmeticParms * )p->ts )->bandMask;
	constrain = ( ( ArithmeticParms * )p->ts )->constrain;
	constrainTech = ( ( ArithmeticParms * )p->ts )->constrainTech;
        in_low[ 0 ] = ( ( ArithmeticParms * )p->ts )->inLow;
	in_low[ 1 ] = 0;
	in_low[ 2 ] = 0;
        in_high[ 0 ] = ( ( ArithmeticParms * )p->ts )->inHigh;
	in_high[ 1 ] = 0;
	in_high[ 2 ] = 0;

	XIERoi = ( XieRoi ) NULL;
	XIEPhotomap1 = ( XiePhotomap ) NULL;
	XIEPhotomap2 = ( XiePhotomap ) NULL;
	flo = ( XiePhotoflo ) NULL;
	flograph = ( XiePhotoElement * ) NULL;
        parms = ( XieClipScaleParam * ) NULL;

	switch( op )
	{
		case xieValAdd:
		case xieValSub:
		case xieValSubRev:
		case xieValMul:
		case xieValDiv:
		case xieValDivRev:
		case xieValMin:
		case xieValMax:
		case xieValGamma:
			break;
		default:
			fprintf( stderr, "Invalid arithmetic op\n" );
			reps = 0;
			break;
	}

	if ( !reps )
		return( reps );	

	image1 = p->finfo.image1;

	/* we must have image1. image2, however, is optional */
 
	if ( !image1 )
		return ( 0 );
	image2 = p->finfo.image2;

	if ( image2 )
		flo_elements = 4;
	else
		flo_elements = 3;
	if ( useROI == True )
		flo_elements++;
	if ( constrain == True )
		if ( image2 )
			flo_elements += 3;
		else
			flo_elements += 2;
	if ( xp->vinfo.depth != 8 && constrain == False )
	{
		monoflag = 1;
		flo_elements++;
		if ( SetupClipScale( xp, p, image1, levels, in_low,
			in_high, out_low, out_high, &parms ) == 0 )
		{
			reps = 0;
		}
	}

	if ( reps )
	{
		flograph = XieAllocatePhotofloGraph( flo_elements );	
		if ( flograph == ( XiePhotoElement * ) NULL )
		{
			fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
			reps = 0;
		}
		else if ( ( XIEPhotomap1 = GetXIEPhotomap( xp, p, 1 ) ) == 
			( XiePhotomap ) NULL )
		{
			reps = 0;
		}
		else if ( image2 ) 
		{
			if ( ( XIEPhotomap2 = GetXIEPhotomap( xp, p, 2 ) ) == 
				( XiePhotomap ) NULL )
			{
				reps = 0;
			}
		}

		if ( useROI == True )
		{
			rect.x = ( ( ArithmeticParms * )p->ts )->x;
			rect.y = ( ( ArithmeticParms * )p->ts )->y;
			rect.width = ( ( ArithmeticParms * )p->ts )->width;
			rect.height = ( ( ArithmeticParms * )p->ts )->height;

			if ( ( XIERoi = GetXIERoi( xp, p, &rect, 1 ) ) == 
				( XieRoi ) NULL )
			{
				reps = 0;
			}
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

		XieFloImportPhotomap(&flograph[idx], XIEPhotomap1, False );
		idx++;
		src1 = idx;

                if ( constrain == True )
                {
                        XieFloUnconstrain( &flograph[idx], idx );
                        idx++;
			src1 = idx;
                }

		if ( image2 )
		{
			XieFloImportPhotomap(&flograph[idx], XIEPhotomap2, 
				False );
			idx++;
			src2 = idx;

			if ( constrain == True )
			{
				XieFloUnconstrain( &flograph[idx], idx );
				idx++;
				src2 = idx;
			}
		}
		else
			src2 = 0;

		XieFloArithmetic(&flograph[idx], 
			src1,
			src2,
			&domain,
			constant,
			op,
			bandMask
		);
		idx++;

                if ( constrain == True )
                {
                        levels[ 0 ] = 1 << xp->vinfo.depth;
                        levels[ 1 ] = 0;
                        levels[ 2 ] = 0;

                        if ( constrainTech == xieValConstrainHardClip )
                                parms = ( XieClipScaleParam * ) NULL;
                        else if ( constrainTech == xieValConstrainClipScale )
                        {
                                out_low[ 0 ] = 0;
                                out_low[ 1 ] = 0;
                                out_low[ 2 ] = 0;
                                out_high[ 0 ] = ( 1 << xp->vinfo.depth ) - 1;
                                out_high[ 1 ] = ( 1 << xp->vinfo.depth ) - 1;
                                out_high[ 2 ] = ( 1 << xp->vinfo.depth ) - 1;

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
	
		if ( reps && monoflag )
		{
			XieFloConstrain( &flograph[ idx ],
				idx,
				levels,
				xieValConstrainClipScale,
				( char * ) parms
			);
			idx++;
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
		
			flo = XieCreatePhotoflo( xp->d, flograph, flo_elements );
			flo_notify = True;
		}
	}
	if ( !reps )
		FreeArithStuff( xp, p );
		
	return( reps );
}

void 
DoArithmetic(xp, p, reps)
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
EndArithmetic(xp, p)
XParms  xp;
Parms   p;
{
	FreeArithStuff( xp, p );
}

int
FreeArithStuff( xp, p )
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

	if ( XIEPhotomap1 && IsPhotomapInCache( XIEPhotomap1 ) == False )
	{
                XieDestroyPhotomap( xp->d, XIEPhotomap1 );
                XIEPhotomap1 = ( XiePhotomap ) NULL;
	}

	if ( XIEPhotomap2 && IsPhotomapInCache( XIEPhotomap2 ) == False )
	{
                XieDestroyPhotomap( xp->d, XIEPhotomap2 );
                XIEPhotomap2 = ( XiePhotomap ) NULL;
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
