/* $XConsortium: smproxy.c,v 1.9 94/07/05 12:35:24 mor Exp $ */
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
#include <stdio.h>

XtAppContext appContext;
Display *disp;
Window root;

Atom wmProtocolsAtom;
Atom wmSaveYourselfAtom;
Atom wmStateAtom;
Atom smClientIdAtom;

typedef struct {
    Window window;
    Bool waiting_for_required_props;
    Bool got_wm_state;
    SmcConn smc_conn;
    XtInputId input_id;
    char *client_id;
    char **wm_command;
    int wm_command_count;
    XClassHint class;
    char *wm_name;
    Bool has_save_yourself;
    Bool waiting_for_update;
} WinInfo;

typedef struct ProxyFileEntry
{
    struct ProxyFileEntry *next;
    int tag;
    char *client_id;
    XClassHint class;
    char *wm_name;
    int wm_command_count;
    char **wm_command;
} ProxyFileEntry;

ProxyFileEntry *proxyFileHead = NULL;

Bool debug = 0;

SmcConn proxy_smcConn;
XtInputId proxy_iceInputId;

WinInfo win_list[256];
int win_count = 0;
int proxy_count = 0;
int die_count = 0;

Bool ok_to_die = 0;

Bool gotFirstSave = 0;

int Argc;
char **Argv;



int
write_byte (file, b)

FILE		*file;
unsigned char   b;

{
    if (fwrite ((char *) &b, 1, 1, file) != 1)
	return 0;
    return 1;
}


int
write_short (file, s)

FILE		*file;
unsigned short	s;

{
    unsigned char   file_short[2];

    file_short[0] = (s & (unsigned)0xff00) >> 8;
    file_short[1] = s & 0xff;
    if (fwrite ((char *) file_short, (int) sizeof (file_short), 1, file) != 1)
	return 0;
    return 1;
}


int
write_counted_string (file, string)

FILE	*file;
char	*string;

{
    if (string)
    {
	unsigned char count = strlen (string);

	if (write_byte (file, count) == 0)
	    return 0;
	if (fwrite (string, (int) sizeof (char), (int) count, file) != count)
	    return 0;
    }
    else
    {
	if (write_byte (file, 0) == 0)
	    return 0;
    }

    return 1;
}



int
read_byte (file, bp)

FILE		*file;
unsigned char	*bp;

{
    if (fread ((char *) bp, 1, 1, file) != 1)
	return 0;
    return 1;
}


int
read_short (file, shortp)

FILE		*file;
unsigned short	*shortp;

{
    unsigned char   file_short[2];

    if (fread ((char *) file_short, (int) sizeof (file_short), 1, file) != 1)
	return 0;
    *shortp = file_short[0] * 256 + file_short[1];
    return 1;
}


int
read_counted_string (file, stringp)

FILE	*file;
char	**stringp;

{
    unsigned char  len;
    char	   *data;

    if (read_byte (file, &len) == 0)
	return 0;
    if (len == 0) {
	data = 0;
    } else {
    	data = (char *) malloc ((unsigned) len + 1);
    	if (!data)
	    return 0;
    	if (fread (data, (int) sizeof (char), (int) len, file) != len) {
	    free (data);
	    return 0;
    	}
	data[len] = '\0';
    }
    *stringp = data;
    return 1;
}



/*
 * An entry in the .smproxy file looks like this:
 *
 * FIELD				BYTES
 * -----                                ----
 * client ID len			1
 * client ID				LIST of bytes
 * WM_CLASS "res name" length		1
 * WM_CLASS "res name"			LIST of bytes
 * WM_CLASS "res class" length          1
 * WM_CLASS "res class"                 LIST of bytes
 * WM_NAME length			1
 * WM_NAME				LIST of bytes
 * WM_COMMAND arg count			1
 * For each arg in WM_COMMAND
 *    arg length			1
 *    arg				LIST of bytes
 */

int
WriteProxyFileEntry (proxyFile, theWindow)

FILE *proxyFile;
WinInfo *theWindow;

{
    int i;

    if (!write_counted_string (proxyFile, theWindow->client_id))
	return 0;
    if (!write_counted_string (proxyFile, theWindow->class.res_name))
	return 0;
    if (!write_counted_string (proxyFile, theWindow->class.res_class))
	return 0;
    if (!write_counted_string (proxyFile, theWindow->wm_name))
	return 0;
    
    if (!theWindow->wm_command || theWindow->wm_command_count == 0)
    {
	if (!write_byte (proxyFile, 0))
	    return 0;
    }
    else
    {
	if (!write_byte (proxyFile, (char) theWindow->wm_command_count))
	    return 0;
	for (i = 0; i < theWindow->wm_command_count; i++)
	    if (!write_counted_string (proxyFile, theWindow->wm_command[i]))
		return 0;
    }

    return 1;
}


int
ReadProxyFileEntry (proxyFile, pentry)

FILE *proxyFile;
ProxyFileEntry **pentry;

{
    ProxyFileEntry *entry;
    char byte;
    int i;

    *pentry = entry = (ProxyFileEntry *) malloc (
	sizeof (ProxyFileEntry));
    if (!*pentry)
	return 0;

    entry->tag = 0;
    entry->client_id = NULL;
    entry->class.res_name = NULL;
    entry->class.res_class = NULL;
    entry->wm_name = NULL;
    entry->wm_command = NULL;
    entry->wm_command_count = 0;

    if (!read_counted_string (proxyFile, &entry->client_id))
	goto give_up;
    if (!read_counted_string (proxyFile, &entry->class.res_name))
	goto give_up;
    if (!read_counted_string (proxyFile, &entry->class.res_class))
	goto give_up;
    if (!read_counted_string (proxyFile, &entry->wm_name))
	goto give_up;
    
    if (!read_byte (proxyFile, &byte))
	goto give_up;
    entry->wm_command_count = byte;

    if (entry->wm_command_count == 0)
	entry->wm_command = NULL;
    else
    {
	entry->wm_command = (char **) malloc (entry->wm_command_count *
	    sizeof (char *));

	if (!entry->wm_command)
	    goto give_up;

	for (i = 0; i < entry->wm_command_count; i++)
	    if (!read_counted_string (proxyFile, &entry->wm_command[i]))
		goto give_up;
    }

    return 1;

give_up:

    if (entry->client_id)
	free (entry->client_id);
    if (entry->class.res_name)
	free (entry->class.res_name);
    if (entry->class.res_class)
	free (entry->class.res_class);
    if (entry->wm_name)
	free (entry->wm_name);
    if (entry->wm_command_count)
    {
	for (i = 0; i < entry->wm_command_count; i++)
	    if (entry->wm_command[i])
		free (entry->wm_command[i]);
    }
    if (entry->wm_command)
	free ((char *) entry->wm_command);
    
    free ((char *) entry);
    *pentry = NULL;

    return 0;
}


void
ReadProxyFile ()

{
    FILE *proxyFile;
    char *home, filename[128];
    ProxyFileEntry *entry;
    int done = 0;

    home = (char *) getenv ("HOME");
    if (!home)
	home = ".";

    sprintf (filename, "%s/.smproxy", home);

    proxyFile = fopen (filename, "rb");
    if (!proxyFile)
	return;

    while (!done)
    {
	if (ReadProxyFileEntry (proxyFile, &entry))
	{
	    entry->next = proxyFileHead;
	    proxyFileHead = entry;
	}
	else
	    done = 1;
    }

    fclose (proxyFile);
}



char *
LookupClientID (theWindow)

WinInfo *theWindow;

{
    ProxyFileEntry *ptr;
    int found = 0;

    ptr = proxyFileHead;
    while (ptr && !found)
    {
	if (!ptr->tag &&
            strcmp (theWindow->class.res_name, ptr->class.res_name) == 0 &&
	    strcmp (theWindow->class.res_class, ptr->class.res_class) == 0 &&
	    strcmp (theWindow->wm_name, ptr->wm_name) == 0)
	{
	    int i;

	    if (theWindow->wm_command_count == ptr->wm_command_count)
	    {
		for (i = 0; i < theWindow->wm_command_count; i++)
		    if (strcmp (theWindow->wm_command[i],
			ptr->wm_command[i]) != 0)
			break;

		if (i == theWindow->wm_command_count)
		    found = 1;
	    }
	}

	if (!found)
	    ptr = ptr->next;
    }

    if (found)
    {
	ptr->tag = 1;
	return (ptr->client_id);
    }
    else
	return NULL;
}



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
    
    for (i = 0; i < winInfo->wm_command_count; i++)
    {
	prop1.vals[i].value = (SmPointer) winInfo->wm_command[i];
	prop1.vals[i].length = strlen (winInfo->wm_command[i]) + 1;
    }
    
    prop2.name = SmCloneCommand;
    prop2.type = SmLISTofARRAY8;
    prop2.num_vals = winInfo->wm_command_count;
    prop2.vals = prop1.vals;
    
    prop3.name = SmProgram;
    prop3.type = SmARRAY8;
    prop3.num_vals = 1;
    prop3.vals = &prop3val;
    prop3val.value = winInfo->wm_command[0];
    prop3val.length = strlen (winInfo->wm_command[0]) + 1;
    
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
    
    free (prop1.vals);
    
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
#if 1
	saveYourselfMessage.data.l[1] = CurrentTime;
#endif

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
	exit();
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
     * Select for Property Notify on the window
     */

    XSelectInput (disp, event->window, PropertyChangeMask);	


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
	win_list[index].class.res_name != NULL &
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
	 * WM_STATE, and WM_CLASS.
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
		win_list[i].class.res_name != NULL &
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
    char *home, filename[128];
    Bool success = True;
    int i;

    if (!gotFirstSave)
    {
	SmProp prop1, prop2, prop3, prop4, *props[4];
	SmPropValue prop3val, prop4val;
	char userId[20];
	int i;

	prop1.name = SmRestartCommand;
	prop1.type = SmLISTofARRAY8;
	prop1.num_vals = Argc;

	prop1.vals = (SmPropValue *) malloc (
	    Argc * sizeof (SmPropValue));

	for (i = 0; i < Argc; i++)
	{
	    prop1.vals[i].value = (SmPointer) Argv[i];
	    prop1.vals[i].length = strlen (Argv[i]) + 1;
	}

	prop2.name = SmCloneCommand;
	prop2.type = SmLISTofARRAY8;
	prop2.num_vals = Argc;
	prop2.vals = prop1.vals;
    
	prop3.name = SmProgram;
	prop3.type = SmARRAY8;
	prop3.num_vals = 1;
	prop3.vals = &prop3val;
	prop3val.value = Argv[0];
	prop3val.length = strlen (Argv[0]) + 1;

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

	free (prop1.vals);
	
	SmcSaveYourselfDone (smcConn, True);

	gotFirstSave = 1;
	return;
    }

    /* We should really get a phase 2 */

    home = (char *) getenv ("HOME");
    if (!home)
	home = ".";

    sprintf (filename, "%s/.smproxy", home);
    proxyFile = fopen (filename, "wb");

    for (i = 0; i < win_count; i++)
    {
	if (win_list[i].client_id)
	    if (!WriteProxyFileEntry (proxyFile, &win_list[i]))
		success = False;
    }

    fclose (proxyFile);

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
	exit();
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



ConnectProxyToSM ()

{
    char errorMsg[256];
    unsigned long mask;
    SmcCallbacks callbacks;
    IceConn iceConn;
    char *clientId;

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
	NULL,			/* previous ID */
	&clientId,
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
    printf ("protocol error!!!!!!!!!!!!\n");
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
    int zero = 0;

    Argc = argc;
    Argv = argv;

    if (argc > 1)
    {
	debug = (argc == 2 && strcmp (argv[1], "-debug") == 0);

	if (!debug)
	{
	    printf ("usage: smproxy [-debug]\n");
	    exit (1);
	}
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

    ReadProxyFile ();
    ConnectProxyToSM ();

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
