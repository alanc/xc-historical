#ifndef lint
static char rcs_id[] = "$XConsortium: tsource.c,v 2.13 88/10/23 12:56:40 swick Exp $";
#endif lint
/*
 *			  COPYRIGHT 1987
 *		   DIGITAL EQUIPMENT CORPORATION
 *		       MAYNARD, MASSACHUSETTS
 *			ALL RIGHTS RESERVED.
 *
 * THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT NOTICE AND
 * SHOULD NOT BE CONSTRUED AS A COMMITMENT BY DIGITAL EQUIPMENT CORPORATION.
 * DIGITAL MAKES NO REPRESENTATIONS ABOUT THE SUITABILITY OF THIS SOFTWARE FOR
 * ANY PURPOSE.  IT IS SUPPLIED "AS IS" WITHOUT EXPRESS OR IMPLIED WARRANTY.
 *
 * IF THE SOFTWARE IS MODIFIED IN A MANNER CREATING DERIVATIVE COPYRIGHT RIGHTS,
 * APPROPRIATE LEGENDS MAY BE PLACED ON THE DERIVATIVE WORK IN ADDITION TO THAT
 * SET FORTH ABOVE.
 *
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting documentation,
 * and that the name of Digital Equipment Corporation not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission.
 */

/* File: tsource.c -- the code for a toc source */

#include "xmh.h"
#include "tocintrnl.h"
#include <X11/Xatom.h>

/* Private definitions. */

#define BUFSIZE	512

Msg MsgFromPosition(toc, position, dir)
  Toc toc;
  XawTextPosition position;
  XawTextScanDirection dir;
{
    Msg msg;
    int     h, l, m;
    if (position > toc->lastPos) position = toc->lastPos;
    if (dir == XawsdLeft) position--;
    l = 0;
    h = toc->nummsgs - 1;
    while (l < h - 1) {
	m = (l + h) / 2;
	if (toc->msgs[m]->position > position)
	    h = m;
	else
	    l = m;
    }
    msg = toc->msgs[h];
    if (msg->position > position)
	msg = toc->msgs[h = l];
    while (!msg->visible)
	msg = toc->msgs[h++];
    if (position < msg->position || position > msg->position + msg->length)
	Punt("Error in MsgFromPosition!");
    return msg;
}


static XawTextPosition CoerceToLegalPosition(toc, position)
  Toc toc;
  XawTextPosition position;
{
    return (position < 0) ? 0 :
		 ((position > toc->lastPos) ? toc->lastPos : position);
}


/*ARGSUSED*/
static Boolean Convert(w, desiredtype, type, value, length)
Widget w;
Atom desiredtype;
Atom *type;
caddr_t *value;
int *length;
{
    TextWidget widget = (TextWidget) w;
    XawTextSource source = widget->text.source;
    Toc toc = (Toc) source->data;
    XawTextBlock block;
    XawTextPosition position, lastpos;
    *type = (Atom) FMT8BIT;		/* Only thing we know! */
    if (toc == NULL || !toc->hasselection) return FALSE;
    *length = toc->right - toc->left;
    *value = XtMalloc((unsigned) *length + 1);
    position = toc->left;
    while (position < toc->right) {
	lastpos = position;
	position = (*source->Read)(source, position, toc->right, &block);
	bcopy(block.ptr, (*value) + lastpos - toc->left, position - lastpos);
    }
    return TRUE;
}

/*ARGSUSED*/
static void LoseSelection(w, selection)
Widget w;
Atom selection;
{
    TextWidget widget = (TextWidget) w;
    Toc toc = (Toc) widget->text.source->data;
    if (toc && toc->hasselection)
	(*toc->source->SetSelection)(toc->source, 1, 0);
}


/* Semi-public definitions */

static void AddWidget(source, widget)
XawTextSource source;
TextWidget widget;
{
    Toc toc = (Toc) source->data;
    toc->numwidgets++;
    toc->widgets = (TextWidget *)
	XtRealloc((char *) toc->widgets,
		  (unsigned) (sizeof(TextWidget) * toc->numwidgets));
    toc->widgets[toc->numwidgets - 1] = widget;
#ifdef notdef
    if (toc->hasselection && toc->numwidgets == 1)
	XtSelectionGrab((Widget) toc->widgets[0], XA_PRIMARY,
			Convert, LoseSelection);
#endif notdef
}

static void RemoveWidget(source, widget)
XawTextSource source;
TextWidget widget;
{
    Toc toc = (Toc) source->data;
    int i;
    for (i=0 ; i<toc->numwidgets ; i++) {
	if (toc->widgets[i] == widget) {
	    toc->numwidgets--;
	    toc->widgets[i] = toc->widgets[toc->numwidgets];
	    if (i == 0 && toc->numwidgets > 0 && toc->hasselection)
#ifdef notdef
		XtSelectionGrab((Widget) toc->widgets[0], XA_PRIMARY,
				Convert, LoseSelection);
#endif notdef
	    return;
	}
    }
}


static XawTextPosition Read(source, position, block, length)
  XawTextSource source;
  XawTextPosition position;
  XawTextBlock *block;
  int length;
{
    Toc toc = (Toc) source->data;
    Msg msg;
    int count;

    if (position < toc->lastPos) {
        block->firstPos = position;
	msg = MsgFromPosition(toc, position, XawsdRight);
	block->ptr = msg->buf + (position - msg->position);
	count = msg->length - (position - msg->position);
	block->length = (count < length) ? count : length;
	position += block->length;
    }
    else {
        block->firstPos = 0;
	block->length = 0;
	block->ptr = "";
    }
    block->format = FMT8BIT;
    return position;
}


/* Right now, we can only replace a piece with another piece of the same size,
   and it can't cross between lines. */

static int Replace(source, startPos, endPos, block)
  XawTextSource source;
  XawTextPosition startPos, endPos;
  XawTextBlock *block;
{
    Toc toc = (Toc) source->data;
    Msg msg;
    int i;
    if (block->length != endPos - startPos)
	return XawEditError;
    msg = MsgFromPosition(toc, startPos, XawsdRight);
    for (i = 0; i < block->length; i++)
	msg->buf[startPos - msg->position + i] = block->ptr[i];
    for (i=0 ; i<toc->numwidgets ; i++)
	XawTextInvalidate(toc->widgets[i], startPos, endPos);
    return XawEditDone;
}


#define Look(index, c)\
{									\
    if ((dir == XawsdLeft && index <= 0) ||				\
	    (dir == XawsdRight && index >= toc->lastPos))		\
	c = 0;								\
    else {								\
	if (index + doff < msg->position ||				\
		index + doff >= msg->position + msg->length)		\
	    msg = MsgFromPosition(toc, index, dir);			\
	c = msg->buf[index + doff - msg->position];			\
    }									\
}



static XawTextPosition Scan(source, position, sType, dir, count, include)
XawTextSource source;
XawTextPosition position;
XawTextScanType sType;
XawTextScanDirection dir;
int count;
Boolean include;
{
    Toc toc = (Toc) source->data;
    XawTextPosition index;
    Msg msg;
    char    c;
    int     ddir, doff, i, whiteSpace;
    ddir = (dir == XawsdRight) ? 1 : -1;
    doff = (dir == XawsdRight) ? 0 : -1;

    if (toc->lastPos == 0) return 0;
    index = position;
    if (index + doff < 0) return 0;
    if (dir == XawsdRight && index >= toc->lastPos) return toc->lastPos;
    msg = MsgFromPosition(toc, index, dir);
    switch (sType) {
	case XawstPositions:
	    if (!include && count > 0)
		count--;
	    index = CoerceToLegalPosition(toc, index + count * ddir);
	    break;
	case XawstWhiteSpace:
/* |||	case XawstWordBreak: */
	    for (i = 0; i < count; i++) {
		whiteSpace = -1;
		while (index >= 0 && index <= toc->lastPos) {
		    Look(index, c);
		    if ((c == ' ') || (c == '\t') || (c == '\n')) {
			if (whiteSpace < 0) whiteSpace = index;
		    } else if (whiteSpace >= 0)
			break;
		    index += ddir;
		}
	    }
	    if (!include) {
		if (whiteSpace < 0 && dir == XawsdRight)
		    whiteSpace = toc->lastPos;
		index = whiteSpace;
	    }
	    index = CoerceToLegalPosition(toc, index);
	    break;
	case XawstEOL:
	    for (i = 0; i < count; i++) {
		while (index >= 0 && index <= toc->lastPos) {
		    Look(index, c);
		    if (c == '\n')
			break;
		    index += ddir;
		}
		if (i < count - 1)
		    index += ddir;
	    }
	    if (include)
		index += ddir;
	    index = CoerceToLegalPosition(toc, index);
	    break;
	case XawstAll:
	    if (dir == XawsdLeft)
		index = 0;
	    else
		index = toc->lastPos;
	    break;
    }
    return index;
}

static Boolean GetSelection(source, left, right)
XawTextSource source;
XawTextPosition *left, *right; 
{
    Toc toc = (Toc) source->data;
    if (toc->hasselection && toc->left < toc->right) {
	*left = toc->left;
	*right = toc->right;
	return TRUE;
    }
    toc->hasselection = FALSE;
    return FALSE;
}


static void SetSelection(source, left, right)
XawTextSource source;
XawTextPosition left, right; 
{
#ifdef notdef
    Toc toc = (Toc) source->data;
    int i;
    for (i=0 ; i<toc->numwidgets; i++) {
	XawTextDisableRedisplay(toc->widgets[i], FALSE);
	if (toc->hasselection)
	    Xt_TextSetHighlight(toc->widgets[i], toc->left, toc->right,
				Normal);
	if (left < right)
	    Xt_TextSetHighlight(toc->widgets[i], left, right,
				Selected);
	XawTextEnableRedisplay(toc->widgets[i]);
    }
    toc->hasselection = (left < right);
    toc->left = left;
    toc->right = right;
    if (toc->numwidgets > 0) {
	Widget widget = (Widget) toc->widgets[0];
	if (toc->hasselection)
	    XtSelectionGrab(widget, XA_PRIMARY, Convert, LoseSelection);
	else
	    XtSelectionUngrab(widget, XA_PRIMARY);
    }
#endif notdef
}


/* Public definitions. */

XawTextSource TSourceCreate(toc)
  Toc toc;
{
    XawTextSource source;
    source = XtNew(XawTextSourceRec);
    source->data = (caddr_t) toc;
    source->AddWidget = AddWidget;
    source->RemoveWidget = RemoveWidget;
    source->Read = Read;
    source->Replace = Replace;
    source->Scan = Scan;
#ifdef notdef
    source->GetSelection = GetSelection;
    source->SetSelection = SetSelection;
#else
    source->SetSelection = NULL;
#endif
    source->ConvertSelection = NULL;
    source->edit_mode = XawtextRead;
    toc->numwidgets = 0;
    toc->widgets = XtNew(TextWidget);
    toc->hasselection = FALSE;
    toc->left = toc->right = 0;
    return source;
}


void TSourceInvalid(toc, position, length)
Toc toc;
{
    int i;
    SetSelection(toc->source, 1, 0); /* %%% A bit of a hack. */
    for (i=0 ; i<toc->numwidgets ; i++)
	XawTextInvalidate(toc->widgets[i], position, position+length-1);
}
