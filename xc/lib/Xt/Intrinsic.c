
#include "Intrinsic.h"

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


void XtRealize (widget)
    
    Widget    widget;
{
    CompositeWidget cwidget;
    Cardinal i;
   if XtWidgetIsRealized(widget) return;
   widget->core.widget_class->coreClass.realize();
   if (IsSubClass (widget, compositeWidgetClass))
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
        XtSetSensitivity (((CompositeWidget)widget)->composite.children[i-1],sensitive);
      
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

