/* $XConsortium: uconstrain.c,v 1.1 93/10/26 10:07:22 rws Exp $ */

/**** module uconstrain.c ****/
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
  
	uconstrain.c -- unconstrain flo element test 

	Syd Logan -- AGE Logic, Inc. July, 1993 - MIT Alpha release
  
*****************************************************************************/

#include "xieperf.h"
#include <stdio.h>

static XiePhotomap XIEPhotomap;
static int flo_notify;
static XiePhotoElement *flograph;
static XiePhotoflo flo;
static XieClipScaleParam *parms;

int InitUnconstrain(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
	int decode_notify;
	XieLTriplet levels;
	XieConstrainTechnique tech;
	char *tech_parms=NULL;
        XieConstant in_low,in_high;
        XieLTriplet out_low,out_high;
	XIEimage *image;

	image = p->finfo.image1;

	flo = ( XiePhotoflo ) NULL;
	flograph = ( XiePhotoElement * ) NULL;
	parms = ( XieClipScaleParam * ) NULL;
	XIEPhotomap = ( XiePhotomap ) NULL;

	if ( ( XIEPhotomap = GetXIEPhotomap( xp, p, 1 ) ) == ( XiePhotomap ) NULL )
		reps = 0;
	else
	{
		tech = ( ( UnconstrainParms * )p->ts )->constrain;
		decode_notify = False;
		levels[ 0 ] = 1 << xp->vinfo.depth;
		levels[ 1 ] = 1 << xp->vinfo.depth;
		levels[ 2 ] = 1 << xp->vinfo.depth;

		in_low[ 0 ] = 0.0;
		in_low[ 1 ] = 0.0;
		in_low[ 2 ] = 0.0;
		in_high[ 0 ] = (1<<image->depth[ 0 ]) - 1.0;
		in_high[ 1 ] = (1<<image->depth[ 1 ]) - 1.0; 
		in_high[ 2 ] = (1<<image->depth[ 2 ]) - 1.0; 
		out_low[ 0 ] = 0;
		out_low[ 1 ] = 0;
		out_low[ 2 ] = 0;
		out_high[ 0 ] = levels[ 0 ] - 1;
		out_high[ 1 ] = levels[ 1 ] - 1;
		out_high[ 2 ] = levels[ 2 ] - 1;

		if ( tech == xieValConstrainHardClip )
		{
			tech_parms = ( char * ) NULL;
		}
		else
		{
			parms = XieTecClipScale( in_low, in_high, out_low, out_high);
			tech_parms = ( char * ) parms;
			if ( tech_parms == ( char * ) NULL )
			{
				fprintf( stderr, 
					"Trouble loading clipscale technique parameters\n" );
				reps = 0;
			}
		}
	}
	if ( reps )
	{
		flograph = XieAllocatePhotofloGraph(4);	
		if ( flograph == ( XiePhotoElement * ) NULL )
		{
			fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
			reps = 0;
		}
	}
	if ( reps )
	{
		XieFloImportPhotomap(&flograph[0],XIEPhotomap,decode_notify);

		XieFloUnconstrain(&flograph[1], 1 );

		XieFloConstrain( &flograph[2], 
			2,
			levels,
			tech,
			tech_parms
		);
					
		XieFloExportDrawable(&flograph[3],
			3,              /* source phototag number */
			xp->w,
			xp->fggc,
			0,       /* x offset in window */
			0        /* y offset in window */
		);

		flo = XieCreatePhotoflo( xp->d, flograph, 4 );

		flo_notify = False;
	}
	if ( !reps )
		FreeUnconstrainStuff( xp, p );
	return( reps );
}

void DoUnconstrain(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    	int     i;

    	for (i = 0; i != reps; i++) {
		XieExecutePhotoflo( xp->d, flo, flo_notify );
		XSync( xp->d, 0 );
    	}
}

void EndUnconstrain(xp, p)
    XParms  xp;
    Parms   p;
{
	FreeUnconstrainStuff( xp, p );
}

int
FreeUnconstrainStuff( xp, p )
XParms	xp;
Parms	p;
{
	if ( parms )
	{
		free( parms );
		parms = ( XieClipScaleParam * ) NULL;
	}
	if ( flo )
	{
		XieDestroyPhotoflo( xp->d, flo );
		flo = ( XiePhotoflo ) NULL;
	}	
	if ( flograph )
	{
		XieFreePhotofloGraph(flograph,4);	
		flograph = ( XiePhotoElement * ) NULL;
	}
	if ( XIEPhotomap && IsPhotomapInCache( XIEPhotomap ) == False )
	{
		XieDestroyPhotomap(xp->d, XIEPhotomap);
		XIEPhotomap = ( XiePhotomap ) NULL;
	}
}
