#ifndef lint
static char rcsid[] = "$Header: Initialize.c,v 6.16 88/01/29 12:28:27 asente Exp $";
#endif lint

/*
 * Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.
 * 
 *                         All Rights Reserved
 * 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in 
 * supporting documentation, and that the name of Digital Equipment
 * Corporation not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.  
 * 
 * 
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */
/* Make sure all wm properties can make it out of the resource manager */

#include <pwd.h>
#include <stdio.h>
#include <strings.h>
#include <sys/param.h>

 /* Xlib definitions  */
 /* things like Window, Display, XEvent are defined herein */
#include "IntrinsicI.h"
#include "Event.h"
#include "Atoms.h"
#include "Shell.h"
#include "ShellP.h"


/*
 This is a set of default records describing the command line arguments that
 Xlib will parse and set into the resource data base.
 
 This list is applied before the users list to enforce these defaults.  This is
 policy, which the toolkit avoids but I hate differing programs at this level.
*/

static XrmOptionDescRec opTable[] = {
{"-xrm",	NULL,		XrmoptionResArg,	(caddr_t) NULL},
#ifndef TRASHEQUALGEOMETRY
{"=",		".geometry",	XrmoptionIsArg,		(caddr_t) NULL},
#endif
{"-g",		".geometry",	XrmoptionIsArg,		(caddr_t) NULL},
{"-geometry",	".geometry",	XrmoptionIsArg,		(caddr_t) NULL},
{"-bd",		"*border",	XrmoptionSepArg,	(caddr_t) NULL},
{"-bordercolor","*border",	XrmoptionSepArg,	(caddr_t) NULL},
{"-bg",		"*background",	XrmoptionSepArg,	(caddr_t) NULL},
{"-background",	"*background",	XrmoptionSepArg,	(caddr_t) NULL},
{"-bw",		".borderWidth",	XrmoptionSepArg,	(caddr_t) NULL},
{"-border",	".borderWidth",	XrmoptionSepArg,	(caddr_t) NULL},
{"-fg",		"*foreground",	XrmoptionSepArg,	(caddr_t) NULL},
{"-foreground",	"*foreground",	XrmoptionSepArg,	(caddr_t) NULL},
{"-fn",		"*font",	XrmoptionSepArg,	(caddr_t) NULL},
{"-font",	"*font",	XrmoptionSepArg,	(caddr_t) NULL},
{"-rv",		"*reverseVideo", XrmoptionNoArg,	(caddr_t) "on"},
{"-reverse",	"*reverseVideo", XrmoptionNoArg,	(caddr_t) "on"},
{"+rv",		"*reverseVideo", XrmoptionNoArg,	(caddr_t) "off"},
{"-n",		".name",	XrmoptionSepArg,	(caddr_t) NULL},
{"-name",	".name",	XrmoptionSepArg,	(caddr_t) NULL},
{"-title",	".title",	XrmoptionSepArg,	(caddr_t) NULL},
{"-t",		".title",	XrmoptionSepArg,	(caddr_t) NULL}
};


/****************************************************************
 *
 * Private Procedures
 *
 ****************************************************************/

/* this is the old initialize routine */
/* This needs to be updated to reflect the new world. */
static void
DO_Initialize() {

extern void _XtResourceListInitialize();

    /* Resource management initialization */
    XrmInitialize();
    _XtResourceListInitialize();

    /* Other intrinsic intialization */
    _XtConvertInitialize();
    _XtEventInitialize();
    _XtTranslateInitialize();
}
Atom XtHasInput;
Atom XtTimerExpired;
extern Atom WM_CONFIGURE_DENIED;
extern Atom WM_MOVED;
static void
init_atoms(dpy)
Display *dpy;
{
	XtHasInput = XInternAtom(dpy, "XtHasInput", False);
	XtTimerExpired = XInternAtom(dpy, "XtTimerExpired", False);
	WM_CONFIGURE_DENIED = XInternAtom(dpy, "WM_CONFIGURE_DENIED", False);
	WM_MOVED= XInternAtom(dpy, "WM_MOVED", False);
}


/*
    Load the resource data base.

    The order that things are loaded into the database follows,
    1) an optional application specific file.
    2) Server defaults
    3) .Xdefaults if no server defaults

 */

static XrmDatabase XGetUsersDatabase(dpy, name)
Display *dpy;
char *name;
{
	XrmDatabase rdb;

	char filenamebuf[1024];
	char *filename = &filenamebuf[0];
	static Boolean first = TRUE;
	
	if(name != NULL) { /* application provided a file */
		rdb = XrmGetFileDatabase(name);
		XrmMergeDatabases(rdb, &XtDefaultDB);
	}

	if (first) {
		first = FALSE;
		/* MERGE server defaults */
		if (dpy->xdefaults != NULL) {
		    rdb = XrmGetStringDatabase(dpy->xdefaults);
		} else {
#ifdef notdef
/* put this stuff in if Xlib stops getting property off of the root window */
/* on XOpenDisplay */
		/* get the resource manager database off the root window. */
		    Atom actual_type;
		    int actual_format;
		    unsigned long nitems;
		    long leftover;
		    if (XGetWindowProperty(dpy, DefaultRootWindow(dpy),
			XA_RESOURCE_MANAGER, 0L, 100000000L, FALSE, XA_STRING,
			&actual_type, &actual_format, &nitems, &leftover,
			&dpy->xdefaults) != Success) {
			dpy->xdefaults = (char *) NULL;
			} else if ( (actual_type != XA_STRING)
			    ||  (actual_format != 8) ) {
			    if (dpy->xdefaults != NULL)
			        XtFree ( dpy->xdefaults );
			    dpy->xdefaults = (char *) NULL;
			    }
#endif
		/* Open .Xdefaults file and merge into existing data base */
		    int uid;
		    extern char *getenv();
		    extern int getuid();
		    extern struct passwd *getpwuid();
		    struct passwd *pw;
		    register char *ptr;

		    if((ptr = getenv("HOME")) != NULL) {
			    (void) strcpy(filename, ptr);

		    } else {
			    if((ptr = getenv("USER")) != NULL) {
				    pw = getpwnam(ptr);
			    } else {
				    uid = getuid();
				    pw = getpwuid(uid);
			    }
			    if (pw) {
				    (void) strcpy(filename, pw->pw_dir);
			    }
		    }
		    (void) strcat(filename, "/.Xdefaults");

		    rdb = XrmGetFileDatabase(filename);
		}
		XrmMergeDatabases(rdb, &XtDefaultDB);
	}
    return rdb;
}

/*
 * This routine creates the desired widget and does the "Right Thing" for
 * the toolkit and for window managers.
 */

Widget
XtInitialize(name, classname, urlist, num_urs, argc, argv)
    char *name;
    char *classname;
    XrmOptionDescRec *urlist;
    Cardinal num_urs;
    Cardinal *argc;
    char *argv[];
{
	char  displayName[256];
	Arg   args[8];
	Cardinal num_args = 0;
	int i;
	char filename[MAXPATHLEN];
	char **saved_argv;
	int    saved_argc = *argc;
	Display *dpy;
	char *ptr, *rindex();
	ApplicationShellWidget w;
	Widget root;
#ifndef TRASHCOLONDISPLAY
	int squish = -1;
#endif
	Boolean dosync = FALSE;

	if( name == NULL) {
	  	ptr = rindex(argv[0], '/');
		if(ptr)
		  name = ++ ptr;
		else
		  name = argv[0];
	}

	/* save away argv and argc so I can set the properties latter */

	saved_argv = (char **) XtCalloc(
	    (unsigned) ((*argc) + 1) , (unsigned)sizeof(*saved_argv));
	for (i = 0 ; i < *argc ; i++)
	  saved_argv[i] = argv[i];
	saved_argv[i] = NULL;
	/*
	   Find the display name and open it
	   While we are at it we look for name because that is needed 
	   soon after to do the arguement parsing.
	 */
	displayName[0] = 0;

	for(i = 1; i < *argc; i++) {
#ifndef TRASHCOLONDISPLAY
	  if (index(argv[i], ':') != NULL) {
		  (void) strncpy(displayName, argv[i], sizeof(displayName));
		  if( *argc == i + 1) {
		    (*argc)--;
		  } else {  /* need to squish this one out of the list */
		    squish = i;
		  }
		  continue;
	  }
#endif
	  if (!strcmp("-d", argv[i]) || !strcmp("-display", argv[i])) {
		  i++;
		  if(i == *argc) break;
		  (void) strncpy(displayName, argv[i], sizeof(displayName));
		  continue;
	  }
	  if(!strcmp("-name", argv[i]) || ! strcmp("-n", argv[i])) {
		  i++;
		  if(i == *argc) break;
		  name = argv[i];
		  continue;
	  }
	  if (!strcmp("-sync", argv[i])) {
		  dosync = TRUE;
		  continue;
	  }
	}
#ifndef TRASHCOLONDISPLAY
	if(squish != -1) {
		(*argc)--;
		for(i = squish; i < *argc; i++) {
			argv[i] = argv[i+1];
		}
	}
#endif
	/* Open display  */
	if (!(dpy = XOpenDisplay(displayName))) {
		char buf[1024];
		(void) strcpy(buf, "Can't Open display: ");
		(void) strcat(buf, displayName);
		XtError(buf);
	}
        toplevelDisplay = dpy;
	if (dosync) XSynchronize(dpy, TRUE);

        XtSetArg(args[num_args], "display", dpy);
        num_args++;
        XtSetArg(args[num_args], "screen", dpy->default_screen);
        num_args++;
	    
	/* initialize the toolkit */
	DO_Initialize();
#define XAPPLOADDIR  "SYS$LIBRARY:"
	(void) strcpy(filename, XAPPLOADDIR);
	(void) strcat(filename, classname);

	/*set up resource database */
	(void) XGetUsersDatabase(dpy, filename);

	/*
	   This routine parses the command line arguments and removes them from
	   argv.
	 */
	XrmParseCommand(&XtDefaultDB, opTable, XtNumber(opTable), name,
	    argc, argv);
	
	if(num_urs != 0) {
	       /* the application has some more defaults */
	       XrmParseCommand(&XtDefaultDB, urlist, num_urs, name, argc, argv);
	}
	/* Resources are initialize and loaded */
	/* I now must handle geometry specs a compond resource */

	/*
	     Create the shell level widget.
	     Unlike most classes the shell widget class has no classname
	     The name is supplied in the call to XtInitialize.
	 */
	(void) strcpy(
	    applicationShellWidgetClass->core_class.class_name
	        = (String)XtMalloc((unsigned)strlen(classname)+1),
	       classname);
        root = XtCreateApplicationShell(name, applicationShellWidgetClass,
                  args,num_args);


	w = (ApplicationShellWidget) root;
	w->application.argc = saved_argc;
	w->application.argv = saved_argv;

	init_atoms(dpy);

	return(root);
}
