#ifndef lint
static char Xrcsid[] = "$XConsortium: Intrinsic.c,v 1.132 89/09/14 10:11:28 swick Exp $";
/* $oHeader: Intrinsic.c,v 1.4 88/08/18 15:40:35 asente Exp $ */
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

#define INTRINSIC_C

#include "IntrinsicI.h"
#include "StringDefs.h"


#ifdef DEBUG
int _XtIdentifyWindows = False;
#endif

static void SetAncestorSensitive();

Boolean XtIsSubclass(widget, widgetClass)
    Widget    widget;
    WidgetClass widgetClass;
{
    register WidgetClass w;

    for (w = widget->core.widget_class; w != NULL; w = w->core_class.superclass)
	if (w == widgetClass) return (TRUE);
    return (FALSE);
} /* XtIsSubclass */


Boolean _XtCheckSubclassFlag(object, flag)
    Widget object;
    XtEnum flag;
{
    if (object->core.widget_class->core_class.class_inited & flag)
	return True;
    else
	return False;

} /*_XtVerifySubclass */


Boolean _XtIsSubclassOf(object, widgetClass, superClass, flag)
    Widget object;
    WidgetClass widgetClass, superClass;
    XtEnum flag;
{
    if (!(object->core.widget_class->core_class.class_inited & flag))
	return False;
    else {
	register WidgetClass c = object->core.widget_class;
	while (c != superClass) {
	    if (c == widgetClass)
		return True;
	    c = c->core_class.superclass;
	}
	return False;
    }
} /*_XtIsSubclassOf */


static void ComputeWindowAttributes(widget,value_mask,values)
    Widget		 widget;
    XtValueMask		 *value_mask;
    XSetWindowAttributes *values;
{
    *value_mask = CWEventMask;
    (*values).event_mask = XtBuildEventMask(widget);
    if (widget->core.background_pixmap != XtUnspecifiedPixmap) {
	*value_mask |= CWBackPixmap;
	(*values).background_pixmap = widget->core.background_pixmap;
    } else {
	*value_mask |= CWBackPixel;
	(*values).background_pixel = widget->core.background_pixel;
    }
    if (widget->core.border_pixmap != XtUnspecifiedPixmap) {
	*value_mask |= CWBorderPixmap;
	(*values).border_pixmap = widget->core.border_pixmap;
    } else {
	*value_mask |= CWBorderPixel;
	(*values).border_pixel = widget->core.border_pixel;
    }
    if (widget->core.widget_class->core_class.expose == (XtExposeProc) NULL) {
	/* Try to avoid redisplay upon resize by making bit_gravity the same
	   as the default win_gravity */
	*value_mask |= CWBitGravity;
	(*values).bit_gravity = NorthWestGravity;
    }
} /* ComputeWindowAttributes */

static void CallChangeManaged(widget)
    register Widget		widget;
{
    register Cardinal		i;
    XtWidgetProc		change_managed;
    register WidgetList		children;
    int    			managed_children = 0;

    register CompositePtr cpPtr;
    register CompositePartPtr clPtr;
   
    if (XtIsComposite (widget)) {
	cpPtr = (CompositePtr)&((CompositeWidget) widget)->composite;
        clPtr = (CompositePartPtr)&((CompositeWidgetClass)
                   widget->core.widget_class)->composite_class;
    } else return;

    children = cpPtr->children;
    change_managed = clPtr->change_managed;

    /* CallChangeManaged for all children */
    for (i = cpPtr->num_children; i != 0; --i) {
	CallChangeManaged (children[i-1]);
	if (XtIsManaged(children[i-1])) managed_children++;
    }

    if (change_managed != NULL && managed_children != 0) {
	(*change_managed) (widget);
    }
} /* CallChangeManaged */


static void MapChildren(cwp)
    CompositePart *cwp;
{
    Cardinal i;
    WidgetList children;
    register Widget child;

    children = cwp->children;
    for (i = 0; i <  cwp->num_children; i++) {
	child = children[i];
	if (XtIsWidget (child)){
	    if (child->core.managed && child->core.mapped_when_managed) {
		XtMapWidget (children[i]);
	    }
	}
    }
} /* MapChildren */


static Boolean ShouldMapAllChildren(cwp)
    CompositePart *cwp;
{
    Cardinal i;
    WidgetList children;
    register Widget child;

    children = cwp->children;
    for (i = 0; i < cwp->num_children; i++) {
	child = children[i];
	if (XtIsWidget(child)) {
	    if (XtIsRealized(child) && (! (child->core.managed 
					  && child->core.mapped_when_managed))){
		    return False;
	    }
	}
    }

    return True;
} /* ShouldMapAllChildren */


static void RealizeWidget(widget)
    register Widget		widget;
{
    XtValueMask			value_mask;
    XSetWindowAttributes	values;
    XtRealizeProc		realize;
    Window			window;

    if (!XtIsWidget(widget) || XtIsRealized(widget)) return;

    if (widget->core.tm.proc_table == NULL)
	_XtBindActions(widget, &widget->core.tm, (unsigned)0);
    _XtInstallTranslations(widget, widget->core.tm.translations);

    ComputeWindowAttributes (widget, &value_mask, &values);
    realize = widget->core.widget_class->core_class.realize;
    if (realize == NULL)
	XtErrorMsg("invalidProcedure","realizeProc","XtToolkitError",
	    "No realize class procedure defined",
	      (String *)NULL, (Cardinal *)NULL);
    else (*realize) (widget, &value_mask, &values);
    window = XtWindow(widget);
#ifdef DEBUG
    if (_XtIdentifyWindows)
	XStoreName( XtDisplay(widget), window, widget->core.name );
#endif
#ifdef notdef
    _XtRegisterAsyncHandlers(widget);
#endif
    _XtRegisterGrabs(widget,&widget->core.tm);
    _XtRegisterWindow (window, widget);

    if (XtIsComposite (widget)) {
	register Cardinal		i;
	register CompositePart *cwp = &(((CompositeWidget)widget)->composite);
	register WidgetList children = cwp->children;
	/* Realize all children */
	for (i = cwp->num_children; i != 0; --i) {
	    RealizeWidget (children[i-1]);
	}
	/* Map children that are managed and mapped_when_managed */

	if (cwp->num_children != 0) {
	    if (ShouldMapAllChildren(cwp)) {
		XMapSubwindows (XtDisplay(widget), window);
	    } else {
		MapChildren(cwp);
	    }
	}
    }

    /* If this is the application's popup shell, map it */
    if (widget->core.parent == NULL && widget->core.mapped_when_managed) {
	XtMapWidget (widget);
    }
} /* RealizeWidget */

void XtRealizeWidget (widget)
    register Widget		widget;
{
    if (XtIsRealized (widget)) return;

    CallChangeManaged(widget);
    RealizeWidget(widget);
} /* XtRealizeWidget */


static void UnrealizeWidget(widget)
    register Widget		widget;
{
    register CompositeWidget	cw;
    register Cardinal		i;
    register WidgetList		children;

    if (! XtIsRealized(widget)) return;

    /* If this is the application's popup shell, unmap it? */
    /* no, the window is being destroyed */

    /* Recurse on children */
    if (XtIsComposite (widget)) {
	cw = (CompositeWidget) widget;
	children = cw->composite.children;
	/* Unrealize all children */
	for (i = cw->composite.num_children; i != 0; --i) {
	    UnrealizeWidget (children[i-1]);
	}
	/* Unmap children that are managed and mapped_when_managed? */
	/* No, it's ok to be managed and unrealized as long as your parent */
	/* is unrealized. XtUnrealize widget makes sure the "top" widget */
	/* is unmanaged, we can ignore all descendents */
    }

    /* Unregister window */
    _XtUnregisterWindow(XtWindow(widget), widget);

    /* Remove Event Handlers */
    /* remove async handlers, how? */
    /* remove grabs. Happens automatically when window is destroyed. */

    /* Destroy X Window, done at outer level with one request */
    widget->core.window = NULL;

    /* Unbind actions? Nope, we check in realize to see if done. */
    /* Uninstall Translations? */
    XtUninstallTranslations(widget);

} /* UnrealizeWidget */


void XtUnrealizeWidget (widget)
    register Widget		widget;
{
    Window window = XtWindow(widget);

    if (! XtIsRealized (widget)) return;

    if (widget->core.parent != NULL) XtUnmanageChild(widget);

    UnrealizeWidget(widget);

    if (window != NULL) XDestroyWindow(XtDisplay(widget), window);
} /* XtUnrealizeWidget */


void XtCreateWindow(widget, window_class, visual, value_mask, attributes)
    Widget		 widget;
    unsigned int	 window_class;
    Visual		 *visual;
    Mask		 value_mask;
    XSetWindowAttributes *attributes;
{
    if (widget->core.window == None) {
	if (widget->core.width == 0 || widget->core.height == 0) {
	    Cardinal count = 1;
	    XtErrorMsg("invalidDimension", "xtCreateWindow", "XtToolkitError",
		       "Widget %s has zero width and/or height",
		       &widget->core.name, &count);
	}
	widget->core.window =
	    XCreateWindow (
		XtDisplay (widget),
		(widget->core.parent ?
		    widget->core.parent->core.window :
		    widget->core.screen->root),
		(int)widget->core.x, (int)widget->core.y,
		(unsigned)widget->core.width, (unsigned)widget->core.height,
		(unsigned)widget->core.border_width, (int) widget->core.depth,
		window_class, visual, value_mask, attributes);
    }
} /* XtCreateWindow */
			
void XtSetSensitive(widget, sensitive)
    register Widget widget;
    Boolean	    sensitive;
{
    Arg			args[1];
    register Cardinal   i;
    register WidgetList children;

    if (widget->core.sensitive == sensitive) return;

    XtSetArg(args[0], XtNsensitive, sensitive);
    XtSetValues(widget, args, XtNumber(args));

    /* If widget's ancestor_sensitive is TRUE, propagate new sensitive to
       children's ancestor_sensitive; else do nothing as children's
       ancestor_sensitive is already FALSE */
    
    if (widget->core.ancestor_sensitive && XtIsComposite (widget)) {
	children = ((CompositeWidget) widget)->composite.children;
	for (i = 0; i < ((CompositeWidget)widget)->composite.num_children; i++){
	    SetAncestorSensitive (children[i], sensitive);
	}
    }
} /* XtSetSensitive */

static void SetAncestorSensitive(widget, ancestor_sensitive)
    register Widget widget;
    Boolean	    ancestor_sensitive;
{
    Arg			args[1];
    register Cardinal   i;
    register WidgetList children;

    if (widget->core.ancestor_sensitive == ancestor_sensitive) return;

    XtSetArg(args[0], XtNancestorSensitive, ancestor_sensitive);
    XtSetValues(widget, args, XtNumber(args));

    /* If widget's sensitive is TRUE, propagate new ancestor_sensitive to
       children's ancestor_sensitive; else do nothing as children's
       ancestor_sensitive is already FALSE */
    
    if (widget->core.sensitive && XtIsComposite(widget)) {
	children = ((CompositeWidget) widget)->composite.children;
	for (i = 0; i < ((CompositeWidget)widget)->composite.num_children; i++){
	    SetAncestorSensitive (children[i], ancestor_sensitive);
	}
    }
} /* SetAncestorSensitive */

/* ---------------- XtNameToWidget ----------------- */

static Widget NameListToWidget(root, names)
    register Widget root;
    XrmNameList     names;
{
    register Cardinal   i;
    register WidgetList children;
    register XrmName    name;

    name = *names;
    if (name == NULLQUARK) return root;
    if (XtIsComposite(root)) {
        children = ((CompositeWidget) root)->composite.children;
        for (i = 0;
                i < ((CompositeWidget) root)->composite.num_children; i++) {
            if (name == children[i]->core.xrm_name)
	        return NameListToWidget(children[i], &names[1]);
        }
    }
    children = root->core.popup_list;
    for (i = 0; i < root->core.num_popups; i++) {
	if (name == children[i]->core.xrm_name)
	    return NameListToWidget(children[i], &names[1]);
    }
    return NULL;
} /* NameListToWidget */

Widget XtNameToWidget(root, name)
    Widget root;
    String name;
{
    XrmName	names[100];
    XrmStringToNameList(name, names);
    if (names[0] == NULLQUARK) return NULL;
    return NameListToWidget(root, names);
} /* XtNameToWidget */

/* Define user versions of intrinsics macros */

#undef XtDisplayOfObject
Display *XtDisplayOfObject(object)
     Widget object;
{
    return XtDisplay(XtIsWidget(object) ? object : _XtWindowedAncestor(object));
}

#undef XtDisplay
Display *XtDisplay(widget)
	Widget widget;
{
    return DisplayOfScreen(widget->core.screen);
}

#undef XtScreenOfObject
Screen *XtScreenOfObject(object)
     Widget object;
{
    return XtScreen(XtIsWidget(object) ? object : _XtWindowedAncestor(object));
}

#undef XtScreen
Screen *XtScreen(widget)
	Widget widget;
{
    return widget->core.screen;
}

#undef XtWindowOfObject
Window XtWindowOfObject(object)
     Widget object;
{
    return XtWindow(XtIsWidget(object) ? object : _XtWindowedAncestor(object));
}


#undef XtWindow
Window XtWindow(widget)
	Widget widget;
{
    return widget->core.window;
}

#undef XtSuperclass
WidgetClass XtSuperclass(widget)
	Widget widget;
{
	return XtClass(widget)->core_class.superclass;
}

#undef XtClass
WidgetClass XtClass(widget)
	Widget widget;
{
	return widget->core.widget_class;
}

#undef XtIsManaged
Boolean XtIsManaged(object)
	Widget object;
{
    if (XtIsRectObj(object))
	return object->core.managed;
    else
	return False;
}

#undef XtIsRealized
Boolean XtIsRealized (object)
	Widget   object;
{
    return XtWindowOfObject(object) != NULL;
} /* XtIsRealized */

#undef XtIsSensitive
Boolean XtIsSensitive(object)
	Widget	object;
{
    if (XtIsRectObj(object))
	return object->core.sensitive && object->core.ancestor_sensitive;
    else
	return False;
}

/*
 * Internal routine; must be called only after XtIsWidget returns false
 */
Widget _XtWindowedAncestor(object)
	Widget object;
{
    for (object = XtParent(object); object && !XtIsWidget(object);)
	object = XtParent(object);

    if (object == NULL) {
	String params = XtName(object);
	Cardinal num_params = 1;
	XtErrorMsg("noWidgetAncestor", "windowedAncestor", "XtToolkitError",
		   "Object \"%s\" does not have windowed ancestor",
		   &params, &num_params);
    }

    return object;
}

#undef XtParent
Widget XtParent(widget)
	Widget widget;
{
	return widget->core.parent;
}

#undef XtName
String XtName(object)
     Widget object;
{
    return XrmQuarkToString(object->core.xrm_name);
}


Boolean XtIsObject(object)
    Widget object;
{
    WidgetClass wc;
    /* perform basic sanity checks */
    if (object->core.self != object) return False;

    wc = object->core.widget_class;
    if (wc->core_class.class_name
	!= XrmClassToString(wc->core_class.xrm_class))
	    return False;

    if (XtIsWidget(object)) {
	if (object->core.name != XrmNameToString(object->core.xrm_name))
	    return False;
    }
    return True;
}


String XtResolvePathname(dpy, type, filename, suffix, path, predicate)
    Display *dpy;
    String type, filename, suffix, path;
    XtFilePredicate predicate;
{
    XtPerDisplay pd = _XtGetPerDisplay(dpy);
    char resolved_name[MAXPATHLEN];
    String class = XrmQuarkToString(pd->class);

    /* stub routine for now */
    if (type != NULL && strcmp(type, "app-defaults") == 0
	  && filename == NULL
	  && suffix == NULL
	  && path == NULL
	  && predicate == NULL) {
	strcpy(resolved_name, "/usr/lib/X11/app-defaults/");
    } else if (type == NULL
	  && filename == NULL
	  && suffix == NULL
	  && predicate == NULL) {
	extern char *getenv();
	char	*dirname;
	if ((dirname = getenv("XAPPLRESDIR")) == NULL) {
	    if ((dirname = getenv("HOME")) == NULL)
		dirname = "/";
	}
	strcpy(resolved_name, dirname);
    } else {
	XtWarning( "XtResolvePathname is only a stub; returning filename" );
	return XtNewString(filename);
    }
    (void) strcat(resolved_name, class);
    return XtNewString(resolved_name);
}
