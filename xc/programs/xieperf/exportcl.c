/* $XConsortium: exportcl.c,v 1.4 93/07/27 14:38:12 rws Exp $ */

/**** module do_exportclient.c ****/
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
  
	do_exportclient.c -- export client flo element tests 

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
extern unsigned int dataCheckSum1;

int InitExportClientPhoto(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
	p->data = ( char * ) NULL;
	if ( ( XIEPhotomap = GetXIEPhotomap( xp, p, 1 ) ) == ( XiePhotomap ) NULL )
		reps = 0;
	return( reps );
}

int InitExportClientLUT(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
	int	i;

	lutSize = ( 1 << xp->vinfo.depth ) * sizeof( unsigned char );
	lut = (unsigned char *)malloc( lutSize );
	p->data = (char *)malloc( lutSize );
	if ( lut == ( unsigned char * ) NULL || p->data == ( char * ) NULL )
	{
		if ( lut )
			free( lut );
		else if ( p->data )
		{
			free( p->data );
			p->data = ( char * ) NULL;
		}
		reps = 0;
	}
	else
	{
		for ( i = 0; i < lutSize; i++ )
		{
			lut[ i ] = ( 1 << xp->vinfo.depth ) - i;
		}
		if ( ( XIELut = GetXIELut( xp, p, lut, lutSize, lutSize ) ) ==
			( XieLut ) NULL )
		{
			reps = 0;
		}	
	}
	return( reps );
}

int InitExportClientROI(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
	int	i;

	rectsSize = 10;
	rects = (XieRectangle *)malloc( rectsSize * sizeof( XieRectangle ) );
	p->data = (char *)malloc( rectsSize * sizeof( XieRectangle ) );
	if ( rects == ( XieRectangle * ) NULL || p->data == ( char * ) NULL )
	{
		reps = 0;
	}
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
		if ( ( XIERoi = GetXIERoi( xp, p, rects, rectsSize ) ) ==
			( XieRoi ) NULL )
		{
			reps = 0;
		}
	}
	if ( !reps )
	{
		if ( rects )
			free( rects );
		if ( p->data )
		{
			free( p->data );
			p->data = ( char * ) NULL;
		}
	}

	return( reps );
}

void DoExportClientPhotoImmediate(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
	XIEimage	*image;
    	int     i;
	int	flo_notify, flo_id;
	XiePhotospace photospace;
        XieEncodeTechnique encode_tech=xieValEncodeServerChoice;
        char *encode_params=NULL;
	XiePhotoElement *flograph;
	unsigned int checksum;

	image = p->finfo.image1;
        if ( !image )
                return;
	photospace = XieCreatePhotospace(xp->d);/* XXX error check */

	flograph = XieAllocatePhotofloGraph(2);
	if ( flograph == ( XiePhotoElement * ) NULL )
	{
		fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
		return;
	}
	XieFloImportPhotomap(&flograph[0], XIEPhotomap, False );

	XieFloExportClientPhoto(&flograph[1],
		1,              /* source phototag number */
		True,
		encode_tech,
		encode_params
	);

	flo_notify = True;
	flo_id = 1;
    	for (i = 0; i != reps; i++) {

		XieExecuteImmediate(xp->d, photospace,
			flo_id,
			flo_notify,
			flograph,       /* photoflo specification */
			2               /* number of elements */
		);
		XSync( xp->d, 0 );
		ReadNotifyExportData( xp, p, photospace, flo_id, 2, 
			image->fsize );
		WaitForFloToFinish( xp, flo_id );
		checksum = CheckSum( p->data, image->fsize );
		if ( checksum != image->chksum )
		{
			fprintf( stderr, "Photomap not read correctly by client\n" );
			break;
		}
		flo_id++;
    	}
	XieFreePhotofloGraph(flograph,2);
	XieDestroyPhotospace( xp->d, photospace );
}

void DoExportClientPhotoStored(xp, p, reps)
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
        XieEncodeTechnique encode_tech=xieValEncodeServerChoice;
        char *encode_params=NULL;
	unsigned int checksum;

	image = p->finfo.image1;
        if ( !image )
                return;
	flograph = XieAllocatePhotofloGraph(2);	
	if ( flograph == ( XiePhotoElement * ) NULL )
	{
		fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
		return;
	}

	XieFloImportPhotomap(&flograph[0], XIEPhotomap, False );

        XieFloExportClientPhoto(&flograph[1],
		1,              /* source phototag number */
		True,
		encode_tech,
		encode_params
	);

	flo = XieCreatePhotoflo( xp->d, flograph, 2 );

	/* crank it */

	photospace = 0;
	flo_notify = True;
    	for (i = 0; i != reps; i++) {
		XieExecutePhotoflo( xp->d, flo, flo_notify );
		XSync( xp->d, 0 );
		ReadNotifyExportData( xp, p, photospace, flo, 2,
			image->fsize );
		WaitForFloToFinish( xp, flo );
		checksum = CheckSum( p->data, image->fsize );
		if ( checksum != image->chksum )
		{
			fprintf( stderr, "Photomap not read correctly by client\n" );
			break;
		}
    	}

	/* destroy the photoflo */

	XieDestroyPhotoflo( xp->d, flo );
	XieFreePhotofloGraph(flograph,2);	
}

void DoExportClientLUTImmediate(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    	int     i;
	int	flo_notify, flo_id;
	XiePhotospace photospace;
	XieOrientation band_order = xieValLSFirst;
        XieLTriplet     start, length;
	XiePhotoElement *flograph;

	photospace = XieCreatePhotospace(xp->d);/* XXX error check */

	flo_id = 1;
	flograph = XieAllocatePhotofloGraph(2);	
	if ( flograph == ( XiePhotoElement * ) NULL )
	{
		fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
		return;
	}

     	length[ 0 ] = 1 << xp->vinfo.depth;
	length[ 1 ] = 0;
	length[ 2 ] = 0;

	XieFloImportLUT(&flograph[0], XIELut);

	start[ 0 ] = 0;
	start[ 1 ] = 0;
	start[ 2 ] = 0;

	XieFloExportClientLUT(&flograph[1],
		1,              /* source phototag number */
		band_order,
		True,
		start,
		length
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
                ReadNotifyExportData( xp, p, photospace, flo_id, 2,
                        lutSize * sizeof( unsigned char ) );
                WaitForFloToFinish( xp, flo_id );
		if ( memcmp( lut, p->data, lutSize * sizeof( unsigned char ) ) )
		{
			fprintf( stderr, "ExportClientLUT failed\n" );
			break;
		}
		flo_id++;
    	}
	XieDestroyPhotospace( xp->d, photospace );
	XieFreePhotofloGraph(flograph,2);	
}

void DoExportClientLUTStored(xp, p, reps)
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
        XieLTriplet     start, length, levels;

	flograph = XieAllocatePhotofloGraph(2);	
	if ( flograph == ( XiePhotoElement * ) NULL )
	{
		fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
		return;
	}

	XieFloImportLUT(&flograph[0], XIELut);

	length[ 0 ] = 1 << xp->vinfo.depth;
	length[ 1 ] = 0;
	length[ 2 ] = 0;

	start[ 0 ] = 0;
	start[ 1 ] = 0;
	start[ 2 ] = 0;

	XieFloExportClientLUT(&flograph[1],
		1,              /* source phototag number */
		band_order,
		True,
		start,
		length
	);

	flo = XieCreatePhotoflo( xp->d, flograph, 2 );

	/* crank it */

	flo_notify = True;
	photospace = 0;
    	for (i = 0; i != reps; i++) {
		XieExecutePhotoflo( xp->d, flo, flo_notify );
                XSync( xp->d, 0 );
                ReadNotifyExportData( xp, p, photospace, flo, 2,
                        lutSize * sizeof( unsigned char ) );
                WaitForFloToFinish( xp, flo );
                if ( memcmp( lut, p->data, lutSize * sizeof( unsigned char ) ) )
                {
                        fprintf( stderr, "ExportClientLUT failed\n" );
                        break;
                }
    	}

	/* destroy the photoflo */

	XieDestroyPhotoflo( xp->d, flo );
	XieFreePhotofloGraph(flograph,2);	
}

void DoExportClientROIImmediate(xp, p, reps)
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

	XieFloImportROI(&flograph[0], XIERoi );

	XieFloExportClientROI(&flograph[1],
		1,              /* source phototag number */
		True 
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
                ReadNotifyExportData( xp, p, photospace, flo_id, 2,
                        rectsSize * sizeof( XieRectangle ) );
                WaitForFloToFinish( xp, flo_id );
		if ( memcmp( rects, p->data, rectsSize * sizeof( XieRectangle ) ) )
		{
			fprintf( stderr, "ExportClientROI failed\n" );
			break;
		}
		flo_id++;
    	}
	XieFreePhotofloGraph(flograph,2);	
	XieDestroyPhotospace( xp->d, photospace );
}

void DoExportClientROIStored(xp, p, reps)
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

        XieFloImportROI(&flograph[0], XIERoi);

	XieFloExportClientROI(&flograph[1],
		1,              /* source phototag number */
		True
	);

	flo = XieCreatePhotoflo( xp->d, flograph, 2 );

	/* crank it */

	flo_notify = True;
	photospace = 0;
    	for (i = 0; i != reps; i++) {
		XieExecutePhotoflo( xp->d, flo, flo_notify );
                XSync( xp->d, 0 );
                ReadNotifyExportData( xp, p, photospace, flo, 2,
                        rectsSize * sizeof( XieRectangle ) );
                WaitForFloToFinish( xp, flo );
                if ( memcmp( rects, p->data, rectsSize * sizeof( XieRectangle )
) )
                {
                        fprintf( stderr, "ExportClientROI failed\n" );
                        break;
                }
    	}

	/* destroy the photoflo */

	XieDestroyPhotoflo( xp->d, flo );
	XieFreePhotofloGraph(flograph,2);	
}

void EndExportClientLUT(xp, p)
    XParms  xp;
    Parms   p;
{
	XieDestroyLUT( xp->d, XIELut );
	free( lut );
	if ( p->data )
	{
		free( p->data );
		p->data = ( char * ) NULL;
	}
}

void EndExportClientPhoto(xp, p)
    XParms  xp;
    Parms   p;
{
	XieDestroyPhotomap(xp->d, XIEPhotomap);
	if ( p->data )
	{
		free( p->data );
		p->data = ( char * ) NULL;
	}
}

void EndExportClientROI(xp, p)
    XParms  xp;
    Parms   p;
{
	XieDestroyROI(xp->d, XIERoi );
	free( rects );
	if ( p->data )
	{
		free( p->data );
		p->data = ( char * ) NULL;
	}
}
