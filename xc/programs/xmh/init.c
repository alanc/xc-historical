#if !defined(lint) && !defined(SABER)
static char rcs_id[] =
    "$XConsortium: init.c,v 2.30 89/07/21 18:55:54 converse Exp $";
#endif
/*
 *			  COPYRIGHT 1987
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
#include <sys/errno.h>

extern char* _XLowerCase();

/* Xmh-specific resources. */
static Boolean defFalse = False;
static Boolean defTrue = True;

static Boolean static_variable;

#define offset(field) XtOffset(struct _resources *, field)

static XtResource resources[] = {
    {"debug", "Debug", XtRBoolean, sizeof(Boolean),
	 offset(debug), XtRBoolean, (caddr_t)&defFalse},
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
	 offset(SkipDeleted), XtRBoolean, (caddr_t)&defTrue},
    {"skipmoved", "SkipMoved", XtRBoolean, sizeof(Boolean),
	 offset(SkipMoved), XtRBoolean, (caddr_t)&defTrue},
    {"skipcopied", "SkipCopied", XtRBoolean, sizeof(Boolean),
	 offset(SkipCopied), XtRBoolean, (caddr_t)&defFalse},
    {"hideboringheaders", "HideBoringHeaders", XtRBoolean, sizeof(Boolean),
	 offset(defHideBoringHeaders), XtRBoolean, (caddr_t)&defTrue},
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
	 offset(defNewMailCheck), XtRBoolean, (caddr_t)&defTrue},
    {"makecheckpoints", "MakeCheckPoints", XtRBoolean, sizeof(Boolean),
	 offset(defMakeCheckpoints), XtRBoolean, (caddr_t)&defFalse},
    {"checkFrequency", "CheckFrequency", XtRInt, sizeof(int),
	 offset(check_frequency), XtRString, "1"},
    {"mailpath", "MailPath", XtRString, sizeof(char *),
	 offset(mailDir), XtRString, NULL},
    {"mailwaitingflag", "MailWaitingFlag", XtRBoolean, sizeof(Boolean),
	 offset(mailWaitingFlag), XtRBoolean, &defFalse},
    {"cursor", "Cursor", XtRCursor, sizeof(Cursor),
	 offset(cursor), XtRString, "left_ptr"},
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

/* command button actions */
extern void
CloseScrn(),ComposeMessage(),OpenFolder(),OpenFolderInNewWindow(),
CreateFolder(),DeleteFolder(),Incorporate(),NextView(),PrevView(),
MarkDelete(),MarkMove(),MarkCopy(),MarkUnmarked(),ViewNew(),TocReply(),
TocForward(),TocUseAsComposition(),CommitChanges(),OpenSequence(),
AddToSequence(),RemoveFromSequence(),DeleteSequence(),PickMessages(),
PrintMessages(),Pack(),Sort(),ForceRescan(),CloseView(),ViewReply(),
ViewForward(),ViewUseAsComposition(),EditView(),SaveView(),PrintView(),
CompReset(),SaveDraft(),SendDraft(),MsgInsertAssoc();

/* menu button actions */
extern void PopupFolderMenu(), SetCurrentFolder(), LeaveFolderButton(),
OpenFolderFromMenu();


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

			/* folderButtons action procedures */

	{"CloseScrn",			CloseScrn},
	{"ComposeMessage",		ComposeMessage},
	{"OpenFolder",			OpenFolder},
	{"OpenFolderInNewWindow",	OpenFolderInNewWindow},
	{"CreateFolder",		CreateFolder},
	{"DeleteFolder",		DeleteFolder},

			/* tocButtons action procedures */

	{"Incorporate",			Incorporate},
	{"NextView",			NextView},
	{"PrevView",			PrevView},
	{"MarkDelete",			MarkDelete},
	{"MarkMove",			MarkMove},
	{"MarkCopy",			MarkCopy},
	{"MarkUnmarked",		MarkUnmarked},
	{"ViewNew",			ViewNew},
	{"TocReply",			TocReply},
	{"TocForward",			TocForward},
	{"TocUseAsComposition",		TocUseAsComposition},
	{"CommitChanges",		CommitChanges},
	{"OpenSequence",		OpenSequence},
	{"AddToSequence",		AddToSequence},
	{"RemoveFromSequence",		RemoveFromSequence},
	{"DeleteSequence",		DeleteSequence},
	{"Pick",			PickMessages},
	{"PrintMessages",		PrintMessages},
	{"Pack",			Pack},
	{"Sort",			Sort},
	{"Rescan",			ForceRescan},

			/* viewButtons action procedures */

	{"CloseView",			CloseView},
	{"ViewReply",			ViewReply},
	{"ViewForward",			ViewForward},
	{"ViewUseAsComposition",	ViewUseAsComposition},
	{"EditView",			EditView},
	{"SaveView",			SaveView},
	{"PrintView",			PrintView},

			/* compButtons action procedures */

	/* Close button			CloseView */
	{"CompReset",			CompReset},
	/* Compose button 		ComposeMessage */
	{"SaveDraft",			SaveDraft},
	{"SendDraft",			SendDraft},
	{"MsgInsertAssoc",		MsgInsertAssoc},

			/* Menu Button action procedures */

        {"PopupFolderMenu",		PopupFolderMenu},
        {"SetCurrentFolder",		SetCurrentFolder},
        {"LeaveFolderButton",		LeaveFolderButton},
	{"OpenFolderFromMenu",		OpenFolderFromMenu}
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

    XtGetApplicationResources( toplevel, (caddr_t)&app_resources,
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

    NullSource = CreateFileSource(toplevel, "/dev/null", FALSE);

    l = strlen(app_resources.defMhPath) - 1;
    if (l > 0 && app_resources.defMhPath[l] == '/')
	app_resources.defMhPath[l] = 0;

    rootwidth = DisplayWidth(theDisplay, DefaultScreen(theDisplay));
    rootheight = DisplayHeight(theDisplay, DefaultScreen(theDisplay));

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

    XtAddActions(actions, XtNumber(actions));

    DEBUG("Making screen ... ")

    scrn = CreateNewScrn(STtocAndView);

    DEBUG(" setting toc ... ")

    TocSetScrn(InitialFolder, scrn);

    DEBUG("done.\n");

    MapScrn(scrn);
}
