/*
* $XConsortium: ShellP.h,v 1.22 89/06/16 18:09:22 jim Exp $
* $oHeader: ShellP.h,v 1.2 88/08/18 15:56:19 asente Exp $
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

/* 
 * ShellP.h - Private definitions for Shell widget
 * 
 * Author:	Paul Asente
 * 		Digital Equipment Corporation
 * 		Western Software Laboratory
 * Date:	Thu Dec 3, 1987
 */

#ifndef _XtShellPrivate_h
#define _XtShellPrivate_h

/* *****
 * ***** VendorP.h is included later on; it needs fields defined in the first
 * ***** part of this header file
 * *****
 */

/***********************************************************************
 *
 * Shell Widget Private Data
 *
 ***********************************************************************/

/* New fields for the Shell widget class record */

typedef struct {
    XtPointer       extension;          /* pointer to extension record      */
} ShellClassPart;

typedef struct _ShellClassRec {
  	CoreClassPart      core_class;
	CompositeClassPart composite_class;
	ShellClassPart  shell_class;
} ShellClassRec;

externalref ShellClassRec shellClassRec;

/* New fields for the shell widget */

typedef struct {
	char       *geometry;
	XtCreatePopupChildProc	create_popup_child_proc;
	XtGrabKind	grab_kind;
	Boolean	    spring_loaded;
	Boolean	    popped_up;
	Boolean	    allow_shell_resize;
	Boolean     client_specified; /* re-using old name */
#define _XtShellPositionValid	0x01
#define _XtShellNotReparented	0x02
	Boolean	    save_under;
	Boolean	    override_redirect;

	XtCallbackList popup_callback;
	XtCallbackList popdown_callback;
} ShellPart;

typedef  struct {
	CorePart 	core;
	CompositePart 	composite;
	ShellPart 	shell;
} ShellRec, *ShellWidget;

/***********************************************************************
 *
 * OverrideShell Widget Private Data
 *
 ***********************************************************************/

/* New fields for the OverrideShell widget class record */

typedef struct {
    XtPointer       extension;          /* pointer to extension record      */
} OverrideShellClassPart;

typedef struct _OverrideShellClassRec {
  	CoreClassPart      core_class;
	CompositeClassPart composite_class;
	ShellClassPart  shell_class;
	OverrideShellClassPart  override_shell_class;
} OverrideShellClassRec;

externalref OverrideShellClassRec overrideShellClassRec;

/* No new fields for the override shell widget */

typedef struct {int frabjous;} OverrideShellPart;

typedef  struct {
	CorePart 	core;
	CompositePart 	composite;
	ShellPart 	shell;
	OverrideShellPart override;
} OverrideShellRec, *OverrideShellWidget;

/***********************************************************************
 *
 * WMShell Widget Private Data
 *
 ***********************************************************************/

/* New fields for the WMShell widget class record */

typedef struct {
    XtPointer       extension;          /* pointer to extension record      */
} WMShellClassPart;

typedef struct _WMShellClassRec {
  	CoreClassPart      core_class;
	CompositeClassPart composite_class;
	ShellClassPart  shell_class;
	WMShellClassPart wm_shell_class;
} WMShellClassRec;

externalref WMShellClassRec wmShellClassRec;

/* New fields for the WM shell widget */

typedef struct {
	char	   *title;
	int 	    wm_timeout;
	Boolean	    wait_for_wm;
	Boolean	    transient;
	Atom	    wm_configure_denied,  wm_moved;
	XSizeHints  size_hints;
	XWMHints    wm_hints;
} WMShellPart;

typedef  struct {
	CorePart 	core;
	CompositePart 	composite;
	ShellPart 	shell;
	WMShellPart	wm;
} WMShellRec, *WMShellWidget;

#include "VendorP.h"

/***********************************************************************
 *
 * TransientShell Widget Private Data
 *
 ***********************************************************************/

/* New fields for the TransientShell widget class record */

typedef struct {
    XtPointer       extension;          /* pointer to extension record      */
} TransientShellClassPart;

typedef struct _TransientShellClassRec {
  	CoreClassPart      core_class;
	CompositeClassPart composite_class;
	ShellClassPart  shell_class;
	WMShellClassPart   wm_shell_class;
	VendorShellClassPart vendor_shell_class;
	TransientShellClassPart transient_shell_class;
} TransientShellClassRec;

externalref TransientShellClassRec transientShellClassRec;

/* New fields for the transient shell widget */

typedef struct {int brillig;} TransientShellPart;

typedef  struct {
	CorePart 	core;
	CompositePart 	composite;
	ShellPart 	shell;
	WMShellPart	wm;
	VendorShellPart	vendor;
	TransientShellPart transient;	
} TransientShellRec, *TransientShellWidget;

/***********************************************************************
 *
 * TopLevelShell Widget Private Data
 *
 ***********************************************************************/

/* New fields for the TopLevelShell widget class record */

typedef struct {
    XtPointer       extension;          /* pointer to extension record      */
} TopLevelShellClassPart;

typedef struct _TopLevelShellClassRec {
  	CoreClassPart      core_class;
	CompositeClassPart composite_class;
	ShellClassPart  shell_class;
	WMShellClassPart   wm_shell_class;
	VendorShellClassPart vendor_shell_class;
	TopLevelShellClassPart top_level_shell_class;
} TopLevelShellClassRec;

externalref TopLevelShellClassRec topLevelShellClassRec;

/* New fields for the top level shell widget */

typedef struct {
	char	   *icon_name;
	Boolean	    iconic;
} TopLevelShellPart;

typedef  struct {
	CorePart 	core;
	CompositePart 	composite;
	ShellPart 	shell;
	WMShellPart	wm;
	VendorShellPart	vendor;
	TopLevelShellPart topLevel;
} TopLevelShellRec, *TopLevelShellWidget;

/***********************************************************************
 *
 * ApplicationShell Widget Private Data
 *
 ***********************************************************************/

/* New fields for the ApplicationShell widget class record */

typedef struct {
    XtPointer       extension;          /* pointer to extension record      */
} ApplicationShellClassPart;

typedef struct _ApplicationShellClassRec {
  	CoreClassPart      core_class;
	CompositeClassPart composite_class;
	ShellClassPart  shell_class;
	WMShellClassPart   wm_shell_class;
	VendorShellClassPart vendor_shell_class;
	TopLevelShellClassPart top_level_shell_class;
	ApplicationShellClassPart application_shell_class;
} ApplicationShellClassRec;

externalref ApplicationShellClassRec applicationShellClassRec;

/* New fields for the application shell widget */

typedef struct {
	char *class;
	XrmClass xrm_class;
	int argc;
	char **argv;
} ApplicationShellPart;

typedef  struct {
	CorePart 	core;
	CompositePart 	composite;
	ShellPart 	shell;
	WMShellPart	wm;
	VendorShellPart	vendor;
	TopLevelShellPart topLevel;
	ApplicationShellPart application;
} ApplicationShellRec, *ApplicationShellWidget;

#endif /* _XtShellPrivate_h */
