/*
 * $XConsortium: IMWrap.c,v 11.8 94/01/20 18:02:05 rws Exp $
 */

/*
 * Copyright 1991 by the Massachusetts Institute of Technology
 * Copyright 1991 by the Open Software Foundation
 * Copyright 1993, 1994 by the Sony Corporation
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Open Software Foundation, M.I.T. and
 * Sony Corporation not be used in advertising or publicity pertaining to 
 * distribution of the software without specific, written prior permission.  
 * Open Software Foundation, M.I.T. and Sony Corporation make no 
 * representations about the suitability of this software for any purpose.  
 * It is provided "as is" without express or implied warranty.
 *
 * OPEN SOFTWARE FOUNDATION, M.I.T. AND SONY CORPORATION DISCLAIM ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL OPEN
 * SOFTWARE FOUNDATIONN, M.I.T. OR SONY CORPORATION BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE. 
 * 
 *		 M. Collins		OSF  
 *               Makoto Wakamatsu       Sony Corporation
 */				

#include "Xlibint.h"
#include "Xlcint.h"

/*
 * Compile the resource name. (resource_name ---> xrm_name)
 */
void
_XIMCompileResourceList(res, num_res)
    register XIMResourceList res;
    unsigned int num_res;
{
    register unsigned int count;

    for (count = 0; count < num_res; res++, count++) {
	res->xrm_name = XrmStringToQuark(res->resource_name);
    }
}

void
_XCopyToArg(src, dst, size)
    XPointer src;
    XPointer *dst;
    register unsigned int size;
{
    if (!*dst) {
	union {
	    long	longval;
	    short	shortval;
	    char	charval;
	    char*	charptr;
	    XPointer	ptr;
	} u;
	if (size <= sizeof(XPointer)) {
	    memcpy((char *)&u, (char *)src, (int)size);
	    if (size == sizeof(long))	       *dst = (XPointer)u.longval;
	    else if (size == sizeof(short))    *dst = (XPointer)u.shortval;
	    else if (size == sizeof(char))     *dst = (XPointer)u.charval;
	    else if (size == sizeof(char*))    *dst = (XPointer)u.charptr;
	    else if (size == sizeof(XPointer)) *dst = (XPointer)u.ptr;
	    else memcpy( (char*)dst, (char*)src, (int)size );
	} else {
	    memcpy( (char*)dst, (char*)src, (int)size );
	}
    } else {
	memcpy( (char*)*dst, (char*)src, (int)size );
    }
}

/*
 * Connects to an input method matching current locale specification, creates
 * a XIM object and return a pointer the newly created XIM back to the caller.
 */

XIM 
XOpenIM( display, rdb, res_name, res_class )
    Display	*display;
    XrmDatabase	 rdb;
    char	*res_name;
    char	*res_class;
{
    XLCd	lcd = _XOpenLC( (char *)NULL );

    if( !lcd )
	return( (XIM)NULL );
    if( lcd->methods->open_im == NULL )
	_XInitIM( lcd );
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
    XLCd lcd = im->core.lcd;
  
    s = (im->methods->close) (im);
    for (ic = im->core.ic_chain; ic; ic = ic->core.next)
	ic->core.im = (XIM)NULL;
    Xfree ((char *) im);
    _XCloseLC (lcd);
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
    return im->core.lcd->core->name;
}

/*
 * Register to a input method instantiation callback to prepare the
 * on-demand input method instantiation.
 */
Bool
XRegisterIMInstantiateCallback( display, rdb, res_name, res_class, callback,
				client_data)
    Display	*display;
    XrmDatabase	 rdb;
    char	*res_name;
    char	*res_class;
    XIMProc	 callback;
    XPointer	*client_data;
{
    XLCd	lcd = _XOpenLC( (char *)NULL );

    if( !lcd )
	return( False );
    if( lcd->methods->register_callback == NULL )
	_XInitIM( lcd );
    return( (*lcd->methods->register_callback)( lcd, display, rdb, res_name,
						res_class, callback,
						client_data ) );
}

/*
 * Unregister to a input method instantiation callback.
 */
Bool
XUnregisterIMInstantiateCallback( display, rdb, res_name, res_class, callback,
				  client_data )
    Display	*display;
    XrmDatabase	 rdb;
    char	*res_name;
    char	*res_class;
    XIMProc	 callback;
    XPointer	*client_data;
{
    XLCd	lcd = _XlcCurrentLC();

    if( !lcd )
	return( False );
    if( lcd->methods->unregister_callback == NULL )
	return( False );
    return( (*lcd->methods->unregister_callback)( lcd, display, rdb, res_name,
						  res_class, callback,
						  client_data ) );
}

