
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
			
		
void XtSetSensitivity(widget,sensitive)
    Widget    widget;
    Boolean   sensitive;
{
    widget->core.sensitive = sensitive;
    if ((widget->core.sensitive == widget->ancestor.sensitive) \
                 && IsSubClassOf (composite)) && i = widget->composite.numChildren != 0) 
      for (i,i != 0, --i) XtSetSensitivity (widget->composite.Children[i-1],sensitive);
      
}

void XtDestroy (widget)
    Widget    widget;

{
    DestroyChildren(widget);
    XtAddCallback( DestroyList, XtPhase2Destroy, widget, NULL);

}

void DestroyChildren (widget)
    Widget    widget;
{
    int i;
    if (widget->core.being_destroyed) return;
    widget-> core.being_destroyed = TRUE;
    if (IsSubClassOf (composite) && i = widget->composite.num_Children != 0)
        for (i, i != 0, --i) DestroyChildren (widget->composite.Children[i-1]);
    return;
}


void XtPhase2Destroy ()

{
    if (widget->core.parent != NULL) XtCompositeRemoveChild(widget);
    XtCallCallbacks(widget->destroy_list);
    Phase2ChildrenCallbacks(widget);
    Phase2ChildrenDestroy(widget);
    widget->core.destroy();
    if (widget->window != NULL) XDestroyWindow(widget->core.window);
    return;
}

void Phase2ChildrenCallbacks(widget);
    Widget    widget;
{
    if (IsSubClassOf (composite) && i = widget->composite.num_Children != 0)
       for (i, i!=0, --i) {
           XtCallCallbacks(widget->composite.children[i-1]->destroy_list);
           Phase2ChildrenCallbacks(widget->composite.Children[i-1]);
       }
     return;
}

void Phase2ChildrenDestroy(widget);
    Widget    widget;
{
    if (IsSubClassOf (composite) && i = widget->composite.num_Children != 0)
       for (i, i!=0, --i) {
           Phase2ChildrenDestroy(widget->composite.Children[i-1]);
           widget->composite.Children[i-1]->destroy();
       }
     return;
}


void XtNextEvent(dpy, event)
Display *dpy;
XEvent *event;
{
	struct Select_event *se_ptr;
	struct Timer_event *te_ptr;
	struct timeval cur_time;
	struct timezone cur_timezone;
	Fd_set rmaskfd, wmaskfd, emaskfd;
	int  nfound, i;
	struct timeval wait_time;
	struct timeval *wait_time_ptr;
	int Claims_X_is_pending = 0;
	XClientMessageEvent *ev = (XClientMessageEvent *)event;
	extern void perror(), exit();

    if (DestroyList != NULL){
       XtCallCallbacks (DestroyList);
       XtRemoveAllCallback(DestroyList);
    }
	
    for(;;) {
        if(XPending(dpy) || Claims_X_is_pending) {
	    XNextEvent(dpy, event);
	    return;
	}
	if((se_ptr = outstanding_queue) != NULL) {
	    *event = *((XEvent *)&se_ptr->Se_event);
	    outstanding_queue = se_ptr->Se_oq;
	    return;
	}
	(void) gettimeofday(&cur_time, &cur_timezone);
	if(Timer_queue)  /* check timeout queue */
	  {
	      if (ISAFTER(&Timer_next, &cur_time)){
		  /* timer has expired */
		  ev->type = ClientMessage;
		  ev->display = dpy;
		  ev->window =  Timer_queue->Te_wID;
		  ev->message_type = XtTimerExpired;
		  ev->format = 32;
		  ev->data.l[0] = (int)Timer_queue->Te_cookie;
		  te_ptr = Timer_queue;
		  Timer_queue = Timer_queue->Te_next;
		  te_ptr->Te_next = NULL;
		  if(Timer_queue) /* set up next time out time */
		    Timer_next = Timer_queue->Te_tim.ti_value;
		  ReQueueTimerEvent(te_ptr);
		  return;
	      }
      }/* No timers ready time to wait */
		/* should be done only once */
	if(ConnectionNumber(dpy) +1 > composite.nfds) 
	  composite.nfds = ConnectionNumber(dpy) + 1;
	while(1) {
		FD_SET(ConnectionNumber(dpy),&composite.rmask);
		if (Timer_queue) {
			TIMEDELTA(&wait_time, &Timer_next, &cur_time);
			wait_time_ptr = &wait_time;
		} else 
		  wait_time_ptr = (struct timeval *)0;
		rmaskfd = composite.rmask;
		wmaskfd = composite.wmask;
		emaskfd = composite.emask;
		if((nfound=select(composite.nfds,
				  (int *)&rmaskfd,(int *)&wmaskfd,
				  (int *)&emaskfd,wait_time_ptr)) == -1) {
			if(errno == EINTR)
			  continue;
		}
		if(nfound == -1) {
			perror("select:");
			exit(-1);
		}
		break;

	}
	if(nfound == 0)
	  continue;
	for(i = 0; i < composite.nfds && nfound > 0;i++) {
	    if(FD_ISSET(i,&rmaskfd)) {
	      if(i == ConnectionNumber(dpy)){
		Claims_X_is_pending= 1;
		nfound--;
	      } else {
		Select_rqueue[i] -> Se_oq = outstanding_queue;
		outstanding_queue = Select_rqueue[i];
		nfound--;
	      }
	    }
	    if(FD_ISSET(i,&wmaskfd)) {
	      Select_rqueue[i] -> Se_oq = outstanding_queue;
	      outstanding_queue = Select_rqueue[i];
	      nfound--;
	    }
	    if(FD_ISSET(i,&emaskfd)) {
	      Select_rqueue[i] -> Se_oq = outstanding_queue;
	      outstanding_queue = Select_rqueue[i];
	      nfound--;
	    }
	 }
    }
}
