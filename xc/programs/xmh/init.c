#ifndef lint
static char rcs_id[] = "$Header: init.c,v 1.7 87/10/09 14:01:36 weissman Exp $";
#endif lint
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
 * IF THE SOFTWARE IS MODIFIED IN A MANNER CREATING DERIVATIVE COPYRIGHT RIGHTS,
 * APPROPRIATE LEGENDS MAY BE PLACED ON THE DERIVATIVE WORK IN ADDITION TO THAT
 * SET FORTH ABOVE.
 *
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting documentation,
 * and that the name of Digital Equipment Corporation not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission.
 */

/* Init.c - Handle start-up initialization. */

#include "xmh.h"

/* Xmh-specific resources. */

static XtResource resources[] = {
    {"debug", "Debug", XrmRBoolean, sizeof(Boolean),
	 (Cardinal)&debug, XrmRString, "on"},
    {"tempdir", "tempDir", XrmRString, sizeof(char *),
	 (Cardinal)&tempDir, XrmRString, "/tmp"},
    {"mhpath", "MhPath", XrmRString, sizeof(char *),
	 (Cardinal)&defMhPath, XrmRString, "/usr/local/mh6"},
    {"initialfolder", "InitialFolder", XrmRString, sizeof(char *),
	 (Cardinal)&initialFolderName, XrmRString, "inbox"},
    {"draftsfolder", "DraftsFolder", XrmRString, sizeof(char *),
	 (Cardinal)&draftsFolderName, XrmRString, "drafts"},
    {"sendwidth", "SendWidth", XrmRInt, sizeof(int),
	 (Cardinal)&defSendLineWidth, XrmRString, "72"},
    {"sendbreakwidth", "SendBreakWidth", XrmRInt, sizeof(int),
	 (Cardinal)&defBreakSendLineWidth, XrmRString, "85"},
    {"printcommand", "PrintCommand", XrmRString, sizeof(char *),
	 (Cardinal)&defPrintCommand, XrmRString,
	 "enscript > /dev/null 2>/dev/null"},
    {"tocwidth", "TocWidth", XrmRInt, sizeof(int),
	 (Cardinal)&defTocWidth, XrmRString, "100"},
    {"skipdeleted", "SkipDeleted", XrmRBoolean, sizeof(Boolean),
	 (Cardinal)&SkipDeleted, XrmRString, "True"},
    {"skipmoved", "SkipMoved", XrmRBoolean, sizeof(Boolean),
	 (Cardinal)&SkipMoved, XrmRString, "True"},
    {"skipCopied", "SkipCopied", XrmRBoolean, sizeof(Boolean),
	 (Cardinal)&SkipCopied, XrmRString, "False"},
    {"hideboringheaders", "HideBoringHeaders", XrmRBoolean, sizeof(Boolean),
	 (Cardinal)&defHideBoringHeaders, XrmRString, "True"},
    {"geometry", "Geometry", XrmRString, sizeof(char *),
	 (Cardinal)&defGeometry, XrmRString, ""},
    {"tocgeometry", "TocGeometry", XrmRString, sizeof(char *),
	 (Cardinal)&defTocGeometry, XrmRString, NULL},
    {"viewgeometry", "ViewGeometry", XrmRString, sizeof(char *),
	 (Cardinal)&defViewGeometry, XrmRString, NULL},
    {"compgeometry", "CompGeometry", XrmRString, sizeof(char *),
	 (Cardinal)&defCompGeometry, XrmRString, NULL},
    {"pickgeometry", "PickGeometry", XrmRString, sizeof(char *),
	 (Cardinal)&defPickGeometry, XrmRString, NULL},
    {"tocpercentage", "TocPercentage", XrmRInt, sizeof(int),
	 (Cardinal)&defTocPercentage, XrmRString, "33"},
    {"checknewmail", "CheckNewMail", XrmRBoolean, sizeof(Boolean),
	 (Cardinal)&defNewMailCheck, XrmRString, "True"},
    {"makecheckpoints", "MakeCheckPoints", XrmRBoolean, sizeof(Boolean),
	 (Cardinal)&defMakeCheckpoints, XrmRString, "False"},
};

static XrmOptionDescRec table[] = {
    {"-debug",	"debug",	XrmoptionNoArg,	"on"}
};

/* Tell the user how to use this program. */
Syntax()
{
    extern void exit();
    (void)fprintf(stderr, "usage:  xmh [display] [=geometry] \n");
    exit(2);
}


static char *FixUpGeometry(geo, defwidth, defheight)
char *geo;
Dimension defwidth, defheight;
{
    int gbits;
    Position x, y;
    Dimension width, height;
    if (geo == NULL) geo = defGeometry;
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


/* All the start-up initialization goes here. */

InitializeWorld(argc, argv)
unsigned int argc;
char **argv;
{
    int l;
    FILEPTR fid;
    XrmResourceDataBase db, db2;
    char str[500], str2[500], *ptr;
    XrmNameList names;
    XrmClassList classes;
    Scrn scrn;
    static XtActionsRec actions[] = {
	{"open-folder", OpenFolder},
	{"open-sequence", OpenSequence},
    };

    ptr = rindex(argv[0], '/');
    if (ptr) progName = ptr + 1;
    else progName = argv[0];

    toplevel = XtInitialize(progName, "Xmh", table, XtNumber(table),
			    &argc, argv);
/*    if (argc > 1) Syntax();*/
    theDisplay = XtDisplay(toplevel);
    theScreen = DefaultScreen(theDisplay);

    homeDir = MallocACopy(getenv("HOME"));

    (void) sprintf(str, "%s/.mh_profile", homeDir);
    fid = myfopen(str, "r");
    if (fid) {
	while (ptr = ReadLine(fid)) {
	    if (strncmp(ptr, "Path:", 5) == 0) {
		ptr += 5;
		while (*ptr == ' ' || *ptr == '\t')
		    ptr++;
		(void) strcpy(str, ptr);
	    }
	}
	(void) myfclose(fid);
    } else {
	fid = myfopen(str, "w");
	if (fid) {
	    (void) fprintf(fid, "Path: Mail\n");
	    (void) myfclose(fid);
	} else Punt("Can't read or create .mh_profile!");
	(void) strcpy(str, "Mail");
    }
    for (l = strlen(str) - 1; l >= 0 && (str[l] == ' ' || str[l] == '\t'); l--)
	str[l] = 0;
    if (str[0] == '/')
	(void) strcpy(str2, str);
    else
	(void) sprintf(str2, "%s/%s", homeDir, str);
    mailDir = MallocACopy(str2);
    (void) sprintf(str, "%s/draft", mailDir);
    draftFile = MallocACopy(str);
    (void) sprintf(str, "%s/xmhdraft", mailDir);
    xmhDraftFile = MallocACopy(str);

    defViewGeometry = defCompGeometry = defPickGeometry = NULL;

    XtGetSubresources((Widget) toplevel, (caddr_t) NULL, progName, "Xmh",
		      resources, XtNumber(resources), NULL, (Cardinal) 0);

    NullSource = XtCreateEDiskSource("/dev/null", FALSE);

    l = strlen(defMhPath) - 1;
    if (l > 0 && defMhPath[l] == '/')
	defMhPath[l] = 0;

    rootwidth = DisplayWidth(theDisplay, theScreen);
    rootheight = DisplayHeight(theDisplay, theScreen);

    defTocGeometry = FixUpGeometry(defTocGeometry,
				   rootwidth / 2, 3 * rootheight / 4);
    defViewGeometry = FixUpGeometry(defViewGeometry,
				    rootwidth / 2, rootheight / 2);
    defCompGeometry = FixUpGeometry(defCompGeometry,
				    rootwidth / 2, rootheight / 2);
    defPickGeometry = FixUpGeometry(defPickGeometry,
				    rootwidth / 2, rootheight / 2);

    numScrns = 0;
    scrnList = (Scrn *) XtMalloc((unsigned) 1);
    LastButtonPressed = NULL;

    TocInit();
    InitPick();
    IconInit();

    XtAddActions(actions, XtNumber(actions));

if (debug) {(void)fprintf(stderr, "Making screen ... "); (void)fflush(stderr);}

    scrn = CreateNewScrn(STtocAndView);

if (debug) {(void)fprintf(stderr, " setting toc ... "); (void)fflush(stderr);}

    TocSetScrn(InitialFolder, scrn);

if (debug) (void)fprintf(stderr, "done\n");

/* if (debug) {(void)fprintf(stderr, "Syncing ... "); (void)fflush(stderr); XSync(theDisplay, 0); (void)fprintf(stderr, "done\n");} */

    MapScrn(scrn);
}
