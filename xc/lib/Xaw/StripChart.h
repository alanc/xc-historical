/*
* $XConsortium: StripChart.h,v 1.3 90/10/22 14:37:17 converse Exp $
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

#ifndef _XawStripChart_h
#define _XawStripChart_h

/***********************************************************************
 *
 * StripChart Widget
 *
 ***********************************************************************/

/* Parameters:

 Name		     Class		RepType		Default Value
 ----		     -----		-------		-------------
 background	     Background		Pixel		XtDefaultBackground
 border		     BorderColor	Pixel		XtDefaultForeground
 borderWidth	     BorderWidth	Dimension	1
 destroyCallback     Callback		Pointer		NULL
 foreground	     Foreground		Pixel		XtDefaultForeground
 getValue	     Callback		Callback	NULL
 height		     Height		Dimension	120
 highlight	     Foreground		Pixel		Black
 mappedWhenManaged   MappedWhenManaged	Boolean		True
 minScale	     Scale		int		1
 update		     Interval		int		5 (seconds)
 width		     Width		Dimension	120
 x		     Position		Position	0
 y		     Position		Position	0

*/

#define DEFAULT_JUMP -1

#ifndef _XtStringDefs_h_
#define XtNhighlight "highlight"
#define XtNupdate "update"
#endif

#define XtCJumpScroll "JumpScroll"
#define XtCScale "Scale"

#define XtNgetValue "getValue"
#define XtNjumpScroll "jumpScroll"
#define XtNminScale "minScale"
#define XtNscale "scale"
#define XtNvmunix "vmunix"
 
typedef struct _StripChartRec *StripChartWidget;
typedef struct _StripChartClassRec *StripChartWidgetClass;

extern WidgetClass stripChartWidgetClass;

#endif /* _XawStripChart_h */
/* DON'T ADD STUFF AFTER THIS #endif */
