/* $XConsortium: XLookupCol.c,v 11.12 91/02/05 13:40:36 dave Exp $ */
/* Copyright    Massachusetts Institute of Technology    1985	*/

/*
Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation, and that the name of M.I.T. not be used in advertising or
publicity pertaining to distribution of the software without specific,
written prior permission.  M.I.T. makes no representations about the
suitability of this software for any purpose.  It is provided "as is"
without express or implied warranty.
*/

#define NEED_REPLIES
#include <stdio.h>
#include "Xlibint.h"
#include "TekCMS.h"

#if NeedFunctionPrototypes
Status XLookupColor (
	register Display *dpy,
        Colormap cmap,
	register _Xconst char *spec,
	XColor *def,
	XColor *scr)
#else
Status XLookupColor (dpy, cmap, spec, def, scr)
	register Display *dpy;
        Colormap cmap;
	register char *spec;
	XColor *def, *scr;
#endif
{
	register int n;
	xLookupColorReply reply;
	register xLookupColorReq *req;
	XcmsCCC *pCCC;
	char tmpName[BUFSIZ];
	XcmsColor cmsColor_exact;

	/*
	 * Let's Attempt to use TekCMS and i18n approach to Parse Color
	 */
	/* copy string to allow overwrite by _XcmsResolveColorString() */
	strncpy(tmpName, spec, BUFSIZ - 1);
	if ((pCCC = XcmsCCCofColormap(dpy, cmap)) != (XcmsCCC *)NULL) {
	    if (_XcmsResolveColorString(pCCC, tmpName,
		    &cmsColor_exact, XCMS_RGB_FORMAT) == XCMS_SUCCESS) {
		_XcmsRGB_to_XColor(&cmsColor_exact, def, 1);
		bcopy((char *)def, (char *)scr, sizeof(XColor));
		_XcmsResolveColor(pCCC, scr, 1);
		return(1);
	    }
	    /*
	     * Otherwise we failed; or tmpName was overwritten with yet another
	     * name.  Thus pass name to the X Server.
	     */
	}


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
	    return (0);
	    }
	def->red   = reply.exactRed;
	def->green = reply.exactGreen;
	def->blue  = reply.exactBlue;

	scr->red   = reply.screenRed;
	scr->green = reply.screenGreen;
	scr->blue  = reply.screenBlue;

	UnlockDisplay(dpy);
	SyncHandle();
	return (1);
}
