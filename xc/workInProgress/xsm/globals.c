/* $XConsortium: globals.c,v 1.16 94/11/14 15:30:27 mor Exp mor $ */
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
List		*FailedSaveList;
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

char		*networkIds = NULL;
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
Bool		*sessionLocked = NULL;

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

