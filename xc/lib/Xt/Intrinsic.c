
Widget XtCreate(widgetClass,parent,args,argCount)
	
    WidgetClass   widgetClass;
    Widget        parent;
    ArgList       args;
    Cardinal      argCount;

{
    Widget    widget;

    if (widgetClass == NULL || parent == NULL)  {
			XtError("invalid parameters to XtCreate");
			return(XtError);
			}
    widget = XtMalloc(widgetClass->size);
    widget->core.widgetClass = widgetClass;
    widget->core.parent = parent;
    widget->core.display = parent->core.display;
    widget->core.screen = parent->core.screen;
    widget->core.managed = FALSE;
    widget->core.visible = TRUE;
    widget->core.being_destroyed = parent -> core.being_destroyed;
    if (widgetClass->composite == TRUE) {
		widget->composite.num_Children = 0;
		widget->composite.num_managed_Children = 0;
		widget->composite.children = NULL;
                }
    XtGetResources(widget,args,argCount);    
    XtRegisterTranslations(widget);
    widgetClass->initialize();
    return (widget);
}


void XtRealize (widget)
    
    Widget    widget
{
    Cardinal i;

   widget->widgetClass->realize();
   if ((widget->widgetClass->composite) && (i = widget->composite.num_Children != 0)) {
	for (i,i!=0,--i) 
		XtRealize(widget->composite.children[i-1]);
        if (widget->composite.num_Children == i = widget->composite.num_managed_Children)
		XMapSubwindows(widget);
	else while (i > 0) {
		if (widget->composite.children[i-1]->core.managed) {
			XMapWindow(widget->composite.children[i-1]);
			i--;
			} }
    if (widget->core.parent == NULL) XMapWindow(widget);
   return;
}
			
		
