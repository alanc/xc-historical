/* $XConsortium: XcmsAllCol.c,v 1.1 91/01/30 18:30:26 dave Exp $" */

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
 *		XcmsAllCol.c
 *
 *	DESCRIPTION
 *		Source for XcmsAllocColor
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
 *	NAME
 *		XcmsAllocColor - Allocate Color
 *
 *	SYNOPSIS
 */
Status
XcmsAllocColor(dpy, colormap, pXcmsColor_in_out, result_format)
    Display *dpy;
    Colormap colormap;
    XcmsColor *pXcmsColor_in_out;
    XcmsSpecFmt result_format;
/*
 *	DESCRIPTION
 *		Given a device-dependent or device-independent color
 *		specification, XcmsAllocColor will convert it to X RGB
 *		values then use it in a call to XAllocColor.
 *
 *	RETURNS
 *		XCMS_FAILURE if failed;
 *		XCMS_SUCCESS if it succeeded without gamut compression;
 *		XCMS_SUCCESS_WITH_COMPRESSION if it succeeded with gamut
 *			compression;
 *
 *		Also returns the pixel value of the color cell and a color
 *		specification of the color actually stored.
 *
 */
{
    return(_XcmsSetGetColors (XAllocColor, dpy, colormap, pXcmsColor_in_out, 1,
	    result_format, (Bool *)NULL));
}
