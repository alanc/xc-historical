/* $XConsortium: save.c,v 1.9 94/08/17 20:04:43 mor Exp mor $ */
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



/*
 * NOTE!!!!!!!!!!!!!!!!!!!!!!!!!!
 *
 * This save yourself code must be re-written.  A new client may try to
 * connect in the middle of this routine.
 */

static void
SaveOkXtProc (w, client_data, callData)

Widget		w;
XtPointer 	client_data;
XtPointer 	callData;

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

    XtSetSensitive (savePopup, 0);

    saveInProgress = True;
    saveDoneCount = 0;
    interactCount = 0;
    phase2RequestCount = 0;

    for (cl = ListFirst (RunningList); cl; cl = ListNext (cl))
    {
	client = (ClientRec *) cl->thing;

	client->wantsPhase2 = False;

	SmsSaveYourself (client->smsConn,
	    saveType, wantShutdown, interactStyle, fast);

	if (verbose) {
	    printf ("Client Id = %s, sent SAVE YOURSELF [", client->clientId);
	    printf ("Save Type = %s, Shutdown = %s, ", _saveType, _shutdown);
	    printf ("Interact Style = %s, Fast = False]\n", _interactStyle);
	}
    }

    if (!wantShutdown)
    {
	for (cl = ListFirst (RunningList); cl; cl = ListNext (cl))
	{
	    client = (ClientRec *) cl->thing;
	    client->userIssuedCheckpoint = True;
	}
    }

    if (verbose) {
	printf ("\n");
	printf ("Sent SAVE YOURSELF to all clients.  Waiting for\n");
	printf ("SAVE YOURSELF DONE, INTERACT REQUEST, or\n");
	printf ("SAVE YOURSELF PHASE 2 REQUEST from each client.\n");
	printf ("\n");
    }

    while (saveDoneCount + interactCount + phase2RequestCount < numClients) {
	XtAppProcessEvent (appContext, XtIMAll);
    }

    if (verbose) {
	printf ("\n");
	printf ("Received %d SAVE YOURSELF DONEs, %d INTERACT REQUESTS\n",
		saveDoneCount, interactCount);
	printf ("	%d SAVE YOURSELF PHASE 2 REQUESTS\n",
		phase2RequestCount);
    }

    if (interactCount == 0 &&
	(saveDoneCount + phase2RequestCount) != numClients) {
	if (verbose) {
	    printf ("\n");
	    printf ("INTERNAL ERROR IN XSM!  EXITING!\n");
	}
	exit (1);
    }

    if (interactCount > 0) {
	
	if (verbose)
	    printf ("\n");

	for (cl = ListFirst (RunningList); cl; cl = ListNext (cl))
	{
	    client = (ClientRec *) cl->thing;

	    if (shutdownCancelled) {
		break;
	    }
	    else if (client->interactPending) {
		SmsInteract (client->smsConn);
		if (verbose) {
		    printf ("Client Id = %s, sent INTERACT\n",
			    client->clientId);
		}
		while (client->interactPending) {
		    XtAppProcessEvent (appContext, XtIMAll);
		}
	    }
	}

	if (verbose) {
	    if (shutdownCancelled)
		printf ("\nThe shutdown was cancelled by a user\n\n");
	    else
		printf ("\nDone interacting with all clients\n\n");
	}
    }

    if (!shutdownCancelled)
    {
	while ((saveDoneCount + phase2RequestCount) < numClients)
	    XtAppProcessEvent (appContext, XtIMAll);

	for (cl = ListFirst (RunningList); cl; cl = ListNext (cl))
	{
	    client = (ClientRec *) cl->thing;

	    if (client->wantsPhase2)
	    {
		SmsSaveYourselfPhase2 (client->smsConn);
		client->wantsPhase2 = False;
	    }
	}
    }

    while (saveDoneCount < numClients)
	XtAppProcessEvent (appContext, XtIMAll);

    if (verbose)
	printf ("\nAll clients issued SAVE YOURSELF DONE\n\n");


    saveInProgress = False;

    if (!shutdownCancelled)
    {
	/*
	 * Now execute discard commands
	 */

	for (cl = ListFirst (RunningList); cl; cl = ListNext (cl))
	{
	    client = (ClientRec *) cl->thing;

	    if (client->discardCommand)
	    {
		system (client->discardCommand);
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
    }

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
		printf ("Client Id = %s, sent DIE\n", client->clientId);
	}
    }
    else
    {
	for (cl = ListFirst (RunningList); cl; cl = ListNext (cl))
	{
	    client = (ClientRec *) cl->thing;

	    SmsSaveComplete (client->smsConn);
	    if (verbose)
		printf ("Client Id = %s, sent SAVE COMPLETE\n",
		    client->clientId);
	}
    }

    if (!shutdownInProgress)
    {
	XtPopdown (savePopup);

	if (naming_session)
	{
	    XtSetSensitive (savePopup, 1);
	    XtPopup (nameSessionPopup, XtGrabNone);
	}
	else
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

    if (naming_session)
    {
	XtSetSensitive (savePopup, 1);
	XtPopup (nameSessionPopup, XtGrabNone);
    }
    else
	SetAllSensitive (1);
}



/*
 * Add toggle button
 */

Widget
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
create_save_popup ()

{
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
	XtNresizable, True,
	NULL);

    saveTypeLabel = XtVaCreateManagedWidget (
	"saveTypeLabel", labelWidgetClass, saveForm,
        XtNfromHoriz, NULL,
        XtNfromVert, saveMessageLabel,
        XtNborderWidth, 0,
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
	XtNresizable, True,
        XtNfromHoriz, NULL,
        XtNfromVert, interactStyleLabel,
        XtNvertDistance, 20,
        NULL);
    
    XtAddCallback (saveOkButton, XtNcallback, SaveOkXtProc, 0);

    saveCancelButton = XtVaCreateManagedWidget (
	"saveCancelButton", commandWidgetClass, saveForm,
	XtNresizable, True,
        XtNfromHoriz, saveOkButton,
        XtNfromVert, interactStyleLabel,
        XtNvertDistance, 20,
        NULL);

    XtAddCallback (saveCancelButton, XtNcallback, SaveCancelXtProc, 0);

    XtInstallAllAccelerators (saveForm, saveForm);
}



void
PopupSaveDialog ()

{
    Position x, y, rootx, rooty;
    char msgLabel[64];

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

    sprintf (msgLabel, "%s for session \"%s\"\n\n",
	wantShutdown ? "Shutdown" : "Checkpoint",
	session_name);

    XtVaSetValues (saveMessageLabel,
	XtNlabel, msgLabel,
	NULL);

    XtVaSetValues (saveOkButton,
	XtNlabel, wantShutdown ? "Shutdown" : "Checkpoint",
	NULL);

    if (naming_session)
	XtPopdown (nameSessionPopup);

    XtPopup (savePopup, XtGrabNone);
}




void
CheckPointXtProc (w, client_data, callData)

Widget		w;
XtPointer 	client_data;
XtPointer 	callData;

{
    if (need_to_name_session)
    {
	checkpoint_after_name = True;
	PopupNameSessionDialog ();
    }
    else
    {
	wantShutdown = False;
	PopupSaveDialog ();
    }
}




void
ShutdownSaveXtProc (w, client_data, callData)

Widget		w;
XtPointer 	client_data;
XtPointer 	callData;

{
    if (need_to_name_session)
    {
	shutdown_after_name = True;
	PopupNameSessionDialog ();
    }
    else
    {
	wantShutdown = True;
	PopupSaveDialog ();
    }
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
	    system (client->discardCommand);
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
	    printf ("Client Id = %s, sent DIE\n", client->clientId);
    }
}
