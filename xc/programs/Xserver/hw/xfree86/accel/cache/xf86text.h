/* $XConsortium: xf86text.h,v 1.1 94/10/05 13:27:54 kaleb Exp $ */
/* $XFree86: xc/programs/Xserver/hw/xfree86/accel/cache/xf86text.h,v 3.0 1994/08/01 12:09:19 dawes Exp $ */
/*
 * Function prototypes for the text output functions.
 */
#ifndef _XF86_TEXT_H
#define _XF86_TEXT_H

/*
 * Entrypoints into text code.
 */

void xf86InitText(
#if NeedFunctionPrototypes
    void (*/*GlyphWriteFunc*/)(int, int, int, unsigned char *,
			       CacheFont8Ptr, GCPtr, BoxPtr, int),
    int (*/*NoCPolyTextFunc*/)(DrawablePtr, GCPtr, int, int, int, char*, Bool),
    int (*/*NoCImageTextFunc*/)(DrawablePtr, GCPtr, int, int, int,
				 char *, Bool)
#endif
);

int xf86PolyText8(
#if NeedFunctionPrototypes
    DrawablePtr /*pDraw*/,
    GCPtr /*pGC*/,
    int /*x*/,
    int /*y*/,
    int /*count*/,
    char * /*chars*/
#endif
);

int xf86PolyText16(
#if NeedFunctionPrototypes
    DrawablePtr /*pDraw*/,
    GCPtr /*pGC*/,
    int /*x*/,
    int /*y*/,
    int /*count*/,
    unsigned short * /*chars*/
#endif
);

void xf86ImageText8(
#if NeedFunctionPrototypes
    DrawablePtr /*pDraw*/,
    GCPtr /*pGC*/,
    int /*x*/,
    int /*y*/,
    int /*count*/,
    char * /*chars*/
#endif
);

void xf86ImageText16(
#if NeedFunctionPrototypes
    DrawablePtr /*pDraw*/,
    GCPtr /*pGC*/,
    int /*x*/,
    int /*y*/,
    int /*count*/,
    unsigned short * /*chars*/
#endif
);

#endif /* _XF86_TEXT_H */
