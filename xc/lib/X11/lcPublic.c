/* $XConsortium: lcPublic.c,v 1.2 93/09/17 14:24:14 rws Exp $ */
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
 Modifier : Takashi Fujiwara     FUJITSU LIMITED 
                                 fujiwara@a80.tech.yk.fujitsu.co.jp

******************************************************************/

#include <stdio.h>
#include "Xlibint.h"
#include "XlcPubI.h"

static char *default_string();

static XLCd create();
static Bool initialize();
static void destroy();
static char *get_values();

static XLCdPublicMethodsRec publicMethods = {
    {
	_XlcDefaultMapModifiers,
	_XomGenericCreateFontSet,
	_XimOpenIM,
	_XimRegisterIMInstantiateCallback,
	_XimUnRegisterIMInstantiateCallback,
	_XrmDefaultInitParseInfo,
	_XmbTextPropertyToTextList,
	_XwcTextPropertyToTextList,
	_XmbTextListToTextProperty,
	_XwcTextListToTextProperty,
	_XwcFreeStringList,
	default_string
    }, 
    {
	NULL,
	create,
	initialize,
	destroy,
	get_values,
	_XlcGetLocaleDataBase
    }
};

XLCdMethods _XlcPublicMethods = (XLCdMethods) &publicMethods;

static char *
default_string(lcd)
    XLCd lcd;
{
    return XLC_PUBLIC(lcd, default_string);
}

static XLCd
create(name, methods)
    char *name;
    XLCdMethods methods;
{
    XLCd lcd;

    lcd = (XLCd) Xmalloc(sizeof(XLCdRec));
    if (lcd == NULL)
        return (XLCd) NULL;
    bzero((char *) lcd, sizeof(XLCdRec));

    lcd->core = (XLCdCore) Xmalloc(sizeof(XLCdPublicRec));
    if (lcd->core == NULL)
	goto err;
    bzero((char *) lcd->core, sizeof(XLCdPublicRec));

    return lcd;

err:
    XFree(lcd);
    return (XLCd) NULL;
}

static Bool
load_public(lcd)
    XLCd lcd;
{
    XLCdPublicPart *pub = XLC_PUBLIC_PART(lcd);
    char **values, *str;
    int num;

    if(_XlcCreateLocaleDataBase(lcd) == NULL)
	return False;

    _XlcGetResource(lcd, "XLC_XLOCALE", "mb_cur_max", &values, &num);
    if (num > 0) {
	pub->mb_cur_max = atoi(values[0]);
	if (pub->mb_cur_max < 1)
	    pub->mb_cur_max = 1;
    } else
	pub->mb_cur_max = 1;

    _XlcGetResource(lcd, "XLC_XLOCALE", "state_dependent", &values, &num);
    if (num > 0 && !_XlcCompareISOLatin1(values[0], "True"))
	pub->is_state_depend = True;
    else
	pub->is_state_depend = False;

    _XlcGetResource(lcd, "XLC_XLOCALE", "encoding_name", &values, &num);
    str = (num > 0) ? values[0] : "STRING";
    pub->encoding_name = (char*) Xmalloc(strlen(str) + 1);
    if (pub->encoding_name == NULL)
	return False;
    strcpy(pub->encoding_name, str);

    return True;
}

static Bool
initialize_core(lcd)
    XLCd lcd;
{
    XLCdMethods methods = lcd->methods;
    XLCdMethods core = &publicMethods.core;

    if (methods->map_modifiers == NULL)
	methods->map_modifiers = core->map_modifiers;

    if (methods->create_fontset == NULL)
	methods->create_fontset = core->create_fontset;

    if (methods->open_im == NULL)
	methods->open_im = core->open_im;

    if (methods->register_callback == NULL)
	methods->register_callback = core->register_callback;

    if (methods->unregister_callback == NULL)
	methods->unregister_callback = core->unregister_callback;

    if (methods->init_parse_info == NULL)
	methods->init_parse_info = core->init_parse_info;

    if (methods->mb_text_prop_to_list == NULL)
	methods->mb_text_prop_to_list = core->mb_text_prop_to_list;

    if (methods->wc_text_prop_to_list == NULL)
	methods->wc_text_prop_to_list = core->wc_text_prop_to_list;

    if (methods->mb_text_list_to_prop == NULL)
	methods->mb_text_list_to_prop = core->mb_text_list_to_prop;

    if (methods->wc_text_list_to_prop == NULL)
	methods->wc_text_list_to_prop = core->wc_text_list_to_prop;

    if (methods->wc_free_string_list == NULL)
	methods->wc_free_string_list = core->wc_free_string_list;

    if (methods->default_string == NULL)
	methods->default_string = core->default_string;

    return True;
}

extern Bool _XlcInitCTInfo();

static Bool
initialize(lcd)
    XLCd lcd;
{
    XLCdPublicMethodsPart *methods = XLC_PUBLIC_METHODS(lcd);
    XLCdPublicMethodsPart *pub_methods = &publicMethods.pub;
    XLCdPublicPart *pub = XLC_PUBLIC_PART(lcd);
    char lang[128], terr[128], code[128], *str;
    char *name;
#if !defined(X_NOT_STDC_ENV) && !defined(X_LOCALE)
    char siname[256];
    char *_XlcMapOSLocaleName();
#endif

    _XlcInitCTInfo();

    if (initialize_core(lcd) == False)
	return False;

    name = lcd->core->name;
#if !defined(X_NOT_STDC_ENV) && !defined(X_LOCALE)
    name = _XlcMapOSLocaleName(name, siname);
#endif
	
    if (_XlcResolveLocaleName(name, lang, terr, code) == NULL)
	return False;

    str = (char*) Xmalloc(strlen(name) + strlen(lang) + strlen(terr) +
			  strlen(code) + 4);
    if (str == NULL)
	return False;

    strcpy(str, name);
    pub->siname = str;
    str += strlen(str) + 1;

    strcpy(str, lang);
    pub->language = str;
    str += strlen(str) + 1;

    strcpy(str, terr);
    pub->territory = str;
    str += strlen(str) + 1;

    strcpy(str, code);
    pub->codeset = str;

    if (pub->default_string == NULL)
	pub->default_string = "";

    if (methods->get_values == NULL)
	methods->get_values = pub_methods->get_values;

    if (methods->get_resource == NULL)
	methods->get_resource = pub_methods->get_resource;

    return load_public(lcd);
}

static void
destroy_core(lcd)
	XLCd	lcd;
{
    if (lcd->core) {
	if (lcd->core->name)
            XFree(lcd->core->name);
	XFree(lcd->core);
    }

    XFree(lcd);
}

static void
destroy(lcd)
    XLCd lcd;
{
    XLCdPublicPart *pub = XLC_PUBLIC_PART(lcd);

    _XlcDestroyLocaleDataBase(lcd);

    if (pub->siname)
	XFree(pub->siname);

    destroy_core(lcd);
}

static char *
get_values(lcd, arg_list)
    register XLCd lcd;
    register XlcArgList arg_list;
{
    /* XXX */
    for ( ; arg_list->name; arg_list++) {
	if (!strcmp(arg_list->name, XlcNCodeset))
	    *((char **) arg_list->value) = XLC_PUBLIC(lcd, codeset);
	else if (!strcmp(arg_list->name, XlcNDefaultString))
	    *((char **) arg_list->value) = XLC_PUBLIC(lcd, default_string);
	else if (!strcmp(arg_list->name, XlcNEncodingName))
	    *((char **) arg_list->value) = XLC_PUBLIC(lcd, encoding_name);
	else if (!strcmp(arg_list->name, XlcNLanguage))
	    *((char **) arg_list->value) = XLC_PUBLIC(lcd, language);
	else if (!strcmp(arg_list->name, XlcNMbCurMax))
	    *((int *) arg_list->value) = XLC_PUBLIC(lcd, mb_cur_max);
	else if (!strcmp(arg_list->name, XlcNStateDependentEncoding))
	    *((Bool *) arg_list->value) = XLC_PUBLIC(lcd, is_state_depend);
	else if (!strcmp(arg_list->name, XlcNTerritory))
	    *((char **) arg_list->value) = XLC_PUBLIC(lcd, territory);
	else
	    return arg_list->name;
    }

    return (char *) NULL;
}
