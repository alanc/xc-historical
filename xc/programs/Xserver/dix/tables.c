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
/* $Header: tables.c,v 1.12 87/08/01 15:06:21 toddb Locked $ */

extern int    ProcBadRequest(), ProcCreateWindow(),
    ProcChangeWindowAttributes(), ProcGetWindowAttributes(),
    ProcDestroyWindow(), ProcDestroySubwindows(), ProcChangeSaveSet(),
    ProcReparentWindow(), ProcMapWindow(), ProcMapSubwindows(),
    ProcUnmapWindow(), ProcUnmapSubwindows(), ProcConfigureWindow(),
    ProcCirculateWindow(), ProcGetGeometry(), ProcQueryTree(),
    ProcInternAtom(), ProcGetAtomName(), ProcChangeProperty(),
    ProcDeleteProperty(), ProcGetProperty(), ProcListProperties(),
    ProcSetSelectionOwner(), ProcGetSelectionOwner(), ProcConvertSelection(),
    ProcSendEvent(), ProcGrabPointer(), ProcUngrabPointer(),
    ProcGrabButton(), ProcUngrabButton(), ProcChangeActivePointerGrab(),
    ProcGrabKeyboard(), ProcUngrabKeyboard(), ProcGrabKey(),
    ProcUngrabKey(), ProcAllowEvents(), ProcGrabServer(),
    ProcUngrabServer(), ProcQueryPointer(), ProcGetMotionEvents(),
    ProcTranslateCoords(), ProcWarpPointer(), ProcSetInputFocus(),
    ProcGetInputFocus(), ProcQueryKeymap(), ProcOpenFont(),
    ProcCloseFont(), ProcQueryFont(), ProcQueryTextExtents(),
    ProcListFonts(), ProcListFontsWithInfo(), ProcSetFontPath(),
    ProcGetFontPath(), ProcCreatePixmap(), ProcFreePixmap(),
    ProcCreateGC(), ProcChangeGC(), ProcCopyGC(),
    ProcSetDashes(), ProcSetClipRectangles(), ProcFreeGC(),
    ProcClearToBackground(), ProcCopyArea(), ProcCopyPlane(),
    ProcPolyPoint(), ProcPolyLine(), ProcPolySegment(),
    ProcPolyRectangle(), ProcPolyArc(), ProcFillPoly(),
    ProcPolyFillRectangle(), ProcPolyFillArc(), ProcPutImage(),
    ProcGetImage(), ProcPolyText(),
    ProcImageText(), ProcCreateColormap(),
    ProcFreeColormap(), ProcCopyColormapAndFree(), ProcInstallColormap(),
    ProcUninstallColormap(), ProcListInstalledColormaps(), ProcAllocColor(),
    ProcAllocNamedColor(), ProcAllocColorCells(), ProcAllocColorPlanes(),
    ProcFreeColors(), ProcStoreColors(), ProcStoreNamedColor(),
    ProcQueryColors(), ProcLookupColor(), ProcCreateCursor(),
    ProcCreateGlyphCursor(), ProcFreeCursor(), ProcRecolorCursor(),
    ProcQueryBestSize(), ProcQueryExtension(), ProcListExtensions(),
    ProcChangeKeyboardMapping(), ProcSetPointerMapping(),
    ProcGetKeyboardMapping(), ProcGetPointerMapping(),
    ProcChangeKeyboardControl(),
    ProcGetKeyboardControl(), ProcBell(), ProcChangePointerControl(),
    ProcGetPointerControl(), ProcSetScreenSaver(), ProcGetScreenSaver(),
    ProcChangeHosts(), ProcListHosts(), ProcChangeAccessControl(),
    ProcChangeCloseDownMode(), ProcKillClient(),
    ProcRotateProperties(), ProcForceScreenSaver(),
    ProcSetModifierMapping(), ProcGetModifierMapping(),
    ProcNoOperation();

extern int    SProcSProcBadRequest(), SProcCreateWindow(),
    SProcChangeWindowAttributes(), 
    SProcDestroyWindow(), SProcDestroySubwindows(), SProcChangeSaveSet(),
    SProcReparentWindow(), SProcMapWindow(), SProcMapSubwindows(),
    SProcUnmapWindow(), SProcUnmapSubwindows(), SProcConfigureWindow(),
    SProcCirculateWindow(), SProcGetGeometry(), SProcQueryTree(),
    SProcInternAtom(), SProcGetAtomName(), SProcChangeProperty(),
    SProcDeleteProperty(), SProcGetProperty(), SProcListProperties(),
    SProcSetSelectionOwner(), SProcGetSelectionOwner(),
    SProcConvertSelection(),
    SProcSendEvent(), SProcGrabPointer(), SProcUngrabPointer(),
    SProcGrabButton(), SProcUngrabButton(), SProcChangeActivePointerGrab(),
    SProcGrabKeyboard(), SProcUngrabKeyboard(), SProcGrabKey(),
    SProcUngrabKey(), SProcAllowEvents(), SProcGrabServer(),
    SProcUngrabServer(), SProcQueryPointer(), SProcGetMotionEvents(),
    SProcTranslateCoords(), SProcWarpPointer(), SProcSetInputFocus(),
    SProcGetInputFocus(), SProcQueryKeymap(), SProcOpenFont(),
    SProcCloseFont(), SProcQueryFont(), SProcQueryTextExtents(),
    SProcListFonts(), SProcListFontsWithInfo(), SProcSetFontPath(),
    SProcGetFontPath(), SProcCreatePixmap(), SProcFreePixmap(),
    SProcCreateGC(), SProcChangeGC(), SProcCopyGC(),
    SProcSetDashes(), SProcSetClipRectangles(), SProcFreeGC(),
    SProcClearToBackground(), SProcCopyArea(), SProcCopyPlane(),
    SProcPolyPoint(), SProcPolyLine(), SProcPolySegment(),
    SProcPolyRectangle(), SProcPolyArc(), SProcFillPoly(),
    SProcPolyFillRectangle(), SProcPolyFillArc(), SProcPutImage(),
    SProcGetImage(), SProcPolyText(), 
    SProcImageText(), SProcCreateColormap(),
    SProcFreeColormap(), SProcCopyColormapAndFree(), SProcInstallColormap(),
    SProcUninstallColormap(), SProcListInstalledColormaps(), SProcAllocColor(),
    SProcAllocNamedColor(), SProcAllocColorCells(), SProcAllocColorPlanes(),
    SProcFreeColors(), SProcStoreColors(), SProcStoreNamedColor(),
    SProcQueryColors(), SProcLookupColor(), SProcCreateCursor(),
    SProcCreateGlyphCursor(), SProcFreeCursor(), SProcRecolorCursor(),
    SProcQueryBestSize(), SProcQueryExtension(), SProcListExtensions(),
    SProcChangeKeyboardMapping(), SProcSetPointerMapping(),
    SProcGetKeyboardMapping(), SProcGetPointerMapping(),
    SProcChangeKeyboardControl(),
    SProcGetKeyboardControl(), SProcBell(), SProcChangePointerControl(),
    SProcGetPointerControl(), SProcSetScreenSaver(), SProcGetScreenSaver(),
    SProcChangeHosts(), SProcListHosts(), SProcChangeAccessControl(),
    SProcChangeCloseDownMode(), SProcKillClient(), 
    SProcRotateProperties(), SProcForceScreenSaver(),
    SProcSetModifierMapping(), SProcGetModifierMapping(),
    SProcNoOperation(), SProcResourceReq();

extern void 
    SErrorEvent(), NotImplemented(), SKeyButtonPtrEvent(), SKeyButtonPtrEvent(),
    SKeyButtonPtrEvent(), SKeyButtonPtrEvent(), SKeyButtonPtrEvent(),
    SEnterLeaveEvent(), SEnterLeaveEvent(), SFocusEvent(),
    SFocusEvent(), SKeymapNotifyEvent(), SExposeEvent(),
    SGraphicsExposureEvent(), SNoExposureEvent(), SVisibilityEvent(),
    SCreateNotifyEvent(), SDestroyNotifyEvent(), SUnmapNotifyEvent(),
    SMapNotifyEvent(), SMapRequestEvent(), SReparentEvent(),
    SConfigureNotifyEvent(), SConfigureRequestEvent(), SGravityEvent(),
    SResizeRequestEvent(), SCirculateEvent(), SCirculateEvent(),
    SPropertyEvent(), SSelectionClearEvent(), SSelectionRequestEvent(),
    SSelectionNotifyEvent(), SColormapEvent(), SClientMessageEvent();

extern void
    SBadRequestReply(), NotImplemented(),
    SGetWindowAttributesReply(), SGetGeometryReply(), SQueryTreeReply(),
    SInternAtomReply(), SGetAtomNameReply(), SGetPropertyReply(),
    SListPropertiesReply(), 
    SGetSelectionOwnerReply(), SGrabPointerReply(), SGrabKeyboardReply(),
    SQueryPointerReply(), SGetMotionEventsReply(), STranslateCoordsReply(),
    SGetInputFocusReply(), SQueryKeymapReply(), SQueryFontReply(),
    SQueryTextExtentsReply(), SListFontsReply(), SListFontsWithInfoReply(),
    SGetFontPathReply(), SGetImageReply(), SListInstalledColormapsReply(),
    SAllocColorReply(), SAllocNamedColorReply(), SAllocColorCellsReply(),
    SAllocColorPlanesReply(), SQueryColorsReply(), SLookupColorReply(),
    SQueryBestSizeReply(), SQueryExtensionReply(), SListExtensionsReply(),
    SGetKeyboardMappingReply(), SGetPointerMappingReply(),
    SGetKeyboardControlReply(), SGetPointerControlReply(),
    SSetModifierMappingReply(), SGetModifierMappingReply(),
    SGetScreenSaverReply(), SListHostsReply();

int (* ProcVector[256]) () =
{
    ProcBadRequest,
    ProcCreateWindow,
    ProcChangeWindowAttributes,
    ProcGetWindowAttributes,
    ProcDestroyWindow,
    ProcDestroySubwindows,		/* 5 */
    ProcChangeSaveSet,
    ProcReparentWindow,
    ProcMapWindow,
    ProcMapSubwindows,
    ProcUnmapWindow,			/* 10 */
    ProcUnmapSubwindows,
    ProcConfigureWindow,
    ProcCirculateWindow,
    ProcGetGeometry,
    ProcQueryTree,			/* 15 */
    ProcInternAtom,
    ProcGetAtomName,
    ProcChangeProperty,
    ProcDeleteProperty,
    ProcGetProperty,			/* 20 */
    ProcListProperties,
    ProcSetSelectionOwner,
    ProcGetSelectionOwner,
    ProcConvertSelection,
    ProcSendEvent,			/* 25 */
    ProcGrabPointer,
    ProcUngrabPointer,
    ProcGrabButton,
    ProcUngrabButton,
    ProcChangeActivePointerGrab,	/* 30 */
    ProcGrabKeyboard,
    ProcUngrabKeyboard,
    ProcGrabKey,
    ProcUngrabKey,
    ProcAllowEvents,			/* 35 */
    ProcGrabServer,
    ProcUngrabServer,
    ProcQueryPointer,
    ProcGetMotionEvents,
    ProcTranslateCoords,		/* 40 */
    ProcWarpPointer,
    ProcSetInputFocus,
    ProcGetInputFocus,
    ProcQueryKeymap,
    ProcOpenFont,			/* 45 */
    ProcCloseFont,
    ProcQueryFont,
    ProcQueryTextExtents,
    ProcListFonts,
    ProcListFontsWithInfo,		/* 50 */
    ProcSetFontPath,
    ProcGetFontPath,
    ProcCreatePixmap,
    ProcFreePixmap,
    ProcCreateGC,			/* 55 */
    ProcChangeGC,
    ProcCopyGC,
    ProcSetDashes,
    ProcSetClipRectangles,
    ProcFreeGC,				/* 60 */
    ProcClearToBackground,
    ProcCopyArea,
    ProcCopyPlane,
    ProcPolyPoint,
    ProcPolyLine,			/* 65 */
    ProcPolySegment,
    ProcPolyRectangle,
    ProcPolyArc,
    ProcFillPoly,
    ProcPolyFillRectangle,		/* 70 */
    ProcPolyFillArc,
    ProcPutImage,
    ProcGetImage,
    ProcPolyText,
    ProcPolyText,			/* 75 */
    ProcImageText,
    ProcImageText,
    ProcCreateColormap,
    ProcFreeColormap,
    ProcCopyColormapAndFree,		/* 80 */
    ProcInstallColormap,
    ProcUninstallColormap,
    ProcListInstalledColormaps,
    ProcAllocColor,
    ProcAllocNamedColor,		/* 85 */
    ProcAllocColorCells,
    ProcAllocColorPlanes,
    ProcFreeColors,
    ProcStoreColors,
    ProcStoreNamedColor,		/* 90 */
    ProcQueryColors,
    ProcLookupColor,
    ProcCreateCursor,
    ProcCreateGlyphCursor,
    ProcFreeCursor,			/* 95 */
    ProcRecolorCursor,
    ProcQueryBestSize,
    ProcQueryExtension,
    ProcListExtensions,
    ProcChangeKeyboardMapping,		/* 100 */
    ProcGetKeyboardMapping,
    ProcChangeKeyboardControl,
    ProcGetKeyboardControl,
    ProcBell,
    ProcChangePointerControl,		/* 105 */
    ProcGetPointerControl,
    ProcSetScreenSaver,
    ProcGetScreenSaver,
    ProcChangeHosts,
    ProcListHosts,			/* 110 */
    ProcChangeAccessControl,
    ProcChangeCloseDownMode,
    ProcKillClient,
    ProcRotateProperties,
    ProcForceScreenSaver,		/* 115 */
    ProcSetPointerMapping,
    ProcGetPointerMapping,
    ProcSetModifierMapping,
    ProcGetModifierMapping,
    0,					/* 120 */
    0,
    0,
    0,
    0,
    0,					/* 125 */
    0,
    ProcNoOperation    
};

int (* SwappedProcVector[256]) () =
{
    ProcBadRequest,
    SProcCreateWindow,
    SProcChangeWindowAttributes,
    SProcResourceReq,			/* GetWindowAttributes */
    SProcResourceReq,			/* DestroyWindow */
    SProcResourceReq,			/* 5 DestroySubwindows */
    SProcChangeSaveSet,
    SProcReparentWindow,
    SProcResourceReq,			/* MapWindow */
    SProcResourceReq,			/* MapSubwindows */
    SProcResourceReq,			/* 10 UnmapWindow */
    SProcResourceReq,			/* UnmapSubwindows */
    SProcConfigureWindow,
    SProcCirculateWindow,
    SProcResourceReq,			/* GetGeometry */
    SProcResourceReq,			/* 15 QueryTree */
    SProcInternAtom,
    SProcResourceReq,			/* SProcGetAtomName, */
    SProcChangeProperty,
    SProcDeleteProperty,
    SProcGetProperty,			/* 20 */
    SProcResourceReq,			/* SProcListProperties, */
    SProcSetSelectionOwner,
    SProcResourceReq, 			/* SProcGetSelectionOwner, */
    SProcConvertSelection,
    SProcSendEvent,			/* 25 */
    SProcGrabPointer,
    SProcResourceReq, 			/* SProcUngrabPointer, */
    SProcGrabButton,
    SProcUngrabButton,
    SProcChangeActivePointerGrab,	/* 30 */
    SProcGrabKeyboard,
    SProcResourceReq,			/* SProcUngrabKeyboard, */
    SProcGrabKey,
    SProcUngrabKey,
    SProcAllowEvents,			/* 35 */
    SProcResourceReq,			/* SProcGrabServer, */
    SProcResourceReq,			/* SProcUngrabServer, */
    SProcResourceReq,			/* SProcQueryPointer, */
    SProcGetMotionEvents,
    SProcTranslateCoords,		/*40 */
    SProcWarpPointer,
    SProcSetInputFocus,
    SProcGetInputFocus,
    SProcQueryKeymap,
    SProcOpenFont,			/* 45 */
    SProcResourceReq,			/* SProcCloseFont, */
    SProcResourceReq, 			/* SProcQueryFont, */
    SProcQueryTextExtents, 
    SProcListFonts,
    SProcListFontsWithInfo,		/* 50 */
    SProcSetFontPath,
    SProcGetFontPath,
    SProcCreatePixmap,
    SProcResourceReq,			/* SProcFreePixmap, */
    SProcCreateGC,			/* 55 */
    SProcChangeGC,
    SProcCopyGC,
    SProcSetDashes,
    SProcSetClipRectangles,
    SProcResourceReq,			/* 60 SProcFreeGC, */
    SProcClearToBackground,
    SProcCopyArea,
    SProcCopyPlane,
    SProcPolyPoint,
    SProcPolyLine,			/* 65 */
    SProcPolySegment,
    SProcPolyRectangle,
    SProcPolyArc,
    SProcFillPoly,
    SProcPolyFillRectangle,		/* 70 */
    SProcPolyFillArc,
    SProcPutImage,
    SProcGetImage,
    SProcPolyText,
    SProcPolyText,			/* 75 */
    SProcImageText,
    SProcImageText,
    SProcCreateColormap,
    SProcResourceReq,			/* SProcFreeColormap, */
    SProcCopyColormapAndFree,		/* 80 */
    SProcResourceReq,			/* SProcInstallColormap, */
    SProcResourceReq,			/* SProcUninstallColormap, */
    SProcResourceReq, 			/* SProcListInstalledColormaps,
    SProcAllocColor,
    SProcAllocNamedColor,		/* 85 */
    SProcAllocColorCells,
    SProcAllocColorPlanes,
    SProcFreeColors,
    SProcStoreColors,
    SProcStoreNamedColor,		/* 90 */
    SProcQueryColors,
    SProcLookupColor,
    SProcCreateCursor,
    SProcCreateGlyphCursor,
    SProcResourceReq,			/* 95 SProcFreeCursor, */
    SProcRecolorCursor,
    SProcQueryBestSize,
    SProcQueryExtension,
    SProcListExtensions,
    SProcChangeKeyboardMapping,		/* 100 */
    SProcGetKeyboardMapping,
    SProcChangeKeyboardControl,
    SProcGetKeyboardControl,
    SProcBell,
    SProcChangePointerControl,		/* 105 */
    SProcGetPointerControl,
    SProcSetScreenSaver,
    SProcGetScreenSaver,
    SProcChangeHosts,
    SProcListHosts,			/* 110 */
    SProcResourceReq,			/* SProcChangeAccessControl, */
    SProcResourceReq,			/* SProcChangeCloseDownMode, */
    SProcResourceReq,			/* SProcKillClient, */
    SProcRotateProperties,
    SProcForceScreenSaver,		/* 115 */
    SProcSetPointerMapping,
    SProcGetPointerMapping,
    SProcSetModifierMapping,
    SProcGetModifierMapping,
    0,					/* 120 */
    0,
    0,
    0,
    0,
    0,					/* 125 */
    0,
    SProcNoOperation
};

void (* EventSwapVector[128]) () =
{
    SErrorEvent,
    NotImplemented,
    SKeyButtonPtrEvent,
    SKeyButtonPtrEvent,
    SKeyButtonPtrEvent,
    SKeyButtonPtrEvent,			/* 5 */
    SKeyButtonPtrEvent,
    SEnterLeaveEvent,
    SEnterLeaveEvent,
    SFocusEvent,
    SFocusEvent,			/* 10 */
    SKeymapNotifyEvent,
    SExposeEvent,
    SGraphicsExposureEvent,
    SNoExposureEvent,
    SVisibilityEvent,			/* 15 */
    SCreateNotifyEvent,
    SDestroyNotifyEvent,
    SUnmapNotifyEvent,
    SMapNotifyEvent,
    SMapRequestEvent,			/* 20 */
    SReparentEvent,
    SConfigureNotifyEvent,
    SConfigureRequestEvent,
    SGravityEvent,
    SResizeRequestEvent,		/* 25 */
    SCirculateEvent,
    SCirculateEvent,
    SPropertyEvent,
    SSelectionClearEvent,
    SSelectionRequestEvent,		/* 30 */
    SSelectionNotifyEvent,
    SColormapEvent,
    SClientMessageEvent
};


void (* ReplySwapVector[256]) () =
{
    NotImplemented,
    NotImplemented,
    NotImplemented,
    SGetWindowAttributesReply,
    NotImplemented,
    NotImplemented,			/* 5 */
    NotImplemented,
    NotImplemented,
    NotImplemented,
    NotImplemented,
    NotImplemented,			/* 10 */
    NotImplemented,
    NotImplemented,
    NotImplemented,
    SGetGeometryReply,
    SQueryTreeReply,			/* 15 */
    SInternAtomReply,
    SGetAtomNameReply,
    NotImplemented,
    NotImplemented,
    SGetPropertyReply,			/* 20 */
    SListPropertiesReply,
    NotImplemented,
    SGetSelectionOwnerReply,
    NotImplemented,
    NotImplemented,			/* 25 */
    SGrabPointerReply,
    NotImplemented,
    NotImplemented,
    NotImplemented,
    NotImplemented,			/* 30 */
    SGrabKeyboardReply,
    NotImplemented,
    NotImplemented,
    NotImplemented,
    NotImplemented,			/* 35 */
    NotImplemented,
    NotImplemented,
    SQueryPointerReply,
    SGetMotionEventsReply,
    STranslateCoordsReply,		/* 40 */
    NotImplemented,
    NotImplemented,
    SGetInputFocusReply,
    SQueryKeymapReply,
    NotImplemented,			/* 45 */
    NotImplemented,
    SQueryFontReply,
    SQueryTextExtentsReply,
    SListFontsReply,
    SListFontsWithInfoReply,		/* 50 */
    NotImplemented,
    SGetFontPathReply,
    NotImplemented,
    NotImplemented,
    NotImplemented,			/* 55 */
    NotImplemented,
    NotImplemented,
    NotImplemented,
    NotImplemented,
    NotImplemented,			/* 60 */
    NotImplemented,
    NotImplemented,
    NotImplemented,
    NotImplemented,
    NotImplemented,			/* 65 */
    NotImplemented,
    NotImplemented,
    NotImplemented,
    NotImplemented,
    NotImplemented,			/* 70 */
    NotImplemented,
    NotImplemented,
    SGetImageReply,
    NotImplemented,
    NotImplemented,			/* 75 */
    NotImplemented,
    NotImplemented,
    NotImplemented,
    NotImplemented,
    NotImplemented,			/* 80 */
    NotImplemented,
    NotImplemented,
    SListInstalledColormapsReply,
    SAllocColorReply,
    SAllocNamedColorReply,		/* 85 */
    SAllocColorCellsReply,
    SAllocColorPlanesReply,
    NotImplemented,
    NotImplemented,
    NotImplemented,			/* 90 */
    SQueryColorsReply,
    SLookupColorReply,
    NotImplemented,
    NotImplemented,
    NotImplemented,			/* 95 */
    NotImplemented,
    SQueryBestSizeReply,
    SQueryExtensionReply,
    SListExtensionsReply,
    NotImplemented,			/* 100 */
    SGetKeyboardMappingReply,
    NotImplemented,
    SGetKeyboardControlReply,
    NotImplemented,
    NotImplemented,			/* 105 */
    SGetPointerControlReply,
    NotImplemented,
    SGetScreenSaverReply,
    NotImplemented,
    SListHostsReply,			/* 110 */
    NotImplemented,
    NotImplemented,
    NotImplemented,
    NotImplemented,
    NotImplemented,			/* 115 */
    NotImplemented,
    SGetPointerMappingReply,
    NotImplemented,			/* 118 */
    SGetModifierMappingReply,		/* 119 */
    NotImplemented,			/* 120 */
    NotImplemented,			/* 121 */
    NotImplemented,			/* 122 */
    NotImplemented,			/* 123 */
    NotImplemented,			/* 124 */
    NotImplemented,			/* 125 */
    NotImplemented,			/* 126 */
    NotImplemented,			/* NoOperation */
    NotImplemented
};
