/* $XConsortium: XKBlibint.h,v 1.3 93/09/28 19:32:53 rws Exp $ */
/************************************************************
Copyright (c) 1993 by Silicon Graphics Computer Systems, Inc.

Permission to use, copy, modify, and distribute this
software and its documentation for any purpose and without
fee is hereby granted, provided that the above copyright
notice appear in all copies and that both that copyright
notice and this permission notice appear in supporting
documentation, and that the name of Silicon Graphics not be 
used in advertising or publicity pertaining to distribution 
of the software without specific prior written permission.
Silicon Graphics makes no representation about the suitability 
of this software for any purpose. It is provided "as is"
without any express or implied warranty.

SILICON GRAPHICS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS 
SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY 
AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SILICON
GRAPHICS BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/

#ifndef _XKBLIBINT_H_
#define	_XKBLIBINT_H_

#include <X11/XKBlib.h>

#define	XkbMapPending	(1<<0)

typedef int	(*XkbKSToMBFunc)(
#if NeedFunctionPrototypes
	XPointer	/* priv */,
	KeySym		/* sym */,
	char *		/* buf */,
	int		/* len */,
	Status *	/* status */
#endif
);

typedef KeySym	(*XkbMBToKSFunc)(
#if NeedFunctionPrototypes
	XPointer	/* priv */,
	char *		/* buf */,
	int		/* len */,
	Status *	/* status */
#endif
);

typedef KeySym	(*XkbToUpperFunc)(
#if NeedFunctionPrototypes
	KeySym		/* sym */
#endif
);

typedef struct _XkbConverters {
	XkbKSToMBFunc	 KSToMB;
	XPointer	 KSToMBPriv;
	XkbMBToKSFunc	 MBToKS;
	XPointer	 MBToKSPriv;
	XkbToUpperFunc	 KSToUpper;
} XkbConverters;

typedef struct _XkbInfoRec {
	unsigned	 flags;
	XExtCodes	 *codes;
	int		 srv_major;
	int		 srv_minor;
	unsigned	 selected_events;
	XkbDescRec	*desc;
	XkbMapChangesRec changes;

	char		*modmap;
	char		*charset;
	XkbConverters	 cvt;
} XkbInfoRec, *XkbInfoPtr;

_XFUNCPROTOBEGIN

extern char	*_XkbGetCharset(
#if NeedFunctionPrototypes
	char *		/* locale */
#endif
);
extern int	 _XkbGetConverters(
#if NeedFunctionPrototypes
	char *		/* charset */,
	XkbConverters *	/* cvt_rtrn */
#endif
);

extern Status _XkbRefreshMapping(
#if NeedFunctionPrototypes
	Display *			/* display */,
	XkbDescRec *			/* xkb */,
	XkbChangesRec *			/* ev */
#endif
);

_XFUNCPROTOEND

#endif /* _XKBLIBINT_H_ */
