/*
 * $XConsortium: Requests.h,v 1.5 90/12/08 17:29:57 dmatic Exp $
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

typedef struct {
    Boolean  success;
    Position at_x, at_y;
    Position from_x, from_y,
             to_x, to_y;
    void   (*draw)();
    int      value;
    Time     btime;
    int      state;
} BWStatus;

void OnePointEngage();
void OnePointTerminate();
void OnePointTerminateTransparent();
void DragOnePointEngage();
void DragOnePointTerminate();
void TwoPointsEngage();
void TwoPointsTerminate();
void TwoPointsTerminateTransparent();
void TwoPointsTerminateTimed();
void DragTwoPointsEngage();
void DragTwoPointsTerminate();
void Interface();
void Paste();

void BWMark();
void BWUnmark();
void BWStore();
void BWDragMarked();
void BWDragStored();
void BWRestore();
void BWCopy();
void BWMove();
void BWDrawPoint();
void BWDrawLine();
void BWBlindLine();
void BWDrawRectangle();
void BWDrawFilledRectangle();
void BWDrawCircle();
void BWDrawFilledCircle();
void BWFloodFill();
void BWDrawHotSpot();
void BWChangeNotify();
void BWZoomIn();

static BWRequestRec requests[] = /* SUPPRESS 592 */
{
{MarkRequest, sizeof(BWStatus),
     TwoPointsEngage, (XtPointer) BWDrawRectangle,
     TwoPointsTerminateTimed, (XtPointer) BWSelect,
     NULL, (XtPointer) NULL},
{RestoreRequest, sizeof(BWStatus),
     OnePointEngage, (XtPointer) BWDragStored,
     OnePointTerminate, (XtPointer) BWRestore,
     NULL, (XtPointer) NULL},
{ImmediateCopyRequest, sizeof(BWStatus),
     OnePointEngage, (XtPointer) BWDragMarked,
     OnePointTerminate, (XtPointer) BWCopy,
     NULL, (XtPointer) NULL},
{ImmediateMoveRequest, sizeof(BWStatus),
     OnePointEngage, (XtPointer) BWDragMarked,
     OnePointTerminate, (XtPointer) BWMove,
     NULL, (XtPointer) NULL},
{CopyRequest, sizeof(BWStatus),
     DragOnePointEngage, (XtPointer) Paste,
     DragOnePointTerminate, (XtPointer) ImmediateCopyRequest,
     Interface, (XtPointer) BWUnmark},
{MoveRequest, sizeof(BWStatus),
     DragOnePointEngage, (XtPointer) Paste,
     DragOnePointTerminate, (XtPointer) ImmediateMoveRequest,
     Interface, (XtPointer) BWUnmark},
{PointRequest, sizeof(BWStatus),
     DragOnePointEngage, (XtPointer) BWDrawPoint,
     DragOnePointTerminate, (XtPointer) BWDrawPoint,
     NULL, (XtPointer) NULL},
{CurveRequest, sizeof(BWStatus),
     DragTwoPointsEngage, (XtPointer) BWBlindLine,
     DragTwoPointsTerminate, (XtPointer) BWBlindLine,
     NULL, (XtPointer) NULL},
{LineRequest, sizeof(BWStatus), 
     TwoPointsEngage, (XtPointer) BWDrawLine, 
     TwoPointsTerminate, (XtPointer) BWDrawLine,
     NULL, (XtPointer) NULL},
{RectangleRequest, sizeof(BWStatus), 
     TwoPointsEngage, (XtPointer) BWDrawRectangle,
     TwoPointsTerminate, (XtPointer) BWDrawRectangle,
     NULL, (XtPointer) NULL},
{FilledRectangleRequest, sizeof(BWStatus), 
     TwoPointsEngage, (XtPointer) BWDrawRectangle,
     TwoPointsTerminate, (XtPointer) BWDrawFilledRectangle,
     NULL, (XtPointer) NULL},
{CircleRequest, sizeof(BWStatus), 
     TwoPointsEngage, (XtPointer) BWDrawCircle,
     TwoPointsTerminate, (XtPointer) BWDrawCircle,
     NULL, (XtPointer) NULL},
{FilledCircleRequest, sizeof(BWStatus), 
     TwoPointsEngage, (XtPointer) BWDrawCircle, 
     TwoPointsTerminate, (XtPointer) BWDrawFilledCircle,
     NULL, (XtPointer) NULL},
{FloodFillRequest, sizeof(BWStatus),
     OnePointEngage, (XtPointer) NULL,
     OnePointTerminate, (XtPointer) BWFloodFill,
     NULL, (XtPointer) NULL},
{HotSpotRequest, sizeof(BWStatus),
     OnePointEngage, (XtPointer) BWDrawHotSpot,
     OnePointTerminate, (XtPointer) BWDrawHotSpot,
     NULL, (XtPointer) NULL},
{ZoomInRequest, sizeof(BWStatus),
     TwoPointsEngage, (XtPointer) BWDrawRectangle,
     TwoPointsTerminate, (XtPointer) BWZoomIn,
     NULL, (XtPointer) NULL},
};

