#if !defined(lint) && !defined(SABER)
static char rcs_id[] =
    "$XConsortium: mlist.c,v 2.8 88/09/06 17:23:22 jim Exp $";
#endif
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
 * IF THE SOFTWARE IS MODIFIED IN A MANNER CREATING DERIVATIVE COPYRIGHT
 * RIGHTS, APPROPRIATE LEGENDS MAY BE PLACED ON THE DERIVATIVE WORK IN
 * ADDITION TO THAT SET FORTH ABOVE.
 *
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Digital Equipment Corporation not be
 * used in advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.
 */

/* mlist.c -- functions to deal with message lists. */

#include "xmh.h"


/* Create a message list containing no messages. */

MsgList MakeNullMsgList()
{
    MsgList mlist;
    mlist = XtNew(MsgListRec);
    mlist->nummsgs = 0;
    mlist->msglist = XtNew(Msg);
    mlist->msglist[0] = NULL;
    return mlist;
}


/* Append a message to the given message list. */

void AppendMsgList(mlist, msg)
  MsgList mlist;
  Msg msg;
{
    mlist->nummsgs++;
    mlist->msglist =
	(Msg *) XtRealloc((char *) mlist->msglist,
			  (unsigned) (mlist->nummsgs + 1) * sizeof(Msg));
    mlist->msglist[mlist->nummsgs - 1] = msg;
    mlist->msglist[mlist->nummsgs] = 0;
}



/* Delete a message from a message list. */

void DeleteMsgFromMsgList(mlist, msg)
MsgList mlist;
Msg msg;
{
    int i;
    for (i=0 ; i<mlist->nummsgs ; i++) {
	if (mlist->msglist[i] == msg) {
	    mlist->nummsgs--;
	    for (; i<mlist->nummsgs ; i++)
		mlist->msglist[i] = mlist->msglist[i+1];
	    return;
	}
    }
}



/* Create a new messages list containing only the one given message. */

MsgList MakeSingleMsgList(msg)
  Msg msg;
{
    MsgList result;
    result = MakeNullMsgList();
    AppendMsgList(result, msg);
    return result;
}


/* We're done with this message list; free it's storage. */

void FreeMsgList(mlist)
   MsgList mlist;
{
    XtFree((char *) mlist->msglist);
    XtFree((char *) mlist);
}



/* Parse the given string into a message list.  The string contains mh-style
   message numbers.  This routine assumes those messages numbers refer to
   messages in the given toc. */

MsgList StringToMsgList(toc, str)
Toc toc;
char *str;
{
    MsgList mlist;
    char *ptr;
    int first, second, i;
    Msg msg;
    mlist = MakeNullMsgList();
    while (*str) {
        while (*str == ' ')
            str++;
        first = second = atoi(str);
        str++;
        for (ptr = str; *ptr >= '0' && *ptr <= '9'; ptr++) ;
        if (*ptr == '-')
            second = atoi(ptr + 1);
        if (first > 0) {
            for (i = first; i <= second; i++) {
		msg = TocMsgFromId(toc, i);
                if (msg) AppendMsgList(mlist, msg);
	    }
	}
        str = ptr;
    }
    return mlist;
}
