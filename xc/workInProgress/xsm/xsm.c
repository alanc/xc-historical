/* $XConsortium: xsm.c,v 1.9 94/01/19 21:05:46 converse Exp $ */
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
 * X Session Manager.
 *
 * Written by Ralph Mor, X Consortium.
 *        and Jordan Brown, Quarterdeck Office Systems
 */

#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Dialog.h>
#include <X11/Xaw/Toggle.h>
#include <X11/SM/SMlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <sys/param.h>

#include "list.h"

#define MAX_PROPS 50

extern Status XtInitializeICE ();

typedef struct _ClientRec {
    SmsConn	 	smsConn;
    IceConn		ice_conn;
    char 		*clientId;
    char		*clientHostname;
    Bool		interactPending;
    int			numProps;
    SmProp *		props[MAX_PROPS];
    struct _ClientRec	*next;
} ClientRec;

typedef struct _PendingClient {
    char		*clientId;
    char		*clientHostname;
    List		*props;
} PendingClient;

typedef struct _PendingProp {
    char		*name;
    char		*type;
    List		*values;
} PendingProp;

typedef struct _PendingValue {
    void		*value;
    int			length;
} PendingValue;

void		FreeClientInfo ();

struct _resources {
    Boolean	verbose; /* whether to report protocol activity to stdout */
} app_resources;

#define Offset(field) XtOffsetOf(struct _resources, field)
static XtResource resources [] = {
    {"verbose",  "Verbose",  XtRBoolean, sizeof(Boolean), 
	 Offset(verbose), XtRImmediate, (XtPointer) False}
};
#undef Offset

static XrmOptionDescRec options[] = {
    {"-verbose",	"*verbose",	XrmoptionNoArg,		"TRUE"},
    {"-quiet",		"*verbose",	XrmoptionNoArg,		"FALSE"},
};

char		session_save_file[MAXPATHLEN];
List		*PendingList;
ClientRec	*ClientList = NULL;
int		numClients = 0;
int		pingCount = 0;
int		saveDoneCount = 0;
int		interactCount = 0;
Bool		shutdownInProgress = False;
Bool		shutdownCancelled = False;
jmp_buf		JumpHere;

int		saveTypeData[] = {SmSaveGlobal,
			  	  SmSaveLocal,
		  		  SmSaveBoth};
Bool		shutdownData[] = {1, 0};
int		interactStyleData[] = {SmInteractStyleNone,
			       	       SmInteractStyleErrors,
		       		       SmInteractStyleAny};
Bool		fastData[] = {1, 0};

XtAppContext	appContext;

Widget		topLevel;

Widget		    mainWindow;

Widget		        listButton;
Widget			saveButton;
Widget			propButton;
Widget			pingButton;
Widget			startButton;

Widget		    savePopup;

Widget			saveForm;

Widget			    saveTypeLabel;
Widget			    saveTypeGlobal;
Widget			    saveTypeLocal;
Widget			    saveTypeBoth;

Widget			    shutdownLabel;
Widget			    shutdownYes;
Widget			    shutdownNo;

Widget			    interactStyleLabel;
Widget			    interactStyleNone;
Widget			    interactStyleErrors;
Widget			    interactStyleAny;

Widget			    fastLabel;
Widget			    fastYes;
Widget			    fastNo;

Widget			    saveOkButton;
Widget			    saveCancelButton;

Widget		    shutdownPopup;

Widget			shutdownDialog;

Widget			    shutdownOkButton;
Widget			    shutdownCancelButton;




print_prop(prop)
SmProp	*prop;
{
    int j;

    printf ("   Name:	%s\n", prop->name);
    printf ("   Type:	%s\n", prop->type);
    printf ("   Num values:	%d\n", prop->num_vals);
    for (j = 0; j < prop->num_vals; j++)
    {
	printf ("   Value %d:	%s\n", j + 1,
	    (char *) prop->vals[j].value);
    }
    printf ("\n");
}

nomem()
{
    fprintf(stderr, "Insufficient memory.\n");
    exit(255);
}

exit_sm ()
{
    if (app_resources.verbose)
	printf ("\nSESSION MANAGER GOING AWAY!\n");
    system ("iceauth source .xsm-rem-auth");
    exit (0);
}


SetInitialProperties(client, pendclient)
ClientRec	*client;
PendingClient	*pendclient;
{
    int			index;
    SmProp		*prop;
    SmPropValue		*val;
    List		*pl;
    List		*vl;
    PendingProp		*pprop;
    PendingValue	*pval;

    if (app_resources.verbose)
	printf("Setting initial properties for %s\n", client->clientId);

    index = 0;
    for(pl = ListFirst(pendclient->props); pl; pl = ListNext(pl)) {
	pprop = (PendingProp *)pl->thing;
	prop = (SmProp *)malloc(sizeof *prop);
	client->props[index] = prop;
	prop->name = pprop->name;
	prop->type = pprop->type;
	prop->num_vals = ListCount(pprop->values);
	prop->vals =
	    (SmPropValue *)malloc(prop->num_vals * sizeof(SmPropValue));
	val = prop->vals;
	for(vl = ListFirst(pprop->values); vl; vl = ListNext(vl)) {
	    pval = (PendingValue *)vl->thing;
	    val->value = pval->value;
	    val->length = pval->length;
	    val++;
	}
	ListFreeAll(pprop->values);
	free(pprop);
	index++;
    }
    client->numProps = index;

    ListFreeAll(pendclient->props);
    XtFree(pendclient->clientId);
    free(pendclient);
}

/*
 * Session Manager callbacks
 */

Status
RegisterClientProc (smsConn, managerData, previousId)

SmsConn 	smsConn;
SmPointer 	managerData;
char 		*previousId;

{
    ClientRec	*client = (ClientRec *) managerData;
    char 	*id;
    List	*cl;

    if (app_resources.verbose) {
	printf (
	"On IceConn fd = %d, received REGISTER CLIENT [Previous Id = %s]\n",
	IceConnectionNumber (client->ice_conn),
	previousId ? previousId : "NULL");
	printf ("\n");
    }

    if (previousId)
    {
	id = malloc (strlen (previousId) + 1);
	strcpy (id, previousId);
    }
    else
	id = SmsGenerateClientID (smsConn);

    SmsRegisterClientReply (smsConn, id);
    client->clientId = id;
    client->clientHostname = SmsClientHostName (smsConn);

    if (app_resources.verbose) {
	printf (
	"On IceConn fd = %d, sent REGISTER CLIENT REPLY [Client Id = %s]\n",
	IceConnectionNumber (client->ice_conn), id);
	printf ("\n");
    }

    if(previousId) {
	for(cl = ListFirst(PendingList); cl; cl = ListNext(cl)) {
	    if(!strcmp(((PendingClient *)cl->thing)->clientId, previousId)) {
		SetInitialProperties(client, (PendingClient *)cl->thing);
		ListFreeOne(cl);
		break;
	    }
	}
    }

    if(previousId) free (previousId);

    return (1);
}



void
InteractRequestProc (smsConn, managerData, dialogType)

SmsConn 	smsConn;
SmPointer  	managerData;
int		dialogType;

{
    ClientRec	*client = (ClientRec *) managerData;

    if (app_resources.verbose) {
	printf ("Client Id = %s, received INTERACT REQUEST [Dialog Type = ",
		client->clientId);
	if (dialogType == SmDialogError)
	    printf ("Error]\n");
	else if (dialogType == SmDialogNormal)
	    printf ("Normal]\n");
	else
	    printf ("Error in SMlib: should have checked for bad value]\n");
    }

    client->interactPending = True;
    interactCount++;
}



void
InteractDoneProc (smsConn, managerData, cancelShutdown)
    SmsConn	smsConn;
    SmPointer 	managerData;
    Bool	cancelShutdown;

{
    ClientRec	*client = (ClientRec *) managerData;

    if (app_resources.verbose) {
	printf (
	"Client Id = %s, received INTERACT DONE [Cancel Shutdown = %s]\n",
	client->clientId, cancelShutdown ? "True" : "False");
    }

    client->interactPending = False;

    if (cancelShutdown && !shutdownCancelled) {
	shutdownCancelled = True;
	for (client = ClientList; client; client = client->next) {
	    SmsShutdownCancelled (client->smsConn);
	    if (app_resources.verbose) 
		printf ("Client Id = %s, sent SHUTDOWN CANCELLED\n",
			client->clientId);
	}
    }
}



void
SaveYourselfDoneProc (smsConn, managerData, success)
    SmsConn     smsConn;
    SmPointer 	managerData;
    Bool	success;

{
    ClientRec	*client = (ClientRec *) managerData;

    if (app_resources.verbose) 
	printf("Client Id = %s, received SAVE YOURSELF DONE [Success = %s]\n",
	       client->clientId, success ? "True" : "False");

    if (shutdownCancelled && client->interactPending)
	client->interactPending = False;

    saveDoneCount++;
}



void
CloseConnectionProc (smsConn, managerData, count, reasonMsgs)
    SmsConn 	smsConn;
    SmPointer  	managerData;
    int		count;
    char 	**reasonMsgs;

{
    ClientRec	*client = (ClientRec *) managerData;
    ClientRec 	*next = client->next;
    ClientRec	*ptr;
    int i;

    if (app_resources.verbose) {
	printf("Client Id = %s, received CONNECTION CLOSED\n",
	       client->clientId);

	for (i = 0; i < count; i++)
	    printf ("   Reason string %d: %s\n", i + 1, reasonMsgs[i]);
	printf ("\n");
    }

    SmFreeReasons (count, reasonMsgs);
    SmsCleanUp (smsConn);

    if (app_resources.verbose) {
	printf ("ICE Connection closed, IceConn fd = %d\n",
		IceConnectionNumber (client->ice_conn));
	printf ("\n");
    }

    IceSetShutdownNegotiation (client->ice_conn, False);
    IceCloseConnection (client->ice_conn);

    if (client == ClientList)
    {
	FreeClientInfo (client);
	ClientList = next;
    }
    else
    {
	ptr = ClientList;
	while (ptr && ptr->next != client)
	    ptr = ptr->next;

	if (ptr->next == client)
	{
	    FreeClientInfo (client);
	    ptr->next = next;
	}
    }

    numClients--;

    if (shutdownInProgress && numClients == 0)
    {
	exit_sm ();
    }
}

SetProperty(client, prop)
ClientRec	*client;
SmProp		*prop;
{
    int	index, j;

    for (j = 0; j < client->numProps; j++)
	if (strcmp (prop->name, client->props[j]->name) == 0)
	{
	    SmFreeProperty (client->props[j]);
	    break;
	}

    if (j < client->numProps)
	index = j;
    else
    {
	index = client->numProps;
	client->numProps++;

	if (client->numProps > MAX_PROPS)
	    return;
    }

    client->props[index] = prop;
}

void
SetPropertiesProc (smsConn, managerData, numProps, props)

SmsConn 	smsConn;
SmPointer 	managerData;
int		numProps;
SmProp 		**props;

{
    ClientRec	*client = (ClientRec *) managerData;
    int		i, j;

    if (app_resources.verbose) {
	printf ("Client Id = %s, received SET PROPERTIES ", client->clientId);
	printf ("[Num props = %d]\n", numProps);
    }

    for (i = 0; i < numProps; i++) {
	if(app_resources.verbose)
	    print_prop(props[i]);
	SetProperty(client, props[i]);
    }
    free ((char *) props);
}



void
GetPropertiesProc (smsConn, managerData)

SmsConn 	smsConn;
SmPointer 	managerData;

{
    ClientRec	*client = (ClientRec *) managerData;
    int		i, j;

    if (app_resources.verbose) {
	printf ("Client Id = %s, received GET PROPERTIES\n", client->clientId);
	printf ("\n");

	for (i = 0; i < client->numProps; i++) {
	    print_prop(client->props[i]);
	}
	printf ("\n");
    }

    SmsReturnProperties (smsConn, client->numProps, client->props);

    if (app_resources.verbose) {
	printf ("Client Id = %s, sent PROPERTIES REPLY [Num props = %d]\n",
		client->clientId, client->numProps);
    }
}



void
PingReplyProc (ice_conn, client_data)

IceConn		ice_conn;
IcePointer	client_data;

{
    ClientRec *client = (ClientRec *) client_data;
    printf ("Client Id = %s, received PING REPLY\n", client->clientId);
    pingCount--;
}



void
NewClientProc (smsConn, managerData, callbacksRet)

SmsConn		smsConn;
SmPointer  	managerData;
SmsCallbacks	*callbacksRet;

{
    ClientRec *newClient = (ClientRec *) malloc (sizeof (ClientRec));

    if(!newClient) nomem();

    newClient->smsConn = smsConn;
    newClient->ice_conn = SmsGetIceConnection (smsConn);
    newClient->clientId = NULL;
    newClient->clientHostname = NULL;
    newClient->interactPending = False;
    newClient->numProps = 0;
    newClient->next = ClientList;

    ClientList = newClient;
    numClients++;

    if (app_resources.verbose) {
	printf("On IceConn fd = %d, client set up session mngmt protocol\n\n",
	       IceConnectionNumber (newClient->ice_conn));
    }

    /*
     * Set up session manager callbacks.
     */

    callbacksRet->register_client.callback 	= RegisterClientProc;
    callbacksRet->register_client.manager_data  = (SmPointer) newClient;

    callbacksRet->interact_request.callback 	= InteractRequestProc;
    callbacksRet->interact_request.manager_data = (SmPointer) newClient;

    callbacksRet->interact_done.callback	= InteractDoneProc;
    callbacksRet->interact_done.manager_data    = (SmPointer) newClient;

    callbacksRet->save_yourself_done.callback 	   = SaveYourselfDoneProc;
    callbacksRet->save_yourself_done.manager_data  = (SmPointer) newClient;

    callbacksRet->close_connection.callback 	 = CloseConnectionProc;
    callbacksRet->close_connection.manager_data  = (SmPointer) newClient;

    callbacksRet->set_properties.callback 	= SetPropertiesProc;
    callbacksRet->set_properties.manager_data   = (SmPointer) newClient;

    callbacksRet->get_properties.callback	= GetPropertiesProc;
    callbacksRet->get_properties.manager_data   = (SmPointer) newClient;
}



/*
 * Xt callbacks
 */

void
ListClientsXtProc (w, client_data, callData)
    Widget	w;
    XtPointer 	client_data;
    XtPointer 	callData;

{
    ClientRec *client = ClientList;

    printf ("\n");
    if (client == NULL) {
	printf ("There are no clients registered with the SM\n");
    } else {
	printf ("The following client IDs are registered with the SM:\n");
	printf ("\n");
    }
    while (client) {
	printf ("  %s\n", client->clientId);
	client = client->next;
    }
    printf ("\n");
}



void
SaveYourselfXtProc (w, client_data, callData)

Widget		w;
XtPointer 	client_data;
XtPointer 	callData;

{
    Position	x, y, rootx, rooty;

    XtVaGetValues (mainWindow, XtNx, &x, XtNy, &y, NULL);
    XtTranslateCoords (mainWindow, x, y, &rootx, &rooty);

    if (ClientList == NULL)
    {
	XtMoveWidget (shutdownPopup, rootx, rooty);
	XtPopup (shutdownPopup, XtGrabNone);
    }
    else
    {
	XawToggleSetCurrent (saveTypeBoth,
	    (XtPointer) &saveTypeData[2]);
	XawToggleSetCurrent (shutdownNo,
	    (XtPointer) &shutdownData[1]);
	XawToggleSetCurrent (interactStyleAny,
	    (XtPointer) &interactStyleData[2]);
	XawToggleSetCurrent (fastNo,
	    (XtPointer) &fastData[1]);

	XtMoveWidget (savePopup, rootx, rooty);
	XtPopup (savePopup, XtGrabNone);
    }

    XtSetSensitive (mainWindow, 0);
}



void
SaveOkXtProc (w, client_data, callData)

Widget		w;
XtPointer 	client_data;
XtPointer 	callData;

{
    ClientRec	*client;
    XtPointer	ptr;
    int		saveType;
    Bool	shutdown;
    int		interactStyle;
    Bool	fast;
    char	*_saveType;
    char	*_shutdown;
    char	*_interactStyle;
    char	*_fast;

    ptr = XawToggleGetCurrent (saveTypeGlobal /* just 1 of the group */);
    saveType = *((int *) ptr);

    ptr = XawToggleGetCurrent (shutdownYes /* just 1 of the group */);
    shutdown = *((Bool *) ptr);

    ptr = XawToggleGetCurrent (interactStyleNone /* just 1 of the group */);
    interactStyle = *((int *) ptr);

    ptr = XawToggleGetCurrent (fastYes /* just 1 of the group */);
    fast = *((Bool *) ptr);

    if (saveType == SmSaveGlobal)
	_saveType = "Global";
    else if (saveType == SmSaveLocal)
	_saveType = "Local";
    else
	_saveType = "Both";

    if (shutdown)
	_shutdown = "True";
    else
	_shutdown = "False";

    if (interactStyle == SmInteractStyleNone)
	_interactStyle = "None";
    else if (interactStyle == SmInteractStyleErrors)
	_interactStyle = "Errors";
    else
	_interactStyle = "Any";

    if (fast)
	_fast = "True";
    else
	_fast = "False";

    XtSetSensitive (savePopup, 0);

    for (client = ClientList; client; client = client->next)
    {
	SmsSaveYourself (client->smsConn,
	    saveType, shutdown, interactStyle, fast);

	if (app_resources.verbose) {
	    printf ("Client Id = %s, sent SAVE YOURSELF [", client->clientId);
	    printf ("Save Type = %s, Shutdown = %s, ", _saveType, _shutdown);
	    printf ("Interact Style = %s, Fast = %s]\n",
		    _interactStyle, _fast);
	}
    }
    if (app_resources.verbose) {
	printf ("\n");
	printf ("Sent SAVE YOURSELF to all clients.  Waiting for\n");
	printf ("SAVE YOURSELF DONE or INTERACT REQUEST from each client.\n");
	printf ("\n");
    }

    saveDoneCount = 0;
    interactCount = 0;

    while (saveDoneCount + interactCount < numClients) {
	XtAppProcessEvent (appContext, XtIMAll);
    }

    if (app_resources.verbose) {
	printf ("\n");
	printf ("Received %d SAVE YOURSELF DONEs, %d INTERACT REQUESTS\n",
		saveDoneCount, interactCount);
    }

    if (interactCount == 0 && saveDoneCount != numClients) {
	if (app_resources.verbose) {
	    printf ("\n");
	    printf ("INTERNAL ERROR IN PSEUDO-SM!  EXITING!\n");
	}
	exit (1);
    }

    if (interactCount > 0) {
	
	if (app_resources.verbose)
	    printf ("\n");

	client = ClientList;
	while (client) {
	    if (shutdownCancelled) {
		break;
	    }
	    else if (client->interactPending) {
		SmsInteract (client->smsConn);
		if (app_resources.verbose) {
		    printf ("Client Id = %s, sent INTERACT\n",
			    client->clientId);
		}
		while (client->interactPending) {
		    XtAppProcessEvent (appContext, XtIMAll);
		}
	    }
	    client = client->next;
	}

	if (app_resources.verbose) {
	    if (shutdownCancelled)
		printf ("\nThe shutdown was cancelled by a user\n\n");
	    else
		printf ("\nDone interacting with all clients\n\n");
	}
    }

    while (saveDoneCount < numClients) {
	XtAppProcessEvent (appContext, XtIMAll);
    }

    if (app_resources.verbose)
	printf ("\nAll clients issued SAVE YOURSELF DONE\n\n");

    write_save();

    if (shutdown && shutdownCancelled) {
	shutdownCancelled = False;
    } else if (shutdown) {
	shutdownInProgress = True;
	client = ClientList;
	while (client) {
	    SmsDie (client->smsConn);
	    if (app_resources.verbose)
		printf ("Client Id = %s, sent DIE\n", client->clientId);
	    client = client->next;
	}
    }

    if (!shutdownInProgress) {
	XtPopdown (savePopup);
	XtSetSensitive (savePopup, 1);
	XtSetSensitive (mainWindow, 1);
    }
}



void
SaveCancelXtProc (w, client_data, callData)

Widget		w;
XtPointer 	client_data;
XtPointer 	callData;

{
    XtPopdown (savePopup);
    XtSetSensitive (mainWindow, 1);
}



void
ShutdownOkXtProc (w, client_data, callData)

Widget		w;
XtPointer 	client_data;
XtPointer 	callData;

{
    write_save();
    exit_sm ();
}



void
ShutdownCancelXtProc (w, client_data, callData)

Widget		w;
XtPointer 	client_data;
XtPointer 	callData;

{
    XtPopdown (shutdownPopup);
    XtSetSensitive (mainWindow, 1);
}



void
ListPropXtProc (w, client_data, callData)

Widget		w;
XtPointer 	client_data;
XtPointer 	callData;

{
    ClientRec *client = ClientList;

    if (!app_resources.verbose) return;

    if (client == NULL) {
	printf ("There are no clients registered with the SM\n");
	return;
    }

    while (client) {
	if (client->numProps == 0) {
	    printf("Client Id = %s, no properties are set\n",
		   client->clientId);
	} else {
	    int i, j;

	    printf ("Client Id = %s, the following properties are set:\n",
		    client->clientId);
	    for (i = 0; i < client->numProps; i++)
		print_prop(client->props[i]);
	}
	client = client->next;
    }
}



void
StartXtProc (w, client_data, callData)

Widget		w;
XtPointer 	client_data;
XtPointer 	callData;

{
    switch(fork()) {
	case -1:
	    perror("fork");
	    break;
	case 0:
	    if (app_resources.verbose)
		execlp("xsmclient", "xsmclient", "-verbose", (char *)NULL);
	    else
		execlp("xsmclient", "xsmclient", (char *)NULL);
	    perror("xsmclient");
	    _exit(255);
	default:
	    break;
    }
}



void
PingXtProc (w, client_data, callData)

Widget		w;
XtPointer 	client_data;
XtPointer 	callData;

{
    ClientRec *client = ClientList;

    pingCount = 0;
    printf ("\n");
    if (client == NULL ) {
	printf ("There are no clients registered with the SM\n");
	printf ("\n");
	return;
    }
    while (client) {
	IcePing (client->ice_conn, PingReplyProc, (IcePointer) client);
	pingCount++;
	printf ("Client Id = %s, sent PING\n", client->clientId);
	client = client->next;
    }
    printf ("\n");
    while (pingCount > 0)
	XtAppProcessEvent(appContext, XtIMAll);
    printf ("\n");
}



/*
 * Xt callback invoked when a client attempts to connect.
 */

void
newConnectionXtProc (client_data, source, id)

XtPointer	client_data;
int 		*source;
XtInputId	*id;

{
    IceConn 	ice_conn;
    char	*connstr;

    ice_conn = IceAcceptConnection((IceListenObj) client_data);
    if (! ice_conn) {
	if (app_resources.verbose)
	    printf ("IceAcceptConnection failed\n");
    } else {
	while (IceConnectionStatus (ice_conn) == IceConnectPending) {
	    XtAppProcessEvent (appContext, XtIMAll);
	}

	if (IceConnectionStatus (ice_conn) == IceConnectAccepted) {
	    if (app_resources.verbose) {
		printf ("ICE Connection opened by client, IceConn fd = %d, ",
			IceConnectionNumber (ice_conn));
		connstr = IceConnectionString (ice_conn);
		printf ("Accept at networkId %s\n", connstr);
		free (connstr);
		printf ("\n");
	    }
	} else {
	    if (app_resources.verbose) printf ("ICE Connection rejected!\n");
	    IceCloseConnection (ice_conn);
	}
    }
}



/*
 * Install IO error handler.  This will detect clients that break their
 * connection with the SM unexpectidly.
 */

void
myIOErrorHandler (ice_conn)

IceConn 	ice_conn;

{
    if (ClientList == NULL)
    {
	/*
	 * The client must have disconnected before the ICE connection
	 * became valid.  Example: ICE authentication failed.
	 */

	;
    }
    else
    {
	ClientRec *ptr = ClientList;
	ClientRec *prev = NULL;
	
	while (ptr && ptr->ice_conn != ice_conn)
	{
	    prev = ptr;
	    ptr = ptr->next;
	}

	if (!ptr)
	{
	    fprintf (stderr, "Internal error; couldn't find ice_conn\n");
	    exit (1);
	}
	else
	{
	    if (prev == NULL)
		ClientList = ptr->next;
	    else
		prev->next = ptr->next;

	    SmsCleanUp (ptr->smsConn);
	    FreeClientInfo (ptr);
	    
	    printf ("ICE Connection terminated (fd = %d)\n",
	        IceConnectionNumber (ice_conn));
	    printf ("\n");

	    IceSetShutdownNegotiation (ice_conn, False);
	    IceCloseConnection (ice_conn);

	    numClients--;

	    if (shutdownInProgress && numClients == 0)
	    {
		exit_sm ();
	    }
	}
    }


    /*
     * We can't return.  Must do a long jump.  Make sure any
     * popups are uppopped.
     */

    XtPopdown (shutdownPopup);
    XtPopdown (savePopup);
    XtSetSensitive (shutdownPopup, 1);
    XtSetSensitive (savePopup, 1);
    XtSetSensitive (mainWindow, 1);

    longjmp (JumpHere, 1);
}    


/*
 * Host Based Authentication Callback.  This callback is invoked if
 * the connecting client can't offer any authentication methods that
 * we can accept.  We can accept/reject based on the hostname.
 */

Bool
HostBasedProc (hostname)

char *hostname;

{
    printf ("Attempt to connect from: %s failed.\n", hostname);
    return (0);
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



void
FreeClientInfo (client)

ClientRec *client;

{
    if (client)
    {
	int i;

	XtFree (client->clientId);
	XtFree (client->clientHostname);

	for (i = 0; i < client->numProps; i++)
	    SmFreeProperty (client->props[i]);

	free ((char *) client);
    }
}



Bool
getline(pbuf, plen, f)
char	**pbuf;
int	*plen;
FILE	*f;
{
	int c;
	int i;

	i = 0;
	while(1) {
	    if(i+2 > *plen) {
		if(*plen) *plen *= 2;
		else *plen = BUFSIZ;
		if(*pbuf) *pbuf = realloc(*pbuf, *plen);
		else *pbuf = malloc(*plen);
	    }
	    c = getc(f);
	    if(c == EOF) break;
	    (*pbuf)[i++] = c;
	    if(c == '\n') break;
	}
	(*pbuf)[i] = '\0';
	return i;
}

read_save()
{
    char		*buf;
    int			buflen;
    char		*p;
    char		*q;
    PendingClient	*c;
    PendingProp		*prop;
    PendingValue	*val;
    FILE		*f;
    int			state;

    PendingList = ListInit();
    if(!PendingList) nomem();

    p = getenv("HOME");
    if(!p) p = ".";
    strcpy(session_save_file, p);
    strcat(session_save_file, "/.SM-save");

    f = fopen(session_save_file, "r");
    if(!f) {
	if (app_resources.verbose)
	    printf("No session save file.\n");
	return;
    }
    if (app_resources.verbose)
	printf("Reading session save file...\n");

    buf = NULL;
    buflen = 0;
    state = 0;
    while(getline(&buf, &buflen, f)) {
	if(p = strchr(buf, '\n')) *p = '\0';
	for(p = buf; *p && isspace(*p); p++) /* LOOP */;
	if(*p == '#') continue;

	if(!*p) {
	    state = 0;
	    continue;
	}

	if(!isspace(buf[0])) {
	    switch(state) {
		case 0:
		    c = (PendingClient *)malloc(sizeof *c);
		    if(!c) nomem();

		    c->clientId = XtNewString(p);
		    c->clientHostname = NULL;  /* set in next state */

		    c->props = ListInit();
		    if(!c->props) nomem();

		    if(!ListAddLast(PendingList, (void *)c)) nomem();

		    state = 1;
		    break;

		case 1:
		    c->clientHostname = XtNewString(p);
                    state = 2;
                    break;

		case 2:
		case 4:
		    prop = (PendingProp *)malloc(sizeof *prop);
		    if(!prop) nomem();

		    prop->name = XtNewString(p);
		    prop->values = ListInit();
		    if(!prop->values) nomem();

		    prop->type = NULL;

		    if(!ListAddLast(c->props, (void *)prop)) nomem();

		    state = 3;
		    break;

		case 3:
		    prop->type = XtNewString(p);
		    state = 4;
		    break;

		default:
		    fprintf(stderr, "state %d\n", state);
		    fprintf(stderr,
			    "Corrupt save file line ignored:\n%s\n", buf);
		    continue;
	    }
	} else {
	    if (state != 4) {
		fprintf(stderr, "Corrupt save file line ignored:\n%s\n", buf);
		continue;
	    }
	    val = (PendingValue *)malloc(sizeof *val);
	    if(!val) nomem();

	    val->length = strlen(p);
	    /* NEEDSWORK:  Binary data */
	    val->value = XtNewString(p);

	    if(!ListAddLast(prop->values, (void *)val)) nomem(); 
	}
    }
    fclose(f);
}

write_save()
{
    FILE *f;
    ClientRec *client;
    SmProp *prop;
    int i, j;

    f = fopen(session_save_file, "w");
    if(!f)
    {
	perror("open session save file for write");
    } else {
	for(client = ClientList; client; client = client->next)
	{
	    fprintf(f, "%s\n", client->clientId);
	    fprintf(f, "%s\n", client->clientHostname);
	    for(i = 0; i < client->numProps; i++) {
		prop = client->props[i];
		fprintf(f, "%s\n", prop->name);
		fprintf(f, "%s\n", prop->type);
		for(j = 0; j < prop->num_vals; j++) {
		    fprintf(f, "\t%s\n", prop->vals[j].value);
		}
	    }
	    fprintf(f, "\n");
	}
	fclose(f);
    }
}

strbw(a, b)
char *a;
char *b;
{
	return !strncmp(a, b, strlen(b));
}

restart_everything()
{
    List *cl;
    List *pl;
    List *vl;
    PendingClient *c;
    PendingProp *prop;
    PendingValue *v;
    char	*cwd;
    char	*program;
    char	**args;
    char	**env;
    char	**pp;
    int		cnt;
    extern char **environ;
    char	*p;
    static char	envDISPLAY[]="DISPLAY";
    static char	envSESSION_MANAGER[]="SESSION_MANAGER";
    static char	envAUDIOSERVER[]="AUDIOSERVER";
    char	*display_env;
    char	*session_env;
    char	*audio_env;

    display_env = NULL;
    if(p = getenv(envDISPLAY)) {
	display_env = malloc(strlen(envDISPLAY)+1+strlen(p)+1);
	if(!display_env) nomem();
	sprintf(display_env, "%s=%s", envDISPLAY, p);
    }

    session_env = NULL;
    if(p = getenv(envSESSION_MANAGER)) {
	session_env = malloc(strlen(envSESSION_MANAGER)+1+strlen(p)+1);
	if(!session_env) nomem();
	sprintf(session_env, "%s=%s", envSESSION_MANAGER, p);
    }

    audio_env = NULL;
    if(p = getenv(envAUDIOSERVER)) {
	audio_env = malloc(strlen(envAUDIOSERVER)+1+strlen(p)+1);
	if(!audio_env) nomem();
	sprintf(audio_env, "%s=%s", envAUDIOSERVER, p);
    }

    for(cl = ListFirst(PendingList); cl; cl = ListNext(cl)) {
	c = (PendingClient *)cl->thing;

	if (app_resources.verbose) {
	    printf("Restarting id '%s'...\n", c->clientId);
	    printf("Host = %s\n", c->clientHostname);
	}
	cwd = ".";
	env = NULL;
	program=NULL;
	args=NULL;

	for(pl = ListFirst(c->props); pl; pl = ListNext(pl)) {
	    prop = (PendingProp *)pl->thing;
	    if(!strcmp(prop->name, "Program")) {
		vl = ListFirst(prop->values);
		if(vl) program = ((PendingValue *)vl->thing)->value;
	    } else if(!strcmp(prop->name, "CurrentDirectory")) {
		vl = ListFirst(prop->values);
		if(vl) cwd = ((PendingValue *)vl->thing)->value;
	    } else if(!strcmp(prop->name, "RestartCommand")) {
		cnt = ListCount(prop->values);
		args = (char **)malloc((cnt+1) * sizeof(char *));
		pp = args;
		for(vl = ListFirst(prop->values); vl; vl = ListNext(vl)) {
		    *pp++ = ((PendingValue *)vl->thing)->value;
		}
		*pp = NULL;
	    } else if(!strcmp(prop->name, "Environment")) {
		cnt = ListCount(prop->values);
		env = (char **)malloc((cnt+3+1) * sizeof(char *));
		pp = env;
		for(vl = ListFirst(prop->values); vl; vl = ListNext(vl)) {
		    p = ((PendingValue *)vl->thing)->value;
		    if((display_env && strbw(p, "DISPLAY="))
		    || (session_env && strbw(p, "SESSION_MANAGER="))
		    || (audio_env && strbw(p, "AUDIOSERVER="))
		        ) continue;
		    *pp++ = p;
		}
		if(display_env) *pp++ = display_env;
		if(session_env) *pp++ = session_env;
		if(audio_env) *pp++ = audio_env;
		*pp = NULL;
	    }
	}

	if(program && args) {
	    if (app_resources.verbose) {
		printf("\t%s\n", program);
		printf("\t");
		for(pp = args; *pp; pp++) printf("%s ", *pp);
		printf("\n");
	    }

	    if (!strncmp(c->clientHostname, "local/", 6))
	    {
		/*
		 * The client is being restarted on the local machine.
		 */

		switch(fork()) {
		case -1:
		    perror("fork");
		    break;
		case 0:		/* kid */
		    chdir(cwd);
		    if(env) environ = env;
		    execvp(program, args);
		    perror("execve");
		    _exit(255);
		default:	/* parent */
		    break;
		}
	    }
	    else
	    {
		/*
		 * The client is being restarted on a remote machine.
		 * Use the xrsh protocol to do the restart.
		 */

		int pipefd[2], i;
		FILE *fp;

		if (pipe (pipefd) < 0)
		{
		    perror ("pipe error");
		}
		else
		{
		    switch(fork()) {
		    case -1:
			perror("fork");
			break;
		    case 0:		/* kid */
			close (pipefd[1]);
			close (0);
			dup (pipefd[0]);
			close (pipefd[0]);

			execlp ("remsh", c->clientHostname,
			    "xrshsrv", (char *) 0);
			perror("execlp");
			_exit(255);
		    default:		/* parent */
			close (pipefd[0]);
			fp = fdopen (pipefd[1], "w");
			fprintf (fp, "CONTEXT X\n");
			fprintf (fp, "DIR %s\n", cwd);

/*
 * There are spaces inside some of the damn env values, and xrshsrv
 * will barf on spaces.  Need to fix this, but for now, just set the
 * important env variables.
 */

			for (i = 0; env[i]; i++)
			    if (strstr (env[i], "PATH"))
				fprintf (fp, "MISC X %s\n", env[i]);
			fprintf (fp, "MISC X %s\n", display_env);
			fprintf (fp, "MISC X %s\n", session_env);
/*
 * To do: set the auth data.
 *   use AUTH authscheme authdata
 *   The remote machine should have it config'd to invoke iceauth add
 */

			fprintf (fp, "EXEC %s %s", program, program);
			for (i = 1; args[i]; i++)
			    fprintf (fp, " %s", args[i]);
			fprintf (fp, "\n\n");
			fclose (fp);
			break;
		    }
		}
	    }
	} else {
	    fprintf(stderr, "Can't restart ID '%s':  no program or no args\n",
		c->clientId);
	}
	if(args) free((char *)args);
	if(env) free((char *)env);
    }
    if(display_env) free(display_env);
    if(session_env) free(session_env);
    if(audio_env) free(audio_env);
}



void
write_iceauth (addfp, removefp, entry)

FILE *addfp;
FILE *removefp;
IceAuthDataEntry *entry;

{
    fprintf (addfp,
	"add %s \"\" %s %s \"%s\"\n",
	entry->protocol_name,
        entry->address,
        entry->auth_name,
        entry->auth_data);

    fprintf (removefp,
	"remove protoname=%s protodata=\"\" address=%s authname=%s\n",
	entry->protocol_name,
        entry->address,
        entry->auth_name);
}



Status
set_auth (count, listenObjs)

int		count;
IceListenObj	*listenObjs;

{
    FILE		*addfp;
    FILE		*removefp;
    IceAuthDataEntry	authDataEntry[2];
    int			i;

    if (!(addfp = fopen (".xsm-add-auth", "w")))
	return (0);

    if (!(removefp = fopen (".xsm-rem-auth", "w")))
	return (0);

    authDataEntry[0].protocol_name = "ICE";
    authDataEntry[0].auth_name = "ICE-MAGIC-COOKIE-1";
    authDataEntry[0].auth_data_length = 14;
    authDataEntry[0].auth_data = "1stMagicCookie";

    authDataEntry[1].protocol_name = "XSMP";
    authDataEntry[1].auth_name = "ICE-MAGIC-COOKIE-1";
    authDataEntry[1].auth_data_length = 14;
    authDataEntry[1].auth_data = "2ndMagicCookie";

    for (i = 0; i < count; i++)
    {
	char *networkId = IceGetListenNetworkId (listenObjs[i]);

	authDataEntry[0].address = networkId;
	authDataEntry[1].address = networkId;

	write_iceauth (addfp, removefp, &authDataEntry[0]);
	write_iceauth (addfp, removefp, &authDataEntry[1]);

	IceSetPaAuthData (2, authDataEntry);

	IceSetHostBasedAuthProc (listenObjs[i], HostBasedProc);

	free (networkId);
    }

    fclose (addfp);
    fclose (removefp);

    system ("iceauth source .xsm-add-auth");

    return (1);
}

static void Syntax(call)
    char *call;
{
    (void) fprintf(stderr, "usage: %s [-verbose] [-quiet]\n", call);
    exit(2);
}

main(argc, argv)
    int  argc;
    char **argv;
{
    IceListenObj *listenObjs;
    char 	*networkIds;
    int  	count, i;
    char	*p;
    char *	progName;
    char 	errormsg[256];
    static	char environment_name[] = "SESSION_MANAGER";

    p = strrchr(argv[0], '/');
    progName = (p ? p + 1 : argv[0]);
    topLevel = XtAppInitialize (&appContext, "SAMPLE-SM", options, 
				XtNumber(options), &argc, argv, NULL, NULL, 0);
    if (argc > 1) Syntax(progName);
    XtGetApplicationResources(topLevel, (XtPointer) &app_resources,
			      resources, XtNumber(resources), NULL, 0);
    
    /*
     * Set my own IO error handler.
     */

    IceSetIOErrorHandler (myIOErrorHandler);

    /*
     * Init SM lib
     */

    /* if these are errors they should write to stderr or an error file. */
    if (!SmsInitialize ("SAMPLE-SM", "1.0",
	NewClientProc, NULL,
	HostBasedProc, 256, errormsg))
    {
	printf ("%s\n", errormsg);
	exit (1);
    }

    if (!IceListenForConnections (&count, &listenObjs,
	256, errormsg))
    {
	printf ("%s\n", errormsg);
	exit (1);
    }

    if (!set_auth (count, listenObjs))
    {
	printf ("Could not set authorization\n");
	exit (1);
    }

    XtInitializeICE (appContext);

    mainWindow = XtCreateManagedWidget (
	"mainWindow", boxWidgetClass, topLevel, NULL, 0);

    listButton = XtVaCreateManagedWidget (
	"listButton", commandWidgetClass, mainWindow,
	XtNlabel, "List all registered clients",
	NULL);

    XtAddCallback (listButton, XtNcallback, ListClientsXtProc, 0);

    saveButton = XtVaCreateManagedWidget (
	"saveButton", commandWidgetClass, mainWindow,
	XtNlabel, "Save Yourself with option to Shutdown",
	NULL);

    XtAddCallback (saveButton, XtNcallback, SaveYourselfXtProc, 0);

    propButton = XtVaCreateManagedWidget (
	"propButton", commandWidgetClass, mainWindow,
	XtNlabel, "List properties of each client",
	NULL);

    XtAddCallback (propButton, XtNcallback, ListPropXtProc, 0);

    pingButton = XtVaCreateManagedWidget (
	"pingButton", commandWidgetClass, mainWindow,
	XtNlabel, "Ping all clients",
	NULL);

    XtAddCallback (pingButton, XtNcallback, PingXtProc, 0);

    startButton = XtVaCreateManagedWidget (
	"startButton", commandWidgetClass, mainWindow,
	XtNlabel, "Start a new xsmclient",
	NULL);

    XtAddCallback (startButton, XtNcallback, StartXtProc, 0);


    /*
     * Pop up for Save Yourself button.
     */

    savePopup = XtVaCreatePopupShell (
	"savePopup", transientShellWidgetClass, topLevel,
	XtNtitle, "Save Yourself Parameters",
	NULL);
    
    saveForm = XtCreateManagedWidget (
	"saveForm", formWidgetClass, savePopup, NULL, 0);

    saveTypeLabel = XtVaCreateManagedWidget (
	"saveTypeLabel", labelWidgetClass, saveForm,
	XtNlabel, "Save Type     ",
        XtNfromHoriz, NULL,
        XtNfromVert, NULL,
        XtNborderWidth, 0,
	NULL);

    saveTypeGlobal = AddToggle (
	"saveTypeGlobal", 			/* widgetName */
	saveForm,				/* parent */
        "Global",				/* label */
	0,					/* state */
        NULL,					/* radioGroup */
        (XtPointer) &saveTypeData[0],		/* radioData */
        saveTypeLabel,				/* fromHoriz */
        NULL					/* fromVert */
    );

    saveTypeLocal = AddToggle (
	"saveTypeLocal", 			/* widgetName */
	saveForm,				/* parent */
        "Local",				/* label */
	0,					/* state */
        saveTypeGlobal,				/* radioGroup */
        (XtPointer) &saveTypeData[1],		/* radioData */
        saveTypeGlobal,				/* fromHoriz */
        NULL					/* fromVert */
    );

    saveTypeBoth = AddToggle (
	"saveTypeBoth", 			/* widgetName */
	saveForm,				/* parent */
        "Both",					/* label */
	1,					/* state */
        saveTypeGlobal,				/* radioGroup */
        (XtPointer) &saveTypeData[2],		/* radioData */
        saveTypeLocal,				/* fromHoriz */
        NULL					/* fromVert */
    );


    shutdownLabel = XtVaCreateManagedWidget (
	"shutdownLabel", labelWidgetClass, saveForm,
	XtNlabel, "Shutdown?",
        XtNfromHoriz, NULL,
        XtNfromVert, saveTypeLabel,
        XtNborderWidth, 0,
	NULL);

    shutdownYes = AddToggle (
	"shutdownYes", 				/* widgetName */
	saveForm,				/* parent */
        "Yes",					/* label */
	0,					/* state */
        NULL,					/* radioGroup */
        (XtPointer) &shutdownData[0],		/* radioData */
        saveTypeLabel,				/* fromHoriz */
        saveTypeLabel				/* fromVert */
    );

    shutdownNo = AddToggle (
	"shutdownNo", 				/* widgetName */
	saveForm,				/* parent */
        "No",					/* label */
	1,					/* state */
        shutdownYes,				/* radioGroup */
        (XtPointer) &shutdownData[1],		/* radioData */
        shutdownYes,				/* fromHoriz */
        saveTypeLabel				/* fromVert */
    );


    interactStyleLabel = XtVaCreateManagedWidget (
	"interactStyleLabel", labelWidgetClass, saveForm,
	XtNlabel, "Interact Style",
        XtNfromHoriz, NULL,
        XtNfromVert, shutdownLabel,
        XtNborderWidth, 0,
	NULL);

    interactStyleNone = AddToggle (
	"interactStyleNone", 			/* widgetName */
	saveForm,				/* parent */
        "None",					/* label */
	0,					/* state */
        NULL,					/* radioGroup */
        (XtPointer) &interactStyleData[0],	/* radioData */
        saveTypeLabel,				/* fromHoriz */
        shutdownLabel				/* fromVert */
    );

    interactStyleErrors = AddToggle (
	"interactStyleErrors", 			/* widgetName */
	saveForm,				/* parent */
        "Errors",				/* label */
	0,					/* state */
        interactStyleNone,			/* radioGroup */
        (XtPointer) &interactStyleData[1],	/* radioData */
        interactStyleNone,			/* fromHoriz */
        shutdownLabel				/* fromVert */
    );

    interactStyleAny = AddToggle (
	"interactStyleAny", 			/* widgetName */
	saveForm,				/* parent */
        "Any",					/* label */
	1,					/* state */
        interactStyleNone,			/* radioGroup */
        (XtPointer) &interactStyleData[2],	/* radioData */
        interactStyleErrors,			/* fromHoriz */
        shutdownLabel				/* fromVert */
    );


    fastLabel = XtVaCreateManagedWidget (
	"fastLabel", labelWidgetClass, saveForm,
	XtNlabel, "Fast?",
        XtNfromHoriz, NULL,
        XtNfromVert, interactStyleLabel,
        XtNborderWidth, 0,
	NULL);

    fastYes = AddToggle (
	"fastYes", 				/* widgetName */
	saveForm,				/* parent */
        "Yes",					/* label */
	0,					/* state */
        NULL,					/* radioGroup */
        (XtPointer) &fastData[0],		/* radioData */
        saveTypeLabel,				/* fromHoriz */
        interactStyleLabel			/* fromVert */
    );

    fastNo = AddToggle (
	"fastNo", 				/* widgetName */
	saveForm,				/* parent */
        "No",					/* label */
	1,					/* state */
        fastYes,				/* radioGroup */
        (XtPointer) &fastData[1],		/* radioData */
        fastYes,				/* fromHoriz */
        interactStyleLabel			/* fromVert */
    );


    saveOkButton = XtVaCreateManagedWidget (
	"saveOkButton",	commandWidgetClass, saveForm,
	XtNlabel, "OK",
        XtNfromHoriz, NULL,
        XtNfromVert, fastLabel,
        XtNvertDistance, 30,
        NULL);
    
    XtAddCallback (saveOkButton, XtNcallback, SaveOkXtProc, 0);

    saveCancelButton = XtVaCreateManagedWidget (
	"saveCancelButton", commandWidgetClass, saveForm,
        XtNlabel, "Cancel",
        XtNfromHoriz, saveOkButton,
        XtNfromVert, fastLabel,
        XtNvertDistance, 30,
        NULL);

    XtAddCallback (saveCancelButton, XtNcallback, SaveCancelXtProc, 0);


    /*
     * Pop up Shutdown confirmation.
     */

    shutdownPopup = XtVaCreatePopupShell ("shutdownPopup",
	transientShellWidgetClass, topLevel,
	XtNtitle, "Shutdown Prompt", NULL);
    
    shutdownDialog = XtVaCreateManagedWidget ("shutdownDialog",
	dialogWidgetClass, shutdownPopup,
	XtNlabel, "There are no active clients.  OK to shutdown?",
        NULL);					      

    shutdownOkButton = XtVaCreateManagedWidget ("shutdownOkButton",
	commandWidgetClass, shutdownDialog, XtNlabel, "OK", NULL);
    
    XtAddCallback (shutdownOkButton, XtNcallback, ShutdownOkXtProc, 0);

    shutdownCancelButton = XtVaCreateManagedWidget ("shutdownCancelButton",
	commandWidgetClass, shutdownDialog, XtNlabel, "Cancel", NULL);

    XtAddCallback (shutdownCancelButton, XtNcallback, ShutdownCancelXtProc, 0);


    /*
     * Realize top level.
     */

    XtRealizeWidget (topLevel);
    
    for (i = 0; i < count; i++)
    {
	XtAppAddInput (appContext,
	    IceGetListenDescrip (listenObjs[i]),
	    (XtPointer) XtInputReadMask,
	    newConnectionXtProc, (XtPointer) listenObjs[i]);
    }

    /* the sizeof includes the \0, so we don't need to count the '=' */
    networkIds = IceComposeNetworkIdList (count, listenObjs);
    p = malloc((sizeof environment_name) + strlen(networkIds) + 1);
    if(!p) nomem();
    sprintf(p, "%s=%s", environment_name, networkIds);
    putenv(p);

    if (app_resources.verbose)
	printf ("setenv %s %s\n", environment_name, networkIds);

    read_save();
    restart_everything();

    if (app_resources.verbose)
	printf ("Waiting for connections...\n");

    free (networkIds);
    setjmp (JumpHere);
    XtAppMainLoop (appContext);
}
