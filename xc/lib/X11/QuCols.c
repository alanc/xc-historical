/* $XConsortium: XcmsQuCols.c,v 1.1 91/01/30 18:59:14 dave Exp $" */

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
 *		XcmsQuCol.c
 *
 *	DESCRIPTION
 *		Source for XcmsQueryColors
 *
 *
 */

#include "Xcmsint.h"


/************************************************************************
 *									*
 *			PUBLIC ROUTINES					*
 *									*
 ************************************************************************/
/*
 *	NAME
 *		XcmsQueryColors - Query Colors
 *
 *	SYNOPSIS
 */
Status
XcmsQueryColors(dpy, colormap, pXcmsColors_in_out, nColors, result_format)
    Display *dpy;
    Colormap colormap;
    XcmsColor *pXcmsColors_in_out;
    unsigned int nColors;
    XcmsSpecFmt result_format;
/*
 *	DESCRIPTION
 *		This routine uses XQueryColors to obtain the X RGB values
 *		stored in the specified colormap for the specified pixels.
 *		The X RGB values are then converted to the target format as
 *		specified by the format component of the XcmsColor structure.
 *
 *	RETURNS
 *		XCMS_FAILURE if failed;
 *		XCMS_SUCCESS if it succeeded.
 *
 *		Returns the color specifications of the colors stored in the
 *		specified pixels.
 */
{
    return(_XcmsSetGetColors (XQueryColors, dpy, colormap,
	    pXcmsColors_in_out, nColors, result_format, (Bool *) NULL));
}
