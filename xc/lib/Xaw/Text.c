#if (!defined(lint) && !defined(SABER))
static char Xrcsid[] = "$XConsortium: Text.c,v 1.95 89/07/18 15:38:37 kit Exp $";
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

#include <stdio.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xatom.h>

#include <X11/Xmu/Atoms.h>
#include <X11/Xmu/CharSet.h>
#include <X11/Xmu/StdSel.h>
#include <X11/Xmu/Misc.h>

#include <X11/Xaw/Cardinals.h>
#include <X11/Xaw/Scroll.h>
#include <X11/Xaw/TextP.h>

Atom FMT8BIT = NULL;

extern void bcopy();
extern int errno, sys_nerr;
extern char* sys_errlist[];

#define BIGNUM ((Dimension)32023)
#ifdef UTEK
# define MULTI_CLICK_TIME 5000	/* Brain Damaged server. */
#else
# define MULTI_CLICK_TIME 500
#endif

#define IsValidLine(ctx, num) (((num) == 0) || \
			       (ctx)->text.lt.info[(num)].position != 0)
/*
 * Defined in Text.c
 */

static void ScrollUpDownProc(), JumpProc(), ClearWindow(); 
static void DisplayTextWindow(), ModifySelection();
static Boolean LineAndXYForPosition();
static XawTextPosition FindGoodPosition(), _BuildLineTable();

void _XawTextAlterSelection(), _XawTextExecuteUpdate();
void _XawTextBuildLineTable();

/****************************************************************
 *
 * Full class record constant
 *
 ****************************************************************/

static XawTextSelectType defaultSelectTypes[] = {
  XawselectPosition, XawselectWord, XawselectLine, XawselectParagraph,
  XawselectAll,      XawselectNull,
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
  {XtNdisplayPosition, XtCTextPosition, XtRInt,
     sizeof(XawTextPosition), offset(text.lt.top), XtRImmediate, (caddr_t)0},
  {XtNinsertPosition, XtCTextPosition, XtRInt,
     sizeof(XawTextPosition), offset(text.insertPos), XtRImmediate,(caddr_t)0},
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
static void 
CvtStringToEditMode(args, num_args, fromVal, toVal)
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

#undef done

static void 
ClassInitialize()
{
  int len1 = strlen (_XawDefaultTextTranslations1);
  int len2 = strlen (_XawDefaultTextTranslations2);
  int len3 = strlen (_XawDefaultTextTranslations3);
  char *buf = XtMalloc (len1 + len2 + len3 + 1);
  char *cp = buf;

/* 
 * Set the number of actions.
 */

  textClassRec.core_class.num_actions = textActionsTableCount;
  
  strcpy (cp, _XawDefaultTextTranslations1); cp += len1;
  strcpy (cp, _XawDefaultTextTranslations2); cp += len2;
  strcpy (cp, _XawDefaultTextTranslations3);
  textWidgetClass->core_class.tm_table = buf;
  
  XtQTextRead   = XrmStringToQuark(XtEtextRead);
  XtQTextAppend = XrmStringToQuark(XtEtextAppend);
  XtQTextEdit   = XrmStringToQuark(XtEtextEdit);

  XtAddConverter(XtRString, XtREditMode, CvtStringToEditMode, NULL, 0);
}

static void 
CreateScrollbar(w)
TextWidget w;
{
  Arg args[1];
  Dimension bw;
  Widget sbar;

  XtSetArg(args[0], XtNheight, w->core.height);
  w->text.sbar = sbar =
    XtCreateWidget("scrollbar", scrollbarWidgetClass, w, args, ONE);
  XtAddCallback( sbar, XtNscrollProc, ScrollUpDownProc, (caddr_t)w );
  XtAddCallback( sbar, XtNjumpProc, JumpProc, (caddr_t)w );
  w->text.leftmargin += sbar->core.width + (bw = sbar->core.border_width);
  XtMoveWidget( sbar, -(Position)bw, -(Position)bw );
}

/* ARGSUSED */
static void 
Initialize(request, new)
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
  ctx->text.file_insert = NULL;
  ctx->text.search = NULL;
  ctx->text.updateFrom = (XawTextPosition *) XtMalloc(ONE);
  ctx->text.updateTo = (XawTextPosition *) XtMalloc(ONE);
  ctx->text.numranges = ctx->text.maxranges = 0;
  ctx->text.gc = DefaultGCOfScreen(XtScreen(ctx));
  ctx->text.hasfocus = FALSE;
  ctx->text.leftmargin = ctx->text.client_leftmargin;
  ctx->text.update_disabled = False;
  ctx->text.old_insert = -1;
  ctx->text.mult = 1;

  if (ctx->text.options & scrollVertical)
    CreateScrollbar(ctx);
}

static void 
Realize( w, valueMask, attributes )
Widget w;
Mask *valueMask;
XSetWindowAttributes *attributes;
{
  TextWidget ctx = (TextWidget)w;
  
  *valueMask |= CWBitGravity;
  attributes->bit_gravity = NorthWestGravity;
  
  (*textClassRec.core_class.superclass->core_class.realize)
    (w, valueMask, attributes);
  
  if (ctx->text.sbar != NULL) {
    XtRealizeWidget(ctx->text.sbar);
    XtMapWidget(ctx->text.sbar);
  }
  _XawTextBuildLineTable(ctx, ctx->text.lt.top, TRUE);
}

/* Utility routines for support of Text */

static void
_CreateCutBuffers(d)
Display *d;
{
  static struct _DisplayRec {
    struct _DisplayRec *next;
    Display *dpy;
  } *dpy_list = NULL;
  struct _DisplayRec *dpy_ptr;

  for (dpy_ptr = dpy_list; dpy_ptr != NULL; dpy_ptr = dpy_ptr->next)
    if (dpy_ptr->dpy == d) return;

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

/*
 * Procedure to manage insert cursor visibility for editable text.  It uses
 * the value of ctx->insertPos and an implicit argument. In the event that
 * position is immediately preceded by an eol graphic, then the insert cursor
 * is displayed at the beginning of the next line.
*/
static void 
InsertCursor (w, state)
Widget w;
XawTextInsertState state;
{
  TextWidget ctx = (TextWidget)w;
  Position x, y;
  int dy, line, visible;
  XawTextBlock text;
  XawTextSource src = ctx->text.source;
  
  if (ctx->text.lt.lines < 1) return;

  visible = LineAndXYForPosition(ctx, ctx->text.insertPos, &line, &x, &y);
  if (line < ctx->text.lt.lines)
    dy = (ctx->text.lt.info[line + 1].y - ctx->text.lt.info[line].y) + 1;
  else
    dy = (ctx->text.lt.info[line].y - ctx->text.lt.info[line - 1].y) + 1;

/*
 * If the insert position is just after eol then put it on next line 
 *
 * Reading the source is bogus and this code should use scan 
 */

  if ( (x > ctx->text.leftmargin) && (ctx->text.insertPos > 0) &&
      (ctx->text.insertPos >= ctx->text.lastPos) ) {
    (*src->Read) (src, ctx->text.insertPos - 1, &text, 1);
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
 * updates. 
*/

void
_XawTextNeedsUpdating(ctx, left, right)
TextWidget ctx;
XawTextPosition left, right;
{
  int i;
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

char *
_XawTextGetText(ctx, left, right)
TextWidget ctx;
XawTextPosition left, right;
{
  char *result, *tempResult;
  XawTextBlock text;
  XawTextSource src = ctx->text.source;
  Cardinal length = (Cardinal) (right - left);

  tempResult = result = XtMalloc(length + ONE);	/* leave space for '\0'. */
  while (left < right) {
    left = (*src->Read)(src, left, &text, right - left);
    (void) strncpy(tempResult, text.ptr, text.length);
    tempResult += text.length;
  }
  result[(int) length] = '\0';	/* NULL terminate this sucker */
  return(result);
}


/* 
 * This routine maps an x and y position in a window that is displaying text
 * into the corresponding position in the source.
 *
 * NOTE: it is illegal to call this routine unless there is a valid line table!
 */

/*** figure out what line it is on ***/

static XawTextPosition
PositionForXY (ctx, x, y)
TextWidget ctx;
Position x,y;
{
  int width, fromx, line;
  XawTextPosition position, resultstart, resultend;
  XawTextSource src = ctx->text.source;

  if (ctx->text.lt.lines == 0) return 0;
  
  for (line = 0; line < ctx->text.lt.lines - 1; line++) {
    if (y <= ctx->text.lt.info[line + 1].y)
      break;
  }
  position = ctx->text.lt.info[line].position;
  if (position >= ctx->text.lastPos)
    return(ctx->text.lastPos);
  fromx = ctx->text.lt.info[line].x;	/* starting x in line */
  width = x - fromx;			/* num of pix from starting of line */
  (*ctx->text.sink->Resolve) ( (Widget) ctx, position, fromx, 
			      width, &resultstart, &resultend);
  if (resultstart >= ctx->text.lt.info[line + 1].position)
    resultstart = (*src->Scan)(src, ctx->text.lt.info[line + 1].position,
			       XawstPositions, XawsdLeft, 1, TRUE);
  return resultstart;
}

/*
 * This routine maps a source position in to the corresponding line number
 * of the text that is displayed in the window.
 *
 * NOTE: It is illegal to call this routine unless there is a valid line table!
 */

static int 
LineForPosition (ctx, position)
TextWidget ctx;
XawTextPosition position;
{
  int line;
  
  for (line = 0; line < ctx->text.lt.lines; line++)
    if (position < ctx->text.lt.info[line + 1].position)
      break;
  return(line);
}

/*
 * This routine maps a source position into the corresponding line number
 * and the x, y coordinates of the text that is displayed in the window.
 *
 * NOTE: It is illegal to call this routine unless there is a valid line table!
 */

static Boolean
LineAndXYForPosition (ctx, pos, line, x, y)
TextWidget ctx;
XawTextPosition pos;
int *line;
Position *x, *y;
{
  XawTextPosition linePos, endPos;
  Boolean visible;
  int realW, realH;

  *line = 0;
  *x = ctx->text.leftmargin;
  *y = yMargin;
  if (visible = IsPositionVisible(ctx, pos)) {
    *line = LineForPosition(ctx, pos);
    *y = ctx->text.lt.info[*line].y;
    *x = ctx->text.lt.info[*line].x;
    linePos = ctx->text.lt.info[*line].position;
    (*ctx->text.sink->FindDistance)((Widget)ctx, linePos,
				    *x, pos, &realW, &endPos, &realH);
    *x += realW;
  }
  return(visible);
}

/*
 * This routine builds a line table. It does this by starting at the
 * specified position and measuring text to determine the staring position
 * of each line to be displayed. It also determines and saves in the
 * linetable all the required metrics for displaying a given line (e.g.
 * x offset, y offset, line length, etc.).
 */

void 
_XawTextBuildLineTable (ctx, position, force_rebuild)
TextWidget ctx;
XawTextPosition position;	/* top. */
Boolean force_rebuild;
{
  int lines = (*ctx->text.sink->MaxLines)((Widget)ctx, ctx->core.height);
  Cardinal size = sizeof(XawTextLineTableEntry) * (lines + 1);

  if ( (lines != ctx->text.lt.lines) || (ctx->text.lt.info == NULL) ) {
    ctx->text.lt.info = (XawTextLineTableEntry *) XtRealloc((char *) ctx->text.
							    lt.info, size);
    ctx->text.lt.lines = lines;
    force_rebuild = TRUE;
  }

  if ( force_rebuild || (position != ctx->text.lt.top) ) {
    bzero((char *) ctx->text.lt.info, size);
    (void) _BuildLineTable(ctx, ctx->text.lt.top = position, zeroPosition, 0);
  }
}

/*
 * This assumes that the line table does not change size.
 */

static XawTextPosition
_BuildLineTable(ctx, position, min_pos, line)
TextWidget ctx;
XawTextPosition position, min_pos;	
int line;
{
  XawTextLineTableEntry * lt = ctx->text.lt.info + line;
  XawTextPosition endPos;
  Position x, y;
  int count, width, realW, realH, options = ctx->text.options;
  int (*FindPosition)() = ctx->text.sink->FindPosition;
  XawTextSource src = ctx->text.source;

  x = ctx->text.leftmargin;
  width = (options & resizeWidth) ? BIGNUM : ctx->core.width - x;

  if (line == 0)
    y = yMargin;		/* #defined in TextP.h */
  else 
    y = lt->y;

  while ( TRUE ) {
    lt->x = x;
    lt->y = y;
    lt->position = position;
    (*FindPosition)(ctx, position, x, width,
		    (options & wordBreak), &endPos, &realW, &realH);
    /*
     * If lineWrap or wordBreak are on, skip remainder of this line 
     */
    
    if ( !(options & (wordBreak | lineWrap)) ) 
      endPos = (*src->Scan)(src, position, XawstEOL, XawsdRight, 1, TRUE);
    lt->endX = realW + x;
    y += realH;

    if ( endPos == ctx->text.lastPos) { /* We have reached the end. */
      if((*src->Scan)(src, position, XawstEOL, XawsdRight, 1, FALSE) == endPos)
	break;
    }

    ++lt;
    ++line;
    if ( (line > ctx->text.lt.lines) ||
	 ((lt->position == (position = endPos)) && (position > min_pos)) )
      return(position);
  }

/*
 * If we are at the end of the buffer put two special lines in the table.
 *
 * a) Both have position > text.lastPos and lt->x = lt->endX.
 * b) The first has a real height, and the second has a height that
 *    is the rest of the screen.
 *
 * I counld fill in the rest of the table with valid heights and a large
 * lastPos, but this method keeps the number of fill regions down to a 
 * minimum.
 *
 * One valid endty is needed at the end of the table so that the cursor
 * does not jump off the bottom of the window.
 */

  for ( count = 0; count < 2 ; count++) 
    if (line++ < ctx->text.lt.lines) { /* make sure not to run of the end. */
      (++lt)->y = (count == 0) ? y : ctx->core.height;
      lt->endX = lt->x = x;
      lt->position = ctx->text.lastPos + 100;
    }

  if (line < ctx->text.lt.lines) /* Clear out rest of table. */
    bzero( (char *) (lt + 1), 
	  (ctx->text.lt.lines - line) * sizeof(XawTextLineTableEntry) );

  ctx->text.lt.info[ctx->text.lt.lines].position = lt->position;

  return(endPos);
}

/*
 * This routine is used by Text to notify an associated scrollbar of the
 * correct metrics (position and shown fraction) for the text being currently
 * displayed in the window.
 */

static void 
SetScrollBar(ctx)
TextWidget ctx;
{
  float first, last;

  if (ctx->text.sbar == NULL) 
    return;

/*
 * Just an approximation 
 */

  if ( (ctx->text.lastPos > 0) && (ctx->text.lt.lines > 0)) {
    first = ctx->text.lt.top;
    first /= (float) ctx->text.lastPos; 
    last = ctx->text.lt.info[ctx->text.lt.lines].position;
    last /= (float) ctx->text.lastPos;
  }
  else {
    first = 0.0;
    last = 1.0;
  }
  XawScrollBarSetThumb(ctx->text.sbar, first, last - first);
}

/*
 * The routine will scroll the displayed text by lines.  If the arg  is
 * positive, move up; otherwise, move down. [note: this is really a private
 * procedure but is used in multiple modules].
 */

void
_XawTextScroll(ctx, n)
TextWidget ctx;
int n;			
{
  XawTextPosition top, target;
  int (*ClearToBG)() = ctx->text.sink->ClearToBackground;
  int y;
  XawTextLineTable * lt = &(ctx->text.lt);

  if (abs(n) > ctx->text.lt.lines) 
    n = (n > 0) ? ctx->text.lt.lines : -ctx->text.lt.lines;

  if (n == 0) return;

  if (n > 0) {
    if ( IsValidLine(ctx, n) )
      top = Min(lt->info[n].position, ctx->text.lastPos);
    else
      top = ctx->text.lastPos;

    _XawTextBuildLineTable(ctx, top, FALSE);
    y = IsValidLine(ctx, n) ? lt->info[n].y : ctx->core.height;
    if (top >= ctx->text.lastPos)
      DisplayTextWindow( (Widget) ctx);
    else {
      XCopyArea(XtDisplay(ctx), XtWindow(ctx), XtWindow(ctx), ctx->text.gc,
		0, y, (int)ctx->core.width, (int)ctx->core.height - y,
		0, lt->info[0].y);
      (*ClearToBG)(ctx, 0, lt->info[0].y + ctx->core.height - y,
		   (int) ctx->core.width, (int) ctx->core.height);

      if (n < lt->lines) n++; /* update descenders at bottom */
      _XawTextNeedsUpdating(ctx, lt->info[lt->lines - n].position, 
			    ctx->text.lastPos);
      SetScrollBar(ctx);
    }
  } 
  else {
    XawTextPosition updateTo;
    unsigned int height;

    n = -n;
    target = lt->top;
    top = (*ctx->text.source->Scan)(ctx->text.source, target, XawstEOL,
				    XawsdLeft, n+1, FALSE);
    _XawTextBuildLineTable(ctx, top, FALSE);

    y = IsValidLine(ctx, n) ? lt->info[n].y : ctx->core.height;
    updateTo = IsValidLine(ctx, n) ? lt->info[n].position : ctx->text.lastPos;
    height = IsValidLine(ctx, lt->lines - n) ? lt->info[lt->lines-n].y - 1
	                                     : ctx->core.height;

    if ( updateTo == target ) {
      XCopyArea(XtDisplay(ctx), XtWindow(ctx), XtWindow(ctx), ctx->text.gc, 
		0, lt->info[0].y, (int)ctx->core.width, height, 0, y);
      (*ClearToBG)(ctx, 0, lt->info[0].y, (int)ctx->core.width, y - 1);
      
      _XawTextNeedsUpdating(ctx, lt->info[0].position, updateTo);
      SetScrollBar(ctx);
    } 
    else if (lt->top != target)
      DisplayTextWindow((Widget)ctx);
  }
}

/*
 * The routine will scroll the displayed text by pixels.  If the calldata is
 * positive, move up; otherwise, move down.
 */

/*ARGSUSED*/
static void 
ScrollUpDownProc (w, closure, callData)
Widget w;
caddr_t closure;		/* TextWidget */
caddr_t callData;		/* #pixels */
{
  TextWidget ctx = (TextWidget)closure;
  int apix, a;			/* 'a' is the number of lines to scroll. */

  _XawTextPrepareToUpdate(ctx);
  apix = abs((int)callData);
  for (a= 1; a < ctx->text.lt.lines && apix > ctx->text.lt.info[a + 1].y; a++);
  if ( ((int) callData) >= 0)
    _XawTextScroll(ctx, a);
  else
    _XawTextScroll(ctx, -a);
  _XawTextExecuteUpdate(ctx);
}

/*
 * The routine "thumbs" the displayed text. Thumbing means reposition the
 * displayed view of the source to a new position determined by a fraction
 * of the way from beginning to end. Ideally, this should be determined by
 * the number of displayable lines in the source. This routine does it as a
 * fraction of the first position and last position and then normalizes to
 * the start of the line containing the position.
 *
 * BUG/deficiency: The normalize to line portion of this routine will
 * cause thumbing to always position to the start of the source.
 */

/*ARGSUSED*/
static void 
JumpProc (w, closure, callData)
Widget w;
caddr_t closure, callData; /* closuer = TextWidget, callData = percent. */
{
  float * percent = (float *) callData;
  TextWidget ctx = (TextWidget)closure;
  XawTextPosition position, old_top, old_bot;
  XawTextLineTable * lt = &(ctx->text.lt);
  XawTextSource src = ctx->text.source;

  _XawTextPrepareToUpdate(ctx);
  old_top = lt->top;
  if (IsValidLine(ctx, lt->lines - 1))
    old_bot = lt->info[lt->lines - 1].position;
  else
    old_bot = ctx->text.lastPos;

  position = (long) (*percent * (float) ctx->text.lastPos);
  position = (*src->Scan)(src, position, XawstEOL, XawsdLeft, 1, FALSE);
  if ( (position >= old_top) && (position <= old_bot) ) {
    int line = 0;
    for (;(line < lt->lines) && (position > lt->info[line].position) ; line++);
    _XawTextScroll(ctx, line);
  }
  else {
    XawTextPosition new_bot;
    _XawTextBuildLineTable(ctx, position, FALSE);
    new_bot = IsValidLine(ctx, lt->lines-1) ? lt->info[lt->lines-1].position 
                                            : ctx->text.lastPos;

    if ((old_top >= lt->top) && (old_top <= new_bot)) {
      int line = 0;
      for (;(line < lt->lines) && (old_top > lt->info[line].position); line++);
      _XawTextBuildLineTable(ctx, old_top, FALSE);
      _XawTextScroll(ctx, -line);
    }
    else 
      DisplayTextWindow( (Widget) ctx);
  }
  _XawTextExecuteUpdate(ctx);
}


static Boolean 
ConvertSelection(w, selection, target, type, value, length, format)
Widget w;
Atom *selection, *target, *type;
caddr_t *value;
unsigned long *length;
int *format;
{
  Display* d = XtDisplay(w);
  TextWidget ctx = (TextWidget)w;
  XawTextSource src = ctx->text.source;
  Boolean (*nullproc)() = NULL, (*CvtSel)() = src->ConvertSelection;

  if (*target == XA_TARGETS(d)) {
    Atom* targetP, * std_targets;
    unsigned long std_length;

    if ( (CvtSel == nullproc) || !(*CvtSel)(d, src, selection, target, 
					    type, value, length, format) ) {
      *value = NULL;
      *length = 0;
    }
    XmuConvertStandardSelection(w, ctx->text.time, selection, 
				target, type, (caddr_t*)&std_targets,
				&std_length, format);
    
    *value = XtRealloc(*value, sizeof(Atom)*(std_length + 6 + *length));
    targetP = *(Atom**)value + *length;
    *length += std_length + 5;
    *targetP++ = XA_STRING;
    *targetP++ = XA_TEXT(d);
    *targetP++ = XA_LENGTH(d);
    *targetP++ = XA_LIST_LENGTH(d);
    *targetP++ = XA_CHARACTER_POSITION(d);
    if (src->edit_mode == XawtextEdit) {
      *targetP++ = XA_DELETE(d);
      (*length)++;
    }
    bcopy((char*)std_targets, (char*)targetP, sizeof(Atom)*std_length);
    XtFree((char*)std_targets);
    *type = XA_ATOM;
    *format = 32;
    return True;
  }
  
  if ( (CvtSel != NULL) && (*CvtSel)(d, src, selection, target, type,
				     value, length, format) )
    return True;
  
  if (*target == XA_STRING || *target == XA_TEXT(d)) {
    *type = XA_STRING;
    *value = _XawTextGetText(ctx, ctx->text.s.left, ctx->text.s.right);
    *length = (long) strlen(*value);
    *format = 8;
    return True;
  }

  if ( (*target == XA_LIST_LENGTH(d)) || (*target == XA_LENGTH(d)) ) {
    long * temp;
    
    temp = (long *) XtMalloc(sizeof(long));
    if (*target == XA_LIST_LENGTH(d))
      *temp = 1L;
    else			/* *target == XA_LENGTH(d) */
      *temp = (long) (ctx->text.s.right - ctx->text.s.left);
    
    *value = (caddr_t) temp;
    *type = XA_INTEGER;
    *length = 1L;
    *format = 32;
    return True;
  }

  if (*target == XA_CHARACTER_POSITION(d)) {
    long * temp;
    
    temp = (long *) XtMalloc(2 * sizeof(long));
    temp[1] = (long) (ctx->text.s.left + 1);
    temp[2] = ctx->text.s.right;
    *value = (caddr_t) temp;
    *type = XA_SPAN(d);
    *length = 2L;
    *format = 32;
    return True;
  }

  if (*target == XA_DELETE(d)) {
    void _XawTextZapSelection(); /* From TextAction.c */
    
    _XawTextZapSelection( ctx, (XEvent *) NULL, TRUE);
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

/*	Function Name: GetCutBuffferNumber
 *	Description: Returns the number of the cut buffer.
 *	Arguments: atom - the atom to check.
 *	Returns: the number of the cut buffer representing this atom or
 *               NOT_A_CUT_BUFFER.
 */

#define NOT_A_CUT_BUFFER -1

static int
GetCutBufferNumber(atom)
register Atom atom;
{
  if (atom == XA_CUT_BUFFER0) return(0);
  if (atom == XA_CUT_BUFFER1) return(1);
  if (atom == XA_CUT_BUFFER2) return(2);
  if (atom == XA_CUT_BUFFER3) return(3);
  if (atom == XA_CUT_BUFFER4) return(4);
  if (atom == XA_CUT_BUFFER5) return(5);
  if (atom == XA_CUT_BUFFER6) return(6);
  if (atom == XA_CUT_BUFFER7) return(7);
  return(NOT_A_CUT_BUFFER);
}

static void 
LoseSelection(w, selection)
Widget w;
Atom *selection;
{
  TextWidget ctx = (TextWidget) w;
  register Atom* atomP;
  register int i;

  _XawTextPrepareToUpdate(ctx);

  atomP = ctx->text.s.selections;
  for (i = 0 ; i < ctx->text.s.atom_count; i++, atomP++)
    if ( (*selection == *atomP) || 
	(GetCutBufferNumber(*atomP) == NOT_A_CUT_BUFFER) )
      *atomP = (Atom)0;

  while (ctx->text.s.atom_count &&
	 ctx->text.s.selections[ctx->text.s.atom_count-1] == 0)
    ctx->text.s.atom_count--;

/*
 * Must walk the selection list in opposite order from UnsetSelection.
 */

  atomP = ctx->text.s.selections;
  for (i = 0 ; i < ctx->text.s.atom_count; i++, atomP++)
    if (*atomP == (Atom)0) {
      *atomP = ctx->text.s.selections[--ctx->text.s.atom_count];
      while (ctx->text.s.atom_count &&
	     ctx->text.s.selections[ctx->text.s.atom_count-1] == 0)
	ctx->text.s.atom_count--;
    }
  
  if (ctx->text.s.atom_count == 0)
    ModifySelection(ctx, ctx->text.insertPos, ctx->text.insertPos);
      
  if (ctx->text.old_insert >= 0) /* Update in progress. */
    _XawTextExecuteUpdate(ctx);
}

void 
_SetSelection(ctx, left, right, selections, count)
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

  if (ctx->text.source->SetSelection != nullProc) 
    (*ctx->text.source->SetSelection)(ctx->text.source, left, right,
				      (count == 0) ? NULL : selections[0]);

  if (left < right) {
    Widget w = (Widget) ctx;
    int buffer;
    
    while (count) {
      Atom selection = selections[--count];
/*
 * If this is a cut buffer.
 */

      if ((buffer = GetCutBufferNumber(selection)) != NOT_A_CUT_BUFFER) {
	char *ptr = _XawTextGetText(ctx, ctx->text.s.left, ctx->text.s.right);
	if (buffer == 0) {
	  _CreateCutBuffers(XtDisplay(w));
	  XRotateBuffers(XtDisplay(w), 1);
	}
	XStoreBuffer(XtDisplay(w), ptr, Min(strlen(ptr), MAXCUT), buffer);
	XtFree (ptr);
      }
      else			/* This is a real selection. */
	XtOwnSelection(w, selection, ctx->text.time,
		       ConvertSelection, LoseSelection, NULL);
    }
  }
  else
    XawTextUnsetSelection((Widget)ctx);
}

/*
 * This internal routine deletes the text from pos1 to pos2 in a source and
 * then inserts, at pos1, the text that was passed. As a side effect it
 * "invalidates" that portion of the displayed text (if any).
 *
 * NOTE: It is illegal to call this routine unless there is a valid line table!
 */

int 
_XawTextReplace (ctx, pos1, pos2, text)
TextWidget ctx;
XawTextPosition pos1, pos2;
XawTextBlock *text;
{
  int i, line1, delta, error;
  XawTextPosition updateFrom, updateTo;
  XawTextSource src = ctx->text.source;
  XawTextPosition (*Scan)() = src->Scan;
  
  ctx->text.update_disabled = True; /* No redisplay during replacement. */

/*
 * The insertPos may not always be set to the right spot in XawtextAppend 
 */

  if ((pos1 == ctx->text.insertPos) && (src->edit_mode == XawtextAppend)) {
    ctx->text.insertPos = ctx->text.lastPos;
    pos2 += ctx->text.insertPos - pos1;
    pos1 = ctx->text.insertPos;
  }

  updateFrom = (*Scan)(src, pos1, XawstWhiteSpace, XawsdLeft, 1, TRUE);
  updateFrom = (*Scan)(src, updateFrom, XawstPositions, XawsdLeft, 1, TRUE);
  updateFrom = Max(updateFrom, ctx->text.lt.top);

  line1 = LineForPosition(ctx, updateFrom);
  if ( (error = (*src->Replace)(src, pos1, pos2, text)) != 0) {
    ctx->text.update_disabled = FALSE; /* rearm redisplay. */
    return(error);
  }

  ctx->text.lastPos = GETLASTPOS;
  if (ctx->text.lt.top >= ctx->text.lastPos) {
    _XawTextBuildLineTable(ctx, ctx->text.lastPos, FALSE);
    ClearWindow( (Widget) ctx);
    SetScrollBar(ctx);
    ctx->text.update_disabled = FALSE; /* rearm redisplay. */
    return error;
  }

  delta = text->length - (pos2 - pos1);
  if (delta < ctx->text.lastPos) {
    for (pos2 += delta, i = 0; i < ctx->text.numranges; i++) {
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

  if (delta != 0) {
    XawTextLineTableEntry *lineP;
    i = LineForPosition(ctx, pos1) + 1;
    for (lineP = ctx->text.lt.info + i; i <= ctx->text.lt.lines; i++, lineP++)
      lineP->position += delta;
  }
  
  /*
   * Now process the line table and fixup in case edits caused
   * changes in line breaks. If we are breaking on word boundaries,
   * this code checks for moving words to and from lines.
   */
  
  if (IsPositionVisible(ctx, updateFrom)) {
    updateTo = _BuildLineTable(ctx, 
			       ctx->text.lt.info[line1].position, pos1, line1);
    _XawTextNeedsUpdating(ctx, updateFrom, updateTo);
  }

  SetScrollBar(ctx);
  ctx->text.update_disabled = FALSE; /* rearm redisplay. */
  return error;
}

/*
 * This routine will display text between two arbitrary source positions.
 * In the event that this span contains highlighted text for the selection, 
 * only that portion will be displayed highlighted.
 *
 * NOTE: it is illegal to call this routine unless there
 *       is a valid line table! 
 */

static void 
DisplayText(w, pos1, pos2)
Widget w;
XawTextPosition pos1, pos2;
{
  TextWidget ctx = (TextWidget)w;
  int (*ClearToBG)() = ctx->text.sink->ClearToBackground;
  Position x, y;
  int height, line, i, lastPos = ctx->text.lastPos;
  XawTextPosition startPos, endPos;
  Boolean clear_eol;

  pos1 = (pos1 < ctx->text.lt.top) ? ctx->text.lt.top : pos1;
  pos2 = FindGoodPosition(ctx, pos2);
  if ( (pos1 >= pos2) || !LineAndXYForPosition(ctx, pos1, &line, &x, &y) )
    return;			/* line not visible, or pos1 >= pos2. */

  for ( startPos = pos1, i = line; i < ctx->text.lt.lines; i++) {
    if ( (endPos = ctx->text.lt.info[i + 1].position) > pos2) 
      clear_eol = ( (endPos = pos2) >= lastPos);
    else 
      clear_eol = True;

    height = ctx->text.lt.info[i + 1].y - ctx->text.lt.info[i].y;

    if ( (endPos > startPos) ) {
      if (x == ctx->text.leftmargin)
	 (*ClearToBG) (w, 0, y, ctx->text.leftmargin, height);

      if ( (startPos >= ctx->text.s.right) || (endPos <= ctx->text.s.left) ) 
	(*ctx->text.sink->Display) (w, x, y, startPos, endPos, FALSE);
      else if ((startPos >= ctx->text.s.left) && (endPos <= ctx->text.s.right))
	(*ctx->text.sink->Display) (w, x, y, startPos, endPos, TRUE);
      else {
	DisplayText(w, startPos, ctx->text.s.left);
	DisplayText(w, Max(startPos, ctx->text.s.left),
		    Min(endPos, ctx->text.s.right));
	DisplayText(w, ctx->text.s.right, endPos);
      }
    }
    startPos = endPos;
    if (clear_eol)
      (*ClearToBG)(w, ctx->text.lt.info[i].endX, y,(int)w->core.width, height);
    x = ctx->text.leftmargin;
    y = ctx->text.lt.info[i + 1].y;
    if ( !clear_eol || (y == ctx->core.height) )
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

static void 
DoSelection (ctx, pos, time, motion)
TextWidget ctx;
XawTextPosition pos;
Time time;
Boolean motion;
{
  XawTextPosition newLeft, newRight;
  XawTextSelectType newType, *sarray;
  XawTextSource src = ctx->text.source;

  if (motion)
    newType = ctx->text.s.type;
  else {
    if ( ((int) abs(time - ctx->text.lasttime) < MULTI_CLICK_TIME) &&
	 ((pos >= ctx->text.s.left) && (pos <= ctx->text.s.right))) {
      sarray = ctx->text.sarray;
      for (;*sarray != XawselectNull && *sarray != ctx->text.s.type; sarray++);

      if (*sarray == XawselectNull)
	newType = *(ctx->text.sarray);
      else
	newType = *(sarray + 1);
    } 
    else 			                      /* single-click event */
      newType = *(ctx->text.sarray);

    ctx->text.lasttime = time;
  }
  switch (newType) {
  case XawselectPosition: 
    newLeft = newRight = pos;
    break;
  case XawselectChar: 
    newLeft = pos;
    newRight = (*src->Scan)(src, pos, XawstPositions, XawsdRight, 1, FALSE);
    break;
  case XawselectWord: 
    newLeft = (*src->Scan)(src, pos, XawstWhiteSpace, XawsdLeft, 1, FALSE);
    newRight = (*src->Scan)(src, pos, XawstWhiteSpace, XawsdRight, 1, FALSE);
    break;
  case XawselectParagraph: 
    newLeft = (*src->Scan)(src, pos, XawstParagraph, XawsdLeft, 1, FALSE);
    newRight = (*src->Scan)(src, pos, XawstParagraph, XawsdRight, 1, FALSE);
    break;
  case XawselectLine: 
    newLeft = (*src->Scan)(src, pos, XawstEOL, XawsdLeft, 1, FALSE);
    newRight = (*src->Scan)(src, pos, XawstEOL, XawsdRight, 1, FALSE);
    break;
  case XawselectAll: 
    newLeft = (*src->Scan)(src, pos, XawstAll, XawsdLeft, 1, FALSE);
    newRight = (*src->Scan)(src, pos, XawstAll, XawsdRight, 1, FALSE);
    break;
  }

  if ( (newLeft != ctx->text.s.left) || (newRight != ctx->text.s.right)
      || (newType != ctx->text.s.type)) {
    ModifySelection(ctx, newLeft, newRight);
    if (pos - ctx->text.s.left < ctx->text.s.right - pos)
      ctx->text.insertPos = newLeft;
    else 
      ctx->text.insertPos = newRight;
    ctx->text.s.type = newType;
  }
  if (!motion) { /* setup so we can freely mix select extend calls*/
    ctx->text.origSel.type = ctx->text.s.type;
    ctx->text.origSel.left = ctx->text.s.left;
    ctx->text.origSel.right = ctx->text.s.right;
    if (pos >= ctx->text.s.left + ((ctx->text.s.right - ctx->text.s.left) / 2))
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

static void 
ExtendSelection (ctx, pos, motion)
TextWidget ctx;
XawTextPosition pos;
Boolean motion;
{
  XawTextScanDirection dir;
  XawTextPosition (*Scan)() = ctx->text.source->Scan;

  if (!motion) {		/* setup for extending selection */
    ctx->text.origSel.type = ctx->text.s.type;
    ctx->text.origSel.left = ctx->text.s.left;
    ctx->text.origSel.right = ctx->text.s.right;
    if (pos >= ctx->text.s.left + ((ctx->text.s.right - ctx->text.s.left) / 2))
      ctx->text.extendDir = XawsdRight;
    else
      ctx->text.extendDir = XawsdLeft;
  }
  else /* check for change in extend direction */
    if ((ctx->text.extendDir == XawsdRight && pos < ctx->text.origSel.left) ||
	(ctx->text.extendDir == XawsdLeft && pos > ctx->text.origSel.right)) {
      ctx->text.extendDir = (ctx->text.extendDir == XawsdRight) ?
	                                            XawsdLeft : XawsdRight;
      ModifySelection(ctx, ctx->text.origSel.left, 
		      ctx->text.origSel.right, NULL, ZERO);
    }

  dir = ctx->text.extendDir;
  switch (ctx->text.s.type) {
  case XawselectWord: 
    pos = (*Scan) (ctx->text.source, pos, XawstWhiteSpace, dir, 1, FALSE);
    break;
  case XawselectLine:
    pos = (*Scan) (ctx->text.source, pos, XawstEOL, dir, 1, dir == XawsdRight);
    break;
  case XawselectParagraph:
    pos = (*Scan) (ctx->text.source, pos, XawstParagraph, dir, 1, FALSE);
    break;
  case XawselectAll: 
    pos = ctx->text.insertPos;
  case XawselectPosition:	/* fall through. */
  default:
    break;
  }
  
  if (ctx->text.extendDir == XawsdRight)
    ModifySelection(ctx, ctx->text.s.left, pos);
  else
    ModifySelection(ctx, pos, ctx->text.s.right);

  ctx->text.insertPos = pos;
}


/*
 * Clear the window to background color.
 */

static void
ClearWindow (w)
Widget w;
{
  int (*ClearToBG)() = (((TextWidget)w)->text.sink->ClearToBackground);

  if (XtIsRealized(w))
    (*ClearToBG) (w, 0, 0, (int)w->core.width, (int)w->core.height);
}

/*
 * Internal redisplay entire window.
 * Legal to call only if widget is realized.
 */

static void
DisplayTextWindow (w)
Widget w;
{
  TextWidget ctx = (TextWidget) w;
  ClearWindow(w);
  _XawTextBuildLineTable(ctx, ctx->text.lt.top, FALSE);
  _XawTextNeedsUpdating(ctx, zeroPosition, ctx->text.lastPos);
  SetScrollBar(ctx);
}

/*
 * This routine checks to see if the window should be resized (grown or
 * shrunk) when text to be painted overflows to the right or
 * the bottom of the window. It is used by the keyboard input routine.
 */

void
_XawTextCheckResize(ctx)
TextWidget ctx;
{
  Widget w = (Widget) ctx;
  int line = 0, old_height;
  XtWidgetGeometry rbox;

  if (ctx->text.options & resizeWidth) {
    XawTextLineTableEntry *lt;
    rbox.width = 0;
    for (lt = ctx->text.lt.info; line < ctx->text.lt.lines; line++, lt++) 
      AssignMax(rbox.width, lt->endX);
    
    if (rbox.width > ctx->core.width) { /* Only get wider. */
      rbox.request_mode = CWWidth;
      if (XtMakeGeometryRequest(w, &rbox, &rbox) == XtGeometryAlmost)
	(void) XtMakeGeometryRequest(w, &rbox, &rbox);
    }
  }

  if (!(ctx->text.options & resizeHeight)) return;

  if (IsPositionVisible(ctx, ctx->text.lastPos))
    line = LineForPosition(ctx, ctx->text.lastPos);
  else
    line = ctx->text.lt.lines;
  
  if ( (line + 1) == ctx->text.lt.lines ) return;
  
  old_height = ctx->core.height;
  rbox.request_mode = CWHeight;
  rbox.height = (*ctx->text.sink->MaxHeight) (ctx, line + 1)+(2 * yMargin)+2;
  
  if (rbox.height < old_height) return; /* It will only get taller. */

  if (XtMakeGeometryRequest(w, &rbox, &rbox) == XtGeometryAlmost)
    if (XtMakeGeometryRequest(w, &rbox, &rbox) != XtGeometryYes)
      return;
  
  _XawTextBuildLineTable(ctx, ctx->text.lt.top, FALSE);
    
/*
 * Clear cruft from bottom margin 
 */

  if ( rbox.height < old_height ) {
    (*ctx->text.sink->ClearToBackground) (ctx, ctx->text.leftmargin,
			     ctx->text.lt.info[ctx->text.lt.lines].y,
		             (int) ctx->core.width, old_height - rbox.height);
  }
}

/*
 * Converts (params, num_params) to a list of atoms & caches the
 * list in the TextWidget instance.
 */

Atom*
_XawTextSelectionList(ctx, list, nelems)
TextWidget ctx;
String *list;
Cardinal nelems;
{
  Atom * sel = ctx->text.s.selections;

  if (nelems > ctx->text.s.array_size) {
    sel = (Atom *) XtRealloc(sel, sizeof(Atom) * nelems);
    ctx->text.s.array_size = nelems;
  }
  XmuInternStrings(XtDisplay((Widget)ctx), list, nelems, sel);
  ctx->text.s.atom_count = nelems;
  return(ctx->text.s.selections = sel);
}

/*	Function Name: SetSelection
 *	Description: Sets the current selection.
 *	Arguments: ctx - the text widget.
 *                 defaultSel - the default selection.
 *                 l, r - the left and right ends of the selection.
 *                 list, nelems - the selection list (as strings).
 *	Returns: none.
 *
 *  NOTE: if (ctx->text.s.left >= ctx->text.s.right) then the selection
 *        is unset.
 */

void
_XawTextSetSelection(ctx, l, r, list, nelems)
TextWidget ctx;
XawTextPosition l, r;
String *list;
Cardinal nelems;
{
  if (nelems == 0) {
    String defaultSel = "PRIMARY";
    list = &defaultSel;
    nelems = 1;
  }
  _SetSelection(ctx, l, r, _XawTextSelectionList(ctx, list, nelems), nelems);
}


/*	Function Name: ModifySelection
 *	Description: Modifies the current selection.
 *	Arguments: ctx - the text widget.
 *                 left, right - the left and right ends of the selection.
 *	Returns: none.
 *
 *  NOTE: if (ctx->text.s.left >= ctx->text.s.right) then the selection
 *        is unset.
 */

static void
ModifySelection(ctx, left, right)
TextWidget ctx;
XawTextPosition left, right;
{
  if (left == right) 
    ctx->text.insertPos = left;
  _SetSelection( ctx, left, right, NULL, ZERO);
}

/*
 * This routine is used to perform various selection functions. The goal is
 * to be able to specify all the more popular forms of draw-through and
 * multi-click selection user interfaces from the outside.
 */

void 
_XawTextAlterSelection (ctx, mode, action, params, num_params)
TextWidget ctx;
XawTextSelectionMode mode;   /* {XawsmTextSelect, XawsmTextExtend} */
XawTextSelectionAction action; /* {XawactionStart, 
				  XawactionAdjust, XawactionEnd} */
String	*params;
Cardinal	*num_params;
{
  XawTextPosition position;
  Boolean flag;

/*
 * This flag is used by TextPop.c:DoReplace() to determine if the selection
 * is okay to use, or if it has been modified.
 */
    
  if (ctx->text.search != NULL)
    ctx->text.search->selection_changed = TRUE;

  position = PositionForXY (ctx, (int) ctx->text.ev_x, (int) ctx->text.ev_y);

  flag = (action != XawactionStart);
  if (mode == XawsmTextSelect)
    DoSelection (ctx, position, ctx->text.time, flag);
  else			/* mode == XawsmTextExtend */
    ExtendSelection (ctx, position, flag);

  if (action == XawactionEnd) 
    _XawTextSetSelection(ctx, ctx->text.s.left, ctx->text.s.right,
			 params, *num_params);
}
 
/*
 * This routine processes all "expose region" XEvents. In general, its job
 * is to the best job at minimal re-paint of the text, displayed in the
 * window, that it can.
 */

static void
ProcessExposeRegion(w, event)
Widget w;
XEvent *event;
{
  TextWidget ctx = (TextWidget) w;
  XawTextPosition pos1, pos2, resultend;
  int line, x = event->xexpose.x, y = event->xexpose.y;
  int width = event->xexpose.width, height = event->xexpose.height;
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
      if ( (!IsValidLine(ctx, line)) || (info->y >= y + height) )
	break;
      (*ctx->text.sink->Resolve)(ctx, info->position, info->x,
				 x - info->x, &pos1, &resultend);
      (*ctx->text.sink->Resolve)(ctx, info->position, info->x,
				 x + width - info->x, &pos2, &resultend);
      pos2 = (*ctx->text.source->Scan)(ctx->text.source, pos2,
				       XawstPositions, XawsdRight, 1, TRUE);
      _XawTextNeedsUpdating(ctx, pos1, pos2);
      line++;
    }
  _XawTextExecuteUpdate(ctx);
}

/*
 * This routine does all setup required to syncronize batched screen updates
 */

int 
_XawTextPrepareToUpdate(ctx)
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

static 
void FlushUpdate(ctx)
TextWidget ctx;
{
  int i, w;
  XawTextPosition updateFrom, updateTo;
  if (!XtIsRealized((Widget)ctx)) {
    ctx->text.numranges = 0;
    return;
  }
  while (ctx->text.numranges > 0) {
    updateFrom = ctx->text.updateFrom[0];
    w = 0;
    for (i = 1 ; i < ctx->text.numranges ; i++) {
      if (ctx->text.updateFrom[i] < updateFrom) {
	updateFrom = ctx->text.updateFrom[i];
	w = i;
      }
    }
    updateTo = ctx->text.updateTo[w];
    ctx->text.numranges--;
    ctx->text.updateFrom[w] = ctx->text.updateFrom[ctx->text.numranges];
    ctx->text.updateTo[w] = ctx->text.updateTo[ctx->text.numranges];
    for (i = ctx->text.numranges - 1 ; i >= 0 ; i--) {
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
  int number, lines = ctx->text.lt.lines;
  XawTextPosition max_pos, top, first, second;

  if ( (!XtIsRealized((Widget)ctx)) || (ctx->text.lt.lines <= 0) )
    return;
  
  if (IsValidLine(ctx, lines))
    max_pos = ctx->text.lt.info[lines].position;
  else
    max_pos = ctx->text.lastPos;
  
  if ( (ctx->text.insertPos >= ctx->text.lt.top) &&
       ((ctx->text.insertPos < max_pos) || ( max_pos > ctx->text.lastPos)) ) 
    return;

  first = ctx->text.lt.top;
  if (IsValidLine(ctx, 1))
    second = ctx->text.lt.info[1].position;
  else
    second = max_pos;
      
  if (ctx->text.insertPos < first)
    number = 1;
  else
    number = lines;

  top = (*ctx->text.source->Scan)(ctx->text.source, ctx->text.insertPos,
				  XawstEOL, XawsdLeft, number, FALSE);

  _XawTextBuildLineTable(ctx, top, FALSE);
  while (ctx->text.insertPos >= ctx->text.lt.info[lines].position) {
    if (ctx->text.lt.info[lines].position > ctx->text.lastPos)
      break; 
    _XawTextBuildLineTable(ctx, ctx->text.lt.info[1].position, FALSE);
  }
  if (ctx->text.lt.top == second) {
    _XawTextBuildLineTable(ctx, first, FALSE);
    _XawTextScroll(ctx, 1);
  } else if (ctx->text.lt.info[1].position == first) {
    _XawTextBuildLineTable(ctx, first, FALSE);
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

void
_XawTextExecuteUpdate(ctx)
TextWidget ctx;
{
  if ( ctx->text.update_disabled || (ctx->text.old_insert < 0) ) 
    return;

  if((ctx->text.old_insert != ctx->text.insertPos) || (ctx->text.showposition))
    _XawTextShowPosition(ctx);
  FlushUpdate(ctx);
  InsertCursor((Widget)ctx, XawisOn);
  ctx->text.old_insert = -1;
}


static void 
TextDestroy(w)
Widget w;
{
  TextWidget ctx = (TextWidget)w;

  if (ctx->text.file_insert != NULL)
    XtDestroyWidget(ctx->text.file_insert);
  
  if (ctx->text.outer)
    (void) XtDestroyWidget(ctx->text.outer);
  if (ctx->text.sbar)
    (void) XtDestroyWidget(ctx->text.sbar);
  XtFree((char *)ctx->text.updateFrom);
  XtFree((char *)ctx->text.updateTo);
}

/*
 * by the time we are managed (and get this far) we had better
 * have both a source and a sink 
 */

static void
Resize(w)
Widget w;
{
  TextWidget ctx = (TextWidget) w;

  if (ctx->text.sbar) {
    Widget sbar = ctx->text.sbar;
    XtResizeWidget( sbar, sbar->core.width, ctx->core.height,
		   sbar->core.border_width );
  }
  _XawTextPrepareToUpdate(ctx);
  _XawTextBuildLineTable(ctx, ctx->text.lt.top, TRUE);
  _XawTextExecuteUpdate(ctx);
}

/*
 * This routine allow the application program to Set attributes.
 */

/*ARGSUSED*/
static Boolean 
SetValues(current, request, new)
Widget current, request, new;
{
  TextWidget oldtw = (TextWidget) current;
  TextWidget newtw = (TextWidget) new;
  Boolean    redisplay = FALSE;
  Boolean    display_caret = newtw->text.display_caret;
  
  newtw->text.display_caret = oldtw->text.display_caret;
  _XawTextPrepareToUpdate(newtw);
  newtw->text.display_caret = display_caret;
  
  if ((oldtw->text.options & scrollVertical) !=
      (newtw->text.options & scrollVertical)) {
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
      newtw->text.leftmargin +=	newtw->text.sbar->core.width +
	                          newtw->text.sbar->core.border_width;
    }
  }
  
  if (oldtw->text.source != newtw->text.source ||
      oldtw->text.sink != newtw->text.sink ||
      oldtw->text.lt.top != newtw->text.lt.top ||
      oldtw->text.leftmargin != newtw->text.leftmargin  ||
      ((oldtw->text.options & lineWrap)
       != (newtw->text.options & lineWrap)) ) 
  {
    _XawTextBuildLineTable(newtw, newtw->text.lt.top, TRUE);
    SetScrollBar(newtw);
    redisplay = TRUE;
  }

  if (oldtw->text.insertPos != newtw->text.insertPos)
    newtw->text.showposition = TRUE;
  
  _XawTextExecuteUpdate(newtw);
  return redisplay;
}

/*	Function Name: SetValuesHook
 *	Description: set the values in the text source and sink.
 *	Arguments: w - the text widget.
 *                 args - the arg list.
 *                 num_args - the number of args in the list.
 *	Returns: True if redisplay is needed.
 */

static Boolean
SetValuesHook(w, args, num_args)
Widget w;
ArgList args;
Cardinal * num_args;
{
  TextWidget ctx = (TextWidget) w;
  Boolean ret = FALSE;

  if (ctx->text.source->SetValuesHook != NULL) 
    ret = (*ctx->text.source->SetValuesHook) (ctx->text.source,
					      args, num_args);

#ifdef notdef
  if (ctx->text.sink->SetValuesHook != NULL) 
    ret = (*ctx->text.sink->SetValuesHook) (w, args, num_args);
#endif

  return(ret);
}

/*	Function Name: GetValuesHook
 *	Description: get the values in the text source and sink.
 *	Arguments: w - the text widget.
 *                 args - the arg list.
 *                 num_args - the number of args in the list.
 *	Returns: none.
 */

static void
GetValuesHook(w, args, num_args)
Widget w;
ArgList args;
Cardinal * num_args;
{
  TextWidget ctx = (TextWidget) w;

  if (ctx->text.source->GetValuesHook != NULL) 
    (void) (*ctx->text.source->GetValuesHook) (ctx->text.source,
					       args, num_args);

#ifdef notdef
  if (ctx->text.sink->GetValuesHook != NULL) 
    (void) (*ctx->text.sink->GetValuesHook) (w, args, num_args);
#endif
}

/*	Function Name: FindGoodPosition
 *	Description: Returns a valid position given any postition
 *	Arguments: pos - any position.
 *	Returns: a position between (0 and lastPos);
 */

static XawTextPosition
FindGoodPosition(ctx, pos)
TextWidget ctx;
XawTextPosition;
{
  if (pos < 0) return(0);
  return ( ((pos > ctx->text.lastPos) ? ctx->text.lastPos : pos) );
}

/*******************************************************************
The following routines provide procedural interfaces to Text window state
setting and getting. They need to be redone so than the args code can use
them. I suggest we create a complete set that takes the context as an
argument and then have the public version lookp the context and call the
internal one. The major value of this set is that they have actual application
clients and therefore the functionality provided is required for any future
version of Text.
********************************************************************/

void 
XawTextDisplay (w)
Widget w;
{
  if (!XtIsRealized(w)) return;
  
  _XawTextPrepareToUpdate( (TextWidget) w);
  DisplayTextWindow(w);
  _XawTextExecuteUpdate( (TextWidget) w);
}

void
XawTextSetSelectionArray(w, sarray)
Widget w;
XawTextSelectType *sarray;
{
  ((TextWidget)w)->text.sarray = sarray;
}

/*
 * This function is no longer needed, and is left only for
 * compatability with the String source.
 */

void
XawTextSetLastPos (w, lastPos)
Widget w;
XawTextPosition lastPos;
{
  TextWidget  ctx = (TextWidget) w;

  _XawTextPrepareToUpdate(ctx);

  (*ctx->text.source->SetLastPos)(ctx->text.source, lastPos);
  ctx->text.lastPos = GETLASTPOS;
  _XawTextBuildLineTable(ctx, ctx->text.lt.top, TRUE);
  if (XtIsRealized(w)) 
    DisplayTextWindow(w);

  _XawTextExecuteUpdate(ctx);
}

void
XawTextGetSelectionPos(w, left, right)
Widget w;
XawTextPosition *left, *right;
{
  *left = ((TextWidget) w)->text.s.left;
  *right = ((TextWidget) w)->text.s.right;
}


void 
XawTextSetSource(w, source, startPos)
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
  
  _XawTextBuildLineTable(ctx, ctx->text.lt.top, TRUE);
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

int 
XawTextReplace(w, startPos, endPos, text)
Widget w;
XawTextPosition  startPos, endPos;
XawTextBlock *text;
{
  TextWidget ctx = (TextWidget) w;
  XawTextPosition (*Scan)() = ctx->text.source->Scan;
  int result;

  _XawTextPrepareToUpdate(ctx);
  endPos = FindGoodPosition(ctx, endPos);
  startPos = FindGoodPosition(ctx, startPos);
  if ((result = _XawTextReplace(ctx, startPos, endPos, text)) == XawEditDone) {
    int delta = text->length - (endPos - startPos);
    if (ctx->text.insertPos >= (endPos + delta)) {
      XawTextScanDirection sd = (delta < 0) ? XawsdLeft : XawsdRight;
      ctx->text.insertPos = (*Scan) (ctx->text.source, ctx->text.insertPos,
				     XawstPositions, sd, abs(delta), TRUE);
    }
  }

  _XawTextCheckResize(ctx);
  _XawTextExecuteUpdate(ctx);
  
  return result;
}

XawTextPosition 
XawTextTopPosition(w)
Widget w;
{
  return( ((TextWidget) w)->text.lt.top );
}

void 
XawTextSetInsertionPoint(w, position)
Widget w;
XawTextPosition position;
{
  TextWidget ctx = (TextWidget) w;

  _XawTextPrepareToUpdate(ctx);
  ctx->text.insertPos = FindGoodPosition(ctx, position);
  ctx->text.showposition = TRUE;
  _XawTextExecuteUpdate(ctx);
}

XawTextPosition
XawTextGetInsertionPoint(w)
Widget w;
{
  return( ((TextWidget) w)->text.insertPos);
}

/*
 * NOTE: Must walk the selection list in opposite order from LoseSelection.
 */

void 
XawTextUnsetSelection(w)
Widget w;
{
  register TextWidget ctx = (TextWidget)w;
  int i = ctx->text.s.atom_count;

  while (i > 0) {
    Atom sel = ctx->text.s.selections[--i];
    if ( (sel != (Atom) 0) && (GetCutBufferNumber(sel) == NOT_A_CUT_BUFFER) ) {
      XtDisownSelection(w, sel);
      LoseSelection(w, &sel); /* in case it wasn't just called */
    }
  }
}

void 
XawTextChangeOptions(w, options)
Widget w;
int options;
{
  ((TextWidget) w)->text.options = options;
}

int 
XawTextGetOptions(w)
Widget w;
{
  return(((TextWidget) w)->text.options);
}

void
XawTextSetSelection (w, left, right)
Widget w;
XawTextPosition left, right;
{
  TextWidget ctx = (TextWidget) w;
  
  _XawTextPrepareToUpdate(ctx);
  _XawTextSetSelection(ctx, FindGoodPosition(ctx, left),
	       FindGoodPosition(ctx, right), NULL, ZERO);
  _XawTextExecuteUpdate(ctx);
}

void 
XawTextInvalidate(w, from, to)
Widget w;
XawTextPosition from,to;
{
  TextWidget ctx = (TextWidget) w;

  from = FindGoodPosition(ctx, from);
  to = FindGoodPosition(ctx, to);
  ctx->text.lastPos = GETLASTPOS;
  _XawTextPrepareToUpdate(ctx);
  _XawTextNeedsUpdating(ctx, from, to);
  _XawTextBuildLineTable(ctx, ctx->text.lt.top, TRUE);
  _XawTextExecuteUpdate(ctx);
}

/*ARGSUSED*/
void 
XawTextDisableRedisplay(w)
Widget w;
{
  ((TextWidget) w)->text.update_disabled = True;
  _XawTextPrepareToUpdate( (TextWidget) w);
}

void 
XawTextEnableRedisplay(w)
Widget w;
{
  register TextWidget ctx = (TextWidget)w;
  XawTextPosition lastPos;

  if (!ctx->text.update_disabled) return;

  ctx->text.update_disabled = False;
  lastPos = ctx->text.lastPos = GETLASTPOS;
  ctx->text.lt.top = FindGoodPosition(ctx, ctx->text.lt.top);
  ctx->text.insertPos = FindGoodPosition(ctx, ctx->text.insertPos);
  if ( (ctx->text.s.left > lastPos) || (ctx->text.s.right > lastPos) ) 
    ctx->text.s.left = ctx->text.s.right = 0;
  
  _XawTextBuildLineTable(ctx, ctx->text.lt.top, TRUE);
  if (XtIsRealized(w))
    DisplayTextWindow(w);
  _XawTextExecuteUpdate(ctx);
}

XawTextSource
XawTextGetSource(w)
Widget w;
{
  return ((TextWidget)w)->text.source;
}

void 
XawTextDisplayCaret (w, display_caret)
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

/*	Function Name: XawTextSearch(w, dir, text).
 *	Description: searches for the given text block.
 *	Arguments: w - The text widget.
 *                 dir - The direction to search. 
 *                 text - The text block containing info about the string
 *                        to search for.
 *	Returns: The position of the text found, or XawTextSearchError on 
 *               an error.
 */

XawTextPosition
XawTextSearch(w, dir, text) 
Widget w;
XawTextScanDirection dir;
XawTextBlock * text;
{
  TextWidget ctx = (TextWidget) w;

  return( (*ctx->text.source->Search)(ctx->text.source, 
				      ctx->text.insertPos, dir, text) );
}
  
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
    /* num_actions      */      0,                /* Set in ClassInitialize. */
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
    /* set_values_hook  */	SetValuesHook,
    /* set_values_almost*/	XtInheritSetValuesAlmost,
    /* get_values_hook  */	GetValuesHook,
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
