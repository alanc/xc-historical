/* $XConsortium: XIEprotost.h,v 1.1 93/07/19 11:43:48 mor Exp $ */

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


******************************************************************************/

#ifndef XIEPROTOST_H
#define XIEPROTOST_H

#include <X11/Xmd.h>			/* defines things like CARD32 */

typedef CARD32	xieTypFloat;

#ifndef WORD64

typedef xieTypFloat 	xieTypConstant[3];

typedef xieTypFloat 	xieTypMatrix[9];

#else /* WORD64 */

typedef CARD8	xieTypConstant[12];

typedef CARD8	xieTypMatrix[36];

#endif /* WORD64 */


typedef CARD8	xieTypAlignment;

typedef CARD8	xieTypArithmeticOp;

typedef struct {
    CARD32	byteCount B32;
    /* LISTofCARD8 (byteCount) */
    /* pad (byteCount) */
} xieTypByteStream;

typedef CARD16  xieTypColorAllocTechnique;

typedef	CARD32	xieTypColorList;

typedef CARD16	xieTypColorspace;

typedef CARD8	xieTypCompareOp;

typedef CARD16  xieTypConstrainTechnique;

typedef CARD16  xieTypConvolveTechnique;

typedef CARD8	xieTypDataClass;

typedef CARD8	xieTypDataType;

typedef CARD16  xieTypDecodeTechnique;

typedef CARD16  xieTypDitherTechnique;

typedef CARD16  xieTypEncodeTechnique;

typedef CARD32	xieTypPhotospace;

typedef CARD32	xieTypPhotoflo;

typedef struct {
  union {
    struct {
      xieTypPhotospace	nameSpace B32;
      CARD32		floID B32;
    } immed;
    struct {
      CARD32		nameSpace B32;
      xieTypPhotoflo	floID B32;
    } stored;
  } u;
} xieTypExecutable;

typedef CARD8	xieTypExportNotify;

typedef CARD8	xieTypExportState;

typedef CARD16	xieTypeGamutTechnique;

typedef CARD16	xieTypeGeometryTechnique;

typedef struct {
    CARD32	index B32;
    CARD32	count B32;
} xieTypHistogramData;

typedef CARD16	xieTypHistogramShape;

typedef CARD8	xieTypInterleave;

typedef CARD32	xieTypLevels[3];

typedef	CARD32	xieTypLUT;

typedef CARD8	xieTypMathOp;

typedef CARD8	xieTypOrientation;

typedef CARD8	xieTypPhotofloOutcome;

typedef CARD8	xieTypPhotofloState;

typedef CARD32	xieTypPhotomap;

typedef CARD16	xieTypPhototag;

typedef struct {
    INT32		offsetX B32;
    INT32		offsetY B32;
    xieTypPhototag	domain B16;
} xieTypProcessDomain;

typedef struct {
    INT32	x B32;
    INT32	y B32;
    CARD32	width B32;
    CARD32	height B32;
} xieTypRectangle;

typedef CARD32	xieTypROI;

typedef CARD8	xieTypServiceClass;

typedef CARD8	xieTypTechniqueGroup;

typedef struct {
    BOOL			needsParam;
    xieTypTechniqueGroup	group;
    CARD16			number B16;
    CARD8			speed;
    CARD8			nameLength;
    CARD8			pad[2];
    /* LISTofCARD8 (nameLength) */
    /* pad (nameLength) */
} xieTypTechniqueRec;

typedef struct {
    xieTypPhototag 	src B16;
    CARD16		pad B16;
    INT32		dstX B32;
    INT32		dstY B32;
} xieTypTile;

typedef CARD8	xieTypTripleofCARD8[3];

typedef CARD16  xieTypTripleofCARD16[3];

typedef CARD32	xieTypTripleofCARD32[3];

typedef CARD16  xieTypWhiteAdjustTechnique;


/*
 * Generic Element
 */

typedef struct {
    CARD16		elemType B16;
    CARD16		elemLength B16;
} xieFlo;

/*
 * Import Elements
 */
typedef struct {
    CARD16		elemType B16;
    CARD16		elemLength B16;
    xieTypDataClass	class;
    xieTypOrientation	bandOrder;
    CARD16		pad B16;
    CARD32		length0 B32;
    CARD32		length1 B32;
    CARD32		length2 B32;
    CARD32		level0 B32;
    CARD32		level1 B32;
    CARD32		level2 B32;
} xieFloImportClientLUT;

typedef struct {
    CARD16			elemType B16;
    CARD16			elemLength B16;
    BOOL			notify;
    xieTypDataClass 		class;
    CARD16			pad B16;
    CARD32			width0 B32;
    CARD32			width1 B32;
    CARD32			width2 B32;
    CARD32			height0 B32;
    CARD32			height1 B32;
    CARD32			height2 B32;
    CARD32			level0 B32;
    CARD32			level1 B32;
    CARD32			level2 B32;
    xieTypDecodeTechnique	decodeTechnique B16;
    CARD16			lenParams B16;
    /* Technique dependent decode params */
} xieFloImportClientPhoto;

typedef struct {
    CARD16	elemType B16;
    CARD16	elemLength B16;
    CARD32	rectangles B32;
} xieFloImportClientROI;

typedef struct {
    CARD16	elemType B16;
    CARD16	elemLength B16;
    Drawable	drawable B32;
    INT16	srcX B16;
    INT16	srcY B16;
    CARD16	width B16;
    CARD16	height B16;
    CARD32	fill B32;
    BOOL	notify;
    CARD8	pad[3];		
} xieFloImportDrawable;

typedef struct {
    CARD16	elemType B16;
    CARD16	elemLength B16;
    Drawable	drawable B32;
    INT16	srcX B16;
    INT16	srcY B16;
    CARD16	width B16;
    CARD16	height B16;
    CARD32	fill B32;
    CARD32	bitPlane B32;
    BOOL	notify;
    CARD8	pad[3];		
} xieFloImportDrawablePlane;

typedef struct {
    CARD16	elemType B16;
    CARD16	elemLength B16;
    xieTypLUT	lut B32;
} xieFloImportLUT;
 
typedef struct {
    CARD16		elemType B16;
    CARD16		elemLength B16;
    xieTypPhotomap	photomap B32;
    BOOL		notify;
    CARD8		pad[3];
} xieFloImportPhotomap;

typedef struct {
    CARD16	elemType B16;
    CARD16	elemLength B16;
    xieTypROI	roi B32;
} xieFloImportROI;


/*
 * Process Elements
 */
typedef struct {
    CARD16		elemType B16;
    CARD16		elemLength B16;
    xieTypPhototag	src1 B16;
    xieTypPhototag	src2 B16;
    INT32		domainOffsetX B32;
    INT32		domainOffsetY B32;
    xieTypPhototag	domainPhototag B16;		
    xieTypArithmeticOp	operator;
    CARD8		bandMask;
    xieTypFloat		constant0 B32;
    xieTypFloat		constant1 B32;
    xieTypFloat		constant2 B32;
} xieFloArithmetic;

typedef struct {
    CARD16		elemType B16;
    CARD16		elemLength B16;
    xieTypPhototag	src1 B16;
    xieTypPhototag	src2 B16;
    xieTypPhototag	src3 B16;
    CARD16		pad B16;
} xieFloBandCombine;

typedef struct {
    CARD16		elemType B16;
    CARD16		elemLength B16;
    xieTypPhototag	src B16;
    CARD16		pad B16;
    xieTypFloat		constant0 B32;
    xieTypFloat		constant1 B32;
    xieTypFloat		constant2 B32;
} xieFloBandExtract;

typedef struct {
    CARD16		elemType B16;
    CARD16		elemLength B16;
    xieTypPhototag	src1 B16;
    xieTypPhototag	src2 B16;
    xieTypPhototag	alpha B16;
    CARD8		pad1[2];
    INT32		domainOffsetX B32;
    INT32		domainOffsetY B32;
    xieTypPhototag	domainPhototag B16;		
    CARD8		bandMask;
    CARD8		pad2;
    xieTypFloat		constant0 B32;
    xieTypFloat		constant1 B32;
    xieTypFloat		constant2 B32;
    xieTypFloat		alphaConst B32;
} xieFloBlend;

typedef struct {
    CARD16		elemType B16;
    CARD16		elemLength B16;
    xieTypPhototag	src1 B16;
    xieTypPhototag	src2 B16;
    INT32		domainOffsetX B32;
    INT32		domainOffsetY B32;
    xieTypPhototag	domainPhototag B16;		
    xieTypCompareOp	operator;
    BOOL		combine;
    xieTypFloat		constant0 B32;
    xieTypFloat		constant1 B32;
    xieTypFloat		constant2 B32;
    CARD8		bandMask;
    CARD8		pad[3];
} xieFloCompare;

typedef struct {
    CARD16			elemType B16;
    CARD16			elemLength B16;
    xieTypPhototag		src B16;
    CARD16			pad B16;
    CARD32			level0 B32;
    CARD32			level1 B32;
    CARD32			level2 B32;
    xieTypConstrainTechnique	constrain B16;
    CARD16			lenParams B16;
    /* Technique dependent constrain params */
} xieFloConstrain;

typedef struct {
    CARD16		elemType B16;
    CARD16		elemLength B16;
    xieTypPhototag	src B16;
    xieTypDataClass	class;
    CARD8		precision;
    Colormap		colormap B32;
} xieFloConvertFromIndex;

typedef struct {
    CARD16		elemType B16;
    CARD16		elemLength B16;
    xieTypPhototag	src B16;
    CARD16		pad B16;
    xieTypColorspace	colorspace B16;
    CARD16		lenParams B16;
    /* Technique dependent color params */
} xieFloConvertFromRGB;

typedef struct {
    CARD16			elemType B16;
    CARD16			elemLength B16;
    xieTypPhototag		src B16;
    BOOL			notify;
    CARD8			pad;
    Colormap			colormap B32;
    xieTypColorList		colorList B32;
    xieTypColorAllocTechnique	colorAlloc B16;
    CARD16			lenParams B16;
    /* Technique dependent color alloc params */
} xieFloConvertToIndex;

typedef struct {
    CARD16		elemType B16;
    CARD16		elemLength B16;
    xieTypPhototag	src B16;
    CARD16		pad B16;
    xieTypColorspace	colorspace B16;
    CARD16		lenParams B16;
    /* Technique dependent color params */
} xieFloConvertToRGB;

typedef struct {
    CARD16			elemType B16;
    CARD16			elemLength B16;
    xieTypPhototag		src B16;
    CARD16			pad B16;
    INT32			domainOffsetX B32;
    INT32			domainOffsetY B32;
    xieTypPhototag		domainPhototag B16;		
    CARD8			bandMask;
    CARD8			kernelSize;
    xieTypConvolveTechnique	convolve B16;
    CARD16			lenParams B16;
    /* LISTofFloat (kernelSize^2) */
    /* Technique dependent convolve params */
} xieFloConvolve;

typedef struct {
    CARD16			elemType B16;
    CARD16			elemLength B16;
    xieTypPhototag		src B16;
    CARD16			pad B16;
    CARD32			level0 B32;
    CARD32			level1 B32;
    CARD32			level2 B32;
    xieTypDitherTechnique	dither B16;
    CARD16			lenParams B16;
    /* Technique dependent dither params */
} xieFloDither;

typedef struct {
    CARD16			elemType B16;
    CARD16			elemLength B16;
    xieTypPhototag 		src B16;
    CARD8			bandMask;
    CARD8			pad;
    CARD32			width B32;
    CARD32			height B32;
    xieTypFloat			a B32;
    xieTypFloat			b B32;
    xieTypFloat			c B32;
    xieTypFloat			d B32;
    xieTypFloat			tx B32;
    xieTypFloat			ty B32;
    xieTypFloat			constant0 B32;
    xieTypFloat			constant1 B32;
    xieTypFloat			constant2 B32;
    xieTypeGeometryTechnique	sample B16;
    CARD16			lenParams B16;
    /* Technique dependent sample params */
} xieFloGeometry;

typedef struct {
    CARD16		elemType B16;
    CARD16		elemLength B16;
    xieTypPhototag	src1 B16;
    xieTypPhototag	src2 B16;
    INT32		domainOffsetX B32;
    INT32		domainOffsetY B32;
    xieTypPhototag	domainPhototag B16;		
    CARD8		operator;
    CARD8		bandMask;
    xieTypFloat		constant0 B32;
    xieTypFloat		constant1 B32;
    xieTypFloat		constant2 B32;
} xieFloLogical;

typedef struct {
    CARD16			elemType B16;
    CARD16			elemLength B16;
    xieTypPhototag		src B16;
    CARD16			pad1 B16;
    INT32			domainOffsetX B32;
    INT32			domainOffsetY B32;
    xieTypPhototag		domainPhototag B16;		
    CARD16			pad2 B16;
    xieTypHistogramShape	shape B16;
    CARD16			lenParams B16;
    /* Technique dependent shape params */
} xieFloMatchHistogram;

typedef struct {
    CARD16		elemType B16;
    CARD16		elemLength B16;
    xieTypPhototag	src B16;
    CARD16		pad B16;
    INT32		domainOffsetX B32;
    INT32		domainOffsetY B32;
    xieTypPhototag	domainPhototag B16;		
    xieTypMathOp	operator;
    CARD8		bandMask;
} xieFloMath;

typedef struct {
    CARD16		elemType B16;
    CARD16		elemLength B16;
    CARD16		numTiles B16;
    CARD16		pad B16;
    CARD32		width B32;
    CARD32		height B32;
    xieTypFloat		constant0 B32;
    xieTypFloat		constant1 B32;
    xieTypFloat		constant2 B32;
    /* LISTofTile (numTiles) */
} xieFloPasteUp;

typedef struct {
    CARD16		elemType B16;
    CARD16		elemLength B16;
    xieTypPhototag	src B16;
    xieTypPhototag	lut B16;
    INT32		domainOffsetX B32;
    INT32		domainOffsetY B32;
    xieTypPhototag	domainPhototag B16;		
    CARD8		bandMask;
    CARD8		pad;
} xieFloPoint;

typedef struct {
    CARD16			elemType B16;
    CARD16			elemLength B16;
    xieTypPhototag		src B16;
    CARD16			pad B16;
} xieFloUnconstrain;


/*
 * Export Elements
 */
typedef struct {
    CARD16		elemType B16;
    CARD16		elemLength B16;
    xieTypPhototag	src B16;
    xieTypExportNotify	notify;
    CARD8		pad1;
    INT32		domainOffsetX B32;
    INT32		domainOffsetY B32;
    xieTypPhototag	domainPhototag B16;		
    CARD16		pad2 B16;
} xieFloExportClientHistogram;

typedef struct {
    CARD16		elemType B16;
    CARD16		elemLength B16;
    xieTypPhototag	src B16;
    xieTypExportNotify	notify;
    xieTypOrientation	bandOrder;
    CARD32		start0  B32;
    CARD32		start1  B32;
    CARD32		start2  B32;
    CARD32		length0 B32;
    CARD32		length1 B32;
    CARD32		length2 B32;
} xieFloExportClientLUT;

typedef struct {
    CARD16			elemType B16;
    CARD16			elemLength B16;
    xieTypPhototag		src B16;
    xieTypExportNotify		notify;
    CARD8			pad;
    xieTypEncodeTechnique	encodeTechnique B16;
    CARD16			lenParams B16;
    /* Technique dependent encode params */
} xieFloExportClientPhoto;

typedef struct {
    CARD16		elemType B16;
    CARD16		elemLength B16;
    xieTypPhototag	src B16;
    xieTypExportNotify	notify;
    CARD8		pad;
} xieFloExportClientROI;

typedef struct {
    CARD16		elemType B16;
    CARD16		elemLength B16;
    xieTypPhototag	src B16;
    INT16		dstX B16;
    INT16		dstY B16;
    CARD16		pad B16;
    Drawable		drawable B32;
    GContext		gc B32;
} xieFloExportDrawable;

typedef struct {
    CARD16		elemType B16;
    CARD16		elemLength B16;
    xieTypPhototag	src B16;
    INT16		dstX B16;
    INT16		dstY B16;
    CARD16		pad B16;
    Drawable		drawable B32;
    GContext		gc B32;
} xieFloExportDrawablePlane;

typedef struct {
    CARD16		elemType B16;
    CARD16		elemLength B16;
    xieTypPhototag	src B16;
    CARD8		merge;
    CARD8		pad;
    xieTypLUT		lut B32;
    CARD32		start0 B32;
    CARD32		start1 B32;
    CARD32		start2 B32;
} xieFloExportLUT;
 
typedef struct {
    CARD16			elemType B16;
    CARD16			elemLength B16;
    xieTypPhototag		src B16;
    CARD16			pad B16;
    xieTypPhotomap		photomap B32;
    xieTypEncodeTechnique	encodeTechnique B16;
    CARD16			lenParams B16;
    /* Technique dependent encode params */
} xieFloExportPhotomap;

typedef struct {
    CARD16		elemType B16;
    CARD16		elemLength B16;
    xieTypPhototag	src B16;
    CARD16		pad B16;
    xieTypROI		roi B32;
} xieFloExportROI;


/*
 * Technique Parameters
 */

typedef struct {
    xieTypFloat		input_low0   B32;
    xieTypFloat		input_low1   B32;
    xieTypFloat		input_low2   B32;
    xieTypFloat		input_high0  B32;
    xieTypFloat		input_high1  B32;
    xieTypFloat		input_high2  B32;
    CARD32		output_low0  B32;
    CARD32		output_low1  B32;
    CARD32		output_low2  B32;
    CARD32		output_high0 B32;
    CARD32		output_high1 B32;
    CARD32		output_high2 B32;
} xieTecClipScale;

typedef struct {
    CARD32		fill B32;
} xieTecColorAllocAll;

typedef struct {
    xieTypFloat		matchLimit B32;
    xieTypFloat		grayLimit B32;
} xieTecColorAllocMatch;

typedef struct {
    CARD32		maxCells B32;
} xieTecColorAllocRequantize;

typedef struct {
    xieTypMatrix		matrix;
    xieTypWhiteAdjustTechnique	whiteAdjusted B16;
    CARD16			lenParams B16;
    /* Technique dependent white params */
} xieTecRGBToCIELab, xieTecRGBToCIEXYZ;

typedef struct {
    xieTypFloat		red B32;
    xieTypFloat		green B32;
    xieTypFloat		blue B32;
} xieTecRGBToYCbCr, xieTecRGBToYCC, xieTecYCbCrToRGB, xieTecYCCToRGB;

typedef struct {
    xieTypMatrix		matrix;
    xieTypWhiteAdjustTechnique	whiteAdjusted B16;
    CARD16			numWhiteParams B16;
    xieTypeGamutTechnique	gamutTechnique B16;
    CARD16			numGamutParams B16;
    /* Technique dependent white params */
    /* Technique dependent gamut params */
} xieTecCIELabToRGB, xieTecCIEXYZToRGB;

typedef struct {
    xieTypFloat		constant0 B32;
    xieTypFloat		constant1 B32;
    xieTypFloat		constant2 B32;
} xieTecConvolveConstant;

typedef struct {
    xieTypOrientation	fillOrder;
    xieTypOrientation	pixelOrder;
    CARD8		pixelStride;
    CARD8		leftPad;
    CARD8		scanlinePad;
    CARD8		pad[3];
} xieTecDecodeUncompressedSingle;

typedef struct {
    CARD8		leftPad[3];
    xieTypOrientation	fillOrder;
    CARD8		pixelStride[3];
    xieTypOrientation	pixelOrder;
    CARD8		scanlinePad[3];
    xieTypOrientation	bandOrder;
    xieTypInterleave	interleave;
    CARD8		pad[3];
} xieTecDecodeUncompressedTriple;

typedef struct {
    xieTypOrientation	encodedOrder;
    BOOL		normal;
    CARD16		pad B16;
} xieTecDecodeG31D, xieTecDecodeG32D, xieTecDecodeG42D,
  xieTecDecodeTIFF2, xieTecDecodeTIFFPackBits;

typedef struct {
    xieTypInterleave	interleave;
    xieTypOrientation	bandOrder;
    CARD16		pad B16;
} xieTecDecodeJPEGBaseline, xieTecDecodeJPEGLossless;

typedef struct {
    CARD8	thresholdOrder;
    CARD8	pad[3];
} xieTecDitherOrdered;

typedef struct {
    xieTypOrientation	fillOrder;
    xieTypOrientation	pixelOrder;
    CARD8		pixelStride;
    CARD8		scanlinePad;
} xieTecEncodeUncompressedSingle;

typedef struct {
    CARD8		pixelStride[3];
    xieTypOrientation	pixelOrder;
    CARD8		scanlinePad[3];
    xieTypOrientation	fillOrder;
    xieTypOrientation	bandOrder;
    xieTypInterleave	interleave;
    CARD16		pad B16;
} xieTecEncodeUncompressedTriple;

typedef struct {
    BOOL		alignEol;
    xieTypOrientation	encodedOrder;
    CARD16		pad B16;
} xieTecEncodeG31D;

typedef struct {
    BOOL		uncompressed;
    BOOL		alignEol;
    xieTypOrientation	encodedOrder;
    CARD8		pad;
    CARD32		kFactor B32;
} xieTecEncodeG32D;

typedef struct {
    BOOL		uncompressed;
    xieTypOrientation	encodedOrder;
    CARD16		pad B16;
} xieTecEncodeG42D;

typedef struct {
    xieTypInterleave	interleave;
    xieTypOrientation	bandOrder;
    CARD16		q B16; 		/* multiple of 4 */
    CARD16		a B16; 		/* multiple of 4 */
    CARD16		d B16; 		/* multiple of 4 */
    /* LISTofCARD8 (q) */
    /* LISTofCARD8 (a) */
    /* LISTofCARD8 (d) */
} xieTecEncodeJPEGBaseline;

typedef struct {
    xieTypInterleave	interleave;
    xieTypOrientation	bandOrder;
    CARD16		tableSize B16;	/* multiple of 4 */
    CARD8		predictor[3];
    CARD8		pad;
    /* LISTofCARD8 (tableSize) */
} xieTecEncodeJPEGLossless;

typedef struct {
    xieTypOrientation	encodedOrder;
    CARD8		pad[3];
} xieTecEncodeTIFF2, xieTecEncodeTIFFPackBits;

typedef struct {
    INT16	simple B16;
    CARD16	pad B16;
} xieTecGeomAntialiasByArea;

typedef struct {
    INT16	kernelSize B16;
    CARD16	pad B16;
} xieTecGeomAntialiasByLowpass;

typedef struct {
    CARD8	radius;
    BOOL	simple;
    CARD16	pad B16;
    xieTypFloat	sigma B32;
    xieTypFloat	normalize B32;
} xieTecGeomGaussian;

typedef struct {
    CARD8	modify;
    CARD8	pad[3];
} xieTecGeomNearestNeighbor;

typedef struct {
    xieTypFloat		mean B32;
    xieTypFloat		variance B32;
} xieTecHistogramGaussian;

typedef struct {
    BOOL		shapeFactor;
    CARD8		pad[3];
    xieTypFloat		constant B32;
} xieTecHistogramHyperbolic;

typedef struct {
    xieTypFloat		whitePoint0 B32;
    xieTypFloat		whitePoint1 B32;
    xieTypFloat		whitePoint2 B32;
} xieTecWhiteAdjustCIELabShift;


/*
 * SIZEOF values
 */
#define sz_xieTypFloat				4
#define sz_xieTypConstant			12
#define sz_xieTypMatrix				36
#define sz_xieTypAlignment			1
#define sz_xieTypArithmeticOp			1
#define sz_xieTypByteStream			4
#define sz_xieTypColorAllocTechnique		2
#define sz_xieTypColorList			4
#define sz_xieTypColorspace			2
#define sz_xieTypCompareOp			1
#define sz_xieTypConstrainTechnique		2
#define sz_xieTypConvolveTechnique		2
#define sz_xieTypDataClass			1
#define sz_xieTypDecodeTechnique		2
#define sz_xieTypDitherTechnique		2
#define sz_xieTypEncodeTechnique		2
#define sz_xieTypPhotospace			4
#define sz_xieTypPhotoflo			4
#define sz_xieTypExecutable			8
#define sz_xieTypExportNotify			1
#define sz_xieTypExportState			1
#define sz_xieTypGamutTechnique			2
#define sz_xieTypGeometryTechnique		2
#define sz_xieTypHistogramData			8
#define sz_xieTypHistogramShape			2
#define sz_xieTypInterleave			1
#define sz_xieTypLevels				12
#define sz_xieTypLUT				4
#define sz_xieTypMathOp				1
#define sz_xieTypOrientation			1
#define sz_xieTypPhotofloOutcome		1
#define sz_xieTypPhotofloState			1
#define sz_xieTypPhotomap			4
#define sz_xieTypPhototag			2
#define sz_xieTypProcessDomain			12
#define sz_xieTypRectangle			16
#define sz_xieTypROI				4
#define sz_xieTypServiceClass			1
#define sz_xieTypTechniqueGroup			1
#define sz_xieTypTechniqueRec			8
#define sz_xieTypTile				12
#define sz_xieTypTripleofCARD8			3
#define sz_xieTypTripleofCARD16			6
#define sz_xieTypTripleofCARD32			12
#define sz_xieTypWhiteAdjustTechnique		2
#define sz_xieFloImportClientLUT		32
#define sz_xieFloImportClientPhoto		48
#define sz_xieFloImportClientROI		8
#define sz_xieFloImportDrawable			24
#define sz_xieFloImportDrawablePlane		28
#define sz_xieFloImportLUT			8
#define sz_xieFloImportPhotomap			12
#define sz_xieFloImportROI			8
#define sz_xieFloArithmetic			32
#define sz_xieFloBandCombine			12
#define sz_xieFloBandExtract			20
#define sz_xieFloBlend				40
#define sz_xieFloCompare			36
#define sz_xieFloConstrain			24
#define sz_xieFloConvertFromIndex		12
#define sz_xieFloConvertFromRGB			12
#define sz_xieFloConvertToIndex			20
#define sz_xieFloConvertToRGB			12
#define sz_xieFloConvolve			24
#define sz_xieFloDither				24
#define sz_xieFloGeometry			56
#define sz_xieFloLogical			32
#define sz_xieFloMatchHistogram			24
#define sz_xieFloMath				20
#define sz_xieFloPasteUp			28
#define sz_xieFloPoint				20
#define sz_xieFloUnconstrain			8
#define sz_xieFloExportClientHistogram		20
#define sz_xieFloExportClientLUT		32
#define sz_xieFloExportClientPhoto		12
#define sz_xieFloExportClientROI		8
#define sz_xieFloExportDrawable			20
#define sz_xieFloExportDrawablePlane		20
#define sz_xieFloExportLUT			24
#define sz_xieFloExportPhotomap			16
#define sz_xieFloExportROI			12
#define sz_xieTecColorAllocAll			4
#define sz_xieTecColorAllocMatch		8
#define sz_xieTecColorAllocRequantize		4
#define sz_xieTecClipScale			48
#define sz_xieTecHardClip			0
#define sz_xieTecConvolveConstant		12
#define sz_xieTecConvolveReplicate		0
#define sz_xieTecRGBToCIELab			40
#define sz_xieTecRGBToCIEXYZ			40
#define sz_xieTecRGBToYCbCr			12
#define sz_xieTecRGBToYCC			12
#define sz_xieTecYCbCrToRGB			12
#define sz_xieTecYCCToRGB			12
#define sz_xieTecCIELabToRGB			44
#define sz_xieTecCIEXYZToRGB			44
#define sz_xieTecDecodeUncompressedSingle	8
#define sz_xieTecDecodeUncompressedTriple	16
#define sz_xieTecDecodeG31D			4
#define sz_xieTecDecodeG32D			4
#define sz_xieTecDecodeG42D			4
#define sz_xieTecDecodeTIFF2			4
#define sz_xieTecDecodeTIFFPackBits		4
#define sz_xieTecDecodeJPEGBaseline		4
#define sz_xieTecDecodeJPEGLossless		4
#define sz_xieTecDitherOrdered			4
#define sz_xieTecDitherErrorDiffusion		4
#define sz_xieTecEncodeUncompressedSingle	4
#define sz_xieTecEncodeUncompressedTriple	12
#define sz_xieTecEncodeG31D			4
#define sz_xieTecEncodeG32D			8
#define sz_xieTecEncodeG42D			4
#define sz_xieTecEncodeJPEGBaseline		8
#define sz_xieTecEncodeJPEGLossless		8
#define sz_xieTecEncodeTIFF2			4
#define sz_xieTecEncodeTIFFPackBits		4
#define sz_xieTecGamutNone			0
#define sz_xieTecGamutClipRGB			0
#define sz_xieTecGeomAntialias			0
#define sz_xieTecGeomAntialiasByArea		4
#define sz_xieTecGeomAntialiasByLowpass		4
#define sz_xieTecGeomBilinearInterpolation	0
#define sz_xieTecGeomGaussian			12
#define sz_xieTecGeomNearestNeighbor		4
#define sz_xieTecHistogramFlat			0
#define sz_xieTecHistogramGaussian		8
#define sz_xieTecHistogramHyperbolic		8
#define sz_xieTecWhiteAdjustNone		0
#define sz_xieTecWhiteAdjustCIELabShift		12

#endif /* XIEPROTOST_H */
