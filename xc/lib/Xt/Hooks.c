/* $XConsortium: Hooks.c,v 1.4 94/01/07 20:01:50 kaleb Exp $ */

/*LINTLIBRARY*/

#include "IntrinsicI.h"
#include "StringDefs.h"

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

void _XtAddShellToHookObj(shell)
    Widget shell;
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

Widget XtHooksOfDisplay(dpy)
    Display* dpy;
{
    extern Widget _XtCreate();
    Widget retval;
    XtPerDisplay pd;
    DPY_TO_APPCON(dpy);

    LOCK_APP(app);
    pd = _XtGetPerDisplay(dpy);
    if (pd->hook_object == NULL)
	pd->hook_object = _XtCreate("hooks", "Hooks", hookObjectClass, 
			(Widget) NULL, (Screen*)DefaultScreenOfDisplay(dpy),
			(ArgList)NULL, 0, (XtTypedArgList)NULL, 0,
			(ConstraintWidgetClass) NULL);
    retval = pd->hook_object;
    UNLOCK_APP(app);
    return retval;
}
