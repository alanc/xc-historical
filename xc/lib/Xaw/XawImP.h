/* $XConsortium: XawImP.h,v 1.0 94/01/01 00:00:00 kaleb Exp $ */

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

#ifndef _XawImP_h
#define _XawImP_h

#define XtNinputMethod		"inputMethod"
#define XtCInputMethod		"InputMethod"
#define XtNpreeditType		"preeditType"
#define XtCPreeditType		"PreeditType"
#define XtNopenIm		"openIm"
#define XtCOpenIm		"OpenIm"
#define XtNsharedIc		"sharedIc"
#define XtCSharedIc		"SharedIc"

#include <X11/Xaw/Text.h>

#define	CIICFocus	(1 << 0)
#define	CIFontSet	(1 << 1)
#define	CIFg		(1 << 2)
#define	CIBg		(1 << 3)
#define	CIBgPixmap	(1 << 4)
#define	CICursorP	(1 << 5)
#define	CILineS		(1 << 6)

typedef	struct _XawImPart
{
    XIM			xim;
    XrmResourceList	resources;
    Cardinal		num_resources;
    Boolean		open_im;
    Boolean		initialized;
    Dimension		area_height;
    String		input_method;
    String		preedit_type;
    String		*im_list;
    Cardinal		im_list_num;
} XawImPart;

typedef struct _XawIcTablePart
{
    Widget		widget;
    XIC			xic;
    XIMStyle		input_style;
    unsigned long	flg;
    unsigned long	prev_flg;
    Boolean		ic_focused;
    XFontSet		font_set;
    Pixel		foreground;
    Pixel		background;
    Pixmap		bg_pixmap;
    XawTextPosition	cursor_position;
    unsigned long	line_spacing;
    Boolean		openic_error;
    struct _XawIcTablePart *next;
} XawIcTablePart, *XawIcTableList;

typedef	struct _XawIcPart
{
    String		*ic_list;
    Cardinal		ic_list_num;
    XIMStyle		input_style;
    Boolean		shared_ic;
    XawIcTableList	shared_ic_table;
    XawIcTableList	current_ic_table;
    XawIcTableList	ic_table;
} XawIcPart;

typedef	struct _contextDataRec
{
    Widget		parent;
    Widget		ve;
} contextDataRec;

typedef	struct _contextErrDataRec
{
    Widget		widget;
    XIM			xim;
} contextErrDataRec;

#if NeedFunctionPrototypes
void _XawImResizeVendorShell( Widget );
Dimension _XawImGetShellHeight( Widget );
void _XawImRealize( Widget );
void _XawImInitialize( Widget, Widget );
void _XawImReconnect( Widget );
void _XawImRegister( Widget );
void _XawImUnregister( Widget );
void _XawImSetValues( Widget, ArgList, Cardinal );
void _XawImVASetValues( Widget, ... );
void _XawImSetFocusValues( Widget, ArgList, Cardinal );
void _XawImVASetFocusValues( Widget, ... );
void _XawImUnsetFocus( Widget );
int  _XawImWcLookupString( Widget,XKeyPressedEvent*,wchar_t*,int,KeySym*,Status* );
int  _XawImGetImAreaHeight( Widget );
void _XawImCallVendorShellExtResize( Widget );
void _XawImDestroy( Widget, Widget );
#else
void _XawImResizeVendorShell();
Dimension _XawImGetShellHeight();
void _XawImRealize();
void _XawImInitialize();
void _XawImReconnect();
void _XawImRegister();
void _XawImUnregister();
void _XawImSetValues();
void _XawImVASetValues();
void _XawImSetFocusValues();
void _XawImVASetFocusValues();
void _XawImUnsetFocus();
int  _XawImWcLookupString();
int  _XawImGetImAreaShellHeight();
void _XawImCallVendorShellExtResize();
void _XawImDestroy();
#endif


#endif	/* _XawImP_h */
