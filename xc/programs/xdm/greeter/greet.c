/*
 * xdm - display manager daemon
 *
 * $XConsortium: $
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

# include <X11/Xlib.h>
# include <X11/Intrinsic.h>
# include <X11/StringDefs.h>
# include <X11/Xmu.h>
# include "Login.h"
# include <X11/Shell.h>
# include <X11/Command.h>
# include "dm.h"

extern Display	*dpy;

static int	done;
static char	name[128], password[128];
static Widget		toplevel;
static Widget		login;
static Widget		loginFailedButton;

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

InitGreet (d)
struct display	*d;
{
	Arg		arglist[10];
	int		i;
	int		argc;
	static char	*argv[] = { "xlogin", "-display", 0, 0 };

	Debug ("greet %s\n", d->name);
	argv[2] = d->name;
	argc = 3;
	toplevel = XtInitialize ("main", "Xlogin", 0, 0, &argc, argv);
	Debug ("top level shell created\n");

	i = 0;
	XtSetArg (arglist[i], XtNnotifyDone, GreetDone); i++;

	login = XtCreateManagedWidget ("login", loginWidgetClass,
					toplevel, arglist, i);
	XtRealizeWidget (toplevel);
}

CloseGreet (d)
struct display	*d;
{
	XCloseDisplay (XtDisplay (toplevel));
}

Greet (d, greet)
struct display		*d;
struct greet_info	*greet;
{
	XEvent		event;

	Debug ("dispatching\n");
	done = 0;
	while (!done) {
		XtAppNextEvent (_XtDefaultAppContext(), &event);
		XtDispatchEvent (&event);
	}
	XFlush (XtDisplay (toplevel));
	greet->name = name;
	greet->password = password;
	greet->string = 0;
}


FailedLogin (d, greet)
struct display	*d;
struct greet_info	*greet;
{
	DrawFail (login);
}
