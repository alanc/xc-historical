/* $XConsortium: choose.c,v 1.9 94/07/21 14:59:07 mor Exp $ */
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
#include <sys/types.h>
#include <dirent.h>

static Pixel save_message_foreground;
static Pixel save_message_background;

static int delete_phase = 0;


int
GetSessionNames (count_ret, names_ret)

int *count_ret;
String **names_ret;

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
    *names_ret = NULL;

    if ((dir = opendir (path)) == NULL)
	return 0;

    count = 0;

    while ((entry = readdir (dir)) != NULL)
    {
	if (strncmp (entry->d_name, ".XSM-", 4) == 0)
	    count++;
    }

    if (count == 0 ||
       (*names_ret = (String *) XtMalloc (count * sizeof (String))) == NULL)
    {
	closedir (dir);
	return 0;
    }

    rewinddir (dir);

    while ((entry = readdir (dir)) != NULL && *count_ret < count)
    {
	if (strncmp (entry->d_name, ".XSM-", 4) == 0)
	    (*names_ret)[(*count_ret)++] = XtNewString (entry->d_name + 4);
    }

    closedir (dir);

    return 1;
}



void
FreeSessionNames (count, names)

int count;
String *names;

{
    int i;

    for (i = 0; i < count; i++)
	XtFree ((char *) names[i]);

    XtFree ((char *) names);
}



static void
AddSessionNames (count, names)

int count;
String *names;

{
    XawListChange (chooseSessionListWidget, names, count, 0, True);
    XawListHighlight (chooseSessionListWidget, 0);
}


void
ChooseSession ()

{
    Dimension   width, height;
    Position	x, y;


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
     * Set the input focus to the choose window and direct all keyboard
     * events to the list widget.  This way, the user can make selections
     * using the keyboard.  Note that when the choose menu comes up, the
     * window manager should not be running yet.  So, it is safe for us to
     * set input focus.
     */

    XtPopup (chooseSessionPopup, XtGrabNone);

    XtSetKeyboardFocus (chooseSessionPopup, chooseSessionListWidget);

    XSetInputFocus (XtDisplay (topLevel), XtWindow (chooseSessionPopup),
	RevertToNone, CurrentTime);

    XSync (XtDisplay (topLevel), 0);
}



static void
ChooseSessionUp (w, event, params, numParams)

Widget w;
XEvent *event;
String *params;
Cardinal *numParams;

{
    XawListReturnStruct *current;
    
    current = XawListShowCurrent (chooseSessionListWidget);
    if (current->list_index > 0)
	XawListHighlight (chooseSessionListWidget, current->list_index - 1);
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
    
    current = XawListShowCurrent (chooseSessionListWidget);
    if (current->list_index < sessionNameCount - 1)
	XawListHighlight (chooseSessionListWidget, current->list_index + 1);
    XtFree ((char *) current);
}



static void
ChooseSessionLoadXtProc (w, client_data, callData)

Widget		w;
XtPointer 	client_data;
XtPointer 	callData;

{
    XawListReturnStruct *current;


    /*
     * Pop down choice of sessions and start the specified session.
     */

    XtPopdown (chooseSessionPopup);

    current = XawListShowCurrent (chooseSessionListWidget);

    if (session_name)
	XtFree (session_name);

    session_name = XtNewString (current->string);

    XtFree ((char *) current);

    FreeSessionNames (sessionNameCount, sessionNames);

    StartSession (session_name,
	False /* look for .XSM-<session name> startup file */);
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

    current = XawListShowCurrent (chooseSessionListWidget);

    if (!current)
	return;

    name = current->string;

    if (!name && *name == '\0')
    {
	XtFree ((char *) current);
	return;
    }

    delete_phase++;

    if (delete_phase == 1)
    {
	XtVaSetValues (chooseSessionMessageLabel,
	    XtNforeground, save_message_foreground,
            NULL);

	XBell (XtDisplay (topLevel), 0);
    }
    else
    {
	char filename[256];
	char *dir;
	int i, j;

	XtVaSetValues (chooseSessionMessageLabel,
	    XtNforeground, save_message_background,
            NULL);

	dir = (char *) getenv ("SM_SAVE_DIR");
	if (!dir)
	{
	    dir = (char *) getenv ("HOME");
	    if (!dir)
		dir = ".";
	}

	sprintf (filename, "%s/.XSM-%s", dir, name);

	ExecuteOldDiscardCommands (name);

	if (remove (filename) != -1)
	{
	    for (i = 0; i < sessionNameCount; i++)
	    {
		if (strcmp (sessionNames[i], name) == 0)
		{
		    XtFree ((char *) sessionNames[i]);
		    for (j = i; j < sessionNameCount - 1; j++)
			sessionNames[j] = sessionNames[j + 1];
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
		    sessionNames, sessionNameCount, longest, True);
	    }
	}

	delete_phase = 0;
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

    XtPopdown (chooseSessionPopup);

    if (session_name)
	XtFree (session_name);

    session_name = XtNewString (FAILSAFE_SESSION_NAME);

    FreeSessionNames (sessionNameCount, sessionNames);

    StartSession (session_name,
	True /* Use ~/.xsmstartup if found, else system.xsm */);
}



static void
ChooseSessionCancelXtProc (w, client_data, callData)

Widget		w;
XtPointer 	client_data;
XtPointer 	callData;

{
    if (delete_phase > 0)
    {
	XtVaSetValues (chooseSessionMessageLabel,
	    XtNforeground, save_message_background,
            NULL);

	delete_phase = 0;
    }
    else
	exit (0);
}



void
create_choose_session_popup ()

{
    XtActionsRec choose_actions[] = {
        {"ChooseSessionUp", ChooseSessionUp},
        {"ChooseSessionDown", ChooseSessionDown}
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

    chooseSessionFailSafeButton = XtVaCreateManagedWidget (
	"chooseSessionFailSafeButton", commandWidgetClass, chooseSessionForm,
        XtNfromHoriz, chooseSessionDeleteButton,
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
