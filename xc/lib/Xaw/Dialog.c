#ifndef lint
static char rcsid[] = "$Header: Dialog.c,v 1.3 87/12/23 07:41:03 swick Locked $";
#endif lint
/*
 * Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.
 * 
 *                         All Rights Reserved
 * 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in 
 * supporting documentation, and that the name of Digital Equipment
 * Corporation not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.  
 * 
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */


/* NOTE: THIS IS NOT A WIDGET!  Rather, this is an interface to a widget.
   It implements policy, and gives a (hopefully) easier-to-use interface
   than just directly making your own form. */


#include <X/Xlib.h>
#include <X/Xos.h>
#include <X/Intrinsic.h>
#include <X/Misc.h>
#include <X/Atoms.h>
#include <X/Form.h>
#include <X/Dialog.h>
#include <X/AsciiText.h>
#include <X/Command.h>
#include <X/Label.h>
#include "FormP.h"
#include "DialogP.h"


XtResource resources[] = {
  {XtNlabel, XtCLabel, XrmRString, sizeof(String),
     XtOffset(DialogWidget, dialog.label), XrmRString, NULL},
  {XtNvalue, XtCValue, XrmRString, sizeof(String),
     XtOffset(DialogWidget, dialog.value), XrmRString, NULL},
  {XtNmaximumLength, XtCMax, XrmRInt, sizeof(int),
     XtOffset(DialogWidget, dialog.max_length), XrmRString, "256"}
};

static void Initialize(), ConstraintInitialize();
static Boolean SetValues();

DialogClassRec dialogClassRec = {
  { /* core_class fields */
    /* superclass         */    (WidgetClass) &formClassRec,
    /* class_name         */    "Dialog",
    /* widget_size        */    sizeof(DialogRec),
    /* class_initialize   */    NULL,
    /* class_inited       */    FALSE,
    /* initialize         */    Initialize,
    /* realize            */    XtInheritRealize,
    /* actions            */    NULL,
    /* num_actions        */    0,
    /* resources          */    resources,
    /* num_resources      */    XtNumber(resources),
    /* xrm_class          */    NULLQUARK,
    /* compress_motion    */    TRUE,
    /* compress_exposure  */    TRUE,
    /* visible_interest   */    FALSE,
    /* destroy            */    NULL,
    /* resize             */    XtInheritResize,
    /* expose             */    XtInheritExpose,
    /* set_values         */    SetValues,
    /* accept_focus       */    NULL,
    /* callback_private   */    NULL,
    /* reserved_private   */    NULL
  },
  { /* composite_class fields */
    /* geometry_manager   */   XtInheritGeometryManager,
    /* change_managed     */   XtInheritChangeManaged,
    /* insert_child       */   XtInheritInsertChild,
    /* delete_child       */   XtInheritDeleteChild,
    /* move_focus_to_next */   NULL,
    /* move_focus_to_prev */   NULL
  },
  { /* constraint_class fields */
    /* subresourses       */   NULL,
    /* subresource_count  */   0,
    /* constraint_size    */   sizeof(DialogConstraintsRec),
    /* initialize         */   ConstraintInitialize,
    /* destroy            */   NULL,
    /* set_values         */   NULL
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
static void Initialize(request, new, args, num_args)
Widget request, new;
ArgList args;
Cardinal num_args;
{
    DialogWidget dw = (DialogWidget)new;
    static Arg label_args[] = {
	{XtNlabel, (XtArgVal)NULL},
	{XtNborderWidth, (XtArgVal) 0}
    };
    static Arg text_args[] = {
	{XtNwidth, (XtArgVal)NULL},
	{XtNstring, (XtArgVal)NULL},
	{XtNlength, (XtArgVal)0},
	{XtNfromVert, (XtArgVal)NULL},
	{XtNresizable, (XtArgVal)TRUE},
	{XtNtextOptions, (XtArgVal)(resizeWidth | resizeHeight)},
	{XtNeditType, (XtArgVal)XttextEdit},
	{XtNright, (XtArgVal)XtChainRight}
    };
    Widget children[2], *childP = children;

    label_args[0].value = (XtArgVal)dw->dialog.label;
    dw->dialog.labelW = XtCreateWidget( "label", labelWidgetClass, new,
				        label_args, XtNumber(label_args) );
    *childP++ = dw->dialog.labelW;

    if (dw->dialog.value) {
        String initial_value = dw->dialog.value;
	Cardinal length = Max( dw->dialog.max_length, strlen(initial_value) );
	dw->dialog.value = XtMalloc( length );
	strcpy( dw->dialog.value, initial_value );
	text_args[0].value = (XtArgVal)dw->dialog.labelW->core.width; /*|||hack*/
	text_args[1].value = (XtArgVal)dw->dialog.value;
	text_args[2].value = (XtArgVal)length;
	text_args[3].value = (XtArgVal)dw->dialog.labelW;
	dw->dialog.valueW = XtCreateWidget("value",asciiStringWidgetClass,new,
					   text_args, XtNumber(text_args) );
	*childP++ = dw->dialog.valueW;
#ifdef notdef
	static int grabfocus;
	static Resource resources[] = {
	    {XtNgrabFocus, XtCGrabFocus, XrmRBoolean, sizeof(int),
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
    } else {
        dw->dialog.valueW = NULL;
    }

    XtManageChildren( children, (Cardinal)(childP - children) );
}


/* ARGSUSED */
static void ConstraintInitialize(request, new, args, num_args)
Widget request, new;
ArgList args;
Cardinal num_args;
{
    DialogWidget dw = (DialogWidget)new->core.parent;
    WidgetList children = dw->composite.children;
    DialogConstraints constraint = (DialogConstraints)new->core.constraints;
    Widget *childP;

    if (dw->composite.num_children == 0			/* is labelW? */
	|| (dw->composite.num_children == 1 &&
	    XtClass(new) == asciiStringWidgetClass))	/* or valueW? */
      return;					/* then just use defaults */

    constraint->form.left = constraint->form.right = XtChainLeft;
    constraint->form.vert_base = dw->dialog.valueW
				 ? dw->dialog.valueW
				 : dw->dialog.labelW;

    if (dw->composite.num_mapped_children > 1) {
        for (childP = children + dw->composite.num_children - 1;
	     childP >= children; childP-- ) {
	    if (*childP == dw->dialog.labelW || *childP == dw->dialog.valueW)
	        break;
	    if (XtIsManaged(*childP)) {
	        constraint->form.horiz_base = *childP;
		break;
	    }
	}
    }
}


/* ARGSUSED */
static Boolean SetValues(current, request, new, last)
Widget current, request, new;
Boolean last;
{
    return False;
}


#ifdef notdef
void XtDialogAddButton(dpy, window, name, function, param)
Display *dpy;
Window window;
char *name;
void (*function)();
caddr_t param;
{
    WidgetData data;
    static Arg arglist1[] = {
	{XtNname, (XtArgVal) NULL},
	{XtNfunction, (XtArgVal) NULL},
	{XtNparameter, (XtArgVal) NULL}
    };
    static Arg arglist2[] = {
	{XtNfromHoriz, (XtArgVal) NULL},
	{XtNfromVert, (XtArgVal) NULL},
	{XtNleft, (XtArgVal) XtChainLeft},
	{XtNright, (XtArgVal) XtChainLeft}
    };
    data = DataFromWindow(dpy, window);
    if (data == NULL) return;
    arglist1[0].value = (XtArgVal) name;
    arglist1[1].value = (XtArgVal) function;
    arglist1[2].value = (XtArgVal) param;
    data->button = (Window *)
	XtRealloc((char *)data->button,
		  (unsigned) ++data->numbuttons * sizeof(Window));
    data->button[data->numbuttons - 1] =
	XtCommandCreate(data->dpy, window, arglist1, XtNumber(arglist1));
    if (data->numbuttons > 1)
	arglist2[0].value = (XtArgVal) data->button[data->numbuttons - 2];
    else
	arglist2[0].value = (XtArgVal) NULL;
    if (data->value) arglist2[1].value = (XtArgVal) data->value;
    else arglist2[1].value = (XtArgVal) data->label;
    XtFormAddWidget(data->dpy, data->mywin, data->button[data->numbuttons - 1],
		    arglist2, XtNumber(arglist2));
}
#endif


char *XtDialogGetValueString(w)
Widget w;
{
    return ((DialogWidget)w)->dialog.value;
}
