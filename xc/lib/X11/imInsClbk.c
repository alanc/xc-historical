/* $XConsortium$ */
/******************************************************************

           Copyright 1993 by Sony Corporation

Permission to use, copy, modify, distribute, and sell this software
and its documentation for any purpose is hereby granted without fee,
provided that the above copyright notice appear in all copies and
that both that copyright notice and this permission notice appear
in supporting documentation, and that the name of Sony Corporation
 not be used in advertising or publicity pertaining to distribution
 of the software without specific, written prior permission.
Sony Corporation makes no representations about the suitability of
 this software for any purpose. It is provided "as is" without
 express or implied warranty.

SONY CORPORATION DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
EVENT SHALL SONY CORPORATION BE LIABLE FOR ANY SPECIAL, INDIRECT OR
CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
PERFORMANCE OF THIS SOFTWARE.

  Author:   Makoto Wakamatsu   Sony Corporation
                               makoto@sm.sony.co.jp

******************************************************************/

#include	<X11/Xatom.h>
#define NEED_EVENTS
#include	"Xlibint.h"
#include	"Xlcint.h"
#include	"XlcPublic.h"
#include	"Ximint.h"
#include	"XimInsClbk.h"

extern XIM _XimOpenIM(
#if NeedFunctionPrototypes
    XLCd lcd,
    Display *dpy,
    XrmDatabase rdb,
    char *res_name,
    char *res_class
#endif
);

Private XimInstCallback	callback_list	= NULL;
Private XimInstCallback destory_list	= NULL;
Private Window		requestor	= None;
Private Bool		lock		= False;

Private void
#if NeedFunctionPrototype
MakeLocale( XLCd lcd, char locale[] )
#else
MakeLocale( lcd, locale )
    XLCd	lcd;
    char	locale[];
#endif
{
    char	*language, *territory, *codeset;

    _XlcGetLCValues( lcd, XlcNLanguage, &language, XlcNTerritory, &territory,
		     XlcNCodeset, &codeset, NULL );

    strcpy( locale, language );
    if( territory  &&  *territory ) {
	strcat( locale, "_" );
	strcat( locale, territory );
    }
    if( codeset  &&  *codeset ) {
	strcat( locale, "." );
	strcat( locale, codeset );
    }
}


Private Bool
#if NeedFunctionPrototypes
CallCallback( Display *display, Atom locale )
#else
CallCallback( display, locale )
    Display	*display;
    Atom	 locale;
#endif
{
    char		*str;
    int			 length;
    Bool		 flag = False;
    XimInstCallback	 icb, picb, tmp;

    if( !(str = XGetAtomName( display, locale )) )
	return( False );

    length = strlen( XIM_LOCAL_CATEGORY );
    if( strncmp( str, XIM_LOCAL_CATEGORY, length ) )
	return( False );
    str += length;

    str = strtok( str, "," );
    lock = True;
    do {
	for( icb = callback_list; icb; icb = icb->next ) {
	    if( icb->name  &&  !icb->call  &&  !icb->destroy  &&
		!strcmp( str, icb->name ) ) {
		flag = True;
		icb->call = True;
		icb->callback( icb->display, icb->client_data, NULL );
	    }
	}
    } while( str = strtok( NULL, "," ) );

    for( icb = callback_list, picb = NULL; icb; ) {
	if( icb->destroy ) {
	    if( picb )
		picb->next = icb->next;
	    else
		callback_list = icb->next;
	    tmp = icb;
	    icb = icb->next;
	    XFree( tmp );
	}
	else {
	    picb = icb;
	    icb = icb->next;
	}
    }
    lock = False;
    return( flag );
}


Private Bool
CheckSNEvent( display, event, arg )
    Display	*display;
    XEvent	*event;
    XPointer	 arg;
{
    if( event->type == SelectionNotify  &&
	(Window)arg == event->xselection.requestor )
	return( True );
    return( False );
}


Private Bool
_XimFilterPropertyNotify( display, window, event, client_data )
    Display	*display;
    Window	 window;
    XEvent	*event;
    XPointer	 client_data;
{
    Atom		ims, actual_type, *atoms, locales, *locale;
    int			actual_format;
    unsigned long	nitems, bytes_after;
    Window		ims_window;
    XEvent		xevent;
    int			ii;
    Bool		flag = False;

    if( (ims = XInternAtom( display, XIM_SERVERS, True )) == None  ||
	event->xproperty.atom != ims  ||
	event->xproperty.state == PropertyDelete )
	return( False );

    if( XGetWindowProperty( display, RootWindow(display, 0), ims, 0L, 1000000L,
			    False, XA_ATOM, &actual_type, &actual_format,
			    &nitems, &bytes_after, (unsigned char **)&atoms )
	    != Success ) {
	return( False );
    }
    if( actual_type != XA_ATOM  ||  actual_format != 32 ) {
	XFree( atoms );
	return( False );
    }

    if( (locales = XInternAtom( display, XIM_LOCALES, True )) == None ) {
	XFree( atoms );
	return( False );
    }
    for( ii = 0; ii < nitems; ii++, atoms ) {
	if( ims_window = XGetSelectionOwner( display, atoms[ii] ) ) {
	    XConvertSelection( display, atoms[ii], locales, locales, requestor,
			       CurrentTime );
	    XIfEvent( display, &xevent, CheckSNEvent, (XPointer)requestor );
	    if( xevent.xselection.property == None )
		continue;
	    if( XGetWindowProperty( display, requestor, locales, 0L, 1000000L,
				    True, locales, &actual_type, &actual_format,
				    &nitems, &bytes_after,
				    (unsigned char **)&locale ) != Success )
		continue;
	    flag |= CallCallback( display, *locale );
	}
    }
    XFree( atoms );
    return( flag );
}


Public Bool
_XimRegisterIMInstantiateCallback( lcd, display, callback, client_data )
    XLCd	 lcd;
    Display	*display;
    XIMProc	 callback;
    XPointer	*client_data;
{
    XimInstCallback	icb, tmp;
    XIM			xim;
    int			ii;

    if( lock )
	return( False );

    icb = (XimInstCallback)Xmalloc(sizeof(XimInstCallbackRec));
    if( !icb )
	return( False );
    icb->call = icb->destroy = False;
    icb->display = display;
    MakeLocale( lcd, icb->name );
    icb->modifiers = lcd->core->modifiers;	/* XXXXX */
    icb->callback = callback;
    icb->client_data = client_data;
    icb->next = NULL;

    if( !callback_list )
	callback_list = icb;
    else {
	for( tmp = callback_list; tmp->next; tmp = tmp->next );
	tmp->next = icb;
    }

    xim = _XimOpenIM( lcd, display, NULL, NULL, NULL );	/* XXXXX */

    if( icb == callback_list ) {
	if( requestor == None )
	    requestor = XCreateSimpleWindow( display, RootWindow(display, 0),
					     0, 0, 1, 1, 1, 0, 0 );
	_XRegisterFilterByType( display, RootWindow(display, 0),
				PropertyNotify, PropertyNotify,
				_XimFilterPropertyNotify,
				(XPointer)NULL );
	XSelectInput( display, RootWindow(display, 0), PropertyChangeMask );
    }

    if( xim ) {
	lock = True;
	xim->methods->close( (XIM)xim );
	lock = False;
	icb->call = True;
	callback( display, client_data, NULL );
    }

    return( True );
}


Public Bool
_XimUnRegisterIMInstantiateCallback( lcd, display, callback )
    XLCd	 lcd;
    Display	*display;
    XIMProc	 callback;
{
    char		locale[XIM_MAXLCNAMELEN];
    XimInstCallback	icb, picb;

    if( !callback_list )
	return( False );

    MakeLocale( lcd, locale );

    for( icb = callback_list, picb = NULL; icb; picb = icb, icb = icb->next ) {
	if( !strcmp( locale, icb->name )  &&
	    (lcd->core->modifiers == icb->modifiers  ||		/* XXXXX */
	     (lcd->core->modifiers  &&  icb->modifiers  &&
	      !strcmp( lcd->core->modifiers, icb->modifiers )))  &&
	    callback == icb->callback  &&  !icb->destroy ) {
	    if( lock )
		icb->destroy = True;
	    else {
		if( !picb ) {
		    callback_list = icb->next;
		    _XUnregisterFilter( display, RootWindow(display, 0),
					_XimFilterPropertyNotify,
					(XPointer)NULL );
		    XSelectInput( display, RootWindow(display, 0),
				  NoEventMask );
		}
		else
		    picb->next = icb->next;
		XFree( icb );
	    }
	    return( True );
	}
    }
    return( False );
}


Public void
_XimResetIMInstantiateCallback( xim )
    Xim		xim;
{
    char		*language, *territory;
    char		locale[XIM_MAXLCNAMELEN];
    XimInstCallback	icb;
    XLCd	 	lcd = xim->core.lcd;

    if( !callback_list  &&  lock )
	return;

    MakeLocale( lcd, locale );

    for( icb = callback_list; icb; icb = icb->next )
	if( !strcmp( locale, icb->name )  &&
	    (lcd->core->modifiers == icb->modifiers  ||
	     (lcd->core->modifiers  &&  icb->modifiers  &&
	      !strcmp( lcd->core->modifiers, icb->modifiers ))) )
	    icb->call = False;
}
