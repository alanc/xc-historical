/* $XConsortium: XcmsStCols.c,v 1.5 91/02/12 16:13:51 dave Exp $" */

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
 *		XcmsStCols.c
 *
 *	DESCRIPTION
 *		Source for XcmsStoreColors
 *
 *
 */

#include "Xlibint.h"
#include "Xcmsint.h"


/************************************************************************
 *									*
 *			PUBLIC ROUTINES					*
 *									*
 ************************************************************************/

/*
 *	NAME
 *		XcmsStoreColors - Store Colors
 *
 *	SYNOPSIS
 */
Status
XcmsStoreColors(dpy, colormap, pColors_in,
	nColors, pCompressed)
    Display *dpy;
    Colormap colormap;
    XcmsColor *pColors_in;
    unsigned int nColors;
    Bool *pCompressed;
/*
 *	DESCRIPTION
 *		Given device-dependent or device-independent color
 *		specifications, this routine will convert them to X RGB
 *		values then use it in a call to XStoreColors.
 *
 *	RETURNS
 *		XCMS_FAILURE if failed;
 *		XCMS_SUCCESS if it succeeded without gamut compression;
 *		XCMS_SUCCESS_WITH_COMPRESSION if it succeeded with gamut
 *			compression;
 *
 *		Since XStoreColors has no return value, this routine
 *		does not return color specifications of the colors actually
 *		stored.
 */
{
    XcmsColor Color1;
    XcmsColor *pColors_tmp;
    Status retval;

    /*
     * Make copy of array of color specifications so we don't
     * overwrite the contents.
     */
    if (nColors > 1) {
	pColors_tmp = (XcmsColor *) Xmalloc(nColors * sizeof(XcmsColor));
    } else {
	pColors_tmp = &Color1;
    }
    bcopy((char *)pColors_in, (char *)pColors_tmp,
	    nColors * sizeof(XcmsColor));

    /*
     * Call routine to store colors using the copied color structures
     */
    retval = _XcmsSetGetColors (XStoreColors, dpy, colormap,
	    pColors_tmp, nColors, XCMS_RGB_FORMAT, pCompressed);

    /*
     * Free copies as needed.
     */
    if (nColors > 1) {
	Xfree((char *)pColors_tmp);
    }

    /*
     * Ah, finally return.
     */
    return(retval);
}
