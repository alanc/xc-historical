/*
* $XConsortium: TextSrcP.h,v 1.2 89/03/30 16:06:03 jim Exp $
*/


/***********************************************************
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
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

#ifndef _XawTextSrcP_h
#define _XawTextSrcP_h

#include <X11/Xaw/Text.h>

typedef enum {XawsdLeft, XawsdRight} XawTextScanDirection;
typedef enum {XawstPositions, XawstWhiteSpace, XawstEOL, XawstParagraph,
		XawstAll} XawTextScanType;

typedef struct _XawTextSource {
    XawTextPosition	(*Read)();
    int			(*Replace)();
    XawTextPosition	(*GetLastPos)();
    int			(*SetLastPos)();
    XawTextPosition	(*Scan)();
    void		(*AddWidget)( /* source, widget */ );
    void		(*RemoveWidget)( /* source, widget */ );
    Boolean		(*GetSelection)( /* source, left, right, selection */);
    void		(*SetSelection)( /* source, left, right, selection */);
    Boolean		(*ConvertSelection)( /* Display*, source, ... */ );
    XawTextEditType	edit_mode;
    caddr_t		data;	    
    };

typedef struct _XawTextSink {
    XFontStruct	*font;
    int foreground;
    int (*Display)();
    int (*InsertCursor)();
    int (*ClearToBackground)();
    int (*FindPosition)();
    int (*FindDistance)();
    int (*Resolve)();
    int (*MaxLines)();
    int (*MaxHeight)();
    void (*SetTabs)();		/* widget, offset, tab_count, *tabs */
    caddr_t data;
    };

typedef enum {XawisOn, XawisOff} XawTextInsertState;

typedef enum {XawsmTextSelect, XawsmTextExtend} XawTextSelectionMode;

typedef enum {XawactionStart, XawactionAdjust, XawactionEnd}
    XawTextSelectionAction;

typedef struct {
    XawTextPosition   left, right;
    XawTextSelectType type;
    Atom*	     selections;
    int		     atom_count;
    int		     array_size;
} XawTextSelection;

typedef enum  {Normal, Selected }highlightType;

#ifndef XAW_NO_COMPATABILITY
/*************************************************************
 * For Compatibility only.                                   */

#define EditDone	   XawEditDone
#define EditError	   XawEditError
#define PositionError	   XawPositionError

#define XtEditDone	   XawEditDone
#define XtEditError	   XawEditError
#define XtPositionError	   XawPositionError

#define _XtTextSink        _XawTextSink
#define _XtTextSource      _XawTextSource

#define XtisOn             XawisOn
#define XtisOff            XawisOff

#define XtsmTextSelect     XawsmTextSelect
#define XtsmTextExtend     XawsmTextExtend

#define XtactionStart      XawactionStart
#define XtactionAdjust     XawactionAdjust
#define XtactionEnd        XawactionEnd

#define XtsdLeft           XawsdLeft
#define XtsdRight          XawsdRight

#define XtstPositions      XawstPositions
#define XtstWhiteSpace     XawstWhiteSpace
#define XtstEOL            XawstEOL
#define XtstParagraph      XawstParagraph
#define XtstAll            XawstAll

#define XtTextSelectionAction XawTextSelectionAction
#define XtTextSelection       XawTextSelection
#define XtTextScanDirection   XawTextScanDirection
#define XtTextScanType        XawTextScanType


/*************************************************************/
#endif /* XAW_NO_COMPATABILITY */

#endif /* _XawTextSrcP_h */



