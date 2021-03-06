/* $XConsortium: tables.c,v 1.8 94/12/01 20:47:07 mor Exp $ */
/*
 * $NCDOr$
 * $NCDId: @(#)tables.c,v 1.18 1994/11/18 20:38:46 lemke Exp $
 *
 * Copyright 1992 Network Computing Devices
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of NCD. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  NCD. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * NCD. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL NCD.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Keith Packard, Network Computing Devices
 */

extern int  ProcInitialConnection(), ProcEstablishConnection();

extern int  ProcStandardRequest();

extern int  ProcLBXInternAtom(),
            ProcLBXGetAtomName(),
            ProcLBXCreateColormap(),
            ProcLBXFreeColormap(),
            ProcLBXCopyColormapAndFree(),
            ProcLBXFreeColors(),
            ProcLBXLookupColor(),
            ProcLBXAllocColor(),
            ProcLBXAllocNamedColor(),
            ProcLBXGetModifierMapping(),
            ProcLBXGetKeyboardMapping(),
            ProcLBXQueryFont(),
            ProcLBXChangeProperty(),
            ProcLBXGetProperty(),
	    ProcLBXCopyArea(),
	    ProcLBXCopyPlane(),
            ProcLBXPolyPoint(),
            ProcLBXPolyLine(),
            ProcLBXPolySegment(),
            ProcLBXPolyRectangle(),
            ProcLBXPolyArc(),
            ProcLBXFillPoly(),
            ProcLBXPolyFillRectangle(),
            ProcLBXPolyFillArc(),
	    ProcLBXPolyText(),
	    ProcLBXImageText(),
            ProcLBXQueryExtension(),
	    ProcLBXGetImage(),
	    ProcLBXPutImage();

extern int  ProcBadRequest();

int         (*InitialVector[3]) () =
{
    0,
    ProcInitialConnection,
    ProcEstablishConnection
};

int         (*ProcVector[256]) () =
{
    ProcBadRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,	/* 5 */
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,	/* 10 */
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,	/* 15 */
    ProcLBXInternAtom,
    ProcLBXGetAtomName,
    ProcLBXChangeProperty,
    ProcStandardRequest,
    ProcLBXGetProperty,		/* 20 */
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,	/* 25 */
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,	/* 30 */
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,	/* 35 */
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,	/* 40 */
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,	/* 45 */
    ProcStandardRequest,
    ProcLBXQueryFont,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,	/* 50 */
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,	/* 55 */
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,	/* 60 */
    ProcStandardRequest,
    ProcLBXCopyArea,
    ProcLBXCopyPlane,
    ProcLBXPolyPoint,
    ProcLBXPolyLine,		/* 65 */
    ProcLBXPolySegment,
    ProcLBXPolyRectangle,
    ProcLBXPolyArc,
    ProcLBXFillPoly,
    ProcLBXPolyFillRectangle,	/* 70 */
    ProcLBXPolyFillArc,
    ProcLBXPutImage,
    ProcLBXGetImage,
    ProcLBXPolyText,
    ProcLBXPolyText,		/* 75 */
    ProcLBXImageText,
    ProcLBXImageText,
    ProcLBXCreateColormap,
    ProcLBXFreeColormap,
    ProcLBXCopyColormapAndFree,	/* 80 */
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcLBXAllocColor,
    ProcLBXAllocNamedColor,	/* 85 */
    ProcStandardRequest,
    ProcStandardRequest,
    ProcLBXFreeColors,
    ProcStandardRequest,
    ProcStandardRequest,	/* 90 */
    ProcStandardRequest,
    ProcLBXLookupColor,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,	/* 95 */
    ProcStandardRequest,
    ProcStandardRequest,
    ProcLBXQueryExtension,
    ProcStandardRequest,
    ProcStandardRequest,	/* 100 */
    ProcLBXGetKeyboardMapping,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,	/* 105 */
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,	/* 110 */
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,	/* 115 */
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcLBXGetModifierMapping,
    0,				/* 120 */
    0,
    0,
    0,
    0,
    0,				/* 125 */
    0,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
    ProcStandardRequest,
};


extern int  ServerProcError(), ServerProcReply(),
            ServerProcStandardEvent();

int         (*ServerVector[256]) () = {
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
    ServerProcStandardEvent,
};
