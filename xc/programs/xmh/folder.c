#if !defined(lint) && !defined(SABER)
static char rcs_id[] =
    "$XConsortium: folder.c,v 2.11 89/07/07 18:04:21 converse Exp $";
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

/* folder.c -- implement buttons relating to folders and other globals. */

#include "xmh.h"


char *GetCurrentFolderName(scrn)
    Scrn	scrn;
{
    return scrn->curfolder;
}


void SetCurrentFolderName(scrn, foldername)
    Scrn	scrn;
    char	*foldername;
{
    scrn->curfolder = foldername;
    ChangeLabel((Widget) scrn->folderlabel, foldername);
}


char	*IsSubFolder(foldername)
    char	*foldername;
{
    return index(foldername, '/');
}


char	*GetParentFolderName(foldername)
    char	*foldername;
{
    char	temp[500];
    char	*c, *p;
    (void) strcpy(temp, foldername);
    c = index(temp, '/');
    *c = '\0';
    p = XtMalloc(strlen(temp)+1);
    (void) strcpy(p, temp);
    return p;
}


char	*GetSubFolderName(foldername)
    char	*foldername;
{
    char	temp[500];
    char	*c, *p;
    (void) strcpy(temp, foldername);
    c = index(temp, '/');
    c++;
    p = XtMalloc(strlen(c) + 1);
    (void) strcpy(p, c);
    return p;
}


/* Close this toc&view scrn.  If this is the last toc&view, quit xmh. */

/*ARGSUSED*/
void CloseScrn(w, event, params, num_params)
    Widget	w;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    extern void exit();
    Toc toc;
    register int i, count;
    Display *dpy;
    count = 0;
    for (i=0 ; i<numScrns ; i++)
	if (scrnList[i]->kind == STtocAndView && scrnList[i]->mapped)
	    count++;
    if (count <= 1) {
	for (i = numScrns - 1; i >= 0; i--)
	    if (scrnList[i] != scrn) {
		if (MsgSetScrn((Msg) NULL, scrnList[i]))
		    return;
	    }
	for (i = 0; i < numFolders; i++) {
	    toc = folderList[i];
	    if (TocConfirmCataclysm(toc))
		return;
	}
/*	if (MsgSetScrn((Msg) NULL, scrn))
	    return;
*/
/*	for (i = 0; i < numFolders; i++) {
	    toc = folderList[i];
	    if (toc->scanfile && toc->curmsg)
		CmdSetSequence(toc, "cur", MakeSingleMsgList(toc->curmsg));
	}
*/
	dpy = XtDisplay(scrn->parent);
	XtUnmapWidget(scrn->parent);
	XCloseDisplay(dpy);
	exit(0);
    }
    else {
	if (MsgSetScrn((Msg) NULL, scrn)) return;
	DestroyScrn(scrn);
    }
}


/* Open the selected folder in this screen. */

/*ARGSUSED*/
void OpenFolder(w, event, params, num_params)
    Widget	w;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    Toc toc = SelectedToc(scrn);
    TocSetScrn(toc, scrn);
}


/* Compose a new message. */

/*ARGSUSED*/
void ComposeMessage(w, event, params, num_params)
    Widget	w;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    Msg msg;
    Scrn scrn = NewCompScrn();
    msg = TocMakeNewMsg(DraftsFolder);
    MsgLoadComposition(msg);
    MsgSetTemporary(msg);
    MsgSetReapable(msg);
    (void) MsgSetScrnForComp(msg, scrn);
    MapScrn(scrn);
}


/* Make a new scrn displaying the given folder. */

/*ARGSUSED*/
void OpenFolderInNewWindow(w, event, params, num_params)
    Widget	w;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    Toc toc = SelectedToc(scrn);
    scrn = CreateNewScrn(STtocAndView);
    TocSetScrn(toc, scrn);
    MapScrn(scrn);
}



/* Create a new xmh folder. */

/*ARGSUSED*/
void CreateFolder(w, event, params, num_params)
    Widget	w;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    void CreateNewFolder();
    MakePrompt(scrn, "Create folder named:", CreateNewFolder);
}


/* Delete the selected folder.  Requires confirmation! */

/*ARGSUSED*/
void DeleteFolder(w, event, params, num_params)
    Widget	w;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    char *foldername, str[100];
    int i;
    Toc toc;
    toc = SelectedToc(scrn);
    if (TocConfirmCataclysm(toc)) return;
    foldername = GetCurrentFolderName(scrn);
    (void) sprintf(str, "Are you sure you want to destroy %s?", foldername);
    if (!Confirm(scrn, str)) return;
    TocSetScrn(toc, (Scrn) NULL);
    TocDeleteFolder(toc);
    for (i=0 ; i<numScrns ; i++)
	if (scrnList[i]->folderbuttons) {
	    if (IsSubFolder(foldername)) {
		char *parentfolder = GetParentFolderName(foldername);
		BBoxDeleteMenuEntry
		    (BBoxFindButtonNamed(scrnList[i]->folderbuttons,
					 parentfolder), foldername);
		XtFree(parentfolder);
	    }
	    else
		BBoxDeleteButton(BBoxFindButtonNamed
				 (scrnList[i]->folderbuttons, foldername));
	}
}


/* Create a new folder with the given name. */

void CreateNewFolder(name)
  char *name;
{
    Toc toc;
    int i, position;

    for (i=0 ; name[i] > ' ' ; i++) ;
    name[i] = 0;
    toc = TocGetNamed(name);
    if (toc || i == 0) {
	Feep();
	return;
    }
    toc = TocCreateFolder(name);
    if (toc == NULL) {
	Feep();
	return;
    }
    for (position = numFolders - 1; position >= 0; position--)
	if (folderList[position] == toc)
	    break;
    for (i = 0; i < numScrns; i++)
	if (scrnList[i]->folderbuttons)
	    BBoxAddButton(scrnList[i]->folderbuttons, name,
			  NoOp, position, TRUE);
}
