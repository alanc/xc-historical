/* $XConsortium: xsm.c,v 1.44 94/07/07 16:46:42 mor Exp $ */
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
#include "choose.h"
#include "mainwin.h"
#include "info.h"
#include "save.h"
#include "name.h"

#include <signal.h>

#define DEFAULT_SESSION_NAME "Default"

AppResources app_resources;

#define Offset(field) XtOffsetOf(struct _AppResources, field)
static XtResource resources [] = {
    {"verbose", "Verbose",  XtRBoolean, sizeof (Boolean), 
	 Offset (verbose), XtRImmediate, (XtPointer) False}
};
#undef Offset

static XrmOptionDescRec options[] = {
    {"-verbose",    "*verbose",	    XrmoptionNoArg,     (XPointer) "TRUE"}
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

char		*session_name = NULL;

IceAuthDataEntry *authDataEntries = NULL;
int		numTransports = 0;

Bool		client_info_visible = 0;

String 		*clientNames = NULL;
int		numClientNames = 0;

XtAppContext	appContext;

Widget		topLevel;

Widget		    chooseSessionPopup;

Widget		    	chooseSessionForm;

Widget			    chooseSessionLabel;

Widget			    chooseSessionListWidget;

Widget			    chooseSessionOkButton;

Widget			    chooseSessionCancelButton;

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

Widget		    nameSessionPopup;

Widget			nameSessionDialog;

Widget			    nameSessionOkButton;
Widget			    nameSessionCancelButton;


XtActionsRec xsm_actions[] = {
	"XsmNameSessionOk",	XsmNameSessionOk
};

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

    XtPopdown (savePopup);
    XtSetSensitive (savePopup, 1);
    XtSetSensitive (mainWindow, 1);
    XtSetSensitive (clientInfoPopup, 1);
    XtSetSensitive (clientPropPopup, 1);

    longjmp (JumpHere, 1);
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



void
start_session (name)

char *name;

{
    int database_read;


    /*
     * Read the session save file.  Make sure the session manager
     * has an SM_CLIENT_ID, so that other managers (like the WM) can
     * identify it.
     */

    database_read = read_save (name, &sm_id);
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


    if (app_resources.verbose)
	printf ("Waiting for connections...\n");
}



main(argc, argv)
    int  argc;
    char **argv;
{
    IceListenObj *listenObjs;
    char 	*networkIds;
    char	*p;
    char	*progName;
    char	title[256];
    char 	errormsg[256];
    static	char environment_name[] = "SESSION_MANAGER";
    Dimension   width, height;
    Position	x, y;
    int		sessionNameCount, i;
    String	*sessionNames;

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
    
    XtAppAddActions (appContext,
	xsm_actions, XtNumber (xsm_actions));


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

    if (!SmsInitialize ("SAMPLE-SM", "1.0",
	NewClientProc, NULL,
	HostBasedProc, 256, errormsg))
    {
	fprintf (stderr, "%s\n", errormsg);
	exit (1);
    }

    if (!IceListenForConnections (&numTransports, &listenObjs,
	256, errormsg))
    {
	fprintf (stderr, "%s\n", errormsg);
	exit (1);
    }

    if (!set_auth (numTransports, listenObjs, &authDataEntries))
    {
	fprintf (stderr, "Could not set authorization\n");
	exit (1);
    }

    InitWatchProcs (appContext);

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

    free (networkIds);

    create_choose_session_popup ();
    create_main_window ();
    create_client_info_popup ();
    create_save_popup ();
    create_name_session_popup ();


    /*
     * Get list of session names
     */

    if (!GetSessionNames(&sessionNameCount, &sessionNames))
    {
	session_name = XtNewString (DEFAULT_SESSION_NAME);

	sprintf (title, "xsm: %s", session_name);

	XtVaSetValues (topLevel,
	    XtNtitle, title,		/* session name */
	    NULL);

	XtRealizeWidget (topLevel);

    	start_session (session_name);
    }
    else
    {
	/*
	 * Add the session names to the list
	 */

	AddSessionNames (sessionNameCount, sessionNames);


	/*
	 * Center popup containing choice of sessions
	 */

	XtRealizeWidget (chooseSessionPopup);

	XtVaGetValues (chooseSessionPopup,
	    XtNwidth, &width,
	    XtNheight, &height,
	    NULL);

	x = (Position)(WidthOfScreen (XtScreen (topLevel)) - width) / 2;
	y = (Position)(HeightOfScreen (XtScreen (topLevel)) - height) / 3;

	XtVaSetValues (chooseSessionPopup,
	    XtNx, x,
	    XtNy, y,
	    NULL);

	XtPopup (chooseSessionPopup, XtGrabNone);
    }


    /*
     * Main loop
     */

    setjmp (JumpHere);
    XtAppMainLoop (appContext);
}
