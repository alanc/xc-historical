/* $XConsortium: tags.c,v 1.5 94/03/27 13:57:43 dpw Exp $ */
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
 * $NCDId: @(#)tags.c,v 1.10 1994/03/24 17:55:03 lemke Exp $
 */

#include	"misc.h"
#include	"util.h"
#include	"cache.h"
#include	"tags.h"
#include	"assert.h"
#include	"wire.h"

Cache       global_cache;
Cache       prop_cache;
Bool        lbxUseTags = TRUE;
int         lbxTagCacheSize = 1000000;


void
TagsInit()
{
    /* XXX make these configurable */
    if (!lbxUseTags) {
	lbxTagCacheSize = 0;
    }
    global_cache = CacheInit(lbxTagCacheSize);
    prop_cache = CacheInit(lbxTagCacheSize);
}

void
FreeTags()
{
    CacheFreeCache(global_cache);
    CacheFreeCache(prop_cache);
}

/* ARGSUSED */
static void
cache_free(id, data, reason)
    CacheID     id;
    pointer     data;
    int         reason;
{
    TagData     tag = (TagData) data;

    /* tell server we toasted this one */
    if (reason != CacheEntryFreed)
	SendInvalidateTag(0, tag->tid);
    xfree(tag->tdata);
    xfree(data);
}

Bool
TagStoreData(cache, id, size, dtype, data)
    Cache       cache;
    CacheID     id;
    int         size;
    int         dtype;
    pointer     data;
{
    TagData     tag;
    Bool	ret;

    assert(lbxUseTags);
    tag = (TagData) xalloc(sizeof(TagDataRec));
    if (!tag)
	return FALSE;
    tag->tdata = (pointer) xalloc(size);
    if (!tag->tdata) {
	xfree(tag);
	return FALSE;
    }
    bcopy((char *) data, (char *) tag->tdata, size);
    tag->tid = id;
    tag->data_type = dtype;
    tag->size = size;

    ret = CacheStoreMemory(cache, id, (pointer) tag, size, cache_free);
    if (!ret) {
    	xfree(tag->tdata);
        xfree(tag);
    }
    return ret;
}

TagData
TagGetTag(cache, id)
    Cache       cache;
    CacheID     id;
{
    TagData     tag;

    assert(lbxUseTags);
    tag = (TagData) CacheFetchMemory(cache, id, TRUE);
    return tag;
}

pointer
TagGetData(cache, id)
    Cache       cache;
    CacheID     id;
{
    TagData     tag;

    assert(lbxUseTags);
    tag = (TagData) CacheFetchMemory(cache, id, TRUE);
    if (tag)
	return tag->tdata;
    else
	return (pointer) NULL;
}

void
TagFreeData(cache, id, notify)
    Cache       cache;
    CacheID     id;
    Bool        notify;
{
    assert(lbxUseTags);
    CacheFreeMemory(cache, id, notify);
}
