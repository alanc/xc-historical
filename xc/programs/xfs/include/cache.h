/* $XConsortium: cache.h,v 1.3 94/02/09 12:32:59 gildea Exp $ */
/* 
 * Copyright 1990, 1991 Network Computing Devices; 
 * Portions Copyright 1987 by Digital Equipment Corporation and the 
 * Massachusetts Institute of Technology
 * 
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of M.I.T., Network Computing Devices,
 * or Digital not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. Network Computing Devices, or Digital
 * make no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 * 
 * NETWORK COMPUTING DEVICES, DIGITAL AND MIT DISCLAIM ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL NETWORK COMPUTING DEVICES, DIGITAL OR MIT BE
 * LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * @(#)cache.h	4.1	91/05/02
 *
 */
#ifndef _CACHE_H_
#define	_CACHE_H_
#include	"misc.h"

#define	CacheWasReset		1
#define	CacheEntryFreed		2
#define	CacheEntryOld		3

typedef unsigned long CacheID;
typedef unsigned long Cache;
typedef void (*CacheFree) ();

typedef struct _cache *CachePtr;

extern CacheID CacheStoreMemory();
extern int  CacheFreeMemory();
extern void CacheSimpleFree();
extern Cache CacheInit();
extern void CacheReset();
extern void CacheResize();
extern pointer CacheFetchMemory();
extern void CacheStats();

#endif				/* _CACHE_H_ */
