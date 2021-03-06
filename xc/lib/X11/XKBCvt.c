/* "$XConsortium: XKBCvt.c,v 1.15 95/06/08 23:20:39 gildea Exp $"; */
/*

Copyright (c) 1988, 1989  X Consortium

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the X Consortium.

*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#define NEED_EVENTS
#include "Xlibint.h"
#include <X11/Xutil.h>
#include <X11/Xmd.h>
#define XK_LATIN1
#define XK_PUBLISHING
#include <X11/keysym.h>
#include <X11/extensions/XKBproto.h>
#include "XKBlibint.h"
#include <X11/Xlocale.h>
#include <ctype.h>
#include <X11/Xos.h>

#ifdef X_NOT_STDC_ENV
extern char *getenv();
#endif

#ifdef __STDC__
#define Const const
#else
#define Const /**/
#endif
#if defined(__STDC__) && !defined(NORCONST)
#define RConst const
#else
#define RConst /**/
#endif

/* bit (1<<i) means character is in codeset i */
static unsigned short Const latin1[128] =
  {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
   0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
   0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
   0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
   0x10ee, 0x0000, 0x1000, 0x1084, 0x102e, 0x1000, 0x1080, 0x108e, /* 10 */
   0x108e, 0x1080, 0x0000, 0x1080, 0x1080, 0x10ee, 0x1000, 0x1008,
   0x108e, 0x1080, 0x1084, 0x1084, 0x108e, 0x1004, 0x1000, 0x1084, /* 11 */
   0x100e, 0x1000, 0x0000, 0x1080, 0x1000, 0x1084, 0x1000, 0x0000,
   0x0004, 0x000e, 0x000e, 0x0008, 0x000e, 0x0008, 0x0008, 0x0006, /* 12 */
   0x0004, 0x000e, 0x0004, 0x000e, 0x0004, 0x000e, 0x000e, 0x0004,
   0x0000, 0x0004, 0x0004, 0x0006, 0x000e, 0x0008, 0x000e, 0x000e, /* 13 */
   0x0008, 0x0004, 0x000e, 0x000c, 0x000e, 0x0002, 0x0000, 0x000e,
   0x0004, 0x000e, 0x000e, 0x0008, 0x000e, 0x0008, 0x0008, 0x0006, /* 14 */
   0x0004, 0x000e, 0x0004, 0x000e, 0x0004, 0x000e, 0x000e, 0x0004,
   0x0000, 0x0004, 0x0004, 0x0006, 0x000e, 0x0008, 0x000e, 0x000e, /* 15 */
   0x0008, 0x0004, 0x000e, 0x000c, 0x000e, 0x0002, 0x0000, 0x0000};

/* bit (1<<i) means character is in codeset i */
static unsigned short Const latin2[128] =
  {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
   0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
   0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
   0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
   0x0000, 0x0008, 0x0004, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, /* 10 */
   0x0000, 0x0008, 0x0004, 0x0000, 0x0000, 0x0000, 0x0008, 0x0004,
   0x0000, 0x0008, 0x0008, 0x0000, 0x0000, 0x0000, 0x0000, 0x0008, /* 11 */
   0x0000, 0x0008, 0x0004, 0x0000, 0x0000, 0x0000, 0x0008, 0x0004,
   0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, /* 12 */
   0x0008, 0x0000, 0x0008, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
   0x0008, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, /* 13 */
   0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
   0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, /* 14 */
   0x0008, 0x0000, 0x0008, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
   0x0008, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, /* 15 */
   0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x000c};

/* maps Cyrillic keysyms to 8859-5 */
static unsigned char Const cyrillic[128] =
   {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xf2, 0xf3, 0xf1, 0xf4, 0xf5, 0xf6, 0xf7, /* 10 */
    0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0x00, 0xfe, 0xff,
    0xf0, 0xa2, 0xa3, 0xa1, 0xa4, 0xa5, 0xa6, 0xa7, /* 11 */
    0xa8, 0xa9, 0xaa, 0xab, 0xac, 0x00, 0xae, 0xaf,
    0xee, 0xd0, 0xd1, 0xe6, 0xd4, 0xd5, 0xe4, 0xd3, /* 12 */
    0xe5, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde,
    0xdf, 0xef, 0xe0, 0xe1, 0xe2, 0xe3, 0xd6, 0xd2, /* 13 */
    0xec, 0xeb, 0xd7, 0xe8, 0xed, 0xe9, 0xe7, 0xea,
    0xce, 0xb0, 0xb1, 0xc6, 0xb4, 0xb5, 0xc4, 0xb3, /* 14 */
    0xc5, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe,
    0xbf, 0xcf, 0xc0, 0xc1, 0xc2, 0xc3, 0xb6, 0xb2, /* 15 */
    0xcc, 0xcb, 0xb7, 0xc8, 0xcd, 0xc9, 0xc7, 0xca};

/* maps Greek keysyms to 8859-7 */
static unsigned char Const greek[128] =
   {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xb6, 0xb8, 0xb9, 0xba, 0xda, 0x00, 0xbc, /* 10 */
    0xbe, 0xdb, 0x00, 0xbf, 0x00, 0x00, 0xb5, 0xaf,
    0x00, 0xdc, 0xdd, 0xde, 0xdf, 0xfa, 0xc0, 0xfc, /* 11 */
    0xfd, 0xfb, 0xe0, 0xfe, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, /* 12 */
    0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
    0xd0, 0xd1, 0xd3, 0x00, 0xd4, 0xd5, 0xd6, 0xd7, /* 13 */
    0xd8, 0xd9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, /* 14 */
    0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
    0xf0, 0xf1, 0xf3, 0xf2, 0xf4, 0xf5, 0xf6, 0xf7, /* 15 */
    0xf8, 0xf9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

#define sLatin1         0
#define sLatin2         1
#define sLatin3         2
#define sLatin4         3
#define sKana           4
#define sX0201          0x01000004
#define sArabic         5
#define sCyrillic       6
#define sGreek          7
#define sAPL            11
#define sHebrew         12

static unsigned long	WantLatin1 = sLatin1;
static unsigned long	WantLatin2 = sLatin2;
static unsigned long	WantLatin3 = sLatin3;
static unsigned long	WantLatin4 = sLatin4;
static unsigned long	WantKana = sKana;
static unsigned long	WantX0201 = sX0201;
static unsigned long	WantArabic = sArabic;
static unsigned long	WantCyrillic = sCyrillic;
static unsigned long	WantGreek = sGreek;
static unsigned long	WantAPL = sAPL;
static unsigned long	WantHebrew = sHebrew;

static int _XkbHandleSpecialSym(keysym, buffer, nbytes, status)
    KeySym	 keysym;
    char	*buffer;
    int		 nbytes;
    Status	*status;
{

    /* try to convert to Latin-1, handling ctrl */
    if (!(((keysym >= XK_BackSpace) && (keysym <= XK_Clear)) ||
	   (keysym == XK_Return) || (keysym == XK_Escape) ||
	   (keysym == XK_KP_Space) || (keysym == XK_KP_Tab) ||
	   (keysym == XK_KP_Enter) || 
	   ((keysym >= XK_KP_Multiply) && (keysym <= XK_KP_9)) ||
	   (keysym == XK_KP_Equal) ||
	   (keysym == XK_Delete)))
	return 0;

    /* if X keysym, convert to ascii by grabbing low 7 bits */
    if (keysym == XK_KP_Space)
	 buffer[0] = XK_space & 0x7F; /* patch encoding botch */
    else if (keysym == XK_hyphen)
	 buffer[0] = XK_minus & 0xFF; /* map to equiv character */
    else buffer[0] = keysym & 0x7F;
    return 1;
}

int _XkbKSToKnownSet (priv, keysym, buffer, nbytes, status)
    XPointer priv;
    KeySym keysym;
    char *buffer;
    int nbytes;
    Status *status;
{
    unsigned long kset,keysymSet;
    int	count,isLatin1;

    keysymSet = *((unsigned long *)priv);
    kset = keysymSet&0xffffff;

    /* convert "dead" diacriticals for dumb applications */
    if ( (keysym&0xffffff00)== 0xfe00 ) {
	switch ( keysym ) {
	    case XK_dead_grave:		keysym = XK_grave; break;
	    case XK_dead_acute:		keysym = XK_acute; break;
	    case XK_dead_circumflex:	keysym = XK_asciicircum; break;
	    case XK_dead_tilde:		keysym = XK_asciitilde; break;
	    case XK_dead_macron:	keysym = XK_macron; break;
	    case XK_dead_breve:		keysym = XK_breve; break;
	    case XK_dead_abovedot:	keysym = XK_abovedot; break;
	    case XK_dead_diaeresis:	keysym = XK_diaeresis; break;
	    case XK_dead_abovering:	keysym = XK_degree; break;
	    case XK_dead_doubleacute:	keysym = XK_doubleacute; break;
	    case XK_dead_caron:		keysym = XK_caron; break;
	    case XK_dead_cedilla:	keysym = XK_cedilla; break;
	    case XK_dead_ogonek	:	keysym = XK_ogonek; break;
	    case XK_dead_iota:		keysym = XK_Greek_iota; break;
#ifdef XK_KATAKANA
	    case XK_dead_voicedsound:	keysym = XK_voicedsound; break;
	    case XK_dead_semivoicedsound:keysym = XK_semivoicedsound; break;
#endif
	}
    }

    isLatin1 = ((keysym&0xffffff00)==0);
    count = 0;

    if ((keysym&0xffffff00)==0xff00) {
	return _XkbHandleSpecialSym(keysym, buffer, nbytes, status);
    }
    else if ( keysym == NoSymbol )
	return 0;
    else if ((keysym >> 8) == kset) {
        count = 1;
        switch (keysymSet) {
        case sKana:
            buffer[0] = (keysym & 0xff);
            if (buffer[0] == 0x7e)
                count = 0;
            break;
        case sCyrillic:
            buffer[0] = cyrillic[keysym & 0x7f];
            break;
        case sGreek:
            buffer[0] = greek[keysym & 0x7f];
            if (!buffer[0])
                count = 0;
            break;
        default:
            buffer[0] = (keysym & 0xff);
            break;
        }
    } else if ((keysymSet != 0) && (isLatin1) && (keysym & 0x80)) {
	if (latin1[keysym & 0x7f] & (1 << kset)) {
            /* Most non-latin1 locales use some latin-1 upper half
               keysyms as defined by bitpatterns in array latin1.
               Enforce it. */
            buffer[0] = (keysym & 0xff);
            count = 1;
        } else {
	    if ((keysymSet == sHebrew) && (keysym == XK_multiply))
		buffer[0] = 0xaa;
            else if ((keysymSet == sHebrew) && (keysym == XK_division))
		buffer[0] = 0xba;
            else if ((keysymSet == sCyrillic) && (keysym == XK_section))
		buffer[0] = 0xfd;
            else if ((keysymSet == sX0201) && (keysym == XK_yen))
		buffer[0] = 0x5c;
            else
		count = 0;
	}
    } else if (isLatin1) {
        if ((keysymSet == sX0201) &&
            ((keysym == XK_backslash) || (keysym == XK_asciitilde)))
            count = 0;
	if ( (keysym&0x80)==0 ) {
	    buffer[0] = keysym&0x7f;
	    count = 1;
	}
    } else if (((keysym >> 8) == sLatin2) &&
               (keysym & 0x80) && (latin2[keysym & 0x7f] & (1 << kset))) {
        buffer[0] = (keysym & 0xff);
        count = 1;
    } else if ((keysymSet == sGreek) &&
               ((keysym == XK_leftsinglequotemark) ||
                (keysym == XK_rightsinglequotemark))) {
        buffer[0] = keysym - (XK_leftsinglequotemark - 0xa1);
        count = 1;
    }
    if (count<nbytes)
	buffer[count]= '\0';
    return count;
}

typedef struct _XkbToKS {
	unsigned	 prefix;
	char		*map;
} XkbToKS;

static KeySym
_XkbKnownSetToKS(priv,buffer,nbytes,status)
    XPointer priv;
    char *buffer;
    int nbytes;
    Status *status;
{
    if (nbytes!=1)
	return NoSymbol;
    if (((buffer[0]&0x80)==0)&&(buffer[0]>=32))
	return buffer[0];
    else if ((buffer[0]&0x7f)>=32) { 
	XkbToKS *map= (XkbToKS *)priv;
	if ( map ) {
	    if ( map->map )	return map->prefix|map->map[buffer[0]&0x7f];
	    else		return map->prefix|buffer[0];
	}
	return buffer[0];
    }
    return NoSymbol;
}

int _XkbKSToThai (priv, keysym, buffer, nbytes, status)
    XPointer priv;
    KeySym keysym;
    char *buffer;
    int nbytes;
    Status *status;
{

    if ((keysym&0xffffff00)==0xff00) {
        return _XkbHandleSpecialSym(keysym, buffer, nbytes, status);
    }
    else if (((keysym&0xffffff80)==0xd80)||((keysym&0xffffff80)==0)) {
        if (nbytes>0) {
            buffer[0]= keysym&0xff;
            if (nbytes>1)
                buffer[1]= '\0';
            return 1;
        }
    }
    return 0;
}

static KeySym
_XkbThaiToKS(priv,buffer,nbytes,status)
    XPointer priv;
    char *buffer;
    int nbytes;
    Status *status;
{
    if (nbytes!=1)
        return NoSymbol;
    if (((buffer[0]&0x80)==0)&&(buffer[0]>=32))
        return buffer[0];
    else if ((buffer[0]&0x7f)>=32) {
        return 0xd00|buffer[0];
    }
    return NoSymbol;
}

static KeySym
__XkbDefaultToUpper(sym)
    KeySym	sym;
{
    KeySym	lower,upper;

    XConvertCase(sym, &lower, &upper);
    return upper;
}

/***====================================================================***/


static XkbConverters RConst cvt_ascii = {
	_XkbKSToKnownSet,(XPointer)&WantLatin1,_XkbKnownSetToKS,NULL,__XkbDefaultToUpper
};

static XkbConverters RConst cvt_latin1 = {
	_XkbKSToKnownSet,(XPointer)&WantLatin1,_XkbKnownSetToKS,NULL,NULL
};

static XkbConverters RConst cvt_latin2 = {
	_XkbKSToKnownSet,(XPointer)&WantLatin2,_XkbKnownSetToKS,NULL,NULL
};

static XkbConverters RConst cvt_latin3 = {
	_XkbKSToKnownSet,(XPointer)&WantLatin3,_XkbKnownSetToKS,NULL,NULL
};

static XkbConverters RConst cvt_latin4 = {
	_XkbKSToKnownSet,(XPointer)&WantLatin4,_XkbKnownSetToKS,NULL,NULL
};

static XkbConverters RConst cvt_kana = {
	_XkbKSToKnownSet,(XPointer)&WantKana,_XkbKnownSetToKS,NULL,NULL
};

static XkbConverters RConst cvt_X0201 = {
	_XkbKSToKnownSet,(XPointer)&WantX0201,_XkbKnownSetToKS,NULL,NULL
};

static XkbConverters RConst cvt_Arabic = {
	_XkbKSToKnownSet,(XPointer)&WantArabic,_XkbKnownSetToKS,NULL,NULL
};

static XkbConverters RConst cvt_Cyrillic = {
	_XkbKSToKnownSet,(XPointer)&WantCyrillic,_XkbKnownSetToKS,NULL,NULL
};

static XkbConverters RConst cvt_Greek = {
	_XkbKSToKnownSet,(XPointer)&WantGreek,_XkbKnownSetToKS,NULL,NULL
};

static XkbConverters RConst cvt_APL = {
	_XkbKSToKnownSet,(XPointer)&WantAPL,_XkbKnownSetToKS,NULL,NULL
};

static XkbConverters RConst cvt_Hebrew = {
	_XkbKSToKnownSet,(XPointer)&WantHebrew,_XkbKnownSetToKS,NULL,NULL
};

static XkbConverters    cvt_Thai = {
        _XkbKSToThai, NULL, _XkbThaiToKS, NULL, NULL
};

static int
Strcmp(str1, str2)
    char *str1, *str2;
{
    char str[256];
    char c, *s;

    for (s = str; c = *str1++; ) {
	if (isupper(c))
	    c = tolower(c);
	*s++ = c;
    }
    *s = '\0';
    return (strcmp(str, str2));
}

int 
_XkbGetConverters(charset, cvt_rtrn)
    char *charset;
    XkbConverters *cvt_rtrn;
{
    if ( cvt_rtrn ) {
	if ( (charset==NULL) || (Strcmp(charset,"ascii")==0) )
	     *cvt_rtrn = cvt_ascii;
	else if (Strcmp(charset,"iso8859-1")==0)
	     *cvt_rtrn = cvt_latin1;
	else if (Strcmp(charset, "iso8859-2")==0)
	     *cvt_rtrn = cvt_latin2;
	else if (Strcmp(charset, "iso8859-3")==0)
	     *cvt_rtrn = cvt_latin3;
	else if (Strcmp(charset, "iso8859-4")==0)
	     *cvt_rtrn = cvt_latin4;
	else if (Strcmp(charset, "iso8859-5")==0)
	     *cvt_rtrn = cvt_Cyrillic;
	else if (Strcmp(charset, "iso8859-6")==0)
	     *cvt_rtrn = cvt_Arabic;
	else if (Strcmp(charset, "iso8859-7")==0)
	     *cvt_rtrn = cvt_Greek;
	else if (Strcmp(charset, "iso8859-8")==0)
	     *cvt_rtrn = cvt_Hebrew;
	else if (Strcmp(charset, "apl")==0)
	     *cvt_rtrn = cvt_APL;
	else if (Strcmp(charset, "jisx0201")==0)
	     *cvt_rtrn = cvt_X0201;
	else if (Strcmp(charset, "kana")==0)
	     *cvt_rtrn = cvt_kana;
	else if (Strcmp(charset, "tis620.2533-1")==0)
	     *cvt_rtrn = cvt_Thai;
	/* other codesets go here */
	else *cvt_rtrn = cvt_latin1;
	return 1;
    }
    *cvt_rtrn= cvt_latin1;
    return 1;
}

/***====================================================================***/

#ifdef XKB_EXTEND_LOOKUP_STRING
#define	CHARSET_FILE	"/usr/lib/X11/input/charsets"
static char *_XkbKnownLanguages = "c=ascii:da,de,en,es,fi,fr,is,it,nl,no,pt,sv=iso8859-1:pl,cs=iso8859-2:ru=iso8859-5:el=iso8859-7:th,th_TH,th_TH.TACTIS:tis620.2533-1";

char	*
_XkbGetCharset(locale)
    char *locale;
{
    static char buf[100];
    char lang[256];
    char *start,*tmp,*end,*next,*set;
    char *country,*charset;
    int	 len;

    if ( locale == NULL ) {
	tmp = getenv( "_XKB_CHARSET" );
	if ( tmp )
	    return tmp;
	else locale = setlocale(LC_CTYPE,NULL);
    }
    if ( locale == NULL )
	return NULL;

    for (tmp = lang; *tmp = *locale++; tmp++) {
	if (isupper(*tmp))
	    *tmp = tolower(*tmp);
    }
    country = strchr( lang, '_');
    if ( country ) {
	*country++ = '\0';
	charset = strchr( country, '.' );
	if ( charset )	*charset++ = '\0';
	if ( charset ) {
	    strncpy(buf,charset,99);
	    buf[99] = '\0';
	    return buf;
	}
    }
    else { 
	charset = NULL;
    }

    if ((tmp = getenv("_XKB_LOCALE_CHARSETS"))!=NULL) {
	start = Xmalloc(strlen(tmp) + 1);
	strcpy(start, tmp);
	tmp = start;
    } else {
	struct stat sbuf;
	FILE *file;
	if ( (stat(CHARSET_FILE,&sbuf)==0) && (sbuf.st_mode&S_IFREG) &&
	    (file = fopen(CHARSET_FILE,"r")) ) {
	    tmp = Xmalloc(sbuf.st_size+1);
	    if (tmp!=NULL) {
		sbuf.st_size = fread(tmp,1,sbuf.st_size,file);
		tmp[sbuf.st_size] = '\0';
	    }
	    fclose(file);
	}
    }

    if ( tmp == NULL ) {
	tmp = Xmalloc(strlen(_XkbKnownLanguages) + 1);
	if (!tmp)
	    return NULL;
	strcpy(tmp, _XkbKnownLanguages);
    }
    start = tmp;
    do {
	if ( (set=strchr(tmp,'=')) == NULL )
	    break;
	*set++ = '\0';
	if ( (next=strchr(set,':')) != NULL ) 
	    *next++ = '\0';
	while ( tmp && *tmp ) {
	    if ( (end=strchr(tmp,',')) != NULL )
		*end++ = '\0';
	    if ( Strcmp( tmp, lang ) == 0 ) {
		strncpy(buf,set,100);
		buf[99] = '\0';
		Xfree(start);
		return buf;
	    }
	    tmp = end;
	}
	tmp = next;
    } while ( tmp && *tmp );
    Xfree(start);
    return NULL;
}
#else
char	*
_XkbGetCharset(locale)
    char *locale;
{
char *tmp;

    if ( locale == NULL ) {
	tmp = getenv( "_XKB_CHARSET" );
	if ( tmp )
	    return tmp;
	else locale = setlocale(LC_CTYPE,NULL);
    }
    return NULL;
}
#endif


