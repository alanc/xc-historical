/*
 * $XConsortium: Repeater.h,v 1.16 90/03/01 17:05:05 jim Exp $
 *
 * Copyright 1989 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Jim Fulton, MIT X Consortium
 */

#ifndef _XawRepeater_h
#define _XawRepeater_h

#include <X11/Xaw/Command.h>

/*****************************************************************************
 * 
 * Repeater Widget (subclass of Command)
 * 
 * This widget is a push button that repeatedly fires when held down.
 * 
 * Parameters:
 * 
 *  Name		Class		Type		Default
 *  ----		-----		----		-------
 * 
 *  initialDelay	Delay		int		200 milliseconds
 *  repeatDealy		Delay		int		50 milliseconds
 *  flash		Boolean		Boolean		FALSE
 * 
 *****************************************************************************/

					/* new instance and class names */
#define XtNinitialDelay "initialDelay"
#define XtCDelay "Delay"
#define XtNrepeatDelay "repeatDelay"
#define XtNflash "flash"
					/* external declarations */
extern WidgetClass repeaterWidgetClass;

typedef struct _RepeaterClassRec *RepeaterWidgetClass;
typedef struct _RepeaterRec      *RepeaterWidget;

#endif /* _XawRepeater_h */
