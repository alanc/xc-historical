/*
 * $XConsortium: editres.c,v 1.1 90/02/15 10:53:00 kit Exp $
 *
 * Copyright 1989 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>
#include <X11/Intrinsic.h>

#include <X11/Xaw/Cardinals.h>	

#include "editresP.h"

/*
 * Global variables. 
 */

TreeInfo *global_tree_info = NULL;
CurrentClient global_client;
ScreenData global_screen_data;
Widget global_tree_parent;

/*
 * external function definitions.
 */

extern void InternAtoms(), SetMessage(), BuildWidgetTree();
extern void SetApplicationActions();

String fallback_resources[] = { 
    NULL,
};

void
main(argc, argv)
int argc;
char **argv;
{
    static void Syntax();
    XtAppContext app_con;
    Widget toplevel;

    toplevel = XtAppInitialize(&app_con, "Editres", NULL, ZERO,
			       &argc, argv, fallback_resources, NULL, ZERO);

    if (argc != 1)		
	Syntax(app_con, argv[0]);

    SetApplicationActions(app_con);

    BuildWidgetTree(toplevel);

    SetMessage(global_screen_data.info_label, 
	       "Welcome to the X Resource Editor version 0.1");

    global_screen_data.set_values_popup = NULL;

    InternAtoms(XtDisplay(toplevel));

    XtRealizeWidget(toplevel);
    XtAppMainLoop(app_con);
}

/*	Function Name: Syntax
 *	Description: Prints a the calling syntax for this function to stdout.
 *	Arguments: app_con - the application context.
 *                 call - the name of the application.
 *	Returns: none - exits tho.
 */

static void 
Syntax(app_con, call)
XtAppContext app_con;
char *call;
{
    XtDestroyApplicationContext(app_con);
    fprintf( stderr, "Usage: %s [ -vspace <value> ] [ -hspace <value> ]\n",
	    call);
    exit(1);
}
