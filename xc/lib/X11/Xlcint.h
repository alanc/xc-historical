/*
 * $XConsortium: Xlcint.h,v 11.14 93/09/17 13:24:53 rws Exp $
 */

/*
 * Copyright 1990, 1991 by OMRON Corporation, NTT Software Corporation,
 *                      and Nippon Telegraph and Telephone Corporation
 * Copyright 1991 by the Massachusetts Institute of Technology
 * Copyright 1991 by the Open Software Foundation
 * Copyright 1993 by the TOSHIBA Corp.
 * Copyright 1993 by the Sony Corporation
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of OMRON, NTT Software, NTT, Open
 * Software Foundation and M.I.T. not be used in advertising or 
 * publicity pertaining to distribution of the software without specific, 
 * written prior permission. OMRON, NTT Software, NTT, Open Software
 * Foundation and M.I.T. make no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without express or
 * implied warranty.
 *
 * OMRON, NTT SOFTWARE, NTT, OPEN SOFTWARE FOUNDATION AND M.I.T. 
 * DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING 
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT 
 * SHALL OMRON, NTT SOFTWARE, NTT, OPEN SOFTWARE FOUNDATIONN OR M.I.T. BE
 * LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES 
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 *	Authors: Li Yuhong		OMRON Corporation
 *		 Tatsuya Kato		NTT Software Corporation
 *		 Hiroshi Kuribayashi	OMRON Coproration
 *		 Muneiyoshi Suzuki	Nippon Telegraph and Telephone Co.
 * 
 *		 M. Collins		OSF  
 *		 Katsuhisa Yano		TOSHIBA Corp.
 *               Makoto Wakamatsu       Sony Corporation
 */				


#ifndef	_XLCINT_H_
#define	_XLCINT_H_

#include <X11/Xresource.h>
#include <X11/Xutil.h>

/* current Ultrix compiler gets horribly confused */
#if defined(FUNCPROTO) && defined(ultrix)
#undef NeedFunctionPrototypes
#endif

typedef struct _XIMFilter {
    struct _XIMFilter *next;
    Window window;
    unsigned long event_mask;
    int start_type, end_type;
    Bool (*filter)(
#if NeedFunctionPrototypes
	Display*, Window, XEvent*, XPointer
#endif
	);
    XPointer client_data;
} XFilterEventRec, *XFilterEventList;

typedef struct {
    char    *name;
    XPointer value;
} XIMArg;

#ifdef offsetof
#define XOffsetOf(s_type,field) offsetof(s_type,field)
#else
#define XOffsetOf(s_type,field) ((unsigned int)&(((s_type*)NULL)->field))
#endif

#define XIMNumber(arr) ((unsigned int) (sizeof(arr) / sizeof(arr[0])))

#define IMResourceWrite		1
#define IMResourceRead 		2
#define IMResourceReadWrite	3

/*
 * define secondary data structs which are part of Input Methods
 * and Input Context
 */
typedef struct {
    XrmQuark		xrm_name;		/* Resource name quark */
    int			xrm_size;		/* Size in bytes of data */
    long		xrm_offset;		/* -offset-1 */
    unsigned short 	mode;			/* Read Write Permission */
    unsigned short 	id;			/* Input Method Protocol */
} XIMrmResource, *XIMrmResourceList;

typedef struct {
    char		*resource_name;		/* Resource string */
    int			resource_size;		/* Size in bytes of data */
    long		resource_offset;	/* Offset from base */
    unsigned short 	mode;			/* Read Write Permission */
    unsigned short 	id;			/* Input Method Protocol */
} XIMResource, *XIMResourceList;

typedef struct {
    XIMCallback start;
    XIMCallback done;
    XIMCallback draw;
    XIMCallback caret;
} ICCallbacks;

/*
 * data block describing the visual attributes associated with
 * an input context
 */
typedef struct {
    XRectangle		area;
    XRectangle		area_needed;
    XPoint		spot_location;
    Colormap		colormap;
    Atom		std_colormap;
    unsigned long	foreground;
    unsigned long	background;
    Pixmap		background_pixmap;
    XFontSet            fontset;
    int	       		line_space;
    Cursor		cursor;
    ICCallbacks		callbacks;
} ICAttributes, *ICAttributesPtr;

/*
 * Methods for Xrm parsing
 */

typedef struct {
    void (*mbinit)(
#if NeedFunctionPrototypes
	XPointer	/* state */
#endif
	);
    char (*mbchar)(
#if NeedFunctionPrototypes
	XPointer	/* state */,
	char*		/* str */,
	int*		/* lenp */
#endif
	);
    void (*mbfinish)(
#if NeedFunctionPrototypes
	XPointer /* state */
#endif
	);
    char* (*lcname)(
#if NeedFunctionPrototypes
	XPointer /* state */
#endif
	);
    void (*destroy)(
#if NeedFunctionPrototypes
	XPointer /* state */
#endif
	);
} XrmMethodsRec, *XrmMethods;

typedef struct _XLCd *XLCd; /* need forward reference */

/*
 * define an LC, it's methods, and data.
 */

typedef struct {
  char* (*map_modifiers)(		/* called by XSetModifiers */
#if NeedFunctionPrototypes
	XLCd, char*, char*
#endif
	);
    XFontSet (*create_fontset)(
#if NeedFunctionPrototypes
	XLCd, Display*, char*, char**, int, char***, int*
#endif
	);
    XIM (*open_im)(
#if NeedFunctionPrototypes
	XLCd, Display*, XrmDatabase, char*, char*
#endif
	);
    Bool (*register_callback)(
#if NeedFunctionPrototypes
	XLCd, Display*, XIMProc, XPointer*
#endif
	);
    Bool (*unregister_callback)(
#if NeedFunctionPrototypes
	XLCd, Display*, XIMProc
#endif
	);
    XrmMethods (*init_parse_info)(
#if NeedFunctionPrototypes
	XLCd, XPointer*
#endif
	);
    int (*mb_text_prop_to_list)(
#if NeedFunctionPrototypes
	XLCd, Display*, XTextProperty*, char***, int*
#endif
	);
    int (*wc_text_prop_to_list)(
#if NeedFunctionPrototypes
	XLCd, Display*, XTextProperty*, wchar_t***, int*
#endif
	);
    int (*mb_text_list_to_prop)(
#if NeedFunctionPrototypes
	XLCd, Display*, char**, int, XICCEncodingStyle, XTextProperty*
#endif
	);
    int (*wc_text_list_to_prop)(
#if NeedFunctionPrototypes
	XLCd, Display*, wchar_t**, int, XICCEncodingStyle, XTextProperty*
#endif
	);
    void (*wc_free_string_list)(
#if NeedFunctionPrototypes
	XLCd, wchar_t**
#endif
	);
    char* (*default_string)(
#if NeedFunctionPrototypes
	XLCd
#endif
	);
} XLCdMethodsRec, *XLCdMethods;


typedef struct {
    char*		name;			/* name of this LC */
    char*		modifiers;		/* modifiers of locale */
} XLCdCoreRec, *XLCdCore;


typedef struct _XLCd {
    XLCdMethods		methods;		/* methods of this LC */
    XLCdCore		core;			/* data of this LC */
    XPointer		opaque;			/* LDX specific data */
} XLCdRec;

typedef int XlcPosition;

#define XlcHead		0
#define XlcTail		-1


/*
 * X Font Sets are an instantiable object, so we define it, the 
 * object itself, a method list and data
 */

/* 
 * XFontSet object method list
 */
typedef struct {
    /* pure font methods */

    void (*free)(
#if NeedFunctionPrototypes
	Display*, XFontSet
#endif
	);
    
    /* multi-byte text drawing methods */

    int (*mb_escapement)(
#if NeedFunctionPrototypes
	XFontSet, char*, int
#endif
	);
    int (*mb_extents)(
#if NeedFunctionPrototypes
	XFontSet, char*, int, XRectangle*, XRectangle*
#endif
	);
    Status (*mb_extents_per_char)(
#if NeedFunctionPrototypes
	XFontSet, char*, int, XRectangle*, XRectangle*, int, int*,
	XRectangle*, XRectangle*
#endif
	);
    int (*mb_draw_string)(
#if NeedFunctionPrototypes
	Display*, Drawable, XFontSet, GC, int, int, char*, int
#endif
	);
    void (*mb_draw_image_string)(
#if NeedFunctionPrototypes
	Display*, Drawable, XFontSet, GC, int, int, char*, int
#endif
	);

    /* wide character text drawing methods */

    int (*wc_escapement)(
#if NeedFunctionPrototypes
	XFontSet, wchar_t*, int
#endif
	);
    int (*wc_extents)(
#if NeedFunctionPrototypes
	XFontSet, wchar_t*, int, XRectangle*, XRectangle*
#endif
	);
    Status (*wc_extents_per_char)(
#if NeedFunctionPrototypes
	XFontSet, wchar_t*, int, XRectangle*, XRectangle*, int, int*,
	XRectangle*, XRectangle*
#endif
	);
    int (*wc_draw_string)(
#if NeedFunctionPrototypes
	Display*, Drawable, XFontSet, GC, int, int, wchar_t*, int
#endif
	);
    void (*wc_draw_image_string)(
#if NeedFunctionPrototypes
	Display*, Drawable, XFontSet, GC, int, int, wchar_t*, int
#endif
	);
} XFontSetMethodsRec, *XFontSetMethods;


/*
 * XFontSet LC independent data
 */

typedef struct {
    XLCd		lcd;			/* LC of this font set */
    char *		base_name_list;     	/* base font name list */
    int            	num_of_fonts;       	/* number of fonts */
    char **		font_name_list;     	/* font names list */
    XFontStruct **	font_struct_list;   	/* font struct list */
    XFontSetExtents  	font_set_extents;   	/* font set extents */
    char *		default_string;     	/* default string */
    Bool		context_dependent;	/* context-dependent drawing */
} XFontSetCoreRec, *XFontSetCore;


/*
 * An XFontSet.  Implementations may need to extend this data structure to
 * accomodate additional data, state information etc.
 */
typedef struct _XFontSet {
    XFontSetMethods	methods;		/* methods of this font set */
    XFontSetCoreRec	core;			/* data of this font set */
} XFontSetRec;



/*
 * X Input Managers are an instantiable object, so we define it, the 
 * object itself, a method list and data.
 */

/*
 * an Input Manager object method list
 */
typedef struct {
    Status (*close)(
#if NeedFunctionPrototypes
	XIM
#endif
	);
    char* (*get_values)(
#if NeedFunctionPrototypes
	XIM, XIMArg*
#endif
	);
    XIC (*create_ic)(
#if NeedFunctionPrototypes
	XIM, XIMArg*
#endif
	);
} XIMMethodsRec, *XIMMethods;

/*
 * Input Manager LC independent data
 */
typedef struct {
    XLCd		lcd;			/* LC of this input method */
    XIC			ic_chain;		/* list of ICs for this IM */

    Display *		display;               	/* display */
    XrmDatabase 	rdb;
    char *		res_name;
    char *		res_class;

    char *		im_name;		/* XIMMODIFIER name */
    XIMResourceList	im_resources;		/* compiled IM resource list */
    unsigned int	im_num_resources;
    XIMResourceList	ic_resources;		/* compiled IC resource list */
    unsigned int	ic_num_resources;
} XIMCoreRec, *XIMCore;



/*
 * An X Input Manager (IM).  Implementations may need to extend this data 
 * structure to accomodate additional data, state information etc.
 */
typedef struct _XIM {
    XIMMethods		methods;		/* method list of this IM */
    XIMCoreRec		core;			/* data of this IM */
} XIMRec;



/*
 * X Input Contexts (IC) are an instantiable object, so we define it, the 
 * object itself, a method list and data for this object
 */

/*
 * Input Context method list
 */ 
typedef struct {
    void (*destroy)(
#if NeedFunctionPrototypes
	XIC
#endif
	);
    void (*set_focus)(
#if NeedFunctionPrototypes
	XIC
#endif
	);
    void (*unset_focus)(
#if NeedFunctionPrototypes
	XIC
#endif
	);
    char* (*set_values)(
#if NeedFunctionPrototypes
	XIC, XIMArg*
#endif
	);
    char* (*get_values)(
#if NeedFunctionPrototypes
	XIC, XIMArg*
#endif
	);
    char* (*mb_reset)(
#if NeedFunctionPrototypes
	XIC
#endif
	);
    wchar_t* (*wc_reset)(
#if NeedFunctionPrototypes
	XIC
#endif
	);
    int (*mb_lookup_string)(
#if NeedFunctionPrototypes
	XIC, XKeyEvent*, char*, int, KeySym*, Status*
#endif
	);
    int (*wc_lookup_string)(
#if NeedFunctionPrototypes
	XIC, XKeyEvent*, wchar_t*, int, KeySym*, Status*
#endif
	);
} XICMethodsRec, *XICMethods;



/*
 * Input Context LC independent data
 */
typedef struct {
    XIM			im;			/* XIM this IC belongs too */
    XIC			next;			/* linked list of ICs for IM */

    Window		client_window;		/* window IM can use for */
						/* display or subwindows */
    XIMStyle		input_style;		/* IM's input style */
    Window		focus_window;		/* where key events go */
    unsigned long	filter_events;		/* event mask from IM */
    XIMCallback		geometry_callback;	/* client callback */
    char *		res_name;
    char *		res_class;

    ICAttributes	preedit_attr;		/* visuals of preedit area */
    ICAttributes	status_attr;		/* visuals of status area */
} XICCoreRec, *XICCore;


/*
 * an Input Context.  Implementations may need to extend this data 
 * structure to accomodate additional data, state information etc.
 */
typedef struct _XIC {
    XICMethods		methods;		/* method list of this IC */
    XICCoreRec		core;			/* data of this IC */
} XICRec;

/* current Ultrix compiler gets horribly confused */
#if !defined(NeedFunctionPrototypes) && defined(FUNCPROTO)
#define NeedFunctionPrototypes 1
#endif

typedef XLCd (*XLCdLoadProc)(
#if NeedFunctionPrototypes
    char*
#endif
);

_XFUNCPROTOBEGIN

extern XLCd _XlcCurrentLC(
#if NeedFunctionPrototypes
    void
#endif
);

extern Bool _XlcValidModSyntax(
#if NeedFunctionPrototypes
    char*	/* mods */,
    char**	/* valid */
#endif
);

extern char *_XlcDefaultMapModifiers(
#if NeedFunctionPrototypes
    XLCd	/* lcd */,
    char*	/* user_mods */,
    char*	/* prog_mods */
#endif
);

extern void _XIMCompileResourceList(
#if NeedFunctionPrototypes
    XIMResourceList	/* res */,
    unsigned int	/* num_res */
#endif
);

extern void _XCopyToArg(
#if NeedFunctionPrototypes
    XPointer		/* src */,
    XPointer*		/* dst */,
    unsigned int	/* size */
#endif
);

extern char ** _XParseBaseFontNameList(
#if NeedFunctionPrototypes
    char*		/* str */,
    int*		/* num */
#endif
);

extern XrmMethods _XrmInitParseInfo(
#if NeedFunctionPrototypes
	XPointer*	/* statep */
#endif
);

extern void _XRegisterFilterByMask(
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    Window		/* window */,
    unsigned long	/* event_mask */,
    Bool (*)(
#if NeedNestedPrototypes
	     Display*	/* display */,
	     Window	/* window */,
	     XEvent*	/* event */,
	     XPointer	/* client_data */
#endif
	     )		/* filter */,
    XPointer		/* client_data */
#endif
);

extern void _XRegisterFilterByType(
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    Window		/* window */,
    int			/* start_type */,
    int			/* end_type */,
    Bool (*)(
#if NeedNestedPrototypes
	     Display*	/* display */,
	     Window	/* window */,
	     XEvent*	/* event */,
	     XPointer	/* client_data */
#endif
	     )		/* filter */,
    XPointer		/* client_data */
#endif
);

extern void _XUnregisterFilter(
#if NeedFunctionPrototypes
    Display*		/* dpy */,
    Window		/* window */,
    Bool (*)(
#if NeedNestedPrototypes
	     Display*	/* display */,
	     Window	/* window */,
	     XEvent*	/* event */,
	     XPointer	/* client_data */
#endif
	     )		/* filter */,
    XPointer		/* client_data */
#endif
);

extern XLCd _XlcGetLC(
#if NeedFunctionPrototypes
    char*		/* name */
#endif
);

extern Bool _XlcAddLoader(
#if NeedFunctionPrototypes
    XLCdLoadProc	/* proc */,
    XlcPosition		/* position */
#endif
);

extern void _XlcRemoveLoader(
#if NeedFunctionPrototypes
    XLCdLoadProc	/* proc */
#endif
);

_XFUNCPROTOEND

#endif	/* _XLCINT_H_ */
