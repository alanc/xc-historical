/* $XConsortium: copyright.h,v 1.14 95/04/13 16:08:25 dpw Exp $ */

/******************************************************************************
 * 
 * Copyright (c) 1994, 1995  Hewlett-Packard Company
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL HEWLETT-PACKARD COMPANY BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
 * THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 * Except as contained in this notice, the name of the Hewlett-Packard
 * Company shall not be used in advertising or otherwise to promote the
 * sale, use or other dealings in this Software without prior written
 * authorization from the Hewlett-Packard Company.
 * 
 * $Header: Xdbe.c,v 1.2 95/04/10 13:12:22 yip Exp $
 *
 *     Xlib DBE code
 *
 *****************************************************************************/


#define NEED_EVENTS
#define NEED_REPLIES
#include <X11/Xlibint.h>
#include "Xext.h"
#include "extutil.h"
#define NEED_DBE_PROTOCOL
#include "Xdbe.h"

static XExtensionInfo _dbe_info_data;
static XExtensionInfo *dbe_info = &_dbe_info_data;
static char *dbe_extension_name = DBE_PROTOCOL_NAME;

#define DbeCheckExtension(dpy,i,val) \
  XextCheckExtension (dpy, i, dbe_extension_name, val)
#define DbeSimpleCheckExtension(dpy,i) \
  XextSimpleCheckExtension (dpy, i, dbe_extension_name)


/*****************************************************************************
 *                                                                           *
 *			   private utility routines                          *
 *                                                                           *
 *****************************************************************************/

/*
 * find_display - locate the display info block
 */
static int close_display();
static char *error_string();
static XExtensionHooks dbe_extension_hooks = {
    NULL,				/* create_gc */
    NULL,				/* copy_gc */
    NULL,				/* flush_gc */
    NULL,				/* free_gc */
    NULL,				/* create_font */
    NULL,				/* free_font */
    close_display,			/* close_display */
    NULL,				/* wire_to_event */
    NULL,				/* event_to_wire */
    NULL,				/* error */
    error_string,			/* error_string */
};

static char *dbe_error_list[] = {
    "BadBuffer",			/* DbeBadBuffer */
};

static XEXT_GENERATE_FIND_DISPLAY (find_display, dbe_info,
				   dbe_extension_name, 
				   &dbe_extension_hooks, 
				   DbeNumberEvents, NULL)

static XEXT_GENERATE_CLOSE_DISPLAY (close_display, dbe_info)

static XEXT_GENERATE_ERROR_STRING (error_string, dbe_extension_name,
				   DbeNumberErrors, 
				   dbe_error_list)


/*****************************************************************************
 *                                                                           *
 *		       Double-Buffering public interfaces                    *
 *                                                                           *
 *****************************************************************************/

/* 
 * XdbeQueryExtension -
 *	Sets major_version_return and minor_verion_return to the major and
 *	minor DBE protocol version supported by the server.  If the DBE
 *	library is compatible with the version returned by the server, this
 *	function returns non-zero.  If dpy does not support the DBE
 *	extension, or if there was an error during communication with the
 *	server, or if the server and library protocol versions are
 *	incompatible, this functions returns zero.  No other Xdbe functions
 *	may be called before this function.   If a client violates this rule,
 *	the effects of all subsequent Xdbe calls are undefined.
 */
Status XdbeQueryExtension (dpy, major_version_return, minor_version_return)
    Display *dpy;
    int *major_version_return, *minor_version_return;
{
    XExtDisplayInfo *info = find_display (dpy);
    xDbeGetVersionReply rep;
    register xDbeGetVersionReq *req;

    if (!XextHasExtension (info))
        return (Status)0; /* failure */

    LockDisplay (dpy);
    DbeGetReq (DbeGetVersion, req, info);
    req->majorVersion = DBE_MAJOR_VERSION;
    req->minorVersion = DBE_MINOR_VERSION;

    if (!_XReply (dpy, (xReply *) &rep, 0, xTrue)) {
	UnlockDisplay (dpy);
	SyncHandle ();
	return (Status)0; /* failure */
    }
    *major_version_return = rep.majorVersion;
    *minor_version_return = rep.minorVersion;
    UnlockDisplay (dpy);

    SyncHandle ();

    if (*major_version_return != DBE_MAJOR_VERSION)
        return (Status)0; /* failure */
    else
        return (Status)1; /* success */
}


/*
 * XdbeAllocateBackBuffer -
 *	This function returns a drawable ID used to refer to the back buffer
 *	of the specified window.  The swap_action is a hint to indicate the
 *	swap action that will likely be used in subsequent calls to
 *	XdbeSwapBuffers.  The actual swap action used in calls to
 *	XdbeSwapBuffers does not have to be the same as the swap_action
 *	passed to this function, though clients are encouraged to provide
 *	accurate information whenever possible.
 */

/*## If NeedFunctionPrototypes is defined, swap_action parameter causes
 *## compiler failure if the function is defined as
 *##
 *## XdbeBackBuffer XdbeAllocateBackBufferName(dpy, window, swap_action)
 *##     Display *dpy;
 *##     Window window;
 *##     XdbeSwapAction swap_action;
 *##
 *## This is because swap_action is defined as an int on the first
 *## line by the compiler, but this type does not match the function
 *## prototype in Xdbe.h which defines swap_action as an unsigned char.
 *##*/

#if NeedFunctionPrototypes
XdbeBackBuffer XdbeAllocateBackBufferName(
    Display *dpy,
    Window window,
    XdbeSwapAction swap_action)
#else
XdbeBackBuffer XdbeAllocateBackBufferName (dpy, window, swap_action)
    Display *dpy;
    Window window;
    XdbeSwapAction swap_action;
#endif
{
    XExtDisplayInfo *info = find_display (dpy);
    register xDbeAllocateBackBufferNameReq *req;
    XdbeBackBuffer buffer;

    /* make sure extension is available; if not, return the
     * third parameter (0).
     */
    DbeCheckExtension (dpy, info, (XdbeBackBuffer)0);

    /* allocate the id */
    buffer = XAllocID (dpy);

    LockDisplay(dpy);
    DbeGetReq(DbeAllocateBackBufferName, req, info);
    req->window = window;
    req->swapAction = (unsigned char)swap_action;
    req->buffer = buffer;

    UnlockDisplay (dpy);
    SyncHandle ();
    return buffer;

} /* XdbeAllocateBackBufferName() */

/*
 * XdbeDeallocateBackBufferName - 
 *	This function frees a drawable ID, buffer, that was obtained via
 *	XdbeAllocateBackBufferName.  The buffer must refer to the back buffer
 *	of the specified window, or a protocol error results.
 */
Status XdbeDeallocateBackBufferName (dpy, buffer)
    Display *dpy;
    XdbeBackBuffer buffer;
{
    XExtDisplayInfo *info = find_display (dpy);
    register xDbeDeallocateBackBufferNameReq *req;

    DbeCheckExtension (dpy, info, (Status)0 /* failure */);

    LockDisplay (dpy);
    DbeGetReq (DbeDeallocateBackBufferName, req, info);
    req->buffer = buffer;
    UnlockDisplay (dpy);
    SyncHandle ();

    return (Status)1; /* success */
}


/*
 * XdbeSwapBuffers - 
 *	This function swaps the front and back buffers for a list of windows.
 *	The argument num_windows specifies how many windows are to have their
 *	buffers swapped; it is the number of elements in the swap_info array.
 *	The argument swap_info specifies the information needed per window
 *	to do the swap.
 */
Status XdbeSwapBuffers (dpy, swap_info, num_windows)
    Display *dpy;
    XdbeSwapInfo *swap_info;
    int num_windows;
{
    XExtDisplayInfo *info = find_display (dpy);
    register xDbeSwapBuffersReq *req;

    DbeCheckExtension (dpy, info, (Status)0 /* failure */);

    LockDisplay (dpy);
    DbeGetReq (DbeSwapBuffers, req, info);
    req->length += 2*num_windows;
    req->n = num_windows;
    PackData32 (dpy, swap_info, num_windows * sizeof(XdbeSwapInfo));
    UnlockDisplay (dpy);
    XSync (dpy, False);

    /*## always return 1 -- is this correct? */
    return (Status)1; /* success */

} /* XdbeSwapBuffers() */


/*
 * XdbeBeginIdiom -
 *	This function marks the beginning of an idiom sequence.
 */
Status XdbeBeginIdiom (dpy)
    Display *dpy;
{
    XExtDisplayInfo *info = find_display(dpy);
    register xDbeBeginIdiomReq *req;

    DbeCheckExtension (dpy, info, (Status)0 /* failure */);

    LockDisplay (dpy);
    DbeGetReq (DbeBeginIdiom, req, info);
    UnlockDisplay (dpy);
    SyncHandle ();

    /*## always return 1 -- is this correct? */
    return (Status)1; /* success */
}


/*
 * XdbeEndIdiom -
 *	This function marks the end of an idiom sequence.
 */
Status XdbeEndIdiom (dpy)
    Display *dpy;
{
    XExtDisplayInfo *info = find_display(dpy);
    register xDbeEndIdiomReq *req;

    DbeCheckExtension (dpy, info, (Status)0 /* failure */);

    LockDisplay (dpy);
    DbeGetReq (DbeEndIdiom, req, info);
    UnlockDisplay (dpy);
    SyncHandle ();

    /*## always return 1 -- is this correct? */
    return (Status)1; /* success */
}


/*
 * XdbeGetVisualInfo -
 *	This function returns information about which visuals support
 *	double buffering.  The argument num_screens specifies how many
 *	elements there are in the screen_specifiers list.  Each drawable
 *	in screen_specifiers designates a screen for which the supported
 *	visuals are being requested.  If num_screens is zero, information
 *	for all screens is requested.  In this case, upon return from this
 *	function, num_screens will be set to the number of screens that were
 *	found.  If an error occurs, this function returns NULL, else it returns
 *	a pointer to a list of XdbeScreenVisualInfo structures of length
 *	num_screens.  The nth element in the returned list corresponds to the
 *	nth drawable in the screen_specifiers list, unless num_screens was
 *	passed in with the value zero, in which case the nth element in the
 *	returned list corresponds to the nth screen of the server, starting
 *	with screen zero.
 */
XdbeScreenVisualInfo *XdbeGetVisualInfo (dpy, screen_specifiers, num_screens)
    Display        *dpy;
    Drawable       *screen_specifiers;
    int            *num_screens;  /* SEND and RETURN */
{
    XExtDisplayInfo *info = find_display(dpy);
    register xDbeGetVisualInfoReq *req;
    xDbeGetVisualInfoReply rep;
    XdbeScreenVisualInfo *scrVisInfo;
    int i;

    DbeCheckExtension (dpy, info, (XdbeScreenVisualInfo *)NULL);

    LockDisplay (dpy);

    DbeGetReq(DbeGetVisualInfo, req, info);
    req->length = 2 + *num_screens;
    req->n      = *num_screens;
    PackData32(dpy, screen_specifiers, (*num_screens * sizeof (Drawable)));

    if (!_XReply (dpy, (xReply *) &rep, 0, xFalse)) {
        UnlockDisplay (dpy);
        SyncHandle ();
        return NULL;
    }

    /* return the number of screens actually found if we
     * requested information about all screens (*num_screens == 0)
     */
    if (*num_screens == 0)
       *num_screens = rep.m;

    /* allocate list of visual information to be returned */
    if (!(scrVisInfo =
        (XdbeScreenVisualInfo *)Xmalloc(
        (unsigned)(*num_screens * sizeof(XdbeScreenVisualInfo))))) {
        UnlockDisplay (dpy);
        SyncHandle ();
        return NULL;
    }

    for (i = 0; i < *num_screens; i++)
    {
        int nbytes;
        int j;

        _XRead32 (dpy, (char *)&scrVisInfo[i].count, sizeof(int));

        nbytes = scrVisInfo[i].count * sizeof(XdbeVisualInfo);

        /* if we can not allocate the list of visual/depth info
         * then free the lists that we already allocate as well
         * as the visual info list itself
         */
        if (!(scrVisInfo[i].visinfo = (XdbeVisualInfo *)Xmalloc(
            (unsigned)nbytes))) {
            for (j = 0; j < i; j++) {
                Xfree ((char *)scrVisInfo[j].visinfo);
            }
            Xfree ((char *)scrVisInfo);
            UnlockDisplay (dpy);
            SyncHandle ();
            return NULL;
        }
    
        /* We can not read visual info item in directly because the
         * reply contains a pad byte.  Therefore, we must read each
         * component of the visual info item in separately.
         */

        for (j = 0; j < scrVisInfo[i].count; j++) {
            char data;

            _XRead (dpy, (char *)&(scrVisInfo[i].visinfo[j].visual),
                sizeof(VisualID));

            _XRead (dpy, (char *)&data, sizeof(char));
            scrVisInfo[i].visinfo[j].depth = (int)data;

            _XRead (dpy, (char *)&data, sizeof(char));
            scrVisInfo[i].visinfo[j].perflevel = (int)data;

            /* read unused bytes */
            _XRead (dpy, (char *)&data, sizeof(char));
            _XRead (dpy, (char *)&data, sizeof(char));
        }

    }

    UnlockDisplay (dpy);
    SyncHandle ();
    return scrVisInfo;

} /* XdbeGetVisualInfo() */


/*
 * XdbeFreeVisualInfo -
 *	This function frees the list of XdbeScreenVisualInfo returned by the
 *	function XdbeGetVisualInfo.
 */
void XdbeFreeVisualInfo(visual_info)
    XdbeScreenVisualInfo *visual_info;
{
    if (visual_info == NULL) {
        return;
    }

    if (visual_info->visinfo) {
        XFree(visual_info->visinfo);
    }

    XFree(visual_info);
}


/*
 * XdbeGetBackBufferAttributes -
 *	This function returns the attributes associated with the specified
 *	buffer.
 */
XdbeBackBufferAttributes *XdbeGetBackBufferAttributes(dpy, buffer)
    Display *dpy;
    XdbeBackBuffer buffer;
{
    XExtDisplayInfo *info = find_display(dpy);
    register xDbeGetBackBufferAttributesReq *req;
    xDbeGetBackBufferAttributesReply rep;
    XdbeBackBufferAttributes *attr;

    DbeCheckExtension(dpy, info, (XdbeBackBufferAttributes *)NULL);

    if (!(attr =
       (XdbeBackBufferAttributes *)Xmalloc(sizeof(XdbeBackBufferAttributes)))) {
        return NULL;
    }

    LockDisplay(dpy);
    DbeGetReq(DbeGetBackBufferAttributes, req, info);
    req->buffer = buffer;

    if (!_XReply (dpy, (xReply *) &rep, 0, xTrue)) {
        UnlockDisplay (dpy);
        SyncHandle ();
        return NULL;
    }
    attr->window = rep.attributes;

    UnlockDisplay (dpy);
    SyncHandle ();

    return attr;
}

