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

#ifndef _XLCPUBLIC_H_
#define _XLCPUBLIC_H_

#include "Xlcint.h"

typedef struct {
    char *name;
    XPointer value;
} XlcArg, *XlcArgList;

#define XlcNCharSize "charSize"
#define XlcNCodeset "codeset"
#define XlcNControlSequence "controlSequence"
#define XlcNDefaultString "defaultString"
#define XlcNEncodingName "encodingName"
#define XlcNLanguage "language"
#define XlcNMbCurMax "mbCurMax"
#define XlcNName "name"
#define XlcNSetSize "setSize"
#define XlcNSide "side"
#define XlcNStateDependentEncoding "stateDependentEncoding"
#define XlcNTerritory "territory"

typedef enum {
    XlcUnknown, XlcC0, XlcGL, XlcC1, XlcGR, XlcGLGR, XlcOther
} XlcSide;

typedef struct _XlcCharSetRec {
    char *name;			/* character set name */
    XrmQuark xrm_name;
    char *encoding_name;	/* XLFD encoding name */
    XrmQuark xrm_encoding_name;
    XlcSide side;		/* GL, GR or others */
    int char_size;		/* number of bytes per character */
    int set_size;		/* graphic character sets */
    char *ct_sequence;		/* control sequence of CT */
    char* (*get_values)(
#if NeedVarargsPrototypes
	struct _XlcCharSetRec*	/* charset */,
	XlcArgList		/* arg_list */
#endif
    );
} XlcCharSetRec, *XlcCharSet;

/*
 * conversion methods
 */

typedef struct _XlcConvRec *XlcConv;

typedef struct _XlcConvMethodsRec{
    void (*close)(
#if NeedFunctionPrototypes
	XlcConv	/* conv */
#endif
    );
    int (*convert)(
#if NeedFunctionPrototypes
	XlcConv		/* conv */,
	XPointer*	/* from */,
	int*		/* from_left */,
	XPointer*	/* to */,
	int*		/* to_left */,
	XPointer*	/* args */,
	int		/* num_args */
#endif
    );
    void (*reset)(
#if NeedFunctionPrototypes
	XlcConv	/* conv */
#endif
    );
} XlcConvMethodsRec, *XlcConvMethods;

/*
 * conversion data
 */

#define XlcNMultiByte "multiByte"
#define XlcNWideChar "wideChar"
#define XlcNCompoundText "compoundText"
#define XlcNString "string"
#define XlcNCharSet "charSet"
#define XlcNChar "char"

typedef struct _XlcConvRec {
    XlcConvMethods methods;
    XPointer state;
} XlcConvRec;

typedef XlcConv (*XlcOpenConverterProc)(
#if NeedFunctionPrototypes
    XLCd	/* from_lcd */,
    char*	/* from_type */,
    XLCd	/* to_lcd */,
    char*	/* to_type */
#endif
);

_XFUNCPROTOBEGIN

extern char *_XlcGetLCValues(
#if NeedVarargsPrototypes
    XLCd	/* lcd */,
    ...
#endif
);

extern XlcCharSet _XlcGetCharSet(
#if NeedFunctionPrototypes
    char*	/* name */
#endif
);

extern Bool _XlcAddCharSet(
#if NeedFunctionPrototypes
    XlcCharSet	/* charset */
#endif
);

extern char *_XlcGetCSValues(
#if NeedVarargsPrototypes
    XlcCharSet	/* charset */,
    ...
#endif
);

extern XlcConv _XlcOpenConverter(
#if NeedFunctionPrototypes
    XLCd	/* from_lcd */,
    char*	/* from_type */,
    XLCd	/* to_lcd */,
    char*	/* to_type */
#endif
);

extern void _XlcCloseConverter(
#if NeedFunctionPrototypes
    XlcConv	/* conv */
#endif
);

extern int _XlcConvert(
#if NeedFunctionPrototypes
    XlcConv	/* conv */,
    XPointer*	/* from */,
    int*	/* from_left */,
    XPointer*	/* to */,
    int*	/* to_left */,
    XPointer*	/* args */,
    int		/* num_args */
#endif
);

extern void _XlcResetConverter(
#if NeedFunctionPrototypes
    XlcConv	/* conv */
#endif
);

extern Bool _XlcSetConverter(
#if NeedFunctionPrototypes
    XLCd			/* from_lcd */,
    char*			/* from_type */,
    XLCd			/* to_lcd */,
    char*			/* to_type */,
    XlcOpenConverterProc	/* open_converter */
#endif
);

extern void _XlcGetResource(
#if NeedFunctionPrototypes
    XLCd	/* lcd */,
    char*	/* category */,
    char*	/* class */,
    char***	/* value */,
    int*	/* count */
#endif
);

_XFUNCPROTOEND

#endif  /* _XLCPUBLIC_H_ */
