/* $XConsortium$ */

/***********************************************************
Copyright 1989, 1990 by Sun Microsystems, Inc. and the X Consortium.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Sun Microsystems,
the X Consortium, and MIT not be used in advertising or publicity 
pertaining to distribution of the software without specific, written 
prior permission.  

SUN MICROSYSTEMS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, 
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT 
SHALL SUN MICROSYSTEMS BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

#ifndef ESPROTOSTR_H
#define ESPROTOSTR_H

/* Matches revision 5.0C */
#include "X11/PEX5/PEXprotost.h"

typedef struct {
    pexColorType	colorType B16;	/* ColorType enumerated type */
    CARD16		unused B16;
    pexColor           color;
} esColorSpecifier;

typedef struct {
    PEXFLOAT		ambient;
    PEXFLOAT		diffuse;
    PEXFLOAT		specular;
    PEXFLOAT		specularConc;
    PEXFLOAT		transmission;  /* 0.0 = opaque, 1.0 = transparent */
    esColorSpecifier   specularColor;
} esReflectionAttr;

typedef struct {
    pexEnumTypeIndex	lineType B16;
    pexEnumTypeIndex	polylineInterp B16;
    pexCurveApprox	curveApprox;
    PEXFLOAT		lineWidth;
    esColorSpecifier	lineColor;
} esLineBundleEntry;

typedef struct {
    pexEnumTypeIndex	markerType B16;
    INT16		unused B16;
    PEXFLOAT		markerScale;
    esColorSpecifier	markerColor;
} esMarkerBundleEntry;

typedef struct {
    CARD16		textFontIndex B16;
    CARD16		textPrecision B16;
    PEXFLOAT		charExpansion;
    PEXFLOAT		charSpacing;
    esColorSpecifier	textColor;
} esTextBundleEntry;

typedef struct {
    pexEnumTypeIndex    interiorStyle B16;
    INT16		interiorStyleIndex B16;
    pexEnumTypeIndex    reflectionModel B16;
    pexEnumTypeIndex    surfaceInterp B16;
    pexEnumTypeIndex    bfInteriorStyle B16;
    INT16		bfInteriorStyleIndex B16;
    pexEnumTypeIndex    bfReflectionModel B16;
    pexEnumTypeIndex    bfSurfaceInterp B16;
    pexSurfaceApprox    surfaceApprox;
    esColorSpecifier  surfaceColor; /* burp! bandaid until PR2.0 */
    esReflectionAttr   reflectionAttr; /* burp! bandaid until PR2.0 */
    esColorSpecifier  bfSurfaceColor; /* burp! bandaid until PR2.0 */
    esReflectionAttr   bfReflectionAttr; /* burp! bandaid until PR2.0 */
} esInteriorBundleEntry;

typedef struct {
    pexSwitch		edges;
    CARD8		unused;
    pexEnumTypeIndex	edgeType B16;
    PEXFLOAT		edgeWidth;
    esColorSpecifier	edgeColor;
} esEdgeBundleEntry;

/* a esColorEntry is just a esColorSpecifier
*/

typedef struct {
    pexEnumTypeIndex	lightType B16;
    INT16		unused B16;
    pexVector3D		direction;
    pexCoord3D		point;
    PEXFLOAT		concentration;
    PEXFLOAT		spreadAngle;
    PEXFLOAT		attenuation1;
    PEXFLOAT		attenuation2;
    esColorSpecifier	lightColor;
} esLightEntry;

typedef struct {
    pexSwitch		mode;
    CARD8		unused;
    CARD16		unused2 B16;
    PEXFLOAT		frontPlane;
    PEXFLOAT		backPlane;
    PEXFLOAT		frontScaling;
    PEXFLOAT		backScaling;
    esColorSpecifier	depthCueColor;
} esDepthCueEntry;

/*  Font structures */

typedef struct {
    CARD32	name B32;
    union {
	PEXFLOAT	fval;
	CARD32	uval B32;
    } value B32;
} esFontProp;

/* record for esAddToNameSet and esRemoveFromNameSet */
typedef struct {
    INT32       maskIndex;
    pexBitmask  nameMask;
} esName;


/* Sphere GDP3 structures  to match those in esgdp.h */

typedef struct {
    INT32	colorType;
    INT32	numSpheres;
    /*  List of sphere structures  */
} esSphereData;

typedef struct {
    pexCoord3D	center;		/* center of sphere */
} esSphereCenter;

typedef struct {
    pexCoord3D	center;		/* center of sphere */
    PEXFLOAT	radius;		/* radius of sphere */
} esSphereCenterRadius;

typedef struct {
    pexCoord3D	center;		/* center of sphere */
    pexColor   color;		/* color of sphere */
} esSphereCenterColor;

typedef struct {
    pexCoord3D  center;		/* center of sphere */
    PEXFLOAT	radius;		/* radius of sphere */
    pexColor   color;		/* color of sphere */
} esSphereCenterRadiusColor;



/* Sphere GSE structures to match those in 
   /syssoft1/Xbeta/mit/extensions/lib/PEX/esgdp.h */

typedef struct {
    PEXFLOAT   	radius;		/* default radius for spheres */
} esSphereRadiusData;

typedef struct {
    INT32   	div;		/* number of lat. and long. divisions */
} esSphereDivisionData;



/* Cylinder GDP3 structures to match those in 
   /syssoft1/Xbeta/mit/extensions/lib/PEX/esgdp.h */

typedef struct {
    INT32	colorType;
    INT32       numLists;	/* number of sets of cylinders */
    /* List of cylinder list structures */
} esCylinderListData;

typedef struct {
    INT32        numCylinders;    	/* number of cylinder definitions */
    /* List of cylinder structures */
} esCylinderList;

typedef struct {
    pexCoord3D    	center;		/* center of cylinder */
} esCylinderCenter;

typedef struct {
    pexCoord3D center;		/* center of cylinder */
    PEXFLOAT      radius;		/* radius of cylinder */
} esCylinderCenterRadius;

typedef struct {
    pexCoord3D    	center;		/* center of cylinder */
    pexColor   	color;		/* color of cylinder */
} esCylinderCenterColor;

typedef struct {
    pexCoord3D  center;		/* center of cylinder */
    PEXFLOAT	radius;		/* radius of cylinder */
    pexColor    color;  	/* color of cylinder */
} esCylinderCenterRadiusColor;

/* Cylinder GSE structures to match those in 
   /syssoft1/Xbeta/mit/extensions/lib/PEX/esgdp.h */

typedef struct {
    PEXFLOAT   	radius;		/* default radius for cylinders */
} esCylinderRadiusData;

typedef struct {
    INT32   	div;		/* number of lat. and long. divisions */
} esCylinderDivisionData;


/* Stereo view indices GSE to match the one in 
   /syssoft1/Xbeta/mit/extensions/lib/PEX/esgdp.h */

typedef struct {
    INT32   mono;
    INT32   left;
    INT32   right;
} esStereViewIndicesData;

/* Polyline over fillarea tolerance GSE to match the one in
   /syssoft1/Xbeta/mit/extensions/lib/PEX/include/esgdp.h */

typedef struct {
    PEXFLOAT     tolerance;
} esFillareaToleranceData;

/* Fillarea front/back z-buffer win GSE to match the one in
   /syssoft1/Xbeta/mit/extensions/lib/PEX/include/esgdp.h */

typedef struct {
    INT32    value;
} esFrontBackDistinguishData;

/* Polyline quality GSE structure */

typedef struct {
    INT32    flag;
} esPolylineQualityData;

typedef struct {
    pexColorType	colorType B16;	/* ColorType enumerated type */
    CARD16		unused B16;
    CARD32		index;
    CARD32		num_colors;
    PEXFLOAT		lower;
    PEXFLOAT		upper;
    /* This is followd by LISTofCOLOR */
} DMSingleUnif;

typedef struct {
    pexColorType	colorType B16;	/* ColorType enumerated type */
    CARD16		unused B16;
    CARD32		index;
    CARD32		num_colors;
    /* This is followed by LISTofFLOAT and LISTofCOLOR */
} DMSingleNUnif;

typedef struct {
    pexColorType	colorType B16;	/* ColorType enumerated type */
    CARD16		unused B16;
    CARD32		index1;
    CARD32		index2;
    PEXFLOAT		lowera;
    PEXFLOAT		uppera;
    PEXFLOAT		lowerb;
    PEXFLOAT		upperb;
    CARD32		color_lengths;
    /* This is followed by LISTofCARD32 and  LISTofLISTofCOLOR */
} DMDoubleUnif;

typedef struct {
    pexColorType	colorType B16;	/* ColorType enumerated type */
    CARD16		unused B16;
    CARD32		index1;
    CARD32		index2;
    CARD32		color_lengths;
    /* This is followed by LISTofCARD32, rangea, rangeb, and */
    /* LISTofLISTofCOLOR  */
} DMDoubleNUnif;

typedef XID             esTexture;

typedef struct {
    CARD16              channelNum;
    CARD16              colorType;
    pexColor            color1;
    pexColor            color2; /* must be same type as color1 */
} esTextureBlendOp;

typedef struct {
    CARD16              channelNum;
    CARD16              colorType;
    pexColor            environmentColor;
} esTextureEnvironmentOp;

typedef struct {
    CARD16              channelNum;
    CARD16              colorType;
} esTextureReplaceOp;

typedef struct {
    CARD16              channelNum;
    CARD16              colorType;
} esTextureModulateOp;

typedef struct {
    pexEnumTypeIndex            component;
    pexEnumTypeIndex            textureOp;
    union {
        esTextureBlendOp        blend;
        esTextureEnvironmentOp  environment;
        esTextureReplaceOp      replace;
        esTextureModulateOp     modulate;
    } info;
} esRenderComponentDesc;

typedef struct {
    pexEnumTypeIndex    boundaryCondition;
    CARD16              unused;
} esBoundaryConditionRec;

typedef struct {
    esTexture                   textureId;
    pexEnumTypeIndex            minInterpMethod;
    pexEnumTypeIndex            magInterpMethod;
    PEXFLOAT                    depthInterpFactor;
    PEXFLOAT                    alpha;
    pexColor                    clampColor;
    CARD16                      colorType;
    CARD16                      numFreqWeights;
    CARD16                      numBoundaryConditions;
    CARD16                      numRenderCompDesc;
    /* LISTof PEXFLOAT(numFreqWeights) */
    /* LISTof esTextureBoundaryConditionRec(numBoundaryConditions) */
    /* LISTof esRenderComponentDesc(numRenderCompDesc) */
} esTextureMipmap;

typedef struct {
    pexEnumTypeIndex    mapType;
    CARD16              unused B16;
    union {
        esTextureMipmap textureMipmap;
    } info;
} esTextureDesc;
 
typedef struct {
    pexMatrix           orientationMatrix;
    CARD16              numTextureCoord;
    CARD16              numTextureDesc;
    /* LISTof CARD32(numTextureCoord) */
    /* LISTof esTextureDesc(numTextureDesc) */
} esTextureInfoRec;
 
typedef struct {
    CARD32              numTextures;
    /* LISTof esTextureInfoRec(numTextures) */
} DMTexture;
 
typedef struct {
    CARD32		method;
    CARD32		num_selectors;
    /* This is followed by the list of selectors and then one of */
    /* DMSingleUnif, DMSingleNUnif, DMDoubleUnif, DMDoubleNUnif */
} esDataMappingRecord;

typedef esDataMappingRecord espexDataMappingEntry;

#endif /* ESPROTOSTR_H */
