/* $XConsortium: xsm.h,v 1.29 94/08/30 17:21:46 mor Exp mor $ */
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

#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Dialog.h>
#include <X11/Xaw/Toggle.h>
#include <X11/Xaw/List.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Xfuncs.h>
#ifndef _POSIX_SOURCE
#define _POSIX_SOURCE
#include <stdio.h>
#undef _POSIX_SOURCE
#else
#include <stdio.h>
#endif
#include <X11/Shell.h>
#include <X11/ICE/ICEutil.h>
#include <X11/SM/SMlib.h>
#include <ctype.h>
#ifndef X_NOT_STDC_ENV
#include <stdlib.h>
#endif
#include <setjmp.h>
#include <limits.h>
#include <sys/param.h>
#ifndef PATH_MAX
#ifdef MAXPATHLEN
#define PATH_MAX MAXPATHLEN
#else
#define PATH_MAX 1024
#endif
#endif
#include <X11/Xos.h>
#include <X11/Xatom.h>

/* Fix ISC brain damage.  When using gcc fdopen isn't declared in <stdio.h>. */
#if defined(SYSV) && defined(SYSV386) && defined(__STDC__) && defined(ISC)
extern FILE *fdopen(int, char const *);
#endif

#include "list.h"

/*
 * Each time the format of the sm's save file changes, bump up
 * the version.
 */

#define SAVEFILE_VERSION 3

#define DEFAULT_SESSION_NAME "Default"
#define FAILSAFE_SESSION_NAME "Fail Safe"

#define RESTART_MANAGERS 	1
#define RESTART_REST_OF_CLIENTS	2

typedef struct _ClientRec {
    SmsConn	 	smsConn;
    IceConn		ice_conn;
    char 		*clientId;
    char		*clientHostname;
    List		*props;
    char		*discardCommand;
    char		*saveDiscardCommand;

    unsigned int	restarted : 1;
    unsigned int	userIssuedCheckpoint : 1;
    unsigned int	restartHint : 2;

} ClientRec;

typedef struct _PendingClient {
    char		*clientId;
    char		*clientHostname;
    List		*props;
} PendingClient;

typedef struct _Prop {
    char		*name;
    char		*type;
    List		*values;
} Prop;

typedef struct _PropValue {
    XtPointer		value;
    int			length;
} PropValue;


extern char		*display_env, *non_local_display_env;
extern char		*session_env, *non_local_session_env;
extern char		*audio_env;

extern Bool		naming_session;
extern Bool		need_to_name_session;

extern Bool		shutdown_after_name;
extern Bool		checkpoint_after_name;

extern Bool		remote_allowed;

extern XtAppContext	appContext;

extern Bool		verbose;

extern char		*sm_id;

extern char		*session_name;

extern List		*RunningList;
extern List		*PendingList;
extern List		*RestartAnywayList;
extern List		*RestartImmedList;

extern List		*WaitForSaveDoneList;
extern List		*WaitForInteractList;
extern List		*WaitForPhase2List;

extern int		saveWaitCount;

extern Bool		client_info_visible;
extern Bool		client_prop_visible;
extern String 		*clientListNames;
extern ClientRec	**clientListRecs;
extern int		numClientListNames;
extern int		current_client_selected;

extern Bool		shutdownInProgress;
extern Bool	        saveInProgress;
extern Bool		shutdownCancelled;
extern Bool		wantShutdown;

extern int		sessionNameCount;
extern String		*sessionNames;

extern int		num_clients_in_last_session;

extern char		**non_session_aware_clients;
extern int		non_session_aware_count;

extern Widget		topLevel;

extern Widget		    chooseSessionPopup;

extern Widget		    	chooseSessionForm;

extern Widget			    chooseSessionLabel;

extern Widget			    chooseSessionListWidget;

extern Widget			    chooseSessionMessageLabel;

extern Widget			    chooseSessionLoadButton;

extern Widget			    chooseSessionDeleteButton;

extern Widget			    chooseSessionFailSafeButton;

extern Widget			    chooseSessionCancelButton;

extern Widget		    mainWindow;

extern Widget		        clientInfoButton;
extern Widget			nameSessionButton;
extern Widget			checkPointButton;
extern Widget			shutdownButton;
extern Widget				shutdownMenu;
extern Widget					shutdownSave;
extern Widget					shutdownDontSave;

extern Widget		    clientInfoPopup;

extern Widget		    	clientInfoForm;

extern Widget			    viewPropButton;
extern Widget			    cloneButton;
extern Widget			    killClientButton;
extern Widget			    clientInfoDoneButton;
extern Widget			    restartHintButton;
extern Widget			    	restartHintMenu;
extern Widget			    		restartIfRunning;
extern Widget			    		restartAnyway;
extern Widget			    		restartImmediately;
extern Widget			    		restartNever;
extern Widget			    clientListWidget;
extern Widget			    manualRestartLabel;
extern Widget			    manualRestartCommands;

extern Widget		    clientPropPopup;

extern Widget		    	clientPropForm;

extern Widget			    clientPropDoneButton;
extern Widget			    clientPropTextWidget;

extern Widget		    savePopup;

extern Widget			saveForm;

extern Widget			    saveMessageLabel;

extern Widget			    saveTypeLabel;
extern Widget			    saveTypeGlobal;
extern Widget			    saveTypeLocal;
extern Widget			    saveTypeBoth;

extern Widget			    interactStyleLabel;
extern Widget			    interactStyleNone;
extern Widget			    interactStyleErrors;
extern Widget			    interactStyleAny;

extern Widget			    saveOkButton;
extern Widget			    saveCancelButton;

extern Widget		    shutdownPopup;

extern Widget			shutdownDialog;

extern Widget			    shutdownOkButton;
extern Widget			    shutdownCancelButton;

extern Widget		    nameSessionPopup;

extern Widget			nameSessionDialog;

extern Widget			    nameSessionCheckpointButton;
extern Widget			    nameSessionOkButton;
extern Widget			    nameSessionCancelButton;

extern void fprintfhex ();

#ifndef X_NOT_STDC_ENV
#define Strstr strstr
#else
extern char *Strstr();
#endif

extern strbw ();
