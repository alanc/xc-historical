#if !defined(lint) && !defined(SABER)
static char rcs_id[] =
    "$XConsortium: tocfuncs.c,v 2.22 89/08/31 19:11:25 converse Exp $";
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

#define MAX_SYSTEM_LEN 510

/*ARGSUSED*/
void DoNextView(widget, client_data, call_data)
    Widget	widget;		/* unused */
    XtPointer	client_data;
    XtPointer	call_data;	/* unused */
{
    Scrn scrn = (Scrn) client_data;
    Toc toc = scrn->toc;
    MsgList mlist;
    FateType fate;
    Msg msg;

    if (toc == NULL) return;
    mlist = TocCurMsgList(toc);
    if (mlist->nummsgs)
	msg = mlist->msglist[0];
    else {
	msg = TocGetCurMsg(toc);
	if (msg && msg == scrn->msg) msg = TocMsgAfter(toc, msg);
	if (msg) fate = MsgGetFate(msg, (Toc *)NULL);
	while (msg && ((app_resources.SkipDeleted && fate == Fdelete)
		|| (app_resources.SkipMoved && fate == Fmove)
		|| (app_resources.SkipCopied && fate == Fcopy))) {
	    msg = TocMsgAfter(toc, msg);
	    if (msg) fate = MsgGetFate(msg, (Toc *)NULL);
	}
    }
    if (msg) {
	XtCallbackRec	confirms[2];
	confirms[0].callback = (XtCallbackProc) DoNextView;
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
void XmhViewNextMessage(w, event, params, num_params)
    Widget	w;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    DoNextView(w, (XtPointer) scrn, (XtPointer) NULL);
}


/*ARGSUSED*/
void DoPrevView(widget, client_data, call_data)
    Widget	widget;		/* unused */
    XtPointer	client_data;	
    XtPointer	call_data;	/* unused */
{
    Scrn scrn = (Scrn) client_data;
    Toc toc = scrn->toc;
    MsgList mlist;
    FateType fate;
    Msg msg;
    if (toc == NULL) return;
    mlist = TocCurMsgList(toc);
    if (mlist->nummsgs)
	msg = mlist->msglist[mlist->nummsgs - 1];
    else {
	msg = TocGetCurMsg(toc);
	if (msg && msg == scrn->msg) msg = TocMsgBefore(toc, msg);
	if (msg) fate = MsgGetFate(msg, (Toc *)NULL);
	while (msg && ((app_resources.SkipDeleted && fate == Fdelete)
		|| (app_resources.SkipMoved && fate == Fmove)
		|| (app_resources.SkipCopied && fate == Fcopy))) {
	    msg = TocMsgBefore(toc, msg);
	    if (msg) fate = MsgGetFate(msg, (Toc *)NULL);
	}
    }
    if (msg) {
	XtCallbackRec	confirms[2];
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
void XmhViewPreviousMessage(w, event, params, num_params)
    Widget	w;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    Scrn scrn = ScrnFromWidget(w);
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
		    XmhViewNextMessage(scrn->widget, (XEvent *) NULL,
				       (String *) NULL, (Cardinal *) NULL);
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
    DoDelete(w, (XtPointer) scrn, (XtPointer) NULL);
}


/*ARGSUSED*/
void DoDelete(w, client_data, call_data)
    Widget	w;
    XtPointer	client_data;
    XtPointer	call_data;
{
    Scrn scrn = (Scrn) client_data;
    MarkMessages(scrn, Fdelete, app_resources.SkipDeleted);
}


/*ARGSUSED*/
void DoCopy(w, client_data, call_data)
    Widget	w;
    XtPointer	client_data;
    XtPointer	call_data;
{
    Scrn scrn = (Scrn) client_data;
    MarkMessages(scrn, Fcopy, app_resources.SkipCopied);
}


/*ARGSUSED*/
void XmhMarkCopy(w, event, params, num_params)
    Widget	w;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    DoCopy(w, (XtPointer) scrn, (XtPointer) NULL);
}


/*ARGSUSED*/
void DoMove(w, client_data, call_data)
    Widget	w;
    XtPointer	client_data;
    XtPointer	call_data;
{
    Scrn scrn = (Scrn) client_data;
    MarkMessages(scrn, Fmove, app_resources.SkipMoved);
}


/*ARGSUSED*/
void XmhMarkMove(w, event, params, num_params)
    Widget	w;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    Scrn scrn = ScrnFromWidget(w);
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
	    (void) strcpy( str, app_resources.defPrintCommand );
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
	PopupNotice( "print: no messages selected", NULL, NULL );
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
    DoCommand(argv, (char *) NULL, (char *) NULL);
    XtFree(argv[1]);
    XtFree((char *) argv);
    TocForceRescan(toc);
}


/*ARGSUSED*/
void XmhPackFolder(w, event, params, num_params)
    Widget	w;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    DoPack(w, (XtPointer) scrn, (XtPointer) NULL);
}


/*ARGSUSED*/
void DoSort(widget, client_data, call_data)
    Widget	widget;
    XtPointer	client_data;
    XtPointer	call_data;	/* unused */
{
    Scrn scrn = (Scrn) client_data;
    Toc toc = scrn->toc;
    char **argv;
    XtCallbackRec confirms[2];

    if (toc == NULL) return;

    confirms[0].callback = (XtCallbackProc) DoPack;
    confirms[0].closure = (XtPointer) scrn;
    confirms[1].callback = (XtCallbackProc) NULL;
    confirms[1].closure = (XtPointer) NULL;

    if (TocConfirmCataclysm(toc, confirms, (XtCallbackRec *) NULL))
	return;
    argv = MakeArgv(3);
    argv[0] = "sortm";
    argv[1] = TocMakeFolderName(toc);
    argv[2] = "-noverbose";
    DoCommand(argv, (char *) NULL, (char *) NULL);
    XtFree(argv[1]);
    XtFree((char *) argv);
    TocForceRescan(toc);
}


/*ARGSUSED*/
void XmhSortFolder(w, event, params, num_params)
    Widget	w;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    Scrn scrn = ScrnFromWidget(w);
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
    Rescan(w, (XtPointer) scrn, (XtPointer) NULL);
}

/*ARGSUSED*/
void Rescan(w, client_data, call_data)
    Widget	w;
    XtPointer	client_data;
    XtPointer	call_data;
{
    Scrn scrn = (Scrn) client_data;
    Toc toc = scrn->toc;
    if (toc == NULL) return;
    TocForceRescan(toc);
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
    Inc(w, (XtPointer) scrn, (XtPointer) NULL);
}


/*ARGSUSED*/
void Inc(w, client_data, call_data)
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
void Reply(w, client_data, call_data)
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
    Reply(w, (XtPointer) scrn, (XtPointer) NULL);
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

    if (toc == NULL) return;
    if ((toseq = RadioBBoxGetCurrent(scrn->seqbuttons)) == (char *) NULL)
	toseq = "temp";
    if (strcmp(toseq, "all") == 0)
	toseq = "temp";
    nscrn = CreateNewScrn(STpick);
    AddPick(nscrn, toc, TocViewedSequence(toc)->name, toseq);
    DEBUG("Realizing Pick...")
    XtRealizeWidget(nscrn->parent);
    DEBUG(" done.\n")
    XDefineCursor(XtDisplay(nscrn->parent), XtWindow(nscrn->parent),
		  app_resources.cursor );
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
    DoPickMessages(w, (XtPointer) scrn, (XtPointer) NULL);
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
    TocChangeViewedSeq
	(toc, TocGetSeqNamed(toc, RadioBBoxGetCurrent(scrn->seqbuttons)));
}


/*ARGSUSED*/
void XmhOpenSequence(w, event, params, num_params)
    Widget	w;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    DoOpenSeq(w, (XtPointer) scrn, (XtPointer) NULL);
}


typedef enum {ADD, REMOVE, DELETE} TwiddleOperation;

static TwiddleSequence(scrn, op)
Scrn scrn;
TwiddleOperation op;
{
    Toc toc = scrn->toc;
    char **argv, str[100], *seqname;
    int i;
    MsgList mlist;
    if (toc == NULL ||
	(seqname = RadioBBoxGetCurrent(scrn->seqbuttons)) == NULL) return;
    if (strcmp(seqname, "all") == 0) {
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
    argv[3] = seqname;
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
}


/*ARGSUSED*/
void XmhDeleteSequence(w, event, params, num_params)
    Widget	w;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    TwiddleSequence(scrn, DELETE);
}


