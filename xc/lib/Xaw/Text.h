/*
* $XConsortium: Text.h,v 1.20 89/05/11 14:20:31 kit Exp $
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

#ifndef _XawText_h
#define _XawText_h

/****************************************************************
 *
 * Text widget
 *
 ****************************************************************/

/* Parameters:

 Name		     Class		RepType		Default Value
 ----		     -----		-------		-------------
 background	     Background		Pixel		XtDefaultBackground
 border		     BorderColor	Pixel		XtDefaultForeground
 borderWidth	     BorderWidth	Dimension	1
 destroyCallback     Callback		Pointer		NULL
 dialogHOffset	     Margin		int		10
 dialogVOffset	     Margin		int		10
 displayCaret	     Output		Boolean		True
 displayNonPrinting  Output		Boolean		True
 displayPosition     TextPosition	int		0
 echo		     Output		Boolean		True
 editType	     EditType		XtTextEditType	XttextRead
 height		     Height		Dimension	font height
 insertPosition	     TextPosition	int		0
 leftMargin	     Margin		Dimension	2
 mappedWhenManaged   MappedWhenManaged	Boolean		True
 selectTypes	     SelectTypes	Pointer		(internal)
 selection	     Selection		Pointer		empty selection
 sensitive	     Sensitive		Boolean		True
 textOptions	     TextOptions	int		0
 textSink	     TextSink		Pointer		(none)
 textSource	     TextSource		Pointer		(none)
 width		     Width		Dimension	100
 x		     Position		int		0
 y		     Position		int		0

*/

#define XtNtextOptions		"textOptions"
#define XtNdialogHOffset	"dialogHOffset"
#define XtNdialogVOffset	"dialogVOffset"
#define XtNdisplayCaret		"displayCaret"
#define XtNdisplayNonPrinting	"displayNonPrinting"
#define XtNdisplayPosition      "displayPosition"
#define XtNecho			"echo"
#define XtNinsertPosition	"insertPosition"
#define XtNleftMargin		"leftMargin"
#define XtNselectTypes		"selectTypes"
#define XtNselection		"selection"

#define XtCOutput		"Output"
#define XtCSelectTypes		"SelectTypes"

/* Return codes from XawTextReplace */

#define XawEditDone		0
#define XawEditError		1
#define XawPositionError	2

/* Class record constants */

extern WidgetClass textWidgetClass;
extern Atom FMT8BIT;

typedef struct _TextClassRec *TextWidgetClass;
typedef struct _TextRec      *TextWidget;

/* other stuff */

typedef long XawTextPosition;
typedef struct _XawTextSource XawTextSourceRec, *XawTextSource;
typedef struct _XawTextSink XawTextSinkRec, *XawTextSink;

typedef enum {XawtextRead, XawtextAppend, XawtextEdit} XawTextEditType;
typedef enum {XawselectNull, XawselectPosition, XawselectChar, XawselectWord,
    XawselectLine, XawselectParagraph, XawselectAll} XawTextSelectType;

#define wordBreak		0x01
#define scrollVertical		0x02
#define scrollHorizontal	0x04
#define scrollOnOverflow	0x08
#define resizeWidth		0x10
#define resizeHeight		0x20
#define editable		0x40

typedef struct {
    int  firstPos;
    int  length;
    char *ptr;
    Atom format;
    } XawTextBlock, *XawTextBlockPtr; 
#ifdef XAW_BC
/************************************************************
 *
 * This Stuff is only for compatibility, and will go away in 
 * future releases.                                         */

typedef long XtTextPosition;
typedef struct _XawTextSource XtTextSourceRec, *XtTextSource;
typedef struct _XawTextSink XtTextSinkRec, *XtTextSink;

#define XtTextBlock                XawTextBlock
#define XtTextBlockPtr             XawTextBlockPtr

#define EditDone	   XawEditDone
#define EditError	   XawEditError
#define PositionError	   XawPositionError

#define XtEditDone	   XawEditDone
#define XtEditError	   XawEditError
#define XtPositionError	   XawPositionError

#define XttextRead                 XawtextRead
#define XttextAppend               XawtextAppend
#define XttextEdit                 XawtextEdit
#define XtTextEditType             XawTextEditType
#define XtselectNull               XawselectNull

#define XtselectPosition           XawselectPosition
#define XtselectChar               XawselectChar
#define XtselectWord               XawselectWord
#define XtselectLine               XawselectLine
#define XtselectParagraph          XawselectParagraph
#define XtselectAll                XawselectAll
#define XtTextSelectType           XawTextSelectType

#define XtTextDisableRedisplay     XawTextDisableRedisplay
#define XtTextEnableRedisplay      XawTextEnableRedisplay
#define XtTextGetSource            XawTextGetSource

#define XtTextDisplay              XawTextDisplay
#define XtTextSetSelectionArray    XawTextSetSelectionArray
#define XtTextSetLastPos           XawTextSetLastPos
#define XtTextGetSelectionPos      XawTextGetSelectionPos
#define XtTextSetSource            XawTextSetSource
#define XtTextReplace              XawTextReplace
#define XtTextTopPosition          XawTextTopPosition
#define XtTextSetInsertionPoint    XawTextSetInsertionPoint
#define XtTextGetInsertionPoint    XawTextGetInsertionPoint
#define XtTextUnsetSelection       XawTextUnsetSelection
#define XtTextChangeOptions        XawTextChangeOptions
#define XtTextGetOptions           XawTextGetOptions
#define XtTextSetSelection         XawTextSetSelection
#define XtTextInvalidate           XawTextInvalidate

#define XtAsciiSinkCreate          XawAsciiSinkCreate
#define XtAsciiSinkDestroy         XawAsciiSinkDestroy
#define XtDiskSourceCreate         XawDiskSourceCreate
#define XtDiskSourceDestroy        XawDiskSourceDestroy
#define XtStringSourceCreate       XawStringSourceCreate
#define XtStringSourceDestroy      XawStringSourceDestroy

/*************************************************************/
#endif /* XAW_BC */

extern void XawTextDisplay(); /* w */
    /* Widget w; */

extern void XawTextSetSelectionArray(); /* w, sarray */
    /* Widget        w;		*/
    /* SelectionType *sarray;   */

extern void XawTextSetLastPos(); /* w, lastPos */
    /* Widget        w;		*/
    /* XawTextPosition lastPos;  */

extern void XawTextGetSelectionPos(); /* dpy, w, left, right */
    /* Widget        w;		*/
    /* XawTextPosition *left, *right;    */

extern void XawTextSetSource(); /* dpy, w, source, startPos */
    /* Widget         w;	    */
    /* XawTextSource   source;       */
    /* XawTextPosition startPos;     */

extern int XawTextReplace(); /* w, startPos, endPos, text */
    /* Widget        w;		*/
    /* XawTextPosition   startPos, endPos; */
    /* XawTextBlock      *text; */

extern XawTextPosition XawTextTopPosition(); /* w */
    /* Widget        w;		*/

extern void XawTextSetInsertionPoint(); /*  w, position */
    /* Widget        w;		*/
    /* XawTextPosition position; */

extern XawTextPosition XawTextGetInsertionPoint(); /* w */
    /* Widget        w;		*/

extern void XawTextUnsetSelection(); /* w */
    /* Widget        w;		*/

extern void XawTextChangeOptions(); /* w, options */
    /* Widget        w;		*/
    /* int    options; */

extern int XawTextGetOptions(); /* w */
    /* Widget        w;		*/

extern void XawTextSetSelection(); /* w, left, right */
    /* Widget        w;		*/
    /* XawTextPosition left, right; */

extern void XawTextInvalidate(); /* w, from, to */
    /* Widget        w;		*/
    /* XawTextPosition from, to; */

extern XawTextSource XawTextGetSource() ; /* w */
    /* Widget        w;		*/

/*
 * Stuff from AsciiSink
 */

extern XawTextSink XawAsciiSinkCreate(); /* parent, args, num_args */
    /* Widget parent;		*/
    /* ArgList args;		*/
    /* Cardinal num_args;	*/

extern void XawAsciiSinkDestroy(); /* sink */
    /* XawTextSink  sink */

/*
 * from DiskSrc
 */
extern XawTextSource XawDiskSourceCreate(); /* parent, args, num_args */
    /* Widget	parent;		*/
    /* ArgList	args;		*/
    /* Cardinal	num_args;	*/

extern void XawDiskSourceDestroy(); /* src */
    /* XawTextSource src;	*/

/*
 * from StringSrc
 */

extern XawTextSource XawStringSourceCreate(); /* parent, args, num_args */
    /* Widget parent;		*/
    /* ArgList args;		*/
    /* Cardinal num_args;	*/

extern void XawStringSourceDestroy(); /* src */
    /* XawTextSource src;	*/


#endif /* _XawText_h */
/* DON'T ADD STUFF AFTER THIS #endif */
