#ifndef lint
static char Xrcsid[] = "$XConsortium: Text.c,v 1.88 89/06/19 17:51:09 jim Exp $";
#endif


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

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xatom.h>

#include <X11/Xmu/Xmu.h>
#include <X11/Xmu/Misc.h>

#include <X11/Xaw/Cardinals.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Dialog.h>
#include <X11/Xaw/Scroll.h>
#include <X11/Xaw/TextP.h>

Atom FMT8BIT = NULL;

extern void bcopy();
extern int errno, sys_nerr;
extern char* sys_errlist[];

#define abs(x)	(((x) < 0) ? (-(x)) : (x))
#define GETLASTPOS  (*ctx->text.source->Scan) (ctx->text.source, 0, XawstAll, XawsdRight, 1, TRUE)

#define zeroPosition ((XawTextPosition) 0)
#define BIGNUM ((Dimension)32023)

static void BuildLineTable ();
static void ScrollUpDownProc();
static void ThumbProc();
static int LineAndXYForPosition(), _XawTextSetNewSelection();
static ClearWindow();

/****************************************************************
 *
 * Full class record constant
 *
 ****************************************************************/

static XawTextSelectType defaultSelectTypes[] = {
	XawselectPosition,
	XawselectWord,
	XawselectLine,
	XawselectParagraph,
	XawselectAll,
	XawselectNull
};

static caddr_t defaultSelectTypesPtr = (caddr_t)defaultSelectTypes;
extern char *_XawDefaultTextTranslations1, *_XawDefaultTextTranslations2,
  *_XawDefaultTextTranslations3;
static Dimension defWidth = 100;
static Dimension defHeight = DEFAULT_TEXT_HEIGHT;

#define offset(field) XtOffset(TextWidget, field)
static XtResource resources[] = {
    {XtNwidth, XtCWidth, XtRDimension, sizeof(Dimension),
        offset(core.width), XtRDimension, (caddr_t)&defWidth},
    {XtNcursor, XtCCursor, XtRCursor, sizeof(Cursor),
	offset(simple.cursor), XtRString, "xterm"},
    {XtNheight, XtCHeight, XtRDimension, sizeof(Dimension),
        offset(core.height), XtRDimension, (caddr_t)&defHeight},
    {XtNtextOptions, XtCTextOptions, XtRInt, sizeof (int),
        offset(text.options), XtRImmediate, (caddr_t)0},
    {XtNdialogHOffset, XtCMargin, XtRInt, sizeof(int),
	 offset(text.dialog_horiz_offset), XtRImmediate, (caddr_t)0},
    {XtNdialogVOffset, XtCMargin, XtRInt, sizeof(int),
	 offset(text.dialog_vert_offset), XtRImmediate, (caddr_t)0},
    {XtNdisplayPosition, XtCTextPosition, XtRInt,
	 sizeof (XawTextPosition), offset(text.lt.top), XtRImmediate, (caddr_t)0},
    {XtNinsertPosition, XtCTextPosition, XtRInt,
        sizeof(XawTextPosition), offset(text.insertPos), XtRImmediate, (caddr_t)0},
    {XtNleftMargin, XtCMargin, XtRDimension, sizeof (Dimension),
        offset(text.client_leftmargin), XtRImmediate, (caddr_t)2},
    {XtNselectTypes, XtCSelectTypes, XtRPointer,
        sizeof(XawTextSelectType*), offset(text.sarray),
	XtRPointer, (caddr_t)&defaultSelectTypesPtr},
    {XtNtextSource, XtCTextSource, XtRPointer, sizeof (caddr_t),
         offset(text.source), XtRPointer, NULL},
    {XtNtextSink, XtCTextSink, XtRPointer, sizeof (caddr_t),
         offset(text.sink), XtRPointer, NULL},
    {XtNselection, XtCSelection, XtRPointer, sizeof(caddr_t),
	 offset(text.s), XtRPointer, NULL},
    {XtNdisplayCaret, XtCOutput, XtRBoolean, sizeof(Boolean),
	offset(text.display_caret), XtRImmediate, (caddr_t)True},
};
#undef offset

  
#define done(address, type) \
        { toVal->size = sizeof(type); toVal->addr = (caddr_t) address; }

/* EditType enumeration constants */

static  XrmQuark  XtQTextRead;
static  XrmQuark  XtQTextAppend;
static  XrmQuark  XtQTextEdit;

/* ARGSUSED */
static void CvtStringToEditMode(args, num_args, fromVal, toVal)
    XrmValuePtr args;		/* unused */
    Cardinal	*num_args;	/* unused */
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
    static XawTextEditType editType;
    XrmQuark    q;
    char        lowerName[1000];

    XmuCopyISOLatin1Lowered (lowerName, (char *)fromVal->addr);
    q = XrmStringToQuark(lowerName);
    if (q == XtQTextRead ) {
        editType = XawtextRead;
        done(&editType, XawTextEditType);
        return;
    }
    if (q == XtQTextAppend) {
        editType = XawtextAppend;
        done(&editType, XawTextEditType);
        return;
    }
    if (q == XtQTextEdit) {
        editType = XawtextEdit;
        done(&editType, XawTextEditType);
        return;
    }
    toVal->size = 0;
    toVal->addr = NULL;
};

static void 
ClassInitialize()
{
    int len1 = strlen (_XawDefaultTextTranslations1);
    int len2 = strlen (_XawDefaultTextTranslations2);
    int len3 = strlen (_XawDefaultTextTranslations3);
    char *buf = XtMalloc (len1 + len2 + len3 + 1);
    char *cp = buf;

    strcpy (cp, _XawDefaultTextTranslations1); cp += len1;
    strcpy (cp, _XawDefaultTextTranslations2); cp += len2;
    strcpy (cp, _XawDefaultTextTranslations3);
    textWidgetClass->core_class.tm_table = buf;

    XtQTextRead   = XrmStringToQuark(XtEtextRead);
    XtQTextAppend = XrmStringToQuark(XtEtextAppend);
    XtQTextEdit   = XrmStringToQuark(XtEtextEdit);

    XtAddConverter(XtRString, XtREditMode, CvtStringToEditMode, NULL, 0);
}

static void CreateScrollbar(w)
    TextWidget w;
{
    Arg args[1];
    Dimension bw;
    Widget sbar;

    XtSetArg(args[0], XtNheight, w->core.height);
    w->text.sbar = sbar =
	    XtCreateWidget("scrollbar", scrollbarWidgetClass, w, args, ONE);
    XtAddCallback( sbar, XtNscrollProc, ScrollUpDownProc, (caddr_t)w );
    XtAddCallback( sbar, XtNjumpProc, ThumbProc, (caddr_t)w );
    w->text.leftmargin += sbar->core.width + (bw = sbar->core.border_width);
    XtMoveWidget( sbar, -(Position)bw, -(Position)bw );
}

/* ARGSUSED */
static void Initialize(request, new)
 Widget request, new;
{
    TextWidget ctx = (TextWidget) new;

    if (!FMT8BIT)
        FMT8BIT = XInternAtom(XtDisplay(new), "FMT8BIT", False);

    if (ctx->core.height == DEFAULT_TEXT_HEIGHT) {
        ctx->core.height = (2*yMargin) + 2;
        if (ctx->text.sink)
	    ctx->core.height += (*ctx->text.sink->MaxHeight)(new, 1);
    }

    ctx->text.lt.lines = 0;
    ctx->text.lt.info = NULL;
    ctx->text.s.left = ctx->text.s.right = 0;
    ctx->text.s.type = XawselectPosition;
    ctx->text.s.selections = NULL;
    ctx->text.s.atom_count = ctx->text.s.array_size = 0;
    ctx->text.sbar = ctx->text.outer = NULL;
    ctx->text.lasttime = 0; /* ||| correct? */
    ctx->text.time = 0; /* ||| correct? */
    ctx->text.showposition = TRUE;
    ctx->text.lastPos = ctx->text.source ? GETLASTPOS : 0;
    ctx->text.dialog = NULL;
    ctx->text.updateFrom = (XawTextPosition *) XtMalloc((unsigned)1);
    ctx->text.updateTo = (XawTextPosition *) XtMalloc((unsigned)1);
    ctx->text.numranges = ctx->text.maxranges = 0;
    ctx->text.gc = DefaultGCOfScreen(XtScreen(ctx));
    ctx->text.hasfocus = FALSE;
    ctx->text.leftmargin = ctx->text.client_leftmargin;
    ctx->text.update_disabled = False;
    ctx->text.old_insert = -1;

    if (ctx->text.options & scrollVertical)
	CreateScrollbar(ctx);
}

void ForceBuildLineTable();

static void Realize( w, valueMask, attributes )
   Widget w;
   Mask *valueMask;
   XSetWindowAttributes *attributes;
{
   TextWidget ctx = (TextWidget)w;

   *valueMask |= CWBitGravity;
   attributes->bit_gravity =
       (ctx->text.options & wordBreak) ? ForgetGravity : NorthWestGravity;

   (*textClassRec.core_class.superclass->core_class.realize)
       (w, valueMask, attributes);

   if (ctx->text.sbar) {
       XtRealizeWidget(ctx->text.sbar);
       XtMapWidget(ctx->text.sbar);
   }
   ForceBuildLineTable(ctx);
}


static /*void*/ _CreateCutBuffers(d)
    Display *d;
{
    static struct _DisplayRec {
	struct _DisplayRec *next;
	Display *dpy;
    } *dpy_list = NULL;
    struct _DisplayRec *dpy_ptr;

    for (dpy_ptr = dpy_list; dpy_ptr != NULL; dpy_ptr = dpy_ptr->next) {
	if (dpy_ptr->dpy == d) return;
    }

    dpy_ptr = XtNew(struct _DisplayRec);
    dpy_ptr->next = dpy_list;
    dpy_ptr->dpy = d;
    dpy_list = dpy_ptr;

#define Create(buffer) \
    XChangeProperty(d, RootWindow(d, 0), buffer, XA_STRING, 8, \
		    PropModeAppend, NULL, 0 );

    Create( XA_CUT_BUFFER0 );
    Create( XA_CUT_BUFFER1 );
    Create( XA_CUT_BUFFER2 );
    Create( XA_CUT_BUFFER3 );
    Create( XA_CUT_BUFFER4 );
    Create( XA_CUT_BUFFER5 );
    Create( XA_CUT_BUFFER6 );
    Create( XA_CUT_BUFFER7 );

#undef Create
}

/* Utility routines for support of Text */


/*
 * Procedure to manage insert cursor visibility for editable text.  It uses
 * the value of ctx->insertPos and an implicit argument. In the event that
 * position is immediately preceded by an eol graphic, then the insert cursor
 * is displayed at the beginning of the next line.
*/
static void InsertCursor (w, state)
  Widget w;
  XawTextInsertState state;
{
    TextWidget ctx = (TextWidget)w;
    Position x, y;
    int dy, line, visible;
    XawTextBlock text;

    if (ctx->text.lt.lines < 1) return;
    visible = LineAndXYForPosition(ctx, ctx->text.insertPos, &line, &x, &y);
    if (line < ctx->text.lt.lines)
	dy = (ctx->text.lt.info[line + 1].y - ctx->text.lt.info[line].y) + 1;
    else
	dy = (ctx->text.lt.info[line].y - ctx->text.lt.info[line - 1].y) + 1;

    /** If the insert position is just after eol then put it on next line **/
    if (x > ctx->text.leftmargin &&
	ctx->text.insertPos > 0 &&
	ctx->text.insertPos >= ctx->text.lastPos) {
	   /* reading the source is bogus and this code should use scan */
	   (*ctx->text.source->Read) (ctx->text.source, ctx->text.insertPos - 1, &text, 1);
	   if (text.ptr[0] == '\n') {
	       x = ctx->text.leftmargin;
	       y += dy;
	   }
    }
    y += dy;
    if (visible && ctx->text.display_caret)
	(*ctx->text.sink->InsertCursor)(w, x, y, state);
    ctx->text.ev_x = x;
    ctx->text.ev_y = y;
}


/*
 * Procedure to register a span of text that is no longer valid on the display
 * It is used to avoid a number of small, and potentially overlapping, screen
 * updates. [note: this is really a private procedure but is used in
 * multiple modules].
*/
_XawTextNeedsUpdating(ctx, left, right)
  TextWidget ctx;
  XawTextPosition left, right;
{
    int     i;
    if (left < right) {
	for (i = 0; i < ctx->text.numranges; i++) {
	    if (left <= ctx->text.updateTo[i] && right >= ctx->text.updateFrom[i]) {
		ctx->text.updateFrom[i] = Min(left, ctx->text.updateFrom[i]);
		ctx->text.updateTo[i] = Max(right, ctx->text.updateTo[i]);
		return;
	    }
	}
	ctx->text.numranges++;
	if (ctx->text.numranges > ctx->text.maxranges) {
	    ctx->text.maxranges = ctx->text.numranges;
	    i = ctx->text.maxranges * sizeof(XawTextPosition);
	    ctx->text.updateFrom = (XawTextPosition *) 
   	        XtRealloc((char *)ctx->text.updateFrom, (unsigned) i);
	    ctx->text.updateTo = (XawTextPosition *) 
		XtRealloc((char *)ctx->text.updateTo, (unsigned) i);
	}
	ctx->text.updateFrom[ctx->text.numranges - 1] = left;
	ctx->text.updateTo[ctx->text.numranges - 1] = right;
    }
}


/*
 * Procedure to read a span of text in Ascii form. This is purely a hack and
 * we probably need to add a function to sources to provide this functionality.
 * [note: this is really a private procedure but is used in multiple modules].
*/
char *_XawTextGetText(ctx, left, right)
  TextWidget ctx;
  XawTextPosition left, right;
{
    char   *result, *tempResult;
    int length, resultLength;
    XawTextBlock text;
    XawTextPosition end, nend;
    
    resultLength = right - left + 10;	/* Bogus? %%% */
    result = (char *)XtMalloc((unsigned) resultLength);
    end = (*ctx->text.source->Read)(ctx->text.source, left, &text, right - left);
    (void) strncpy(result, text.ptr, text.length);
    length = text.length;
    while (end < right) {
        nend = (*ctx->text.source->Read)(ctx->text.source, end, &text, right - end);
	tempResult = result + length;
        (void) strncpy(tempResult, text.ptr, text.length);
	length += text.length;
        end = nend;
    }
    result[length] = 0;
    return result;
}



/* 
 * This routine maps an x and y position in a window that is displaying text
 * into the corresponding position in the source.
 */
static XawTextPosition PositionForXY (ctx, x, y)
  TextWidget ctx;
  Position x,y;
{
 /* it is illegal to call this routine unless there is a valid line table! */
    int     width, fromx, line;
    XawTextPosition position, resultstart, resultend;

    /*** figure out what line it is on ***/
    if (ctx->text.lt.lines == 0) return 0;

    for (line = 0; line < ctx->text.lt.lines - 1; line++) {
	if (y <= ctx->text.lt.info[line + 1].y)
	    break;
    }
    position = ctx->text.lt.info[line].position;
    if (position >= ctx->text.lastPos)
	return ctx->text.lastPos;
    fromx = ctx->text.lt.info[line].x;	/* starting x in line */
    width = x - fromx;			/* num of pix from starting of line */
    (*ctx->text.sink->Resolve) (ctx, position, fromx, width,
	    &resultstart, &resultend);
    if (resultstart >= ctx->text.lt.info[line + 1].position)
	resultstart = (*ctx->text.source->Scan)(ctx->text.source,
		ctx->text.lt.info[line + 1].position, XawstPositions, XawsdLeft, 1, TRUE);
    return resultstart;
}

/*
 * This routine maps a source position in to the corresponding line number
 * of the text that is displayed in the window.
*/
static int LineForPosition (ctx, position)
  TextWidget ctx;
  XawTextPosition position;
  /* it is illegal to call this routine unless there is a valid line table!*/
{
    int     line;

    if (position <= ctx->text.lt.info[0].position)
	return 0;
    for (line = 0; line < ctx->text.lt.lines; line++)
	if (position < ctx->text.lt.info[line + 1].position)
	    break;
    return line;
}

/*
 * This routine maps a source position into the corresponding line number
 * and the x, y coordinates of the text that is displayed in the window.
*/
static int LineAndXYForPosition (ctx, pos, line, x, y)
  TextWidget ctx;
  XawTextPosition pos;
  int *line;
  Position *x, *y;
  /* it is illegal to call this routine unless there is a valid line table!*/
{
    XawTextPosition linePos, endPos;
    int     visible, realW, realH;

    *line = 0;
    *x = ctx->text.leftmargin;
    *y = yMargin;
    visible = IsPositionVisible(ctx, pos);
    if (visible) {
	*line = LineForPosition(ctx, pos);
	*y = ctx->text.lt.info[*line].y;
	*x = ctx->text.lt.info[*line].x;
	linePos = ctx->text.lt.info[*line].position;
	(*ctx->text.sink->FindDistance)((Widget)ctx, linePos,
                                     *x, pos, &realW, &endPos, &realH);
	*x = *x + realW;
    }
    return visible;
}

/*
 * This routine builds a line table. It does this by starting at the
 * specified position and measuring text to determine the staring position
 * of each line to be displayed. It also determines and saves in the
 * linetable all the required metrics for displaying a given line (e.g.
 * x offset, y offset, line length, etc.).
*/
static void BuildLineTable (ctx, position)
  TextWidget ctx;
  XawTextPosition position;
{
    Position x, y;
    int width, realW, realH;
    int line, lines;
    XawTextPosition startPos, endPos;
    Boolean     rebuild;

    rebuild = (Boolean) (position != ctx->text.lt.top);
    lines = (*ctx->text.sink->MaxLines)((Widget)ctx, ctx->core.height);
    if (ctx->text.lt.info != NULL && lines != ctx->text.lt.lines) {
	XtFree((char *) ctx->text.lt.info);
	ctx->text.lt.info = NULL;
    }
    if (ctx->text.lt.info == NULL) {
	ctx->text.lt.info = (XawTextLineTableEntry *)
	    XtCalloc((unsigned)lines + 1, (unsigned)sizeof(XawTextLineTableEntry));
	rebuild = TRUE;
    }
    if (rebuild) {
	XawTextLineTableEntry *lt;
	int options = ctx->text.options;
	int (*FindPosition)() = ctx->text.sink->FindPosition;
	XawTextPosition (*Scan)() = ctx->text.source->Scan;
	ctx->text.lt.top = position;
	ctx->text.lt.lines = lines;
	startPos = position;
	x = ctx->text.leftmargin;
	y = yMargin;
	for (line = 0, lt = ctx->text.lt.info; line <= ctx->text.lt.lines;
	     line++, lt++) {
	    lt->x = x;
	    lt->y = y;
	    lt->position = startPos;
	    if (startPos <= ctx->text.lastPos) {
		width = (options & resizeWidth) ? BIGNUM : ctx->core.width - x;
		(*FindPosition)((Widget)ctx, startPos, x,
				width, (options & wordBreak),
				&endPos, &realW, &realH);
		if (!(options & wordBreak) && endPos < ctx->text.lastPos) {
		    endPos = (*Scan)(ctx->text.source, startPos,
				     XawstEOL, XawsdRight, 1, TRUE);
		    if (endPos == startPos)
			endPos = ctx->text.lastPos + 1;
		}
		lt->endX = realW + x;
		startPos = endPos;
	    }
	    else lt->endX = x;
	    y = y + realH;
	}
    }
}

/*
 * This routine is used to re-display the entire window, independent of
 * its current state.
*/
void ForceBuildLineTable(ctx)
    TextWidget ctx;
{
    XawTextPosition position;

    position = ctx->text.lt.top;
    ctx->text.lt.top++; /* ugly, but it works */
    BuildLineTable(ctx, position);
}

/*
 * This routine is used by Text to notify an associated scrollbar of the
 * correct metrics (position and shown fraction) for the text being currently
 * displayed in the window.
*/
static void SetScrollBar(ctx)
    TextWidget ctx;
{
    float   first, last;
    if (ctx->text.sbar) {
	if ((ctx->text.lastPos > 0)  &&  (ctx->text.lt.lines > 0)) {
	    first = ctx->text.lt.top;
	    first /= ctx->text.lastPos; 
					/* Just an approximation */
	    last = ctx->text.lt.info[ctx->text.lt.lines].position;
	    last /= ctx->text.lastPos;
	}
	else {
	    first = 0.0;
	    last = 1.0;
	}
	XawScrollBarSetThumb(ctx->text.sbar, first, last - first);
    }
}


/*
 * The routine will scroll the displayed text by lines.  If the arg  is
 * positive, move up; otherwise, move down. [note: this is really a private
 * procedure but is used in multiple modules].
*/
_XawTextScroll(ctx, n)
  TextWidget ctx;
  int n;			/* assumed <= ctx->text.lt.lines */
{
    XawTextPosition top, target;
    if (n >= 0) {
	top = Min(ctx->text.lt.info[n].position, ctx->text.lastPos);
	BuildLineTable(ctx, top);
	if (top >= ctx->text.lastPos)
	    DisplayTextWindow(ctx);
	else {
	    XCopyArea(XtDisplay(ctx), XtWindow(ctx), XtWindow(ctx), ctx->text.gc,
		      0, ctx->text.lt.info[n].y, (int)ctx->core.width,
		      (int)ctx->core.height - ctx->text.lt.info[n].y,
		      0, ctx->text.lt.info[0].y);
	    (*ctx->text.sink->ClearToBackground)(ctx, 0,
		ctx->text.lt.info[0].y + ctx->core.height - ctx->text.lt.info[n].y,
		(int)ctx->core.width, (int)ctx->core.height);
	    if (n < ctx->text.lt.lines) n++; /* update descenders at bottom */
	    _XawTextNeedsUpdating(ctx,
		    ctx->text.lt.info[ctx->text.lt.lines - n].position, ctx->text.lastPos);
	    SetScrollBar(ctx);
	}
    } else {
	int tempHeight;
	n = -n;
	target = ctx->text.lt.top;
	top = (*ctx->text.source->Scan)(ctx->text.source, target, XawstEOL,
				     XawsdLeft, n+1, FALSE);
	BuildLineTable(ctx, top);
	if (ctx->text.lt.info[n].position == target) {
	    tempHeight = ctx->text.lt.info[ctx->text.lt.lines-n].y - 1;
	    XCopyArea(XtDisplay(ctx), XtWindow(ctx), XtWindow(ctx), ctx->text.gc,
		      0, ctx->text.lt.info[0].y, (int)ctx->core.width, tempHeight,
		      0, ctx->text.lt.info[n].y);
	    (*ctx->text.sink->ClearToBackground)(ctx, 0,
		ctx->text.lt.info[0].y,
		(int)ctx->core.width, ctx->text.lt.info[n].y - 1);
	    _XawTextNeedsUpdating(ctx, 
		    ctx->text.lt.info[0].position, ctx->text.lt.info[n].position);
	    SetScrollBar(ctx);
	} else if (ctx->text.lt.top != target) DisplayTextWindow(ctx);
    }
}

/*
 * The routine will scroll the displayed text by pixels.  If the arg is
 * positive, move up; otherwise, move down.
*/
/*ARGSUSED*/
static void ScrollUpDownProc (w, closure, callData)
    Widget w;
    caddr_t closure;		/* TextWidget */
    caddr_t callData;		/* #pixels */
{
    TextWidget ctx = (TextWidget)closure;
    int     apix, line;
    _XawTextPrepareToUpdate(ctx);
    apix = abs((int)callData);
    for (line = 1;
	    line < ctx->text.lt.lines && apix > ctx->text.lt.info[line + 1].y;
	    line++);
    if (((int)callData) >= 0)
	_XawTextScroll(ctx, line);
    else
	_XawTextScroll(ctx, -line);
    _XawTextExecuteUpdate(ctx);
}

/*
 * The routine "thumbs" the displayed text. Thumbing means reposition the
 * displayed view of the source to a new position determined by a fraction
 * of the way from beginning to end. Ideally, this should be determined by
 * the number of displayable lines in the source. This routine does it as a
 * fraction of the first position and last position and then normalizes to
 * the start of the line containing the position.
*/
/*ARGSUSED*/
static void ThumbProc (w, closure, callData)
    Widget w;
    caddr_t closure;		/* TextWidget */
    float *callData;
  /* BUG/deficiency: The normalize to line portion of this routine will
   * cause thumbing to always position to the start of the source.
   */
{
    TextWidget ctx= (TextWidget)closure;
    XawTextPosition position, old_top, old_bot;
    _XawTextPrepareToUpdate(ctx);
    old_top = ctx->text.lt.top;
    old_bot = ctx->text.lt.info[ctx->text.lt.lines-1].position;
    position = *callData * ctx->text.lastPos;
    position = (*ctx->text.source->
		Scan)(ctx->text.source, position, XawstEOL, XawsdLeft, 1, FALSE);
    if (position >= old_top && position <= old_bot) {
	int line;
	for (line = 0; line < ctx->text.lt.lines &&
		       position > ctx->text.lt.info[line].position; line++);
	if (line)
	    _XawTextScroll(ctx, line);
    }
    else {
	BuildLineTable(ctx, position);
	if (old_top >= ctx->text.lt.top &&
	    old_top <= ctx->text.lt.info[ctx->text.lt.lines-1].position) {
	    int line;
	    for (line = 0;
		 line < ctx->text.lt.lines &&
		 old_top > ctx->text.lt.info[line].position; line++);
	    BuildLineTable(ctx, old_top);
	    if (line)
		_XawTextScroll(ctx, -line);
	}
	else {
	    DisplayTextWindow(ctx);
	}
    }
    _XawTextExecuteUpdate(ctx);
}


static Boolean ConvertSelection(w, selection, target,
				type, value, length, format)
  Widget w;
  Atom *selection, *target, *type;
  caddr_t *value;
  unsigned long *length;
  int *format;
{
    Display* d = XtDisplay(w);
    TextWidget ctx = (TextWidget)w;

    if (*target == XA_TARGETS(d)) {
	Atom* targetP;
	Atom* std_targets;
	unsigned long std_length;
	if (ctx->text.source->ConvertSelection == NULL ||
	    !(*ctx->text.source->
	      ConvertSelection) (d, ctx->text.source, selection, target,
				 type, value, length, format)) {
	    *value = NULL;
	    *length = 0;
	}
	XmuConvertStandardSelection(w, ctx->text.time, selection, target, type,
				   (caddr_t*)&std_targets, &std_length, format);
	*value = XtRealloc(*value, sizeof(Atom)*(std_length + 6 + *length));
	targetP = *(Atom**)value + *length;
	*length += std_length + 5;
	if (ctx->text.source->edit_mode == XawtextEdit)
	    (*length)++;
	*targetP++ = XA_STRING;
	*targetP++ = XA_TEXT(d);
	*targetP++ = XA_LENGTH(d);
	*targetP++ = XA_LIST_LENGTH(d);
	*targetP++ = XA_CHARACTER_POSITION(d);
	if (ctx->text.source->edit_mode == XawtextEdit)
	    *targetP++ = XA_DELETE(d);
	bcopy((char*)std_targets, (char*)targetP, sizeof(Atom)*std_length);
	XtFree((char*)std_targets);
	*type = XA_ATOM;
	*format = 32;
	return True;
    }

    if (ctx->text.source->ConvertSelection != NULL &&
	(*ctx->text.source->
	 ConvertSelection) (d, ctx->text.source, selection,
			    target, type, value, length, format))
	return True;

    if (*target == XA_STRING || *target == XA_TEXT(d)) {
	*type = XA_STRING;
	*value = _XawTextGetText(ctx, ctx->text.s.left, ctx->text.s.right);
	*length = strlen(*value);
	*format = 8;
	return True;
    }
    if (*target == XA_LIST_LENGTH(d)) {
	*value = XtMalloc(4);
	if (sizeof(long) == 4)
	    *(long*)*value = 1;
	else {
	    long temp = 1;
	    bcopy( ((char*)&temp)+sizeof(long)-4, (char*)*value, 4);
	}
	*type = XA_INTEGER;
	*length = 1;
	*format = 32;
	return True;
    }
    if (*target == XA_LENGTH(d)) {
	*value = XtMalloc(4);
	if (sizeof(long) == 4)
	    *(long*)*value = ctx->text.s.right - ctx->text.s.left;
	else {
	    long temp = ctx->text.s.right - ctx->text.s.left;
	    bcopy( ((char*)&temp)+sizeof(long)-4, (char*)*value, 4);
	}
	*type = XA_INTEGER;
	*length = 1;
	*format = 32;
	return True;
    }
    if (*target == XA_CHARACTER_POSITION(d)) {
	*value = XtMalloc(8);
	(*(long**)value)[0] = ctx->text.s.left + 1;
	(*(long**)value)[1] = ctx->text.s.right;
	*type = XA_SPAN(d);
	*length = 2;
	*format = 32;
	return True;
    }
    if (*target == XA_DELETE(d)) {
	static void KillCurrentSelection();
	KillCurrentSelection(ctx, (XEvent*)NULL);
	*value = NULL;
	*type = XA_NULL(d);
	*length = 0;
	*format = 32;
	return True;
    }
    if (XmuConvertStandardSelection(w, ctx->text.time, selection, target, type,
				    value, length, format))
	return True;

    /* else */
    return False;
}


static void LoseSelection(w, selection)
  Widget w;
  Atom *selection;
{
    TextWidget ctx = (TextWidget)w;
    Boolean update_in_progress = (ctx->text.old_insert >= 0);
    register Atom* atomP;
    int i, empty;

    _XawTextPrepareToUpdate(ctx);

    for (i = 0, atomP = ctx->text.s.selections;
	 i < ctx->text.s.atom_count; i++, atomP++)
    {
	if (*selection == *atomP) *atomP = (Atom)0;
	switch (*atomP) {
	  case XA_CUT_BUFFER0:
	  case XA_CUT_BUFFER1:
	  case XA_CUT_BUFFER2:
	  case XA_CUT_BUFFER3:
	  case XA_CUT_BUFFER4:
	  case XA_CUT_BUFFER5:
	  case XA_CUT_BUFFER6:
	  case XA_CUT_BUFFER7:	*atomP = (Atom)0;
	}
    }

    while (ctx->text.s.atom_count &&
	   ctx->text.s.selections[ctx->text.s.atom_count-1] == 0) {
	ctx->text.s.atom_count--;
    }

    /* must walk the selection list in opposite order from UnsetSelection */
    for (i = 0, atomP = ctx->text.s.selections;
	 i < ctx->text.s.atom_count; i++, atomP++)
    {
	if (*atomP == (Atom)0) {
	    *atomP = ctx->text.s.selections[--ctx->text.s.atom_count];
	    while (ctx->text.s.atom_count &&
		   ctx->text.s.selections[ctx->text.s.atom_count-1] == 0) {
		ctx->text.s.atom_count--;
	    }
	}
    }

    if (ctx->text.s.atom_count == 0)
	_XawTextSetNewSelection(ctx, ctx->text.insertPos, ctx->text.insertPos,
			       NULL, ZERO);

    if (!update_in_progress) {
	_XawTextExecuteUpdate(ctx);
    }
}


static int _XawTextSetNewSelection(ctx, left, right, selections, count)
  TextWidget ctx;
  XawTextPosition left, right;
  Atom *selections;
  Cardinal count;
{
    XawTextPosition pos;
    void (*nullProc)() = NULL;

    if (left < ctx->text.s.left) {
	pos = Min(right, ctx->text.s.left);
	_XawTextNeedsUpdating(ctx, left, pos);
    }
    if (left > ctx->text.s.left) {
	pos = Min(left, ctx->text.s.right);
	_XawTextNeedsUpdating(ctx, ctx->text.s.left, pos);
    }
    if (right < ctx->text.s.right) {
	pos = Max(right, ctx->text.s.left);
	_XawTextNeedsUpdating(ctx, pos, ctx->text.s.right);
    }
    if (right > ctx->text.s.right) {
	pos = Max(left, ctx->text.s.right);
	_XawTextNeedsUpdating(ctx, pos, right);
    }

    ctx->text.s.left = left;
    ctx->text.s.right = right;
    if (ctx->text.source->SetSelection != nullProc) {
	(*ctx->text.source->SetSelection) (ctx->text.source,
					   left, right,
					   count ? selections[0] : NULL);
    }
    if (left < right) {
	int buffer;
	while (count) {
	    Atom selection = selections[--count];
	    switch (selection) {
	      case XA_CUT_BUFFER0: buffer = 0; break;
	      case XA_CUT_BUFFER1: buffer = 1; break;
	      case XA_CUT_BUFFER2: buffer = 2; break;
	      case XA_CUT_BUFFER3: buffer = 3; break;
	      case XA_CUT_BUFFER4: buffer = 4; break;
	      case XA_CUT_BUFFER5: buffer = 5; break;
	      case XA_CUT_BUFFER6: buffer = 6; break;
	      case XA_CUT_BUFFER7: buffer = 7; break;
	      default:		   buffer = -1;
	    }
	    if (buffer >= 0) {
		char *ptr =
		    _XawTextGetText(ctx, ctx->text.s.left, ctx->text.s.right);
		if (buffer == 0) {
		    _CreateCutBuffers(XtDisplay((Widget)ctx));
		    XRotateBuffers(XtDisplay((Widget)ctx), 1);
		}
		XStoreBuffer(XtDisplay((Widget)ctx), ptr,
			     Min(strlen(ptr), MAXCUT), buffer);
		XtFree (ptr);
	    } else {
		XtOwnSelection((Widget)ctx, selection, ctx->text.time,
			       ConvertSelection, LoseSelection, NULL);
	    }
	}
    }
}



/*
 * This internal routine deletes the text from pos1 to pos2 in a source and
 * then inserts, at pos1, the text that was passed. As a side effect it
 * "invalidates" that portion of the displayed text (if any).
*/
static
int ReplaceText (ctx, pos1, pos2, text)
  TextWidget ctx;
  XawTextPosition pos1, pos2;
  XawTextBlock *text;

 /* it is illegal to call this routine unless there is a valid line table!*/
{
    int i, line1, visible, delta, error;
    Position x, y;
    int realW, realH, width;
    XawTextPosition startPos, endPos, updateFrom;
    XawTextPosition (*Scan)() = ctx->text.source->Scan;

    /*the insertPos may not always be set to the right spot in XawtextAppend */
    if ((pos1 == ctx->text.insertPos) &&
	(ctx->text.source->edit_mode == XawtextAppend)) {
      ctx->text.insertPos = ctx->text.lastPos;
      pos2 = pos2 - pos1 + ctx->text.insertPos;
      pos1 = ctx->text.insertPos;
    }
    updateFrom = (*Scan)(ctx->text.source, pos1, XawstWhiteSpace, XawsdLeft,
	    1, TRUE);
    updateFrom = (*Scan)(ctx->text.source, updateFrom, XawstPositions, XawsdLeft,
	    1, TRUE);
    startPos = Max(updateFrom, ctx->text.lt.top);
    visible = LineAndXYForPosition(ctx, startPos, &line1, &x, &y);
    error = (*ctx->text.source->Replace)(ctx->text.source, pos1, pos2, text);
    if (error) return error;
    ctx->text.lastPos = GETLASTPOS;
    if (ctx->text.lt.top >= ctx->text.lastPos) {
	BuildLineTable(ctx, ctx->text.lastPos);
	ClearWindow(ctx);
	SetScrollBar(ctx);
	return error;
    }
    delta = text->length - (pos2 - pos1);
    if (delta < ctx->text.lastPos) {
	pos2 += delta;
	for (i = 0; i < ctx->text.numranges; i++) {
	    if (ctx->text.updateFrom[i] > pos1)
		ctx->text.updateFrom[i] += delta;
	    if (ctx->text.updateTo[i] >= pos1)
		ctx->text.updateTo[i] += delta;
	}
    }

    /* 
     * fixup all current line table entries to reflect edit.
     * %%% it is not legal to do arithmetic on positions.
     * using Scan would be more proper.
     */
    if (delta) {
	XawTextLineTableEntry *lineP;
	int line2 = LineForPosition(ctx, pos1) + 1;
	for (i = line2, lineP = ctx->text.lt.info + line2;
	     i <= ctx->text.lt.lines; i++, lineP++)
	    lineP->position += delta;
    }

    /*
     * Now process the line table and fixup in case edits caused
     * changes in line breaks. If we are breaking on word boundaries,
     * this code checks for moving words to and from lines.
    */
    if (visible) {
	XawTextLineTableEntry *thisLine, *nextLine = ctx->text.lt.info + line1;
	Boolean resizeable = ctx->text.options & resizeWidth;
	Boolean wordwrap = ctx->text.options & wordBreak;
	int (*FindPosition)() = ctx->text.sink->FindPosition;
	int (*ClearToBackground)() = ctx->text.sink->ClearToBackground;
	register XawTextPosition lastPos = ctx->text.lastPos;
	for (i = line1; i < ctx->text.lt.lines; i++) {/* fixup line table */
	    thisLine = nextLine++;
	    width = resizeable ? BIGNUM : ctx->core.width - x;
	    if (startPos <= lastPos) {
		(*FindPosition)(ctx, startPos, x, width, wordwrap,
				&endPos, &realW, &realH);
		if (!wordwrap && endPos < lastPos) {
		    /* if not wordBreak, skip remainder of this line */
		    endPos = (*Scan)(ctx->text.source, startPos,
				     XawstEOL, XawsdRight, 1, TRUE);
		    if (endPos == startPos)
			endPos = lastPos + 1;
		}
		thisLine->endX = x + realW;
		nextLine->y = thisLine->y + realH;
		if ((endPos > pos1) && (endPos == nextLine->position))
		    break;	/* %%% why not update remaining y's? */
		startPos = endPos;
	    }
	    if (startPos > lastPos) {
		if (nextLine->position <= lastPos) {
		    (*ClearToBackground) (ctx, nextLine->x, nextLine->y,
					  nextLine->endX,
					  (nextLine+1)->y - nextLine->y);
		}
		nextLine->endX = ctx->text.leftmargin;
	    }
	    nextLine->position = startPos;
	    x = nextLine->x;
	}
	if (delta >= lastPos)
	    endPos = lastPos;
	if (endPos < pos2)	/* might scroll if word wrapped off bottom */
	    endPos = pos2;
	if (endPos > lastPos && delta > 0)
	    endPos = lastPos;	/* optimize insert at end; don't clear below */
	if (pos2 >= ctx->text.lt.top || delta >= lastPos)
	    _XawTextNeedsUpdating(ctx, updateFrom, endPos);
    }
    SetScrollBar(ctx);
    return error;
}


/*
 * This routine will display text between two arbitrary source positions.
 * In the event that this span contains highlighted text for the selection, 
 * only that portion will be displayed highlighted.
 */
static void DisplayText(w, pos1, pos2)
  Widget w;
  XawTextPosition pos1, pos2;
  /* it is illegal to call this routine unless there is a valid line table! */
{
    TextWidget ctx = (TextWidget)w;
    Position x, y;
    int height;
    int line, i, visible;
    XawTextPosition startPos, endPos;
    int lastPos = ctx->text.lastPos;
    Boolean clear_eol;
    Boolean clear_eos = True;

    if (pos1 < ctx->text.lt.top)
	pos1 = ctx->text.lt.top;
    if (pos2 > ctx->text.lastPos)
	pos2 = ctx->text.lastPos;
    else if (pos2 == ctx->text.lastPos)
	clear_eos = False;
    if (pos1 >= pos2) return;
    visible = LineAndXYForPosition(ctx, pos1, &line, &x, &y);
    if (!visible)
	return;
    startPos = pos1;
    for (i = line; i < ctx->text.lt.lines; i++) {
	endPos = ctx->text.lt.info[i + 1].position;
	if (endPos > pos2) {
	    if (endPos >= lastPos)
		clear_eol = True;
	    else
		clear_eol = False;
	    endPos = pos2;
	}
	else clear_eol = True;
	height = ctx->text.lt.info[i + 1].y - ctx->text.lt.info[i].y;
	if (endPos > startPos) {
	    if (x == ctx->text.leftmargin)
                (*ctx->text.sink->ClearToBackground)
		    (w, 0, y, ctx->text.leftmargin, height);
	    if (startPos >= ctx->text.s.right || endPos <= ctx->text.s.left) {
		(*ctx->text.sink->Display) (w, x, y, startPos, endPos, FALSE);
	    } else if (startPos >= ctx->text.s.left && endPos <= ctx->text.s.right) {
		(*ctx->text.sink->Display) (w, x, y, startPos, endPos, TRUE);
	    } else {
		DisplayText(w, startPos, ctx->text.s.left);
		DisplayText(w, Max(startPos, ctx->text.s.left), 
			    Min(endPos, ctx->text.s.right));
		DisplayText(w, ctx->text.s.right, endPos);
	    }
	}
	startPos = endPos;
	if (clear_eol)
	    (*ctx->text.sink->ClearToBackground)(ctx,
		ctx->text.lt.info[i].endX, y, (int)ctx->core.width, height);
	x = ctx->text.leftmargin;
	y = ctx->text.lt.info[i + 1].y;
	if ((endPos == pos2) && !clear_eos)
	    break;
    }
}

/*
 * This routine implements multi-click selection in a hardwired manner.
 * It supports multi-click entity cycling (char, word, line, file) and mouse
 * motion adjustment of the selected entitie (i.e. select a word then, with
 * button still down, adjust wich word you really meant by moving the mouse).
 * [NOTE: This routine is to be replaced by a set of procedures that
 * will allows clients to implements a wide class of draw through and
 * multi-click selection user interfaces.]
*/
static void DoSelection (ctx, position, time, motion)
  TextWidget ctx;
  XawTextPosition position;
  Time time;
  Boolean motion;
{
    int     delta;
    XawTextPosition newLeft, newRight;
    XawTextSelectType newType;
    XawTextSelectType *sarray;

    delta = (time < ctx->text.lasttime) ?
	ctx->text.lasttime - time : time - ctx->text.lasttime;
    if (motion)
	newType = ctx->text.s.type;
    else {
	if ((delta < 500) && ((position >= ctx->text.s.left)
		    && (position <= ctx->text.s.right))) { /* multi-click event */
	    for (sarray = ctx->text.sarray;
		*sarray != XawselectNull && *sarray != ctx->text.s.type;
		sarray++) ;
	    if (*sarray != XawselectNull) sarray++;
	    if (*sarray == XawselectNull) sarray = ctx->text.sarray;
	    newType = *sarray;
	} else {			/* single-click event */
	    newType = *(ctx->text.sarray);
	}
        ctx->text.lasttime = time;
    }
    switch (newType) {
	case XawselectPosition: 
            newLeft = newRight = position;
	    break;
	case XawselectChar: 
            newLeft = position;
            newRight = (*ctx->text.source->Scan)(
                    ctx->text.source, position, position, XawsdRight, 1, FALSE);
	    break;
	case XawselectWord: 
	    newLeft = (*ctx->text.source->Scan)(
		    ctx->text.source, position, XawstWhiteSpace, XawsdLeft, 1, FALSE);
	    newRight = (*ctx->text.source->Scan)(
		    ctx->text.source, position, XawstWhiteSpace, XawsdRight, 1, FALSE);
	    break;
	case XawselectLine: 
	case XawselectParagraph:  /* need "para" scan mode to implement pargraph */
 	    newLeft = (*ctx->text.source->Scan)(
		    ctx->text.source, position, XawstEOL, XawsdLeft, 1, FALSE);
	    newRight = (*ctx->text.source->Scan)(
		    ctx->text.source, position, XawstEOL, XawsdRight, 1, FALSE);
	    break;
	case XawselectAll: 
	    newLeft = (*ctx->text.source->Scan)(
		    ctx->text.source, position, XawstAll, XawsdLeft, 1, FALSE);
	    newRight = (*ctx->text.source->Scan)(
		    ctx->text.source, position, XawstAll, XawsdRight, 1, FALSE);
	    break;
    }
    if ((newLeft != ctx->text.s.left) || (newRight != ctx->text.s.right)
	    || (newType != ctx->text.s.type)) {
	_XawTextSetNewSelection(ctx, newLeft, newRight, NULL, ZERO);
	ctx->text.s.type = newType;
	if (position - ctx->text.s.left < ctx->text.s.right - position)
	    ctx->text.insertPos = newLeft;
	else 
	    ctx->text.insertPos = newRight;
    }
    if (!motion) { /* setup so we can freely mix select extend calls*/
	ctx->text.origSel.type = ctx->text.s.type;
	ctx->text.origSel.left = ctx->text.s.left;
	ctx->text.origSel.right = ctx->text.s.right;
	if (position >= ctx->text.s.left + ((ctx->text.s.right - ctx->text.s.left) / 2))
	    ctx->text.extendDir = XawsdRight;
	else
	    ctx->text.extendDir = XawsdLeft;
    }
}

/*
 * This routine implements extension of the currently selected text in
 * the "current" mode (i.e. char word, line, etc.). It worries about
 * extending from either end of the selection and handles the case when you
 * cross through the "center" of the current selection (e.g. switch which
 * end you are extending!).
 * [NOTE: This routine will be replaced by a set of procedures that
 * will allows clients to implements a wide class of draw through and
 * multi-click selection user interfaces.]
*/
static void ExtendSelection (ctx, position, motion)
  TextWidget ctx;
  XawTextPosition position;
  Boolean motion;
{
    XawTextPosition newLeft, newRight;
	

    if (!motion) {		/* setup for extending selection */
	ctx->text.origSel.type = ctx->text.s.type;
	ctx->text.origSel.left = ctx->text.s.left;
	ctx->text.origSel.right = ctx->text.s.right;
	if (position >= ctx->text.s.left + ((ctx->text.s.right - ctx->text.s.left) / 2))
	    ctx->text.extendDir = XawsdRight;
	else
	    ctx->text.extendDir = XawsdLeft;
    }
    else /* check for change in extend direction */
	if ((ctx->text.extendDir == XawsdRight && position < ctx->text.origSel.left) ||
		(ctx->text.extendDir == XawsdLeft && position > ctx->text.origSel.right)) {
	    ctx->text.extendDir = (ctx->text.extendDir == XawsdRight)? XawsdLeft : XawsdRight;
	    _XawTextSetNewSelection(ctx, ctx->text.origSel.left, ctx->text.origSel.right, NULL, ZERO);
	}
    newLeft = ctx->text.s.left;
    newRight = ctx->text.s.right;
    switch (ctx->text.s.type) {
	case XawselectPosition: 
	    if (ctx->text.extendDir == XawsdRight)
		newRight = position;
	    else
		newLeft = position;
	    break;
	case XawselectWord: 
	    if (ctx->text.extendDir == XawsdRight)
		newRight = position = (*ctx->text.source->Scan)(
			ctx->text.source, position, XawstWhiteSpace, XawsdRight, 1, FALSE);
	    else
		newLeft = position = (*ctx->text.source->Scan)(
			ctx->text.source, position, XawstWhiteSpace, XawsdLeft, 1, FALSE);
	    break;
        case XawselectLine:
	case XawselectParagraph: /* need "para" scan mode to implement pargraph */
	    if (ctx->text.extendDir == XawsdRight)
		newRight = position = (*ctx->text.source->Scan)(
			ctx->text.source, position, XawstEOL, XawsdRight, 1, TRUE);
	    else
		newLeft = position = (*ctx->text.source->Scan)(
			ctx->text.source, position, XawstEOL, XawsdLeft, 1, FALSE);
	    break;
	case XawselectAll: 
	    position = ctx->text.insertPos;
	    break;
    }
    _XawTextSetNewSelection(ctx, newLeft, newRight, NULL, ZERO);
    ctx->text.insertPos = position;
}


/*
 * Clear the window to background color.
 */
static ClearWindow (w)
  Widget w;
{
    if (XtIsRealized(w))
	(*((TextWidget)w)->text.sink->
	 ClearToBackground) (w, 0, 0, (int)w->core.width, (int)w->core.height);
}


/*
 * Internal redisplay entire window.
 * Legal to call only if widget is realized.
 */
DisplayTextWindow (w)
  Widget w;
{
    TextWidget ctx = (TextWidget) w;
    ClearWindow(w);
    BuildLineTable(ctx, ctx->text.lt.top);
    _XawTextNeedsUpdating(ctx, zeroPosition, ctx->text.lastPos);
    SetScrollBar(ctx);
}

/*
 * This routine checks to see if the window should be resized (grown or
 * shrunk) when text to be painted overflows to the right or
 * the bottom of the window. It is used by the keyboard input routine.
 */

static void
CheckResize(ctx)
TextWidget ctx;
{
    int line = 0, old_height;
    XtWidgetGeometry rbox;
    XtGeometryResult reply;
    register int options = ctx->text.options;

    if (options & resizeWidth) {
	XawTextLineTableEntry *lt;
	rbox.width = 0;
	for (lt = ctx->text.lt.info; line < ctx->text.lt.lines; line++, lt++) 
	    AssignMax(rbox.width, lt->endX);

	if (rbox.width > ctx->core.width) { /* Only get wider. */
	    rbox.request_mode = CWWidth;
	    reply = XtMakeGeometryRequest((Widget)ctx, &rbox, &rbox);
	    if (reply == XtGeometryAlmost)
	        (void) XtMakeGeometryRequest((Widget)ctx, &rbox, &rbox);
	}
    }

    if (!options & resizeHeight)
      return;

    if (IsPositionVisible(ctx, ctx->text.lastPos))
        line = LineForPosition(ctx, ctx->text.lastPos);
    else
        line = ctx->text.lt.lines;

    if ( (line + 1) == ctx->text.lt.lines )
      return;

    old_height = ctx->core.height;
    rbox.request_mode = CWHeight;
    rbox.height = (*ctx->text.sink->MaxHeight) (ctx, line + 1)+(2 * yMargin)+2;
    
    if (rbox.height < old_height) /* It will only get taller. */
      return;

    reply = XtMakeGeometryRequest(ctx, &rbox, &rbox);
    if (reply == XtGeometryAlmost)
        reply = XtMakeGeometryRequest((Widget)ctx, &rbox, &rbox);
    if (reply != XtGeometryYes) 
      return;

    BuildLineTable(ctx, ctx->text.lt.top);
    
/*
 * Clear cruft from bottom margin 
 */

    if ( !(options & wordBreak) && (rbox.height < old_height) ) {
        (*ctx->text.sink->ClearToBackground) (ctx, ctx->text.leftmargin,
			     ctx->text.lt.info[ctx->text.lt.lines].y,
		             (int) ctx->core.width, old_height - rbox.height);
    }
}

static Atom* _SelectionList(ctx, params, num_params)
  TextWidget ctx;
  String *params;
  Cardinal num_params;
{
    /* converts (params, num_params) to a list of atoms & caches the
     * list in the TextWidget instance.
     */

    if (num_params > ctx->text.s.array_size) {
	ctx->text.s.selections =
	    (Atom*)XtRealloc(ctx->text.s.selections, num_params*sizeof(Atom));
	ctx->text.s.array_size = num_params;
    }
    XmuInternStrings( XtDisplay((Widget)ctx), params, num_params,
		      ctx->text.s.selections );
    ctx->text.s.atom_count = num_params;
    return ctx->text.s.selections;
}


/*
 * This routine is used to perform various selection functions. The goal is
 * to be able to specify all the more popular forms of draw-through and
 * multi-click selection user interfaces from the outside.
 */
void AlterSelection (ctx, mode, action, params, num_params)
    TextWidget     ctx;
    XawTextSelectionMode   mode;	/* {XawsmTextSelect, XawsmTextExtend} */
    XawTextSelectionAction action; /* {XawactionStart, XawactionAdjust, XawactionEnd} */
    String	*params;
    Cardinal	*num_params;
{
    XawTextPosition position;

    position = PositionForXY (ctx, (int) ctx->text.ev_x, (int) ctx->text.ev_y);
    if (action == XawactionStart) {
	switch (mode) {
	case XawsmTextSelect: 
	    DoSelection (ctx, position, ctx->text.time, FALSE);
	    break;
	case XawsmTextExtend: 
	    ExtendSelection (ctx, position, FALSE);
	    break;
	}
    }
    else {
	switch (mode) {
	case XawsmTextSelect: 
	    DoSelection (ctx, position, ctx->text.time, TRUE);
	    break;
	case XawsmTextExtend: 
	    ExtendSelection (ctx, position, TRUE);
	    break;
	}
    }
    if (action == XawactionEnd) {
	if (ctx->text.s.left < ctx->text.s.right) {
	    Cardinal count = *num_params;
	    if (count == 0) {
		static String defaultSelection = "CUT_BUFFER0";
		params = &defaultSelection;
		count = 1;
	    }
	    _XawTextSetNewSelection(
		   ctx, ctx->text.s.left, ctx->text.s.right,
		   _SelectionList(ctx, params, count),
		   count );
	}
	else XawTextUnsetSelection((Widget)ctx);
    }
}

/*
 * This routine processes all "expose region" XEvents. In general, its job
 * is to the best job at minimal re-paint of the text, displayed in the
 * window, that it can.
*/
static void ProcessExposeRegion(w, event)
  Widget w;
  XEvent *event;
{
    TextWidget ctx = (TextWidget) w;
    XawTextPosition pos1, pos2, resultend;
    int line;
    int x = event->xexpose.x;
    int y = event->xexpose.y;
    int width = event->xexpose.width;
    int height = event->xexpose.height;
    XawTextLineTableEntry *info;

   _XawTextPrepareToUpdate(ctx);
    if (x < ctx->text.leftmargin) /* stomp on caret tracks */
        (*ctx->text.sink->ClearToBackground)(ctx, x, y, width, height);
   /* figure out starting line that was exposed */
    line = LineForPosition(ctx, PositionForXY(ctx, x, y));
    while (line < ctx->text.lt.lines && ctx->text.lt.info[line + 1].y < y)
	line++;
    while (line < ctx->text.lt.lines) {
	info = &(ctx->text.lt.info[line]);
	if (info->y >= y + height)
	    break;
	(*ctx->text.sink->Resolve)(ctx, 
                                info->position, info->x,
			        x - info->x, &pos1, &resultend);
	(*ctx->text.sink->Resolve)(ctx, 
                                info->position, info->x,
			        x + width - info->x, &pos2, 
                                &resultend);
	pos2 = (*ctx->text.source->Scan)(ctx->text.source, pos2, XawstPositions, 
                                      XawsdRight, 1, TRUE);
	_XawTextNeedsUpdating(ctx, pos1, pos2);
	line++;
    }
    _XawTextExecuteUpdate(ctx);
}

/*
 * This routine does all setup required to syncronize batched screen updates
*/
int _XawTextPrepareToUpdate(ctx)
  TextWidget ctx;
{
    if (ctx->text.old_insert < 0) {
	InsertCursor((Widget)ctx, XawisOff);
	ctx->text.numranges = 0;
	ctx->text.showposition = FALSE;
	ctx->text.old_insert = ctx->text.insertPos;
    }
}


/*
 * This is a private utility routine used by _XawTextExecuteUpdate. It
 * processes all the outstanding update requests and merges update
 * ranges where possible.
*/
static void FlushUpdate(ctx)
  TextWidget ctx;
{
    int     i, w;
    XawTextPosition updateFrom, updateTo;
    if (!XtIsRealized((Widget)ctx)) {
	ctx->text.numranges = 0;
	return;
    }
    while (ctx->text.numranges > 0) {
	updateFrom = ctx->text.updateFrom[0];
	w = 0;
	for (i=1 ; i<ctx->text.numranges ; i++) {
	    if (ctx->text.updateFrom[i] < updateFrom) {
		updateFrom = ctx->text.updateFrom[i];
		w = i;
	    }
	}
	updateTo = ctx->text.updateTo[w];
	ctx->text.numranges--;
	ctx->text.updateFrom[w] = ctx->text.updateFrom[ctx->text.numranges];
	ctx->text.updateTo[w] = ctx->text.updateTo[ctx->text.numranges];
	for (i=ctx->text.numranges-1 ; i>=0 ; i--) {
	    while (ctx->text.updateFrom[i] <= updateTo && i < ctx->text.numranges) {
		updateTo = ctx->text.updateTo[i];
		ctx->text.numranges--;
		ctx->text.updateFrom[i] = ctx->text.updateFrom[ctx->text.numranges];
		ctx->text.updateTo[i] = ctx->text.updateTo[ctx->text.numranges];
	    }
	}
	DisplayText((Widget)ctx, updateFrom, updateTo);
    }
}


/*
 * This is a private utility routine used by _XawTextExecuteUpdate. This
 * routine worries about edits causing new data or the insertion point becoming
 * invisible (off the screen). Currently it always makes it visible by
 * scrolling. It probably needs generalization to allow more options.
 */

_XawTextShowPosition(ctx)
  TextWidget ctx;
{
    int lines = ctx->text.lt.lines;
    XawTextPosition top, first, second;

    if ( (!XtIsRealized((Widget)ctx)) || (ctx->text.lt.lines <= 0) ||
	 ( (ctx->text.insertPos >= ctx->text.lt.top) &&
	   ((ctx->text.insertPos < ctx->text.lt.info[lines].position) ||
	    (ctx->text.lt.info[lines].position > ctx->text.lastPos))) )
      return;

    first = ctx->text.lt.top;
    second = ctx->text.lt.info[1].position;
    if (ctx->text.insertPos < first)
      top = (*ctx->text.source->Scan)(ctx->text.source, ctx->text.insertPos,
				      XawstEOL, XawsdLeft, 1, FALSE);
    else
      top = (*ctx->text.source->Scan)(ctx->text.source, ctx->text.insertPos,
				      XawstEOL, XawsdLeft, lines, FALSE);
    BuildLineTable(ctx, top);
    while (ctx->text.insertPos >= ctx->text.lt.info[lines].position) {
      if (ctx->text.lt.info[lines].position >
	  ctx->text.lastPos)
	break;
      BuildLineTable(ctx, ctx->text.lt.info[1].position);
    }
    if (ctx->text.lt.top == second) {
      BuildLineTable(ctx, first);
      _XawTextScroll(ctx, 1);
    } else if (ctx->text.lt.info[1].position == first) {
      BuildLineTable(ctx, first);
      _XawTextScroll(ctx, -1);
    } else {
      ctx->text.numranges = 0;
      if (ctx->text.lt.top != first)
	DisplayTextWindow((Widget)ctx);
    }
}

/*
 * This routine causes all batched screen updates to be performed
*/
_XawTextExecuteUpdate(ctx)
  TextWidget ctx;
{
    if (ctx->text.update_disabled) return;

    if (ctx->text.old_insert >= 0) {
	if (ctx->text.old_insert != ctx->text.insertPos
	    || ctx->text.showposition)
	    _XawTextShowPosition(ctx);
	FlushUpdate(ctx);
	InsertCursor((Widget)ctx, XawisOn);
	ctx->text.old_insert = -1;
    }
}


static void TextDestroy(w)
    Widget w;
{
    TextWidget ctx = (TextWidget)w;
    register struct _dialog *dialog, *next;

    for (dialog = ctx->text.dialog; dialog; dialog = next) {
	/* no need to destroy the widgets here; they should go automatically */
	next = dialog->next;
	XtFree( dialog );
    }
    if (ctx->text.outer)
	(void) XtDestroyWidget(ctx->text.outer);
    if (ctx->text.sbar)
	(void) XtDestroyWidget(ctx->text.sbar);
    XtFree((char *)ctx->text.updateFrom);
    XtFree((char *)ctx->text.updateTo);
}


/* by the time we are managed (and get this far),
 * we had better have both a source and a sink */
static void Resize(w)
    Widget          w;
{
    TextWidget ctx = (TextWidget) w;

    if (ctx->text.sbar) {
	Widget sbar = ctx->text.sbar;
	XtResizeWidget( sbar, sbar->core.width, ctx->core.height,
		        sbar->core.border_width );
    }
    _XawTextPrepareToUpdate(ctx);
    ForceBuildLineTable(ctx);
    _XawTextExecuteUpdate(ctx);
}


/*
 * This routine allow the application program to Set attributes.
 */

/*ARGSUSED*/
static Boolean SetValues(current, request, new)
Widget current, request, new;
{
    TextWidget oldtw = (TextWidget) current;
    TextWidget newtw = (TextWidget) new;
    Boolean    redisplay = FALSE;
    Boolean    display_caret = newtw->text.display_caret;
    
    newtw->text.display_caret = oldtw->text.display_caret;
    _XawTextPrepareToUpdate(newtw);
    newtw->text.display_caret = display_caret;
    
    if ((oldtw->text.options & scrollVertical)
		!= (newtw->text.options & scrollVertical)) {
	newtw->text.leftmargin = newtw->text.client_leftmargin;
	if (newtw->text.options & scrollVertical)
	    CreateScrollbar(newtw);
	else {
	    XtDestroyWidget(oldtw->text.sbar);
	    newtw->text.sbar = NULL;
	}
    }
    else if (oldtw->text.client_leftmargin != newtw->text.client_leftmargin) {
	newtw->text.leftmargin = newtw->text.client_leftmargin;
	if (newtw->text.options & scrollVertical) {
	    newtw->text.leftmargin +=
		    newtw->text.sbar->core.width +
		    newtw->text.sbar->core.border_width;
	}
    }

    if (oldtw->text.source != newtw->text.source ||
	oldtw->text.sink != newtw->text.sink ||
	oldtw->text.lt.top != newtw->text.lt.top ||
	oldtw->text.leftmargin != newtw->text.leftmargin ||
	((oldtw->text.options & wordBreak)
			!= (newtw->text.options & wordBreak)))
    {
	ForceBuildLineTable(newtw);
	SetScrollBar(newtw);
	redisplay = TRUE;
    }

    if (oldtw->text.insertPos != newtw->text.insertPos)
	newtw->text.showposition = TRUE;

    if (XtIsRealized(newtw)
	&& ((oldtw->text.options & wordBreak)
	    != (newtw->text.options & wordBreak))) {
	XSetWindowAttributes attributes;
	Mask valueMask;
	valueMask = CWBitGravity;
	attributes.bit_gravity =
	  (newtw->text.options & wordBreak) ? ForgetGravity : NorthWestGravity;
	XChangeWindowAttributes(XtDisplay(newtw), XtWindow(newtw),
				valueMask, &attributes);
	redisplay = TRUE;
    }

    if (!redisplay)
	_XawTextExecuteUpdate(newtw);

    return redisplay;
}



void XawTextDisplay (w)
    Widget w;
{
    TextWidget ctx = (TextWidget) w;

    if (!XtIsRealized(w)) return;

	_XawTextPrepareToUpdate(ctx);
	DisplayTextWindow(w);
	_XawTextExecuteUpdate(ctx);
}

/*******************************************************************
The following routines provide procedural interfaces to Text window state
setting and getting. They need to be redone so than the args code can use
them. I suggest we create a complete set that takes the context as an
argument and then have the public version lookup the context and call the
internal one. The major value of this set is that they have actual application
clients and therefore the functionality provided is required for any future
version of Text.
********************************************************************/

void XawTextSetSelectionArray(w, sarray)
    Widget w;
    XawTextSelectType *sarray;
{
    ((TextWidget)w)->text.sarray = sarray;
}

void XawTextSetLastPos (w, lastPos)
    Widget w;
    XawTextPosition lastPos;
{
    TextWidget  ctx = (TextWidget) w;

	_XawTextPrepareToUpdate(ctx);
	(*ctx->text.source->SetLastPos)(ctx->text.source, lastPos);
	ctx->text.lastPos = GETLASTPOS;
	ForceBuildLineTable(ctx);
        if (XtIsRealized(w))
	    DisplayTextWindow(w);
	_XawTextExecuteUpdate(ctx);
}


void XawTextGetSelectionPos(w, left, right)
  Widget w;
  XawTextPosition *left, *right;
{
    TextWidget ctx = (TextWidget) w;
    *left = ctx->text.s.left;
    *right = ctx->text.s.right;
}


void XawTextSetSource(w, source, startPos)
    Widget w;
    XawTextSource   source;
    XawTextPosition startPos;
{
    TextWidget ctx = (TextWidget) w;

	ctx->text.source = source;
	ctx->text.lt.top = startPos;
	ctx->text.s.left = ctx->text.s.right = 0;
	ctx->text.insertPos = startPos;
	ctx->text.lastPos = GETLASTPOS;

	ForceBuildLineTable(ctx);
        if (XtIsRealized(w)) {
	    _XawTextPrepareToUpdate(ctx);
	    DisplayTextWindow(w);
	    _XawTextExecuteUpdate(ctx);
	}
}

/*
 * This public routine deletes the text from startPos to endPos in a source and
 * then inserts, at startPos, the text that was passed. As a side effect it
 * "invalidates" that portion of the displayed text (if any), so that things
 * will be repainted properly.
 */
int XawTextReplace(w, startPos, endPos, text)
    Widget	    w;
    XawTextPosition  startPos, endPos;
    XawTextBlock     *text;
{
    TextWidget ctx = (TextWidget) w;
    int result;

    _XawTextPrepareToUpdate(ctx);
    if (endPos > ctx->text.lastPos) endPos = ctx->text.lastPos;
    if (startPos > ctx->text.lastPos) startPos = ctx->text.lastPos;
    if ((result = ReplaceText(ctx, startPos, endPos, text)) == XawEditDone) {
	if (ctx->text.insertPos >= endPos) {
	    int delta = text->length - (endPos - startPos);
	    XawTextScanDirection sd;
	    if (delta < 0) {
		sd = XawsdLeft;
		delta = -delta;
	    }
	    else
		sd = XawsdRight;

	    ctx->text.insertPos =
		(*ctx->text.source->Scan)(ctx->text.source,
					  ctx->text.insertPos,
					  XawstPositions, sd,
					  delta, TRUE);
	}
	else if (ctx->text.insertPos > startPos)
	    ctx->text.insertPos =
		(*ctx->text.source->Scan)(ctx->text.source, startPos,
					  XawstPositions, XawsdRight,
					  text->length, TRUE);
    }
    CheckResize(ctx);
    _XawTextExecuteUpdate(ctx);

    return result;
}


XawTextPosition XawTextTopPosition(w)
    Widget	    w;
{
    TextWidget ctx = (TextWidget) w;

     return ctx->text.lt.top;
}


void XawTextSetInsertionPoint(w, position)
    Widget	    w;
    XawTextPosition position;
{
    TextWidget ctx = (TextWidget) w;

	_XawTextPrepareToUpdate(ctx);
	ctx->text.insertPos = (position > ctx->text.lastPos)
			       ? ctx->text.lastPos : position;
	ctx->text.showposition = TRUE;
	_XawTextExecuteUpdate(ctx);
}


XawTextPosition XawTextGetInsertionPoint(w)
    Widget	    w;
{
    TextWidget ctx = (TextWidget) w;

    return(ctx->text.insertPos);
}


void XawTextUnsetSelection(w)
    Widget	    w;
{
    register TextWidget ctx = (TextWidget)w;
    int i;
    void (*nullProc)() = NULL;

    /* must walk the selection list in opposite order from LoseSelection */
    for (i = ctx->text.s.atom_count; i;) {
	Atom selection = ctx->text.s.selections[--i];
	switch (selection) {
	  case 0:		/* LoseSelection called once */
	  case XA_CUT_BUFFER0:
	  case XA_CUT_BUFFER1:
	  case XA_CUT_BUFFER2:
	  case XA_CUT_BUFFER3:
	  case XA_CUT_BUFFER4:
	  case XA_CUT_BUFFER5:
	  case XA_CUT_BUFFER6:
	  case XA_CUT_BUFFER7: continue;
	}
	XtDisownSelection(w, selection);
	LoseSelection(w, &selection); /* in case it wasn't just called */
	if (i > ctx->text.s.atom_count) i = ctx->text.s.atom_count;
    }
    /* LoseSelection will have called SetNewSelection on the empty list */
}


void XawTextChangeOptions(w, options)
    Widget	    w;
    int    options;
{
    TextWidget ctx = (TextWidget) w;

	ctx->text.options = options;
}


int XawTextGetOptions(w)
    Widget	    w;
{
    TextWidget ctx = (TextWidget) w;

 	return ctx->text.options;
}

void XawTextSetSelection (w, left, right)
    Widget	    w;
    XawTextPosition left, right;
{
    TextWidget ctx = (TextWidget) w;
    Atom selection = XA_PRIMARY;

	_XawTextPrepareToUpdate(ctx);
        if (left == right)
	    XawTextUnsetSelection(w);
	else
	    _XawTextSetNewSelection(ctx, left, right, &selection, ONE);
	_XawTextExecuteUpdate(ctx);
}

void XawTextInvalidate(w, from, to)
    Widget	    w;
    XawTextPosition from,to;
{
    TextWidget ctx = (TextWidget) w;

        ctx->text.lastPos = GETLASTPOS;
        _XawTextPrepareToUpdate(ctx);
        _XawTextNeedsUpdating(ctx, from, to);
        ForceBuildLineTable(ctx);
        _XawTextExecuteUpdate(ctx);
}

/*ARGSUSED*/
void XawTextDisableRedisplay(w, d)
    Widget w;
    int d;
{
    register TextWidget ctx = (TextWidget)w;

    ctx->text.update_disabled = True;
    _XawTextPrepareToUpdate(ctx);
}

void XawTextEnableRedisplay(w)
    Widget w;
{
    register TextWidget ctx = (TextWidget)w;
    register XawTextPosition lastPos;

    if (!ctx->text.update_disabled) return;

    ctx->text.update_disabled = False;
    lastPos = ctx->text.lastPos = GETLASTPOS;
    if (ctx->text.lt.top > lastPos)    ctx->text.lt.top = ctx->text.lastPos;
    if (ctx->text.insertPos > lastPos) ctx->text.insertPos = ctx->text.lastPos;
    if (ctx->text.s.left > lastPos ||
	ctx->text.s.right > lastPos)  ctx->text.s.left = ctx->text.s.right = 0;

    ForceBuildLineTable(ctx);
    if (XtIsRealized(w))
	DisplayTextWindow(w);
    _XawTextExecuteUpdate(ctx);
}

XawTextSource XawTextGetSource(w)
    Widget w;
{
    return ((TextWidget)w)->text.source;
}


void XawTextDisplayCaret (w, display_caret)
    Widget w;
    Boolean display_caret;
{
    TextWidget ctx = (TextWidget) w;

    if (ctx->text.display_caret == display_caret) return;

    if (XtIsRealized(w)) {
	_XawTextPrepareToUpdate(ctx);
	ctx->text.display_caret = display_caret;
	_XawTextExecuteUpdate(ctx);
    }
    else
	ctx->text.display_caret = display_caret;
}

/* The following used to be a separate file, Textacts.c, but
   is included here because textActionsTable can't be external
   to the file declaring textClassRec */

/* Misc. routines */
void TextAcceptFocus(w)
    Widget          w;
{
    TextWidget ctx = (TextWidget) w;
    if (!ctx->text.hasfocus)
	XSetInputFocus(XtDisplay(ctx), XtWindow(ctx), 
	  RevertToPointerRoot,  CurrentTime);
}

static StartAction(ctx, event)
   TextWidget ctx;
   XEvent *event;
{
    _XawTextPrepareToUpdate(ctx);
    if (event != NULL) {
	switch (event->type) {
	  case ButtonPress:
	  case ButtonRelease:
				    ctx->text.time = event->xbutton.time;
				    ctx->text.ev_x = event->xbutton.x;
				    ctx->text.ev_y = event->xbutton.y;
				    break;
	  case KeyPress:
	  case KeyRelease:
				    ctx->text.time = event->xkey.time;
				    ctx->text.ev_x = event->xkey.x;
				    ctx->text.ev_y = event->xkey.y;
				    break;
	  case MotionNotify:
				    ctx->text.time = event->xmotion.time;
				    ctx->text.ev_x = event->xmotion.x;
				    ctx->text.ev_y = event->xmotion.y;
				    break;
	  case EnterNotify:
	  case LeaveNotify:
				    ctx->text.time = event->xcrossing.time;
				    ctx->text.ev_x = event->xcrossing.x;
				    ctx->text.ev_y = event->xcrossing.y;
	}
    }
}

static EndAction(ctx)
   TextWidget ctx;
{
    CheckResize(ctx);
    _XawTextExecuteUpdate(ctx);
}

#ifdef notdef
static DoFeep(ctx)
    TextWidget ctx;
{
    XBell(XtDisplay(ctx), 50);
}
#endif

static DeleteOrKill(ctx, from, to, kill)
    TextWidget	   ctx;
    XawTextPosition from, to;
    Boolean	   kill;
{
    XawTextBlock text;
    char *ptr;

    if (kill && from < to) {
	ptr = _XawTextGetText(ctx, from, to);
	XStoreBuffer(XtDisplay(ctx), ptr, strlen(ptr), 1);
	XtFree(ptr);
    }
    text.length = 0;
    text.firstPos = 0;
    if (ReplaceText(ctx, from, to, &text)) {
	XBell(XtDisplay(ctx), 50);
	return;
    }
    ctx->text.insertPos = from;
    XawTextUnsetSelection((Widget)ctx);
    ctx->text.showposition = TRUE;
}


StuffFromBuffer(ctx, buffer)
  TextWidget ctx;
  int buffer;
{
    extern char *XFetchBuffer();
    XawTextBlock text;
    text.ptr = XFetchBuffer(XtDisplay(ctx), &(text.length), buffer);
    text.firstPos = 0;
    if (ReplaceText(ctx, ctx->text.insertPos, ctx->text.insertPos, &text)) {
	XBell(XtDisplay(ctx), 50);
	return;
    }
    ctx->text.insertPos = (*ctx->text.source->Scan)(ctx->text.source, 
    	ctx->text.insertPos, XawstPositions, XawsdRight, text.length, TRUE);
    XawTextUnsetSelection((Widget)ctx);
    XtFree(text.ptr);
}


static void UnKill(ctx, event)
  TextWidget ctx;
   XEvent *event;
{
   StartAction(ctx, event);
    StuffFromBuffer(ctx, 1);
   EndAction(ctx);
}


static void Stuff(ctx, event)
  TextWidget ctx;
   XEvent *event;
{
   StartAction(ctx, event);
    StuffFromBuffer(ctx, 0);
   EndAction(ctx);
}


struct _SelectionList {
    String *params;
    Cardinal count;
    Time time;
};

static void _GetSelection(w, time, params, num_params)
Widget w;
Time time;
String *params;			/* selections in precedence order */
Cardinal num_params;
{
    static void _SelectionReceived();
    Atom selection;
    int buffer;

    XmuInternStrings(XtDisplay(w), params, (Cardinal)1, &selection);
    switch (selection) {
      case XA_CUT_BUFFER0: buffer = 0; break;
      case XA_CUT_BUFFER1: buffer = 1; break;
      case XA_CUT_BUFFER2: buffer = 2; break;
      case XA_CUT_BUFFER3: buffer = 3; break;
      case XA_CUT_BUFFER4: buffer = 4; break;
      case XA_CUT_BUFFER5: buffer = 5; break;
      case XA_CUT_BUFFER6: buffer = 6; break;
      case XA_CUT_BUFFER7: buffer = 7; break;
      default:	       buffer = -1;
    }
    if (buffer >= 0) {
	unsigned long nbytes;
	int fmt8 = 8;
	Atom type = XA_STRING;
	char *line = XFetchBuffer(XtDisplay(w), &nbytes, buffer);
	if (nbytes > 0)
	    _SelectionReceived(w, NULL, &selection, &type, (caddr_t)line,
			       &nbytes, &fmt8);
	else if (num_params > 1)
	    _GetSelection(w, time, params+1, num_params-1);
    } else {
	struct _SelectionList* list;
	if (--num_params) {
	    list = XtNew(struct _SelectionList);
	    list->params = params + 1;
	    list->count = num_params;
	    list->time = time;
	} else list = NULL;
	XtGetSelectionValue(w, selection, XA_STRING, _SelectionReceived,
			    (caddr_t)list, time);
    }
}


/* ARGSUSED */
static void _SelectionReceived(w, client_data, selection, type,
			       value, length, format)
Widget w;
caddr_t client_data;
Atom *selection, *type;
caddr_t value;
unsigned long *length;
int *format;
{
    TextWidget ctx = (TextWidget)w;
    XawTextBlock text;
				  
    if (*type == 0 /*XT_CONVERT_FAIL*/ || *length == 0) {
	struct _SelectionList* list = (struct _SelectionList*)client_data;
	if (list != NULL) {
	    _GetSelection(w, list->time, list->params, list->count);
	    XtFree(client_data);
	}
	return;
    }

    StartAction(ctx, NULL);

    text.ptr = (char*)value;
    text.firstPos = 0;
    text.length = *length;
    text.format = FMT8BIT;
    if (ReplaceText(ctx, ctx->text.insertPos, ctx->text.insertPos, &text)) {
	XBell(XtDisplay(ctx), 50);
	return;
    }
    ctx->text.insertPos = (*ctx->text.source->Scan)(ctx->text.source, 
    	ctx->text.insertPos, XawstPositions, XawsdRight, text.length, TRUE);
    XawTextUnsetSelection((Widget)ctx);

    EndAction(ctx);

    XtFree(client_data);
    XtFree(value);
}


static void InsertSelection(w, event, params, num_params)
   Widget w;
   XEvent *event;
   String *params;		/* precedence list of selections to try */
   Cardinal *num_params;
{
   static String default_params[] = {"PRIMARY", "CUT_BUFFER0"};
   int count;
   StartAction((TextWidget)w, event);
    if ((count = *num_params) == 0) {
	params = default_params;
	count = XtNumber(default_params);
    }
    _GetSelection(w, ((TextWidget)w)->text.time, params, count);
   EndAction((TextWidget)w);
}


static XawTextPosition NextPosition(ctx, position, kind, direction)
    TextWidget ctx;
    XawTextPosition position;
    XawTextScanType kind;
    XawTextScanDirection direction;
{
    XawTextPosition pos;

     pos = (*ctx->text.source->Scan)(
	    ctx->text.source, position, kind, direction, 1, FALSE);
     if (pos == ctx->text.insertPos) 
         pos = (*ctx->text.source->Scan)(
            ctx->text.source, position, kind, direction, 2, FALSE);
     return pos;
}

/* routines for moving around */

static void MoveForwardChar(ctx, event)
   TextWidget ctx;
   XEvent *event;
{
   StartAction(ctx, event);
   ctx->text.insertPos = (*ctx->text.source->Scan)(
        ctx->text.source, ctx->text.insertPos, XawstPositions, XawsdRight, 1, 
	TRUE);
   EndAction(ctx);
}

static void MoveBackwardChar(ctx, event)
    TextWidget ctx;
   XEvent *event;
{
   StartAction(ctx, event);
    ctx->text.insertPos = (*ctx->text.source->Scan)(
            ctx->text.source, ctx->text.insertPos, XawstPositions, XawsdLeft,
	    1, TRUE);
   EndAction(ctx);
}

static void MoveForwardWord(ctx, event)
    TextWidget ctx;
   XEvent *event;
{
   StartAction(ctx, event);
    ctx->text.insertPos = NextPosition(ctx, ctx->text.insertPos, 
      XawstWhiteSpace, XawsdRight);
   EndAction(ctx);
}

static void MoveBackwardWord(ctx, event)
    TextWidget ctx;
   XEvent *event;
{
   StartAction(ctx, event);
    ctx->text.insertPos = NextPosition(ctx, ctx->text.insertPos, 
      XawstWhiteSpace, XawsdLeft);
   EndAction(ctx);
}

static void MoveBackwardParagraph(ctx, event)
    TextWidget ctx;
   XEvent *event;
{
   StartAction(ctx, event);
    ctx->text.insertPos = NextPosition(ctx, ctx->text.insertPos, 
      XawstEOL, XawsdLeft);
   EndAction(ctx);
}

static void MoveForwardParagraph(ctx, event)
    TextWidget ctx;
   XEvent *event;
{
   StartAction(ctx, event);
    ctx->text.insertPos = NextPosition(ctx, ctx->text.insertPos, 
      XawstEOL, XawsdRight);
   EndAction(ctx);
}


static void MoveToLineStart(ctx, event)
  TextWidget ctx;
   XEvent *event;
{
    int line;
   StartAction(ctx, event);
    _XawTextShowPosition(ctx);
    line = LineForPosition(ctx, ctx->text.insertPos);
    ctx->text.insertPos = ctx->text.lt.info[line].position;
   EndAction(ctx);
}

static void MoveToLineEnd(ctx, event)
  TextWidget ctx;
   XEvent *event;
{
    int line;
    XawTextPosition next;
   StartAction(ctx, event);
    _XawTextShowPosition(ctx);
    line = LineForPosition(ctx, ctx->text.insertPos);
    next = ctx->text.lt.info[line+1].position;
    if (next > ctx->text.lastPos)
	next = ctx->text.lastPos;
    else
	next = (*ctx->text.source->Scan)(ctx->text.source, next, XawstPositions, 
	  XawsdLeft, 1, TRUE);
    ctx->text.insertPos = next;
   EndAction(ctx);
}


static void MoveNextLine(ctx, event)
  TextWidget ctx;
   XEvent *event;
{
    int     width, width2, height, line;
    XawTextPosition position, maxp;
   StartAction(ctx, event);
    _XawTextShowPosition(ctx);
    line = LineForPosition(ctx, ctx->text.insertPos);
    if (line == ctx->text.lt.lines - 1 &&
	ctx->text.lt.info[ctx->text.lt.lines].position <= ctx->text.lastPos) {
	_XawTextScroll(ctx, 1);
	line = LineForPosition(ctx, ctx->text.insertPos);
    }
    (*ctx->text.sink->FindDistance)(ctx,
		ctx->text.lt.info[line].position, ctx->text.lt.info[line].x,
		ctx->text.insertPos, &width, &position, &height);
    line++;
    if (ctx->text.lt.info[line].position > ctx->text.lastPos) {
	ctx->text.insertPos = ctx->text.lastPos;
	EndAction(ctx);
	return;
    }
    (*ctx->text.sink->FindPosition)(ctx,
	    ctx->text.lt.info[line].position, ctx->text.lt.info[line].x,
	    width, FALSE, &position, &width2, &height);
    maxp = (*ctx->text.source->Scan)(ctx->text.source,
            ctx->text.lt.info[line+1].position,
	    XawstPositions, XawsdLeft, 1, TRUE);
    if (position > maxp)
	position = maxp;
    ctx->text.insertPos = position;
   EndAction(ctx);
}

static void MovePreviousLine(ctx, event)
  TextWidget ctx;
   XEvent *event;
{
    int     width, width2, height, line;
    XawTextPosition position, maxp;
   StartAction(ctx, event);
    _XawTextShowPosition(ctx);
    line = LineForPosition(ctx, ctx->text.insertPos);
    if (line == 0) {
	_XawTextScroll(ctx, -1);
	line = LineForPosition(ctx, ctx->text.insertPos);
    }
    if (line > 0) {
	(*ctx->text.sink->FindDistance)(ctx,
		    ctx->text.lt.info[line].position, 
		    ctx->text.lt.info[line].x,
		    ctx->text.insertPos, &width, &position, &height);
	line--;
	(*ctx->text.sink->FindPosition)(ctx,
		ctx->text.lt.info[line].position, ctx->text.lt.info[line].x,
		width, FALSE, &position, &width2, &height);
	maxp = (*ctx->text.source->Scan)(ctx->text.source, 
		ctx->text.lt.info[line+1].position,
		XawstPositions, XawsdLeft, 1, TRUE);
	if (position > maxp)
	    position = maxp;
	ctx->text.insertPos = position;
    }
   EndAction(ctx);
}



static void MoveBeginningOfFile(ctx, event)
  TextWidget ctx;
   XEvent *event;
{
   StartAction(ctx, event);
    ctx->text.insertPos = (*ctx->text.source->Scan)(ctx->text.source, 
    	ctx->text.insertPos, XawstAll, XawsdLeft, 1, TRUE);
   EndAction(ctx);
}


static void MoveEndOfFile(ctx, event)
  TextWidget ctx;
   XEvent *event;
{
   StartAction(ctx, event);
    ctx->text.insertPos = (*ctx->text.source->Scan)(ctx->text.source, 
    	ctx->text.insertPos, XawstAll,  XawsdRight, 1, TRUE);
   EndAction(ctx);
}

static void ScrollOneLineUp(ctx, event)
  TextWidget ctx;
   XEvent *event;
{
   StartAction(ctx, event);
    _XawTextScroll(ctx, 1);
   EndAction(ctx);
}

static void ScrollOneLineDown(ctx, event)
  TextWidget ctx;
   XEvent *event;
{
   StartAction(ctx, event);
    _XawTextScroll(ctx, -1);
   EndAction(ctx);
}

static void MoveNextPage(ctx, event)
  TextWidget ctx;
   XEvent *event;
{
   StartAction(ctx, event);
    _XawTextScroll(ctx, Max(1, ctx->text.lt.lines - 2));
    ctx->text.insertPos = ctx->text.lt.top;
   EndAction(ctx);
}

static void MovePreviousPage(ctx, event)
  TextWidget ctx;
   XEvent *event;
{
   StartAction(ctx, event);
    _XawTextScroll(ctx, -Max(1, ctx->text.lt.lines - 2));
    ctx->text.insertPos = ctx->text.lt.top;
   EndAction(ctx);
}




/* delete routines */

static void DeleteForwardChar(ctx, event)
    TextWidget ctx;
   XEvent *event;
{
    XawTextPosition next;

   StartAction(ctx, event);
    next = (*ctx->text.source->Scan)(
            ctx->text.source, ctx->text.insertPos, XawstPositions, 
	    XawsdRight, 1, TRUE);
    DeleteOrKill(ctx, ctx->text.insertPos, next, FALSE);
   EndAction(ctx);
}

static void DeleteBackwardChar(ctx, event)
    TextWidget ctx;
   XEvent *event;
{
    XawTextPosition next;

   StartAction(ctx, event);
    next = (*ctx->text.source->Scan)(
            ctx->text.source, ctx->text.insertPos, XawstPositions, 
	    XawsdLeft, 1, TRUE);
    DeleteOrKill(ctx, next, ctx->text.insertPos, FALSE);
   EndAction(ctx);
}

static void DeleteForwardWord(ctx, event)
    TextWidget ctx;
   XEvent *event;
{
    XawTextPosition next;

   StartAction(ctx, event);
    next = NextPosition(ctx, ctx->text.insertPos, XawstWhiteSpace, XawsdRight);
    DeleteOrKill(ctx, ctx->text.insertPos, next, FALSE);
   EndAction(ctx);
}

static void DeleteBackwardWord(ctx, event)
    TextWidget ctx;
   XEvent *event;
{
    XawTextPosition next;

   StartAction(ctx, event);
    next = NextPosition(ctx, ctx->text.insertPos, XawstWhiteSpace, XawsdLeft);
    DeleteOrKill(ctx, next, ctx->text.insertPos, FALSE);
   EndAction(ctx);
}

static void KillForwardWord(ctx, event)
    TextWidget ctx;
   XEvent *event;
{
    XawTextPosition next;

   StartAction(ctx, event);
    next = NextPosition(ctx, ctx->text.insertPos, XawstWhiteSpace, XawsdRight);
    DeleteOrKill(ctx, ctx->text.insertPos, next, TRUE);
   EndAction(ctx);
}

static void KillBackwardWord(ctx, event)
    TextWidget ctx;
   XEvent *event;
{
    XawTextPosition next;

   StartAction(ctx, event);
    next = NextPosition(ctx, ctx->text.insertPos, XawstWhiteSpace, XawsdLeft);
    DeleteOrKill(ctx, next, ctx->text.insertPos, TRUE);
   EndAction(ctx);
}

static void KillCurrentSelection(ctx, event)
    TextWidget ctx;
   XEvent *event;
{
   StartAction(ctx, event);
    DeleteOrKill(ctx, ctx->text.s.left, ctx->text.s.right, TRUE);
   EndAction(ctx);
}

static void DeleteCurrentSelection(ctx, event)
    TextWidget ctx;
   XEvent *event;
{
   StartAction(ctx, event);
    DeleteOrKill(ctx, ctx->text.s.left, ctx->text.s.right, FALSE);
   EndAction(ctx);
}

static void KillToEndOfLine(ctx, event)
    TextWidget ctx;
   XEvent *event;
{
    int     line;
    XawTextPosition last, next;
   StartAction(ctx, event);
    _XawTextShowPosition(ctx);
    line = LineForPosition(ctx, ctx->text.insertPos);
    last = ctx->text.lt.info[line + 1].position;
    next = (*ctx->text.source->Scan)(ctx->text.source, ctx->text.insertPos,
       XawstEOL, XawsdRight, 1, FALSE);
    if (last > ctx->text.lastPos)
	last = ctx->text.lastPos;
    if (last > next && ctx->text.insertPos < next)
	last = next;
    DeleteOrKill(ctx, ctx->text.insertPos, last, TRUE);
   EndAction(ctx);
}

static void KillToEndOfParagraph(ctx, event)
    TextWidget ctx;
   XEvent *event;
{
    XawTextPosition next;

   StartAction(ctx, event);
    next = (*ctx->text.source->Scan)(ctx->text.source, ctx->text.insertPos,
				       XawstEOL, XawsdRight, 1, FALSE);
    if (next == ctx->text.insertPos)
	next = (*ctx->text.source->Scan)(ctx->text.source, next, XawstEOL,
					   XawsdRight, 1, TRUE);
    DeleteOrKill(ctx, ctx->text.insertPos, next, TRUE);
   EndAction(ctx);
}

static void InsertNewLineAndBackup(ctx, event)
  TextWidget ctx;
   XEvent *event;
{
   static int InsertNewLineAndBackupInternal();
   StartAction(ctx, event);
   InsertNewLineAndBackupInternal(ctx);
   EndAction(ctx);
}

static int InsertNewLineAndBackupInternal(ctx)
  TextWidget ctx;
{
    XawTextBlock text;
    text.length = 1;
    text.ptr = "\n";
    text.firstPos = 0;
    if (ReplaceText(ctx, ctx->text.insertPos, ctx->text.insertPos, &text)) {
	XBell( XtDisplay(ctx), 50);
	return(XawEditError);
    }
    XawTextUnsetSelection((Widget)ctx);
    ctx->text.showposition = TRUE;
    return(XawEditDone);
}



static int InsertNewLine(ctx, event)
    TextWidget ctx;
   XEvent *event;
{
    XawTextPosition next;

   StartAction(ctx, event);
    if (InsertNewLineAndBackupInternal(ctx))
	return(XawEditError);
    next = (*ctx->text.source->Scan)(ctx->text.source, ctx->text.insertPos,
	    XawstPositions, XawsdRight, 1, TRUE);
    ctx->text.insertPos = next;
   EndAction(ctx);
    return(XawEditDone);
}


static void InsertNewLineAndIndent(ctx, event)
  TextWidget ctx;
   XEvent *event;
{
    XawTextBlock text;
    XawTextPosition pos1, pos2;

   StartAction(ctx, event);
    pos1 = (*ctx->text.source->Scan)(ctx->text.source, ctx->text.insertPos, 
    	XawstEOL, XawsdLeft, 1, FALSE);
    pos2 = (*ctx->text.source->Scan)(ctx->text.source, pos1, XawstEOL, 
    	XawsdLeft, 1, TRUE);
    pos2 = (*ctx->text.source->Scan)(ctx->text.source, pos2, XawstWhiteSpace, 
    	XawsdRight, 1, TRUE);
    text.ptr = _XawTextGetText(ctx, pos1, pos2);
    text.length = strlen(text.ptr);
    if (InsertNewLine(ctx, event)) return;
    text.firstPos = 0;
    if (ReplaceText(ctx, ctx->text.insertPos, ctx->text.insertPos, &text)) {
	XBell(XtDisplay(ctx), 50);
	EndAction(ctx);
	return;
    }
    ctx->text.insertPos = (*ctx->text.source->Scan)(ctx->text.source, 
    	ctx->text.insertPos, XawstPositions, XawsdRight, text.length, TRUE);
    XtFree(text.ptr);
   EndAction(ctx);
}

static void SelectWord(ctx, event, params, num_params)
  TextWidget ctx;
   XEvent *event;
   String *params;
   Cardinal *num_params;
{
    XawTextPosition l, r;
   StartAction(ctx, event);
    l = (*ctx->text.source->Scan)(ctx->text.source, ctx->text.insertPos, 
    	XawstWhiteSpace, XawsdLeft, 1, FALSE);
    r = (*ctx->text.source->Scan)(ctx->text.source, l, XawstWhiteSpace, 
    	XawsdRight, 1, FALSE);
    _XawTextSetNewSelection(ctx, l, r,
			   _SelectionList(ctx, params, *num_params),
			   *num_params);
   EndAction(ctx);
}


static void SelectAll(ctx, event, params, num_params)
  TextWidget ctx;
   XEvent *event;
   String *params;
   Cardinal *num_params;
{
   StartAction(ctx, event);
   _XawTextSetNewSelection(ctx, (XawTextPosition)0, ctx->text.lastPos,
			  _SelectionList(ctx, params, *num_params),
			  *num_params);
   EndAction(ctx);
}

static void SelectStart(ctx, event, params, num_params)
  TextWidget ctx;
   XEvent *event;
   String *params;		/* unused */
   Cardinal *num_params;	/* unused */
{
   StartAction(ctx, event);
    AlterSelection(ctx, XawsmTextSelect, XawactionStart, NULL, ZERO);
   EndAction(ctx);
}

static void SelectAdjust(ctx, event, params, num_params)
  TextWidget ctx;
   XEvent *event;
   String *params;		/* unused */
   Cardinal *num_params;	/* unused */
{
   StartAction(ctx, event);
    AlterSelection(ctx, XawsmTextSelect, XawactionAdjust, NULL, ZERO);
   EndAction(ctx);
}

static void SelectEnd(ctx, event, params, num_params)
  TextWidget ctx;
   XEvent *event;
   String *params;
   Cardinal *num_params;
{
   StartAction(ctx, event);
    AlterSelection(ctx, XawsmTextSelect, XawactionEnd, params, num_params);
   EndAction(ctx);
}

static void ExtendStart(ctx, event, params, num_params)
  TextWidget ctx;
   XEvent *event;
   String *params;		/* unused */
   Cardinal *num_params;	/* unused */
{
   StartAction(ctx, event);
    AlterSelection(ctx, XawsmTextExtend, XawactionStart, NULL, ZERO);
   EndAction(ctx);
}

static void ExtendAdjust(ctx, event, params, num_params)
  TextWidget ctx;
   XEvent *event;
   String *params;		/* unused */
   Cardinal *num_params;	/* unused */
{
   StartAction(ctx, event);
    AlterSelection(ctx, XawsmTextExtend, XawactionAdjust, NULL, ZERO);
   EndAction(ctx);
}

static void ExtendEnd(ctx, event, params, num_params)
  TextWidget ctx;
   XEvent *event;
   String *params;
   Cardinal *num_params;
{
   StartAction(ctx, event);
    AlterSelection(ctx, XawsmTextExtend, XawactionEnd, params, num_params);
   EndAction(ctx);
}


static void RedrawDisplay(ctx, event)
  TextWidget ctx;
   XEvent *event;
{
   StartAction(ctx, event);
    ForceBuildLineTable(ctx);
    DisplayTextWindow((Widget)ctx);
   EndAction(ctx);
}


/* ARGSUSED */
void _XawTextAbortDialog(w, closure, call_data)
     Widget w;			/* unused */
     caddr_t closure;		/* dialog */
     caddr_t call_data;		/* unused */
{
   struct _dialog *dialog = (struct _dialog*)closure;
   Widget popup = dialog->widget->core.parent;
   TextWidget ctx = dialog->text; 

   StartAction(ctx, (XEvent*)NULL);
     XtPopdown(popup);
     dialog->mapped = False;
     if (dialog->message)
	 XtUnmanageChild( dialog->message );
   EndAction(ctx);
}


/* Insert a file of the given name into the text.  Returns 0 if file found, 
   -1 if not. */

static int InsertFileNamed(ctx, str)
  TextWidget ctx;
  char *str;
{
    int fid;
    XawTextBlock text;
    char    buf[1000];
    XawTextPosition position;

    if (str == NULL || strlen(str) == 0) return -1;
    fid = open(str, O_RDONLY);
    if (fid <= 0) return -1;
    _XawTextPrepareToUpdate(ctx);
    position = ctx->text.insertPos;
    text.firstPos = 0;
    while ((text.length = read(fid, buf, 512)) > 0) {
	text.ptr = buf;
	(void) ReplaceText(ctx, position, position, &text);
	position = (*ctx->text.source->Scan)(ctx->text.source, position, 
		XawstPositions, XawsdRight, text.length, TRUE);
    }
    (void) close(fid);
    ctx->text.insertPos = position;
    _XawTextExecuteUpdate(ctx);
    return 0;
}

/* ARGSUSED */
static void DoInsert(w, closure, call_data)
     Widget w;			/* unused */
     caddr_t closure;		/* text widget */
     caddr_t call_data;		/* unused */
{
    struct _dialog *dialog = (struct _dialog*)closure;

    if (InsertFileNamed( dialog->text,
			 XawDialogGetValueString(dialog->widget) )) {
	char msg[128];
	static Arg args[] = {
	    {XtNlabel, NULL},
	    {XtNfromVert, NULL},
	    {XtNleft, (XtArgVal)XtChainLeft},
	    {XtNright, (XtArgVal)XtChainRight},
	    {XtNborderWidth, 0},
	};
	sprintf( msg, "*** Error: %s ***",
		 (errno > 0 && errno < sys_nerr) ?
			sys_errlist[errno] : "Can't open file" );
	args[0].value = (XtArgVal)msg;
	if (dialog->message) {
	    XtSetValues( dialog->message, args, ONE );
	    XtManageChild( dialog->message );
	}
	else {
	    args[1].value = (XtArgVal)dialog->doit;
	    dialog->message =
		XtCreateManagedWidget( "message", labelWidgetClass,
				       dialog->widget, args, XtNumber(args) );
	}
/*	XBell(XtDisplay(w), 50); */
    }
    else {
	_XawTextAbortDialog(w, closure, NULL);
    }
}

/*ARGSUSED*/
static void TextFocusIn (ctx, event)
  TextWidget ctx;
   XEvent *event;
{ ctx->text.hasfocus = TRUE; }

/*ARGSUSED*/
static void TextFocusOut(ctx, event)
  TextWidget ctx;
   XEvent *event;
{ ctx->text.hasfocus = FALSE; }

#define STRBUFSIZE 100

static XComposeStatus compose_status = {NULL, 0};
static void InsertChar(ctx, event)
  TextWidget ctx;
   XEvent *event;
{
   char strbuf[STRBUFSIZE];
   int     keycode;
   XawTextBlock text;
   text.length = XLookupString (event, strbuf, STRBUFSIZE,
                &keycode, &compose_status);
   if (text.length==0) return;
   StartAction(ctx, event);
   text.ptr = &strbuf[0];
   text.firstPos = 0;
   if (ReplaceText(ctx, ctx->text.insertPos, ctx->text.insertPos, &text)) {
	XBell(XtDisplay(ctx), 50);
	EndAction(ctx);
	return;
    }
    ctx->text.insertPos =
	(*ctx->text.source->Scan)(ctx->text.source, ctx->text.insertPos,
			    XawstPositions, XawsdRight, text.length, TRUE);
   XawTextUnsetSelection((Widget)ctx);

   EndAction(ctx);
}

static void InsertFile(w, event)
    Widget w;
    XEvent *event;
{
    TextWidget ctx = (TextWidget)w;
    register struct _dialog *dialog, *prev;
    char *ptr;
    static char *dialog_label = "Insert File:";
#ifdef notdef
    XawTextBlock text;
#endif
    register Widget popup;
    static Arg popup_args[] = {
	{XtNx, NULL},
	{XtNy, NULL},
	{XtNiconName, NULL},
	{XtNgeometry, NULL},
	{XtNallowShellResize, True},
	{XtNsaveUnder, True},
    };
    Arg args[2];
    int x, y;
    Window j;

   StartAction(ctx, event);
    if (ctx->text.source->edit_mode != XawtextEdit) {
	XBell(XtDisplay(w), 50);
	EndAction(ctx);
	return;
    }
    if (ctx->text.s.left < ctx->text.s.right) {
	ptr = _XawTextGetText(ctx, ctx->text.s.left, ctx->text.s.right);
	DeleteCurrentSelection(ctx, (XEvent*)NULL);
#ifdef notdef
	if (InsertFileNamed(ctx, ptr)) {
	    XBell( XtDisplay(w), 50);
	    text.ptr = ptr;
	    text.length = strlen(ptr);
	    text.firstPos = 0;
	    (void) ReplaceText(ctx, ctx->text.insertPos, ctx->text.insertPos, &text);
	    ctx->text.s.left = ctx->text.insertPos;
	    ctx->text.s.right = ctx->text.insertPos = 
	      (*ctx->text.source->Scan)(ctx->text.source, ctx->text.insertPos, 
		  XawstPositions, XawsdRight, text.length, TRUE);
	}
	XtFree(ptr);
	EndAction(ctx);
	return;
#endif
    }
    else {
	ptr = "";
    }
    XTranslateCoordinates( XtDisplay(w), XtWindow(w),
			   RootWindowOfScreen(XtScreen(w)), 0, 0, &x, &y, &j );
    x += ctx->text.dialog_horiz_offset;
    y += ctx->text.dialog_vert_offset;
    if (ctx->text.sbar)
	x += ctx->text.sbar->core.width + ctx->text.sbar->core.border_width;
    prev = NULL;
    for (dialog = ctx->text.dialog; dialog; dialog = dialog->next) {
	if (!dialog->mapped)
	    break;
	x += ctx->text.dialog_horiz_offset;
	y += ctx->text.dialog_vert_offset;
	prev = dialog;
    }
    if (dialog) {
	_XawTextAbortDialog(w, (caddr_t)dialog, NULL);
	XtMoveWidget(popup = dialog->widget->core.parent, x, y);
    }
    else {
	XtCallbackRec callbacks[2];
	dialog = XtNew(struct _dialog);
	if (prev)
	    prev->next = dialog; /* add to end of list to make visual */
	else			 /* placement easier next time 'round */
	    ctx->text.dialog = dialog;
	dialog->text = ctx;
	dialog->message = (Widget)NULL;
	dialog->next = NULL;
	popup_args[0].value = (XtArgVal)x;
	popup_args[1].value = (XtArgVal)y;
	popup_args[2].value = (XtArgVal)dialog_label;
	popup = XtCreatePopupShell( "insertFile", transientShellWidgetClass, w,
				    popup_args, XtNumber(popup_args) );

	XtSetArg( args[0], XtNlabel, dialog_label );
	XtSetArg( args[1], XtNvalue, ptr ); 
	dialog->widget =
	    XtCreateManagedWidget("fileInsert", dialogWidgetClass, popup,
				  args, TWO);

	XtSetKeyboardFocus( dialog->widget,
			    XtNameToWidget( dialog->widget, "value" ));
	callbacks[0].callback = _XawTextAbortDialog;
	callbacks[0].closure = (caddr_t)dialog;
	callbacks[1].callback = (XtCallbackProc)NULL;
	callbacks[1].closure = (caddr_t)NULL;
	XtSetArg( args[0], XtNcallback, callbacks );
	XtCreateManagedWidget( "Cancel", commandWidgetClass, dialog->widget,
			       args, ONE );

	callbacks[0].callback = DoInsert;
	dialog->doit =
	    XtCreateManagedWidget( "DoIt", commandWidgetClass, dialog->widget,
				   args, ONE );

	XtRealizeWidget( popup );
    }
    XtPopup(popup, XtGrabNone);
    dialog->mapped = True;

   EndAction(ctx);
}

/*ARGSUSED*/
static void InsertString(w, event, params, paramCount)
   Widget w;
   XEvent *event;
   String *params;
   Cardinal *paramCount;
{
   TextWidget ctx = (TextWidget)w;
   char strbuf[STRBUFSIZE];
   int     keycode;
   XawTextBlock text;
   int	   i;
   XawTextPosition (*Scan)() = ctx->text.source->Scan;
   text.firstPos = 0;
   StartAction(ctx, event);
   for (i = *paramCount; i; i--, params++) {
      unsigned char hexval;
      if ((*params)[0] == '0' && (*params)[1] == 'x' && (*params)[2] != '\0') {
	  char c, *p;
	  hexval = 0;
	  for (p = *params+2; (c = *p); p++) {
	      hexval *= 16;
	      if (c >= '0' && c <= '9')
		  hexval += c - '0';
	      else if (c >= 'a' && c <= 'f')
		  hexval += c - 'a' + 10;
	      else if (c >= 'A' && c <= 'F')
		  hexval += c - 'A' + 10;
	      else break;
	  }
	  if (c == '\0') {
	      text.ptr = (char*)&hexval;
	      text.length = 1;
	  } else text.length = strlen(text.ptr = *params);
      } else text.length = strlen(text.ptr = *params);
      if (text.length == 0) continue;
      if (ReplaceText(ctx, ctx->text.insertPos, ctx->text.insertPos, &text)) {
	  XBell(XtDisplay(ctx), 50);
	  EndAction(ctx);
	  return;
      }
      ctx->text.insertPos =
	  (*Scan)(ctx->text.source, ctx->text.insertPos,
		  XawstPositions, XawsdRight, text.length, TRUE);
   }
   XawTextUnsetSelection((Widget)ctx);
   EndAction(ctx);
}


static void DisplayCaret(w, event, params, num_params)
   Widget w;
   XEvent *event;		/* CrossingNotify special-cased */
   String *params;		/* Off, False, No, On, True, Yes, etc. */
   Cardinal *num_params;	/* 0, 1 or 2 */
{
   TextWidget ctx = (TextWidget)w;
   Boolean display_caret = True;

   if (event->type == EnterNotify || event->type == LeaveNotify) {
       /* for Crossing events, the default case is to check the focus
	* field and only change the caret when focus==True.  A second
	* argument of "always" will cause the focus field to be ignored.
	*/
       Boolean check_focus = True;
       if (*num_params == 2 && strcmp(params[1], "always") == 0)
	   check_focus = False;
       if (check_focus && !event->xcrossing.focus) return;
   }

   if (*num_params > 0) {	/* default arg is "True" */
       XrmValue from, to;
       from.size = strlen(from.addr = params[0]);
       XtConvert(w, XtRString, &from, XtRBoolean, &to);
       if (to.addr != NULL) display_caret = *(Boolean*)to.addr;
       if (ctx->text.display_caret == display_caret) return;
   }
   StartAction(ctx, event);
   ctx->text.display_caret = display_caret;
   EndAction(ctx);
}

/* Actions Table */

XtActionsRec textActionsTable [] = {
/* motion bindings */
  {"forward-character", 	MoveForwardChar},
  {"backward-character", 	MoveBackwardChar},
  {"forward-word", 		MoveForwardWord},
  {"backward-word", 		MoveBackwardWord},
  {"forward-paragraph", 	MoveForwardParagraph},
  {"backward-paragraph", 	MoveBackwardParagraph},
  {"beginning-of-line", 	MoveToLineStart},
  {"end-of-line", 		MoveToLineEnd},
  {"next-line", 		MoveNextLine},
  {"previous-line", 		MovePreviousLine},
  {"next-page", 		MoveNextPage},
  {"previous-page", 		MovePreviousPage},
  {"beginning-of-file", 	MoveBeginningOfFile},
  {"end-of-file", 		MoveEndOfFile},
  {"scroll-one-line-up", 	ScrollOneLineUp},
  {"scroll-one-line-down", 	ScrollOneLineDown},
/* delete bindings */
  {"delete-next-character", 	DeleteForwardChar},
  {"delete-previous-character", DeleteBackwardChar},
  {"delete-next-word", 		DeleteForwardWord},
  {"delete-previous-word", 	DeleteBackwardWord},
  {"delete-selection", 		DeleteCurrentSelection},
/* kill bindings */
  {"kill-word", 		KillForwardWord},
  {"backward-kill-word", 	KillBackwardWord},
  {"kill-selection", 		KillCurrentSelection},
  {"kill-to-end-of-line", 	KillToEndOfLine},
  {"kill-to-end-of-paragraph", 	KillToEndOfParagraph},
/* unkill bindings */
  {"unkill", 			UnKill},
  {"stuff", 			Stuff},
/* new line stuff */
  {"newline-and-indent", 	InsertNewLineAndIndent},
  {"newline-and-backup", 	InsertNewLineAndBackup},
  {"newline", 			(XtActionProc)InsertNewLine},
/* Selection stuff */
  {"select-word", 		SelectWord},
  {"select-all", 		SelectAll},
  {"select-start", 		SelectStart},
  {"select-adjust", 		SelectAdjust},
  {"select-end", 		SelectEnd},
  {"extend-start", 		ExtendStart},
  {"extend-adjust", 		ExtendAdjust},
  {"extend-end", 		ExtendEnd},
  {"insert-selection",		InsertSelection},
/* Miscellaneous */
  {"redraw-display", 		RedrawDisplay},
  {"insert-file", 		InsertFile},
  {"insert-char", 		InsertChar},
  {"insert-string",		InsertString},
  {"focus-in", 	 	        TextFocusIn},
  {"focus-out", 		TextFocusOut},
  {"display-caret",		DisplayCaret},
};

Cardinal textActionsTableCount = XtNumber(textActionsTable); /* for subclasses */

TextClassRec textClassRec = {
  { /* core fields */
    /* superclass       */      (WidgetClass) &simpleClassRec,
    /* class_name       */      "Text",
    /* widget_size      */      sizeof(TextRec),
    /* class_initialize */      ClassInitialize,
    /* class_part_init  */	NULL,
    /* class_inited     */      FALSE,
    /* initialize       */      Initialize,
    /* initialize_hook  */	NULL,
    /* realize          */      Realize,
    /* actions          */      textActionsTable,
    /* num_actions      */      XtNumber(textActionsTable),
    /* resources        */      resources,
    /* num_ resource    */      XtNumber(resources),
    /* xrm_class        */      NULLQUARK,
    /* compress_motion  */      TRUE,
    /* compress_exposure*/      FALSE,
    /* compress_enterleave*/	TRUE,
    /* visible_interest */      FALSE,
    /* destroy          */      TextDestroy,
    /* resize           */      Resize,
    /* expose           */      ProcessExposeRegion,
    /* set_values       */      SetValues,
    /* set_values_hook  */	NULL,
    /* set_values_almost*/	XtInheritSetValuesAlmost,
    /* get_values_hook  */	NULL,
    /* accept_focus     */      NULL,
    /* version          */	XtVersion,
    /* callback_private */      NULL,
    /* tm_table         */      NULL,    /* set in ClassInitialize */
    /* query_geometry   */	XtInheritQueryGeometry,
    /* display_accelerator*/	XtInheritDisplayAccelerator,
    /* extension	*/	NULL
  },
  { /* text fields */
    /* empty            */	0
  }
};

WidgetClass textWidgetClass = (WidgetClass)&textClassRec;
