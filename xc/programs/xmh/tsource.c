#ifndef lint
static char rcs_id[] = "$Header: tsource.c,v 1.7 87/10/09 14:01:59 weissman Exp $";
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
#include <TextPrivate.h>
#include <Xatom.h>

/* Private definitions. */

#define BUFSIZE	512

Msg MsgFromPosition(toc, position, dir)
  Toc toc;
  DwtTextPosition position;
  DwtTextScanDirection dir;
{
    Msg msg;
    int     h, l, m;
    if (position > toc->lastPos) position = toc->lastPos;
    if (dir == DwtsdLeft) position--;
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


static DwtTextPosition CoerceToLegalPosition(toc, position)
  Toc toc;
  DwtTextPosition position;
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
    DwtTextSource source = widget->source;
    Toc toc = (Toc) source->data;
    DwtTextBlockRec block;
    DwtTextPosition position, lastpos;
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
    Toc toc = (Toc) widget->source->data;
    if (toc && toc->hasselection)
	(*toc->source->SetSelection)(toc->source, 1, 0);
}


/* Semi-public definitions */

static void AddWidget(source, widget)
DwtTextSource source;
TextWidget widget;
{
    Toc toc = (Toc) source->data;
    toc->numwidgets++;
    toc->widgets = (TextWidget *)
	XtRealloc((char *) toc->widgets,
		  (unsigned) (sizeof(TextWidget) * toc->numwidgets));
    toc->widgets[toc->numwidgets - 1] = widget;
    if (toc->hasselection && toc->numwidgets == 1)
	XtSelectionGrab((Widget) toc->widgets[0], XA_PRIMARY,
			Convert, LoseSelection);
}

static void RemoveWidget(source, widget)
DwtTextSource source;
TextWidget widget;
{
    Toc toc = (Toc) source->data;
    int i;
    for (i=0 ; i<toc->numwidgets ; i++) {
	if (toc->widgets[i] == widget) {
	    toc->numwidgets--;
	    toc->widgets[i] = toc->widgets[toc->numwidgets];
	    if (i == 0 && toc->numwidgets > 0 && toc->hasselection)
		XtSelectionGrab((Widget) toc->widgets[0], XA_PRIMARY,
				Convert, LoseSelection);
	    return;
	}
    }
}


static DwtTextPosition Read(source, position, lastPos, block)
  DwtTextSource source;
  DwtTextPosition position, lastPos;
  DwtTextBlock block;
{
    Toc toc = (Toc) source->data;
    Msg msg;
    int count;
    int maxRead = lastPos - position;
    if (position < toc->lastPos) {
	msg = MsgFromPosition(toc, position, DwtsdRight);
	block->ptr = msg->buf + (position - msg->position);
	count = msg->length - (position - msg->position);
	block->length = (count < maxRead) ? count : maxRead;
	position += block->length;
    }
    else {
	block->length = 0;
	block->ptr = "";
    }
    block->format = FMT8BIT;
    return position;
}


/* Right now, we can only replace a piece with another piece of the same size,
   and it can't cross between lines. */

static DwtTextStatus Replace(source, startPos, endPos, block)
  DwtTextSource source;
  DwtTextPosition startPos, endPos;
  DwtTextBlock block;
{
    Toc toc = (Toc) source->data;
    Msg msg;
    int i;
    if (block->length != endPos - startPos)
	return EditError;
    msg = MsgFromPosition(toc, startPos, DwtsdRight);
    for (i = 0; i < block->length; i++)
	msg->buf[startPos - msg->position + i] = block->ptr[i];
    for (i=0 ; i<toc->numwidgets ; i++)
	Dwt_TextMarkRedraw(toc->widgets[i], startPos, endPos);
    return EditDone;
}


#define Look(index, c)\
{									\
    if ((dir == DwtsdLeft && index <= 0) ||				\
	    (dir == DwtsdRight && index >= toc->lastPos))		\
	c = 0;								\
    else {								\
	if (index + doff < msg->position ||				\
		index + doff >= msg->position + msg->length)		\
	    msg = MsgFromPosition(toc, index, dir);			\
	c = msg->buf[index + doff - msg->position];			\
    }									\
}



static DwtTextPosition Scan(source, position, sType, dir, count, include)
DwtTextSource source;
DwtTextPosition position;
DwtTextScanType sType;
DwtTextScanDirection dir;
int count;
Boolean include;
{
    Toc toc = (Toc) source->data;
    DwtTextPosition index;
    Msg msg;
    char    c;
    int     ddir, doff, i, whiteSpace;
    ddir = (dir == DwtsdRight) ? 1 : -1;
    doff = (dir == DwtsdRight) ? 0 : -1;

    if (toc->lastPos == 0) return 0;
    index = position;
    if (index + doff < 0) return 0;
    if (dir == DwtsdRight && index >= toc->lastPos) return toc->lastPos;
    msg = MsgFromPosition(toc, index, dir);
    switch (sType) {
	case DwtstPositions:
	    if (!include && count > 0)
		count--;
	    index = CoerceToLegalPosition(toc, index + count * ddir);
	    break;
	case DwtstWhiteSpace:
	case DwtstWordBreak:
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
		if (whiteSpace < 0 && dir == DwtsdRight)
		    whiteSpace = toc->lastPos;
		index = whiteSpace;
	    }
	    index = CoerceToLegalPosition(toc, index);
	    break;
	case DwtstEOL:
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
	case DwtstAll:
	    if (dir == DwtsdLeft)
		index = 0;
	    else
		index = toc->lastPos;
	    break;
    }
    return index;
}

static Boolean GetSelection(source, left, right)
DwtTextSource source;
DwtTextPosition *left, *right; 
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
DwtTextSource source;
DwtTextPosition left, right; 
{
    Toc toc = (Toc) source->data;
    int i;
    for (i=0 ; i<toc->numwidgets; i++) {
	DwtTextDisableRedisplay(toc->widgets[i], FALSE);
	if (toc->hasselection)
	    Dwt_TextSetHighlight(toc->widgets[i], toc->left, toc->right,
				Normal);
	if (left < right)
	    Dwt_TextSetHighlight(toc->widgets[i], left, right,
				Selected);
	DwtTextEnableRedisplay(toc->widgets[i]);
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
}


/* Public definitions. */

DwtTextSource TSourceCreate(toc)
  Toc toc;
{
    DwtTextSource source;
    source = XtNew(DwtTextSourceRec);
    source->data = (struct _DwtSourceDataRec *) toc;
    source->AddWidget = AddWidget;
    source->RemoveWidget = RemoveWidget;
    source->Read = Read;
    source->Replace = Replace;
    source->Scan = Scan;
    source->GetSelection = GetSelection;
    source->SetSelection = SetSelection;
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
	Dwt_TextInvalidate(toc->widgets[i], position, length);
}
