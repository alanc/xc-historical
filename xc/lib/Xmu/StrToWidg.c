/* $XConsortium: StrToWidg.c,v 1.5 90/12/19 19:08:59 converse Exp $ */

/* Copyright 1988 Massachusetts Institute of Technology
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
 *
 *
 * XmuCvtStringToWidget
 *
 *   static XtConvertArgRec parentCvtArgs[] = {
 *	{XtBaseOffset, (caddr_t)XtOffset(Widget, core.parent), sizeof(Widget)},
 *   };
 *
 * matches the string against the name of the immediate children (normal
 * or popup) of the parent.  If none match, compares string to classname
 * & returns first match.  Case is significant.
 */

#include <X11/IntrinsicP.h>
#include <X11/ObjectP.h>
#include <X11/Object.h>

#define	done(address, type) \
	{ toVal->size = sizeof(type); \
	  toVal->addr = (caddr_t) address; \
	  return; \
	}


#define NewDone(where_flag,address,type) \
{ \
    if (where_flag) \
	done(address,type) \
    else \
	{ \
	      toVal->size = sizeof(type); \
	      (type)*(toVal->addr) = *address; \
	      return; \
	} \
}


/* ARGSUSED */
void XmuCvtStringToWidget(args, num_args, fromVal, toVal)
    XrmValuePtr args;		/* parent */
    Cardinal    *num_args;      /* 1 */
    XrmValuePtr fromVal;
    XrmValuePtr toVal;
{
    static Widget widget, *widgetP, parent;
    XrmName name = XrmStringToName(fromVal->addr);
    int i;

    if (*num_args != 1)
	XtErrorMsg("wrongParameters", "cvtStringToWidget", "xtToolkitError",
		   "StringToWidget conversion needs parent arg", NULL, 0);

    parent = *(Widget*)args[0].addr;
    /* try to match names of normal children */
    if (XtIsComposite(parent)) {
	i = ((CompositeWidget)parent)->composite.num_children;
	for (widgetP = ((CompositeWidget)parent)->composite.children;
	     i; i--, widgetP++) {
	    if ((*widgetP)->core.xrm_name == name) {
		widget = *widgetP;
		done(&widget, Widget);
	    }
	}
    }
    /* try to match names of popup children */
    i = parent->core.num_popups;
    for (widgetP = parent->core.popup_list; i; i--, widgetP++) {
	if ((*widgetP)->core.xrm_name == name) {
	    widget = *widgetP;
	    done(&widget, Widget);
	}
    }
    /* try to match classes of normal children */
    if (XtIsComposite(parent)) {
	i = ((CompositeWidget)parent)->composite.num_children;
	for (widgetP = ((CompositeWidget)parent)->composite.children;
	     i; i--, widgetP++) {
	    if ((*widgetP)->core.widget_class->core_class.xrm_class == name) {
		widget = *widgetP;
		done(&widget, Widget);
	    }
	}
    }
    /* try to match classes of popup children */
    i = parent->core.num_popups;
    for (widgetP = parent->core.popup_list; i; i--, widgetP++) {
	if ((*widgetP)->core.widget_class->core_class.xrm_class == name) {
	    widget = *widgetP;
	    done(&widget, Widget);
	}
    }
    XtStringConversionWarning(fromVal->addr, "Widget");
    toVal->addr = NULL;
    toVal->size = 0;
}


/* ARGSUSED */
void XmuNewCvtStringToWidget(display, args, num_args, fromVal, toVal, 
			     converter_data)
     Display *display;
     XrmValue *args;		/* parent */
     Cardinal *num_args;      /* 1 */
     XrmValue *fromVal;
     XrmValue *toVal;
     XtPointer *converter_data;
{
    static Widget widget, *widgetP, parent;
    XrmName name = XrmStringToName(fromVal->addr);
    int i;
    Boolean alloc_to_space;

    if (*num_args != 1)
	XtErrorMsg("wrongParameters", "cvtStringToWidget", "xtToolkitError",
		   "StringToWidget conversion needs parent arg", NULL, 0);

    alloc_to_space = True;
    if (toVal->addr != NULL)
	{
	    if (toVal->size < sizeof(Widget))
		done(toVal->addr, Widget);
	    alloc_to_space = False;
	}


    parent = *(Widget*)args[0].addr;
    /* try to match names of normal children */
    if (XtIsComposite(parent)) {
	i = ((CompositeWidget)parent)->composite.num_children;
	for (widgetP = ((CompositeWidget)parent)->composite.children;
	     i; i--, widgetP++) {
	    if ((*widgetP)->core.xrm_name == name) {
		widget = *widgetP;
		NewDone(alloc_to_space,&widget, Widget);
	    }
	}
    }
    /* try to match names of popup children */
    i = parent->core.num_popups;
    for (widgetP = parent->core.popup_list; i; i--, widgetP++) {
	if ((*widgetP)->core.xrm_name == name) {
	    widget = *widgetP;
	    NewDone(alloc_to_space,&widget, Widget);
	}
    }
    /* try to match classes of normal children */
    if (XtIsComposite(parent)) {
	i = ((CompositeWidget)parent)->composite.num_children;
	for (widgetP = ((CompositeWidget)parent)->composite.children;
	     i; i--, widgetP++) {
	    if ((*widgetP)->core.widget_class->core_class.xrm_class == name) {
		widget = *widgetP;
		NewDone(alloc_to_space,&widget, Widget);
	    }
	}
    }
    /* try to match classes of popup children */
    i = parent->core.num_popups;
    for (widgetP = parent->core.popup_list; i; i--, widgetP++) {
	if ((*widgetP)->core.widget_class->core_class.xrm_class == name) {
	    widget = *widgetP;
	    NewDone(alloc_to_space,&widget, Widget);
	}
    }
    XtStringConversionWarning(fromVal->addr, "Widget");
    toVal->addr = NULL;
    toVal->size = 0;
}

