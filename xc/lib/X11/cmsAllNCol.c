/* $XConsortium: XcmsAlNCol.c,v 1.3 91/02/11 18:17:21 dave Exp $" */

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
 *		XcmsAlNCol.c
 *
 *	DESCRIPTION
 *		Source for XcmsAllocNamedColor
 *
 *
 */

#define NEED_REPLIES
#include <stdio.h>
#include "Xlibint.h"
#include "Xcmsint.h"

/*
 *      EXTERNS
 */
extern void _XColor_to_XcmsRGB();
extern void _XcmsRGB_to_XColor();


/*
 *	NAME
 *		XcmsAllocNamedColor - 
 *
 *	SYNOPSIS
 */
#if NeedFunctionPrototypes
Status
XcmsAllocNamedColor (
    Display *dpy,
    Colormap cmap,
    _Xconst char *color_name,
    XcmsColor *pColor_scrn_return,
    XcmsColor *pColor_exact_return,
    XcmsSpecFmt result_format)
#else
Status
XcmsAllocNamedColor(dpy, cmap, color_name, pColor_scrn_return,
	pColor_exact_return, result_format)
    Display *dpy;
    Colormap cmap;
    char *color_name;
    XcmsColor *pColor_scrn_return;
    XcmsColor *pColor_exact_return;
    XcmsSpecFmt result_format;
#endif
/*
 *	DESCRIPTION
 *		Finds the color specification associated with the color
 *		name in the Device-Independent Color Name Database, then
 *		converts that color specification to an RGB format.  This
 *		RGB value is then used in a call to XAllocColor to allocate
 *		a read-only color cell.
 *
 *	RETURNS
 *		0 if failed to parse string or find any entry in the database.
 *		1 if succeeded in converting color name to XcmsColor.
 *		2 if succeeded in converting color name to another color name.
 *
 *	CAVEATS
 *		Assumes name is an array of BUFSIZ characters so we can
 *		overwrite!
 *
 */
{
    long nbytes;
    xAllocNamedColorReply rep;
    xAllocNamedColorReq *req;
    XColor hard_def;
    XColor exact_def;
    Status retval1 = 1;
    Status retval2 = XCMS_SUCCESS;
    XcmsColor tmpColor;
    XColor XColor_in_out;
    char tmpName[BUFSIZ];
    XcmsCCC *pCCC;

    /*
     * 0. Check for invalid arguments.
     */
    if (dpy == NULL || color_name[0] == '\0' || pColor_scrn_return == 0
	    || pColor_exact_return == NULL) {
	return(XCMS_FAILURE);
    }

    if ((pCCC = XcmsCCCofColormap(dpy, cmap)) == (XcmsCCC *)NULL) {
	return(XCMS_FAILURE);
    }

    /*
     * 1. Convert string to a XcmsColor using TekCMS and i18n mechanism
     */
    /* copy string to allow overwrite by _XcmsResolveColorString() */
    strncpy(tmpName, color_name, BUFSIZ - 1);
    if ((retval1 = _XcmsResolveColorString(pCCC, tmpName,
	    &tmpColor, result_format)) == XCMS_FAILURE) {
	return(XCMS_FAILURE);
    }
    if (retval1 == _XCMS_NEWNAME) {
	goto PassToServer;
    }
    bcopy((char *)&tmpColor, (char *)pColor_exact_return, sizeof(XcmsColor));

    /*
     * 2. Convert tmpColor to RGB
     *	Assume pColor_exact_return is now adjusted to Client White Point
     */
    if ((retval2 = XcmsConvertColors(pCCC, &tmpColor,
	    1, XCMS_RGB_FORMAT, (Bool *) NULL)) == XCMS_FAILURE) {
	return(XCMS_FAILURE);
    }

    /*
     * 3. Convert to XColor and call XAllocColor
     */
    _XcmsRGB_to_XColor(&tmpColor, &XColor_in_out, 1);
    if (XAllocColor(pCCC->dpy, cmap, &XColor_in_out) == 0) {
	return(XCMS_FAILURE);
    }

    /*
     * 4. pColor_scrn_return
     *
     * Now convert to the target format.
     *    We can ignore the return value because we're already in a
     *    device-dependent format.
     */
    _XColor_to_XcmsRGB(pCCC, &XColor_in_out, pColor_scrn_return, 1);
    if (result_format != XCMS_RGB_FORMAT) {
	if (result_format == XCMS_UNDEFINED_FORMAT) {
	    result_format = pColor_exact_return->format;
	}
	if (XcmsConvertColors(pCCC, pColor_scrn_return, 1, result_format,
		(Bool *) NULL) == XCMS_FAILURE) {
	    return(XCMS_FAILURE);
	}
    }

    return(retval1 > retval2 ? retval1 : retval2);

PassToServer:
    /*
     * All previous methods failed, so lets pass it to the server
     * for parsing.
     */
    dpy = pCCC->dpy;
    LockDisplay(dpy);
    GetReq(AllocNamedColor, req);

    req->cmap = cmap;
    nbytes = req->nbytes = color_name ? strlen(color_name) : 0;
    req->length += (nbytes + 3) >> 2; /* round up to mult of 4 */

    _XSend(dpy, color_name, nbytes);
       /* _XSend is more efficient that Data, since _XReply follows */

    if (!_XReply (dpy, (xReply *) &rep, 0, xTrue)) {
	UnlockDisplay(dpy);
        SyncHandle();
        return (0);
    }

    exact_def.red = rep.exactRed;
    exact_def.green = rep.exactGreen;
    exact_def.blue = rep.exactBlue;

    hard_def.red = rep.screenRed;
    hard_def.green = rep.screenGreen;
    hard_def.blue = rep.screenBlue;

    exact_def.pixel = hard_def.pixel = rep.pixel;

    UnlockDisplay(dpy);
    SyncHandle();

    /*
     * Now convert to the target format.
     */
    _XColor_to_XcmsRGB(pCCC, &exact_def, pColor_exact_return, 1);
    _XColor_to_XcmsRGB(pCCC, &hard_def, pColor_scrn_return, 1);
    if (result_format != XCMS_RGB_FORMAT
	    && result_format != XCMS_UNDEFINED_FORMAT) {
	if (XcmsConvertColors(pCCC, pColor_exact_return, 1, result_format,
		(Bool *) NULL) == XCMS_FAILURE) {
	    return(XCMS_FAILURE);
	}
	if (XcmsConvertColors(pCCC, pColor_scrn_return, 1, result_format,
		(Bool *) NULL) == XCMS_FAILURE) {
	    return(XCMS_FAILURE);
	}
    }

    return(XCMS_SUCCESS);
}
