/* $XConsortium: xsm.c,v 1.41 94/06/30 12:06:46 mor Exp $ */
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

/*
 * X Session Manager.
 *
 * Authors:
 *	Ralph Mor, X Consortium
 *      Jordan Brown, Quarterdeck Office Systems
 */

#include "xsm.h"
#include <X11/Xatom.h>
#include <signal.h>

#define CURRENT_SESSION_NAME "Current"

AppResources app_resources;

#define Offset(field) XtOffsetOf(struct _AppResources, field)
static XtResource resources [] = {
    {"verbose", "Verbose",  XtRBoolean, sizeof (Boolean), 
	 Offset (verbose), XtRImmediate, (XtPointer) False},
    {"name", "Name", XtRString, sizeof (XtRString), 
	 Offset (name), XtRString, (XtPointer) CURRENT_SESSION_NAME}
};
#undef Offset

static XrmOptionDescRec options[] = {
    {"-verbose",    "*verbose",	    XrmoptionNoArg,     (XPointer) "TRUE"},
    {"-name",	    "*name",	    XrmoptionSepArg,    (XPointer) NULL},
};

List		*PendingList;
ClientRec	*ClientList = NULL;
int		numClients = 0;
int		saveDoneCount = 0;
int		interactCount = 0;
Bool		shutdownDialogUp = False;
Bool		shutdownInProgress = False;
Bool		shutdownCancelled = False;
jmp_buf		JumpHere;

char		*sm_id;

char		*session_name;

IceAuthDataEntry *authDataEntries = NULL;
int		numTransports = 0;

int		saveTypeData[] = {SmSaveGlobal,
			  	  SmSaveLocal,
		  		  SmSaveBoth};

int		interactStyleData[] = {SmInteractStyleNone,
			       	       SmInteractStyleErrors,
		       		       SmInteractStyleAny};

Bool		client_info_visible = 0;

String 		*clientNames = NULL;
int		numClientNames = 0;

XtAppContext	appContext;

Widget		topLevel;

Widget		    mainWindow;

Widget		        clientInfoButton;
Widget			checkPointButton;
Widget			shutdownButton;
Widget			nameSessionButton;

Widget		    clientInfoPopup;

Widget		    	clientInfoForm;

Widget			    viewPropButton;
Widget			    killClientButton;
Widget			    clientInfoDoneButton;
Widget			    clientListWidget;

Widget		    clientPropPopup;

Widget		    	clientPropForm;

Widget			    clientPropDoneButton;
Widget			    clientPropTextWidget;

Widget		    savePopup;

Widget			saveForm;

Widget			    saveTypeLabel;
Widget			    saveTypeGlobal;
Widget			    saveTypeLocal;
Widget			    saveTypeBoth;

Widget			    interactStyleLabel;
Widget			    interactStyleNone;
Widget			    interactStyleErrors;
Widget			    interactStyleAny;

Widget			    saveOkButton;
Widget			    saveCancelButton;

Widget		    shutdownPopup;

Widget			shutdownDialog;

Widget			    shutdownOkButton;
Widget			    shutdownCancelButton;


void FreeClientInfo ();

extern Status InitWatchProcs ();
extern void start_default_apps ();
extern void restart_everything ();
extern int read_save ();
extern void write_save ();
extern Bool HostBasedProc ();
extern Status set_auth ();
extern void free_auth ();



print_prop(prop)
SmProp	*prop;
{
    int j;

    printf ("   Name:	%s\n", prop->name);
    printf ("   Type:	%s\n", prop->type);
    printf ("   Num values:	%d\n", prop->num_vals);
    if (strcmp(prop->type, SmCARD8) == 0) {
	char *card8 = prop->vals->value;
	int value = *card8;
	printf ("   Value 1:\t%d\n", value);
    } else {
	for (j = 0; j < prop->num_vals; j++) {
	    printf ("   Value %d:	%s\n", j + 1,
		    (char *) prop->vals[j].value);
	}
    }
    printf ("\n");
}



exit_sm ()
{
    if (app_resources.verbose)
	printf ("\nSESSION MANAGER GOING AWAY!\n");
    free_auth (numTransports, authDataEntries);
    exit (0);
}



void
UpdateClientList ()

{
    ClientRec *client = ClientList;
    char *progName;
    int i, j, k;

    if (clientNames)
    {
	/*
	 * Free the previous list of names.  Xaw doesn't make a copy of
	 * our list, so we need to keep it around.
	 */

	for (i = 0; i < numClientNames; i++)
	    XtFree (clientNames[i]);
	free ((char *) clientNames);
    }

    clientNames = (String *) malloc (numClients * sizeof (String));
    numClientNames = numClients;

    for (i = 0; i < numClients; i++, client = client->next)
    {
	progName = NULL;

	for (j = 0; j < client->numProps && !progName; j++)
	    if (strcmp (client->props[j]->name, SmProgram) == 0)
	    {
		char *temp = client->props[j]->vals[0].value;
		char *lastSlash = NULL;

		for (k = 0; k < strlen (temp); k++)
		    if (temp[k] == '/')
			lastSlash = &temp[k];

		if (lastSlash)
		    temp = lastSlash + 1;

		progName = (String) XtMalloc (
		    strlen (client->clientHostname) + strlen (temp) + 4);
		sprintf (progName, "%s : %s", client->clientHostname, temp);
	    }

	if (!progName)
	    progName = "";
	clientNames[i] = progName;
    }

    XawListChange (clientListWidget, clientNames, numClients, 0, True);
    XawListHighlight (clientListWidget, 0);
}



SetInitialProperties(client, pendclient)
ClientRec	*client;
PendingClient	*pendclient;
{
    int			idx;
    SmProp		*prop;
    SmPropValue		*val;
    List		*pl;
    List		*vl;
    PendingProp		*pprop;
    PendingValue	*pval;

    if (app_resources.verbose)
	printf("Setting initial properties for %s\n", client->clientId);

    idx = 0;
    for(pl = ListFirst(pendclient->props); pl; pl = ListNext(pl)) {
	pprop = (PendingProp *)pl->thing;
	prop = (SmProp *)malloc(sizeof *prop);
	client->props[idx] = prop;
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
	idx++;
    }
    client->numProps = idx;

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
	id = (char *)malloc (strlen (previousId) + 1);
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
	free (previousId);
    } else {
	SmsSaveYourself(smsConn, SmSaveLocal, False, SmInteractStyleNone,
			False);
    }

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
SaveYourselfReqProc (smsConn, managerData, saveType,
    shutdown, interactStyle, fast, global)

SmsConn     smsConn;
SmPointer   managerData;
int	    saveType;
Bool	    shutdown;
int         interactStyle;
Bool        fast;
Bool        global;

{
    if (app_resources.verbose) 
	printf("SAVE YOURSELF REQUEST not supported!\n");
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
    else if (client_info_visible)
	UpdateClientList ();
}



void
SetProperty(client, prop)
ClientRec	*client;
SmProp		*prop;
{
    int	idx, j;

    for (j = 0; j < client->numProps; j++)
	if (strcmp (prop->name, client->props[j]->name) == 0)
	{
	    SmFreeProperty (client->props[j]);
	    break;
	}

    if (j < client->numProps)
	idx = j;
    else
    {
	idx = client->numProps;
	client->numProps++;

	if (client->numProps > MAX_PROPS)
	    return;
    }

    client->props[idx] = prop;
}



void
DeleteProperty(client, propname)
ClientRec	*client;
char		*propname;
{
    int	j;

    for (j = 0; j < client->numProps; j++)
	if (strcmp (propname, client->props[j]->name) == 0)
	{
	    SmFreeProperty (client->props[j]);

	    if (j < client->numProps - 1)
		client->props[j] = client->props[client->numProps - 1];

	    client->numProps--;
	    break;
	}
}



void
SetPropertiesProc (smsConn, managerData, numProps, props)

SmsConn 	smsConn;
SmPointer 	managerData;
int		numProps;
SmProp 		**props;

{
    ClientRec	*client = (ClientRec *) managerData;
    int		update, i;

    if (app_resources.verbose) {
	printf ("Client Id = %s, received SET PROPERTIES ", client->clientId);
	printf ("[Num props = %d]\n", numProps);
    }

    update = client->numProps == 0 && numProps > 0 && client_info_visible;

    for (i = 0; i < numProps; i++) {
	if(app_resources.verbose)
	    print_prop(props[i]);
	SetProperty(client, props[i]);
    }
    free ((char *) props);

    if (update)
	UpdateClientList ();
}



void
DeletePropertiesProc (smsConn, managerData, numProps, propNames)

SmsConn 	smsConn;
SmPointer 	managerData;
int		numProps;
char **		propNames;

{
    ClientRec	*client = (ClientRec *) managerData;
    int		i;

    if (app_resources.verbose) {
	printf ("Client Id = %s, received DELETE PROPERTIES ",
	    client->clientId);
	printf ("[Num props = %d]\n", numProps);
    }

    for (i = 0; i < numProps; i++) {
	if(app_resources.verbose)
	    printf ("   Name:	%s\n", propNames[i]);
	DeleteProperty(client, propNames[i]);
	free (propNames[i]);
    }
    free ((char *) propNames);
}



void
GetPropertiesProc (smsConn, managerData)

SmsConn 	smsConn;
SmPointer 	managerData;

{
    ClientRec	*client = (ClientRec *) managerData;
    int		i;

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



Status
NewClientProc (smsConn, managerData, maskRet, callbacksRet, failureReasonRet)

SmsConn		smsConn;
SmPointer  	managerData;
unsigned long	*maskRet;
SmsCallbacks	*callbacksRet;
char 		**failureReasonRet;

{
    ClientRec *newClient = (ClientRec *) malloc (sizeof (ClientRec));
    *maskRet = 0;

    if (!newClient)
    {
	char *str = "Memory allocation failed";

	if ((*failureReasonRet = (char *) malloc (strlen (str) + 1)) != NULL)
	    strcpy (*failureReasonRet, str);

	return (0);
    }

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

    *maskRet |= SmsRegisterClientProcMask;
    callbacksRet->register_client.callback 	= RegisterClientProc;
    callbacksRet->register_client.manager_data  = (SmPointer) newClient;

    *maskRet |= SmsInteractRequestProcMask;
    callbacksRet->interact_request.callback 	= InteractRequestProc;
    callbacksRet->interact_request.manager_data = (SmPointer) newClient;

    *maskRet |= SmsInteractDoneProcMask;
    callbacksRet->interact_done.callback	= InteractDoneProc;
    callbacksRet->interact_done.manager_data    = (SmPointer) newClient;

    *maskRet |= SmsSaveYourselfRequestProcMask;
    callbacksRet->save_yourself_request.callback     = SaveYourselfReqProc;
    callbacksRet->save_yourself_request.manager_data = (SmPointer) newClient;

    *maskRet |= SmsSaveYourselfDoneProcMask;
    callbacksRet->save_yourself_done.callback 	   = SaveYourselfDoneProc;
    callbacksRet->save_yourself_done.manager_data  = (SmPointer) newClient;

    *maskRet |= SmsCloseConnectionProcMask;
    callbacksRet->close_connection.callback 	 = CloseConnectionProc;
    callbacksRet->close_connection.manager_data  = (SmPointer) newClient;

    *maskRet |= SmsSetPropertiesProcMask;
    callbacksRet->set_properties.callback 	= SetPropertiesProc;
    callbacksRet->set_properties.manager_data   = (SmPointer) newClient;

    *maskRet |= SmsDeletePropertiesProcMask;
    callbacksRet->delete_properties.callback	= DeletePropertiesProc;
    callbacksRet->delete_properties.manager_data   = (SmPointer) newClient;

    *maskRet |= SmsGetPropertiesProcMask;
    callbacksRet->get_properties.callback	= GetPropertiesProc;
    callbacksRet->get_properties.manager_data   = (SmPointer) newClient;

    return (1);
}



/*
 * Xt callbacks
 */

void
ClientInfoXtProc (w, client_data, callData)
    Widget	w;
    XtPointer 	client_data;
    XtPointer 	callData;

{
    static Bool did_first_popup = 0;

    if (!client_info_visible)
    {
	UpdateClientList ();

	if (!did_first_popup)
	{
	    Position x, y, rootx, rooty;

	    XtVaGetValues (mainWindow, XtNx, &x, XtNy, &y, NULL);
	    XtTranslateCoords (mainWindow, x, y, &rootx, &rooty);
	    XtMoveWidget (clientInfoPopup, rootx + 100, rooty + 50);
	    did_first_popup = 1;
	}

	XtPopup (clientInfoPopup, XtGrabNone);

	client_info_visible = 1;
    }
}



void
GetClientInfoXtProc (w, client_data, callData)

Widget		w;
XtPointer 	client_data;
XtPointer 	callData;

{
    ClientRec *client = ClientList;
    XawListReturnStruct *current;
    int client_index, i, j;
    static Bool did_first_popup = 0;

    current = XawListShowCurrent (clientListWidget);
    client_index = current->list_index;

    while (client_index > 0)
    {
	client = client->next;
	client_index--;
    }

    if (client->numProps > 0)
    {
	char buffer[1024];
	char number[10];

	buffer[0] = '\0';

	for (i = 0; i < client->numProps; i++)
	{
	    SmProp *prop = client->props[i];

	    strcat (buffer, "Name:		");
	    strcat (buffer, prop->name);
	    strcat (buffer, "\n");
	    strcat (buffer, "Type:		");
	    strcat (buffer, prop->type);
	    strcat (buffer, "\n");
	    strcat (buffer, "Num values:	");
	    sprintf (number, "%d", prop->num_vals);
	    strcat (buffer, number);
	    strcat (buffer, "\n");

	    if (strcmp (prop->type, SmCARD8) == 0)
	    {
		char *card8 = prop->vals->value;
		int value = *card8;
		strcat (buffer, "Value 1:	");
		sprintf (number, "%d", value);
		strcat (buffer, number);
		strcat (buffer, "\n");
	    }
	    else
	    {
		for (j = 0; j < prop->num_vals; j++)
		{
		    strcat (buffer, "Value ");
		    sprintf (number, "%d", j + 1);
		    strcat (buffer, number);
		    strcat (buffer, ":	");
		    strcat (buffer, (char *) prop->vals[j].value);
		    strcat (buffer, "\n");
		}
	    }

	    if (i < client->numProps - 1)
		strcat (buffer, "\n");
	}

	XtVaSetValues (clientPropTextWidget,
	    XtNstring, buffer,
	    NULL);

	sprintf (buffer, "SM Properties : %s", current->string);

	XtVaSetValues (clientPropPopup,
	    XtNtitle, buffer,
	    NULL);

	if (!did_first_popup)
	{
	    Position x, y, rootx, rooty;

	    XtVaGetValues (mainWindow, XtNx, &x, XtNy, &y, NULL);
	    XtTranslateCoords (mainWindow, x, y, &rootx, &rooty);
	    XtMoveWidget (clientPropPopup, rootx + 50, rooty + 150);
	    did_first_popup = 1;
	}

	XtPopup (clientPropPopup, XtGrabNone);
    }

    XtFree ((char *) current);
}



void
KillClientXtProc (w, client_data, callData)

Widget		w;
XtPointer 	client_data;
XtPointer 	callData;

{
    ClientRec *client = ClientList;
    XawListReturnStruct *current;
    int client_index, i, j;

    current = XawListShowCurrent (clientListWidget);
    client_index = current->list_index;

    while (client_index > 0)
    {
	client = client->next;
	client_index--;
    }

    SmsDie (client->smsConn);
}



void
listDoneXtProc (w, client_data, callData)

Widget		w;
XtPointer 	client_data;
XtPointer 	callData;

{
    XtPopdown (clientInfoPopup);
    client_info_visible = 0;
}



void
clientPropDoneXtProc (w, client_data, callData)

Widget		w;
XtPointer 	client_data;
XtPointer 	callData;

{
    XtPopdown (clientPropPopup);
}



void
PopupSaveDialog ()

{
    static Bool did_first_popup = 0;

    if (!did_first_popup)
    {
	Position x, y, rootx, rooty;

	XtVaGetValues (mainWindow, XtNx, &x, XtNy, &y, NULL);
	XtTranslateCoords (mainWindow, x, y, &rootx, &rooty);

	if (ClientList == NULL)
	    XtMoveWidget (shutdownPopup, rootx + 25, rooty + 100);
	else
	    XtMoveWidget (savePopup, rootx + 25, rooty + 100);

	did_first_popup = 1;
    }

    if (ClientList == NULL)
    {
	XtPopup (shutdownPopup, XtGrabNone);
    }
    else
    {
	XawToggleSetCurrent (saveTypeBoth,
	    (XtPointer) &saveTypeData[2]);
	XawToggleSetCurrent (interactStyleAny,
	    (XtPointer) &interactStyleData[2]);

	XtVaSetValues (savePopup,
	    XtNtitle, shutdownDialogUp ? "Shutdown" : "Checkpoint",
	    NULL);

	XtPopup (savePopup, XtGrabNone);
    }

    XtSetSensitive (mainWindow, 0);
    XtSetSensitive (clientInfoPopup, 0);
    XtSetSensitive (clientPropPopup, 0);
}



void
CheckPointXtProc (w, client_data, callData)

Widget		w;
XtPointer 	client_data;
XtPointer 	callData;

{
    shutdownDialogUp = False;
    PopupSaveDialog ();
}




void
ShutdownXtProc (w, client_data, callData)

Widget		w;
XtPointer 	client_data;
XtPointer 	callData;

{
    shutdownDialogUp = True;
    PopupSaveDialog ();
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
    int		interactStyle;
    Bool	fast = False;
    char	*_saveType;
    char	*_shutdown;
    char	*_interactStyle;

    ptr = XawToggleGetCurrent (saveTypeGlobal /* just 1 of the group */);
    saveType = *((int *) ptr);

    ptr = XawToggleGetCurrent (interactStyleNone /* just 1 of the group */);
    interactStyle = *((int *) ptr);

    if (saveType == SmSaveGlobal)
	_saveType = "Global";
    else if (saveType == SmSaveLocal)
	_saveType = "Local";
    else
	_saveType = "Both";

    if (shutdownDialogUp)
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

    for (client = ClientList; client; client = client->next)
    {
	SmsSaveYourself (client->smsConn,
	    saveType, shutdownDialogUp, interactStyle, fast);

	if (app_resources.verbose) {
	    printf ("Client Id = %s, sent SAVE YOURSELF [", client->clientId);
	    printf ("Save Type = %s, Shutdown = %s, ", _saveType, _shutdown);
	    printf ("Interact Style = %s, Fast = False]\n", _interactStyle);
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

    write_save(sm_id);

    if (shutdownDialogUp && shutdownCancelled) {
	shutdownCancelled = False;
    } else if (shutdownDialogUp) {
	shutdownInProgress = True;
	client = ClientList;
	while (client) {
	    SmsDie (client->smsConn);
	    if (app_resources.verbose)
		printf ("Client Id = %s, sent DIE\n", client->clientId);
	    client = client->next;
	}
    } else {
	client = ClientList;
	while (client) {
	    SmsSaveComplete (client->smsConn);
	    if (app_resources.verbose)
		printf ("Client Id = %s, sent SAVE COMPLETE\n",
		    client->clientId);
	    client = client->next;
	}
    }

    if (!shutdownInProgress) {
	XtPopdown (savePopup);
	XtSetSensitive (savePopup, 1);
	XtSetSensitive (mainWindow, 1);
	XtSetSensitive (clientInfoPopup, 1);
	XtSetSensitive (clientPropPopup, 1);
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
    XtSetSensitive (clientInfoPopup, 1);
    XtSetSensitive (clientPropPopup, 1);
}



void
ShutdownOkXtProc (w, client_data, callData)

Widget		w;
XtPointer 	client_data;
XtPointer 	callData;

{
    write_save(sm_id);
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
    XtSetSensitive (clientInfoPopup, 1);
    XtSetSensitive (clientPropPopup, 1);
}



void
NameSessionXtProc (w, client_data, callData)

Widget		w;
XtPointer 	client_data;
XtPointer 	callData;

{
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
    IceAcceptStatus status;

    ice_conn = IceAcceptConnection((IceListenObj) client_data, &status);
    if (! ice_conn) {
	if (app_resources.verbose)
	    printf ("IceAcceptConnection failed\n");
    } else {
	IceConnectStatus cstatus;

	while ((cstatus = IceConnectionStatus (ice_conn))==IceConnectPending) {
	    XtAppProcessEvent (appContext, XtIMAll);
	}

	if (cstatus == IceConnectAccepted) {
	    if (app_resources.verbose) {
		printf ("ICE Connection opened by client, IceConn fd = %d, ",
			IceConnectionNumber (ice_conn));
		connstr = IceConnectionString (ice_conn);
		printf ("Accept at networkId %s\n", connstr);
		free (connstr);
		printf ("\n");
	    }
	} else {
	    if (app_resources.verbose)
	    {
		if (cstatus == IceConnectIOError)
		    printf ("IO error opening ICE Connection!\n");
		else
		    printf ("ICE Connection rejected!\n");
	    }

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

	IceSetShutdownNegotiation (ice_conn, False);
	IceCloseConnection (ice_conn);
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
	    
	    if (app_resources.verbose)
	    {
		printf ("ICE Connection terminated (fd = %d)\n",
			IceConnectionNumber (ice_conn));
		printf ("\n");
	    }

	    IceSetShutdownNegotiation (ice_conn, False);
	    IceCloseConnection (ice_conn);

	    numClients--;

	    if (shutdownInProgress && numClients == 0)
	    {
		exit_sm ();
	    }
	    else if (client_info_visible)
		UpdateClientList ();
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
    XtSetSensitive (clientInfoPopup, 1);
    XtSetSensitive (clientPropPopup, 1);

    longjmp (JumpHere, 1);
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
    Widget 		toggle;
    XtTranslations	translations;

    toggle = XtVaCreateManagedWidget (
	widgetName, toggleWidgetClass, parent,
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


static void Syntax(call)
    char *call;
{
    (void) fprintf(stderr, "usage: %s [-verbose]\n", call);
    exit(2);
}



main(argc, argv)
    int  argc;
    char **argv;
{
    IceListenObj *listenObjs;
    char 	*networkIds;
    int  	database_read, i;
    char	*p;
    char	*progName;
    char	title[256];
    char 	errormsg[256];
    static	char environment_name[] = "SESSION_MANAGER";

    umask (0077);	/* disallow non-owner access */

    p = strrchr(argv[0], '/');
    progName = (p ? p + 1 : argv[0]);
    topLevel = XtVaAppInitialize (&appContext, "XSm", options, 
	XtNumber(options), &argc, argv, NULL,
        XtNmappedWhenManaged, False,
	NULL);
	
    if (argc > 1) Syntax(progName);
    XtGetApplicationResources(topLevel, (XtPointer) &app_resources,
			      resources, XtNumber(resources), NULL, 0);
    
    sprintf (title, "xsm: %s", app_resources.name);

    XtVaSetValues (topLevel,
	XtNtitle, title,		/* session name */
	NULL);


    /*
     * Set my own IO error handler.
     */

    IceSetIOErrorHandler (myIOErrorHandler);


    /*
     * Ignore SIGPIPE
     */

    signal (SIGPIPE, SIG_IGN);


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

    if (!IceListenForConnections (&numTransports, &listenObjs,
	256, errormsg))
    {
	printf ("%s\n", errormsg);
	exit (1);
    }

    if (!set_auth (numTransports, listenObjs, &authDataEntries))
    {
	printf ("Could not set authorization\n");
	exit (1);
    }

    InitWatchProcs (appContext);

    mainWindow = XtVaCreateManagedWidget (
	"mainWindow", formWidgetClass, topLevel,
	NULL);

    clientInfoButton = XtVaCreateManagedWidget (
	"clientInfoButton", commandWidgetClass, mainWindow,
        XtNfromHoriz, NULL,
        XtNfromVert, NULL,
	NULL);

    XtAddCallback (clientInfoButton, XtNcallback, ClientInfoXtProc, 0);

    checkPointButton = XtVaCreateManagedWidget (
	"checkPointButton", commandWidgetClass, mainWindow,
        XtNfromHoriz, NULL,
        XtNfromVert, clientInfoButton,
	NULL);

    XtAddCallback (checkPointButton, XtNcallback, CheckPointXtProc, 0);

    shutdownButton = XtVaCreateManagedWidget (
	"shutdownButton", commandWidgetClass, mainWindow,
        XtNfromHoriz, checkPointButton,
        XtNfromVert, clientInfoButton,
	NULL);

    XtAddCallback (shutdownButton, XtNcallback, ShutdownXtProc, 0);

    nameSessionButton = XtVaCreateManagedWidget (
	"nameSessionButton", commandWidgetClass, mainWindow,
        XtNfromHoriz, NULL,
        XtNfromVert, shutdownButton,
	NULL);

    XtAddCallback (nameSessionButton, XtNcallback, NameSessionXtProc, 0);


    /*
     * Pop up for List Clients button.
     */

    clientInfoPopup = XtVaCreatePopupShell (
	"clientInfoPopup", transientShellWidgetClass, topLevel,
	XtNallowShellResize, True,
	NULL);
    

    clientInfoForm = XtVaCreateManagedWidget (
	"clientInfoForm", formWidgetClass, clientInfoPopup,
	NULL);

    viewPropButton = XtVaCreateManagedWidget (
	"viewPropButton", commandWidgetClass, clientInfoForm,
        XtNfromHoriz, NULL,
        XtNfromVert, NULL,
        NULL);
    
    XtAddCallback (viewPropButton, XtNcallback, GetClientInfoXtProc, 0);


    killClientButton = XtVaCreateManagedWidget (
	"killClientButton", commandWidgetClass, clientInfoForm,
        XtNfromHoriz, viewPropButton,
        XtNfromVert, NULL,
        NULL);
    
    XtAddCallback (killClientButton, XtNcallback, KillClientXtProc, 0);


    clientInfoDoneButton = XtVaCreateManagedWidget (
	"clientInfoDoneButton", commandWidgetClass, clientInfoForm,
        XtNfromHoriz, killClientButton,
        XtNfromVert, NULL,
        NULL);

    XtAddCallback (clientInfoDoneButton, XtNcallback, listDoneXtProc, 0);


    clientListWidget = XtVaCreateManagedWidget (
	"clientListWidget", listWidgetClass, clientInfoForm,
	XtNresizable, True,
        XtNdefaultColumns, 1,
	XtNforceColumns, True,
        XtNfromHoriz, NULL,
        XtNfromVert, viewPropButton,
	NULL);



    /*
     * Pop up for viewing client properties
     */

    clientPropPopup = XtVaCreatePopupShell (
	"clientPropPopup", transientShellWidgetClass, topLevel,
	XtNallowShellResize, True,
	NULL);
    

    clientPropForm = XtVaCreateManagedWidget (
	"clientPropForm", formWidgetClass, clientPropPopup,
	XtNresizable, True,
	NULL);

    clientPropDoneButton = XtVaCreateManagedWidget (
	"clientPropDoneButton", commandWidgetClass, clientPropForm,
        XtNfromHoriz, NULL,
        XtNfromVert, NULL,
        NULL);

    XtAddCallback (clientPropDoneButton, XtNcallback, clientPropDoneXtProc, 0);


    clientPropTextWidget = XtVaCreateManagedWidget (
	"clientPropTextWidget", asciiTextWidgetClass, clientPropForm,
        XtNfromHoriz, NULL,
        XtNfromVert, clientPropDoneButton,
	XtNeditType, XawtextRead,
        XtNdisplayCaret, False,
/*	XtNresize, XawtextResizeBoth, */
	XtNscrollVertical, XawtextScrollWhenNeeded,
	XtNscrollHorizontal, XawtextScrollWhenNeeded,
	NULL);


    /*
     * Pop up for Save Yourself button.
     */

    savePopup = XtVaCreatePopupShell (
	"savePopup", transientShellWidgetClass, topLevel,
	NULL);
    
    saveForm = XtCreateManagedWidget (
	"saveForm", formWidgetClass, savePopup, NULL, 0);

    saveTypeLabel = XtVaCreateManagedWidget (
	"saveTypeLabel", labelWidgetClass, saveForm,
        XtNfromHoriz, NULL,
        XtNfromVert, NULL,
        XtNborderWidth, 0,
	NULL);

    saveTypeGlobal = AddToggle (
	"saveTypeGlobal", 			/* widgetName */
	saveForm,				/* parent */
	0,					/* state */
        NULL,					/* radioGroup */
        (XtPointer) &saveTypeData[0],		/* radioData */
        saveTypeLabel,				/* fromHoriz */
        NULL					/* fromVert */
    );

    saveTypeLocal = AddToggle (
	"saveTypeLocal", 			/* widgetName */
	saveForm,				/* parent */
	0,					/* state */
        saveTypeGlobal,				/* radioGroup */
        (XtPointer) &saveTypeData[1],		/* radioData */
        saveTypeGlobal,				/* fromHoriz */
        NULL					/* fromVert */
    );

    saveTypeBoth = AddToggle (
	"saveTypeBoth", 			/* widgetName */
	saveForm,				/* parent */
	1,					/* state */
        saveTypeGlobal,				/* radioGroup */
        (XtPointer) &saveTypeData[2],		/* radioData */
        saveTypeLocal,				/* fromHoriz */
        NULL					/* fromVert */
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
        XtNfromHoriz, NULL,
        XtNfromVert, interactStyleLabel,
        XtNvertDistance, 30,
        NULL);
    
    XtAddCallback (saveOkButton, XtNcallback, SaveOkXtProc, 0);

    saveCancelButton = XtVaCreateManagedWidget (
	"saveCancelButton", commandWidgetClass, saveForm,
        XtNfromHoriz, saveOkButton,
        XtNfromVert, interactStyleLabel,
        XtNvertDistance, 30,
        NULL);

    XtAddCallback (saveCancelButton, XtNcallback, SaveCancelXtProc, 0);


    /*
     * Pop up Shutdown confirmation.
     */

    shutdownPopup = XtVaCreatePopupShell ("shutdownPopup",
	transientShellWidgetClass, topLevel,
	NULL);
    
    shutdownDialog = XtVaCreateManagedWidget ("shutdownDialog",
	dialogWidgetClass, shutdownPopup,
        NULL);					      

    shutdownOkButton = XtVaCreateManagedWidget ("shutdownOkButton",
	commandWidgetClass, shutdownDialog,
	NULL);
    
    XtAddCallback (shutdownOkButton, XtNcallback, ShutdownOkXtProc, 0);

    shutdownCancelButton = XtVaCreateManagedWidget ("shutdownCancelButton",
	commandWidgetClass, shutdownDialog,
	NULL);

    XtAddCallback (shutdownCancelButton, XtNcallback, ShutdownCancelXtProc, 0);


    /*
     * Realize top level.
     */

    XtRealizeWidget (topLevel);
    
    for (i = 0; i < numTransports; i++)
    {
	XtAppAddInput (appContext,
	    IceGetListenConnectionNumber (listenObjs[i]),
	    (XtPointer) XtInputReadMask,
	    newConnectionXtProc, (XtPointer) listenObjs[i]);
    }

    /* the sizeof includes the \0, so we don't need to count the '=' */
    networkIds = IceComposeNetworkIdList (numTransports, listenObjs);
    p = (char *) malloc((sizeof environment_name) + strlen(networkIds) + 1);
    if(!p) nomem();
    sprintf(p, "%s=%s", environment_name, networkIds);
    putenv(p);

    if (app_resources.verbose)
	printf ("setenv %s %s\n", environment_name, networkIds);


    /*
     * Read the session save file.  Make sure the session manager
     * has an SM_CLIENT_ID, so that other managers (like the WM) can
     * identify it.
     */

    database_read = read_save(&sm_id);
    if (!sm_id)
	sm_id = SmsGenerateClientID (NULL);
    XChangeProperty (XtDisplay (topLevel), XtWindow (topLevel),
	XInternAtom (XtDisplay (topLevel), "SM_CLIENT_ID", False),
	XA_STRING, 8, PropModeReplace,
	(unsigned char *) sm_id, strlen (sm_id));

    XtMapWidget (topLevel);


    if (!database_read)
    {
	/*
	 * Start default apps (e.g. twm, smproxy)
	 */

	start_default_apps ();
    }


    /*
     * Restart clients.
     */

    restart_everything();


    /*
     * Main loop
     */

    if (app_resources.verbose)
	printf ("Waiting for connections...\n");

    free (networkIds);
    setjmp (JumpHere);
    XtAppMainLoop (appContext);
}
