
/* $XConsortium: Hooks.c,v 1.2 93/09/30 10:11:25 kaleb Exp $ */
/************************************************************
Copyright 1993 by The Massachusetts Institute of Technology

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

********************************************************/

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
