/* $XConsortium: dither.c,v 1.1 93/10/26 10:05:59 rws Exp $ */

/**** module dither.c ****/
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
  
	dither.c -- dither flo element tests 

	Syd Logan -- AGE Logic, Inc. July, 1993 - MIT Alpha release
  
*****************************************************************************/
#include "xieperf.h"
#include <stdio.h>

static XiePhotomap XIEPhotomap;
static int flo_notify, flo_id;
static XiePhotospace photospace;
static XiePhotoElement *flograph;
static int decode_notify;
static XieLTriplet levels;
static XieConstant in_low,in_high;
static XieLTriplet out_low,out_high;
static char *dithertech_parms=NULL;
static XieClipScaleParam *parms;
static int maxlevels;
static int flo_elements;

int InitDither(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
	int	threshold;

	photospace = ( XiePhotospace ) NULL;
	XIEPhotomap = ( XiePhotomap ) NULL;
	parms = ( XieClipScaleParam * ) NULL; 
	dithertech_parms = ( char * ) NULL;
	flograph = ( XiePhotoElement * ) NULL; 
	maxlevels = xp->vinfo.depth;

	if ( ( ( DitherParms * ) p->ts )->dither != xieValDitherDefault )
	{
		if ( TechniqueSupported( xp, xieValDither, 
			( ( DitherParms * ) p->ts )->dither ) == False )
		{
			fprintf( stderr, "Dither technique %d not supported\n", 
				( ( DitherParms * ) p->ts )->dither );
			reps = 0;
		}
	}
	if ( reps )
	{
        	if ( ( XIEPhotomap = GetXIEPhotomap( xp, p, 1 ) ) == 
			( XiePhotomap ) NULL )
		{
			fprintf( stderr, "Couldn't get photomap\n" );
			reps = 0;
		}
	}
	if ( reps )
	{
		photospace = XieCreatePhotospace(xp->d);
		threshold = ( ( DitherParms * )p->ts )->threshold;
		decode_notify = False;
		levels[ 0 ] = levels[ 1 ] = levels[ 2 ] = 0;
		in_low[ 0 ] = in_low[ 1 ] = in_low[ 2 ] = 0.0;
		out_low[ 0 ] = out_low[ 1 ] = out_low[ 2 ] = 0;
		out_high[ 0 ] = out_high[ 1 ] = out_high[ 2 ] = 0;
		if ( ( ( DitherParms * )p->ts )->drawable == Drawable )
			out_high[ 0 ] = ( 1 << xp->vinfo.depth ) - 1;
		else
			out_high[ 0 ] = 1;
		flo_elements = 4;
		flograph = XieAllocatePhotofloGraph(flo_elements);	
		if ( flograph == ( XiePhotoElement * ) NULL )
		{
			fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
			reps = 0;
		}
	}
	if ( reps )
	{
		flo_notify = False;	
		XieFloImportPhotomap(&flograph[0],XIEPhotomap, decode_notify);

		if ( ( ( DitherParms * ) p->ts )->dither == xieValDitherOrdered )
		{	 
			dithertech_parms = ( char * ) 
				XieTecDitherOrderedParam(threshold); 
			if ( dithertech_parms == ( char * ) NULL )
			{
				fprintf( stderr, 
				"Trouble loading dither technique parameters\n" );
				reps = 0;
			}
		}
	}
	if ( !reps )
	{
		FreeDitherStuff( xp, p );
	}
        return( reps );
}

void DoDither(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
	int	i, j, idx;
	char    *tech_parms=NULL;

	j = 0;
    	for (i = 0; i != reps; i++) {

		idx = 1;
		flo_id = i + 1;		
		if ( ( ( DitherParms * ) p->ts )->drawable == Drawable )
			j += 2;
		else
			j = 2;
		if ( j >= maxlevels )
			j = 2;
		levels[ 0 ] = j;

		XieFloDither( &flograph[ idx ], 
			idx,
			( ( DitherParms * ) p->ts )->bandMask,
			levels,
			( ( DitherParms * ) p->ts )->dither,
			dithertech_parms
		);
		idx++;

		in_high[ 0 ] =  ( float ) j - 1.0;
		in_high[ 1 ] =  0.0;
		in_high[ 2 ] =  0.0;
		if ( tech_parms )
		{
			free( tech_parms );
			tech_parms = ( char * ) NULL;
		}
		parms = XieTecClipScale( in_low, in_high, out_low, out_high);
		tech_parms = ( char * ) parms;
		if ( tech_parms == ( char * ) NULL )
		{
			fprintf( stderr, 
				"Trouble loading clipscale technique parameters\n" );
			return;
		}

		if ( levels[ 0 ] != xp->vinfo.depth && 
			( ( DitherParms * ) p->ts )->drawable == Drawable )
		{
			if ( ( ( DitherParms * ) p->ts )->drawable == Drawable )
				levels[ 0 ] = 1 << xp->vinfo.depth;
			else
				levels[ 0 ] = 2;

			XieFloConstrain( &flograph[idx], 
				idx,
				levels,
				xieValConstrainClipScale,
				tech_parms
			);
			idx++;
		}

		if ( ( ( DitherParms * ) p->ts )->drawable == Drawable )
			XieFloExportDrawable(&flograph[idx],
				idx,       /* source phototag number */
				xp->w,
				xp->fggc,
				0,       /* x offset in window */
				0        /* y offset in window */
			);
		else
			XieFloExportDrawablePlane(&flograph[idx],
				idx,       /* source phototag number */
				xp->w,
				xp->fggc,
				0,       /* x offset in window */
				0        /* y offset in window */
			);
		idx++;
       		XieExecuteImmediate(xp->d, photospace,
               		flo_id,		
               		flo_notify,     
               		flograph,       /* photoflo specification */
               		idx    		/* number of elements */
       		);
		XSync( xp->d, 0 );
    	}
	if ( tech_parms )
	{
		free( tech_parms );
		tech_parms = ( char * ) NULL;
	}
}

void 
EndDither(xp, p)
    XParms  xp;
    Parms   p;
{
	FreeDitherStuff( xp, p );
}

int
FreeDitherStuff( xp, p )
XParms	xp;
Parms	p;
{
	if ( XIEPhotomap && IsPhotomapInCache( XIEPhotomap ) == False )
	{
		XieDestroyPhotomap(xp->d, XIEPhotomap);
		XIEPhotomap = ( XiePhotomap ) NULL;
	}
	if ( dithertech_parms )
	{
		free( dithertech_parms );
		dithertech_parms = ( char * ) NULL;
	}
	if ( flograph != (XiePhotoElement *) NULL )
	{
		XieFreePhotofloGraph(flograph,4);	
		flograph = ( XiePhotoElement * ) NULL; 
	}
	if ( photospace )
	{
		XieDestroyPhotospace( xp->d, photospace );
		photospace = ( XiePhotospace ) NULL;
	}
}

