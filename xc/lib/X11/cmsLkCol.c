/* $XConsortium: XcmsLkCol.c,v 1.2 91/02/07 15:42:49 dave Exp $ */

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
 *		XcmsLkCol.c
 *
 *	DESCRIPTION
 *		Source for XcmsLookupColor
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
#include "TekCMSP.h"


/*
 *      INTERNAL INCLUDES
 *              Include files for local use only, therefore, NOT exported
 *		to any package or program using this package.
 */
#define NEED_REPLIES
#include <stdio.h>
#include "Xlibint.h"

/*
 *      EXTERNS
 */
extern void _XColor_to_XcmsRGB();
extern void _XcmsRGB_to_XColor();
extern char *XResolvePathname();
extern char *getenv();


/*
 *	NAME
 *		XcmsLookupColor - 
 *
 *	SYNOPSIS
 */
#if NeedFunctionPrototypes
Status
XcmsLookupColor (
    Display *dpy,
    Colormap cmap,
    _Xconst char *color_name,
    XcmsColor *pColor_exact_return,
    XcmsColor *pColor_scrn_return,
    XcmsSpecFmt result_format)
#else
Status
XcmsLookupColor(dpy, cmap, color_name, pColor_exact_return, pColor_scrn_return,
	result_format)
    Display *dpy;
    Colormap cmap;
    char *color_name;
    XcmsColor *pColor_exact_return;
    XcmsColor *pColor_scrn_return;
    XcmsSpecFmt result_format;
#endif
/*
 *	DESCRIPTION
 *		The XcmsLookupColor function finds the color specification
 *		associated with a color name in the Device-Independent Color
 *		Name Database.
 *	RETURNS
 *		This function returns both the color specification found in the
 *		database (db specification) and the color specification for the
 *		color displayable by the specified screen (screen
 *		specification).  The calling routine sets the format for these
 *		returned specifications in the XcmsColor format component.
 *		If XCMS_UNDEFINED_FORMAT, the specification is returned in the
 *		format used to store the color in the database.
 */
{
    Status retval1 = XCMS_SUCCESS;
    Status retval2 = XCMS_SUCCESS;
    char tmpName[BUFSIZ];
    XcmsCCC *pCCC;
    register int n;
    xLookupColorReply reply;
    register xLookupColorReq *req;
    XColor def, scr;

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
 * 1. Convert string to a XcmsColor
 */
    strncpy(tmpName, color_name, BUFSIZ - 1);
    if ((retval1 = _XcmsResolveColorString(pCCC, color_name,
	    pColor_exact_return, result_format)) == XCMS_FAILURE) {
	return(XCMS_FAILURE);
    }
    if (retval1 == _XCMS_NEWNAME) {
	goto PassToServer;
    }

/*
 * 2. pColor_scrn_return
 *	    Assume the pColor_exact_return has already been adjusted to
 *	    the Client White Point.
 *
 */
    /*
     * Convert to RGB, adjusting for white point differences if necessary.
     */
    bcopy((char *)pColor_exact_return, (char *)pColor_scrn_return,
	    sizeof(XcmsColor));
    if (pColor_scrn_return->format == XCMS_RGB_FORMAT) {
	_XcmsUnresolveColor(pCCC, pColor_scrn_return, 1);
	retval2 = XCMS_SUCCESS;
    } else if ((retval2 = XcmsConvertColors(pCCC, pColor_scrn_return, 1,
	    XCMS_RGB_FORMAT, (Bool *)NULL)) == XCMS_FAILURE) {
	return(XCMS_FAILURE);
    }

    /*
     * Then, convert XcmsColor structure to the target specification
     *    format.  Note that we must use NULL instead of passing
     *    pCompressed.
     */

    switch (result_format) {
      case XCMS_RGB_FORMAT :
	break;
      case XCMS_UNDEFINED_FORMAT :
	result_format = pColor_exact_return->format;
	/* fall through */
      default :
	if (XcmsConvertColors(pCCC, pColor_scrn_return, 1, result_format,
		(Bool *) NULL) == XCMS_FAILURE) {
	    return(XCMS_FAILURE);
	}
	break;
    }

    return(retval1 > retval2 ? retval1 : retval2);

PassToServer:
    /*
     * TekCMS and i18n methods failed, so lets pass it to the server
     * for parsing.  Remember to use tmpName since it may have been
     * overwritten by XcmsResolveColorString().
     */

    n = tmpName ? strlen (tmpName) : 0;
    LockDisplay(dpy);
    GetReq (LookupColor, req);
    req->cmap = cmap;
    req->nbytes = n;
    req->length += (n + 3) >> 2;
    Data (dpy, tmpName, (long)n);
    if (!_XReply (dpy, (xReply *) &reply, 0, xTrue)) {
	UnlockDisplay(dpy);
	SyncHandle();
	return (XCMS_FAILURE);
	}
    def.red   = reply.exactRed;
    def.green = reply.exactGreen;
    def.blue  = reply.exactBlue;

    scr.red   = reply.screenRed;
    scr.green = reply.screenGreen;
    scr.blue  = reply.screenBlue;

    UnlockDisplay(dpy);
    SyncHandle();

    _XColor_to_XcmsRGB(pCCC, &def, pColor_exact_return, 1);
    _XColor_to_XcmsRGB(pCCC, &scr, pColor_scrn_return, 1);

    /*
     * Then, convert XcmsColor structure to the target specification
     *    format.  Note that we must use NULL instead of passing
     *    pCompressed.
     */

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
