#if !defined(lint) && !defined(SABER)
static char rcs_id[] =
    "$XConsortium: viewfuncs.c,v 2.12 89/07/12 16:24:15 converse Exp $";
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

/* view.c -- action procedures to handle viewing of a message */

#include "xmh.h"


/*ARGSUSED*/
void DoCloseView(widget, client_data, call_data)
    Widget	widget;		/* unused */
    caddr_t	client_data;
    caddr_t	call_data;	/* unused */
{
    Scrn scrn = (Scrn) client_data;
    if (MsgSetScrn((Msg) NULL, scrn, DoCloseView, (caddr_t) scrn)
	== NEEDS_CONFIRMATION)
	return;
    DestroyScrn(scrn);
}
    

/*ARGSUSED*/
void CloseView(w, event, params, num_params)
    Widget	w;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    DoCloseView(w, (caddr_t) scrn, (caddr_t) NULL);
}


/*ARGSUSED*/
void ViewReply(w, event, params, num_params)
    Widget	w;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    Msg msg;
    Scrn nscrn;
    if (scrn->msg == NULL) return;
    nscrn = NewCompScrn();
    ScreenSetAssocMsg(nscrn, scrn->msg);
    msg = TocMakeNewMsg(DraftsFolder);
    MsgSetTemporary(msg);
    MsgLoadReply(msg, scrn->msg);
    MsgSetScrnForComp(msg, nscrn);
    MapScrn(nscrn);
}


/*ARGSUSED*/
void ViewForward(w, event, params, num_params)
    Widget	w;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    MsgList mlist;
    if (scrn->msg == NULL) return;
    mlist = MakeSingleMsgList(scrn->msg);
    CreateForward(mlist);
    FreeMsgList(mlist);
}


/*ARGSUSED*/
void ViewUseAsComposition(w, event, params, num_params)
    Widget	w;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    Msg msg;
    Scrn nscrn;
    if (scrn->msg == NULL) return;
    nscrn = NewCompScrn();
    if (MsgGetToc(scrn->msg) == DraftsFolder)
	msg = scrn->msg;
    else {
	msg = TocMakeNewMsg(DraftsFolder);
	MsgLoadCopy(msg, scrn->msg);
	MsgSetTemporary(msg);
    }
    MsgSetScrnForComp(msg, nscrn);
    MapScrn(nscrn);
}


/*ARGSUSED*/
void EditView(w, event, params, num_params)
    Widget	w;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    if (scrn->msg == NULL) return;
    MsgSetEditable(scrn->msg);
}
    

/*ARGSUSED*/
void SaveView(w, event, params, num_params)
    Widget	w;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    if (scrn->msg == NULL) return;
    if (MsgSaveChanges(scrn->msg))
	MsgClearEditable(scrn->msg);
}
    

/*ARGSUSED*/
void PrintView(w, event, params, num_params)
    Widget	w;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    char str[200];
    if (scrn->msg == NULL) return;
    (void) sprintf(str, "%s %s", app_resources.defPrintCommand,
		   MsgFileName(scrn->msg));
    (void) system(str);
}

