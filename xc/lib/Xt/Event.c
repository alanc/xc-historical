#include "Intrinsic.h"

void XtSetEventHandler(widget, eventMask, proc,closure,other)
    Widget	    widget;
    EventMask   eventMask;
    XtEventHandler  proc;
    caddr_t	    closure;
    Boolean         other;
{
   register EventRec *p,*p1;
   register EventMask tempMask;

   tempMask = 0;
   p = widget -> core.event_table;
   while (p != NULL && p->proc != proc && p->closure != closure) {
         tempMask |= p->mask;
         p1 = p;
         p = p -> next;
   }
   if (p == NULL) {
         if (eventMask == 0 && other == FALSE) return;
         p = (EventRec*) XtMalloc(sizeof(EventRec));
         p ->next = widget->core.event_table;
         widget -> core.event_table = p;
         p -> proc = proc;
         p -> closure = closure;
         widget -> core.event_mask |= eventMask;
         p ->mask = eventMask;
         p ->non_filter = other;

    }
    else {
         if (eventMask == 0 && other == FALSE) {
             p1 -> next = p -> next;
             XtFree ((char*)p);
             p = p1;
         }  
         else {
               p->mask = eventMask;
               p->non_filter = other;
          }
	 while (p != NULL) {
          tempMask |= p -> mask;
          p = p-> next;
         }
         if (widget->core.widget_class->coreClass.expose != NULL) tempMask |= ExposureMask;
         if (widget->core.widget_class->coreClass.visible_interest) 
                                                  tempMask |= VisibilityChangeMask;
         widget->core.event_mask = tempMask;
     }

     if (widget->core.window != NULL) 
         XSelectInput(widget->core.screen->display,widget->core.window,widget->core.event_mask);

}

void ConvertWindowToWidget(window,widget)
    Window  window;
    Widget *widget;
{
}
extern void ConvertTypeToMask();
extern Boolean onGrabList();
extern void DispatchEvent();

void XtDispatchEvent (event)
    XEvent  *event;

{
    Widget widget;
    EventMask mask;
    GrabType grabType;
    Boolean sensitivity;
#define IsSensitive ((!sensitivity) || (widget->core.sensitive && widget ->core.ancestor_sensitive))

    ConvertWindowToWidget (event->xany.window, &widget);
    if (widget == NULL) {
         XtError("XtDispatchEvent - no widget registered");
         return;
    }
    ConvertTypeToMask(event->xany.type, &mask, &grabType, &sensitivity);
    if ((grabType == pass || grabList == NULL) && IsSensitive)
           DispatchEvent(event,widget, mask);
    else if (onGrabList) {
           if (IsSensitive) DispatchEvent(event,widget,mask);
           else DispatchEvent(event, grabList->widget, mask);
           }  
    else if (grabType == remap)
           DispatchEvent(event,grabList->widget, mask);
    return;
}

Boolean onGrabList (widget)
    Widget widget;

{
   GrabRec* gl;
    for (; widget != NULL;(CompositeWidget)widget = widget->core.parent) {
        for (gl = grabList; gl != NULL && gl->exclusive; gl = gl ->next) 
             if (gl -> widget == widget) return (TRUE);
    }
    return (FALSE);
}

void ConvertTypeToMask (eventType,mask,grabType,sensitive)
    unsigned long eventType;
    EventMask *mask;
    GrabType *grabType;
    Boolean *sensitive;
    
{

static MaskRec masks[] = {
	{0,pass,not_sensitive},	/* should never see type = 0*/
        {0,pass,not_sensitive},   /* should never see type = 1*/
        {KeyPressMask,remap,is_sensitive}, /*KeyPress*/
        {KeyReleaseMask,remap,is_sensitive}, /*KeyRelease*/
        {ButtonPressMask,remap,is_sensitive}, /*ButtonPress*/
        {ButtonReleaseMask,remap,is_sensitive}, /*ButtonRelease*/
        {PointerMotionMask | Button1MotionMask | Button2MotionMask |
            Button3MotionMask | Button4MotionMask | Button5MotionMask | ButtonMotionMask,
           ignore,is_sensitive},		/*MotionNotify*/
        {EnterWindowMask,ignore,is_sensitive}, /*EnterNotify*/
        {LeaveWindowMask,ignore,is_sensitive}, /*LeaveNotify*/
        {FocusChangeMask,ignore,is_sensitive}, /*FocusIn*/
        {FocusChangeMask,ignore,is_sensitive}, /*FocusOut*/
        {ExposureMask,pass,not_sensitive},    /*Expose*/
        {0,pass,not_sensitive},               /*GraphicsExpose*/
        {0,pass,not_sensitive},               /*NoExpose*/
        {VisibilityChangeMask,pass,not_sensitive}, /*VisibilityNotify*/
        {0,pass,not_sensitive},		    /*CreateNotify should never come in*/
        {StructureNotifyMask,pass,not_sensitive}, /*DestroyNotify*/
        {StructureNotifyMask,pass,not_sensitive}, /*UnmapNotify*/
        {StructureNotifyMask,pass,not_sensitive}, /*MapNotify*/
        {0,pass,not_sensitive},			/*MapRequest*/
        {StructureNotifyMask,pass,not_sensitive}, /*ReparentNotify*/
        {StructureNotifyMask,pass,not_sensitive}, /*ConfigureNotify*/
        {0,pass,not_sensitive},			/*ConfigureRequest*/
        {StructureNotifyMask,pass,not_sensitive}, /*GravityNotify*/
        {0,pass,not_sensitive},			/*ResizeRequest*/
        {StructureNotifyMask,pass,not_sensitive}, /*CirculateNotify*/
        {0,pass,not_sensitive},			/*ConfigureRequest*/
        {PropertyChangeMask,ignore,not_sensitive}, /*PropertyNotify*/
        {0,ignore,not_sensitive},			/*SelectionClear*/
        {0,ignore,not_sensitive},			/*SelectionRequest*/
        {StructureNotifyMask,pass,not_sensitive}, /*SelectionNotify*/
        {ColormapChangeMask,ignore,not_sensitive}, /*ColormapNotify*/
        {0,ignore,not_sensitive}			/*ClientMessage*/
  };
	(*mask) = masks[eventType].mask;
        (*grabType) = masks[eventType].grabType;
        (*sensitive) = masks[eventType].sensitive;
   return;
};

void DispatchEvent(event, widget, mask)
    XEvent    *event;
    Widget    widget;
    unsigned long mask;


{
    EventRec *p;   
    if (mask == ExposureMask) {
              if ((widget->core.compress_exposure) && (event->xexpose.count != 0)) return;
              if(widget->core.widget_class->coreClass.expose != NULL)
                   widget->core.widget_class->coreClass.expose ();
    }
    if ((mask == VisibilityNotify) &&
                       !(widget->core.widget_class->coreClass.visible_interest)) return;
    for (p; p != NULL; p = p -> next) {
	if ((mask && p->mask != 0) || (mask==0 && p->non_filter == TRUE)) 
              p->proc(event,p->closure);
         }
    return;
}


