/* $XConsortium: importcl.c,v 1.2 93/07/19 14:44:06 rws Exp $ */

/**** module do_importclient.c ****/
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
  
	do_importclient.c -- import client flo element tests 

	Syd Logan -- AGE Logic, Inc. July, 1993 - MIT Alpha release
  
*****************************************************************************/
#include "xieperf.h"
#include <stdio.h>

static XiePhotomap XIEPhotomap;
static XieLut	XIELut;
static XieRoi	XIERoi;
static unsigned char *lut;
static int lutSize;
static XieRectangle *rects;
static int rectsSize;

int InitImportClientPhoto(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
	p->data = ( char * ) NULL;
	if (!GetImageData( xp, p, 1 ))
		reps = 0;
	else
		XIEPhotomap = XieCreatePhotomap(xp->d);
	return( reps );
}

int InitImportClientLUT(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
	int	i;

	XIELut = XieCreateLUT(xp->d);

	lutSize = ( 1 << xp->vinfo.depth ) * sizeof( unsigned char );
	lut = (unsigned char *)malloc( lutSize );
	if ( lut == ( unsigned char * ) NULL )
		reps = 0;
	else
	{
		for ( i = 0; i < lutSize; i++ )
		{
			lut[ i ] = ( ( 1 << xp->vinfo.depth ) - 1 ) - i;
		}
	}
	return( reps );
}

int InitImportClientROI(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
	int	i;

	XIERoi = XieCreateROI( xp->d );

	rectsSize = 10;
	rects = (XieRectangle *)malloc( rectsSize * sizeof( XieRectangle ) );
	if ( rects == ( XieRectangle * ) NULL )
		reps = 0;
	else
	{
		/* who cares what the data is */

		for ( i = 0; i < rectsSize; i++ )
		{
			rects[ i ].x = i;
			rects[ i ].y = i;
			rects[ i ].width = i + 10;
			rects[ i ].height = i + 10;
		}
	}
	return( reps );
}

void DoImportClientPhotoImmediate(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
	XIEimage	*image;
    	int     i;
	int	flo_notify, flo_id;
	XiePhotospace photospace;
        XieEncodeTechnique encode_tech=xieValEncodeServerChoice;
	XieDecodeUncompressedSingleParam *decode_params=NULL;
        XieLTriplet width, height, levels;
        char *encode_params=NULL;
	XiePhotoElement *flograph;
	int	decode_notify;

	photospace = XieCreatePhotospace(xp->d);/* XXX error check */

	image = p->finfo.image1;
	if ( !image )
		return;
	decode_notify = False;
        width[ 0 ] = image->width;
        height[ 0 ] = image->height;
        levels[ 0 ] = image->levels;

        decode_params = XieTecDecodeUncompressedSingle(
                image->fill_order,
                image->pixel_order,
                image->pixel_stride,
                image->left_pad,
                image->scanline_pad
        );
	flo_id = 1;
	flograph = XieAllocatePhotofloGraph(2);
	if ( flograph == ( XiePhotoElement * ) NULL )
	{
		fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
		return;
	}
	XieFloImportClientPhoto(&flograph[0],
		image->data_class,
		width, height, levels,
		decode_notify,
		image->decode, (char *)decode_params
	);

	XieFloExportPhotomap(&flograph[1],
		1,              /* source phototag number */
		XIEPhotomap,
		encode_tech,
		encode_params
	);

	flo_notify = True;
    	for (i = 0; i != reps; i++) {

		XieExecuteImmediate(xp->d, photospace,
			flo_id,
			flo_notify,
			flograph,       /* photoflo specification */
			2               /* number of elements */
		);
		XSync( xp->d, 0 );
		PumpTheClientData( xp, p, flo_id, photospace, 1,
                        p->data, image->fsize );
		WaitForFloToFinish( xp, flo_id );
		flo_id++;
    	}
	XieFreePhotofloGraph(flograph,2);
	XieDestroyPhotospace( xp->d, photospace );
	free( decode_params );
}

void DoImportClientPhotoStored(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
	XIEimage	*image;
    	int     i;
        XiePhotospace photospace;
	int	flo_notify;
	XiePhotoElement *flograph;
	XiePhotoflo flo;
	XieDecodeUncompressedSingleParam *decode_params=NULL;
        XieLTriplet width, height, levels;
        int     decode_notify;
        XieEncodeTechnique encode_tech=xieValEncodeServerChoice;
        char *encode_params=NULL;

	image = p->finfo.image1;
        if ( !image )
                return;
	decode_notify = False;
	flograph = XieAllocatePhotofloGraph(2);	
	if ( flograph == ( XiePhotoElement * ) NULL )
	{
		fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
		return;
	}

        width[ 0 ] = image->width;
        height[ 0 ] = image->height;
        levels[ 0 ] = image->levels;

        decode_params = XieTecDecodeUncompressedSingle(
                image->fill_order,
                image->pixel_order,
                image->pixel_stride,
                image->left_pad,
                image->scanline_pad
        );

      	XieFloImportClientPhoto(&flograph[0],
		image->data_class,
		width, height, levels,
		decode_notify,
		image->decode, (char *)decode_params
	);

        XieFloExportPhotomap(&flograph[1],
	    	1,              /* source phototag number */
		XIEPhotomap,
		encode_tech,
		encode_params
	);

	flo = XieCreatePhotoflo( xp->d, flograph, 2 );

	/* crank it */

	flo_notify = True;
	photospace = 0;
    	for (i = 0; i != reps; i++) {
		XieExecutePhotoflo( xp->d, flo, flo_notify );
		XSync( xp->d, 0 );
                PumpTheClientData( xp, p, flo, photospace, 1,
                        p->data, image->fsize );
                WaitForFloToFinish( xp, flo );
    	}

	/* destroy the photoflo */

	free( decode_params );
	XieDestroyPhotoflo( xp->d, flo );
	XieFreePhotofloGraph(flograph,2);	
}

void DoImportClientLUTImmediate(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    	int     i;
	int	flo_notify, flo_id;
	XiePhotospace photospace;
	XieOrientation band_order = xieValLSFirst;
        XieDataClass    data_class;
        XieLTriplet     start, length, levels;
        Bool    merge;
	XiePhotoElement *flograph;

	photospace = XieCreatePhotospace(xp->d);/* XXX error check */

	flo_id = 1;
	flograph = XieAllocatePhotofloGraph(2);	
	if ( flograph == ( XiePhotoElement * ) NULL )
	{
		fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
		return;
	}

        data_class = xieValSingleBand;
        band_order = xieValLSFirst;
     	length[ 0 ] = 1 << xp->vinfo.depth;
	length[ 1 ] = 0;
	length[ 2 ] = 0;
	levels[ 0 ] = 1 << xp->vinfo.depth;
	levels[ 1 ] = 0;
	levels[ 2 ] = 0;

	XieFloImportClientLUT(&flograph[0],
		data_class,
		band_order,
		length,
		levels
	);

	merge = False;
	start[ 0 ] = 0;
	start[ 1 ] = 0;
	start[ 2 ] = 0;

	XieFloExportLUT(&flograph[1],
		1,              /* source phototag number */
		XIELut,
		merge,
		start
	);

	flo_notify = True;	
    	for (i = 0; i != reps; i++) {

       		XieExecuteImmediate(xp->d, photospace,
               		flo_id,		
               		flo_notify,     
               		flograph,       /* photoflo specification */
               		2               /* number of elements */
       		);
                PumpTheClientData( xp, p, flo_id, photospace, 1,
                        lut, lutSize );
                WaitForFloToFinish( xp, flo_id );

		flo_id++;
		XSync( xp->d, 0 );
    	}
	XieDestroyPhotospace( xp->d, photospace );
	XieFreePhotofloGraph(flograph,2);	
}

void DoImportClientLUTStored(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    	int     i;
	XiePhotospace photospace;
	int	flo_notify;
	XiePhotoElement *flograph;
	XiePhotoflo flo;
        XieOrientation band_order = xieValLSFirst;
        XieDataClass    data_class;
        XieLTriplet     start, length, levels;
        Bool    merge;

	flograph = XieAllocatePhotofloGraph(2);	
	if ( flograph == ( XiePhotoElement * ) NULL )
	{
		fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
		return;
	}

	data_class = xieValSingleBand;
	band_order = xieValLSFirst;
	length[ 0 ] = 1 << xp->vinfo.depth;
	length[ 1 ] = 0;
	length[ 2 ] = 0;
	levels[ 0 ] = 1 << xp->vinfo.depth;
	levels[ 1 ] = 0;
	levels[ 2 ] = 0;

	XieFloImportClientLUT(&flograph[0],
		data_class,
		band_order,
		length,
		levels
	);

	merge = False;
	start[ 0 ] = 0;
	start[ 1 ] = 0;
	start[ 2 ] = 0;

	XieFloExportLUT(&flograph[1],
		1,              /* source phototag number */
		XIELut,
		merge,
		start
	);

	flo = XieCreatePhotoflo( xp->d, flograph, 2 );

	/* crank it */

	flo_notify = True;
	photospace = 0;
    	for (i = 0; i != reps; i++) {
		XieExecutePhotoflo( xp->d, flo, flo_notify );
		XSync( xp->d, 0 );
                PumpTheClientData( xp, p, flo, photospace, 1,
                        lut, lutSize );
                WaitForFloToFinish( xp, flo );
    	}

	/* destroy the photoflo */

	XieDestroyPhotoflo( xp->d, flo );
	XieFreePhotofloGraph(flograph,2);	
}

void DoImportClientROIImmediate(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    	int     i;
	int	flo_notify, flo_id;
	XiePhotospace photospace;
	XiePhotoElement *flograph;

	photospace = XieCreatePhotospace(xp->d);/* XXX error check */

	flo_id = 1;
	flograph = XieAllocatePhotofloGraph(2);	
	if ( flograph == ( XiePhotoElement * ) NULL )
	{
		fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
		return;
	}

	XieFloImportClientROI(&flograph[0], rectsSize);

	XieFloExportROI(&flograph[1],
		1,              /* source phototag number */
		XIERoi
	);

	flo_notify = True;	
    	for (i = 0; i != reps; i++) {

       		XieExecuteImmediate(xp->d, photospace,
               		flo_id,		
               		flo_notify,     
               		flograph,       /* photoflo specification */
               		2               /* number of elements */
       		);
		
	        XSync( xp->d, 0 );
       		PumpTheClientData( xp, p, flo_id, photospace, 1, 
			rects, rectsSize * sizeof( XieRectangle ) );
        	WaitForFloToFinish( xp, flo_id );
		flo_id++;
    	}
	XieFreePhotofloGraph(flograph,2);	
	XieDestroyPhotospace( xp->d, photospace );
}

void DoImportClientROIStored(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    	int     i;
	XiePhotospace	photospace;
	int	flo_notify;
	XiePhotoElement *flograph;
	XiePhotoflo flo;

	flograph = XieAllocatePhotofloGraph(2);	
	if ( flograph == ( XiePhotoElement * ) NULL )
	{
		fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
		return;
	}

        XieFloImportClientROI(&flograph[0],
		rectsSize
	);

	XieFloExportROI(&flograph[1],
		1,              /* source phototag number */
		XIERoi
	);

	flo = XieCreatePhotoflo( xp->d, flograph, 2 );

	/* crank it */

	flo_notify = True;
	photospace = 0;
    	for (i = 0; i != reps; i++) {
		XieExecutePhotoflo( xp->d, flo, flo_notify );
		XSync( xp->d, 0 );
                PumpTheClientData( xp, p, flo, photospace, 1,
                        rects, rectsSize * sizeof( XieRectangle ) );
                WaitForFloToFinish( xp, flo );
    	}

	/* destroy the photoflo */

	XieDestroyPhotoflo( xp->d, flo );
	XieFreePhotofloGraph(flograph,2);	
}

void EndImportClientLUT(xp, p)
    XParms  xp;
    Parms   p;
{
        /* deallocate the data buffer */

	XieDestroyLUT( xp->d, XIELut );
	free( lut );
	if ( p->data )
	{
		free( p->data );
		p->data = ( char * ) NULL;
	}
}

void EndImportClientPhoto(xp, p)
    XParms  xp;
    Parms   p;
{
        /* deallocate the data buffer */

	XieDestroyPhotomap(xp->d, XIEPhotomap);
	if ( p->data )
	{
		free( p->data );
		p->data = ( char * ) NULL;
	}
}

void EndImportClientROI(xp, p)
    XParms  xp;
    Parms   p;
{
        /* deallocate the data buffer */

	XieDestroyROI(xp->d, XIERoi );
	if ( p->data )
	{
		free( p->data );
		p->data = ( char * ) NULL;
	}
	free( rects );
}


