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
         Offset(Widget,core.width), XtRString, "1"},
    {XtNheight, XtCHeight, XrmRInt, sizeof(int),
         Offset(Widget,core.height), XtRString, "1"},
    {XtNdepth, XtCDepth,XrmRInt,sizeof(int),
         Offset(Widget,core.depth), XtRString, "1"},
    {XtNbackground,XtCBackground,XrmRPixel,sizeof(Pixel),
         Offset(Widget,core.background_pixel), XtRString, "White"},
    {XtNborderWidth, XtCBorderWidth,XrmRInt, sizeof(int),
         Offset(Widget,core.border_width),XtRString, "1"},
    {XtNborder,XtCBorderColor,XrmRPixel,sizeof(Pixel),
         Offset(Widget,core.border_pixel),XtRString, "Black"},
    {XtNsensitive,XtCSensitive,XrmRBoolean, sizeof(Boolean),
         Offset(Widget,core.sensitive),XtRString,"TRUE"}
    };
extern void CoreDestroy();
extern void SetValues ();
WidgetClassData widgetClassData = {
         (WidgetClass)NULL,	/*superclass pointer*/
         "Core",		/*class_name*/
          sizeof(WidgetData),   /*size of core data record*/
          (WidgetProc)NULL,	/*Initialize*/
          (WidgetProc)NULL,	/*Realize*/
          NULL,			/*actions*/
          resources,		/*resource list*/
          XtNumber(resources),	/*resource_count*/
          NULL,			/*xrm_extra*/
          NULLQUARK,		/*xrm_class*/
          FALSE,		/*visible_interest*/
          CoreDestroy,		/*destroy proc*/
          (WidgetProc) NULL,	 /*resize*/
          (WidgetExposeProc)NULL, /*expose*/
          SetValues,		/*set_values*/
          (WidgetProc)NULL      /*accept_focus*/
};
Widget TopLevelCreate(name,widgetClass,screen,args,argCount)
    char *name;
    WidgetClass widgetClass;
    Screen* screen;
    ArgList args;
    Cardinal argCount;
{
   Widget widget;
    widget = (Widget)XtMalloc(widgetClass->coreClass.size);
    widget->core.name = (char *)strcpy(XtMalloc(strlen(name)+1), name);
    widget->core.widget_class = widgetClass;
    widget->core.parent = NULL;
    widget->core.screen = screen;
    widget->core.visible = TRUE;
   if (XtIsSubClass (widget,compositeWidgetClass)) {
                ((CompositeWidget)widget)->composite.num_children = 0;
               ((CompositeWidget)widget)->composite.num_managed_children = 0;
                ((CompositeWidget)widget)->composite.children = NULL;
                }
    XtGetResources(widget,args,argCount);
   widgetClass->coreClass.initialize();
   return (widget);
}

Widget XtWidgetCreate(name,widgetClass,parent,args,argCount)
    char *name;	
    WidgetClass   widgetClass;
    CompositeWidget   parent;
    ArgList       args;
    Cardinal      argCount;

{
    Widget    widget;

    if (widgetClass == NULL || parent == NULL)  {
			XtError("invalid parameters to XtCreate");
			return;
			}
    widget = (Widget)XtMalloc(widgetClass->coreClass.size); 
    widget->core.name = (char *)strcpy(XtMalloc(strlen(name)+1), name);
    widget->core.widget_class = widgetClass;
    widget->core.parent = parent;
    widget->core.screen = parent->core.screen;
    widget->core.managed = FALSE;
    widget->core.visible = TRUE;
    widget->core.being_destroyed = parent -> core.being_destroyed;
    if (XtIsSubClass (widget,compositeWidgetClass)) {
		((CompositeWidget)widget)->composite.num_children = 0;
		((CompositeWidget)widget)->composite.num_managed_children = 0;
		((CompositeWidget)widget)->composite.children = NULL;
                }
    XtGetResources(widget,args,argCount);    
    XtDefineTranslation(widget);
    widgetClass->coreClass.initialize();
    ((CompositeWidgetClass)(widget->core.parent->core.widget_class))->compositeClass.insert_child(widget);
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

void RegisterWindow(widget,window)
    Widget    widget;
    Window    window;
{
}
Boolean XtWidgetIsRealized (widget)
    Widget   widget;
{
   return (widget->core.window != NULL);
}

Boolean XtIsSubClass(widget, widgetClass)
    Widget    widget;
    WidgetClass widgetClass;
{
  WidgetClass w;
  for(w=widget->core.widget_class; w != NULL; w = w->coreClass.superclass)
        if (w == widgetClass) return (TRUE);
  return (FALSE);
}

void XtWidgetRealize (widget)
    
    Widget    widget;
{
    CompositeWidget cwidget;
    ValueMask valuemask;
    XSetWindowAttributes values;
    Cardinal i;
   if (XtWidgetIsRealized(widget)) return;
   FillInParameters (widget,&valuemask,&values);
   widget->core.widget_class->coreClass.realize(widget,valuemask,&values);
   RegisterWindow(widget,widget->core.window);
   if (XtIsSubClass (widget, compositeWidgetClass)) {
        cwidget = (CompositeWidget)widget;
	for (i= cwidget->composite.num_children;i!=0;--i) 
		XtWidgetRealize(cwidget->composite.children[i-1]);
        if (cwidget->composite.num_children == cwidget->composite.num_managed_children)
		XMapSubwindows(widget);
	else while (i= cwidget->composite.num_managed_children > 0) {
		if (cwidget->composite.children[i-1]->core.managed) {
			XMapWindow(cwidget->composite.children[i-1]);
			i--;
			} }
   }
    if (widget->core.parent == NULL) XMapWindow(widget);
   return;
}
			
		
void XtWidgetSetSensitive(widget,sensitive)
    Widget    widget;
    Boolean   sensitive;
{
    int i;
    widget->core.sensitive = sensitive;
    if ((widget->core.sensitive == widget->core.ancestor_sensitive) 
                                 && XtIsSubClass (widget,compositeWidgetClass))
      for (i= ((CompositeWidget)widget)->composite.num_children;i != 0; --i)
        XtWidgetSetSensitive (((CompositeWidget)widget)->composite.children[i-1],sensitive);
      
}

#define ClassToSuperclass(widget, proc)	\
{  WidgetClass widgetClass; \
  for(widgetClass = widget->core.widget_class;widgetClass != NULL; \
                         widgetClass = widgetClass ->coreClass.superclass) \
                         widgetClass->coreClass.proc; \
}

void XtAddCallback (callbackList,callback,widget,closure)
    CallbackList *callbackList;
    CallbackProc callback;
    Widget widget;
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

void XtRemoveCallback (callbackList, callback, widget, closure)
    CallbackList *callbackList;
    CallbackProc callback;
    Widget widget;
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

void XtRemoveAllCallbacks (callbackList)
    CallbackList *callbackList;

{
   CallbackRec *cl;
   if (*callbackList == NULL) return;
   for (cl = (*callbackList); cl->next != NULL; cl = cl->next) 
        XtFree (*cl);
   (*callbackList) == NULL;
   return;
}

void XtCallCallbacks (callbackList)
    CallbackList *callbackList;
{
    CallbackRec *cl;
    if ((*callbackList) == NULL )return;
    for (cl = (*callbackList); cl != NULL; cl = cl->next) 
             cl->callback(cl->widget,cl->closure);
}

void DestroyChildren (widget)
    Widget    widget;
{
    int i;
    if (widget->core.being_destroyed) return;
    widget-> core.being_destroyed = TRUE;
    if (XtIsSubClass (widget,compositeWidgetClass))
        for (i= ((CompositeWidget)widget)->composite.num_children; i != 0; --i)
                   DestroyChildren (((CompositeWidget)widget)->composite.children[i-1]);
    return;
}


void Phase2ChildrenCallbacks(widget)
    Widget    widget;
{
     CompositeWidget cwidget;
     int i;
    if (XtIsSubClass(widget,compositeWidgetClass))
     cwidget = (CompositeWidget)widget;
       for (i=cwidget->composite.num_children; i!=0; --i) {
  XtCallCallbacks(cwidget->composite.children[i-1]->core.destroy_callbacks);
           Phase2ChildrenCallbacks(cwidget->composite.children[i-1]);
       }
     return;
}

void Phase2ChildrenDestroy(widget)
    Widget    widget;
{
      CompositeWidget cwidget;
      int i;
   if (XtIsSubClass(widget,compositeWidgetClass))
       cwidget = (CompositeWidget)widget;
       for (i=cwidget->composite.num_children; i!=0; --i) {
           Phase2ChildrenDestroy(cwidget->composite.children[i-1]);
	ClassToSuperclass(cwidget->composite.children[i-1],destroy())
       }
     return;
}


void XtPhase2Destroy (widget)
    Widget    widget;
{
    if (widget->core.parent != NULL) XtCompositeRemoveChild(widget);
    XtCallCallbacks(widget->core.destroy_callbacks);
    Phase2ChildrenCallbacks(widget);
    Phase2ChildrenDestroy(widget);
    ClassToSuperclass(widget,destroy())
    if (widget->core.window != NULL) XDestroyWindow(widget->core.window);
    return;
}


void XtWidgetDestroy (widget)
    Widget    widget;

{
    DestroyChildren(widget);
    XtAddCallback( DestroyList, XtPhase2Destroy, widget, NULL);

}

void CoreDestroy (widget)
    Widget    widget;
{
}
