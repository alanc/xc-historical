/*
 * $XConsortium: TextSrc.h,v 1.3 89/10/04 13:56:11 kit Exp $
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

#ifndef _XawTextSrc_h
#define _XawTextSrc_h

/***********************************************************************
 *
 * TextSrc Object
 *
 ***********************************************************************/

#include <X11/Object.h>

/* Resources:

 Name		     Class		RepType		Default Value
 ----		     -----		-------		-------------
 editType	     EditType		XawTextEditType	XawtextRead

*/
 
/* Class record constants */

extern WidgetClass textSrcObjectClass;

typedef struct _TextSrcClassRec *TextSrcObjectClass;
typedef struct _TextSrcRec      *TextSrcObject;

typedef enum {XawstPositions, XawstWhiteSpace, XawstEOL, XawstParagraph,
              XawstAll} XawTextScanType;
typedef enum {Normal, Selected }highlightType;
typedef enum {XawsmTextSelect, XawsmTextExtend} XawTextSelectionMode;
typedef enum {XawactionStart, XawactionAdjust, XawactionEnd}
    XawTextSelectionAction;

typedef long XawTextPosition;

/*
 * Error Conditions:
 */

#define XawTextReadError -1
#define XawTextScanError -1

/************************************************************
 *
 * Public Functions.
 *
 ************************************************************/

/*	Function Name: XawTextSourceRead
 *	Description: This function reads the source.
 *	Arguments: w - the TextSrc Object.
 *                 pos - position of the text to retreive.
 * RETURNED        text - text block that will contain returned text.
 *                 length - maximum number of characters to read.
 *	Returns: The number of characters read into the buffer.
 */

XawTextPosition XawTextSourceRead(/* w, pos, text, length */);
/*
Widget w;
XawTextPosition pos;
XawTextBlock *text;	
int length;		
*/

/*	Function Name: XawTextSourceReplace.
 *	Description: Replaces a block of text with new text.
 *	Arguments: src - the Text Source Object.
 *                 startPos, endPos - ends of text that will be removed.
 *                 text - new text to be inserted into buffer at startPos.
 *	Returns: XawEditError or XawEditDone.
 */

int XawTextSourceReplace (/* w, startPos, endPos, text */);
/*
Widget w;
XawTextPosition startPos, endPos;
XawTextBlock *text;
*/

/*	Function Name: XawTextSourceScan
 *	Description: Scans the text source for the number and type
 *                   of item specified.
 *	Arguments: w - the TextSrc Object.
 *                 position - the position to start scanning.
 *                 type - type of thing to scan for.
 *                 dir - direction to scan.
 *                 count - which occurance if this thing to search for.
 *                 include - whether or not to include the character found in
 *                           the position that is returned. 
 *	Returns: The position of the text.
 *
 */

XawTextPosition XawTextSourceScan(/* w, position, type, dir, count, include*/);
/*
Widget                w;
XawTextPosition       position;
XawTextScanType       type;
XawTextScanDirection  dir;
int     	      count;
Boolean	              include;
*/

/*	Function Name: XawTextSourceSearch
 *	Description: Searchs the text source for the text block passed
 *	Arguments: w - the TextSource Object.
 *                 position - the position to start scanning.
 *                 dir - direction to scan.
 *                 text - the text block to search for.
 *	Returns: The position of the text we are searching for or
 *               XawTextSearchError.
 */

XawTextPosition XawTextSourceSearch(/* w, position, dir, text */);
/*
Widget                w;
XawTextPosition       position;
XawTextScanDirection  dir;
XawTextBlock *        text;
*/

/*	Function Name: XawTextSourceConvertSelection
 *	Description: Dummy selection converter.
 *	Arguments: w - the TextSrc object.
 *                 selection - the current selection atom.
 *                 target    - the current target atom.
 *                 type      - the type to conver the selection to.
 * RETURNED        value, length - the return value that has been converted.
 * RETURNED        format    - the format of the returned value.
 *	Returns: TRUE if the selection has been converted.
 *
 */

Boolean XawTextSourceConvertSelection(/* w, selection, target, 
					 type, value, length, format */);
/*
Widget w;
Atom * selection, * target, * type;
caddr_t * value;
unsigned long * length;
int * format;
*/

/*	Function Name: XawTextSourceSetSelection
 *	Description: allows special setting of the selection.
 *	Arguments: w - the TextSrc object.
 *                 left, right - bounds of the selection.
 *                 selection - the selection atom.
 *	Returns: none
 */

void XawTextSourceSetSelection(/* w, left, right, selection */);
/*
Widget w;
XawTextPosition left, right;
Atom selection;
*/

#ifdef XAW_BC
/*************************************************************
 * For Compatibility only.                                   */

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
#endif /* XAW_BC */

#endif /* _XawTextSrc_h */
/* DON'T ADD STUFF AFTER THIS #endif */
