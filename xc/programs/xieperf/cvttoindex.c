/* $XConsortium: cvttoindex.c,v 1.2 93/07/19 14:43:41 rws Exp $ */

/**** module do_converttoindex.c ****/
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
  
	do_converttoindex.c -- converttoindex flo element tests 

	Syd Logan -- AGE Logic, Inc. July, 1993 - MIT Alpha release
  
*****************************************************************************/
#include "xieperf.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>

static XiePhotomap XIEPhotomap, ditheredPhotomap;
static XieColorList clist;

int InitConvertToIndex(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
	XiePhotospace photospace;
	XiePhotoElement *flograph;
	int	threshold;
	XieLTriplet levels;
	char    *dithertech_parms=NULL;
	int	flo_notify, flo_id;
	static XieEncodeTechnique encode_tech = xieValEncodeServerChoice;
	char	*encode_param = ( char * ) NULL;
	int	decode_notify;

	p->data = ( char * ) NULL;
	if ( !( clist = XieCreateColorList( xp->d ) ) )
		return( 0 );
        if ( ( XIEPhotomap = GetXIETriplePhotomap( xp, p, 1 ) ) == 
		( XiePhotomap ) NULL )
	{
		XieDestroyColorList( xp->d, clist );
                return( 0 );
	}

	if ( ( ditheredPhotomap = XieCreatePhotomap( xp->d ) ) == 
		( XiePhotomap ) NULL )
	{
		if ( p->data )
		{
			free( p->data );
			p->data = ( char * ) NULL;
		}
		XieDestroyPhotomap( xp->d, XIEPhotomap );	
		XieDestroyColorList( xp->d, clist );
		return( 0 );
	}

	photospace = XieCreatePhotospace(xp->d);

	threshold = 0;

	levels[ 0 ] = 6;
	levels[ 1 ] = 6;
	levels[ 2 ] = 6;

	flograph = XieAllocatePhotofloGraph(3);	
	if ( flograph == ( XiePhotoElement * ) NULL )
	{
		fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
		XieDestroyPhotomap( xp->d, XIEPhotomap );
		XieDestroyPhotomap( xp->d, ditheredPhotomap );
		XieDestroyColorList( xp->d, clist );
		XieDestroyPhotospace( xp->d, photospace );
		if ( p->data )
		{
			free( p->data );
			p->data = ( char * ) NULL;
		}
		return( 0 );
	}

	decode_notify = False;

	XieFloImportPhotomap(&flograph[0],XIEPhotomap, decode_notify);

	dithertech_parms = ( char * ) NULL;
	if ( ( ( CvtToIndexParms * ) p->ts )->dither == xieValDitherOrdered )
	{	 
		dithertech_parms = ( char * ) 
			XieTecDitherOrderedParam(threshold); 
		if ( dithertech_parms == ( char * ) NULL )
		{
			fprintf( stderr, 
			"Trouble loading dither technique parameters\n" );
			if ( p->data )
			{
				free( p->data );
				p->data = ( char * ) NULL;
			}
			XieDestroyPhotomap( xp->d, XIEPhotomap );	
			XieDestroyColorList( xp->d, clist );
			XieFreePhotofloGraph(flograph,3);	
			XieDestroyPhotospace( xp->d, photospace );
			return( 0 );
		}
	}

	XieFloDither( &flograph[ 1 ], 
		1,
		levels,
		( ( CvtToIndexParms * ) p->ts )->dither,
		dithertech_parms
	);

	XieFloExportPhotomap(&flograph[2],
		2,              /* source phototag number */
		ditheredPhotomap,
		encode_tech,
		encode_param
	);

	flo_id = 1;		
	flo_notify = True;	

      	XieExecuteImmediate(xp->d, photospace,
		flo_id,		
		flo_notify,     
		flograph,       /* photoflo specification */
		3               /* number of elements */
	);
	XSync( xp->d, 0 );
	WaitForFloToFinish( xp, flo_id );
	XieFreePhotofloGraph(flograph,3);	
	XieDestroyPhotospace( xp->d, photospace );
	UninstallXIECmap( xp->d );
	if ( dithertech_parms )
		free( dithertech_parms );
	return( reps );
}

void DoConvertToIndex(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
	int	i;
        XiePhotospace photospace;
        XiePhotoElement *flograph;
        int     flo_notify, flo_id;
	int	decode_notify;
	XieColorAllocAllParam *color_param = NULL;
	Colormap cmap, GetColormap();
	XWindowAttributes xwa;

       	photospace = XieCreatePhotospace(xp->d);/* XXX error check */
        flograph = XieAllocatePhotofloGraph(3);
        if ( flograph == ( XiePhotoElement * ) NULL )
        {
                fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
                return;
        }

	color_param = XieTecColorAllocAll( 123 );

	decode_notify = False;
        XieFloImportPhotomap(&flograph[0],ditheredPhotomap,decode_notify);

        XieFloExportDrawable(&flograph[2],
                2,              /* source phototag number */
                xp->w,
                xp->fggc,
                0,       /* x offset in window */
                0        /* y offset in window */
        );

	flo_id = 1;
	flo_notify = True;
	XGetWindowAttributes( xp->d, DefaultRootWindow( xp->d ), &xwa );
	for ( i = 0; i < reps; i++ )
	{
		XSetWindowColormap( xp->d, xp->w, xwa.colormap );
		XSync( xp->d, 0 );
		XieFloConvertToIndex(&flograph[1],
			1,
			xwa.colormap,	
			clist,	
			False,
			xieValColorAllocAll,
			(char *)color_param
		);
		
		XSync( xp->d, 0 );
                XieExecuteImmediate(xp->d, photospace,
                        flo_id,
                        flo_notify,
                        flograph,       /* photoflo specification */
                        3               /* number of elements */
                );
		XSync( xp->d, 0 );
		WaitForFloToFinish( xp, flo_id );
		XSync( xp->d, 0 );
               	flo_id = i + 1;
        }
        XieDestroyPhotospace( xp->d, photospace );
        XieFreePhotofloGraph(flograph,3);
	if ( color_param )
		free( color_param );
}


void EndConvertToIndex(xp, p)
    XParms  xp;
    Parms   p;
{
        /* deallocate the data buffer */

	if ( p->data )
	{
        	free( p->data );
		p->data = ( char * ) NULL;
	}

        /* destroy the photomaps */

        XieDestroyPhotomap(xp->d, XIEPhotomap);
        XieDestroyPhotomap(xp->d, ditheredPhotomap);

	/* destroy the ColorList */

	XieDestroyColorList( xp->d, clist );
	InstallXIECmap( xp->d );

}

