/* $XConsortium: XcmsSetGet.c,v 1.9 91/01/27 00:39:41 alt Exp $" */

/*
 * (c) Copyright 1989 1990 1991 Tektronix Inc.
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
 *		XcmsSetGet.c
 *
 *	DESCRIPTION
 *		Source for _XcmsSetGetColors()
 *
 *
 */

/*
 *      EXTERNAL INCLUDES
 *              Include files that must be exported to any package or
 *              program using this package.
 */
#include "Xlib.h"
#include "TekCMS.h"


/*
 *      INTERNAL INCLUDES
 *              Include files for local use only, therefore, NOT exported
 *		to any package or program using this package.
 */
#include "Xlibos.h"


/*
 *      EXTERNS
 */

extern void _XcmsRGB_to_XColor();
extern void _XColor_to_XcmsRGB();



/************************************************************************
 *									*
 *			API PRIVATE ROUTINES				*
 *									*
 ************************************************************************/

/*
 *	NAME
 *		XcmsSetColors - 
 *
 *	SYNOPSIS
 */
Status
_XcmsSetGetColors(xColorProc, dpy, cmap, pColors_in_out, nColors,
	result_format, pCompressed)
    Status (*xColorProc)();
    Display *dpy;
    Colormap cmap;
    XcmsColor *pColors_in_out;
    unsigned int nColors;
    XcmsSpecFmt result_format;
    Bool *pCompressed;
/*
 *	DESCRIPTION
 *		Routine containing code common to:
 *			XcmsAllocColor
 *			XcmsQueryColor
 *			XcmsQueryColors
 *			XcmsStoreColor
 *			XcmsStoreColors
 *
 *	RETURNS
 *		XCMS_FAILURE if failed;
 *		XCMS_SUCCESS if it succeeded without gamut compression;
 *		XCMS_SUCCESS_WITH_COMPRESSION if it succeeded with gamut
 *			compression;
 */
{
    XcmsCCC *pCCC;
    XColor *pXColors_in_out;
    Status retval = XCMS_SUCCESS;

    /*
     * Argument Checking
     *	1. Assume xColorProc is correct
     *	2. Insure pCCC not NULL
     *	3. Assume cmap correct (should be checked by Server)
     *	4. Insure pColors_in_out valid
     *	5. Assume method_in is valid (should be checked by Server)
     *	6. Insure nColors > 0
     */

    if (dpy == NULL) {
	return(XCMS_FAILURE);
    }

    if (nColors == 0) {
	return(XCMS_SUCCESS);
    }

    if (result_format == XCMS_UNDEFINED_FORMAT) {
	return(XCMS_FAILURE);
    }

    if (!((*xColorProc == XAllocColor) || (*xColorProc == XStoreColor)
	    || (*xColorProc == XStoreColors) || (*xColorProc == XQueryColor) 
	    || (*xColorProc == XQueryColors))) {
	return(XCMS_FAILURE);
    }

    if ((pCCC = XcmsCCCofColormap(dpy, cmap)) == (XcmsCCC *)NULL) {
	return(XCMS_FAILURE);
    }

    if ((*xColorProc == XAllocColor) || (*xColorProc == XStoreColor)
	    || (*xColorProc == XQueryColor)) {
	nColors = 1;
    }

    /*
     * Allocate space for XColors
     */
    if ((pXColors_in_out = (XColor *)Xcalloc(nColors, sizeof(XColor))) ==
	    NULL) {
	return(XCMS_FAILURE);
    }

    if ((*xColorProc == XQueryColor) || (*xColorProc == XQueryColors)) {
	goto Query;
    }
    /*
     * Convert to RGB, adjusting for white point differences if necessary.
     */
    if ((retval = XcmsConvertColors(pCCC, pColors_in_out, nColors, XCMS_RGB_FORMAT,
	    pCompressed)) == XCMS_FAILURE) {
	return(XCMS_FAILURE);
    }

Query:
    /*
     * Convert XcmsColor to XColor structures
     */
    _XcmsRGB_to_XColor(pColors_in_out, pXColors_in_out, nColors);

    /*
     * Now make appropriate X Call
     */
    if (*xColorProc == XAllocColor) {
	if ((*xColorProc)(pCCC->dpy, cmap, pXColors_in_out) == 0) {
	    Xfree((char *)pXColors_in_out);
	    return(XCMS_FAILURE);
	}
    } else if ((*xColorProc == XQueryColor) || (*xColorProc == XStoreColor)) {
	/* Note: XQueryColor and XStoreColor do not return any Status */
	(*xColorProc)(pCCC->dpy, cmap, pXColors_in_out);
    } else if ((*xColorProc == XQueryColors) || (*xColorProc == XStoreColors)){
	/* Note: XQueryColors and XStoreColors do not return any Status */
	(*xColorProc)(pCCC->dpy, cmap, pXColors_in_out, nColors);
    } else {
	Xfree((char *)pXColors_in_out);
	return(XCMS_FAILURE);
    }

    if ((*xColorProc == XStoreColor) || (*xColorProc == XStoreColors)) {
	Xfree((char *)pXColors_in_out);
	return(retval);
    }

    /*
     * Now, convert returned XColor(i.e., rgb) to XcmsColor structures
     */
    _XColor_to_XcmsRGB(pCCC, pXColors_in_out, pColors_in_out, nColors);
    Xfree((char *)pXColors_in_out);

    /*
     * Then, convert XcmsColor structures to the original specification
     *    format.  Note that we must use NULL instead of passing
     *    pCompressed.
     */

    if (result_format != XCMS_RGB_FORMAT) {
	if (XcmsConvertColors(pCCC, pColors_in_out, nColors, result_format,
		(Bool *) NULL) == XCMS_FAILURE) {
	    return(XCMS_FAILURE);
	}
    }
    return(retval);
}
