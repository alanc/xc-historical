#include "extutil.h"

XExtDisplayInfo *XextInitDisplay (display_listp, lastp, dpy,
				  ext_name, close_display, wire_to_event, 
				  event_to_wire, nevents, data)
    XExtDisplayInfo **display_list;
    XExtDisplayIfno **lastp;
    Display *dpy;
    char *ext_name;
    int (*close_display)();
    int (*wire_to_event)(), (*event_to_wire)();
    int nevents;
    caddr_t data;
{
    XExtDisplayInfo *dpyinfo;

    dpyinfo = (XExtDisplayInfo *) Xmalloc (sizeof (XExtDisplayInfo));
    if (!dpyinfo) return NULL;
    dpyinfo->next = (*display_listp);
    dpyinfo->display = dpy;
    dpyinfo->data = data;
    dpyinfo->codes = XInitExtension (dpy, ext_name);

    /*
     * if the server has the extension, then we can initialize the 
     * appropriate function vectors
     */
    if (dpyinfo->codes) {
	int i, j;

	XESetCloseDisplay (dpy, dpyinfo->codes->extension, close_proc);
	for (i = 0, j = dpyinfo->codes->first_event; i < nevents; i++, j++) {
	    XESetWireToEvent (dpy, j, wire_to_event);
	    XESetEventToWire (dpy, j, event_to_wire);
	}
    }

    /*
     * now, chain it onto the list
     */
    *display_listp = dpyinfo;
    *lastp = dpyinfo;
    return dpyinfo;
}


XExtDisplayInfo *XextFindDisplay (display_listp, lastp, dpy)
    XExtDisplayInfo **display_list;
    XExtDisplayIfno **lastp;
    Display *dpy;
{
    XExtDisplayInfo *dpyinfo = *lastp;

    /*
     * see if this was the most recently accessed display
     */
    if (dpyinfo && dpyinfo->display == dpy) return dpyinfo;


    /*
     * look for display in list
     */
    for (dpyinfo = *display_listp; dpyinfo; dpyinfo = dpyinfo->next) {
	if (dpyinfo->display == dpy) {
	    *lastp = dpyinfo;
	    return dpyinfo;
	}
    }

    return NULL;
}




int XextCloseDisplay (display_listp, lastp, dpy)
    XExtDisplayInfo **display_list;
    XExtDisplayIfno **lastp;
    Display *dpy;
{
    XExtDisplayInfo *dpyinfo, *prev;

    /*
     * locate this display and its back link so that it can be removed
     */
    prev = NULL;
    for (dpyinfo = (*display_listp); dpyinfo; dpyinfo = dpyinfo->next) {
	if (dpyinfo->display == dpy) break;
	prev = dpyinfo;
    }
    if (!dpyinfo) return 0;		/* hmm, actually an error */

    /*
     * remove the display from the list; handles going to zero
     */
    if (prev)
	prev->next = dpyinfo->next;
    else
	*display_listp = dpyinfo->next;

    if (dpyinfo == *lastp) *lastp = NULL;

    Xfree ((char *) dpyinfo);
    return 0;
}
