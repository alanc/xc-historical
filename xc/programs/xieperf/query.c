/* $XConsortium: query.c,v 1.6 93/11/06 15:06:19 rws Exp $ */

/**** module query.c ****/
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
     
     "Copyright 1993, 1994 by AGE Logic, Inc. and the Massachusetts
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
  
	query.c -- query flo element test 

	Syd Logan -- AGE Logic, Inc. July, 1993 - MIT Alpha release
  
*****************************************************************************/

#include "xieperf.h"
#include <stdio.h>

static XiePhotomap XIEPhotomap;
static XieLut   PhotofloTestLut1, PhotofloTestLut2;
static XieColorList XIEColorList;

static int flo_elements;
static XiePhotoElement *flograph;
static XiePhotoflo flo;

int InitQueryTechniques(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
	return reps;
}

int InitQueryColorList(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
	/* allocate a color list */

	if ( !(XIEColorList = XieCreateColorList( xp->d ) ) )
	{
		fprintf( stderr, "XieCreateColorList failed\n" );
		reps = 0;
	}
	return reps;
}

int InitQueryPhotomap(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
	if ( ( XIEPhotomap = 
		GetXIEPhotomap( xp, p, 1 ) ) == ( XiePhotomap ) NULL )
	{
		reps = 0;
	}
	return reps;
}

int InitQueryPhotoflo(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
        int     lutSize, lutLevels;
        unsigned char   *lut1, *lut2;
        Bool    merge;
        XieLTriplet start;

	flograph = ( XiePhotoElement * ) NULL;
	flo = ( XiePhotoflo ) NULL;
	lut1 = lut2 = ( unsigned char * ) NULL;
	PhotofloTestLut1 = PhotofloTestLut2 = ( XieLut ) NULL;

	flo_elements = 2;
        flograph = XieAllocatePhotofloGraph(flo_elements);
	lutSize = ( ( QueryParms * ) p->ts )->lutSize;
	lutLevels = ( ( QueryParms * ) p->ts )->lutLevels;
        if ( flograph == ( XiePhotoElement * ) NULL )
        {
                fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
		reps = 0;
        }
	else
	{
		lut1 = (unsigned char *)
			malloc( lutSize * sizeof( unsigned char ) );
		if ( lut1 == ( unsigned char * ) NULL )
			reps = 0;
		else
		{
			lut2 = (unsigned char *)
				malloc( lutSize * sizeof( unsigned char ) );
			if ( lut2 == ( unsigned char * ) NULL )
			{
				reps = 0;
			}
		}
        }
	
	if ( reps )
	{
		if ( ( PhotofloTestLut1 = GetXIELut( xp, p, lut1, lutSize,
			lutLevels ) ) == ( XieLut ) NULL )
		{
			reps = 0;
		}
		else if ( ( PhotofloTestLut2 = GetXIELut( xp, p, lut2, lutSize,
			lutLevels ) ) == ( XieLut ) NULL )
		{
			reps = 0;
		}
        }

	if ( reps )
	{

		XieFloImportLUT(&flograph[0], PhotofloTestLut1 );

		merge = False;
		start[ 0 ] = 0;
		start[ 1 ] = 0;
		start[ 2 ] = 0;

		XieFloExportLUT(&flograph[1],
			1,              /* source phototag number */
			PhotofloTestLut2,
			merge,
			start
		);
		flo = XieCreatePhotoflo( xp->d, flograph, flo_elements );
	}

	if ( !reps )
		FreeQueryPhotofloStuff( xp, p );

	if ( lut1 )
		free( lut1 );
	if ( lut2 )
		free( lut2 );

	return reps;
}

void DoQueryTechniques(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
	XieTechniqueGroup	techGroup;
	XieTechnique		*techVector;
	int			j, i, numTech;

	techGroup = ( ( QueryParms * ) p->ts )->techGroup;

	for ( i = 0; i < reps; i++ )
	{
		if ( !XieQueryTechniques( xp->d, techGroup, &numTech, 
				&techVector ) )
		{
			fprintf( stderr, "XieQueryTechniques: failed\n" );
			break;
		}

		/* this bites */

		for ( j = 0; j < numTech; j++ )
		{
			free( techVector[ j ].name );
		}
		free( techVector );
	}
}

void DoQueryColorList(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
	int	i;
	Colormap cmap;
	unsigned int ncolors;
	unsigned long *colors;

	for ( i = 0; i < reps; i++ )
	{
		colors = ( unsigned long * ) NULL;
		if ( !XieQueryColorList( xp->d, XIEColorList, &cmap, 
			&ncolors, &colors ) )
		{
			fprintf( stderr, "XieQueryColorList failed\n" );
			break;
		}
		
		/* for alpha, the colorlist is empty. This will change
		   later */

		if ( cmap != ( Colormap ) 0 )
		{
			fprintf( stderr, "XieQueryColorList returned non-zero colormap\n" );
			break;
		}
		if ( ncolors != 0 )
		{
			fprintf( stderr, "XieQueryColorList returned non-zero ncolors\n" );
			break;
		}
		if ( colors != ( unsigned long * ) NULL )
		{
			fprintf( stderr, "XieQueryColorList returned non-NULL colorlist\n" );
			break;
		}
	}
}

void DoQueryPhotomap(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
	int	i;
	XieLTriplet	width;
	XieLTriplet	height;
	XieLTriplet	levels;
	XieDataType	data_type;
	XieDataClass	cclass;
	Bool 	pop, error;
	XieDecodeTechnique decode;
        XIEimage *image;

        image = p->finfo.image1;
        if ( !image )
                return;

	error = False;
	for ( i = 0; i < reps && error == False; i++ )
	{
		if ( !XieQueryPhotomap( xp->d, XIEPhotomap, &pop, &data_type,
			&cclass, &decode, width, height, levels ) )
		{
			fprintf( stderr, "XieQueryPhotomap failed\n" );
			fflush( stderr );
			error = True;
		}
		if ( levels[ 0 ] != image->levels[ 0 ] )
		{
			fprintf( stderr, "XieQueryPhotomap levels return invalid should be 0x%x got 0x%x\n", image->levels[ 0 ], levels[ 0 ] );
			fflush( stderr );
			error = True;
		}
		if ( width[ 0 ] != image->width[ 0 ] )
		{
			fprintf( stderr, "XieQueryPhotomap width return invalid should be 0x%x got 0x%x\n", image->width[ 0 ], width[ 0 ] );
			fflush( stderr );
			error = True;
		}
		if ( height[ 0 ] != image->height[ 0 ] )
		{
			fprintf( stderr, "XieQueryPhotomap height return invalid should be 0x%x got 0x%x\n", image->height[ 0 ], height[ 0 ] );
			fflush( stderr );
			error = True;
		}
	}
}

void DoQueryPhotoflo(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
	int	i;
	XiePhotofloState state;
	XiePhototag *expected, *avail;
	unsigned int nexpected, navail;

	for ( i = 0; i < reps; i++ )
	{
		if ( !XieQueryPhotoflo( xp->d, 0, flo, &state, &expected,
			&nexpected, &avail, &navail ) )
		{
			fprintf( stderr, "XieQueryPhotoflo failed\n" );
		}
		if ( expected )
			free( expected );
		if ( avail )
			free( avail );
	}
}

int 
EndQueryTechniques(xp, p)
    XParms  xp;
    Parms   p;
{
}

int 
EndQueryColorList(xp, p)
    XParms  xp;
    Parms   p;
{
	FreeQueryColorListStuff( xp, p );
}

int
FreeQueryColorListStuff( xp, p )
XParms	xp;
Parms	p;
{
	if ( XIEColorList )
	{
		XieDestroyColorList( xp->d, XIEColorList );
		XIEColorList = ( XieColorList ) NULL;
	}
}

int 
EndQueryPhotomap(xp, p)
    XParms  xp;
    Parms   p;
{
	FreeQueryPhotomapStuff( xp, p );
}

int
FreeQueryPhotomapStuff( xp, p )
XParms	xp;
Parms	p;
{
	if ( XIEPhotomap && IsPhotomapInCache( XIEPhotomap ) == False )
	{
		XieDestroyPhotomap( xp->d, XIEPhotomap );
		XIEPhotomap = ( XiePhotomap ) NULL;
	}
}

int 
EndQueryPhotoflo(xp, p)
    XParms  xp;
    Parms   p;
{
	FreeQueryPhotofloStuff( xp, p );
}

int
FreeQueryPhotofloStuff( xp, p )
XParms	xp;
Parms	p;
{
	if ( PhotofloTestLut1 )
	{
		XieDestroyLUT( xp->d, PhotofloTestLut1 );
		PhotofloTestLut1 = ( XieLut ) NULL;
	}

	if ( PhotofloTestLut2 )
	{
		XieDestroyLUT( xp->d, PhotofloTestLut2 );
		PhotofloTestLut2 = ( XieLut ) NULL;
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


