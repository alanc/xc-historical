/* $XConsortium: smproxy.c,v 1.14 94/07/07 15:18:09 mor Exp $ */
/******************************************************************************

Copyright (c) 1994  X Consortium

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

Author:  Ralph Mor, X Consortium
******************************************************************************/

#include "smproxy.h"

XtAppContext appContext;
Display *disp;
Window root;

Atom wmProtocolsAtom;
Atom wmSaveYourselfAtom;
Atom wmStateAtom;
Atom smClientIdAtom;

Bool debug = 0;

SmcConn proxy_smcConn;
XtInputId proxy_iceInputId;
char *proxy_clientId = NULL;

WinInfo win_list[256];		/* TODO: use linked list or hash table */
int win_count = 0;
int proxy_count = 0;
int die_count = 0;

Bool ok_to_die = 0;

int Argc;
char **Argv;



Bool
HasSaveYourself (window)

Window window;

{
    Atom *protocols;
    int numProtocols;
    int i, found;

    protocols = NULL;

    if (XGetWMProtocols (disp, window, &protocols, &numProtocols) != True)
	return (False);

    found = 0;

    if (protocols != NULL)
    {
	for (i = 0; i < numProtocols; i++)
	    if (protocols[i] == wmSaveYourselfAtom)
		found = 1;

	XFree (protocols);
    }

    return (found);
}



Bool
HasXSMPsupport (window)

Window window;

{
    XTextProperty tp;
    Bool hasIt = 0;

    if (XGetTextProperty (disp, window, &tp, smClientIdAtom))
    {
	if (tp.encoding == XA_STRING && tp.format == 8 && tp.nitems != 0)
	    hasIt = 1;

	if (tp.value)
	    XFree ((char *) tp.value);
    }

    return (hasIt);
}



void FinishSaveYourself (winInfo)

WinInfo *winInfo;

{
    SmProp prop1, prop2, prop3, prop4, *props[4];
    SmPropValue prop3val, prop4val;
    char userId[20];
    int i;

    prop1.name = SmRestartCommand;
    prop1.type = SmLISTofARRAY8;
    prop1.num_vals = winInfo->wm_command_count;
    
    prop1.vals = (SmPropValue *) malloc (
	winInfo->wm_command_count * sizeof (SmPropValue));
    
    if (!prop1.vals)
    {
	SmcSaveYourselfDone (winInfo->smc_conn, False);
	return;
    }

    for (i = 0; i < winInfo->wm_command_count; i++)
    {
	prop1.vals[i].value = (SmPointer) winInfo->wm_command[i];
	prop1.vals[i].length = strlen (winInfo->wm_command[i]);
    }
    
    prop2.name = SmCloneCommand;
    prop2.type = SmLISTofARRAY8;
    prop2.num_vals = winInfo->wm_command_count;
    prop2.vals = prop1.vals;
    
    prop3.name = SmProgram;
    prop3.type = SmARRAY8;
    prop3.num_vals = 1;
    prop3.vals = &prop3val;
    prop3val.value = (SmPointer) winInfo->wm_command[0];
    prop3val.length = strlen (winInfo->wm_command[0]);
    
    sprintf (userId, "%d", getuid());
    prop4.name = SmUserID;
    prop4.type = SmARRAY8;
    prop4.num_vals = 1;
    prop4.vals = &prop4val;
    prop4val.value = (SmPointer) userId;
    prop4val.length = strlen (userId);
    
    props[0] = &prop1;
    props[1] = &prop2;
    props[2] = &prop3;
    props[3] = &prop4;
    
    SmcSetProperties (winInfo->smc_conn, 4, props);
    
    free ((char *) prop1.vals);
    
    SmcSaveYourselfDone (winInfo->smc_conn, True);
}



void
SaveYourselfCB (smcConn, clientData, saveType, shutdown, interactStyle, fast)

SmcConn smcConn;
SmPointer clientData;
int saveType;
Bool shutdown;
int interactStyle;
Bool fast;

{
    WinInfo *winInfo = (WinInfo *) clientData;

    if (!winInfo->has_save_yourself)
    {
	FinishSaveYourself (winInfo);
    }
    else
    {
	XClientMessageEvent saveYourselfMessage;


	/* Send WM_SAVE_YOURSELF */

	saveYourselfMessage.type = ClientMessage;
	saveYourselfMessage.window = winInfo->window;
	saveYourselfMessage.message_type = wmProtocolsAtom;
	saveYourselfMessage.format = 32;
	saveYourselfMessage.data.l[0] = wmSaveYourselfAtom;
	saveYourselfMessage.data.l[1] = CurrentTime;

	if (XSendEvent (disp, winInfo->window, False, NoEventMask,
	    (XEvent *) &saveYourselfMessage))
	{
	    winInfo->waiting_for_update = 1;

	    if (debug)
	    {
		printf ("Sent SAVE YOURSELF to 0x%x\n", winInfo->window);    
		printf ("\n");
	    }
	}
	else
	{
	    if (debug)
	    {
		printf ("Failed to send SAVE YOURSELF to 0x%x\n",
		    winInfo->window);    
		printf ("\n");
	    }
	}
    }
}



void
DieCB (smcConn, clientData)

SmcConn smcConn;
SmPointer clientData;

{
    WinInfo *winInfo = (WinInfo *) clientData;

    SmcCloseConnection (winInfo->smc_conn, 0, NULL);
    winInfo->smc_conn = NULL;
    XtRemoveInput (winInfo->input_id);

    /* Now tell the client to die */

    if (debug)
	printf ("Trying to kill 0x%x\n", winInfo->window);

    XSync (disp, 0);
    XKillClient (disp, winInfo->window);
    XSync (disp, 0);


    /*
     * Proxy must exit when all clients die, and the proxy itself
     * must have received a Die.
     */

    die_count++;

    if (die_count == proxy_count && ok_to_die)
    {
	SmcCloseConnection (proxy_smcConn, 0, NULL);
	exit (0);
    }
}



void
SaveCompleteCB (smcConn, clientData)

SmcConn smcConn;
SmPointer clientData;

{
    WinInfo *winInfo = (WinInfo *) clientData;

}



void
ShutdownCancelledCB (smcConn, clientData)

SmcConn smcConn;
SmPointer clientData;

{
    WinInfo *winInfo = (WinInfo *) clientData;

}



void
ProcessIceMsgProc (client_data, source, id)

XtPointer	client_data;
int 		*source;
XtInputId	*id;

{
    IceConn	ice_conn = (IceConn) client_data;

    IceProcessMessages (ice_conn, NULL, NULL);
}



void
ConnectClientToSM (winInfo)

WinInfo *winInfo;

{
    char errorMsg[256];
    unsigned long mask;
    SmcCallbacks callbacks;
    IceConn ice_conn;
    char *prevId;

    mask = SmcSaveYourselfProcMask | SmcDieProcMask |
	SmcSaveCompleteProcMask | SmcShutdownCancelledProcMask;

    callbacks.save_yourself.callback = SaveYourselfCB;
    callbacks.save_yourself.client_data = (SmPointer) winInfo;

    callbacks.die.callback = DieCB;
    callbacks.die.client_data = (SmPointer) winInfo;

    callbacks.save_complete.callback = SaveCompleteCB;
    callbacks.save_complete.client_data = (SmPointer) winInfo;

    callbacks.shutdown_cancelled.callback = ShutdownCancelledCB;
    callbacks.shutdown_cancelled.client_data = (SmPointer) winInfo;

    prevId = LookupClientID (winInfo);

    winInfo->smc_conn = SmcOpenConnection (
	NULL, 			/* use SESSION_MANAGER env */
	(SmPointer) winInfo,	/* force a new connection */
	SmProtoMajor,
	SmProtoMinor,
	mask,
	&callbacks,
	prevId,
	&winInfo->client_id,
	256, errorMsg);

    if (winInfo->smc_conn == NULL)
	return;

    ice_conn = SmcGetIceConnection (winInfo->smc_conn);

    winInfo->input_id = XtAppAddInput (
	    appContext,
	    IceConnectionNumber (ice_conn),
            (XtPointer) XtInputReadMask,
	    ProcessIceMsgProc,
	    (XtPointer) ice_conn);

    if (debug)
    {
	printf ("Connected to SM, window = 0x%x\n", winInfo->window);
	printf ("\n");
    }

    proxy_count++;
}



int
AddNewWindow (window)

Window window;

{
    int i;

    for (i = 0; i < win_count; i++)
    {
	if (win_list[i].window == window)
	    return -1;
    }

    win_list[win_count].window = window;
    win_list[win_count].smc_conn = NULL;
    win_list[win_count].waiting_for_required_props = 1;
    win_list[win_count].got_wm_state = 0;
    win_list[win_count].client_id = NULL;
    win_list[win_count].wm_command = NULL;
    win_list[win_count].wm_command_count = 0;
    win_list[win_count].class.res_name = NULL;
    win_list[win_count].class.res_class = NULL;
    win_list[win_count].wm_name = NULL;
    win_list[win_count].has_save_yourself = 0;
    win_list[win_count].waiting_for_update = 0;
    
    win_count++;

    return (win_count - 1);
}



void
HandleCreate (event)

XCreateWindowEvent *event;

{
    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytesafter;
    unsigned long *datap = NULL;

    /*
     * Add the new window
     */

    int index;

    if ((index = AddNewWindow (event->window)) < 0)
	return;


    /*
     * Select for Property Notify on the window so we can determine
     * when the client has set all required properties.
     *
     * Select for Substructure Notify so we can determine when the
     * window is destroyed.
     */

    XSelectInput (disp, event->window,
	SubstructureNotifyMask | PropertyChangeMask);	


    /*
     * There might be a race condition because a property might
     * have already changed right after the CreateNotify.  To get
     * around this, we must check for all properties now.
     */

    XFetchName (disp, event->window, &win_list[index].wm_name);

    XGetCommand (disp, event->window,
	&win_list[index].wm_command,
	&win_list[index].wm_command_count);

    XGetClassHint (disp, event->window, &win_list[index].class);

    if (XGetWindowProperty (disp, event->window, wmStateAtom,
	0L, 2L, False, wmStateAtom,
	&actual_type, &actual_format, &nitems, &bytesafter,
	(unsigned char **) &datap) && datap)
    {
	if (nitems > 0)
	    win_list[index].got_wm_state = 1;

	if (datap)
	    XFree ((char *) datap);
    }

    if (win_list[index].got_wm_state &&
	win_list[index].wm_name != NULL &&
	win_list[index].wm_command != NULL &&
	win_list[index].wm_command_count > 0 &&
	win_list[index].class.res_name != NULL &&
	win_list[index].class.res_class != NULL)
    {
	win_list[index].waiting_for_required_props = 0;

	if (!HasXSMPsupport (event->window))
	{
	    win_list[index].has_save_yourself =
		HasSaveYourself (event->window);

	    ConnectClientToSM (&win_list[index]);
	}
    }
}



void
HandleDestroy (event)

XDestroyWindowEvent *event;

{
    int i;

    for (i = 0; i < win_count; i++)
    {
	if (win_list[i].window == event->window)
	{
	    if (win_list[i].smc_conn)
	    {
		SmcCloseConnection (win_list[i].smc_conn, 0, NULL);
		win_list[i].smc_conn = NULL;
		XtRemoveInput (win_list[i].input_id);
	    }

	    if (debug)
	    {
		printf ("Removed window (window = 0x%x)\n",
		    win_list[i].window);
		printf ("\n");
	    }

	    if (win_list[i].client_id)
		free (win_list[i].client_id);

	    if (win_list[i].wm_command)
		XFreeStringList (win_list[i].wm_command);

	    if (win_list[i].wm_name)
		XFree (win_list[i].wm_name);

	    if (win_list[i].class.res_name)
		XFree (win_list[i].class.res_name);

	    if (win_list[i].class.res_class)
		XFree (win_list[i].class.res_class);

	    if (i < win_count - 1)
		win_list[i] = win_list[win_count - 1];

	    win_count--;
	    break;
	}
    }
}



void
HandleUpdate (event)

XPropertyEvent *event;

{
    Window window = event->window;
    int i;

    if (event->atom != XA_WM_NAME && event->atom != XA_WM_COMMAND &&
	event->atom != XA_WM_CLASS && event->atom != wmStateAtom)
    {
	/*
	 * We are only interested in WM_NAME, WM_COMMAND,
	 * WM_CLASS, and WM_STATE.
	 */

	return;
    }

    for (i = 0; i < win_count; i++)
	if (win_list[i].window == window)
	    break;

    if (i < win_count)
    {
	if (event->atom == XA_WM_NAME)
	{
	    if (win_list[i].wm_name)
	    {
		XFree (win_list[i].wm_name);
		win_list[i].wm_name = NULL;
	    }

	    XFetchName (disp, window, &win_list[i].wm_name);
	}
	else if (event->atom == XA_WM_COMMAND)
	{
	    if (win_list[i].wm_command)
	    {
		XFreeStringList (win_list[i].wm_command);
		win_list[i].wm_command = NULL;
		win_list[i].wm_command_count = 0;
	    }

	    XGetCommand (disp, window,
		&win_list[i].wm_command,
		&win_list[i].wm_command_count);

	    if (win_list[i].waiting_for_update)
	    {
		/* Finish off the Save Yourself */

		win_list[i].waiting_for_update = 0;
		FinishSaveYourself (&win_list[i]);
	    }
	}
	else if (event->atom == XA_WM_CLASS)
	{
	    if (win_list[i].class.res_name)
	    {
		XFree (win_list[i].class.res_name);
		win_list[i].class.res_name = NULL;
	    }

	    if (win_list[i].class.res_class)
	    {
		XFree (win_list[i].class.res_class);
		win_list[i].class.res_class = NULL;
	    }

	    XGetClassHint (disp, window, &win_list[i].class);
	}
	else if (event->atom == wmStateAtom)
	{
	    win_list[i].got_wm_state = 1;
	}

	if (win_list[i].waiting_for_required_props)
	{
	    if (win_list[i].got_wm_state &&
		win_list[i].wm_name != NULL &&
		win_list[i].wm_command != NULL &&
		win_list[i].wm_command_count > 0 &&
		win_list[i].class.res_name != NULL &&
		win_list[i].class.res_class != NULL)
	    {
		win_list[i].waiting_for_required_props = 0;
		
		if (!HasXSMPsupport (window))
		{
		    win_list[i].has_save_yourself = HasSaveYourself (window);

		    ConnectClientToSM (&win_list[i]);
		}
	    }
	}
    }
}



void
ProxySaveYourselfCB (smcConn, clientData, saveType,
    shutdown, interactStyle, fast)

SmcConn smcConn;
SmPointer clientData;
int saveType;
Bool shutdown;
int interactStyle;
Bool fast;

{
    FILE *proxyFile;
    char *path, *filename;
    Bool success = True;
    SmProp prop1, prop2, prop3, prop4, prop5, *props[5];
    SmPropValue prop3val, prop4val, prop5val;
    char discardCommand[80], userId[20];
    int numVals, i;

    path = getenv ("SM_SAVE_DIR");
    if (!path)
    {
	path = getenv ("HOME");
	if (!path)
	    path = ".";
    }

    filename = tempnam (path, ".PRX");
    proxyFile = fopen (filename, "wb");

    for (i = 0; i < win_count; i++)
    {
	if (win_list[i].client_id)
	    if (!WriteProxyFileEntry (proxyFile, &win_list[i]))
	    {
		success = False;
		break;
	    }
    }

    fclose (proxyFile);

    prop1.name = SmRestartCommand;
    prop1.type = SmLISTofARRAY8;

    prop1.vals = (SmPropValue *) malloc (
	(Argc + 4) * sizeof (SmPropValue));

    if (!prop1.vals)
    {
	SmcSaveYourselfDone (smcConn, False);
	return;
    }

    numVals = 0;

    for (i = 0; i < Argc; i++)
    {
	if (strcmp (Argv[i], "-clientId") == 0 ||
	    strcmp (Argv[i], "-restore") == 0)
	{
	    i++;
	}
	else
	{
	    prop1.vals[numVals].value = (SmPointer) Argv[i];
	    prop1.vals[numVals++].length = strlen (Argv[i]);
	}
    }

    prop1.vals[numVals].value = (SmPointer) "-clientId";
    prop1.vals[numVals++].length = 9;

    prop1.vals[numVals].value = (SmPointer) proxy_clientId;
    prop1.vals[numVals++].length = strlen (proxy_clientId);

    prop1.vals[numVals].value = (SmPointer) "-restore";
    prop1.vals[numVals++].length = 8;

    prop1.vals[numVals].value = (SmPointer) filename;
    prop1.vals[numVals++].length = strlen (filename);

    prop1.num_vals = numVals;

    prop2.name = SmCloneCommand;
    prop2.type = SmLISTofARRAY8;
    prop2.num_vals = numVals - 4;		/* don't include restart */
    prop2.vals = prop1.vals;
    
    prop3.name = SmProgram;
    prop3.type = SmARRAY8;
    prop3.num_vals = 1;
    prop3.vals = &prop3val;
    prop3val.value = Argv[0];
    prop3val.length = strlen (Argv[0]);

    sprintf (discardCommand, "rm %s", filename);
    prop4.name = SmDiscardCommand;
    prop4.type = SmARRAY8;
    prop4.num_vals = 1;
    prop4.vals = &prop4val;
    prop4val.value = (SmPointer) discardCommand;
    prop4val.length = strlen (discardCommand);

    sprintf (userId, "%d", getuid());
    prop5.name = SmUserID;
    prop5.type = SmARRAY8;
    prop5.num_vals = 1;
    prop5.vals = &prop5val;
    prop5val.value = (SmPointer) userId;
    prop5val.length = strlen (userId);

    props[0] = &prop1;
    props[1] = &prop2;
    props[2] = &prop3;
    props[3] = &prop4;
    props[4] = &prop5;

    SmcSetProperties (smcConn, 5, props);
    free ((char *) prop1.vals);

    SmcSaveYourselfDone (smcConn, success);
}



void
ProxyDieCB (smcConn, clientData)

SmcConn smcConn;
SmPointer clientData;

{
    ok_to_die = 1;

    /* should really use phase 2 stuff here!!!!!!!!!!!!! */

    if (die_count == proxy_count)
    {
	SmcCloseConnection (proxy_smcConn, 0, NULL);
	exit (0);
    }
    else
	XtRemoveInput (proxy_iceInputId);
}



void
ProxySaveCompleteCB (smcConn, clientData)

SmcConn smcConn;
SmPointer clientData;

{
    ;
}



void
ProxyShutdownCancelledCB (smcConn, clientData)

SmcConn smcConn;
SmPointer clientData;

{
    ;
}



ConnectProxyToSM (previous_id)

char *previous_id;

{
    char errorMsg[256];
    unsigned long mask;
    SmcCallbacks callbacks;
    IceConn iceConn;

    mask = SmcSaveYourselfProcMask | SmcDieProcMask |
	SmcSaveCompleteProcMask | SmcShutdownCancelledProcMask;

    callbacks.save_yourself.callback = ProxySaveYourselfCB;
    callbacks.save_yourself.client_data = (SmPointer) NULL;

    callbacks.die.callback = ProxyDieCB;
    callbacks.die.client_data = (SmPointer) NULL;

    callbacks.save_complete.callback = ProxySaveCompleteCB;
    callbacks.save_complete.client_data = (SmPointer) NULL;

    callbacks.shutdown_cancelled.callback = ProxyShutdownCancelledCB;
    callbacks.shutdown_cancelled.client_data = (SmPointer) NULL;

    proxy_smcConn = SmcOpenConnection (
	NULL, 			/* use SESSION_MANAGER env */
	(SmPointer) appContext,
	SmProtoMajor,
	SmProtoMinor,
	mask,
	&callbacks,
	previous_id,
	&proxy_clientId,
	256, errorMsg);

    if (proxy_smcConn == NULL)
	return;

    iceConn = SmcGetIceConnection (proxy_smcConn);

    proxy_iceInputId = XtAppAddInput (
	    appContext,
	    IceConnectionNumber (iceConn),
            (XtPointer) XtInputReadMask,
	    ProcessIceMsgProc,
	    (XtPointer) iceConn);
}



int
my_error (display, event)

Display *display;
XErrorEvent *event;

{
    fprintf (stderr, "protocol error!!!!!!!!!!!!\n");
    exit (1);
}



void
CheckFirst ()

{
    Window dontCare1, dontCare2, *children, client_window;
    unsigned int nchildren, i;
    XCreateWindowEvent event;

    XGrabServer (disp);
    XSync (disp, 0);

    XQueryTree (disp, root, &dontCare1, &dontCare2, &children, &nchildren);

    for (i = 0; i < nchildren; i++)
    {
	event.window = children[i];
	HandleCreate (&event);

	client_window = XmuClientWindow (disp, children[i]);

	if (client_window != children[i])
	{
	    event.window = client_window;
	    HandleCreate (&event);
	}
    }
    
    XUngrabServer (disp);
    XSync (disp, 0);
}



main (argc, argv)

int argc;
char **argv;

{
    char *restore_filename = NULL;
    char *client_id = NULL;
    int i, zero = 0;

    Argc = argc;
    Argv = argv;

    for (i = 1; i < argc; i++)
    {
	if (argv[i][0] == '-')
	{
	    switch (argv[i][1])
	    {
	      case 'd':				/* -debug */
		debug = 1;
		continue;

	      case 'c':				/* -clientId */
		if (++i >= argc) goto usage;
		client_id = argv[i];
		continue;

	      case 'r':				/* -restore */
		if (++i >= argc) goto usage;
		restore_filename = argv[i];
		continue;
	    }
	}

    usage:

	fprintf (stderr,
	    "usage:  %s [-clientId id] [-restore file] [-debug]\n", argv[0]);
	exit (1);
    }


    XtToolkitInitialize ();
    appContext = XtCreateApplicationContext ();

    if (!(disp = XtOpenDisplay (appContext, NULL, "SM-PROXY", "SM-PROXY",
	NULL, 0, &zero, NULL)))
    {
	fprintf (stderr, "smproxy: unable to open display\n");
	exit (1);
    }

#if 0
    XSetErrorHandler (my_error);
#endif

    if (restore_filename)
	ReadProxyFile (restore_filename);

    ConnectProxyToSM (client_id);

    root = DefaultRootWindow (disp);

    wmProtocolsAtom = XInternAtom (disp, "WM_PROTOCOLS", False);
    wmSaveYourselfAtom = XInternAtom (disp, "WM_SAVE_YOURSELF", False);
    wmStateAtom = XInternAtom (disp, "WM_STATE", False);
    smClientIdAtom = XInternAtom (disp, "SM_CLIENT_ID", False);

    XSelectInput (disp, root, SubstructureNotifyMask | PropertyChangeMask);

    CheckFirst ();

    while (1)
    {
	XEvent event;

	XtAppNextEvent (appContext, &event);

	switch (event.type)
	{
	case CreateNotify:
	    HandleCreate (&event.xcreatewindow);
	    break;

	case DestroyNotify:
	    HandleDestroy (&event.xdestroywindow);
	    break;

	case PropertyNotify:
	    HandleUpdate (&event.xproperty);
	    break;

	default:
	    XtDispatchEvent (&event);
	    break;
	}
    }
}
