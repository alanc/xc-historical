/* $XConsortium$ */
/******************************************************************

              Copyright 1991, 1992 by TOSHIBA Corp.
              Copyright 1992 by FUJITSU LIMITED

 Permission to use, copy, modify, distribute, and sell this software
 and its documentation for any purpose is hereby granted without fee,
 provided that the above copyright notice appear in all copies and
 that both that copyright notice and this permission notice appear
 in supporting documentation, and that the name of TOSHIBA Corp. and
 FUJITSU LIMITED not be used in advertising or publicity pertaining to
 distribution of the software without specific, written prior permission.
 TOSHIBA Corp. and FUJITSU LIMITED makes no representations about the
 suitability of this software for any purpose.
 It is provided "as is" without express or implied warranty.
 
 TOSHIBA CORP. AND FUJITSU LIMITED DISCLAIMS ALL WARRANTIES WITH REGARD
 TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 AND FITNESS, IN NO EVENT SHALL TOSHIBA CORP. AND FUJITSU LIMITED BE
 LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

 Author   : Katsuhisa Yano       TOSHIBA Corp.
                                 mopi@osa.ilab.toshiba.co.jp
 Modifier : Takashi Fujiwara     FUJITSU LIMITED 
                                 fujiwara@a80.tech.yk.fujitsu.co.jp

******************************************************************/

#ifndef _XLCPUBLICI_H_
#define _XLCPUBLICI_H_

#include "XlcPublic.h"

#define XLC_PUBLIC(lcd, x)	(((XLCdPublic) lcd->core)->pub.x)
#define XLC_PUBLIC_PART(lcd)	(&(((XLCdPublic) lcd->core)->pub))
#define XLC_PUBLIC_METHODS(lcd)	(&(((XLCdPublicMethods) lcd->methods)->pub))

/*
 * XLCd public methods
 */

typedef struct _XLCdPublicMethodsRec *XLCdPublicMethods;

typedef struct _XLCdPublicMethodsPart {
    XLCdPublicMethods superclass;
    XLCd (*create)(
#if NeedVarargsPrototypes
	char*			/* name */,
	XLCdMethods		/* methods */
#endif
    );
    Bool (*initialize)(
#if NeedVarargsPrototypes
	XLCd	/* lcd */
#endif
    );
    void (*destroy)(
#if NeedVarargsPrototypes
	XLCd	/* lcd */
#endif
    );
    char* (*get_values)(
#if NeedVarargsPrototypes
	XLCd		/* lcd */,
	XlcArgList	/* arg_list */
#endif
    );
    void (*get_resource)(
#if NeedFunctionPrototypes
	XLCd	/* lcd */,
	char*	/* category */,
	char*	/* class */,
	char***	/* value */,
	int*	/* count */
#endif
    );
} XLCdPublicMethodsPart;

typedef struct _XLCdPublicMethodsRec {
    XLCdMethodsRec core;
    XLCdPublicMethodsPart pub;
} XLCdPublicMethodsRec;

/*
 * XLCd public data
 */

typedef struct _XLCdPublicPart {
    char *siname;			/* for _XlcMapOSLocaleName() */
    char *language;			/* language part of locale name */
    char *territory;			/* territory part of locale name */
    char *codeset;			/* codeset part of locale name */
    char *encoding_name;		/* encoding name */
    int mb_cur_max;			/* ANSI C MB_CUR_MAX */
    Bool is_state_depend;		/* state-depend encoding */
    char *default_string;		/* for XDefaultString() */
    XPointer xlocale_db;
} XLCdPublicPart;

typedef struct _XLCdPublicRec {
    XLCdCoreRec core;	
    XLCdPublicPart pub;
} XLCdPublicRec, *XLCdPublic;

extern XLCdMethods _xlcPublicMethods;

_XFUNCPROTOBEGIN

extern XLCd _XlcCreateLC(
#if NeedFunctionPrototypes
    char*	/* name */,
    XLCdMethods	/* methods */
#endif
);

extern void _XlcDestroyLC(
#if NeedFunctionPrototypes
    XLCd	/* lcd */
#endif
);

extern Bool _XlcParseCharSet(
#if NeedFunctionPrototypes
    XlcCharSet	/* charset */
#endif
);

extern XlcCharSet _XlcCreateDefaultCharSet(
#if NeedFunctionPrototypes
    char*	/* name */,
    char*	/* control_sequence */
#endif
);

extern XFontSet _XomGenericCreateFontSet(
#if NeedFunctionPrototypes
    XLCd	/* lcd */,
    Display*	/* dpy */,
    char*	/* base_name */,
    char**	/* name_list */,
    int		/* count */,
    char***	/* missing_charset_list */,
    int*	/* missing_charset_count */
#endif
);

extern XIM _XimOpenIM(
#if NeedFunctionPrototypes
    XLCd	/* lcd */,
    Display*	/* dpy */,
    XrmDatabase	/* rdb */,
    char*	/* res_name */,
    char*	/* res_class */
#endif
);

extern Bool _XimRegisterIMInstantiateCallback(
#if NeedFunctionPrototypes
    XLCd	/* lcd */,
    Display*	/* dpy */,
    XIMProc	/* callback */,
    XPointer*	/* client_data */
#endif
);

extern Bool _XimUnRegisterIMInstantiateCallback(
#if NeedFunctionPrototypes
    XLCd	/* lcd */,
    Display*	/* dpy */,
    XIMProc	/* callback */
#endif
);

extern XrmMethods _XrmDefaultInitParseInfo(
#if NeedFunctionPrototypes
    XLCd	/* lcd */,
    XPointer*	/* state */
#endif
);

extern int _XmbTextPropertyToTextList(
#if NeedFunctionPrototypes
    XLCd		/* lcd */,
    Display*		/* dpy */,
    XTextProperty*	/* text_prop */,
    char***		/* list_ret */,
    int*		/* count_ret */
#endif
);

extern int _XwcTextPropertyToTextList(
#if NeedFunctionPrototypes
    XLCd		/* lcd */,
    Display*		/* dpy */,
    XTextProperty*	/* text_prop */,
    wchar_t***		/* list_ret */,
    int*		/* count_ret */
#endif
);

extern int _XmbTextListToTextProperty(
#if NeedFunctionPrototypes
    XLCd		/* lcd */,
    Display*		/* dpy */,
    char**		/* list */,
    int			/* count */,
    XICCEncodingStyle	/* style */,
    XTextProperty*	/* text_prop */
#endif
);

extern int _XwcTextListToTextProperty(
#if NeedFunctionPrototypes
    XLCd		/* lcd */,
    Display*		/* dpy */,
    wchar_t**		/* list */,
    int			/* count */,
    XICCEncodingStyle	/* style */,
    XTextProperty*	/* text_prop */
#endif
);

extern void _XwcFreeStringList(
#if NeedFunctionPrototypes
    XLCd	/* lcd */,
    wchar_t**	/* list */
#endif
);

extern char *_XlcResolveLocaleName(
#if NeedFunctionPrototypes
    char*	/* name */,
    char*	/* language */,
    char*	/* territory */,
    char*	/* codeset */
#endif
);

extern char *_XlcResolveDBName(
#if NeedFunctionPrototypes
    char*	/* name */
#endif
);

extern char *_XlcResolveI18NPath(
#if NeedFunctionPrototypes
    char*	/* name */
#endif
);

extern XPointer _XlcCreateLocaleDataBase(
#if NeedFunctionPrototypes
    XLCd	/* lcd */
#endif
);

extern void _XlcDestroyLocaleDataBase(
#if NeedFunctionPrototypes
    XLCd	/* lcd */
#endif
);

extern void _XlcGetLocaleDataBase(
#if NeedFunctionPrototypes
    XLCd	/* lcd */,
    char*	/* category */,
    char*	/* name */,
    char***	/* value */,
    int*	/* count */
#endif
);

extern int _Xwcslen(
#if NeedFunctionPrototypes
    wchar_t*	/* wstr */
#endif
);

extern wchar_t *_Xwcscpy(
#if NeedFunctionPrototypes
    wchar_t*	/* wstr1 */,
    wchar_t*	/* wstr2 */
#endif
);

extern int _Xlcctstombs(
#if NeedFunctionPrototypes
    XLCd	/* lcd */,
    char*	/* to */,
    char*	/* from */,
    int		/* to_len */
#endif
);

extern int _Xlcctstowcs(
#if NeedFunctionPrototypes
    XLCd	/* lcd */,
    wchar_t*	/* to */,
    char*	/* from */,
    int		/* to_len */
#endif
);

extern int _XlcCompareISOLatin1(
#if NeedFunctionPrototypes
    char*	/* str1 */,
    char*	/* str2 */
#endif
);

extern int _XlcNCompareISOLatin1(
#if NeedFunctionPrototypes
    char*	/* str1 */,
    char*	/* str2 */,
    int		/* len */
#endif
);

extern char *_XlcAlloc(
#if NeedFunctionPrototypes
    int	/* length */
#endif
);

extern void _XlcFree(
#if NeedFunctionPrototypes
    char*	/* ptr */
#endif
);

_XFUNCPROTOEND

#endif  /* _XLCPUBLICI_H_ */
