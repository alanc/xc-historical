/* $XConsortium: blend.c,v 1.2 93/07/19 14:43:32 rws Exp $ */

/**** module do_blend.c ****/
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
  
	do_blend.c -- blend flo element tests 

	Syd Logan -- AGE Logic, Inc. July, 1993 - MIT Alpha release
  
*****************************************************************************/
#include "xieperf.h"
#include <stdio.h>

static XiePhotomap XIEPhotomap;
static XieRoi XIERoi;
static XiePhotomap src1, src2;
static XiePhotomap alpha;

static XieLTriplet levels;
static XieConstrainTechnique tech = xieValConstrainClipScale;
static XieClipScaleParam *parms;
static XieConstant in_low,in_high;
static XieLTriplet out_low,out_high;
static int monoflag = 0;

int InitBlendMonadic(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
	XIEimage *image;

	monoflag = 0;
	p->data = ( char * ) NULL;
	image = p->finfo.image1;
	if ( !image )
		return( 0 );
	alpha = ( XiePhotomap ) NULL; 
	parms = NULL;
	if ( xp->vinfo.depth == 1 )
	{
		monoflag = 1;
		if ( !SetupMonoClipScale( image, levels, in_low, in_high, 
			out_low, out_high, &parms ) )
		{
			reps = 0;
		}
	}	

	if ( reps )
	{
		if ( ( XIEPhotomap = GetXIEPhotomap( xp, p, 1 ) ) == ( XiePhotomap ) NULL )
			reps = 0;
	}
	if ( p->data )
	{
		free( p->data );
		p->data = ( char * ) NULL;
	}
	if ( reps && p->finfo.image3 != ( XIEimage * ) NULL )
	{
		/* alpha plane */

		if ( ( alpha = GetXIEPhotomap( xp, p, 3 ) ) == ( XiePhotomap ) NULL )
		{
			XieDestroyPhotomap( xp->d, XIEPhotomap );
			reps = 0;
		}
	}
	if (p->data != NULL)
	{
		free( p->data );
		p->data = ( char * ) NULL;
	}
	if ( !reps )
		free( parms );
	return( reps );
}

int InitBlendDyadic(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
	XIEimage *image;

	monoflag = 0;
	p->data = ( char * ) NULL;
	image = p->finfo.image1;
	if ( !image )
		return( 0 );
	alpha = ( XiePhotomap ) NULL; 
	parms = NULL;
        if ( xp->vinfo.depth == 1 )
        {
		monoflag = 1;
		if ( SetupMonoClipScale( image, levels, in_low, in_high, 
			out_low, out_high, &parms ) == 0 )
		{
			return( 0 );
		}
        }

        if ( ( src1 = GetXIEPhotomap( xp, p, 1 ) ) == ( XiePhotomap ) NULL )
                reps = 0;
        else
        {
		free( p->data );
		p->data = ( char * ) NULL;
                if ( ( src2 = GetXIEPhotomap( xp, p, 2 ) ) == ( XiePhotomap ) NULL )
		{
			XieDestroyPhotomap( xp->d, src1 );
                        reps = 0;
		}
        }
	if ( p->data )
	{
		free( p->data );
		p->data = ( char * ) NULL;
	}
        if ( reps && p->finfo.image3 != ( XIEimage * ) NULL )
        {
                /* alpha plane */

                if ( ( alpha = GetXIEPhotomap( xp, p, 3 ) ) == ( XiePhotomap ) NULL )
                {
                        XieDestroyPhotomap( xp->d, src1 );
                        XieDestroyPhotomap( xp->d, src2 );
                        reps = 0;
                }
		if ( p->data )
		{
			free( p->data );
			p->data = ( char * ) NULL;
		}
        }
	if ( !reps )
		free( parms );
        return( reps );
}

int InitROIBlendMonadic(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
	XIEimage *image;
        XieRectangle *rects;
        int     rectsSize, i;

	image = p->finfo.image1;
	if ( !image )
		return( 0 );
	alpha = ( XiePhotomap ) NULL; 
	monoflag = 0;
	parms = NULL;
	p->data = ( char * ) NULL;
        if ( xp->vinfo.depth == 1 )
        {
		monoflag = 1;
		if ( SetupMonoClipScale( image, levels, in_low, in_high, 
			out_low, out_high, &parms ) == 0 )
		{
			return( 0 );
		}
        }
        rectsSize = 5;
        rects = (XieRectangle *)malloc( rectsSize * sizeof( XieRectangle ) );
        if ( rects == ( XieRectangle * ) NULL )
                reps = 0;
        else
        {
                for ( i = 0; i < rectsSize; i++ )
                {
                        rects[ i ].x = i * 100;
                        rects[ i ].y = i * 100;
                        rects[ i ].width = 100;
                        rects[ i ].height = 100;
                }
        }
        if ( ( XIERoi = GetXIERoi( xp, p, rects, rectsSize ) ) == ( XieRoi ) NULL )
        {
                reps = 0;
        }
        if ( rects )
                free( rects );
        if ( ( XIEPhotomap = GetXIEPhotomap( xp, p, 1 ) ) == ( XiePhotomap ) NULL )
	{
		XieDestroyROI( xp->d, XIERoi );
                reps = 0;
	}
	if ( p->data )
	{
		free( p->data );
		p->data = ( char * ) NULL;
	}
        if ( reps && p->finfo.image3 != ( XIEimage * ) NULL )
        {
                /* alpha plane */

                if ( ( alpha = GetXIEPhotomap( xp, p, 3 ) ) == ( XiePhotomap ) NULL )
                {
                        XieDestroyPhotomap( xp->d, XIEPhotomap );
			XieDestroyROI( xp->d, XIERoi );
                        reps = 0;
                }
		if ( p->data )
		{
			free( p->data );
			p->data = ( char * ) NULL;
		}
        }
        return( reps );
}

int InitROIBlendDyadic(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
	XIEimage *image;
        XieRectangle *rects;
        int     rectsSize, i;

	image = p->finfo.image1;
        if ( !image )
                return( 0 );
	alpha = ( XiePhotomap ) NULL; 
	monoflag = 0;
	parms = NULL;
	p->data = ( char * ) NULL;
        if ( xp->vinfo.depth == 1 )
        {
		monoflag = 1;
		if ( SetupMonoClipScale( image, levels, in_low, in_high, 
			out_low, out_high, &parms ) == 0 )
		{
			return( 0 );
		}
        }
        rectsSize = 5;
        rects = (XieRectangle *)malloc( rectsSize * sizeof( XieRectangle ) );
        if ( rects == ( XieRectangle * ) NULL )
                reps = 0;
        else
        {
                for ( i = 0; i < rectsSize; i++ )
                {
                        rects[ i ].x = i * 100;
                        rects[ i ].y = 0;
                        rects[ i ].width = 100;
                        rects[ i ].height = 200;
                }
        }
        if ( ( XIERoi = GetXIERoi( xp, p, rects, rectsSize ) ) == ( XieRoi ) NULL )
        {
                reps = 0;
        }
        if ( rects )
                free( rects );

        if ( ( src1 = GetXIEPhotomap( xp, p, 1 ) ) == ( XiePhotomap ) NULL )
	{
                reps = 0;
		XieDestroyROI( xp->d, XIERoi );
	}
        else
        {
		free( p->data );
		p->data = ( char * ) NULL;
                if ( ( src2 = GetXIEPhotomap( xp, p, 2 ) ) == ( XiePhotomap ) NULL )
		{
                        reps = 0;
			XieDestroyPhotomap( xp->d, src1 );
			XieDestroyROI( xp->d, XIERoi );
		}
        }
	if ( p->data )
	{
		free( p->data );
		p->data = ( char * ) NULL;
	}
        if ( reps && p->finfo.image3 != ( XIEimage * ) NULL )
        {
                /* alpha plane */

                if ( ( alpha = GetXIEPhotomap( xp, p, 3 ) ) == ( XiePhotomap ) NULL )
                {
                        XieDestroyPhotomap( xp->d, src1 );
                        XieDestroyPhotomap( xp->d, src2 );
                        XieDestroyROI( xp->d, XIERoi );
                        reps = 0;
                }
		if ( p->data )
		{
                	free( p->data );
			p->data = ( char * ) NULL;
		}
        }
	if ( !reps )
		free( parms );
        return( reps );
}

void DoBlendMonadicImmediate(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    	int     i;
	int	flo_notify, flo_id;
	int	flo_elements;
	XiePhotospace photospace;
	XieProcessDomain domain;
	XiePhotoElement *flograph;
	int	decode_notify;

	decode_notify = False;
        photospace = XieCreatePhotospace(xp->d);/* XXX error check */
	flo_id = 1;
	i = 0;
	if ( monoflag )
		flo_elements = 4;
	else
		flo_elements = 3;
	if ( alpha != ( XiePhotomap ) NULL )
		flo_elements++;

	flograph = XieAllocatePhotofloGraph(flo_elements);	
	if ( flograph == ( XiePhotoElement * ) NULL )
	{
		fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
		return;
	}
	XieFloImportPhotomap(&flograph[i], XIEPhotomap, decode_notify); i++;

	domain.offset_x = 0;
	domain.offset_y = 0;
	domain.phototag = 0;

	if ( alpha != ( XiePhotomap ) NULL )
	{
		XieFloImportPhotomap(&flograph[i], alpha, decode_notify);
		i++;
	}

	XieFloBlend(&flograph[i], 
		1,
		0,
		( ( BlendParms * ) p->ts )->constant,
		( alpha != ( XiePhotomap ) NULL ? i : 0 ),	
		( ( BlendParms * ) p->ts )->alphaConstant,	
		&domain,
		( ( BlendParms * ) p->ts )->bandMask ); i++;

	if ( monoflag )
	{
		XieFloConstrain(&flograph[i],
			i,
			levels,
			tech,
			(char *)parms
		); i++;		
	}	
	XieFloExportDrawable(&flograph[i],
		i, /* source phototag number */
		xp->w,
		xp->fggc,
		0,       /* x offset in window */
		0        /* y offset in window */
	); i++;

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

void DoBlendDyadicImmediate(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
        int     i, flo_elements;
        int     flo_notify, flo_id;
        XiePhotospace photospace;
        XieProcessDomain domain;
        XiePhotoElement *flograph;
        int     decode_notify;

        photospace = XieCreatePhotospace(xp->d);/* XXX error check */
	decode_notify = False;
        flo_id = 1;
	i = 0;
        if ( monoflag )
                flo_elements = 5;
        else
                flo_elements = 4;
	if ( alpha != ( XiePhotomap ) NULL )
		flo_elements++;

        flograph = XieAllocatePhotofloGraph(flo_elements);
        if ( flograph == ( XiePhotoElement * ) NULL )
        {
                fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
                return;
        }

	XieFloImportPhotomap(&flograph[i], src1, decode_notify); i++;

	XieFloImportPhotomap(&flograph[i], src2, decode_notify); i++;

	domain.offset_x = 0;
	domain.offset_y = 0;
	domain.phototag = 0;

        if ( alpha != ( XiePhotomap ) NULL )
	{
	        XieFloImportPhotomap(&flograph[i], alpha, decode_notify); i++;
	}

	XieFloBlend(&flograph[i],
		1,
		2,
		( ( BlendParms * ) p->ts )->constant,
		( alpha != ( XiePhotomap ) NULL ? i : 0 ),	
		( ( BlendParms * ) p->ts )->alphaConstant,	
		&domain,
		( ( BlendParms * ) p->ts )->bandMask ); i++;
	if ( monoflag )
	{
		XieFloConstrain(&flograph[i],
			i,
			levels,
			tech,
			(char *)parms
		); i++;
	}

       XieFloExportDrawable(&flograph[i],
		i,         /* source phototag number */
		xp->w,
		xp->fggc,
		0,       /* x offset in window */
		0        /* y offset in window */
	); i++;

	flo_notify = True;
        for (i = 0; i != reps; i++) {

                XieExecuteImmediate(xp->d, photospace,
                        flo_id,
                        flo_notify,
                        flograph,       /* photoflo specification */
                        flo_elements    /* number of elements */
                );
		WaitForFloToFinish( xp, flo_id );
                XSync( xp->d, 0 );
                flo_id++;
        }
        XieDestroyPhotospace( xp->d, photospace );
        XieFreePhotofloGraph(flograph,flo_elements);
}

void DoROIBlendMonadicImmediate(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    	int     i, flo_elements;
	int     band_mask = 1;
	int	flo_notify, flo_id;
	XiePhotospace photospace;
	XieProcessDomain domain;
	XiePhotoElement *flograph;
	int	decode_notify;

	photospace = XieCreatePhotospace(xp->d);/* XXX error check */
	decode_notify = False;
	flo_id = 1;
	i = 0;
        if ( monoflag )
                flo_elements = 5;
        else
                flo_elements = 4;
	if ( alpha != ( XiePhotomap ) NULL )
		flo_elements++;

	flograph = XieAllocatePhotofloGraph(flo_elements);	
	if ( flograph == ( XiePhotoElement * ) NULL )
	{
		fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
		return;
	}

	XieFloImportPhotomap(&flograph[i], XIEPhotomap, decode_notify); i++;

	XieFloImportROI(&flograph[i], XIERoi); i++;

	domain.offset_x = 0;
	domain.offset_y = 0;
	domain.phototag = i;

	if ( alpha != ( XiePhotomap ) NULL )
	{
                XieFloImportPhotomap(&flograph[i], alpha, decode_notify); i++;
	}

	XieFloBlend(&flograph[i], 
		1,
		0,
		( ( BlendParms * ) p->ts )->constant,
		( alpha != ( XiePhotomap ) NULL ? i : 0 ),
                ( ( BlendParms * ) p->ts )->alphaConstant,
                &domain,
                ( ( BlendParms * ) p->ts )->bandMask ); i++;


	if ( monoflag )
	{
		XieFloConstrain(&flograph[i],
			i,
			levels,
			tech,
			(char *)parms
		); i++;
	}

       XieFloExportDrawable(&flograph[i],
		i,       /* source phototag number */
		xp->w,
		xp->fggc,
		0,       /* x offset in window */
		0        /* y offset in window */
	); i++;

	flo_notify = True;	
    	for (i = 0; i != reps; i++) {

       		XieExecuteImmediate(xp->d, photospace,
               		flo_id,		
               		flo_notify,     
               		flograph,       /* photoflo specification */
               		flo_elements    /* number of elements */
       		);

		WaitForFloToFinish( xp, flo_id );
		XSync( xp->d, 0 );

		flo_id++;
    	}
	XieDestroyPhotospace( xp->d, photospace );
	XieFreePhotofloGraph(flograph,flo_elements);	
}

void DoROIBlendDyadicImmediate(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
        int     i, flo_elements;
        int     flo_notify, flo_id;
        XiePhotospace photospace;
        XieProcessDomain domain;
        XiePhotoElement *flograph;
        int     decode_notify;

        photospace = XieCreatePhotospace(xp->d);/* XXX error check */
	decode_notify = False;
	i = 0;
        flo_id = 1;
        if ( monoflag )
                flo_elements = 6;
        else
                flo_elements = 5;
	if ( alpha != ( XiePhotomap ) NULL )
		flo_elements++;

        flograph = XieAllocatePhotofloGraph(flo_elements);
        if ( flograph == ( XiePhotoElement * ) NULL )
        {
                fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
                return;
        }

	XieFloImportPhotomap(&flograph[i], src1, decode_notify); i++;

	XieFloImportPhotomap(&flograph[i], src2, decode_notify); i++;

	XieFloImportROI(&flograph[i], XIERoi ); i++;

	domain.offset_x = 0;
	domain.offset_y = 0;
	domain.phototag = i;

        if ( alpha != ( XiePhotomap ) NULL )
        {
                XieFloImportPhotomap(&flograph[i], alpha, decode_notify); i++;
        }

	XieFloBlend(&flograph[i],
		1,
		2,
                ( ( BlendParms * ) p->ts )->constant,
                ( alpha != ( XiePhotomap ) NULL ? i : 0 ),
                ( ( BlendParms * ) p->ts )->alphaConstant,
                &domain,
                ( ( BlendParms * ) p->ts )->bandMask ); i++;

	if ( monoflag )
	{
		XieFloConstrain(&flograph[i],
			i,
			levels,
			tech,
			(char *)parms
		); i++;
	}
       XieFloExportDrawable(&flograph[i],
		i,   		/* source phototag number */
		xp->w,
		xp->fggc,
		0,       /* x offset in window */
		0        /* y offset in window */
	); i++;

	flo_notify = True;
        for (i = 0; i != reps; i++) {

                XieExecuteImmediate(xp->d, photospace,
                        flo_id,
                        flo_notify,
                        flograph,       /* photoflo specification */
                        flo_elements    /* number of elements */
                );

		WaitForFloToFinish( xp, flo_id );
                XSync( xp->d, 0 );
                flo_id++;
        }
        XieFreePhotofloGraph(flograph,flo_elements);
        XieDestroyPhotospace( xp->d, photospace );
}

int EndBlendMonadic(xp, p)
    XParms  xp;
    Parms   p;
{
	if ( p->data )
	{
		free( p->data );
		p->data = ( char * ) NULL;
	}
	free( parms );
	XieDestroyPhotomap( xp->d, XIEPhotomap );
	if ( alpha != ( XiePhotomap ) NULL )
		XieDestroyPhotomap( xp->d, alpha );
}

int EndBlendDyadic(xp, p)
    XParms  xp;
    Parms   p;
{
	if ( p->data )
	{
		free( p->data );
		p->data = ( char * ) NULL;
	}
	free( parms );
	XieDestroyPhotomap( xp->d, src1 );
	XieDestroyPhotomap( xp->d, src2 );
	if ( alpha != ( XiePhotomap ) NULL )
		XieDestroyPhotomap( xp->d, alpha );
}

int EndROIBlendMonadic(xp, p)
    XParms  xp;
    Parms   p;
{
	if ( p->data )
	{
        	free( p->data );
		p->data = ( char * ) NULL;
	}
	free( parms );
        XieDestroyPhotomap( xp->d, XIEPhotomap );
        XieDestroyROI( xp->d, XIERoi );
	if ( alpha != ( XiePhotomap ) NULL )
		XieDestroyPhotomap( xp->d, alpha );
}

int EndROIBlendDyadic(xp, p)
    XParms  xp;
    Parms   p;
{
	if ( p->data )
	{
        	free( p->data );
		p->data = ( char * ) NULL;
	}
	free( parms );
        XieDestroyPhotomap( xp->d, src1 );
        XieDestroyPhotomap( xp->d, src2 );
        XieDestroyROI( xp->d, XIERoi );
	if ( alpha != ( XiePhotomap ) NULL )
		XieDestroyPhotomap( xp->d, alpha );
}

