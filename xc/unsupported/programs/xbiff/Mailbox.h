/*
 * $XConsortium: Mailbox.h,v 1.9 88/09/26 11:20:29 jim Exp $
 *
 * Copyright 1988 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * Author:  Jim Fulton, MIT X Consortium
 */

#ifndef _XtMailbox_h
#define _XtMailbox_h

/*
 * Mailbox widget; looks a lot like the clock widget, don't it...
 */

/* resource names used by mailbox widget that aren't defined in StringDefs.h */

#define XtNupdate		"update"
#define XtNfile			"file"
#define XtNcheckCommand "checkCommand"	/* command to exec for mail check */
#define XtNvolume "volume"		/* Int: volume for bell */
#define XtNbell "bell"			/* Boolean: t:ring bell, f:don't */

#define XtCCheckCommand "CheckCommand"
#define XtCVolume "Volume"
#define XtCBell "Bell"


/* structures */

typedef struct _MailboxRec *MailboxWidget;  /* see MailboxP.h */
typedef struct _MailboxClassRec *MailboxWidgetClass;  /* see MailboxP.h */


extern WidgetClass mailboxWidgetClass;

#endif _XtMailbox_h
/* DON'T ADD STUFF AFTER THIS #endif */
