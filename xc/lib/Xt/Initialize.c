#ifndef lint
static char rcsid[] = "$xHeader: Initialize.c,v 1.6 88/08/29 16:45:39 asente Exp $";
/* $oHeader: Initialize.c,v 1.6 88/08/29 16:45:39 asente Exp $ */
#endif lint

/***********************************************************
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

/* Make sure all wm properties can make it out of the resource manager */

#include <pwd.h>
#include <sys/param.h>

#include <stdio.h>

 /* Xlib definitions  */
 /* things like Window, Display, XEvent are defined herein */
#include "IntrinsicI.h"
#include "StringDefs.h"
#include "Shell.h"
#include "ShellP.h"
#include "ConvertI.h"
#include "Quarks.h"

/*
 This is a set of default records describing the command line arguments that
 Xlib will parse and set into the resource data base.
 
 This list is applied before the users list to enforce these defaults.  This is
 policy, which the toolkit avoids but I hate differing programs at this level.
*/

static XrmOptionDescRec opTable[] = {
{"-xrm",	NULL,		XrmoptionResArg,	(caddr_t) NULL},
{"-g",		".geometry",	XrmoptionSepArg,	(caddr_t) NULL},
{"-geometry",	".geometry",	XrmoptionSepArg,	(caddr_t) NULL},
{"-bd",		"*borderColor",	XrmoptionSepArg,	(caddr_t) NULL},
{"-bordercolor","*borderColor",	XrmoptionSepArg,	(caddr_t) NULL},
{"-bg",		"*background",	XrmoptionSepArg,	(caddr_t) NULL},
{"-background",	"*background",	XrmoptionSepArg,	(caddr_t) NULL},
{"-bw",		".borderWidth",	XrmoptionSepArg,	(caddr_t) NULL},
{"-borderwidth",".borderWidth",	XrmoptionSepArg,	(caddr_t) NULL},
{"-fg",		"*foreground",	XrmoptionSepArg,	(caddr_t) NULL},
{"-foreground",	"*foreground",	XrmoptionSepArg,	(caddr_t) NULL},
{"-fn",		"*font",	XrmoptionSepArg,	(caddr_t) NULL},
{"-font",	"*font",	XrmoptionSepArg,	(caddr_t) NULL},
{"-rv",		"*reverseVideo", XrmoptionNoArg,	(caddr_t) "on"},
{"-reverse",	"*reverseVideo", XrmoptionNoArg,	(caddr_t) "on"},
{"+rv",		"*reverseVideo", XrmoptionNoArg,	(caddr_t) "off"},
{"-n",		".name",	XrmoptionSepArg,	(caddr_t) NULL},
{"-name",	".name",	XrmoptionSepArg,	(caddr_t) NULL},
{"-selectionTimeout",
		".selectionTimeout", XrmoptionSepArg,	(caddr_t) NULL},
{"-synchronize","*synchronize",	XrmoptionNoArg,		(caddr_t) "on"},
{"+synchronize","*synchronize",	XrmoptionNoArg,		(caddr_t) "off"},
{"-title",	".title",	XrmoptionSepArg,	(caddr_t) NULL},
{"-t",		".title",	XrmoptionSepArg,	(caddr_t) NULL}
};

void XtToolkitInitialize()
{

    extern void _XtResourceListInitialize();

    /* Resource management initialization */
    XrmInitialize();
    _XtResourceListInitialize();

    /* Other intrinsic intialization */
    _XtConvertInitialize();
    _XtEventInitialize();
    _XtTranslateInitialize();
    _XtPerDisplayInitialize();
}

static String XtGetRootDirName(buf)
     String buf;
{
     int uid;
     extern char *getenv();
     extern int getuid();
     extern struct passwd *getpwuid();
     struct passwd *pw;
     static char *ptr = NULL;

     if (ptr == NULL) {
	if((ptr = getenv("HOME")) == NULL) {
	    if((ptr = getenv("USER")) != NULL) pw = getpwnam(ptr);
	    else {
		uid = getuid();
 		pw = getpwuid(uid);
	    }
	    if (pw) ptr = pw->pw_dir;
	    else ptr = NULL;
	}
     }

     if (ptr != NULL) {
 	(void) strcpy(buf, ptr);
	 buf += strlen(buf);
	 *buf = '/';
	 buf++;
	 *buf = '\0';
     }

     return buf;
}

static XrmDatabase GetAppSystemDefaults(classname)
     char *classname;
{
	XrmDatabase rdb;
	char	filenamebuf[MAXPATHLEN];
	char	*filename = &filenamebuf[0];

	(void) strcpy(filename, XAPPLOADDIR);
	(void) strcat(filename, classname);
	
	rdb = XrmGetFileDatabase(filename);

	return rdb;
}

static XrmDatabase GetAppUserDefaults(classname)
	char *classname;
{
	XrmDatabase rdb;
	char	filenamebuf[MAXPATHLEN];
	char	*filename = &filenamebuf[0];

	(void) XtGetRootDirName(filename);
	(void) strcat(filename, ".");
	(void) strcat(filename, classname);
	
	rdb = XrmGetFileDatabase(filename);

	return rdb;
}

static XrmDatabase GetUserDefaults(dpy)
	Display *dpy;
{
	XrmDatabase rdb;
	char	filenamebuf[MAXPATHLEN];
	char	*filename = &filenamebuf[0];

	if (dpy->xdefaults != NULL) {
	    rdb = XrmGetStringDatabase(dpy->xdefaults);
	} else {
	    (void) XtGetRootDirName(filename);
	    (void) strcat(filename, ".Xdefaults");
	    rdb = XrmGetFileDatabase(filename);
	}

	return rdb;
}

static XrmDatabase GetEnvironmentDefaults(dpy)
	Display *dpy;
{
	XrmDatabase rdb;
	extern char *getenv();
	char	filenamebuf[MAXPATHLEN];
	char	*filename = &filenamebuf[0];

	if ((filename = getenv("XENVIRONMENT")) == NULL) {
	    int len;
	    (void) XtGetRootDirName(filename = &filenamebuf[0]);
	    (void) strcat(filename, ".Xdefaults-");
	    len = strlen(filename);
	    gethostname(filename+len, MAXPATHLEN-len);
	}

	rdb = XrmGetFileDatabase(filename);
	return rdb;
}

static void GetInitialResourceDatabase(dpy, classname)
	Display *dpy;
	char *classname;
{
	XrmDatabase rdb;

	dpy->db = NULL;

	rdb = GetAppSystemDefaults(classname);
	if (rdb != NULL) XrmMergeDatabases(rdb, &(dpy->db));

	rdb = GetAppUserDefaults(classname);
	if (rdb != NULL) XrmMergeDatabases(rdb, &(dpy->db));

	rdb = GetUserDefaults(dpy);
	if (rdb != NULL) XrmMergeDatabases(rdb, &(dpy->db));

	rdb = GetEnvironmentDefaults(dpy);
	if (rdb != NULL) XrmMergeDatabases(rdb, &(dpy->db));
}

void _XtDisplayInitialize(dpy, app, name, classname, urlist, num_urs, argc, argv)
	Display *dpy;
	XtAppContext app;
	String name, classname;
	XrmOptionDescRec *urlist;
	Cardinal num_urs;
	Cardinal *argc;
	char *argv[];
{
	char names[1000], classes[1000], lowerName[1000];
	int namelen, classlen;
	char *return_type;
	XrmValue value;
	XrmQuark q;

	/* save first app name & class for R2 compatibility onlyX */
	if (app->class == NULL) {
	    app->name = XrmStringToName(name);
	    app->class = XrmStringToName(classname);
	}

	GetInitialResourceDatabase(dpy, classname);

	/*
	   This routine parses the command line arguments and removes them from
	   argv.
	 */
	XrmParseCommand(
	    (XrmDatabase *)&(dpy->db), opTable, (int) XtNumber(opTable),
	    name, (int *)argc, argv);

	namelen = strlen(name);
	classlen = strlen(classname);
	strcpy(names, name);
	strcat(names, ".synchronize");
	strcpy(classes, classname);
	strcat(classes, ".Synchronize");
	if (XrmGetResource((XrmDatabase) (dpy->db), names, classes,
		&return_type, &value)) {
	    LowerCase((char *) value.addr, lowerName);
	    q = XrmStringToQuark(lowerName);
	
	    if (q == XtQEtrue || q == XtQEon || q == XtQEyes) {
		app->sync = TRUE;
		(void) XSynchronize(dpy, TRUE);
	    }
	}

	names[namelen] = classes[classlen] = '\0';
	strcat(names, ".reverseVideo");
	strcat(classes, ".ReverseVideo");
	if (XrmGetResource((XrmDatabase) (dpy->db), names, classes,
		&return_type, &value)) {
	    LowerCase((char *) value.addr, lowerName);
	    q = XrmStringToQuark(lowerName);
	
	    if (q == XtQEtrue || q == XtQEon || q == XtQEyes) {
		app->rv = TRUE;
	    }
	}

	names[namelen] = classes[classlen] = '\0';
	strcat(names, ".selectionTimeout");
	strcat(classes, ".SelectionTimeout");
	if (XrmGetResource((XrmDatabase) (dpy->db), names, classes,
		&return_type, &value)) {
	    (void) sscanf((char *) value.addr, "%lu", &app->selectionTimeout);
	}

	if(num_urs != 0) { 	/* the application has some more defaults */
	    XrmParseCommand(
	        (XrmDatabase *)&(dpy->db), urlist, (int) num_urs,
		name, (int *)argc, argv);
	}
}
	
/*
 * This routine creates the desired widget and does the "Right Thing" for
 * the toolkit and for window managers.
 */

Widget XtInitialize(name, classname, urlist, num_urs, argc, argv)
	char *name;
	char *classname;
	XrmOptionDescRec *urlist;
	Cardinal num_urs;
	Cardinal *argc;
	char *argv[];
{
	Display *dpy;
	Screen *scrn;
	char **saved_argv;
	int saved_argc = *argc;
	Widget root;
	int i;
	Arg   args[8];
	Cardinal num_args = 0;

	XtToolkitInitialize();

	/* save away argv and argc so I can set the properties latter */

	saved_argv = (char **) XtCalloc(
		(unsigned) ((*argc) + 1) , (unsigned)sizeof(*saved_argv));

	for (i = 0 ; i < *argc ; i++) saved_argv[i] = argv[i];
	saved_argv[i] = NULL;

	dpy = XtOpenDisplay((XtAppContext) NULL, (String) NULL, name,
		classname, urlist, num_urs, argc, argv);
	if (dpy == NULL) {
             XtErrorMsg("invalidDisplay","xtInitialize","XtToolkitError",
                   "Can't Open display", (String *) NULL, (Cardinal *)NULL);
	}
	scrn = DefaultScreenOfDisplay(dpy);

        XtSetArg(args[num_args], XtNscreen, scrn);	num_args++;
	XtSetArg(args[num_args], XtNargc, saved_argc);	num_args++;
	XtSetArg(args[num_args], XtNargv, saved_argv);	num_args++;

        root = XtAppCreateShell(name, classname, 
		applicationShellWidgetClass, dpy, args, num_args);

	return root;
}
