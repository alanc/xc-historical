/* $XConsortium: choose.c,v 1.3 94/07/08 14:06:19 mor Exp $ */
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
#include <sys/types.h>
#include <dirent.h>


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
	if (strncmp (entry->d_name, ".SM-", 4) == 0)
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
	if (strncmp (entry->d_name, ".SM-", 4) == 0)
	    (*names_ret)[(*count_ret)++] = XtNewString (entry->d_name + 4);
    }

    closedir (dir);

    return 1;
}



void
AddSessionNames (count, names)

int count;
String *names;

{
    XawListChange (chooseSessionListWidget, names, count, 0, True);
    XawListHighlight (chooseSessionListWidget, 0);
}



static void
ChooseSessionOkXtProc (w, client_data, callData)

Widget		w;
XtPointer 	client_data;
XtPointer 	callData;

{
    XawListReturnStruct *current;
    char title[256];


    /*
     * Pop down choice of sessions, realize main window, and
     * start the specified session.
     */

    XtPopdown (chooseSessionPopup);

    current = XawListShowCurrent (chooseSessionListWidget);

    if (session_name)
	XtFree (session_name);

    session_name = XtNewString (current->string);

    XtFree ((char *) current);

    sprintf (title, "xsm: %s", session_name);
    XtVaSetValues (topLevel,
	XtNtitle, title,		/* session name */
	NULL);

    XtRealizeWidget (topLevel);
    
    StartSession (session_name);
}



static void
ChooseSessionCancelXtProc (w, client_data, callData)

Widget		w;
XtPointer 	client_data;
XtPointer 	callData;

{
    exit (0);
}



void
create_choose_session_popup ()

{
    /*
     * Pop up for choosing session at startup
     */

    chooseSessionPopup = XtVaCreatePopupShell (
	"chooseSessionPopup", transientShellWidgetClass, topLevel,
        XtNmappedWhenManaged, False,
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

    chooseSessionOkButton = XtVaCreateManagedWidget (
	"chooseSessionOkButton", commandWidgetClass, chooseSessionForm,
        XtNfromHoriz, NULL,
        XtNfromVert, chooseSessionListWidget,
	XtNvertDistance, 25,
        NULL);

    XtAddCallback (chooseSessionOkButton, XtNcallback,
	ChooseSessionOkXtProc, 0);


    chooseSessionCancelButton = XtVaCreateManagedWidget (
	"chooseSessionCancelButton", commandWidgetClass, chooseSessionForm,
        XtNfromHoriz, chooseSessionOkButton,
        XtNfromVert, chooseSessionListWidget,
	XtNvertDistance, 25,
        NULL);

    XtAddCallback (chooseSessionCancelButton, XtNcallback,
	ChooseSessionCancelXtProc, 0);

    XtInstallAllAccelerators (chooseSessionListWidget, chooseSessionPopup);
}
