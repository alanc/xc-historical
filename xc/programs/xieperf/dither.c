/* $XConsortium: dither.c,v 1.2 93/07/26 14:14:55 rws Exp $ */

/**** module do_dither.c ****/
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
  
	do_dither.c -- dither flo element tests 

	Syd Logan -- AGE Logic, Inc. July, 1993 - MIT Alpha release
  
*****************************************************************************/
#include "xieperf.h"
#include <stdio.h>

static XiePhotomap XIEPhotomap;

int InitDitherFloMap(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
	p->data = ( char * ) NULL;

        if ( ( XIEPhotomap = GetXIEPhotomap( xp, p, 1 ) ) == ( XiePhotomap ) NULL )
                reps = 0;
        return( reps );
}

void DoDitherFloMapImmediate(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    	int     i, j;
	GC	pgc;
	int	flo_notify, flo_id;
	XiePhotospace photospace;
	XiePhotoElement *flograph;
	int	decode_notify;
	XieLTriplet levels;
	char    *tech_parms=NULL;
	char    *dithertech_parms=NULL;
        XieClipScaleParam *parms;
        XieConstant in_low,in_high;
        XieLTriplet out_low,out_high;
	int	threshold;

	photospace = XieCreatePhotospace(xp->d);/* XXX error check */

	threshold = 0;
	decode_notify = False;
	levels[ 1 ] = 0;
	levels[ 2 ] = 0;
	in_low[ 0 ] = 0.0;
	out_low[ 0 ] = 0;
	if ( ( ( DitherParms * )p->ts )->drawable == Drawable )
		out_high[ 0 ] = ( 1 << xp->vinfo.depth ) - 1;
	else
		out_high[ 0 ] = 1;
	pgc = xp->fggc;
	flograph = XieAllocatePhotofloGraph(4);	
	if ( flograph == ( XiePhotoElement * ) NULL )
	{
		fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
		return;
	}
	XieFloImportPhotomap(&flograph[0],XIEPhotomap, decode_notify);

	dithertech_parms = ( char * ) NULL;
	if ( ( ( DitherParms * ) p->ts )->dither == xieValDitherOrdered )
	{	 
		dithertech_parms = ( char * ) 
			XieTecDitherOrderedParam(threshold); 
		if ( dithertech_parms == ( char * ) NULL )
		{
			fprintf( stderr, 
			"Trouble loading dither technique parameters\n" );
			return;
		}
	}

	if ( ( ( DitherParms * ) p->ts )->drawable == Drawable )
		XieFloExportDrawable(&flograph[3],
			3,              /* source phototag number */
			xp->w,
			pgc,
			0,       /* x offset in window */
			0        /* y offset in window */
		);
	else
		XieFloExportDrawablePlane(&flograph[3],
			3,              /* source phototag number */
			xp->w,
			pgc,
			0,       /* x offset in window */
			0        /* y offset in window */
		);

	flo_notify = False;	
    	for (i = 0; i != reps; i++) {

		flo_id = i + 1;		
		if ( ( ( DitherParms * ) p->ts )->drawable == Drawable )
			j = i + 2;
		else
			j = 2;
		levels[ 0 ] = j;

		XieFloDither( &flograph[ 1 ], 
			1,
			levels,
			( ( DitherParms * ) p->ts )->dither,
			dithertech_parms
		);

		in_high[ 0 ] =  ( float ) j - 1.0;
		parms = XieTecClipScale( in_low, in_high, out_low, out_high);
		tech_parms = ( char * ) parms;
		if ( tech_parms == ( char * ) NULL )
		{
			fprintf( stderr, 
				"Trouble loading clipscale technique parameters\n" );
			return;
		}

		if ( ( ( DitherParms * ) p->ts )->drawable == Drawable )
			levels[ 0 ] = 1 << xp->vinfo.depth;
		else
			levels[ 0 ] = 2;

		XieFloConstrain( &flograph[2], 
			2,
			levels,
			xieValConstrainClipScale,
			tech_parms
		);

       		XieExecuteImmediate(xp->d, photospace,
               		flo_id,		
               		flo_notify,     
               		flograph,       /* photoflo specification */
               		4               /* number of elements */
       		);
		if ( tech_parms )
		    free( tech_parms );	

		XSync( xp->d, 0 );
    	}
	if ( dithertech_parms )
		free( dithertech_parms );
	XieFreePhotofloGraph(flograph,4);	
	XieDestroyPhotospace( xp->d, photospace );
}

void DoDitherFloMapStored(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    	int     i;
	int	flo_notify, decode_notify;
	XiePhotoElement *flograph;
	XiePhotoflo flo;
	XieLTriplet levels;
	char    *tech_parms=NULL;
        XieClipScaleParam *parms;
        XieConstant in_low,in_high;
        XieLTriplet out_low,out_high;
	int	threshold;

	threshold = 0;
	decode_notify = False;
	levels[ 0 ] = 2;
	levels[ 1 ] = 0;
	levels[ 2 ] = 0;

	in_low[ 0 ] = 0.0;
	in_high[ 0 ] = ( float ) 1;
	out_low[ 0 ] = 0;
        if ( ( ( DitherParms * )p->ts )->drawable == Drawable )
                out_high[ 0 ] = ( 1 << xp->vinfo.depth ) - 1;
        else
                out_high[ 0 ] = 1;
	flograph = XieAllocatePhotofloGraph(4);	
	if ( flograph == ( XiePhotoElement * ) NULL )
	{
		fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
		return;
	}

	XieFloImportPhotomap(&flograph[0],XIEPhotomap,decode_notify);

        tech_parms = ( char * ) NULL;
        if ( ( ( DitherParms * ) p->ts )->dither == xieValDitherOrdered )
        {
                tech_parms = ( char * )
                XieTecDitherOrderedParam(threshold);
                if ( tech_parms == ( char * ) NULL )
                {
                        fprintf( stderr,
                                "Trouble loading dither technique parameters\n" );
                        return;
                }
        }

	XieFloDither( &flograph[ 1 ],
                1,
                levels,
                ( ( DitherParms * ) p->ts )->dither,
                tech_parms
        );

	if ( tech_parms )
		free( tech_parms );
        parms = XieTecClipScale( in_low, in_high, out_low, out_high);
        tech_parms = ( char * ) parms;
        if ( tech_parms == ( char * ) NULL )
        {
                fprintf( stderr,
                       "Trouble loading clipscale technique parameters\n" );
                return;
        }

	if ( ( ( DitherParms * ) p->ts )->drawable == Drawable )
		levels[ 0 ] = 1 << xp->vinfo.depth;

        XieFloConstrain( &flograph[2],
                2,
                levels,
                xieValConstrainClipScale,
                tech_parms
	);

	if ( ( ( DitherParms * ) p->ts )->drawable == Drawable )
		XieFloExportDrawable(&flograph[3],
			3,              /* source phototag number */
			xp->w,
			xp->fggc,
			0,       /* x offset in window */
			0        /* y offset in window */
		);
	else
		XieFloExportDrawablePlane(&flograph[3],
			3,              /* source phototag number */
			xp->w,
			xp->fggc,
			0,       /* x offset in window */
			0        /* y offset in window */
		);

	flo = XieCreatePhotoflo( xp->d, flograph, 4 );

	if ( tech_parms )
		free( tech_parms );	

	/* crank it */

	flo_notify = 0;
    	for (i = 0; i != reps; i++) {
		XieExecutePhotoflo( xp->d, flo, flo_notify );
		XSync( xp->d, 0 );
    	}

	/* destroy the photoflo */

	XieDestroyPhotoflo( xp->d, flo );
	XieFreePhotofloGraph(flograph,4);	
}

void EndDitherFloMap(xp, p)
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

