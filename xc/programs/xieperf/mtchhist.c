/* $XConsortium$ */

/**** module mtchhist.c ****/
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
  
	mtchhist.c -- match histogram flo element tests 

	Syd Logan -- AGE Logic, Inc. August, 1993 - MIT Beta release
  
*****************************************************************************/

#include "xieperf.h"
#include <stdio.h>

static XiePhotomap XIEPhotomap;
static XieRoi XIERoi;

static XIEimage	*image;
static int flo_notify;
static XiePhotoElement *flograph;
static XiePhotoflo flo;
static char *shape_parms;
static XieHistogramData *histos;
static int flo_elements;
static int histo1, histo2;
static XieClipScaleParam *parms;

extern Window monitorWindow;
extern Window monitor2Window;
extern Window drawableWindow; 
extern Bool dontClear;

int 
InitMatchHistogram(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
	XieProcessDomain domain;
	XieHistogramShape shape;
	double	mean, sigma;
	double	constant;
	Bool	shape_factor;	
	Bool	useROI;
	XieRectangle rect;
	int	idx;
	int	src1, src2;
	XieLTriplet levels;
	XieConstant in_low,in_high;
	XieLTriplet out_low,out_high;
	int	monoflag;

	XIEPhotomap = ( XiePhotomap ) NULL;
        flograph = ( XiePhotoElement * ) NULL;
        flo = ( XiePhotoflo ) NULL;
	XIERoi = ( XieRoi ) NULL;
	histos = ( XieHistogramData * ) NULL;
	shape_parms = ( char * ) NULL;	
	parms = ( XieClipScaleParam * ) NULL;
	monoflag = 0;

	useROI = ( ( MatchHistogramParms * )p->ts )->useROI;
	image = p->finfo.image1;
	flo_elements = 6;
	if ( !image )
		reps = 0;
	else
	{
		histos = ( XieHistogramData * ) 
			malloc( sizeof( XieHistogramData ) * 
			( 1 << image->depth[ 0 ] ) );
		if ( histos == ( XieHistogramData * ) NULL )
			reps = 0;
	}
	if ( reps )
	{
		if ( xp->vinfo.depth != 8 )
		{
			flo_elements += 2;
	                monoflag = 1;
			if ( !SetupClipScale( xp, p, image, levels, in_low,
				in_high, out_low, out_high, &parms ) )
			{
				reps = 0;
			}
		}
	}
	if ( reps )
	{
		if ( useROI == True )
		{
			flo_elements++;
			rect.x = (( MatchHistogramParms *)p->ts )->x;
			rect.y = (( MatchHistogramParms *)p->ts )->y;
			rect.width = (( MatchHistogramParms *)p->ts )->width;
			rect.height = (( MatchHistogramParms *)p->ts )->height;
			if ( ( XIERoi = GetXIERoi( xp, p, rect, 1 ) ) ==
				( XieRoi ) NULL )
			{
				reps = 0;
			}
		}
	}
	if ( reps )
	{
		shape = ( ( MatchHistogramParms * )p->ts )->shape;

		if ( shape == xieValHistogramFlat )
			shape_parms = ( char * ) NULL;
		else if ( shape == xieValHistogramGaussian )
		{
			mean = ( ( MatchHistogramParms * )p->ts )->mean;
			sigma = ( ( MatchHistogramParms * )p->ts )->sigma;
			shape_parms = 
				(char *)XieTecHistogramGaussian(mean,sigma);
			if ( !shape_parms )
			{
				fprintf( stderr, "NULL value returned by XieTecHistogramGaussian\n" );
				reps = 0;
			}
		}
		else if ( shape == xieValHistogramHyperbolic )
		{	
			constant = ( ( MatchHistogramParms * )p->ts )->constant;
			shape_factor = 
				((MatchHistogramParms *)p->ts)->shape_factor;
			shape_parms = (char *) XieTecHistogramHyperbolic(
				constant,shape_factor);
			if ( !shape_parms )
			{
				fprintf( stderr, "NULL value returned by XieTecHistogramGaussian\n" );
				reps = 0;
			}
		}
		else
		{
			fprintf( stderr, "Invalid histogram technique\n" );
			reps = 0;
		}
	}

	if ( reps )
	{
		flograph = XieAllocatePhotofloGraph(flo_elements);	
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
		else
		{
			idx = 0;

			domain.offset_x = 0;
			domain.offset_y = 0;
			domain.phototag = 0;
			if ( useROI == True )
			{
				XieFloImportROI(&flograph[idx], XIERoi);
				idx++;
				domain.phototag = idx;
			}

			XieFloImportPhotomap(&flograph[idx], 
				XIEPhotomap, False );
			idx++;
			src1 = idx;

			XieFloMatchHistogram(&flograph[idx], 
				src1,
				&domain,
				shape,
				shape_parms
			);
			idx++;
			src2 = idx;

			XieFloExportClientHistogram(&flograph[idx],
				src2,              /* source phototag number */
				&domain,
				xieValNewData
			);
			idx++;
			histo1 = idx;

			XieFloExportClientHistogram(&flograph[idx],
				src1,              /* source phototag number */
				&domain,
				xieValNewData
			);
			idx++;
			histo2 = idx;

			if ( monoflag )
			{
				XieFloConstrain(&flograph[idx],
					src1,
					levels,
					xieValConstrainClipScale,
					(char *)parms
				); idx++;
				src1 = idx;
			}

			if ( monoflag )
			{
				XieFloConstrain(&flograph[idx],
					src2,
					levels,
					xieValConstrainClipScale,
					(char *)parms
				); idx++;
				src2 = idx;
			}

			XieFloExportDrawable(&flograph[idx],
				src1,     	/* source phototag number */
				xp->w,
				xp->fggc,
				0,       /* x offset in window */
				0        /* y offset in window */
			);
			idx++;

			XieFloExportDrawable(&flograph[idx],
				src2,     	/* source phototag number */
				drawableWindow,
				xp->fggc,
				0,       /* x offset in window */
				0        /* y offset in window */
			);
			idx++;

			flo = XieCreatePhotoflo(xp->d, flograph, flo_elements);
			flo_notify = True;
		}
	}
	if ( reps )
	{
		XMoveWindow( xp->d, drawableWindow, WIDTH + 10, 0 );
		XMoveWindow( xp->d, monitorWindow, 0, HEIGHT - MONHEIGHT );
		XMoveWindow( xp->d, monitor2Window, WIDTH + 10, 
			HEIGHT - MONHEIGHT );
		XMapRaised( xp->d, drawableWindow );
		XMapRaised( xp->d, monitor2Window );
		XMapRaised( xp->d, monitorWindow );
		XSync( xp->d, 0 );
		dontClear = True;
	}
	else 
		FreeMatchHistogramStuff( xp, p );	
	return( reps );
}

void 
DoMatchHistogram(xp, p, reps)
XParms  xp;
Parms   p;
int     reps;
{
    	int     i, done, numHistos;
	unsigned int checksum;

    	for (i = 0; i != reps; i++) {
		XieExecutePhotoflo( xp->d, flo, flo_notify );
		XSync( xp->d, 0 );
		numHistos = ReadNotifyExportData( xp, p, 0, flo, histo2, 
			sizeof( XieHistogramData ), 0, &histos, &done ) 
			/ sizeof( XieHistogramData );
		DrawHistogram( xp, monitorWindow, 
			( XieHistogramData * ) histos,
			numHistos, 1<< xp->vinfo.depth ); 
		numHistos = ReadNotifyExportData( xp, p, 0, flo, histo1, 
			sizeof( XieHistogramData ), 0, &histos, &done ) 
			/ sizeof( XieHistogramData );
		DrawHistogram( xp, monitor2Window, 
			( XieHistogramData * ) histos,
			numHistos, 1 << xp->vinfo.depth ); 
		WaitForXIEEvent( xp, xieEvnNoPhotofloDone, flo, 0, False );
    	}
}

void 
EndMatchHistogram(xp, p)
XParms  xp;
Parms   p;
{
	XUnmapWindow( xp->d, monitorWindow );
	XUnmapWindow( xp->d, monitor2Window );
	XUnmapWindow( xp->d, drawableWindow );
	dontClear = False;
	FreeMatchHistogramStuff( xp, p );
}

int
FreeMatchHistogramStuff( xp, p )
XParms	xp;
Parms	p;
{
        if ( parms )
        {
                free( parms );
                parms = ( XieClipScaleParam * ) NULL;
        }
        if ( XIEPhotomap && IsPhotomapInCache( XIEPhotomap ) == False )
        {
                XieDestroyPhotomap( xp->d, XIEPhotomap );
                XIEPhotomap = ( XiePhotomap ) NULL;
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
        if ( XIERoi )
        {
                XieDestroyROI( xp->d, XIERoi );
                XIERoi = ( XieRoi ) NULL;
        }
	if ( histos )
	{
		free( histos );
		histos = ( XieHistogramData * ) NULL;
	}
	if ( shape_parms )
	{
		free( shape_parms );
		shape_parms = ( char * ) NULL;
	}
}	

