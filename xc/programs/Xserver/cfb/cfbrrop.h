/*
 * $XConsortium$
 *
 * Copyright 1989 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

#ifndef GXcopy
#include "X.h"
#endif

#define RROP_FETCH_GC(gc) \
    RROP_FETCH_GCPRIV(((cfbPrivGCPtr)(gc)->devPrivates[cfbGCPrivateIndex].ptr))

#ifndef RROP
#define RROP GXset
#endif

#if RROP == GXcopy
#define RROP_DECLARE	register unsigned long	rrop_xor;
#define RROP_FETCH_GCPRIV(devPriv)  rrop_xor = (devPriv)->xor;
#define RROP_SOLID(dst)	    (*(dst) = (rrop_xor))
#define RROP_SOLID_MASK(dst,mask) (*(dst) = (*(dst) & ~(mask)) | ((rrop_xor) & (mask)))
#define RROP_NAME(prefix)   RROP_NAME_CAT(prefix,Copy)
#endif

#if RROP == GXxor
#define RROP_DECLARE	register unsigned long	rrop_xor;
#define RROP_FETCH_GCPRIV(devPriv)  rrop_xor = (devPriv)->xor;
#define RROP_SOLID(dst)	    (*(dst) ^= (rrop_xor))
#define RROP_SOLID_MASK(dst,mask) (*(dst) ^= ((rrop_xor) & (mask)))
#define RROP_NAME(prefix)   RROP_NAME_CAT(prefix,Xor)
#endif

#if RROP == GXand
#define RROP_DECLARE	register unsigned long	rrop_and;
#define RROP_FETCH_GCPRIV(devPriv)  rrop_and = (devPriv)->and;
#define RROP_SOLID(dst)	    (*(dst) &= (rrop_and))
#define RROP_SOLID_MASK(dst,mask) (*(dst) &= ((rrop_and) | ~(mask)))
#define RROP_NAME(prefix)   RROP_NAME_CAT(prefix,And)
#endif

#if RROP == GXor
#define RROP_DECLARE	register unsigned long	rrop_or;
#define RROP_FETCH_GCPRIV(devPriv)  rrop_or = (devPriv)->xor;
#define RROP_SOLID(dst)	    (*(dst) |= (rrop_or))
#define RROP_SOLID_MASK(dst,mask) (*(dst) |= ((rrop_or) & (mask)))
#define RROP_NAME(prefix)   RROP_NAME_CAT(prefix,Or)
#endif

#if RROP == GXnoop
#define RROP_DECLARE
#define RROP_FETCH_GCPRIV(devPriv)
#define RROP_SOLID(dst)
#define RROP_SOLID_MASK(dst,mask)
#define RROP_NAME(prefix)   RROP_NAME_CAT(prefix,Noop)
#endif

#if RROP ==  GXset
#define RROP_DECLARE	    register unsigned long	rrop_and, rrop_xor;
#define RROP_FETCH_GCPRIV(devPriv)  rrop_and = (devPriv)->and; \
				    rrop_xor = (devPriv)->xor;
#define RROP_SOLID(dst)	    (*(dst) = DoRRop (*(dst), rrop_and, rrop_xor))
#define RROP_SOLID_MASK(dst,mask)   (*(dst) = DoMaskRRop (*(dst), rrop_and, rrop_xor, (mask)))
#define RROP_NAME(prefix)   RROP_NAME_CAT(prefix,General)
#endif

#ifdef __STDC__
#define RROP_NAME_CAT(prefix,suffix)	prefix##suffix
#else
#define RROP_NAME_CAT(prefix,suffix)	prefix/**/suffix
#endif
