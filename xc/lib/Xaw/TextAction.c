#if (!defined(lint) && !defined(SABER))
static char Xrcsid[] = "$XConsortium: TextAction.c,v 1.9 89/08/17 19:22:37 kit Exp $";
#endif /* lint && SABER */

/***********************************************************
Copyright 1989 by the Massachusetts Institute of Technology,
Cambridge, Massachusetts.

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
#include <ctype.h>
#include <X11/Xatom.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>

#include <X11/Xmu/Atoms.h>
#include <X11/Xmu/Misc.h>

#include <X11/Xaw/TextP.h>

/*
 * These are defined in TextPop.c
 */

void _XawTextInsertFileAction(), _XawTextInsertFile(), _XawTextSearch();
void _XawTextSearch(), _XawTextDoSearchAction(), _XawTextDoReplaceAction();
void _XawTextSetField(), _XawTextPopdownSearchAction();

/*
 * These are defined in Text.c
 */

char * _XawTextGetText();
void _XawTextBuildLineTable(), _XawTextAlterSelection(), _XawTextVScroll();
void _XawTextSetSelection(), _XawTextCheckResize(), _XawTextExecuteUpdate();
void _XawTextSetScrollBars(), _XawTextClearAndCenterDisplay();
Atom * _XawTextSelectionList();

static void
StartAction(ctx, event)
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

static void
EndAction(ctx)
TextWidget ctx;
{
  _XawTextCheckResize(ctx);
  _XawTextExecuteUpdate(ctx);
  ctx->text.mult = 1;
}

static void
StuffFromBuffer(ctx, buffer)
TextWidget ctx;
int buffer;
{
  extern char *XFetchBuffer();
  XawTextBlock text;
  text.ptr = XFetchBuffer(XtDisplay(ctx), &(text.length), buffer);
  text.firstPos = 0;
  if (_XawTextReplace(ctx, ctx->text.insertPos, ctx->text.insertPos, &text)) {
    XBell(XtDisplay(ctx), 50);
    return;
  }
  ctx->text.insertPos = (*ctx->text.source->Scan)(ctx->text.source, 
	   ctx->text.insertPos, XawstPositions, XawsdRight, text.length, TRUE);
  XawTextUnsetSelection((Widget)ctx);
  XtFree(text.ptr);
}

static void 
UnKill(ctx, event)
TextWidget ctx;
XEvent *event;
{
  StartAction(ctx, event);
  StuffFromBuffer(ctx, 1);
  EndAction(ctx);
}

static void
Stuff(ctx, event)
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

static void GetSelection();

/* ARGSUSED */
static void 
_SelectionReceived(w, client_data, selection, type, value, length, format)
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
      GetSelection(w, list->time, list->params, list->count);
      XtFree(client_data);
    }
    return;
  }
  
  StartAction(ctx, NULL);
  text.ptr = (char*)value;
  text.firstPos = 0;
  text.length = *length;
  text.format = FMT8BIT;
  if (_XawTextReplace(ctx, ctx->text.insertPos, ctx->text.insertPos, &text)) {
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

static void 
GetSelection(w, time, params, num_params)
Widget w;
Time time;
String *params;			/* selections in precedence order */
Cardinal num_params;
{
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
	    GetSelection(w, time, params+1, num_params-1);
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

static void 
InsertSelection(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;		/* precedence list of selections to try */
Cardinal *num_params;
{
  static String default_params[] = {"PRIMARY", "CUT_BUFFER0"};
  Cardinal count;

  StartAction((TextWidget)w, event);
  if ((count = *num_params) == 0) {
    params = default_params;
    count = XtNumber(default_params);
  }
  GetSelection(w, ((TextWidget)w)->text.time, params, count);
  EndAction((TextWidget)w);
  _XawTextSetScrollBars((TextWidget) w);
}

/************************************************************
 *
 * Routines for Moving Around.
 *
 ************************************************************/

static void
Move(ctx, event, dir, type, include)
TextWidget ctx;
XEvent *event;
XawTextScanDirection dir;
XawTextScanType type;
Boolean include;
{
  StartAction(ctx, event);
  ctx->text.insertPos = (*ctx->text.source->Scan)(ctx->text.source,
		    ctx->text.insertPos, type, dir, ctx->text.mult, include);
  EndAction(ctx);
}

static void 
MoveForwardChar(w, event)
Widget w;
XEvent *event;
{
   Move((TextWidget) w, event, XawsdRight, XawstPositions, TRUE);
}

static void 
MoveBackwardChar(w, event)
Widget w;
XEvent *event;
{
  Move((TextWidget) w, event, XawsdLeft, XawstPositions, TRUE);
}

static void 
MoveForwardWord(w, event)
Widget w;
XEvent *event;
{
  Move((TextWidget) w, event, XawsdRight, XawstWhiteSpace, FALSE);
}

static void 
MoveBackwardWord(w, event)
Widget w;
XEvent *event;
{
  Move((TextWidget) w, event, XawsdLeft, XawstWhiteSpace, FALSE);
}

static void MoveForwardParagraph(w, event)
Widget w;
XEvent *event;
{
  Move((TextWidget) w, event, XawsdRight, XawstParagraph, FALSE);
}

static void MoveBackwardParagraph(w, event)
Widget w;
XEvent *event;
{
  Move((TextWidget) w, event, XawsdLeft, XawstParagraph, FALSE);
}

static void 
MoveToLineEnd(w, event)
Widget w;
XEvent *event;
{
  Move((TextWidget) w, event, XawsdRight, XawstEOL, FALSE);
}

static void 
MoveToLineStart(w, event)
Widget w;
XEvent *event;
{
  Move((TextWidget) w, event, XawsdLeft, XawstEOL, FALSE);
}


static void
MoveLine(ctx, event, dir)
TextWidget ctx;
XEvent *event;
XawTextScanDirection dir;
{
  XawTextPosition next_line, new, from_left;

  StartAction(ctx, event);

  if (dir == XawsdLeft)
    ctx->text.mult++;

  new = (*ctx->text.source->Scan)(ctx->text.source, ctx->text.insertPos,
				  XawstEOL, XawsdLeft, 1, FALSE);

  from_left = (ctx->text.insertPos - new);

  new = (*ctx->text.source->Scan)(ctx->text.source, ctx->text.insertPos,
				  XawstEOL, dir,
				  ctx->text.mult, (dir == XawsdRight));

  next_line = (*ctx->text.source->Scan)(ctx->text.source, new,
					XawstEOL, XawsdRight, 1, FALSE);

  ctx->text.insertPos = (*ctx->text.source->Scan)(ctx->text.source, new,
				  XawstPositions, XawsdRight, from_left, TRUE);

  if (ctx->text.insertPos > next_line)
    ctx->text.insertPos = next_line;

  EndAction(ctx);
}

static void 
MoveNextLine(w, event)
Widget w;
XEvent *event;
{
  MoveLine( (TextWidget) w, event, XawsdRight);
}

static void 
MovePreviousLine(w, event)
Widget w;
XEvent *event;
{
  MoveLine( (TextWidget) w, event, XawsdLeft);
}

static void
MoveBeginningOfFile(w, event)
Widget w;
XEvent *event;
{
  Move((TextWidget) w, event, XawsdLeft, XawstAll, TRUE);
}

static void 
MoveEndOfFile(w, event)
Widget w;
XEvent *event;
{
  Move((TextWidget) w, event, XawsdRight, XawstAll, TRUE);
}

static void 
Scroll(ctx, event, dir)
TextWidget ctx;
XEvent *event;
XawTextScanDirection dir;
{
  StartAction(ctx, event);

  if (dir == XawsdLeft)
    _XawTextVScroll(ctx, ctx->text.mult);
  else
    _XawTextVScroll(ctx, -ctx->text.mult);

  EndAction(ctx);
}

static void 
ScrollOneLineUp(w, event)
Widget w;
XEvent *event;
{
  Scroll( (TextWidget) w, event, XawsdLeft);
}

static void 
ScrollOneLineDown(w, event)
Widget w;
XEvent *event;
{
  Scroll( (TextWidget) w, event, XawsdRight);
}

static void 
MovePage(ctx, event, dir)
TextWidget ctx;
XEvent *event;
XawTextScanDirection dir;
{
  int scroll_val = Max(1, ctx->text.lt.lines - 2);

  if (dir == XawsdLeft)
    scroll_val = -scroll_val;

  StartAction(ctx, event);
  _XawTextVScroll(ctx, scroll_val);
  ctx->text.insertPos = ctx->text.lt.top;
  EndAction(ctx);
}

static void 
MoveNextPage(w, event)
Widget w;
XEvent *event;
{
  MovePage((TextWidget) w, event, XawsdRight);
}

static void 
MovePreviousPage(w, event)
Widget w;
XEvent *event;
{
  MovePage((TextWidget) w, event, XawsdLeft);
}

/************************************************************
 *
 * Delete Routines.
 *
 ************************************************************/

static void 
_DeleteOrKill(ctx, from, to, kill)
TextWidget ctx;
XawTextPosition from, to;
Boolean	kill;
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
  if (_XawTextReplace(ctx, from, to, &text)) {
    XBell(XtDisplay(ctx), 50);
    return;
  }
  ctx->text.insertPos = from;
  ctx->text.showposition = TRUE; 
  XawTextUnsetSelection((Widget)ctx);
}


static void
DeleteOrKill(ctx, event, dir, type, include, kill)
TextWidget	   ctx;
XEvent *event;
XawTextScanDirection dir;
XawTextScanType type;
Boolean	   include, kill;
{
  XawTextPosition from, to;
  
  StartAction(ctx, event);
  to = (*ctx->text.source->Scan)(ctx->text.source, ctx->text.insertPos,
				 type, dir, ctx->text.mult, include);
  
  if (dir == XawsdLeft) {
    from = to;
    to = ctx->text.insertPos;
  }
  else 
    from = ctx->text.insertPos;

  _DeleteOrKill(ctx, from, to, kill);
  EndAction(ctx);
}

static void 
DeleteForwardChar(w, event)
Widget w;
XEvent *event;
{
  DeleteOrKill((TextWidget) w, event, XawsdRight, XawstPositions, TRUE, FALSE);
}

static void
DeleteBackwardChar(w, event)
Widget w;
XEvent *event;
{
  DeleteOrKill((TextWidget) w, event, XawsdLeft, XawstPositions, TRUE, FALSE);
}

static void 
DeleteForwardWord(w, event)
Widget w;
XEvent *event;
{
  DeleteOrKill((TextWidget) w, event,
	       XawsdRight, XawstWhiteSpace, FALSE, FALSE);
}

static void 
DeleteBackwardWord(w, event)
Widget w;
XEvent *event;
{
  DeleteOrKill((TextWidget) w, event,
	       XawsdLeft, XawstWhiteSpace, FALSE, FALSE);
}

static void 
KillForwardWord(w, event)
Widget w;
XEvent *event;
{
  DeleteOrKill((TextWidget) w, event, 
	       XawsdRight, XawstWhiteSpace, FALSE, TRUE);
}

static void 
KillBackwardWord(w, event)
TextWidget w;
XEvent *event;
{
  DeleteOrKill((TextWidget) w, event,
	       XawsdLeft, XawstWhiteSpace, FALSE, TRUE);
}

static void
KillToEndOfLine(w, event)
Widget w;
XEvent *event;
{
  TextWidget ctx = (TextWidget) w;
  XawTextPosition end_of_line;

  StartAction(ctx, event);
  end_of_line = (*ctx->text.source->Scan)(ctx->text.source,
					  ctx->text.insertPos, XawstEOL, 
					  XawsdRight, ctx->text.mult, FALSE);
  if (end_of_line == ctx->text.insertPos)
    end_of_line = (*ctx->text.source->Scan)(ctx->text.source,
					    ctx->text.insertPos, XawstEOL, 
					    XawsdRight, ctx->text.mult, TRUE);

  _DeleteOrKill(ctx, ctx->text.insertPos, end_of_line, TRUE);
  EndAction(ctx);
}

static void 
KillToEndOfParagraph(w, event)
Widget w;
XEvent *event;
{
  DeleteOrKill((TextWidget) w, event, XawsdRight, XawstParagraph, FALSE, TRUE);
}

void 
_XawTextZapSelection(ctx, event, kill)
TextWidget ctx;
XEvent *event;
Boolean kill;
{
   StartAction(ctx, event);
   _DeleteOrKill(ctx, ctx->text.s.left, ctx->text.s.right, kill);
   EndAction(ctx);
}

static void 
KillCurrentSelection(w, event)
Widget w;
XEvent *event;
{
  _XawTextZapSelection( (TextWidget) w, event, TRUE);
}

static void 
DeleteCurrentSelection(w, event)
Widget w;
XEvent *event;
{
  _XawTextZapSelection( (TextWidget) w, event, FALSE);
}

/************************************************************
 *
 * Insertion Routines.
 *
 ************************************************************/

static int 
InsertNewLineAndBackupInternal(ctx)
TextWidget ctx;
{
  int count, error = XawEditDone;
  XawTextBlock text;
  char *buf, *ptr;

  ptr = buf = XtMalloc(sizeof(char) * ctx->text.mult);
  for (count = 0; count < ctx->text.mult; count++, ptr++)
    ptr[0] = '\n';

  text.length = ctx->text.mult;
  text.ptr = buf;
  text.firstPos = 0;
  text.format = FMT8BIT;

  if (_XawTextReplace(ctx, ctx->text.insertPos, ctx->text.insertPos, &text)) {
    XBell( XtDisplay(ctx), 50);
    error = XawEditError;
  }
  else {
    XawTextUnsetSelection((Widget)ctx);
    ctx->text.showposition = TRUE;
  }

  XtFree(buf);
  return(error);
}

static void 
InsertNewLineAndBackup(w, event)
Widget w;
XEvent *event;
{
  StartAction( (TextWidget) w, event );
  (void) InsertNewLineAndBackupInternal( (TextWidget) w );
  EndAction( (TextWidget) w );
}

static int
LocalInsertNewLine(ctx, event)
TextWidget ctx;
XEvent *event;
{
  StartAction(ctx, event);
  if (InsertNewLineAndBackupInternal(ctx) == XawEditError)
    return(XawEditError);
  ctx->text.insertPos = (*ctx->text.source->Scan)(ctx->text.source,
        ctx->text.insertPos, XawstPositions, XawsdRight, ctx->text.mult, TRUE);
  EndAction(ctx);
  return(XawEditDone);
}

static void
InsertNewLine(w, event)
Widget w;
XEvent *event;
{
  (void) LocalInsertNewLine( (TextWidget) w, event);
}

static void 
InsertNewLineAndIndent(w, event)
Widget w;
XEvent *event;
{
  XawTextBlock text;
  XawTextPosition pos1, pos2;
  TextWidget ctx = (TextWidget) w;

  StartAction(ctx, event);
  pos1 = (*ctx->text.source->Scan)(ctx->text.source, ctx->text.insertPos, 
				   XawstEOL, XawsdLeft, 1, FALSE);
  pos2 = (*ctx->text.source->Scan)(ctx->text.source, pos1, XawstEOL, 
				   XawsdLeft, 1, TRUE);
  pos2 = (*ctx->text.source->Scan)(ctx->text.source, pos2, XawstWhiteSpace, 
				   XawsdRight, 1, TRUE);
  text.ptr = _XawTextGetText(ctx, pos1, pos2);
  text.length = strlen(text.ptr);
  if (LocalInsertNewLine(ctx, event)) return;
  text.firstPos = 0;
  if (_XawTextReplace(ctx,ctx->text.insertPos, ctx->text.insertPos, &text)) {
    XBell(XtDisplay(ctx), 50);
    EndAction(ctx);
    return;
  }
  ctx->text.insertPos = (*ctx->text.source->Scan)(ctx->text.source, 
	   ctx->text.insertPos, XawstPositions, XawsdRight, text.length, TRUE);
  XtFree(text.ptr);
  EndAction(ctx);
}

/************************************************************
 *
 * Selection Routines.
 *
 *************************************************************/

static void 
SelectWord(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
  TextWidget ctx = (TextWidget) w;
  XawTextPosition l, r;

  StartAction(ctx, event);
  l = (*ctx->text.source->Scan)(ctx->text.source, ctx->text.insertPos, 
				XawstWhiteSpace, XawsdLeft, 1, FALSE);
  r = (*ctx->text.source->Scan)(ctx->text.source, l, XawstWhiteSpace, 
				XawsdRight, 1, FALSE);
  _XawTextSetSelection(ctx, l, r, params, *num_params);
  EndAction(ctx);
}

static void 
SelectAll(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
  TextWidget ctx = (TextWidget) w;

  StartAction(ctx, event);
  _XawTextSetSelection(ctx,zeroPosition,ctx->text.lastPos,params,*num_params);
  EndAction(ctx);
}

static void
ModifySelection(ctx, event, mode, action, params, num_params)
TextWidget ctx;
XEvent *event;
XawTextSelectionMode mode;
XawTextSelectionAction action;
String *params;		/* unused */
Cardinal *num_params;	/* unused */
{
  StartAction(ctx, event);
  _XawTextAlterSelection(ctx, mode, action, params, num_params);
  EndAction(ctx);
}
		
/* ARGSUSED */
static void 
SelectStart(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;		/* unused */
Cardinal *num_params;	/* unused */
{
  ModifySelection((TextWidget) w, event, 
		  XawsmTextSelect, XawactionStart, params, num_params);
}

/* ARGSUSED */
static void 
SelectAdjust(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;		/* unused */
Cardinal *num_params;	/* unused */
{
  ModifySelection((TextWidget) w, event, 
		  XawsmTextSelect, XawactionAdjust, params, num_params);
}

static void 
SelectEnd(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
  ModifySelection((TextWidget) w, event, 
		  XawsmTextSelect, XawactionEnd, params, num_params);
}

/* ARGSUSED */
static void 
ExtendStart(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;		/* unused */
Cardinal *num_params;	/* unused */
{
  ModifySelection((TextWidget) w, event, 
		  XawsmTextExtend, XawactionStart, params, num_params);
}

/* ARGSUSED */
static void 
ExtendAdjust(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;		/* unused */
Cardinal *num_params;	/* unused */
{
  ModifySelection((TextWidget) w, event, 
		  XawsmTextExtend, XawactionAdjust, params, num_params);
}

static void 
ExtendEnd(w, event, params, num_params)
TextWidget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
  ModifySelection((TextWidget) w, event, 
		  XawsmTextExtend, XawactionEnd, params, num_params);
}

/************************************************************
 *
 * Misc. Routines.
 *
 ************************************************************/

/* ARGSUSED */
static void 
RedrawDisplay(w, event)
Widget w;
XEvent *event;
{
  StartAction( (TextWidget) w, event);
  _XawTextClearAndCenterDisplay((TextWidget) w);
  EndAction( (TextWidget) w);
}

/*ARGSUSED*/
static void
TextFocusIn (w, event)
TextWidget w;
XEvent *event;
{
  TextWidget ctx = (TextWidget) w;

  ctx->text.hasfocus = TRUE; 
}

/*ARGSUSED*/
static void
TextFocusOut(w, event)
TextWidget w;
XEvent *event;
{
  TextWidget ctx = (TextWidget) w;

  ctx->text.hasfocus = FALSE;
}

static XComposeStatus compose_status = {NULL, 0};

/*	Function Name: AutoFill
 *	Description: Breaks the line at the previous word boundry when
 *                   called inside InsertChar.
 *	Arguments: ctx - The text widget.
 *	Returns: none
 */

static void
AutoFill(ctx)
TextWidget ctx;
{
  void (*FindPosition)() = ctx->text.sink->FindPosition;
  int width, height, x, line_num, max_width;
  XawTextPosition ret_pos;
  XawTextBlock text;

  if ( !((ctx->text.auto_fill) && (ctx->text.mult == 1)) )
    return;

  for ( line_num = 0; line_num < ctx->text.lt.lines ; line_num++)
    if ( ctx->text.lt.info[line_num].position >= ctx->text.insertPos )
      break;
  line_num--;			/* backup a line. */

  max_width = Max(0, ctx->core.width - HMargins(ctx));

  x = ctx->text.margin.left;
  (*FindPosition) ( (Widget) ctx, ctx->text.lt.info[line_num].position, x, 
		   max_width, TRUE, &ret_pos, &width, &height);
  
  if ( ret_pos >= ctx->text.insertPos )
    return;
  
  /*
   * Do not make any changes if we could not find a word break.
   */
  
  (*ctx->text.source->Read)(ctx->text.source, ret_pos - 1, &text, 1);
  if ( (text.length != 1) || !isspace(text.ptr[0]) )
    return;
  
  text.ptr = "\n";
  text.length = 1;
  text.firstPos = 0;
  text.format = FMT8BIT;
  
  _XawTextReplace(ctx, ret_pos - 1, ret_pos, &text);
  
  ctx->text.lt.info[line_num].textWidth = width;
  ctx->text.lt.info[line_num + 1].position = ret_pos;
  
  (*FindPosition) ( (Widget) ctx,
		   ctx->text.lt.info[line_num + 1].position + 1, x,
		   (int) (ctx->core.width - x), FALSE, 
		   &ret_pos, &width, &height);
  
  ctx->text.lt.info[line_num + 1].textWidth = width;
  
  if (ret_pos != ctx->text.lt.info[line_num + 2].position)
    _XawTextBuildLineTable(ctx, ctx->text.lt.top, TRUE);
}

static void
InsertChar(w, event)
Widget w;
XEvent *event;
{
  TextWidget ctx = (TextWidget) w;
  char *ptr, strbuf[BUFSIZ];
  int     keycode, count, error;
  XawTextBlock text;

  if ( (text.length = XLookupString (event, strbuf, BUFSIZ,
			       &keycode, &compose_status)) == 0) {
    return;
  }
  
  text.ptr = ptr = XtMalloc(sizeof(char) * text.length * ctx->text.mult);
  for (count = 0 ; count < ctx->text.mult ; count++) {
    strncpy(ptr, strbuf, text.length);
    ptr += text.length;
  }

  text.length = text.length * ctx->text.mult;
  text.firstPos = 0;
  text.format = FMT8BIT;
  
  StartAction(ctx, event);
  
  error = _XawTextReplace(ctx, ctx->text.insertPos,ctx->text.insertPos, &text);

  if (error == XawEditDone) {
    ctx->text.insertPos = 
      (*ctx->text.source->Scan)(ctx->text.source, ctx->text.insertPos,
				XawstPositions, XawsdRight, text.length, TRUE);
    AutoFill(ctx);
    XawTextUnsetSelection((Widget)ctx);
  }
  else 
    XBell(XtDisplay(ctx), 50);

  XtFree(text.ptr);
  EndAction(ctx);
  _XawTextSetScrollBars(ctx);
}

/*ARGSUSED*/
static void 
InsertString(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
{
  TextWidget ctx = (TextWidget) w;
  XawTextBlock text;
  int	   i;

  text.firstPos = 0;
  StartAction(ctx, event);
  for (i = *num_params; i; i--, params++) {
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
    if (_XawTextReplace(ctx, ctx->text.insertPos, 
			ctx->text.insertPos, &text)) {
      XBell(XtDisplay(ctx), 50);
      EndAction(ctx);
      return;
    }
    ctx->text.insertPos =
      (*ctx->text.source->Scan)(ctx->text.source, ctx->text.insertPos,
				XawstPositions, XawsdRight, text.length, TRUE);
  }
  XawTextUnsetSelection((Widget)ctx);
  EndAction(ctx);
}

static void 
DisplayCaret(w, event, params, num_params)
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

/*	Function Name: Multiply
 *	Description: Multiplies the current action by the number passed.
 *	Arguments: w - the text widget.
 *                 event - ** NOT USED **.
 *                 params, num_params - The parameter list, see below.
 *	Returns: none.
 *
 * Parameter list;
 *  
 * The parameter list may contain either a number or the string 'Reset'.
 * 
 * A number will multiply the current multiplication factor by that number.
 * Many of the text widget actions will will perform n actions, where n is
 * the multiplication factor.
 *
 * The string reset will reset the mutiplication factor to 1.
 * 
 */

/* ARGSUSED */
static void 
Multiply(w, event, params, num_params)
Widget w;
XEvent *event;
String * params;
Cardinal * num_params;
{
  TextWidget ctx = (TextWidget) w;
  int mult;

  if (*num_params != 1) {
    XtAppError(XtWidgetToApplicationContext(w), 
	       "The multiply action takes exactly one argument.");
    XBell(XtDisplay(w), 0);
    return;
  }

  if ( (params[0][0] == 'r') || (params[0][0] == 'R') ) {
    XBell(XtDisplay(w), 0);
    ctx->text.mult = 1;
    return;
  }

  if ( (mult = atoi(params[0])) == 0 ) {
    char buf[BUFSIZ];
    sprintf(buf, "%s %s", "Text Widget: The multiply action's argument",
	    "must be a number greater than zero, or 'Reset'.");
    XtAppError(XtWidgetToApplicationContext(w), buf);
    XBell(XtDisplay(w), 0);
    return;
  }

  ctx->text.mult *= mult;
}

/*	Function Name: StripOutOldCRs
 *	Description: strips out the old carrige returns.
 *	Arguments: ctx - the text widget.
 *                 from - starting point.
 *                 to - the ending point
 *	Returns: the new ending location (we may add some caracters).
 */

static XawTextPosition
StripOutOldCRs(ctx, from, to)
TextWidget ctx;
XawTextPosition from, to;
{
  XawTextPosition (*Scan)() = ctx->text.source->Scan;
  XawTextPosition startPos, endPos, eop_begin, eop_end;
  XawTextBlock text;
  char *buf;

  text.ptr= "  ";
  text.firstPos = 0;
  text.format = FMT8BIT;
   
/*
 * Strip out CR's. 
 */

  eop_begin = eop_end = startPos = endPos = from;
  while (TRUE) {
    endPos=(*Scan)(ctx->text.source, startPos, XawstEOL, XawsdRight, 1, FALSE);
    if (endPos >= to)
      break;

    if (endPos >= eop_begin) {
      startPos = eop_end;
      eop_begin = (*Scan)(ctx->text.source, startPos, XawstParagraph,
			  XawsdRight, 1, FALSE);
      eop_end = (*Scan)(ctx->text.source, startPos, XawstParagraph,
			XawsdRight, 1, TRUE);
    }
    else {
      XawTextPosition periodPos, next_word;
      int i, len, start;

      periodPos=(*Scan)(ctx->text.source, endPos, 
			XawstPositions, XawsdLeft, 1, TRUE);
      next_word = (*Scan)(ctx->text.source, endPos, XawstWhiteSpace,
			  XawsdRight, 1, FALSE);

      len = next_word - periodPos;

      text.length = 1;
      buf = _XawTextGetText(ctx, periodPos, next_word);
      start = 0;
      if (periodPos < endPos) {
	if (buf[0] == '.')
	  text.length++;	/* Put in two spaces. */
	start++;
      }

      /*
       * Remove all spaces following the CR. 
       */

      for (i = 1 ; i < len; i++) 
	if ( !isspace(buf[i + start]) )
	  break;
      
      XtFree(buf);

      to -= i - text.length;
      startPos = (*Scan)(ctx->text.source, periodPos,
			 XawstPositions, XawsdRight, i + start, TRUE);
      _XawTextReplace(ctx, endPos, startPos, &text);
	startPos -= i - text.length;
    }
  }
  return(to);
}

/*	Function Name: InsertNewCRs
 *	Description: Inserts the new Carrige Returns.
 *	Arguments: ctx - the text widget.
 *                 from, to - the ends of the region.
 *	Returns: none
 */

static void
InsertNewCRs(ctx, from, to)
TextWidget ctx;
XawTextPosition from, to;
{
  void (*FindPosition)() = ctx->text.sink->FindPosition;
  XawTextPosition (*Scan)() = ctx->text.source->Scan;
  XawTextPosition startPos, endPos, space, eol;
  XawTextBlock text;
  int i, width, height, len;
  char * buf;

  text.ptr = "\n";
  text.length = 1;
  text.firstPos = 0;
  text.format = FMT8BIT;

  startPos = from;
  while (TRUE) {
    (*FindPosition) ( (Widget) ctx, startPos, (int) ctx->text.margin.left,
		      (int) (ctx->core.width - HMargins(ctx)), 
		      TRUE, &eol, &width, &height);
    if (eol >= to)
      break;

    eol = (*Scan)(ctx->text.source, eol, XawstPositions, XawsdLeft, 1,
		  TRUE);
    space = (*Scan)(ctx->text.source, eol, XawstWhiteSpace, XawsdRight, 1,
		    TRUE);
    
    startPos = endPos = eol;
    if (eol == space) 
      return;

    len = (int) (space - eol);
    buf = _XawTextGetText(ctx, eol, space);
    for ( i = 0 ; i < len ; i++)
      if (!isspace(buf[i]))
	break;

    to -= (i - 1);
    endPos = (*Scan)(ctx->text.source, endPos,
		     XawstPositions, XawsdRight, i, TRUE);
    XtFree(buf);
    
    _XawTextReplace(ctx, startPos, endPos, &text);
    startPos = (*Scan)(ctx->text.source, startPos,
		       XawstPositions, XawsdRight, 1, TRUE);
  }
}  
  
/*	Function Name: FormRegion
 *	Description: Forms up the region specified.
 *	Arguments: ctx - the text widget.
 *                 from, to - the ends of the region.
 *	Returns: none.
 */

static void
FormRegion(ctx, from, to)
TextWidget ctx;
XawTextPosition from, to;
{
  if (from >= to) return;

  to = StripOutOldCRs(ctx, from, to);
  InsertNewCRs(ctx, from, to);
  _XawTextBuildLineTable(ctx, ctx->text.lt.top, TRUE);
}

/*	Function Name: FromParagraph.
 *	Description: reforms up the current paragraph.
 *	Arguments: w - the text widget.
 *                 event - the X event.
 *                 params, num_params *** NOT USED ***.
 *	Returns: none
 */

/* ARGSUSED */
static void 
FormParagraph(w, event, params, num_params)
Widget w;
XEvent *event;
String * params;
Cardinal * num_params;
{
  TextWidget ctx = (TextWidget) w;
  XawTextPosition from, to;

  StartAction(ctx, event);

  from =  (*ctx->text.source->Scan)(ctx->text.source, ctx->text.insertPos,
				    XawstParagraph, XawsdLeft, 1, FALSE);
  to  =  (*ctx->text.source->Scan)(ctx->text.source, from,
				   XawstParagraph, XawsdRight, 1, FALSE);

  FormRegion(ctx, from, to);
  EndAction(ctx);
  _XawTextSetScrollBars(ctx);
}

/*	Function Name: TransposeCharacters
 *	Description: Swaps the character to the left of the mark with
 *                   the character to the right of the mark.
 *	Arguments: w - the text widget.
 *                 event - the event that cause this action.
 *                 params, num_params *** NOT USED ***.
 *	Returns: none.
 */
	     
/* ARGSUSED */
static void 
TransposeCharacters(w, event, params, num_params)
Widget w;
XEvent *event;
String * params;
Cardinal * num_params;
{
  TextWidget ctx = (TextWidget) w;
  XawTextSource src = ctx->text.source;
  XawTextPosition start, end;
  XawTextBlock text;
  unsigned char * buf, c;
  int i;

  StartAction(ctx, event);

/*
 * Get bounds. 
 */

  start = (*src->Scan) (src, ctx->text.insertPos, XawstPositions, XawsdLeft,
			1, TRUE);
  end = (*src->Scan) (src, ctx->text.insertPos, XawstPositions, XawsdRight,
		      ctx->text.mult, TRUE);

  if ( (start == ctx->text.insertPos) || (end == ctx->text.insertPos) ) 
    XBell(XtDisplay(w), 0);	/* complain. */
  else {
    ctx->text.insertPos = end;

    /*
     * Retrieve text and swap the characters. 
     */
    
    buf = (unsigned char *) _XawTextGetText(ctx, start, end);
    text.length = strlen(buf);
    text.firstPos = 0;
    text.format = FMT8BIT;
    
    c = buf[0];
    for (i = 1 ; i < text.length ; i++)
      buf[i - 1] = buf[i];
    buf[i - 1] = c;
    
    /* 
     * Store new text is source.
     */
    
    text.ptr = (char *) buf;
    _XawTextReplace (ctx, start, end, &text);
    
    XtFree(buf);
  }
  EndAction(ctx);
}

/* Action Table */

XtActionsRec textActionsTable[] = {
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
  {"insert-file", 		_XawTextInsertFile},
  {"search",		        _XawTextSearch},
  {"insert-char", 		InsertChar},
  {"insert-string",		InsertString},
  {"focus-in", 	 	        TextFocusIn},
  {"focus-out", 		TextFocusOut},
  {"display-caret",		DisplayCaret},
  {"multiply",		        Multiply},
  {"form-paragraph",            FormParagraph},
  {"transpose-characters",      TransposeCharacters},
/* Action to bind special translations for text Dialogs. */
  {"InsertFileAction",          _XawTextInsertFileAction},
  {"DoSearchAction",            _XawTextDoSearchAction},
  {"DoReplaceAction",           _XawTextDoReplaceAction},
  {"SetField",                  _XawTextSetField},
  {"PopdownSearchAction",       _XawTextPopdownSearchAction},
};

Cardinal textActionsTableCount = XtNumber(textActionsTable);
