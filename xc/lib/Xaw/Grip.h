/*
* $Header: Knob.h,v 1.3 87/12/17 16:42:28 swick Locked $
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
 *  Knob.h - Public Definitions for Knob widget (used by VPane Widget)
 *
 */

#ifndef _XtKnob_h
#define _XtKnob_h

/***************************************************************************
 *
 * Knob Widget 
 *
 **************************************************************************/

/* Parameters:

 Name		     Class		RepType		Default Value
 ----		     -----		-------		-------------
 background	     Background		pixel		Black
 border		     BorderColor	pixel		Black
 borderWidth	     BorderWidth	int		0
 callback	     Callback		Pointer		KnobAction
 destroyCallback     Callback		Pointer		NULL
 height		     Height		int		6
 mappedWhenManaged   MappedWhenManaged	Boolean		True
 sensitive	     Sensitive		Boolean		True
 width		     Width		int		6
 x		     Position		int		0
 y		     Position		int		0

*/


#define XtNknobTranslations	"knobTranslations"

typedef struct {
  XEvent *event;		/* the event causing the KnobAction */
  String *params;		/* the TranslationTable params */
  Cardinal num_params;		/* count of params */
} KnobCallDataRec, *KnobCallData;

/* Class Record Constant */

extern WidgetClass knobWidgetClass;

typedef struct _KnobClassRec *KnobWidgetClass;
typedef struct _KnobRec      *KnobWidget;

#endif _XtKnob_h
/* DON'T ADD STUFF AFTER THIS #endif */
