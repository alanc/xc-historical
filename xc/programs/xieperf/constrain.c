/* $XConsortium: constrain.c,v 1.1 93/07/19 13:02:11 rws Exp $ */

/**** module do_constrain.c ****/
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
  
	do_constrain.c -- constrain flo element tests 

	Syd Logan -- AGE Logic, Inc. July, 1993 - MIT Alpha release
  
*****************************************************************************/
#include "xieperf.h"
#include <stdio.h>

static XiePhotomap XIEPhotomap;

int InitConstrainFlo(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
	p->data = ( char * ) NULL;
	if ( ( XIEPhotomap = GetXIEPhotomap( xp, p, 1 ) ) == ( XiePhotomap ) NULL )
		reps = 0;
	return( reps );
}

void DoConstrainFloMapImmediate(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    	int     i;
	int	flo_notify, flo_id;
	XiePhotospace photospace;
	XiePhotoElement *flograph;
	int	decode_notify;
	XieLTriplet levels;
	XieConstrainTechnique   tech;
	char    *tech_parms=NULL;
        XieClipScaleParam *parms;
        XieConstant in_low,in_high;
        XieLTriplet out_low,out_high;
	int	InClampDelta = 0;
	int	OutClampDelta = 0;

	decode_notify = False;
	photospace = XieCreatePhotospace(xp->d);/* XXX error check */

	tech = ( ( ConstrainParms * ) p->ts )->constrain;

	levels[ 0 ] = 1 << xp->vinfo.depth;
	levels[ 1 ] = 0;
	levels[ 2 ] = 0;

	in_low[ 0 ] = 0.0;
	in_high[ 0 ] = ( float ) ( 1 << xp->vinfo.depth ) - 1.0;
	out_low[ 0 ] = 0;
	out_high[ 0 ] = ( 1 << xp->vinfo.depth ) - 1;

	if ( ( ( ConstrainParms * ) p->ts )->clamp & ClampInputs )
	{
		InClampDelta = levels[ 0 ] / reps;
	}
	if ( ( ( ConstrainParms * ) p->ts )->clamp & ClampOutputs )
	{
		OutClampDelta = out_high[ 0 ] / reps;
	}	
	flograph = XieAllocatePhotofloGraph(3);	
	if ( flograph == ( XiePhotoElement * ) NULL )
	{
		fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
		return;
	}
	XieFloImportPhotomap(&flograph[0],XIEPhotomap,decode_notify);

       	XieFloExportDrawable(&flograph[2],
               	2,              /* source phototag number */
               	xp->w,
               	xp->fggc,
               	0,       /* x offset in window */
               	0        /* y offset in window */
       	);
	flo_notify = False;	
    	for (i = 0; i != reps; i++) {

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
				return;
			}
			in_low[ 0 ] += ( float ) InClampDelta;
			in_high[ 0 ] -= ( float ) InClampDelta;
			out_low[ 0 ] += OutClampDelta;
			out_high[ 0 ] -= OutClampDelta;
		}

		XieFloConstrain( &flograph[1], 
			1,
			levels,
			tech,
			tech_parms
		);

		flo_id = i + 1;		
        	XieExecuteImmediate(xp->d, photospace,
                	flo_id,		
                	flo_notify,     
                	flograph,       /* photoflo specification */
                	3               /* number of elements */
        	);
    	}
	XieDestroyPhotospace( xp->d, photospace );
	XieFreePhotofloGraph(flograph,3);	
}

void DoConstrainFloMapStored(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    	int     i;
	int	flo_notify, decode_notify;
	XiePhotoElement *flograph;
	XiePhotoflo flo;
	XieLTriplet levels;
	XieConstrainTechnique   tech;
	char    *tech_parms=NULL;
        XieClipScaleParam *parms;
        XieConstant in_low,in_high;
        XieLTriplet out_low,out_high;

	decode_notify = False;
	tech = ( ( ConstrainParms * ) p->ts )->constrain;

	levels[ 0 ] = 1 << xp->vinfo.depth;
	levels[ 1 ] = 0;
	levels[ 2 ] = 0;

	in_low[ 0 ] = 0.0;
	in_high[ 0 ] = ( float ) ( 1 << xp->vinfo.depth ) - 1.0 ;
	out_low[ 0 ] = 0;
	if ( xp->vinfo.depth == 1 ) 
		out_high[ 0 ] = 1;
	else
		out_high[ 0 ] = ( 1 << xp->vinfo.depth ) - 1;
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
			return;
		}
	}

	flograph = XieAllocatePhotofloGraph(3);	
	if ( flograph == ( XiePhotoElement * ) NULL )
	{
		fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
		return;
	}

	XieFloImportPhotomap(&flograph[0],XIEPhotomap,decode_notify);

	XieFloConstrain( &flograph[1], 
		1,
		levels,
		tech,
		tech_parms
	);
					
       	XieFloExportDrawable(&flograph[2],
               	2,              /* source phototag number */
               	xp->w,
               	xp->fggc,
               	0,       /* x offset in window */
               	0        /* y offset in window */
       	);

	flo = XieCreatePhotoflo( xp->d, flograph, 3 );

	/* crank it */

	flo_notify = False;
    	for (i = 0; i != reps; i++) {
		XieExecutePhotoflo( xp->d, flo, flo_notify );
		XSync( xp->d, 0 );
    	}

	/* destroy the photoflo */

	XieFreePhotofloGraph(flograph,3);	
	XieDestroyPhotoflo( xp->d, flo );
}

void EndConstrainFlo(xp, p)
    XParms  xp;
    Parms   p;
{
	/* deallocate the data buffer */

	if ( p->data )
	{
		free( p->data );
		p->data = ( char * ) NULL;
	}

	/* destroy the photomap */

        XieDestroyPhotomap(xp->d, XIEPhotomap);

}
