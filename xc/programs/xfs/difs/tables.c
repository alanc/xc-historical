/* $XConsortium$ */
/*
 * all the dispatch, error, event and reply vectors
 */
/*
 * Copyright 1990, 1991 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation and the
 * Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this protoype software
 * and its documentation to Members and Affiliates of the MIT X Consortium
 * any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Network Computing Devices, Digital or
 * MIT not be used in advertising or publicity pertaining to distribution of
 * the software without specific, written prior permission.
 *
 * NETWORK COMPUTING DEVICES, DIGITAL AND MIT DISCLAIM ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL NETWORK COMPUTING DEVICES, DIGITAL OR MIT BE
 * LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * @(#)tables.c	4.1	5/2/91
 *
 */
#include	"globals.h"

extern int  ProcInitialConnection(), ProcEstablishConnection();

extern int  ProcSetAuthorization(), ProcGetAuthorization(),
            ProcSetResolution(), ProcGetResolution(), ProcNoop(),
            ProcListExtensions(), ProcQueryExtension(),
            ProcListFonts(), ProcListFontsWithXInfo(),
            ProcOpenBitmapFont(), ProcQueryXInfo(), ProcQueryXExtents8(),
            ProcQueryXBitmaps8(), ProcCloseFont(),
            ProcListCatalogues(), ProcSetCatalogues(), ProcGetCatalogues(),
            ProcSetEventMask(), ProcGetEventMask(),
            ProcCreateAC(), ProcFreeAC();

extern int  SProcSimpleRequest(), SProcResourceRequest(),
            SProcSetResolution(), SProcGetResolution(),
            SProcListExtensions(), SProcQueryExtension(),
            SProcListFonts(), SProcListFontsWithXInfo(),
            SProcOpenBitmapFont(), SProcQueryXExtents8(),
            SProcQueryXBitmaps8(),
            SProcCreateAC();

extern void SErrorEvent();

extern void NotImplemented(), SGenericReply(),
            SListExtensionsReply(),
            SQueryExtensionReply(),
            SListFontsReply(), SListFontsWithXInfoReply(),
            SOpenBitmapFontReply(),
            SQueryXInfoReply(), SQueryXExtents8Reply(),
            SQueryXBitmaps8Reply(),
            SGetEventMaskReply(), SCreateACReply(), SGetResolutionReply(),
            SOpenBitmapFontReply();


int         (*InitialVector[3]) () =
{
    0,
    ProcInitialConnection,
    ProcEstablishConnection
};

int         (*ProcVector[NUM_PROC_VECTORS]) () =
{
    ProcNoop,			/* 0 */
    ProcListExtensions,
    ProcQueryExtension,
    ProcListCatalogues,
    ProcSetCatalogues,
    ProcGetCatalogues,		/* 5 */
    ProcSetEventMask,
    ProcGetEventMask,
    ProcCreateAC,
    ProcFreeAC,
    ProcSetAuthorization,	/* 10 */
    ProcSetResolution,
    ProcGetResolution,
    ProcListFonts,
    ProcListFontsWithXInfo,
    ProcOpenBitmapFont,		/* 15 */
    ProcQueryXInfo,
    ProcQueryXExtents8,
    ProcQueryXExtents8,
    ProcQueryXBitmaps8,
    ProcQueryXBitmaps8,		/* 20 */
    ProcCloseFont,
    0,
    0,
    0
};

int         (*SwappedProcVector[NUM_PROC_VECTORS]) () =
{
    SProcSimpleRequest,		/* 0 */
    SProcSimpleRequest,
    ProcQueryExtension,
    SProcSimpleRequest,
    SProcSimpleRequest,		/* SetCatalogues */
    SProcSimpleRequest,		/* 5 */
    SProcResourceRequest,	/* SetEventMask */
    SProcSimpleRequest,
    SProcCreateAC,
    SProcResourceRequest,
    SProcResourceRequest,	/* 10 */
    SProcSetResolution,
    SProcSimpleRequest,
    SProcListFonts,
    SProcListFontsWithXInfo,
    SProcOpenBitmapFont,	/* 15 */
    SProcResourceRequest,
    SProcQueryXExtents8,
    SProcQueryXExtents8,
    SProcQueryXBitmaps8,
    SProcQueryXBitmaps8,	/* 20 */
    SProcResourceRequest,
    0,
    0,
    0
};

void        (*EventSwapVector[NUM_EVENT_VECTORS]) () =
{
    SErrorEvent,
    NotImplemented,
    0,
    0,
    0,
    0,
    0,
    0
};

void        (*ReplySwapVector[NUM_PROC_VECTORS]) () =
{
    NotImplemented,		/* NoOp */
    SListExtensionsReply,
    SQueryExtensionReply,	/* SQueryExtensionReply */
    SGenericReply,		/* ListCatalogues */
    NotImplemented,		/* SetCatalogues */
    SGenericReply,		/* GetCatalogues -5 */
    NotImplemented,		/* SetEventMask */
    SGetEventMaskReply,
    SCreateACReply,
    NotImplemented,		/* FreeAC */
    NotImplemented,		/* SetAuthorization - 10 */
    NotImplemented,		/* SetResolution */
    SGetResolutionReply,
    SListFontsReply,
    SListFontsWithXInfoReply,
    SOpenBitmapFontReply,	/* 15 */
    SQueryXInfoReply,
    SQueryXExtents8Reply,
    SQueryXExtents8Reply,
    SQueryXBitmaps8Reply,
    SQueryXBitmaps8Reply,	/* 20 */
    NotImplemented,		/* Close */
    NotImplemented,
    NotImplemented
};
