#ifndef lint
static char rcs_id[] = "$XConsortium: bbox.c,v 2.20 89/04/10 11:51:00 converse Exp $";
#endif lint
/*
 *			  COPYRIGHT 1987
 *		   DIGITAL EQUIPMENT CORPORATION
 *		       MAYNARD, MASSACHUSETTS
 *			ALL RIGHTS RESERVED.
 *
 * THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT NOTICE AND
 * SHOULD NOT BE CONSTRUED AS A COMMITMENT BY DIGITAL EQUIPMENT CORPORATION.
 * DIGITAL MAKES NO REPRESENTATIONS ABOUT THE SUITABILITY OF THIS SOFTWARE FOR
 * ANY PURPOSE.  IT IS SUPPLIED "AS IS" WITHOUT EXPRESS OR IMPLIED WARRANTY.
 *
 * IF THE SOFTWARE IS MODIFIED IN A MANNER CREATING DERIVATIVE COPYRIGHT RIGHTS,
 * APPROPRIATE LEGENDS MAY BE PLACED ON THE DERIVATIVE WORK IN ADDITION TO THAT
 * SET FORTH ABOVE.
 *
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting documentation,
 * and that the name of Digital Equipment Corporation not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission.
 */

/* bbox.c -- management of buttons and buttonbox's. */

/* This module implements a simple interface to buttonboxes, allowing a client
   to create new buttonboxes and manage their contents.  It is a layer hiding
   the toolkit interfaces. */

#include <X11/Cardinals.h>
#include "xmh.h"
#include "bboxint.h"


/* Free the storage for the given button. */

FreeButton(button)
Button button;
{
    XtFree(button->name);
    XtFree((char *) button);
}


/* Create a new button box.  The widget for it will be a child of the given
   scrn's widget, and it will be added to the scrn's pane. */

ButtonBox BBoxRadioCreate(scrn, name, radio)
  Scrn scrn;
  char *name;			/* Name of the buttonbox widget. */
  Bool radio;			/* True if this is a radio buttonbox */
{
    static Arg arglist[] = {
	{XtNallowVert, True},
	{XtNskipAdjust, True},
    };
    ButtonBox buttonbox;

    buttonbox = XtNew(ButtonBoxRec);
    bzero((char *)buttonbox, sizeof(ButtonBoxRec));
    buttonbox->scrn = scrn;
    buttonbox->outer =
	XtCreateManagedWidget(name, viewportWidgetClass, scrn->widget,
			      arglist, XtNumber(arglist));
    buttonbox->inner =
	XtCreateManagedWidget(name, boxWidgetClass, buttonbox->outer,
			      (ArgList)NULL, (Cardinal)0);
    buttonbox->numbuttons = 0;
    buttonbox->button = (Button *) XtMalloc((unsigned) 1);
    buttonbox->maxheight = 5;
    buttonbox->radio = radio;
    return buttonbox;
}



/* Create a new buttonbox which does not manage radio buttons. */

ButtonBox BBoxCreate(scrn, name)
  Scrn scrn;
  char *name;
{
    return BBoxRadioCreate(scrn, name, False);
}



/* Set the current button in a radio buttonbox. */

void BBoxSetRadio(button)
Button button;
{
    XtToggleSetCurrent(button->widget, button->name);
}



/* Get the name of the current button in a radio buttonbox. */

char *BBoxGetRadioName(buttonbox)
ButtonBox buttonbox;
{
    return ((char *) XtToggleGetCurrent(buttonbox->button[0]->widget));
}


/* Create a new button, and add it to a buttonbox. */

void BBoxAddButton(buttonbox, name, func, position, enabled)
ButtonBox buttonbox;
char *name;			/* Name of button. */
void (*func)();			/* Func to call when button pressed. */
int position;			/* Position to put button in box. */
int enabled;			/* Whether button is initially enabled. */
{
    extern void DoButtonPress();
    Button button;
    int i;
    static XtCallbackRec callback[] = { {DoButtonPress, NULL}, {NULL, NULL} };
    Arg arglist[3];

    if (position > buttonbox->numbuttons) position = buttonbox->numbuttons;
    buttonbox->numbuttons++;
    buttonbox->button = (Button *)
	XtRealloc((char *) buttonbox->button,
		  (unsigned) buttonbox->numbuttons * sizeof(Button));
    for (i=buttonbox->numbuttons-1 ; i>position ; i--)
	buttonbox->button[i] = buttonbox->button[i-1];
    button = buttonbox->button[position] = XtNew(ButtonRec);
    bzero((char *) button, sizeof(ButtonRec));
    callback[0].closure = (caddr_t)button;
    button->buttonbox = buttonbox;
    button->name = MallocACopy(name);

    i = 0;
    if (!enabled)
    {
	XtSetArg(arglist[i], XtNsensitive, False);		i++;
    }
    if (buttonbox->radio)
    {
	Widget	radio_group = NULL;
	if (buttonbox->numbuttons > 1)
	    radio_group = (button == buttonbox->button[0]) 
		? (buttonbox->button[1]->widget)
	        : (buttonbox->button[0]->widget);
	XtSetArg(arglist[i], XtNradioGroup, radio_group);	i++;
	XtSetArg(arglist[i], XtNradioData, button->name);	i++;
    }
    else
    {
	XtSetArg(arglist[i], XtNcallback, callback);		i++;
    }
    button->widget = XtCreateManagedWidget(name, (buttonbox->radio) 
				    ? toggleWidgetClass
				    : commandWidgetClass,
				    buttonbox->inner, arglist, (Cardinal)i);

    if (buttonbox->radio)
	XtOverrideTranslations(button->widget,
			       XtParseTranslationTable
			       ("<Btn1Down>,<Btn1Up>:set()\n"));

    button->func = func;
}



/* Remove the given button from its buttonbox.  The button widget is
   destroyed.  If it was the current button in a radio buttonbox, then the
   current button becomes the first button in the box. */

void BBoxDeleteButton(button)
Button button;
{
    ButtonBox buttonbox = button->buttonbox;
    int i, found, reradio;
    found = reradio = FALSE;
    for (i=0 ; i<buttonbox->numbuttons; i++) {
	if (found) buttonbox->button[i-1] = buttonbox->button[i];
	else if (buttonbox->button[i] == button) {
	    found = TRUE;
	    if (buttonbox->radio)
	    {
		if (strcmp((char *) XtToggleGetCurrent(button->widget),
			   button->name) == 0)
		    reradio = TRUE;
		XtDestroyWidget(button->widget);
		FreeButton(button);
	    }
	    
	}
    }
    if (found) {
	buttonbox->numbuttons--;
	if (reradio && buttonbox->numbuttons)
		BBoxSetRadio(buttonbox->button[0]);
    }
}
	    


/* Enable or disable the given button widget. */

static void SendEnableMsg(widget, value)
Widget widget;
int value;			/* TRUE for enable, FALSE for disable. */
{
    static Arg arglist[] = {XtNsensitive, NULL};
    arglist[0].value = (XtArgVal) value;
    XtSetValues(widget, arglist, XtNumber(arglist));
}



/* Enable the given button (if it's not already). */

void BBoxEnable(button)
Button button;
{
    SendEnableMsg(button->widget, TRUE);
}



/* Disable the given button (if it's not already). */

void BBoxDisable(button)
Button button;
{
    SendEnableMsg(button->widget, FALSE);
}


/* Given a buttonbox and a button name, find the button in the box with that
   name. */

Button BBoxFindButtonNamed(buttonbox, name)
ButtonBox buttonbox;
char *name;
{
    int i;
    for (i=0 ; i<buttonbox->numbuttons; i++)
	if (strcmp(name, buttonbox->button[i]->name) == 0)
	    return buttonbox->button[i];
    return NULL;
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


/* Set the minimum and maximum size for a bbox so that it cannot be resized
   any bigger than its total height. */

void BBoxLockSize(buttonbox)
ButtonBox buttonbox;
{
#ifdef notyet
    static Arg args[] = {
	{XtNmax, NULL},		/* first is for VPaned */
/*	{XtNmin, 5}, */		/* let user select this */
    };

    buttonbox->maxheight = GetHeight(buttonbox->inner);
    args[0].value = (XtArgVal)buttonbox->maxheight;
    XtSetValues(buttonbox->outer, args, XtNumber(args));
#endif
}





/* Destroy the given buttonbox. */

void BBoxDeleteBox(buttonbox)
ButtonBox buttonbox;
{
    XtDestroyWidget(buttonbox->outer);
}



/* Change the borderwidth of the given button. */

void BBoxChangeBorderWidth(button, borderWidth)
Button button;
unsigned int borderWidth;
{
    static Arg arglist[] = {XtNborderWidth, NULL};
    arglist[0].value = (XtArgVal) borderWidth;
    XtSetValues(button->widget, arglist, XtNumber(arglist));
}
