/* $XConsortium: encode.c,v 1.2 93/10/26 20:58:29 rws Exp $ */
/**** module encode.c ****/
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
  
	encode.c -- EP/ECP encode technique tests 

	Syd Logan -- AGE Logic, Inc. October, 1993 - MIT Beta release
  
*****************************************************************************/
#include "xieperf.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <math.h>

static XiePhotomap XIEPhotomap;
static XiePhotomap XIEPhotomap2;
static XieLut XIELut;

static XiePhotoElement *flograph1;
static XiePhotoElement *flograph2;
static XiePhotoflo flo1;
static XiePhotoflo flo2;
static int flo1_elements;
static int flo2_elements;

extern Bool WMSafe;
extern Bool dontClear;
static XStandardColormap stdCmap;
extern Window drawableWindow;
static int type, interleave;

int 
InitEncodePhotomap(xp, p, reps)
XParms  xp;
Parms   p;
int     reps;
{
	XieLTriplet levels;
	int encodeTech;
	char *encodeTechParms = ( char * ) NULL;
	int i, idx;
	XieProcessDomain domain;
	int needconstrain;
	float bias;
	XieConstant c1;
	int visclass;
	XIEimage *image;
        GeometryParms gp;
	Bool exportClient;
	int decodeTech;
	char *decodeTechParms;
	XieLTriplet width, height, rgblevels; 
	XieConstant rgbbias;
	XieRGBToYCbCrParam *RGBToYCbCrParm = (XieRGBToYCbCrParam *) NULL;
	XieYCbCrToRGBParam *YCbCrToRGBParm = (XieYCbCrToRGBParam *) NULL;
	XieEncodeJPEGBaselineParam *JPEGB; 
	int doYCbCr = True;
	unsigned char scale;

	needconstrain = 0;
	decodeTechParms = ( char * ) NULL;
	XIELut = ( XieLut ) NULL;
	XIEPhotomap = (XiePhotomap) NULL;
	XIEPhotomap2 = (XiePhotomap) NULL;
        flo1 = ( XiePhotoflo ) NULL;
        flograph1 = ( XiePhotoElement * ) NULL;
        flo2 = ( XiePhotoflo ) NULL;
        flograph2 = ( XiePhotoElement * ) NULL;

	image = p->finfo.image1;
	interleave = image->interleave;
	type = image->bandclass;

#if     defined(__cplusplus) || defined(c_plusplus)
	visclass = xp->vinfo.c_class;
#else
	visclass = xp->vinfo.class;
#endif
	if ( type == xieValTripleBand && IsDISServer() )
		return( 0 ); 

	if ( type == xieValTripleBand && !IsColorVisual( visclass ) )
		return( 0 );

	if ( reps && type == xieValTripleBand && GetStandardColormap( xp, 
		&stdCmap, XA_RGB_BEST_MAP ) == False )
	{
                fprintf( stderr, "Couldn't get a standard colormap\n" );
        	return( 0 );
        }

	XIEPhotomap2 = XieCreatePhotomap( xp->d );
	if ( XIEPhotomap2 == ( XiePhotomap ) NULL )
		return( 0 );

	width[ 0 ] = image->width[ 0 ];
	width[ 1 ] = image->width[ 1 ];
	width[ 2 ] = image->width[ 2 ];
	height[ 0 ] = image->height[ 0 ];
	height[ 1 ] = image->height[ 1 ];
	height[ 2 ] = image->height[ 2 ];

	if ( type == xieValSingleBand )
	{
               	gp.geoType = GEO_TYPE_SCALE;
		gp.geoHeight = 512;
		gp.geoWidth = 512;
		gp.geoXOffset = 0;
		gp.geoYOffset = 0;
		gp.geoTech = xieValGeomNearestNeighbor;

		if ( IsFaxImage( image->decode ) )
		{
			width[ 0 ] = 512;
			width[ 1 ] = 0;
			width[ 2 ] = 0;
			height[ 0 ] = 512;
			height[ 1 ] = 0;
			height[ 2 ] = 0;

			XIEPhotomap = GetXIEGeometryPhotomap( xp, p, &gp, 1 );
			GetXIEGeometryWindow( xp, p, xp->w, &gp, 1 );
		}
		else
		{
			XIEPhotomap = GetXIEPhotomap( xp, p, 1 );
			GetXIEWindow( xp, p, xp->w, 1 );
		}
		if ( image->depth[ 0 ] != xp->vinfo.depth )
			needconstrain = 1;
	}
	else 
	{
		XIEPhotomap = GetXIETriplePhotomap( xp, p, 1 );

		levels[ 0 ] = stdCmap.red_max + 1;
		levels[ 1 ] = stdCmap.green_max + 1;
		levels[ 2 ] = stdCmap.blue_max + 1;
	
		GetXIEDitheredStdTripleWindow( xp, p, xp->w, 1,
			xieValDitherDefault, 0, levels, &stdCmap );
	}

	if ( XIEPhotomap == ( XiePhotomap ) NULL )
	{
                fprintf( stderr, "Couldn't get a photomap for image\n" );
		reps = 0;
	}

	if ( reps && needconstrain == 1 )
	{
		if ( type == xieValSingleBand )
		{
			XIELut = CreatePointLut( xp, p, image->depth[ 0 ],
				xp->vinfo.depth );
		}
	}

	if ( reps )
	{
		encodeTech = ( ( EncodeParms * )p->ts )->encode;	
		encodeTechParms = ( ( EncodeParms * )p->ts )->parms;
		exportClient = ( ( EncodeParms * )p->ts )->exportClient;

		/* determine flo size */
	
		flo1_elements = 2;
		flo2_elements = 2;

		if ( type == xieValTripleBand ) 
			flo2_elements+=2;
		if ( needconstrain == 1 && type == xieValSingleBand )
			flo2_elements+=2;
		if ( type == xieValTripleBand &&
			encodeTech == xieValEncodeJPEGBaseline &&
			doYCbCr == True )
		{
			flo1_elements += 1;
			flo2_elements += 1;
		}

		flograph1 = XieAllocatePhotofloGraph(flo1_elements);
		flograph2 = XieAllocatePhotofloGraph(flo2_elements);
		if ( flograph1 == ( XiePhotoElement * ) NULL || 
		 flograph2 == ( XiePhotoElement * ) NULL )
		{
			fprintf( stderr, "XieAllocatePhotofloGraph failed\n" );
			reps = 0;
		}
	}

	if ( reps )
	{
		idx = 0;

                XieFloImportPhotomap(&flograph1[idx], XIEPhotomap, False);
		idx++;

		if ( type == xieValTripleBand &&
		     encodeTech == xieValEncodeJPEGBaseline )
		{
			if ( doYCbCr == True ) 
			{

				rgblevels[0] = 256; rgbbias[0]=0;
				rgblevels[1] = 256; rgbbias[1]=127;
				rgblevels[2] = 256; rgbbias[2]=127;

				RGBToYCbCrParm = XieTecRGBToYCbCr(
					rgblevels,
					 0.299, 0.587, 0.114, 
					rgbbias
				);
				/* XXX check for error return */
				XieFloConvertFromRGB( &flograph1[idx],
					idx,
					xieValYCbCr,
					(char *) RGBToYCbCrParm
				);
				idx++;
			}
			scale = ( ( EncodeParms * )p->ts )->scale; 
			JPEGB = ( XieEncodeJPEGBaselineParam * ) encodeTechParms;
			for ( i = 0; i < JPEGB->q_size; i++ )
			{
				JPEGB->q_table[ i ] =
					(JPEGB->q_table[ i ] * scale + 50)/100;	
			}
		} 

		if ( exportClient == True )
		{
			XieFloExportClientPhoto(&flograph1[idx],
				idx,
				xieValFirstData,
				encodeTech,
				encodeTechParms
			);
			idx = 0;
			GetDecodeParms( encodeTech, encodeTechParms,
				&decodeTech, &decodeTechParms );

			XieFloImportClientPhoto(&flograph2[idx], 
				type,
				width,
				height,
				image->levels,
				True,
				decodeTech,
				decodeTechParms
			);
		}
		else
		{
			XieFloExportPhotomap(&flograph1[idx],
				idx,
				XIEPhotomap2,
				encodeTech,
				encodeTechParms
			);
			idx = 0;
			XieFloImportPhotomap(&flograph2[idx], XIEPhotomap2, 
				False );
		}
		idx++;

	}
	if ( reps )
	{
		if ( type == xieValTripleBand )
		{
			if ( encodeTech == xieValEncodeJPEGBaseline &&
			     doYCbCr == True )
			{

				YCbCrToRGBParm = XieTecYCbCrToRGB(
					rgblevels,
					0.299, 0.587, 0.114,
					rgbbias,
					xieValGamutNone,
					(char *) NULL
				);
				/* XXX check for error return */
				XieFloConvertToRGB( &flograph2[idx],
					idx,
					xieValYCbCr,
					(char *) YCbCrToRGBParm
				);
				idx++;
			}

			levels[ 0 ] = ( long ) stdCmap.red_max + 1;
			levels[ 0 ] = ( long ) stdCmap.red_max + 1;
			levels[ 1 ] = ( long ) stdCmap.green_max + 1;
			levels[ 2 ] = ( long ) stdCmap.blue_max + 1;
			XieFloDither(&flograph2[idx],
				idx,
				0x7,
				levels,
				xieValDitherDefault,
				(char *) NULL
			);
			idx++;

			c1[ 0 ] = stdCmap.red_mult;
			c1[ 1 ] = stdCmap.green_mult;
			c1[ 2 ] = stdCmap.blue_mult;
			
			bias = ( float ) stdCmap.base_pixel;

        		XieFloBandExtract( &flograph2[idx], idx, 
				1 << xp->vinfo.depth, bias, c1 ); 
			idx++;
		}

		if ( needconstrain && type == xieValSingleBand )
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

		XieFloExportDrawable( &flograph2[idx],
			idx,
			drawableWindow,
			xp->fggc,
			0,
			0
		);

		flo1 = XieCreatePhotoflo( xp->d, flograph1, flo1_elements );
		flo2 = XieCreatePhotoflo( xp->d, flograph2, flo2_elements );
	}

	if ( !reps )
		FreeEncodePhotomapStuff( xp, p );
	else 
	{
		dontClear = True;
		if ( type == xieValTripleBand )
			InstallThisColormap( xp, stdCmap.colormap );
               	XMoveWindow( xp->d, drawableWindow, WIDTH + 10, 0 );
                XMapRaised( xp->d, drawableWindow );
                XSync( xp->d, 0 );
		dontClear = True;
	}
	if ( decodeTechParms )
		free( decodeTechParms );
	if ( RGBToYCbCrParm )
		free( (char *) RGBToYCbCrParm );
	if ( YCbCrToRGBParm )
		free( (char *) YCbCrToRGBParm );
	return( reps );
}

int
GetDecodeParms( encode, eparms, decode, dparms )
int	encode;
char	*eparms;
int	*decode;
char	**dparms;
{
	static unsigned char left_pad[3] = { 0, 0, 0 };
	XieEncodeUncompressedSingleParam *USP = ( XieEncodeUncompressedSingleParam * ) eparms;
	XieEncodeUncompressedTripleParam *UTP = ( XieEncodeUncompressedTripleParam * ) eparms;
	XieEncodeG31DParam *G31 = ( XieEncodeG31DParam * ) eparms;
	XieEncodeG32DParam *G32 = ( XieEncodeG32DParam * ) eparms;
	XieEncodeG42DParam *G42 = ( XieEncodeG42DParam * ) eparms;
	XieEncodeJPEGBaselineParam *JPEGB = ( XieEncodeJPEGBaselineParam * ) eparms;
	XieEncodeTIFF2Param *TIFF2 = ( XieEncodeTIFF2Param * ) eparms;
	XieEncodeTIFFPackBitsParam *TIFFPB = ( XieEncodeTIFFPackBitsParam * ) eparms;

	switch( encode )
	{
	case xieValEncodeUncompressedSingle:
		*decode = xieValDecodeUncompressedSingle;
		*dparms = ( char * ) XieTecDecodeUncompressedSingle(
			USP->fill_order,
			USP->pixel_order,
			USP->pixel_stride,
			0,
			USP->scanline_pad
			);
		break;
	case xieValEncodeUncompressedTriple:
		*decode = xieValDecodeUncompressedTriple;
		*dparms = ( char * ) XieTecDecodeUncompressedTriple(
			UTP->fill_order,
			UTP->pixel_order,
			UTP->band_order,
			UTP->interleave,
			UTP->pixel_stride,
			left_pad,
			UTP->scanline_pad
			);
		interleave = UTP->interleave;
		break;
	case xieValEncodeG31D:
		*decode = xieValDecodeG31D;
		*dparms = ( char * ) XieTecDecodeG31D(
			G31->encoded_order,
			True,
			G31->radiometric	
			);
		break;
	case xieValEncodeG32D:
		*decode = xieValDecodeG32D;
		*dparms = ( char * ) XieTecDecodeG32D(
			G32->encoded_order,
			True,
			G32->radiometric
			);
		break;
	case xieValEncodeG42D:
		*decode = xieValDecodeG42D;
		*dparms = ( char * ) XieTecDecodeG42D(
			G42->encoded_order,
			True,
			G42->radiometric
			);
		break;
	case xieValEncodeJPEGBaseline:
		*decode = xieValDecodeJPEGBaseline;
		*dparms = ( char * ) XieTecDecodeJPEGBaseline(
			JPEGB->interleave,
			JPEGB->band_order
			);
		interleave = JPEGB->interleave;
		break;
	case xieValEncodeTIFF2:
		*decode = xieValDecodeTIFF2;
		*dparms = ( char * ) XieTecDecodeTIFF2(
			TIFF2->encoded_order,
			True,
			TIFF2->radiometric
			);
		break;
	case xieValEncodeTIFFPackBits:
		*decode = xieValDecodeTIFFPackBits;
		*dparms = ( char * ) XieTecDecodeTIFFPackBits(
			TIFFPB->encoded_order,
			True
			);
		break;
	}
	return( 1 );
}

void 
DoEncodePhotomap(xp, p, reps)
XParms  xp;
Parms   p;
int     reps;
{
	int	i;

	for ( i = 0; i < reps; i++ )
	{
                XieExecutePhotoflo(xp->d, flo1, True );
		WaitForXIEEvent( xp, xieEvnNoPhotofloDone, flo1, 0, False );
                XieExecutePhotoflo(xp->d, flo2, True );
		WaitForXIEEvent( xp, xieEvnNoPhotofloDone, flo2, 0, False );
        }
}

void 
DoEncodeClientPhotomap(xp, p, reps)
XParms  xp;
Parms   p;
int     reps;
{
	int	i, done, size;
	char	*data;

	data = ( char * ) NULL;
	size = 2048;
	for ( i = 0; i < reps; i++ )
	{
                XieExecutePhotoflo(xp->d, flo1, True );
		XSync( xp->d, 0 );
		WaitForXIEEvent( xp, xieEvnNoExportAvailable,
					flo1, flo1_elements, False );
                if ( type == xieValTripleBand && interleave == xieValBandByPlane )
		{
			size = ReadNotifyExportTripleData( xp, p, 0,
				flo1, flo1_elements, 1, size, &data, &done );
		}
		else
		{
			size = ReadNotifyExportData( xp, p, 0,
				flo1, flo1_elements, 1, size, &data, &done );
		}
		WaitForXIEEvent( xp, xieEvnNoPhotofloDone, flo1, 0, False );
                XieExecutePhotoflo(xp->d, flo2, True );
		PumpTheClientData( xp, p, flo2, 0, 1, data, size, 0 );
		WaitForXIEEvent( xp, xieEvnNoPhotofloDone, flo2, 0, False );
        }
	if ( data )
	{
		free( data );
		data = ( char * ) NULL;
	}
}

void EndEncodePhotomap(xp, p)
XParms  xp;
Parms   p;
{
	InstallCustomColormap( xp );
	XUnmapWindow( xp->d, drawableWindow );
        dontClear = False;
	FreeEncodePhotomapStuff( xp, p );
}

int
FreeEncodePhotomapStuff( xp, p )
XParms	xp;
Parms	p;
{
	if ( XIELut )
	{
		XieDestroyLUT(xp->d, XIELut);
		XIELut = ( XieLut ) NULL;
	}

	if ( XIEPhotomap && IsPhotomapInCache( XIEPhotomap ) == False )
	{
		XieDestroyPhotomap(xp->d, XIEPhotomap);
		XIEPhotomap = ( XiePhotomap ) NULL;
	}

	if ( XIEPhotomap2 )
	{
		XieDestroyPhotomap(xp->d, XIEPhotomap2);
		XIEPhotomap2 = ( XiePhotomap ) NULL;
	}

        if ( flograph1 )
        {
                XieFreePhotofloGraph(flograph1,flo1_elements);
                flograph1 = ( XiePhotoElement * ) NULL;
        }

        if ( flo1 )
        {
                XieDestroyPhotoflo( xp->d, flo1 );
                flo1 = ( XiePhotoflo ) NULL;
        }

        if ( flograph2 )
        {
                XieFreePhotofloGraph(flograph2,flo2_elements);
                flograph2 = ( XiePhotoElement * ) NULL;
        }

        if ( flo2 )
        {
                XieDestroyPhotoflo( xp->d, flo2 );
                flo2 = ( XiePhotoflo ) NULL;
        }
}
