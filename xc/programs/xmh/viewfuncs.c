#if !defined(lint) && !defined(SABER)
static char rcs_id[] =
    "$XConsortium: viewfuncs.c,v 2.15 89/08/31 19:12:44 converse Exp $";
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
    XtPointer	client_data;
    XtPointer	call_data;	/* unused */
{
    Scrn scrn = (Scrn) client_data;
    XtCallbackRec	confirms[2];

    confirms[0].callback = DoCloseView;
    confirms[0].closure = (XtPointer) scrn;
    confirms[1].callback = (XtCallbackProc) NULL;
    confirms[1].closure = (XtPointer) NULL;
    
    if (MsgSetScrn((Msg) NULL, scrn, confirms, (XtCallbackList) NULL) ==
	NEEDS_CONFIRMATION)
	return;
    DestroyScrn(scrn);
}
    

/*ARGSUSED*/
void XmhCloseView(w, event, params, num_params)
    Widget	w;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    DoCloseView(w, (XtPointer) scrn, (XtPointer) NULL);
}


/*ARGSUSED*/
void DoViewReply(w, client_data, call_data)
    Widget	w;
    XtPointer	client_data;
    XtPointer	call_data;
{
    Scrn	scrn = (Scrn) client_data;
    Msg		msg;
    Scrn	nscrn;

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
void XmhViewReply(w, event, params, num_params)
    Widget	w;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    DoViewReply(w, (XtPointer) scrn, (XtPointer) NULL);
}


/*ARGSUSED*/
void DoViewForward(w, client_data, call_data)
    Widget	w;
    XtPointer	client_data;
    XtPointer	call_data;
{
    Scrn	scrn = (Scrn) client_data;
    MsgList	mlist;

    if (scrn->msg == NULL) return;
    mlist = MakeSingleMsgList(scrn->msg);
    CreateForward(mlist);
    FreeMsgList(mlist);
}

/*ARGSUSED*/
void XmhViewForward(w, event, params, num_params)
    Widget	w;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    DoViewForward(w, (XtPointer) scrn, (XtPointer) NULL);
}


/*ARGSUSED*/
void DoViewUseAsComposition(w, client_data, call_data)
    Widget	w;
    XtPointer	client_data;
    XtPointer	call_data;
{
    Scrn	scrn = (Scrn) client_data;
    Msg		msg;
    Scrn	nscrn;

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
void XmhViewUseAsComposition(w, event, params, num_params)
    Widget	w;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    Scrn	scrn = ScrnFromWidget(w);
    DoViewUseAsComposition(w, (XtPointer) scrn, (XtPointer) NULL);
}


/*ARGSUSED*/
void DoEditView(w, client_data, call_data)
    Widget	w;
    XtPointer	client_data;
    XtPointer	call_data;
{
    Scrn	scrn = (Scrn) client_data;
    Arg		args[1];
    XtTranslations editTranslations = scrn->edit_translations;

    if (scrn->msg == NULL) return;
    if (scrn->kind == STtocAndView) {
	XtSetArg(args[0], XtNtranslations, editTranslations);
	XtSetValues(scrn->viewwidget, args, (Cardinal) 1);
    }
    MsgSetEditable(scrn->msg);
}


/*ARGSUSED*/
void XmhEditView(w, event, params, num_params)
    Widget	w;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    Scrn	scrn = ScrnFromWidget(w);
    DoEditView(w, (XtPointer) scrn, (XtPointer) NULL);
}


/*ARGSUSED*/
void DoSaveView(w, client_data, call_data)
    Widget	w;
    XtPointer	client_data;
    XtPointer	call_data;
{
    Scrn	scrn = (Scrn) client_data;
    Arg		args[2];

    if (scrn->msg == NULL) return;
    if (MsgSaveChanges(scrn->msg)) {
	if (scrn->kind == STtocAndView) {
	    XtSetArg(args[0], XtNtranslations, scrn->read_translations);
	    XtSetValues(scrn->viewwidget, args, (Cardinal) 1);
	}
	MsgClearEditable(scrn->msg);
    }
}


/*ARGSUSED*/
void XmhSaveView(w, event, params, num_params)
    Widget	w;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    Scrn	scrn = ScrnFromWidget(w);
    DoSaveView(w, (XtPointer) scrn, (XtPointer) NULL);
}


/*ARGSUSED*/
void DoPrintView(w, client_data, call_data)
    Widget	w;
    XtPointer	client_data;
    XtPointer	call_data;
{
    Scrn	scrn = (Scrn) client_data;
    char str[200];

    if (scrn->msg == NULL) return;
    (void) sprintf(str, "%s %s", app_resources.defPrintCommand,
		   MsgFileName(scrn->msg));
    (void) system(str);
}


/*ARGSUSED*/
void XmhPrintView(w, event, params, num_params)
    Widget	w;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    DoPrintView(w, (XtPointer) scrn, (XtPointer) NULL);
}
