/*
* $XConsortium: PassivGraI.h,v 1.7 90/12/30 12:53:33 rws Exp $
*/

/********************************************************

Copyright (c) 1988 by Hewlett-Packard Company
Copyright (c) 1987, 1988, 1989 by Digital Equipment Corporation, Maynard, 
              Massachusetts, and the Massachusetts Institute of Technology, 
              Cambridge, Massachusetts

Permission to use, copy, modify, and distribute this software 
and its documentation for any purpose and without fee is hereby 
granted, provided that the above copyright notice appear in all 
copies and that both that copyright notice and this permission 
notice appear in supporting documentation, and that the names of 
Hewlett-Packard, Digital or  M.I.T.  not be used in advertising or 
publicity pertaining to distribution of the software without specific, 
written prior permission.

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

********************************************************/

#ifndef _PDI_h_
#define _PDI_h_


#define KEYBOARD TRUE
#define POINTER  FALSE

typedef enum {
    XtNoServerGrab, 
    XtPassiveServerGrab,
    XtActiveServerGrab,
    XtPseudoPassiveServerGrab,
    XtPseudoActiveServerGrab
}XtServerGrabType;

typedef struct _DetailRec {
    unsigned short 	exact;
    Mask  		*pMask;
} DetailRec;

typedef struct _XtServerGrabRec {
    struct _XtServerGrabRec 	*next;
    Widget			widget;
    unsigned int		ownerEvents:1;
    unsigned int		pointerMode:1;
    unsigned int		keyboardMode:1;
    DetailRec 			modifiersDetail;
    Mask			eventMask;
    DetailRec			detail;		/* key or button */
    Window			confineTo;	/* always NULL for keyboards */
    Cursor			cursor;		/* always NULL for keyboards */
} XtServerGrabRec, *XtServerGrabPtr;

typedef struct _XtDeviceRec{
    XtServerGrabRec	grab; 	/* need copy in order to protect
				   during grab */
    XtServerGrabType	grabType;
}XtDeviceRec, *XtDevice;

typedef enum {
    XtMyAncestor,
    XtMyDescendant,
    XtMyCousin,
    XtMySelf,
    XtUnrelated
}XtGeneology;


typedef struct {
    Widget		focusKid;
    XtServerGrabPtr	keyList, ptrList;
    Widget		queryEventDescendant;
    unsigned int	map_handler_added:1;
    unsigned int	realize_handler_added:1;
    unsigned int	active_handler_added:1;
    unsigned int	haveFocus:1;
    XtGeneology		focalPoint;
}XtPerWidgetInputRec, *XtPerWidgetInput;

typedef struct XtPerDisplayInputRec{
    XtGrabList 	grabList;
    XtDeviceRec keyboard, pointer;
    KeyCode	activatingKey;
    Widget 	*trace;
    int		traceDepth, traceMax;
    Widget 	focusWidget;
}XtPerDisplayInputRec, *XtPerDisplayInput;

#define IsServerGrab(g) ((g == XtPassiveServerGrab) ||\
			 (g == XtActiveServerGrab))

#define IsAnyGrab(g) ((g == XtPassiveServerGrab) ||\
		      (g == XtActiveServerGrab)  ||\
		      (g == XtPseudoPassiveServerGrab))

#define IsEitherPassiveGrab(g) ((g == XtPassiveServerGrab) ||\
				(g == XtPseudoPassiveServerGrab))

#define IsPseudoGrab(g) ((g == XtPseudoPassiveServerGrab))

extern void _XtDestroyServerGrabs(
#if NeedFunctionPrototypes
    Widget		/* w */,
    XtPointer		/* pwi */, /*XtPerWidgetInput*/
    XtPointer		/* call_data */
#endif
);

extern XtPerWidgetInput _XtGetPerWidgetInput(
#if NeedFunctionPrototypes
    Widget	/* widget */,
    _XtBoolean	/* create */
#endif
);

extern XtServerGrabPtr _XtCheckServerGrabsOnWidget(
#if NeedFunctionPrototypes
    XEvent*		/* event */,
    Widget		/* widget */,
    _XtBoolean		/* isKeyboard */
#endif
);

/*
extern XtGrabList* _XtGetGrabList( XtPerDisplayInput );
*/

#define _XtGetGrabList(pdi) (&(pdi)->grabList)

extern void _XtFreePerWidgetInput(
#if NeedFunctionPrototypes
    Widget		/* w */,
    XtPerWidgetInput	/* pwi */
#endif
);

extern Widget _XtProcessKeyboardEvent(
#if NeedFunctionPrototypes
    XKeyEvent*		/* event */,
    Widget		/* widget */,
    XtPerDisplayInput	/* pdi */
#endif
);

extern Widget _XtProcessPointerEvent(
#if NeedFunctionPrototypes
    XButtonEvent*	/* event */,
    Widget		/* widget */,
    XtPerDisplayInput	/* pdi */
#endif
);

#endif /* _PDI_h_ */
