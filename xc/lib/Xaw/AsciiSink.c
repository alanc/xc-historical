#if (!defined(lint) && !defined(SABER))
static char Xrcsid[] = "$XConsortium: AsciiSink.c,v 1.34 89/08/17 16:44:55 kit Exp $";
#endif /* lint && SABER */

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

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/TextP.h>

#ifdef GETLASTPOS
#undef GETLASTPOS		/* We will use our own GETLASTPOS. */
#endif

#define GETLASTPOS (*source->Scan)(source, 0, XawstAll, XawsdRight, 1, TRUE)

/* Private Ascii TextSink Definitions */

static unsigned bufferSize = 200;

typedef struct _AsciiSinkData {
      /* public resources */
    Pixel foreground;
    XFontStruct *font;
    Boolean echo;
    Boolean display_nonprinting;
      /* private state */
    GC normgc, invgc, xorgc;
    int em;
    Pixmap insertCursorOn;
    XawTextInsertState laststate;
    int tab_count;
    short cursor_x, cursor_y;	/* Cursor Location. */
    Position *tabs;
} AsciiSinkData, *AsciiSinkPtr;

static char *buf = NULL;

static XtResource SinkResources[] = {
    {XtNfont, XtCFont, XtRFontStruct, sizeof (XFontStruct *),
        XtOffset(AsciiSinkPtr, font), XtRString, "Fixed"},
    {XtNforeground, XtCForeground, XtRPixel, sizeof (int),
        XtOffset(AsciiSinkPtr, foreground), XtRString, "XtDefaultForeground"}, 
    {XtNecho, XtCOutput, XtRBoolean, sizeof(Boolean),
	XtOffset(AsciiSinkPtr, echo), XtRImmediate, (caddr_t)True},
    {XtNdisplayNonPrinting, XtCOutput, XtRBoolean, sizeof(Boolean),
	XtOffset(AsciiSinkPtr, display_nonprinting), XtRImmediate,
	(caddr_t)True},
};

/* Utilities */

static int CharWidth (w, x, c)
  Widget w;
  int x;
  char c;
{
    AsciiSinkData *data = (AsciiSinkData*) ((TextWidget)w)->text.sink->data;
    int     width, nonPrinting;
    XFontStruct *font = data->font;

    if ( c == LF ) return(0);

    if (c == TAB) {
	int i;
	Position *tab;
	x -= ((TextWidget) w)->text.margin.left; /* Adjust for Left Margin. */

	if (x >= w->core.width) return 0;
	for (i = 0, tab = data->tabs ; i < data->tab_count ; i++, tab++) {
	    if (x < *tab) {
		if (*tab < w->core.width)
		    return *tab - x;
		else
		    return 0;
	    }
	}
	return 0;
    }

    if ( (nonPrinting = (c < SP)) ) {         /* Yes, This is right. */
	if (data->display_nonprinting)
	    c += '@';
	else {
	    c = SP;
	    nonPrinting = False;
	}
    }

    if (font->per_char &&
	    (c >= font->min_char_or_byte2 && c <= font->max_char_or_byte2))
	width = font->per_char[c - font->min_char_or_byte2].width;
    else
	width = font->min_bounds.width;

    if (nonPrinting)
	width += CharWidth(w, x, '^');

    return width;
}

/*	Function Name: PaintText
 *	Description: Actually paints the text into the windoe.
 *	Arguments: w - the text widget.
 *                 gc - gc to paint text with.
 *                 x, y - location to paint the text.
 *                 buf, len - buffer and length of text to paint.
 *	Returns: the width of the text painted, or 0.
 *
 * NOTE:  If this string attempts to paint past the end of the window
 *        then this function will return zero.
 */

static Dimension
PaintText(w, gc, x, y, buf, len)
Widget w;
GC gc;
Position x, y;
char * buf;
int len;
{
    TextWidget ctx = (TextWidget) w;
    XawTextSink sink = ctx->text.sink;
    AsciiSinkData *data = (AsciiSinkData *) sink->data;
    Position max_x;
    Dimension width = XTextWidth(data->font, buf, len); 
    max_x = (Position) ctx->core.width;

    if ( ((int) width) <= -x)	           /* Don't draw if we can't see it. */
      return(width);

    XDrawImageString(XtDisplay(w), XtWindow(w), gc,(int) x, (int) y, buf, len);
    if ( (((Position) width + x) > max_x) && (ctx->text.margin.right != 0) ) {
	x = ctx->core.width - ctx->text.margin.right;
	width = ctx->text.margin.right;
	XFillRectangle(XtDisplay(w), XtWindow(w), data->normgc, (int) x,
		       (int) y - data->font->ascent, (unsigned int) width,
		       (unsigned int) (data->font->ascent +
				       data->font->descent));
	return(0);
    }
    return(width);
}

/* Sink Object Functions */

static /*void*/ AsciiDisplayText (w, x, y, pos1, pos2, highlight)
  Widget w;
  Position x, y;
  int highlight;
  XawTextPosition pos1, pos2;
{
    TextWidget ctx = (TextWidget) w;
    XawTextSink sink = ctx->text.sink;
    AsciiSinkData *data = (AsciiSinkData *) sink->data;
    XawTextSource source = ctx->text.source;

    int j, k;
    XawTextBlock blk;
    GC gc = highlight ? data->invgc : data->normgc;
    GC invgc = highlight ? data->normgc : data->invgc;

    if (!data->echo) return;

    y += data->font->ascent;
    for ( j = 0 ; pos1 < pos2 ; ) {
	pos1 = (*source->Read)(source, pos1, &blk, pos2 - pos1);
	for (k = 0; k < blk.length; k++) {
	    if (j >= bufferSize - 5) {
		bufferSize *= 2;
		buf = XtRealloc(buf, bufferSize);
	    }
	    buf[j] = blk.ptr[k];
	    if (buf[j] == LF)	/* line feeds ('\n') are not printed. */
	        continue;

	    else if (buf[j] == '\t') {
	        Position temp = 0;
		Dimension width;

	        if ((j != 0) && ((temp = PaintText(w, gc, x, y, buf, j)) == 0))
		  return;

	        x += temp;
		width = CharWidth(w, x, '\t');
		XFillRectangle(XtDisplay(w), XtWindow(w), invgc, (int) x,
			       (int) y - data->font->ascent,
			       (unsigned int) width,
			       (unsigned int) (data->font->ascent +
					       data->font->descent));
		x += width;
		j = -1;
	    }
	    else if (buf[j] < ' ') {
	        if (data->display_nonprinting) {
		    buf[j + 1] = buf[j] + '@';
		    buf[j] = '^';
		    j++;
		}
		else
		    buf[j] = ' ';
	    }
	    j++;
	}
    }
    if (j > 0)
        (void) PaintText(w, gc, x, y, buf, j);
}

#define insertCursor_width 6
#define insertCursor_height 3
static char insertCursor_bits[] = {0x0c, 0x1e, 0x33};

static Pixmap CreateInsertCursor(s)
Screen *s;
{
    return (XCreateBitmapFromData (DisplayOfScreen(s), RootWindowOfScreen(s),
        insertCursor_bits, insertCursor_width, insertCursor_height));
}

/*	Function Name: GetCursorBounds
 *	Description: Returns the size and location of the cursor.
 *	Arguments: w - the text widget.
 * RETURNED        rect - an X rectangle to return the cursor bounds in.
 *	Returns: none.
 */

static void
GetCursorBounds(w, rect)
Widget w;
XRectangle * rect;
{
    XawTextSink sink = ((TextWidget)w)->text.sink;
    AsciiSinkData *data = (AsciiSinkData *) sink->data;
  
    rect->width = (unsigned short) insertCursor_width;
    rect->height = (unsigned short) insertCursor_height;
    rect->x = data->cursor_x - (short) (rect->width / 2);
    rect->y = data->cursor_y - (short) rect->height;
}

/*
 * The following procedure manages the "insert" cursor.
 */

static AsciiInsertCursor (w, x, y, state)
  Widget w;
  Position x, y;
  XawTextInsertState state;
{
    XawTextSink sink = ((TextWidget)w)->text.sink;
    AsciiSinkData *data = (AsciiSinkData *) sink->data;
    XRectangle rect;

    data->cursor_x = x;
    data->cursor_y = y;

    GetCursorBounds(w, &rect);
    if (state != data->laststate && XtIsRealized(w)) 
        XCopyPlane(XtDisplay(w),
		   data->insertCursorOn, XtWindow(w), data->xorgc,
		   0, 0, (unsigned int) rect.width, (unsigned int) rect.height,
		   (int) rect.x, (int) rect.y, 1);
    data->laststate = state;
}

/*
 * Clear the passed region to the background color.
 */

static AsciiClearToBackground (w, x, y, width, height)
  Widget w;
  Position x, y;
  Dimension width, height;
{

/* 
 * Don't clear in height or width are zero.
 * XClearArea() has special semantic for these values.
 */

    if ( (height == 0) || (width == 0) ) return;
    XClearArea(XtDisplay(w), XtWindow(w), x, y, width, height, False);
}

/*
 * Given two positions, find the distance between them.
 */

static AsciiFindDistance (w, fromPos, fromx, toPos,
			  resWidth, resPos, resHeight)
  Widget w;
  XawTextPosition fromPos;	/* First position. */
  int fromx;			/* Horizontal location of first position. */
  XawTextPosition toPos;		/* Second position. */
  int *resWidth;		/* Distance between fromPos and resPos. */
  XawTextPosition *resPos;	/* Actual second position used. */
  int *resHeight;		/* Height required. */
{
    XawTextSink sink = ((TextWidget)w)->text.sink;
    XawTextSource source = ((TextWidget)w)->text.source;

    AsciiSinkData *data;
    register    XawTextPosition index, lastPos;
    register char   c;
    XawTextBlock blk;

    data = (AsciiSinkData *) sink->data;
    /* we may not need this */
    lastPos = GETLASTPOS;
    (*source->Read)(source, fromPos, &blk, toPos - fromPos);
    *resWidth = 0;
    for (index = fromPos; index != toPos && index < lastPos; index++) {
	if (index - blk.firstPos >= blk.length)
	    (*source->Read)(source, index, &blk, toPos - fromPos);
	c = blk.ptr[index - blk.firstPos];
	if (c == LF) {
	    *resWidth += CharWidth(w, fromx + *resWidth, SP);
	    index++;
	    break;
	}
	*resWidth += CharWidth(w, fromx + *resWidth, c);
    }
    *resPos = index;
    *resHeight = data->font->ascent + data->font->descent;
}


static AsciiFindPosition(w, fromPos, fromx, width, stopAtWordBreak, 
			 resPos, resWidth, resHeight)
  Widget w;
  XawTextPosition fromPos; 	/* Starting position. */
  int fromx;			/* Horizontal location of starting position.*/
  int width;			/* Desired width. */
  int stopAtWordBreak;		/* Whether the resulting position should be at
				   a word break. */
  XawTextPosition *resPos;	/* Resulting position. */
  int *resWidth;		/* Actual width used. */
  int *resHeight;		/* Height required. */
{
    XawTextSink sink = ((TextWidget)w)->text.sink;
    XawTextSource source = ((TextWidget)w)->text.source;
    AsciiSinkData *data;
    XawTextPosition lastPos, index, whiteSpacePosition;
    int     lastWidth, whiteSpaceWidth;
    Boolean whiteSpaceSeen;
    char    c;
    XawTextBlock blk;
    data = (AsciiSinkData *) sink->data;
    lastPos = GETLASTPOS;

    (*source->Read)(source, fromPos, &blk, bufferSize);
    *resWidth = 0;
    whiteSpaceSeen = FALSE;
    c = 0;
    for (index = fromPos; *resWidth <= width && index < lastPos; index++) {
	lastWidth = *resWidth;
	if (index - blk.firstPos >= blk.length)
	    (*source->Read)(source, index, &blk, bufferSize);
	c = blk.ptr[index - blk.firstPos];
	*resWidth += CharWidth(w, fromx + *resWidth, c);

	if ((c == SP || c == TAB) && *resWidth <= width) {
	    whiteSpaceSeen = TRUE;
	    whiteSpacePosition = index;
	    whiteSpaceWidth = *resWidth;
	}
	if (c == LF) {
	    index++;
	    break;
	}
    }
    if (*resWidth > width && index > fromPos) {
	*resWidth = lastWidth;
	index--;
	if (stopAtWordBreak && whiteSpaceSeen) {
	    index = whiteSpacePosition + 1;
	    *resWidth = whiteSpaceWidth;
	}
    }
    if (index == lastPos && c != LF) index = lastPos + 1;
    *resPos = index;
    *resHeight = data->font->ascent + data->font->descent;
}

/*	Function Name: AsciiResolveToPosition
 *	Description: Resolves an x location into a position.
 *	Arguments: w - The text widget.
 *                 pos - The position at the reference location.
 *                 ref_x - A reference location for the position "pos".
 *                 x - The x location that we need the position for.
 *	Returns: The position corrosponding to the X location.
 */

static XawTextPosition
AsciiResolveToPosition (w, pos, ref_x, x)
Widget w;
XawTextPosition pos;
Position ref_x, x;
{
  int resWidth, resHeight, width = x - ref_x;
  XawTextPosition ret_pos;

  AsciiFindPosition(w, pos, ref_x, width,
		    FALSE, &ret_pos, &resWidth, &resHeight);

  return (ret_pos);
}


static int AsciiMaxLinesForHeight (w, height)
  Widget w;
  Dimension height;
{
    AsciiSinkData *data;
    XawTextSink sink = ((TextWidget)w)->text.sink;

    data = (AsciiSinkData *) sink->data;
    return(height / (data->font->ascent + data->font->descent));
}


static int AsciiMaxHeightForLines (w, lines)
  Widget w;
  int lines;
{
    AsciiSinkData *data;
    XawTextSink sink = ((TextWidget)w)->text.sink;

    data = (AsciiSinkData *) sink->data;
    return(lines * (data->font->ascent + data->font->descent));
}

/* ARGSUSED */
static void AsciiSetTabs (w, offset, tab_count, tabs)
  Widget w;			/* for context */
  Position offset;		/* UNUSED.*/
  int tab_count;		/* count of entries in tabs */
  Position *tabs;		/* list of character positions */
{
    AsciiSinkData *data = (AsciiSinkData*)((TextWidget)w)->text.sink->data;
    int i;

    if (tab_count > data->tab_count) {
	data->tabs = (Position*)XtRealloc(data->tabs,
				    (unsigned)tab_count * sizeof(Position));
    }
    
    for (i=0; i < tab_count; i++) data->tabs[i] = tabs[i] * data->em;
    data->tab_count = tab_count;
}

/***** Public routines *****/

XawTextSink XawAsciiSinkCreate (parent, args, num_args)
    Widget	parent;
    ArgList 	args;
    Cardinal 	num_args;
{
    XawTextSink sink;
    AsciiSinkData *data;
    unsigned long valuemask = (GCFont | GCGraphicsExposures |
			       GCForeground | GCBackground | GCFunction);
    XGCValues values;
    long wid;
    XFontStruct *font;
    Atom XA_FIGURE_WIDTH;

    if (!buf) buf = XtMalloc(bufferSize);

    sink = XtNew(XawTextSinkRec);
    sink->Display = AsciiDisplayText;
    sink->InsertCursor = AsciiInsertCursor;
    sink->ClearToBackground = AsciiClearToBackground;
    sink->FindPosition = AsciiFindPosition;
    sink->FindDistance = AsciiFindDistance;
    sink->Resolve = AsciiResolveToPosition;
    sink->MaxLines = AsciiMaxLinesForHeight;
    sink->MaxHeight = AsciiMaxHeightForLines;
    sink->SetTabs = AsciiSetTabs;
    sink->GetCursorBounds = GetCursorBounds;
    data = XtNew(AsciiSinkData);
    sink->data = (caddr_t)data;

    XtGetSubresources (parent, (caddr_t)data, XtNtextSink, XtCTextSink, 
		       SinkResources, XtNumber(SinkResources),
		       args, num_args);

    font = data->font;
    values.function = GXcopy;
    values.font = font->fid;
    values.graphics_exposures = (Bool) FALSE;
    values.foreground = data->foreground;
    values.background = parent->core.background_pixel;
    data->normgc = XtGetGC(parent, valuemask, &values);
    values.foreground = parent->core.background_pixel;
    values.background = data->foreground;
    data->invgc = XtGetGC(parent, valuemask, &values);
    values.function = GXxor;
    values.foreground = data->foreground ^ parent->core.background_pixel;
    values.background = 0;
    data->xorgc = XtGetGC(parent, valuemask, &values);

    XA_FIGURE_WIDTH = XInternAtom(XtDisplay(parent), "FIGURE_WIDTH", FALSE);
    wid = -1;
    if ( (XA_FIGURE_WIDTH != NULL) && 
	((!XGetFontProperty(font, XA_FIGURE_WIDTH, &wid)) || wid <= 0) ) {
        if (font->per_char && font->min_char_or_byte2 <= '$' &&
	    		      font->max_char_or_byte2 >= '$')
	    wid = font->per_char['$' - font->min_char_or_byte2].width;
	else
	    wid = font->max_bounds.width;
    }
    if (wid <= 0)
	data->em = 1;
    else
	data->em = wid;

    data->font = font;
    data->insertCursorOn = CreateInsertCursor(XtScreen(parent));
    data->laststate = XawisOff;
    data->tab_count = 0;
    data->tabs = NULL;
    return sink;
}

void XawAsciiSinkDestroy (sink)
    XawTextSink sink;
{
    AsciiSinkData *data;

    data = (AsciiSinkData *) sink->data;
    XtFree((char *) data->tabs);
    XtFree((char *) data);
    XtFree((char *) sink);
}
