/* $XConsortium: logical.c,v 1.3 93/10/27 21:52:28 rws Exp $ */

/**** module logical.c ****/
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
  
	logical.c -- logical flo element tests 

	Syd Logan -- AGE Logic, Inc. July, 1993 - MIT Alpha release
  
*****************************************************************************/
#include "xieperf.h"
#include <stdio.h>

static XiePhotomap XIEPhotomap1;
static XiePhotomap XIEPhotomap2;
static XieRoi XIERoi;
static XIEimage *image1, *image2;

static XieLTriplet levels;
static XieConstrainTechnique tech = xieValConstrainClipScale;
static XieClipScaleParam *parms;
static XieConstant in_low,in_high;
static XieLTriplet out_low,out_high;
static int monoflag = 0;

static XiePhotoflo flo;
static int flo_notify;
static XiePhotoElement *flograph;
static int flo_elements;
static XieRectangle *rects;

int InitLogical(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
        int i, idx;
	XieProcessDomain domain;
        unsigned int bandMask;
        Bool useROI;
	unsigned long logicalOp;
        int numROIs, src1, src2;
	XieConstant logicalConstant;

        useROI = ( ( LogicalParms * )p->ts )->useROI;
        numROIs = ( ( LogicalParms * )p->ts )->numROIs;
        bandMask = ( ( LogicalParms * )p->ts )->logicalBandMask;
	logicalOp = ( ( LogicalParms * )p->ts )->logicalOp;
	logicalConstant[ 0 ] = (( LogicalParms * )p->ts)->logicalConstant[ 0 ];
	logicalConstant[ 1 ] = (( LogicalParms * )p->ts)->logicalConstant[ 1 ];
	logicalConstant[ 2 ] = (( LogicalParms * )p->ts)->logicalConstant[ 2 ];

        image1 = p->finfo.image1;

        /* we must have image1. image2, however, is optional ( dyadic only ) */

        if ( !image1 )
		return( 0 );

        image2 = p->finfo.image2;

        if ( image2 )
                flo_elements = 4;
        else
                flo_elements = 3;

        if ( useROI == True )
                flo_elements++;

	monoflag = 0;
        parms = ( XieClipScaleParam * ) NULL;

        if ( xp->vinfo.depth != 8 )
        {
		monoflag = 1;
		flo_elements++;
		if ( SetupClipScale( xp, p, image1, levels, in_low, 
			in_high, out_low, out_high, &parms ) == 0 )
		{
			return( 0 );
		}
        }

	rects = ( XieRectangle * ) NULL;
	if ( useROI == True )
	{
		rects = (XieRectangle *)
			malloc( numROIs * sizeof( XieRectangle ) );
		if ( rects == ( XieRectangle * ) NULL )
			reps = 0;
		else
		{
			for ( i = 0; i < numROIs; i++ )
			{
				rects[ i ].x = i * 100;
				rects[ i ].y = i * 100;
				rects[ i ].width = 100;
				rects[ i ].height = 100;
			}
		}
        }

	flograph = ( XiePhotoElement * ) NULL;
	XIEPhotomap1 = XIEPhotomap2 = ( XiePhotomap ) NULL;
	XIERoi = ( XieRoi ) NULL;
	flo = ( XiePhotoflo ) NULL;
	if ( reps )
	{
		flograph = XieAllocatePhotofloGraph( flo_elements );
		if ( flograph == ( XiePhotoElement * ) NULL )
		{
			fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
			reps = 0;
		}
		else if ( ( XIEPhotomap1 =
			GetXIEPhotomap( xp, p, 1 ) ) == ( XiePhotomap ) NULL )
		{
			reps = 0;
		}
		else if ( image2 )
		{
			if ( ( XIEPhotomap2 = GetXIEPhotomap( xp, p, 2 ) ) == 
				( XiePhotomap ) NULL )
			{
				reps = 0;
			}
		}

		if ( useROI == True )
		{
			if ( ( XIERoi = GetXIERoi( xp, p, rects, numROIs ) ) ==
				( XieRoi ) NULL )
			{
				reps = 0;
			}
		}
	}
	if ( reps )
	{
		idx = 0;

		domain.offset_x = 0;
		domain.offset_y = 0;

		if ( useROI == True )
		{
			XieFloImportROI(&flograph[idx], XIERoi);
			idx++;
			domain.phototag = idx;
		}
                else
                {
                        domain.phototag = 0;
                }

                XieFloImportPhotomap(&flograph[idx], XIEPhotomap1, False );
                idx++;
                src1 = idx;

                if ( image2 )
                {
                        XieFloImportPhotomap(&flograph[idx], XIEPhotomap2,
                                False );
                        idx++;
                        src2 = idx;
                }
                else
                        src2 = 0;

                XieFloLogical(&flograph[idx],
                        src1,
                        src2,
                        &domain,
                        logicalConstant,
                        logicalOp,
                        bandMask
                );
                idx++;

		if ( monoflag )
		{
			XieFloConstrain( &flograph[ idx ],
				idx,
				levels,
				tech,
				( char * ) parms
			);
			idx++;
		}

                XieFloExportDrawable(&flograph[idx],
                        idx,            /* source phototag number */
                        xp->w,
                        xp->fggc,
                        0,       /* x offset in window */
                        0        /* y offset in window */
                );

                idx++;

                flo = XieCreatePhotoflo( xp->d, flograph, flo_elements );
                flo_notify = True;
        }
	if ( !reps )
	{
		FreeLogicalStuff( xp, p );
	}
        return( reps );
}

void DoLogical(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
        int     i;

        for (i = 0; i != reps; i++) {
		XieExecutePhotoflo( xp->d, flo, flo_notify );
		WaitForXIEEvent( xp, xieEvnNoPhotofloDone, flo, 0, False );	
                XSync( xp->d, 0 );
        }
}

int EndLogical(xp, p)
    XParms  xp;
    Parms   p;
{
	FreeLogicalStuff( xp, p );
}

int
FreeLogicalStuff( xp, p )
XParms	xp;
Parms	p;
{
	if ( XIEPhotomap1 && IsPhotomapInCache( XIEPhotomap1 ) == False )
	{
		XieDestroyPhotomap( xp->d, XIEPhotomap1 );
		XIEPhotomap1 = ( XiePhotomap ) NULL;
	}
	if ( XIEPhotomap2 && IsPhotomapInCache( XIEPhotomap2 ) == False )
	{
		XieDestroyPhotomap( xp->d, XIEPhotomap2 );
		XIEPhotomap1 = ( XiePhotomap ) NULL;
	}
	if ( rects )
	{
		free( rects );
		rects = ( XieRectangle * ) NULL;
	}
	if ( XIERoi )
	{
		XieDestroyROI( xp->d, XIERoi );
		XIERoi = ( XieRoi ) NULL;
	}
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
		XieFreePhotofloGraph(flograph,flo_elements);
		flograph = ( XiePhotoElement * ) NULL;
	}
}
