#include "Intrinsic.h"

void XtSetEventHandler(widget, eventMask,other, proc,closure)
    Widget	    widget;
    EventMask   eventMask;
    XtEventHandler  proc;
    caddr_t	    closure;
    Boolean         other;
{
   register EventRec *p,**pp;
   register EventMask tempMask;

   tempMask = 0;
   pp = & widget -> core.event_table;
   p = *pp;
   while (p != NULL && p->proc != proc && p->closure != closure) {
         tempMask |= p->mask;
         pp = &p->next;
         p = *pp;
   }
   if (p == NULL) {
	/* new proc to add to list */
         if (eventMask == 0 && other == FALSE) return;
         p = (EventRec*) XtMalloc(sizeof(EventRec));
         p ->next = widget->core.event_table;
         widget -> core.event_table = p;
         p -> proc = proc;
         p -> closure = closure;
         tempMask = widget -> core.event_mask | eventMask;
         p ->mask = eventMask;
         p ->non_filter = other;

    }
    else {
	/* it was there, either override or delete */
         if (eventMask == 0 && other == FALSE) {
	    /* delete */
	     *pp = p->next;
             XtFree ((char*)p);
             p = *pp;
         }  
         else {
	    /* override */
               p->mask = eventMask;
               p->non_filter = other;
          }
	 while (p != NULL) {
          tempMask |= p -> mask;
          p = p-> next;
         }
    }     
    if (widget->core.widget_class->core_class.expose != NULL)
       tempMask |= ExposureMask;
    if (widget->core.widget_class->core_class.visible_interest) 
       tempMask |= VisibilityChangeMask;
    widget->core.event_mask = tempMask;
     

     if (widget->core.window != NULL) 
         XSelectInput(widget->core.screen->display,widget->core.window,widget->core.event_mask);

}

typedef struct _HashRec *HashPtr;

typedef struct _HashRec {
    Window	window;
    Widget	widget;
    HashPtr	next;
} HashRec;

int sizes[] = {1009, 2003, 4007, 8017, 16033, 32063, 64151, 128257};
#define NUMSIZES 8

typedef struct {
    unsigned int	sizeIndex;
    unsigned int	size;
    unsigned int	count;
    HashPtr		entries[1];
} HashTableRec, *HashTable;

static HashTable table = NULL;

static void ExpandTable();

void RegisterWindow(window, widget)
    Window window;
    Widget widget;
{
    HashPtr hp, *hpp;
    unsigned int hashValue;

    if ((table->count + (table->count / 5)) >= table->size) ExpandTable();

    hpp = &table->entries[(unsigned int)window % table->size];
    hp = *hpp;

    while (hp != NULL) {
        if (hp->window == window) {
	    if (hp->widget != hp->widget)
		XtWarning("Attempt to change already registered window.\n");
	    return;
	}
        hpp = &hp->next;
	hp = *hpp;
    }

    hp = *hpp = (HashPtr) XtMalloc(sizeof(HashRec));
    hp->window = window;
    hp->widget = widget;
    hp->next = NULL;
    table->count++;
}
void UnregisterWindow(window, widget)
    Window window;
    Widget widget;
{
    HashPtr hp, *hpp;
    unsigned int hashValue;

    hpp = &table->entries[(unsigned int)window % table->size];
    hp = *hpp;

    while (hp != NULL) {
        if (hp->window == window) {
	    if (hp->widget != hp->widget) {
		XtWarning("Unregister-window does not match widget.\n");
                return;
                }
             else /* found entry to delete */
                  (*hpp) = hp ->next;
                  XtFree((char*)hp);
                  table->count--;
                  return;
	}
        hpp = &hp->next;
	hp = *hpp;
    }
    
}

static void ExpandTable()
{
    HashTable	oldTable = table;
    unsigned int i;

    if (oldTable->sizeIndex == NUMSIZES) return;

    table = (HashTable) XtMalloc(
	sizeof(HashTableRec)+sizes[oldTable->sizeIndex+1]*sizeof(HashRec));
    table->sizeIndex = oldTable->sizeIndex+1;
    table->size = sizes[table->sizeIndex];
    table->count = oldTable->count;
    for (i = 0; i<oldTable->size; i++) {
	HashPtr hp;
	hp = oldTable->entries[i];
	while (hp != NULL) {
	    HashPtr temp = hp;
	    RegisterWindow(hp->window, hp->widget);
	    hp = hp->next;
	    XtFree((char *) temp);
	}
    }
    XtFree((char *)oldTable);
}


Widget ConvertWindowToWidget(window)
    Window window;
{
    Widget widget;
    HashPtr hp;

    for (
        hp = table->entries[(unsigned int)window % table->size];
        hp != NULL;
	hp = hp->next)
	if (hp->window == window) return hp->widget;
}

void InitializeHash()
{
    int i;

    table = (HashTable) XtMalloc(
        sizeof(HashTableRec)+sizes[0]*sizeof(HashPtr));

    table->sizeIndex = 0;
    table->size = sizes[0];
    table->count = 0;
    for (i=0; i<table->size; i++) table->entries[i] = NULL;
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

    widget =ConvertWindowToWidget (event->xany.window);
    if (widget == NULL) {
         XtError("XtDispatchEvent - no widget registered");
         return;
    }
    ConvertTypeToMask(event->xany.type, &mask, &grabType, &sensitivity);
    if ((grabType == pass || grabList == NULL) && IsSensitive)
           DispatchEvent(event,widget, mask);
    else if (onGrabList(widget)) {
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
        {KeymapStateMask,ignore,not_sensitive},/*KeymapNotify*/
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
        {0,pass,not_sensitive},			/*CirculateRequest*/
        {PropertyChangeMask,ignore,not_sensitive}, /*PropertyNotify*/
        {0,ignore,not_sensitive},			/*SelectionClear*/
        {0,ignore,not_sensitive},			/*SelectionRequest*/
        {StructureNotifyMask,pass,not_sensitive}, /*SelectionNotify*/
        {ColormapChangeMask,ignore,not_sensitive}, /*ColormapNotify*/
        {0,ignore,not_sensitive},			/*ClientMessage*/
        {0 ,ignore,not_sensitive},              /*MappingNotify*/
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
      if ((widget->core.widget_class->core_class.compress_exposure)
        && (event->xexpose.count != 0)) 
        return;
      if(widget->core.widget_class->core_class.expose != NULL)
         widget->core.widget_class->core_class.expose (widget,event,NULL);
    }
    if ((mask == VisibilityNotify) &&
            !(widget->core.widget_class->core_class.visible_interest)) return;

    for (p=widget->core.event_table; p != NULL; p = p -> next) {
	if ((mask && p->mask != 0) || (mask==0 && p->non_filter == TRUE)) 
              p->proc(widget,event,p->closure);
         }
    return;
}



void XtMainLoop()
{
    XEvent event;

    for (;;) {
    	XtNextEvent(&event);
	XtDispatchEvent(&event);
    }
}


void EventInitialize()
{
    grabList = NULL;
    DestroyList = NULL;
    InitializeHash();
}
