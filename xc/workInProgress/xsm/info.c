/* $XConsortium: info.c,v 1.8 94/07/26 14:17:54 mor Exp $ */
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
#include "restart.h"



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
	char buffer[1024];		/* ugh, gotta fix this */
	char number[10];

	buffer[0] = '\0';

	strcat (buffer, "*** ID = ");
	strcat (buffer, client->clientId);
	strcat (buffer, " ***\n\n");

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
CloneXtProc (w, client_data, callData)

Widget		w;
XtPointer 	client_data;
XtPointer 	callData;

{
    ClientRec *client = ClientList;
    XawListReturnStruct *current;
    int client_index;

    current = XawListShowCurrent (clientListWidget);
    client_index = current->list_index;

    while (client_index > 0)
    {
	client = client->next;
	client_index--;
    }

    if (client)
	Clone (client);
}



static void
KillClientXtProc (w, client_data, callData)

Widget		w;
XtPointer 	client_data;
XtPointer 	callData;

{
    ClientRec *client = ClientList;
    XawListReturnStruct *current;
    int client_index;

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



static char *
GetProgramName (fullname)

char *fullname;

{
    char *lastSlash = NULL;
    int i;

    for (i = 0; i < strlen (fullname); i++)
	if (fullname[i] == '/')
	    lastSlash = &fullname[i];
    
    if (lastSlash)
	return (lastSlash + 1);
    else
	return (fullname);
}



void
UpdateClientList ()

{
    ClientRec *client;
    char *progName, *hostname, *tmp1, *tmp2;
    String clientInfo;
    int maxlen1, maxlen2;
    char extraBuf1[80], extraBuf2[80];
    char *restart_service_prop;
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

    maxlen1 = maxlen2 = 0;
    client = ClientList;

    for (i = 0; i < numClients; i++, client = client->next)
    {
	restart_service_prop = NULL;

	for (j = 0; j < client->numProps; j++)
	{
	    if (strcmp (client->props[j]->name, SmProgram) == 0)
	    {
		char *temp = GetProgramName (client->props[j]->vals[0].value);

		if (strlen (temp) > maxlen1)
		    maxlen1 = strlen (temp);
	    }
	    else if (strcmp (client->props[j]->name,
		"_XC_RestartService") == 0)
	    {
		restart_service_prop =
		    (char *) client->props[j]->vals[0].value;
	    }
	}

	if (restart_service_prop)
	    tmp1 = restart_service_prop;
	else
	    tmp1 = client->clientHostname;

	if ((tmp2 = (char *) strchr (tmp1, '/')) == NULL)
	    hostname = tmp1;
	else
	    hostname = tmp2 + 1;

	if (strlen (hostname) > maxlen2)
	    maxlen2 = strlen (hostname);
    }

    clientNames = (String *) malloc (numClients * sizeof (String));
    numClientNames = numClients;

    client = ClientList;

    for (i = 0; i < numClients; i++, client = client->next)
    {
	progName = NULL;
	restart_service_prop = NULL;

	for (j = 0; j < client->numProps; j++)
	{
	    if (strcmp (client->props[j]->name, SmProgram) == 0)
	    {
		progName = GetProgramName (client->props[j]->vals[0].value);
	    }
	    else if (strcmp (client->props[j]->name,
		"_XC_RestartService") == 0)
	    {
		restart_service_prop =
		    (char *) client->props[j]->vals[0].value;
	    }
	    
	}

	if (!progName)
	{
	    clientInfo = XtNewString ("???????????");
	}
	else
	{
	    int extra1, extra2;

	    if (restart_service_prop)
		tmp1 = restart_service_prop;
	    else
		tmp1 = client->clientHostname;

	    if ((tmp2 = (char *) strchr (tmp1, '/')) == NULL)
		hostname = tmp1;
	    else
		hostname = tmp2 + 1;

	    extra1 = maxlen1 - strlen (progName) + 5;
	    extra2 = maxlen2 - strlen (hostname);

	    clientInfo = (String) XtMalloc (strlen (progName) +
		extra1 + extra2 + 4 + strlen (hostname));

	    for (k = 0; k < extra1; k++)
		extraBuf1[k] = ' ';
	    extraBuf1[extra1] = '\0';

	    for (k = 0; k < extra2; k++)
		extraBuf2[k] = ' ';
	    extraBuf2[extra2] = '\0';

	    sprintf (clientInfo, "%s%s (%s%s)", progName, extraBuf1,
		    hostname, extraBuf2);
	}

	clientNames[i] = clientInfo;
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

    if (!client_info_visible)
    {
	UpdateClientList ();
	XtRealizeWidget (clientInfoPopup);

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


    cloneButton = XtVaCreateManagedWidget (
	"cloneButton", commandWidgetClass, clientInfoForm,
        XtNfromHoriz, viewPropButton,
        XtNfromVert, NULL,
        NULL);
    
    XtAddCallback (cloneButton, XtNcallback, CloneXtProc, 0);


    killClientButton = XtVaCreateManagedWidget (
	"killClientButton", commandWidgetClass, clientInfoForm,
        XtNfromHoriz, cloneButton,
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
        XtNdefaultColumns, 1,
	XtNforceColumns, True,
        XtNfromHoriz, NULL,
        XtNfromVert, viewPropButton,
	XtNresizable, True,
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
	NULL);

    clientPropDoneButton = XtVaCreateManagedWidget (
	"clientPropDoneButton", commandWidgetClass, clientPropForm,
        XtNfromHoriz, NULL,
        XtNfromVert, NULL,
	XtNresizable, True,
        NULL);

    XtAddCallback (clientPropDoneButton, XtNcallback, clientPropDoneXtProc, 0);


    clientPropTextWidget = XtVaCreateManagedWidget (
	"clientPropTextWidget", asciiTextWidgetClass, clientPropForm,
        XtNfromHoriz, NULL,
        XtNfromVert, clientPropDoneButton,
	XtNeditType, XawtextRead,
        XtNdisplayCaret, False,
	XtNscrollVertical, XawtextScrollWhenNeeded,
	XtNscrollHorizontal, XawtextScrollWhenNeeded,
	XtNresizable, True,
	NULL);
}
