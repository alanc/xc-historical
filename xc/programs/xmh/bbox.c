#if !defined(lint) && !defined(SABER)
static char rcs_id[] = 
    "$XConsortium: bbox.c,v 2.28 89/08/14 15:42:14 converse Exp $";
#endif
/*
 *
 *			  COPYRIGHT 1987, 1989
 *		   DIGITAL EQUIPMENT CORPORATION
 *		       MAYNARD, MASSACHUSETTS
 *			ALL RIGHTS RESERVED.
 *
 * THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT NOTICE AND
 * SHOULD NOT BE CONSTRUED AS A COMMITMENT BY DIGITAL EQUIPMENT CORPORATION.
 * DIGITAL MAKES NO REPRESENTATIONS ABOUT THE SUITABILITY OF THIS SOFTWARE FOR
 * ANY PURPOSE.  IT IS SUPPLIED "AS IS" WITHOUT EXPRESS OR IMPLIED WARRANTY.
 *
 * IF THE SOFTWARE IS MODIFIED IN A MANNER CREATING DERIVATIVE COPYRIGHT
 * RIGHTS, APPROPRIATE LEGENDS MAY BE PLACED ON THE DERIVATIVE WORK IN
 * ADDITION TO THAT SET FORTH ABOVE.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Digital Equipment Corporation not be
 * used in advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.
 */

/* bbox.c -- management of buttons and buttonboxes. 
 *
 * This module implements a simple interface to buttonboxes, allowing a client
 * to create new buttonboxes and manage their contents. 
 *
 */

#include <X11/Xaw/Cardinals.h>
#include "xmh.h"
#include "bboxint.h"

static XtTranslations	RadioButtonTranslations = NULL;


void BBoxInit()
{
    RadioButtonTranslations =
	XtParseTranslationTable("<Btn1Down>,<Btn1Up>:set()\n");
}


/*
 * Create a new button box.  The widget for it will be a child of the given
 * scrn's widget, and it will be added to the scrn's pane. 
 */

ButtonBox BBoxCreate(scrn, name)
    Scrn	scrn;
    char	*name;	/* name of the buttonbox widgets */
{
    Cardinal	n;
    ButtonBox	buttonbox = XtNew(ButtonBoxRec);
    Arg		args[5];

    n = 0;
    XtSetArg(args[n], XtNallowVert, True);	n++;
    XtSetArg(args[n], XtNskipAdjust, True);	n++;
    
    buttonbox->outer =
	XtCreateManagedWidget(name, viewportWidgetClass, scrn->widget,
			      args, n);
    buttonbox->inner =
	XtCreateManagedWidget(name, boxWidgetClass, buttonbox->outer,
			      args, (Cardinal) 0);
    buttonbox->numbuttons = 0;
    buttonbox->button = (Button *) XtMalloc((Cardinal) 1);
    buttonbox->scrn = scrn;
    return buttonbox;
}


ButtonBox RadioBBoxCreate(scrn, name)
    Scrn	scrn;
    char	*name;	/* name of the buttonbox widgets */
{
    return BBoxCreate(scrn, name);
}



/* Create a new button, and add it to a buttonbox. */

static void bboxAddButton(buttonbox, name, kind, position, enabled, radio)

    ButtonBox	buttonbox;
    char	*name;
    WidgetClass	kind;
    int		position;
    Boolean	enabled;
    Boolean	radio;
{
    Button	button;
    Cardinal	i;
    Widget	radio_group;
    Arg		args[5];

    if (position > buttonbox->numbuttons) position = buttonbox->numbuttons;
    buttonbox->numbuttons++;
    buttonbox->button = (Button *) 
	XtRealloc((char *) buttonbox->button,
		  (unsigned) buttonbox->numbuttons * sizeof(Button));
    for (i=buttonbox->numbuttons-1 ; i>position ; i--)
	buttonbox->button[i] = buttonbox->button[i-1];
    button = buttonbox->button[position] = XtNew(ButtonRec);
    button->buttonbox = buttonbox;
    button->name = XtNewString(name);
    button->menu = (Widget) NULL;

    i = 0;
    if (!enabled) {
	XtSetArg(args[i], XtNsensitive, False);		i++;
    }

    if (radio && kind == toggleWidgetClass) {
	if (buttonbox->numbuttons > 1)
	    radio_group = (button == buttonbox->button[0]) 
		? (buttonbox->button[1]->widget)
		: (buttonbox->button[0]->widget);
	else radio_group = NULL;
	XtSetArg(args[i], XtNradioGroup, radio_group);		i++;
	XtSetArg(args[i], XtNradioData, button->name);		i++;
    }

    button->widget =
	XtCreateManagedWidget(name, kind, buttonbox->inner, args, i);

    if (radio)
	XtOverrideTranslations(button->widget, RadioButtonTranslations);
}


void BBoxAddButton(buttonbox, name, kind, position, enabled)
    ButtonBox	buttonbox;
    char	*name;
    WidgetClass	kind;
    int		position;
    Boolean	enabled;
{
    bboxAddButton(buttonbox, name, kind, position, enabled, False);
}    


void RadioBBoxAddButton(buttonbox, name, position, enabled)
    ButtonBox	buttonbox;
    char	*name;
    int		position;
    Boolean	enabled;
{
    bboxAddButton(buttonbox, name, toggleWidgetClass, position, enabled, True);
}


/* Set the current button in a radio buttonbox. */

void RadioBBoxSet(button)
    Button button;
{
    XawToggleSetCurrent(button->widget, button->name);
}


/* Get the name of the current button in a radio buttonbox. */

char *RadioBBoxGetCurrent(buttonbox)
    ButtonBox buttonbox;
{
    return ((char *) XawToggleGetCurrent(buttonbox->button[0]->widget));
}


/* Remove the given button from its buttonbox, and free all resources
 * used in association with the button.  If the button was the current
 * button in a radio buttonbox, the current button becomes the first 
 * button in the box.
 */

void BBoxDeleteButton(button)
    Button	button;
{
    ButtonBox	buttonbox;
    int		i, found;
    
    if (button == NULL) return;
    buttonbox = button->buttonbox;
    found = False;

    for (i=0 ; i<buttonbox->numbuttons; i++) {
	if (found)
	    buttonbox->button[i-1] = buttonbox->button[i];
	else if (buttonbox->button[i] == button) {
	    found = True;
 
	    /* Free the resources used by the given button. */

	    if (button->menu != NULL && button->menu != NoMenuForButton)
		XtDestroyWidget(button->menu);
	    XtDestroyWidget(button->widget);
	    XtFree(button->name);
	    XtFree((char *) button);
	} 
    }
    if (found)
	buttonbox->numbuttons--;
}


void RadioBBoxDeleteButton(button)
    Button	button;
{
    ButtonBox	buttonbox;
    Boolean	reradio = False;
    char *	current;

    buttonbox = button->buttonbox;
    current = RadioBBoxGetCurrent(buttonbox);
    if (current) reradio = ! strcmp(current, button->name);
    BBoxDeleteButton(button);

    if (reradio && BBoxNumButtons(buttonbox))
	RadioBBoxSet(buttonbox->button[0]);
}


/* Enable or disable the given button widget. */

static void SendEnableMsg(widget, value)
    Widget	widget;
    int		value;	/* TRUE for enable, FALSE for disable. */
{
    static Arg arglist[] = {XtNsensitive, NULL};
    arglist[0].value = (XtArgVal) value;
    XtSetValues(widget, arglist, XtNumber(arglist));
}


/* Enable the given button (if it's not already). */

void BBoxEnable(button)
Button button;
{
    SendEnableMsg(button->widget, True);
}


/* Disable the given button (if it's not already). */

void BBoxDisable(button)
    Button button;
{
    SendEnableMsg(button->widget, False);
}


/* Given a buttonbox and a button name, find the button in the box with that
   name. */

Button BBoxFindButtonNamed(buttonbox, name)
    ButtonBox buttonbox;
    char *name;
{
    register int i;
    for (i=0 ; i<buttonbox->numbuttons; i++)
	if (strcmp(name, buttonbox->button[i]->name) == 0)
	    return buttonbox->button[i];
    return (Button) NULL;
}


/* Given a buttonbox and a widget, find the button which is that widget. */

Button BBoxFindButton(buttonbox, w)
    ButtonBox	buttonbox;
    Widget	w;
{
    register int i;
    for (i=0; i < buttonbox->numbuttons; i++)
	if (buttonbox->button[i]->widget == w)
	    return buttonbox->button[i];
    return (Button) NULL;
}


/* Return the nth button in the given buttonbox. */

Button BBoxButtonNumber(buttonbox, n)
    ButtonBox buttonbox;
    int n;
{
    return buttonbox->button[n];
}


/* Return how many buttons are in a buttonbox. */

int BBoxNumButtons(buttonbox)
    ButtonBox buttonbox;
{
    return buttonbox->numbuttons;
}


/* Given a button, return its name. */

char *BBoxNameOfButton(button)
    Button button;
{
    return button->name;
}


/* Set maximum size for a bbox so that it cannot be resized any bigger 
 * than its total height.  Allow the user to set the minimum size.
 * The problem, we think, is that the box computes it's height based on
 * each button being in a separate row; i.e. a column of buttons. 
 */

void BBoxLockSize(buttonbox)
    ButtonBox buttonbox;
{
    Dimension	maxheight;
    Arg		args[1];
    maxheight = (Dimension) GetHeight(buttonbox->inner);
    DEBUG1("maxheight is %d\n", maxheight);
    XtSetArg(args[0], XtNmax, maxheight);	/* for Paned widget */
    XtSetValues(buttonbox->outer, args, (Cardinal) 1);
}

