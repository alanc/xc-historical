#if !defined(lint) && !defined(SABER)
static char rcs_id[] =
    "$XConsortium: screen.c,v 2.46 89/09/05 19:34:56 converse Exp $";
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

/* scrn.c -- management of scrns. */

#include "xmh.h"

/* Fill in the buttons for the view commands. */

static void FillViewButtons(scrn)
Scrn scrn;
{
    ButtonBox buttonbox = scrn->viewbuttons;
    if (scrn->tocwidget == NULL)
	BBoxAddButton(buttonbox, "close", commandWidgetClass, 999, True);
    BBoxAddButton(buttonbox, "reply", commandWidgetClass, 999, True);
    BBoxAddButton(buttonbox, "forward", commandWidgetClass, 999, True);
    BBoxAddButton(buttonbox, "useAsComp", commandWidgetClass, 999, True);
    BBoxAddButton(buttonbox, "edit", commandWidgetClass, 999, True);
    BBoxAddButton(buttonbox, "save", commandWidgetClass, 999, False);
    BBoxAddButton(buttonbox, "print", commandWidgetClass, 999, True);
}
    


static void FillCompButtons(scrn)
Scrn scrn;
{
    ButtonBox buttonbox = scrn->viewbuttons;
    if (scrn->tocwidget == NULL)
	BBoxAddButton(buttonbox, "close", commandWidgetClass, 999, True);
    BBoxAddButton(buttonbox, "send", commandWidgetClass, 999, True);
    BBoxAddButton(buttonbox, "reset", commandWidgetClass, 999, True);
    BBoxAddButton(buttonbox, "compose", commandWidgetClass, 999, True);
    BBoxAddButton(buttonbox, "save", commandWidgetClass, 999, True);
    BBoxAddButton(buttonbox, "insert", commandWidgetClass, 999, True);
}


/* Figure out which buttons should and shouldn't be enabled in the given
   screen.  This should be called whenever something major happens to the
   screen. */

#define SetButton(buttonbox, name, value) \
    if (value) BBoxEnable(BBoxFindButtonNamed(buttonbox, name)); \
    else BBoxDisable(BBoxFindButtonNamed(buttonbox, name));

void EnableProperButtons(scrn)
Scrn scrn;
{
    static void EnableCallback();
    int value, changed, reapable;

    if (scrn) {
	switch (scrn->kind) {
	  case STtocAndView:
#if 0 /* %%% to be re-written */
	    SetButton(scrn->tocbuttons, "inc", TocCanIncorporate(scrn->toc));
	    value = TocHasSequences(scrn->toc);
	    SetButton(scrn->tocbuttons, "openSeq", value);
	    SetButton(scrn->tocbuttons, "addToSeq", value);
	    SetButton(scrn->tocbuttons, "removeFromSeq", value);
	    SetButton(scrn->tocbuttons, "deleteSeq", value);
	    value = (scrn->msg != NULL && !MsgGetEditable(scrn->msg));
	    SetButton(scrn->viewbuttons, "edit", value);
	    SetButton(scrn->viewbuttons, "save", scrn->msg != NULL && !value);
#endif
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
			  scrn->assocmsg != NULL ? True : FALSE);

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


/*ARGSUSED*/
static void EnableCallback(w, data, junk)
Widget w;
XtPointer data, junk;
{
  EnableProperButtons( (Scrn) data);
}  


/* Create subwidgets for a toc&view window. */

static void MakeTocAndView(scrn)
Scrn scrn;
{
    int		i, theight, min, max;
    Button	button;
    ButtonBox	buttonbox;
    static XawTextSelectType sarray[] = {XawselectLine,
					XawselectWord,
					XawselectAll,
					XawselectNull};
    static Arg args[] = {
	{ XtNselectTypes,	(XtArgVal) sarray},
	{ XtNdisplayCaret,	(XtArgVal) False}
    };

    scrn->folderlabel   = CreateTitleBar(scrn, "folderTitlebar");
    scrn->folderbuttons = BBoxCreate(scrn, "folders");
    scrn->mainbuttons   = BBoxCreate(scrn, "folderButtons");
    scrn->toclabel      = CreateTitleBar(scrn, "tocTitlebar");
    scrn->tocwidget     = CreateTextSW(scrn, "toc", args, XtNumber(args));
    scrn->seqbuttons    = RadioBBoxCreate(scrn, "seqButtons");
    scrn->tocbuttons    = BBoxCreate(scrn, "tocButtons");
    scrn->viewlabel     = CreateTitleBar(scrn, "viewTitlebar");
    scrn->viewwidget    = CreateTextSW(scrn, "view", args, (Cardinal) 0);

    buttonbox = scrn->folderbuttons;
    for (i=0 ; i<numFolders ; i++)
      BBoxAddButton(buttonbox, TocName(folderList[i]), menuButtonWidgetClass,
		    999, True);

    buttonbox = scrn->mainbuttons;
    BBoxAddButton(buttonbox, "close", commandWidgetClass, 999, True);
    BBoxAddButton(buttonbox, "compose", commandWidgetClass, 999, True);
    BBoxAddButton(buttonbox, "open", commandWidgetClass, 999, True);
    BBoxAddButton(buttonbox, "openInNew", commandWidgetClass, 999, True);
    BBoxAddButton(buttonbox, "create", commandWidgetClass, 999, True);
    BBoxAddButton(buttonbox, "delete", commandWidgetClass, 999, True);

    buttonbox = scrn->seqbuttons;
    RadioBBoxAddButton(buttonbox, "all", 999, True);

    buttonbox = scrn->tocbuttons;

    /* the menu of operations on the Table Of Contents */

    BBoxAddButton(buttonbox, "tocOps", menuButtonWidgetClass, 999, True);
    button = BBoxFindButtonNamed(buttonbox, "tocOps");
    CreateMenu(button, False);
    AddMenuEntry(button, "inc", Inc, (XtPointer) scrn, True);
    AddMenuEntry(button, "commit", DoCommit, (XtPointer) scrn, True);
    AddMenuEntry(button, "pack", DoPack, (XtPointer) scrn, True);
    AddMenuEntry(button, "sort", DoSort, (XtPointer) scrn, True);
    AddMenuEntry(button, "rescan", Rescan, (XtPointer) scrn, True);

    /* the menu of operations on the selected message(s) */

    BBoxAddButton(buttonbox, "msgOps", menuButtonWidgetClass, 999, True);
    button = BBoxFindButtonNamed(buttonbox, "msgOps");
    CreateMenu(button, False);
    AddMenuEntry(button, "next", DoNextView, (XtPointer) scrn, True);
    AddMenuEntry(button, "prev", DoPrevView, (XtPointer) scrn, True);
    AddMenuEntry(button, "delete", DoDelete, (XtPointer) scrn, True);
    AddMenuEntry(button, "move", DoMove, (XtPointer) scrn, True);
    AddMenuEntry(button, "copy", DoCopy, (XtPointer) scrn, True);
    AddMenuEntry(button, "unmark", DoUnmark, (XtPointer) scrn, True);
    AddMenuEntry(button, "viewNew", DoViewNew, (XtPointer) scrn, True);
    AddMenuEntry(button, "reply", Reply, (XtPointer) scrn, True);
    AddMenuEntry(button, "forward", DoForward, (XtPointer) scrn, True);
    AddMenuEntry(button, "useAsComp", DoTocUseAsComp, (XtPointer) scrn, True);
    AddMenuEntry(button, "print", DoPrint, (XtPointer) scrn, True);

    /* the menu of operations concerning message sequences */

    BBoxAddButton(buttonbox, "sequenceOps", menuButtonWidgetClass, 999, True);
    button = BBoxFindButtonNamed(buttonbox, "sequenceOps");
    CreateMenu(button, False);
    AddMenuEntry(button, "pick", DoPickMessages, (XtPointer) scrn, True);
    AddMenuEntry(button, "openSeq", DoOpenSeq, (XtPointer) scrn, True);
    AddMenuEntry(button, "addToSeq", DoAddToSeq, (XtPointer) scrn, True);
    AddMenuEntry(button, "removeFromSeq", DoRemoveFromSeq, (XtPointer) scrn,
		 True);
    AddMenuEntry(button, "deleteSeq", DoDeleteSeq, (XtPointer) scrn, True);

    /* the menu of operations upon the viewed message. */

    BBoxAddButton(buttonbox, "viewedMsgOps", menuButtonWidgetClass, 999, True);
    button = BBoxFindButtonNamed(buttonbox, "viewedMsgOps");
    CreateMenu(button, False);
    AddMenuEntry(button, "reply", DoViewReply, (XtPointer) scrn, True);
    AddMenuEntry(button, "forward", DoViewForward, (XtPointer) scrn, True);
    AddMenuEntry(button, "useAsComp", DoViewUseAsComposition, (XtPointer) scrn,
		 True);
    AddMenuEntry(button, "edit", DoEditView, (XtPointer) scrn, True);
    AddMenuEntry(button, "save", DoSaveView, (XtPointer) scrn, True);
    AddMenuEntry(button, "print", DoPrintView, (XtPointer) scrn, True);

    if (app_resources.mailWaitingFlag) {
	static Arg arglist[] = {XtNiconPixmap, NULL};
	arglist[0].value = (XtArgVal) NoMailPixmap;
	XtSetValues(scrn->parent, arglist, XtNumber(arglist));
    }

    XtRealizeWidget(scrn->parent);

    BBoxLockSize(scrn->folderbuttons);
    BBoxLockSize(scrn->mainbuttons);
    BBoxLockSize(scrn->seqbuttons);
    BBoxLockSize(scrn->tocbuttons);

    theight = GetHeight(scrn->tocwidget) + GetHeight(scrn->viewwidget);
    theight = app_resources.defTocPercentage * theight / 100;
    XawPanedGetMinMax((Widget) scrn->tocwidget, &min, &max);
    XawPanedSetMinMax((Widget) scrn->tocwidget, theight, theight);
    XawPanedSetMinMax((Widget) scrn->tocwidget, min, max);
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
	XtCreateManagedWidget(progName, vPanedWidgetClass, scrn->parent,
			      (ArgList)NULL, (Cardinal)0);

    switch (kind) {
	case STtocAndView:	MakeTocAndView(scrn);	break;
	case STview:		MakeView(scrn);	break;
	case STcomp:		MakeComp(scrn);	break;
    }

    if (kind != STpick) {
	DEBUG("Realizing...")
	XtRealizeWidget(scrn->parent);
	DEBUG(" done.\n")
	if (kind == STtocAndView) {
	    Arg	args[1];
	    XtSetArg(args[0], XtNtranslations, &(scrn->edit_translations));
	    XtGetValues(scrn->viewwidget, args, (Cardinal) 1);
	    XtInstallAllAccelerators(scrn->widget, scrn->widget);
	    XtInstallAllAccelerators(scrn->tocwidget, scrn->widget);
	    XtInstallAllAccelerators(scrn->viewwidget, scrn->widget);
	    XtSetArg(args[0], XtNtranslations, &(scrn->read_translations));
	    XtGetValues(scrn->viewwidget, args, (Cardinal) 1);
	}
	else
	    XtSetKeyboardFocus(scrn->parent, scrn->viewwidget);
	XDefineCursor(XtDisplay(scrn->parent), XtWindow(scrn->parent),
		      app_resources.cursor );
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
    while (w && XtClass(w) != vPanedWidgetClass)
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
