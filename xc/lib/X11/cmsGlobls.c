/*$XConsortium: XcmsGlobls.c,v 1.4 91/01/28 14:33:39 alt Exp $ */

/*
 * (c) Copyright 1990 1991 Tektronix Inc.
 * 	All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of Tektronix not be used
 * in advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.
 *
 * Tektronix disclaims all warranties with regard to this software, including
 * all implied warranties of merchantability and fitness, in no event shall
 * Tektronix be liable for any special, indirect or consequential damages or
 * any damages whatsoever resulting from loss of use, data or profits,
 * whether in an action of contract, negligence or other tortious action,
 * arising out of or in connection with the use or performance of this
 * software.
 *
 *
 *	NAME
 *		XcmsGlobls.c
 *
 *	DESCRIPTION
 *		Source file containing TekCMS globals
 *
 *
 */

/*
 *      EXTERNAL INCLUDES
 *              Include files that must be exported to any package or
 *              program using this package.
 */
#include "TekCMS.h"


/*
 *      INTERNAL INCLUDES
 *              Include files for local use only, therefore, NOT exported
 *		to any package or program using this package.
 */
#include "TekCMSP.h"
#include "TekCMSext.h"


/*
 *      EXTERNS
 *              External declarations required locally to this package
 *              that are not already declared in any of the included header
 *		files (external includes or internal includes).
 */
extern XcmsSCCFuncSet	LINEAR_RGB_SCCFuncSet;
extern XcmsSCCFuncSet	GRAY_SCCFuncSet;

/* UNDEFINED Color Space */
extern XcmsColorSpace	XcmsUNDEFINED_ColorSpace;

/* CIE XYZ Color Space */
extern XcmsColorSpace	XcmsCIEXYZ_ColorSpace;

/* CIE uvY Color Space */
extern XcmsColorSpace	XcmsCIEuvY_ColorSpace;

/* CIE xyY Color Space */
extern XcmsColorSpace	XcmsCIExyY_ColorSpace;

/* CIE Lab Color Space */
extern XcmsColorSpace	XcmsCIELab_ColorSpace;

/* CIE Luv Color Space */
extern XcmsColorSpace	XcmsCIELuv_ColorSpace;

#ifndef TekHVC_NOTIN_APILIB
/* TekHVC Color Space */
extern XcmsColorSpace	XcmsTekHVC_ColorSpace;
#endif

/* Device Dependent Color Space Structures */
extern XcmsColorSpace	XcmsLRGB_RGBi_ColorSpace;
extern XcmsColorSpace	XcmsLRGB_RGB_ColorSpace;


/*
 *      GLOBALS
 *              Variables declared in this package that are allowed
 *		to be used globally.
 */

    /*
     * Initial array of Device Independent Color Spaces
     */
XcmsColorSpace *_XcmsDIColorSpacesInit[] = {
    &XcmsCIEXYZ_ColorSpace,
    &XcmsCIEuvY_ColorSpace,
    &XcmsCIExyY_ColorSpace,
    &XcmsCIELab_ColorSpace,
    &XcmsCIELuv_ColorSpace,
#ifdef TekHVC_IN_APILIB
    &XcmsTekHVC_ColorSpace,
#endif
    &XcmsUNDEFINED_ColorSpace,
    NULL
};
    /*
     * Pointer to the array of pointers to XcmsColorSpace structures for
     * Device-Independent Color Spaces that are currently accessible by
     * the color management system.  End of list is indicated by a NULL pointer.
     */
XcmsColorSpace **_XcmsDIColorSpaces = _XcmsDIColorSpacesInit;

    /*
     * Initial array of Device Dependent Color Spaces
     */
XcmsColorSpace *_XcmsDDColorSpacesInit[] = {
    &XcmsLRGB_RGB_ColorSpace,
    &XcmsLRGB_RGBi_ColorSpace,
    NULL
};
    /*
     * Pointer to the array of pointers to XcmsColorSpace structures for
     * Device-Dependent Color Spaces that are currently accessible by
     * the color management system.  End of list is indicated by a NULL pointer.
     */
XcmsColorSpace **_XcmsDDColorSpaces = &_XcmsDDColorSpacesInit[0];

    /*
     * Initial array of Screen Color Characterization Function Sets
     */
XcmsSCCFuncSet	*_XcmsSCCFuncSetsInit[] = {
	&LINEAR_RGB_SCCFuncSet,
	&GRAY_SCCFuncSet,
	NULL};
    /*
     * Pointer to the array of pointers to XcmsSCCFuncSet structures
     * (Screen Color Characterization Function Sets) that are currently
     * accessible by the color management system.  End of list is
     * indicated by a NULL pointer.
     */
XcmsSCCFuncSet **_XcmsSCCFuncSets = _XcmsSCCFuncSetsInit;

    /*
     * X Consortium Registered Device-Independent Color Spaces
     *	Note that prefix must be in lowercase.
     */
char	XcmsCIEXYZ_prefix[] = "ciexyz";
char	XcmsCIEuvY_prefix[] = "cieuvy";
char	XcmsCIExyY_prefix[] = "ciexyy";
char	XcmsCIELab_prefix[] = "cielab";
char 	XcmsCIELuv_prefix[] = "cieluv";
char	XcmsTekHVC_prefix[] = "tekhvc";
    /*
     * Registered Device-Dependent Color Spaces
     */
char	XcmsRGBi_prefix[] = "rgbi";
char	XcmsRGB_prefix[] = "rgb";

XcmsRegColorSpaceEntry _XcmsRegColorSpaces[] = {
    XcmsCIEXYZ_prefix,	XCMS_CIEXYZ_FORMAT,
    XcmsCIEuvY_prefix,	XCMS_CIEuvY_FORMAT,
    XcmsCIExyY_prefix,	XCMS_CIExyY_FORMAT,
    XcmsCIELab_prefix,	XCMS_CIELab_FORMAT,
    XcmsCIELuv_prefix,	XCMS_CIELuv_FORMAT,
    XcmsTekHVC_prefix,	XCMS_TekHVC_FORMAT,
    XcmsRGB_prefix,	XCMS_RGB_FORMAT,
    XcmsRGBi_prefix,	XCMS_RGBi_FORMAT,
    NULL, 0
};
