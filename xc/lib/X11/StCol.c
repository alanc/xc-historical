/* $XConsortium: XcmsStCol.c,v 1.2 91/02/11 18:18:04 dave Exp $" */

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
 *		XcmsStCol.c
 *
 *	DESCRIPTION
 *		Source for XcmsStoreColor
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
 *		XcmsStoreColor - Store Color
 *
 *	SYNOPSIS
 */
Status
XcmsStoreColor(dpy, colormap, pColor_in)
    Display *dpy;
    Colormap colormap;
    XcmsColor *pColor_in;
/*
 *	DESCRIPTION
 *		Given a device-dependent or device-independent color
 *		specification, this routine will convert it to X RGB
 *		values then use it in a call to XStoreColor.
 *
 *	RETURNS
 *		XCMS_FAILURE if failed;
 *		XCMS_SUCCESS if it succeeded without gamut compression;
 *		XCMS_SUCCESS_WITH_COMPRESSION if it succeeded with gamut
 *			compression;
 *
 *		Since XStoreColor has no return value this routine
 *		does not return the color specification of the color actually
 *		stored.
 */
{
    XcmsColor tmpColor;

    bcopy((char *)pColor_in, (char *)&tmpColor, (unsigned) sizeof(XcmsColor));
    return(_XcmsSetGetColors (XStoreColor, dpy, colormap,
	    pColor_in, 1, XCMS_RGB_FORMAT, (Bool *) NULL));
}
