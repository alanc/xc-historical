/* $XConsortium: XGetColor.c,v 11.20 91/02/12 16:11:15 dave Exp $ */
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

#define NEED_REPLIES
#include <stdio.h>
#include "Xlibint.h"
#include "Xcmsint.h"

extern void _XcmsRGB_to_XColor();

#if NeedFunctionPrototypes
Status XAllocNamedColor(
register Display *dpy,
Colormap cmap,
_Xconst char *colorname, /* STRING8 */
XColor *hard_def, /* RETURN */
XColor *exact_def) /* RETURN */
#else
Status XAllocNamedColor(dpy, cmap, colorname, hard_def, exact_def)
register Display *dpy;
Colormap cmap;
char *colorname; /* STRING8 */
XColor *hard_def; /* RETURN */
XColor *exact_def; /* RETURN */
#endif
{

    long nbytes;
    xAllocNamedColorReply rep;
    xAllocNamedColorReq *req;

    char tmpName[BUFSIZ];
    XcmsCCC ccc;
    XcmsColor cmsColor_exact;
    Status ret;

    /*
     * Let's Attempt to use TekCMS and i18n approach to Parse Color
     */
    /* copy string to allow overwrite by _XcmsResolveColorString() */
    strncpy(tmpName, colorname, BUFSIZ - 1);
    if ((ccc = XcmsCCCOfColormap(dpy, cmap)) != (XcmsCCC)NULL) {
	if (_XcmsResolveColorString(ccc, tmpName, &cmsColor_exact,
		XcmsRGBFormat) == XcmsSuccess) {
	    _XcmsRGB_to_XColor(&cmsColor_exact, exact_def, 1);
	    bcopy((char *)exact_def, (char *)hard_def, sizeof(XColor));
	    ret = XAllocColor(dpy, cmap, hard_def);
	    exact_def->pixel = hard_def->pixel;
	    return(ret);
	}
	/*
	 * Otherwise we failed; or tmpName was overwritten with yet another
	 * name.  Thus pass name to the X Server.
	 */
    }

    /*
     * TekCMS and i18n approach failed.
     */
    LockDisplay(dpy);
    GetReq(AllocNamedColor, req);

    req->cmap = cmap;
    nbytes = req->nbytes = tmpName[0] == '\0' ? strlen(tmpName) : 0;
    req->length += (nbytes + 3) >> 2; /* round up to mult of 4 */

    _XSend(dpy, tmpName, nbytes);
       /* _XSend is more efficient that Data, since _XReply follows */

    if (!_XReply (dpy, (xReply *) &rep, 0, xTrue)) {
	UnlockDisplay(dpy);
        SyncHandle();
        return (0);
    }

    exact_def->red = rep.exactRed;
    exact_def->green = rep.exactGreen;
    exact_def->blue = rep.exactBlue;

    hard_def->red = rep.screenRed;
    hard_def->green = rep.screenGreen;
    hard_def->blue = rep.screenBlue;

    exact_def->pixel = hard_def->pixel = rep.pixel;

    UnlockDisplay(dpy);
    SyncHandle();
    return (1);
}
