/*-
 * mibstore.h --
 *	Header file for users of the MI backing-store scheme.
 *
 * Copyright (c) 1987 by the Regents of the University of California
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies.  The University of California
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 *
 *	"$XConsortium: mibstore.h,v 5.1 91/05/10 17:39:33 keith Exp $ SPRITE (Berkeley)"
 */

#ifndef _MIBSTORE_H
#define _MIBSTORE_H

typedef struct _miBSFuncRec {
    void	    (*SaveAreas)(
#if NeedNestedPrototypes
		    PixmapPtr /*pBackingPixmap*/,
		    RegionPtr /*pObscured*/,
		    int /*x*/,
		    int /*y*/,
		    WindowPtr /*pWin*/
#endif
);
    void	    (*RestoreAreas)(
#if NeedNestedPrototypes
		    PixmapPtr /*pBackingPixmap*/,
		    RegionPtr /*pExposed*/,
		    int /*x*/,
		    int /*y*/,
		    WindowPtr /*pWin*/
#endif
);
    void	    (*SetClipmaskRgn)(
#if NeedNestedPrototypes
		    GCPtr /*pBackingGC*/,
		    RegionPtr /*pbackingCompositeClip*/
#endif
);
    PixmapPtr	    (*GetImagePixmap)(); /* unused */
    PixmapPtr	    (*GetSpansPixmap)(); /* unused */
} miBSFuncRec;

#ifndef _XTYPEDEF_MIBSFUNCPTR
typedef struct _miBSFuncRec *miBSFuncPtr;
#define _XTYPEDEF_MIBSFUNCPTR
#endif

extern void miInitializeBackingStore(
#if NeedFunctionPrototypes
    ScreenPtr /*pScreen*/,
    miBSFuncPtr /*funcs*/
#endif
);

#endif /* _MIBSTORE_H */
