/*
* $XConsortium: IntrinsicI.h,v 1.33 89/09/12 16:46:48 swick Exp $
* $oHeader: IntrinsicI.h,v 1.5 88/08/31 16:21:08 asente Exp $
*/

/***********************************************************
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

#ifndef _XtintrinsicI_h
#define _XtintrinsicI_h

#include "IntrinsicP.h"

#include "Object.h"
#include "RectObj.h"
#include "ObjectP.h"
#include "RectObjP.h"

#include "TranslateI.h"
#include "CallbackI.h"
#include "CompositeI.h"
#include "ConvertI.h"
#include "InitialI.h"
#include "ResourceI.h"
#include "EventI.h"

#define RectObjClassFlag	0x02
#define WidgetClassFlag		0x04
#define CompositeClassFlag	0x08
#define ConstraintClassFlag	0x10
#define ShellClassFlag		0x20
#define WMShellClassFlag	0x40
#define TopLevelClassFlag	0x80

/****************************************************************
 *
 * Byte utilities
 *
 ****************************************************************/

extern void bcopy();
extern void bzero();
extern int bcmp();

/* If the alignment characteristics of your machine are right, these may be
   faster */

#ifdef UNALIGNED

#define XtBCopy(src, dst, size)				    \
    if (size == sizeof(int))				    \
	*((int *) (dst)) = *((int *) (src));		    \
    else if (size == sizeof(char))			    \
	*((char *) (dst)) = *((char *) (src));		    \
    else if (size == sizeof(short))			    \
	*((short *) (dst)) = *((short *) (src));	    \
    else						    \
	bcopy((char *) (src), (char *) (dst), (int) (size));

#define XtBZero(dst, size)				    \
    if (size == sizeof(int))				    \
	*((int *) (dst)) = 0;				    \
    else						    \
	bzero((char *) (dst), (int) (size));

#define XtBCmp(b1, b2, size)				    \
    (size == sizeof(int) ?				    \
	*((int *) (b1)) != *((int *) (b2))		    \
    :   bcmp((char *) (b1), (char *) (b2), (int) (size))    \
    )

#else

#define XtBCopy(src, dst, size)		\
	bcopy((char *) (src), (char *) (dst), (int) (size));

#define XtBZero(dst, size) bzero((char *) (dst), (int) (size));

#define XtBCmp(b1, b2, size) bcmp((char *) (b1), (char *) (b2), (int) (size))

#endif


/****************************************************************
 *
 * Stack cache allocation/free
 *
 ****************************************************************/

#define XtStackAlloc(size, stack_cache_array)     \
    (size <= sizeof(stack_cache_array)		  \
    ?  (XtPointer)stack_cache_array		  \
    :  XtMalloc((unsigned) size))

#define XtStackFree(pointer, stack_cache_array) \
    if ((pointer) != ((XtPointer)stack_cache_array)) XtFree(pointer); else

/***************************************************************
 *
 * Filename defines
 *
 **************************************************************/

#ifndef XAPPLOADDIR
#define XAPPLOADDIR "/usr/lib/X11/app-defaults/"
#endif
#ifndef ERRORDB
#define ERRORDB "/usr/lib/X11/XtErrorDB"
#endif

/*************************************************************
 *
 * Misc
 ************************************************************/

extern Widget _XtWindowedAncestor();
   /* Object object; */

#endif /* _XtintrinsicI_h */
/* DON'T ADD STUFF AFTER THIS #endif */
