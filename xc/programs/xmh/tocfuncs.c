/*
 * $XConsortium: tocfuncs.c,v 2.30 89/12/14 21:18:05 converse Exp $
 *
 *
 *			COPYRIGHT 1987, 1989
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

/* tocfuncs.c -- action procedures concerning things in the toc widget. */

#include "xmh.h"
#include "tocutil.h"

#define MAX_SYSTEM_LEN 510

Boolean UserWantsAction(w, scrn) /* general action procedure "filter" */
    Widget	w;
    Scrn	scrn;
{
    /* Commands in the command menus invoke callbacks directly. 
     * Keyboard accelerators use the command menus as source widgets.
     * Actions can also be specified in the translations for menu buttons.
     * Actions can also be specified in the translations for menus.
     * In fact, the user can attach actions to any (reasonable) widget.
     *
     * The purpose of this check is to prevent actions specified as
     * translations for folder menus and for folder buttons from executing
     * after the mouse pointer has left the folder button or the when the
     * mouse button is released outside of the folder menu.
     *
     * The side effect of this routine is that it restricts keyboard 
     * accelerators from originating from folder buttons or folder menus.
     */
       
    if (XtIsSubclass(w, menuButtonWidgetClass) && /* w is a menu button */
	w != LastMenuButtonPressed)		  /* pointer left the window */
	return False;

    if (XtIsSubclass(w, simpleMenuWidgetClass) &&	/* w is a menu */
	(! XawSimpleMenuGetActiveEntry(w)) &&	/* no entry was selected */
	(BBoxIsGrandparent(scrn->folderbuttons, w)))  /* w is a folder menu */
	return False;

    return True;
}


/*ARGSUSED*/
static void NextAndPreviousView(scrn, next)
    Scrn	scrn;
    Boolean	next;	/* if true, next or forward; if false, previous */
{
    Toc		toc = scrn->toc;
    MsgList	mlist;
    FateType	fate;
    Msg		msg;

    if (toc == NULL) return;
    mlist = TocCurMsgList(toc);
    if (mlist->nummsgs) 
	msg = (next ? mlist->msglist[0] : mlist->msglist[mlist->nummsgs - 1]);
    else {
	msg = TocGetCurMsg(toc);
	if (msg && msg == scrn->msg) 
	    msg = (next ? TocMsgAfter(toc, msg) : TocMsgBefore(toc, msg));
	if (msg) fate = MsgGetFate(msg, (Toc *)NULL);
	while (msg && ((app_resources.skip_deleted && fate == Fdelete)
		|| (app_resources.skip_moved && fate == Fmove)
		|| (app_resources.skip_copied && fate == Fcopy))) {
	    msg = (next ? TocMsgAfter(toc, msg) : TocMsgBefore(toc, msg));
	    if (msg) fate = MsgGetFate(msg, (Toc *)NULL);
	}
    }

    if (msg) {
	XtCallbackRec	confirms[2];
	if (next)
	    confirms[0].callback = (XtCallbackProc) DoNextView;
	else
	    confirms[0].callback = (XtCallbackProc) DoPrevView;
	confirms[0].closure = (XtPointer) scrn;
	confirms[1].callback = (XtCallbackProc) NULL;
	confirms[1].closure = (XtPointer) NULL;
	if (MsgSetScrn(msg, scrn, confirms, (XtCallbackList) NULL) !=
	    NEEDS_CONFIRMATION) {
	    TocUnsetSelection(toc);
	    TocSetCurMsg(toc, msg);
	}
    }
    FreeMsgList(mlist);
}


/*ARGSUSED*/
void DoReverseReadOrder(widget, client_data, call_data)
    Widget	widget;		/* the menu entry widget */
    XtPointer	client_data;
    XtPointer	call_data;
{
    app_resources.reverse_read_order =
	(app_resources.reverse_read_order ? False : True);
    ToggleMenuItem(widget, app_resources.reverse_read_order);
}


/*ARGSUSED*/
void DoNextView(widget, client_data, call_data)
    Widget	widget;		/* unused */
    XtPointer	client_data;
    XtPointer	call_data;	/* unused */
{
    NextAndPreviousView((Scrn) client_data,
			(app_resources.reverse_read_order ? False : True));
}

/*ARGSUSED*/
void XmhViewNextMessage(w, event, params, num_params)
    Widget	w;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    if (UserWantsAction(w, scrn))
	DoNextView(w, (XtPointer) scrn, (XtPointer) NULL);
}

/*ARGSUSED*/
void DoPrevView(widget, client_data, call_data)
    Widget	widget;		/* unused */
    XtPointer	client_data;	
    XtPointer	call_data;	/* unused */
{
    NextAndPreviousView((Scrn) client_data, 
			(app_resources.reverse_read_order ? True : False));
}

/*ARGSUSED*/
void XmhViewPreviousMessage(w, event, params, num_params)
    Widget	w;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    if (UserWantsAction(w, scrn))
	DoPrevView(w, (XtPointer) scrn, (XtPointer) NULL);
}


/*ARGSUSED*/
void DoViewNew(w, client_data, call_data)
    Widget	w;
    XtPointer	client_data;
    XtPointer	call_data;
{
    Scrn	scrn = (Scrn) client_data;
    Toc		toc = scrn->toc;
    Scrn	vscrn;
    MsgList	mlist;

    if (toc == NULL) return;
    mlist = CurMsgListOrCurMsg(toc);
    if (mlist->nummsgs) {
	vscrn = NewViewScrn();
	(void) MsgSetScrn(mlist->msglist[0], vscrn, (XtCallbackList) NULL,
			  (XtCallbackList) NULL);
	MapScrn(vscrn);
    }
    FreeMsgList(mlist);
}


/*ARGSUSED*/
void XmhViewInNewWindow(w, event, params, num_params)
    Widget	w;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    if (UserWantsAction(w, scrn))
	DoViewNew(w, (XtPointer) scrn, (XtPointer) NULL);
}


/*ARGSUSED*/
void DoForward(w, client_data, call_data)
    Widget	w;
    XtPointer	client_data;
    XtPointer	call_data;
{
    Scrn	scrn = (Scrn) client_data;
    Toc		toc = scrn->toc;
    MsgList	mlist;

    if (toc == NULL) return;
    mlist = CurMsgListOrCurMsg(toc);
    if (mlist->nummsgs)
	CreateForward(mlist);
    FreeMsgList(mlist);
}


/*ARGSUSED*/
void XmhForward(w, event, params, num_params)
    Widget	w;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    if (UserWantsAction(w, scrn))
	DoForward(w, (XtPointer) scrn, (XtPointer) NULL);
}


/*ARGSUSED*/
void DoTocUseAsComp(w, client_data, call_data)
    Widget	w;
    XtPointer	client_data;
    XtPointer	call_data;
{
    Scrn	scrn = (Scrn) client_data;
    Toc		toc = scrn->toc;
    Scrn	vscrn;
    MsgList	mlist;
    Msg		msg;

    if (toc == NULL) return;
    mlist = CurMsgListOrCurMsg(toc);
    if (mlist->nummsgs) {
	vscrn = NewCompScrn();
	if (DraftsFolder == toc) {
	    msg = mlist->msglist[0];
	} else {
	    msg = TocMakeNewMsg(DraftsFolder);
	    MsgLoadCopy(msg, mlist->msglist[0]);
	    MsgSetTemporary(msg);
	}
	MsgSetScrnForComp(msg, vscrn);
	MapScrn(vscrn);
    }
    FreeMsgList(mlist);
}


/*ARGSUSED*/
void XmhUseAsComposition(w, event, params, num_params)
    Widget	w;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    if (UserWantsAction(w, scrn))
	DoTocUseAsComp(w, (XtPointer) scrn, (XtPointer) NULL);
}


/* Utility: change the fate of a set of messages. */

static MarkMessages(scrn, fate, skip)
Scrn scrn;
FateType fate;
int skip;
{
    Toc toc = scrn->toc;
    Toc desttoc;
    int i;
    MsgList mlist;
    Msg msg;
    if (toc == NULL) return;
    if (fate == Fcopy || fate == Fmove)
	desttoc = SelectedToc(scrn);
    else
	desttoc = NULL;
    if (desttoc == toc)
	Feep();
    else {
	mlist = TocCurMsgList(toc);
	if (mlist->nummsgs == 0) {
	    msg = TocGetCurMsg(toc);
	    if (msg) {
		MsgSetFate(msg, fate, desttoc);
		if (skip)
		    DoNextView(scrn->widget, (XtPointer) scrn,
			       (XtPointer) NULL);
	    }
	} else {
	    for (i = 0; i < mlist->nummsgs; i++)
		MsgSetFate(mlist->msglist[i], fate, desttoc);
	}
	FreeMsgList(mlist);
    }
}


/*ARGSUSED*/
void XmhMarkDelete(w, event, params, num_params)
    Widget	w;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    if (UserWantsAction(w, scrn))
	DoDelete(w, (XtPointer) scrn, (XtPointer) NULL);
}


/*ARGSUSED*/
void DoDelete(w, client_data, call_data)
    Widget	w;
    XtPointer	client_data;
    XtPointer	call_data;
{
    Scrn scrn = (Scrn) client_data;
    MarkMessages(scrn, Fdelete, app_resources.skip_deleted);
}


/*ARGSUSED*/
void DoCopy(w, client_data, call_data)
    Widget	w;
    XtPointer	client_data;
    XtPointer	call_data;
{
    Scrn scrn = (Scrn) client_data;
    MarkMessages(scrn, Fcopy, app_resources.skip_copied);
}


/*ARGSUSED*/
void XmhMarkCopy(w, event, params, num_params)
    Widget	w;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    if (UserWantsAction(w, scrn))
	DoCopy(w, (XtPointer) scrn, (XtPointer) NULL);
}


/*ARGSUSED*/
void DoMove(w, client_data, call_data)
    Widget	w;
    XtPointer	client_data;
    XtPointer	call_data;
{
    Scrn scrn = (Scrn) client_data;
    MarkMessages(scrn, Fmove, app_resources.skip_moved);
}


/*ARGSUSED*/
void XmhMarkMove(w, event, params, num_params)
    Widget	w;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    if (UserWantsAction(w, scrn))
	DoMove(w, (XtPointer) scrn, (XtPointer) NULL);
}


/*ARGSUSED*/
void DoUnmark(w, client_data, call_data)
    Widget	w;
    XtPointer	client_data;
    XtPointer	call_data;
{
    Scrn scrn = (Scrn) client_data;
    MarkMessages(scrn, Fignore, FALSE);
}


/*ARGSUSED*/
void XmhUnmark(w, event, params, num_params)
    Widget	w;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    if (UserWantsAction(w, scrn))
	DoUnmark(w, (XtPointer) scrn, (XtPointer) NULL);
}


/*ARGSUSED*/
void DoCommit(w, client_data, call_data)
    Widget	w;
    XtPointer	client_data;
    XtPointer	 call_data;
{
    Scrn	scrn = (Scrn) client_data;
    TocCommitChanges(w, (XtPointer) scrn->toc, (XtPointer) NULL);
}


/*ARGSUSED*/
void XmhCommitChanges(w, event, params, num_params)
    Widget	w;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    if (UserWantsAction(w, scrn))
	TocCommitChanges(w, (XtPointer) scrn->toc, (XtPointer) NULL);
}


/*ARGSUSED*/
void DoPrint(w, client_data, call_data)
    Widget	w;
    XtPointer	client_data;
    XtPointer	call_data;
{
    Scrn	scrn = (Scrn) client_data;
    Toc		toc = scrn->toc;
    MsgList	mlist;
    char	str[MAX_SYSTEM_LEN], *msg;
    int		i, used, len;

    if (toc == NULL) return;
    mlist = CurMsgListOrCurMsg(toc);
    i = 0;
    if (mlist->nummsgs) {
	while (i < mlist->nummsgs) {
	    (void) strcpy( str, app_resources.print_command );
	    used = strlen(str) + 2;
	    while (i < mlist->nummsgs &&
		   (msg = MsgFileName(mlist->msglist[i])) &&
		   (used + (len = strlen(msg) + 1)) < MAX_SYSTEM_LEN) {
		(void) strcat( str, " " );
		(void) strcat( str, msg );
		used += len;
		i++;
	    }
	    DEBUG( str );
	    (void) system(str);
	}
    }
    else {
	PopupNotice( "print: no messages selected.", 
		    (XtCallbackProc) NULL, (XtPointer) NULL);
    }
    FreeMsgList(mlist);
}


/*ARGSUSED*/
void XmhPrint(w, event, params, num_params)
    Widget	w;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    if (UserWantsAction(w, scrn))
	DoPrint(w, (XtPointer) scrn, (XtPointer) NULL);
}


/*ARGSUSED*/
void DoPack(widget, client_data, call_data)
    Widget	widget;
    XtPointer	client_data;
    XtPointer	call_data;	/* unused */
{
    Scrn	scrn = (Scrn) client_data;
    Toc		toc = scrn->toc;
    XtCallbackRec confirms[2];
    char	**argv;
    
    if (toc == NULL) return;
    confirms[0].callback = (XtCallbackProc) DoPack;
    confirms[0].closure = (XtPointer) scrn;
    confirms[1].callback = (XtCallbackProc) NULL;
    confirms[1].closure = (XtPointer) NULL;
    if (TocConfirmCataclysm(toc, confirms, (XtCallbackRec *) NULL))
	return;
    argv = MakeArgv(4);
    argv[0] = "folder";
    argv[1] = TocMakeFolderName(toc);
    argv[2] = "-pack";
    argv[3] = "-fast";
    if (app_resources.block_events_on_busy) ShowBusyCursor();

    DoCommand(argv, (char *) NULL, (char *) NULL);
    XtFree(argv[1]);
    XtFree((char *) argv);
    TocForceRescan(toc);

    if (app_resources.block_events_on_busy) UnshowBusyCursor();

}


/*ARGSUSED*/
void XmhPackFolder(w, event, params, num_params)
    Widget	w;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    if (UserWantsAction(w, scrn))
	DoPack(w, (XtPointer) scrn, (XtPointer) NULL);
}


/*ARGSUSED*/
void DoSort(widget, client_data, call_data)
    Widget	widget;
    XtPointer	client_data;
    XtPointer	call_data;	/* unused */
{
    Scrn	scrn = (Scrn) client_data;
    Toc		toc = scrn->toc;
    char **	argv;
    XtCallbackRec confirms[2];

    if (toc == NULL) return;
    confirms[0].callback = (XtCallbackProc) DoSort;
    confirms[0].closure = (XtPointer) scrn;
    confirms[1].callback = (XtCallbackProc) NULL;
    confirms[1].closure = (XtPointer) NULL;
    if (TocConfirmCataclysm(toc, confirms, (XtCallbackRec *) NULL))
	return;
    argv = MakeArgv(3);
    argv[0] = "sortm";
    argv[1] = TocMakeFolderName(toc);
    argv[2] = "-noverbose";
    if (app_resources.block_events_on_busy) ShowBusyCursor();

    DoCommand(argv, (char *) NULL, (char *) NULL);
    XtFree(argv[1]);
    XtFree((char *) argv);
    TocForceRescan(toc);

    if (app_resources.block_events_on_busy) UnshowBusyCursor();
}


/*ARGSUSED*/
void XmhSortFolder(w, event, params, num_params)
    Widget	w;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    if (UserWantsAction(w, scrn))
	DoSort(w, (XtPointer) scrn, (XtPointer) NULL);
}


/*ARGSUSED*/
void XmhForceRescan(w, event, params, num_params)
    Widget	w;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    if (UserWantsAction(w, scrn))
	DoForceRescan(w, (XtPointer) scrn, (XtPointer) NULL);
}

/*ARGSUSED*/
void DoForceRescan(w, client_data, call_data)
    Widget	w;
    XtPointer	client_data;
    XtPointer	call_data;
{
    Scrn	scrn = (Scrn) client_data;
    Toc		toc = scrn->toc;
    if (toc == NULL) return;
    if (app_resources.block_events_on_busy) ShowBusyCursor();

    TocForceRescan(toc);
    
    if (app_resources.block_events_on_busy) UnshowBusyCursor();
}


/* Incorporate new mail. */

/*ARGSUSED*/
void XmhIncorporateNewMail(w, event, params, num_params)
    Widget	w;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    if (UserWantsAction(w, scrn)) {
	if (TocCanIncorporate(scrn->toc))
	    DoIncorporateNewMail(w, (XtPointer) scrn, (XtPointer) NULL);
    }
}


/*ARGSUSED*/
void DoIncorporateNewMail(w, client_data, call_data)
    Widget	w;		/* unused */
    XtPointer	client_data;	/* screen */
    XtPointer	call_data;	/* unused */
{
    Scrn scrn = (Scrn) client_data;
    if (scrn->toc == NULL) return;
    TocIncorporate(scrn->toc);
    TocCheckForNewMail();
}


/*ARGSUSED*/
void DoReply(w, client_data, call_data)
    Widget	w;
    XtPointer	client_data;
    XtPointer	call_data;
{
    Scrn	scrn = (Scrn) client_data;
    Toc		toc = scrn->toc;
    Scrn	nscrn;
    MsgList	mlist;
    Msg		msg;

    if (toc == NULL) return;
    mlist = CurMsgListOrCurMsg(toc);
    if (mlist->nummsgs) {
	nscrn = NewCompScrn();
	ScreenSetAssocMsg(nscrn, mlist->msglist[0]);
	msg = TocMakeNewMsg(DraftsFolder);
	MsgSetTemporary(msg);
	MsgLoadReply(msg, mlist->msglist[0]);
	MsgSetScrnForComp(msg, nscrn);
	MapScrn(nscrn);
    }
    FreeMsgList(mlist);
}
    

/*ARGSUSED*/
void XmhReply(w, event, params, num_params)
    Widget	w;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    if (UserWantsAction(w, scrn))
	DoReply(w, (XtPointer) scrn, (XtPointer) NULL);
}


/*ARGSUSED*/
void DoPickMessages(w, client_data, call_data)
    Widget	w;
    XtPointer	client_data;
    XtPointer	call_data;
{
    Scrn	scrn = (Scrn) client_data;
    Toc		toc = scrn->toc;
    Scrn	nscrn;
    char *	toseq;
    Sequence	selectedseq;
    Boolean	recycled;

    if (toc == NULL) return;
    if ((selectedseq = TocSelectedSequence(toc)) == NULL)
	toseq = "temp";
    else {
	toseq = selectedseq->name;
	if (strcmp(toseq, "all") == 0)
	    toseq = "temp";
    }
    nscrn = CreateNewScrn(STpick);
    recycled = (nscrn->pick) ? True : False;
    AddPick(nscrn, toc, (TocViewedSequence(toc))->name, toseq);
    DEBUG("Realizing Pick...")
    XtRealizeWidget(nscrn->parent);
    DEBUG(" done.\n")
    if (! recycled) {
	InitBusyCursor(nscrn);
	XDefineCursor(XtDisplay(nscrn->parent), XtWindow(nscrn->parent),
		      app_resources.cursor);
	XSetWMProtocols(XtDisplay(toplevel), XtWindow(nscrn->parent),
			&wm_delete_window, 1);
    }
    MapScrn(nscrn);
}


/*ARGSUSED*/
void XmhPickMessages(w, event, params, num_params)
    Widget	w;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    if (UserWantsAction(w, scrn))
	DoPickMessages(w, (XtPointer) scrn, (XtPointer) NULL);
}


/*ARGSUSED*/
void DoSelectSequence(widget, client_data, call_data)
    Widget	widget;		/* sequence menu entry object */
    XtPointer	client_data;	/* the screen */
    XtPointer	call_data;
{
    Scrn	scrn = (Scrn) client_data;
    Toc		toc  = (Toc) scrn->toc;
    Sequence	seq;

    if ((seq = TocSelectedSequence(toc)) != NULL) {
	Widget	item, menu;
	Button	button;

	button = BBoxFindButtonNamed
	    (scrn->mainbuttons, MenuBoxButtons[XMH_SEQUENCE].button_name);
	menu = BBoxMenuOfButton(button);
	if ((item = XtNameToWidget(menu, seq->name)) != NULL)
	    ToggleMenuItem(item, False);
    }

    ToggleMenuItem(widget, True);
    TocSetSelectedSequence(toc, TocGetSeqNamed(toc, XtName(widget)));
}


/*ARGSUSED*/
void DoOpenSeq(w, client_data, call_data)
    Widget	w;
    XtPointer	client_data;
    XtPointer	call_data;
{
    Scrn	scrn = (Scrn) client_data;
    Toc		toc = scrn->toc;
    if (toc == NULL) return;
    TocChangeViewedSeq(toc, TocSelectedSequence(toc));
}


/*ARGSUSED*/
void XmhOpenSequence(w, event, params, num_params)
    Widget	w;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    Widget	entry_object;
    Scrn	scrn = ScrnFromWidget(w);
    Sequence	selected_sequence;

    /* In case this action is called from translations defined by the
     * user on folder menu buttons or on folder menu widgets.
     */
    if (! UserWantsAction(w, scrn))
	return;

    /* In case there is nothing to do anyway. */
    if (! TocHasSequences(scrn->toc))
	return;

    /* In case the action was given the name of a sequence to open. */
    if (*num_params) {
	Toc	toc = scrn->toc;
	if (selected_sequence = TocGetSeqNamed(toc, params[0])) {
	    TocSetSelectedSequence(toc, selected_sequence);
	    TocChangeViewedSeq(toc, selected_sequence);
	}
	return;
    }

    /* In case this action is a translation on the sequence menu.  */

    if ((strcmp(XtName(w), "sequenceMenu") == 0) &&
	(event->type == ButtonRelease)) {

	/* The user released the mouse button.  We must distinguish between
	 * a button release on a selectable menu entry, and a button release
	 * occuring elsewhere.  The button releases occuring elsewhere are 
	 * either outside of the menu, or on unselectable menu entries.
	 */

	if ((entry_object = XawSimpleMenuGetActiveEntry(w)) == NULL)
	    return;

	/* Some entry in the menu was selected.  The menu entry's callback
	 * procedure has already executed.  If a sequence name was selected,
	 * the callback procedure has caused that sequence to become the
	 * currently selected sequence.  If selected menu entry object's 
	 * name matches the currently selected sequence, we should open
	 * that sequence.  Otherwise, the user must have selected a sequence
	 * manipulation command, such as Pick.  The assumptions here are that
	 * the name of a menu entry object which represents a sequence is
	 * identical to the name of the sequence, and in the translations,
	 * that the notify() action was specified before this action.
	 */

	if ((selected_sequence = TocSelectedSequence(scrn->toc)) &&
	    (strcmp(XtName(entry_object), selected_sequence->name) == 0))
	    DoOpenSeq(w, (XtPointer) scrn, (XtPointer) NULL);
	return;
    }
    
    /* An accelerator open sequence function */

    DoOpenSeq(w, (XtPointer) scrn, (XtPointer) NULL);
}


typedef enum {ADD, REMOVE, DELETE} TwiddleOperation;

static TwiddleSequence(scrn, op)
Scrn scrn;
TwiddleOperation op;
{
    Toc toc = scrn->toc;
    char **argv, str[100];
    int i;
    MsgList mlist;
    Sequence	selectedseq;

    if (toc == NULL || ((selectedseq = TocSelectedSequence(toc)) == NULL))
	return;
    if (strcmp(selectedseq->name, "all") == 0) {
	Feep();
	return;
    }
    if (op == DELETE)
	mlist = MakeNullMsgList();
    else {
	mlist = CurMsgListOrCurMsg(toc);
	if (mlist->nummsgs == 0) {
	    FreeMsgList(mlist);
	    Feep();
	    return;
	}
    }
    argv = MakeArgv(6 + mlist->nummsgs);
    argv[0] = "mark";
    argv[1] = TocMakeFolderName(toc);
    argv[2] = "-sequence";
    argv[3] = selectedseq->name;
    switch (op) {
      case ADD:
	argv[4] = "-add";
	argv[5] = "-nozero";
	break;
      case REMOVE:
	argv[4] = "-delete";
	argv[5] = "-nozero";
	break;
      case DELETE:
	argv[4] = "-delete";
	argv[5] = "all";
	break;
    }
    for (i = 0; i < mlist->nummsgs; i++) {
	(void) sprintf(str, "%d", MsgGetId(mlist->msglist[i]));
	argv[6 + i] = XtNewString(str);
    }
    DoCommand(argv, (char *) NULL, (char *) NULL);
    for (i = 0; i < mlist->nummsgs; i++)
        XtFree((char *) argv[6 + i]);
    XtFree(argv[1]);
    XtFree((char *) argv);
    FreeMsgList(mlist);
    TocReloadSeqLists(toc);
}


/*ARGSUSED*/
void DoAddToSeq(w, client_data, call_data)
    Widget	w;
    XtPointer	client_data;
    XtPointer	call_data;
{
    Scrn	scrn = (Scrn) client_data;
    TwiddleSequence(scrn, ADD);
}


/*ARGSUSED*/
void XmhAddToSequence(w, event, params, num_params)
    Widget	w;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    if (! UserWantsAction(w, scrn))
	return;
    if ((strcmp(XtName(w), "sequenceMenu") == 0) &&
	(event->type == ButtonRelease) &&
	(XawSimpleMenuGetActiveEntry(w) == NULL))
	return;
    if (TocHasSequences(scrn->toc))
	TwiddleSequence(scrn, ADD);
}


/*ARGSUSED*/
void DoRemoveFromSeq(w, client_data, call_data)
    Widget	w;
    XtPointer	client_data;
    XtPointer	call_data;
{
    Scrn	scrn = (Scrn) client_data;
    TwiddleSequence(scrn, REMOVE);
}


/*ARGSUSED*/
void XmhRemoveFromSequence(w, event, params, num_params)
    Widget	w;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    if (UserWantsAction(w, scrn))
	if (TocHasSequences(scrn->toc))
	    TwiddleSequence(scrn, REMOVE);
}


/*ARGSUSED*/
void DoDeleteSeq(w, client_data, call_data)
    Widget	w;
    XtPointer	client_data;
    XtPointer	call_data;
{
    Scrn	scrn = (Scrn) client_data;
    TwiddleSequence(scrn, DELETE);
    TUCheckSequenceMenu(scrn->toc);
}


/*ARGSUSED*/
void XmhDeleteSequence(w, event, params, num_params)
    Widget	w;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    if (! UserWantsAction(w, scrn))
	return;
    if ((strcmp(XtName(w), "sequenceMenu") == 0) &&
	(event->type == ButtonRelease) &&
	(XawSimpleMenuGetActiveEntry(w) == NULL))
	return;
    if (TocHasSequences(scrn->toc))
	DoDeleteSeq(w, (XtPointer) scrn, (XtPointer) NULL);
}

