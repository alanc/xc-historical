#ifndef lint
static char Xrcsid[] = "$XConsortium: Dialog.c,v 1.19 89/01/13 19:44:18 kit Exp $";
#endif lint


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

/* NOTE: THIS IS NOT A WIDGET!  Rather, this is an interface to a widget.
   It implements policy, and gives a (hopefully) easier-to-use interface
   than just directly making your own form. */


#include <X11/Xlib.h>
#include <X11/Xos.h>
#include <X11/IntrinsicP.h>
#include <X11/XawMisc.h>
#include <X11/StringDefs.h>
#include <X11/AsciiText.h>
#include <X11/Command.h>
#include <X11/Label.h>
#include <X11/DialogP.h>


static XtResource resources[] = {
  {XtNlabel, XtCLabel, XtRString, sizeof(String),
     XtOffset(DialogWidget, dialog.label), XtRString, NULL},
  {XtNvalue, XtCValue, XtRString, sizeof(String),
     XtOffset(DialogWidget, dialog.value), XtRString, NULL},
  {XtNmaximumLength, XtCMax, XtRInt, sizeof(int),
     XtOffset(DialogWidget, dialog.max_length), XtRString, "256"}
};

static void Initialize(), ConstraintInitialize(), CreateDialogValueWidget();
static Boolean SetValues();

DialogClassRec dialogClassRec = {
  { /* core_class fields */
    /* superclass         */    (WidgetClass) &formClassRec,
    /* class_name         */    "Dialog",
    /* widget_size        */    sizeof(DialogRec),
    /* class_initialize   */    NULL,
    /* class_part init    */    NULL,
    /* class_inited       */    FALSE,
    /* initialize         */    Initialize,
    /* initialize_hook    */    NULL,
    /* realize            */    XtInheritRealize,
    /* actions            */    NULL,
    /* num_actions        */    0,
    /* resources          */    resources,
    /* num_resources      */    XtNumber(resources),
    /* xrm_class          */    NULLQUARK,
    /* compress_motion    */    TRUE,
    /* compress_exposure  */    TRUE,
    /* compress_enterleave*/    TRUE,
    /* visible_interest   */    FALSE,
    /* destroy            */    NULL,
    /* resize             */    XtInheritResize,
    /* expose             */    XtInheritExpose,
    /* set_values         */    SetValues,
    /* set_values_hook    */    NULL,
    /* set_values_almost  */    XtInheritSetValuesAlmost,
    /* get_values_hook    */    NULL,
    /* accept_focus       */    NULL,
    /* version            */    XtVersion,
    /* callback_private   */    NULL,
    /* tm_table           */    NULL,
    /* query_geometry     */	XtInheritQueryGeometry,
    /* display_accelerator*/	XtInheritDisplayAccelerator,
    /* extension          */	NULL
  },
  { /* composite_class fields */
    /* geometry_manager   */   XtInheritGeometryManager,
    /* change_managed     */   XtInheritChangeManaged,
    /* insert_child       */   XtInheritInsertChild,
    /* delete_child       */   XtInheritDeleteChild,
    /* extension          */   NULL
  },
  { /* constraint_class fields */
    /* subresourses       */   NULL,
    /* subresource_count  */   0,
    /* constraint_size    */   sizeof(DialogConstraintsRec),
    /* initialize         */   ConstraintInitialize,
    /* destroy            */   NULL,
    /* set_values         */   NULL,
    /* extension          */   NULL
  },
  { /* form_class fields */
    /* empty              */   0
  },
  { /* dialog_class fields */
    /* empty              */   0
  }
};

WidgetClass dialogWidgetClass = (WidgetClass)&dialogClassRec;


/* ARGSUSED */
static void Initialize(request, new)
Widget request, new;
{
    DialogWidget dw = (DialogWidget)new;
    static Arg arglist[5];
    Cardinal num_args = 0;

    XtSetArg(arglist[num_args], XtNlabel, dw->dialog.label); num_args++;
    XtSetArg(arglist[num_args], XtNborderWidth, 0); num_args++;

    dw->dialog.labelW = XtCreateManagedWidget( "label", labelWidgetClass,
					      new, arglist, num_args);

    if (dw->dialog.value != NULL) 
        CreateDialogValueWidget( (Widget) dw);
    else
        dw->dialog.valueW = NULL;
}


/* ARGSUSED */
static void ConstraintInitialize(request, new)
Widget request, new;
{
    DialogWidget dw = (DialogWidget)new->core.parent;
    WidgetList children = dw->composite.children;
    DialogConstraints constraint = (DialogConstraints)new->core.constraints;
    Widget *childP;

    if (!XtIsSubclass(new, commandWidgetClass))	/* if not a button */
	return;					/* then just use defaults */

    constraint->form.left = constraint->form.right = XtChainLeft;
    if (dw->dialog.valueW == NULL) 
      constraint->form.vert_base = dw->dialog.labelW;
    else
      constraint->form.vert_base = dw->dialog.valueW;

    if (dw->composite.num_children > 1) {
        for (childP = children + dw->composite.num_children - 1;
	     childP >= children; childP-- ) {
	    if (*childP == dw->dialog.labelW || *childP == dw->dialog.valueW)
	        break;
	    if (XtIsManaged(*childP) &&
		XtIsSubclass(*childP, commandWidgetClass)) {
	        constraint->form.horiz_base = *childP;
		break;
	    }
	}
    }
}


/* ARGSUSED */
static Boolean SetValues(current, request, new)
Widget current, request, new;
{
    DialogWidget w = (DialogWidget)new;
    DialogWidget old = (DialogWidget)current;
    Arg args[5];
    Cardinal num_args;

    if (w->dialog.max_length != old->dialog.max_length) {
        XtWarning(
		"Dialog Widget does not support changes in XtNmaximumLength.");
        w->dialog.max_length = old->dialog.max_length;
    }

    if ( (w->dialog.label != old->dialog.label) ||
	 (w->dialog.label != NULL && old->dialog.label != NULL &&
	  strcmp(w->dialog.label, old->dialog.label)) ) {
        num_args = 0;
        XtSetArg( args[num_args], XtNlabel, w->dialog.label ); num_args++;
	XtSetValues( w->dialog.labelW, args, num_args );
    }

    if ( (w->dialog.value != old->dialog.value) ) {
        if (w->dialog.value == NULL) { /* only get here if it
					  wasn't NULL before. */
	    XtDestroyWidget(old->dialog.valueW);
	    XtFree(old->dialog.value);
	}
	else if (old->dialog.value == NULL) { /* create a new value widget. */
	    CreateDialogValueWidget( (Widget) w);
	}
	else {			/* Widget ok, just change string. */
	    XtTextBlock t_block;

	    t_block.firstPos = 0;
	    t_block.length = strlen(w->dialog.value);
	    t_block.ptr = w->dialog.value;
	    t_block.format = FMT8BIT;

	    if (XtTextReplace(w->dialog.valueW, 
			      0, strlen(old->dialog.value), &t_block) !=
		XawEditDone) 
	        XtWarning("Error while changing value in Dialog Widget.");

	    w->dialog.value = old->dialog.value;
	}
    }
    return False;
}

/*	Function Name: CreateDialogValueWidget
 *	Description: Creates the dialog widgets value widget.
 *	Arguments: w - the dialog widget.
 *	Returns: none.
 */

static void
CreateDialogValueWidget(w)
Widget w;
{
    DialogWidget dw = (DialogWidget) w;    
    String initial_value = dw->dialog.value;
    Cardinal length = Max( dw->dialog.max_length, strlen(initial_value) );
    static Arg arglist[10];
    Cardinal num_args = 0;

    dw->dialog.value = XtMalloc( length );
    strcpy( dw->dialog.value, initial_value );
    XtSetArg(arglist[num_args], XtNwidth,
	     dw->dialog.labelW->core.width); num_args++; /* ||| hack */
    XtSetArg(arglist[num_args], XtNstring, dw->dialog.value); num_args++;
    XtSetArg(arglist[num_args], XtNlength, length); num_args++;
    XtSetArg(arglist[num_args], XtNfromVert, dw->dialog.labelW); num_args++;
    XtSetArg(arglist[num_args], XtNtextOptions, (resizeWidth | resizeHeight));
    num_args++;
    XtSetArg(arglist[num_args], XtNeditType, XttextEdit); num_args++;
    
    dw->dialog.valueW = XtCreateManagedWidget("value",asciiStringWidgetClass,
					      w, arglist, num_args);
#ifdef notdef
    static int grabfocus;
    static Resource resources[] = {
        {XtNgrabFocus, XtCGrabFocus, XtRBoolean, sizeof(int),
	   (caddr_t)&grabfocus, (caddr_t)NULL}
    };
    XrmNameList names;
    XrmClassList classes;
    grabfocus = FALSE;
    XtGetResources(dpy, resources, XtNumber(resources), args, argCount,
		   parent, "dialog", "Dialog", &names, &classes);
    XrmFreeNameList(names);
    XrmFreeClassList(classes);
    if (grabfocus) XSetInputFocus(dpy, data->value, RevertToParent,
				  CurrentTime); /* !!! Hackish. |||*/
#endif notdef
}

void XtDialogAddButton(dialog, name, function, param)
Widget dialog;
char *name;
void (*function)();
caddr_t param;
{
/*
 * Correct Constraints are all set in ConstraintInitialize().
 */
    Widget button;

    button = XtCreateManagedWidget( name, commandWidgetClass, dialog, 
				    NULL, (Cardinal) 0 );

    if (function != NULL)	/* don't add NULL callback func. */
        XtAddCallback(button, XtNcallback, function, param);
}


char *XtDialogGetValueString(w)
Widget w;
{
    return ((DialogWidget)w)->dialog.value;
}
