#ifndef lint
static char *sccsid = "@(#)Command.c	1.13	2/25/87";
#endif lint

/*
 * Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.
 * 
 *                         All Rights Reserved
 * 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in 
 * supporting documentation, and that the name of Digital Equipment
 * Corporation not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.  
 * 
 * 
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */


#include <stdio.h>
#include <string.h>
#include "Xlib.h"
#include "Intrinsic.h"
#include "Label.h"
#include "Command.h"
#include "Atoms.h"

/* Private Definitions */

typedef void  (*CommandProc)();
        
typedef struct _WidgetRec {
	core_data	core;	/* commonWidget fields */
	label_data	super; 	/* label super-class fields */
	command_data	instance; /* command instance variables */
} WidgetRec, *Widget;

static char *defaultEventBindings[] = {
    "<ButtonPress>left:       set\n",
    "<ButtonRelease>left:      notify unset\n",
    "<EnterWindow>:             highlight\n",
    "<LeaveWindow>:             unhighlight unset\n",
    NULL
};

static Resources resources[] = {
    {XtNfunction, XtCFunction, XtRFunction,
	sizeof(CommandProc), (caddr_t)&data.proc, (caddr_t)NULL},
    {XtNparameter, XtCParameter, XrmRPointer,
	sizeof(caddr_t), (caddr_t)&data.tag, (caddr_t)NULL},
    {XtNsensitive, XtCBoolean, XrmRBoolean, sizeof(Boolean),
	 (caddr_t)&data.sensitive, NULL},
    {XtNgrayPixmap, XtCPixmap, XtRPixmap,
	sizeof(Pixmap),XtRString,"grey"},
    {XtNeventBindings, XtCEventBindings, XtREventBindings, sizeof(caddr_t),
        (caddr_t)&data.eventTable, XtREventTable,defaultEventBindings},
};

/****************************************************************
 *
 * Private Procedures
 *
 ****************************************************************/



/*ARGSUSED*/
static void Dummy(p)                  /* default call back proc */
caddr_t p;
{
    (void) printf("dummy call back for Command\n");
}

static void Ignore()
{}


/*
 * Build the gc's for the widget.
 */

#define gray_width	32
#define gray_height	32

static BuildGcs(data)
Widget data;
{


    valuemask = GCForeground | GCBackground | GCFont | GCFillStyle;
    values.font = data->super.fontstruct->fid;
    values.foreground = data->core.background_pixel;
    values.background = data->instance.foreground_pixel;
    values.fill_style = FillSolid;
    data->invgc = XtGetGC(data->core.dpy,  data->core.window,
			  valuemask, &values);
    values.foreground = data->instance.foreground_pixel;
    values.background = data->core.background_pixel;
    data->normgc = XtGetGC(data->core.dpy, widgetContext, data->instance.window,
			   valuemask, &values);
    valuemask |= GCStipple;
    values.fill_style = FillStippled;
    values.stipple = data->instance.grayPixamp;
    data->graygc = XtGetGC(data->core.dpy, widgetContext, data->core.window,
			   valuemask, &values);
}


/*
 *
 * Repaint the widget window
 *
 */

static void Redisplay(data)
Widget	data;
{
    int     textx, texty;
    Window  w = data->core.window;


    /*
     * Calculate Text x,y given window width and text width
     * to the specified justification
     */

    if (data->needsfreeing) return;

    if (data->super.justify == XtjustifyLeft) 
	textx = 2;
    else if (data->super.justify == XtjustifyRight)
	textx = data->core.width - data->super.twidth - 2;
    else
        textx = ((data->core.width - data->super.twidth) / 2);
    if (textx < 0) textx = 0;
    texty = (data->core.height - data->super.theight) / 2 +
	data->super.fontstruct->max_bounds.ascent;

    XFillRectangle(data->core.dpy, w,
		   ((data->instance.highlighted || data->instance.set)
		    ? data->instance.normgc : data->instance.invgc),
		   0, 0, data->core.width, data->core.height);

    if (data->instance.highlighted || data->instance.set)
	XFillRectangle(data->core.dpy, w,
		       data->instance.set ? data->instance.normgc : data->instance.invgc,
		       textx, (int) ((data->core.height - data->super.theight) / 2),
		       data->super.twidth, data->super.theight);

    XDrawString(data->core.dpy, w,
		data->instance.set ? data->instance.invgc : (data->core.sensitive ? data->core.normgc : data->instance.graygc),
		textx, texty, data->super.text, strlen(data->super.text));
}

extern void Destroy();

/*
 *
 * Generic widget event handler
 *
 */

void Resize(data)
Widget data;
{
    data->core.eventlevels++;
	    data->core.x = event->xconfigure.x;
	    data->core.y = event->xconfigure.y;
	    data->core.width = event->xconfigure.width;
	    data->core.height = event->xconfigure.height;
	    data->core.borderWidth = event->xconfigure.border_width;
}

void Expose(data)
Widget data;
{
	    if (event->xexpose.count == 0)
		Redisplay(data);

}


    data->core.eventlevels--;
    if (data->core.needsfreeing && data->core.eventlevels == 0) {
	XtClearAllEventHandlers(data->core.dpy, data->core.window);
        XtFree((char*)data->super.text);
        XtFree((char *) data);


/*
 *
 * Widget hilight event handler
 *
 */

static void Highlight(data)
Widget data;
{
    data->instance.highlighted = TRUE;
    Redisplay(data);
}

/*
 *
 * Widget un-hilight event handler
 *
 */

static void UnHighlight(data)
Widget data;
{
    data->instance.highlighted = FALSE;
    Redisplay(data);
}

/*
 *
 * Widget set event handler
 *
 */

static void Set(data)
Widget data;
{
    data->instance.set = TRUE;
    Redisplay(data);
}

/*
 *
 * Widget un-set event handler
 *
 */

static void UnSet(data)
Widget data;
{
    data->instance.set = FALSE;
    Redisplay(data);
}

/*
 *
 * Widget notify event handler
 *
 */

static void Notify(data)
Widget data;
{
    data->instance.notifying = TRUE;
    XFlush(data->core.dpy);
    data->instance.proc(data->instance.tag);
    data->instance.notifying = FALSE;
}

/*
 *
 * Destroy the widget; the window's been destroyed already.
 *
 */

static void Destroy(data)
    Widget	data;
{
    data->core.needsfreeing = TRUE;
}

/****************************************************************
 *
 * Public Procedures
 *
 ****************************************************************/

Widget XtCommandCreate(dpy, parent, args, argCount)
 Display  *dpy;
 Window   parent;
 ArgList  args;
 int      argCount;
 {
 Widget	data;
 XrmNameList  names;
 XrmClassList	classes;
 Drawable 	root;
 Position x, y;
 unsigned int depth;
 unsigned long valuemask;
 XSetWindowAttributes wvals;

    static XtActionsRec actionsTable[] = {
        {"set",		(caddr_t)Set},
        {"unset",	(caddr_t)UnSet},
        {"highlight",	(caddr_t)Highlight},
        {"unhighlight",	(caddr_t)UnHighlight},
        {"notify",	(caddr_t)Notify},
        {NULL, NULL}
    };

    data = (Widget) XtMalloc (sizeof (WidgetRec));

    /* Set Default Values */
    data.dpy = dpy;
    XtGetResources(dpy, resources, XtNumber(resources), args, argCount, parent,
	"command", "Command", &names, &classes);

    data->core.state = (TranslationPtr) XtSetActionBindings(
	data->core.dpy,
	data->core.eventTable, actionsTable, (caddr_t) Ignore);

    /* obtain text dimensions and calculate the window size */
    SetTextWidthAndHeight(data);
    if (data->core.width == 0) data->core.width = data->super.twidth + 2*data->super.ibw;
    if (data->core.height == 0) data->core.height = data->super.theight + 2*data->super.ibh;

    wvals.background_pixel = data->core.bgpixel;
    wvals.border_pixel = data->core.brpixel;
    wvals.bit_gravity = CenterGravity;
    
    valuemask = CWBackPixel | CWBorderPixel | CWBitGravity;
    
    BuildGcs(data);

    XtSetNameAndClass(data->core.dpy,&data, names, classes);
    XrmFreeNameList(names);
    XrmFreeClassList(classes);


}



/*
 * Set Attributes
 */

void XtCommandSetValues(dpy, window, args, argCount)
Display *dpy;
Window window;
ArgList args;
int argCount;
{
    Widget data;
    if (data == NULL) return;

    data = *data;
    XtSetValues(resources, XtNumber(resources), args, argCount);

    if (strcmp (data->super.text, data.text)
	  || data->super.fontstruct != data.fontstruct) {
	XtGeometryReturnCode reply;
	WindowBox reqbox, replybox;

	data.text = strcpy(
	    XtMalloc ((unsigned) strlen(data.text) + 1),
	    data.text);
        XtFree ((char *) data->super.text);

	/* obtain text dimensions and calculate the window size */
	SetTextWidthAndHeight(&data);
	reqbox.width = data.twidth + 2*data.ibw;
	reqbox.height = data.theight + 2*data.ibh;
	reply = XtMakeGeometryRequest(data.dpy, data.mywin, XtgeometryResize, 
				      &reqbox, &replybox);
	if (reply == XtgeometryAlmost) {
	    reqbox = replybox;
	    (void) XtMakeGeometryRequest(data.dpy, data.mywin, XtgeometryResize, 
					 &reqbox, &replybox);
	}
    }

    if (data->core.fgpixel != data.fgpixel ||
	data->core.bgpixel != data.bgpixel ||
	data->super.fontstruct != data.fontstruct) BuildGcs(&data);
    *data = data;
    if (!data->core.sensitive) data->instance.set = data->instance.highlighted = FALSE;
    Redisplay (data);
}

