/* $XConsortium$ */
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

#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/SM/SMlib.h>

XtAppContext appContext;
Display *disp;
Window root;

Atom wmSaveYourselfAtom;
Atom smClientIdAtom;

typedef struct {
    Window frame;
    Window window;
    Bool mapped;
    Bool has_save_yourself;
    SmcConn smc_conn;
    XtInputId input_id;
    char *client_id;
} WinInfo;

Bool debug = 0;

WinInfo win_list[100];
int win_count = 0;
int proxy_count = 0;
int die_count = 0;



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
HasWmCommand (window)

Window window;

{
    char **argv_ret = NULL;
    int argc_ret;

    if (!XGetCommand (disp, window, &argv_ret, &argc_ret))
	return (0);

    if (argv_ret)
	XFreeStringList (argv_ret);

    return (argc_ret > 0);
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



void SaveYourselfCB (smcConn, clientData,
    saveType, shutdown, interactStyle, fast)

SmcConn smcConn;
SmPointer clientData;
int saveType;
Bool shutdown;
int interactStyle;
Bool fast;

{
    WinInfo *winInfo = (WinInfo *) clientData;
    char **argv_ret = NULL;
    int argc_ret;

    if (winInfo->has_save_yourself)
    {
	/* Send WM_SAVE_YOURSELF */
    }

    if (!XGetCommand (disp, winInfo->window,
	&argv_ret, &argc_ret) || argc_ret == 0)
    {
	SmcSaveYourselfDone (winInfo->smc_conn, False);
    }
    else
    {
	SmProp prop1, prop2, prop3, *props[3];
	SmPropValue prop3val;
	int i;

	prop1.name = SmRestartCommand;
	prop1.type = SmLISTofARRAY8;
	prop1.num_vals = argc_ret;

	prop1.vals = (SmPropValue *) malloc (
	    argc_ret * sizeof (SmPropValue));

	for (i = 0; i < argc_ret; i++)
	{
	    prop1.vals[i].value = (SmPointer) argv_ret[i];
	    prop1.vals[i].length = strlen (argv_ret[i]) + 1;
	}

	prop2.name = SmCloneCommand;
	prop2.type = SmLISTofARRAY8;
	prop2.num_vals = argc_ret;
	prop2.vals = prop1.vals;

	prop3.name = SmProgram;
	prop3.type = SmARRAY8;
	prop3.num_vals = 1;
	prop3.vals = &prop3val;
	prop3val.value = argv_ret[0];
	prop3val.length = strlen (argv_ret[0]) + 1;

	props[0] = &prop1;
	props[1] = &prop2;
	props[2] = &prop3;

	SmcSetProperties (winInfo->smc_conn, 3, props);

	free (prop1.vals);
	XFreeStringList (argv_ret);

	SmcSaveYourselfDone (winInfo->smc_conn, True);
    }
}


void DieCB (smcConn, clientData)

SmcConn smcConn;
SmPointer clientData;

{
    WinInfo *winInfo = (WinInfo *) clientData;

    SmcCloseConnection (winInfo->smc_conn, 0, NULL);
    XtRemoveInput (winInfo->input_id);

    /* Now tell the client to die */

    if (debug)
	printf ("Trying to kill %x\n", winInfo->window);

    XSync (disp, 0);
    XKillClient (disp, winInfo->window);
    XSync (disp, 0);


    /* Proxy must exit when all clients die */

    die_count++;

    if (die_count == proxy_count)
	exit();
}


void SaveCompleteCB (smcConn, clientData)

SmcConn smcConn;
SmPointer clientData;

{
    WinInfo *winInfo = (WinInfo *) clientData;

}


void ShutdownCancelledCB (smcConn, clientData)

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
ConnectToSM (winInfo)

WinInfo *winInfo;

{
    char errorMsg[256];
    unsigned long mask;
    SmcCallbacks callbacks;
    IceConn ice_conn;

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

    winInfo->smc_conn = SmcOpenConnection (
	NULL, 			/* use SESSION_MANAGER env */
	(SmPointer) winInfo,	/* force a new connection */
	SmProtoMajor,
	SmProtoMinor,
	mask,
	&callbacks,
	NULL,			/* previous ID */
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
	printf ("Connected to SM, window = %x\n", winInfo->window);
	printf ("\n");
    }

    proxy_count++;
}



void
HandleMap (event, map)

XMapEvent *event;
Bool map;

{
    Window frame;
    Window window;
    int i;

    frame = event->window;

    for (i = 0; i < win_count; i++)
    {
	if (win_list[i].frame == frame)
	{
	    win_list[i].mapped = map;
	    break;
	}
    }

    if (i >= win_count)
    {
	Bool has_xsmp_support, has_save_yourself, has_wm_command;

	window = XmuClientWindow (disp, frame);

	has_xsmp_support = HasXSMPsupport (window);
	has_save_yourself = HasSaveYourself (window);
	has_wm_command = HasWmCommand (window);

	win_list[win_count].frame = frame;
	win_list[win_count].window = window;
	win_list[win_count].mapped = map;
	win_list[win_count].has_save_yourself = has_save_yourself;

	if (!has_xsmp_support && (has_save_yourself || has_wm_command))
	    ConnectToSM (&win_list[win_count]);
	else
	    win_list[win_count].smc_conn = NULL;

	if (debug)
	{
	    printf ("Added window\n");
	    printf ("    frame = %x\n", frame);
	    printf ("    window = %x\n", window);
	    printf ("    has SM_CLIENT_ID = %d\n", has_xsmp_support);
	    printf ("    has WM_SAVE_YOURSELF = %d\n", has_save_yourself);
	    printf ("    has WM_COMMAND = %d\n", has_wm_command);
	    printf ("\n");
	}

	win_count++;
    }
}


void
HandleDestroy (event)

XDestroyWindowEvent *event;

{
    Window frame;
    int i;

    frame = event->window;

    for (i = 0; i < win_count; i++)
    {
	if (win_list[i].frame == frame)
	{
	    if (win_list[i].smc_conn)
	    {
		SmcCloseConnection (win_list[i].smc_conn, 0, NULL);
		XtRemoveInput (win_list[i].input_id);
	    }

	    if (debug)
	    {
		printf ("Removed window (frame = %x, window = %x)\n",
		    win_list[i].frame, win_list[i].window);
		printf ("\n");
	    }

	    if (i < win_count - 1)
		win_list[i] = win_list[win_count - 1];

	    win_count--;
	    break;
	}
    }
}



main (argc, argv)

int argc;
char **argv;

{
    Widget topLevel;

    if (argc > 1)
    {
	debug = (argc == 2 && strcmp (argv[1], "-debug") == 0);

	if (!debug)
	{
	    printf ("usage: smproxy [-debug]\n");
	    exit (1);
	}
    }

    topLevel = XtAppInitialize (&appContext, "SM-PROXY", NULL, 0, &argc, argv,
	NULL, NULL, 0);

    disp =  XtDisplay (topLevel);
    root = DefaultRootWindow (disp);

    XSelectInput (disp, root, SubstructureNotifyMask);

    wmSaveYourselfAtom = XInternAtom (disp, "WM_SAVE_YOURSELF", False);
    smClientIdAtom = XInternAtom (disp, "SM_CLIENT_ID", False);

    while (1)
    {
	XEvent event;

	XtAppNextEvent (appContext, &event);

	if (event.type == MapNotify)
	    HandleMap (&event.xmap, 1);
	else if (event.type == UnmapNotify)
	    HandleMap (&event.xunmap, 0);
	else if (event.type == DestroyNotify)
	    HandleDestroy (&event.xdestroywindow);
	else
	    XtDispatchEvent (&event);
    }
}
