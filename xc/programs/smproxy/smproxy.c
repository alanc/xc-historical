/* $XConsortium: smproxy.c,v 1.20 94/07/26 12:38:04 mor Exp $ */
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

WinInfo *win_head = NULL;

int proxy_count = 0;
int die_count = 0;

Bool ok_to_die = 0;

Bool caught_error = 0;

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
    SmProp prop1, prop2, prop3, prop4, prop5, *props[5];
    SmPropValue prop3val, prop4val, prop5val;
    char userId[20], restartService[80];
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
    
    sprintf (restartService, "rstart-rsh/%s",
	(char *) winInfo->wm_client_machine.value);
    prop5.name = "_XC_RestartService";
    prop5.type = SmLISTofARRAY8;
    prop5.num_vals = 1;
    prop5.vals = &prop5val;
    prop5val.value = (SmPointer) restartService;
    prop5val.length = strlen (restartService);

    props[0] = &prop1;
    props[1] = &prop2;
    props[2] = &prop3;
    props[3] = &prop4;
    props[4] = &prop5;
    
    SmcSetProperties (winInfo->smc_conn, 5, props);
    
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
NullIceErrorHandler (iceConn, swap,
    offendingMinorOpcode, offendingSequence, errorClass, severity, values)

IceConn		iceConn;
Bool		swap;
int		offendingMinorOpcode;
unsigned long	offendingSequence;
int 		errorClass;
int		severity;
IcePointer	values;

{
    return;
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

    /*
     * In case a protocol error occurs when opening the connection,
     * (e.g. an authentication error), we set a null error handler
     * before the open, then restore the default handler after the open.
     */

    IceSetErrorHandler (NullIceErrorHandler);

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

    IceSetErrorHandler (NULL);

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
MyErrorHandler (display, event)

Display *display;
XErrorEvent *event;

{
    caught_error = 1;
}



Bool
LookupWindow (window, ptr_ret, prev_ptr_ret)

Window window;
WinInfo **ptr_ret;
WinInfo **prev_ptr_ret;

{
    WinInfo *ptr, *prev;

    ptr = win_head;
    prev = NULL;

    while (ptr)
    {
	if (ptr->window == window)
	    break;
	else
	{
	    prev = ptr;
	    ptr = ptr->next;
	}
    }

    if (ptr)
    {
	if (ptr_ret)
	    *ptr_ret = ptr;
	if (prev_ptr_ret)
	    *prev_ptr_ret = prev;
	return (1);
    }
    else
	return (0);
}



WinInfo *
AddNewWindow (window)

Window window;

{
    WinInfo *newptr;

    if (LookupWindow (window, NULL, NULL))
	return (NULL);

    newptr = (WinInfo *) malloc (sizeof (WinInfo));

    if (newptr == NULL)
	return (NULL);

    newptr->next = win_head;
    win_head = newptr;

    newptr->window = window;
    newptr->smc_conn = NULL;
    newptr->waiting_for_required_props = 1;
    newptr->got_wm_state = 0;
    newptr->client_id = NULL;
    newptr->wm_command = NULL;
    newptr->wm_command_count = 0;
    newptr->class.res_name = NULL;
    newptr->class.res_class = NULL;
    newptr->wm_name = NULL;
    newptr->wm_client_machine.value = NULL;
    newptr->wm_client_machine.nitems = 0;
    newptr->has_save_yourself = 0;
    newptr->waiting_for_update = 0;

    return (newptr);
}



void
RemoveWindow (winptr)

WinInfo *winptr;

{
    WinInfo *ptr, *prev;

    if (LookupWindow (winptr->window, &ptr, &prev))
    {
	if (prev == NULL)
	    win_head = ptr->next;
	else
	    prev->next = ptr->next;

	if (ptr->client_id)
	    free (ptr->client_id);
	
	if (ptr->wm_command)
	    XFreeStringList (ptr->wm_command);
	
	if (ptr->wm_name)
	    XFree (ptr->wm_name);
	
	if (ptr->wm_client_machine.value)
	    XFree (ptr->wm_client_machine.value);
	
	if (ptr->class.res_name)
	    XFree (ptr->class.res_name);
	
	if (ptr->class.res_class)
	    XFree (ptr->class.res_class);
	
	free ((char *) ptr);
    }
}



void
HandleCreate (event, errorCheck)

XCreateWindowEvent *event;
Bool errorCheck;

{
    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytesafter;
    unsigned long *datap = NULL;
    WinInfo *winptr;

    /*
     * Add the new window
     */

    if ((winptr = AddNewWindow (event->window)) == NULL)
	return;


    /*
     * Right after the window was created, it might have ben destroyed,
     * so the following Xlib calls might fail.  Need to catch the error
     * by installing an error handler.
     */

    if (errorCheck)
    {
	caught_error = 0;
	XSetErrorHandler (MyErrorHandler);
    }


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

    XFetchName (disp, event->window, &winptr->wm_name);

    XGetCommand (disp, event->window,
	&winptr->wm_command,
	&winptr->wm_command_count);

    XGetClassHint (disp, event->window, &winptr->class);

    if (XGetWindowProperty (disp, event->window, wmStateAtom,
	0L, 2L, False, AnyPropertyType,
	&actual_type, &actual_format, &nitems, &bytesafter,
	(unsigned char **) &datap) == Success && datap)
    {
	if (nitems > 0)
	    winptr->got_wm_state = 1;

	if (datap)
	    XFree ((char *) datap);
    }

    XGetWMClientMachine (disp, event->window, &winptr->wm_client_machine);

    if (winptr->got_wm_state &&
	winptr->wm_name != NULL &&
	winptr->wm_command != NULL &&
	winptr->wm_command_count > 0 &&
	winptr->class.res_name != NULL &&
	winptr->class.res_class != NULL &&
	winptr->wm_client_machine.value != NULL &&
	winptr->wm_client_machine.nitems != 0)
    {
	winptr->waiting_for_required_props = 0;

	if (!HasXSMPsupport (event->window))
	{
	    winptr->has_save_yourself =	HasSaveYourself (event->window);

	    ConnectClientToSM (winptr);
	}
    }

    if (errorCheck)
    {
	XSync (disp, 0);
	XSetErrorHandler (NULL);

	if (caught_error)
	{
	    caught_error = 0;
	    RemoveWindow (winptr);
	}
    }
}



void
HandleDestroy (event)

XDestroyWindowEvent *event;

{
    WinInfo *winptr;

    if (LookupWindow (event->window, &winptr, NULL))
    {
	if (winptr->smc_conn)
	{
	    SmcCloseConnection (winptr->smc_conn, 0, NULL);
	    XtRemoveInput (winptr->input_id);
	    proxy_count--;
	}

	if (debug)
	{
	    printf ("Removed window (window = 0x%x)\n", winptr->window);
	    printf ("\n");
	}

	RemoveWindow (winptr);
    }
}



void
HandleUpdate (event)

XPropertyEvent *event;

{
    Window window = event->window;
    WinInfo *winptr;

    if (event->atom != XA_WM_NAME && event->atom != XA_WM_COMMAND &&
	event->atom != XA_WM_CLASS && event->atom != XA_WM_CLIENT_MACHINE &&
	event->atom != wmStateAtom)
    {
	/*
	 * We are only interested in WM_NAME, WM_COMMAND,
	 * WM_CLASS, WM_CLIENT_MACHINE, and WM_STATE.
	 */

	return;
    }

    if (LookupWindow (window, &winptr, NULL))
    {
	if (event->atom == XA_WM_NAME)
	{
	    if (winptr->wm_name)
	    {
		XFree (winptr->wm_name);
		winptr->wm_name = NULL;
	    }

	    XFetchName (disp, window, &winptr->wm_name);
	}
	else if (event->atom == XA_WM_COMMAND)
	{
	    if (winptr->wm_command)
	    {
		XFreeStringList (winptr->wm_command);
		winptr->wm_command = NULL;
		winptr->wm_command_count = 0;
	    }

	    XGetCommand (disp, window,
		&winptr->wm_command,
		&winptr->wm_command_count);

	    if (winptr->waiting_for_update)
	    {
		/* Finish off the Save Yourself */

		winptr->waiting_for_update = 0;
		FinishSaveYourself (winptr);
	    }
	}
	else if (event->atom == XA_WM_CLASS)
	{
	    if (winptr->class.res_name)
	    {
		XFree (winptr->class.res_name);
		winptr->class.res_name = NULL;
	    }

	    if (winptr->class.res_class)
	    {
		XFree (winptr->class.res_class);
		winptr->class.res_class = NULL;
	    }

	    XGetClassHint (disp, window, &winptr->class);
	}
	else if (event->atom == XA_WM_CLIENT_MACHINE)
	{
	    if (winptr->wm_client_machine.value)
	    {
		XFree (winptr->wm_client_machine.value);
		winptr->wm_client_machine.value = NULL;
		winptr->wm_client_machine.nitems = 0;
	    }

	    XGetWMClientMachine (disp, event->window,
		&winptr->wm_client_machine);
	}
	else if (event->atom == wmStateAtom)
	{
	    winptr->got_wm_state = 1;
	}

	if (winptr->waiting_for_required_props)
	{
	    if (winptr->got_wm_state &&
		winptr->wm_name != NULL &&
		winptr->wm_command != NULL &&
		winptr->wm_command_count > 0 &&
		winptr->class.res_name != NULL &&
		winptr->class.res_class != NULL &&
		winptr->wm_client_machine.value != NULL &&
		winptr->wm_client_machine.nitems != 0)
	    {
		winptr->waiting_for_required_props = 0;
		
		if (!HasXSMPsupport (window))
		{
		    winptr->has_save_yourself = HasSaveYourself (window);

		    ConnectClientToSM (winptr);
		}
	    }
	}
    }
}



void
ProxySaveYourselfPhase2CB (smcConn, clientData)

SmcConn smcConn;
SmPointer clientData;

{
    FILE *proxyFile;
    char *path, *filename;
    Bool success = True;
    SmProp prop1, prop2, prop3, prop4, *props[4];
    SmPropValue prop2val, prop3val, prop4val;
    char discardCommand[80], userId[20];
    int numVals, i;
    WinInfo *winptr;

    path = getenv ("SM_SAVE_DIR");
    if (!path)
    {
	path = getenv ("HOME");
	if (!path)
	    path = ".";
    }

    filename = tempnam (path, ".PRX");
    proxyFile = fopen (filename, "wb");

    winptr = win_head;
    while (winptr)
    {
	if (winptr->client_id)
	    if (!WriteProxyFileEntry (proxyFile, winptr))
	    {
		success = False;
		break;
	    }

	winptr = winptr->next;
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

    prop2.name = SmProgram;
    prop2.type = SmARRAY8;
    prop2.num_vals = 1;
    prop2.vals = &prop2val;
    prop2val.value = Argv[0];
    prop2val.length = strlen (Argv[0]);

    sprintf (discardCommand, "rm %s", filename);
    prop3.name = SmDiscardCommand;
    prop3.type = SmARRAY8;
    prop3.num_vals = 1;
    prop3.vals = &prop3val;
    prop3val.value = (SmPointer) discardCommand;
    prop3val.length = strlen (discardCommand);

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

    SmcSetProperties (smcConn, 4, props);
    free ((char *) prop1.vals);

    SmcSaveYourselfDone (smcConn, success);
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
    /*
     * We want the proxy to respond to the Save Yourself after all
     * the regular XSMP clients have finished with the save (and possibly
     * interacted with the user).
     */

    if (!SmcRequestSaveYourselfPhase2 (smcConn,
	ProxySaveYourselfPhase2CB, NULL))
    {
	SmcSaveYourselfDone (smcConn, False);
    }
}



void
ProxyDieCB (smcConn, clientData)

SmcConn smcConn;
SmPointer clientData;

{
    ok_to_die = 1;

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
    SmcSaveYourselfDone (smcConn, False);
}



Status
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
	return (0);

    iceConn = SmcGetIceConnection (proxy_smcConn);

    proxy_iceInputId = XtAppAddInput (
	    appContext,
	    IceConnectionNumber (iceConn),
            (XtPointer) XtInputReadMask,
	    ProcessIceMsgProc,
	    (XtPointer) iceConn);

    return (1);
}



void
CheckForExistingWindows ()

{
    Window dontCare1, dontCare2, *children, client_window;
    unsigned int nchildren, i;
    XCreateWindowEvent event;

    /*
     * We query the root tree for all windows created thus far.
     * Since a window can be deleted after the query, we grab the
     * server to make sure this doesn't happen.  The alternative
     * is to catch bad window errors by installing an error handler.
     * This would require many XSyncs, so I'm not sure if this is
     * better than doing a grab.
     */

    XGrabServer (disp);
    XSync (disp, 0);

    XQueryTree (disp, root, &dontCare1, &dontCare2, &children, &nchildren);

    for (i = 0; i < nchildren; i++)
    {
	event.window = children[i];

	HandleCreate (&event, 0 /* don't error check, we did a grab */);

	client_window = XmuClientWindow (disp, children[i]);

	if (client_window != children[i])
	{
	    event.window = client_window;
	    HandleCreate (&event, 0);
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

    if (restore_filename)
	ReadProxyFile (restore_filename);

    if (!ConnectProxyToSM (client_id))
    {
	fprintf (stderr, "smproxy: unable to connect to session manager\n");
	exit (1);
    }

    root = DefaultRootWindow (disp);

    wmProtocolsAtom = XInternAtom (disp, "WM_PROTOCOLS", False);
    wmSaveYourselfAtom = XInternAtom (disp, "WM_SAVE_YOURSELF", False);
    wmStateAtom = XInternAtom (disp, "WM_STATE", False);
    smClientIdAtom = XInternAtom (disp, "SM_CLIENT_ID", False);

    XSelectInput (disp, root, SubstructureNotifyMask | PropertyChangeMask);

    CheckForExistingWindows ();

    while (1)
    {
	XEvent event;

	XtAppNextEvent (appContext, &event);

	switch (event.type)
	{
	case CreateNotify:
	    HandleCreate (&event.xcreatewindow, 1 /* error check */);
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
