/* $XConsortium: creatdstry.c,v 1.4 93/11/06 15:01:59 rws Exp $ */

/**** module creatdstry.c ****/
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
  
	creatdstry.c -- create/destroy resource tests

	Syd Logan -- AGE Logic, Inc. July, 1993 - MIT Alpha release
  
*****************************************************************************/
#include "xieperf.h"
#include <stdio.h>

static XieLut	PhotofloTestLut1, PhotofloTestLut2;
static XiePhotoElement *flograph;
static int flo_elements;

int InitCreateDestroyPhotoflo(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
	int	lutSize;
	unsigned char	*lut1, *lut2;
        Bool    merge;
        XieLTriplet start;

	lut1 = ( unsigned char * ) NULL;
	lut2 = ( unsigned char * ) NULL;
	PhotofloTestLut1 = PhotofloTestLut2 = ( XieLut ) NULL;
	flo_elements = 2;
        flograph = XieAllocatePhotofloGraph(flo_elements);

	lutSize = xp->vinfo.colormap_size;
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
	}
	if ( reps )
	{
		lut2 = (unsigned char *)
			malloc( lutSize * sizeof( unsigned char ) );
		if ( lut2 == ( unsigned char * ) NULL )
		{
			reps = 0;
		}
	}
	if ( reps )
	{
		if ( ( PhotofloTestLut1 = 
			GetXIELut( xp, p, lut1, lutSize, lutSize ) ) 
			== ( XieLut ) NULL )
		{
			reps = 0;
		}
        }
	if ( reps )
	{
		if ( ( PhotofloTestLut2 = 
			GetXIELut( xp, p, lut2, lutSize, lutSize ) ) 
			== ( XieLut ) NULL )
		{
			reps = 0;
		}
        }
	if ( lut1 )
	{
		free( lut1 );
	}
	if ( lut2 )
	{
		free( lut2 );
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
	}

	if ( !reps )
		FreeCreateDestroyPhotofloStuff( xp, p );
	return reps;

}

int InitCreateDestroy(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
	return( reps );
}

void DoCreateDestroyColorList(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
	int	i;
	XieColorList clist;

	for ( i = 0; i < reps; i++ )
	{
		if ( !( clist = XieCreateColorList( xp->d ) ) )
		{
			fprintf( stderr, "XieCreateColorList failed\n" );
			break;
		}
		else 
			XieDestroyColorList( xp->d, clist );
	}	
}

void DoCreateDestroyLUT(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
	XieLut	lut;
	int	i;

	for ( i = 0; i < reps; i++ )
	{
		if ( !(lut = XieCreateLUT( xp->d ) ) )
		{
			fprintf( stderr, "XieCreateLUT failed\n" );
			break;
		}
		else
			XieDestroyLUT( xp->d, lut );
	}
}

void DoCreateDestroyPhotomap(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
	XiePhotomap photomap;
	int	i;

	for ( i = 0; i < reps; i++ )
	{
		if ( !(photomap = XieCreatePhotomap( xp->d ) ) )
		{
			fprintf( stderr, "XieCreatePhotomap failed\n" );
			break;
		}
		else
			XieDestroyPhotomap( xp->d, photomap );
	}
}

void DoCreateDestroyROI(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
	XieRoi	roi;
	int	i;

	for ( i = 0; i < reps; i++ )
	{
		if ( !( roi = XieCreateROI( xp->d ) ) )
		{
			fprintf( stderr, "XieCreateROI failed\n" );
			break;
		}
		else 
			XieDestroyROI( xp->d, roi );
	}
}

void DoCreateDestroyPhotospace(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
	XiePhotospace photospace;
	int	i;

	for ( i = 0; i < reps; i++ )
	{
		if ( !( photospace = XieCreatePhotospace( xp->d ) ) )
		{
			fprintf( stderr, "XieCreatePhotospace failed\n" );
			break;
		}
		else
			XieDestroyPhotospace( xp->d, photospace );
	}
}

void DoCreateDestroyPhotoflo(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
	XiePhotoflo	flo;
	int	i;
	for ( i = 0; i < reps; i++ )
	{
        	if ( !( flo = XieCreatePhotoflo( xp->d, flograph, flo_elements ) ) )
		{
			fprintf( stderr, "XieCreatePhotoflo failed\n" );
			break;
		}
		else
			XieDestroyPhotoflo( xp->d, flo );
	}
}

void EndCreateDestroy(xp, p)
    XParms  xp;
    Parms   p;
{
	return;
}

void EndCreateDestroyPhotoflo(xp, p)
    XParms  xp;
    Parms   p;
{
	FreeCreateDestroyPhotofloStuff( xp, p );
}

int
FreeCreateDestroyPhotofloStuff( xp, p )
XParms	xp;
Parms	p;
{
	if ( flograph )
	{
                XieFreePhotofloGraph(flograph,flo_elements);
                flograph = ( XiePhotoElement * ) NULL;
        }

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
}
