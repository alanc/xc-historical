/* $XConsortium$ */
/**** module technq.h ****/
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
*******************************************************************************

	technq.h: contains technique definitions

	Dean Verheiden -- AGE Logic, Inc. April 1993

******************************************************************************/

#ifndef _XIEH_TECHNQ
#define _XIEH_TECHNQ

#include <flostr.h>

/*
 * dixie import client photo technique entry points
 */
extern Bool	CopyICPhotoUnSingle();
extern Bool	CopyICPhotoUnTriple();
extern Bool	CopyICPhotoG31D();
extern Bool	CopyICPhotoG32D();
extern Bool	CopyICPhotoG42D();
extern Bool	CopyICPhotoJPEGBaseline();
extern Bool	CopyICPhotoJPEGLossless();
extern Bool	CopyICPhotoTIFF2();
extern Bool	CopyICPhotoTIFFPackBits();
extern Bool	PrepICPhotoUnSingle();
extern Bool	PrepICPhotoUnTriple();
extern Bool	PrepICPhotoG31D();
extern Bool	PrepICPhotoG32D();
extern Bool	PrepICPhotoG42D();
extern Bool	PrepICPhotoJPEGBaseline();
extern Bool	PrepICPhotoJPEGLossless();
extern Bool	PrepICPhotoTIFF2();
extern Bool	PrepICPhotoTIFFPackBits();

/*
 * dixie constrain technique entry points
 */
extern Bool	CopyPConstrainStandard();
extern Bool	CopyPConstrainClipScale();
extern Bool	PrepPConstrainStandard();
extern Bool	PrepPConstrainClipScale();

/*
 * dixie convolve technique entry points
 */
extern Bool     CopyConvolveConstant();
extern Bool     CopyConvolveReplicate();
extern Bool     PrepConvolveStandard();

/*
 * dixie dither technique entry points
 */
extern Bool	CopyPDitherErrorDiffusion();
extern Bool	PrepPDitherErrorDiffusion();

/*
 * dixie geometry technique entry points
 */

extern Bool     CopyGeomNearestNeighbor();
extern Bool     PrepGeomNearestNeighbor();
extern Bool     CopyGeomBilinearInterp();
extern Bool     PrepGeomBilinearInterp();
extern Bool     CopyGeomAntiAlias();
extern Bool     PrepGeomAntiAlias();

/*
 * dixie convert to index technique entry points
 */

extern Bool 	CopyCtoIAllocAll();
extern Bool 	CopyCtoIAllocMatch();
extern Bool 	CopyCtoIAllocRequantize();
extern Bool 	PrepCtoIAllocAll();
extern Bool 	PrepCtoIAllocMatch();
/*
 * dixie export client photo technique entry points
 */
extern Bool	CopyECPhotoUnSingle();
extern Bool	CopyECPhotoUnTriple();
extern Bool	CopyECPhotoG31D();
extern Bool	CopyECPhotoG32D();
extern Bool	CopyECPhotoG42D();
extern Bool	CopyECPhotoJPEGBaseline();
extern Bool	CopyECPhotoJPEGLossless();
extern Bool	CopyECPhotoTIFF2();
extern Bool	CopyECPhotoTIFFPackBits();

extern Bool	PrepECPhotoUnSingle();
extern Bool	PrepECPhotoUnTriple();
extern Bool	PrepECPhotoG31D();
extern Bool	PrepECPhotoG32D();
extern Bool	PrepECPhotoG42D();
extern Bool	PrepECPhotoJPEGBaseline();
extern Bool	PrepECPhotoJPEGLossless();
extern Bool	PrepECPhotoTIFF2();
extern Bool	PrepECPhotoTIFFPackBits();



/* Global definitions for referencing techniques */

typedef struct _techvec {
  xieBoolProc   copyfnc;
  xieBoolProc   prepfnc;
} techVecRec;


#if defined(_XIEC_TECHNQ)
/*
 *  Technique resource definition
 */

/* 
   Always assume that if there is a default technique, then it is the first
   element of the group specific array. A non zero value in default indicates
   that this element is the default technique. If the first element's default
   flag is not set, then there is no default technique. No search is made of
   the remaining elements in the array for a default technique.
*/


typedef struct _technique {
	BOOL			needsParam;
	xieTypTechniqueGroup	group;	
	CARD16			number;
	CARD8			speed;
	CARD8			nameLength;
	CARD16			Param_size;   /* Minimum parameter size   */
	BOOL			tech_default; /* TRUE = default technique */
	BOOL			fixed_size;   /* TRUE = fixed size tech   */
	BOOL			swap; 	      /* TRUE = has "swapables"	  */
	BOOL			has_floats;   /* TRUE = has floats	  */
	CARD8			*name;
	techVecRec		tip;
} Technique;

#define DEFAULT_SPEED		128
#define NUM_TECH_DEFAULTS	6

/* 
   Initialize nameLength to 0 and compute at runtime. Some compilers do not
   support the use of sizeof(static string) at compile time
*/

static Bool NoParamCheck();
static Bool NoTechYet(); 


/* Array of techniques for coloralloc */
Technique	Tcoloralloc[] = { 
	{	TRUE,
		xieValColorAlloc, 
		xieValColorAllocAll,
		DEFAULT_SPEED,	
		0,
		sz_xieTecColorAllocAll,
		FALSE,
		TRUE,
		TRUE,
		FALSE,
		(CARD8 *)"ALLOC-ALL",
		{
			CopyCtoIAllocAll,
			PrepCtoIAllocAll
                }
	},
	{	TRUE,
		xieValColorAlloc, 
		xieValColorAllocMatch,
		DEFAULT_SPEED,
		0,
		sz_xieTecColorAllocMatch,	
		TRUE,
		TRUE,
		TRUE,
		TRUE,	
		(CARD8 *)"MATCH",
		{
			CopyCtoIAllocMatch,
			PrepCtoIAllocMatch
		}
#ifdef	BEYOND_SI
	},
	{	TRUE,
		xieValColorAlloc, 
		xieValColorAllocRequantize,
		DEFAULT_SPEED,
		0,
		sz_xieTecColorAllocRequantize,
		FALSE,
		TRUE,
		TRUE,
		FALSE,
		(CARD8 *)"REQUANTIZE",
		{
			CopyCtoIAllocRequantize,
			NoTechYet
                }
#endif
	}
};
	
/* Array of techniques for constrain */
Technique	Tconstrain[] = { 
	{	FALSE,
		xieValConstrain, 
		xieValConstrainClipScale,
		DEFAULT_SPEED,
		0,
		sz_xieTecClipScale,
		FALSE,
		FALSE,
		TRUE,
		TRUE,
		(CARD8 *)"CLIP-SCALE",
		{
			CopyPConstrainClipScale,
			PrepPConstrainClipScale
                }
	},
	{	FALSE,
		xieValConstrain, 
		xieValConstrainHardClip,
		DEFAULT_SPEED,
		0,
		sz_xieTecHardClip,
		FALSE,
		TRUE,
		FALSE,
		FALSE,
		(CARD8 *)"HARD-CLIP",
		{
			CopyPConstrainStandard,
			PrepPConstrainStandard
                }
	}
};

/* Array of techniques for conversion from RGB to another colorspace */
Technique	Tconvertfromrgb[] = { 
	{	TRUE,
		xieValConvertFromRGB, 
		xieValCIELab,
		DEFAULT_SPEED,
		0,
		sz_xieTecRGBToCIELab,
		FALSE,
		FALSE,
		TRUE,
		TRUE,
		(CARD8 *)"CIELAB",
		{
                        NoParamCheck, 
			NoTechYet
                }
	},
	{	TRUE,
		xieValConvertFromRGB, 
		xieValCIEXYZ,
		DEFAULT_SPEED,
		0,
		sz_xieTecRGBToCIEXYZ,
		FALSE,
		FALSE,
		TRUE,
		TRUE,
		(CARD8 *)"CIEXYZ",
		{
                        NoParamCheck, 
			NoTechYet
                }
	},
	{	TRUE,
		xieValConvertFromRGB, 
		xieValYCbCr,
		DEFAULT_SPEED,
		0,
		sz_xieTecRGBToYCbCr,
		FALSE,
		TRUE,
		TRUE,
		TRUE,
		(CARD8 *)"YCbCr",
		{
                        NoParamCheck, 
			NoTechYet
                }
	},
	{	TRUE,
		xieValConvertFromRGB, 
		xieValYCC,
		DEFAULT_SPEED,
		0,
		sz_xieTecRGBToYCC,
		FALSE,
		TRUE,
		TRUE,
		TRUE,
		(CARD8 *)"YCC",
		{
                        NoParamCheck, 
			NoTechYet
                }
	}
};

/* Array of techniques for converting to RGB from another colorspace */
Technique	Tconverttorgb[] = { 
	{	TRUE,
		xieValConvertToRGB, 
		xieValCIELab,
		DEFAULT_SPEED,
		0,
		sz_xieTecCIELabToRGB,
		FALSE,
		FALSE,
		TRUE,
		TRUE,
		(CARD8 *)"CIELAB",
		{
                        NoParamCheck, 
			NoTechYet
                }
	},
	{	TRUE,
		xieValConvertToRGB, 
		xieValCIEXYZ,
		DEFAULT_SPEED,
		0,
		sz_xieTecCIEXYZToRGB,
		FALSE,
		FALSE,
		TRUE,
		TRUE,
		(CARD8 *)"CIEXYZ",
		{
                        NoParamCheck, 
			NoTechYet
                }
	},
	{	TRUE,
		xieValConvertToRGB, 
		xieValYCbCr,
		DEFAULT_SPEED,
		0,
		sz_xieTecYCbCrToRGB,
		FALSE,
		TRUE,
		TRUE,
		TRUE,
		(CARD8 *)"YCbCr",
		{
                        NoParamCheck, 
			NoTechYet
                }
	},
	{	TRUE,
		xieValConvertToRGB, 
		xieValYCC,
		DEFAULT_SPEED,
		0,
		sz_xieTecYCCToRGB,
		FALSE,
		TRUE,
		TRUE,
		TRUE,
		(CARD8 *)"YCC",
		{
                        NoParamCheck, 
			NoTechYet
                }
	}
};

/* Array of techniques for convolve */
Technique	Tconvolve[] = { 
	{	FALSE,
		xieValConvolve, 
		xieValConvolveConstant,
		DEFAULT_SPEED,
		0,
		sz_xieTecConvolveConstant,
		TRUE,
		TRUE,
		TRUE,
		TRUE,
		(CARD8 *)"CONSTANT",
		{
                        CopyConvolveConstant, 
			PrepConvolveStandard
                }
	},
	{	FALSE,
		xieValConvolve, 
		xieValConvolveReplicate,
		DEFAULT_SPEED,
		0,
		sz_xieTecConvolveReplicate,
		FALSE,
		TRUE,
		FALSE,
		FALSE,
		(CARD8 *)"REPLICATE",
		{
                        CopyConvolveReplicate, 
			PrepConvolveStandard
                }
	}
};

/* Array of techniques for decode */
Technique	Tdecode[] = { 
	{	TRUE,
		xieValDecode, 
		xieValDecodeUncompressedSingle,
		DEFAULT_SPEED,
		0,
		sz_xieTecDecodeUncompressedSingle,	
		FALSE,	
		TRUE,
		FALSE,
		FALSE,
		(CARD8 *)"UNCOMPRESSED-SINGLE",
		{
			CopyICPhotoUnSingle,
			PrepICPhotoUnSingle
		}
	},
	{	TRUE,
		xieValDecode, 
		xieValDecodeUncompressedTriple,
		DEFAULT_SPEED,
		0,
		sz_xieTecDecodeUncompressedTriple,	
		FALSE,	
		TRUE,
		FALSE,
		FALSE,
		(CARD8 *)"UNCOMPRESSED-TRIPLE",
		{
			CopyICPhotoUnTriple, 
			PrepICPhotoUnTriple
		}
	},
	{	TRUE,
		xieValDecode, 
		xieValDecodeG31D,
		DEFAULT_SPEED,
		0,
		sz_xieTecDecodeG31D,	
		FALSE,	
		TRUE,
		FALSE,
		FALSE,
		(CARD8 *)"CCITT-G31D",
		{
			CopyICPhotoG31D, 
			PrepICPhotoG31D 
		}
	},
	{	TRUE,
		xieValDecode, 
		xieValDecodeG32D,
		DEFAULT_SPEED,
		0,
		sz_xieTecDecodeG32D,	
		FALSE,	
		TRUE,
		FALSE,
		FALSE,
		(CARD8 *)"CCITT-G32D",
		{
			CopyICPhotoG32D, 
			PrepICPhotoG32D
		}
	},
	{	TRUE,
		xieValDecode, 
		xieValDecodeG42D,
		DEFAULT_SPEED,
		0,
		sz_xieTecDecodeG42D,	
		FALSE,	
		TRUE,
		FALSE,
		FALSE,
		(CARD8 *)"CCITT-G42D",
		{
			CopyICPhotoG42D, 
			PrepICPhotoG42D
		}
	},
	{	TRUE,
		xieValDecode, 
		xieValDecodeJPEGBaseline,
		DEFAULT_SPEED,
		0,
		sz_xieTecDecodeJPEGBaseline,	
		FALSE,	
		FALSE,
		FALSE,
		FALSE,
		(CARD8 *)"JPEG-BASELINE",
		{
			CopyICPhotoJPEGBaseline, 
			PrepICPhotoJPEGBaseline
		}
	},
#ifdef	BEYOND_SI
	{	TRUE,
		xieValDecode, 
		xieValDecodeJPEGLossless,
		DEFAULT_SPEED,
		0,
		sz_xieTecDecodeJPEGLossless,	
		FALSE,	
		FALSE,
		FALSE,
		FALSE,
		(CARD8 *)"JPEG-LOSSLESS",
		{
			CopyICPhotoJPEGLossless, 
			PrepICPhotoJPEGLossless
		}
	},
#endif
	{	TRUE,
		xieValDecode, 
		xieValDecodeTIFF2,
		DEFAULT_SPEED,
		0,
		sz_xieTecDecodeTIFF2,	
		FALSE,	
		TRUE,
		FALSE,
		FALSE,
		(CARD8 *)"TIFF-2",
		{
			CopyICPhotoTIFF2, 
			PrepICPhotoTIFF2
		}
	},
	{	TRUE,
		xieValDecode, 
		xieValDecodeTIFFPackBits,
		DEFAULT_SPEED,
		0,
		sz_xieTecDecodeTIFFPackBits,	
		FALSE,	
		TRUE,
		FALSE,
		FALSE,
		(CARD8 *)"TIFF-PACKBITS",
		{
			CopyICPhotoTIFFPackBits, 
			PrepICPhotoTIFFPackBits
		}
	}
};

/* Array of techniques for dither */
Technique	Tdither[] = { 
	{	FALSE,
		xieValDecode, 
		xieValDitherErrorDiffusion,
		DEFAULT_SPEED,
		0,
		sz_xieTecDitherErrorDiffusion,	
		TRUE,		/* Default ! */
		TRUE,
		FALSE,
		FALSE,
		(CARD8 *)"ERROR-DIFFUSION",
		{
                        CopyPDitherErrorDiffusion, 
			PrepPDitherErrorDiffusion
                }
	},
	{	TRUE,
		xieValDecode, 
		xieValDitherOrdered,
		DEFAULT_SPEED,
		0,
		sz_xieTecDitherOrdered,	
		FALSE,
		TRUE,
		FALSE,
		FALSE,
		(CARD8 *)"ORDERED",
		{
                        NoParamCheck, 
			NoTechYet
                }
	}
};

/* Array of techniques for encode */
Technique	Tencode[] = { 
	{	TRUE,
		xieValEncode, 
		xieValEncodeUncompressedSingle,
		DEFAULT_SPEED,
		0,
		sz_xieTecEncodeUncompressedSingle,	
		FALSE,	
		TRUE,
		FALSE,
		FALSE,
		(CARD8 *)"UNCOMPRESSED-SINGLE",
		{
                        CopyECPhotoUnSingle, 
			PrepECPhotoUnSingle
                }
	},
	{	TRUE,
		xieValEncode, 
		xieValEncodeUncompressedTriple,
		DEFAULT_SPEED,
		0,
		sz_xieTecEncodeUncompressedTriple,	
		FALSE,	
		TRUE,
		FALSE,
		FALSE,
		(CARD8 *)"UNCOMPRESSED-TRIPLE",
		{
                        CopyECPhotoUnTriple, 
			PrepECPhotoUnTriple
                }
	},
	{	TRUE,
		xieValEncode, 
		xieValEncodeG31D,
		DEFAULT_SPEED,
		0,
		sz_xieTecEncodeG31D,	
		FALSE,	
		TRUE,
		FALSE,
		FALSE,
		(CARD8 *)"CCITT-G31D",
		{
			CopyECPhotoG31D, 
			PrepECPhotoG31D 
                }
	},
	{	TRUE,
		xieValEncode, 
		xieValEncodeG32D,
		DEFAULT_SPEED,
		0,
		sz_xieTecEncodeG32D,	
		FALSE,	
		TRUE,
		FALSE,
		FALSE,
		(CARD8 *)"CCITT-G32D",
		{
			CopyECPhotoG32D, 
			PrepECPhotoG32D
                }
	},
	{	TRUE,
		xieValEncode, 
		xieValEncodeG42D,
		DEFAULT_SPEED,
		0,
		sz_xieTecEncodeG42D,	
		FALSE,	
		TRUE,
		FALSE,
		FALSE,
		(CARD8 *)"CCITT-G42D",
		{
			CopyECPhotoG42D, 
			PrepECPhotoG42D
                }
	},
	{	TRUE,
		xieValEncode, 
		xieValEncodeJPEGBaseline,
		DEFAULT_SPEED,
		0,
		sz_xieTecEncodeJPEGBaseline,	
		FALSE,	
		FALSE,
		FALSE,
		FALSE,
		(CARD8 *)"JPEG-BASELINE",
		{
			CopyECPhotoJPEGBaseline, 
			PrepECPhotoJPEGBaseline
                }
	},
#ifdef	BEYOND_SI
	{	TRUE,
		xieValEncode, 
		xieValEncodeJPEGLossless,
		DEFAULT_SPEED,
		0,
		sz_xieTecEncodeJPEGLossless,	
		FALSE,	
		FALSE,
		FALSE,
		FALSE,
		(CARD8 *)"JPEG-LOSSLESS",
		{
			CopyECPhotoJPEGLossless, 
			PrepECPhotoJPEGLossless
                }
	},
#endif
	{	TRUE,
		xieValEncode, 
		xieValEncodeTIFF2,
		DEFAULT_SPEED,
		0,
		sz_xieTecEncodeTIFF2,	
		FALSE,	
		TRUE,
		FALSE,
		FALSE,
		(CARD8 *)"TIFF-2",
		{
			CopyECPhotoTIFF2, 
			PrepECPhotoTIFF2
                }
	},
	{	TRUE,
		xieValEncode, 
		xieValEncodeTIFFPackBits,
		DEFAULT_SPEED,
		0,
		sz_xieTecEncodeTIFFPackBits,	
		FALSE,	
		TRUE,
		FALSE,
		FALSE,
		(CARD8 *)"TIFF-PACKBITS",
		{
			CopyECPhotoTIFFPackBits, 
			PrepECPhotoTIFFPackBits
                }
	}
};

/* Array of techniques for gamut */
Technique	Tgamut[] = { 
	{	FALSE,
		xieValGamut, 
		xieValGamutNone,
		DEFAULT_SPEED,
		0,
		sz_xieTecGamutNone,	
		TRUE,	
		TRUE,
		FALSE,
		FALSE,
		(CARD8 *)"NONE",
		{
                        NoParamCheck, 
			NoTechYet
                }
	},
	{	FALSE,
		xieValGamut, 
		xieValGamutClipRGB,
		DEFAULT_SPEED,
		0,
		sz_xieTecGamutClipRGB,	
		FALSE,	
		TRUE,
		FALSE,
		FALSE,
		(CARD8 *)"CLIP-RGB",
		{
                        NoParamCheck, 
			NoTechYet
                }
	}
};

/* Array of techniques for geometry */
Technique	Tgeometry[] = { 
	{	FALSE,
		xieValGeometry, 
		xieValGeomNearestNeighbor,
		DEFAULT_SPEED,
		0,
		sz_xieTecGeomNearestNeighbor,
		TRUE,
		TRUE,
		FALSE,
		FALSE,
		(CARD8 *)"NEAREST-NEIGHBOR",
		{
                        CopyGeomNearestNeighbor, 
			PrepGeomNearestNeighbor
                }
	},
	{	FALSE,
		xieValGeometry, 
		xieValGeomAntialias,
		DEFAULT_SPEED,
		0,
		sz_xieTecGeomAntialias,
		FALSE,
		TRUE,
		FALSE,
		FALSE,
		(CARD8 *)"ANTIALIAS",
		{
                        CopyGeomAntiAlias, 
			PrepGeomAntiAlias
                }
	},
	{	TRUE,
		xieValGeometry, 
		xieValGeomAntialiasByArea,
		DEFAULT_SPEED,
		0,
		sz_xieTecGeomAntialiasByArea,
		FALSE,
		TRUE,
		TRUE,
		FALSE,
		(CARD8 *)"ANTIALIAS-BY-AREA",
		{
                        NoParamCheck, 
			NoTechYet
                }
	},
	{	TRUE,
		xieValGeometry, 
		xieValGeomAntialiasByLPF,
		DEFAULT_SPEED,
		0,
		sz_xieTecGeomAntialiasByLowpass,
		FALSE,
		TRUE,
		TRUE,
		FALSE,
		(CARD8 *)"ANTIALIAS-BY-LOWPASS",
		{
                        NoParamCheck, 
			NoTechYet
                }
	},
	{	FALSE,
		xieValGeometry, 
		xieValGeomBilinearInterp,
		DEFAULT_SPEED,
		0,
		sz_xieTecGeomBilinearInterpolation,
		FALSE,
		TRUE,
		FALSE,
		FALSE,
		(CARD8 *)"BILINEAR-INTERPOLATION",
		{
                        CopyGeomBilinearInterp, 
                        PrepGeomBilinearInterp 
                }
	},
	{	TRUE,
		xieValGeometry, 
		xieValGeomGaussian,
		DEFAULT_SPEED,
		0,
		sz_xieTecGeomGaussian,
		FALSE,
		TRUE,
		TRUE,
		TRUE,
		(CARD8 *)"GAUSSIAN",
		{
                        NoParamCheck, 
			NoTechYet
                }
	}
};

/* Array of techniques for histogram */
Technique	Thistogram[] = { 
	{	FALSE,
		xieValHistogram, 
		xieValHistogramFlat,
		DEFAULT_SPEED,
		0,
		sz_xieTecHistogramFlat,
		FALSE,
		TRUE,
		FALSE,
		FALSE,
		(CARD8 *)"FLAT",
		{
                        NoParamCheck, 
			NoTechYet
                }
	},
	{	TRUE,
		xieValHistogram, 
		xieValHistogramGaussian,
		DEFAULT_SPEED,
		0,
		sz_xieTecHistogramGaussian,
		FALSE,
		TRUE,
		TRUE,
		TRUE,
		(CARD8 *)"GAUSSIAN",
		{
                        NoParamCheck, 
			NoTechYet
                }
	},
	{	TRUE,
		xieValHistogram, 
		xieValHistogramHyperbolic,
		DEFAULT_SPEED,
		0,
		sz_xieTecHistogramHyperbolic,
		FALSE,
		TRUE,
		TRUE,
		TRUE,
		(CARD8 *)"HYPERBOLIC",
		{
                        NoParamCheck, 
			NoTechYet
                }
	}
};

/* Array of techniques for white adjust */
Technique	Twhiteadjust[] = { 
	{	FALSE,
		xieValWhiteAdjust, 
		xieValWhiteAdjustNone,
		DEFAULT_SPEED,
		0,
		sz_xieTecWhiteAdjustNone,
		TRUE,
		TRUE,
		FALSE,
		FALSE,
		(CARD8 *)"NONE",
		{
                        NoParamCheck, 
			NoTechYet
                }
	},
	{	TRUE,
		xieValWhiteAdjust, 
		xieValWhiteAdjustCIELabShift,
		DEFAULT_SPEED,
		0,
		sz_xieTecWhiteAdjustCIELabShift,
		FALSE,
		TRUE,
		TRUE,
		TRUE,
		(CARD8 *)"CIELAB-SHIFT",
		{
                        NoParamCheck, 
			NoTechYet
                }
	}
};
#else	/* if defined(_XIEC_TECHNQ) */

extern	int 		technique_init();
extern	Bool 		TechNeedsParams();
extern	CARD16 		TechDefault();
extern	techVecPtr 	FindTechnique();

#endif	/* if defined(_XIEC_TECHNQ) */

#endif /* end _XIEH_TECHNQ */
