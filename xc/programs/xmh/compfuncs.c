#if !defined(lint) && !defined(SABER)
static char rcs_id[] =
    "$XConsortium: compfuncs.c,v 2.14 89/08/31 19:09:25 converse Exp $";
#endif
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
 * IF THE SOFTWARE IS MODIFIED IN A MANNER CREATING DERIVATIVE COPYRIGHT
 * RIGHTS, APPROPRIATE LEGENDS MAY BE PLACED ON THE DERIVATIVE WORK IN
 * ADDITION TO THAT SET FORTH ABOVE.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Digital Equipment Corporation not be
 * used in advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.
 */

/* comp.c -- action procedures to handle composition buttons. */

#include "xmh.h"


/* Reset this composition widget to be one with just a blank message
   template. */

/*ARGSUSED*/
static void ResetCompose(widget, client_data, call_data)
    Widget	widget;		/* unused */
    caddr_t	client_data;
    caddr_t	call_data;	/* unused */
{
    Scrn	scrn = (Scrn) client_data;
    Msg		msg;
    XtCallbackRec	confirms[2];

    confirms[0].callback = (XtCallbackProc) ResetCompose;
    confirms[0].closure = (caddr_t) scrn;
    confirms[1].callback = (XtCallbackProc) NULL;
    confirms[1].closure = (caddr_t) NULL;

    if (MsgSetScrn((Msg) NULL, scrn, confirms, (XtCallbackList) NULL) ==
	NEEDS_CONFIRMATION)
	return;

    msg = TocMakeNewMsg(DraftsFolder);
    MsgLoadComposition(msg);
    MsgSetTemporary(msg);
    MsgSetReapable(msg);
    (void) MsgSetScrn(msg, scrn, (XtCallbackList) NULL, (XtCallbackList) NULL);
}

/*ARGSUSED*/
void XmhResetCompose(w, event, params, num_params)
    Widget	w;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    ResetCompose(w, (caddr_t) scrn, (caddr_t) NULL);
}


/* Send the message in this widget.  Avoid sending the same message twice.
   (Code elsewhere actually makes sure this button is disabled to avoid
   sending the same message twice, but it doesn't hurt to be safe here.) */

/*ARGSUSED*/
void XmhSend(w, event, params, num_params)
    Widget	w;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    if (scrn->msg == NULL) return;
    if (!MsgGetReapable(scrn->msg)) {
	MsgSend(scrn->msg);
	MsgSetReapable(scrn->msg);
    }
}


/* Save any changes to the message.  This also makes this message permanent. */

/*ARGSUSED*/
void XmhSave(w, event, params, num_params)
    Widget	w;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    if (scrn->msg == NULL) return;
    MsgSetPermanent(scrn->msg);
    if (MsgSaveChanges(scrn->msg))
	MsgClearReapable(scrn->msg);
}


/* Utility routine; creates a composition screen containing a forward message
   of the messages in the given msglist. */

CreateForward(mlist)
  MsgList mlist;
{
    Scrn scrn;
    Msg msg;
    scrn = NewCompScrn();
    msg = TocMakeNewMsg(DraftsFolder);
    MsgLoadForward(scrn, msg, mlist);
    MsgSetTemporary(msg);
    MsgSetScrnForComp(msg, scrn);
    MapScrn(scrn);
}
