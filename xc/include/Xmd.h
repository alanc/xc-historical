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
#ifndef XMD_H
#define XMD_H 1
/* $Header: Xmd.h,v 1.30 88/09/01 16:26:00 jim Exp $ */
/*
 *  Xmd.h: MACHINE DEPENDENT DECLARATIONS.
 */

#ifdef CRAY
#define WORD64
#define UNSIGNEDBITFIELDS
#endif


/*
 * ibm pcc doesn't understand pragmas.
 */
#if defined(ibm032) && defined(__HIGHC__)
pragma on(pointers_compatible);
pragma off(char_default_unsigned);
#endif

/*
 * Bitfield suffixes for the protocol structure elements, if you
 * need them.  Note that bitfields are not guarranteed to be signed
 * (or even unsigned) according to ANSI C.
 */
#ifdef WORD64
#define B32 :32
#define B16 :16
#else
#define B32
#define B16
#endif /* WORD64 */

#if defined(WORD64) && defined(UNSIGNEDBITFIELDS)
#define cvtINT8toInt(val)   (((val) & 0x00000080) ? ((val) | 0xffffffffffffff00) : (val))
#define cvtINT16toInt(val)  (((val) & 0x00008000) ? ((val) | 0xffffffffffff0000) : (val))
#define cvtINT32toInt(val)  (((val) & 0x80000000) ? ((val) | 0xffffffff00000000) : (val))
#define cvtINT8toLong(val)  cvtINT8ToInt(val)
#define cvtINT16toLong(val) cvtINT16ToInt(val)
#define cvtINT32toLong(val) cvtINT32ToInt(val)
#else
#define cvtINT8toInt(val) (val)
#define cvtINT16toInt(val) (val)
#define cvtINT32toInt(val) (val)
#define cvtINT8toLong(val) (val)
#define cvtINT16toLong(val) (val)
#define cvtINT32toLong(val) (val)
#endif /* UNSIGNEDBITFIELDS */


typedef long           INT32;
typedef short          INT16;
typedef char           INT8;

typedef unsigned long CARD32;
typedef unsigned short CARD16;
typedef unsigned char  CARD8;

typedef unsigned long		BITS32;
typedef unsigned short		BITS16;
typedef unsigned char		BYTE;

typedef unsigned char            BOOL;


/*
 * Stuff to handle large architecture machines; the constants were generated
 * on a 32-bit machine and must coorespond to the protocol.
 */
#ifdef WORD64
#define MUSTCOPY
#endif /* WORD64 */


#ifdef MUSTCOPY

#if defined(__STDC__) && !defined(UNIXCPP)
#define SIZEOF(x) sizeof_##x
#else
#define SIZEOF(x) sizeof_/**/x
#endif /* if ANSI C compiler else not */

/*
 * This macro must not cast or else pointers will get aligned and be wrong
 */
#define NEXTPTR(p,t)  (((char *) p) + SIZEOF(t))

#define sizeof_xSegment 8
#define sizeof_xPoint 4
#define sizeof_xRectangle 8
#define sizeof_xArc 12
#define sizeof_xConnClientPrefix 12
#define sizeof_xConnSetupPrefix 8
#define sizeof_xConnSetup 32
#define sizeof_xPixmapFormat 8
#define sizeof_xDepth 8
#define sizeof_xVisualType 24
#define sizeof_xWindowRoot 40
#define sizeof_xTimecoord 8
#define sizeof_xHostEntry 4
#define sizeof_xCharInfo 12
#define sizeof_xFontProp 8
#define sizeof_xTextElt 2
#define sizeof_xColorItem 12
#define sizeof_xrgb 8
#define sizeof_xGenericReply 32
#define sizeof_xGetWindowAttributesReply 44
#define sizeof_xGetGeometryReply 32
#define sizeof_xQueryTreeReply 32
#define sizeof_xInternAtomReply 32
#define sizeof_xGetAtomNameReply 32
#define sizeof_xGetPropertyReply 32
#define sizeof_xListPropertiesReply 32
#define sizeof_xGetSelectionOwnerReply 32
#define sizeof_xGrabPointerReply 32
#define sizeof_xQueryPointerReply 32
#define sizeof_xGetMotionEventsReply 32
#define sizeof_xTranslateCoordsReply 32
#define sizeof_xGetInputFocusReply 32
#define sizeof_xQueryKeymapReply 40
#define sizeof_xQueryFontReply 60
#define sizeof_xQueryTextExtentsReply 32
#define sizeof_xListFontsReply 32
#define sizeof_xGetFontPathReply 32
#define sizeof_xGetImageReply 32
#define sizeof_xListInstalledColormapsReply 32
#define sizeof_xAllocColorReply 32
#define sizeof_xAllocNamedColorReply 32
#define sizeof_xAllocColorCellsReply 32
#define sizeof_xAllocColorPlanesReply 32
#define sizeof_xQueryColorsReply 32
#define sizeof_xLookupColorReply 32
#define sizeof_xQueryBestSizeReply 32
#define sizeof_xQueryExtensionReply 32
#define sizeof_xListExtensionsReply 32
#define sizeof_xSetMappingReply 32
#define sizeof_xGetKeyboardControlReply 52
#define sizeof_xGetPointerControlReply 32
#define sizeof_xGetScreenSaverReply 32
#define sizeof_xListHostsReply 32
#define sizeof_xSetModifierMappingReply 32
#define sizeof_xError 32
#define sizeof_xEvent 32
#define sizeof_xKeymapEvent 32
#define sizeof_xReq 4
#define sizeof_xResourceReq 8
#define sizeof_xCreateWindowReq 32
#define sizeof_xChangeWindowAttributesReq 12
#define sizeof_xChangeSaveSetReq 8
#define sizeof_xReparentWindowReq 16
#define sizeof_xConfigureWindowReq 12
#define sizeof_xCirculateWindowReq 8
#define sizeof_xInternAtomReq 8
#define sizeof_xChangePropertyReq 24
#define sizeof_xDeletePropertyReq 12
#define sizeof_xGetPropertyReq 24
#define sizeof_xSetSelectionOwnerReq 16
#define sizeof_xConvertSelectionReq 24
#define sizeof_xSendEventReq 44
#define sizeof_xGrabPointerReq 24
#define sizeof_xGrabButtonReq 24
#define sizeof_xUngrabButtonReq 12
#define sizeof_xChangeActivePointerGrabReq 16
#define sizeof_xGrabKeyboardReq 16
#define sizeof_xGrabKeyReq 16
#define sizeof_xUngrabKeyReq 12
#define sizeof_xAllowEventsReq 8
#define sizeof_xGetMotionEventsReq 16
#define sizeof_xTranslateCoordsReq 16
#define sizeof_xWarpPointerReq 24
#define sizeof_xSetInputFocusReq 12
#define sizeof_xOpenFontReq 12
#define sizeof_xQueryTextExtentsReq 8
#define sizeof_xListFontsReq 8
#define sizeof_xSetFontPathReq 8
#define sizeof_xCreatePixmapReq 16
#define sizeof_xCreateGCReq 16
#define sizeof_xChangeGCReq 12
#define sizeof_xCopyGCReq 16
#define sizeof_xSetDashesReq 12
#define sizeof_xSetClipRectanglesReq 12
#define sizeof_xCopyAreaReq 28
#define sizeof_xCopyPlaneReq 32
#define sizeof_xPolyPointReq 12
#define sizeof_xPolySegmentReq 12
#define sizeof_xFillPolyReq 16
#define sizeof_xPutImageReq 24
#define sizeof_xGetImageReq 20
#define sizeof_xPolyTextReq 16
#define sizeof_xImageTextReq 16
#define sizeof_xCreateColormapReq 16
#define sizeof_xCopyColormapAndFreeReq 12
#define sizeof_xAllocColorReq 16
#define sizeof_xAllocNamedColorReq 12
#define sizeof_xAllocColorCellsReq 12
#define sizeof_xAllocColorPlanesReq 16
#define sizeof_xFreeColorsReq 12
#define sizeof_xStoreColorsReq 8
#define sizeof_xStoreNamedColorReq 16
#define sizeof_xQueryColorsReq 8
#define sizeof_xLookupColorReq 12
#define sizeof_xCreateCursorReq 32
#define sizeof_xCreateGlyphCursorReq 32
#define sizeof_xRecolorCursorReq 20
#define sizeof_xQueryBestSizeReq 12
#define sizeof_xQueryExtensionReq 8
#define sizeof_xChangeKeyboardControlReq 8
#define sizeof_xBellReq 4
#define sizeof_xChangePointerControlReq 12
#define sizeof_xSetScreenSaverReq 12
#define sizeof_xChangeHostsReq 8
#define sizeof_xListHostsReq 4
#define sizeof_xChangeModeReq 4
#define sizeof_xRotatePropertiesReq 12
#define sizeof_xReply 32
#define sizeof_xGrabKeyboardReply 32
#define sizeof_xListFontsWithInfoReply 60
#define sizeof_xSetPointerMappingReply 32
#define sizeof_xGetKeyboardMappingReply 32
#define sizeof_xGetPointerMappingReply 32
#define sizeof_xListFontsWithInfoReq 8
#define sizeof_xPolyLineReq 12
#define sizeof_xPolyArcReq 12
#define sizeof_xPolyRectangleReq 12
#define sizeof_xPolyFillRectangleReq 12
#define sizeof_xPolyFillArcReq 12
#define sizeof_xPolyText8Req 16
#define sizeof_xPolyText16Req 16
#define sizeof_xImageText8Req 16
#define sizeof_xImageText16Req 16
#define sizeof_xSetPointerMappingReq 4
#define sizeof_xForceScreenSaverReq 4
#define sizeof_xSetCloseDownModeReq 4
#define sizeof_xClearAreaReq 16
#define sizeof_xSetAccessControlReq 4
#define sizeof_xGetKeyboardMappingReq 8
#define sizeof_xSetModifierMappingReq 4
#define sizeof_xPropIconSize 24
#define sizeof_xChangeKeyboardMappingReq 8

#else /* else not MUSTCOPY, this is used for 32-bit machines */

#define SIZEOF(x) sizeof(x)
/*
 * this version should leave result of type (t *), but that should only be 
 * used when not in MUSTCOPY
 */  
#define NEXTPTR(p,t) (((t *)(p)) + 1)

#endif /* MUSTCOPY - used machines whose C structs don't line up with proto */

#endif /* XMD_H */
