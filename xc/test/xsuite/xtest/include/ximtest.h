/* $XConsortium: ximtest.h,v 1.2 94/01/29 16:34:41 rws Exp $ */
/*
 * Copyright 1993 by Sun Microsystems, Inc. Mountain View, CA.
 *
 *                   All Rights Reserved
 *
 * Permission  to  use,  copy,  modify,  and  distribute   this
 * software  and  its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright no-
 * tice  appear  in all copies and that both that copyright no-
 * tice and this permission notice appear in  supporting  docu-
 * mentation,  and  that the names of Sun or MIT not be used in
 * advertising or publicity pertaining to distribution  of  the
 * software  without specific prior written permission. Sun and
 * M.I.T. make no representations about the suitability of this
 * software for any purpose. It is provided "as is" without any
 * express or implied warranty.
 *
 * SUN DISCLAIMS ALL WARRANTIES WITH REGARD TO  THIS  SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-
 * NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SUN BE  LI-
 * ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
 * PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
 * THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include <X11/Xfuncproto.h>
#include <X11/Xresource.h>

#define	CB_MAX	8
#define	CB_PE_START	0
#define	CB_PE_DONE	1
#define	CB_PE_DRAW	2
#define	CB_PE_CARET	3
#define	CB_ST_START	4
#define	CB_ST_DONE	5
#define	CB_ST_DRAW	6
#define	CB_GEOM	7

#define MAXIDLEN                  32
#define MAXLINELEN                256
#define MAXFNAME		1024

#define	IM_FNAME_PREFIX	"im"
#define	IM_FNAME_RESPONSE	"response"
#define	IM_FNAME_SAVE		"im_sav"
#define	IM_FNAME_DATA		"im_dat"
#define	IM_FNAME_ACTION	"im_act"

/* bit indexs for styles mask */
#define IM_PE_AREA                      0
#define IM_PE_CALLBACKS         1
#define IM_PE_POSITION          2
#define IM_PE_NOTHING           3
#define IM_PE_NONE                      4

#define IM_STATUS_AREA          9
#define IM_STATUS_CALLBACKS     10
#define IM_STATUS_NOTHING       11
#define IM_STATUS_NONE          12

#define MAX_IM_STYLES   12

#define RESPONSE_KEY_VERSION	0
#define RESPONSE_KEY_XRELEASE	1
#define RESPONSE_KEY_PE_STYLE	2
#define RESPONSE_KEY_STATUS_STYLE	3
#define RESPONSE_KEY_RESPONSE	4

#define RESPONSE_VERSION	"11.0"
#define RESPONSE_XRELEASE	"3200"

#define XIM_TEXT_LENGTH		0
#define XIM_TEXT_FEEDBACK	1
#define XIM_TEXT_IS_WCHAR	2
#define XIM_TEXT_STRING		3

#define PE_DRAW_CARET	0
#define PE_DRAW_FIRST	1
#define PE_DRAW_LENGTH	2
#define PE_DRAW_TEXT		3

#define PE_CARET_POSITION	0
#define PE_CARET_DIRECTION	1
#define PE_CARET_STYLE		2

#define ST_DRAW_TYPE	0
#define ST_DRAW_DATA	1

#define ACTION_KEY_VERSION			0
#define ACTION_KEY_XRELEASE		1
#define ACTION_KEY_PE_STYLE		2
#define ACTION_KEY_STATUS_STYLE	3
#define ACTION_KEY_ACTION			4

#define ACTION_VERSION	"11.0"
#define ACTION_XRELEASE	"3200"

#define SUBACT_KEY		0
#define SUBACT_MBRESET	1
#define SUBACT_WCRESET	2

#define MAX_CB_RESPONSE				256 /* this is a guess */
typedef struct {
	int top;
	int stack[MAX_CB_RESPONSE];
	char *data[MAX_CB_RESPONSE];
} cbstk_def;

/*
 * Xim Configuration struct
 */
struct ximconfig {
	char *locale;           /* locales to test */
	char *locale_modifiers;    /* locale modifiers to test */
	char *fontsets;            /* font sets for XCreateFontSet */
	int	save_im;
};

extern struct ximconfig ximconfig;

void localestartup(
#if NeedFunctionPrototypes
    void
#endif
);
void localecleanup(
#if NeedFunctionPrototypes
    void
#endif
);
XrmDatabase rm_db_open(
#if NeedFunctionPrototypes
    void
#endif
);
XIM im_open(
#if NeedFunctionPrototypes
    XrmDatabase /* db */
#endif
);
XIC ic_open(
#if NeedFunctionPrototypes
    XIM /* im */,
    Window /* win */,
    XIMStyle /* which_style */
#endif
);
XIC xim_ic_init(
#if NeedFunctionPrototypes
    char* /* plocale */
#endif
);
