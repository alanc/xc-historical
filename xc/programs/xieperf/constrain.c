/* $XConsortium: constrain.c,v 1.2 93/10/27 21:52:00 rws Exp $ */

/**** module constrain.c ****/
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
  
	constrain.c -- constrain flo element tests 

	Syd Logan -- AGE Logic, Inc. July, 1993 - MIT Alpha release
  
*****************************************************************************/
#include "xieperf.h"
#include <stdio.h>

static XiePhotomap XIEPhotomap;
static XiePhotomap XIEPhotomap2;
static int flo_notify;
static XiePhotoElement *flograph;
static XiePhotoflo flo;
static int flo_elements;
static XieClipScaleParam *parms;

int InitConstrain(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{

	XieLTriplet levels;
	XieConstrainTechnique tech;
	char *tech_parms=NULL;
        XieConstant in_low,in_high;
        XieLTriplet out_low,out_high;
	int InClampDelta = 0;
	int OutClampDelta = 0;
	int decode_notify;
	XIEimage *image;
	Bool photoDest;
        XieEncodeTechnique encode_tech=xieValEncodeServerChoice;
        char *encode_params=NULL;

	image = p->finfo.image1;
        XIEPhotomap = ( XiePhotomap ) NULL;
        XIEPhotomap2 = ( XiePhotomap ) NULL;
        parms = ( XieClipScaleParam * ) NULL;
        flograph = ( XiePhotoElement * ) NULL;
        flo = ( XiePhotoflo ) NULL;

	tech = ( ( ConstrainParms * ) p->ts )->constrain;
        photoDest = ( ( ConvolveParms * )p->ts )->photoDest;

	XIEPhotomap2 = XieCreatePhotomap( xp->d );

        if ( TechniqueSupported( xp, xieValConstrain, tech ) == False )
                reps = 0;
	else if ( xp->vinfo.depth < 8 && tech == xieValConstrainHardClip )
	{
		if ( ( XIEPhotomap = GetXIEDitheredPhotomap( xp, p, 1, 
			1 << xp->vinfo.depth ) ) == 
				( XiePhotomap ) NULL )
			reps = 0;
	}
	else 
	{
		if ( ( XIEPhotomap = GetXIEPhotomap( xp, p, 1 ) ) == 
        		( XiePhotomap ) NULL )
        		reps = 0;
	}
	if ( reps )
	{
		decode_notify = False;
		flo_elements = 3;

		levels[ 0 ] = 1 << xp->vinfo.depth;
		levels[ 1 ] = 0;
		levels[ 2 ] = 0;
		out_high[ 0 ] = ( 1 << xp->vinfo.depth ) - 1;
		out_high[ 1 ] = 0; 
		out_high[ 2 ] = 0; 

		if ( levels[ 0 ] == 256 && 
			( ( ConstrainParms * ) p->ts )->clamp & ClampInputs )
		{
			InClampDelta = levels[ 0 ] / 2;
		}
		else
		{
			InClampDelta = 0;
		}
		if ( levels[ 0 ] == 256 && 
			( ( ConstrainParms * ) p->ts )->clamp & ClampOutputs )
		{
			OutClampDelta = out_high[ 0 ] / 2;
		}	
		else
		{
			OutClampDelta = 0;
		}
		in_low[ 0 ] = 0.0 + ( float ) InClampDelta; 
		in_low[ 1 ] = 0.0; 
		in_low[ 2 ] = 0.0; 
		in_high[ 0 ] = ( ( float ) image->levels[ 0 ] - 1.0 ) -
			( float ) InClampDelta;
		in_high[ 1 ] = 0.0; 
		in_high[ 2 ] = 0.0; 
		out_low[ 0 ] = 0 + OutClampDelta;
		out_low[ 1 ] = 0;
		out_low[ 2 ] = 0;
		out_high[ 0 ] = out_high[ 0 ] - OutClampDelta;
		out_high[ 1 ] = 0; 
		out_high[ 2 ] = 0; 

		flograph = XieAllocatePhotofloGraph( flo_elements );	
		if ( flograph == ( XiePhotoElement * ) NULL )
		{
			fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
			reps = 0;
		}
	}
	if ( reps )
	{
		XieFloImportPhotomap(&flograph[0],XIEPhotomap,decode_notify);

		if ( photoDest == False )
		{
			XieFloExportDrawable(&flograph[2],
				2,              /* source phototag number */
				xp->w,
				xp->fggc,
				0,       /* x offset in window */
				0        /* y offset in window */
			);
		}
		else
		{
                        XieFloExportPhotomap(&flograph[2],
                                2,              /* source phototag number */
                                XIEPhotomap2,
                                encode_tech,
                                encode_params
                        );
		}

		if ( tech == xieValConstrainHardClip )
		{
			tech_parms = ( char * ) NULL;
		}
		else
		{
			parms = XieTecClipScale( in_low, in_high,
				out_low, out_high);
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
		XieFloConstrain( &flograph[1], 
			1,
			levels,
			tech,
			tech_parms
		);

		flo_notify = False;	
		flo = XieCreatePhotoflo( xp->d, flograph, flo_elements ); 
	}
	if ( !reps )
		FreeConstrainStuff( xp, p );
	return( reps );
}

void DoConstrain(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    	int     i;

    	for (i = 0; i != reps; i++) {
        	XieExecutePhotoflo(xp->d, flo, flo_notify );
    	}
}

void EndConstrain(xp, p)
    XParms  xp;
    Parms   p;
{
	FreeConstrainStuff( xp, p );
}

int
FreeConstrainStuff( xp, p )
XParms	xp;
Parms	p;
{
        if ( XIEPhotomap && IsPhotomapInCache( XIEPhotomap ) == False )
        {
                XieDestroyPhotomap( xp->d, XIEPhotomap );
                XIEPhotomap = ( XiePhotomap ) NULL;
        }

        if ( XIEPhotomap2 ) 
        {
                XieDestroyPhotomap( xp->d, XIEPhotomap2 );
                XIEPhotomap2 = ( XiePhotomap ) NULL;
        }

        if ( parms )
        {
                free( parms );
                parms = ( XieClipScaleParam * ) NULL;
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
