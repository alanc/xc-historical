/* $XConsortium: dixfont.h,v 1.7 91/05/11 11:45:47 rws Exp $ */
/***********************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

#ifndef DIXFONT_H
#define DIXFONT_H 1

#include    <font.h>

#define NullDIXFontProp ((DIXFontPropPtr)0)

typedef struct _DIXFontProp *DIXFontPropPtr;

extern void UseFPE(
#if NeedFunctionPrototypes
    FontPathElementPtr /*fpe*/
#endif
);

extern void FreeFPE(
#if NeedFunctionPrototypes
    FontPathElementPtr /*fpe*/,
    Bool /*force*/
#endif
);

extern int FontToXError(
#if NeedFunctionPrototypes
    int /*err*/
#endif
);

extern Bool SetDefaultFont(
#if NeedFunctionPrototypes
    char */*defaultfontname*/
#endif
);

extern void QueueFontWakeup(
#if NeedFunctionPrototypes
    FontPathElementPtr /*fpe*/
#endif
);

extern void RemoveFontWakeup(
#if NeedFunctionPrototypes
    FontPathElementPtr /*fpe*/
#endif
);

extern int FontWakeup(
#if NeedFunctionPrototypes
    pointer /*data*/,
    int /*count*/,
    long */*LastSelectMask*/
#endif
);

extern int OpenFont(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    XID /*fid*/,
    Mask /*flags*/,
    unsigned /*lenfname*/,
    char */*pfontname*/
#endif
);

extern int CloseFont(
#if NeedFunctionPrototypes
    pointer /*pfont*/,
    XID /*fid*/
#endif
);

typedef struct _xQueryFontReply *xQueryFontReplyPtr;

extern void QueryFont(
#if NeedFunctionPrototypes
    FontPtr /*pFont*/,
    xQueryFontReplyPtr /*pReply*/,
    int /*nProtoCCIStructs*/
#endif
);

extern int ListFonts(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    unsigned char */*pattern*/,
    unsigned int /*length*/,
    unsigned int /*max_names*/
#endif
);

extern int SetFontPath(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    int /*npaths*/,
    unsigned char */*paths*/,
    int */*error*/
#endif
);

extern int SetDefaultFontPath(
#if NeedFunctionPrototypes
    char */*path*/
#endif
);

extern unsigned char *GetFontPath(
#if NeedFunctionPrototypes
    int */*count*/,
    int */*length*/
#endif
);

extern int LoadGlyphs(
#if NeedFunctionPrototypes
    ClientPtr /*client*/,
    FontPtr /*pfont*/,
    unsigned /*nchars*/,
    int /*item_size*/,
    unsigned char */*data*/
#endif
);

extern int DeleteClientFontStuff(
#if NeedFunctionPrototypes
    ClientPtr /*client*/
#endif
);

extern int InitFonts(
#if NeedFunctionPrototypes
    void
#endif
);

extern int GetDefaultPointSize(
#if NeedFunctionPrototypes
    void
#endif
);

extern struct resolution *GetClientResolutions(
#if NeedFunctionPrototypes
    int */*num*/
#endif
);

/* XXX leave these unprototyped for now -- need to do groundwork in 
 * fonts/include/fontstruct.h before we do these
 */
extern int RegisterFPEFunctions(
#if NeedFunctionPrototypes
    Bool (*/*name_func*/)(),
    int (*/*init_func*/)(),
    int (*/*free_func*/)(),
    int (*/*reset_func*/)(),
    int (*/*open_func*/)(),
    int (*/*close_func*/)(),
    int (*/*list_func*/)(),
    int (*/*start_lfwi_func*/)(),
    int (*/*next_lfwi_func*/)(),
    int (*/*wakeup_func*/)(),
    int (*/*client_died*/)()
#endif
);

extern int FreeFonts(
#if NeedFunctionPrototypes
    void
#endif
);

extern FontPtr find_old_font(
#if NeedFunctionPrototypes
    XID /*id*/
#endif
);

extern Font GetNewFontClientID(
#if NeedFunctionPrototypes
    void
#endif
);

extern int StoreFontClientFont(
#if NeedFunctionPrototypes
    FontPtr /*pfont*/,
    Font /*id*/
#endif
);

extern int DeleteFontClientID(
#if NeedFunctionPrototypes
    Font /*id*/
#endif
);

extern int init_fs_handlers(
#if NeedFunctionPrototypes
    FontPathElementPtr /*fpe*/,
    int (*/*block_handler*/)(/*XXX*/)
#endif
);

extern int remove_fs_handlers(
#if NeedFunctionPrototypes
    FontPathElementPtr /*fpe*/,
    int (*/*block_handler*/)(/*XXX*/),
    Bool /*all*/
#endif
);

#endif				/* DIXFONT_H */
