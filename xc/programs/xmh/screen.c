#if !defined(lint) && !defined(SABER)
static char rcs_id[] =
    "$XConsortium: screen.c,v 2.48 89/09/17 19:40:54 converse Exp $";
#endif
/*
 *		        COPYRIGHT 1987, 1989
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

/* scrn.c -- management of scrns. */

#include "xmh.h"


typedef struct _XmhMenuEntry {
    char	*name;			/* menu entry name */
    void   	(*function)();		/* menu entry callback function */
} XmhMenuEntryRec, *XmhMenuEntry;	


typedef struct _XmhMenuButtonDesc {
    char	*button_name;		/* menu button name */
    char	*menu_name;		/* menu name */
    XmhMenuEntry entry;			/* list of menu entries */
    Cardinal	num_entries;		/* count of menu entries in list */
} XmhMenuButtonDescRec, *XmhMenuButtonDesc;


XmhMenuEntryRec	folderOps[] = {
    {"compose",			DoComposeMessage},
    {"close",			DoClose},
    {"XawMenuSeparator",	(XtCallbackProc) NULL},
    {"open",			DoOpenFolder},
    {"openInNew", 		DoOpenFolderInNewWindow},
    {"create",			DoCreateFolder},
    {"delete",			DoDeleteFolder},
};

XmhMenuEntryRec	tocOps[] = {
    {"inc",			DoIncorporateNewMail},
    {"commit",			DoCommit},
    {"pack",			DoPack},
    {"sort",			DoSort},
    {"rescan",			DoForceRescan},
};

XmhMenuEntryRec	messageOps[] = {
    {"next",			DoNextView},
    {"prev",			DoPrevView},
    {"delete",			DoDelete},
    {"move",			DoMove},
    {"copy",			DoCopy},
    {"unmark",			DoUnmark},
    {"viewNew",			DoViewNew},
    {"reply",			DoReply},
    {"forward",			DoForward},
    {"useAsComp",		DoTocUseAsComp},
    {"print",			DoPrint},
};

XmhMenuEntryRec	sequenceOps[] = {
    {"pick",			DoPickMessages},
    {"openSeq",			DoOpenSeq},
    {"addToSeq",		DoAddToSeq},
    {"removeFromSeq",		DoRemoveFromSeq},
    {"deleteSeq",		DoDeleteSeq},
};

XmhMenuEntryRec	sequences[] = {
    {"all",			(XtCallbackProc) NULL},
};

XmhMenuEntryRec	viewOps[] = {
    {"reply",			DoViewReply},
    {"forward",			DoViewForward},
    {"useAsComp",		DoViewUseAsComposition},
    {"edit",			DoEditView},
    {"save",			DoSaveView},
    {"print",			DoPrintView},
};

XmhMenuEntryRec	options[] = {
    {"reverse",			DoReverseReadOrder},
};

XmhMenuButtonDescRec	MenuBoxButtons[] = {
    {"folderOpsButton",
     "folderOpsMenu",	folderOps,	XtNumber(folderOps) },
    {"tocOpsButton",
     "tocOpsMenu",	tocOps,		XtNumber(tocOps) },
    {"messageOpsButton",
     "messageOpsMenu",	messageOps,	XtNumber(messageOps) },
    {"sequenceOpsButton",
     "sequenceOpsMenu",	sequenceOps,	XtNumber(sequenceOps) },
    {"sequenceButton",
     "sequenceMenu",	sequences,	XtNumber(sequences) },
    {"viewOpsButton",
     "viewOpsMenu",	viewOps,	XtNumber(viewOps) },
    {"optionButton",
     "optionMenu",	options,	XtNumber(options) },
};


/* Fill in the buttons for the view commands. */

static void FillViewButtons(scrn)
Scrn scrn;
{
    ButtonBox buttonbox = scrn->viewbuttons;
    BBoxAddButton(buttonbox, "close", commandWidgetClass, True);
    BBoxAddButton(buttonbox, "reply", commandWidgetClass, True);
    BBoxAddButton(buttonbox, "forward", commandWidgetClass, True);
    BBoxAddButton(buttonbox, "useAsComp", commandWidgetClass, True);
    BBoxAddButton(buttonbox, "edit", commandWidgetClass, True);
    BBoxAddButton(buttonbox, "save", commandWidgetClass, False);
    BBoxAddButton(buttonbox, "print", commandWidgetClass, True);
}
    


static void FillCompButtons(scrn)
Scrn scrn;
{
    ButtonBox buttonbox = scrn->viewbuttons;
    BBoxAddButton(buttonbox, "close", commandWidgetClass, True);
    BBoxAddButton(buttonbox, "send", commandWidgetClass, True);
    BBoxAddButton(buttonbox, "reset", commandWidgetClass, True);
    BBoxAddButton(buttonbox, "compose", commandWidgetClass, True);
    BBoxAddButton(buttonbox, "save", commandWidgetClass, True);
    BBoxAddButton(buttonbox, "insert", commandWidgetClass, True);
}


static void MakeCommandMenu(scrn, mbd)
    Scrn		scrn;
    XmhMenuButtonDesc	mbd;
{
    register int i, n;
    Widget	menu;
    ButtonBox	buttonbox = scrn->mainbuttons;
    XmhMenuEntry	e;
    Arg		args[4];
    static XtCallbackRec callbacks[] = {
	{ (XtCallbackProc) NULL, (XtPointer) NULL},
	{ (XtCallbackProc) NULL, (XtPointer) NULL},
	{ (XtCallbackProc) NULL, (XtPointer) NULL},
    };
    extern void DoRememberMenuSelection();

    /* Menus are created as childen of the shell of the scrn in order
     * that they can be used both as pop-up and as pull-down menus.
     */

    menu = XtCreatePopupShell(mbd->menu_name, simpleMenuWidgetClass,
			      scrn->widget, args, (Cardinal) 0);

    e = mbd->entry;
    for (i=0; i < mbd->num_entries; i++, e++) {
	n = 0;
	if (e->function) {
	    callbacks[0].callback = e->function;
	    callbacks[0].closure  = (XtPointer) scrn;
	    if (app_resources.sticky_menu) {	/* this may disappear */
		callbacks[1].callback = DoRememberMenuSelection;
		callbacks[1].closure  = (XtPointer) e->name;
	    } else {
		callbacks[1].callback = (XtCallbackProc) NULL;
		callbacks[1].closure  = (XtPointer) NULL;
	    }
	    XtSetArg(args[n], XtNcallback, callbacks);		n++;
	}
	else if (strcmp(e->name, "XawMenuSeparator") == 0) {
	    XtSetArg(args[n], XtNtype, XawMenuSeparator);	n++;
	}

	XawSimpleMenuAddEntry(menu, XtNewString(e->name), args, (Cardinal) n);
    }

    AttachMenuToButton( BBoxFindButtonNamed( buttonbox, mbd->button_name),
		       menu, mbd->menu_name);

    /* awkward */
    if (strcmp("optionMenu", mbd->menu_name) == 0) {
	n = 0;
	XtSetArg(args[n], XtNleftMargin, 18);	n++;
	XtSetValues(menu, args, (Cardinal) n);
	if (app_resources.reverse_read_order)
	    ToggleMenuItem(menu, "reverse", True);
    }
}

/* Create subwidgets for a toc&view window. */

static void MakeTocAndView(scrn)
Scrn scrn;
{
    register int	i;
    XmhMenuButtonDesc	mbd;
    ButtonBox		buttonbox;
    char		*name;
    static XawTextSelectType sarray[] = {XawselectLine,
					XawselectAll,
					XawselectPosition,
					XawselectNull};
    static Arg args[] = {
	{ XtNselectTypes,	(XtArgVal) sarray},
	{ XtNdisplayCaret,	(XtArgVal) False}
    };

    scrn->folderlabel   = CreateTitleBar(scrn, "folderTitlebar");
    scrn->folderbuttons = BBoxCreate(scrn, "folders");
    scrn->seqbuttons    = RadioBBoxCreate(scrn, "seqButtons");
    scrn->mainbuttons   = BBoxCreate(scrn, "menuBox");
    scrn->toclabel      = CreateTitleBar(scrn, "tocTitlebar");
    scrn->tocwidget	= CreateTextSW(scrn, "toc", args, XtNumber(args));
    if (app_resources.command_button_count > 0) 
	scrn->miscbuttons = BBoxCreate(scrn, "commandBox");
    scrn->viewlabel     = CreateTitleBar(scrn, "viewTitlebar");
    scrn->viewwidget    = CreateTextSW(scrn, "view", args, (Cardinal) 0);

    /* the folder buttons and menus */

    buttonbox = scrn->folderbuttons;
    for (i=0 ; i<numFolders ; i++) {
	name = TocName(folderList[i]);
	if (numScrns == 1 || (! IsSubfolder(name)))
	    BBoxAddButton(buttonbox, name, menuButtonWidgetClass, True);
    }

    /* the command buttons and menus */

    buttonbox = scrn->mainbuttons;
    mbd = MenuBoxButtons;
    for (i=0; i < XtNumber(MenuBoxButtons); i++, mbd++) {
	name = mbd->button_name;
	BBoxAddButton(buttonbox, name, menuButtonWidgetClass, True);
	MakeCommandMenu(scrn, mbd);
    }

    /* the sequence buttons, soon to disappear? */

    buttonbox = scrn->seqbuttons;
    RadioBBoxAddButton(buttonbox, "all", True);

    /* the optional miscellaneous command buttons */

    if (app_resources.command_button_count > 0) {
	char	name[30];
	if (app_resources.command_button_count > 1000)
	    app_resources.command_button_count = 1000;
	for (i=1; i <= app_resources.command_button_count; i++) {
	    sprintf(name, "button%d", i);
	    BBoxAddButton(scrn->miscbuttons, name, commandWidgetClass, True);
	}
    }

    if (app_resources.mailWaitingFlag) {
	static Arg arglist[] = {XtNiconPixmap, NULL};
	arglist[0].value = (XtArgVal) NoMailPixmap;
	XtSetValues(scrn->parent, arglist, XtNumber(arglist));
    }
}


static void MakeView(scrn)
Scrn scrn;
{
    scrn->viewlabel = CreateTitleBar(scrn, "viewTitlebar");
    scrn->viewwidget = CreateTextSW(scrn, "view", (ArgList)NULL, (Cardinal)0);
    scrn->viewbuttons = BBoxCreate(scrn, "viewButtons");
    FillViewButtons(scrn);
}


static void MakeComp(scrn)
Scrn scrn;
{
    scrn->viewlabel = CreateTitleBar(scrn, "composeTitlebar");
    scrn->viewwidget = CreateTextSW(scrn, "comp", (ArgList)NULL, (Cardinal)0);
    scrn->viewbuttons = BBoxCreate(scrn, "compButtons");
    FillCompButtons(scrn);
}


/* Create a scrn of the given type. */

Scrn CreateNewScrn(kind)
ScrnKind kind;
{
    int i;
    Scrn scrn;
    static Arg arglist[] = {
	{ XtNgeometry,	(XtArgVal) NULL},
	{ XtNinput,	(XtArgVal) True}
    };

    for (i=0 ; i<numScrns ; i++)
	if (scrnList[i]->kind == kind && !scrnList[i]->mapped)
	    return scrnList[i];
    switch (kind) {
       case STtocAndView: arglist[0].value =
			   (XtArgVal)app_resources.defTocGeometry;	break;
       case STview:	  arglist[0].value =
			   (XtArgVal)app_resources.defViewGeometry;	break;
       case STcomp:	  arglist[0].value =
			   (XtArgVal)app_resources.defCompGeometry;	break;
       case STpick:	  arglist[0].value =
			   (XtArgVal)app_resources.defPickGeometry;	break;
    }

    numScrns++;
    scrnList = (Scrn *)
	XtRealloc((char *) scrnList, (unsigned) numScrns*sizeof(Scrn));
    scrn = scrnList[numScrns - 1] = XtNew(ScrnRec);
    bzero((char *)scrn, sizeof(ScrnRec));
    scrn->kind = kind;
    if (numScrns == 1) scrn->parent = toplevel;
    else scrn->parent = XtCreatePopupShell(
				   progName, topLevelShellWidgetClass,
				   toplevel, arglist, XtNumber(arglist));
    scrn->widget =
	XtCreateManagedWidget(progName, panedWidgetClass, scrn->parent,
			      (ArgList) NULL, (Cardinal) 0);

    switch (kind) {
	case STtocAndView:	MakeTocAndView(scrn);	break;
	case STview:		MakeView(scrn);	break;
	case STcomp:		MakeComp(scrn);	break;
    }

    if (kind != STpick) {
	int	theight, min, max;
	Arg	args[1];

	DEBUG("Realizing...")
	XtRealizeWidget(scrn->parent);
	DEBUG(" done.\n")

	switch (kind) {
	  case STtocAndView:

	    BBoxLockSize(scrn->folderbuttons);
	    BBoxLockSize(scrn->seqbuttons);	/* %%% too early, only one */
	    BBoxLockSize(scrn->mainbuttons);
	    theight = GetHeight(scrn->tocwidget) + GetHeight(scrn->viewwidget);
	    theight = app_resources.defTocPercentage * theight / 100;
	    XawPanedGetMinMax((Widget) scrn->tocwidget, &min, &max);
	    XawPanedSetMinMax((Widget) scrn->tocwidget, theight, theight);
	    XawPanedSetMinMax((Widget) scrn->tocwidget, min, max);
	    if (scrn->miscbuttons)
		BBoxLockSize(scrn->miscbuttons);

	    /* Install accelerators; not active while editing in the view */

	    XtSetArg(args[0], XtNtranslations, &(scrn->edit_translations));
	    XtGetValues(scrn->viewwidget, args, (Cardinal) 1);
	    XtInstallAllAccelerators(scrn->widget, scrn->widget);
	    XtInstallAllAccelerators(scrn->tocwidget, scrn->widget);
	    XtInstallAllAccelerators(scrn->viewwidget, scrn->widget);
	    XtSetArg(args[0], XtNtranslations, &(scrn->read_translations));
	    XtGetValues(scrn->viewwidget, args, (Cardinal) 1);
	    break;

	  case STview:
	  case STcomp:
	    BBoxLockSize(scrn->viewbuttons);
	    XtInstallAllAccelerators(scrn->widget, scrn->widget);
	    XtInstallAllAccelerators(scrn->viewwidget, scrn->widget);
	    break;
	}

	InitBusyCursor(scrn);
	XDefineCursor(XtDisplay(scrn->parent), XtWindow(scrn->parent),
		      app_resources.cursor);
    }
    scrn->mapped = (numScrns == 1);
    return scrn;
}


Scrn NewViewScrn()
{
    return CreateNewScrn(STview);
}

Scrn NewCompScrn()
{
    Scrn scrn;
    scrn = CreateNewScrn(STcomp);
    scrn->assocmsg = (Msg)NULL;
    return scrn;
}

void ScreenSetAssocMsg(scrn, msg)
  Scrn scrn;
  Msg msg;
{
    scrn->assocmsg = msg;
}

/* Destroy the screen.  If unsaved changes are in a msg, too bad. */

void DestroyScrn(scrn)
  Scrn scrn;
{
    XtPopdown(scrn->parent);		/* cannot popdown the first one? */
    TocSetScrn((Toc) NULL, scrn);
    MsgSetScrnForce((Msg) NULL, scrn);
    scrn->mapped = FALSE;
    lastInput.win = -1;
}


void MapScrn(scrn)
Scrn scrn;
{
    if (!scrn->mapped) {
	XtPopup(scrn->parent, XtGrabNone);
	scrn->mapped = True;
    }
}


Scrn ScrnFromWidget(w)		/* heavily used, should be efficient */
Widget w;
{
    register int i;
    while (w && XtClass(w) != panedWidgetClass)
	w = XtParent(w);
    if (w) {
	for (i=0 ; i<numScrns ; i++) {
	    if (w == (Widget) scrnList[i]->widget)
		return scrnList[i];
	}
    }
    Punt("ScrnFromWidget failed!");
    /*NOTREACHED*/
}
 

/* Figure out which buttons should and shouldn't be enabled in the given
 * screen.  This should be called whenever something major happens to the
 * screen.
 */


/*ARGSUSED*/
static void EnableCallback(w, data, junk)
Widget w;
XtPointer data, junk;
{
  EnableProperButtons( (Scrn) data);
}  


#define SetButton(buttonbox, name, value) \
    if (value) BBoxEnable(BBoxFindButtonNamed(buttonbox, name)); \
    else BBoxDisable(BBoxFindButtonNamed(buttonbox, name));


void EnableProperButtons(scrn)
Scrn scrn;
{
    static void EnableCallback();
    int value, changed, reapable;
    Button	button;

    DEBUG("EnbleProperButtons\n")
    if (scrn) {
	switch (scrn->kind) {
	  case STtocAndView:
	    button = BBoxFindButtonNamed(scrn->mainbuttons, "tocOpsButton");
	    value = TocCanIncorporate(scrn->toc);
	    SendMenuEntryEnableMsg(button, "inc", value);

	    button = BBoxFindButtonNamed(scrn->mainbuttons,
					 "sequenceOpsButton");
	    value = TocHasSequences(scrn->toc);
	    SendMenuEntryEnableMsg(button, "openSeq", value);
	    SendMenuEntryEnableMsg(button, "addToSeq", value);
	    SendMenuEntryEnableMsg(button, "removeFromSeq", value);
	    SendMenuEntryEnableMsg(button, "deleteSeq", value);

	    button = BBoxFindButtonNamed(scrn->mainbuttons, "viewOpsButton");
	    value = (scrn->msg != NULL && !MsgGetEditable(scrn->msg));
	    SendMenuEntryEnableMsg(button, "edit", value);
	    SendMenuEntryEnableMsg(button, "save",
				   scrn->msg != NULL && !value);
	    break;
	  case STview:
	    value = (scrn->msg != NULL && !MsgGetEditable(scrn->msg));
	    SetButton(scrn->viewbuttons, "edit", value);
	    SetButton(scrn->viewbuttons, "save", scrn->msg != NULL && !value);
	    break;
	  case STcomp:
	    if (scrn->msg != NULL) {
		changed = MsgChanged(scrn->msg);
		reapable = MsgGetReapable(scrn->msg);
		SetButton(scrn->viewbuttons, "send", changed || !reapable);
		SetButton(scrn->viewbuttons, "save", changed || reapable);
		SetButton(scrn->viewbuttons, "insert",
			  scrn->assocmsg != NULL ? True : False);

		if (!changed) 
		    MsgSetCallOnChange(scrn->msg, EnableCallback,
				       (XtPointer) scrn);
		else 
		    MsgSetCallOnChange(scrn->msg, (XtCallbackProc) NULL,
				       (XtPointer) NULL);

	    } else {
		BBoxDisable( BBoxFindButtonNamed(scrn->viewbuttons, "send"));
		BBoxDisable( BBoxFindButtonNamed(scrn->viewbuttons, "save"));
		BBoxDisable( BBoxFindButtonNamed(scrn->viewbuttons, "insert"));
	    }
	    break;
	}
    }
}
