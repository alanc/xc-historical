#ifndef lint
static char Xrcsid[] = "$XConsortium: Keyboard.c,v 1.5 89/12/14 12:36:18 swick Exp $";
#endif

/********************************************************

Copyright (c) 1988 by Hewlett-Packard Company
Copyright (c) 1987, 1988, 1989 by Digital Equipment Corporation, Maynard, 
              Massachusetts, and the Massachusetts Institute of Technology, 
              Cambridge, Massachusetts

Permission to use, copy, modify, and distribute this software 
and its documentation for any purpose and without fee is hereby 
granted, provided that the above copyright notice appear in all 
copies and that both that copyright notice and this permission 
notice appear in supporting documentation, and that the names of 
Hewlett-Packard, Digital or  M.I.T.  not be used in advertising or 
publicity pertaining to distribution of the software without specific, 
written prior permission.

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

********************************************************/

#include "IntrinsicI.h"
#include <X11/Xatom.h>
#include "StringDefs.h"

#include "PassivGraI.h"

#define _GetWindowedAncestor(w) (XtIsWidget(w) ? w : _XtWindowedAncestor(w))
extern void _XtFillAncestorList();
extern void _XtSendCrossingEvent();

static XtServerGrabPtr CheckServerGrabs(event, trace,
					traceDepth, pdi)
    XEvent	*event;
    Widget	*trace;
    Cardinal	traceDepth;
    XtPerDisplayInput pdi;
{
    XtServerGrabPtr 	grab;
    Cardinal		i;

    for (i = traceDepth;  i > 0; i--)
      {
	 if (grab = _XtCheckServerGrabsOnWidget(event, trace[i-1],
						KEYBOARD, pdi))
	   return (grab);
     }
    return (XtServerGrabPtr)0;
}

static Boolean IsParent(a, b)
    Widget	a, b;
{
    for (b = XtParent(b); b; b = XtParent(b)) {
      if (b == a) return TRUE;
      if (XtIsShell(b)) return FALSE;
    }
    return FALSE;
}

#define RelRtn(lca, type) {*relTypeRtn = type; return lca;}

static Widget CommonAncestor(a, b, relTypeRtn)
    register Widget a, b;
    XtGeneology  * relTypeRtn;
{
    if (a == b) 
      {
	  RelRtn(a, XtMySelf)
	}
    else if (IsParent(a, b))
      {
	  RelRtn(a, XtMyAncestor)
	}
    else if (IsParent(b, a))
      {
	  RelRtn(b, XtMyDescendant)
	}
    else
      for (b = XtParent(b);
	   b && !XtIsShell(b); 
	   b = XtParent(b))
	if (IsParent(b, a)) 
	  {
	      RelRtn(b, XtMyCousin)
	    }
    RelRtn(NULL, XtUnrelated)
}
#undef RelRtn

	  
    


static Widget _FindFocusWidget(widget, trace, traceDepth, activeCheck)
    Widget 	widget;
    Widget	*trace;
    Cardinal	traceDepth;
    Boolean	activeCheck;
{
    int i;
    int src;
    Widget dst;
    XtPerWidgetInput	pwi;
    Boolean	isAncestor;
    
    /* For each ancestor, starting at the top, see if it's forwarded */


    /* first check the trace list till done or we go to branch */
    isAncestor = TRUE;
    for (src = traceDepth-1, dst = widget;
	 ((src > 0) && isAncestor);
	 )
      {
	  if (pwi = _XtGetPerWidgetInput(trace[src], FALSE))
	    {
		if (pwi->focusKid)
		  {
		      dst = pwi->focusKid;
		      for (i = src-1; i >= 0 && trace[i] != dst; i--) {}
		      
		      if (i < 0) 
			isAncestor = FALSE;
		      else
			src = i;
		  }
		else dst = trace[--src];
	    }
	  else dst = trace[--src];
      }	

    if (!activeCheck)
      while (XtIsWidget(dst) && (pwi = _XtGetPerWidgetInput(dst, FALSE)) && pwi->focusKid)
	dst = pwi->focusKid;
    return dst;
}


static Widget FindFocusWidget(widget, pdi)
    Widget		widget;
    XtPerDisplayInput 	pdi; 
{
    if (pdi->focusWidget) 
      return  pdi->focusWidget;
    else
      return _FindFocusWidget(widget, pdi->trace, pdi->traceDepth, FALSE);
}


static Boolean IsOutside(e, w) 
    XKeyEvent	*e;
    Widget	w;
{
    int left, right, top, bottom;
    
    /*
     * if the pointer is outside the shell or inside
     * the window try to see if it would recieve the
     * focus 
     */
    XtTranslateCoords(w, 0, 0, &left, &top);
    /* We need to take borders into consideration */
    left = left - w->core.border_width;
    top =  top - w->core.border_width;
    right = left +  w->core.width + w->core.border_width;
    bottom = top +  w->core.height + w->core.border_width;

    if (
	(e->x_root < left) || (e->y_root < top) ||
        (e->x_root > right) || (e->y_root > bottom))
      return TRUE;
    else
      return FALSE;
}

static Widget 	FindKeyDestination(widget, event,
				   prevGrab, prevGrabType,
				   devGrab, devGrabType,
				   pdi)
    Widget		widget;
    XKeyEvent		*event;
    XtServerGrabPtr	prevGrab;
    XtServerGrabType 	prevGrabType;
    XtServerGrabPtr	devGrab;
    XtServerGrabType	devGrabType;
    XtPerDisplayInput	pdi;
{
    
    Widget		dspWidget;
    Widget		focusWidget;
    
    dspWidget = 
      focusWidget = 
	pdi->focusWidget = 
	  _GetWindowedAncestor(FindFocusWidget(widget, pdi));
    
    
    /*
     * If a grab is active from a previous activation then dispatch
     * based on owner_events ala protocol but with focus being
     * determined by XtSetKeyboardFocus.
     */
    if 	(IsAnyGrab(prevGrabType))
      {
	  if (prevGrab->ownerEvents) 
	    dspWidget = focusWidget;
	  else
	    dspWidget = prevGrab->widget;
      }
    else 
      {
	  /*
           * If the focus widget is the event widget or a descendant
	   * then we can avoid the rest of this. Else ugh...
	   */
	  if (focusWidget != widget) 
	    {
		XtGeneology 	ewRelFw; /* relationship of event widget to
					    focusWidget */
		Widget		lca;

		lca = CommonAncestor(widget, focusWidget, &ewRelFw);
		    
		/*
		 * if the event widget is an ancestor of focus due to the pointer
		 * and/or the grab being in an ancestor and it's a passive grab
		 * send to grab widget.
		 * we are also dispatching to widget if ownerEvents and the event
		 * is outside the client
		 */
		if ((ewRelFw == XtMyAncestor) &&
		    (devGrabType == XtPassiveServerGrab))
		     {
			 if (IsOutside(event, widget) ||
#ifdef OWNER_EVENTS_FIX
			     !devGrab->ownerEvents
#else
			     event->type ==KeyPress
#endif /* OWNER_EVENTS_FIX */
			     
			     )
			   dspWidget = devGrab->widget;
		     }
		else 
		  {
		      /*
		       * if the grab widget is not an ancestor of the focus
		       * release the grab in order to avoid locking. There
		       * is a possible case  in that ownerEvents true will fall
		       * through and if synch is set and the event widget
		       * could turn it off we'll lock. check for it ? why not
		       */
		      if ((ewRelFw != XtMyAncestor) 
			  && (devGrabType == XtPassiveServerGrab)
			  && (!IsAnyGrab(prevGrabType))
#ifdef OWNER_EVENTS_FIX
			  &&
			  (!devGrab->ownerEvents || 
			   (devGrab->keyboardMode == GrabModeSync))
#endif /* OWNER_EVENTS_FIX */
			  )
			{
			    XtUngrabKeyboard(devGrab->widget,
					     event->time);
			    devGrabType = XtNoServerGrab;
			}
		      /*
		       * if there isn't a grab with then check
		       * for a logical grab that would have been activated
		       * if the server was using Xt focus instead of server
		       * focus 
		       */
		      if (
#ifdef OWNER_EVENTS_FIX
			  IsAnyGrab(devGrabType) && 
			  (devGrab->ownerEvents)
#else
			  (!event->type == KeyPress)
#endif /* OWNER_EVENTS_FIX */
			  )
			dspWidget = focusWidget;
		      else
			{
			    static Display	*pseudoTraceDisplay = NULL;
			    static Widget	*pseudoTrace = NULL;
			    static Cardinal     pseudoTraceDepth = 0;
			    static Cardinal	pseudoTraceMax = 0;
			    XtServerGrabPtr	grab;

			    if (!pseudoTraceDepth || 
				!(pseudoTraceDisplay ==
				 XtDisplay(widget)) ||
				!(focusWidget == pseudoTrace[0]) ||
				!(lca == pseudoTrace[pseudoTraceDepth]))
			      {
				  /*
				   * fill ancestor list from lca
				   * (non-inclusive)to focusWidget by
				   * passing in lca as breakWidget
				   */
				  _XtFillAncestorList(&pseudoTrace, 
						   &pseudoTraceMax, 
						   &pseudoTraceDepth,
						   focusWidget,
						   lca);
				  pseudoTraceDisplay = XtDisplay(widget);
				  /* ignore lca */
				  pseudoTraceDepth--;
			      }
			    if (grab = CheckServerGrabs((XEvent*)event,
							pseudoTrace,
							pseudoTraceDepth, pdi))
			      {
				  XtDevice device = &pdi->keyboard;
				  
				  device->grabType = XtPseudoPassiveServerGrab;
				  pdi->activatingKey = event->keycode;
				  device->grab = *grab;

				  if (grab 
#ifdef OWNER_EVENTS_FIX
				      && !grab->ownerEvents
#endif /* OWNER_EVENTS_FIX */
				      )
				    dspWidget = grab->widget;
				  else
				    dspWidget = focusWidget;
			      }
			}
		  }
	    }
      }
    return dspWidget;
}

Widget _XtProcessKeyboardEvent(event, widget, pdi)
    XKeyEvent  *event;
    Widget widget;
    XtPerDisplayInput pdi;
{    
    XtDevice		device = &pdi->keyboard;
    XtServerGrabPtr	newGrab, devGrab = &device->grab;
    XtServerGrabRec	prevGrabRec;
    XtServerGrabType	prevGrabType = device->grabType;
    Widget		dspWidget = NULL;
    Boolean		deactivateGrab = FALSE;

    prevGrabRec = *devGrab;

    switch (event->type)
      {
	case KeyPress:
	  {
	      if (!IsServerGrab(device->grabType) && 
		  (newGrab = CheckServerGrabs((XEvent*)event,
					      pdi->trace,
					      pdi->traceDepth, pdi)))
		{
		    /*
		     * honor pseudo-grab from prior event by X
		     * unlocking keyboard. Not Xt Unlock !
		     */
		    if (IsPseudoGrab(prevGrabType))
		      XUngrabKeyboard(XtDisplay(newGrab->widget),
				      event->time);
		    else
		      {
			  /* Activate the grab */
			  device->grab = *newGrab;
			  pdi->activatingKey = event->keycode;
			  device->grabType = XtPassiveServerGrab;
		      }
		}
	    }
	  break;
	  
	case KeyRelease:
	  {
	      if (IsEitherPassiveGrab(device->grabType) && 
		  (event->keycode == pdi->activatingKey))
		deactivateGrab = TRUE;
	  }
	  break;
      }
    dspWidget = FindKeyDestination(widget, event,
				   &prevGrabRec, prevGrabType,
				   devGrab, device->grabType,
				   pdi);
    if (deactivateGrab)
      {
	  /* Deactivate the grab */
	  device->grabType = XtNoServerGrab;
	  pdi->activatingKey = 0;
      }
    return dspWidget;
}    
    
static Widget GetShell(widget)
    Widget	widget;
{
    Widget	shell;

    for (shell = widget; 
	 shell && !XtIsShell(shell);
	 shell = XtParent(shell)){}
    return shell;
}


/*
 * Check that widget really has Xt focus due to it having recieved an
 * event 
 */
static Boolean InActiveSubtree(widget)
    Widget	widget;
{
    static Widget	*pathTrace = NULL;
    static Cardinal     pathTraceDepth = 0;
    static Cardinal	pathTraceMax = 0;
    static Display	*display = NULL;
    
    if (!pathTraceDepth || 
	!(display == XtDisplay(widget)) ||
	!(widget == pathTrace[0]))
      {
	  _XtFillAncestorList(&pathTrace, 
			      &pathTraceMax, 
			      &pathTraceDepth,
			      widget,
			      NULL);
	  display = XtDisplay(widget);
      }
    if (widget == _FindFocusWidget(widget, 
				   pathTrace,
				   pathTraceDepth, 
				   TRUE))
      return TRUE;
    else
      return FALSE;
}




/* ARGSUSED */
void _XtHandleFocus(widget, client_data, event)
    Widget widget;
    caddr_t client_data;	/* child who wants focus */
    XEvent *event;
{
    XtPerDisplayInput 	pdi = _XtGetPerDisplayInput(XtDisplay(widget));
    XtPerWidgetInput	pwi = (XtPerWidgetInput)client_data;
    XtGeneology		oldFocalPoint = pwi->focalPoint;
    XtGeneology		newFocalPoint = pwi->focalPoint;

    switch( event->type ) {
	
      case KeyPress:
      case KeyRelease:
	/*
	 * We're getting the keyevents used to guarantee propagating
         * child interest ala ForwardEvent in R3
	 */
	return;

      case EnterNotify:
      case LeaveNotify:
	
	/*
	 * If operating in a focus driven model, then enter and
	 * leave events do not affect the keyboard focus.
	 */
	if ((event->xcrossing.detail != NotifyInferior)
	    &&	(event->xcrossing.focus))
	  {	      
	      switch (oldFocalPoint)
		{
		  case XtMyAncestor:
		    if (event->type == LeaveNotify)
		      newFocalPoint = XtUnrelated;
		  case XtUnrelated:
		    if (event->type == EnterNotify)
		      newFocalPoint = XtMyAncestor;
		  case XtMySelf:
		    break;
		  case XtMyDescendant:
		    break;
		    
		}	
	  }
	break;
      case FocusIn:
	switch (event->xfocus.detail)
	  {
	    case NotifyNonlinear:
	    case NotifyAncestor:
	    case NotifyInferior:
	      newFocalPoint = XtMySelf;
	      break;
	    case NotifyNonlinearVirtual:
	    case NotifyVirtual:
	      newFocalPoint = XtMyDescendant;
	      break;
	    case NotifyPointer:
	      newFocalPoint = XtMyAncestor;
	      break;
	  }
	break;
      case FocusOut:
	switch (event->xfocus.detail)
	  {
	    case NotifyPointer:
	    case NotifyNonlinear:
	    case NotifyAncestor:
	    case NotifyNonlinearVirtual:
	    case NotifyVirtual:
	      newFocalPoint = XtUnrelated;
	      break;
	    case NotifyInferior:
	      newFocalPoint = XtMyDescendant;
	      return;
	      break;
	  }
	break;
    }
    
    if (newFocalPoint != oldFocalPoint)
      {
	  Boolean 	add;
	  Widget	descendant = pwi->focusKid;

	  if ((oldFocalPoint == XtUnrelated) &&
	      InActiveSubtree(widget))
	    {
		pdi->focusWidget = NULL; /* invalidate the cache */
		pwi->haveFocus = TRUE;
		pwi->focalPoint = newFocalPoint;
		add = TRUE;
	    }
	  else if (newFocalPoint == XtUnrelated)
	    {
		pdi->focusWidget = NULL; /* invalidate the cache */
		pwi->haveFocus = FALSE;
		pwi->focalPoint = newFocalPoint;
		add = FALSE;
	    }
	  else
	    return;

	  if (descendant)
	    {
		if (add)
		  {
		      _XtSendCrossingEvent(descendant, FocusIn);
		  }
		else
		  {
		      _XtSendCrossingEvent(descendant, FocusOut);
		  }
	    }
      }
}

/* ARGSUSED */
static void FocusDestroyCallback(widget, closure, call_data)
    Widget  widget;
    caddr_t closure;		/* Widget */
    caddr_t call_data;
{
    XtSetKeyboardFocus((Widget)closure, None);
}

void XtSetKeyboardFocus(widget, descendant)
    Widget widget;
    Widget descendant;
{
    Widget windowedDesc;
    XtPerDisplayInput pdi = _XtGetPerDisplayInput(XtDisplay(widget));
    XtPerWidgetInput pwi = _XtGetPerWidgetInput(widget, TRUE);
    Widget oldDesc = pwi->focusKid;
    Widget oldWindowedDesc ;
    
    windowedDesc = descendant ? _GetWindowedAncestor(descendant) : NULL;
    oldWindowedDesc = oldDesc ? _GetWindowedAncestor(oldDesc) : NULL;
    
    if (descendant != oldDesc) {
	
	/* update the forward path */
	pwi->focusKid = descendant;


	/* all the rest handles focus ins and focus outs and misc gunk */
	
	if (oldDesc) {
	    XtRemoveCallback (oldDesc, XtNdestroyCallback, 
			      FocusDestroyCallback, (caddr_t) widget);
	    /*
	     * If there was a forward path then remove the handler if
	     * the path is being set to null and it isn't a shell.
	     * shells always have a handler for tracking focus for the
	     * hierarchy. 
	     */
	    if (!XtIsShell(widget) && !windowedDesc)
	      XtRemoveEventHandler(widget, XtAllEvents, True, 
				   _XtHandleFocus, (caddr_t)pwi);
	    /*
	     * If we had the focus send a focus out to the
	     * windowed parent if it's not also the new parent (from
	     * and to are gadgets).
	     */
	    if (
#ifdef NO_DOUBLE_FOCUS		
		(oldWindowedDesc != windowedDesc) && 
#endif
		pwi->haveFocus) {
		_XtSendCrossingEvent( oldWindowedDesc, FocusOut);
	    }
	}
	
	if (!descendant)
	  pwi->haveFocus = FALSE;
	else {
	    XtPerWidgetInput	psi;
	    Widget		shell;
	    EventMask	 	mask, eventMask, oldEventMask;

	    XtAddCallback (descendant, XtNdestroyCallback, 
			   FocusDestroyCallback, (caddr_t) widget);

	    /*
	     * shell borders are not occluded by the child, they're occluded
	     * by reparenting window managers. !!!
	     */
	    mask = FocusChangeMask | EnterWindowMask | LeaveWindowMask;
	    
	    /*
	     * we are having this handler server double duty as the
	     * hier of ForwardEvent in R3. One thing that is needed is
	     * to guarantee that the descendant gets keyevents if
	     * interested 
	     */
	    shell = GetShell(widget);
	    psi = _XtGetPerWidgetInput(shell, TRUE);
	    
	    eventMask = XtBuildEventMask(windowedDesc);
	    eventMask &= KeyPressMask | KeyReleaseMask;
	    eventMask |= mask;

	    if (oldWindowedDesc)
	      {
		  oldEventMask = XtBuildEventMask(oldWindowedDesc);
		  oldEventMask &= KeyPressMask | KeyReleaseMask;
		  oldEventMask |= mask;
		  if (oldEventMask != eventMask)
		    XtRemoveEventHandler(widget, 
					 oldEventMask, 
					 False, 
					 _XtHandleFocus, 
					 (caddr_t)pwi);
	      }
	    else
	      oldEventMask = 0;
	    
	    if (oldEventMask != eventMask)
	      XtAddEventHandler(widget, eventMask, False, 
				_XtHandleFocus, (caddr_t)pwi);
	    
	    if (!pwi->haveFocus) {
		if (psi->haveFocus) {
		    Window root, child;
		    int root_x, root_y, win_x, win_y, shellX, shellY;
		    int left, right, top, bottom;
		    unsigned int modMask;
		    Widget 	w = widget;
		    Boolean	maybe = FALSE;
		    
		    
		    /*
		     * if the pointer is outside the shell or inside
		     * the window try to see if it would recieve the
		     * focus 
		     */
		    XtTranslateCoords(shell, 0, 0, &shellX, &shellY);
		    /* We need to take borders into consideration */
		    left = top = -((int) shell->core.border_width);
		    right = (int) (shell->core.width + (shell->core.border_width << 1));
		    bottom = (int) (shell->core.height + (shell->core.border_width << 1));
		    
		    XQueryPointer(XtDisplay(w), XtWindow(w), &root, &child,
				  &root_x, &root_y, &win_x, &win_y, &modMask );
		    
		    if (root_x >= left && root_x < right &&
			root_y >= top && root_y < bottom) {
			
			/* We need to take borders into consideration */
			left = top = -((int) w->core.border_width);
			right = (int) (w->core.width + (w->core.border_width << 1));
			bottom = (int) (w->core.height + (w->core.border_width << 1));
			
			if (win_x >= left && win_x < right &&
			    win_y >= top && win_y < bottom)
			  maybe = TRUE;
		    }
		    else maybe = TRUE;
		    
		    /*
		     * if the hierarchy has the focus and the widget
		     * is a focus candidate, then see if it is a
		     * descendant of the focus path
		     */
		    if (maybe && (FindFocusWidget(w, pdi) == w))
		      pwi->haveFocus = TRUE;
		    
		}
	    }
	    if (
#ifdef NO_DOUBLE_FOCUS
		(oldWindowedDesc != windowedDesc) &&
#endif 
		pwi->haveFocus)
	      {
		  pdi->focusWidget = NULL; /* invalidate the cache */
		  _XtSendCrossingEvent( windowedDesc, FocusIn);
	      }
	}
    }
}	
