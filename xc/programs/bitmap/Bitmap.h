/*
 * $XConsortium: Bitmap.h,v 1.6 90/11/01 19:34:14 dave Exp $
 *
 * Copyright 1989 Massachusetts Institute of Technology
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
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Davor Matic, MIT X Consortium
 */


#ifndef _Bitmap_h
#define _Bitmap_h

/****************************************************************
 *
 * Bitmap widget
 *
 ****************************************************************/

#include <X11/Xaw/Simple.h>

/* Resources:

 Name		     Class		RepType		Default Value
 ----		     -----		-------		------------- 
 background	     Background		Pixel		XtDefaultBackground
 foreground          Foredround         Pixel           XtDefaultForeground
 highlight           Highlight          Pixel           XtDefaultForeground
 frame               Frame              Pixel           XtDefaultForeground
 border		     BorderColor	Pixel		XtDefaultForeground
 borderWidth	     BorderWidth	Dimension	1
 mappedWhenManaged   MappedWhenManaged	Boolean		True
 resize              Resize             Boolean         True
 sensitive	     Sensitive		Boolean		True
 width		     Width		Dimension	0
 height		     Height		Dimension	0
 bitmapWidth         BitmapWidth        Dimension       16
 bitmapHeight        BitmapHeight       Dimension       16
 squareSize          SquareSize         Dimension       20
 x		     Position		Position	320
 y		     Position		Position	320
 xHot                XHot               Position        NotSet
 yHot                YHot               Position        NotSet
 distance            Distance           Dimension       10
 grid                Grid               Boolean         True
 gridTolerance       GridTolerance      Dimension       5
 dashed              Dashed             Boolean         True
 stippled            Stippled           Boolean         True
 proportional        Proportional       Boolean         True
 axes                Axes               Boolean         True
 button1Action       Button1Action      Action          Set  
 button2Action       Button2Action      Action          Invert
 button3Action       Button3Action      Action          Clear
 button4Action       Button4Action      Action          Invert
 button5Action       Button5Action      Action          Invert
 filename            Filename           String          "scratch"
 basename            Basename           String          "dummy"
*/

/* define any special resource names here that are not in <X11/StringDefs.h> */

#define XtNdummy "dummy"
#define XtNscratch "scratch"
#define XtNbitmapResource "bitmapResource"
#define XtNstipple "stipple"
#define XtNstippled "stippled"
#define XtNdashes "dashes"
#define XtNdashed "dashed"
#define XtNgrid "grid"
#define XtNgridTolerance "gridTolerance"
#define XtNaxes "axes"
#define XtNbitmapSize "bitmapSize"
#define XtNbitmapWidth "bitmapWidth"
#define XtNbitmapHeight "bitmapHeight"
#define XtNsquareSize "squareSize"
#define XtNxHot "xHot"
#define XtNyHot "yHot"
#define XtNbutton1Action "button1Action"
#define XtNbutton2Action "button2Action"
#define XtNbutton3Action "button3Action"
#define XtNbutton4Action "button4Action"
#define XtNbutton5Action "button5Action"
#define XtNfilename "filename"
#define XtNbasename "basename"
#define XtNmouseForeground "mouseForeground"
#define XtNmouseBackground "mouseBackground"
#define XtNframe "frame"
#define XtNdistance "distance"
#define XtNproportional "proportional"

#define XtCBitmapResource "BitmapResource"
#define XtCHighlight "Highlight"
#define XtCStipple "Stipple"
#define XtCStippled "Stippled"
#define XtCDashes "Dashes"
#define XtCDashed "Dashed"
#define XtCGrid "Grid"
#define XtCGridTolerance "GridTolerance"
#define XtCAxes "Axes"
#define XtBitmapSize "BitmapSize"
#define XtCBitmapWidth "BitmapWidth"
#define XtCBitmapHeight "BitmapHeight"
#define XtCSquareSize "SquareSize"
#define XtCXHot "XHot"
#define XtCYHot "YHot"
#define XtCButton1Action "Button1Action"
#define XtCButton2Action "Button2Action"
#define XtCButton3Action "Button3Action"
#define XtCButton4Action "Button4Action"
#define XtCButton5Action "Button5Action"
#define XtCFilename "Filename"
#define XtCBasename "Basename"
#define XtCFrame "Frame"
#define XtCDistance "Distance"
#define XtCProportional "Proportional"

/* bitmap defines */

#define NotSet   -1
#define Clear     0
#define Set       1
#define Invert    2
#define Highlight 3
#define On        True
#define Off       False

#define MarkRequest "MarkRequest"
#define StoreRequest "StoreRequest"
#define RestoreRequest "RestoreRequest"
#define CopyRequest "CopyRequest"
#define MoveRequest "MoveRequest"
#define PointRequest "PointRequest"
#define LineRequest "LineRequest"
#define CurveRequest "CurveRequest"
#define RectangleRequest "RectangleRequest"
#define FilledRectangleRequest "FilledRectangleRequest"
#define CircleRequest "CircleRequest"
#define FilledCircleRequest "FilledCircleRequest"
#define FloodFillRequest "FloodFillRequest"
#define HotSpotRequest "HotSpotRequest"
#define ZoomInRequest "ZoomInRequest"
#define PasteRequest "PasteRequest"
#define ImmediateCopyRequest "ImmediateCopyRequest"
#define ImmediateMoveRequest "ImmediateMoveRequest"

/* bitmap exports */

extern Boolean BWEngageRequest();
extern Boolean BWTreminateRequest();

extern void BWClearAll();
extern void BWSetAll();
extern void BWInvertAll();
extern void BWUp();
extern void BWDown();
extern void BWLeft();
extern void BWRight();
extern void BWRotateRight();
extern void BWRotateLeft();
extern void BWSwitchGrid();
extern void BWGrid();
extern void BWSwitchDashed();
extern void BWDashed();
extern void BWSwitchAxes();
extern void BWAxes();
extern void BWDrawSquare();
extern void BWDrawLine();
extern void BWDrawRectangle();
extern void BWDrawFilledRectangle();
extern void BWDrawCircle();
extern void BWDrawFilledCircle();
extern void BWFloodFill();
extern void BWMark();
extern void BWSelect();
extern void BWUnmark();
extern void BWStore();
extern void BWStoreToBuffer();
extern void BWUndo();
extern void BWResize();
extern void BWClip();
extern void BWUnclip();
extern void BWGrabSelection();
extern void BWRequestSelection();
extern void BWSetChanged();
extern Boolean BWQueryChanged();
extern int  BWReadFile();
extern int  BWWriteFile();
extern String BWUnparseStatus();
extern String BWGetFilename();
extern String BWGetBasename();
extern void BWChangeBasename();
extern void BWRemoveAllRequests();
extern void BWClearHotSpot();
extern Boolean BWQueryMarked();
extern void BWFold();
extern void BWClear();
extern void BWSet();
extern void BWInvert();
extern void BWFlipHoriz();
extern void BWFlipVert();
extern void BWClearMarked();
extern Boolean BWAddRequest();
extern void BWChangeNotify();
extern Pixmap BWGetUnzoomedPixmap();
extern void BWClearChanged();
extern Boolean BWQueryStored();
extern Boolean BWQueryStippled();
extern void BWSwitchStippled();
extern void BWRedrawMark();
extern Boolean BWQueryAxes();
extern void BWHighlightAxes();
extern void CopyImageData();
extern void BWChangedFilename();
extern String BWGetFilepath();
extern void BWZoomOut();
extern void BWZoomMarked();
extern void BWRescale();
extern Boolean BWQueryZooming();
extern void BWRedrawGrid();
extern void BWRedrawSquares();
extern void BWRedrawHotSpot();
extern Boolean BWQueryGrid();
extern Boolean BWQueryDashed();
extern Boolean BWQueryProportional();
extern void BWSwitchProportional();
extern void BWDrawGrid();
extern void BWChangeFilename();
extern void Notify();
extern void TransferImageData();

typedef struct _BWRequestRec BWRequestRec;
typedef char *BWRequest;

/* declare specific BitmapWidget class and instance datatypes */

typedef struct _BitmapClassRec *BitmapWidgetClass;
typedef struct _BitmapRec      *BitmapWidget;
/* declare the class constant */

extern WidgetClass bitmapWidgetClass;

#endif /* _Bitmap_h */


