/*
 * Copyright 1993 Massachusetts Institute of Technology
 * 
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 * 
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * M.I.T. BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY
 * DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 */

/* $XConsortium: */

extern void miRegisterGCPrivateIndex(
#if NeedFunctionPrototypes
    int /*gcindex*/
#endif
);

extern void miChangeGC(
#if NeedFunctionPrototypes
    GCPtr  /*pGC*/,
    unsigned long /*mask*/
#endif
);

extern void miDestroyGC(
#if NeedFunctionPrototypes
    GCPtr  /*pGC*/
#endif
);

extern GCOpsPtr miCreateGCOps(
#if NeedFunctionPrototypes
    GCOpsPtr /*prototype*/
#endif
);

extern void miDestroyGCOps(
#if NeedFunctionPrototypes
    GCOpsPtr /*ops*/
#endif
);

extern void miDestroyClip(
#if NeedFunctionPrototypes
    GCPtr /*pGC*/
#endif
);

extern void miChangeClip(
#if NeedFunctionPrototypes
    GCPtr   /*pGC*/,
    int     /*type*/,
    pointer /*pvalue*/,
    int     /*nrects*/
#endif
);

extern void miCopyClip(
#if NeedFunctionPrototypes
    GCPtr /*pgcDst*/,
    GCPtr /*pgcSrc*/
#endif
);

extern void miCopyGC(
#if NeedFunctionPrototypes
    GCPtr /*pGCSrc*/,
    unsigned long /*changes*/,
    GCPtr /*pGCDst*/
#endif
);

extern void miComputeCompositeClip(
#if NeedFunctionPrototypes
    GCPtr       /*pGC*/,
    DrawablePtr /*pDrawable*/
#endif
);
