#if !defined(lint) && !defined(SABER)
static char rcs_id[] =
    "$XConsortium: folder.c,v 2.16 89/08/31 19:10:19 converse Exp $";
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

#include <X11/Xos.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <ctype.h>
#include <X11/Xaw/Cardinals.h>
#include "xmh.h"
#include "bboxint.h"

typedef struct {	/* client data structure for callbacks */
    Scrn	scrn;		/* the xmh scrn of action */
    Toc		toc;		/* the toc of the selected folder */
    Toc		original_toc;	/* the toc of the current folder */
} DeleteDataRec, *DeleteData;


/* Close this toc&view scrn.  If this is the last toc&view, quit xmh. */

/*ARGSUSED*/
void DoClose(widget, client_data, call_data)
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

    confirm_callbacks[0].callback = (XtCallbackProc) DoClose;
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
void XmhClose(w, event, params, num_params)
    Widget	w;
    XEvent	*event;		/* unused */
    String	*params;	/* unused */
    Cardinal	*num_params;	/* unused */
{
    Scrn scrn = ScrnFromWidget(w);
    DoClose(w, (caddr_t) scrn, (caddr_t) NULL);
}

/* Open the selected folder in this screen. */

/*ARGSUSED*/
void XmhOpenFolder(w, event, params, num_params)
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
void XmhComposeMessage(w, event, params, num_params)
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
void XmhOpenFolderInNewWindow(w, event, params, num_params)
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
	if (scrnList[i]->folderbuttons) {
	    char	*c;
	    Button	button;
	    if (c = index(name, '/')) {
		c[0] = '\0';
		button = BBoxFindButtonNamed(scrnList[i]->folderbuttons,
					     name);
		c[0] = '/';
		if (button) MenuAddEntry(button, name);
	    }
	    else
		BBoxAddButton(scrnList[i]->folderbuttons, name,
			      menuButtonWidgetClass, position, True);
	}
}


/* Create a new folder.  Requires the user to name the new folder. */

/*ARGSUSED*/
void XmhCreateFolder(w, event, params, num_params)
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

		DeleteMenuEntry(BBoxFindButtonNamed(scrnList[i]->folderbuttons,
						    parentfolder), foldername);
	    }
	    else {
		Boolean	reset;

		/* Is this the current folder of this screen? */
		reset = ! strcmp(scrnList[i]->curfolder, foldername);

		BBoxDeleteButton(BBoxFindButtonNamed
				 (scrnList[i]->folderbuttons, foldername));

		/* If so, reset the current folder of the screen. */
		if (reset && BBoxNumButtons(scrnList[i]->folderbuttons))
		    SetCurrentFolderName(scrnList[i], BBoxNameOfButton
		     (BBoxButtonNumber(scrnList[i]->folderbuttons, 0)));
	   }
	}
    XtFree(foldername);
    if (deleteData->original_toc != NULL) 
	TocSetScrn(deleteData->original_toc, scrn);
    XtFree((char *) deleteData);
}


/* Delete the selected folder.  Requires confirmation! */

/*ARGSUSED*/
void XmhDeleteFolder(w, event, params, num_params)
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


/*-----	Notes on MenuButtons as folder buttons ---------------------------
 *
 * I assume that the name of the button is identical to the name of the folder.
 * Only top-level folders have buttons.
 * Only top-level folders may have subfolders.
 * Top-level folders and their subfolders may have messages.
 *
 */

static char filename[500];	/* for IsFolder() and for callback */
static int  flen = 0;		/* length of a substring of filename */


typedef struct _MenuButtonDataRec {
    char 	*foldername;
    Button	button;
} MenuButtonDataRec, *MenuButtonData;


/* Function name:	IsFolder
 * Description:		determines if a file is an mh subfolder.
 */
static int IsFolder(ent)
    struct direct *ent;
{
    register int i, len;
    char *name = ent->d_name;
    struct stat buf;

    /* mh does not like subfolder names to be strings of digits */

    if (isdigit(name[0]) || name[0] == '#') {
	len = strlen(name);
	for(i=1; i < len && isdigit(name[i]); i++)
	    ;
	if (i == len) return FALSE;
    }
    else if (name[0] == '.')
	return FALSE;

    (void) sprintf(filename + flen, "/%s", name);
    if (stat(filename, &buf) /* failed */) return False;
    return (buf.st_mode & S_IFMT) == S_IFDIR;
}


/* menu entry selection callback for folder menus. */

/*ARGSUSED*/
static void SelectFolder(w, closure, data)
    Widget 	w;		/* unused */
    XtPointer	closure;
    XtPointer	data;		/* unused */
{

    MenuButtonData menu_button_data = (MenuButtonData) closure;
    Button	button = menu_button_data->button;
    Scrn	scrn = button->buttonbox->scrn;

    SetCurrentFolderName(scrn, menu_button_data->foldername);
}


/* Function name:	MenuAddEntry
 * Description:	
 *	Add an entry to a menu.  If the menu is not already created,
 *	create it, including the (already existing) new subfolder directory.
 * 	If the menu is already created,	add the new entry.
 */

void MenuAddEntry(button, entryname)
    Button	button;		/* the corresponding menu button */
    char	*entryname;	/* the new entry, relative to MailDir */
{
    int		n = 0;
    Arg		args[3];
    extern void MenuCreate();
    static XtCallbackRec callbacks[] = {
        {SelectFolder, NULL},
        {NULL, NULL},
    };

    if (button->menu == NULL || button->menu == NoMenuForButton)
	/* creating a subfolder before the menu has been created */
	MenuCreate(button);
    else {
	char *name;
	MenuButtonData menu_button_data = XtNew(MenuButtonDataRec);

	name = menu_button_data->foldername = XtNewString(entryname);
	menu_button_data->button = button;
	callbacks[0].closure = (XtPointer) menu_button_data;
	XtSetArg(args[n], XtNcallback, (XtArgVal) callbacks);	n++;

	if (IsSubFolder(entryname)) {
	    char	*parent_folder = MakeParentFolderName(entryname);
	    char	*sub_folder = MakeSubFolderName(entryname);

	    if (strcmp(parent_folder, sub_folder) == 0) {
		/* subfolder name identical to parent folder name */
		char	temp[200];
		temp[0] = '_';
		(void) strcpy(temp + 1, sub_folder);
		name = XtNewString(temp);
		XtSetArg(args[n], XtNlabel, sub_folder);	n++;
	    }
	    else name = sub_folder;
	    XtFree(parent_folder);
	}
	AddMenuEntry(button, name, SelectFolder, (XtPointer) menu_button_data,
		     True);
/*
	XawSimpleMenuAddEntry(button->menu, name, args, n);
*/
    }
}


/* Function name:	MenuCreate
 * Description:	
 *	This is a menu button action routine.  Menus are created for folder
 *	buttons if the folder has at least one subfolder.  For the directory
 *	given by the concatentation of app_resources.mailDir, '/', and the
 *	name of the button, MenuCreate creates the menu whose entries are
 *	the subdirectories which do not begin with '.' and do not have
 *	names which are all digits, and do not have names which are a '#'
 *	followed by all digits.  The first entry is always the name of the
 *	parent folder.  Remaining entries are alphabetized.
 */

void MenuCreate(button)
    Button	button;
{
    Arg		args[3];
    struct direct **namelist;
    register int i, n, length;
    extern	alphasort();
    char	directory[500];

    n = strlen(app_resources.mailDir);
    (void) strncpy(directory, app_resources.mailDir, n);
    directory[n++] = '/';
    (void) strcpy(directory + n, button->name);
    flen = strlen(directory);
    (void) strcpy(filename, directory);
    n = scandir(directory, &namelist, IsFolder, alphasort);
    if (n <= 0) {
	/* no subfolders, therefore no menu */
	button->menu = NoMenuForButton;
	return;
    }

    /* Create the menu widget, allowing the menu to show entry changes. */

    CreateMenu(button, True);
	
    /* The first entry is always the parent folder */

    MenuAddEntry(button, button->name);

    /* Build the menu by adding all the current entries to the new menu. */

    length = strlen(button->name);
    (void) strncpy(directory, button->name, length);
    directory[length++] = '/';
    for (i=0; i < n; i++) {
	(void) strcpy(directory + length, namelist[i]->d_name);
	XtFree((char *) namelist[i]);
	MenuAddEntry(button, directory);
    }
    XtFree((char *) namelist);

    DEBUG1("Built menu for %s.\n", button->name);
}

/*---------------------- %%% hack to get action procedures --------------*/

static XtActionProc Reset = NULL, PopupMenu = NULL;

static void external_action_init_hack()
{
    extern XtActionList xaw_command_actions_list, xaw_mbutton_actions_list;
    XtActionList command_actions = xaw_command_actions_list;
    XtActionList mbutton_actions = xaw_mbutton_actions_list;
    register int i;

    for (i=0; i < 6; i++) {
	if (strcmp(command_actions[i].string, "reset") == 0) {
	    Reset = command_actions[i].proc;
	    break;
	}
    }

    if (strcmp(mbutton_actions[0].string, "PopupMenu") == 0) 
	    PopupMenu = mbutton_actions[0].proc;
}

/*------------------- end of hack to get action procedures -------------*/


static Widget LastMenuButtonPressed = NULL;	/* to `toggle' menu buttons */


/* Function Name:	PopupFolderMenu
 * Description:		This action should alwas be taken when the user
 *	selects a folder button.  A folder button represents a folder 
 *	and zero or more subfolders.  The menu of subfolders is built upon
 *	the first reference to it, by this routine.  If there are no 
 *	subfolders, this routine will mark the folder as having no 
 *	subfolders, and no menu will be built.  In that case, the menu
 *	button emulates a command button.  Wwhen subfolders exist,
 *	the menu will popup, using the menu button action PopupMenu.
 */

/*ARGSUSED*/
void XmhPopupFolderMenu(w, event, vector, count)
    Widget	w;
    XEvent	*event;		/* unused */
    String	*vector;	/* unused */
    Cardinal	*count;		/* unused */
{
    Button	button;
    Scrn	scrn;

    if (! XtIsSubclass(w, menuButtonWidgetClass)) return;
    scrn = ScrnFromWidget(w);
    if ((button = BBoxFindButton(scrn->folderbuttons, w)) == NULL) return;
    if (button->menu == NULL) MenuCreate(button);

    if (button->menu == NoMenuForButton)
	LastMenuButtonPressed = w;
    else {
	if (PopupMenu == NULL || Reset == NULL)
	    external_action_init_hack();	/* %%% hack employed */
	if (PopupMenu != NULL) PopupMenu(button->widget, NULL, NULL, 0);
	if (Reset != NULL) Reset(button->widget, NULL, NULL, 0);
    }
}



/* Function Name:	XmhSetCurrentFolder
 * Description:		This action procedure allows menu buttons to 
 *	emulate toggle buttons as folder selection buttons.  Because of
 *	this, mh folders with no subfolders will not be represented by
 * 	a menu with one entry.  Sets the current folder without a menu
 *	callback.
 */

/*ARGSUSED*/
void XmhSetCurrentFolder(w, event, vector, count)
    Widget	w;
    XEvent	*event;		/* unused */
    String	*vector;	/* unused */
    Cardinal	*count;		/* unused */
{
    Button	button;
    Scrn	scrn;

    /* The MenuButton widget has a button grab currently active; the
     * menu entry selection callback routine will be invoked if the
     * user selects a menu entry.
     */
    if (w != LastMenuButtonPressed ||
	(! XtIsSubclass(w, menuButtonWidgetClass)))
	return;
    scrn = ScrnFromWidget(w);
    if ((button = BBoxFindButton(scrn->folderbuttons, w)) == NULL)
	return;
    SetCurrentFolderName(scrn, button->name);
}


/*ARGSUSED*/
void XmhLeaveFolderButton(w, event, vector, count)
    Widget	w;
    XEvent	*event;
    String	*vector;
    Cardinal	*count;
{
    LastMenuButtonPressed = NULL;
}


/*ARGSUSED*/
void XmhOpenFolderFromMenu(w, event, vector, count)
    Widget	w;
    XEvent	*event;
    String	*vector;
    Cardinal	*count;
{
    Position	x, y;
    Dimension	width, height;
    Arg		args[2];

    /* Open the folder upon a button event within the widget's window. */

    if (event->type != ButtonRelease && event->type != ButtonPress)
	return;
    x = event->xbutton.x;
    y = event->xbutton.y;
    XtSetArg(args[0], XtNwidth, &width);
    XtSetArg(args[1], XtNheight, &height);
    XtGetValues(w, args, TWO);
    if ((x < 0) || (x  >= width) || (y < 0) || (y >= height))
	return;

    XmhOpenFolder(w, event, vector, count);
}


/* Function:	DeleteMenuEntry
 * Description:	Remove a subfolder from a menu.
 */

void DeleteMenuEntry(button, foldername)
    Button	button;
    char	*foldername;	/* guaranteed to be a subfolder */
{

    if (XawSimpleMenuEntryCount(button->menu, XawMenuTextMask) <= 2) {
	XtDestroyWidget(button->menu);	
	button->menu = NoMenuForButton;
    }
    else {
	char *subfolder = MakeSubFolderName(foldername);
	if (strcmp(button->name, subfolder) == 0) {
	    char name[200];
	    name[0] = '_';
	    strcpy(name + 1, subfolder);
	    XawSimpleMenuRemoveEntry(button->menu, name);
	}
	else
	    XawSimpleMenuRemoveEntry(button->menu, subfolder);
	XtFree(subfolder);
    }
    SetCurrentFolderName(button->buttonbox->scrn, button->name);
}





