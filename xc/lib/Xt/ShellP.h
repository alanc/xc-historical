
/*
* $Header: ShellP.h,v 1.9 87/10/09 13:11:51 chow BL5 $
*/

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
 * 
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */
/* 
 * ShellPrivate.h - Private definitions for Shell widget
 * 
 * Author:	Ricahrd Hyde
 * 		Digital Equipment Corporation
 * 		Western Software Laboratory
 * Date:	Thu Sept 24, 1987
 */

#ifndef _XtShellPrivate_h
#define _XtShellPrivate_h
#ifndef VMS
#include <X11/Xutil.h>
#else
#include Xutil
#endif

/***********************************************************************
 *
 * Shell Widget Private Data
 *
 ***********************************************************************/

/* New fields for the Shell widget class record */

typedef struct {int foo;} ShellClassPart;


typedef struct _ShellClassRec {
  	CoreClassPart      core_class;
	CompositeClassPart composite_class;
	ShellClassPart  shell_class;
} ShellClassRec;

extern ShellClassRec shellClassRec;

/* New fields for the shell widget */

typedef struct {
	int	    argc;
	char      **argv;
	char	   *icon_name;
	char	   *title;
	char       *geostr;
	XtCreatePopupChildProc	create_popup_child;
	XtGrabKind	grab_kind;
	Boolean		spring_loaded;
	Boolean		popped_up;
	Boolean	    iconic;
	Boolean	    resizeable;
	Boolean	    clientspecified;
	Boolean	    transient;
	Boolean	    save_under;
	Boolean	    override_redirect;
	XSizeHints  sizehints;
	XWMHints    wmhints;
#ifdef DECHINTS
	DECWMHints  dechints
#endif
} ShellPart;

/****************************************************************
 *
 * Full instance record declaration
 *
 ****************************************************************/

typedef  struct {
	CorePart 	core;
	CompositePart 	composite;
	ShellPart 	shell;
} ShellRec, *ShellWidget;

#endif  _XtShellPrivate_h
