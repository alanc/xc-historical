/* $XConsortium: pasteup.c,v 1.2 93/07/19 14:44:24 rws Exp $ */

/**** module do_pasteup.c ****/
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
  
	do_pasteup.c -- pasteup flo element tests 

	Syd Logan -- AGE Logic, Inc. July, 1993 - MIT Alpha release
  
*****************************************************************************/
#include "xieperf.h"
#include <stdio.h>

#define NTILES 16	/* has an integer square root and be a power of 2 */
#define	SPLIT  4 	/* square root of NTILES */

static int BuildPasteUpFlograph();

static XiePhotomap XIEPhotomap;
static XiePhotomap segments[NTILES];		
static XieTile	tiles[NTILES];
static XiePhotoElement *flograph;
static XiePhotoflo	flo;
static int	flo_elements;
static XieConstant constant = { 0.0, 0.0, 0.0 };

static XieLTriplet levels;
static XieConstrainTechnique tech = xieValConstrainClipScale;
static XieClipScaleParam *parms;
static XieConstant in_low,in_high;
static XieLTriplet out_low,out_high;
static int monoflag = 0;

static int pasteUpIdx;

int InitPasteUp(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{	
	XIEimage *image;
	int	i;

	parms = NULL;
	p->data = ( char * ) NULL;
	image = p->finfo.image1;
        if ( !image )
                return( 0 );

	monoflag = 0;
        if ( xp->vinfo.depth == 1 )
        {
		monoflag = 1;
		if ( SetupMonoClipScale( image, levels, 
			in_low, in_high, out_low, out_high, &parms ) == 0 )
		{
			return( 0 );
		}
        }

	if ( ( XIEPhotomap = GetXIEPhotomap( xp, p, 1 ) ) == 
		( XiePhotomap ) NULL )
	{
		reps = 0;
	}

	else if ( !SegmentPhotomap( xp, p, XIEPhotomap, 
		segments, NTILES, SPLIT, image->width, image->height ) )
	{
		CloseXIEPhotomap( xp, p, XIEPhotomap );
		reps = 0;
	}

	else if ( !BuildPasteUpFlograph( xp, p, 
		&flograph, segments, NTILES, SPLIT, image->width, 
		image->height ) )
	{
		CloseXIEPhotomap( xp, p, XIEPhotomap );
		for ( i = 0; i < NTILES; i++ )
			XieDestroyPhotomap( xp->d, segments[ i ] );
		reps = 0;
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

/* cut a photoflo into "size" little pieces. Assumption is that the values
   width and height are each congruent to 0 mod size. Caller allocates segvec */
 
int
SegmentPhotomap( xp, p, pm, segvec, size, split, width, height )
XParms	xp;
Parms	p;
XiePhotomap pm;		
XiePhotomap segvec[];
int	size;
int	split;
int	width;
int	height;
{
	XiePhotomap *pp;
	int	xoff, yoff;
	int     flo_elements;
	XiePhotoElement *flograph;
	int	segidx; 
	XiePhotospace photospace;
        XieEncodeTechnique encode_tech=xieValEncodeServerChoice;
        char *encode_param=NULL;
	float coefficients[ 6 ];
	int	i, j, retval;
	XieGeometryTechnique sample_tech = xieValGeomNearestNeighbor;
	char	*sample_param = NULL;
	unsigned long flo_id;
	int	piecewide, piecehigh;

	retval = 1;
	piecehigh = height / split; 
	piecewide = width / split; 
	photospace = XieCreatePhotospace(xp->d);
	flo_elements = 3;
	flograph = XieAllocatePhotofloGraph(flo_elements);

        if ( flograph == ( XiePhotoElement * ) NULL )
        {
                fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
		retval = 0;
        }

	/* build the photomaps */

	if ( retval )
	{
		for ( i = 0; i < size; i++ )
		{
			if ( ( segments[ i ] = XieCreatePhotomap( xp->d ) ) ==
				( XiePhotomap ) NULL )
			{
				fprintf( stderr,"XieCreatePhotomap failed\n" );
				retval = 0;
				for ( j = i - 1; j >= 0; j-- )
					XieDestroyPhotomap( xp->d, 
						segments[ j ] );
			}
		}
	}
	if ( retval )
	{			
		segidx = 0;
		coefficients[ 0 ] = 1.0;
		coefficients[ 1 ] = 0.0;
		coefficients[ 2 ] = 0.0;
		coefficients[ 3 ] = 1.0;
		flo_id = 1;
		for ( xoff = 0; xoff < width; xoff+= piecewide )
		{
			for (yoff = 0;yoff < width;yoff += piecehigh)
			{
				XieFloImportPhotomap( &flograph[ 0 ], 
					pm,
					False );
				coefficients[ 4 ] = (float) xoff;
				coefficients[ 5 ] = (float) yoff;
				XieFloGeometry( &flograph[ 1 ],
					1,
					piecewide, 
					piecehigh,
					coefficients,
					constant,
					7,
					sample_tech,
					sample_param
				);
				XieFloExportPhotomap( &flograph[ 2 ],
					2,
					segvec[ segidx++ ],
					encode_tech,
					encode_param
				);
				XieExecuteImmediate(xp->d, photospace,
					flo_id,
					True,
					flograph,   /* photoflo specification */
					flo_elements/* number of elements */
				);
				XSync( xp->d, 0 );
				WaitForFloToFinish( xp, flo_id );
				flo_id++;
			}
		}			
	}
	XieFreePhotofloGraph(flograph, flo_elements);
	XieDestroyPhotospace(xp->d, photospace);
	return( retval );
}

static int
BuildPasteUpFlograph( xp, p, flograph, segments, size, split, width, height )
XParms	xp;
Parms	p;
XiePhotoElement **flograph;
XiePhotomap segments[];
int	size;
int	split;
int	width;
int	height;
{
	int	i, j, flo_elements;
	int	tile_width, tile_height;

	tile_width = width / split;
	tile_height = height / split;

	flo_elements = size + 2;	
	if ( monoflag )
		flo_elements++;

	*flograph = XieAllocatePhotofloGraph(flo_elements);	
	if ( *flograph == ( XiePhotoElement * ) NULL )
	{
		fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
		return( 0 );
	}

	/* do all of the import photo elements */

	for ( i = 0; i < size; i++ )
	{
		XieFloImportPhotomap(&(*flograph)[i], segments[ i ], False);
	}

	/* build all of the tiles, with initial dst_x and dst_y */

	if ( !BuildMeSomeTiles( tiles, split, tile_width, tile_height, 1,
		( ( PasteUpParms * ) p->ts )->overlap ) )
		return( 0 );

	/* gee, that was easy */

	/* now, for the pasteup element */ 	

	pasteUpIdx = size;
	XieFloPasteUp( &(*flograph)[ size ], width, height, constant, tiles, 
		size );
	
        if ( monoflag )
        {
                XieFloConstrain(&(*flograph)[size + 1],
                        size + 1,
                        levels,
                        tech,
                        (char *)parms
                );
        }

	XieFloExportDrawable(&(*flograph)[flo_elements - 1],
		flo_elements - 1,       /* source phototag number */
		xp->w,
		xp->fggc,
		0,       /* x offset in window */
		0        /* y offset in window */
	);

	flo = XieCreatePhotoflo( xp->d, *flograph, flo_elements );
	return( 1 );
}

int
BuildMeSomeTiles( tiles, split, width, height, method, overlap )
XieTile tiles[];
int	split;
int	width;
int	height;
int	method;
int	overlap;
{
	int i, j, idx;

	SetTileXY( tiles, split, width, height, method, overlap );

        idx = 0;
        for ( i = 0; i < split; i++ )
        {
                for ( j = 0; j < split; j++ )
                {
                        tiles[ idx ].src = idx + 1;
                        idx++;
                }
        }
	return( 1 );
}

int
SetTileXY( tiles, split, width, height, method, overlap )
XieTile	tiles[];
int	split;
int	width;
int	height;
int	method;
int	overlap;
{
	int	i, j, idx;

	idx = 0;
	if ( method == 0 ) 
	{
		for ( i = 0; i < split; i++ )
		{
			for ( j = 0; j < split; j++ )
			{
				if ( overlap == Overlap && i == j  )
				{
					tiles[ idx ].dst_x = i * width - ( width / 2 );
					tiles[ idx ].dst_y = j * height - ( height / 2 );
				}
				else
				{
					tiles[ idx ].dst_x = i * width;
					tiles[ idx ].dst_y = j * height;
				}
				idx++;
			}
		}
	}
	else if ( method == 1 )
	{
		for ( i = 0; i < split; i++ )
		{
			for ( j = 0; j < split; j++ )
			{
				if ( overlap == Overlap && i == j )
				{
					tiles[ idx ].dst_x = j * width - ( width / 2 );
					tiles[ idx ].dst_y = i * height - ( height / 2 );
				}
				else
				{
					tiles[ idx ].dst_x = j * width;
					tiles[ idx ].dst_y = i * height;
				}
				idx++;
			}
		}
	}
}

void DoPasteUp(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    	int     i, method;
	int	flo_notify;
	XIEimage *image;
	int	width, height, overlap;

	flo_notify = True;	
	image = p->finfo.image1;
	if ( !image )
		return;
	method = 0;
	width = image->width;
	height = image->height;
	overlap = ( ( PasteUpParms * ) p->ts )->overlap;
	for ( i = 0; i != reps; i++ )
	{
                XieExecutePhotoflo( xp->d, flo, flo_notify );
		WaitForFloToFinish( xp, flo );
		XSync( xp->d, 0 );
		BuildMeSomeTiles( tiles, SPLIT, width / SPLIT,
			height / SPLIT, method++, overlap );
		if ( method == 2 )
			method = 0;
		XieFloPasteUp( &flograph[ pasteUpIdx ], width, height, 
			constant, tiles, NTILES );
		XieModifyPhotoflo( xp->d, flo, pasteUpIdx + 1, &flograph[pasteUpIdx], 1 );
		XSync( xp->d, 0 );
    	}
}

int EndPasteUp(xp, p)
    XParms  xp;
    Parms   p;
{
	int	i;

	if ( p->data )
	{
		free( p->data );
		p->data = ( char * ) NULL;
	}
	if ( parms )
		free( parms );
	XieDestroyPhotomap( xp->d, XIEPhotomap );
	for ( i = 0; i < NTILES; i++ )
		XieDestroyPhotomap( xp->d, segments[ i ] );
        XieFreePhotofloGraph(flograph,flo_elements);
        XieDestroyPhotoflo( xp->d, flo );
}

