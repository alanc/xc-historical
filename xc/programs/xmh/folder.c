#if !defined(lint) && !defined(SABER)
static char rcs_id[] =
    "$XConsortium: folder.c,v 2.15 89/07/21 18:56:15 converse Exp $";
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

typedef struct {	/* client data structure for callbacks */
    Scrn	scrn;		/* the xmh scrn of action */
    Toc		toc;		/* the toc of the selected folder */
    Toc		original_toc;	/* the toc of the current folder */
} DeleteDataRec, *DeleteData;


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
    XtCallbackRec	confirm_callbacks[2];
    extern void exit();

    count = 0;
    for (i=0 ; i<numScrns ; i++)
	if (scrnList[i]->kind == STtocAndView && scrnList[i]->mapped)
	    count++;

    confirm_callbacks[0].callback = (XtCallbackProc) DoCloseScrn;
    confirm_callbacks[0].closure = (caddr_t) scrn;
    confirm_callbacks[1].callback = (XtCallbackProc) NULL;
    confirm_callbacks[1].closure = (caddr_t) NULL;

    if (count <= 1) {

	for (i = numScrns - 1; i >= 0; i--)
	    if (scrnList[i] != scrn) {
		if (MsgSetScrn((Msg) NULL, scrnList[i], confirm_callbacks,
			       (XtCallbackList) NULL) == NEEDS_CONFIRMATION)
		    return;
	    }
	for (i = 0; i < numFolders; i++) {
	    toc = folderList[i];

	    if (TocConfirmCataclysm(toc, confirm_callbacks,
				    (XtCallbackList) NULL))
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
	if (MsgSetScrn((Msg) NULL, scrn, confirm_callbacks, 
		       (XtCallbackList) NULL) == NEEDS_CONFIRMATION)
	    return;
	DestroyScrn(scrn);	/* doesn't destroy first toc&view scrn */
    }
}

/*ARGSUSED*/
void CloseScrn(w, event, params, num_params)
    Widget	w;
    XEvent	*event;		/* unused */
    String	*params;	/* unused */
    Cardinal	*num_params;	/* unused */
{
    Scrn scrn = ScrnFromWidget(w);
    DoCloseScrn(w, (caddr_t) scrn, (caddr_t) NULL);
}

/* Open the selected folder in this screen. */

/*ARGSUSED*/
void OpenFolder(w, event, params, num_params)
    Widget	w;
    XEvent	*event;		/* unused */
    String	*params;	/* unused */
    Cardinal	*num_params;	/* unused */
{
    Scrn scrn = ScrnFromWidget(w);
    Toc toc = SelectedToc(scrn);
    TocSetScrn(toc, scrn);
}


/* Compose a new message. */

/*ARGSUSED*/
void ComposeMessage(w, event, params, num_params)
    Widget	w;		/* unused */
    XEvent	*event;		/* unused */
    String	*params;	/* unused */
    Cardinal	*num_params;	/* unused */
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
    XEvent	*event;		/* unused */
    String	*params;	/* unused */
    Cardinal	*num_params;	/* unused */
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
    Widget	w;		/* unused */
    XEvent	*event;		/* unused */
    String	*params;	/* unused */
    Cardinal	*num_params;	/* unused */
{
    PopupPrompt("Create folder named:", CreateNewFolder);
}


/*ARGSUSED*/
void CancelDeleteFolder(widget, client_data, call_data)
    Widget	widget;		/* unused */
    caddr_t	client_data;
    caddr_t	call_data;	/* unused */
{
    DeleteData	deleteData = (DeleteData) client_data;

    TocClearDeletePending(deleteData->toc);

    /* When the delete request is made, the toc currently being viewed is
     * changed if necessary to be the toc under consideration for deletion.
     * Once deletion has been confirmed or cancelled, we revert to display
     * the toc originally under view, unless the toc originally under
     * view has been deleted.
     */

    if (deleteData->original_toc != NULL)
	TocSetScrn(deleteData->original_toc, deleteData->scrn);
    XtFree((char *) deleteData);
}


/*ARGSUSED*/
void CheckAndConfirmDeleteFolder(widget, client_data, call_data)
    Widget	widget;		/* unreliable; sometimes NULL */
    caddr_t	client_data;	/* data structure */
    caddr_t	call_data;	/* unused */
{
    DeleteData  deleteData = (DeleteData) client_data;
    Scrn	scrn = deleteData->scrn;
    Toc		toc  = deleteData->toc;
    char	*foldername;
    char	str[300];
    XtCallbackRec confirms[2];
    XtCallbackRec cancels[2];
    void CheckAndDeleteFolder();

    static XtCallbackRec yes_callbacks[] = {
	{CheckAndDeleteFolder,	(caddr_t) NULL},
	{(XtCallbackProc) NULL,	(caddr_t) NULL}
    };

    static XtCallbackRec no_callbacks[] = {
	{CancelDeleteFolder,	(caddr_t) NULL},
	{(XtCallbackProc) NULL,	(caddr_t) NULL}
    };

    /* Display the toc of the folder to be deleted. */

    TocSetScrn(toc, scrn);

    /* Check for pending delete, copy, move, or edits on messages in the
     * folder to be deleted, and ask for confirmation if they are found.
     */

    confirms[0].callback = (XtCallbackProc) CheckAndConfirmDeleteFolder;
    confirms[0].closure = client_data;
    confirms[1].callback = (XtCallbackProc) NULL;
    confirms[1].closure = (caddr_t) NULL;
    
    cancels[0].callback = (XtCallbackProc) CancelDeleteFolder;
    cancels[0].closure = client_data;
    cancels[1].callback = (XtCallbackProc) NULL;
    cancels[1].closure = (caddr_t) NULL;

    if (TocConfirmCataclysm(toc, confirms, cancels) ==	NEEDS_CONFIRMATION)
	return;

    /* Ask the user for confirmation on destroying the folder. */

    yes_callbacks[0].closure = client_data;
    no_callbacks[0].closure =  client_data;
    foldername = TocName(toc);
    (void) sprintf(str, "Are you sure you want to destroy %s?", foldername);
    PopupConfirm(str, yes_callbacks, no_callbacks);
}


/*ARGSUSED*/
void CheckAndDeleteFolder(widget, client_data, call_data)
    Widget	widget;		/* unused */
    caddr_t	client_data;	/* data structure */
    caddr_t	call_data;	/* unused */
{
    DeleteData  deleteData = (DeleteData) client_data;
    Scrn	scrn = deleteData->scrn;
    Toc		toc =  deleteData->toc;
    XtCallbackRec confirms[2];
    XtCallbackRec cancels[2];
    int 	i;
    char	*foldername;
    
    /* Check for changes occurring after the popup was first presented. */

    confirms[0].callback = (XtCallbackProc) CheckAndConfirmDeleteFolder;
    confirms[0].closure = client_data;
    confirms[1].callback = (XtCallbackProc) NULL;
    confirms[1].closure = (caddr_t) NULL;
    
    cancels[0].callback = (XtCallbackProc) CancelDeleteFolder;
    cancels[0].closure = client_data;
    cancels[1].callback = (XtCallbackProc) NULL;
    cancels[1].closure = (caddr_t) NULL;
    
    if (TocConfirmCataclysm(toc, confirms, cancels) == NEEDS_CONFIRMATION)
	return;

    /* Delete.  Restore the previously viewed toc, if it wasn't deleted. */

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
    if (deleteData->original_toc != NULL) 
	TocSetScrn(deleteData->original_toc, scrn);
    XtFree((char *) deleteData);
}


/* Delete the selected folder.  Requires confirmation! */

/*ARGSUSED*/
void DeleteFolder(w, event, params, num_params)
    Widget	w;
    XEvent	*event;		/* unused */
    String	*params;	/* unused */
    Cardinal	*num_params;	/* unused */
{
    Scrn	scrn = ScrnFromWidget(w);
    Toc		toc  = SelectedToc(scrn);
    DeleteData	deleteData;

    /* Prevent more than one confirmation popup on the same folder. 
     * TestAndSet returns true if there is a delete pending on this folder.
     */
    if (TocTestAndSetDeletePending(toc))	{
	Feep();
	return;
    }

    deleteData = XtNew(DeleteDataRec);
    deleteData->scrn = scrn;
    deleteData->toc = toc;
    deleteData->original_toc = CurrentToc(scrn);
    if (deleteData->original_toc == toc)
	deleteData->original_toc = (Toc) NULL;

    CheckAndConfirmDeleteFolder(w, (caddr_t) deleteData, (caddr_t) NULL);
}




