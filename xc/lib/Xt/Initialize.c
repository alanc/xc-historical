#ifndef lint
static char rcsid[] = "$Header: Initialize.c,v 1.101 88/02/02 16:56:23 swick Locked $";
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
#include <X/Xos.h>
#include <sys/param.h>

 /* Xlib definitions  */
 /* things like Window, Display, XEvent are defined herein */
#include "IntrinsicI.h"
#include "Event.h"
#include "Atoms.h"
#include "Shell.h"
#include "ShellP.h"

#define min(a, b)     (((a) < (b)) ? (a) : (b))

/*
 This is a set of default records describing the command line arguments that
 Xlib will parse and set into the resource data base.
 
 This list is applied before the users list to enforce these defaults.  This is
 policy, which the toolkit avoids but I hate differing programs at this level.
*/

static XrmOptionDescRec opTable[] = {
{"+rv",		"*reverseVideo", XrmoptionNoArg,	(caddr_t) "off"},
{"-background",	"*background",	XrmoptionSepArg,	(caddr_t) NULL},
{"-bd",		"*border",	XrmoptionSepArg,	(caddr_t) NULL},
{"-bg",		"*background",	XrmoptionSepArg,	(caddr_t) NULL},
{"-border",	".borderWidth",	XrmoptionSepArg,	(caddr_t) NULL},
{"-bordercolor","*border",	XrmoptionSepArg,	(caddr_t) NULL},
{"-bw",		".borderWidth",	XrmoptionSepArg,	(caddr_t) NULL},
{"-display",	".display",	XrmoptionSepArg,	(caddr_t) NULL},
{"-fg",		"*foreground",	XrmoptionSepArg,	(caddr_t) NULL},
{"-fn",		"*font",	XrmoptionSepArg,	(caddr_t) NULL},
{"-font",	"*font",	XrmoptionSepArg,	(caddr_t) NULL},
{"-foreground",	"*foreground",	XrmoptionSepArg,	(caddr_t) NULL},
{"-geometry",	".geometry",	XrmoptionSepArg,	(caddr_t) NULL},
{"-iconic",	".iconic",	XrmOptionNoArg,		(caddr_t) "on"},
{"-name",	".name",	XrmoptionSepArg,	(caddr_t) NULL},
{"-reverse",	"*reverseVideo", XrmoptionNoArg,	(caddr_t) "on"},
{"-rv",		"*reverseVideo", XrmoptionNoArg,	(caddr_t) "on"},
{"-synchronous",".synchronous", XrmoptionNoArg,		(caddr_t) "on"},
{"-title",	".title",	XrmoptionSepArg,	(caddr_t) NULL},
{"-xrm",	NULL,		XrmoptionResArg,	(caddr_t) NULL},
#ifndef TRASHEQUALGEOMETRY
{"=",		".geometry",	XrmoptionIsArg,		(caddr_t) NULL},
#endif
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
 * Merge two option tables, allowing the second to over-ride the first,
 * so that ambiguous abbreviations can be noticed.  The merge attempts
 * to make the resulting table lexicographically sorted, but succeeds
 * only if the first source table is sorted.
 */

void
XtMergeOptionTables(src1, num_src1, src2, num_src2, dst, num_dst)
    XrmOptionDescRec *src1, *src2;
    Cardinal num_src1, num_src2;
    XrmOptionDescRec **dst;
    Cardinal *num_dst;
{
    XrmOptionDescRec *table, *endP;
    register XrmOptionDescRec *opt1, *opt2, *dstP, *startP; 
    int i1, i2, len1, len2, dst_len, order;
    Boolean found;

    *dst = table = (XrmOptionDescRec*)
	XtMalloc( sizeof(XrmOptionDescRec) * (num_src1 + num_src2) );

    bcopy( src1, table, sizeof(XrmOptionDescRec) * num_src1 );
    if (num_src2 == 0) {
	*num_dst = num_src1;
	return;
    }
    endP = &table[dst_len = num_src1];
    for (opt2 = src2, i2= 0; i2 < num_src2; opt2++, i2++) {
	found = False;
	for (opt1 = table, i1 = 0; i1 < dst_len; opt1++, i1++) {
	    /* have to walk the entire new table so new list is ordered */
	    if ((order = strcmp(opt1->option, opt2->option)) == 0) {
		*opt1 = *opt2;
		found = True;
		break;
	    }
	    len1 = strlen(opt1->option);
	    len2 = strlen(opt2->option);
	    if (len1 != len2 &&
		strncmp(opt1->option, opt2->option, min(len1, len2)) == 0) {
		/* one is abbrev of the other; make sure shorter one
		   is found first in the table.  We'll shift the
		   remainder of the table to try to preserve sort if
		   src1 was sorted. */
		startP = opt1;
		if (len1 < len2) startP++;
		for (dstP = endP; dstP > startP;)
		    *dstP-- = *(dstP-1);
		*startP = *opt2;
		dst_len++;
		endP++;
		found = True;
		break;
	    }
	    if (order < 0) startP = opt1+1;
	}
	if (!found) {		/* insert after startP to preserve order */
	    for (dstP = endP; dstP > startP;)
		*dstP-- = *(dstP-1);
	    *startP = *opt2;
	    dst_len++;
	    endP++;
	}
    }
    *num_dst = dst_len;
}


static void
ComputeAbbrevLen(string, name, len)
    String string;		/* the variable */
    String name;		/* the constant */
    int *len;			/* the current ambiguous length */
{
    int string_len = strlen(string);
    int name_len = strlen(name);
    int i;

    for (i=0; i<string_len && i<name_len && *string++ == *name++; i++);

    if (i < name_len && i > *len)
	*len = i;
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
	XrmOptionDescRec *options;
	Cardinal num_options;
	Boolean found_display = FALSE;
	int min_display_len = 0;
	int min_name_len = 0;
	int min_sync_len = 0;

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

	XtMergeOptionTables( opTable, XtNumber(opTable), urlist, num_urs,
			     &options, &num_options );

	for (i = 0; i < num_options; i++) {
	    ComputeAbbrevLen(options[i].option, "-display",  &min_display_len);
	    ComputeAbbrevLen(options[i].option, "-name",        &min_name_len);
	    ComputeAbbrevLen(options[i].option, "-synchronous", &min_sync_len);
	}

	for(i = 1; i < *argc; i++) {
	  int len = strlen(argv[i]);
#ifndef TRASHCOLONDISPLAY
	  if (!found_display && index(argv[i], ':') != NULL) {
		  (void) strncpy(displayName, argv[i], sizeof(displayName));
		  squish = i;
		  continue;
	  }
#endif
	  if(len > min_display_len && !strncmp("-display", argv[i], len)) {
	          i++;
		  if(i == *argc) break;
		  strncpy(displayName, argv[i], sizeof(displayName));
		  found_display = TRUE;
		  continue;
	  }
	  if(len > min_name_len && !strncmp("-name", argv[i], len)) {
		  i++;
		  if(i == *argc) break;
		  name = argv[i];
		  continue;
	  }
	  if (len > min_sync_len && !strncmp("-synchronous", argv[i], len)) {
		  dosync = TRUE;
		  continue;
	  }
	}
#ifndef TRASHCOLONDISPLAY
	if(!found_display && squish != -1) {
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
#ifndef VMS
#define XAPPLOADDIR  "/usr/lib/X/app-defaults/"
#else
#define XAPPLOADDIR  "SYS$LIBRARY:"
#endif VMS
	(void) strcpy(filename, XAPPLOADDIR);
	(void) strcat(filename, classname);

	/*set up resource database */
	(void) XGetUsersDatabase(dpy, filename);

	/*
	   This routine parses the command line arguments and removes them from
	   argv.
	 */
	XrmParseCommand(&XtDefaultDB, options, num_options, name, argc, argv);
	XtFree( (char*)options );

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
