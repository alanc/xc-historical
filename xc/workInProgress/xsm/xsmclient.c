/* $XConsortium: xpsmclient.c,v 1.4 93/09/28 17:39:43 mor Exp $ */
/******************************************************************************
Copyright 1993 by the Massachusetts Institute of Technology,

                        All Rights Reserved

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that copyright
notice and this permission notice appear in supporting documentation, and that
the name of M.I.T. not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior permission.
M.I.T. makes no representations about the suitability of this software for any
purpose.  It is provided "as is" without express or implied warranty.
******************************************************************************/

/*
 * Sample client for Pseudo Session Manager.
 *
 * Written by Ralph Mor, X Consortium.
 *
 * This client simply opens a connection with a session manager,
 * sets some test property values, then responds to any session
 * manager events (SaveYourself, Die, Interact, etc...).
 *
 * Two push buttons are set up.  One to retrieve the properties
 * that have been set by the client, the other to quit.
 * Plus a toggle to save the state of.
 */


#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Dialog.h>
#include <X11/Xaw/Toggle.h>
#include <X11/Xaw/Cardinals.h>
#include <X11/SM/SMlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/param.h>

extern Status XtInitializeICE ();

SmcConn 	smcConn;
char 		*clientId;
int		interactRequestType;
Bool		shutdownInProgress = False;

XtAppContext	appContext;

Widget		topLevel;

Widget		mainWindow;

Widget		    getPropButton;
Widget		    quitButton;

Widget		    testLabel;
Widget		    testYes;
Widget		    testNo;

Widget		    cwdLabel;
Widget		    cwdDataLabel;

Bool		testData[] = {1, 0};

Widget		dialogPopup;

Widget		    dialog;
Widget		    dialogOkButton;
Widget		    dialogCancelButton;

static XrmOptionDescRec options[] = {
{ "-smid", "*smid", XrmoptionSepArg, (caddr_t) NULL },
};

/*	resources specific to the application */
static struct resources {
    String	smid;
} appResources;

#define offset(field) XtOffsetOf(struct resources, field)
static XtResource Resources[] = {
{"smid",		"Smid",		XtRString,	sizeof(String),
     offset(smid),	XtRString,	(XtPointer) NULL},
};
#undef offset

int		saveargc;
char		**saveargv;

SaveState()
{
    XtPointer		ptr;
    SmProp *		props[6];
    SmProp		CurrentDirectory;
    SmPropValue		CurrentDirectory_val;
    SmProp		Toggle;
    SmPropValue		Toggle_val;
    SmProp		Program;
    SmPropValue		Program_val;
    SmProp		RestartCommand;
    SmProp		Environment;
    int			i, j;
    Bool		toggle;
    int			nprops;
    char		**pp;
    extern char		**environ;

    nprops = 0;

    Program.name = "Program";
    Program.type = "ARRAY8";
    Program.num_vals = 1;
    Program.vals = &Program_val;

    Program_val.length = strlen (saveargv[0]);
    Program_val.value = (SmPointer) saveargv[0];

    props[nprops++] = &Program;

    CurrentDirectory.name = "CurrentDirectory";
    CurrentDirectory.type = "ARRAY8";
    CurrentDirectory.num_vals = 1;
    CurrentDirectory.vals = &CurrentDirectory_val;

    CurrentDirectory_val.value = getcwd((char *)NULL, MAXPATHLEN+2);
    CurrentDirectory_val.length = strlen (CurrentDirectory_val.value);

    props[nprops++] = &CurrentDirectory;

    RestartCommand.name = "RestartCommand";
    RestartCommand.type = "ARRAY8";
    RestartCommand.vals =
	(SmPropValue *)malloc((saveargc+2) * (sizeof *RestartCommand.vals));

    j = 0;
    for(i=0; i < saveargc; i++) {
	if(!strcmp(saveargv[i], "-smid")) {
	    i++;	/* Skip argument */
	} else {
	    RestartCommand.vals[j].length = strlen (saveargv[i]);
	    RestartCommand.vals[j].value = (SmPointer) saveargv[i];
	    j++;
	}
    }

    RestartCommand.vals[j].value = "-smid";
    RestartCommand.vals[j].length = strlen(RestartCommand.vals[j].value);
    j++;

    RestartCommand.vals[j].value = (SmPointer)clientId;
    RestartCommand.vals[j].length = strlen(RestartCommand.vals[j].value);
    j++;
    RestartCommand.num_vals = j;

    props[nprops++] = &RestartCommand;

    ptr = XawToggleGetCurrent (testYes /* just 1 of the group */);
    toggle = *((Bool *) ptr);

    Toggle.name = "Toggle";
    Toggle.type = "CARD8";
    Toggle.num_vals = 1;
    Toggle.vals = &Toggle_val;

    Toggle_val.length = 1;
    Toggle_val.value = (SmPointer) ("NY" + toggle);

    props[nprops++] = &Toggle;
	
    Environment.num_vals = 0;
    for(pp = environ; *pp; pp++) Environment.num_vals++;
    Environment.name = "Environment";
    Environment.type = "ARRAY8";
    Environment.vals =
	(SmPropValue *)malloc(Environment.num_vals
			      * (sizeof *Environment.vals));
    for(i = 0; i < Environment.num_vals; i++) {
	Environment.vals[i].value = environ[i];
	Environment.vals[i].length = strlen(environ[i]);
    }
    printf("%s:  %d Environment entries\n", clientId, Environment.num_vals);

    props[nprops++] = &Environment;

    SmcSetProperties (smcConn, 0, nprops, props);
    printf("%s:  Set %d properties\n", clientId, nprops);

    free((char *)RestartCommand.vals);
    free((char *)Environment.vals);
    free(CurrentDirectory_val.value);
}


/*
 * Session management callbacks
 */

void
SaveYourselfProc (smcConn, clientData, saveType, shutdown, interactStyle, fast)

SmcConn   smcConn;
SmPointer clientData;
int  	  saveType;
Bool	  shutdown;
int	  interactStyle;
Bool	  fast;

{
    char *_saveType;
    char *_shutdown;
    char *_interactStyle;
    char *_fast;

    void InteractProc ();

    if (saveType == SmSaveGlobal)
	_saveType = "Global";
    else if (saveType == SmSaveLocal)
	_saveType = "Local";
    else if (saveType == SmSaveBoth)
	_saveType = "Both";
    else
	_saveType = "??? - ERROR IN SMlib, should have checked for bad value";

    if (shutdown)
	_shutdown = "True";
    else
	_shutdown = "False";

    if (interactStyle == SmInteractStyleNone)
	_interactStyle = "None";
    else if (interactStyle == SmInteractStyleErrors)
	_interactStyle = "Errors";
    else if (interactStyle == SmInteractStyleAny)
	_interactStyle = "Any";
    else
	_interactStyle =
	"??? - ERROR IN SMlib, should have checked for bad value";

    if (fast)
	_fast = "True";
    else
	_fast = "False";

    printf ("Client Id = %s, received SAVE YOURSELF [", clientId);

    printf ("Save Type = %s, Shutdown = %s, ", _saveType, _shutdown);
    printf ("Interact Style = %s, Fast = %s]\n", _interactStyle, _fast);

    shutdownInProgress = shutdown;
    if (interactStyle == SmInteractStyleAny)
    {
	SmcInteractRequest (smcConn, SmDialogNormal, InteractProc, NULL);

	printf (
	    "Client Id = %s, sent INTERACT REQUEST [Dialog Type = Normal]\n",
	    clientId);

	interactRequestType = SmDialogNormal;
    }
    else if (interactStyle == SmInteractStyleErrors)
    {
	SmcInteractRequest (smcConn, SmDialogError, InteractProc, NULL);

	printf (
	    "Client Id = %s, sent INTERACT REQUEST [Dialog Type = Errors]\n",
	    clientId);

	interactRequestType = SmDialogError;
    }
    else
    {
	SaveState();
	SmcSaveYourselfDone (smcConn, True);

	printf ("Client Id = %s, sent SAVE YOURSELF DONE [Success = True]\n",
	    clientId);
    }

    if(shutdownInProgress) XtSetSensitive (mainWindow, 0);

    printf ("\n");
}



void
InteractProc (smcConn, clientData)

SmcConn	  smcConn;
SmPointer clientData;

{
    Position	x, y, rootx, rooty;
    char        *label;
   
    printf ("Client Id = %s, received INTERACT\n", clientId);

    if (shutdownInProgress && interactRequestType == SmDialogNormal)
	XtSetSensitive (dialogCancelButton, 1);
    else
	XtSetSensitive (dialogCancelButton, 0);

    if (interactRequestType == SmDialogNormal)
    {
	if (shutdownInProgress)
	    label = "Shutdown in progress, Normal interact with user";
	else
	    label = "Normal interact with user";
    }
    else
    {
	if (shutdownInProgress)
	    label = "Shutdown in progress, Error interact with user";
	else
	    label = "Error interact with user";
    }

    /* If a shutdown is in progress, we turned this off on the first */
    /* notification.  If not, we only need to turn it off during the */
    /* interaction. */
    if(!shutdownInProgress) XtSetSensitive (mainWindow, 0);

    XtVaSetValues (dialog, XtNlabel, label, NULL);

    XtVaGetValues (mainWindow, XtNx, &x, XtNy, &y, NULL);
    XtTranslateCoords (mainWindow, x, y, &rootx, &rooty);
    XtMoveWidget (dialogPopup, rootx, rooty);

    XtPopup (dialogPopup, XtGrabNone);
}



void
DieProc (smcConn, clientData)
SmPointer clientData;

SmcConn smcConn;

{
    printf ("Client Id = %s, received DIE\n", clientId);
    printf ("\n");

    SmcCloseConnection (smcConn, NULL, 0, NULL);

    free (clientId);

    exit (0);
}



void
ShutdownCancelledProc (smcConn, clientData)

SmcConn   smcConn;
SmPointer clientData;

{
    printf ("Client Id = %s, received SHUTDOWN CANCELLED\n", clientId);
    printf ("\n");

    shutdownInProgress = False;
    XtSetSensitive (mainWindow, 1);
}



void
InitialPropReplyProc (smcConn, clientData, numProps, props)

SmcConn   smcConn;
SmPointer clientData;
int       numProps;
SmProp    **props;

{
    int i;
    Bool toggle;

    for (i = 0; i < numProps; i++)
    {
	if(!strcmp(props[i]->name, "Toggle")) {
	    toggle = *(char *)props[i]->vals[0].value == 'Y';
	    if(toggle) XawToggleSetCurrent (testYes, (XtPointer) &testData[0]);
	    else XawToggleSetCurrent (testNo, (XtPointer) &testData[1]);
	}
	SmFreeProperty (props[i]);
    }

    free ((char *) props);
}



void
PropReplyProc (smcConn, clientData, numProps, props)

SmcConn   smcConn;
SmPointer clientData;
int       numProps;
SmProp    **props;

{
    int i, j;

    printf ("Client Id = %s, there are %d properties set:\n",
	clientId, numProps);
    printf ("\n");

    for (i = 0; i < numProps; i++)
    {
	printf ("Name:		%s\n", props[i]->name);
	printf ("Type:		%s\n", props[i]->type);
	printf ("Num values:	%d\n", props[i]->num_vals);
	for (j = 0; j < props[i]->num_vals; j++)
	{
	    printf ("Value %d:	%s\n", j + 1,
		(char *) props[i]->vals[j].value);
	}
	printf ("\n");

	SmFreeProperty (props[i]);
    }

    free ((char *) props);
    printf ("\n");
}



/*
 * Xt callbacks
 */

void
GetPropXtProc (w, clientData, callData)

Widget		w;
XtPointer 	clientData;
XtPointer 	callData;

{
    SmcGetProperties (smcConn, PropReplyProc, NULL);
}



void
QuitXtProc (w, clientData, callData)

Widget		w;
XtPointer 	clientData;
XtPointer 	callData;

{
    char *reasonMsg[2];

    reasonMsg[0] = "Quit Reason 1";
    reasonMsg[1] = "Quit Reason 2";

    SmcCloseConnection (smcConn, "locale", 2, reasonMsg);

    printf ("Quit\n");

    exit (0);
}



void
DialogOkXtProc (w, clientData, callData)

Widget		w;
XtPointer 	clientData;
XtPointer 	callData;

{
    XtPopdown (dialogPopup);

    SmcInteractDone (smcConn, False);
    printf ("Client Id = %s, sent INTERACT DONE [Cancel Shutdown = False]\n",
	clientId);

    SaveState();

    SmcSaveYourselfDone (smcConn, True);
    printf ("Client Id = %s, sent SAVE YOURSELF DONE [Success = True]\n",
	clientId);
    printf ("\n");

    /* If a shutdown is in progress, we need to stay "frozen" until the */
    /* shutdown is ultimately resolved.  If not, we can unfreeze */
    /* immediately. */
    if(!shutdownInProgress) XtSetSensitive (mainWindow, 1);
}



void
DialogCancelXtProc (w, clientData, callData)

Widget		w;
XtPointer 	clientData;
XtPointer 	callData;

{
    XtPopdown (dialogPopup);

    SmcInteractDone (smcConn, True);
    printf ("Client Id = %s, sent INTERACT DONE [Cancel Shutdown = True]\n",
	clientId);

    SmcSaveYourselfDone (smcConn, True);
    printf ("Client Id = %s, sent SAVE YOURSELF DONE [Success = True]\n",
	clientId);
    printf ("\n");
    /* If a shutdown is in progress, we need to stay "frozen" until the */
    /* shutdown is ultimately resolved.  If not, we can unfreeze */
    /* immediately. */
    if(!shutdownInProgress) XtSetSensitive (mainWindow, 1);
}



/*
 * Add toggle button
 */

Widget
AddToggle (widgetName, parent, label, state, radioGroup, radioData,
    fromHoriz, fromVert)

char 		*widgetName;
Widget 		parent;
char 		*label;
int 		state;
Widget 		radioGroup;
XtPointer 	radioData;
Widget 		fromHoriz;
Widget 		fromVert;

{
    Widget 		toggle;
    XtTranslations	translations;

    toggle = XtVaCreateManagedWidget (
	widgetName, toggleWidgetClass, parent,
	XtNlabel, label,
        XtNstate, state,
        XtNradioGroup, radioGroup,
        XtNradioData, radioData,
        XtNfromHoriz, fromHoriz,
        XtNfromVert, fromVert,
        NULL);

    translations = XtParseTranslationTable ("<Btn1Down>,<Btn1Up>:set()\n");
    XtOverrideTranslations (toggle, translations);

    return (toggle);
}


/*
 * Main program
 */

main (argc, argv)

int  argc;
char **argv;

{
    SmcCallbacks 	callbacks;
    char 		errorString[256];
    char		*connectString;

    saveargv = (char **)malloc((argc+1) * (sizeof *saveargv));
    memcpy((char *)saveargv, (char *)argv, (argc+1) * (sizeof *saveargv));
    saveargc = argc;

    topLevel = XtAppInitialize (&appContext, "SAMPLE-SM-CLIENT",
	options, XtNumber(options), &argc, argv, NULL, NULL, 0);

    XtGetApplicationResources(topLevel, (XtPointer)&appResources, Resources,
			      XtNumber(Resources), (ArgList) NULL, ZERO);

    XtInitializeICE (appContext);

    mainWindow = XtCreateManagedWidget (
	"mainWindow", formWidgetClass, topLevel, NULL, 0);

    getPropButton = XtVaCreateManagedWidget (
	"getPropButton", commandWidgetClass, mainWindow,
	XtNlabel, "Get Properties",
	NULL);

    XtAddCallback (getPropButton, XtNcallback, GetPropXtProc, 0);

    quitButton = XtVaCreateManagedWidget (
	"quitButton", commandWidgetClass, mainWindow,
	XtNlabel, "Quit (and exit session)",
        XtNfromVert, getPropButton,
	NULL);

    XtAddCallback (quitButton, XtNcallback, QuitXtProc, 0);

    testLabel = XtVaCreateManagedWidget (
	"testLabel", labelWidgetClass, mainWindow,
	XtNlabel, "Test:",
        XtNfromVert, quitButton,
        XtNborderWidth, 0,
	NULL);

    testYes = AddToggle (
	"testYes", 				/* widgetName */
	mainWindow,				/* parent */
        "Yes",					/* label */
	0,					/* state */
        NULL,					/* radioGroup */
        (XtPointer) &testData[0],		/* radioData */
        testLabel,				/* fromHoriz */
        quitButton				/* fromVert */
    );

    testNo = AddToggle (
	"testNo", 				/* widgetName */
	mainWindow,				/* parent */
        "No",					/* label */
	1,					/* state */
        testYes,				/* radioGroup */
        (XtPointer) &testData[1],		/* radioData */
        testYes,				/* fromHoriz */
        quitButton				/* fromVert */
    );

    cwdLabel = XtVaCreateManagedWidget (
	"cwdLabel", labelWidgetClass, mainWindow,
	XtNlabel, "Dir:",
        XtNfromVert, testLabel,
        XtNborderWidth, 0,
	NULL);

    cwdDataLabel = XtVaCreateManagedWidget (
	"cwdDataLabel", labelWidgetClass, mainWindow,
	XtNlabel, getcwd((char *)NULL, MAXPATHLEN),
        XtNfromHoriz, cwdLabel,
        XtNfromVert, testLabel,
        XtNborderWidth, 0,
	NULL);

    dialogPopup = XtVaCreatePopupShell (
	"dialogPopup", transientShellWidgetClass, topLevel,
        XtNtitle, "Dialog",
	XtNallowShellResize, True,
        NULL);
    
    dialog = XtVaCreateManagedWidget (
	"dialog", dialogWidgetClass, dialogPopup,
        NULL);

    XtVaSetValues (XtNameToWidget (dialog, "label"),
	XtNresizable, True,
	NULL);

    dialogOkButton = XtVaCreateManagedWidget (
	"dialogOkButton", commandWidgetClass, dialog,
	XtNlabel, "OK",
	NULL);
    
    XtAddCallback (dialogOkButton, XtNcallback, DialogOkXtProc, 0);

    dialogCancelButton = XtVaCreateManagedWidget (
	"dialogCancelButton", commandWidgetClass, dialog,
	XtNlabel, "Cancel Shutdown", NULL);

    XtAddCallback (dialogCancelButton, XtNcallback, DialogCancelXtProc, 0);

    XtRealizeWidget (topLevel);


    callbacks.save_yourself.callback = SaveYourselfProc;
    callbacks.die.callback = DieProc;
    callbacks.shutdown_cancelled.callback = ShutdownCancelledProc;

    if ((smcConn = SmcOpenConnection (NULL, &callbacks,
	appResources.smid, &clientId, 256, errorString)) == NULL)
    {
	printf ("%s\n", errorString);
	return (0);
    }

    connectString = IceConnectionString (SmcGetIceConnection (smcConn));
    printf ("Connected to: %s\n", connectString);
    free (connectString);
    printf ("Client ID : %s\n", clientId);
    printf ("\n");

    SmcGetProperties (smcConn, InitialPropReplyProc, NULL);

    XtAppMainLoop (appContext);
}
