/*
 * xdm - display manager daemon
 *
 * $XConsortium: greet.c,v 1.12 89/07/16 16:11:41 jim Exp $
 *
 * Copyright 1988 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

/*
 * widget to get username/password
 *
 */

# include <X11/Intrinsic.h>
# include <X11/StringDefs.h>
# include <X11/Shell.h>

# include <X11/Xaw/Command.h>
# include <X11/Xaw/Logo.h>

# include "Login.h"
# include "dm.h"

extern Display	*dpy;

extern void	exit ();
static int	done;
static char	name[128], password[128];
static Widget		toplevel;
static Widget		login;
static XtAppContext	context;

/*ARGSUSED*/
GreetDone (w, data, status)
    Widget	w;
    LoginData	*data;
    int		status;
{
	Debug ("GreetDone: %s, %s\n", data->name, data->passwd);
	switch (status) {
	case NOTIFY_OK:
		strcpy (name, data->name);
		strcpy (password, data->passwd);
		done = 1;
		break;
	case NOTIFY_ABORT:
		Debug ("ABORT_DISPLAY\n");
		exit (ABORT_DISPLAY);
	case NOTIFY_RESTART:
		Debug ("RESTART_DISPLAY\n");
		exit (RESTART_DISPLAY);
	case NOTIFY_ABORT_DISPLAY:
		Debug ("DISABLE_DISPLAY\n");
		exit (DISABLE_DISPLAY);
	}
}

Display *
InitGreet (d)
struct display	*d;
{
	Arg		arglist[10];
	int		i;
	static int	argc;
	Screen		*scrn;
	static char	*argv[] = { "xlogin", 0 };
	Display		*dpy;

	Debug ("greet %s\n", d->name);
	argc = 1;
	XtToolkitInitialize ();
	Debug ("name: %s\n", d->name);
	context = XtCreateApplicationContext();
	Debug ("name: %s\n", d->name);
	dpy = XtOpenDisplay (context, d->name, "xlogin", "Xlogin", 0,0,
				&argc, argv);

	Debug ("name: %s\n", d->name);
	SecureDisplay (d, dpy);

	i = 0;
	scrn = DefaultScreenOfDisplay(dpy);
        XtSetArg(arglist[i], XtNscreen, scrn);	i++;
	XtSetArg(arglist[i], XtNargc, argc);	i++;
	XtSetArg(arglist[i], XtNargv, argv);	i++;

	toplevel = XtAppCreateShell ((String) NULL, "Xlogin",
			applicationShellWidgetClass, dpy, arglist, i);

	i = 0;
	XtSetArg (arglist[i], XtNnotifyDone, GreetDone); i++;
	if (!d->authorize || d->authorization)
		XtSetArg (arglist[i], XtNsecureSession, True); i++;
	login = XtCreateManagedWidget ("login", loginWidgetClass, toplevel,
					arglist, i);
	XtRealizeWidget (toplevel);

	XWarpPointer(dpy, None, RootWindowOfScreen (scrn),
			0, 0, 0, 0,
			WidthOfScreen(scrn) / 2,
 			HeightOfScreen(scrn) / 2);

#ifdef DRAWLOGO
	i = 0;
	XtSetArg (arglist[i], XtNgeometry, "100x100-0-0"); i++;
	logoToplevel = XtCreateApplicationShell (0, topLevelShellWidgetClass,
						arglist, i);
	i = 0;
	logo = XtCreateManagedWidget ("logo", logoWidgetClass, logoToplevel,
					arglist, i);
	XtRealizeWidget (logoToplevel);
#endif
	return dpy;
}

CloseGreet (d)
struct display	*d;
{
	Boolean	    allow;
	Arg	    arglist[1];

	UnsecureDisplay (d, XtDisplay (toplevel));
	XtSetArg (arglist[0], XtNallowAccess, (char *) &allow);
	XtGetValues (login, arglist, 1);
	if (allow) {
		Debug ("Disabling access control\n");
		XSetAccessControl (XtDisplay (toplevel), DisableAccess);
	}
	XCloseDisplay (XtDisplay (toplevel));
}

Greet (d, greet)
struct display		*d;
struct greet_info	*greet;
{
	XEvent		event;
	Arg		arglist[1];

	XtSetArg (arglist[0], XtNallowAccess, False);
	XtSetValues (login, arglist, 1);

	Debug ("dispatching %s\n", d->name);
	done = 0;
	while (!done) {
		XtAppNextEvent (context, &event);
		XtDispatchEvent (&event);
	}
	XFlush (XtDisplay (toplevel));
	Debug ("Done dispatch %s\n", d->name);
	greet->name = name;
	greet->password = password;
	XtSetArg (arglist[0], XtNsessionArgument, (char *) &(greet->string));
	XtGetValues (login, arglist, 1);
	Debug ("sessionArgument: %s\n", greet->string ? greet->string : "<NULL>");
}


/*ARGSUSED*/
FailedLogin (d, greet)
struct display	*d;
struct greet_info	*greet;
{
	DrawFail (login);
}
