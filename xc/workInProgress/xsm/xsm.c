/* $XConsortium: xsm.c,v 1.60 94/08/10 15:32:51 mor Exp mor $ */
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
#include "globals.c"
#include "xtwatch.h"
#include "choose.h"
#include "mainwin.h"
#include "info.h"
#include "save.h"
#include "name.h"
#include "auth.h"
#include "restart.h"
#include "saveutil.h"

#include <signal.h>

Atom wmStateAtom;
List *dead_clients;


/*
 * Forward declarations
 */

void StartSession ();
void NewConnectionXtProc ();
Status NewClientProc ();
void IoErrorHandler ();



/*
 * Main program
 */

main (argc, argv)

int  argc;
char **argv;

{
    IceListenObj *listenObjs;
    char 	*networkIds;
    char	*p;
    char	*progName;
    char 	errormsg[256];
    static	char environment_name[] = "SESSION_MANAGER";
    int		success, found_command_line_name, i;

    for (i = 1; i < argc; i++)
    {
	if (argv[i][0] == '-')
	{
	    switch (argv[i][1])
	    {
	    case 'n':					/* -name */
		if (++i >= argc) goto usage;
		session_name = XtNewString (argv[i]);
		continue;

	    case 'v':					/* -verbose */
		verbose = 1;
		continue;
	    }
	}

    usage:
	fprintf (stderr, "usage: %s [-name session_name][-verbose]\n",
	    argv[0]);
	exit (1);
    }

    umask (0077);	/* disallow non-owner access */

    p = strrchr(argv[0], '/');
    progName = (p ? p + 1 : argv[0]);
    topLevel = XtVaAppInitialize (&appContext, "XSm", NULL, 0,
	&argc, argv, NULL,
	XtNmappedWhenManaged, False,
	NULL);
	
    wmStateAtom = XInternAtom (XtDisplay (topLevel), "WM_STATE", False);


    /*
     * Set my own IO error handler.
     */

    IceSetIOErrorHandler (IoErrorHandler);


    /*
     * Ignore SIGPIPE
     */

    signal (SIGPIPE, SIG_IGN);


    /*
     * Init SM lib
     */

    if (!SmsInitialize ("SAMPLE-SM", "1.0",
	NewClientProc, NULL,
	HostBasedAuthProc, 256, errormsg))
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

    if (!SetAuthentication (numTransports, listenObjs, &authDataEntries))
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
	    NewConnectionXtProc, (XtPointer) listenObjs[i]);
    }

    /* the sizeof includes the \0, so we don't need to count the '=' */
    networkIds = IceComposeNetworkIdList (numTransports, listenObjs);
    p = (char *) malloc((sizeof environment_name) + strlen(networkIds) + 1);
    if(!p) nomem();
    sprintf(p, "%s=%s", environment_name, networkIds);
    putenv(p);

    if (verbose)
	printf ("setenv %s %s\n", environment_name, networkIds);

    free (networkIds);

    create_choose_session_popup ();
    create_main_window ();
    create_client_info_popup ();
    create_save_popup ();
    create_name_session_popup ();

    /*
     * We have to keep track of clients that go away.  They may come
     * back trying to use the same client ID.  So we keep a list of
     * dead clients.
     */

    dead_clients = ListInit ();
    if (!dead_clients)
	nomem ();


    /*
     * Get list of session names.  If a session name was found on the
     * command line, and it is in the list of session names we got, then
     * use that session name.  If there were no session names found, then
     * use the default session name.  Otherwise, present a list of session
     * names for the user to choose from.
     */

    success = GetSessionNames (&sessionNameCount, &sessionNames);

    found_command_line_name = 0;
    if (success && session_name)
    {
	for (i = 0; i < sessionNameCount; i++)
	    if (strcmp (session_name, sessionNames[i]) == 0)
	    {
		found_command_line_name = 1;
		break;
	    }
    }

    if (!success || found_command_line_name)
    {
	if (!found_command_line_name)
	    session_name = XtNewString (DEFAULT_SESSION_NAME);

    	StartSession (session_name, !found_command_line_name);
    }
    else
    {
	ChooseSession ();
    }
    

    /*
     * Main loop
     */

    setjmp (JumpHere);
    XtAppMainLoop (appContext);
}



void
PropertyChangeXtHandler (w, closure, event, continue_to_dispatch)

Widget w;
XtPointer closure;
XEvent *event;
Boolean *continue_to_dispatch;

{
    if (w == topLevel && event->type == PropertyNotify &&
	event->xproperty.atom == wmStateAtom)
    {
	XtRemoveEventHandler (topLevel, PropertyChangeMask, False,
	    PropertyChangeXtHandler, NULL);

	Restart (RESTART_REST_OF_CLIENTS);
    }
}



void
GetEnvironment ()

{
    static char	envDISPLAY[]="DISPLAY";
    static char	envSESSION_MANAGER[]="SESSION_MANAGER";
    static char	envAUDIOSERVER[]="AUDIOSERVER";
    char	*p, *temp;

    remote_allowed = 1;

    display_env = NULL;
    if(p = (char *) getenv(envDISPLAY)) {
	display_env = (char *) malloc(strlen(envDISPLAY)+1+strlen(p)+1);
	if(!display_env) nomem();
	sprintf(display_env, "%s=%s", envDISPLAY, p);

	/*
	 * When we restart a remote client, we have to make sure the
	 * display environment we give it has the SM's hostname.
	 */

	if ((temp = strchr (p, '/')) == 0)
	    temp = p;
	else
	    temp++;

	if (*temp != ':')
	{
	    /* we have a host name */

	    non_local_display_env = (char *) malloc (strlen (display_env) + 1);
	    if (!non_local_display_env) nomem();

	    strcpy (non_local_display_env, display_env);
	}
	else
	{
	    char hostnamebuf[256];

	    gethostname (hostnamebuf, sizeof hostnamebuf);
	    non_local_display_env = (char *) malloc (strlen (envDISPLAY) + 1 +
		strlen (hostnamebuf) + strlen (temp) + 1);
	    if (!non_local_display_env) nomem();
	    sprintf(non_local_display_env, "%s=%s%s",
		envDISPLAY, hostnamebuf, temp);
	}
    }

    session_env = NULL;
    if(p = (char *) getenv(envSESSION_MANAGER)) {
	session_env = (char *) malloc(
	    strlen(envSESSION_MANAGER)+1+strlen(p)+1);
	if(!session_env) nomem();
	sprintf(session_env, "%s=%s", envSESSION_MANAGER, p);

	/*
	 * When we restart a remote client, we have to make sure the
	 * session environment does not have the SM's local connection port.
	 */

	non_local_session_env = (char *) malloc (strlen (session_env) + 1);
	if (!non_local_session_env) nomem();
	strcpy (non_local_session_env, session_env);

	if ((temp = Strstr (non_local_session_env, "local/")) != NULL)
	{
	    char *delim = strchr (temp, ',');
	    if (delim == NULL)
	    {
		if (temp == non_local_session_env +
		    strlen (envSESSION_MANAGER) + 1)
		{
		    *temp = '\0';
		    remote_allowed = 0;
		}
		else
		    *(temp - 1) = '\0';
	    }
	    else
	    {
		int bytes = strlen (delim + 1);
		memmove (temp, delim + 1, bytes);
		*(temp + bytes) = '\0';
	    }
	}
    }

    audio_env = NULL;
    if(p = (char *) getenv(envAUDIOSERVER)) {
	audio_env = (char *) malloc(strlen(envAUDIOSERVER)+1+strlen(p)+1);
	if(!audio_env) nomem();
	sprintf(audio_env, "%s=%s", envAUDIOSERVER, p);
    }
}



void
StartSession (name, use_default)

char *name;
Bool use_default;

{
    int database_read = 0;
    Dimension width;
    char title[256];


    /*
     * Get important environment variables.
     */

    GetEnvironment ();


    /*
     * Set the main window's title to the session name.
     */

    sprintf (title, "xsm: %s", name);

    XtVaSetValues (topLevel,
	XtNtitle, title,		/* session name */
	NULL);

    XtRealizeWidget (topLevel);


    /*
     * Read the session save file.  Make sure the session manager
     * has an SM_CLIENT_ID, so that other managers (like the WM) can
     * identify it.
     */

    set_session_save_file_name (name);

    if (use_default)
	need_to_name_session = True;
    else
    {
	database_read = ReadSave (name, &sm_id);
	need_to_name_session = !database_read;
    }

    if (!sm_id)
	sm_id = SmsGenerateClientID (NULL);
    XChangeProperty (XtDisplay (topLevel), XtWindow (topLevel),
	XInternAtom (XtDisplay (topLevel), "SM_CLIENT_ID", False),
	XA_STRING, 8, PropModeReplace,
	(unsigned char *) sm_id, strlen (sm_id));


    /*
     * Adjust some label widths
     */

    XtVaGetValues (clientInfoButton,
	XtNwidth, &width,
	NULL);

    XtVaSetValues (checkPointButton,
	XtNwidth, width,
	NULL);

    XtVaGetValues (nameSessionButton,
	XtNwidth, &width,
	NULL);

    XtVaSetValues (shutdownButton,
	XtNwidth, width,
	NULL);
    

    XtMapWidget (topLevel);


    if (!database_read)
    {
	/*
	 * Start default apps (e.g. twm, smproxy)
	 */

	StartDefaultApps ();
    }
    else
    {
	/*
	 * Restart window manager first.  When the session manager
	 * gets a WM_STATE stored on its top level window, we know
	 * the window manager is running.  At that time, we can start
	 * the rest of the applications.
	 */

	XtAddEventHandler (topLevel, PropertyChangeMask, False,
	    PropertyChangeXtHandler, NULL);

	if (!Restart (RESTART_MANAGERS))
	{
	    XtRemoveEventHandler (topLevel, PropertyChangeMask, False,
	        PropertyChangeXtHandler, NULL);

	    Restart (RESTART_REST_OF_CLIENTS);
	}
    }
}



EndSession ()

{
    if (verbose)
	printf ("\nSESSION MANAGER GOING AWAY!\n");

    FreeAuthenticationData (numTransports, authDataEntries);

    if (display_env)
	free (display_env);
    if (session_env)
	free (session_env);
    if (non_local_display_env)
	free (non_local_display_env);
    if (non_local_session_env)
	free (non_local_session_env);
    if (audio_env)
	free (audio_env);

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

    if (verbose)
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

	if (strcmp (prop->name, SmDiscardCommand) == 0)
	{
	    if (client->discardCommand)
		XtFree (client->discardCommand);
	    client->discardCommand = (char *) XtNewString(prop->vals[0].value);
	}
	else if (strcmp (prop->name, SmRestartStyleHint) == 0)
	{
	    int hint = (int) *((char *) (prop->vals[0].value));

	    if (hint == SmRestartIfRunning || hint == SmRestartAnyway ||
		hint == SmRestartImmediately || hint == SmRestartNever)
	    {
		client->restartHint = hint;
	    }
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



void
SetProperty(client, theProp, mallocFlag)
ClientRec	*client;
SmProp		*theProp;
Bool		mallocFlag;

{
    SmProp *prop;
    int	idx, i, j;

    for (j = 0; j < client->numProps; j++)
	if (strcmp (theProp->name, client->props[j]->name) == 0)
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

    if (!mallocFlag)
    {
	prop = theProp;
    }
    else
    {
	prop = (SmProp *) malloc (sizeof (SmProp));

	prop->name = (char *) malloc (strlen (theProp->name) + 1);
	strcpy (prop->name, theProp->name);

	prop->type = (char *) malloc (strlen (theProp->type) + 1);
	strcpy (prop->type, theProp->type);

	prop->num_vals = theProp->num_vals;

	prop->vals = (SmPropValue *) malloc (
	    theProp->num_vals * sizeof (SmPropValue));

	for (i = 0; i < theProp->num_vals; i++)
	{
	    prop->vals[i].length = theProp->vals[i].length;
	    prop->vals[i].value = (SmPointer) malloc (
		theProp->vals[i].length + 1);
	    memcpy (prop->vals[i].value, theProp->vals[i].value,
		theProp->vals[i].length);
	    ((char *) prop->vals[i].value)[theProp->vals[i].length] = '\0';
	}
    }

    client->props[idx] = prop;

    if (strcmp (prop->name, SmDiscardCommand) == 0)
    {
	if (saveInProgress)
	{
	    /*
	     * We are in the middle of a save yourself.  We save the
	     * discard command we get now, and make it the current discard
	     * command when the save is over.
	     */

	    if (client->saveDiscardCommand)
		XtFree (client->saveDiscardCommand);
	    client->saveDiscardCommand =
		(char *) XtNewString (prop->vals[0].value);
	}
	else
	{
	    if (client->discardCommand)
		XtFree (client->discardCommand);
	    client->discardCommand =
		(char *) XtNewString (prop->vals[0].value);
	}
    }
    else if (strcmp (prop->name, SmRestartStyleHint) == 0)
    {
	int hint = (int) *((char *) (prop->vals[0].value));

	if (hint == SmRestartIfRunning || hint == SmRestartAnyway ||
	    hint == SmRestartImmediately || hint == SmRestartNever)
	{
	    client->restartHint = hint;
	}
    }
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

	    if (strcmp (propname, SmDiscardCommand) == 0)
	    {
		if (client->discardCommand)
		{
		    XtFree (client->discardCommand);
		    client->discardCommand = NULL;
		}

		if (client->saveDiscardCommand)
		{
		    XtFree (client->saveDiscardCommand);
		    client->saveDiscardCommand = NULL;
		}
	    }

	    break;
	}
}



PrintProperty (prop)

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
    int		send_save;

    if (verbose)
    {
	printf (
	"On IceConn fd = %d, received REGISTER CLIENT [Previous Id = %s]\n",
	IceConnectionNumber (client->ice_conn),
	previousId ? previousId : "NULL");
	printf ("\n");
    }

    if (!previousId)
    {
	id = SmsGenerateClientID (smsConn);
	send_save = 1;
    }
    else
    {
	int found_match = 0;
	send_save = 1;

	if (PendingList)
	{
	    for (cl = ListFirst (PendingList); cl; cl = ListNext (cl))
	    {
		if (!strcmp (((PendingClient *) cl->thing)->clientId,
		    previousId))
		{
		    SetInitialProperties (client, (PendingClient *) cl->thing);
		    ListFreeOne (cl);
		    found_match = 1;
		    send_save = 0;
		    break;
		}
	    }
	}

	if (!found_match)
	{
	    for (cl = ListFirst (dead_clients); cl; cl = ListNext (cl))
	    {
		if (strcmp ((char *) cl->thing, previousId) == 0)
		{
		    found_match = 1;
		    XtFree (cl->thing);
		    ListFreeOne (cl);
		    break;
		}
	    }
	}

	if (!found_match)
	{
	    /* previous id was bogus, return bad status */

	    free (previousId);
	    return (0);
	}
	else
	{
	    id = previousId;
	}
    }

    SmsRegisterClientReply (smsConn, id);

    if (verbose) {
	printf (
	"On IceConn fd = %d, sent REGISTER CLIENT REPLY [Client Id = %s]\n",
	IceConnectionNumber (client->ice_conn), id);
	printf ("\n");
    }

    client->clientId = id;
    client->clientHostname = SmsClientHostName (smsConn);
    client->restarted = (previousId != NULL);

    if (send_save) {
	SmsSaveYourself(smsConn, SmSaveLocal, False, SmInteractStyleNone,
			False);
    } else if (client_info_visible) {
	/* We already have all required client info */

	UpdateClientList ();
	XawListHighlight (clientListWidget, current_client_selected);
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

    if (verbose) {
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

    if (verbose) {
	printf (
	"Client Id = %s, received INTERACT DONE [Cancel Shutdown = %s]\n",
	client->clientId, cancelShutdown ? "True" : "False");
    }

    client->interactPending = False;

    if (cancelShutdown && !shutdownCancelled) {
	shutdownCancelled = True;
	for (client = ClientList; client; client = client->next) {
	    if (!client->running)
		continue;
	    SmsShutdownCancelled (client->smsConn);
	    if (verbose) 
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
    if (verbose) 
	printf("SAVE YOURSELF REQUEST not supported!\n");
}



void
SaveYourselfPhase2ReqProc (smsConn, managerData)

SmsConn     smsConn;
SmPointer   managerData;

{
    ClientRec	*client = (ClientRec *) managerData;

    if (verbose) {
	printf ("Client Id = %s, received SAVE YOURSELF PHASE 2 REQUEST\n",
	    client->clientId);
    }

    if (!saveInProgress)
    {
	/*
	 * If we are not in the middle of a checkpoint (ie. we just
	 * started the client and sent the initial save yourself), just
	 * send the save yourself phase2 now.
	 */
	 
	SmsSaveYourselfPhase2 (client->smsConn);
    }
    else if (!client->wantsPhase2)
    {
	client->wantsPhase2 = True;
	phase2RequestCount++;
    }
}



void
SaveYourselfDoneProc (smsConn, managerData, success)
    SmsConn     smsConn;
    SmPointer 	managerData;
    Bool	success;

{
    ClientRec	*client = (ClientRec *) managerData;

    if (verbose) 
	printf("Client Id = %s, received SAVE YOURSELF DONE [Success = %s]\n",
	       client->clientId, success ? "True" : "False");

    if (shutdownCancelled && client->interactPending)
	client->interactPending = False;

    saveDoneCount++;
}



static void
CloseDownClient (client)

ClientRec *client;

{
    ClientRec 	*next = client->next;
    ClientRec	*ptr;
    List	*cl;
    int		index_deleted, i;

    if (verbose) {
	printf ("ICE Connection closed, IceConn fd = %d\n",
		IceConnectionNumber (client->ice_conn));
	printf ("\n");
    }

    SmsCleanUp (client->smsConn);
    IceSetShutdownNegotiation (client->ice_conn, False);
    IceCloseConnection (client->ice_conn);

    client->ice_conn = NULL;
    client->smsConn = NULL;
    client->running = False;

    ListAddLast (dead_clients, XtNewString (client->clientId));

    if (client->restartHint == SmRestartImmediately && !shutdownInProgress)
    {
	Clone (client, True /* use saved state */);
    }

    if (!shutdownInProgress && client_info_visible)
    {
	for (index_deleted = 0;
	    index_deleted < numClientListNames; index_deleted++)
	{
	    if (clientListRecs[index_deleted] == client)
		break;
	}
    }

    if (client->restartHint != SmRestartAnyway || shutdownInProgress)
    {
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
    }

    numClients--;

    if (shutdownInProgress)
    {
	if (numClients == 0)
	    EndSession ();
    }
    else if (client_info_visible)
    {
	UpdateClientList ();

	if (current_client_selected == index_deleted)
	{
	    if (current_client_selected == numClients)
		current_client_selected--;

	    if (current_client_selected >= 0)
	    {
		XawListHighlight (clientListWidget, current_client_selected);
		ShowHint (clientListRecs[current_client_selected]);
		if (client_prop_visible)
		{
		    DisplayProps (clientListRecs[current_client_selected],
			clientListNames[current_client_selected]);
		}
	    }
	}
	else
	{
	    if (index_deleted < current_client_selected)
		current_client_selected--;
	    XawListHighlight (clientListWidget, current_client_selected);
	}
    }
}




void
CloseConnectionProc (smsConn, managerData, count, reasonMsgs)
    SmsConn 	smsConn;
    SmPointer  	managerData;
    int		count;
    char 	**reasonMsgs;

{
    ClientRec	*client = (ClientRec *) managerData;

    if (verbose)
    {
	int i;

	printf ("Client Id = %s, received CONNECTION CLOSED\n",
	    client->clientId);

	for (i = 0; i < count; i++)
	    printf ("   Reason string %d: %s\n", i + 1, reasonMsgs[i]);
	printf ("\n");
    }

    SmFreeReasons (count, reasonMsgs);

    CloseDownClient (client);
}



void
SetPropertiesProc (smsConn, managerData, numProps, props)

SmsConn 	smsConn;
SmPointer 	managerData;
int		numProps;
SmProp 		**props;

{
    ClientRec	*client = (ClientRec *) managerData;
    int		updateList, i;

    if (verbose) {
	printf ("Client Id = %s, received SET PROPERTIES ", client->clientId);
	printf ("[Num props = %d]\n", numProps);
    }

    updateList = client->numProps == 0 && numProps > 0 && client_info_visible;

    for (i = 0; i < numProps; i++) {
	if(verbose)
	    PrintProperty (props[i]);
	SetProperty (client, props[i], False /* Don't malloc - use this */);
    }
    free ((char *) props);

    if (updateList)
    {
	/*
	 * We have enough info from the client to display it in our list.
	 */

	UpdateClientList ();
	XawListHighlight (clientListWidget, current_client_selected);
    }
    else if (client_prop_visible && clientListRecs &&
	clientListRecs[current_client_selected] == client)
    {
	DisplayProps (client, clientListNames[current_client_selected]);
    }
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

    if (verbose) {
	printf ("Client Id = %s, received DELETE PROPERTIES ",
	    client->clientId);
	printf ("[Num props = %d]\n", numProps);
    }

    for (i = 0; i < numProps; i++) {
	if(verbose)
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

    if (verbose) {
	printf ("Client Id = %s, received GET PROPERTIES\n", client->clientId);
	printf ("\n");

	for (i = 0; i < client->numProps; i++) {
	    PrintProperty (client->props[i]);
	}
	printf ("\n");
    }

    SmsReturnProperties (smsConn, client->numProps, client->props);

    if (verbose) {
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
    ClientRec *ptr, *prev;

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
    newClient->restarted = False; /* wait till RegisterClient for true value */
    newClient->userIssuedCheckpoint = False;
    newClient->interactPending = False;
    newClient->wantsPhase2 = False;
    newClient->numProps = 0;
    newClient->discardCommand = NULL;
    newClient->saveDiscardCommand = NULL;
    newClient->running = True;
    newClient->restartHint = SmRestartIfRunning;
    newClient->next = NULL;

    ptr = ClientList;
    prev = NULL;
	
    while (ptr)
    {
	prev = ptr;
	ptr = ptr->next;
    }

    if (prev)
	prev->next = newClient;
    else
	ClientList = newClient;

    numClients++;

    if (verbose) {
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

    *maskRet |= SmsSaveYourselfP2RequestProcMask;
    callbacksRet->save_yourself_phase2_request.callback =
	SaveYourselfPhase2ReqProc;
    callbacksRet->save_yourself_phase2_request.manager_data =
	(SmPointer) newClient;

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
NewConnectionXtProc (client_data, source, id)

XtPointer	client_data;
int 		*source;
XtInputId	*id;

{
    IceConn 	ice_conn;
    char	*connstr;
    IceAcceptStatus status;

    ice_conn = IceAcceptConnection((IceListenObj) client_data, &status);
    if (! ice_conn) {
	if (verbose)
	    printf ("IceAcceptConnection failed\n");
    } else {
	IceConnectStatus cstatus;

	while ((cstatus = IceConnectionStatus (ice_conn))==IceConnectPending) {
	    XtAppProcessEvent (appContext, XtIMAll);
	}

	if (cstatus == IceConnectAccepted) {
	    if (verbose) {
		printf ("ICE Connection opened by client, IceConn fd = %d, ",
			IceConnectionNumber (ice_conn));
		connstr = IceConnectionString (ice_conn);
		printf ("Accept at networkId %s\n", connstr);
		free (connstr);
		printf ("\n");
	    }
	} else {
	    if (verbose)
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



SetAllSensitive (on)

Bool on;

{
    XtSetSensitive (mainWindow, on);
    XtSetSensitive (savePopup, on);
    XtSetSensitive (clientInfoPopup, on);
    XtSetSensitive (clientPropPopup, on);

    if (on)
	XawListHighlight (clientListWidget, current_client_selected);
}



/*
 * Install IO error handler.  This will detect clients that break their
 * connection with the SM unexpectidly.
 */

void
IoErrorHandler (ice_conn)

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
	ClientRec *client = ClientList;
	
	while (client && client->ice_conn != ice_conn)
	    client = client->next;

	if (!client)
	{
	    fprintf (stderr,
	"Internal error found in IO error handler - couldn't find ice_conn\n");
	    exit (1);
	}

	if (verbose)
	{
	    printf ("IO error on connection (fd = %d)\n",
		IceConnectionNumber (ice_conn));
	    printf ("\n");
	}

	CloseDownClient (client);
    }


    /*
     * We can't return.  Must do a long jump.  Make sure any
     * popups are uppopped.
     */

    XtPopdown (savePopup);
    SetAllSensitive (1);

    longjmp (JumpHere, 1);
}    
