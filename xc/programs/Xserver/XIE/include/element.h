/* $XConsortium: element.h,v 1.1 93/10/26 10:03:41 rws Exp $ */
/* module element.h */
/*****************************************************************************
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
******************************************************************************

	element.h -- device independent definitions for elements

	Dean Verheiden -- AGE Logic, Inc. April 1993

*****************************************************************************/


#ifndef _XIEH_ELEMENT
#define _XIEH_ELEMENT

#include <flostr.h>

/*
 *  Generic Definitions
 */
#define SRCtag   0	/* inFlo index for src of single input elements	    */
#define SRCt1	 0	/* inFlo index for src1 of multi-input elements	    */
#define SRCt2	 1	/* inFlo index for src2 of multi-input elements	    */
#define SRCt3	 2	/* inFlo index for src3 of multi-input elements	    */
#define LUTtag	 1	/* inFlo index for lut input of point element	    */
			/* inFlo index for alpha stored in private structure*/
			/* inFlo index for domain input is inFloCnt - 1	    */

#define IMPORT	 0	/* inFlo index for "imported" data		    */


#if defined(_XIEC_IDRAW) || defined(_XIEC_IDRAWP)
/*
 * dixie element-private data for the ImportDrawable and ImportDrawablePlane 
 * elements
 */
typedef struct _idrawdef {
  CARD32	pvtBytes;
  DrawablePtr	pDraw;
} iDrawDefRec, *iDrawDefPtr;
#endif /* _XIEC_IDRAW || _XIEC_IDRAWP */

#if defined(_XIEC_ILUT) 
/*
 * dixie element-private data for the ImportLUT element
 */
typedef struct _ilutdef {
  CARD32        pvtBytes;
  lutPtr        lut;
} iLUTDefRec, *iLUTDefPtr;
#endif /* _XIEC_ILUT */

#if defined(_XIEC_IPHOTO)
/*
 * dixie element-private data for the ImportPhotomap element
 */
typedef struct _iphotodef {
  CARD32		pvtBytes;
  photomapPtr		map;		
} iPhotoDefRec, *iPhotoDefPtr;
#endif /* _XIEC_IPHOTO || _XIEC_ICPHOTO */

#if defined(_XIEC_IROI)
/*
 * dixie element-private data for the ImportROI element
 */
typedef struct _iroidef {
  CARD32	pvtBytes;
  roiPtr roi;
} iROIDefRec, *iROIDefPtr;
#endif /* _XIEC_IROI */

#if defined(_XIEC_PARITH)
/*
 * dixie element-private data for the Arithmetic element
 */
typedef struct _parith {
  CARD32	pvtBytes;
  double        constant[xieValMaxBands];
} pArithDefRec, *pArithDefPtr;
#endif /* _XIEC_PARITH */

#if defined(_XIEC_PBANDE)
/*
 * dixie element-private data for the band extract element
 */
typedef struct _pbandext {
  CARD32	pvtBytes;
  double	coef[3];
  double	bias;
} pBandExtDefRec, *pBandExtDefPtr;
#endif /* _XIEC_PBANDE */

#if defined(_XIEC_PBLEND)
/*
 * dixie element-private data for the Arithmetic element
 */
typedef struct _pblend {
  CARD32	pvtBytes;
  CARD16	aindex;			/* if non zero, index of alpha plane */
  CARD16	dindex;			/* if non zero, index of proc domain */
  double        constant[xieValMaxBands];
  double	alphaConst;
} pBlendDefRec, *pBlendDefPtr;
#endif /* _XIEC_PBLEND */

#if defined(_XIEC_PCFRGB)
/*
 * dixie element-private technique data for the ConvertFromRGB element
 */
typedef struct _ptecRBGtoCIELab_and_XYZdef {
  double	             matrix[9];
  double		     whitePoint[3]; /* Optional whitePoint correction */
  techVecPtr		     whiteTec;
  xieTypWhiteAdjustTechnique whiteAdjusted; 
  CARD16		     lenParams;	    /* Length for whitePoint params   */
} pTecRGBToCIEDefRec, *pTecRGBToCIEDefPtr;

typedef struct _ptecRGBToYCbCr {
  CARD32 levels0;
  CARD32 levels1;
  CARD32 levels2;
  double red;
  double green;
  double blue;
  double bias0;
  double bias1;
  double bias2;
} pTecRGBToYCbCrDefRec, *pTecRGBToYCbCrDefPtr;

typedef struct _ptecRGBToYCC {
  CARD32 levels0;
  CARD32 levels1;
  CARD32 levels2;
  double red;
  double green;
  double blue;
  double scale;
} pTecRGBToYCCDefRec, *pTecRGBToYCCDefPtr;

#endif /* _XIEC_PCFRGB */

#if defined(_XIEC_PCFROMI)
/*
 * dixie element-private data for the ConvertFromIndex element
 */
typedef struct _pcfromidef {
  CARD32	pvtBytes;
  ColormapPtr   cmap;
  VisualPtr	visual;
  CARD16	class;
  CARD16	cells;
  CARD32	pixMsk[3];
  CARD8		pixPos[3];
  CARD8		precShift;
} pCfromIDefRec, *pCfromIDefPtr;
#endif /* _XIEC_PCFROMI */

#if defined(_XIEC_PCNST)
/*
 * dixie element-private data for the Constrain element
 */
typedef struct _pcnst {
  double	input_low[3];
  double	input_high[3];
  CARD32        output_low[3];
  CARD32        output_high[3];
} pCnstDefRec, *pCnstDefPtr;
#endif /* _XIEC_PCNST */

#if defined(_XIEC_PCOMP)
/*
 * dixie element-private data for the Arithmetic element
 */
typedef struct _pcompare {
  CARD32	pvtBytes;
  double        constant[xieValMaxBands];
} pCompareDefRec, *pCompareDefPtr;
#endif /* _XIEC_PCOMP */

#if defined(_XIEC_PCONV)
/*
 * constant technique private data for the Convolve element
 */ 
#define	ConvFloat	float

typedef struct _pconv {
  ConvFloat	constant[3];
} pTecConvolveConstantDefRec, *pTecConvolveConstantDefPtr;
#endif /* _XIEC_PCONV */

#if defined(_XIEC_PCTOI)
/*
 * dixie element-private data for the ConvertToIndex element
 */
typedef struct _pctoidef {
  CARD32	pvtBytes;
  colorListPtr  list;
  ColormapPtr   cmap;
  VisualPtr	visual;
  CARD16	mask[3];
  CARD16	cells;
  CARD8		shft[3];
  CARD8		class;
  CARD8		depth;
  CARD8		stride;
  BOOL		graySrc;
  BOOL		dynamic;
  BOOL		preFmt;
  BOOL		doHist;
  CARD32	levels[3];
} pCtoIDefRec, *pCtoIDefPtr;

typedef struct _pTecConvertToIndexMatchdef {
  CARD32	pvtBytes;
  double	matchLimit;
  double	grayLimit;
} pTecConvertToIndexMatchDefRec, *pConvertToIndexMatchDefPtr;
#endif /* _XIEC_PCTOI */

#if defined(_XIEC_PCTRGB)
/*
 * dixie element-private technique data for the ConvertToRGB element
 */
typedef struct _ptecCIELab_and_XYZtoRGBdef {
  double	             matrix[9];
  double		     whitePoint[3]; /* Optional whitePoint correction */
  techVecPtr		     whiteTec;
  xieTypWhiteAdjustTechnique whiteAdjusted; 
  CARD16		     numWhiteParams;/* Length for whitePoint params   */
  techVecPtr		     gamutTec;
  xieTypGamutTechnique       gamutTechnique; 
  CARD16		     numGamutParams;/* Length for gamut params   */
} pTecCIEToRGBDefRec, *pTecCIEToRGBDefPtr;

typedef struct _ptecYCCToRGB {
  CARD32 levels0;
  CARD32 levels1;
  CARD32 levels2;
  double red;
  double green;
  double blue;
  double scale;
  techVecPtr		     gamutTec;
  xieTypGamutTechnique       gamutTechnique; 
  CARD16		     numGamutParams;/* Length for gamut params   */
} pTecYCCToRGBDefRec, *pTecYCCToRGBDefPtr;

typedef struct _ptecYCbCrToRGB {
  CARD32 levels0;
  CARD32 levels1;
  CARD32 levels2;
  double red;
  double green;
  double blue;
  double bias0;
  double bias1;
  double bias2;
  techVecPtr		     gamutTec;
  xieTypGamutTechnique       gamutTechnique; 
  CARD16		     numGamutParams;/* Length for gamut params   */
} pTecYCbCrToRGBDefRec, *pTecYCbCrToRGBDefPtr;

#endif /* _XIEC_PCTRGB */


#if defined(_XIEC_PGEOM)
/*
 * dixie element-private data for the Geoemtry element
 */
typedef struct _geom_elem {
  double        coeffs[6];
  double        constant[xieValMaxBands];
  int		do_band[3];
} pGeomDefRec, *pGeomDefPtr;

/*
 * constant technique private data for the Geometry element
 */ 
typedef struct _geom_nn {
  int modify;
} pTecGeomNearestNeighborDefRec, *pTecGeomNearestNeighborDefPtr;

typedef struct _geom_gauss {
  double	sigma;
  double	normalize;
  int 		radius;
  Bool		simple;
} pTecGeomGaussianDefRec, *pTecGeomGaussianDefPtr;

#endif /* _XIEC_PGEOM */

#if defined(_XIEC_PHIST)

/* 
 * technique private structures
 */

typedef struct _pmhistgaussian {
  double	mean;
  double	sigma;
} pTecHistogramGaussianDefRec, *pTecHistogramGaussianDefPtr;

typedef struct _pmhisthyperbolic {
  double	constant;
  Bool		shapeFactor;
} pTecHistogramHyperbolicDefRec, *pTecHistogramHyperbolicDefPtr;

#endif /* _XIEC_PHIST */

#if defined(_XIEC_PLOGIC)
/*
 * dixie element-private data for the Logical element
 */
typedef struct _plogic {
  CARD32	pvtBytes;
  double        constant[xieValMaxBands];
} pLogicDefRec, *pLogicDefPtr;
#endif /* _XIEC_PLOGIC */

#if defined(_XIEC_PPASTE)
/*
 * dixie element-private data for the Paste Up element
 */
#define	PasteUpFloat	float

typedef struct _ppasteup {
  CARD32	pvtBytes;
  PasteUpFloat  constant[xieValMaxBands];
} pPasteUpDefRec, *pPasteUpDefPtr;
#endif /* _XIEC_PPASTE */

#if defined(_XIEC_ECPHOTO)
/*
 * JPEG Baseline technique private data for the export client photo element
 */
typedef struct _eEndcodeJPEGBaselinedef {
  CARD32	pvtBytes;
  CARD8		*q;
  CARD8		*a;
  CARD8		*d;
} eTecEncodeJPEGBaselineDefRec, *eTecEncodeJPEGBaselineDefPtr;
#endif /* _XIEC_ECPHOTO */

#if defined(_XIEC_EDRAW)
/*
 * dixie element-private data for the ExportDrawable element
 */
typedef struct _edrawdef {
  CARD32	pvtBytes;
  DrawablePtr	pDraw;
  GCPtr		pGC;
} eDrawDefRec, *eDrawDefPtr;
#endif /* _XIEC_EDRAW */

#if defined(_XIEC_EDRAWP)
/*
 * dixie element-private data for the ExportDrawable element
 */
typedef struct _edrawpdef {
  CARD32	pvtBytes;
  DrawablePtr	pDraw;
  GCPtr		pGC;
} eDrawPDefRec, *eDrawPDefPtr;
#endif /* _XIEC_EDRAWP */

#if defined(_XIEC_ELUT)
/*
 * dixie element-private data for the ExportLUT element
 */
typedef struct _elutdef {
  CARD32        pvtBytes;
  lutPtr        lut;
} eLUTDefRec, *eLUTDefPtr;
#endif /* _XIEC_ELUT */

#if defined(_XIEC_EPHOTO)
/*
 * dixie element-private data for the ExportPhotomap element
 */
typedef struct _ephotodef {
  CARD32	pvtBytes;
  photomapPtr   map;
  void		*pvtParms;
} ePhotoDefRec, *ePhotoDefPtr;
#endif /* _XIEC_EPHOTO */

#if defined(_XIEC_EROI)
/*
 * dixie element-private data for the ExportROI element
 */
typedef struct _eroidef {
  CARD32	pvtBytes;
  roiPtr   	roi;
} eROIDefRec, *eROIDefPtr;
#endif /* _XIEC_EROI */

#endif /* module _XIEH_ELEMENT */
