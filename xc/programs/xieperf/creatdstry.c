/* $XConsortium: do_createdestroy.c,v 1.1 93/07/19 13:02:24 rws Exp $ */

/**** module do_createdestroy.c ****/
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
  
	do_createdestroy.c -- create/destroy resource tests

	Syd Logan -- AGE Logic, Inc. July, 1993 - MIT Alpha release
  
*****************************************************************************/
#include "xieperf.h"
#include <stdio.h>

static XieLut	PhotofloTestLut1, PhotofloTestLut2;
static XiePhotoElement *flograph;

int InitCreateDestroyPhotoflo(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
	int	lutSize;
	unsigned char	*lut1, *lut2;
        int     decode_notify;
        Bool    merge;
        XieLTriplet start;

        flograph = XieAllocatePhotofloGraph(2);
        if ( flograph == ( XiePhotoElement * ) NULL )
        {
                fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
                return 0;
        }

        lutSize = 1 << p->levelsIn;
        lut1 = (unsigned char *)malloc( lutSize * sizeof( unsigned char ) );
        if ( lut1 == ( unsigned char * ) NULL )
                return 0;
        lut2 = (unsigned char *)malloc( lutSize * sizeof( unsigned char ) );
        if ( lut2 == ( unsigned char * ) NULL )
	{
	        XieFreePhotofloGraph(flograph,2);
		free( lut1 );
		return( 0 );
	}
        if ( ( PhotofloTestLut1 = GetXIELut( xp, p, lut1, lutSize ) ) ==
		( XieLut ) NULL )
        {
	        XieFreePhotofloGraph(flograph,2);
		free( lut1 );
		free( lut2 );
                return 0;
        }
        if ( ( PhotofloTestLut2 = GetXIELut( xp, p, lut2, lutSize ) ) ==
		( XieLut ) NULL )
        {
	        XieFreePhotofloGraph(flograph,2);
		free( lut1 );
		free( lut2 );
		XieDestroyLUT( xp->d, PhotofloTestLut1 );
                return 0;
        }
	free( lut1 );
	free( lut2 );

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
        	if ( !( flo = XieCreatePhotoflo( xp->d, flograph, 2 ) ) )
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
}

void EndCreateDestroyPhotoflo(xp, p)
    XParms  xp;
    Parms   p;
{
        XieFreePhotofloGraph(flograph,2);
	XieDestroyLUT( xp->d, PhotofloTestLut1 );
	XieDestroyLUT( xp->d, PhotofloTestLut2 );
}
