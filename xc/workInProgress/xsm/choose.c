/* $XConsortium: choose.c,v 1.24 95/05/24 20:43:29 mor Exp $ */
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

#include "xsm.h"
#include "saveutil.h"
#include "lock.h"
#include <sys/types.h>

#include <X11/Shell.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/List.h>
#include <X11/Xaw/Command.h>

#ifndef X_NOT_POSIX
#include <dirent.h>
#else
#ifdef SYSV
#include <dirent.h>
#else
#ifdef USG
#include <dirent.h>
#else
#include <sys/dir.h>
#ifndef dirent
#define dirent direct
#endif
#endif
#endif
#endif

static Pixel save_message_foreground;
static Pixel save_message_background;

static int delete_session_phase = 0;
static int break_lock_phase = 0;

Widget chooseSessionPopup;
Widget chooseSessionForm;
Widget chooseSessionLabel;
Widget chooseSessionListWidget;
Widget chooseSessionMessageLabel;
Widget chooseSessionLoadButton;
Widget chooseSessionDeleteButton;
Widget chooseSessionBreakLockButton;
Widget chooseSessionFailSafeButton;
Widget chooseSessionCancelButton;



int
GetSessionNames (count_ret, short_names_ret, long_names_ret, locked_ret)

int *count_ret;
String **short_names_ret;
String **long_names_ret;
Bool **locked_ret;

{
    DIR *dir;
    struct dirent *entry;
    char *path;
    int count;

    path = (char *) getenv ("SM_SAVE_DIR");
    if (!path)
    {
	path = (char *) getenv ("HOME");
	if (!path)
	    path = ".";
    }
    
    *count_ret = 0;
    *short_names_ret = NULL;
    *locked_ret = NULL;
    if (long_names_ret)
	*long_names_ret = NULL;

    if ((dir = opendir (path)) == NULL)
	return 0;

    count = 0;

    while ((entry = readdir (dir)) != NULL)
    {
	if (strncmp (entry->d_name, ".XSM-", 5) == 0)
	    count++;
    }

    if (count == 0 ||
       ((*short_names_ret = (String *) XtMalloc (
           count * sizeof (String))) == NULL) ||
       (long_names_ret && (*long_names_ret =
           (String *) XtMalloc (count * sizeof (String))) == NULL) ||
       ((*locked_ret = (Bool *) XtMalloc (count * sizeof (Bool))) == NULL))
    {
	closedir (dir);
	if (*short_names_ret)
	    XtFree ((char *) *short_names_ret);
	if (long_names_ret && *long_names_ret)
	    XtFree ((char *) *long_names_ret);
	return 0;
    }

    rewinddir (dir);

    while ((entry = readdir (dir)) != NULL && *count_ret < count)
    {
	if (strncmp (entry->d_name, ".XSM-", 5) == 0)
	{
	    char *name = (char *) entry->d_name + 5;
	    char *id = NULL;
	    Bool locked = CheckSessionLocked (name, long_names_ret, &id);

	    (*short_names_ret)[*count_ret] = XtNewString (name);
	    (*locked_ret)[*count_ret] = locked;

	    if (long_names_ret)
	    {
		if (!locked)
		{
		    (*long_names_ret)[*count_ret] =
			(*short_names_ret)[*count_ret];
		}
		else
		{
		    char *host = ((char *) strchr (id, '/')) + 1;
		    char *colon = (char *) strchr (host, ':');

		    *colon = '\0';

		    (*long_names_ret)[*count_ret] =
		        XtMalloc (strlen (name) + strlen (host) + 14);

		    sprintf ((*long_names_ret)[*count_ret],
		        "%s (locked at %s)", name, host);
		    *colon = ':';

		    XtFree (id);
		}
	    }

	    (*count_ret)++;
	}
    }

    closedir (dir);

    return 1;
}



void
FreeSessionNames (count, namesShort, namesLong, lockFlags)

int count;
String *namesShort;
String *namesLong;
Bool *lockFlags;

{
    int i;

    for (i = 0; i < count; i++)
	XtFree ((char *) namesShort[i]);
    XtFree ((char *) namesShort);

    if (namesLong)
    {
	for (i = 0; i < count; i++)
	    if (lockFlags[i])
		XtFree ((char *) namesLong[i]);
	XtFree ((char *) namesLong);
    }

    XtFree ((char *) lockFlags);
}



static void
SessionSelected (number, highlight)

int number;
Bool highlight;

{
    if (number >= 0)
    {
	Bool locked = sessionsLocked[number];

	if (highlight)
	    XawListHighlight (chooseSessionListWidget, number);

	XtSetSensitive (chooseSessionLoadButton, !locked);
	XtSetSensitive (chooseSessionDeleteButton, !locked);
	XtSetSensitive (chooseSessionBreakLockButton, locked);
    }
    else
    {
	XtSetSensitive (chooseSessionLoadButton, False);
	XtSetSensitive (chooseSessionDeleteButton, False);
	XtSetSensitive (chooseSessionBreakLockButton, False);
    }
}



static void
AddSessionNames (count, names)

int count;
String *names;

{
    int i;

    XawListChange (chooseSessionListWidget, names, count, 0, True);

    /*
     * Highlight the first unlocked session, if any.
     */

    for (i = 0; i < sessionNameCount; i++)
	if (!sessionsLocked[i])
	    break;

    SessionSelected (i < sessionNameCount ? i : -1, True);
}



void
ChooseWindowStructureNotifyXtHandler (w, closure, event, continue_to_dispatch)

Widget w;
XtPointer closure;
XEvent *event;
Boolean *continue_to_dispatch;

{
    if (event->type == MapNotify)
    {
	/*
	 * Set the input focus to the choose window and direct all keyboard
	 * events to the list widget.  This way, the user can make selections
	 * using the keyboard.
	 */

	XtSetKeyboardFocus (chooseSessionPopup, chooseSessionListWidget);

	XSetInputFocus (XtDisplay (topLevel), XtWindow (chooseSessionPopup),
	    RevertToPointerRoot, CurrentTime);

	XSync (XtDisplay (topLevel), 0);

	XtRemoveEventHandler (chooseSessionPopup, StructureNotifyMask, False,
	    ChooseWindowStructureNotifyXtHandler, NULL);
    }
}


void
ChooseSession ()

{
    Dimension   width, height;
    Position	x, y;


    /*
     * Add the session names to the list
     */

    AddSessionNames (sessionNameCount, sessionNamesLong);


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

    XtVaSetValues (chooseSessionListWidget,
	XtNlongest, width,
	NULL);

    XtVaSetValues (chooseSessionLabel,
	XtNwidth, width,
	NULL);

    XtVaGetValues (chooseSessionMessageLabel,
	XtNforeground, &save_message_foreground,
	XtNbackground, &save_message_background,
	NULL);

    XtVaSetValues (chooseSessionMessageLabel,
	XtNwidth, width,
	XtNforeground, save_message_background,
	NULL);

    /*
     * Wait for a map notify on the popup, then set input focus.
     */

    XtAddEventHandler (chooseSessionPopup, StructureNotifyMask, False,
	ChooseWindowStructureNotifyXtHandler, NULL);

    XtPopup (chooseSessionPopup, XtGrabNone);
}



static void
CheckDeleteCancel ()

{
    if (delete_session_phase > 0)
    {
	XtVaSetValues (chooseSessionMessageLabel,
	    XtNforeground, save_message_background,
            NULL);

	delete_session_phase = 0;
    }
}


static void
CheckBreakLockCancel ()

{
    if (break_lock_phase > 0)
    {
	XtVaSetValues (chooseSessionMessageLabel,
	    XtNforeground, save_message_background,
            NULL);

	break_lock_phase = 0;
    }
}



static void
ChooseSessionUp (w, event, params, numParams)

Widget w;
XEvent *event;
String *params;
Cardinal *numParams;

{
    XawListReturnStruct *current;
    
    CheckDeleteCancel ();
    CheckBreakLockCancel ();

    current = XawListShowCurrent (chooseSessionListWidget);
    if (current->list_index > 0)
	SessionSelected (current->list_index - 1, True);
    XtFree ((char *) current);
}


static void
ChooseSessionDown (w, event, params, numParams)

Widget w;
XEvent *event;
String *params;
Cardinal *numParams;

{
    XawListReturnStruct *current;
    
    CheckDeleteCancel ();
    CheckBreakLockCancel ();

    current = XawListShowCurrent (chooseSessionListWidget);
    if (current->list_index < sessionNameCount - 1)
	SessionSelected (current->list_index + 1, True);
    XtFree ((char *) current);
}



static void
ChooseSessionBtn1Down (w, event, params, numParams)

Widget w;
XEvent *event;
String *params;
Cardinal *numParams;

{
    XawListReturnStruct *current;

    CheckDeleteCancel ();
    CheckBreakLockCancel ();

    current = XawListShowCurrent (chooseSessionListWidget);
    SessionSelected (current->list_index, False /* already highlighted */);
    XtFree ((char *) current);
}



static void
ChooseSessionLoadXtProc (w, client_data, callData)

Widget		w;
XtPointer 	client_data;
XtPointer 	callData;

{
    XawListReturnStruct *current;

    CheckDeleteCancel ();
    CheckBreakLockCancel ();

    current = XawListShowCurrent (chooseSessionListWidget);

    if (!current || !current->string || *(current->string) == '\0')
    {
	if (current)
	    XtFree ((char *) current);
	XBell (XtDisplay (topLevel), 0);
	return;
    }

    /*
     * Pop down choice of sessions and start the specified session.
     */

    XtPopdown (chooseSessionPopup);

    if (session_name)
	XtFree (session_name);

    session_name = XtNewString (current->string);

    XtFree ((char *) current);

    FreeSessionNames (sessionNameCount,
	sessionNamesShort, sessionNamesLong, sessionsLocked);


    /*
     * Start the session, looking for .XSM-<session name> startup file.
     */

    if (!StartSession (session_name, False))
	UnableToLockSession (session_name);
}



static void
ChooseSessionDeleteXtProc (w, client_data, callData)

Widget		w;
XtPointer 	client_data;
XtPointer 	callData;

{
    XawListReturnStruct *current;
    int longest;
    char *name;

    CheckBreakLockCancel ();

    current = XawListShowCurrent (chooseSessionListWidget);

    if (!current || !(name = current->string) || *name == '\0')
    {
	if (current)
	    XtFree ((char *) current);
	XBell (XtDisplay (topLevel), 0);
	return;
    }

    delete_session_phase++;

    if (delete_session_phase == 1)
    {
	XtVaSetValues (chooseSessionMessageLabel,
	    XtNforeground, save_message_foreground,
            NULL);

	XBell (XtDisplay (topLevel), 0);
    }
    else
    {
	XtVaSetValues (chooseSessionMessageLabel,
	    XtNforeground, save_message_background,
            NULL);

	if (DeleteSession (name))
	{
	    int i, j;

	    for (i = 0; i < sessionNameCount; i++)
	    {
		if (strcmp (sessionNamesLong[i], name) == 0)
		{
		    XtFree ((char *) sessionNamesShort[i]);

		    if (sessionsLocked[i])
			XtFree ((char *) sessionNamesLong[i]);

		    for (j = i; j < sessionNameCount - 1; j++)
		    {
			sessionNamesLong[j] = sessionNamesLong[j + 1];
			sessionNamesShort[j] = sessionNamesShort[j + 1];
			sessionsLocked[j] = sessionsLocked[j + 1];
		    }
		    sessionNameCount--;
		    break;
		}
	    }

	    if (sessionNameCount == 0)
	    {
		XtSetSensitive (chooseSessionLoadButton, 0);
		XtSetSensitive (chooseSessionDeleteButton, 0);
		XtUnmanageChild (chooseSessionListWidget);
	    }
	    else
	    {
		XtVaGetValues (chooseSessionListWidget,
		    XtNlongest, &longest,
		    NULL);

		XawListChange (chooseSessionListWidget,
		    sessionNamesLong, sessionNameCount, longest, True);

		SessionSelected (-1, False);
	    }
	}

	delete_session_phase = 0;
    }

    XtFree ((char *) current);
}



static void
ChooseSessionBreakLockXtProc (w, client_data, callData)

Widget		w;
XtPointer 	client_data;
XtPointer 	callData;

{
    XawListReturnStruct *current;
    char *name;

    CheckDeleteCancel ();

    current = XawListShowCurrent (chooseSessionListWidget);

    if (!current || !(name = current->string) || *name == '\0')
    {
	if (current)
	    XtFree ((char *) current);
	XBell (XtDisplay (topLevel), 0);
	return;
    }

    break_lock_phase++;

    if (break_lock_phase == 1)
    {
	XtVaSetValues (chooseSessionMessageLabel,
	    XtNforeground, save_message_foreground,
            NULL);

	XBell (XtDisplay (topLevel), 0);
    }
    else
    {
	char *id;
	int longest;

	XtVaSetValues (chooseSessionMessageLabel,
	    XtNforeground, save_message_background,
            NULL);

	name = sessionNamesShort[current->list_index];

	id = GetLockId (name);

	UnlockSession (name);

	sessionsLocked[current->list_index] = False;
	XtFree ((char *) sessionNamesLong[current->list_index]);
	sessionNamesLong[current->list_index] =
	    sessionNamesShort[current->list_index];

	XtVaGetValues (chooseSessionListWidget,
	    XtNlongest, &longest,
	    NULL);

	XawListChange (chooseSessionListWidget,
	    sessionNamesLong, sessionNameCount, longest, True);

	SessionSelected (current->list_index, True);

	break_lock_phase = 0;
    }

    XtFree ((char *) current);
}



static void
ChooseSessionFailSafeXtProc (w, client_data, callData)

Widget		w;
XtPointer 	client_data;
XtPointer 	callData;

{
    /*
     * Pop down choice of sessions, and start the fail safe session.
     */

    CheckDeleteCancel ();
    CheckBreakLockCancel ();

    XtPopdown (chooseSessionPopup);

    if (session_name)
	XtFree (session_name);

    session_name = XtNewString (FAILSAFE_SESSION_NAME);

    FreeSessionNames (sessionNameCount,
	sessionNamesShort, sessionNamesLong, sessionsLocked);


    /*
     * We don't need to check return value of StartSession in this case,
     * because we are using the default session, and StartSession will
     * not try to lock the session at this time.  It will try to lock
     * it as soon as the user gives the session a name.
     */

    StartSession (session_name,
	True /* Use ~/.xsmstartup if found, else system.xsm */);
}



static void
ChooseSessionCancelXtProc (w, client_data, callData)

Widget		w;
XtPointer 	client_data;
XtPointer 	callData;

{
    if (delete_session_phase > 0 || break_lock_phase > 0)
    {
	XtVaSetValues (chooseSessionMessageLabel,
	    XtNforeground, save_message_background,
            NULL);

	delete_session_phase = 0;
	break_lock_phase = 0;
    }
    else
	EndSession (2);
}



void
create_choose_session_popup ()

{
    static XtActionsRec choose_actions[] = {
        {"ChooseSessionUp", ChooseSessionUp},
        {"ChooseSessionDown", ChooseSessionDown},
        {"ChooseSessionBtn1Down", ChooseSessionBtn1Down}
    };

    /*
     * Pop up for choosing session at startup
     */

    chooseSessionPopup = XtVaCreatePopupShell (
	"chooseSessionPopup", transientShellWidgetClass, topLevel,
	XtNallowShellResize, True,
	NULL);
    

    chooseSessionForm = XtVaCreateManagedWidget (
	"chooseSessionForm", formWidgetClass, chooseSessionPopup,
	NULL);


    chooseSessionLabel = XtVaCreateManagedWidget (
	"chooseSessionLabel", labelWidgetClass, chooseSessionForm,
        XtNfromHoriz, NULL,
        XtNfromVert, NULL,
        XtNborderWidth, 0,
	XtNresizable, True,
	XtNjustify, XtJustifyCenter,
	NULL);

    chooseSessionListWidget = XtVaCreateManagedWidget (
	"chooseSessionListWidget", listWidgetClass, chooseSessionForm,
	XtNresizable, True,
        XtNdefaultColumns, 1,
	XtNforceColumns, True,
        XtNfromHoriz, NULL,
        XtNfromVert, chooseSessionLabel,
	XtNvertDistance, 25,
	NULL);

    chooseSessionMessageLabel = XtVaCreateManagedWidget (
	"chooseSessionMessageLabel", labelWidgetClass, chooseSessionForm,
        XtNfromHoriz, NULL,
        XtNfromVert, chooseSessionListWidget,
        XtNborderWidth, 0,
	XtNresizable, True,
	XtNjustify, XtJustifyCenter,
	NULL);

    chooseSessionLoadButton = XtVaCreateManagedWidget (
	"chooseSessionLoadButton", commandWidgetClass, chooseSessionForm,
        XtNfromHoriz, NULL,
        XtNfromVert, chooseSessionMessageLabel,
        NULL);

    XtAddCallback (chooseSessionLoadButton, XtNcallback,
	ChooseSessionLoadXtProc, 0);

    chooseSessionDeleteButton = XtVaCreateManagedWidget (
	"chooseSessionDeleteButton", commandWidgetClass, chooseSessionForm,
        XtNfromHoriz, chooseSessionLoadButton,
        XtNfromVert, chooseSessionMessageLabel,
        NULL);

    XtAddCallback (chooseSessionDeleteButton, XtNcallback,
	ChooseSessionDeleteXtProc, 0);

    chooseSessionBreakLockButton = XtVaCreateManagedWidget (
	"chooseSessionBreakLockButton",
	commandWidgetClass, chooseSessionForm,
        XtNfromHoriz, chooseSessionDeleteButton,
        XtNfromVert, chooseSessionMessageLabel,
        NULL);

    XtAddCallback (chooseSessionBreakLockButton, XtNcallback,
	ChooseSessionBreakLockXtProc, 0);

    chooseSessionFailSafeButton = XtVaCreateManagedWidget (
	"chooseSessionFailSafeButton", commandWidgetClass, chooseSessionForm,
        XtNfromHoriz, chooseSessionBreakLockButton,
        XtNfromVert, chooseSessionMessageLabel,
        NULL);

    XtAddCallback (chooseSessionFailSafeButton, XtNcallback,
	ChooseSessionFailSafeXtProc, 0);


    chooseSessionCancelButton = XtVaCreateManagedWidget (
	"chooseSessionCancelButton", commandWidgetClass, chooseSessionForm,
        XtNfromHoriz, chooseSessionFailSafeButton,
        XtNfromVert, chooseSessionMessageLabel,
        NULL);

    XtAddCallback (chooseSessionCancelButton, XtNcallback,
	ChooseSessionCancelXtProc, 0);

    XtAppAddActions (appContext, choose_actions, XtNumber (choose_actions));

    XtInstallAllAccelerators (chooseSessionListWidget, chooseSessionPopup);
}
