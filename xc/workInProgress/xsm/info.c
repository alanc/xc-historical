/* $XConsortium: info.c,v 1.3 94/07/12 14:25:24 mor Exp $ */
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


static void
GetClientInfoXtProc (w, client_data, callData)

Widget		w;
XtPointer 	client_data;
XtPointer 	callData;

{
    ClientRec *client = ClientList;
    XawListReturnStruct *current;
    int client_index, i, j;
    static Bool did_first_popup = 0;

    current = XawListShowCurrent (clientListWidget);
    client_index = current->list_index;

    while (client_index > 0)
    {
	client = client->next;
	client_index--;
    }

    if (client->numProps > 0)
    {
	char buffer[1024];
	char number[10];

	buffer[0] = '\0';

	for (i = 0; i < client->numProps; i++)
	{
	    SmProp *prop = client->props[i];

	    strcat (buffer, "Name:		");
	    strcat (buffer, prop->name);
	    strcat (buffer, "\n");
	    strcat (buffer, "Type:		");
	    strcat (buffer, prop->type);
	    strcat (buffer, "\n");
	    strcat (buffer, "Num values:	");
	    sprintf (number, "%d", prop->num_vals);
	    strcat (buffer, number);
	    strcat (buffer, "\n");

	    if (strcmp (prop->type, SmCARD8) == 0)
	    {
		char *card8 = prop->vals->value;
		int value = *card8;
		strcat (buffer, "Value 1:	");
		sprintf (number, "%d", value);
		strcat (buffer, number);
		strcat (buffer, "\n");
	    }
	    else
	    {
		for (j = 0; j < prop->num_vals; j++)
		{
		    strcat (buffer, "Value ");
		    sprintf (number, "%d", j + 1);
		    strcat (buffer, number);
		    strcat (buffer, ":	");
		    strcat (buffer, (char *) prop->vals[j].value);
		    strcat (buffer, "\n");
		}
	    }

	    if (i < client->numProps - 1)
		strcat (buffer, "\n");
	}

	XtVaSetValues (clientPropTextWidget,
	    XtNstring, buffer,
	    NULL);

	sprintf (buffer, "SM Properties : %s", current->string);

	XtVaSetValues (clientPropPopup,
	    XtNtitle, buffer,
	    NULL);

	if (!did_first_popup)
	{
	    Position x, y, rootx, rooty;

	    XtVaGetValues (mainWindow, XtNx, &x, XtNy, &y, NULL);
	    XtTranslateCoords (mainWindow, x, y, &rootx, &rooty);
	    XtMoveWidget (clientPropPopup, rootx + 50, rooty + 150);
	    did_first_popup = 1;
	}

	XtPopup (clientPropPopup, XtGrabNone);
    }

    XtFree ((char *) current);
}



static void
KillClientXtProc (w, client_data, callData)

Widget		w;
XtPointer 	client_data;
XtPointer 	callData;

{
    ClientRec *client = ClientList;
    XawListReturnStruct *current;
    int client_index, i, j;

    current = XawListShowCurrent (clientListWidget);
    client_index = current->list_index;

    while (client_index > 0)
    {
	client = client->next;
	client_index--;
    }

    SmsDie (client->smsConn);
}



static void
listDoneXtProc (w, client_data, callData)

Widget		w;
XtPointer 	client_data;
XtPointer 	callData;

{
    XtPopdown (clientInfoPopup);
    client_info_visible = 0;
}



static void
clientPropDoneXtProc (w, client_data, callData)

Widget		w;
XtPointer 	client_data;
XtPointer 	callData;

{
    XtPopdown (clientPropPopup);
}



void
UpdateClientList ()

{
    ClientRec *client = ClientList;
    char *progName;
    int i, j, k;

    if (clientNames)
    {
	/*
	 * Free the previous list of names.  Xaw doesn't make a copy of
	 * our list, so we need to keep it around.
	 */

	for (i = 0; i < numClientNames; i++)
	    XtFree (clientNames[i]);
	free ((char *) clientNames);
    }

    clientNames = (String *) malloc (numClients * sizeof (String));
    numClientNames = numClients;

    for (i = 0; i < numClients; i++, client = client->next)
    {
	progName = NULL;

	for (j = 0; j < client->numProps && !progName; j++)
	    if (strcmp (client->props[j]->name, SmProgram) == 0)
	    {
		char *temp = client->props[j]->vals[0].value;
		char *lastSlash = NULL;

		for (k = 0; k < strlen (temp); k++)
		    if (temp[k] == '/')
			lastSlash = &temp[k];

		if (lastSlash)
		    temp = lastSlash + 1;

		progName = (String) XtMalloc (
		    strlen (client->clientHostname) + strlen (temp) + 4);
		sprintf (progName, "%s : %s", client->clientHostname, temp);
	    }

	if (!progName)
	    progName = XtNewString ("???????????");

	clientNames[i] = progName;
    }

    XawListChange (clientListWidget, clientNames, numClients, 0, True);
    XawListHighlight (clientListWidget, 0);
}



void
ClientInfoXtProc (w, client_data, callData)
    Widget	w;
    XtPointer 	client_data;
    XtPointer 	callData;

{
    static Bool did_first_popup = 0;
    Dimension width;

    if (!client_info_visible)
    {
	UpdateClientList ();
	XtRealizeWidget (clientInfoPopup);

	XtVaGetValues (clientInfoPopup,
	    XtNwidth, &width,
	    NULL);
    
	XtVaSetValues (clientListWidget,
	    XtNlongest, width,
	    NULL);

	if (!did_first_popup)
	{
	    Position x, y, rootx, rooty;

	    XtVaGetValues (mainWindow, XtNx, &x, XtNy, &y, NULL);
	    XtTranslateCoords (mainWindow, x, y, &rootx, &rooty);
	    XtMoveWidget (clientInfoPopup, rootx + 100, rooty + 50);
	    did_first_popup = 1;
	}

	XtPopup (clientInfoPopup, XtGrabNone);

	client_info_visible = 1;
    }
}



void
create_client_info_popup ()

{
    /*
     * Pop up for List Clients button.
     */

    clientInfoPopup = XtVaCreatePopupShell (
	"clientInfoPopup", transientShellWidgetClass, topLevel,
        XtNmappedWhenManaged, False,
	XtNallowShellResize, True,
	NULL);
    

    clientInfoForm = XtVaCreateManagedWidget (
	"clientInfoForm", formWidgetClass, clientInfoPopup,
	NULL);

    viewPropButton = XtVaCreateManagedWidget (
	"viewPropButton", commandWidgetClass, clientInfoForm,
        XtNfromHoriz, NULL,
        XtNfromVert, NULL,
        NULL);
    
    XtAddCallback (viewPropButton, XtNcallback, GetClientInfoXtProc, 0);


    killClientButton = XtVaCreateManagedWidget (
	"killClientButton", commandWidgetClass, clientInfoForm,
        XtNfromHoriz, viewPropButton,
        XtNfromVert, NULL,
        NULL);
    
    XtAddCallback (killClientButton, XtNcallback, KillClientXtProc, 0);


    clientInfoDoneButton = XtVaCreateManagedWidget (
	"clientInfoDoneButton", commandWidgetClass, clientInfoForm,
        XtNfromHoriz, killClientButton,
        XtNfromVert, NULL,
        NULL);

    XtAddCallback (clientInfoDoneButton, XtNcallback, listDoneXtProc, 0);


    clientListWidget = XtVaCreateManagedWidget (
	"clientListWidget", listWidgetClass, clientInfoForm,
	XtNresizable, True,
        XtNdefaultColumns, 1,
	XtNforceColumns, True,
        XtNfromHoriz, NULL,
        XtNfromVert, viewPropButton,
	NULL);



    /*
     * Pop up for viewing client properties
     */

    clientPropPopup = XtVaCreatePopupShell (
	"clientPropPopup", transientShellWidgetClass, topLevel,
	XtNallowShellResize, True,
	NULL);
    

    clientPropForm = XtVaCreateManagedWidget (
	"clientPropForm", formWidgetClass, clientPropPopup,
	XtNresizable, True,
	NULL);

    clientPropDoneButton = XtVaCreateManagedWidget (
	"clientPropDoneButton", commandWidgetClass, clientPropForm,
        XtNfromHoriz, NULL,
        XtNfromVert, NULL,
        NULL);

    XtAddCallback (clientPropDoneButton, XtNcallback, clientPropDoneXtProc, 0);


    clientPropTextWidget = XtVaCreateManagedWidget (
	"clientPropTextWidget", asciiTextWidgetClass, clientPropForm,
        XtNfromHoriz, NULL,
        XtNfromVert, clientPropDoneButton,
	XtNeditType, XawtextRead,
        XtNdisplayCaret, False,
/*	XtNresize, XawtextResizeBoth, */
	XtNscrollVertical, XawtextScrollWhenNeeded,
	XtNscrollHorizontal, XawtextScrollWhenNeeded,
	NULL);
}
