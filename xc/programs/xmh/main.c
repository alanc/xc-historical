/* $XConsortium: main.c,v 2.23 91/07/13 22:55:27 converse Exp $
 *
 *
 *		       COPYRIGHT 1987, 1989
 *		   DIGITAL EQUIPMENT CORPORATION
 *		       MAYNARD, MASSACHUSETTS
 *			ALL RIGHTS RESERVED.
 *
 * THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT NOTICE AND
 * SHOULD NOT BE CONSTRUED AS A COMMITMENT BY DIGITAL EQUIPMENT CORPORATION.
 * DIGITAL MAKES NO REPRESENTATIONS ABOUT THE SUITABILITY OF THIS SOFTWARE FOR
 * ANY PURPOSE.  IT IS SUPPLIED "AS IS" WITHOUT EXPRESS OR IMPLIED WARRANTY.
 *
 * IF THE SOFTWARE IS MODIFIED IN A MANNER CREATING DERIVATIVE COPYRIGHT
 * RIGHTS, APPROPRIATE LEGENDS MAY BE PLACED ON THE DERIVATIVE WORK IN
 * ADDITION TO THAT SET FORTH ABOVE.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Digital Equipment Corporation not be
 * used in advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.
 */

#define MAIN 1			/* Makes global.h actually declare vars */
#include "xmh.h"

/*ARGSUSED*/
static void NeedToCheckScans(client_data, id)
    XtPointer client_data;
    XtIntervalId *id;		/* unused */
{
    int i;
    (void) XtAppAddTimeOut((XtAppContext)client_data,
			   (unsigned long) app_resources.rescan_interval,
			   NeedToCheckScans, client_data);
    if (!subProcessRunning) {
        DEBUG("[magic toc check ...")
	for (i = 0; i < numScrns; i++) {
	    if (scrnList[i]->toc)
		TocRecheckValidity(scrnList[i]->toc);
	    if (scrnList[i]->msg)
		TocRecheckValidity(MsgGetToc(scrnList[i]->msg));
	}
        DEBUG(" done]\n")
    }
}

/*ARGSUSED*/
static void Checkpoint(client_data, id)
    XtPointer client_data;
    XtIntervalId *id;		/* unused */
{
    register int i;
    (void) XtAppAddTimeOut((XtAppContext)client_data,
			   (unsigned long) app_resources.checkpoint_interval,
			   Checkpoint, client_data);
    if (!subProcessRunning) {
	DEBUG("(Checkpointing...")
   	for (i=0; i<numScrns; i++)
	    if (scrnList[i]->msg) 
		MsgCheckPoint(scrnList[i]->msg);
        DEBUG(" done)\n")
    }
}

/*ARGSUSED*/
static void CheckMail(client_data, id)
    XtPointer client_data;
    XtIntervalId *id;		/* unused */
{
    extern void XmhCheckForNewMail();

    (void) XtAppAddTimeOut((XtAppContext)client_data,
			   (unsigned long) app_resources.mail_interval,
			   CheckMail, client_data);
    if (!subProcessRunning) {
        DEBUG("(Checking for new mail...")
        XmhCheckForNewMail(NULL, NULL, NULL, NULL);
        DEBUG(" done)\n")
    }
}

/* Main loop. */

#ifdef DEBUG_CLEANUP
Boolean ExitLoop = FALSE;
#endif

main(argc, argv)
int argc;
char **argv;
{
    XtAppContext appCtx;

    InitializeWorld(argc, argv);
    subProcessRunning = False;
    appCtx = XtWidgetToApplicationContext(toplevel);

    if (app_resources.new_mail_check && app_resources.mail_interval > 0) {
	app_resources.mail_interval *= 60000;
	(void) XtAppAddTimeOut(appCtx,
			       (unsigned long) app_resources.mail_interval,
			       CheckMail, (XtPointer)appCtx);
    }
    if (app_resources.rescan_interval > 0) {
	app_resources.rescan_interval *= 60000;
	(void) XtAppAddTimeOut(appCtx,
			       (unsigned long) app_resources.rescan_interval,
			       NeedToCheckScans, (XtPointer)appCtx);
    }
    if (app_resources.make_checkpoints &&
	app_resources.checkpoint_interval > 0) {
	app_resources.checkpoint_interval *= 60000;
	(void) XtAppAddTimeOut(appCtx, (unsigned long)
			       app_resources.checkpoint_interval,
			       Checkpoint, (XtPointer)appCtx);
    }

    lastInput.win = -1;		/* nothing mapped yet */
#ifdef DEBUG_CLEANUP
    while (!ExitLoop) {
#else
    for (;;) {
#endif
	XEvent ev;
	XtAppNextEvent( appCtx, &ev );
	if (ev.type == KeyPress) {
	    lastInput.win = ev.xany.window;
	    lastInput.x = ev.xkey.x_root;
	    lastInput.y = ev.xkey.y_root;
	} else if (ev.type == ButtonPress) {
	    lastInput.win = ev.xany.window;
	    lastInput.x = ev.xbutton.x_root;
	    lastInput.y = ev.xbutton.y_root;
	}
	XtDispatchEvent( &ev );
    }
#ifdef DEBUG_CLEANUP
    XtDestroyApplicationContext(appCtx);
    exit(0);
#endif    
}
