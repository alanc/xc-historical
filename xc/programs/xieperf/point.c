/* $XConsortium: point.c,v 1.2 93/07/19 14:44:28 rws Exp $ */

/**** module do_point.c ****/
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
  
	do_point.c -- point flo element tests 

	Syd Logan -- AGE Logic, Inc. July, 1993 - MIT Alpha release
  
*****************************************************************************/
#include "xieperf.h"
#include <stdio.h>

static XieLTriplet levels;
static XieConstrainTechnique tech = xieValConstrainClipScale;
static XieClipScaleParam *parms;
static XieConstant in_low,in_high;
static XieLTriplet out_low,out_high;
static int monoflag = 0;

static XiePhotomap XIEPhotomap;
static XieLut	XIELut;

int InitPoint(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
	unsigned char *lut;
	int lutSize, i, levelsIn, levelsOut;
        XIEimage *image;

        image = p->finfo.image1;
        if ( !image )
                return( 0 );

	parms = NULL;
	monoflag = 0;
        if ( xp->vinfo.depth == 1 )
        {
                monoflag = 1;
                if ( !SetupMonoClipScale( image, levels, 
			in_low, in_high, out_low, out_high, &parms ) )
                {
			return( 0 );
                }
        }

	levelsIn = (( PointParms * ) p->ts )->levelsIn;
	levelsOut = (( PointParms * ) p->ts )->levelsOut;
	lutSize = 1 << levelsIn;
	lut = (unsigned char *)malloc( lutSize * sizeof( unsigned char ) );
	if ( lut == ( unsigned char * ) NULL )
		reps = 0;
	else if ( levelsIn == 8 && levelsOut == 8 )
	{
		for ( i = 0; i < 75; i++ )
			lut[ i ] = i;
		for ( i = 75; i < lutSize; i++ )
		{
			lut[ i ] = ( 1 << levelsOut ) - i;
		}
	}
	else if ( levelsIn == 1 && levelsOut == 8 )
	{
		lut[ 0 ] = 0;
		lut[ 1 ] = 255;
	}
	else if ( levelsIn == 8 && levelsOut == 1 )
	{
		for ( i = 0; i < 75; i++ )
			lut[ i ] = 0;
		for ( i = 75; i < 256; i++ )
			lut[ i ] = 1;
	}	
	if ( levelsIn == 1 )
	{
		if ( ( XIEPhotomap = GetXIEBitonalPhotomap( xp, p, 1 ) ) == ( XiePhotomap ) NULL )
			reps = 0;
	}
	else if ( ( XIEPhotomap = GetXIEPhotomap( xp, p, 1 ) ) == ( XiePhotomap ) NULL )
	{
		reps = 0;
	}
	if ( reps != 0 )
	{
		if ( ( XIELut = GetXIELut( xp, p, lut, lutSize, 1 << levelsOut ) ) 
			== ( XieLut ) NULL )
		{
			reps = 0;
		}
	}
	if ( lut )
		free( lut );
	if ( !reps && parms )
		free( parms );
	return( reps );
}

void DoPointImmediate(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    	int     i, flo_elements;
	int	levelsOut;
	int     band_mask = 1;
	char 	*datatmp;
	int	flo_notify, flo_id;
	XiePhotospace photospace;
	int     band_order = xieValLSFirst;
	XieProcessDomain domain;
	XiePhotoElement *flograph;
	int	decode_notify;

	photospace = XieCreatePhotospace(xp->d);/* XXX error check */

	levelsOut = (( PointParms * ) p->ts )->levelsOut;
	flo_id = 1;
        if ( monoflag )
                flo_elements = 5;
        else
                flo_elements = 4;

	flograph = XieAllocatePhotofloGraph(flo_elements);	
	if ( flograph == ( XiePhotoElement * ) NULL )
	{
		fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
		return;
	}

	decode_notify = False;
	XieFloImportPhotomap(&flograph[0], XIEPhotomap, decode_notify);

	domain.offset_x = 0;
	domain.offset_y = 0;
	domain.phototag = 0;

	XieFloImportLUT(&flograph[1], XIELut );

	XieFloPoint(&flograph[2],
		1,
		&domain,
		2,
		band_mask 
	);

	if ( monoflag )
	{
		XieFloConstrain(&flograph[3],
			3,
			levels,
			tech,
			(char *)parms
		);
	}

	if ( levelsOut == 1 )
	{
		XieFloExportDrawablePlane(&flograph[flo_elements - 1],
			flo_elements - 1,  /* source phototag number */
			xp->w,
			xp->fggc,
			0,       /* x offset in window */
			0        /* y offset in window */
		);
	}
	else
	{
		XieFloExportDrawable(&flograph[flo_elements - 1],
			flo_elements - 1,  /* source phototag number */
			xp->w,
			xp->fggc,
			0,       /* x offset in window */
			0        /* y offset in window */
		);
	}

	flo_notify = False;	
    	for (i = 0; i != reps; i++) {

       		XieExecuteImmediate(xp->d, photospace,
               		flo_id,		
               		flo_notify,     
               		flograph,       /* photoflo specification */
               		flo_elements    /* number of elements */
       		);

		flo_id++;
		XSync( xp->d, 0 );
    	}
	XieFreePhotofloGraph(flograph,flo_elements);	
	XieDestroyPhotospace( xp->d, photospace );
}

void DoPointStored(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    	int     i, flo_elements;
	int	flo_notify;
	XiePhotoElement *flograph;
	XiePhotoflo flo;
        int     band_mask = 1;
        int     band_order = xieValLSFirst;
        XieProcessDomain domain;
	int	levelsOut;
        int     decode_notify;

	levelsOut = ( ( PointParms * ) p->ts )->levelsOut;
        if ( monoflag )
                flo_elements = 5;
        else
                flo_elements = 4;

	flograph = XieAllocatePhotofloGraph(flo_elements);	
	if ( flograph == ( XiePhotoElement * ) NULL )
	{
		fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
		return;
	}

	decode_notify = False;
        XieFloImportPhotomap(&flograph[0], XIEPhotomap, decode_notify);

	domain.offset_x = 0;
	domain.offset_y = 0;
	domain.phototag = 0;

	XieFloImportLUT(&flograph[1], XIELut );

	XieFloPoint(&flograph[2],
		1,
		&domain,
		2,
		band_mask
	);

      	if ( monoflag )
	{
		XieFloConstrain(&flograph[3],
			3,
			levels,
			tech,
			(char *)parms
		);
	}

	if ( levelsOut == 1 )
	{
		XieFloExportDrawablePlane(&flograph[flo_elements-1],
			flo_elements-1,      /* source phototag number */
			xp->w,
			xp->fggc,
			0,       /* x offset in window */
			0        /* y offset in window */
		);
	}
	else
	{
                XieFloExportDrawable(&flograph[flo_elements-1],
                        flo_elements-1,  /* source phototag number */
                        xp->w,
                        xp->fggc,
                        0,       /* x offset in window */
                        0        /* y offset in window */
                );
        }


	flo = XieCreatePhotoflo( xp->d, flograph, flo_elements );

	/* crank it */

	flo_notify = False;
    	for (i = 0; i != reps; i++) {
		XieExecutePhotoflo( xp->d, flo, flo_notify );
		XSync( xp->d, 0 );
    	}

	/* destroy the photoflo */

	XieDestroyPhotoflo( xp->d, flo );
	XieFreePhotofloGraph(flograph,flo_elements);	
}

void EndPoint(xp, p)
    XParms  xp;
    Parms   p;
{
	CloseXIEPhotomap( xp, p, XIEPhotomap );
	CloseXIELut( xp, p, XIELut );
	if ( parms )
		free( parms );
}

