/* $XConsortium: XStNColor.c,v 11.16 91/02/07 17:35:37 dave Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

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

#include <stdio.h>
#include "Xlibint.h"
#include "Xcmsint.h"

#if NeedFunctionPrototypes
XStoreNamedColor(
register Display *dpy,
Colormap cmap,
_Xconst char *name, /* STRING8 */
unsigned long pixel, /* CARD32 */
int flags)  /* DoRed, DoGreen, DoBlue */
#else
XStoreNamedColor(dpy, cmap, name, pixel, flags)
register Display *dpy;
Colormap cmap;
char *name; /* STRING8 */
unsigned long pixel; /* CARD32 */
int flags;  /* DoRed, DoGreen, DoBlue */
#endif
{
    unsigned int nbytes;
    register xStoreNamedColorReq *req;
    XcmsCCC *pCCC;
    XcmsColor cmsColor_scr;
    XcmsColor cmsColor_exact;
    XColor scr_def;
    char tmpName[BUFSIZ];
    XcmsSpecFmt result_format;

    /*
     * Let's Attempt to use TekCMS approach to Parse Color
     */
    /* copy string to allow overwrite by _XcmsResolveColorString() */
    strncpy(tmpName, name, BUFSIZ - 1);
    if ((pCCC = XcmsCCCofColormap(dpy, cmap)) != (XcmsCCC *)NULL) {
	if (_XcmsResolveColorString(pCCC, tmpName, &cmsColor_exact,
		XCMS_RGB_FORMAT) == XCMS_SUCCESS) {
	    _XcmsRGB_to_XColor(&cmsColor_exact, &scr_def, 1);
	    scr_def.pixel = pixel;
	    scr_def.flags = flags;
	    XStoreColor(dpy, cmap, &scr_def);
	    return;
	}
	/*
	 * Otherwise we failed; or tmpName was overwritten with yet another
	 * name.  Thus pass name to the X Server.
	 */
    }

    /*
     * The TekCMS and i18n methods failed, so lets pass it to the server
     * for parsing.  Remember to use tmpName since it may have been
     * overwritten by _XcmsResolveColorString().
     */

    LockDisplay(dpy);
    GetReq(StoreNamedColor, req);

    req->cmap = cmap;
    req->flags = flags;
    req->pixel = pixel;
    req->nbytes = nbytes = tmpName ? strlen(tmpName) : 0;
    req->length += (nbytes + 3) >> 2; /* round up to multiple of 4 */
    Data(dpy, tmpName, (long)nbytes);
    UnlockDisplay(dpy);
    SyncHandle();
}


