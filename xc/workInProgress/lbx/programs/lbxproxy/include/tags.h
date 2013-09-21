/* $XConsortium: tags.h,v 1.5 95/05/17 18:23:24 dpw Exp $ */
/*
 * Copyright 1993 Network Computing Devices, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name Network Computing Devices, Inc. not be
 * used in advertising or publicity pertaining to distribution of this
 * software without specific, written prior permission.
 *
 * THIS SOFTWARE IS PROVIDED `AS-IS'.  NETWORK COMPUTING DEVICES, INC.,
 * DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING WITHOUT
 * LIMITATION ALL IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, OR NONINFRINGEMENT.  IN NO EVENT SHALL NETWORK
 * COMPUTING DEVICES, INC., BE LIABLE FOR ANY DAMAGES WHATSOEVER, INCLUDING
 * SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES, INCLUDING LOSS OF USE, DATA,
 * OR PROFITS, EVEN IF ADVISED OF THE POSSIBILITY THEREOF, AND REGARDLESS OF
 * WHETHER IN AN ACTION IN CONTRACT, TORT OR NEGLIGENCE, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * $NCDId: @(#)tags.h,v 1.4 1994/03/04 00:07:36 lemke Exp $
 */

#ifndef _TAGS_H_
#define _TAGS_H_

#include	"cache.h"

typedef struct _tagdata {
    XID         tid;
    int         data_type;
    unsigned long size;
    pointer     tdata;
}           TagDataRec;

typedef struct _tagdata *TagData;

extern void TagsInit(
#if NeedFunctionPrototypes
    void
#endif
);

extern void FreeTags(
#if NeedFunctionPrototypes
    void
#endif
);

extern Bool TagStoreData(
#if NeedFunctionPrototypes
    Cache /*cache*/,
    CacheID /*id*/,
    int /*size*/,
    int /*dtype*/,
    pointer /*data*/
#endif
);

extern TagData TagGetTag(
#if NeedFunctionPrototypes
    Cache /*cache*/,
    CacheID /*id*/
#endif
);

extern pointer TagGetData(
#if NeedFunctionPrototypes
    Cache /*cache*/,
    CacheID /*id*/
#endif
);

extern void TagFreeData(
#if NeedFunctionPrototypes
    Cache /*cache*/,
    CacheID /*id*/,
    Bool /*notify*/
#endif
);

extern Cache global_cache;
extern Cache prop_cache;

#endif				/* _TAGS_H_ */
