/*
 * $XConsortium: XIMWrap.c,v 11.2 91/04/06 18:47:25 rws Exp $
 */

/*
 * Copyright 1991 by the Massachusetts Institute of Technology
 * Copyright 1991 by the Open Software Foundation
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Open Software Foundation and M.I.T.
 * not be used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  Open Software
 * Foundation and M.I.T. make no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without express or
 * implied warranty.
 *
 * OPEN SOFTWARE FOUNDATION AND M.I.T. DISCLAIM ALL WARRANTIES WITH REGARD TO
 * THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL OPEN SOFTWARE FOUNDATIONN OR M.I.T. BE
 * LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES 
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 *		 M. Collins		OSF  
 */				

#include "Xlibint.h"
#include "Xlcint.h"

/*
 * Connects to an input method matching current locale specification, creates
 * a XIM object and return a pointer the newly created XIM back to the caller.
 */

XIM 
XOpenIM(display, rdb, res_name, res_class)
    Display *display;
    XrmDatabase rdb;
    char *res_name;
    char *res_class;
{
    XLCd lcd = _XlcCurrentLC();

    if (!lcd)
	return (XIM) NULL;
    return (*lcd->methods->open_im) (lcd, display, rdb, res_name, res_class);
}

/*
 * Close the connection to the input manager, and free the XIM structure
 */
Status
XCloseIM(im)
    XIM im;
{
    Status s;
    XIC ic;
  
    s = (im->methods->close) (im);
    for (ic = im->core.ic_chain; ic; ic = ic->core.next)
	ic->core.im = (XIM)NULL;
    Xfree ((char *) im);
    return (s);
}

/*
 * Return the Display associated with the input method.
 */
Display *
XDisplayOfIM(im)
    XIM im;
{
    return im->core.display;
}

/*
 * Return the Locale associated with the input method.
 */
char *
XLocaleOfIM(im)
    XIM im;
{
    return im->core.lcd->core.name;
}
