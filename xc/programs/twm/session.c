/* $XConsortium: session.c,v 1.8 94/07/26 12:39:34 mor Exp mor $ */
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

#include <X11/Xlib.h>
#include <X11/SM/SMlib.h>
#include <X11/Xatom.h>
#include <stdio.h>
#include "twm.h"
#include "screen.h"

SmcConn smcConn;
XtInputId iceInputId;
char *twm_clientId;
TWMWinConfigEntry *winConfigHead = NULL;
Bool gotFirstSave = 0;



char *
GetClientID (window)

Window window;

{
    char *client_id = NULL;
    Window client_leader;
    XTextProperty tp;
    Atom actual_type;
    int actual_format;
    unsigned long nitems;
    unsigned long nbytes;
    unsigned long bytes_after;
    unsigned char *prop = NULL;

    if (XGetWindowProperty (dpy, window, _XA_WM_CLIENT_LEADER,
	0L, 1L, False, AnyPropertyType,	&actual_type, &actual_format,
	&nitems, &bytes_after, &prop) == Success)
    {
	if (actual_type == XA_WINDOW && actual_format == 32 &&
	    nitems == 1 && bytes_after == 0)
	{
	    client_leader = *((Window *) prop);

	    if (XGetTextProperty (dpy, client_leader, &tp, _XA_SM_CLIENT_ID))
	    {
		if (tp.encoding == XA_STRING &&
		    tp.format == 8 && tp.nitems != 0)
		    client_id = (char *) tp.value;
	    }
	}

	if (prop)
	    XFree (prop);
    }
    
    return client_id;
}



char *
GetWindowRole (window)

Window window;

{
    XTextProperty tp;

    if (XGetTextProperty (dpy, window, &tp, _XA_WM_WINDOW_ROLE))
    {
	if (tp.encoding == XA_STRING && tp.format == 8 && tp.nitems != 0)
	    return ((char *) tp.value);
    }

    return NULL;
}



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
    	data = malloc ((unsigned) len + 1);
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
 * An entry in the saved window config file looks like this:
 *
 * FIELD				BYTES
 * -----                                ----
 * SM_CLIENT_ID ID len			1	       (may be 0)
 * SM_CLIENT_ID				LIST of bytes  (may be NULL)
 *
 * WM_WINDOW_ROLE length		1	       (may be 0)
 * WM_WINDOW_ROLE			LIST of bytes  (may be NULL)
 *
 * if no WM_WINDOW_ROLE (length = 0)
 *
 *   WM_CLASS "res name" length		1
 *   WM_CLASS "res name"		LIST of bytes
 *   WM_CLASS "res class" length        1
 *   WM_CLASS "res class"               LIST of bytes
 *   WM_NAME length			1		(0 if name changed)
 *   WM_NAME				LIST of bytes
 *   WM_COMMAND arg count		1      		(0 if no SM_CLIENT_ID)
 *   For each arg in WM_COMMAND
 *      arg length			1
 *      arg				LIST of bytes
 *
 * Iconified bool			1
 * Geom x				2
 * Geom y				2
 * Geom width				2
 * Geom height				2
 */

int
WriteWinConfigEntry (configFile, theWindow, clientId, windowRole)

FILE *configFile;
TwmWindow *theWindow;
char *clientId;
char *windowRole;

{
    char **wm_command;
    int wm_command_count, i;

    if (!write_counted_string (configFile, clientId))
	return 0;

    if (!write_counted_string (configFile, windowRole))
	return 0;

    if (!windowRole)
    {
	if (!write_counted_string (configFile, theWindow->class.res_name))
	    return 0;
	if (!write_counted_string (configFile, theWindow->class.res_class))
	    return 0;
	if (theWindow->nameChanged)
	{
	    /*
	     * If WM_NAME changed on this window, we can't use it as
	     * a criteria for looking up window configurations.  See the
	     * longer explanation in the GetWindowConfig() function below.
	     */

	    if (!write_counted_string (configFile, NULL))
		return 0;
	}
	else
	{
	    if (!write_counted_string (configFile, theWindow->name))
		return 0;
	}
    
	wm_command = NULL;
	wm_command_count = 0;
	XGetCommand (dpy, theWindow->w, &wm_command, &wm_command_count);

	if (clientId || !wm_command || wm_command_count == 0)
	{
	    if (!write_byte (configFile, 0))
		return 0;
	}
	else
	{
	    if (!write_byte (configFile, (char) wm_command_count))
		return 0;
	    for (i = 0; i < wm_command_count; i++)
		if (!write_counted_string (configFile, wm_command[i]))
		    return 0;
	    XFreeStringList (wm_command);
	}
    }

    if (!write_byte (configFile, theWindow->icon ? 1 : 0))
	return 0;
    if (!write_short (configFile, (unsigned short) theWindow->frame_x))
	return 0;
    if (!write_short (configFile, (unsigned short) theWindow->frame_y))
	return 0;
    if (!write_short (configFile, (unsigned short) theWindow->attr.width))
	return 0;
    if (!write_short (configFile, (unsigned short) theWindow->attr.height))
	return 0;

    return 1;
}


int
ReadWinConfigEntry (configFile, pentry)

FILE *configFile;
TWMWinConfigEntry **pentry;

{
    TWMWinConfigEntry *entry;
    char byte;
    int i;

    *pentry = entry = (TWMWinConfigEntry *) malloc (
	sizeof (TWMWinConfigEntry));
    if (!*pentry)
	return 0;

    entry->tag = 0;
    entry->client_id = NULL;
    entry->window_role = NULL;
    entry->class.res_name = NULL;
    entry->class.res_class = NULL;
    entry->wm_name = NULL;
    entry->wm_command = NULL;
    entry->wm_command_count = 0;

    if (!read_counted_string (configFile, &entry->client_id))
	goto give_up;

    if (!read_counted_string (configFile, &entry->window_role))
	goto give_up;

    if (!entry->window_role)
    {
	if (!read_counted_string (configFile, &entry->class.res_name))
	    goto give_up;
	if (!read_counted_string (configFile, &entry->class.res_class))
	    goto give_up;
	if (!read_counted_string (configFile, &entry->wm_name))
	    goto give_up;
    
	if (!read_byte (configFile, &byte))
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
		if (!read_counted_string (configFile, &entry->wm_command[i]))
		    goto give_up;
	}
    }

    if (!read_byte (configFile, &byte))
	goto give_up;
    entry->iconified = byte;

    if (!read_short (configFile, &entry->x))
	goto give_up;
    if (!read_short (configFile, &entry->y))
	goto give_up;
    if (!read_short (configFile, &entry->width))
	goto give_up;
    if (!read_short (configFile, &entry->height))
	goto give_up;

    return 1;

give_up:

    if (entry->client_id)
	free (entry->client_id);
    if (entry->window_role)
	free (entry->window_role);
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
ReadWinConfigFile (filename)

char *filename;

{
    FILE *configFile;
    TWMWinConfigEntry *entry;
    int done = 0;

    configFile = fopen (filename, "rb");
    if (!configFile)
	return;

    while (!done)
    {
	if (ReadWinConfigEntry (configFile, &entry))
	{
	    entry->next = winConfigHead;
	    winConfigHead = entry;
	}
	else
	    done = 1;
    }

    fclose (configFile);
}



int
GetWindowConfig (theWindow, x, y, width, height, iconified)

TwmWindow *theWindow;
unsigned short *x, *y, *width, *height;
Bool *iconified;

{
    char *clientId, *windowRole;
    TWMWinConfigEntry *ptr;
    int found = 0;

    ptr = winConfigHead;

    if (!ptr)
	return 0;

    clientId = GetClientID (theWindow->w);
    windowRole = GetWindowRole (theWindow->w);

    while (ptr && !found)
    {
	int client_id_match = (!clientId && !ptr->client_id) ||
	    (clientId && ptr->client_id &&
	    strcmp (clientId, ptr->client_id) == 0);

	if (!ptr->tag && client_id_match)
	{
	    if (windowRole || ptr->window_role)
	    {
		found = (windowRole && ptr->window_role &&
		    strcmp (windowRole, ptr->window_role) == 0);
	    }
	    else
	    {
		/*
		 * Compare WM_CLASS + only compare WM_NAME if the
		 * WM_NAME in the saved file is non-NULL.  If the
		 * WM_NAME in the saved file is NULL, this means that
		 * the client changed the value of WM_NAME during the
		 * session, and we can not use it as a criteria for
		 * our search.  For example, with xmh, at save time
		 * the window name might be "xmh: folderY".  However,
		 * if xmh does not properly restore state when it is
		 * restarted, the initial window name might be
		 * "xmh: folderX".  This would cause the window manager
		 * to fail in finding the saved window configuration.
		 * The best we can do is ignore WM_NAME if its value
		 * changed in the previous session.
		 */

		if (strcmp (theWindow->class.res_name,
		        ptr->class.res_name) == 0 &&
		    strcmp (theWindow->class.res_class,
			ptr->class.res_class) == 0 &&
	    	   (ptr->wm_name == NULL ||
		    strcmp (theWindow->name, ptr->wm_name) == 0))
		{
		    if (clientId)
		    {
			/*
			 * If a client ID was present, we should not check
			 * WM_COMMAND because Xt will put a -xtsessionID arg
			 * on the command line.
			 */

			found = 1;
		    }
		    else
		    {
			/*
			 * For non-XSMP clients, also check WM_COMMAND.
			 */

			char **wm_command = NULL;
			int wm_command_count = 0, i;

			XGetCommand (dpy, theWindow->w,
		            &wm_command, &wm_command_count);

			if (wm_command_count == ptr->wm_command_count)
			{
			    for (i = 0; i < wm_command_count; i++)
				if (strcmp (wm_command[i],
				    ptr->wm_command[i]) != 0)
				    break;

			    if (i == wm_command_count)
				found = 1;
			}
		    }
		}
	    }
	}

	if (!found)
	    ptr = ptr->next;
    }

    if (found)
    {
	*x = ptr->x;
	*y = ptr->y;
	*width = ptr->width;
	*height = ptr->height;
	*iconified = ptr->iconified;
	ptr->tag = 1;
    }
    else
	*iconified = 0;

    if (clientId)
	XFree (clientId);

    if (windowRole)
	XFree (windowRole);

    return found;
}



void
SaveYourselfPhase2CB (smcConn, clientData)

SmcConn smcConn;
SmPointer clientData;

{
    int scrnum;
    ScreenInfo *theScreen;
    TwmWindow *theWindow;
    char *clientId, *windowRole;
    FILE *configFile;
    char *path, *filename;
    Bool success = True;
    SmProp prop1, prop2, prop3, prop4, *props[4];
    SmPropValue prop2val, prop3val, prop4val;
    char discardCommand[80], userId[20];
    int numVals, i;
    char yes = 1;

    path = getenv ("SM_SAVE_DIR");
    if (!path)
    {
	path = getenv ("HOME");
	if (!path)
	    path = ".";
    }

    filename = tempnam (path, ".twm");
    configFile = fopen (filename, "wb");

    for (scrnum = 0; scrnum < NumScreens && success; scrnum++)
    {
	if (ScreenList[scrnum] != NULL)
	{
	    theScreen = ScreenList[scrnum];
	    theWindow = theScreen->TwmRoot.next;

	    while (theWindow && success)
	    {
		clientId = GetClientID (theWindow->w);
		windowRole = GetWindowRole (theWindow->w);

		if (!WriteWinConfigEntry (configFile, theWindow,
		    clientId, windowRole))
		    success = False;

		if (clientId)
		    XFree (clientId);

		if (windowRole)
		    XFree (windowRole);

		theWindow = theWindow->next;
	    }
	}
    }
    
    fclose (configFile);

    prop1.name = SmRestartCommand;
    prop1.type = SmLISTofARRAY8;

    prop1.vals = (SmPropValue *) malloc (
	(Argc + 4) * sizeof (SmPropValue));

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

    prop1.vals[numVals].value = (SmPointer) twm_clientId;
    prop1.vals[numVals++].length = strlen (twm_clientId);

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
    free (prop1.vals);

    SmcSaveYourselfDone (smcConn, success);
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
    if (!SmcRequestSaveYourselfPhase2 (smcConn, SaveYourselfPhase2CB, NULL))
	SmcSaveYourselfDone (smcConn, False);
}



void
DieCB (smcConn, clientData)

SmcConn smcConn;
SmPointer clientData;

{
    SmcCloseConnection (smcConn, 0, NULL);
    XtRemoveInput (iceInputId);
    Done();
}



void
SaveCompleteCB (smcConn, clientData)

SmcConn smcConn;
SmPointer clientData;

{
    ;
}



void
ShutdownCancelledCB (smcConn, clientData)

SmcConn smcConn;
SmPointer clientData;

{
    SmcSaveYourselfDone (smcConn, False);
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



ConnectToSessionManager (previous_id)

char *previous_id;

{
    char errorMsg[256];
    unsigned long mask;
    SmcCallbacks callbacks;
    IceConn iceConn;

    mask = SmcSaveYourselfProcMask | SmcDieProcMask |
	SmcSaveCompleteProcMask | SmcShutdownCancelledProcMask;

    callbacks.save_yourself.callback = SaveYourselfCB;
    callbacks.save_yourself.client_data = (SmPointer) NULL;

    callbacks.die.callback = DieCB;
    callbacks.die.client_data = (SmPointer) NULL;

    callbacks.save_complete.callback = SaveCompleteCB;
    callbacks.save_complete.client_data = (SmPointer) NULL;

    callbacks.shutdown_cancelled.callback = ShutdownCancelledCB;
    callbacks.shutdown_cancelled.client_data = (SmPointer) NULL;

    smcConn = SmcOpenConnection (
	NULL, 			/* use SESSION_MANAGER env */
	(SmPointer) appContext,
	SmProtoMajor,
	SmProtoMinor,
	mask,
	&callbacks,
	previous_id,
	&twm_clientId,
	256, errorMsg);

    if (smcConn == NULL)
	return;

    iceConn = SmcGetIceConnection (smcConn);

    iceInputId = XtAppAddInput (
	    appContext,
	    IceConnectionNumber (iceConn),
            (XtPointer) XtInputReadMask,
	    ProcessIceMsgProc,
	    (XtPointer) iceConn);
}



