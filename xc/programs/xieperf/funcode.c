/* $XConsortium$ */

/**** module funcode.c ****/
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
  
	funcode.c -- "Let's mess up the encode parameters and make sure
		      things are lossless" tests. Both SingleBand and
		      TripleBand.

	Syd Logan -- AGE Logic, Inc. September, 1993 - MIT Beta release
  
*****************************************************************************/
#include "xieperf.h"
#include <stdio.h>
#include <math.h>

static XiePhotomap XIEPhotomap;
static XiePhotomap XIEPhotomap2;

static int flo_notify;
static XiePhotoElement *flograph1, *flograph2;
static int flo1_elements;
static int flo2_elements;
static XiePhotoflo *flo;
static XieLut XIELut;
static char **encode_parms;
static int testSize;
static int type;
char *parms;
static XStandardColormap stdCmap;
extern Bool WMSafe;

int 
InitFunnyEncode(xp, p, reps)
XParms  xp;
Parms   p;
int     reps;
{
	int decode_notify, i, idx;
	XIEimage *image;
	extern Window drawableWindow;
	XieProcessDomain domain;
	XieEncodeTechnique encode_tech;
	XieEncodeUncompressedSingleParam *encode_single;
	XieEncodeUncompressedTripleParam *encode_triple;
	extern Bool dontClear;
	XieLTriplet levels;
	int cclass;
	int techParmOffset, floIdx;
	FunnyEncodeParms *ptr;
	int depthmismatch;
        XieConstant in_low,in_high;
        XieLTriplet out_low,out_high;
	XieConstant c1;
	float bias;
        GeometryParms gp;

	depthmismatch = 0;
	parms = ( char * ) NULL;
	image = p->finfo.image1;
	type = image->bandclass;
#if     defined(__cplusplus) || defined(c_plusplus)
        cclass = xp->vinfo.c_class;
#else
        cclass = xp->vinfo.class;
#endif
	ptr = ( FunnyEncodeParms * ) p->ts; 

        if ( type == xieValTripleBand && !IsColorVisual( cclass ) )
                return( 0 );

	if ( IsDISServer() )
		return( 0 );

	XIEPhotomap = ( XiePhotomap ) NULL;
	XIEPhotomap2 = ( XiePhotomap ) NULL;
	XIELut = ( XieLut ) NULL;
	flo = ( XiePhotoflo * ) NULL;
	flograph1 = ( XiePhotoElement * ) NULL;
	flograph2 = ( XiePhotoElement * ) NULL;
	encode_parms = ( char ** ) NULL;

	/* check for match errors in the ED */

	if ( type == xieValSingleBand )
	{	
		/* for single band, insert point element */

		if ( ptr->useMyLevelsPlease && ptr->myBits[ 0 ] != xp->vinfo.depth )
		{
			depthmismatch = 1;
			XIELut = CreatePointLut( xp, p, ptr->myBits[ 0 ], 
				xp->vinfo.depth );
		}
		else if ( xp->vinfo.depth != image->depth[ 0 ] )
		{
			depthmismatch = 1;
			XIELut = CreatePointLut( xp, p, image->depth[ 0 ],
                                xp->vinfo.depth ); 
		}
	}	
	
	if ( type == xieValTripleBand )
	{
	        if ( GetStandardColormap( xp, &stdCmap, XA_RGB_BEST_MAP ) == False )
                {
                        fprintf( stderr, "Couldn't get a standard colormap\n" );
                        fflush( stderr );
			reps = 0;
                }
	}
	if ( reps && type == xieValTripleBand )
	{
		if ( WMSafe == True )
			InstallThisColormap( xp->d, xp->p, stdCmap.colormap );
		else
		{
			InstallThisColormap( xp->d, xp->w, stdCmap.colormap );
			InstallThisColormap( xp->d, drawableWindow, stdCmap.colormap );
		}
	}

	if ( reps )
	{
		if ( type == xieValSingleBand ) 
		{
			if ( ptr->useMyLevelsPlease == True )
			{
				levels[ 0 ] = 1 << ptr->myBits[ 0 ];
				levels[ 1 ] = 1 << ptr->myBits[ 1 ];
				levels[ 2 ] = 1 << ptr->myBits[ 2 ];
				in_low[ 0 ] = 0;
				in_low[ 1 ] = 0;
				in_low[ 2 ] = 0;
				in_high[ 0 ] = ( 1 << image->depth[ 0 ] ) - 1;
				in_high[ 1 ] = ( 1 << image->depth[ 1 ] ) - 1;
				in_high[ 2 ] = ( 1 << image->depth[ 2 ] ) - 1;
				out_low[ 0 ] = 0.0;
				out_low[ 1 ] = 0.0;
				out_low[ 2 ] = 0.0;
				out_high[ 0 ] = ( float ) ( ( 1 << ptr->myBits[ 0 ] ) - 1 );
				out_high[ 1 ] = ( float ) ( ( 1 << ptr->myBits[ 1 ] ) - 1 );
				out_high[ 2 ] = ( float ) ( ( 1 << ptr->myBits[ 2 ] ) - 1 );
				XIEPhotomap = GetXIEConstrainedPhotomap( xp, p, 1,
					levels, xieValConstrainClipScale,
					in_low, in_high, out_low, out_high );
			}
			else
			{
				XIEPhotomap = GetXIEPhotomap( xp, p, 1 );
			}
			encode_tech = xieValEncodeUncompressedSingle;
		}
		else
		{
			encode_tech = xieValEncodeUncompressedTriple;

			gp.geoType = GEO_TYPE_SCALE;
			gp.geoHeight = 128;
			gp.geoWidth = 128;
			gp.geoXOffset = 0;
			gp.geoYOffset = 0;
			gp.geoTech = xieValGeomNearestNeighbor;

			if ( ptr->useMyLevelsPlease == True )
			{
				levels[ 0 ] = 1 << ptr->myBits[ 0 ];
				levels[ 1 ] = 1 << ptr->myBits[ 1 ];
				levels[ 2 ] = 1 << ptr->myBits[ 2 ];
				out_high[ 0 ] = ( float ) ( ( 1 << ptr->myBits[ 0 ] ) - 1 );
				out_high[ 1 ] = ( float ) ( ( 1 << ptr->myBits[ 1 ] ) - 1 );
				out_high[ 2 ] = ( float ) ( ( 1 << ptr->myBits[ 2 ] ) - 1 );
				in_low[ 0 ] = 0;
				in_low[ 1 ] = 0;
				in_low[ 2 ] = 0;
				in_high[ 0 ] = ( 1 << image->depth[ 0 ] ) - 1;
				in_high[ 1 ] = ( 1 << image->depth[ 1 ] ) - 1;
				in_high[ 2 ] = ( 1 << image->depth[ 2 ] ) - 1;
				out_low[ 0 ] = 0.0;
				out_low[ 1 ] = 0.0;
				out_low[ 2 ] = 0.0;

				XIEPhotomap = GetXIEConstrainedGeometryTriplePhotomap( 
					xp, p, 1, levels, 
					xieValConstrainClipScale, in_low, 
					in_high, out_low, out_high, &gp );
			}
			else
			{
				XIEPhotomap = GetXIEGeometryTriplePhotomap( xp, p, 1, &gp );
			}
		}
	}
	if ( reps )
	{
		XIEPhotomap2 = XieCreatePhotomap( xp->d ); 
		if ( XIEPhotomap == ( XiePhotomap ) NULL || XIEPhotomap2 == ( XiePhotomap ) NULL )
		{
			reps = 0;
		}
		else if ( reps )
		{
			decode_notify = False;
			testSize = ptr->floElements;

			if ( type == xieValSingleBand ) 
			{
				flo1_elements = 2;
				flo2_elements = 2;
			}
			else
			{
				flo1_elements = 2;
				flo2_elements = 4;
			}
			if ( depthmismatch == 1 && type == xieValSingleBand )
				flo2_elements+=2;

			flograph1 = XieAllocatePhotofloGraph(flo1_elements);	
			if ( flograph1 == ( XiePhotoElement * ) NULL )
			{
				fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
				reps = 0;
			}
		}
	}
	if ( reps )
	{
		flograph2 = XieAllocatePhotofloGraph(flo2_elements);	
		if ( flograph2 == ( XiePhotoElement * ) NULL )
		{
			fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
			reps = 0;
		}

		flo = ( XiePhotoflo * ) malloc( ( testSize << 1 ) * sizeof( XiePhotoflo ) );
		if ( flo == ( XiePhotoflo * ) NULL )
		{
			fprintf( stderr, "Couldn't allocate vector of flos\n" );
			reps = 0;
		}
		else
		{
			for ( i = 0; i < testSize; i++ )
			{
				flo[ i ] = ( XiePhotoflo ) NULL;
			}
		}

		encode_parms = (char **) malloc( sizeof( char * ) * testSize );
		if ( encode_parms == ( char ** ) NULL )
		{
			fprintf( stderr, "Couldn't allocate encode parameter table\n" );
                        reps = 0;
		}
		else
		{
			for ( i = 0; i < testSize; i++ )
			{
				encode_parms[ i ] = ( char * ) NULL;
			}
		}
	}

	if ( reps )
	{
		in_low[ 0 ] = 0.0;
		in_low[ 1 ] = 0.0;
		in_low[ 2 ] = 0.0;
		if ( ptr->useMyLevelsPlease == True )
		{
			in_high[ 0 ] = ( float ) ( 1 << ptr->myBits[ 0 ] ) - 1;
			in_high[ 1 ] = ( float ) ( 1 << ptr->myBits[ 1 ] ) - 1;
			in_high[ 2 ] = ( float ) ( 1 << ptr->myBits[ 2 ] ) - 1;
		}
		else
		{
			in_high[ 0 ] = ( float ) ( 1 << image->depth[ 0 ] ) - 1;
			in_high[ 1 ] = ( float ) ( 1 << image->depth[ 1 ] ) - 1;
			in_high[ 2 ] = ( float ) ( 1 << image->depth[ 2 ] ) - 1;
		}
		out_low[ 0 ] = 0; 
		out_low[ 1 ] = 0;
		out_low[ 2 ] = 0;
		out_high[ 0 ] = stdCmap.red_max; 
		out_high[ 1 ] = stdCmap.green_max;
		out_high[ 2 ] = stdCmap.blue_max;
		parms = ( char * ) XieTecClipScale( in_low, in_high,
                                out_low, out_high);
		if ( parms == ( char * ) NULL )
			reps = 0;
	}

	if ( reps )	
	{
		techParmOffset = 0;
		floIdx = 0;

		for ( i = 0; i < testSize; i++ )
		{
			idx = 0;
			
			XieFloImportPhotomap(&flograph1[idx], 
				( i == 0 ? XIEPhotomap : XIEPhotomap2 ), 
				decode_notify);
			idx++; 
			if ( type == xieValSingleBand )
			{
				encode_single = XieTecEncodeUncompressedSingle(
					ptr->fillOrder[ techParmOffset ],
					ptr->pixelOrder[ techParmOffset ],
					ptr->pixelStride[ techParmOffset ],
					ptr->scanlinePad[ techParmOffset ]
				);
				encode_parms[i] = ( char * ) encode_single;
			}
			else
			{
				encode_triple = XieTecEncodeUncompressedTriple(
					ptr->fillOrder[ techParmOffset ],
					ptr->pixelOrder[ techParmOffset ],
					ptr->bandOrder[ techParmOffset ],
					ptr->interleave[ techParmOffset ],
					&ptr->pixelStride[ techParmOffset * 3],
					&ptr->scanlinePad[ techParmOffset * 3 ]
				);
				encode_parms[i] = ( char * ) encode_triple;
			}

			techParmOffset++;
			if ( encode_parms[i] == ( char * ) NULL )
			{
				fprintf( stderr, "Could not get encode technique parameters\n" );
				reps = 0;
				break;
			}

			XieFloExportPhotomap(&flograph1[idx],
				idx,
				XIEPhotomap2,
				encode_tech,
				encode_parms[i]
			);

			idx = 0;
			XieFloImportPhotomap(&flograph2[idx], XIEPhotomap2, 
				decode_notify);
			idx++; 

			if ( type == xieValSingleBand && depthmismatch == 1 )
			{
				XieFloImportLUT(&flograph2[idx], XIELut );
				idx++;

				domain.offset_x = 0;
				domain.offset_y = 0;
				domain.phototag = 0;

				XieFloPoint(&flograph2[idx],
					idx - 1,
					&domain,
					idx,
					0x01
				);
				idx++;	
			}
			else if ( type == xieValTripleBand )
			{
				levels[ 0 ] = ( long ) stdCmap.red_max + 1;
				levels[ 1 ] = ( long ) stdCmap.green_max + 1;
				levels[ 2 ] = ( long ) stdCmap.blue_max + 1;

				XieFloConstrain(&flograph2[idx],
					1, 
					levels,
					xieValConstrainClipScale,
					parms
				);
				idx++;
			}

			if ( type == xieValTripleBand )
			{
				c1[ 0 ] = stdCmap.red_mult; 
				c1[ 1 ] = stdCmap.green_mult; 
				c1[ 2 ] = stdCmap.blue_mult;
				bias = ( float ) stdCmap.base_pixel;
				
			        XieFloBandExtract( &flograph2[idx], idx, 
					1 << xp->vinfo.depth, bias, c1 ); 
				idx++;
			}

			XieFloExportDrawable(&flograph2[idx],
				idx,           /* source phototag number */
				drawableWindow,
				xp->fggc,
				0,
				0
			);
			flo[floIdx] = XieCreatePhotoflo( xp->d, flograph1, flo1_elements );
			floIdx++;
			flo[floIdx] = XieCreatePhotoflo( xp->d, flograph2, flo2_elements );
			floIdx++;
		}
	}

	if ( reps )
	{
		flo_notify = False;
		dontClear = True;

		/* display the normal image, dithered to the levels of
		   the output screen, in the left hand window */

		if ( type == xieValSingleBand )
		{
			GetXIEDitheredWindow( xp, p, xp->w, 1,
				1 << xp->vinfo.depth );
		}
		else
		{
			levels[ 0 ] = stdCmap.red_max + 1;
			levels[ 1 ] = stdCmap.green_max + 1;
                        levels[ 2 ] = stdCmap.blue_max + 1;

			GetXIEDitheredStdTripleWindow( xp, p, xp->w, 1,
				xieValDitherDefault, 0, levels, &stdCmap );
		}
                XMoveWindow( xp->d, drawableWindow, WIDTH + 10, 0 );
                XMapRaised( xp->d, drawableWindow );
                XSync( xp->d, 0 );
	}
	if ( !reps )
		FreeFunnyEncodeStuff( xp, p );
	return( reps );
}

void DoFunnyEncode(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    	int     i, j;

    	for (i = 0; i != reps; i++) 
	{
		for ( j = 0; j < testSize; j++ )
		{
			XieExecutePhotoflo( xp->d, flo[ j ], True );
		        WaitForXIEEvent( xp, xieEvnNoPhotofloDone, flo[ j ], 
				0, False );
		}
    	}
}

void 
EndFunnyEncode(xp, p)
XParms  xp;
Parms   p;
{
	extern Window drawableWindow;
	extern Bool dontClear;

	dontClear = False;
	if ( type == xieValTripleBand )
                if ( WMSafe == True )
                        InstallCustomColormap( xp->d, xp->p );
                else
		{
                        InstallCustomColormap( xp->d, xp->w );
                        InstallCustomColormap( xp->d, drawableWindow );
		}

        XUnmapWindow( xp->d, drawableWindow );
	FreeFunnyEncodeStuff( xp, p );
}

int
FreeFunnyEncodeStuff( xp, p )
XParms	xp;
Parms	p;
{
	int	i;

	if ( parms )
	{
		free( parms );
		parms = ( char * ) NULL;
	}
	if ( encode_parms )
	{
		for ( i = 0; i < testSize; i++ )
		{
			if ( encode_parms[ i ] )
				free( encode_parms[ i ] ); 
		}
		free( encode_parms );
		encode_parms = ( char ** ) NULL;
	}

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

        if ( flo )
        {
		for ( i = 0; i < testSize; i++ )
		{
                       if ( flo[ i ] )
                                XieDestroyPhotoflo( xp->d, flo[ i ] );
		}
		free( flo );
                flo = ( XiePhotoflo * ) NULL;
        }

        if ( flograph1 )
        {
                XieFreePhotofloGraph(flograph1, flo1_elements);
                flograph1 = ( XiePhotoElement * ) NULL;
        }

        if ( flograph2 )
        {
                XieFreePhotofloGraph(flograph2, flo2_elements);
                flograph2 = ( XiePhotoElement * ) NULL;
        }
}


