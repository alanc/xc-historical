#ifndef lint
static char Xrcsid[] = "$XConsortium: Dialog.c,v 1.30 89/05/31 09:01:08 swick Exp $";
#endif /* lint */


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
#include <X11/StringDefs.h>
#include <X11/Xmu/Misc.h>

#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Command.h>	
#include <X11/Xaw/Label.h>
#include <X11/Xaw/DialogP.h>
#include <X11/Xaw/Cardinals.h>

#define streq(a,b) (strcmp( (a), (b) ) == 0)

static XtResource resources[] = {
  {XtNlabel, XtCLabel, XtRString, sizeof(String),
     XtOffset(DialogWidget, dialog.label), XtRString, NULL},
  {XtNvalue, XtCValue, XtRString, sizeof(String),
     XtOffset(DialogWidget, dialog.value), XtRString, NULL},
  {XtNicon, XtCIcon, XtRPixmap, sizeof(Pixmap),
     XtOffset(DialogWidget, dialog.icon), XtRImmediate, 0},
  {XtNmaximumLength, XtCMax, XtRInt, sizeof(int),
     XtOffset(DialogWidget, dialog.max_length), XtRImmediate, (caddr_t)256}
};

static void Initialize(), ConstraintInitialize(), CreateDialogValueWidget();
static void Destroy();
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
    /* destroy            */    Destroy,
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
    /* layout             */   XtInheritLayout
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
    Arg arglist[9];
    Cardinal num_args = 0;

    XtSetArg(arglist[num_args], XtNborderWidth, 0); num_args++;
    XtSetArg(arglist[num_args], XtNleft, XtChainLeft); num_args++;

    if (dw->dialog.icon != (Pixmap)0) {
	XtSetArg(arglist[num_args], XtNbitmap, dw->dialog.icon); num_args++;
	XtSetArg(arglist[num_args], XtNright, XtChainLeft); num_args++;
	dw->dialog.iconW =
	    XtCreateManagedWidget( "icon", labelWidgetClass,
				   new, arglist, num_args );
	num_args = 2;
	XtSetArg(arglist[num_args], XtNfromHoriz, dw->dialog.iconW);num_args++;
    } else dw->dialog.iconW = (Widget)NULL;

    XtSetArg(arglist[num_args], XtNlabel, dw->dialog.label); num_args++;
    XtSetArg(arglist[num_args], XtNright, XtChainRight); num_args++;

    dw->dialog.labelW = XtCreateManagedWidget( "label", labelWidgetClass,
					      new, arglist, num_args);

    if (dw->dialog.iconW != (Widget)NULL &&
	(dw->dialog.labelW->core.height < dw->dialog.iconW->core.height)) {
	XtSetArg( arglist[0], XtNheight, dw->dialog.iconW->core.height );
	XtSetValues( dw->dialog.labelW, arglist, ONE );
    }
    if (dw->dialog.value != NULL) 
        CreateDialogValueWidget( (Widget) dw);
    else
        dw->dialog.valueW = NULL;
}

static void Destroy(w)
Widget w;
{
    DialogWidget dw = (DialogWidget) w;

    if (dw->dialog.value != NULL) XtFree(dw->dialog.value);
}

/* ARGSUSED */
static void ConstraintInitialize(request, new)
Widget request, new;
{
    DialogWidget dw = (DialogWidget)new->core.parent;
    DialogConstraints constraint = (DialogConstraints)new->core.constraints;

    if (!XtIsSubclass(new, commandWidgetClass))	/* if not a button */
	return;					/* then just use defaults */

    constraint->form.left = constraint->form.right = XtChainLeft;
    if (dw->dialog.valueW == NULL) 
      constraint->form.vert_base = dw->dialog.labelW;
    else
      constraint->form.vert_base = dw->dialog.valueW;

    if (dw->composite.num_children > 1) {
	WidgetList children = dw->composite.children;
	Widget *childP;
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

    if (w->dialog.icon != old->dialog.icon) {
	if (w->dialog.icon != (Pixmap)0) {
	    Arg args[5];
	    XtSetArg( args[0], XtNbitmap, w->dialog.icon );
	    if (old->dialog.iconW != (Widget)NULL) {
		XtSetValues( old->dialog.iconW, args, ONE );
	    } else {
		XtSetArg( args[1], XtNborderWidth, 0);
		XtSetArg( args[2], XtNleft, XtChainLeft);
		XtSetArg( args[3], XtNright, XtChainLeft);
		w->dialog.iconW =
		    XtCreateWidget( "icon", labelWidgetClass,
				    new, args, FOUR );
		((DialogConstraints)w->dialog.labelW->core.constraints)->
		    form.horiz_base = w->dialog.iconW;
		XtManageChild(w->dialog.iconW);
	    }
	} else {
	    ((DialogConstraints)w->dialog.labelW->core.constraints)->
		    form.horiz_base = (Widget)NULL;
	    XtDestroyWidget(old->dialog.iconW);
	    w->dialog.iconW = (Widget)NULL;
	}
    }
    if ( (w->dialog.label != old->dialog.label) ||
	 ((w->dialog.label != NULL) && /* so both are non-NULL */
	   !streq(w->dialog.label, old->dialog.label)) ) {
        num_args = 0;
        XtSetArg( args[num_args], XtNlabel, w->dialog.label ); num_args++;
	if (w->dialog.iconW != (Widget)NULL &&
	    (w->dialog.labelW->core.height <= w->dialog.iconW->core.height)) {
	    XtSetArg(args[num_args], XtNheight, w->dialog.iconW->core.height);
	    num_args++;
	}
	XtSetValues( w->dialog.labelW, args, num_args );
    }

    if ( w->dialog.value != old->dialog.value ) {
        if (w->dialog.value == NULL) { /* only get here if it
					  wasn't NULL before. */
	    XtDestroyWidget(old->dialog.valueW);
	    XtFree(old->dialog.value);
	}
	else if (old->dialog.value == NULL) { /* create a new value widget. */
	    w->core.width = old->core.width;
	    w->core.height = old->core.height;
#ifdef notdef
/* this would be correct if Form had the same semantics on Resize
 * as on MakeGeometryRequest.  Unfortunately, Form botched it, so
 * any subclasses will currently have to deal with the fact that
 * we're about to change our real size.
 */
	    w->form.resize_in_layout = False; */
	    CreateDialogValueWidget( (Widget) w);
	    w->core.width = w->form.preferred_width;
	    w->core.height = w->form.preferred_height;
	    w->form.resize_in_layout = True;
#else /*notdef*/
	    CreateDialogValueWidget( (Widget) w);
#endif /*notdef*/
	}
	else {			/* Widget ok, just change string. */
	    XawTextBlock t_block;

	    t_block.firstPos = 0;
	    t_block.length = strlen(w->dialog.value);
	    t_block.ptr = w->dialog.value;
	    t_block.format = FMT8BIT;

	    if (XawTextReplace(w->dialog.valueW, 
			      0, strlen(old->dialog.value), &t_block) !=
		XawEditDone) 
	        XtWarning("Error while changing value in Dialog Widget.");

	    /* the new value will have just been copied into our (private)
	     * buffer copy, and we want to preserve our private pointer...
	     */
	    w->dialog.value = old->dialog.value;
	}
    }
    return False;
}

/*	Function Name: CreateDialogValueWidget
 *	Description: Creates the dialog widgets value widget.
 *	Arguments: w - the dialog widget.
 *	Returns: none.
 *
 *	must be called only when w->dialog.value is non-nil.
 */

static void
CreateDialogValueWidget(w)
Widget w;
{
    DialogWidget dw = (DialogWidget) w;    
    String initial_value = dw->dialog.value;
    Cardinal length = strlen(initial_value)+1;
    Arg arglist[10];
    Cardinal num_args = 0;

    if (dw->dialog.max_length < length)
	dw->dialog.max_length = length;
    else
	length = dw->dialog.max_length;
    dw->dialog.value = XtMalloc( length );
    strcpy( dw->dialog.value, initial_value );
#ifdef notdef
    XtSetArg(arglist[num_args], XtNwidth,
	     dw->dialog.labelW->core.width); num_args++; /* ||| hack */
#endif /*notdef*/
    XtSetArg(arglist[num_args], XtNstring, dw->dialog.value); num_args++;
    XtSetArg(arglist[num_args], XtNlength, length); num_args++;
    XtSetArg(arglist[num_args], XtNfromVert, dw->dialog.labelW); num_args++;
    XtSetArg(arglist[num_args], XtNresizable, True); num_args++;
    XtSetArg(arglist[num_args], XtNtextOptions, (resizeWidth | resizeHeight));
    num_args++;
    XtSetArg(arglist[num_args], XtNeditType, XawtextEdit); num_args++;
    XtSetArg(arglist[num_args], XtNleft, XtChainLeft); num_args++;
    XtSetArg(arglist[num_args], XtNright, XtChainRight); num_args++;
    
    dw->dialog.valueW = XtCreateWidget("value",asciiStringWidgetClass,
				       w, arglist, num_args);

    /* if the value widget is being added after buttons,
     * then the buttons need new layout constraints.
     */
    if (dw->composite.num_children > 1) {
	WidgetList children = dw->composite.children;
	Widget *childP;
        for (childP = children + dw->composite.num_children - 1;
	     childP >= children; childP-- ) {
	    if (*childP == dw->dialog.labelW || *childP == dw->dialog.valueW)
		continue;
	    if (XtIsManaged(*childP) &&
		XtIsSubclass(*childP, commandWidgetClass)) {
	        ((DialogConstraints)(*childP)->core.constraints)->
		    form.vert_base = dw->dialog.valueW;
	    }
	}
    }
    XtManageChild(dw->dialog.valueW);

/* 
 * Value widget gets the keyboard focus.
 */

   XtSetKeyboardFocus(w, dw->dialog.valueW);
}


void XawDialogAddButton(dialog, name, function, param)
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


char *XawDialogGetValueString(w)
Widget w;
{
    return ((DialogWidget)w)->dialog.value;
}
