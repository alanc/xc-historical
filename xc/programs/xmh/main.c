#ifndef lint
static char rcs_id[] = "$Header: main.c,v 1.7 87/10/09 14:01:38 weissman Exp $";
#endif lint
/*
 *			  COPYRIGHT 1987
 *		   DIGITAL EQUIPMENT CORPORATION
 *		       MAYNARD, MASSACHUSETTS
 *			ALL RIGHTS RESERVED.
 *
 * THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT NOTICE AND
 * SHOULD NOT BE CONSTRUED AS A COMMITMENT BY DIGITAL EQUIPMENT CORPORATION.
 * DIGITAL MAKES NO REPRESENTATIONS ABOUT THE SUITABILITY OF THIS SOFTWARE FOR
 * ANY PURPOSE.  IT IS SUPPLIED "AS IS" WITHOUT EXPRESS OR IMPLIED WARRANTY.
 *
 * IF THE SOFTWARE IS MODIFIED IN A MANNER CREATING DERIVATIVE COPYRIGHT RIGHTS,
 * APPROPRIATE LEGENDS MAY BE PLACED ON THE DERIVATIVE WORK IN ADDITION TO THAT
 * SET FORTH ABOVE.
 *
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting documentation,
 * and that the name of Digital Equipment Corporation not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission.
 */

/* main.c */

#define MAIN 1			/* Makes global.h actually declare vars */
#include "xmh.h"
#include <signal.h>


static XtIntervalId timerid;

/* This gets called every five minutes. */

static void NeedToCheckScans()
{
    int i;
    if (debug) {
	(void) fprintf(stderr, "[magic toc check ...");
	(void) fflush(stderr);
    }
    for (i = 0; i < numScrns; i++) {
	if (scrnList[i]->toc)
	    TocRecheckValidity(scrnList[i]->toc);
	if (scrnList[i]->msg)
	    TocRecheckValidity(MsgGetToc(scrnList[i]->msg));
    }
    if (debug) {(void)fprintf(stderr, "done]\n");(void)fflush(stderr);}
}



/*ARGSUSED*/
static void CheckMail(widget, closure, event)
Widget widget;
Opaque closure;
XEvent *event;
{
    static int count = 0;
    int i;
    if (event->type == ClientMessage &&
	    ((int)event->xclient.message_type) == XtTimerExpired) {
	timerid = XtAddTimeOut(toplevel, (int)60000);
	if (defNewMailCheck) {
if (debug) {(void)fprintf(stderr, "(Checking for new mail..."); (void)fflush(stderr);}
	    TocCheckForNewMail();
if (debug) (void)fprintf(stderr, "done)\n");
	}
	if (count++ % 5 == 0) {
	    NeedToCheckScans();
	    if (defMakeCheckpoints) {
if (debug) {(void)fprintf(stderr, "(Checkpointing..."); (void)fflush(stderr);}
		for (i=0 ; i<numScrns ; i++)
		    if (scrnList[i]->msg) 
			MsgCheckPoint(scrnList[i]->msg);
if (debug) (void)fprintf(stderr, "done)\n");
	    }
	}
    }
}

/* Main loop. */

main(argc, argv)
unsigned int argc;
char **argv;
{
    InitializeWorld(argc, argv);
    if (defNewMailCheck)
	TocCheckForNewMail();
    XtAddEventHandler(toplevel, (EventMask) 0, TRUE, CheckMail, (Opaque) NULL);
    timerid = XtAddTimeOut(toplevel, (int)60000);
    XtMainLoop();
}
