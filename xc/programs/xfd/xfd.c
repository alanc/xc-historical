/*
 * $XConsortium$
 *
 * Copyright 1989 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
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
 *
 * Author:  Jim Fulton, MIT X Consortium
 *          rewritten from an Xlib version by Mark Lillibridge
 */

#include <stdio.h>
#include <X11/Xos.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xatom.h>
#include <X11/Shell.h>
#include <X11/Xaw/Cardinals.h>
#include <X11/Xaw/Paned.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Command.h>
#include "fontgrid.h"

char *ProgramName;

static XrmOptionDescRec xfd_options[] = {
{"-bf",		"bottomFont",	XrmoptionSepArg,	(caddr_t) NULL },
{"-start",	"*startChar",	XrmoptionSepArg, 	(caddr_t) NULL },
{"-verbose",	"verbose",	XrmoptionNoArg,		(caddr_t) "on" },
};

static struct resources {
  XFontStruct *bottom_font;
  Boolean verbose;
} xfd_resources;

static XtResource Resources[] = {
#define offset(field) XtOffset(struct resources *, field)
{ "bottomFont", "Font", XtRFontStruct, sizeof(XFontStruct *),
    offset(bottom_font), XtRString, (caddr_t) "XtDefaultFont" },
{ "verbose", "Verbose", XtRBoolean, sizeof(Boolean),
    offset(verbose), XtRString, (caddr_t) "FALSE" },
#undef offset
};

static void do_quit(), do_next(), do_prev();

static XtActionsRec xfd_actions[] = {
  { "Quit", do_quit },
  { "Next", do_next },
  { "Prev", do_prev },
};

static char *button_list[] = { "quit", "next", "prev", NULL };


usage()
{
    fprintf (stderr, "usage:  %s [-v] [-s num] -fn font\n",
	     ProgramName);
    exit (1);
}


main (argc, argv) 
    int argc;
    char **argv;
{
    Widget toplevel, pane, toplabel, box, form, grid, bottomlabel;
    Arg av[10];
    Cardinal i;
    char **cpp;
    static void GotCharacter();
    static XtCallbackRec cb[2] = { { GotCharacter, NULL }, { NULL, NULL } };

    ProgramName = argv[0];

    toplevel = XtInitialize (NULL, "Xfd", xfd_options, XtNumber(xfd_options),
			     &argc, argv);
    if (argc != 1) usage ();
    XtAppAddActions (XtWidgetToApplicationContext (toplevel),
                     xfd_actions, XtNumber (xfd_actions));

    XtGetApplicationResources (toplevel, (caddr_t)&xfd_resources, Resources,
			       XtNumber(Resources), NULL, 0);


    /*
     * In the application shell, create a pane that separates the various
     * parts.  Put buttons in a box on top, a core within a form in the middle,
     * and text widgets at the bottom.
     */

    /* pane wrapping everything */
    pane = XtCreateManagedWidget ("pane", panedWidgetClass, toplevel,
				  NULL, ZERO);

    /* font name */
    toplabel = XtCreateManagedWidget ("title", labelWidgetClass, pane, 
				      NULL, ZERO);

    /* button box */
    box = XtCreateManagedWidget ("box", boxWidgetClass, pane, NULL, ZERO);
    for (cpp = button_list; *cpp; cpp++) {
        (void) XtCreateManagedWidget (*cpp, commandWidgetClass, box,
                                      NULL, ZERO);
    }

    /* form in which to draw */
    form = XtCreateManagedWidget ("form", formWidgetClass, pane, NULL, ZERO);

    
    /*
     * set the sucker to expand; really ought to compute this according to 
     * the font being displayed...
     */

#define DEFAULT_DRAWING_WIDTH 200
#define DEFAULT_DRAWING_HEIGHT 200

    i = 0;
    XtSetArg (av[i], XtNtop, XtChainTop); i++;
    XtSetArg (av[i], XtNbottom, XtChainBottom); i++;
    XtSetArg (av[i], XtNleft, XtChainLeft); i++;
    XtSetArg (av[i], XtNright, XtChainRight); i++;
    XtSetArg (av[i], XtNwidth, DEFAULT_DRAWING_WIDTH); i++;
    XtSetArg (av[i], XtNheight, DEFAULT_DRAWING_HEIGHT); i++;
    XtSetArg (av[i], XtNcallback, cb); i++;
    grid = XtCreateManagedWidget ("grid", fontgridWidgetClass, form, av, i);

    /* and a label in which to put information */
    bottomlabel = XtCreateManagedWidget ("label", labelWidgetClass, pane,
					 NULL, ZERO);


    XtRealizeWidget (toplevel);
    XtMainLoop ();
}


static void GotCharacter (w, closure, data)
    Widget w;
    caddr_t closure, data;
{
    FontGridCharRec *p = (FontGridCharRec *) data;
    XFontStruct *fs = p->thefont;
    unsigned n = ((((unsigned) p->thechar.byte1) << 8) |
		  ((unsigned) p->thechar.byte2));
    XCharStruct *pc;

    /*
     * XXX - display in a text widget, along with perchar info
     */
    if (fs->per_char) {
	pc = fs->per_char + ((p->thechar.byte2 - fs->min_char_or_byte2) *
			     (p->thechar.byte1 - fs->min_byte1));
    } else {
	pc = &fs->max_bounds;
    }

    printf ("Got character %u, 0x%02x%02x (%d, %d)\n", n,
	    (unsigned) p->thechar.byte1, (unsigned) p->thechar.byte2,
	    (unsigned) p->thechar.byte1, (unsigned) p->thechar.byte2);
    printf ("width %d, left %d, right %d, ascent %d, descent %d\n",
	    pc->width, pc->lbearing, pc->rbearing, pc->ascent, pc->descent);

    return;
}



#ifdef comment
/*
 * Go_Back: Routine to page back a gridful of characters.
 */
Go_Back()
{
	/* If try and page back past first 0th character, beep */
	if (xfd_resources.start_char == 0) {
		Beep();
		return;
	}

	xfd_resources.start_char -= x_boxes*y_boxes;
	if (xfd_resources.start_char<0)
	  xfd_resources.start_char = 0;

	Display_Contents();
}

/*
 * Go_Forward: Routine to page forward a gridful of characters.
 */
Go_Forward()
{
	int delta = x_boxes*y_boxes;

	if (xfd_resources.start_char + delta > max_char) {
		Beep ();
		return;
	}

	xfd_resources.start_char += delta;

	Display_Contents();
}
#endif /* comment */


static void do_quit (w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    exit (0);
}

static void do_next (w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    /* goto next page */
}

static void do_prev (w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    /* goto prev page */
}
