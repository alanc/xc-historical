/* $XConsortium: info.c,v 1.12 94/08/10 21:27:18 mor Exp mor $ */
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

static int		longest = 400;

static int restartHints[] = {
	SmRestartIfRunning,
	SmRestartAnyway,
	SmRestartImmediately,
	SmRestartNever
};



void
ShowHint (client)

ClientRec *client;

{
    int currentHint = client->restartHint;

    XawToggleSetCurrent (restartIfRunning /* just 1 of the group */,
	(XtPointer) &restartHints[currentHint]);
}



void
DisplayProps (client, clientName)

ClientRec *client;
char *clientName;

{
    Position x, y, rootx, rooty;
    int i, j;

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

		if (strcmp (prop->name, SmRestartStyleHint) == 0)
		{
		    if (value == SmRestartAnyway)
			strcat (buffer, " (Restart Anyway)");
		    else if (value == SmRestartImmediately)
			strcat (buffer, " (Restart Immediately)");
		    else if (value == SmRestartNever)
			strcat (buffer, " (Restart Never)");
		    else
			strcat (buffer, " (Restart If Running)");
		}

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

	sprintf (buffer, "SM Properties : %s", clientName);

	XtVaSetValues (clientPropPopup,
	    XtNtitle, buffer,
	    XtNiconName, buffer,
	    NULL);

	if (!client_prop_visible)
	{
	    XtVaGetValues (mainWindow, XtNx, &x, XtNy, &y, NULL);
	    XtTranslateCoords (mainWindow, x, y, &rootx, &rooty);
	    XtVaSetValues (clientPropPopup,
	        XtNx, rootx + 50,
	        XtNy, rooty + 150,
	        NULL);

	    XtPopup (clientPropPopup, XtGrabNone);

	    client_prop_visible = 1;
	}
    }
}



static void
ClientListXtProc (w, client_data, callData)

Widget		w;
XtPointer 	client_data;
XtPointer 	callData;

{
    XawListReturnStruct *current = (XawListReturnStruct *) callData;
    ClientRec *client;

    if (!current || current->list_index < 0)
	return;
	
    client = clientListRecs[current->list_index];
    ShowHint (client);
    current_client_selected = current->list_index;
    if (client_prop_visible)
	DisplayProps (client, current->string);
}




static void
ViewPropXtProc (w, client_data, callData)

Widget		w;
XtPointer 	client_data;
XtPointer 	callData;

{
    ClientRec *client;
    XawListReturnStruct *current;

    current = XawListShowCurrent (clientListWidget);

    if (!current || current->list_index < 0)
    {
	if (current)
	    XtFree ((char *) current);
	return;
    }

    client = clientListRecs[current->list_index];
    DisplayProps (client, current->string);
    XtFree ((char *) current);
}



static void
CloneXtProc (w, client_data, callData)

Widget		w;
XtPointer 	client_data;
XtPointer 	callData;

{
    ClientRec *client;
    XawListReturnStruct *current;

    current = XawListShowCurrent (clientListWidget);

    if (!current || current->list_index < 0)
    {
	if (current)
	    XtFree ((char *) current);
	return;
    }

    client = clientListRecs[current->list_index];

    if (client)
	Clone (client, False /* don't use saved state */);
}



static void
KillClientXtProc (w, client_data, callData)

Widget		w;
XtPointer 	client_data;
XtPointer 	callData;

{
    ClientRec *client;
    XawListReturnStruct *current;

    current = XawListShowCurrent (clientListWidget);

    if (!current || current->list_index < 0)
    {
	if (current)
	    XtFree ((char *) current);
	return;
    }

    client = clientListRecs[current->list_index];

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
RestartHintXtProc (w, client_data, callData)

Widget		w;
XtPointer 	client_data;
XtPointer 	callData;

{
    XawListReturnStruct *current;
    ClientRec *client;
    XtPointer ptr;
    char hint;
    int i;

    current = XawListShowCurrent (clientListWidget);

    if (!current || current->list_index < 0)
    {
	if (current)
	    XtFree ((char *) current);
	return;
    }

    client = clientListRecs[current->list_index];

    ptr = XawToggleGetCurrent (restartIfRunning /* just 1 of the group */);

    if (!ptr)
	return;

    hint = *((int *) ptr);

    if (hint == SmRestartIfRunning || hint == SmRestartAnyway ||
	hint == SmRestartImmediately || hint == SmRestartNever)
    {
	SmProp prop;
	SmPropValue propval;
	int found = 0;

	client->restartHint = hint;

	for (i = 0; i < client->numProps; i++)
	    if (strcmp (SmRestartStyleHint, client->props[i]->name) == 0)
	    {
		*((char *) (client->props[i]->vals[0].value)) = hint;
		found = 1;
		break;
	    }

	if (!found)
	{
	    prop.name = SmRestartStyleHint;
	    prop.type = SmCARD8;
	    prop.num_vals = 1;
	    prop.vals = &propval;
	    propval.value = (SmPointer) &hint;
	    propval.length = 1;

	    SetProperty (client, &prop, True /* Malloc for us */);
	}

	if (client_prop_visible && clientListRecs &&
	    clientListRecs[current_client_selected] == client)
	{
	    DisplayProps (client, clientListNames[current_client_selected]);
	}
    }
}



static void
clientPropDoneXtProc (w, client_data, callData)

Widget		w;
XtPointer 	client_data;
XtPointer 	callData;

{
    XtPopdown (clientPropPopup);
    client_prop_visible = 0;
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

    if (clientListNames)
    {
	/*
	 * Free the previous list of names.  Xaw doesn't make a copy of
	 * our list, so we need to keep it around.
	 */

	for (i = 0; i < numClientListNames; i++)
	    XtFree (clientListNames[i]);

	free ((char *) clientListNames);

	clientListNames = NULL;
    }

    if (clientListRecs)
    {
	/*
	 * Free the mapping of client names to client records
	 */

	free ((char *) clientListRecs);
	clientListRecs = NULL;
    }

    maxlen1 = maxlen2 = 0;
    numClientListNames = 0;

    for (client = ClientList; client; client = client->next)
    {
	if (!client->running)
	    continue;

	progName = NULL;
	restart_service_prop = NULL;

	for (j = 0; j < client->numProps; j++)
	{
	    if (strcmp (client->props[j]->name, SmProgram) == 0)
	    {
		progName = GetProgramName (client->props[j]->vals[0].value);

		if (strlen (progName) > maxlen1)
		    maxlen1 = strlen (progName);
	    }
	    else if (strcmp (client->props[j]->name,
		"_XC_RestartService") == 0)
	    {
		restart_service_prop =
		    (char *) client->props[j]->vals[0].value;
	    }
	}

	if (!progName)
	    continue;

	if (restart_service_prop)
	    tmp1 = restart_service_prop;
	else if (client->clientHostname)
	    tmp1 = client->clientHostname;
	else
	    continue;

	if ((tmp2 = (char *) strchr (tmp1, '/')) == NULL)
	    hostname = tmp1;
	else
	    hostname = tmp2 + 1;

	if (strlen (hostname) > maxlen2)
	    maxlen2 = strlen (hostname);

	numClientListNames++;
    }

    clientListNames = (String *) malloc (numClientListNames * sizeof (String));
    clientListRecs = (ClientRec **) malloc (
	numClientListNames * sizeof (ClientRec *));

    for (client = ClientList, i = 0; client; client = client->next)
    {
	int extra1, extra2;

	if (!client->running)
	    continue;

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
	    continue;

	if (restart_service_prop)
	    tmp1 = restart_service_prop;
	else if (client->clientHostname)
	    tmp1 = client->clientHostname;
	else
	    continue;

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

	clientListRecs[i] = client;
	clientListNames[i++] = clientInfo;
    }

    XawListChange (clientListWidget,
	clientListNames, numClientListNames, longest, True);
}



void
ClientInfoXtProc (w, client_data, callData)
    Widget	w;
    XtPointer 	client_data;
    XtPointer 	callData;

{
    Position x, y, rootx, rooty;

    if (!client_info_visible)
    {
	UpdateClientList ();

	current_client_selected = 0;
	XawListHighlight (clientListWidget, 0);
	ShowHint (clientListRecs[0]);
	if (client_prop_visible)
	    DisplayProps (clientListRecs[0], clientListNames[0]);

	XtVaGetValues (mainWindow, XtNx, &x, XtNy, &y, NULL);
	XtTranslateCoords (mainWindow, x, y, &rootx, &rooty);
	XtVaSetValues (clientInfoPopup,
	    XtNx, rootx + 100,
	    XtNy, rooty + 50,
	    NULL);
	XtPopup (clientInfoPopup, XtGrabNone);

	client_info_visible = 1;
    }
}



void
create_client_info_popup ()

{
    extern Widget AddToggle ();


    /*
     * Pop up for List Clients button.
     */

    clientInfoPopup = XtVaCreatePopupShell (
	"clientInfoPopup", topLevelShellWidgetClass, topLevel,
	XtNallowShellResize, True,
	NULL);
    

    clientInfoForm = XtVaCreateManagedWidget (
	"clientInfoForm", formWidgetClass, clientInfoPopup,
	NULL);

    viewPropButton = XtVaCreateManagedWidget (
	"viewPropButton", commandWidgetClass, clientInfoForm,
        XtNfromHoriz, NULL,
        XtNfromVert, NULL,
	XtNtop, XawChainTop,
	XtNbottom, XawChainTop,
        NULL);
    
    XtAddCallback (viewPropButton, XtNcallback, ViewPropXtProc, 0);


    cloneButton = XtVaCreateManagedWidget (
	"cloneButton", commandWidgetClass, clientInfoForm,
        XtNfromHoriz, viewPropButton,
        XtNfromVert, NULL,
	XtNtop, XawChainTop,
	XtNbottom, XawChainTop,
        NULL);
    
    XtAddCallback (cloneButton, XtNcallback, CloneXtProc, 0);


    killClientButton = XtVaCreateManagedWidget (
	"killClientButton", commandWidgetClass, clientInfoForm,
        XtNfromHoriz, cloneButton,
        XtNfromVert, NULL,
	XtNtop, XawChainTop,
	XtNbottom, XawChainTop,
        NULL);
    
    XtAddCallback (killClientButton, XtNcallback, KillClientXtProc, 0);


    clientInfoDoneButton = XtVaCreateManagedWidget (
	"clientInfoDoneButton", commandWidgetClass, clientInfoForm,
        XtNfromHoriz, killClientButton,
        XtNfromVert, NULL,
	XtNtop, XawChainTop,
	XtNbottom, XawChainTop,
        NULL);

    XtAddCallback (clientInfoDoneButton, XtNcallback, listDoneXtProc, 0);


    clientListWidget = XtVaCreateManagedWidget (
	"clientListWidget", listWidgetClass, clientInfoForm,
        XtNdefaultColumns, 1,
	XtNforceColumns, True,
        XtNfromHoriz, NULL,
        XtNfromVert, viewPropButton,
	XtNresizable, True,
	XtNtop, XawChainTop,
	XtNbottom, XawChainBottom,
	NULL);

    XtAddCallback (clientListWidget, XtNcallback, ClientListXtProc, 0);


    restartHintLabel = XtVaCreateManagedWidget (
	"restartHintLabel", labelWidgetClass, clientInfoForm,
        XtNfromHoriz, NULL,
        XtNfromVert, clientListWidget,
        XtNborderWidth, 0,
	XtNtop, XawChainBottom,
	XtNbottom, XawChainBottom,
	NULL);

    restartIfRunning = AddToggle (
	"restartIfRunning", 			/* widgetName */
	clientInfoForm,				/* parent */
	1,					/* state */
        NULL,					/* radioGroup */
        (XtPointer) &restartHints[0],		/* radioData */
        restartHintLabel,			/* fromHoriz */
        clientListWidget,			/* fromVert */
	XawChainBottom,				/* top */
	XawChainBottom				/* bottom */
    );

    restartAnyway = AddToggle (
	"restartAnyway", 			/* widgetName */
	clientInfoForm,				/* parent */
	0,					/* state */
        restartIfRunning,			/* radioGroup */
        (XtPointer) &restartHints[1],		/* radioData */
        restartIfRunning,			/* fromHoriz */
        clientListWidget,			/* fromVert */
	XawChainBottom,				/* top */
	XawChainBottom				/* bottom */
    );

    restartImmediately = AddToggle (
	"restartImmediately", 			/* widgetName */
	clientInfoForm,				/* parent */
	0,					/* state */
        restartIfRunning,			/* radioGroup */
        (XtPointer) &restartHints[2],		/* radioData */
        restartAnyway,				/* fromHoriz */
        clientListWidget,			/* fromVert */
	XawChainBottom,				/* top */
	XawChainBottom				/* bottom */
    );

    restartNever = AddToggle (
	"restartNever", 			/* widgetName */
	clientInfoForm,				/* parent */
	0,					/* state */
        restartIfRunning,			/* radioGroup */
        (XtPointer) &restartHints[3],		/* radioData */
        restartImmediately,			/* fromHoriz */
        clientListWidget,			/* fromVert */
	XawChainBottom,				/* top */
	XawChainBottom				/* bottom */
    );

    XtAddCallback (restartIfRunning, XtNcallback, RestartHintXtProc, 0);
    XtAddCallback (restartAnyway, XtNcallback, RestartHintXtProc, 0);
    XtAddCallback (restartImmediately, XtNcallback, RestartHintXtProc, 0);
    XtAddCallback (restartNever, XtNcallback, RestartHintXtProc, 0);


    /*
     * Pop up for viewing client properties
     */

    clientPropPopup = XtVaCreatePopupShell (
	"clientPropPopup", topLevelShellWidgetClass, topLevel,
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
	XtNtop, XawChainTop,
	XtNbottom, XawChainTop,
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
	XtNtop, XawChainTop,
	XtNbottom, XawChainBottom,
	NULL);
}
