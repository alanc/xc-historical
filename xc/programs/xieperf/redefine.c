/* $XConsortium: redefine.c,v 1.5 93/11/06 15:06:33 rws Exp $ */

/**** module redefine.c ****/
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
  
	redefine.c -- redefine flo element test 

	Syd Logan -- AGE Logic, Inc. July, 1993 - MIT Alpha release
  
*****************************************************************************/
#include "xieperf.h"
#include <stdio.h>

static int BuildRedefineFlograph();

static XiePhotomap XIEPhotomap;
XieLut XIELut;
static int monoflag = 0;
static XiePhotoElement *flograph1;
static XiePhotoElement *flograph2;
static XiePhotoflo flo;
static int flo_elements;
extern Bool dontClear;

int InitRedefine(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
	XIEimage *image;

	XIELut = ( XieLut ) NULL;
	XIEPhotomap = ( XiePhotomap ) NULL;
        flo = ( XiePhotoflo ) NULL;
        flograph1 = ( XiePhotoElement * ) NULL;
        flograph2 = ( XiePhotoElement * ) NULL;

	image = p->finfo.image1;
        if ( !image )
		reps = 0;
	else
	{
		monoflag = 0;
		if ( xp->screenDepth != image->depth[ 0 ] )
		{
			flo_elements = 4;
			monoflag = 1;
			if ( ( XIELut = CreatePointLut( xp, p, 
				1 << image->depth[ 0 ],
				1 << xp->screenDepth, False ) ) == ( XieLut ) NULL )
			{
				reps = 0;
			}
		}	
		else
			flo_elements = 2;
	}
	if ( reps )
	{
		if ( ( XIEPhotomap = 
			GetXIEPhotomap( xp, p, 1 ) ) == ( XiePhotomap ) NULL )
			reps = 0;
		else if ( !BuildRedefineFlograph( xp, p, &flograph1, 0 ) )
		{
			reps = 0;
		}
		else if ( !BuildRedefineFlograph( xp, p, &flograph2, 1 ) )
		{
			reps = 0;
		}

		/* start off with flograph 1 */

	        flo = XieCreatePhotoflo( xp->d, flograph1, flo_elements );
	}
	if ( !reps )
		FreeRedefineStuff( xp, p );
	else
		dontClear = False;
	return( reps );
}

static int
BuildRedefineFlograph( xp, p, flograph, which )
XParms	xp;
Parms	p;
XiePhotoElement **flograph;
int	which;
{
	XieProcessDomain domain;

	*flograph = XieAllocatePhotofloGraph(flo_elements);	
	if ( *flograph == ( XiePhotoElement * ) NULL )
	{
		fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
		return( 0 );
	}

	XieFloImportPhotomap(&(*flograph)[0], XIEPhotomap, False);

	domain.offset_x = 0;
	domain.offset_y = 0;
	domain.phototag = 0;

	if ( monoflag )
	{
		XieFloImportLUT(&(*flograph)[1], XIELut );

		XieFloPoint(&(*flograph)[2],
			1,
			&domain,
			2,
			0x1
		);
	}
	XieFloExportDrawable(&(*flograph)[flo_elements - 1],
		flo_elements - 1, /* source phototag number */
		xp->w,
		xp->fggc,
		( which ? 0 : 100 ),       /* x offset in window */
		( which ? 100 : 0 )        /* y offset in window */
	);
	return( 1 );
}

void DoRedefine(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    	int     i;
	int	toggle;

	toggle = 0;
    	for (i = 0; i != reps; i++) {
                XieExecutePhotoflo( xp->d, flo, False );
		if ( toggle == 0 )
		{
			XieRedefinePhotoflo( xp->d, flo, flograph1, flo_elements );
			toggle = 1;
		}
		else
		{
			XieRedefinePhotoflo( xp->d, flo, flograph2, flo_elements );
			toggle = 0;
		}
		XSync( xp->d, 0 );
    	}
}

int 
EndRedefine(xp, p)
    XParms  xp;
    Parms   p;
{
	dontClear = True;
	FreeRedefineStuff( xp, p );
}

int
FreeRedefineStuff( xp, p )
XParms	xp;
Parms	p;
{
        if ( XIEPhotomap && IsPhotomapInCache( XIEPhotomap ) == False )
        {
                XieDestroyPhotomap( xp->d, XIEPhotomap );
                XIEPhotomap = ( XiePhotomap ) NULL;
        }
	if ( XIELut )
	{
                XieDestroyLUT( xp->d, XIELut );
                XIELut = ( XieLut ) NULL;
	}
        if ( flo )
        {
                XieDestroyPhotoflo( xp->d, flo );
                flo = ( XiePhotoflo ) NULL;
        }
        if ( flograph1 )
        {
                XieFreePhotofloGraph(flograph1,flo_elements);
                flograph1 = ( XiePhotoElement * ) NULL;
        }
        if ( flograph2 )
        {
                XieFreePhotofloGraph(flograph2,flo_elements);
                flograph2 = ( XiePhotoElement * ) NULL;
        }
}
