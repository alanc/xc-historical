/* $Header: Dialog.h,v 1.2 87/12/23 07:41:49 swick Locked $ */
/*
 * Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.
 * 
 *                         All Rights Reserved
 * 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in 
 * supporting documentation, and that the name of Digital Equipment
 * Corporation not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.  
 * 
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */


/* NOTE: THIS IS NOT A WIDGET!  Rather, this is an interface to a widget.
   It implements policy, and gives a (hopefully) easier-to-use interface
   than just directly making your own form. */

#ifndef _Dialog_h
#define _Dialog_h

#include <X/Form.h>

#define XtNgrabFocus	"grabFocus"
#define XtCGrabFocus	"GrabFocus"

typedef struct _DialogClassRec	*DialogWidgetClass;
typedef struct _DialogRec	*DialogWidget;

extern WidgetClass dialogWidgetClass;

extern void XtDialogAddButton(); /* parent, name, function, param */
    /* Wiget parent; */
    /* char *name; */
    /* void (*function)(); */
    /* caddr_t param; */

extern char *XtDialogGetValueString(); /* dpy, window */
    /* Display *dpy; */
    /* Window window; */

#endif _Dialog_h
/* DON'T ADD STUFF AFTER THIS #endif */
