#if !defined(lint) && !defined(SABER)
static char rcs_id[] = 
    "$XConsortium: bbox.c,v 2.26 89/07/09 16:22:01 converse Exp $";
#endif
/*
 *
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
 * to create new buttonboxes and manage their contents.  It is a layer hiding
 * the toolkit interfaces. 
 *
 * Buttonboxes contain buttons which may be one of three kinds: command,
 * toggle, or menuButton.
 */

#include <X11/Xaw/Cardinals.h>
#include "xmh.h"
#include "bboxint.h"

/* ------------------------  Creation ----------------------------------- */

/*
 * Create a new button box.  The widget for it will be a child of the given
 * scrn's widget, and it will be added to the scrn's pane. 
 */

static ButtonBox buttonboxCreate(scrn, name, button_type)
  Scrn scrn;
  char *name;			/* Name of the buttonbox widget. */
  WidgetClass button_type;	/* command, toggle (radio), or menu buttons */
{
    static Arg arglist[] = {
	{XtNallowVert,	True},
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
    buttonbox->button_type = button_type;
    return buttonbox;
}


/* Create a new buttonbox which manages command buttons */

ButtonBox BBoxCreate(scrn, name)
  Scrn scrn;
  char *name;
{
    return buttonboxCreate(scrn, name, commandWidgetClass);
}


/* Create a new buttonbox which manages toggle buttons */

ButtonBox BBoxRadioCreate(scrn, name)
  Scrn scrn;
  char *name;
{
    return buttonboxCreate(scrn, name, toggleWidgetClass);
}


/* Create a new buttonbox which manages menu buttons */

ButtonBox BBoxMenuCreate(scrn, name)
  Scrn scrn;
  char *name;
{
    return buttonboxCreate(scrn, name, menuButtonWidgetClass);
}


/* Create a new button, and add it to a buttonbox. */

void BBoxAddButton(buttonbox, name, position, enabled)
ButtonBox buttonbox;
char *name;			/* Name of button. */
int position;			/* Position to put button in box. */
int enabled;			/* Whether button is initially enabled. */
{
    Button		button;
    register int	i;
    Arg			 arglist[5];
    char		menuName[300];

    /* Add menu entries, not new buttons, for new subfolders */

    if (buttonbox->button_type == menuButtonWidgetClass) {
	extern void MenuAddEntry();
	char	*c;
	if (c = index(name, '/')) {	
	    c[0] = '\0';
	    button = BBoxFindButtonNamed(buttonbox, name);
	    c[0] = '/';
	    MenuAddEntry(button, name);
	    return;
	}
    }

    if (position > buttonbox->numbuttons) position = buttonbox->numbuttons;
    buttonbox->numbuttons++;
    buttonbox->button = (Button *)
	XtRealloc((char *) buttonbox->button,
		  (unsigned) buttonbox->numbuttons * sizeof(Button));
    for (i=buttonbox->numbuttons-1 ; i>position ; i--)
	buttonbox->button[i] = buttonbox->button[i-1];
    button = buttonbox->button[position] = XtNew(ButtonRec);
    bzero((char *) button, sizeof(ButtonRec));
    button->buttonbox = buttonbox;
    button->name = XtNewString(name);

    i = 0;
    if (!enabled) {
	XtSetArg(arglist[i], XtNsensitive, False);		i++;
    }

    if (buttonbox->button_type == toggleWidgetClass) {
	Widget	radio_group = NULL;
	if (buttonbox->numbuttons > 1)
	    radio_group = (button == buttonbox->button[0]) 
		? (buttonbox->button[1]->widget)
	        : (buttonbox->button[0]->widget);
	XtSetArg(arglist[i], XtNradioGroup, radio_group);	i++;
	XtSetArg(arglist[i], XtNradioData, button->name);	i++;
    }
    else if (buttonbox->button_type == menuButtonWidgetClass) {
	button->menu = NULL;
	strcpy(menuName, name);
	strcpy(menuName + strlen(name), "Menu");
	button->menu_name = XtNewString(menuName);
	/* %%% MenuButton requires malloc of the menuName */
	XtSetArg(arglist[i], XtNmenuName, button->menu_name);	i++;   
    }

    button->widget = XtCreateManagedWidget(name, buttonbox->button_type,
					   buttonbox->inner, arglist,
					   (Cardinal) i);

    if (buttonbox->button_type == toggleWidgetClass)
	XtOverrideTranslations(button->widget, XtParseTranslationTable
			       ("<Btn1Down>,<Btn1Up>:set()\n"));
    else if (buttonbox->button_type == menuButtonWidgetClass)
	XtOverrideTranslations(button->widget, XtParseTranslationTable
                               ("<LeaveWindow>:reset()leave()\n\
			         <BtnDown>:set()folder-menu()\n\
                                 <BtnUp>:folder-button()reset()\n"));
}


/*----------------------- Get & Set ----------------------------------------*/

/* Set the current button in a radio buttonbox. */

void BBoxSetRadio(button)
Button button;
{
    XawToggleSetCurrent(button->widget, button->name);
}


/* Get the name of the current button in a radio buttonbox. */

char *BBoxGetRadioName(buttonbox)
ButtonBox buttonbox;
{
    return ((char *) XawToggleGetCurrent(buttonbox->button[0]->widget));
}

/*----------------------- Enable & Disable ------------------------*/

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

/*---------------------------- Miscellaneous Ops ----------------------*/

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
    return NULL;
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


/* Change the borderwidth of the given button. */

void BBoxChangeBorderWidth(button, borderWidth)
Button button;
unsigned int borderWidth;
{
    static Arg arglist[] = {XtNborderWidth, NULL};
    arglist[0].value = (XtArgVal) borderWidth;
    XtSetValues(button->widget, arglist, XtNumber(arglist));
}


/*---------------------- Deletion Routines --------------------------------*/

 
/* Free the storage for the given button. */
static void FreeButton(button)
Button button;
{
    XtFree(button->name);
    XtFree((char *) button);
}


/* Remove the given button from its buttonbox.  The button widget is
   destroyed.  If it was the current button in a radio buttonbox, then the
   current button becomes the first button in the box. */

void BBoxDeleteButton(button)
Button button;
{
    ButtonBox buttonbox;
    int i, found, reradio;
    
    if (button == NULL) return;
    buttonbox = button->buttonbox;
    found = reradio = FALSE;

    for (i=0 ; i<buttonbox->numbuttons; i++) {
	if (found)
	    buttonbox->button[i-1] = buttonbox->button[i];
	else if (buttonbox->button[i] == button) {
	    found = TRUE;
	    if (buttonbox->button_type == toggleWidgetClass) {
		if (strcmp((char *) XawToggleGetCurrent(button->widget),
			   button->name) == 0)
		    reradio = TRUE;
	    }
	    else if (buttonbox->button_type == menuButtonWidgetClass) {
		if (strcmp(buttonbox->scrn->curfolder, button->name) == 0)
		    reradio = TRUE;
		if (button->menu != NULL &&
		    button->menu != NoMenuForButton)
		    XtDestroyWidget(button->menu);
	    }

	    XtDestroyWidget(button->widget);
	    FreeButton(button);
	}
    }

    if (found) {
	buttonbox->numbuttons--;
	if (reradio && buttonbox->numbuttons) {
	    if (buttonbox->button_type == toggleWidgetClass)
		BBoxSetRadio(buttonbox->button[0]);
	    else if (buttonbox->button_type == menuButtonWidgetClass)
		SetCurrentFolderName(buttonbox->scrn, 
				     buttonbox->button[0]->name);
	}
    }
}


/* Function:	BBoxDeleteMenuEntry
 * Description:	Remove a subfolder from a menu.
 */
void BBoxDeleteMenuEntry(button, foldername)
    Button	button;
    char	*foldername;	/* guaranteed to be a subfolder */
{

    if (XawSimpleMenuEntryCount(button->menu, XawMenuTextMask) <= 2) {
	XtDestroyWidget(button->menu);	/* %%% leaving behind some memory
					 * in the callback closure struct
					 */
	button->menu = NoMenuForButton;
    }
    else {
	char *subfolder = MakeSubFolderName(foldername);
	if (strcmp(button->name, subfolder) == 0) {
	    char name[200];
	    name[0] = '_';
	    strcpy(name + 1, subfolder);
	    XawSimpleMenuRemoveEntry(button->menu, name);
	}
	else
	    XawSimpleMenuRemoveEntry(button->menu, subfolder);
	XtFree(subfolder);
    }
    SetCurrentFolderName(button->buttonbox->scrn, button->name);
}


/* Destroy the given buttonbox. */

void BBoxDeleteBox(buttonbox)
ButtonBox buttonbox;
{
    XtDestroyWidget(buttonbox->outer);
}
