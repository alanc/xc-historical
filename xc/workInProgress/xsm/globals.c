/* $XConsortium: globals.c,v 1.4 94/07/18 15:01:49 mor Exp $ */
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

List		*PendingList;
ClientRec	*ClientList = NULL;
int		numClients = 0;
int		saveDoneCount = 0;
int		interactCount = 0;
int		phase2RequestCount = 0;
Bool		shutdownDialogUp = False;
Bool		shutdownInProgress = False;
Bool		saveInProgress = False;
Bool		shutdownCancelled = False;
jmp_buf		JumpHere;

Bool		verbose = False;

char		*sm_id = NULL;

char		*session_name = NULL;

IceAuthDataEntry *authDataEntries = NULL;
int		numTransports = 0;

Bool		client_info_visible = False;

String 		*clientNames = NULL;
int		numClientNames = 0;

int		sessionNameCount = 0;
String		*sessionNames = NULL;

Bool		issued_checkpoint = False;

XtAppContext	appContext;

Widget		topLevel;

Widget		    chooseSessionPopup;

Widget		    	chooseSessionForm;

Widget			    chooseSessionLabel;

Widget			    chooseSessionListWidget;

Widget			    chooseSessionMessageLabel;

Widget			    chooseSessionLoadButton;

Widget			    chooseSessionDeleteButton;

Widget			    chooseSessionFailSafeButton;

Widget			    chooseSessionCancelButton;

Widget		    mainWindow;

Widget		        clientInfoButton;
Widget			checkPointButton;
Widget			shutdownButton;
Widget			nameSessionButton;

Widget		    clientInfoPopup;

Widget		    	clientInfoForm;

Widget			    viewPropButton;
Widget			    killClientButton;
Widget			    clientInfoDoneButton;
Widget			    clientListWidget;

Widget		    clientPropPopup;

Widget		    	clientPropForm;

Widget			    clientPropDoneButton;
Widget			    clientPropTextWidget;

Widget		    savePopup;

Widget			saveForm;

Widget			    saveTypeLabel;
Widget			    saveTypeGlobal;
Widget			    saveTypeLocal;
Widget			    saveTypeBoth;

Widget			    interactStyleLabel;
Widget			    interactStyleNone;
Widget			    interactStyleErrors;
Widget			    interactStyleAny;

Widget			    saveOkButton;
Widget			    saveCancelButton;

Widget		    nameSessionPopup;

Widget			nameSessionDialog;

Widget			    nameSessionOkButton;
Widget			    nameSessionCancelButton;
