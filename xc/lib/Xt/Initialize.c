/* "$XConsortium: Initialize.c,v 1.165 90/12/19 20:06:15 rws Exp $"; */
/* $oHeader: Initialize.c,v 1.7 88/08/31 16:33:39 asente Exp $ */

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

#ifdef pegasus
#undef dirty			/* some bozo put this in sys/param.h */
#endif /* pegasus */

#include <stdio.h>
#include "IntrinsicI.h"
#include "StringDefs.h"
#include "CoreP.h"
#include "ShellP.h"
#include "Quarks.h"

#if __STDC__
#define Const const
#else
#define Const /**/
#endif

extern void _XtConvertInitialize();

#if defined(SUNSHLIB) && defined(SHAREDCODE)
/*
 * If used as a shared library, generate code under a different name so that
 * the stub routines in sharedlib.c get loaded into the application binary.
 */
#define _XtInherit __XtInherit
#define XtToolkitInitialize _XtToolkitInitialize
#define XtAppInitialize _XtAppInitialize
#define XtInitialize _XtInitialize
#endif /* SUNSHLIB && SHAREDCODE */

/*
 * hpux
 * Hand-patched versions of HP-UX prior to version 7.0 can usefully add
 * -DUSE_UNAME in the appropriate config file to get long hostnames.
 */

#ifdef USG
#define USE_UNAME
#endif

#ifdef USE_UNAME
#include <sys/utsname.h>
#endif

/* some unspecified magic number of expected search levels for Xrm */
#define SEARCH_LIST_SIZE 1000

/*
 This is a set of default records describing the command line arguments that
 Xlib will parse and set into the resource data base.
 
 This list is applied before the users list to enforce these defaults.  This is
 policy, which the toolkit avoids but I hate differing programs at this level.
*/

static XrmOptionDescRec Const opTable[] = {
{"+rv",		"*reverseVideo", XrmoptionNoArg,	(XtPointer) "off"},
{"+synchronous","*synchronous",	XrmoptionNoArg,		(XtPointer) "off"},
{"-background",	"*background",	XrmoptionSepArg,	(XtPointer) NULL},
{"-bd",		"*borderColor",	XrmoptionSepArg,	(XtPointer) NULL},
{"-bg",		"*background",	XrmoptionSepArg,	(XtPointer) NULL},
{"-bordercolor","*borderColor",	XrmoptionSepArg,	(XtPointer) NULL},
{"-borderwidth",".borderWidth",	XrmoptionSepArg,	(XtPointer) NULL},
{"-bw",		".borderWidth",	XrmoptionSepArg,	(XtPointer) NULL},
{"-display",	".display",     XrmoptionSepArg,	(XtPointer) NULL},
{"-fg",		"*foreground",	XrmoptionSepArg,	(XtPointer) NULL},
{"-fn",		"*font",	XrmoptionSepArg,	(XtPointer) NULL},
{"-font",	"*font",	XrmoptionSepArg,	(XtPointer) NULL},
{"-foreground",	"*foreground",	XrmoptionSepArg,	(XtPointer) NULL},
{"-geometry",	".geometry",	XrmoptionSepArg,	(XtPointer) NULL},
{"-iconic",	".iconic",	XrmoptionNoArg,		(XtPointer) "on"},
{"-name",	".name",	XrmoptionSepArg,	(XtPointer) NULL},
{"-reverse",	"*reverseVideo", XrmoptionNoArg,	(XtPointer) "on"},
{"-rv",		"*reverseVideo", XrmoptionNoArg,	(XtPointer) "on"},
{"-selectionTimeout",
		".selectionTimeout", XrmoptionSepArg,	(XtPointer) NULL},
{"-synchronous","*synchronous",	XrmoptionNoArg,		(XtPointer) "on"},
{"-title",	".title",	XrmoptionSepArg,	(XtPointer) NULL},
{"-xnllanguage",".xnlLanguage",	XrmoptionSepArg,	(XtPointer) NULL},
{"-xrm",	NULL,		XrmoptionResArg,	(XtPointer) NULL},
};


/*
 * _XtGetHostname - emulates gethostname() on non-bsd systems.
 */

static int _XtGetHostname (buf, maxlen)
    char *buf;
    int maxlen;
{
    int len;

#ifdef USE_UNAME
    struct utsname name;

    uname (&name);
    len = strlen (name.nodename);
    if (len >= maxlen) len = maxlen - 1;
    (void) strncpy (buf, name.nodename, len);
    buf[len] = '\0';
#else
    buf[0] = '\0';
    (void) gethostname (buf, maxlen);
    buf [maxlen - 1] = '\0';
    len = strlen(buf);
#endif
    return len;
}


void _XtInherit()
{
    XtErrorMsg("invalidProcedure","inheritanceProc",XtCXtToolkitError,
            "Unresolved inheritance operation",
              (String *)NULL, (Cardinal *)NULL);
}


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
}


static String XtGetRootDirName(buf)
     String buf;
{
     int uid;
     extern char *getenv();
     extern int getuid();
     extern struct passwd *getpwuid();
     extern struct passwd *getpwnam();
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
	    else {
		ptr = NULL;
		*buf = '\0';
	    }
	}
     }

     if (ptr != NULL) 
 	(void) strcpy(buf, ptr);

     buf += strlen(buf);
     *buf = '/';
     buf++;
     *buf = '\0';
     return buf;
}

static Status CombineAppSystemDefaults(dpy, pdb)
    Display *dpy;
    XrmDatabase *pdb;
{
    char	*filename;
    Status	res;

    if ((filename = XtResolvePathname(dpy, "app-defaults",
				      NULL, NULL, NULL, NULL, 0, NULL))
	== NULL) {
	return 0;
    }

    res = XrmCombineFileDatabase(filename, pdb, False);
    XtFree(filename);
    return res;
}

static void CombineAppUserDefaults(dpy, pdb)
    Display *dpy;
    XrmDatabase *pdb;
{
    char* filename;
    char* path;
    Boolean deallocate = False;
    extern char *getenv();

    if ((path = getenv("XUSERFILESEARCHPATH")) == NULL) {
	char *old_path;
	char homedir[MAXPATHLEN];
	XtGetRootDirName(homedir);
	if ((old_path = getenv("XAPPLRESDIR")) == NULL) {
	    char *path_default = "%s/%%L/%%N%%C:%s/%%l/%%N%%C:%s/%%N%%C:%s/%%L/%%N:%s/%%l/%%N:%s/%%N";
	    if ((
	      path = ALLOCATE_LOCAL(6*strlen(homedir) + strlen(path_default)))
		== NULL) _XtAllocError(NULL);
	    sprintf( path, path_default,
		    homedir, homedir, homedir, homedir, homedir, homedir );
	} else {
	    char *path_default = "%s/%%L/%%N%%C:%s/%%l/%%N%%C:%s/%%N%%C:%s/%%N%%C:%s/%%L/%%N:%s/%%l/%%N:%s/%%N:%s/%%N";
	    if ((
	      path = ALLOCATE_LOCAL( 8*strlen(old_path)
				     + strlen(homedir) + strlen(path_default)))
		== NULL) _XtAllocError(NULL);
	    sprintf(path, path_default, old_path, old_path, old_path, homedir,
		    old_path, old_path, old_path, homedir );
	}
	deallocate = True;
    }

    filename = XtResolvePathname(dpy, NULL, NULL, NULL, path, NULL, 0, NULL);
    if (filename) {
	(void)XrmCombineFileDatabase(filename, pdb, False);
	XtFree(filename);
    }

    if (deallocate) DEALLOCATE_LOCAL(path);
}

static XrmDatabase GetUserDefaults(dpy)
	Display *dpy;
{
	char *dpy_defaults = XResourceManagerString(dpy);
	XrmDatabase rdb;

	if (dpy_defaults != NULL) {
	    rdb = XrmGetStringDatabase(dpy_defaults);
	} else {
	    char filename[MAXPATHLEN];
	    (void) XtGetRootDirName(filename);
	    (void) strcat(filename, ".Xdefaults");
	    rdb = XrmGetFileDatabase(filename);
	}

	return rdb;
}

static void CombineEnvironmentDefaults(pdb)
    XrmDatabase *pdb;
{
	XrmDatabase rdb;
	extern char *getenv();
	char	filenamebuf[MAXPATHLEN];
	char	*filename;

	if ((filename = getenv("XENVIRONMENT")) == NULL) {
	    int len;
	    (void) XtGetRootDirName(filename = filenamebuf);
	    (void) strcat(filename, ".Xdefaults-");
	    len = strlen(filename);
	    (void) _XtGetHostname (filename+len, MAXPATHLEN-len);
	}

	(void)XrmCombineFileDatabase(filename, pdb, False);
}

static void CombineFallbackDefaults(dpy, pdb)
        Display * dpy;
        XrmDatabase *pdb;
{
	XtPerDisplay pd = _XtGetPerDisplay(dpy);
        XrmDatabase db = NULL;
	String *res;

	if ( (res = pd->appContext->fallback_resources) == NULL)
	    return;
	
	for ( ; *res != NULL ; res++) {
	    XrmPutLineResource(&db, (char*)*res);
	}

	(void)XrmCombineDatabase(db, pdb, False);
}

static XrmDatabase GetInitialResourceDatabase(dpy, cmd_db, user_db)
	Display *dpy;
        XrmDatabase cmd_db;
        XrmDatabase user_db;
{
	XrmDatabase db = cmd_db;

	CombineEnvironmentDefaults(&db);
	if (user_db)
	    (void)XrmCombineDatabase(user_db, &db, False);
	if (!db)
	    db = XrmGetStringDatabase("");
	/* set database now, for XtResolvePathname to use */
	XrmSetDatabase(dpy, db);
	CombineAppUserDefaults(dpy, &db);
	if (!CombineAppSystemDefaults(dpy, &db))
	    CombineFallbackDefaults(dpy, &db);
	return db;
}


/*
 * Merge two option tables, allowing the second to over-ride the first,
 * so that ambiguous abbreviations can be noticed.  The merge attempts
 * to make the resulting table lexicographically sorted, but succeeds
 * only if the first source table is sorted.  Though it _is_ recommended
 * (for optimizations later in XrmParseCommand), it is not required
 * that either source table be sorted.
 *
 * Caller is responsible for freeing the returned option table.
 */

static void _MergeOptionTables(src1, num_src1, src2, num_src2, dst, num_dst)
    XrmOptionDescRec *src1, *src2;
    Cardinal num_src1, num_src2;
    XrmOptionDescRec **dst;
    Cardinal *num_dst;
{
    XrmOptionDescRec *table, *endP;
    register XrmOptionDescRec *opt1, *opt2, *whereP, *dstP; 
    int i1, i2, dst_len, order;
    Boolean found;
    enum {Check, NotSorted, IsSorted} sort_order = Check;

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
	whereP = endP-1;	/* assume new option goes at the end */
	for (opt1 = table, i1 = 0; i1 < dst_len; opt1++, i1++) {
	    /* have to walk the entire new table so new list is ordered
	       (if src1 was ordered) */
	    if (sort_order == Check && i1 > 0
		&& strcmp(opt1->option, (opt1-1)->option) < 0)
		sort_order = NotSorted;
	    if ((order = strcmp(opt1->option, opt2->option)) == 0) {
		/* same option names; just overwrite opt1 with opt2 */
		*opt1 = *opt2;
		found = True;
		break;
		}
	    /* else */
	    if (sort_order == IsSorted && order > 0) {
		/* insert before opt1 to preserve order */
		/* shift rest of table forward to make room for new entry */
		for (dstP = endP++; dstP > opt1; dstP--)
		    *dstP = *(dstP-1);
		*opt1 = *opt2;
		dst_len++;
		found = True;
		break;
	    }
	    /* else */
	    if (order < 0)
		/* opt2 sorts after opt1, so remember this position */
		whereP = opt1;
	}
	if (sort_order == Check && i1 == dst_len)
	    sort_order = IsSorted;
	if (!found) {
	   /* when we get here, whereP points to the last entry in the
	      destination that sorts before "opt2".  Shift rest of table
	      forward and insert "opt2" after whereP. */
	    whereP++;
	    for (dstP = endP++; dstP > whereP; dstP--)
		*dstP = *(dstP-1);
	    *whereP = *opt2;
	    dst_len++;
	}
    }
    *num_dst = dst_len;
}


/* NOTE: name, class, and type must be permanent strings */
static Boolean _GetResource(dpy, list, name, class, type, value)
    Display *dpy;
    XrmSearchList list;
    String name, class, type;
    XrmValue* value;
{
    XrmRepresentation db_type;
    XrmValue db_value;
    XrmName Qname = XrmPermStringToQuark(name);
    XrmClass Qclass = XrmPermStringToQuark(class);
    XrmRepresentation Qtype = XrmPermStringToQuark(type);

    if (XrmQGetSearchResource(list, Qname, Qclass, &db_type, &db_value)) {
	if (db_type == Qtype) {
	    if (Qtype == XtQString)
		*(String*)value->addr = db_value.addr;
	    else
		bcopy( db_value.addr, value->addr, value->size );
	    return True;
	} else {
	    WidgetRec widget; /* hack, hack */
	    bzero( &widget, sizeof(widget) );
	    widget.core.self = &widget;
	    widget.core.widget_class = coreWidgetClass;
	    widget.core.screen = (Screen*)DefaultScreenOfDisplay(dpy);
	    XtInitializeWidgetClass(coreWidgetClass);
	    if (_XtConvert(&widget,db_type,&db_value,Qtype,value,NULL)) {
		return True;
	    }
	}
    }
    return False;
}


void _XtDisplayInitialize(dpy, pd, name, class, urlist, num_urs, argc, argv)
	Display *dpy;
        XtPerDisplay pd;
	String name, class;
	XrmOptionDescRec *urlist;
	Cardinal num_urs;
	Cardinal *argc;
	char *argv[];
{
	Boolean tmp_bool;
	XrmRepresentation type;
	XrmValue value;
	XrmOptionDescRec *options;
	Cardinal num_options;
	XrmDatabase cmd_db = NULL;
	XrmDatabase user_db;
	XrmDatabase db;
	XrmName name_list[3];
	XrmClass class_list[3];
	XrmHashTable* search_list;
	int search_list_size = SEARCH_LIST_SIZE;
	extern char *getenv();

	_MergeOptionTables( opTable, XtNumber(opTable), urlist, num_urs,
			    &options, &num_options );

	/*
	   This routine parses the command line arguments and removes them from
	   argv.
	 */
	XrmParseCommand(&cmd_db, options, num_options, name, (int*)argc, argv);

	name_list[0] = pd->name;
	name_list[1] = XrmPermStringToQuark("xnlLanguage");
	name_list[2] = NULLQUARK;
	class_list[0] = pd->class;
	class_list[1] = XrmPermStringToQuark("XnlLanguage");
	class_list[2] = NULLQUARK;

	user_db = GetUserDefaults(dpy);

	if ((!cmd_db ||
	     !XrmQGetResource(cmd_db, name_list, class_list, &type, &value))
	    &&
	    (!user_db ||
	     !XrmQGetResource(user_db, name_list, class_list, &type, &value))){
	    if (!(pd->language = getenv("LANG")))
		pd->language = "";
	} else {
	    pd->language = (char *)value.addr;
	}

	db = GetInitialResourceDatabase(dpy, cmd_db, user_db);

	if ((
	  search_list = (XrmHashTable*)
	     ALLOCATE_LOCAL( SEARCH_LIST_SIZE*sizeof(XrmHashTable) ))
	    == NULL) _XtAllocError(NULL);

	while (!XrmQGetSearchList(db, name_list, class_list,
				  search_list, search_list_size)) {
	    XrmHashTable* old = search_list;
	    Cardinal size = (search_list_size*=2)*sizeof(XrmHashTable);
	    if ((
	      search_list = (XrmHashTable*)ALLOCATE_LOCAL(size))
		== NULL) _XtAllocError(NULL);
	    bcopy( (char*)old, (char*)search_list, (size>>1) );
	    DEALLOCATE_LOCAL(old);
	}

	value.size = sizeof(tmp_bool);
	value.addr = (XtPointer)&tmp_bool;
	if (_GetResource(dpy, search_list, "synchronous", "Synchronous",
			 XtRBoolean, &value)) {
	    int i;
	    Display **dpyP = pd->appContext->list;
	    pd->appContext->sync = tmp_bool;
	    for (i = pd->appContext->count; i; dpyP++, i--) {
		(void) XSynchronize(*dpyP, (Bool)tmp_bool);
	    }
	} else {
	    (void) XSynchronize(dpy, (Bool)pd->appContext->sync);
	}

	if (_GetResource(dpy, search_list, "reverseVideo", "ReverseVideo",
			 XtRBoolean, &value)
	        && tmp_bool) {
	    pd->rv = True;
	}

	value.size = sizeof(pd->multi_click_time);
	value.addr = (XtPointer)&pd->multi_click_time;
	if (!_GetResource(dpy, search_list,
			  "multiClickTime", "MultiClickTime",
			  XtRInt, &value)) {
	    pd->multi_click_time = 200;
	}

	value.size = sizeof(pd->appContext->selectionTimeout);
	value.addr = (XtPointer)&pd->appContext->selectionTimeout;
	(void)_GetResource(dpy, search_list,
			   "selectionTimeout", "SelectionTimeout",
			   XtRInt, &value);

#ifndef NO_IDENTIFY_WINDOWS
	value.size = sizeof(pd->appContext->identify_windows);
	value.addr = (XtPointer)&pd->appContext->identify_windows;
	(void)_GetResource(dpy, search_list,
			   "xtIdentifyWindows", "XtDebug",
			   XtRBoolean, &value);
#endif

	XtFree( (XtPointer)options );
	DEALLOCATE_LOCAL( search_list );
}

/*	Function Name: XtAppSetFallbackResources
 *	Description: Sets the fallback resource list that will be loaded
 *                   at display initialization time.
 *	Arguments: app_context - the app context.
 *                 specification_list - the resource specification list.
 *	Returns: none.
 */

#if NeedFunctionPrototypes
void
XtAppSetFallbackResources(
XtAppContext app_context,
String *specification_list
)
#else
void
XtAppSetFallbackResources(app_context, specification_list)
XtAppContext app_context;
String *specification_list;
#endif
{
    app_context->fallback_resources = specification_list;
}

/*	Function Name: XtAppInitialize
 *	Description: A convience routine for Initializing the toolkit.
 *	Arguments: app_context_return - The application context of the
 *                                      application
 *                 application_class  - The class of the application.
 *                 options            - The option list.
 *                 num_options        - The number of options in the above list
 *                 argc_in_out, argv_in_out - number and list of command line
 *                                            arguments.
 *                 fallback_resource  - The fallback list of resources.
 *                 args, num_args     - Arguements to use when creating the 
 *                                      shell widget.
 *	Returns: The shell widget.
 */
	
#if NeedFunctionPrototypes
Widget
XtAppInitialize(
XtAppContext * app_context_return,
_Xconst char* application_class,
XrmOptionDescRec *options,
Cardinal num_options,
Cardinal *argc_in_out,
String *argv_in_out,
String *fallback_resources,
ArgList args_in,
Cardinal num_args_in
)
#else
Widget
XtAppInitialize(app_context_return, application_class, options, num_options,
		argc_in_out, argv_in_out, fallback_resources, 
		args_in, num_args_in)
XtAppContext * app_context_return;
String application_class;
XrmOptionDescRec *options;
Cardinal num_options, *argc_in_out, num_args_in;
String *argv_in_out, * fallback_resources;     
ArgList args_in;
#endif
{
    XtAppContext app_con;
    Display * dpy;
    String *saved_argv;
    register int i, saved_argc = *argc_in_out;
    Widget root;
    Arg args[3], *merged_args;
    Cardinal num = 0;
    
    XtToolkitInitialize();
    
/*
 * Save away argv and argc so we can set the properties later 
 */
    
    saved_argv = (String *)
	ALLOCATE_LOCAL( (Cardinal)((*argc_in_out + 1) * sizeof(String)) );

    for (i = 0 ; i < saved_argc ; i++) saved_argv[i] = argv_in_out[i];
    saved_argv[i] = NULL;	/* NULL terminate that sucker. */


    app_con = XtCreateApplicationContext();

    if (fallback_resources != NULL) /* save a procedure call */
	XtAppSetFallbackResources(app_con, fallback_resources);

    dpy = XtOpenDisplay(app_con, (String) NULL, NULL, application_class,
			options, num_options, argc_in_out, argv_in_out);

    if (dpy == NULL)
	XtErrorMsg("invalidDisplay","xtInitialize",XtCXtToolkitError,
                   "Can't Open display", (String *) NULL, (Cardinal *)NULL);

    XtSetArg(args[num], XtNscreen, DefaultScreenOfDisplay(dpy)); num++;
    XtSetArg(args[num], XtNargc, saved_argc);	                 num++;
    XtSetArg(args[num], XtNargv, saved_argv);	                 num++;

    merged_args = XtMergeArgLists(args_in, num_args_in, args, num);
    num += num_args_in;

    root = XtAppCreateShell(NULL, application_class, 
			    applicationShellWidgetClass,dpy, merged_args, num);
    
    if (app_context_return != NULL)
	*app_context_return = app_con;

    XtFree((XtPointer)merged_args);
    DEALLOCATE_LOCAL((XtPointer)saved_argv);
    return(root);
}

/*	Function Name: XtInitialize
 *	Description: This function can be used to initialize the toolkit.
 *		     It is obsolete; XtAppInitialize is more useful.
 *	Arguments: name - ** UNUSED **
 *                 classname - name of the application class.
 *                 options, num_options - the command line option info.
 *                 argc, argc - the command line args from main().
 *	Returns: a shell widget.
 */
	
/*ARGSUSED*/
#if NeedFunctionPrototypes
Widget 
XtInitialize(
_Xconst char* name,
_Xconst char* classname,
XrmOptionDescRec *options,
Cardinal num_options,
Cardinal *argc,
String *argv
)
#else
Widget 
XtInitialize(name, classname, options, num_options, argc, argv)
String name, classname;
XrmOptionDescRec *options;
Cardinal num_options, *argc;
String *argv;
#endif
{
    Widget root;
    XtAppContext app_con;
    register ProcessContext process = _XtGetProcessContext();

    root = XtAppInitialize(&app_con, classname, options, num_options,
			   argc, argv, NULL, NULL, (Cardinal) 0);

    process->defaultAppContext = app_con;
    
    return(root);
}
