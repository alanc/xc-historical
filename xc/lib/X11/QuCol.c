/* $XConsortium: XcmsQuCol.c,v 1.1 91/01/30 18:58:17 dave Exp $" */

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
 *		XcmsQueryColor - Query Color
 *
 *	SYNOPSIS
 */
Status
XcmsQueryColor(dpy, colormap, pXcmsColor_in_out, result_format)
    Display *dpy;
    Colormap colormap;
    XcmsColor *pXcmsColor_in_out;
    XcmsSpecFmt result_format;
/*
 *	DESCRIPTION
 *		This routine uses XQueryColor to obtain the X RGB values
 *		stored in the specified colormap for the specified pixel.
 *		The X RGB values are then converted to the target format as
 *		specified by the format component of the XcmsColor structure.
 *
 *	RETURNS
 *		XCMS_FAILURE if failed;
 *		XCMS_SUCCESS if it succeeded.
 *
 *		Returns a color specification of the color stored in the
 *		specified pixel.
 */
{
    return(_XcmsSetGetColors (XQueryColor, dpy, colormap,
	    pXcmsColor_in_out, 1, result_format, (Bool *) NULL));
}
