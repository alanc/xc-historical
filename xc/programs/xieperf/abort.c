/* $XConsortium$ */

/**** module do_abort.c ****/
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
  
	do_abort.c -- abort flo test 

	Syd Logan -- AGE Logic, Inc. July, 1993 - MIT Alpha release
  
*****************************************************************************/
#include "xieperf.h"
#include <stdio.h>

static XieLut XIELut;
static XiePhotoElement *flograph;
static XiePhotoflo flo;

int InitAbort(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
        XieDataClass    class;
        XieOrientation  band_order;
        XieLTriplet     length, levels;
        Bool    	merge;
        XieLTriplet     start;

	if ( !(XIELut = XieCreateLUT( xp->d ) ) )
	{
		fprintf( stderr, "XieCreateLUT failed\n" );
		return( 0 );
	}

	/* set up a flo to read a lut from client */

        flograph = XieAllocatePhotofloGraph(2);
        if ( flograph == ( XiePhotoElement * ) NULL )
        {
                fprintf(stderr,"XieAllocatePhotofloGraph failed\n");
		XieDestroyLUT( xp->d, XIELut );
		return( 0 );
        }

        class = xieValSingleBand;
        band_order = xieValLSFirst;
        length[ 0 ] = 1 << p->levelsIn;
        length[ 1 ] = 0;
        length[ 2 ] = 0;
        levels[ 0 ] = 1 << p->levelsOut;
        levels[ 1 ] = 0;
        levels[ 2 ] = 0;

        XieFloImportClientLUT(&flograph[0],
                class,
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
	return( reps );
}

void DoAbort(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
	int	i;

    	for (i = 0; i != reps; i++) {

		/* now, the flo should be active and waiting for data */
		/* XXX should verify this? */

		XieExecutePhotoflo( xp->d, flo, True );
		if ( !AbortAndWaitForEvent( xp, p, 0, flo ) )
			break;
    	}
}

void EndAbort(xp, p)
    XParms  xp;
    Parms   p;
{
        XieFreePhotofloGraph(flograph,2);
        XieDestroyPhotoflo( xp->d, flo );
	XieDestroyLUT( xp->d, XIELut );
}

static int
AbortAndWaitForEvent( xp, p, namespace, flo_id )
XParms	xp;
Parms	p;
unsigned long namespace; 
unsigned long flo_id;
{
	XieAbort( xp->d, namespace, flo_id );
	return( WaitForFloToFinish( xp, flo_id ) );
}

