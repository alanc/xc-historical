/* 
 * $XConsortium: Xct.c,v 1.0 89/05/09 08:36:44 rws Exp $
 * Copyright 1989 by the Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of M.I.T. not be used in advertising
 * or publicity pertaining to distribution of the software without specific, 
 * written prior permission. M.I.T. makes no representations about the 
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 */

#include <stdio.h>
#include "Xct.h"

typedef struct _XctPriv {
    XctString		ptr;
    XctString		ptrend;
    XctHDirection	*dirstack;
    unsigned		dirsize;
} *XctPriv;

#define IsMore(priv) ((priv)->ptr != (priv)->ptrend)
#define AmountLeft(priv) ((priv)->ptrend - (priv)->ptr)

char *malloc();
char *realloc();

#define HT	0x09
#define NL	0x0a
#define ESC	0x1b
#define CSI	0x9b

#define IsLegalC0(data, c) (((c) == HT) || ((c) == NL) || \
			    (((data)->version > XctVersion) && \
			     ((data)->flags & XctAcceptC0Extensions)))

#define IsLegalC1(priv, c) (((data)->version > XctVersion) && \
			    ((data)->flags & XctAcceptC1Extensions))

#define IsI2(c) (((c) >= 0x20) && ((c) <= 0x2f))
#define IsI3(c) (((c) >= 0x30) && ((c) <= 0x3f))
#define IsESCF(c) (((c) >= 0x30) && ((c) <= 0x7e))
#define IsCSIF(c) (((c) >= 0x40) && ((c) <= 0x7e))
#define IsC0(c) ((c) <= 0x1f)
#define IsGL(c) (((c) >= 0x20) && ((c) <= 0x7f))
#define IsC1(c) (((c) >= 0x80) && ((c) <= 0x9f))
#define IsGR(c) ((c) >= 0xa0)

static void
ComputeGLGR(data)
    register XctData data;
{
    /* XXX this will need more work if more sets are registered */
    if ((data->GL_set_size == 94) && (data->GL_char_size == 1) &&
	(data->GR_set_size == 96) && (data->GR_char_size == 1))
	data->GLGR_encoding = data->GR_encoding;
    else
	data->GLGR_encoding = (char *)NULL;
}

static int
HandleGL(data, c)
    register XctData data;
    unsigned char c;
{
    switch (c) {
    case 0x42:
	data->GL = "\102";
	data->GL_encoding = "ISO8859-1";
	break;
    default:
	return 0;
    }
    data->GL_set_size = 94;
    data->GL_char_size = 1;
    ComputeGLGR(data);
    return 1;
}

static int
HandleMultiGL(data, c)
    register XctData data;
    unsigned char c;
{
    switch (c) {
    case 0x41:
	data->GL = "\101";
	data->GL_char_size = 2;
	data->GL_encoding = "GB2312-1980.0";
	break;
    case 0x42:
	data->GL = "\102";
	data->GL_char_size = 2;
	data->GL_encoding = "JISX0208-1983.0";
	break;
    case 0x43:
	data->GL = "\103";
	data->GL_char_size = 2;
	data->GL_encoding = "KSC5601-1987.0";
	break;
    default:
	return 0;
    }
    data->GL_set_size = 94;
    if (c < 0x60)
	data->GL_char_size = 2;
    else if (c < 0x70)
	data->GL_char_size = 3;
    else
	data->GL_char_size = 4;
    data->GLGR_encoding = (char *)NULL;
    return 1;
}

static int
Handle94GR(data, c)
    register XctData data;
    unsigned char c;
{
    switch (c) {
    case 0x49:
	data->GR = "\111";
	data->GR_encoding = "JISX0201-1983";
	break;
    default:
	return 0;
    }
    data->GR_set_size = 94;
    data->GR_char_size = 1;
    data->GLGR_encoding = (char *)NULL;
    return 1;
}

static int
Handle96GR(data, c)
    register XctData data;
    unsigned char c;
{
    switch (c) {
    case 0x41:
	data->GR = "\101";
	data->GR_encoding = "ISO8859-1";
	break;
    case 0x42:
	data->GR = "\102";
	data->GR_encoding = "ISO8859-2";
	break;
    case 0x43:
	data->GR = "\103";
	data->GR_encoding = "ISO8859-3";
	break;
    case 0x44:
	data->GR = "\104";
	data->GR_encoding = "ISO8859-4";
	break;
    case 0x46:
	data->GR = "\106";
	data->GR_encoding = "ISO8859-7";
	break;
    case 0x47:
	data->GR = "\107";
	data->GR_encoding = "ISO8859-6";
	break;
    case 0x48:
	data->GR = "\110";
	data->GR_encoding = "ISO8859-8";
	break;
    case 0x4d:
	data->GR = "\115";
	data->GR_encoding = "ISO8859-9";
	break;
    default:
	return 0;
    }
    data->GR_set_size = 96;
    data->GR_char_size = 1;
    ComputeGLGR(data);
    return 1;
}

static int
HandleMultiGR(data, c)
    register XctData data;
    unsigned char c;
{
    switch (c) {
    case 0x41:
	data->GR = "\101";
	data->GR_encoding = "GB2312-1980.1";
	break;
    case 0x42:
	data->GR = "\102";
	data->GR_encoding = "JISX0208-1983.1";
	break;
    case 0x43:
	data->GR = "\103";
	data->GR_encoding = "KSC5601-1987.1";
	break;
    default:
	return 0;
    }
    data->GR_set_size = 94;
    data->GR_char_size = 2;
    if (c < 0x60)
	data->GR_char_size = 2;
    else if (c < 0x70)
	data->GR_char_size = 3;
    else
	data->GR_char_size = 4;
    data->GLGR_encoding = (char *)NULL;
    return 1;
}

static unsigned enc_count = 0;
static XctString *encodings = (XctString *)NULL;

static int
HandleExtended(data, c)
    register XctData data;
    unsigned char c;
{
    register XctPriv priv = data->priv;
    XctString enc = data->item + 6;
    register XctString ptr = enc;
    int i, len;

    while (*ptr != 0x02) {
	if (!*ptr || (++ptr == priv->ptr))
	    return 0;
    }
    data->item = ptr + 1;
    data->item_length = priv->ptr - data->item;
    len = ptr - enc;
    for (i = 0;
	 (i < enc_count) && strncmp((char *)encodings[i], (char *)enc, len);
	 i++)
	;
    if (i == enc_count) {
	XctString cp;

	for (cp = enc; cp != ptr; cp++) {
	    if ((!IsGL(*cp) && !IsGR(*cp)) || (*cp == 0x2a) || (*cp == 0x3f))
		return 0;
	}
	ptr = (XctString)malloc((unsigned)len + 1);
	bcopy((char *)enc, (char *)ptr, len);
	ptr[len] = 0x00;
	enc_count++;
	if (encodings)
	    encodings = (XctString *)realloc((char *)encodings,
					     enc_count * sizeof(XctString));
	else
	    encodings = (XctString *)malloc(sizeof(XctString));
	encodings[i] = ptr;
    }
    data->encoding = (char *)encodings[i];
    data->char_size = c - 0x30;
    return 1;
}

/* Create an XctData structure for parsing a Compound Text string. */
XctData
XctCreate(string, length, flags)
    XctString string;
    int length;
    XctFlags flags;
{
    register XctData data;
    register XctPriv priv;

    data = (XctData)malloc(sizeof(struct _XctRec) + sizeof(struct _XctPriv));
    if (!data)
	return data;
    data->priv = priv = (XctPriv)((char *)data + sizeof(struct _XctRec));
    data->total_string = string;
    data->total_length = length;
    data->flags = flags;
    priv->dirstack = (XctHDirection *)NULL;
    priv->dirsize = 0;
    XctReset(data);
    return data;
}

/* Reset the XctData structure to re-parse the string from the beginning. */
void
XctReset(data)
    register XctData data;
{
    register XctPriv priv = data->priv;

    priv->ptr = data->total_string;
    priv->ptrend = data->total_string + data->total_length;
    data->item = (XctString)NULL;
    data->item_length = 0;
    data->encoding = (char *)NULL;
    data->char_size = 1;
    data->horizontal = XctUnspecified;
    data->horz_depth = 0;
    data->GL_set_size = data->GR_set_size = 0; /* XXX */
    (void)HandleGL(data, (unsigned char)0x42);
    (void)Handle96GR(data, (unsigned char)0x41);
    data->version = 1;
    data->can_ignore_exts = 0;
    /* parse version, if present */
    if ((data->total_length >= 4) &&
	(priv->ptr[0] == ESC) && (priv->ptr[1] == 0x23) &&
	IsI2(priv->ptr[2]) &&
	((priv->ptr[3] == 0x30) || (priv->ptr[3] == 0x31))) {
	data->version = priv->ptr[2] - 0x1f;
	if (priv->ptr[3] == 0x30)
	    data->can_ignore_exts = 1;
	priv->ptr += 4;
    }
}

/* Parse the next "item" from the Compound Text string.  The return value
 * indicates what kind of item is returned.  The item itself, and the current
 * contextual state, are reported as components of the XctData structure.
 */
XctResult
XctNextItem(data)
    register XctData data;
{
    register XctPriv priv = data->priv;
    unsigned char c;
    int len;

#define NEXT data->item_length++; priv->ptr++

    while (IsMore(priv)) {
	data->item = priv->ptr;
	data->item_length = 0;
	c = *priv->ptr;
	if (c == ESC) {
	    NEXT;
	    while (IsMore(priv) && IsI2(*priv->ptr)) {
		NEXT;
	    }
	    if (!IsMore(priv))
		return XctSyntaxError;
	    c = *priv->ptr;
	    NEXT;
	    if (!IsESCF(c))
		return XctSyntaxError;
	    switch (data->item[1]) {
	    case 0x24:
		if (data->item_length > 3) {
		    if (data->item[2] == 0x28) {
			if (HandleMultiGL(data, c))
			    continue;
		    } else if (data->item[2] == 0x29) {
			if (HandleMultiGR(data, c))
			    continue;
		    }
		}
		break;
	    case 0x25:
		if ((data->item_length == 4) && (data->item[2] = 0x2f) &&
		    (c <= 0x3f)) {
		    if ((AmountLeft(priv) < 2) ||
			(priv->ptr[0] < 0x80) || (priv->ptr[1] < 0x80))
			return XctSyntaxError;
		    len = *priv->ptr - 0x80;
		    NEXT;
		    len = (len << 7) + (*priv->ptr - 0x80);
		    NEXT;
		    if (AmountLeft(priv) < len)
			return XctSyntaxError;
		    data->item_length += len;
		    priv->ptr += len;
		    if (c <= 0x34) {
			if (!HandleExtended(data, c))
			    return XctSyntaxError;
			return XctExtendedSegment;
		    }
		}
		break;
	    case 0x28:
		if (HandleGL(data, c))
		    continue;
		break;
	    case 0x29:
		if (Handle94GR(data, c))
		    continue;
		break;
	    case 0x2d:
		if (Handle96GR(data, c))
		    continue;
		break;
	    }
	} else if (c == CSI) {
	    NEXT;
	    while (IsMore(priv) && IsI3(*priv->ptr)) {
		NEXT;
	    }
	    while (IsMore(priv) && IsI2(*priv->ptr)) {
		NEXT;
	    }
	    if (!IsMore(priv))
		return XctSyntaxError;
	    c = *priv->ptr;
	    NEXT;
	    if (!IsCSIF(c))
		return XctSyntaxError;
	    if (c == 0x5d) {
		if ((data->item_length == 3) &&
		    ((data->item[1] == 0x31) || (data->item[1] == 0x32))) {
		    data->horz_depth++;
		    if (priv->dirsize < data->horz_depth) {
			priv->dirsize += 10;
			if (priv->dirstack)
			    priv->dirstack = (XctHDirection *)
					     realloc((char *)priv->dirstack,
						     priv->dirsize *
						     sizeof(XctHDirection));
			else
			    priv->dirstack = (XctHDirection *)
					     malloc(priv->dirsize *
						    sizeof(XctHDirection));
		    }
		    priv->dirstack[data->horz_depth - 1] = data->horizontal;
		    if (data->item[1] == 0x31)
			data->horizontal = XctLeftToRight;
		    else
			data->horizontal = XctRightToLeft;
		    if (data->flags & XctHideDirection)
			continue;
		    return XctHorizontal;
		} else if (data->item_length == 2) {
		    if (!data->horz_depth)
			return XctError;
		    data->horz_depth--;
		    data->horizontal = priv->dirstack[data->horz_depth];
		    if (data->flags & XctHideDirection)
			continue;
		    return XctHorizontal;
		}
	    }
	} else if (data->flags & XctSingleSetSegments) {
	    NEXT;
	    if IsC0(c) {
		data->encoding = (char *)NULL;
		data->char_size = 1;
		if (IsLegalC0(data, c))
		    return XctC0Segment;
	    } else if (IsGL(c)) {
		data->encoding = data->GL_encoding;
		data->char_size = data->GL_char_size;
		while (IsMore(priv) && IsGL(*priv->ptr)) {
		    NEXT;
		}
		if (data->char_size && (data->item_length % data->char_size))
		    return XctSyntaxError;
		return XctGLSegment;
	    } else if (IsC1(c)) {
		data->encoding = (char *)NULL;
		data->char_size = 1;
		if (IsLegalC1(data, c))
		    return XctC1Segment;
	    } else {
		data->encoding = data->GR_encoding;
		data->char_size = data->GR_char_size;
		while (IsMore(priv) && IsGR(*priv->ptr)) {
		    NEXT;
		}
		if (data->char_size && (data->item_length % data->char_size))
		    return XctSyntaxError;
		return XctGRSegment;
	    }
	} else {
	    data->encoding = data->GLGR_encoding;
	    if (data->GL_char_size == data->GR_char_size)
		data->char_size = data->GL_char_size;
	    else
		data->char_size = 0;
	    while (1) {
		if (IsC0(c) || IsC1(c)) {
		    if ((c == ESC) || (c == CSI))
			return XctSegment;
		    if (IsC0(c) ? !IsLegalC0(data, c) : !IsLegalC1(data, c)) {
			if (data->item_length)
			    return XctSegment;
			NEXT;
			break;
		    }
		    if (data->char_size > 1)
			data->char_size = 0;
		    NEXT;
		} else {
		    len = data->item_length;
		    NEXT;
		    if (IsGL(c)) {
			while (IsMore(priv) && IsGL(*priv->ptr)) {
			    NEXT;
			}
			if (data->GL_char_size && (len % data->GL_char_size))
			    return XctSyntaxError;
		    } else {
			while (IsMore(priv) && IsGR(*priv->ptr)) {
			    NEXT;
			}
			if (data->GR_char_size && (len % data->GR_char_size))
			    return XctSyntaxError;
		    }
		}
		if (!IsMore(priv))
		    return XctSegment;
		c = *priv->ptr;
	    }
	}
	if (data->version <= XctVersion)
	    return XctSyntaxError;
	if (data->flags & XctProvideExtensions)
	    return XctExtension;
	if (!data->can_ignore_exts)
	    return XctError;
    }
    return XctEndOfText;
}

/* Free all data associated with an XctDataStructure. */
void
XctFree(data)
    register XctData data;
{
    if (data->priv->dirstack)
	free((char *)data->priv->dirstack);
    if (data->flags & XctFreeString)
	free((char *)data->total_string);
    free((char *)data);
}
