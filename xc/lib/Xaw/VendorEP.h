/* $XConsortium: VendorEP.h,v 1.0 94/01/01 00:00:00 kaleb Exp $ */

/*
 * Copyright 1991 by OMRON Corporation
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of OMRON not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  OMRON makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * OMRON DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * OMRON BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTUOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE. 
 *
 *	Author:	Seiji Kuwari	OMRON Corporation
 *				kuwa@omron.co.jp
 *				kuwa%omron.co.jp@uunet.uu.net
 */				

#ifndef _VendorEP_h
#define _VendorEP_h

#include <X11/Xaw/XawImP.h>

typedef struct {
    XtPointer	extension;
} XawVendorShellExtClassPart;

typedef	struct _VendorShellExtClassRec {
    ObjectClassPart	object_class;
    XawVendorShellExtClassPart	vendor_shell_ext_class;
} XawVendorShellExtClassRec;

typedef struct {
    Widget	parent;
    XawImPart	im;
    XawIcPart	ic;
} XawVendorShellExtPart;

typedef	struct XawVendorShellExtRec {
    ObjectPart	object;
    XawVendorShellExtPart	vendor_ext;
} XawVendorShellExtRec, *XawVendorShellExtWidget;

#endif	/* _VendorEP_h */
