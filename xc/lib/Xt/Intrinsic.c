#ifndef lint
static char rcsid[] = "$Header: Intrinsic.c,v 1.96 87/12/21 10:50:01 swick Locked $";
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
 * 
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */
#include "Intrinsic.h"
#include "Resource.h"
#include "Shell.h"
#include "ShellP.h"
#include "TM.h"
#include "Atoms.h"

extern char *strcpy();
/******************************************************************
 *
 * Core Resources
 *
 ******************************************************************/

static XtResource resources[] = {
/*    {XtNscreen,XtCScreen,XtRPointer,sizeof(int),
      XtOffset(Widget,core.screen),XtRString,"0"}, 
    {XtNcolormap,XtCColormap,XtRPointer,sizeof(Colormap),
      XtOffset(Widget,core.colormap),XtRString,"0"},*/
    {XtNancestorSensitive,XtCSensitive,XrmRBoolean, sizeof(Boolean),
         XtOffset(Widget,core.ancestor_sensitive),XtRString,"TRUE"},
    {XtNx, XtCPosition, XrmRInt, sizeof(int),
         XtOffset(Widget,core.x), XtRString, "0"},
    {XtNy, XtCPosition, XrmRInt, sizeof(int),
         XtOffset(Widget,core.y), XtRString, "0"},
    {XtNwidth, XtCWidth, XrmRInt, sizeof(int),
         XtOffset(Widget,core.width), XtRString, "0"},
    {XtNheight, XtCHeight, XrmRInt, sizeof(int),
         XtOffset(Widget,core.height), XtRString, "0"},
/* default depth should be "InheritFromParent", and we should have a */
/* string to depth type converter, but it needs the widget to be able */
/* to find the parent's depth... right now, we kludge it and check for */
/* a depth of 0 in XtCreateWidget. Gross. */
    {XtNdepth, XtCDepth,XrmRInt,sizeof(int),
         XtOffset(Widget,core.depth), XtRString, "0"},
    {XtNbackground,XtCBackground,XrmRPixel,sizeof(Pixel),
         XtOffset(Widget,core.background_pixel), XtRString, "White"},
    {XtNborderWidth, XtCBorderWidth,XrmRInt, sizeof(int),
         XtOffset(Widget,core.border_width),XtRString, "1"},
    {XtNborder,XtCBorderColor,XrmRPixel,sizeof(Pixel),
         XtOffset(Widget,core.border_pixel),XtRString, "Black"},
    {XtNsensitive,XtCSensitive,XrmRBoolean, sizeof(Boolean),
         XtOffset(Widget,core.sensitive),XtRString,"TRUE"},
    {XtNmappedWhenManaged,XtCMappedWhenManaged,XrmRBoolean, sizeof(Boolean),
         XtOffset(Widget,core.mapped_when_managed),XtRString,"TRUE"},
    {XtNdestroyCallback,XtCCallback,XtRPointer,sizeof(caddr_t),
         XtOffset(Widget,core.destroy_callbacks), XtRPointer, (caddr_t)NULL}
    };
static void CoreDestroy();
static void CompileCallbackList();
static void RecurseInitialize();
static void RecurseConstraintInitialize();
static void SetAncestorSensitive();
static Boolean CoreSetValues ();
static void CoreRealize ();
static void CoreClassInitialize();
static void CoreInitialize();

XrmResourceDataBase XtDefaultDB = NULL;
WidgetClass widgetClass = &widgetClassRec;
CompositeWidgetClass compositeWidgetClass = &compositeClassRec;
ConstraintWidgetClass constraintWidgetClass = &constraintClassRec;

/* ||| Should have defaults for Inherit from superclass to work */

WidgetClassRec widgetClassRec = {
         (WidgetClass)NULL,	/*superclass pointer*/
         "Core",		/*class_name*/
          sizeof(WidgetRec),	/*size of core data record*/
	  (XtProc)CoreClassInitialize,	/* class initializer routine */
	  FALSE,		/* not init'ed */
          (XtWidgetProc)CoreInitialize,	/* Instance Initializer routine*/
          CoreRealize,		/*Realize*/
          NULL,			/*actions*/
          0,			/*number of actions in translation table*/
          resources,		/*resource list*/
          XtNumber(resources),	/*resource_count*/
          NULLQUARK,		/*xrm_class*/
          FALSE,		/*compress motion*/
          TRUE,			/*compress exposure*/
          FALSE,		/*visible_interest*/
          CoreDestroy,		/*destroy proc*/
          (XtWidgetProc) NULL,	/*resize*/
          (XtExposeProc) NULL,	/*expose*/
          CoreSetValues,	/*set_values*/
          (XtWidgetProc)NULL,	/*accept_focus*/
           NULL,                /*callback offset list*/
           NULL                 /*reserved*/
};

static void CompositeInsertChild();
static void CompositeDeleteChild();
static void CompositeDestroy();

ConstraintClassRec constraintClassRec = {
    {
	 (WidgetClass)&compositeClassRec,	/*superclass pointer*/
	 "Constraint",		/*class_name*/
	  sizeof(ConstraintRec),/*size of core data record*/
	  (XtWidgetProc)NULL,	/* class initializer routine */
	  FALSE,		/* not init'ed */
	  (XtWidgetProc)NULL,	/* Instance Initializer routine*/
	  XtInheritRealize,	/*Realize*/
	  NULL,			/*actions*/
	  0,			/*number of actions*/
	  NULL,			/*resource list*/
	  0,			/*resource_count*/
	  NULLQUARK,		/*xrm_class*/
	  FALSE,		/*compress motion*/
	  TRUE,			/*compress expose*/
	  FALSE,		/*visible_interest*/
	  (XtWidgetProc)NULL,	/*destroy proc*/
	  (XtWidgetProc)NULL,	/*resize*/
	  (XtExposeProc)NULL,	/*expose*/
	  NULL,			/*set_values*/
	  (XtWidgetProc)NULL,	/*accept_focus*/
	  NULL,			/*callback offsets*/
	  NULL,			/*reserved*/
    },{
	(XtGeometryHandler) NULL,	/* geometry_manager */
	(XtWidgetProc) NULL,
	XtInheritInsertChild,
	XtInheritDeleteChild,
	(XtWidgetProc) NULL,
	(XtWidgetProc) NULL,
    },{
	NULL,			/* constraint resources           */
	0,			/* number of constraint resources */
        0,			/* size of instance constraint rec*/
        NULL,			/* constraint init proc           */
        NULL,			/* constraint destroy proc        */
        NULL			/* constraint set_values proc     */

    }
};

static void CompositeInitialize();

CompositeClassRec compositeClassRec = {
    {
	 (WidgetClass)&widgetClassRec,	/*superclass pointer*/
	 "Composite",		/*class_name*/
	  sizeof(CompositeRec), /*size of core data record*/
	  (XtWidgetProc)NULL,	/* class initializer routine */
	  FALSE,		/* not init'ed */
	  (XtInitProc)CompositeInitialize,  /* Instance Initializer routine */
	  XtInheritRealize,	/*Realize*/
	  NULL,			/*actions*/
	  0,			/*number of actions*/
	  NULL,			/*resource list*/
	  0,			/*resource_count*/
	  NULLQUARK,		/*xrm_class*/
	  FALSE,		/*compress motion*/
	  TRUE,			/*compress expose*/
	  FALSE,		/*visible_interest*/
	  (XtWidgetProc)CompositeDestroy,	/*destroy proc*/
	  (XtWidgetProc)NULL,	/*resize*/
	  (XtExposeProc)NULL,	/*expose*/
	  NULL,			/*set_values*/
	  (XtWidgetProc)NULL,	/*accept_focus*/
	  NULL,			/*callback offsets*/
	  NULL,			/*reserved*/
    },{
	(XtGeometryHandler) NULL,	/* geometry_manager */
	(XtWidgetProc) NULL,
	CompositeInsertChild,
	CompositeDeleteChild,
	(XtWidgetProc) NULL,
	(XtWidgetProc) NULL,
    }
};

typedef struct _CallbackRec {
    CallbackList  next;
    Widget	    widget;
    XtCallbackProc  callback;
    Opaque	    closure;
} CallbackRec;

typedef struct _XtOffsetRec {
    _XtOffsetList   next;
     XrmQuark       name;
     int	    offset;
} XtOffsetRec;

static void ConstructCallbackOffsets(widgetClass)
    WidgetClass widgetClass;
{
    Cardinal i;
    XrmResourceList resourceList;
    _XtOffsetList newItem;
    XrmQuark xtQCallback = XrmAtomToQuark(XtCCallback);

    if (widgetClass->core_class.superclass !=NULL)
         widgetClass->core_class.callback_private = 
         widgetClass->core_class.superclass->core_class.callback_private;
    for (i=widgetClass->core_class.num_resources,
	resourceList=(XrmResourceList)widgetClass->core_class.resources;
         i!=0; i--)
     if (resourceList[i-1].xrm_class == xtQCallback) {
         newItem = XtNew(XtOffsetRec);
         newItem->next = widgetClass->core_class.callback_private;
         newItem->offset = resourceList[i-1].xrm_offset;
         newItem->name   =  resourceList[i-1].xrm_name;
         widgetClass->core_class.callback_private=newItem;
     }
}

static void ClassInit(widgetClass)
    WidgetClass widgetClass;
{

    if ((widgetClass->core_class.superclass != NULL) 
         && (!(widgetClass->core_class.superclass-> core_class.class_inited)))
 	ClassInit(widgetClass->core_class.superclass);
    if (widgetClass->core_class.resources != NULL)
             XtCompileResourceList(widgetClass->core_class.resources,
               widgetClass->core_class.num_resources);
    ConstructCallbackOffsets(widgetClass);
    if (widgetClass->core_class.class_initialize != NULL)
       (*(widgetClass->core_class.class_initialize))();
    widgetClass->core_class.class_inited = TRUE;
    return;
}

static Cardinal InsertAtEnd( w )
    CompositeWidget w;
{
    return w->composite.num_children;
}

static void CompositeDestroy(w)
    CompositeWidget	w;
{
    XtFree((char *) w->composite.children);
}

/* ARGSUSED */
static void CompositeInsertChild(w, args, num_argsP)
    Widget	w;
    ArgList	args;
    Cardinal	*num_argsP;
{
    Cardinal	    num_args = *num_argsP;
    Cardinal	    position;
    Cardinal	    i;
    CompositeWidget cw;

    cw = (CompositeWidget) w->core.parent;

    position = (*cw->composite.insert_position)(cw);

    /* ||| Some better allocation, don't realloc every time ! */
    cw->composite.children = 
        (WidgetList) XtRealloc((caddr_t) cw->composite.children,
    	(unsigned) (cw->composite.num_children + 1) * sizeof(Widget));
    /* Ripple children up one space from "position" */
    for (i = cw->composite.num_children; i > position; i--) {
        cw->composite.children[i] = cw->composite.children[i-1];
    }
    cw->composite.children[position] = w;
    cw->composite.num_children++;
}

static void CompositeDeleteChild(w)
    Widget	w;
{
    Cardinal	    position;
    Cardinal	    i;
    CompositeWidget cw;

    cw = (CompositeWidget) w->core.parent;

    for (position = 0; position < cw->composite.num_children; position++) {
        if (cw->composite.children[position] == w) {
	    break;
	}
    }

    /* Ripple children down one space from "position" */
    cw->composite.num_children--;
    for (i = position; i < cw->composite.num_children; i++) {
        cw->composite.children[i] = cw->composite.children[i+1];
    }
}
static void RecurseInitialize (reqWidget, newWidget, args, num_args, class)
    Widget reqWidget;
    Widget newWidget;
    ArgList args;
    Cardinal num_args;
    WidgetClass class;
{
    if (class->core_class.superclass)
        RecurseInitialize (reqWidget, newWidget, args, num_args,
           class->core_class.superclass);
    if (class->core_class.initialize!=NULL)
        (*class->core_class.initialize)(reqWidget, newWidget, args, &num_args);
}

static void RecurseConstraintInitialize (reqWidget, newWidget, args, num_args, class)
    Widget reqWidget;
    Widget newWidget;
    ArgList args;
    Cardinal num_args;
    WidgetClass class;
{
    if (class->core_class.superclass != (WidgetClass)constraintWidgetClass)
        RecurseConstraintInitialize (reqWidget, newWidget, args, num_args,
           class->core_class.superclass);
    if (((ConstraintWidgetClass)class)->constraint_class.initialize)
        (*((ConstraintWidgetClass)class)->constraint_class.initialize)
	    (reqWidget, newWidget, args, &num_args);
}

static void CoreInitialize(reqWidget,newWidget,args,num_args)
    Widget   reqWidget,newWidget;
    ArgList  args;
    Cardinal *num_args;
{
    newWidget->core.window = (Window) NULL;
    newWidget->core.managed = FALSE;
    newWidget->core.visible = TRUE;
    newWidget->core.background_pixmap = (Pixmap) NULL;
    newWidget->core.border_pixmap = (Pixmap) NULL;
    newWidget->core.event_table = NULL;
    newWidget->core.popup_list = NULL;
    newWidget->core.num_popups = 0;

/* verify valid screen ?*/

/* check for valid depth and colormap for this screen */


}

/* ARGSUSED */
static void CompositeInitialize(reqWidget, newWidget, args, num_args)
    Widget   reqWidget, newWidget;
    ArgList  args;
    Cardinal *num_args;
{
    CompositeWidget w = (CompositeWidget) newWidget;

    w->composite.num_children = 0;
    w->composite.num_mapped_children = 0;
    w->composite.children = NULL;
    w->composite.num_slots = 0;
    w->composite.insert_position = (XtOrderProc)InsertAtEnd;
}

/* we should be able to merge _XtCreate1 and _XtCreate2 with
   changes to default resource management */
Widget _XtCreate1(name,widgetClass,parent)
    char        *name;
    WidgetClass widgetClass;
    Widget      parent;

{
    Widget widget;

    if (! (widgetClass->core_class.class_inited))
	ClassInit(widgetClass);
    widget = (Widget) XtMalloc((unsigned)widgetClass->core_class.widget_size);
    widget->core.self = widget;
    if (name != NULL && *name != '\0')
      widget->core.name = strcpy(XtMalloc((unsigned)strlen(name)+1), name);
    else			/* all widgets should have a name */
      widget->core.name = "";	/* ...but save space by not duplicating this */
    widget->core.parent = parent;
    widget->core.widget_class = widgetClass;
    widget->core.translations = NULL;
    widget->core.constraints = NULL;
    widget->core.being_destroyed =
	(parent != NULL ? parent->core.being_destroyed : FALSE);
/*||| garbage - set up default - need Inherit in Resource List */
    widget->core.screen = DefaultScreenOfDisplay(toplevelDisplay);
/*||| hack for setting colormap until issues resolved |||*/
    widget->core.colormap = 
             DefaultColormapOfScreen(XtScreen(widget));

    return(widget);
}
static void _XtCreate2(widget,args,num_args)
    Widget      widget;
    ArgList     args;
    Cardinal    num_args;
{
    Widget reqWidget;
    WidgetClass wClass = widget->core.widget_class;
    _XtOffsetList offsetList;

    for (offsetList = widget->core.widget_class->core_class.callback_private;
         offsetList != NULL;
	 offsetList = offsetList->next) {
	if (*(XtCallbackList*)((int)widget - offsetList->offset - 1) != NULL) {
	    CompileCallbackList(widget,((int)widget - offsetList->offset - 1));
	}
    }

    reqWidget = (Widget) XtMalloc(wClass->core_class.widget_size);
    bcopy ((char *) widget, (char *) reqWidget,
                   (unsigned)wClass->core_class.widget_size);
    RecurseInitialize (reqWidget, widget, args, num_args, wClass);

    if ((widget->core.parent != (Widget)NULL) &&
	XtIsSubclass(widget->core.parent, (WidgetClass)constraintWidgetClass)) 
       RecurseConstraintInitialize(reqWidget, widget, args, num_args,
                       widget->core.parent->core.widget_class);

    XtFree ((char *) reqWidget);

    _XtDefineTranslation(widget);
}
Widget XtCreateWidget(name,widgetClass,parent,args,num_args)
    char        *name;
    WidgetClass widgetClass;
    Widget      parent;
    ArgList     args;
   Cardinal    num_args;
{
    Widget    widget,reqWidget;
    unsigned long widget_size;

/*||| this will go away with later changes to resource management|||*/
   widget =  _XtCreate1(name,widgetClass,parent);
   if (XtIsSubclass(widget->core.parent, (WidgetClass)constraintWidgetClass)) {
   ConstraintWidgetClass cwc =(ConstraintWidgetClass) widget->core.parent->core.widget_class;
     widget->core.constraints = 
       (caddr_t)XtMalloc ((unsigned)cwc->constraint_class.constraint_size); 
   }
	XtGetResources(widget,args,num_args);
    if (widget->core.depth == 0)
        widget->core.depth = widget->core.parent->core.depth;
    if (widget->core.screen == NULL)
        widget->core.screen = widget->core.parent->core.screen;
    widget->core.ancestor_sensitive = 
         (widget->core.parent->core.ancestor_sensitive && 
          widget->core.parent->core.sensitive);
     _XtCreate2(widget,args,num_args);

   (*(((CompositeWidgetClass)(widget->core.parent->core.widget_class))
        ->composite_class.insert_child))(widget, args, &num_args);

    return (widget);
}

Widget XtCreateManagedWidget(name,widgetClass,parent,args,num_args)
    char        *name;
    WidgetClass widgetClass;
    Widget      parent;
    ArgList     args;
    Cardinal    num_args;
{
    Widget widget =

    XtCreateWidget( name, widgetClass, parent, args, num_args );
    XtManageChild( widget );

    return widget;
}


Widget XtCreatePopupShell(name, widgetClass, parent, args, num_args)
    char *name;
    WidgetClass widgetClass;
    Widget parent;
    ArgList args;
    Cardinal num_args;
{
        Widget widget;

         if(parent == NULL || widgetClass == NULL) {
                XtError("invalid parameters to XtCreatePopupChild"); /* XXX */
                return NULL;
        }
    widget =  _XtCreate1(name,widgetClass,parent);
     XtGetResources(widget,args,num_args);
      if (widget->core.depth == 0)
        widget->core.depth = DefaultDepthOfScreen(XtScreen(widget));
   widget->core.ancestor_sensitive =
         (widget->core.parent->core.ancestor_sensitive &
          widget->core.parent->core.sensitive);
     _XtCreate2(widget,args,num_args);
      parent->core.popup_list =
      (WidgetList) XtRealloc((caddr_t) parent->core.popup_list,
               (unsigned) (parent->core.num_popups + 1)*sizeof(Widget));
        parent->core.popup_list[parent->core.num_popups ++] = widget;

        return(widget);
}
Widget XtCreateApplicationShell(name, widgetClass,  args, num_args)
    char *name;
    WidgetClass widgetClass;
    ArgList args;
    Cardinal num_args;
{
        Widget widget;


    widget =  _XtCreate1(name,widgetClass,NULL);
      XtGetResources(widget,args,num_args);
      if (widget->core.depth == 0)
        widget->core.depth = DefaultDepthOfScreen(XtScreen(widget));
     widget->core.ancestor_sensitive = TRUE;
     _XtCreate2(widget,args,num_args);
    return(widget);
}





static void FillInParameters(widget,valuemask,values)
    Widget  widget;
    XtValueMask *valuemask;
    XSetWindowAttributes *values;
{
    *valuemask = (CWBackPixel | CWBorderPixel | CWEventMask);
    (*values).event_mask = _XtBuildEventMask(widget);
    (*values).background_pixel = widget->core.background_pixel;
    (*values).border_pixel = widget->core.border_pixel;
    if (widget->core.widget_class->core_class.expose == (XtExposeProc) NULL) {
	/* Try to avoid redisplay upon resize by making bit_gravity the same
	   as the default win_gravity */
	*valuemask |= CWBitGravity;
	(*values).bit_gravity = NorthWestGravity;
    }
    return;
}

Boolean XtIsRealized (widget)
    Widget   widget;

{
    return (widget->core.window != NULL);
}


Boolean XtIsSubclass(widget, widgetClass)
    Widget    widget;
    WidgetClass widgetClass;
{
  WidgetClass w;

  for(w=widget->core.widget_class; w != NULL; w = w->core_class.superclass)
        if (w == widgetClass) return (TRUE);

  return (FALSE);
}

void XtRealizeWidget (widget)
    Widget   widget;
{
    CompositeWidget cwidget;
    XtValueMask valuemask;
    XSetWindowAttributes values;
    Cardinal left_to_map, i;

    if (XtIsRealized (widget)) return;

    _XtBindActions(widget, widget->core.translations);
    _XtInstallTranslations(widget, widget->core.translations);

    FillInParameters (widget, &valuemask, &values);
    (*(widget->core.widget_class->core_class.realize)) (
	    widget, &valuemask, &values);

    _XtRegisterGrabs(widget);

    _XtRegisterWindow (widget->core.window, widget);

    if (XtIsComposite (widget)) {
	/* Map its children that are managed and mapped_when_managed */
	cwidget = (CompositeWidget) widget;
	for (i = cwidget->composite.num_children; i != 0; --i)
	    XtRealizeWidget (cwidget->composite.children[i-1]);
	if (cwidget->composite.num_children
		== cwidget->composite.num_mapped_children)
	    XMapSubwindows (XtDisplay (widget), XtWindow (widget));
	else
	    for (i = 0, left_to_map = cwidget->composite.num_mapped_children;
                 left_to_map != 0;
                 i++) {
		if (cwidget->composite.children[i]->core.managed &&
		    cwidget->composite.children[i]->core.mapped_when_managed) {
		    XtMapWidget (cwidget->composite.children[i]);
		    left_to_map--;
		}
	    }
    }

    /* If this is the application's popup shell, map it */
    if (widget->core.parent == NULL) XtMapWidget (widget);

    return;
}

void XtCreateWindow(widget, windowClass, visual, valueMask, attributes)
    Widget widget;
    unsigned int windowClass;
    Visual *visual;
    Mask valueMask;
    XSetWindowAttributes *attributes;
{

    if (widget->core.window == None) {
	widget->core.window =
	    XCreateWindow (
		XtDisplay (widget),
		(widget->core.parent ?
		    widget->core.parent->core.window :
		    widget->core.screen->root),
		widget->core.x, widget->core.y,
		widget->core.width, widget->core.height,
		widget->core.border_width, (int) widget->core.depth,
		windowClass, visual, valueMask, attributes);
    }
}

	
static void CoreClassInitialize()
{
      
}
    
static void CoreRealize(widget, valueMaskP, attributes)
    register Widget widget;
    Mask *valueMaskP;
    XSetWindowAttributes *attributes;
{
    Mask valueMask = *valueMaskP;
  XtCreateWindow(widget, (unsigned int) InputOutput, (Visual *) CopyFromParent,
		 valueMask, attributes);
}

void XtUnmanageChildren(children, num_children)
    WidgetList children;
    Cardinal num_children;
{

    CompositeWidget	parent;
    register Widget	child;
    Cardinal		num_unique_children, i;

    if (num_children == 0) return;
    parent = (CompositeWidget) children[0]->core.parent;
    if (parent->core.being_destroyed) return;

    num_unique_children = 0;
    for (i = 0; i < num_children; i++) {
	child = children[i];
        if ((CompositeWidget) child->core.parent != parent) {
	    XtWarning("Not all children have same parent in XtUnmanageChildren");
	} else if ((! child->core.managed) || (child->core.being_destroyed)) {
	    /* Do nothing */
	} else {
	    if (child->core.mapped_when_managed) {
		if (XtIsRealized(child)) {
		    XtUnmapWidget(child);
		}
		num_unique_children++;
	    }
	    child->core.managed = FALSE;
	}
    }
    parent->composite.num_mapped_children =
    	parent->composite.num_mapped_children-num_unique_children;

    (*(((CompositeWidgetClass)parent->core.widget_class)
        ->composite_class.change_managed))(parent);
}


void XtUnmanageChild(child)
    Widget child;
{
    XtUnmanageChildren(&child, 1);
}

void XtManageChildren(children, num_children)
    WidgetList children;
    Cardinal num_children;
{

    CompositeWidget	parent;
    register Widget	child;
    Cardinal		num_unique_children, i;

    if (num_children == 0) return;
    parent = (CompositeWidget) children[0]->core.parent;
    if (parent->core.being_destroyed) return;

    num_unique_children = 0;
    for (i = 0; i < num_children; i++) {
	child = children[i];
        if ((CompositeWidget) child->core.parent != parent) {
	    XtWarning("Not all children have same parent in XtManageChildren");
	} else if ((child->core.managed) || (child->core.being_destroyed)) {
	    /* Do nothing */
	} else {
	    if (XtIsRealized(child->core.parent) && ! XtIsRealized(child))
		XtRealizeWidget(child);
	    if (child->core.mapped_when_managed) {
		if (XtIsRealized(child)) {
		    /* ||| Should really do mapping after change_managed */
		    XtMapWidget(child);
		}
		num_unique_children++;
	    }
	    child->core.managed = TRUE;
	}
    }
    parent->composite.num_mapped_children =
    	parent->composite.num_mapped_children + num_unique_children;

    (*(((CompositeWidgetClass)parent->core.widget_class)
        ->composite_class.change_managed))(parent);
}

void XtManageChild(child)
    Widget child;
{

    XtManageChildren(&child, (Cardinal) 1);
}

void XtSetMappedWhenManaged(widget, mappedWhenManaged)
    Widget    widget;
    Boolean   mappedWhenManaged;
{

    if (widget->core.mapped_when_managed == mappedWhenManaged) return;
    if (mappedWhenManaged) {
	/* we didn't used to be mapped when managed. If we are realized and */
        /* managed then map us, increment parent's count of mapped children */
     if (XtIsRealized(widget) && XtIsManaged(widget)) {
       XtMapWidget(widget);
      ((CompositeWidget) (widget->core.parent))
                        ->composite.num_mapped_children++;
     }
    } else {
	/* we used to be mapped when managed. If we were realized and */
	/* managed then unmap us, decrement parent's mapped children count */
       if (XtIsRealized(widget) && XtIsManaged(widget)) {
         XtUnmapWidget(widget);
         ((CompositeWidget) (widget->core.parent))
                        ->composite.num_mapped_children--;
     }
    }
    widget->core.mapped_when_managed = mappedWhenManaged;
}


void XtSetSensitive(widget,sensitive)
    Widget    widget;
    Boolean   sensitive;
{
    int i;
    Arg al[2];

    XtSetArg(al[0],XtNsensitive,sensitive);
    XtSetValues(widget,al,1);
    if( XtIsComposite (widget))
      for (i= ((CompositeWidget)widget)->composite.num_children;i != 0; --i)
       SetAncestorSensitive (
        ((CompositeWidget)widget)->composite.children[i-1],
        (widget->core.sensitive & widget->core.ancestor_sensitive));
      
}

static void SetAncestorSensitive(widget,sensitive)
    Widget    widget;
    Boolean   sensitive;
{
    int i;
    Arg al[2];
    XtSetArg(al[0],XtNancestorSensitive,sensitive);
    XtSetValues(widget,al,1);
     if ((widget->core.sensitive == widget->core.ancestor_sensitive)
                                 && XtIsComposite (widget))
      for (i= ((CompositeWidget)widget)->composite.num_children;i != 0; --i)
       SetAncestorSensitive (
         ((CompositeWidget)widget)->composite.children[i-1],sensitive);
}


static CallbackList *FetchCallbackList (widget,name)
    Widget  widget;
    String  name;
{
    _XtOffsetList offsetList;
    CallbackList* mumble;
    XrmQuark quark;
    char* i;
    quark = StringToQuark(name);
    if (quark == NULL) return (NULL);
    for (
    offsetList = widget->core.widget_class->core_class.callback_private;
    offsetList != NULL; offsetList = offsetList->next) {
      if (quark == offsetList ->name){
        i = (char*)((int)widget - offsetList->offset - 1);
     return((CallbackList*)i);
      }
    }
    return(NULL);

}


static void AddCallback (widget,callbackList,callback,closure)
    Widget widget;
    CallbackList *callbackList;
    XtCallbackProc callback;
    Opaque closure;
{

     CallbackRec *c,*cl;
     c =  XtNew(CallbackRec);
     c->next = NULL;
     c->widget = widget;
     c->closure = closure;
     c->callback = callback; 
    if (*callbackList == NULL){
             (*callbackList) = c;
             return;
    }
    for (cl = (*callbackList); cl->next != NULL; cl = cl->next) {}
    cl->next = c;
    return;
}

void XtAddCallback(widget,name,callback,closure)
    Widget    widget;
    String    name;
    XtCallbackProc callback;
    Opaque      closure;
{

    CallbackList *callbackList;
    callbackList = FetchCallbackList(widget,name);
    if (callbackList == NULL) {
       XtError("invalid parameters to XtAddCallback");
       return;
    }
    AddCallback(widget,callbackList,callback,closure);
    return;
}

void RemoveCallback (widget, callbackList, callback, closure)
    Widget  widget;
    CallbackList *callbackList;
    XtCallbackProc callback;
    Opaque closure;

{
   register CallbackList cl;

    for (cl = *callbackList; cl != NULL; (cl = *(callbackList = &cl->next))) {
	if (( cl->widget == widget) && (cl->closure == closure)
                            && (cl->callback == callback) ) {
	    *callbackList = cl->next;
	    XtFree ((char *)cl);
	    return;
	}
    }
}

void XtRemoveCallback (widget, name, callback, closure)
    Widget    widget;
    String    name;
    XtCallbackProc callback;
    Opaque      closure;
{

   CallbackList *callbackList;
   callbackList = FetchCallbackList(widget,name);
   if (callbackList == NULL) {
      XtError("invalid parameters to XtRemoveCallback");
      return;
   }
   RemoveCallback(widget,callbackList,callback,closure);
}


void _XtRemoveAllCallbacks (callbackList)
    CallbackList *callbackList;

{
   CallbackList cl, tcl;

   cl = *callbackList;
   while (cl != NULL) {
	tcl = cl;
	cl = cl->next;
	XtFree((char *) tcl);
   }

   (*callbackList) = NULL;
}

void XtRemoveAllCallbacks(widget,name)
    Widget widget;
    String name;
{

   CallbackList *callbackList;
   callbackList = FetchCallbackList(widget,name);
   if (callbackList == NULL) {
      XtError("invalid parameters to XtRemoveAllCallbacks");
     return;
   }
   _XtRemoveAllCallbacks(callbackList);
   return;
}


#ifdef notdef
void CallCallbacks (callbacks,call_data)
    CallbackList *callbacks;
    Opaque call_data;
{
    CallbackRec *cl;
    if ((*callbacks) == NULL )return;
    for (cl = (*callbacks); cl != NULL; cl = cl->next) 
             (*(cl->callback))(cl->widget,cl->closure,call_data);
}
#else
/*  
 *  Code which replace the CallCallbacks routine.
 *  This code supports modifying a callback list during the execution
 *  of a callback by making a copy of the list first.
 */


#define CALLBACK_CACHE_SIZE	10	/* defines the number of callbacks */
					/* that are kept in a stack variable */
					/* if the number exceeds this a */
					/* temporary record is malloced */


void _XtCallCallbacks (callbacks, call_data)
    CallbackList *callbacks;
    Opaque call_data;
{
    CallbackRec *cl;
    CallbackRec stack_cache [CALLBACK_CACHE_SIZE];
    CallbackRec *scl;
    CallbackRec *dcl;
    CallbackRec *alloc_start = NULL;
    CallbackRec **prevnext = NULL;
    int i = 0;

    if ((*callbacks) == NULL )return;

/*
 * copy callback list.
 * If the number of entries > max, allocate temporary records
 */

    for (scl = (*callbacks), dcl = &stack_cache[0];
	 scl != NULL; 
	 scl = scl->next, *dcl++) 
    {
	if (i >= CALLBACK_CACHE_SIZE)
	{
	    dcl = XtNew(CallbackRec);
	    if (i == CALLBACK_CACHE_SIZE)
		alloc_start = dcl;
	}
	*dcl = *scl;
	if (prevnext != NULL)
	    *prevnext = dcl;
	prevnext = &dcl->next;
	i++;
    }

    /*
     * now execute each call back
     */
    for (cl = &stack_cache[0]; cl != NULL; cl = cl->next) 
             (*(cl->callback))(cl->widget,cl->closure,call_data);

    /*
     * If any temporary records were allocated, free them
     */
    if (alloc_start != NULL)
   	_XtRemoveAllCallbacks(&alloc_start);
}
#endif


#ifdef ndef
/* Untested replacement for Leo's old code */
void CallCallbacks (callbacks, call_data)
    CallbackList *callbacks;
    Opaque call_data;
{
    register CallbackRec *cl;
    CallbackRec		 stack_cache [CALLBACK_CACHE_SIZE];
    CallbackList	 head;
    register Cardinal    i;

    if ((*callbacks) == NULL ) return;

/*
 * copy callback list.
 * If the number of entries > max, allocate an array, otherwise use stack_cache
 */

    for (i = 0, cl = *callbacks; cl != NULL; i++, cl = cl->next) {};
    if (i > CALLBACK_CACHE_SIZE) {
	head = (CallbackList) XtMalloc((unsigned) (i * sizeof(CallbackRec)));
    } else {
	head = stack_cache;
    }
    for (i = 0, cl = *callbacks; cl != NULL; i++, cl = cl->next) {
	head[i] = *cl;
    }

    /*
     * now execute each call back
     */

    for (cl = head; i != 0; cl++, i--) {
	(*(cl->callback)) (cl->widget, cl->closure, call_data);
    }

    /*
     * If temporary array allocated, free it
     */
    if (head != stack_cache)
	XtFree(head);
}
#endif


static void CompileCallbackList(widget, pList)
    Widget	    widget;
    XtCallbackList  *pList;
{
    /* Turn a public XtCallbackList into a private CallbackList */

    XtCallbackList  xtList;
    CallbackList    head, new, *pLast;

    pLast = &head;
    for (xtList= *pList; xtList->callback != NULL; xtList++) {
	new		= XtNew(CallbackRec);
	*pLast		= new;
	pLast		= &(new->next);
	new->widget     = widget;
	new->callback   = xtList->callback;
	new->closure    = (Opaque) xtList->closure;
    };
    *pLast = NULL;

    *pList = (XtCallbackList) head;
}

void XtCallCallbacks (widget, name, call_data)
    Widget   widget;
    String   name;
    Opaque  call_data;
{
   CallbackList *callbacks;

   callbacks = FetchCallbackList(widget, name);
   if (callbacks == NULL) {
     XtWarning("Cannot find callback list in XtCallCallbacks");
     return;
   }
   _XtCallCallbacks(callbacks, call_data);
}

static void OverrideCallback (callbackList, callback)
    CallbackList *callbackList;
    XtCallbackProc	callback;
{
   CallbackList cl;

   cl = *callbackList;
   while (cl != NULL) { cl->callback = callback; cl = cl->next; }
}

void XtOverrideCallback(widget, callback_name, callback)
     Widget		widget;
     String		callback_name;
     XtCallbackProc	callback;
{
    CallbackList *callbackList;
    callbackList = FetchCallbackList(widget,callback_name);
   if (callbackList == NULL) {
      XtError("invalid parameters to XtOverrideCallback");
     return;
   }
   OverrideCallback(callbackList, callback);
}

extern Boolean XtHasCallbacks(widget, callback_name)
     Widget		widget;
     String		callback_name;
{
    CallbackList *callbackList;
    callbackList = FetchCallbackList(widget,callback_name);
   if (callbackList == NULL) {
      XtError("invalid parameters to XtHasCallbacks");
     return (FALSE);
   }
    return (*callbackList != NULL);
}

/* --------------------- XtDestroy ------------------- */

static void Recursive(widget, proc)
    Widget       widget;
    XtWidgetProc proc;
{
    register int i;
    CompositeWidget cwidget;

    /* Recurse down normal children */
    if (XtIsComposite(widget)) {
	cwidget = (CompositeWidget) widget;
	for (i = 0; i < cwidget->composite.num_children; i++) {
	    Recursive(cwidget->composite.children[i], proc);
	}
    } 

    /* Recurse down popup children */
    for (i = 0; i < widget->core.num_popups; i++) {
	Recursive(widget->core.popup_list[i], proc);
    }

    /* Finally, apply procedure to this widget */
    (*proc) (widget);  
}

static void Phase1Destroy (widget)
    Widget    widget;
{
    widget->core.being_destroyed = TRUE;
}

static void Phase2Callbacks(widget)
    Widget    widget;
{
    _XtCallCallbacks(&(widget->core.destroy_callbacks), (Opaque) NULL);
}

static void Phase2Destroy(widget)
    register Widget widget;
{
    register WidgetClass	    class;
    register ConstraintWidgetClass  cwClass;

    /* Call constraint destroy procedures */
    if (widget->core.parent != NULL && widget->core.constraints != NULL) {
	cwClass = (ConstraintWidgetClass)widget->core.parent->core.widget_class;
	for (;;) {
	    if (cwClass->constraint_class.destroy != NULL)
		(*(cwClass->constraint_class.destroy)) (widget);
            if (cwClass == constraintWidgetClass) break;
            cwClass = (ConstraintWidgetClass) cwClass->core_class.superclass;
	}
    }

    /* Call widget destroy procedures */
    for (class = widget->core.widget_class;
	 class != NULL; 
	 class = class->core_class.superclass) {
	if ((class->core_class.destroy) != NULL)
	    (*(class->core_class.destroy))(widget);
    }
}

/*ARGSUSED*/
static void XtPhase2Destroy (widget, closure, call_data)
    register Widget widget;
    caddr_t	    closure;
    caddr_t	    call_data;
{
    Display	    *display;
    Window	    window;
    XtWidgetProc    delete_child;

    if (widget->core.parent != NULL) {
	XtUnmanageChild(widget);
	delete_child =
	    (((CompositeWidgetClass) widget->core.parent->core.widget_class)
		->composite_class.delete_child);
	if (delete_child == NULL) {
	    XtWarning("NULL delete_child procedure");
	} else {
	    (*delete_child) (widget);
	}
    }
    display = XtDisplay(widget); /* widget is freed in Phase2Destroy */
    window = widget->core.window;
    Recursive(widget, Phase2Callbacks);
    Recursive(widget, Phase2Destroy);
    if (window != NULL) XDestroyWindow(display,window);
}


void XtDestroyWidget (widget)
    Widget    widget;
{
    if (widget->core.being_destroyed) return;
    Recursive(widget, Phase1Destroy);
    AddCallback(widget, &DestroyList, XtPhase2Destroy, (Opaque) NULL);
}


/* ---------------- XtNameToWidget, XtNameToClass ----------------- */

static Widget NameListToWidget(root, names)
    register Widget root;
    XrmNameList     names;
{
    register Cardinal   i;
    register WidgetList children;
    register XrmName    name;

    name = *names;
    if (name == NULLQUARK) return root;
    if (! XtIsComposite(root)) return NULL;

    children = ((CompositeWidget) root)->composite.children;
    for (i = 0; i < ((CompositeWidget) root)->composite.num_children; i++) {
	if (name == children[i]->core.xrm_name)
	    return NameListToWidget(children[i], &names[1]);
    }
    children = root->core.popup_list;
    for (i = 0; i < root->core.num_popups; i++) {
	if (name == children[i]->core.xrm_name)
	    return NameListToWidget(children[i], &names[1]);
    }
    return NULL;
}

Widget XtNameToWidget(root, name)
    Widget root;
    String name;
{
    XrmName	names[100];

    XrmStringToNameList(name, names);
    if (names[0] != root->core.xrm_name) return NULL;
    return NameListToWidget(root, &names[1]);
}


/*
 * the following defs were stolen from TMparse.c and should be in
 * IntrinsicPrivate.h or IntrinsicInternal.h whatever we call it
 */

typedef unsigned int	Value;

typedef struct {
    char	*name;
    XrmQuark	signature;
    Value       value;
} NameValueRec, *NameValueTable;

static void CompileNameValueTable(table)
    register NameValueTable table;
{
    for ( ; table->name != NULL; table++)
        table->signature = StringToQuark(table->name);
}

extern Boolean _XtLookupTableSym(); /* comes from TMparse.c ||| */

#ifdef undef
extern WidgetClassRec buttonBoxClassRec;
extern WidgetClassRec clockClassRec;
extern WidgetClassRec commandClassRec;
extern WidgetClassRec widgetClassRec;
/*
extern WidgetClassRec compositeClassRec;
extern WidgetClassRec constraintClassRec;
*/
extern WidgetClassRec knobClassRec;
extern WidgetClassRec labelClassRec;
extern WidgetClassRec loadClassRec;
extern WidgetClassRec textClassRec;
extern WidgetClassRec paneClassRec;

static NameValueRec classes[] = {
    {"ButtonBox",	NULL,	(Value)&buttonBoxClassRec},
    {"Clock",		NULL,	(Value)&clockClassRec},
    {"Command",		NULL,	(Value)&commandClassRec},
    {"Widget",		NULL,	(Value)&widgetClassRec},
    {"Composite",	NULL,	(Value)&compositeClassRec},
    {"Constraint",	NULL,	(Value)&constraintClassRec},
    {"Knob",		NULL,	(Value)&knobClassRec},
    {"Label",		NULL,	(Value)&labelClassRec},
    {"Load",		NULL,	(Value)&loadClassRec},
    {"Text",		NULL,	(Value)&textClassRec},
    {"Pane",		NULL,	(Value)&paneClassRec},
    {NULL, NULL, NULL},
};
#else
static NameValueRec classes[] = {
    {"Widget",		NULL,	(Value)&widgetClassRec},
    {"Composite",	NULL,	(Value)&compositeClassRec},
    {"Constraint",	NULL,	(Value)&constraintClassRec},
    {NULL, NULL, NULL},
};
#endif

static Boolean classesCompiled = FALSE;

WidgetClass XtNameToClass(name)
    String name;
{
    WidgetClass wc;

    if (! classesCompiled) {
	CompileNameValueTable(classes);
	classesCompiled = TRUE;
    }

    if (_XtLookupTableSym(classes, name, (Value *) &wc)) {
        return wc;
    }

    return NULL;
}


/* ------------------- XtPopup, XtPopdown, etc. ----------------- */

void _XtPopup(widget, grab_kind, spring_loaded)
    Widget      widget;
    XtGrabKind  grab_kind;
    Boolean     spring_loaded;
{
    register ShellWidget shell_widget;

    if (! XtIsSubclass(widget,shellWidgetClass)) {
	XtError("XtPopup requires a subclass of shellWidgetClass");
    }
    shell_widget = (ShellWidget) widget;

    if (! shell_widget->shell.popped_up) {
	shell_widget->shell.popped_up = TRUE;
	shell_widget->shell.grab_kind = grab_kind;
	shell_widget->shell.spring_loaded = spring_loaded;
	if (shell_widget->shell.create_popup_child != NULL) {
	    (*(shell_widget->shell.create_popup_child))((Widget)shell_widget);
	}
	if (grab_kind == XtGrabExclusive) {
	    XtAddGrab((Widget) shell_widget, TRUE, spring_loaded);
	} else if (grab_kind == XtGrabNonexclusive) {
	    XtAddGrab((Widget) shell_widget, FALSE, spring_loaded);
	}
	XtRealizeWidget((Widget) shell_widget);
	XtMapWidget((Widget) shell_widget);
    }
}

void XtPopup (widget,grab_kind)
    Widget  widget;
    XtGrabKind grab_kind;
{
    _XtPopup(widget, grab_kind, FALSE);
}

void XtCallbackNone(widget, closure, call_data)
    Widget  widget;
    caddr_t closure;
    caddr_t call_data;
{
    XtSetSensitive(widget, FALSE);
    _XtPopup((Widget) closure, XtGrabNone, FALSE);
}

void XtCallbackNonexclusive(widget, closure, call_data)
    Widget  widget;
    caddr_t closure;
    caddr_t call_data;
{

    XtSetSensitive(widget, FALSE);
    _XtPopup((Widget) closure, XtGrabNonexclusive, FALSE);
}

void XtCallbackExclusive(widget, closure, call_data)
    Widget  widget;
    caddr_t closure;
    caddr_t call_data;
{
    XtSetSensitive(widget, FALSE);
    _XtPopup((Widget) closure, XtGrabExclusive, FALSE);
}



void XtPopdown(widget)
    Widget  widget;
{
    /* Unmap a shell widget if it is mapped, and remove from grab list */

    register ShellWidget shell_widget;

    if (! XtIsSubclass(widget, shellWidgetClass)) {
	XtError("XtPopdown requires a subclass of shellWidgetClass");
    }
    shell_widget = (ShellWidget) widget;
    if (shell_widget->shell.popped_up) {
	XtUnmapWidget(shell_widget);
	if (shell_widget->shell.grab_kind != XtGrabNone) {
	    XtRemoveGrab(shell_widget);
	}
	shell_widget->shell.popped_up = FALSE;
    }

}

void XtCallbackPopdown(widget, closure, call_data)
    Widget  widget;
    caddr_t closure;
    caddr_t call_data;
{

    register XtPopdownID id = (XtPopdownID) closure;

    XtPopdown(id->shell_widget);
    if (id->enable_widget != NULL) {
	XtSetSensitive(id->enable_widget, TRUE);
    }
}


/* -------------------- Core ------------------- */

/*
 * Start of core class method inheritance routines.
 */


void XtInheritRealize(w, mask, attr)
    register Widget w;
    Mask	    *mask;
    XSetWindowAttributes *attr;
{
    WidgetClass class;

    if (! XtIsSubclass(w, widgetClass)) {
	XtError("Must be a subclass of Core to inherit realize");
    }
    if ((XtSuperclass(w))->core_class.realize) {
  	/* Here we fake runtime casting by changing the class field in  */
	/* the current class record and resetting it when we return.    */
	class = XtClass(w);
	XtClass(w) = XtSuperclass(w);

	(*((XtClass(w))->core_class.realize)) (w, mask, attr);
	XtClass(w) = class;
    }

    if (XtClass(w)->core_class.realize == XtInheritRealize)
      XtClass(w)->core_class.realize = (XtSuperclass(w))->core_class.realize;
}

void XtInheritResize(w)
    register Widget w;
{
    WidgetClass class;

    if (! XtIsSubclass(w, widgetClass)) {
	  XtError("Must be a subclass of Core to inherit resize");
    }
    if ((XtSuperclass(w))->core_class.resize) {
  	/* Here we fake runtime casting by changing the class field in  */
	/* the current class record and resetting it when we return.    */
	class = XtClass(w);
	XtClass(w) = XtSuperclass(w);

	(*((XtClass(w))->core_class.resize)) (w);
	XtClass(w) = class;
    }

    if (XtClass(w)->core_class.resize == XtInheritResize)
      XtClass(w)->core_class.resize = (XtSuperclass(w))->core_class.resize;
}

void XtInheritExpose(w, event)
    register Widget w;
    XEvent	    *event;
{
    WidgetClass class;

    if (! XtIsSubclass(w, widgetClass)) {
	XtError("Must be a subclass of Core to inherit expose");
	}
    if ((XtSuperclass(w))->core_class.expose) {
  	/* Here we fake runtime casting by changing the class field in  */
	/* the current class record and resetting it when we return.    */
	class = XtClass(w);
	XtClass(w) = XtSuperclass(w);

	(*((XtClass(w))->core_class.expose)) (w, event);
	XtClass(w) = class;
    }

    if (XtClass(w)->core_class.expose == XtInheritExpose)
      XtClass(w)->core_class.expose = (XtSuperclass(w))->core_class.expose;
}

void XtInheritAcceptFocus(w)
    Widget w;
{
  XtError("InheritAcceptFocus doesn't work");
}

/* end of Core class method inheritance routines. */

static void CoreDestroy (widget)
    Widget    widget;
{
    register XtEventRec *event, *next;

    if (*widget->core.name != '\0') /* special case; we didn't copy this */
        XtFree((char*)(widget->core.name));
    if (widget->core.background_pixmap != NULL) 
	XFreePixmap(XtDisplay(widget), widget->core.background_pixmap);
    if (widget->core.border_pixmap != NULL)
	XFreePixmap(XtDisplay(widget), widget->core.border_pixmap);
    event = widget->core.event_table;
    while (event != NULL) {
	next = event->next;
	XtFree((char*) event);
	event = next;
    }
    if (widget->core.translations != NULL)
	XtFreeTranslations(widget->core.translations);
    _XtUnregisterWindow(widget->core.window,widget);
    if (widget->core.constraints != NULL)
	XtFree((char*) widget->core.constraints);
    XtFree((char*) widget);
}

static Boolean CoreSetValues()
{
/* ||| */
return (FALSE);

}


/* ------------------- Composite --------------------- */

/*
 * Composite class method inheritance routines.
 */

XtGeometryResult XtInheritGeometryManager(child, request, reply)
    Widget	     child;
    XtWidgetGeometry *request;
    XtWidgetGeometry *reply;
{
    WidgetClass	     class;
    register Widget  parent;
    XtGeometryResult res;

    parent = child->core.parent;
    res = XtGeometryNo;

    if (! XtIsComposite(parent)) {
	XtError("Must be subclass of Composite to inherit geometry_manager");
    }
    if (((CompositeWidgetClass) XtSuperclass(parent))->
	    composite_class.geometry_manager) {
	/* Here we fake runtime casting by changing the class field in  */
	/* the current class record and reseting it when we return.     */
	class = XtClass(parent);
	XtClass(parent) = XtSuperclass(parent);

	res = (*(((CompositeWidgetClass) XtClass(parent))->
	    composite_class.geometry_manager))(child, request, reply);
	XtClass(parent) = class;
    }

    if (((CompositeWidgetClass)XtClass(parent))->
	composite_class.geometry_manager == XtInheritGeometryManager) {
      ((CompositeWidgetClass)(XtClass(parent)))->
	composite_class.geometry_manager =
	  ((CompositeWidgetClass) XtSuperclass(parent))->
	    composite_class.geometry_manager;
    }

    return res;
}

void XtInheritChangeManaged(w)
    register Widget w;
{
    WidgetClass class;

    if (! XtIsComposite(w)) {
	XtError("Must be subclass of Composite to inherit change_managed");
    }
    if (((CompositeWidgetClass) XtSuperclass(w))->
	    composite_class.change_managed) {
	/* Here we fake runtime casting by changing the class field in  */
	/* the current class record and resetting it when we return.    */
	class = XtClass(w);
	XtClass(w) = XtSuperclass(w);

	(*(((CompositeWidgetClass) XtClass(w))->
	    composite_class.change_managed))(w);
	XtClass(w) = class;
    }

    if (((CompositeWidgetClass)XtClass(w))->composite_class.change_managed
	== XtInheritChangeManaged) {
      ((CompositeWidgetClass)(XtClass(w)))->composite_class.change_managed =
	((CompositeWidgetClass) XtSuperclass(w))->
	    composite_class.change_managed;
    }
}

void XtInheritInsertChild(child, args, num_args)
    Widget      child;
    ArgList	args;
    Cardinal	num_args;
{
    WidgetClass     class;
    register Widget parent;

    parent = child->core.parent;

    if (! XtIsComposite(parent)) {
	XtError("Must be subclass of Composite to inherit insert_child");
    }
    if (((CompositeWidgetClass) XtSuperclass(parent))->
	    composite_class.insert_child) {
	/* Here we fake runtime casting by changing the class field in  */
	/* the current class record and resetting it when we return.    */
        class = XtClass(parent);
	XtClass(parent) = XtSuperclass(parent);

	(*(((CompositeWidgetClass) XtClass(parent))->
	    composite_class.insert_child))(child, args, num_args);
	XtClass(parent) = class;
    }
    if (((CompositeWidgetClass)XtClass(parent))->composite_class.insert_child
	== XtInheritInsertChild) {
      ((CompositeWidgetClass)XtClass(parent))->composite_class.insert_child =
	((CompositeWidgetClass) XtSuperclass(parent))->
	    composite_class.insert_child;
    }
}

void XtInheritDeleteChild(child)
    Widget child;
{
    WidgetClass     class;
    register Widget parent;

    parent = child->core.parent;

    if (! XtIsComposite(parent)) {
	XtError("Must be subclass of Composite to inherit delete_child");
    }
    if (((CompositeWidgetClass) XtSuperclass(parent))->
	   composite_class.delete_child) {
	/* Here we fake runtime casting by changing the class field in  */
	/* the current class record and resetting it when we return.    */
	class = XtClass(parent);
	XtClass(parent) = XtSuperclass(parent);

	(*(((CompositeWidgetClass) XtClass(parent))->
	     composite_class.delete_child))(child);
	XtClass(parent) = class;
    }
    if (((CompositeWidgetClass)XtClass(parent))->composite_class.delete_child
	== XtInheritDeleteChild) {
      ((CompositeWidgetClass)(XtClass(parent)))->composite_class.delete_child =
	((CompositeWidgetClass) XtSuperclass(parent))->
	    composite_class.delete_child;
    }
}

/*
 * I don't understand the args to these routine and as such I can not write the
 * proc's
 */
void XtInheritMoveFocusToNext(w) 
    Widget w;
{
    XtError("InheritMoveFocusToNext is not written");
}

void XtInheritMoveFocusToPrev(w) 
    Widget w;
{
    XtError("XtInheritMoveFocusToPrev is not written");
}
/* end of composite inherit functions */

