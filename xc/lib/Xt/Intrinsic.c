#include "Intrinsic.h"
#include "Atoms.h"
/******************************************************************
 *
 * Core Resources
 *
 ******************************************************************/

static Resource resources[] = {
    {XtNx, XtCPosition, XrmRInt, sizeof(int),
         Offset(Widget,core.x), XtRString, "0"},
    {XtNy, XtCPosition, XrmRInt, sizeof(int),
         Offset(Widget,core.y), XtRString, "0"},
    {XtNwidth, XtCWidth, XrmRInt, sizeof(int),
         Offset(Widget,core.width), XtRString, "0"},
    {XtNheight, XtCHeight, XrmRInt, sizeof(int),
         Offset(Widget,core.height), XtRString, "0"},
/* default depth should be "InheritFromParent", and we should have a */
/* string to depth type converter, but it needs the widget to be able */
/* to find the parent's depth... right now, we kludge it and check for */
/* a depth of 0 in XtCreateWidget. Gross. */
    {XtNdepth, XtCDepth,XrmRInt,sizeof(int),
         Offset(Widget,core.depth), XtRString, "0"},
    {XtNbackground,XtCBackground,XrmRPixel,sizeof(Pixel),
         Offset(Widget,core.background_pixel), XtRString, "White"},
    {XtNborderWidth, XtCBorderWidth,XrmRInt, sizeof(int),
         Offset(Widget,core.border_width),XtRString, "1"},
    {XtNborder,XtCBorderColor,XrmRPixel,sizeof(Pixel),
         Offset(Widget,core.border_pixel),XtRString, "Black"},
    {XtNsensitive,XtCSensitive,XrmRBoolean, sizeof(Boolean),
         Offset(Widget,core.sensitive),XtRString,"TRUE"}
    };
static void CoreDestroy();
static void CoreSetValues ();

CompositeWidgetClass compositeWidgetClass = &compositeClassRec;
ConstraintWidgetClass constraintWidgetClass = &constraintClassRec;

/* ||| Should have defaults for Inherit from superclass to work */

WidgetClassRec widgetClassRec = {
         (WidgetClass)NULL,	/*superclass pointer*/
         "Core",		/*class_name*/
          sizeof(WidgetRec),   /*size of core data record*/
	  (WidgetProc)NULL,     /* class initializer routine */
	  FALSE,		/* not init'ed */
          (WidgetProc)NULL,	/* Instance Initializer routine*/
          (WidgetProc)NULL,	/*Realize*/
          NULL,			/*actions*/
          0,                   /*number of actions in translation table*/
          resources,		/*resource list*/
          XtNumber(resources),	/*resource_count*/
          NULLQUARK,		/*xrm_class*/
          FALSE,                /*compress motion*/
          TRUE,                 /*compress exposure*/
          FALSE,		/*visible_interest*/
          CoreDestroy,		/*destroy proc*/
          (WidgetProc) NULL,	 /*resize*/
          (ExposeProc)NULL, /*expose*/
          CoreSetValues,		/*set_values*/
          (WidgetProc)NULL      /*accept_focus*/
};

static void CompositeInsertChild();
static void CompositeDeleteChild();
static void CompositeDestroy();

ConstraintClassRec constraintClassRec = {
    {
         (WidgetClass)&widgetClassRec,	/*superclass pointer*/
         "Composite",		/*class_name*/
          sizeof(CompositeRec),   /*size of core data record*/
	  (WidgetProc)NULL,     /* class initializer routine */
	  FALSE,		/* not init'ed */
          (WidgetProc)NULL,	/* Instance Initializer routine*/
          (WidgetProc)NULL,	/*Realize*/
          NULL,			/*actions*/
          0,                    /*number of actions*/
          NULL,			/*resource list*/
          0,			/*resource_count*/
          NULLQUARK,		/*xrm_class*/
          FALSE,                /*compress motion*/
          TRUE,                 /*compress expose*/
          FALSE,		/*visible_interest*/
          (WidgetProc) CompositeDestroy,	/*destroy proc*/
          (WidgetProc) NULL,	 /*resize*/
          (ExposeProc)NULL, /*expose*/
          NULL,			/*set_values*/
          (WidgetProc)NULL,      /*accept_focus*/
    },{
	(XtGeometryHandler) NULL,	/* geometry_manager */
	(WidgetProc) NULL,
	CompositeInsertChild,
	CompositeDeleteChild,
	(WidgetProc) NULL,
	(WidgetProc) NULL,
    },{
	NULL,
	0
    }
};

CompositeClassRec compositeClassRec = {
    {
         (WidgetClass)&widgetClassRec,	/*superclass pointer*/
         "Composite",		/*class_name*/
          sizeof(CompositeRec),   /*size of core data record*/
	  (WidgetProc)NULL,     /* class initializer routine */
	  FALSE,		/* not init'ed */
          (WidgetProc)NULL,	/* Instance Initializer routine*/
          (WidgetProc)NULL,	/*Realize*/
          NULL,			/*actions*/
          0,                    /*number of actions*/
          NULL,			/*resource list*/
          0,			/*resource_count*/
          NULLQUARK,		/*xrm_class*/
          FALSE,                /*compress motion*/
          TRUE,                 /*compress expose*/
          FALSE,		/*visible_interest*/
          (WidgetProc) CompositeDestroy,	/*destroy proc*/
          (WidgetProc) NULL,	 /*resize*/
          (ExposeProc)NULL, /*expose*/
          NULL,			/*set_values*/
          (WidgetProc)NULL,      /*accept_focus*/
    },{
	(XtGeometryHandler) NULL,	/* geometry_manager */
	(WidgetProc) NULL,
	CompositeInsertChild,
	CompositeDeleteChild,
	(WidgetProc) NULL,
	(WidgetProc) NULL,
    }
};

void ClassInit(widgetClass)
    WidgetClass widgetClass;
{
    if ((widgetClass->core_class.superclass != NULL) 
           && (!(widgetClass->core_class.superclass-> core_class.class_inited)))
 	ClassInit(widgetClass->core_class.superclass);
    if (widgetClass->core_class.class_initialize != NULL)
       widgetClass->core_class.class_initialize();
    widgetClass->core_class.class_inited = TRUE;
    return;
}

Widget TopLevelCreate(name,widgetClass,screen,args,argCount)
    char *name;
    WidgetClass widgetClass;
    Screen* screen;
    ArgList args;
    Cardinal argCount;
{
   Widget widget;
    widget = (Widget)XtMalloc(widgetClass->core_class.size);
    widget->core.window = (Window) NULL;
    widget->core.name = (char *)strcpy(XtMalloc(strlen(name)+1), name);
    widget->core.widget_class = widgetClass;
    widget->core.parent = NULL;
    widget->core.screen = screen;
    widget->core.managed = FALSE;
    widget->core.visible = TRUE;
    widget->core.background_pixmap = (Pixmap) NULL;
    widget->core.border_pixmap = (Pixmap) NULL;
    widget->core.event_mask = 0;
    widget->core.event_table = NULL;
    widget->core.sensitive = TRUE;
    widget->core.ancestor_sensitive = TRUE;
    widget->core.translations = NULL;
    widget->core.destroy_callbacks = NULL;
    widget->core.being_destroyed = FALSE;

    if(!(widget->core.widget_class->core_class.class_inited))
	 ClassInit(widgetClass);
   if (XtIsSubclass (widget,compositeWidgetClass)) {
                ((CompositeWidget)widget)->composite.num_children = 0;
               ((CompositeWidget)widget)->composite.num_managed_children = 0;
                ((CompositeWidget)widget)->composite.children = NULL;
                }
   XtGetResources(widget,args,argCount);
   if (widget->core.depth == 0)
    /* ||| gross kludge! fix this!!! */
	widget->core.depth = XtScreen(widget)->root_depth;
   widgetClass->core_class.initialize();
   return (widget);
}

static void CompositeDestroy(w)
    CompositeWidget	w;
{
    XtFree((char *) w->composite.children);
}

static void CompositeInsertChild(w)
    Widget	w;
{
    Cardinal	    position;
    Cardinal	    i;
    CompositeWidget cw;

    cw = (CompositeWidget) w->core.parent;

    /* ||| Get position from "insert_position" procedure */
    position = cw->composite.num_children;

    /* ||| Some better allocation, don't realloc every time ! */
    cw->composite.children = 
        (WidgetList) XtRealloc((caddr_t) cw->composite.children,
    	(cw->composite.num_children + 1) * sizeof(Widget));
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
        if (cw->composite.children[position] = w) {
	    break;
	}
    }

    /* Ripple children down one space from "position" */
    cw->composite.num_children--;
    for (i = position; i < cw->composite.num_children; i++) {
        cw->composite.children[i] = cw->composite.children[i+1];
    }
}

Widget XtCreateWidget(name,widgetClass,parent,args,argCount)
    char *name;	
    WidgetClass   widgetClass;
    CompositeWidget   parent;
    ArgList       args;
    Cardinal      argCount;

{
    Widget    widget;

    if (widgetClass == NULL || parent == NULL)  {
			XtError("invalid parameters to XtCreateWidget");
			return;
			}
    if (! (widgetClass->core_class.class_inited))
	ClassInit(widgetClass);

    widget = (Widget)XtMalloc(widgetClass->core_class.size); 
    widget->core.window = (Window) NULL;
    widget->core.name = (char *)strcpy(XtMalloc(strlen(name)+1), name);
    widget->core.widget_class = widgetClass;
    widget->core.parent = parent;
    widget->core.screen = parent->core.screen;
    widget->core.managed = FALSE;
    widget->core.visible = TRUE;
    widget->core.background_pixmap = (Pixmap) NULL;
    widget->core.border_pixmap = (Pixmap) NULL;
    widget->core.event_mask = 0;
    widget->core.event_table = NULL;
    widget->core.sensitive = TRUE;
    widget->core.ancestor_sensitive = TRUE;
    widget->core.translations = NULL;
    widget->core.destroy_callbacks = NULL;
    widget->core.being_destroyed = parent -> core.being_destroyed;
    /* ||| Should be in CompositeInitialize */
    if (XtIsSubclass (widget,compositeWidgetClass)) {
		((CompositeWidget)widget)->composite.num_children = 0;
		((CompositeWidget)widget)->composite.num_managed_children = 0;
		((CompositeWidget)widget)->composite.children = NULL;
		((CompositeWidget)widget)->composite.num_slots = 0;
                }
    XtGetResources(widget,args,argCount);
    if (widget->core.depth == 0)
    /* ||| gross kludge! fix this!!! */
	widget->core.depth = widget->core.parent->core.depth;
    DefineTranslation(widget);
    widgetClass->core_class.initialize();
    if (widget->core.widget_class->core_class.expose != NULL)
       widget->core.event_mask |= ExposureMask;
    if (widget->core.widget_class->core_class.visible_interest) 
       widget->core.event_mask |= VisibilityChangeMask;

    ((CompositeWidgetClass)(widget->core.parent->core.widget_class))
    	->composite_class.insert_child(widget);
    return (widget);
}

void FillInParameters(widget,valuemask,values)
    Widget  widget;
    ValueMask *valuemask;
    XSetWindowAttributes *values;
{
    *valuemask = (CWBackPixel | CWBorderPixel | CWEventMask);
    (*values).event_mask = widget->core.event_mask;
    (*values).background_pixel = widget->core.background_pixel;
    (*values).border_pixel = widget->core.border_pixel;
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
    
    Widget    widget;
{
    CompositeWidget cwidget;
    ValueMask valuemask;
    XSetWindowAttributes values;
    Cardinal i;
   if (XtIsRealized(widget)) return;
   FillInParameters (widget,&valuemask,&values);
   widget->core.widget_class->core_class.realize(widget,valuemask,&values);
   RegisterWindow(widget->core.window, widget);
   if (XtIsSubclass (widget, compositeWidgetClass)) {
        cwidget = (CompositeWidget)widget;
	for (i= cwidget->composite.num_children;i!=0;--i) 
		XtRealizeWidget(cwidget->composite.children[i-1]);
        if (cwidget->composite.num_children == cwidget->composite.num_managed_children)
		XMapSubwindows(XtDisplay(widget), XtWindow(widget));
	else while (i= cwidget->composite.num_managed_children > 0) {
		if (cwidget->composite.children[i-1]->core.managed) {
			XtMapWidget(cwidget->composite.children[i-1]);
			i--;
			} }
   }
    if (widget->core.parent == NULL) XtMapWidget(widget);
   return;
}
			
		
void XtCompositeRemoveChildren(children, childCount)
    WidgetList children;
    Cardinal childCount;
{
    CompositeWidget	parent;
    register Widget	child;
    Cardinal		newCount, oldCount;

    if (childCount == 0) return;
    parent = (CompositeWidget) children[0]->core.parent;
    if (parent->core.being_destroyed) return;

    newCount = 0;
    for (oldCount = 0; oldCount < childCount; oldCount++) {
	child = children[oldCount];
        if ((CompositeWidget) child->core.parent != parent) {
	    XtWarning("Not all children have same parent in XtRemoveChildren");
	} else if ((! child->core.managed) || (child->core.being_destroyed)) {
	    /* Do nothing */
	} else {
	    if (XtIsRealized(child)) {
		XtUnmapWidget(child);
	    }
	    child->core.managed = FALSE;
	    newCount++;
	}
    }
    parent->composite.num_managed_children =
    	parent->composite.num_managed_children - newCount;

    ((CompositeWidgetClass)parent->core.widget_class)
        ->composite_class.change_managed(parent);
}

void XtCompositeRemoveChild(child)
    Widget child;
{
    XtCompositeRemoveChildren(&child, 1);
}

void XtCompositeAddChildren(children, childCount)
    WidgetList children;
    Cardinal childCount;
{
    CompositeWidget	parent;
    register Widget	child;
    Cardinal		newCount, oldCount;

    if (childCount == 0) return;
    parent = (CompositeWidget) children[0]->core.parent;
    if (parent->core.being_destroyed) return;

    newCount = 0;
    for (oldCount = 0; oldCount < childCount; oldCount++) {
	child = children[oldCount];
        if ((CompositeWidget) child->core.parent != parent) {
	    XtWarning("Not all children have same parent in XtAddChildren");
	} else if ((child->core.managed) || (child->core.being_destroyed)) {
	    /* Do nothing */
	} else {
	    if (XtIsRealized(child)) {
		/* ||| Do mapping after change_managed */
		XtMapWidget(child);
	    }
	    child->core.managed = TRUE;
	    newCount++;
	}
    }
    parent->composite.num_managed_children =
    	parent->composite.num_managed_children + newCount;

    ((CompositeWidgetClass)parent->core.widget_class)
        ->composite_class.change_managed(parent);
}

void XtCompositeAddChild(child)
    Widget child;
{
    XtCompositeAddChildren(&child, 1);
}

void XtSetSensitive(widget,sensitive)
    Widget    widget;
    Boolean   sensitive;
{
    int i;
    widget->core.sensitive = sensitive;
    if ((widget->core.sensitive == widget->core.ancestor_sensitive) 
                                 && XtIsSubclass (widget,compositeWidgetClass))
      for (i= ((CompositeWidget)widget)->composite.num_children;i != 0; --i)
        XtSetSensitive (((CompositeWidget)widget)->composite.children[i-1],sensitive);
      
}

#define ClassToSuperclass(widget, proc)	\
{  WidgetClass widgetClass; \
  for(widgetClass = widget->core.widget_class;widgetClass != NULL; \
                         widgetClass = widgetClass ->core_class.superclass) \
  if ((widgetClass->core_class.proc) != (WidgetProc)NULL) widgetClass->core_class.proc; \
}
#define TABLESIZE 20
typedef struct {
    int offset;
    WidgetClass widgetClass;
} CallbackTableRec,*CallbackTable;

static CallbackTable callbackTable = NULL;
static XtCallbackType currentIndex = 1;
static XtCallbackType maxIndex;
InitializeCallbackTable ()
{
  callbackTable = (CallbackTable) XtMalloc(TABLESIZE*sizeof(CallbackTableRec));
   maxIndex = TABLESIZE;
}

static void ExpandTable()
{

   callbackTable = (CallbackTable)XtRealloc(callbackTable,
                    (currentIndex+TABLESIZE)*sizeof(CallbackTableRec));
   maxIndex = currentIndex + TABLESIZE;
} 
                                             

XtCallbackType XtNewCallbackType(widgetClass,offset)
    WidgetClass widgetClass;
    Cardinal  offset;
{
    if (currentIndex ==  maxIndex) ExpandTable();
    callbackTable[currentIndex].offset = offset;
    callbackTable[currentIndex].widgetClass = widgetClass;
    return(currentIndex++);
}



CallbackList *FetchCallbackList (widget,callbackType)
    Widget  widget;
    XtCallbackType  callbackType;
{
    if ( callbackType >= maxIndex ||
       !XtIsSubclass(widget,callbackTable[callbackType].widgetClass) )
          return(NULL);

    return ((CallbackList*)((int)widget + callbackTable[callbackType].offset));
}


void AddCallback (widget,callbackList,callback,closure)
    Widget widget;
    CallbackList *callbackList;
    CallbackProc callback;
    Opaque closure;
{

     CallbackRec *c,*cl;
     c =  (CallbackRec*) XtMalloc(sizeof(CallbackRec));
     c -> next = NULL;
     c -> widget = widget;
     c -> closure = closure;
     c -> callback = callback; 
    if (*callbackList == NULL){
             (*callbackList) = c;
             return;
    }
    for (cl = (*callbackList); cl->next != NULL; cl = cl->next) {}
    cl->next = c;
    return;
}

void XtAddCallback(widget, callbackType,callback,closure)
    Widget    widget;
    XtCallbackType callbackType;
    CallbackProc callback;
    caddr_t      closure;
{
    CallbackList *callbackList;
    callbackList = FetchCallbackList(widget,callbackType);
    if (callbackList == NULL) {
       XtError("invalid parameters to XtAddCallback");
       return;
    }
    AddCallback(widget,callbackList,callback,closure);
    return;
}

void RemoveCallback (widget,callbackList, callback, closure)
    Widget  widget;
    CallbackList *callbackList;
    CallbackProc callback;
    Opaque closure;

{
   CallbackRec *cl;
   if (*callbackList == NULL) return;
   for (cl = (*callbackList); cl->next != NULL; cl = cl->next) 
           if (( cl->widget == widget) && (cl->closure == closure)
                            && (cl->callback == callback) ) {
               cl = (*cl).next;
               XtFree (*cl);
           }
   return;
}

void XtRemoveCallback (widget, callbackType, callback, closure)
    Widget    widget;
    XtCallbackType callbackType;
    CallbackProc callback;
    caddr_t      closure;
{
   CallbackList *callbackList;
   callbackList = FetchCallbackList(widget,callbackType);
   if (callbackList == NULL) {
      XtError("invalid parameters to XtRemoveCallback");
      return;
   }
   RemoveCallback(widget,callbackList,callback,closure);
    return;
}


void RemoveAllCallbacks (callbackList)
    CallbackList *callbackList;

{
   CallbackRec *cl;
   if (*callbackList == NULL) return;
   for (cl = (*callbackList); cl->next != NULL; cl = cl->next) 
        XtFree (*cl);
   (*callbackList) = NULL;
   return;
}

void XtRemoveAllCallbacks(widget, callbackType)
    Widget widget;
    XtCallbackType  callbackType;
{
   CallbackList *callbackList;
   callbackList = FetchCallbackList(widget,callbackType);
   if (callbackList == NULL) {
      XtError("invalid parameters to XtRemoveAllCallbacks");
     return;
   }
   RemoveAllCallbacks(callbackList);
   return;
}


void CallCallbacks (callbackList,callData)
    CallbackList *callbackList;
    caddr_t callData;
{
    CallbackRec *cl;
    if ((*callbackList) == NULL )return;
    for (cl = (*callbackList); cl != NULL; cl = cl->next) 
             cl->callback(cl->widget,cl->closure,callData);
}

void XtCallCallbacks (widget, callbackType, callData)
    Widget   widget;
    XtCallbackType callbackType;
    caddr_t  callData;
{
   CallbackList *callbackList;
   callbackList = FetchCallbackList(widget,callbackType);
   if (callbackList == NULL) {
     XtError("invalid parameters to XtCallCallbacks");
     return;
   }
   CallCallbacks(callbackList,callData);
   return;
}


void DestroyChildren (widget)
    Widget    widget;
{
    int i;
    if (widget->core.being_destroyed) return;
    widget-> core.being_destroyed = TRUE;
    if (XtIsSubclass (widget,compositeWidgetClass))
        for (i= ((CompositeWidget)widget)->composite.num_children; i != 0; --i)
            DestroyChildren (((CompositeWidget)widget)->composite.children[i-1]);
    return;
}


void Phase2ChildrenCallbacks(widget)
    Widget    widget;
{
     CompositeWidget cwidget;
     int i;
    if (XtIsSubclass(widget,compositeWidgetClass)) {
     cwidget = (CompositeWidget)widget;
       for (i=cwidget->composite.num_children; i!=0; --i) {
    CallCallbacks(&(cwidget->composite.children[i-1]->core.destroy_callbacks),
                                                 NULL);
           Phase2ChildrenCallbacks(cwidget->composite.children[i-1]);
       }
    }
     return;
}

void Phase2ChildrenDestroy(widget)
    Widget    widget;
{
      CompositeWidget cwidget;
      int i;
   if (XtIsSubclass(widget,compositeWidgetClass)){
       cwidget = (CompositeWidget)widget;
       for (i=cwidget->composite.num_children; i!=0; --i) {
           Phase2ChildrenDestroy(cwidget->composite.children[i-1]);
	ClassToSuperclass(cwidget->composite.children[i-1],destroy())
       }
   }
     return;
}


void XtPhase2Destroy (widget)
    Widget    widget;
{
    if (widget->core.parent != NULL) {
	XtCompositeRemoveChild(widget);
	((CompositeWidgetClass) widget->core.parent->core.widget_class)
		->composite_class.delete_child(widget);
    }

    CallCallbacks(&(widget->core.destroy_callbacks),NULL);
    Phase2ChildrenCallbacks(widget);
    Phase2ChildrenDestroy(widget);
    /* ||| All this shit needs to check procs for NULL before calling */
    ClassToSuperclass(widget,destroy())
    if (widget->core.window != NULL) XDestroyWindow(widget->core.window);
    return;
}


void XtDestroyWidget (widget)
    Widget    widget;

{
    if (widget->core.being_destroyed) return;

    DestroyChildren(widget);
    AddCallback(widget, &DestroyList, XtPhase2Destroy, NULL);

}

static void CoreDestroy (widget)
    Widget    widget;
{
/* ||| */
}

static void CoreSetValues()
{
/* ||| */
}
