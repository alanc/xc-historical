
void XtSetEventHandler(widget, eventMask, proc,closure,other)
    Widget	    widget;
    unsigned long   eventMask;
    XtEventHandler  proc;
    caddr_t	    closure;
    Boolean         other;
{

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
