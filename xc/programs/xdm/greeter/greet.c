/*
 * widget to get username/password
 *
 */

# include <X11/Xlib.h>
# include <X11/Intrinsic.h>
# include <X11/StringDefs.h
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

failedcallback ()
{
	done = 1;
}

GreetDone (w, data)
    Widget	w;
    LoginData	*data;
{
	Debug ("GreetDone: %s, %s\n", data->name, data->passwd);
	strcpy (name, data->name);
	strcpy (password, data->passwd);
	done = 1;
}

InitGreet (d)
struct display	*d;
{
	Arg		arglist[10];
	int		i;
	int		argc;
	static char	*argv[] = { "xlogin", "-display", 0, 0 };
	static XtCallbackRec	callback[2];
	static XtCallbackList	callback_list;

	Debug ("greet %s\n", d->name);
	argv[2] = d->name;
	argc = 3;
	toplevel = XtInitialize ("main", "Xlogin", 0, 0, &argc, argv);
	Debug ("top level shell created\n");

	i = 0;
	XtSetArg (arglist[i], XtNnotifyDone, GreetDone); i++;

	login = XtCreateManagedWidget ("login", loginWidgetClass,
					toplevel, arglist, i);
	callback[0].callback = failedcallback;
	callback_list = callback;
	i = 0;
	XtSetArg (arglist[i], XtNx, 50); i++;
	XtSetArg (arglist[i], XtNy, 50); i++;
	XtSetArg (arglist[i], XtNlabel, "Login Failed"); i++;
	XtSetArg (arglist[i], XtNcallback, callback_list); i++;
	XtSetArg (arglist[i], XtNmappedWhenManaged, FALSE); i++;
	loginFailedButton = XtCreateManagedWidget ("login failed",
		commandWidgetClass, toplevel, arglist, i);
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
	greet->name = name;
	greet->password = password;
}


FailedLogin (d, greet)
struct display	*d;
struct greet_info	*greet;
{
	XEvent		event;

	Debug ("failed login\n");
	XtMapWidget (loginFailedButton);
	XRaiseWindow (XtDisplay (loginFailedButton), XtWindow (loginFailedButton));
/*	XtUnmapWidget (login); */
	XtAddGrab (loginFailedButton, TRUE, FALSE);
	Debug ("dispatching\n");
	done = 0;
	while (!done) {
		XtAppNextEvent (_XtDefaultAppContext (), &event);
		XtDispatchEvent (&event);
	}
	Debug ("LoginFailed pressed\n");
	XtRemoveGrab (loginFailedButton);
	XtUnmapWidget (loginFailedButton);
	ResetLogin (login);
/*	XtMapWidget (login); */
}
