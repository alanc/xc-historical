/* $XConsortium: save.c,v 1.15 94/12/14 20:00:58 mor Exp mor $ */
/******************************************************************************

Copyright (c) 1993  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.
******************************************************************************/

#include "xsm.h"
#include "saveutil.h"

#include <X11/Shell.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/List.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Toggle.h>
#include <X11/Xaw/AsciiText.h>


Widget savePopup;
Widget   saveForm;
Widget	   saveMessageLabel;
Widget	   saveName;
Widget	   saveTypeLabel;
Widget	   saveTypeGlobal;
Widget	   saveTypeLocal;
Widget	   saveTypeBoth;
Widget	   interactStyleLabel;
Widget	   interactStyleNone;
Widget	   interactStyleErrors;
Widget	   interactStyleAny;
Widget	 saveOkButton;
Widget	 saveCancelButton;
Widget nameInUsePopup;
Widget   nameInUseForm;
Widget	   nameInUseLabel;
Widget     nameInUseOverwriteButton;
Widget	   nameInUseCancelButton;
Widget badSavePopup;
Widget   badSaveForm;
Widget	   badSaveLabel;
Widget     badSaveOkButton;
Widget	   badSaveCancelButton;
Widget     badSaveListWidget;

extern Widget clientInfoPopup;
extern Widget clientPropPopup;
extern Widget nameSessionPopup;

static int saveTypeData[] = {
	SmSaveLocal,
	SmSaveGlobal,
	SmSaveBoth
};

static int interactStyleData[] = {
	SmInteractStyleNone,
	SmInteractStyleErrors,
	SmInteractStyleAny
};

static String *failedNames = NULL;
static int numFailedNames = 0;

static String name_in_use = NULL;
static Bool name_locked = False;

void SetSaveSensitivity ();


static void
MakeCurrentSession (new_name, name_changed)

String new_name;
Bool name_changed;

{
    char title[256];
    List *cl;

    if (session_name)
    {
	/*
	 * In the old session, for any client that was not restarted by the
	 * session manager (previous ID was NULL), if we did not issue a
	 * checkpoint to this client after the initial startup, remove the
	 * client's checkpoint file using the discard command.
	 */

	for (cl = ListFirst (RunningList); cl; cl = ListNext (cl))
	{
	    ClientRec *client = (ClientRec *) cl->thing;

	    if (!client->restarted &&
		!client->userIssuedCheckpoint &&
		client->discardCommand)
	    {
		execute_system_command (client->discardCommand);
		XtFree (client->discardCommand);
		client->discardCommand = NULL;
	    }
	}
	
	/*
	 * Unlock the old session.
	 */
	
	if (!need_to_name_session)
	    UnlockSession (session_name);
    }

    if (name_changed)
    {
	if (session_name)
	    XtFree (session_name);

	session_name = XtNewString (new_name);
    }

    LockSession (session_name, True);

    sprintf (title, "xsm: %s", session_name);

    XtVaSetValues (topLevel,
	XtNtitle, title,
	NULL);

    set_session_save_file_name (session_name);


    /*
     * For each client, set the DiscardCommand ptr to NULL.
     * This is so when we do a checkpoint with the new session
     * name, we don't wipe out the checkpoint files needed by
     * the previous session.  We also set the userIssuedCheckpoint
     * flag to false for each client in the new session.
     */

    for (cl = ListFirst (RunningList); cl; cl = ListNext (cl))
    {
	ClientRec *client = (ClientRec *) cl->thing;

	client->userIssuedCheckpoint = False;

	if (client->discardCommand)
	{
	    XtFree (client->discardCommand);
	    client->discardCommand = NULL;
	}
    }

    need_to_name_session = False;
}




#define NAME_OK     0
#define NAME_EMPTY  1
#define NAME_EXISTS 2
#define NAME_LOCKED 3

static int
GetSaveName (nameRet)

String *nameRet;

{
    String new_name = NULL;
    Bool name_changed;

    /*
     * Get the name of the session for the save
     */

    XtVaGetValues (saveName,
	XtNstring, &new_name,
	NULL);

    *nameRet = new_name;

    if (!new_name || *new_name == '\0')
	return (NAME_EMPTY);

    /*
     * See if this is a new session.  If not return.
     */

    name_changed = !session_name ||
	(session_name && strcmp (session_name, new_name) != 0);

    if (!need_to_name_session && !name_changed)
	return (NAME_OK);


    /*
     * Make sure the session name is unique.
     */

    if (GetSessionNames (&sessionNameCount,
	&sessionNamesShort, NULL, &sessionsLocked))
    {
	int i, no_good = 0, locked;

	for (i = 0; i < sessionNameCount; i++)
	    if (strcmp (new_name, sessionNamesShort[i]) == 0)
	    {
		no_good = 1;
		locked = sessionsLocked[i];
		break;
	    }

	FreeSessionNames (sessionNameCount,
	    sessionNamesShort, NULL, sessionsLocked);
	
	if (no_good)
	    return (locked ? NAME_LOCKED : NAME_EXISTS);
    }

    MakeCurrentSession (new_name, name_changed);

    return (NAME_OK);
}


static void
DoSave ()

{
    ClientRec	*client;
    XtPointer	ptr;
    List	*cl;
    int		saveType;
    int		interactStyle;
    Bool	fast = False;
    char	*_saveType;
    char	*_shutdown;
    char	*_interactStyle;

    ptr = XawToggleGetCurrent (saveTypeLocal /* just 1 of the group */);
    saveType = *((int *) ptr);

    ptr = XawToggleGetCurrent (interactStyleNone /* just 1 of the group */);
    interactStyle = *((int *) ptr);

    if (saveType == SmSaveLocal)
	_saveType = "Local";
    else if (saveType == SmSaveGlobal)
	_saveType = "Global";
    else
	_saveType = "Both";

    if (wantShutdown)
	_shutdown = "True";
    else
	_shutdown = "False";

    if (interactStyle == SmInteractStyleNone)
	_interactStyle = "None";
    else if (interactStyle == SmInteractStyleErrors)
	_interactStyle = "Errors";
    else
	_interactStyle = "Any";

    SetSaveSensitivity (False);

    saveInProgress = True;
    
    shutdownCancelled = False;
    phase2InProgress = False;

    for (cl = ListFirst (RunningList); cl; cl = ListNext (cl))
    {
	client = (ClientRec *) cl->thing;

	SmsSaveYourself (client->smsConn,
	    saveType, wantShutdown, interactStyle, fast);

	ListAddLast (WaitForSaveDoneList, (char *) client);

	client->userIssuedCheckpoint = True;
	client->receivedDiscardCommand = False;

	if (verbose)
	{
	    printf ("Client Id = %s, sent SAVE YOURSELF [", client->clientId);
	    printf ("Save Type = %s, Shutdown = %s, ", _saveType, _shutdown);
	    printf ("Interact Style = %s, Fast = False]\n", _interactStyle);
	}
    }

    if (verbose)
    {
	printf ("\n");
	printf ("Sent SAVE YOURSELF to all clients.  Waiting for\n");
	printf ("SAVE YOURSELF DONE, INTERACT REQUEST, or\n");
	printf ("SAVE YOURSELF PHASE 2 REQUEST from each client.\n");
	printf ("\n");
    }
}



static void
SaveOkXtProc (w, client_data, callData)

Widget		w;
XtPointer 	client_data;
XtPointer 	callData;

{
    Position x, y, rootx, rooty;
    String name = NULL;
    char label[256];
    int	status;

    if ((status = GetSaveName (&name)) != NAME_OK)
    {
	XBell (XtDisplay (topLevel), 0);

	if (status == NAME_EXISTS || status == NAME_LOCKED)
	{
	    XtVaGetValues (mainWindow, XtNx, &x, XtNy, &y, NULL);
	    XtTranslateCoords (mainWindow, x, y, &rootx, &rooty);
	    XtVaSetValues (nameInUsePopup,
		XtNx, rootx + 25, XtNy, rooty + 100, NULL);

	    name_in_use = name;

	    if (status == NAME_LOCKED)
	    {
		name_locked = True;

		sprintf (label, "Another session by the name '%s' is active.\nChoose another name for the session.", name);

		XtUnmanageChild (nameInUseOverwriteButton);

		XtVaSetValues (nameInUseCancelButton,
		    XtNlabel, "OK",
		    XtNfromHoriz, NULL,
		    NULL);
	    }
	    else
	    {
		name_locked = False;

		sprintf (label, "Another session by the name '%s' already exists.\nWould you like to overwrite it?", name);

		XtManageChild (nameInUseOverwriteButton);

		XtVaSetValues (nameInUseCancelButton,
		    XtNlabel, "Cancel",
		    XtNfromHoriz, nameInUseOverwriteButton,
		    NULL);
	    }

	    XtVaSetValues (nameInUseLabel,
		XtNlabel, label,
		NULL);

	    XtPopdown (savePopup);
	    XtPopup (nameInUsePopup, XtGrabNone);
	}

	return;
    }

    DoSave ();
}



void
SaveOkAction (w, event, params, num_params)

Widget	 w;
XEvent	 *event;
String	 *params;
Cardinal *num_params;

{
    XtCallCallbacks (saveOkButton, XtNcallback, NULL);
}



void
LetClientInteract (cl)

List *cl;

{
    ClientRec *client = (ClientRec *) cl->thing;

    SmsInteract (client->smsConn);

    ListSearchAndFreeOne (WaitForInteractList, (char *) client);

    if (verbose)
    {
	printf ("Client Id = %s, sent INTERACT\n", client->clientId);
    }
}



void
StartPhase2 ()

{
    List *cl;

    if (verbose)
    {
	printf ("\n");
	printf ("Starting PHASE 2 of SAVE YOURSELF\n");
	printf ("\n");
    }

    for (cl = ListFirst (WaitForPhase2List); cl; cl = ListNext (cl))
    {
	ClientRec *client = (ClientRec *) cl->thing;

	SmsSaveYourselfPhase2 (client->smsConn);

	if (verbose)
	{
	    printf ("Client Id = %s, sent SAVE YOURSELF PHASE 2",
		client->clientId);
	}
    }

    ListFreeAllButHead (WaitForPhase2List);

    phase2InProgress = True;
}


void
FinishUpSave ()

{
    ClientRec	*client;
    List	*cl;

    if (verbose)
    {
	printf ("\n");
	printf ("All clients issued SAVE YOURSELF DONE\n");
	printf ("\n");
    }

    saveInProgress = False;
    phase2InProgress = False;

    /*
     * Now execute discard commands
     */

    for (cl = ListFirst (RunningList); cl; cl = ListNext (cl))
    {
	client = (ClientRec *) cl->thing;

	if (!client->receivedDiscardCommand)
	    continue;

	if (client->discardCommand)
	{
	    execute_system_command (client->discardCommand);
	    XtFree (client->discardCommand);
	    client->discardCommand = NULL;
	}
	    
	if (client->saveDiscardCommand)
	{
	    client->discardCommand = client->saveDiscardCommand;
	    client->saveDiscardCommand = NULL;
	}
    }


    /*
     * Write the save file
     */
    
    WriteSave (sm_id);


    if (wantShutdown && shutdownCancelled)
    {
	shutdownCancelled = False;
    }
    else if (wantShutdown)
    {
	shutdownInProgress = True;

	for (cl = ListFirst (RunningList); cl; cl = ListNext (cl))
	{
	    client = (ClientRec *) cl->thing;

	    SmsDie (client->smsConn);

	    if (verbose)
	    {
		printf ("Client Id = %s, sent DIE\n", client->clientId);
	    }
	}
    }
    else
    {
	for (cl = ListFirst (RunningList); cl; cl = ListNext (cl))
	{
	    client = (ClientRec *) cl->thing;

	    SmsSaveComplete (client->smsConn);

	    if (verbose)
	    {
		printf ("Client Id = %s, sent SAVE COMPLETE\n",
		    client->clientId);
	    }
	}
    }

    if (!shutdownInProgress)
    {
	XtPopdown (savePopup);
	SetAllSensitive (1);
    }
}



static void
SaveCancelXtProc (w, client_data, callData)

Widget		w;
XtPointer 	client_data;
XtPointer 	callData;

{
    XtPopdown (savePopup);
    SetAllSensitive (1);
}



/*
 * Add toggle button
 */

static Widget
AddToggle (widgetName, parent, state, radioGroup, radioData,
    fromHoriz, fromVert)

char 		*widgetName;
Widget 		parent;
int 		state;
Widget 		radioGroup;
XtPointer 	radioData;
Widget 		fromHoriz;
Widget 		fromVert;

{
    Widget toggle;

    toggle = XtVaCreateManagedWidget (
	widgetName, toggleWidgetClass, parent,
        XtNstate, state,
        XtNradioGroup, radioGroup,
        XtNradioData, radioData,
        XtNfromHoriz, fromHoriz,
        XtNfromVert, fromVert,
        NULL);

    return (toggle);
}



void
SetSaveSensitivity (on)

Bool on;

{
    XtSetSensitive (savePopup, on);

#if 0
    /*
     * When we turn of sensitivity in the save dialog, we want to keep
     * the cancel button sensitive (so the user can cancel in case of
     * a problem).  Unfortunately, we can not turn off the sensitivity on
     * the save popup, and then just turn on sensitivity for the cancel
     * button.  We must do each widget individually.
     */

    XtSetSensitive (saveTypeLabel, on);
    XtSetSensitive (saveTypeGlobal, on);
    XtSetSensitive (saveTypeLocal, on);
    XtSetSensitive (saveTypeBoth, on);
    XtSetSensitive (interactStyleLabel, on);
    XtSetSensitive (interactStyleNone, on);
    XtSetSensitive (interactStyleErrors, on);
    XtSetSensitive (interactStyleAny, on);
    XtSetSensitive (saveOkButton, on);
#endif
}



void
SavePopupStructureNotifyXtHandler (w, closure, event, continue_to_dispatch)

Widget w;
XtPointer closure;
XEvent *event;
Boolean *continue_to_dispatch;

{
    if (event->type == MapNotify)
    {
	/*
	 * Now that the Save Dialog is back up, we can do the save.
	 */

	if (name_locked)
	{
	    /* Force shutdown */
	}

	DeleteSession (name_in_use);

	MakeCurrentSession (name_in_use, True);

	name_in_use = NULL;

	DoSave ();

	XtRemoveEventHandler (savePopup, StructureNotifyMask, False,
	    SavePopupStructureNotifyXtHandler, NULL);
    }
}



static void
NameInUseOverwriteXtProc (w, client_data, callData)

Widget		w;
XtPointer 	client_data;
XtPointer 	callData;

{
    if (name_locked)
    {
	/* force shutdown not implemented yet */

	return;
    }

    XtPopdown (nameInUsePopup);

    /*
     * We want to popup the Save dialog again.  In order to avoid a race
     * condition with the BadSave handler trying to pop down the Save Dialog,
     * we wait for the MapNotify on the Save dialog, and then do the save.
     */

    XtAddEventHandler (savePopup, StructureNotifyMask, False,
	SavePopupStructureNotifyXtHandler, NULL);

    XtPopup (savePopup, XtGrabNone);
}



static void
NameInUseCancelXtProc (w, client_data, callData)

Widget		w;
XtPointer 	client_data;
XtPointer 	callData;

{
    XtPopdown (nameInUsePopup);
    XtPopup (savePopup, XtGrabNone);

    name_in_use = NULL;
}



static void
BadSaveOkXtProc (w, client_data, callData)

Widget		w;
XtPointer 	client_data;
XtPointer 	callData;

{
    ListFreeAllButHead (FailedSaveList);
    XtPopdown (badSavePopup);
    FinishUpSave ();
}



static void
BadSaveCancelXtProc (w, client_data, callData)

Widget		w;
XtPointer 	client_data;
XtPointer 	callData;

{
    ListFreeAllButHead (FailedSaveList);
    XtPopdown (badSavePopup);

    if (wantShutdown)
    {
	List *cl;

	shutdownCancelled = True;

	for (cl = ListFirst (RunningList); cl; cl = ListNext (cl))
	{
	    ClientRec *client = (ClientRec *) cl->thing;

	    SmsShutdownCancelled (client->smsConn);

	    if (verbose) 
	    {
		printf ("Client Id = %s, sent SHUTDOWN CANCELLED\n",
			client->clientId);
	    }
	}
    }

    FinishUpSave ();
}



static void
BadSaveListXtProc (w, client_data, callData)

Widget		w;
XtPointer 	client_data;
XtPointer 	callData;

{

}



void
create_save_popup ()

{
    XtTranslations translations;

    static XtActionsRec save_actions[] = {
        {"SaveOkAction", SaveOkAction}
    };


    /*
     * Pop up for Save Yourself button.
     */

    savePopup = XtVaCreatePopupShell (
	"savePopup", transientShellWidgetClass, topLevel,
	XtNallowShellResize, True,
	NULL);
    
    saveForm = XtCreateManagedWidget (
	"saveForm", formWidgetClass, savePopup, NULL, 0);

    saveMessageLabel = XtVaCreateManagedWidget (
	"saveMessageLabel", labelWidgetClass, saveForm,
        XtNfromHoriz, NULL,
        XtNfromVert, NULL,
        XtNborderWidth, 0,
	NULL);

    saveName = XtVaCreateManagedWidget (
	"saveName", asciiTextWidgetClass, saveForm,
        XtNfromVert, NULL,
	XtNeditType, XawtextEdit,
	XtNresizable, True,
	XtNresize, XawtextResizeWidth,
	NULL);

    saveTypeLabel = XtVaCreateManagedWidget (
	"saveTypeLabel", labelWidgetClass, saveForm,
        XtNfromHoriz, NULL,
        XtNfromVert, saveMessageLabel,
        XtNborderWidth, 0,
        XtNvertDistance, 20,
	NULL);

    saveTypeLocal = AddToggle (
	"saveTypeLocal", 			/* widgetName */
	saveForm,				/* parent */
	0,					/* state */
        NULL,					/* radioGroup */
        (XtPointer) &saveTypeData[0],		/* radioData */
        saveTypeLabel,				/* fromHoriz */
        saveMessageLabel			/* fromVert */
    );

    saveTypeGlobal = AddToggle (
	"saveTypeGlobal", 			/* widgetName */
	saveForm,				/* parent */
	0,					/* state */
        saveTypeLocal,				/* radioGroup */
        (XtPointer) &saveTypeData[1],		/* radioData */
        saveTypeLocal,				/* fromHoriz */
        saveMessageLabel			/* fromVert */
    );

    saveTypeBoth = AddToggle (
	"saveTypeBoth", 			/* widgetName */
	saveForm,				/* parent */
	1,					/* state */
        saveTypeLocal,				/* radioGroup */
        (XtPointer) &saveTypeData[2],		/* radioData */
        saveTypeGlobal,				/* fromHoriz */
        saveMessageLabel			/* fromVert */
    );


    XtVaSetValues (saveName, XtNfromHoriz, saveTypeLabel, NULL);
    XtVaSetValues (saveTypeLocal, XtNvertDistance, 20, NULL);
    XtVaSetValues (saveTypeGlobal, XtNvertDistance, 20, NULL);
    XtVaSetValues (saveTypeBoth, XtNvertDistance, 20, NULL);

    interactStyleLabel = XtVaCreateManagedWidget (
	"interactStyleLabel", labelWidgetClass, saveForm,
        XtNfromHoriz, NULL,
        XtNfromVert, saveTypeLabel,
        XtNborderWidth, 0,
	NULL);

    interactStyleNone = AddToggle (
	"interactStyleNone", 			/* widgetName */
	saveForm,				/* parent */
	0,					/* state */
        NULL,					/* radioGroup */
        (XtPointer) &interactStyleData[0],	/* radioData */
        saveTypeLabel,				/* fromHoriz */
        saveTypeLabel				/* fromVert */
    );

    interactStyleErrors = AddToggle (
	"interactStyleErrors", 			/* widgetName */
	saveForm,				/* parent */
	0,					/* state */
        interactStyleNone,			/* radioGroup */
        (XtPointer) &interactStyleData[1],	/* radioData */
        interactStyleNone,			/* fromHoriz */
        saveTypeLabel				/* fromVert */
    );

    interactStyleAny = AddToggle (
	"interactStyleAny", 			/* widgetName */
	saveForm,				/* parent */
	1,					/* state */
        interactStyleNone,			/* radioGroup */
        (XtPointer) &interactStyleData[2],	/* radioData */
        interactStyleErrors,			/* fromHoriz */
        saveTypeLabel				/* fromVert */
    );


    saveOkButton = XtVaCreateManagedWidget (
	"saveOkButton",	commandWidgetClass, saveForm,
        XtNfromHoriz, NULL,
        XtNfromVert, interactStyleLabel,
        XtNvertDistance, 20,
	XtNresizable, True,
        NULL);
    
    XtAddCallback (saveOkButton, XtNcallback, SaveOkXtProc, 0);

    saveCancelButton = XtVaCreateManagedWidget (
	"saveCancelButton", commandWidgetClass, saveForm,
        XtNfromHoriz, saveOkButton,
        XtNfromVert, interactStyleLabel,
        XtNvertDistance, 20,
        NULL);

    XtAddCallback (saveCancelButton, XtNcallback, SaveCancelXtProc, 0);

    XtSetKeyboardFocus (saveForm, saveName);

    XtAppAddActions (appContext, save_actions, XtNumber (save_actions));

    translations = XtParseTranslationTable
	("<Key>Return: SaveOkAction()\n");
    XtOverrideTranslations(saveName, translations);

    XtInstallAllAccelerators (saveForm, saveForm);


    /*
     * Pop up when user tries to save the session under an
     * already used name.
     */

    nameInUsePopup = XtVaCreatePopupShell (
	"nameInUsePopup", transientShellWidgetClass, topLevel,
	XtNallowShellResize, True,
	NULL);
    

    nameInUseForm = XtVaCreateManagedWidget (
	"nameInUseForm", formWidgetClass, nameInUsePopup,
	NULL);

    nameInUseLabel = XtVaCreateManagedWidget (
	"nameInUseLabel", labelWidgetClass, nameInUseForm,
	XtNresizable, True,
        XtNfromHoriz, NULL,
        XtNfromVert, NULL,
        XtNborderWidth, 0,
	XtNtop, XawChainTop,
	XtNbottom, XawChainTop,
	NULL);

    nameInUseOverwriteButton = XtVaCreateManagedWidget (
	"nameInUseOverwriteButton", commandWidgetClass, nameInUseForm,
        XtNfromHoriz, NULL,
        XtNfromVert, nameInUseLabel,
	XtNtop, XawChainBottom,
	XtNbottom, XawChainBottom,
        NULL);
    
    XtAddCallback (nameInUseOverwriteButton, XtNcallback,
	NameInUseOverwriteXtProc, 0);


    nameInUseCancelButton = XtVaCreateManagedWidget (
	"nameInUseCancelButton", commandWidgetClass, nameInUseForm,
	XtNresizable, True,
        XtNfromHoriz, nameInUseOverwriteButton,
        XtNfromVert, nameInUseLabel,
	XtNtop, XawChainBottom,
	XtNbottom, XawChainBottom,
        NULL);
    
    XtAddCallback (nameInUseCancelButton, XtNcallback,
	NameInUseCancelXtProc, 0);



    /*
     * Pop up when not all clients returned SaveSuccess
     */

    badSavePopup = XtVaCreatePopupShell (
	"badSavePopup", transientShellWidgetClass, topLevel,
	XtNallowShellResize, True,
	NULL);
    

    badSaveForm = XtVaCreateManagedWidget (
	"badSaveForm", formWidgetClass, badSavePopup,
	NULL);

    badSaveLabel = XtVaCreateManagedWidget (
	"badSaveLabel", labelWidgetClass, badSaveForm,
        XtNfromHoriz, NULL,
        XtNfromVert, NULL,
        XtNborderWidth, 0,
	XtNtop, XawChainTop,
	XtNbottom, XawChainTop,
	NULL);

    badSaveListWidget = XtVaCreateManagedWidget (
	"badSaveListWidget", listWidgetClass, badSaveForm,
	XtNresizable, True,
        XtNdefaultColumns, 1,
	XtNforceColumns, True,
        XtNfromHoriz, NULL,
        XtNfromVert, badSaveLabel,
	XtNtop, XawChainTop,
	XtNbottom, XawChainBottom,
	NULL);

    XtAddCallback (badSaveListWidget, XtNcallback, BadSaveListXtProc, 0);

    badSaveOkButton = XtVaCreateManagedWidget (
	"badSaveOkButton", commandWidgetClass, badSaveForm,
        XtNfromHoriz, NULL,
        XtNfromVert, badSaveListWidget,
	XtNtop, XawChainBottom,
	XtNbottom, XawChainBottom,
        NULL);
    
    XtAddCallback (badSaveOkButton, XtNcallback, BadSaveOkXtProc, 0);


    badSaveCancelButton = XtVaCreateManagedWidget (
	"badSaveCancelButton", commandWidgetClass, badSaveForm,
        XtNfromHoriz, badSaveOkButton,
        XtNfromVert, badSaveListWidget,
	XtNtop, XawChainBottom,
	XtNbottom, XawChainBottom,
        NULL);
    
    XtAddCallback (badSaveCancelButton, XtNcallback, BadSaveCancelXtProc, 0);

    XtInstallAllAccelerators (badSaveForm, badSaveForm);
}



void
PopupSaveDialog ()

{
    Position x, y, rootx, rooty;

    XtVaGetValues (mainWindow, XtNx, &x, XtNy, &y, NULL);
    XtTranslateCoords (mainWindow, x, y, &rootx, &rooty);
    XtVaSetValues (savePopup, XtNx, rootx + 25, XtNy, rooty + 100, NULL);

    XtSetSensitive (mainWindow, 0);
    XtSetSensitive (clientInfoPopup, 0);
    XtSetSensitive (clientPropPopup, 0);

    XawToggleSetCurrent (saveTypeBoth,
	(XtPointer) &saveTypeData[2]);
    XawToggleSetCurrent (interactStyleAny,
	(XtPointer) &interactStyleData[2]);

    XtVaSetValues (savePopup,
	XtNtitle, wantShutdown ? "Shutdown" : "Checkpoint",
	NULL);

    XtVaSetValues (saveName,
	XtNstring, need_to_name_session ? "" : session_name,
	NULL);

    XtVaSetValues (saveOkButton,
	XtNlabel, wantShutdown ? "Shutdown" : "Checkpoint",
	NULL);

    XtPopup (savePopup, XtGrabNone);
}




void
CheckPointXtProc (w, client_data, callData)

Widget		w;
XtPointer 	client_data;
XtPointer 	callData;

{
    wantShutdown = False;
    PopupSaveDialog ();
}




void
ShutdownSaveXtProc (w, client_data, callData)

Widget		w;
XtPointer 	client_data;
XtPointer 	callData;

{
    wantShutdown = True;
    PopupSaveDialog ();
}



void
PopupBadSave ()

{
    ClientRec *client;
    char *progName, *hostname, *tmp1, *tmp2;
    String clientInfo;
    int maxlen1, maxlen2;
    char extraBuf1[80], extraBuf2[80];
    char *restart_service_prop;
    Position x, y, rootx, rooty;
    List *cl, *pl;
    int i, k;

    if (failedNames)
    {
	/*
	 * Free the previous list of names.  Xaw doesn't make a copy of
	 * our list, so we need to keep it around.
	 */

	for (i = 0; i < numFailedNames; i++)
	    XtFree (failedNames[i]);

	XtFree ((char *) failedNames);

	failedNames = NULL;
    }

    maxlen1 = maxlen2 = 0;
    numFailedNames = 0;

    for (cl = ListFirst (FailedSaveList); cl; cl = ListNext (cl))
    {
	client = (ClientRec *) cl->thing;

	progName = NULL;
	restart_service_prop = NULL;

	for (pl = ListFirst (client->props); pl; pl = ListNext (pl))
	{
	    Prop *pprop = (Prop *) pl->thing;
	    List *vl = ListFirst (pprop->values);
	    PropValue *pval = (PropValue *) vl->thing;

	    if (strcmp (pprop->name, SmProgram) == 0)
	    {
		progName = (char *) GetProgramName ((char *) pval->value);

		if ((int) strlen (progName) > maxlen1)
		    maxlen1 = strlen (progName);
	    }
	    else if (strcmp (pprop->name, "_XC_RestartService") == 0)
	    {
		restart_service_prop = (char *) pval->value;
	    }
	}

	if (!progName)
	    continue;

	if (restart_service_prop)
	    tmp1 = restart_service_prop;
	else if (client->clientHostname)
	    tmp1 = client->clientHostname;
	else
	    continue;

	if ((tmp2 = (char *) strchr (tmp1, '/')) == NULL)
	    hostname = tmp1;
	else
	    hostname = tmp2 + 1;

	if ((int) strlen (hostname) > maxlen2)
	    maxlen2 = strlen (hostname);

	numFailedNames++;
    }

    failedNames = (String *) XtMalloc (
	numFailedNames * sizeof (String));

    i = 0;
    for (cl = ListFirst (FailedSaveList); cl; cl = ListNext (cl))
    {
	ClientRec *client = (ClientRec *) cl->thing;
	int extra1, extra2;

	progName = NULL;
	restart_service_prop = NULL;

	for (pl = ListFirst (client->props); pl; pl = ListNext (pl))
	{
	    Prop *pprop = (Prop *) pl->thing;
	    List *vl = ListFirst (pprop->values);
	    PropValue *pval = (PropValue *) vl->thing;

	    if (strcmp (pprop->name, SmProgram) == 0)
	    {
		progName = (char *) GetProgramName ((char *) pval->value);
	    }
	    else if (strcmp (pprop->name, "_XC_RestartService") == 0)
	    {
		restart_service_prop = (char *) pval->value;
	    }
	}

	if (!progName)
	    continue;

	if (restart_service_prop)
	    tmp1 = restart_service_prop;
	else if (client->clientHostname)
	    tmp1 = client->clientHostname;
	else
	    continue;

	if ((tmp2 = (char *) strchr (tmp1, '/')) == NULL)
	    hostname = tmp1;
	else
	    hostname = tmp2 + 1;

	extra1 = maxlen1 - strlen (progName) + 5;
	extra2 = maxlen2 - strlen (hostname);

	clientInfo = (String) XtMalloc (strlen (progName) +
	    extra1 + extra2 + 3 + strlen (hostname) + 1);

	for (k = 0; k < extra1; k++)
	    extraBuf1[k] = ' ';
	extraBuf1[extra1] = '\0';

	for (k = 0; k < extra2; k++)
	    extraBuf2[k] = ' ';
	extraBuf2[extra2] = '\0';

	sprintf (clientInfo, "%s%s (%s%s)", progName, extraBuf1,
	    hostname, extraBuf2);

	failedNames[i++] = clientInfo;

	if (client->freeAfterBadSavePopup)
	{
	    FreeClient (client, True /* free props */);
	}
    }

    XawListChange (badSaveListWidget,
	failedNames, numFailedNames, 0, True);

    XtPopdown (savePopup);

    if (wantShutdown && !shutdownCancelled)
	XtManageChild (badSaveCancelButton);
    else
	XtUnmanageChild (badSaveCancelButton);

    XtVaGetValues (mainWindow, XtNx, &x, XtNy, &y, NULL);
    XtTranslateCoords (mainWindow, x, y, &rootx, &rooty);
    XtVaSetValues (badSavePopup, XtNx, rootx + 25, XtNy, rooty + 100, NULL);

    XtPopup (badSavePopup, XtGrabNone);
}



void
ShutdownDontSaveXtProc (w, client_data, callData)

Widget		w;
XtPointer 	client_data;
XtPointer 	callData;

{
    /*
     * For any client that was not restarted by the session
     * manager (previous ID was NULL), if we did not issue a
     * checkpoint to this client, remove the client's checkpoint
     * file using the discard command.
     */

    List	*cl;
    ClientRec 	*client;

    for (cl = ListFirst (RunningList); cl; cl = ListNext (cl))
    {
	client = (ClientRec *) cl->thing;

	if (!client->restarted &&
	    !client->userIssuedCheckpoint &&
	    client->discardCommand)
	{
	    execute_system_command (client->discardCommand);
	    XtFree (client->discardCommand);
	    client->discardCommand = NULL;
	}
    }

    shutdownInProgress = True;

    for (cl = ListFirst (RunningList); cl; cl = ListNext (cl))
    {
	client = (ClientRec *) cl->thing;

	SmsDie (client->smsConn);

	if (verbose)
	{
	    printf ("Client Id = %s, sent DIE\n", client->clientId);
	}
    }
}
