/*
 * Copyright 1988-1993 Network Computing Devices
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

/* $XConsortium:$ */

/* decides what tupe of request it is */
#include	<X11/Xproto.h>
#include        "os.h"          /* in server/include */
#include	"reqtype.h"

static Bool cause_replies[] = {
    FALSE,			/* BadRequest */
    FALSE,			/* CreateWindow */
    FALSE,			/* ChangeWindowAttributes */
    TRUE,			/* GetWindowAttributes */
    FALSE,			/* DestroyWindow */
    FALSE,			/* DestroySubwindows */
    FALSE,			/* ChangeSaveSet */
    FALSE,			/* ReparentWindow */
    FALSE,			/* MapWindow */
    FALSE,			/* MapSubwindows */
    FALSE,			/* UnmapWindow */
    FALSE,			/* UnmapSubwindows */
    FALSE,			/* ConfigureWindow */
    FALSE,			/* CirculateWindow */
    TRUE,			/* GetGeometry */
    TRUE,			/* QueryTree */
    TRUE,			/* InternAtom */
    TRUE,			/* GetAtomName */
    FALSE,			/* ChangeProperty */
    FALSE,			/* DeleteProperty */
    TRUE,			/* GetProperty */
    TRUE,			/* ListProperties */
    FALSE,			/* SetSelectionOwner */
    TRUE,			/* GetSelectionOwner */
    FALSE,			/* ConvertSelection */
    FALSE,			/* SendEvent */
    TRUE,			/* GrabPointer */
    FALSE,			/* UngrabPointer */
    FALSE,			/* GrabButton */
    FALSE,			/* UngrabButton */
    FALSE,			/* ChangeActivePointerGrab */
    TRUE,			/* GrabKeyboard */
    FALSE,			/* UngrabKeyboard */
    FALSE,			/* GrabKey */
    FALSE,			/* UngrabKey */
    FALSE,			/* AllowEvents */
    FALSE,			/* GrabServer */
    FALSE,			/* UngrabServer */
    TRUE,			/* QueryPointer */
    TRUE,			/* GetMotionEvents */
    TRUE,			/* TranslateCoords */
    FALSE,			/* WarpPointer */
    FALSE,			/* SetInputFocus */
    TRUE,			/* GetInputFocus */
    TRUE,			/* QueryKeymap */
    FALSE,			/* OpenFont */
    FALSE,			/* CloseFont */
    TRUE,			/* QueryFont */
    TRUE,			/* QueryTextExtents */
    TRUE,			/* ListFonts */
    TRUE,			/* ListFontsWithInfo */
    FALSE,			/* SetFontPath */
    TRUE,			/* GetFontPath */
    FALSE,			/* CreatePixmap */
    FALSE,			/* FreePixmap */
    FALSE,			/* CreateGC */
    FALSE,			/* ChangeGC */
    FALSE,			/* CopyGC */
    FALSE,			/* SetDashes */
    FALSE,			/* SetClipRectangles */
    FALSE,			/* FreeGC */
    FALSE,			/* ClearToBackground */
    FALSE,			/* CopyArea */
    FALSE,			/* CopyPlane */
    FALSE,			/* PolyPoint */
    FALSE,			/* PolyLine */
    FALSE,			/* PolySegment */
    FALSE,			/* PolyRectangle */
    FALSE,			/* PolyArc */
    FALSE,			/* FillPoly */
    FALSE,			/* PolyFillRectangle */
    FALSE,			/* PolyFillArc */
    FALSE,			/* PutImage */
    TRUE,			/* GetImage */
    FALSE,			/* PolyText */
    FALSE,			/* PolyText */
    FALSE,			/* ImageText8 */
    FALSE,			/* ImageText16 */
    FALSE,			/* CreateColormap */
    FALSE,			/* FreeColormap */
    FALSE,			/* CopyColormapAndFree */
    FALSE,			/* InstallColormap */
    FALSE,			/* UninstallColormap */
    TRUE,			/* ListInstalledColormaps */
    TRUE,			/* AllocColor */
    TRUE,			/* AllocNamedColor */
    TRUE,			/* AllocColorCells */
    TRUE,			/* AllocColorPlanes */
    FALSE,			/* FreeColors */
    FALSE,			/* StoreColors */
    FALSE,			/* StoreNamedColor */
    TRUE,			/* QueryColors */
    TRUE,			/* LookupColor */
    FALSE,			/* CreateCursor */
    FALSE,			/* CreateGlyphCursor */
    FALSE,			/* FreeCursor */
    FALSE,			/* RecolorCursor */
    TRUE,			/* QueryBestSize */
    TRUE,			/* QueryExtension */
    TRUE,			/* ListExtensions */
    FALSE,			/* ChangeKeyboardMapping */
    TRUE,			/* GetKeyboardMapping */
    FALSE,			/* ChangeKeyboardControl */
    TRUE,			/* GetKeyboardControl */
    FALSE,			/* Bell */
    FALSE,			/* ChangePointerControl */
    TRUE,			/* GetPointerControl */
    FALSE,			/* SetScreenSaver */
    TRUE,			/* GetScreenSaver */
    FALSE,			/* ChangeHosts */
    TRUE,			/* ListHosts */
    FALSE,			/* ChangeAccessControl */
    FALSE,			/* ChangeCloseDownMode */
    FALSE,			/* KillClient */
    FALSE,			/* RotateProperties */
    FALSE,			/* ForceScreenSaver */
    TRUE,			/* SetPointerMapping */
    TRUE,			/* GetPointerMapping */
    TRUE,			/* SetModifierMapping */
    TRUE,			/* GetModifierMapping */
    FALSE,			/* NoOp */
};

static Bool cause_events[] = {
    FALSE,			/* BadRequest */
    TRUE,			/* CreateWindow */
    TRUE,			/* ChangeWindowAttributes */
    FALSE,			/* GetWindowAttributes */
    TRUE,			/* DestroyWindow */
    TRUE,			/* DestroySubwindows */
    FALSE,			/* ChangeSaveSet */
    TRUE,			/* ReparentWindow */
    TRUE,			/* MapWindow */
    TRUE,			/* MapSubwindows */
    TRUE,			/* UnmapWindow */
    TRUE,			/* UnmapSubwindows */
    TRUE,			/* ConfigureWindow */
    TRUE,			/* CirculateWindow */
    FALSE,			/* GetGeometry */
    FALSE,			/* QueryTree */
    FALSE,			/* InternAtom */
    FALSE,			/* GetAtomName */
    TRUE,			/* ChangeProperty */
    TRUE,			/* DeleteProperty */
    TRUE,			/* GetProperty */
    FALSE,			/* ListProperties */
    TRUE,			/* SetSelectionOwner */
    FALSE,			/* GetSelectionOwner */
    TRUE,			/* ConvertSelection */
    TRUE,			/* SendEvent */
    TRUE,			/* GrabPointer */
    TRUE,			/* UngrabPointer */
    FALSE,			/* GrabButton */
    FALSE,			/* UngrabButton */
    FALSE,			/* ChangeActivePointerGrab */
    TRUE,			/* GrabKeyboard */
    TRUE,			/* UngrabKeyboard */
    FALSE,			/* GrabKey */
    FALSE,			/* UngrabKey */
    TRUE,			/* AllowEvents */
    FALSE,			/* GrabServer */
    FALSE,			/* UngrabServer */
    FALSE,			/* QueryPointer */
    FALSE,			/* GetMotionEvents */
    FALSE,			/* TranslateCoords */
    TRUE,			/* WarpPointer */
    TRUE,			/* SetInputFocus */
    FALSE,			/* GetInputFocus */
    FALSE,			/* QueryKeymap */
    FALSE,			/* OpenFont */
    FALSE,			/* CloseFont */
    FALSE,			/* QueryFont */
    FALSE,			/* QueryTextExtents */
    FALSE,			/* ListFonts */
    FALSE,			/* ListFontsWithInfo */
    FALSE,			/* SetFontPath */
    FALSE,			/* GetFontPath */
    FALSE,			/* CreatePixmap */
    FALSE,			/* FreePixmap */
    FALSE,			/* CreateGC */
    FALSE,			/* ChangeGC */
    FALSE,			/* CopyGC */
    FALSE,			/* SetDashes */
    FALSE,			/* SetClipRectangles */
    FALSE,			/* FreeGC */
    TRUE,			/* ClearToBackground */
    TRUE,			/* CopyArea */
    TRUE,			/* CopyPlane */
    FALSE,			/* PolyPoint */
    FALSE,			/* PolyLine */
    FALSE,			/* PolySegment */
    FALSE,			/* PolyRectangle */
    FALSE,			/* PolyArc */
    FALSE,			/* FillPoly */
    FALSE,			/* PolyFillRectangle */
    FALSE,			/* PolyFillArc */
    FALSE,			/* PutImage */
    FALSE,			/* GetImage */
    FALSE,			/* PolyText */
    FALSE,			/* PolyText */
    FALSE,			/* ImageText8 */
    FALSE,			/* ImageText16 */
    FALSE,			/* CreateColormap */
    TRUE,			/* FreeColormap */
    FALSE,			/* CopyColormapAndFree */
    TRUE,			/* InstallColormap */
    TRUE,			/* UninstallColormap */
    FALSE,			/* ListInstalledColormaps */
    FALSE,			/* AllocColor */
    FALSE,			/* AllocNamedColor */
    FALSE,			/* AllocColorCells */
    FALSE,			/* AllocColorPlanes */
    FALSE,			/* FreeColors */
    FALSE,			/* StoreColors */
    FALSE,			/* StoreNamedColor */
    FALSE,			/* QueryColors */
    FALSE,			/* LookupColor */
    FALSE,			/* CreateCursor */
    FALSE,			/* CreateGlyphCursor */
    FALSE,			/* FreeCursor */
    FALSE,			/* RecolorCursor */
    FALSE,			/* QueryBestSize */
    FALSE,			/* QueryExtension */
    FALSE,			/* ListExtensions */
    TRUE,			/* ChangeKeyboardMapping */
    FALSE,			/* GetKeyboardMapping */
    FALSE,			/* ChangeKeyboardControl */
    FALSE,			/* GetKeyboardControl */
    FALSE,			/* Bell */
    FALSE,			/* ChangePointerControl */
    FALSE,			/* GetPointerControl */
    FALSE,			/* SetScreenSaver */
    FALSE,			/* GetScreenSaver */
    FALSE,			/* ChangeHosts */
    FALSE,			/* ListHosts */
    FALSE,			/* ChangeAccessControl */
    FALSE,			/* ChangeCloseDownMode */
    FALSE,			/* KillClient */
    TRUE,			/* RotateProperties */
    FALSE,			/* ForceScreenSaver */
    TRUE,			/* SetPointerMapping */
    FALSE,			/* GetPointerMapping */
    TRUE,			/* SetModifierMapping */
    FALSE,			/* GetModifierMapping */
    FALSE,			/* NoOp */
};


Bool
generates_errors(req)
    int         req;
{
    return TRUE;		/* eveyrthing causes an error */
}

Bool
generates_events(req)
    int         req;
{
    /* don't grok extensions */
    if (req > X_NoOperation)
	return TRUE;
    return cause_events[req];
}

Bool
generates_replies(req)
    int         req;
{
    /* don't grok extensions */
    if (req > X_NoOperation)
	return TRUE;
    return cause_replies[req];
}