#if !defined(lint) && !defined(SABER)
static char rcs_id[] =
    "$XConsortium: init.c,v 2.34 89/09/17 19:40:37 converse Exp $";
#endif
/*
 *		        COPYRIGHT 1987, 1989
 *		   DIGITAL EQUIPMENT CORPORATION
 *		       MAYNARD, MASSACHUSETTS
 *			ALL RIGHTS RESERVED.
 *
 * THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT NOTICE AND
 * SHOULD NOT BE CONSTRUED AS A COMMITMENT BY DIGITAL EQUIPMENT CORPORATION.
 * DIGITAL MAKES NO REPRESENTATIONS ABOUT THE SUITABILITY OF THIS SOFTWARE FOR
 * ANY PURPOSE.  IT IS SUPPLIED "AS IS" WITHOUT EXPRESS OR IMPLIED WARRANTY.
 *
 * IF THE SOFTWARE IS MODIFIED IN A MANNER CREATING DERIVATIVE COPYRIGHT
 * RIGHTS, APPROPRIATE LEGENDS MAY BE PLACED ON THE DERIVATIVE WORK IN
 * ADDITION TO THAT SET FORTH ABOVE.
 *
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Digital Equipment Corporation not be
 * used in advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.
 *
 */

/* Init.c - Handle start-up initialization. */

#include "xmh.h"
#include "actions.h"
#include <sys/errno.h>

extern char* _XLowerCase();	/* %%% what is this doing here. */

/* Xmh-specific resources. */

static Boolean defFalse = False;
static Boolean defTrue = True;
static Boolean static_variable;

#define check_width 9
#define check_height 8
static char check_bits[] = {
   0x00, 0x01, 0x80, 0x01, 0xc0, 0x00, 0x60, 0x00,
   0x31, 0x00, 0x1b, 0x00, 0x0e, 0x00, 0x04, 0x00
};

#define offset(field) XtOffset(struct _resources *, field)

static XtResource resources[] = {
    {"debug", "Debug", XtRBoolean, sizeof(Boolean),
	 offset(debug), XtRBoolean, (XtPointer)&defFalse},
    {"tempdir", "tempDir", XtRString, sizeof(char *),
	 offset(tempDir), XtRString, "/tmp"},
    {"mhpath", "MhPath", XtRString, sizeof(char *),
	 offset(defMhPath), XtRString, "/usr/local/mh6"},
    {"initialfolder", "InitialFolder", XtRString, sizeof(char *),
	 offset(initialFolderName), XtRString, "inbox"},
    {"initialincfile", "InitialIncFile", XtRString, sizeof(char *),
         offset(initialIncFile), XtRString, NULL},
    {"replyinsertfilter", "ReplyInsertFilter", XtRString, sizeof(char *),
	 offset(defInsertFilter), XtRString, NULL},
    {"draftsfolder", "DraftsFolder", XtRString, sizeof(char *),
	 offset(draftsFolderName), XtRString, "drafts"},
    {"sendwidth", "SendWidth", XtRInt, sizeof(int),
	 offset(defSendLineWidth), XtRString, "72"},
    {"sendbreakwidth", "SendBreakWidth", XtRInt, sizeof(int),
	 offset(defBreakSendLineWidth), XtRString, "85"},
    {"printcommand", "PrintCommand", XtRString, sizeof(char *),
	 offset(defPrintCommand), XtRString,
	 "enscript > /dev/null 2>/dev/null"},
    {"tocwidth", "TocWidth", XtRInt, sizeof(int),
	 offset(defTocWidth), XtRString, "100"},
    {"skipdeleted", "SkipDeleted", XtRBoolean, sizeof(Boolean),
	 offset(SkipDeleted), XtRBoolean, (XtPointer)&defTrue},
    {"skipmoved", "SkipMoved", XtRBoolean, sizeof(Boolean),
	 offset(SkipMoved), XtRBoolean, (XtPointer)&defTrue},
    {"skipcopied", "SkipCopied", XtRBoolean, sizeof(Boolean),
	 offset(SkipCopied), XtRBoolean, (XtPointer)&defFalse},
    {"hideboringheaders", "HideBoringHeaders", XtRBoolean, sizeof(Boolean),
	 offset(defHideBoringHeaders), XtRBoolean, (XtPointer)&defTrue},
    {"geometry", "Geometry", XtRString, sizeof(char *),
	 offset(defGeometry), XtRString, NULL},
    {"tocgeometry", "TocGeometry", XtRString, sizeof(char *),
	 offset(defTocGeometry), XtRString, NULL},
    {"viewgeometry", "ViewGeometry", XtRString, sizeof(char *),
	 offset(defViewGeometry), XtRString, NULL},
    {"compgeometry", "CompGeometry", XtRString, sizeof(char *),
	 offset(defCompGeometry), XtRString, NULL},
    {"pickgeometry", "PickGeometry", XtRString, sizeof(char *),
	 offset(defPickGeometry), XtRString, NULL},
    {"tocpercentage", "TocPercentage", XtRInt, sizeof(int),
	 offset(defTocPercentage), XtRString, "33"},
    {"checknewmail", "CheckNewMail", XtRBoolean, sizeof(Boolean),
	 offset(defNewMailCheck), XtRBoolean, (XtPointer)&defTrue},
    {"makecheckpoints", "MakeCheckPoints", XtRBoolean, sizeof(Boolean),
	 offset(defMakeCheckpoints), XtRBoolean, (XtPointer)&defFalse},
    {"checkfrequency", "CheckFrequency", XtRInt, sizeof(int),
	 offset(check_frequency), XtRString, "1"},
    {"mailpath", "MailPath", XtRString, sizeof(char *),
	 offset(mailDir), XtRString, NULL},
    {"mailwaitingflag", "MailWaitingFlag", XtRBoolean, sizeof(Boolean),
	 offset(mailWaitingFlag), XtRBoolean, &defFalse},
    {"cursor", "Cursor", XtRCursor, sizeof(Cursor),
	 offset(cursor), XtRString, "left_ptr"},
    {"cursorcolor", "CursorColor", XtRPixel, sizeof(Pixel),
	 offset(cursor_color), XtRString, XtDefaultForeground},
    {"stickymenu", "StickyMenu", XtRBoolean, sizeof(Boolean), 	
	 offset(sticky_menu), XtRBoolean, (XtPointer) &defFalse},
    {"prefixwmandiconname", "PrefixWmAndIconName", XtRBoolean, sizeof(Boolean),
	 offset(prefix_wm_and_icon_name), XtRBoolean, (XtPointer)&defTrue},
    {"reverseReadOrder", "ReverseReadOrder", XtRBoolean, sizeof(Boolean),
	 offset(reverse_read_order), XtRBoolean, (XtPointer)&defFalse},
    {"blockEventsOnBusy", "BlockEventsOnBusy", XtRBoolean, sizeof(Boolean),
	 offset(block_events_on_busy), XtRBoolean, (XtPointer)&defTrue},
    {"busyCursor", "BusyCursor", XtRCursor, sizeof(Cursor),
	 offset(busy_cursor), XtRString, "watch"},
    {"busyCursorColor", "BusyCursorColor", XtRPixel, sizeof(Pixel),
	 offset(busy_cursor_color), XtRString, XtDefaultForeground},
    {"commandButtonCount", "CommandButtonCount", XtRInt, sizeof(int),
	 offset(command_button_count), XtRString, "0"},
};

#undef offset

static XrmOptionDescRec table[] = {
    {"-debug",	"debug",		XrmoptionNoArg,	"on"},
    {"-flag",	"mailwaitingflag",	XrmoptionNoArg, "on"},
    {"-initial","initialfolder",	XrmoptionSepArg, NULL},
    {"-path",	"mailpath",		XrmoptionSepArg, NULL},
};

/* Tell the user how to use this program. */
Syntax(call)
    char *call;
{
    extern void exit();
    (void) fprintf(stderr, "usage: %s [-path <path>] [-initial <folder>]\n",
		   call);
    exit(2);
}


static char *FixUpGeometry(geo, defwidth, defheight)
char *geo;
Dimension defwidth, defheight;
{
    int gbits;
    int x, y, width, height;
    if (geo == NULL) geo = app_resources.defGeometry;
    x = y = 0;
    gbits = XParseGeometry(geo, &x, &y, &width, &height);
    if (!(gbits & WidthValue)) {
	width = defwidth;
	gbits |= WidthValue;
    }
    if (!(gbits & HeightValue)) {
	height = defheight;
	gbits |= HeightValue;
    }
    return CreateGeometry(gbits, x, y, width, height);
}


static _IOErrorHandler(dpy)
    Display *dpy;
{
    extern char* SysErrMsg();
    (void) fprintf (stderr,
	     "%s:\tfatal IO error after %lu requests (%lu known processed)\n",
		    progName,
		    NextRequest(dpy) - 1, LastKnownRequestProcessed(dpy));
    (void) fprintf (stderr, "\t%d unprocessed events remaining.\r\n",
		    QLength(dpy));

    if (errno == EPIPE) {
	(void) fprintf (stderr,
     "\tThe connection was probably broken by a server shutdown or KillClient.\r\n");
    }

    Punt("Cannot continue from server error.");
}

/* All the start-up initialization goes here. */

InitializeWorld(argc, argv)
unsigned int argc;
char **argv;
{
    int l;
    FILEPTR fid;
    char str[500], str2[500], *ptr;
    Scrn scrn;
    static XtActionsRec actions[] = {

		/* general Xmh action procedures */

	{"XmhClose",			XmhClose},
	{"XmhComposeMessage",		XmhComposeMessage},

		/* actions upon folders */

	{"XmhOpenFolder",		XmhOpenFolder},
	{"XmhOpenFolderInNewWindow",	XmhOpenFolderInNewWindow},
	{"XmhCreateFolder",		XmhCreateFolder},
	{"XmhDeleteFolder",		XmhDeleteFolder},

		/* actions upon the Table of Contents */

	{"XmhIncorporateNewMail",	XmhIncorporateNewMail},
	{"XmhCommitChanges",		XmhCommitChanges},
	{"XmhPackFolder",		XmhPackFolder},
	{"XmhSortFolder",		XmhSortFolder},
	{"XmhForceRescan",		XmhForceRescan},
	{"XmhPushFolder",		XmhPushFolder},
	{"XmhPopFolder",		XmhPopFolder},

		/* actions upon the currently selected message(s) */

	{"XmhViewNextMessage",		XmhViewNextMessage},
	{"XmhViewPreviousMessage",	XmhViewPreviousMessage},
	{"XmhMarkDelete",		XmhMarkDelete},
	{"XmhMarkMove",			XmhMarkMove},
	{"XmhMarkCopy",			XmhMarkCopy},
	{"XmhUnmark",			XmhUnmark},
	{"XmhViewInNewWindow",		XmhViewInNewWindow},
	{"XmhReply",			XmhReply},
	{"XmhForward",			XmhForward},
	{"XmhUseAsComposition",		XmhUseAsComposition},
	{"XmhPrint",			XmhPrint},

		/* actions upon sequences */

	{"XmhPickMessages",		XmhPickMessages},
	{"XmhOpenSequence",		XmhOpenSequence},
	{"XmhAddToSequence",		XmhAddToSequence},
	{"XmhRemoveFromSequence",	XmhRemoveFromSequence},
	{"XmhDeleteSequence",		XmhDeleteSequence},

		/* actions upon the currently viewed message */

	{"XmhCloseView",		XmhCloseView},
	{"XmhViewReply",		XmhViewReply},
	{"XmhViewForward",		XmhViewForward},
	{"XmhViewUseAsComposition",	XmhViewUseAsComposition},
	{"XmhEditView",			XmhEditView},
	{"XmhSaveView",			XmhSaveView},
	{"XmhPrintView",		XmhPrintView},

       		/* actions upon a composition, reply, or forward */

	/* Close button			XmhCloseView	  (see above) */
	{"XmhResetCompose",		XmhResetCompose},
	/* Compose button 		XmhComposeMessage (see above) */
	{"XmhSave",			XmhSave},
	{"XmhSend",			XmhSend},
	{"XmhInsert",			XmhInsert},

		/* Menu Button action procedures for folders  */

        {"XmhPopupFolderMenu",		XmhPopupFolderMenu},
        {"XmhSetCurrentFolder",		XmhSetCurrentFolder},
        {"XmhLeaveFolderButton",	XmhLeaveFolderButton},
	{"XmhOpenFolderFromMenu",	XmhOpenFolderFromMenu},

		/* popup dialog box button action procedures */

	{"XmhPromptOkayAction",		XmhPromptOkayAction}
    };

    static Arg shell_args[] = {
	{XtNinput, (XtArgVal)True},
    };

    ptr = rindex(argv[0], '/');
    if (ptr) progName = ptr + 1;
    else progName = argv[0];

    toplevel = XtInitialize("main", "Xmh", table, XtNumber(table),
			    &argc, argv);
    if (argc > 1) Syntax(progName);

    XSetIOErrorHandler(_IOErrorHandler);

    XtSetValues(toplevel, shell_args, XtNumber(shell_args));

    theDisplay = XtDisplay(toplevel);

    homeDir = XtNewString(getenv("HOME"));

    XtGetApplicationResources( toplevel, (XtPointer)&app_resources,
			       resources, XtNumber(resources),
			       NULL, (Cardinal)0 );

    if (app_resources.mailWaitingFlag) app_resources.defNewMailCheck = True;

    (void) sprintf(str, "%s/.mh_profile", homeDir);
    fid = myfopen(str, "r");
    if (fid) {
	while (ptr = ReadLine(fid)) {
	    (void) strncpy(str2, ptr, 5);
	    str2[5] = '\0';
	    LowerCase(str2, str2);
	    if (strcmp(str2, "path:") == 0) {
		ptr += 5;
		while (*ptr == ' ' || *ptr == '\t')
		    ptr++;
		(void) strcpy(str, ptr);
	    }
	}
	(void) myfclose(fid);
    } else {
	(void) strcpy(str, "Mail");
    }
    for (l=strlen(str) - 1; l>=0 && (str[l] == ' ' || str[l] == '\t'); l--)
	str[l] = 0;
    if (str[0] == '/')
	(void) strcpy(str2, str);
    else
	(void) sprintf(str2, "%s/%s", homeDir, str);

    (void) sprintf(str, "%s/draft", str2);
    draftFile = XtNewString(str);
    (void) sprintf(str, "%s/xmhdraft", str2);
    xmhDraftFile = XtNewString(str);

    if (app_resources.mailDir == NULL)
	app_resources.mailDir = XtNewString(str2);

    NullSource = (Widget) NULL;

    l = strlen(app_resources.defMhPath) - 1;
    if (l > 0 && app_resources.defMhPath[l] == '/')
	app_resources.defMhPath[l] = 0;

    rootwidth = WidthOfScreen(XtScreen(toplevel));
    rootheight = HeightOfScreen(XtScreen(toplevel));

    app_resources.defTocGeometry =
	FixUpGeometry(app_resources.defTocGeometry,
		      rootwidth / 2, 3 * rootheight / 4);
    app_resources.defViewGeometry =
	FixUpGeometry(app_resources.defViewGeometry,
		      rootwidth / 2, rootheight / 2);
    app_resources.defCompGeometry =
	FixUpGeometry(app_resources.defCompGeometry,
		      rootwidth / 2, rootheight / 2);
    app_resources.defPickGeometry =
	FixUpGeometry(app_resources.defPickGeometry,
		      rootwidth / 2, rootheight / 2);

    numScrns = 0;
    scrnList = (Scrn *) XtMalloc((unsigned) 1);
    NoMenuForButton = (Widget) &static_variable;

    TocInit();
    InitPick();
    IconInit();
    BBoxInit();

    XtAppAddActions( XtWidgetToApplicationContext(toplevel),
		    actions, XtNumber(actions));

    MenuItemBitmap =
	XCreateBitmapFromData( XtDisplay(toplevel),
			      RootWindowOfScreen( XtScreen(toplevel)),
			      check_bits, check_width, check_height);

    DEBUG("Making screen ... ")

    scrn = CreateNewScrn(STtocAndView);

    SetCursorColor(scrn->parent, app_resources.cursor,
		   app_resources.cursor_color);
    if (app_resources.block_events_on_busy)
	SetCursorColor(scrn->parent, app_resources.busy_cursor, 
		       app_resources.busy_cursor_color);

    DEBUG(" setting toc ... ")

    TocSetScrn(InitialFolder, scrn);

    DEBUG("done.\n");

    MapScrn(scrn);
}
