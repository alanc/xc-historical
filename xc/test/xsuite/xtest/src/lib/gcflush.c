/*
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology and
 * UniSoft Group Limited.
 * 
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the names of MIT and UniSoft not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  MIT and UniSoft
 * make no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * $XConsortium$
 */
#include <sys/types.h>
#include "Xlibint.h"
#include "xtest.h"
#include	"pixval.h"

/*
 * Flush the gc cache.
 */
void
gcflush(dpy, gc)
Display *dpy;
GC gc;
{
	 LockDisplay(dpy);
	_XFlushGCCache(dpy,gc);
	 UnlockDisplay(dpy);
	 SyncHandle();
}
