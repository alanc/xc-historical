/* $XConsortium: info.c,v 1.14 94/08/11 19:14:54 mor Exp mor $ */
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

static Pixmap checkBitmap;



void
ShowHint (client)

ClientRec *client;

{
    static Widget active = NULL;
    int hint = client->restartHint;

    if (active)
	XtVaSetValues (active, XtNleftBitmap, None, NULL);

    if (hint == SmRestartIfRunning)
	active = restartIfRunning;
    else if (hint == SmRestartAnyway)
	active = restartAnyway;
    else if (hint == SmRestartImmediately)
	active = restartImmediately;
    else if (hint == SmRestartNever)
	active = restartNever;

    XtVaSetValues (active, XtNleftBitmap, checkBitmap, NULL);
}



void
DisplayProps (client)

ClientRec *client;

{
    Position x, y, rootx, rooty;
    int index;
    List *pl, *pj, *vl;
    PropValue *pval;

    for (index = 0; index < numClientListNames; index++)
	if (clientListRecs[index] == client)
	    break;

    if (index >= numClientListNames)
	return;

    if (ListCount (client->props) > 0)
    {
	char buffer[1024];		/* ugh, gotta fix this */
	char number[10];
	char *ptr;

	buffer[0] = '\0';

	strcat (buffer, "*** ID = ");
	strcat (buffer, client->clientId);
	strcat (buffer, " ***\n\n");

	for (pl = ListFirst (client->props); pl; pl = ListNext (pl))
	{
	    Prop *pprop = (Prop *) pl->thing;

	    strcat (buffer, "Name:		");
	    strcat (buffer, pprop->name);
	    strcat (buffer, "\n");
	    strcat (buffer, "Type:		");
	    strcat (buffer, pprop->type);
	    strcat (buffer, "\n");
	    strcat (buffer, "Num values:	");
	    sprintf (number, "%d", ListCount (pprop->values));
	    strcat (buffer, number);
	    strcat (buffer, "\n");

	    if (strcmp (pprop->type, SmCARD8) == 0)
	    {
		char *card8;
		int value;

		vl = ListFirst (pprop->values);
		pval = (PropValue *) vl->thing;

		card8 = pval->value;
		value = *card8;

		strcat (buffer, "Value 1:	");
		sprintf (number, "%d", value);
		strcat (buffer, number);

		if (strcmp (pprop->name, SmRestartStyleHint) == 0)
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
		int propnum = 0;

		for (pj = ListFirst (pprop->values); pj; pj = ListNext (pj))
		{
		    propnum++;

		    pval = (PropValue *) pj->thing;
		    strcat (buffer, "Value ");
		    sprintf (number, "%d", propnum);
		    strcat (buffer, number);
		    strcat (buffer, ":	");
		    strcat (buffer, (char *) pval->value);
		    strcat (buffer, "\n");
		}
	    }

	    strcat (buffer, "\n");
	}

	XtVaSetValues (clientPropTextWidget,
	    XtNstring, buffer,
	    NULL);

	sprintf (buffer, "SM Properties : %s", clientListNames[index]);
	ptr = Strstr (buffer, ")   Restart");
	if (ptr) *(ptr + 1) = '\0';

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
	DisplayProps (client);
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
    DisplayProps (client);
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
    List *cl, *pl;
    int i, k;

    if (clientListNames)
    {
	/*
	 * Free the previous list of names.  Xaw doesn't make a copy of
	 * our list, so we need to keep it around.
	 */

	for (i = 0; i < numClientListNames; i++)
	    XtFree (clientListNames[i]);

	XtFree ((char *) clientListNames);

	clientListNames = NULL;
    }

    if (clientListRecs)
    {
	/*
	 * Free the mapping of client names to client records
	 */

	XtFree ((char *) clientListRecs);
	clientListRecs = NULL;
    }

    maxlen1 = maxlen2 = 0;
    numClientListNames = 0;

    for (cl = ListFirst (RunningList); cl; cl = ListNext (cl))
    {
	client = (ClientRec *) cl->thing;

	progName = NULL;
	restart_service_prop = NULL;

	for (pl = ListFirst (client->props); pl; pl = ListNext (pl))
	{
	    Prop *pprop = (Prop *) pl->thing;
	    List *vl = ListFirst (pprop->values);
	    PropValue *pval = (PropValue *) vl->thing;

	    if (strcmp (pprop->name, SmProgram) == 0)
	    {
		progName = GetProgramName ((char *) pval->value);

		if (strlen (progName) > maxlen1)
		    maxlen1 = strlen (progName);
	    }
	    else if (strcmp (pprop->name, "_XC_RestartService") == 0)
	    {
		restart_service_prop = (char *) pval->value;
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

    clientListNames = (String *) XtMalloc (
	numClientListNames * sizeof (String));
    clientListRecs = (ClientRec **) XtMalloc (
	numClientListNames * sizeof (ClientRec *));

    i = 0;
    for (cl = ListFirst (RunningList); cl; cl = ListNext (cl))
    {
	ClientRec *client = (ClientRec *) cl->thing;
	int extra1, extra2;
	char *hint;

	progName = NULL;
	restart_service_prop = NULL;

	for (pl = ListFirst (client->props); pl; pl = ListNext (pl))
	{
	    Prop *pprop = (Prop *) pl->thing;
	    List *vl = ListFirst (pprop->values);
	    PropValue *pval = (PropValue *) vl->thing;

	    if (strcmp (pprop->name, SmProgram) == 0)
	    {
		progName = GetProgramName ((char *) pval->value);
	    }
	    else if (strcmp (pprop->name, "_XC_RestartService") == 0)
	    {
		restart_service_prop = (char *) pval->value;
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

	if (client->restartHint == SmRestartIfRunning)
	    hint = "Restart If Running";
	else if (client->restartHint == SmRestartAnyway)
	    hint = "Restart Anyway";
	else if (client->restartHint == SmRestartImmediately)
	    hint = "Restart Immediately";
	else if (client->restartHint == SmRestartNever)
	    hint = "Restart Never";
	else
	    hint = "";

	clientInfo = (String) XtMalloc (strlen (progName) +
	    extra1 + extra2 + 3 + strlen (hostname) + 3 + strlen (hint) + 1);

	for (k = 0; k < extra1; k++)
	    extraBuf1[k] = ' ';
	extraBuf1[extra1] = '\0';

	for (k = 0; k < extra2; k++)
	    extraBuf2[k] = ' ';
	extraBuf2[extra2] = '\0';

	sprintf (clientInfo, "%s%s (%s%s)   %s", progName, extraBuf1,
	    hostname, extraBuf2, hint);

	clientListRecs[i] = client;
	clientListNames[i++] = clientInfo;
    }

    XawListChange (clientListWidget,
	clientListNames, numClientListNames, 0, True);
}



static void
RestartHintXtProc (w, client_data, callData)

Widget		w;
XtPointer 	client_data;
XtPointer 	callData;

{
    XawListReturnStruct *current;
    ClientRec *client;
    Widget active;
    int found = 0;
    List *pl;
    char hint;

    current = XawListShowCurrent (clientListWidget);

    if (!current || current->list_index < 0)
    {
	if (current)
	    XtFree ((char *) current);
	return;
    }

    client = clientListRecs[current->list_index];

    active = XawSimpleMenuGetActiveEntry (restartHintMenu);

    if (active == restartIfRunning)
	hint = SmRestartIfRunning;
    else if (active == restartAnyway)
	hint = SmRestartAnyway;
    else if (active == restartImmediately)
	hint = SmRestartImmediately;
    else if (active == restartNever)
	hint = SmRestartNever;
    else
	return;

    client->restartHint = hint;

    for (pl = ListFirst (client->props); pl; pl = ListNext (pl))
    {
	Prop *pprop = (Prop *) pl->thing;

	if (strcmp (SmRestartStyleHint, pprop->name) == 0)
	{
	    List *vl = ListFirst (pprop->values);
	    PropValue *pval = (PropValue *) vl->thing;

	    *((char *) (pval->value)) = hint;
	    found = 1;
	    break;
	}
    }

    if (!found)
    {
	SmProp prop;
	SmPropValue propval;

	prop.name = SmRestartStyleHint;
	prop.type = SmCARD8;
	prop.num_vals = 1;
	prop.vals = &propval;
	propval.value = (SmPointer) &hint;
	propval.length = 1;
	
	SetProperty (client, &prop);
    }

    UpdateClientList ();
    XawListHighlight (clientListWidget, current_client_selected);
    ShowHint (client);

    if (client_prop_visible && clientListRecs &&
	clientListRecs[current_client_selected] == client)
    {
	DisplayProps (client);
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
	    DisplayProps (clientListRecs[0]);

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


#define CHECK_WIDTH 9
#define CHECK_HEIGHT 8

static unsigned char check_bits[] = {
   0x00, 0x01, 0x80, 0x01, 0xc0, 0x00, 0x60, 0x00,
   0x31, 0x00, 0x1b, 0x00, 0x0e, 0x00, 0x04, 0x00
};


void
create_client_info_popup ()

{
    /*
     * Make checkmark bitmap
     */

    checkBitmap = XCreateBitmapFromData (
	XtDisplay (topLevel), RootWindowOfScreen (XtScreen (topLevel)),
	(char *) check_bits, CHECK_WIDTH, CHECK_HEIGHT);


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


    restartHintButton = XtVaCreateManagedWidget (
	"restartHintButton", menuButtonWidgetClass, clientInfoForm,
	XtNmenuName, "restartHintMenu",
        XtNfromHoriz, killClientButton,
        XtNfromVert, NULL,
	XtNtop, XawChainTop,
	XtNbottom, XawChainTop,
	NULL);

    restartHintMenu = XtVaCreatePopupShell (
	"restartHintMenu", simpleMenuWidgetClass, clientInfoForm,
	NULL);

    restartIfRunning = XtVaCreateManagedWidget (
	"restartIfRunning", smeBSBObjectClass, restartHintMenu,
	XtNleftMargin, 18,
	NULL);

    restartAnyway = XtVaCreateManagedWidget (
	"restartAnyway", smeBSBObjectClass, restartHintMenu,
	XtNleftMargin, 18,
	NULL);

    restartImmediately = XtVaCreateManagedWidget (
	"restartImmediately", smeBSBObjectClass, restartHintMenu,
	XtNleftMargin, 18,
	NULL);

    restartNever = XtVaCreateManagedWidget (
	"restartNever", smeBSBObjectClass, restartHintMenu,
	XtNleftMargin, 18,
	NULL);

    XtAddCallback (restartIfRunning, XtNcallback, RestartHintXtProc, 0);
    XtAddCallback (restartAnyway, XtNcallback, RestartHintXtProc, 0);
    XtAddCallback (restartImmediately, XtNcallback, RestartHintXtProc, 0);
    XtAddCallback (restartNever, XtNcallback, RestartHintXtProc, 0);


    clientInfoDoneButton = XtVaCreateManagedWidget (
	"clientInfoDoneButton", commandWidgetClass, clientInfoForm,
        XtNfromHoriz, restartHintButton,
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
