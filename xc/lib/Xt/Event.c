
void XtSetEventHandler(widget, eventMask, proc,closure,other)
    Widget	    widget;
    unsigned long   eventMask;
    XtEventHandler  proc;
    caddr_t	    closure;
    Boolean         other;
{
   register ProcPtr p,p1;
   register unsigned long tempMask;

   tempMask = 0;
   p = widget -> core.eventTable;
   while (p != NULL && p->proc != proc && p->closure != closure) {
         tempMask |= p->mask;
         p1 = p;
         p = p -> next;
   }
   if (p == NULL) {
         if (eventMask == 0 && other == FALSE) return;
         p = (ProcPtr) XtMalloc(sizeof(ProcRec));
         p ->next = widget-> core.eventTable;
         widget -> core.eventTable = p;
         p -> proc = proc;
         p -> closure = closure;
         widget -> core.eventMask |= eventMask;
         p ->mask = eventMask;
         p ->non_filter = other;

    }
    else {
         if (eventMask == 0 && other == FALSE) {
             p2 -> next = p -> next;
             XtFree (char*)p;
             p = p2;
         }  
         else {
               p->mask = eventMask;
               p->non_filter = other;
          }
	 while (p != NULL) {
          tempMask |= p -> mask;
          p = p-> next;
         }
         if (widget->core.expose != NULL) tempMask |= ExposureMask;
         widget->core.eventMask = tempMask;
     }

     if (widget->core.window != NULL) 
            XSelectInput(widget->core.dpy, widget->core.window,widget->core.event_mask);

}

void XtDispatchEvent (event)
    XEvent  event;

{
    Widget widget;
    unsigned long mask;
    unsigned long grabType;
    Boolean sensitivity;
#define IsSensitive ((!sensitivity) || (widget->core.sensitive && widget ->ancestor.sensitive))

    ConvertWindowToWidget (event->xany.window, &widget);
    if (widget == NULL) {
         XtErrorMessage("XtDispatchEvent - no widget registered);
         return;
    }
    ConvertTypeToMask(eventType, &mask, &grabType, &sensitivity);
    if ((grabType == pass || grabList == NULL) && IsSensitive)
           DispatchEvent(event,widget, mask);
    elsif (onGrabList) {
           if (IsSensitive) DispatchEvent(event,widget,mask);
           else DispatchEvent(event, grabList->widget, mask);
           }  
    elsif (grabType == remap)
           DispatchEvent(event,grabList->widget, mask)
    return;

Boolean onGrabList (widget)
    Widget widget;

{
   GrabPtr gl;
    for ( widget; widget != NULL; widget = widget -> parent) {
        for (gl = grabList; gl != NULL && gl->exclsive; gl = gl ->next) 
             if (gl -> widget == widget) return (TRUE);
    }
    return (FALSE);
}


void DispatchEvent(event, widget, mask)
    XEvent    event;
    Widget    widget;
    unsigned long mask;


{
    ProcPtr p;   
    for (p; p != NULL; p = p -> next) {
	if ((mask && p->mask != 0) || (mask==0 && p->non_filter == TRUE)) 
              p->proc(event,closure);
         }
    return;
}

void ConvertTypeToMask (eventType, mask, grabType, sensitivity)
    unsigned long eventType;
    unsigned long *mask;
    unsigned long *grabType;
    Boolean       *sensitivity;

{
   mask = typeArray[eventType].mask;
   grabType = typeArray[eventType].grabType;
   sensitivity = typeArray[eventType].sensitivity;
   return;
}
