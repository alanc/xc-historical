/* $XConsortium: globals.c,v 1.15 94/08/30 17:21:19 mor Exp mor $ */
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

List		*RunningList;
List		*PendingList;
List		*RestartAnywayList;
List		*RestartImmedList;

List		*WaitForSaveDoneList;
List		*WaitForInteractList;
List		*WaitForPhase2List;

int		saveWaitCount = 0;

Bool		wantShutdown = False;
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
Bool		client_prop_visible = False;

String 		*clientListNames = NULL;
ClientRec	**clientListRecs = NULL;
int		numClientListNames = 0;

int		current_client_selected;

int		sessionNameCount = 0;
String		*sessionNames = NULL;

int		num_clients_in_last_session = -1;

char		**non_session_aware_clients = NULL;
int		non_session_aware_count = 0;

char		*display_env = NULL, *non_local_display_env = NULL;
char		*session_env = NULL, *non_local_session_env = NULL;
char		*audio_env = NULL;

Bool		naming_session = False;
Bool		need_to_name_session = False;
Bool		shutdown_after_name = False;
Bool		checkpoint_after_name = False;

Bool		remote_allowed;

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
Widget			nameSessionButton;
Widget			checkPointButton;
Widget			shutdownButton;
Widget				shutdownMenu;
Widget					shutdownSave;
Widget					shutdownDontSave;

Widget		    clientInfoPopup;

Widget		    	clientInfoForm;

Widget			    viewPropButton;
Widget			    cloneButton;
Widget			    killClientButton;
Widget			    clientInfoDoneButton;
Widget			    restartHintButton;
Widget			    	restartHintMenu;
Widget			    		restartIfRunning;
Widget			    		restartAnyway;
Widget			    		restartImmediately;
Widget			    		restartNever;
Widget			    clientListWidget;
Widget			    manualRestartLabel;
Widget			    manualRestartCommands;

Widget		    clientPropPopup;

Widget		    	clientPropForm;

Widget			    clientPropDoneButton;
Widget			    clientPropTextWidget;

Widget		    savePopup;

Widget			saveForm;

Widget			    saveMessageLabel;

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

Widget			    nameSessionCheckpointButton;
Widget			    nameSessionOkButton;
Widget			    nameSessionCancelButton;
