#include "Intrinsic.h"
#include "TM.h"
#include "Atoms.h"

extern char *strcpy();
extern void RegisterWindow(); /* from event, should be in .h ||| */
extern void UnregisterWindow(); /* from event, should be in .h ||| */
/******************************************************************
 *
 * Core Resources
 *
 ******************************************************************/

static XtResource resources[] = {
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
         XtOffset(Widget,core.mapped_when_managed),XtRString,"TRUE"}
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
	  (XtWidgetProc)NULL,     /* class initializer routine */
	  FALSE,		/* not init'ed */
          (XtWidgetProc)NULL,	/* Instance Initializer routine*/
          (XtWidgetProc)NULL,	/*Realize*/
          NULL,			/*actions*/
          0,                   /*number of actions in translation table*/
          resources,		/*resource list*/
          XtNumber(resources),	/*resource_count*/
          NULLQUARK,		/*xrm_class*/
          FALSE,                /*compress motion*/
          TRUE,                 /*compress exposure*/
          FALSE,		/*visible_interest*/
          CoreDestroy,		/*destroy proc*/
          (XtWidgetProc) NULL,	 /*resize*/
          (XtExposeProc)NULL, /*expose*/
          CoreSetValues,		/*set_values*/
          (XtWidgetProc)NULL      /*accept_focus*/
};

static void CompositeInsertChild();
static void CompositeDeleteChild();
static void CompositeDestroy();

ConstraintClassRec constraintClassRec = {
    {
         (WidgetClass)&widgetClassRec,	/*superclass pointer*/
         "Composite",		/*class_name*/
          sizeof(CompositeRec),   /*size of core data record*/
	  (XtWidgetProc)NULL,     /* class initializer routine */
	  FALSE,		/* not init'ed */
          (XtWidgetProc)NULL,	/* Instance Initializer routine*/
          (XtWidgetProc)NULL,	/*Realize*/
          NULL,			/*actions*/
          0,                    /*number of actions*/
          NULL,			/*resource list*/
          0,			/*resource_count*/
          NULLQUARK,		/*xrm_class*/
          FALSE,                /*compress motion*/
          TRUE,                 /*compress expose*/
          FALSE,		/*visible_interest*/
          (XtWidgetProc) CompositeDestroy,	/*destroy proc*/
          (XtWidgetProc) NULL,	 /*resize*/
          (XtExposeProc)NULL, /*expose*/
          NULL,			/*set_values*/
          (XtWidgetProc)NULL,      /*accept_focus*/
    },{
	(XtGeometryHandler) NULL,	/* geometry_manager */
	(XtWidgetProc) NULL,
	CompositeInsertChild,
	CompositeDeleteChild,
	(XtWidgetProc) NULL,
	(XtWidgetProc) NULL,
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
	  (XtWidgetProc)NULL,     /* class initializer routine */
	  FALSE,		/* not init'ed */
          (XtWidgetProc)NULL,	/* Instance Initializer routine*/
          (XtWidgetProc)NULL,	/*Realize*/
          NULL,			/*actions*/
          0,                    /*number of actions*/
          NULL,			/*resource list*/
          0,			/*resource_count*/
          NULLQUARK,		/*xrm_class*/
          FALSE,                /*compress motion*/
          TRUE,                 /*compress expose*/
          FALSE,		/*visible_interest*/
          (XtWidgetProc) CompositeDestroy,	/*destroy proc*/
          (XtWidgetProc) NULL,	 /*resize*/
          (XtExposeProc)NULL, /*expose*/
          NULL,			/*set_values*/
          (XtWidgetProc)NULL,      /*accept_focus*/
    },{
	(XtGeometryHandler) NULL,	/* geometry_manager */
	(XtWidgetProc) NULL,
	CompositeInsertChild,
	CompositeDeleteChild,
	(XtWidgetProc) NULL,
	(XtWidgetProc) NULL,
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

Widget TopLevelCreate(name,widgetClass,screen,args,num_args)
    char *name;
    WidgetClass widgetClass;
    Screen* screen;
    ArgList args;
    Cardinal num_args;
{
   Widget widget;
    widget = (Widget)XtMalloc((unsigned)widgetClass->core_class.widget_size);
    widget->core.window = (Window) NULL;
    widget->core.name = strcpy(XtMalloc((unsigned) strlen(name)+1), name);
    widget->core.widget_class = widgetClass;
    widget->core.parent = NULL;
    widget->core.screen = screen;
    widget->core.managed = FALSE;
    widget->core.visible = TRUE;
    widget->core.background_pixmap = (Pixmap) NULL;
    widget->core.border_pixmap = (Pixmap) NULL;
    widget->core.event_table = NULL;
    widget->core.sensitive = TRUE;
    widget->core.ancestor_sensitive = TRUE;
    widget->core.translations = NULL;
    widget->core.destroy_callbacks = NULL;
    widget->core.being_destroyed = FALSE;

    if(!(widget->core.widget_class->core_class.class_inited))
	 ClassInit(widgetClass);
   if (XtIsComposite(widget)) {
                ((CompositeWidget)widget)->composite.num_children = 0;
               ((CompositeWidget)widget)->composite.num_mapped_children = 0;
                ((CompositeWidget)widget)->composite.children = NULL;
                }
   XtGetResources(widget,args,num_args);
   if (widget->core.depth == 0)
    /* ||| gross kludge! fix this!!! */
	widget->core.depth = XtScreen(widget)->root_depth;
   widgetClass->core_class.initialize(widget);
   return (widget);
}

static void CompositeDestroy(w)
    CompositeWidget	w;
{
    XtFree((char *) w->composite.children);
}

/* ARGSUSED */
static void CompositeInsertChild(w, args, num_args)
    Widget	w;
    ArgList	args;
    Cardinal	num_args;
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

Widget XtCreateWidget(name,widgetClass,parent,args,num_args)
    char	*name;	
    WidgetClass widgetClass;
    Widget	parent;
    ArgList     args;
    Cardinal    num_args;

{
    Widget    widget;

    if (widgetClass == NULL || parent == NULL || ! XtIsComposite(parent))  {
			XtError("invalid parameters to XtCreateWidget");
			return NULL;
			}
    if (! (widgetClass->core_class.class_inited))
	ClassInit(widgetClass);

    widget = (Widget)XtMalloc((unsigned)widgetClass->core_class.widget_size); 
    widget->core.window = (Window) NULL;
    widget->core.name = strcpy(XtMalloc((unsigned)strlen(name)+1), name);
    widget->core.widget_class = widgetClass;
    widget->core.parent = parent;
    widget->core.screen = parent->core.screen;
    widget->core.managed = FALSE;
    widget->core.visible = TRUE;
    widget->core.background_pixmap = (Pixmap) NULL;
    widget->core.border_pixmap = (Pixmap) NULL;
    widget->core.event_table = NULL;
    widget->core.sensitive = TRUE;
    widget->core.ancestor_sensitive = TRUE;
    widget->core.translations = NULL;
    widget->core.destroy_callbacks = NULL;
    widget->core.being_destroyed = parent -> core.being_destroyed;
    /* ||| Should be in CompositeInitialize */
    if (XtIsComposite (widget)) {
		((CompositeWidget)widget)->composite.num_children = 0;
		((CompositeWidget)widget)->composite.num_mapped_children = 0;
		((CompositeWidget)widget)->composite.children = NULL;
		((CompositeWidget)widget)->composite.num_slots = 0;
                }
    XtGetResources(widget,args,num_args);
    if (widget->core.depth == 0)
    /* ||| gross kludge! fix this!!! */
	widget->core.depth = widget->core.parent->core.depth;
    DefineTranslation(widget);
    widgetClass->core_class.initialize(widget);

    ((CompositeWidgetClass)(widget->core.parent->core.widget_class))
    	->composite_class.insert_child(widget, args, num_args);
    return (widget);
}

void FillInParameters(widget,valuemask,values)
    Widget  widget;
    XtValueMask *valuemask;
    XSetWindowAttributes *values;
{
    *valuemask = (CWBackPixel | CWBorderPixel | CWEventMask);
    (*values).event_mask = _XtBuildEventMask(widget);
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
    XtValueMask valuemask;
    XSetWindowAttributes values;
    Cardinal i;
   if (XtIsRealized(widget)) return;
   FillInParameters (widget,&valuemask,&values);
   widget->core.widget_class->core_class.realize(widget,valuemask,&values);
   RegisterWindow(widget->core.window, widget);
   if (XtIsComposite (widget)) {
        cwidget = (CompositeWidget)widget;
	for (i= cwidget->composite.num_children;i!=0;--i) 
		XtRealizeWidget(cwidget->composite.children[i-1]);
        if (cwidget->composite.num_children == cwidget->composite.num_mapped_children)
		XMapSubwindows(XtDisplay(widget), XtWindow(widget));
	else while (i = cwidget->composite.num_mapped_children != 0) {
		if (cwidget->composite.children[i-1]->core.managed) {
			XtMapWidget(cwidget->composite.children[i-1]);
			i--;
			} }
   }
    if (widget->core.parent == NULL) XtMapWidget(widget);
   return;
}
			
		
void XtUnmanageChildren(children, num_children)
    WidgetList children;
    Cardinal num_children;
{
    CompositeWidget	parent;
    register Widget	child;
    Cardinal		newCount, oldCount;

    if (num_children == 0) return;
    parent = (CompositeWidget) children[0]->core.parent;
    if (parent->core.being_destroyed) return;

    newCount = 0;
    for (oldCount = 0; oldCount < num_children; oldCount++) {
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
    parent->composite.num_mapped_children =
    	parent->composite.num_mapped_children - newCount;

    ((CompositeWidgetClass)parent->core.widget_class)
        ->composite_class.change_managed(parent);
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
    Cardinal		newCount, oldCount;

    if (num_children == 0) return;
    parent = (CompositeWidget) children[0]->core.parent;
    if (parent->core.being_destroyed) return;

    newCount = 0;
    for (oldCount = 0; oldCount < num_children; oldCount++) {
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
    parent->composite.num_mapped_children =
    	parent->composite.num_mapped_children + newCount;

    ((CompositeWidgetClass)parent->core.widget_class)
        ->composite_class.change_managed(parent);
}

void XtManageChild(child)
    Widget child;
{
    XtManageChildren(&child, 1);
}

void XtSetMappedWhenManaged(widget, mappedWhenManaged)
    Widget    widget;
    Boolean   mappedWhenManaged;
{
    if (widget->core.mapped_when_managed == mappedWhenManaged) return;
    XtWarning("Mapped when managed not implemented."); /* ||| */
    if (mappedWhenManaged) {
	/* we didn't used to be mapped when managed. If we are realized and */
        /* managed then map us, increment parent's count of mapped children */
    } else {
	/* we used to be mapped when managed. If we were realized and */
	/* managed then unmap us, decrement parent's mapped children count */
    }
}

void XtSetSensitive(widget,sensitive)
    Widget    widget;
    Boolean   sensitive;
{
    int i;
    widget->core.sensitive = sensitive;
    if ((widget->core.sensitive == widget->core.ancestor_sensitive) 
                                 && XtIsComposite (widget))
      for (i= ((CompositeWidget)widget)->composite.num_children;i != 0; --i)
        XtSetSensitive (((CompositeWidget)widget)->composite.children[i-1],sensitive);
      
}

#define TABLESIZE 20
typedef struct {
    int offset;
    WidgetClass widgetClass;
} CallbackTableRec,*CallbackTable;
typedef void (*fooProc)();
static CallbackTable callbackTable = NULL;
static XtCallbackKind currentIndex = 1;
static XtCallbackKind maxIndex;
InitializeCallbackTable ()
{
  callbackTable = (CallbackTable) XtMalloc(
	(unsigned)TABLESIZE*sizeof(CallbackTableRec));
   maxIndex = TABLESIZE;
}

static void ExpandTable()
{

   callbackTable = (CallbackTable)XtRealloc(
	(char *)callbackTable,
	(unsigned)(currentIndex+TABLESIZE)*sizeof(CallbackTableRec));
   maxIndex = currentIndex + TABLESIZE;
} 
                                             

XtCallbackKind XtNewCallbackKind(widgetClass,offset)
    WidgetClass widgetClass;
    Cardinal  offset;
{
    if (currentIndex ==  maxIndex) ExpandTable();
    callbackTable[currentIndex].offset = offset;
    callbackTable[currentIndex].widgetClass = widgetClass;
    return(currentIndex++);
}



XtCallbackList *FetchXtCallbackList (widget,callbackKind)
    Widget  widget;
    XtCallbackKind  callbackKind;
{
    if ( callbackKind >= maxIndex ||
       !XtIsSubclass(widget,callbackTable[callbackKind].widgetClass) )
          return(NULL);

    return ((XtCallbackList*)((int)widget + callbackTable[callbackKind].offset));
}


void AddCallback (widget,callbackList,callback,closure)
    Widget widget;
    XtCallbackList *callbackList;
    XtCallbackProc callback;
    Opaque closure;
{

     XtCallbackRec *c,*cl;
     c =  (XtCallbackRec*) XtMalloc((unsigned)sizeof(XtCallbackRec));
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

void XtAddCallback(widget, callbackKind,callback,closure)
    Widget    widget;
    XtCallbackKind callbackKind;
    XtCallbackProc callback;
    Opaque      closure;
{
    XtCallbackList *callbackList;
    callbackList = FetchXtCallbackList(widget,callbackKind);
    if (callbackList == NULL) {
       XtError("invalid parameters to XtAddCallback");
       return;
    }
    AddCallback(widget,callbackList,callback,closure);
    return;
}

void RemoveCallback (widget,callbackList, callback, closure)
    Widget  widget;
    XtCallbackList *callbackList;
    XtCallbackProc callback;
    Opaque closure;

{
   XtCallbackList cl;

   for (
       cl = *callbackList;
       cl != NULL;
       (cl = *(callbackList = &cl->next)))
           if (( cl->widget == widget) && (cl->closure == closure)
                            && (cl->callback == callback) ) {
	       *callbackList = cl->next;
               XtFree ((char *)cl);
           }
   return;
}

void XtRemoveCallback (widget, callbackKind, callback, closure)
    Widget    widget;
    XtCallbackKind callbackKind;
    XtCallbackProc callback;
    Opaque      closure;
{
   XtCallbackList *callbackList;
   callbackList = FetchXtCallbackList(widget,callbackKind);
   if (callbackList == NULL) {
      XtError("invalid parameters to XtRemoveCallback");
      return;
   }
   RemoveCallback(widget,callbackList,callback,closure);
    return;
}


void RemoveAllCallbacks (callbackList)
    XtCallbackList *callbackList;

{
   XtCallbackList cl, tcl;

   cl = *callbackList;
   while (cl != NULL) {
	tcl = cl;
	cl = cl->next;
	XtFree((char *) tcl);
   }

   (*callbackList) = NULL;
}

void XtRemoveAllCallbacks(widget, callbackKind)
    Widget widget;
    XtCallbackKind  callbackKind;
{
   XtCallbackList *callbackList;
   callbackList = FetchXtCallbackList(widget,callbackKind);
   if (callbackList == NULL) {
      XtError("invalid parameters to XtRemoveAllCallbacks");
     return;
   }
   RemoveAllCallbacks(callbackList);
   return;
}


void CallCallbacks (callbackList,callData)
    XtCallbackList *callbackList;
    Opaque callData;
{
    XtCallbackRec *cl;
    if ((*callbackList) == NULL )return;
    for (cl = (*callbackList); cl != NULL; cl = cl->next) 
             cl->callback(cl->widget,cl->closure,callData);
}

void XtCallCallbacks (widget, callbackKind, callData)
    Widget   widget;
    XtCallbackKind callbackKind;
    Opaque  callData;
{
   XtCallbackList *callbackList;
   callbackList = FetchXtCallbackList(widget,callbackKind);
   if (callbackList == NULL) {
     XtError("invalid parameters to XtCallCallbacks");
     return;
   }
   CallCallbacks(callbackList,callData);
   return;
}


void Phase1Destroy (widget)
    Widget    widget;
{
    int i;
    if (widget->core.being_destroyed) return;
    widget-> core.being_destroyed = TRUE;
    if (XtIsComposite (widget))
        for (i= ((CompositeWidget)widget)->composite.num_children; i != 0; --i)
            Phase1Destroy (((CompositeWidget)widget)->composite.children[i-1]);
    return;
}
void Recursive(widget,proc)
   Widget  widget;
   fooProc proc;
{
  CompositeWidget cwidget;
  int i;
  if (XtIsComposite(widget)) {
      cwidget=(CompositeWidget)widget;
      for (i=cwidget->composite.num_children;
           i != 0; --i) {
         Recursive(cwidget->composite.children[i-1],proc);
      }
  } 
  (*proc)(widget);  
  return;
}

void Phase2Callbacks(widget)
    Widget    widget;
{
     CallCallbacks(&(widget->core.destroy_callbacks),
         (Opaque) NULL);
     return;
}

void Phase2Destroy(widget)
    Widget    widget;
{
  WidgetClass widgetClass;
  for(widgetClass = widget->core.widget_class;
      widgetClass != NULL; 
      widgetClass = widgetClass ->core_class.superclass) 
  if ((widgetClass->core_class.destroy) != NULL)
       widgetClass->core_class.destroy(widget);
}

void XtPhase2Destroy (widget)
    Widget    widget;
{
    Display *display;
    Window window;
    if (widget->core.parent != NULL) {
	XtUnmanageChild(widget);
	((CompositeWidgetClass) widget->core.parent->core.widget_class)
		->composite_class.delete_child(widget);
    }
    display = XtDisplay(widget); /* Phase2Destroy removes Widget*/
    window = widget->core.window;
    Recursive(widget,Phase2Callbacks);
    Recursive(widget,Phase2Destroy);
    if (window != NULL) XDestroyWindow(display,window);
    return;
}


void XtDestroyWidget (widget)
    Widget    widget;

{
    if (widget->core.being_destroyed) return;

    Phase1Destroy(widget);
    AddCallback(widget, &DestroyList, XtPhase2Destroy, (Opaque)NULL);

}

static void CoreDestroy (widget)
    Widget    widget;
{
   register XtEventRec *p1,*p2;
   XtFree((char*)(widget->core.name));
   if (widget->core.background_pixmap != NULL) 
      XFreePixmap(XtDisplay(widget),widget->core.background_pixmap);
   if (widget->core.border_pixmap != NULL)
      XFreePixmap(XtDisplay(widget),widget->core.border_pixmap);
   p1 = widget->core.event_table;
   while(p1 != NULL){
     p2 = p1;
     p1 = p1->next;
     XtFree((char*)p2);
   }
   if (widget->core.translations != NULL)
     TranslateTableFree(widget->core.translations);
   UnregisterWindow(widget->core.window,widget);
/* if (onGrabList(widget))RemoveGrab(widget); */
   XtFree((char*)widget);
   return;
     
}

static void CoreSetValues()
{
/* ||| */
}
