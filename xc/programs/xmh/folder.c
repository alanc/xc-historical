#if !defined(lint) && !defined(SABER)
static char rcs_id[] =
    "$XConsortium: folder.c,v 2.14 89/07/20 21:15:17 converse Exp $";
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


/* Close this toc&view scrn.  If this is the last toc&view, quit xmh. */

/*ARGSUSED*/
void DoCloseScrn(widget, client_data, call_data)
    Widget	widget;
    caddr_t	client_data;
    caddr_t	call_data;
{
    Scrn	scrn = (Scrn) client_data;
    register int i, count;
    Toc		toc;
    Display	*dpy;
    extern void exit();

    count = 0;
    for (i=0 ; i<numScrns ; i++)
	if (scrnList[i]->kind == STtocAndView && scrnList[i]->mapped)
	    count++;
    if (count <= 1) {
	for (i = numScrns - 1; i >= 0; i--)
	    if (scrnList[i] != scrn) {
		if (MsgSetScrn((Msg) NULL, scrnList[i], DoCloseScrn, 
			       (caddr_t) scrn) == NEEDS_CONFIRMATION)
		    return;
	    }
	for (i = 0; i < numFolders; i++) {
	    toc = folderList[i];
	    if (TocConfirmCataclysm(toc, (XtCallbackProc) DoCloseScrn,
				    (caddr_t)scrn))
		return;
	}
/*	if (MsgSetScrn((Msg) NULL, scrn))  I think this can be deleted.
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
	if (MsgSetScrn((Msg) NULL, scrn, DoCloseScrn, (caddr_t) scrn)
	    == NEEDS_CONFIRMATION) return;
	DestroyScrn(scrn);	/* doesn't destroy first toc&view scrn */
    }
}

/*ARGSUSED*/
void CloseScrn(w, event, params, num_params)
    Widget	w;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    DoCloseScrn(w, (caddr_t) scrn, (caddr_t) NULL);
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
    MsgSetScrnForComp(msg, scrn);
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



/* Create a new folder with the given name. */

void CreateNewFolder(name)
    char	*name;
{
    Toc		toc;
    int		i, position;

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
	    BBoxAddButton(scrnList[i]->folderbuttons, name, position, TRUE);
}


/* Create a new folder.  Requires the user to name the new folder. */

/*ARGSUSED*/
void CreateFolder(w, event, params, num_params)
    Widget	w;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    PopupPrompt("Create folder named:", CreateNewFolder);
}



/*ARGSUSED*/
void CheckAndConfirmDeleteFolder(widget, client_data, call_data)
    Widget	widget;		/* unreliable; sometimes NULL */
    caddr_t	client_data;
    caddr_t	call_data;	/* unused */
{
    Toc		toc = (Toc) client_data;
    char	*foldername;
    char	str[300];
    void CheckAndDeleteFolder();
    static XtCallbackRec yes_callbacks[] = {
	{CheckAndDeleteFolder,	(caddr_t) NULL},
	{(XtCallbackProc) NULL,	(caddr_t) NULL}
    };
    static XtCallbackRec no_callbacks[] = {
	{TocClearDeletePending,	(caddr_t) NULL},
	{(XtCallbackProc) NULL,	(caddr_t) NULL}
    };

    /* check */
    if (TocConfirmCataclysm(toc, (XtCallbackProc) CheckAndConfirmDeleteFolder,
			    (caddr_t) toc) == NEEDS_CONFIRMATION)
	return;

    /* confirm */
    foldername = TocName(toc);
    yes_callbacks[0].closure = (caddr_t) toc;
    no_callbacks[0].closure =  (caddr_t) toc;
    (void) sprintf(str, "Are you sure you want to destroy %s?", foldername);
    PopupConfirm(str, yes_callbacks, no_callbacks);
}



/*ARGSUSED*/
void CheckAndDeleteFolder(widget, client_data, call_data)
    Widget	widget;
    caddr_t	client_data;
    caddr_t	call_data;	/* unused */
{
    int 	i;
    Toc		toc = (Toc) client_data;
    char	*foldername;
    
    /* check */
    if (TocConfirmCataclysm(toc, (XtCallbackProc) CheckAndConfirmDeleteFolder,
			    (caddr_t) toc) == NEEDS_CONFIRMATION)
	return;

    /* delete */
    foldername = TocName(toc);
    TocSetScrn(toc, (Scrn) NULL);
    TocDeleteFolder(toc);
    for (i=0 ; i<numScrns ; i++)
	if (scrnList[i]->folderbuttons) {
	    char	*p;

	    if (p = index(foldername, '/')) {
		char	parentfolder[300];
		(void) strcpy(parentfolder, foldername);
		p = index(parentfolder, '/');
		*p = '\0';

		BBoxDeleteMenuEntry
		    (BBoxFindButtonNamed(scrnList[i]->folderbuttons,
					 parentfolder), foldername);
	    }
	    else
		BBoxDeleteButton(BBoxFindButtonNamed
				 (scrnList[i]->folderbuttons, foldername));
	}
    XtFree(foldername);
}


/* Delete the selected folder.  Requires confirmation! */

/*ARGSUSED*/
void DeleteFolder(w, event, params, num_params)
    Widget	w;
    XEvent	*event;
    String	*params;
    Cardinal	*num_params;
{
    Scrn	scrn;
    Toc		toc;

    scrn = ScrnFromWidget(w);
    toc = SelectedToc(scrn);

    /* Prevent more than one confirmation popup on the same folder. 
     * TestAndSet returns true if there is a delete pending on this folder.
     */
    if (TocTestAndSetDeletePending(toc))	{
	Feep();
	return;
    }

    CheckAndConfirmDeleteFolder(w, (caddr_t) toc, (caddr_t) NULL);
}




