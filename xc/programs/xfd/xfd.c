/*
 * $XConsortium: xfd.c,v 1.5 89/06/05 16:31:45 jim Exp $
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
{"-fn",		"*grid.font",	XrmoptionSepArg,	(caddr_t) NULL },
{"-start",	"*startChar",	XrmoptionSepArg, 	(caddr_t) NULL },
{"-box",	"*grid.boxChars", XrmoptionNoArg,	(caddr_t) "on" },
{"-bc",		"*grid.boxColor", XrmoptionSepArg, 	(caddr_t) NULL },
{"-center",	"*grid.centerChars", XrmoptionNoArg,	(caddr_t) "on" },
};

static void do_quit(), do_next(), do_prev();
static void initialize_description_labels ();
static char *get_font_name();

static XtActionsRec xfd_actions[] = {
  { "Quit", do_quit },
  { "Prev", do_prev },
  { "Next", do_next },
};

static char *button_list[] = { "quit", "prev", "next", NULL };


usage()
{
    fprintf (stderr,
	     "usage:  %s [-start num] [-box] [-center] -fn font\n",
	     ProgramName);
    exit (1);
}


static Widget chardesc1Label, charlabel2Label, fontGrid;

main (argc, argv) 
    int argc;
    char **argv;
{
    Widget toplevel, pane, toplabel, box, form;
    Arg av[10];
    Cardinal i;
    char **cpp;
    static void SelectChar();
    static XtCallbackRec cb[2] = { { SelectChar, NULL }, { NULL, NULL } };
    XFontStruct *fs;
    char *fontname;

    ProgramName = argv[0];

    toplevel = XtInitialize (NULL, "Xfd", xfd_options, XtNumber(xfd_options),
			     &argc, argv);
    if (argc != 1) usage ();
    XtAppAddActions (XtWidgetToApplicationContext (toplevel),
                     xfd_actions, XtNumber (xfd_actions));


    /* pane wrapping everything */
    pane = XtCreateManagedWidget ("pane", panedWidgetClass, toplevel,
				  NULL, ZERO);

    /* font name */
    toplabel = XtCreateManagedWidget ("fontname", labelWidgetClass, pane, 
				      NULL, ZERO);

    /* button box */
    box = XtCreateManagedWidget ("box", boxWidgetClass, pane, NULL, ZERO);
    for (cpp = button_list; *cpp; cpp++) {
        (void) XtCreateManagedWidget (*cpp, commandWidgetClass, box,
                                      NULL, ZERO);
    }

    /* and labels in which to put information */
    chardesc1Label = XtCreateManagedWidget ("chardesc1", labelWidgetClass,
					    pane, NULL, ZERO);
    charlabel2Label = XtCreateManagedWidget ("charlabel2", labelWidgetClass,
					     pane, NULL, ZERO);

    /* form in which to draw */
    form = XtCreateManagedWidget ("form", formWidgetClass, pane, NULL, ZERO);
    
    i = 0;
    XtSetArg (av[i], XtNcallback, cb); i++;
    fontGrid = XtCreateManagedWidget ("grid", fontgridWidgetClass, form,
				      av, i);

    /* set the label at the top to tell us which font this is */
    i = 0;
    XtSetArg (av[i], XtNfont, &fs); i++;
    XtGetValues (fontGrid, av, i);
    fontname = get_font_name (XtDisplay(toplevel), fs);
    if (!fontname) fontname = "unknown font!";
    i = 0;
    XtSetArg (av[i], XtNlabel, fontname); i++;
    XtSetValues (toplabel, av, i);
    initialize_description_labels (fs, 0, False);

    XtRealizeWidget (toplevel);
    XtMainLoop ();
}


static void SelectChar (w, closure, data)
    Widget w;
    caddr_t closure, data;
{
    FontGridCharRec *p = (FontGridCharRec *) data;
    XFontStruct *fs = p->thefont;
    unsigned n = ((((unsigned) p->thechar.byte1) << 8) |
		  ((unsigned) p->thechar.byte2));
    unsigned minn = ((((unsigned) fs->min_byte1) << 8) |
		     ((unsigned) fs->min_char_or_byte2));
    unsigned maxn = ((((unsigned) fs->max_byte1) << 8) |
		     ((unsigned) fs->max_char_or_byte2));
    int direction, fontascent, fontdescent;
    XCharStruct metrics;
    char buf[256];
    Arg arg;

    if (n < minn || n > maxn) {
	initialize_description_labels (fs, n, True);
	return;
    }

    XTextExtents16 (fs, &p->thechar, 1, &direction, &fontascent, &fontdescent,
		    &metrics);

    XtSetArg (arg, XtNlabel, buf);
    sprintf (buf, "character %u, 0x%02x%02x (%d, %d)",
	     n, (unsigned) p->thechar.byte1, (unsigned) p->thechar.byte2,
	     (unsigned) p->thechar.byte1, (unsigned) p->thechar.byte2);
    XtSetValues (chardesc1Label, &arg, ONE);

    sprintf (buf, "width %d, left %d, right %d, ascent %d, descent %d",
	     metrics.width, metrics.lbearing, metrics.rbearing,
	     metrics.ascent, metrics.descent);
    XtSetValues (charlabel2Label, &arg, ONE);

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

change_page (page)
    int page;
{
    unsigned int start, ncols, nrows;
    unsigned delta;
    Arg arg;

    GetFontGridCellDimensions (fontGrid, &start, &ncols, &nrows);

    if (page < 0) {
	delta = ncols * nrows * ((unsigned) -page);

	if (start <= delta) 
	  start = 0;
	else
	  start -= delta;
    } else {
	delta = ncols * nrows * (unsigned) page;
	start += delta;
    }
    XtSetArg (arg, XtNstartChar, start);
    XtSetValues (fontGrid, &arg, ONE);
}


/* ARGSUSED */
static void do_prev (w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    change_page (-1);
}


/* ARGSUSED */
static void do_next (w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    change_page (1);
}


static char *get_font_name (dpy, fs)
    Display *dpy;
    XFontStruct *fs;
{
    register XFontProp *fp;
    register int i;
    Atom fontatom = XInternAtom (dpy, "FONT", False);

    for (i = 0, fp = fs->properties; i < fs->n_properties; i++, fp++) {
	if (fp->name == fontatom) {
	    return (XGetAtomName (dpy, fp->card32));
	}
    }
    return NULL;
}


static void initialize_description_labels (fs, charnum, valid)
    XFontStruct *fs;
    unsigned int charnum;
    Bool valid;
{
    char buf[256];
    Arg arg;

    XtSetArg (arg, XtNlabel, buf);

    if (valid) {
	unsigned low = (charnum & 0xff), hi = ((charnum >> 8) & 0xff);

	sprintf (buf, "Character 0x%02x%02x (%u,%u) is out of the range",
		 hi, low, hi, low);
    } else {
    	sprintf (buf, "Select a character in the range");
    }
    XtSetValues (chardesc1Label, &arg, ONE);

    sprintf (buf, "0x%02x%02x (%u,%u) through 0x%02x%02x (%u,%u).",
	     fs->min_byte1, fs->min_char_or_byte2,
	     fs->min_byte1, fs->min_char_or_byte2,
	     fs->max_byte1, fs->max_char_or_byte2,
	     fs->max_byte1, fs->max_char_or_byte2);
    XtSetValues (charlabel2Label, &arg, ONE);
}
