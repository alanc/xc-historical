/* $XConsortium: Hooks.c,v 1.6 94/01/14 17:56:13 kaleb Exp $ */

/*
 * Copyright 1993 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */

/*LINTLIBRARY*/

#include "IntrinsicI.h"

static void FreeBlockHookList( widget, closure, call_data )
    Widget widget;		/* unused (and invalid) */
    XtPointer closure;		/* ActionHook* */
    XtPointer call_data;	/* unused */
{
    BlockHook list = *(BlockHook*)closure;
    while (list != NULL) {
	BlockHook next = list->next;
	XtFree( (XtPointer)list );
	list = next;
    }
}


XtBlockHookId XtAppAddBlockHook( app, proc, closure )
    XtAppContext app;
    XtBlockHookProc proc;
    XtPointer closure;
{
    BlockHook hook = XtNew(BlockHookRec);
    LOCK_APP(app);
    hook->next = app->block_hook_list;
    hook->app = app;
    hook->proc = proc;
    hook->closure = closure;
    if (app->block_hook_list == NULL) {
	_XtAddCallback( &app->destroy_callbacks,
		        FreeBlockHookList,
		        (XtPointer)&app->block_hook_list
		      );
    }
    app->block_hook_list = hook;
    UNLOCK_APP(app);
    return (XtBlockHookId)hook;
}


void XtRemoveBlockHook( id )
    XtBlockHookId id;
{
    BlockHook *p, hook = (BlockHook)id;
    XtAppContext app = hook->app;
    LOCK_APP(app);
    for (p = &app->block_hook_list; p != NULL && *p != hook; p = &(*p)->next);
    if (p == NULL) {
#ifdef DEBUG
	XtAppWarningMsg(app, "badId", "xtRemoveBlockHook", XtCXtToolkitError,
			"XtRemoveBlockHook called with bad or old hook id",
			(String*)NULL, (Cardinal*)NULL);
#endif /*DEBUG*/	
	UNLOCK_APP(app);
	return;
    }
    *p = hook->next;
    XtFree( (XtPointer)hook );
    UNLOCK_APP(app);
}

static void DeleteShellFromHookObj(shell, closure, call_data)
    Widget shell;
    XtPointer closure, call_data;
{
    /* app_con is locked when this function is called */
    int ii, jj;
    HookObject ho = (HookObject) closure;

    for (ii = 0; ii < ho->hooks.num_shells; ii++)
	if (ho->hooks.shells[ii] == shell) {
	    /* collapse the list */
	    for (jj = ii; jj < ho->hooks.num_shells; jj++) {
		if ((jj+1) < ho->hooks.num_shells)
		    ho->hooks.shells[jj] = ho->hooks.shells[jj+1];
	    }
	    break;
	}
    ho->hooks.num_shells--;
}

#define SHELL_INCR 4

#if NeedFunctionPrototypes
void _XtAddShellToHookObj(
    Widget shell)
#else
void _XtAddShellToHookObj(shell)
    Widget shell;
#endif
{
    /* app_con is locked when this function is called */
    HookObject ho = (HookObject) XtHooksOfDisplay(XtDisplay(shell));

    if (ho->hooks.num_shells == ho->hooks.max_shells) {
	ho->hooks.max_shells += SHELL_INCR;
	ho->hooks.shells = 
	    (WidgetList)XtRealloc((char*)ho->hooks.shells, 
		ho->hooks.max_shells * sizeof (Widget));
    }
    ho->hooks.shells[ho->hooks.num_shells++] = shell;

    XtAddCallback(shell, XtNdestroyCallback, DeleteShellFromHookObj, 
		  (XtPointer)ho);
}

#if NeedFunctionPrototypes
Boolean _XtIsHookObject(
    Widget widget)
#else
Boolean _XtIsHookObject(widget)
    Widget widget;
#endif
{
    return (widget->core.widget_class == hookObjectClass);
}

Widget XtHooksOfDisplay(dpy)
    Display* dpy;
{
    extern Widget _XtCreateHookObj();
    Widget retval;
    XtPerDisplay pd;
    DPY_TO_APPCON(dpy);

    LOCK_APP(app);
    pd = _XtGetPerDisplay(dpy);
    if (pd->hook_object == NULL)
	pd->hook_object = 
	    _XtCreateHookObj((Screen*)DefaultScreenOfDisplay(dpy));
    retval = pd->hook_object;
    UNLOCK_APP(app);
    return retval;
}
