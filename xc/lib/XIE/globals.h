/* $XConsortium: globals.h,v 1.1 93/07/19 11:39:22 mor Exp $ */

/******************************************************************************
Copyright 1993 by the Massachusetts Institute of Technology

                        All Rights Reserved

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation, and that the name of M.I.T. not be used in advertising or
publicity pertaining to distribution of the software without specific,
written prior permission.  M.I.T. makes no representations about the
suitability of this software for any purpose.  It is provided "as is"
without express or implied warranty.
******************************************************************************/

XieExtInfo *_XieExtInfoHeader = NULL;

extern void _XieElemImportClientLUT();
extern void _XieElemImportClientPhoto();
extern void _XieElemImportClientROI();
extern void _XieElemImportDrawable();
extern void _XieElemImportDrawablePlane();
extern void _XieElemImportLUT();
extern void _XieElemImportPhotomap();
extern void _XieElemImportROI();
extern void _XieElemArithmetic();
extern void _XieElemBandCombine();
extern void _XieElemBandExtract();
extern void _XieElemBandSelect();
extern void _XieElemBlend();
extern void _XieElemCompare();
extern void _XieElemConstrain();
extern void _XieElemConvertFromIndex();
extern void _XieElemConvertFromRGB();
extern void _XieElemConvertToIndex();
extern void _XieElemConvertToRGB();
extern void _XieElemConvolve();
extern void _XieElemDither();
extern void _XieElemGeometry();
extern void _XieElemLogical();
extern void _XieElemMatchHistogram();
extern void _XieElemMath();
extern void _XieElemPasteUp();
extern void _XieElemPoint();
extern void _XieElemUnconstrain();
extern void _XieElemExportClientHistogram();
extern void _XieElemExportClientLUT();
extern void _XieElemExportClientPhoto();
extern void _XieElemExportClientROI();
extern void _XieElemExportDrawable();
extern void _XieElemExportDrawablePlane();
extern void _XieElemExportLUT();
extern void _XieElemExportPhotomap();
extern void _XieElemExportROI();

void (*(_XieElemFuncs[]))() =
{
    _XieElemImportClientLUT,
    _XieElemImportClientPhoto,
    _XieElemImportClientROI,
    _XieElemImportDrawable,
    _XieElemImportDrawablePlane,
    _XieElemImportLUT,
    _XieElemImportPhotomap,
    _XieElemImportROI,
    _XieElemArithmetic,
    _XieElemBandCombine,
    _XieElemBandExtract,
    _XieElemBandSelect,
    _XieElemBlend,
    _XieElemCompare,
    _XieElemConstrain,
    _XieElemConvertFromIndex,
    _XieElemConvertFromRGB,
    _XieElemConvertToIndex,
    _XieElemConvertToRGB,
    _XieElemConvolve,
    _XieElemDither,
    _XieElemGeometry,
    _XieElemLogical,
    _XieElemMatchHistogram,
    _XieElemMath,
    _XieElemPasteUp,
    _XieElemPoint,
    _XieElemUnconstrain,
    _XieElemExportClientHistogram,
    _XieElemExportClientLUT,
    _XieElemExportClientPhoto,
    _XieElemExportClientROI,
    _XieElemExportDrawable,
    _XieElemExportDrawablePlane,
    _XieElemExportLUT,
    _XieElemExportPhotomap,
    _XieElemExportROI
};


XieTechFuncRec *_XieTechFuncs[xieValMaxTechGroup];

Bool _XieTechFuncsInitialized = 0;
