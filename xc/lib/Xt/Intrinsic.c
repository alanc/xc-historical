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
          (WidgetGeometryProc)NULL, /*reconfigure*/
          (WidgetExposeProc)NULL, /*expose*/
          SetValues,		/*set_values*/
          FALSE,		/*accept_focus*/
          (WidgetProc)NULL      /*accept_focus*/
};
          


Widget XtCreate(widgetClass,parent,args,argCount)
	
    WidgetClass   widgetClass;
    Widget        parent;
    ArgList       args;
    Cardinal      argCount;

{
    Widget    widget;

    if (widgetClass == NULL || parent == NULL)  {
			XtError("invalid parameters to XtCreate");
			return;
			}
    widget = (Widget)XtMalloc(widgetClass->coreClass.size); 
    widget->core.widget_class = widgetClass;
    widget->core.parent = parent;
    widget->core.display = parent->core.display;
    widget->core.screen = parent->core.screen;
    widget->core.managed = FALSE;
    widget->core.visible = TRUE;
    widget->core.being_destroyed = parent -> core.being_destroyed;
    if (IsSubClass (widget,compositeWidgetClass)) {
		((CompositeWidget)widget)->composite.num_children = 0;
		((CompositeWidget)widget)->composite.num_managed_children = 0;
		((CompositeWidget)widget)->composite.children = NULL;
                }
    XtGetResources(widget,args,argCount);    
    XtRegisterTranslations(widget);
    widgetClass->coreClass.initialize();
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

void XtRealize (widget)
    
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
   if (IsSubClass (widget, compositeWidgetClass)) {
        cwidget = (CompositeWidget)widget;
	for (i= cwidget->composite.num_children;i!=0;--i) 
		XtRealize(cwidget->composite.children[i-1]);
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
			
		
void XtSetSensitive(widget,sensitive)
    Widget    widget;
    Boolean   sensitive;
{
    int i;
    widget->core.sensitive = sensitive;
    if ((widget->core.sensitive == widget->core.ancestor_sensitive) 
                                 && IsSubClass (widget,compositeWidgetClass))
      for (i= ((CompositeWidget)widget)->composite.num_children;i != 0; --i)
        XtSetSensitive (((CompositeWidget)widget)->composite.children[i-1],sensitive);
      
}

#define ClassToSuperclass(widget, proc)	\
{  WidgetClass widgetClass; \
  for(widgetClass = widget->core.widget_class;widgetClass != NULL; \
                         widgetClass = widgetClass ->coreClass.superclass) \
                         widgetClass->coreClass.proc; \
}

void DestroyChildren (widget)
    Widget    widget;
{
    int i;
    if (widget->core.being_destroyed) return;
    widget-> core.being_destroyed = TRUE;
    if (IsSubClass (widget,compositeWidgetClass))
        for (i= ((CompositeWidget)widget)->composite.num_children; i != 0; --i)
                   DestroyChildren (((CompositeWidget)widget)->composite.children[i-1]);
    return;
}


void Phase2ChildrenCallbacks(widget)
    Widget    widget;
{
     CompositeWidget cwidget;
     int i;
    if (IsSubClass(widget,compositeWidgetClass))
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
   if (IsSubClass(widget,compositeWidgetClass))
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


void XtDestroy (widget)
    Widget    widget;

{
    DestroyChildren(widget);
    XtAddCallback( DestroyList, XtPhase2Destroy, widget, NULL);

}

void CoreDestroy (widget)
    Widget    widget;
{
}
