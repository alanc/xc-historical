#if !defined(lint) && !defined(SABER)
static char rcs_id[] =
    "$XConsortium: screen.c,v 2.44 89/07/27 18:50:33 converse Exp $";
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

static FillViewButtons(scrn)
Scrn scrn;
{
    ButtonBox buttonbox = scrn->viewbuttons;
    if (scrn->tocwidget == NULL)
	BBoxAddButton(buttonbox, "close", 999, TRUE);
    BBoxAddButton(buttonbox, "reply", 999, TRUE);
    BBoxAddButton(buttonbox, "forward", 999, TRUE);
    BBoxAddButton(buttonbox, "useAsComp", 999, TRUE);
    BBoxAddButton(buttonbox, "edit", 999, TRUE);
    BBoxAddButton(buttonbox, "save", 999, FALSE);
    BBoxAddButton(buttonbox, "print", 999, TRUE);
}
    


static FillCompButtons(scrn)
Scrn scrn;
{
    ButtonBox buttonbox = scrn->viewbuttons;
    if (scrn->tocwidget == NULL)
	BBoxAddButton(buttonbox, "close", 999, TRUE);
    BBoxAddButton(buttonbox, "send", 999, TRUE);
    BBoxAddButton(buttonbox, "reset", 999, TRUE);
    BBoxAddButton(buttonbox, "compose", 999, TRUE);
    BBoxAddButton(buttonbox, "save", 999, TRUE);
    BBoxAddButton(buttonbox, "insert", 999, TRUE);
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
	    SetButton(scrn->tocbuttons, "inc", TocCanIncorporate(scrn->toc));
	    value = TocHasSequences(scrn->toc);
	    SetButton(scrn->tocbuttons, "openSeq", value);
	    SetButton(scrn->tocbuttons, "addToSeq", value);
	    SetButton(scrn->tocbuttons, "removeFromSeq", value);
	    SetButton(scrn->tocbuttons, "deleteSeq", value);
	    /* Fall through */

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
			  scrn->assocmsg != NULL ? TRUE : FALSE);

		if (!changed) 
		    MsgSetCallOnChange(scrn->msg, EnableCallback,
				       (caddr_t) scrn);
		else 
		    MsgSetCallOnChange(scrn->msg, (XtCallbackProc) NULL,
				       (caddr_t) NULL);

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
caddr_t data, junk;
{
  EnableProperButtons( (Scrn) data);
}  


/* Create subwidgets for a toc&view window. */

static MakeTocAndView(scrn)
Scrn scrn;
{
    int i, theight, min, max;
    ButtonBox buttonbox;
    static XawTextSelectType sarray[] = {XawselectLine,
					XawselectPosition,
					XawselectWord,
					XawselectAll,
					XawselectNull};
/*    static Arg arglist2[] = {
 *	{XtNselectionArray, (XtArgVal) sarray},
 *	{XtNselectionArrayCount, (XtArgVal) XtNumber(sarray)}
 */

    scrn->folderlabel = CreateTitleBar(scrn, "folderTitlebar");
    scrn->folderbuttons = BBoxMenuCreate(scrn, "folders");
    scrn->mainbuttons = BBoxCreate(scrn, "folderButtons");
    scrn->toclabel = CreateTitleBar(scrn, "tocTitlebar");
    scrn->tocwidget = CreateTextSW(scrn, "toc");
/* %%%				   arglist2, XtNumber(arglist2)); */

    XawTextSetSelectionArray(scrn->tocwidget, sarray);

    scrn->seqbuttons = BBoxRadioCreate(scrn, "seqButtons");
    scrn->tocbuttons = BBoxCreate(scrn, "tocButtons");
    scrn->viewlabel = CreateTitleBar(scrn, "viewTitlebar");
    scrn->viewwidget = CreateTextSW(scrn, "view");
    scrn->viewbuttons = BBoxCreate(scrn, "viewButtons");

    buttonbox = scrn->folderbuttons;
    for (i=0 ; i<numFolders ; i++)
      BBoxAddButton(buttonbox, TocName(folderList[i]), 999, TRUE);

    buttonbox = scrn->mainbuttons;
    BBoxAddButton(buttonbox, "close", 999, TRUE);
    BBoxAddButton(buttonbox, "compose", 999, TRUE);
    BBoxAddButton(buttonbox, "open", 999, TRUE);
    BBoxAddButton(buttonbox, "openInNew", 999, TRUE);
    BBoxAddButton(buttonbox, "create", 999, TRUE);
    BBoxAddButton(buttonbox, "delete", 999, TRUE);

    buttonbox = scrn->seqbuttons;
    BBoxAddButton(buttonbox, "all", 999, TRUE);

    buttonbox = scrn->tocbuttons;
    BBoxAddButton(buttonbox, "inc", 999, TRUE);
    BBoxAddButton(buttonbox, "next", 999, TRUE);
    BBoxAddButton(buttonbox, "prev", 999, TRUE);
    BBoxAddButton(buttonbox, "delete", 999, TRUE);
    BBoxAddButton(buttonbox, "move", 999, TRUE);
    BBoxAddButton(buttonbox, "copy", 999, TRUE);
    BBoxAddButton(buttonbox, "unmark", 999, TRUE);
    BBoxAddButton(buttonbox, "viewNew", 999, TRUE);
    BBoxAddButton(buttonbox, "reply", 999, TRUE);
    BBoxAddButton(buttonbox, "forward", 999, TRUE);
    BBoxAddButton(buttonbox, "useAsComp", 999, TRUE);
    BBoxAddButton(buttonbox, "commit", 999, TRUE);
    BBoxAddButton(buttonbox, "print", 999, TRUE);
    BBoxAddButton(buttonbox, "pack", 999, TRUE);
    BBoxAddButton(buttonbox, "sort", 999, TRUE);
    BBoxAddButton(buttonbox, "rescan", 999, TRUE);
    BBoxAddButton(buttonbox, "pick", 999, TRUE);
    BBoxAddButton(buttonbox, "openSeq", 999, TRUE);
    BBoxAddButton(buttonbox, "addToSeq", 999, TRUE);
    BBoxAddButton(buttonbox, "removeFromSeq", 999, TRUE);
    BBoxAddButton(buttonbox, "deleteSeq", 999, TRUE);

    FillViewButtons(scrn);

    BBoxLockSize(scrn->folderbuttons);
    BBoxLockSize(scrn->mainbuttons);
    BBoxLockSize(scrn->seqbuttons);
    BBoxLockSize(scrn->tocbuttons);
    BBoxLockSize(scrn->viewbuttons);

    if (app_resources.mailWaitingFlag) {
	static Arg arglist[] = {XtNiconPixmap, NULL};
	arglist[0].value = (XtArgVal) NoMailPixmap;
	XtSetValues(scrn->parent, arglist, XtNumber(arglist));
    }

    XtRealizeWidget(scrn->parent);

    theight = GetHeight((Widget)scrn->tocwidget) +
	GetHeight((Widget)scrn->viewwidget);
    theight = app_resources.defTocPercentage * theight / 100;
    XawPanedGetMinMax((Widget) scrn->tocwidget, &min, &max);
    XawPanedSetMinMax((Widget) scrn->tocwidget, theight, theight);
    XawPanedSetMinMax((Widget) scrn->tocwidget, min, max);
}


MakeView(scrn)
Scrn scrn;
{
    scrn->viewlabel = CreateTitleBar(scrn, "viewTitlebar");
    scrn->viewwidget = CreateTextSW(scrn, "view");
    scrn->viewbuttons = BBoxCreate(scrn, "viewButtons");
    FillViewButtons(scrn);
}


MakeComp(scrn)
Scrn scrn;
{
    scrn->viewlabel = CreateTitleBar(scrn, "composeTitlebar");
    scrn->viewwidget = CreateTextSW(scrn, "comp");
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
	{XtNgeometry, NULL},
	{XtNinput, (XtArgVal)True},
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
	if (kind != STtocAndView)
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
    XtPopdown(scrn->parent);
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
	scrn->mapped = TRUE;
    }
}


Scrn ScrnFromWidget(w)
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
