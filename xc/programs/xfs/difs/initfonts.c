/* $XConsortium: initfonts.c,v 1.3 91/05/13 16:55:52 gildea Exp $ */
/*
 * Copyright 1990, 1991 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation and the
 * Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this protoype software
 * and its documentation to Members and Affiliates of the MIT X Consortium
 * any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Network Computing Devices, Digital or
 * MIT not be used in advertising or publicity pertaining to distribution of
 * the software without specific, written prior permission.
 *
 * NETWORK COMPUTING DEVICES, DIGITAL AND MIT DISCLAIM ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL NETWORK COMPUTING DEVICES, DIGITAL OR MIT BE
 * LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * %W%	%E%
 *
 */

#include        "FS.h"
#include        "FSproto.h"
#include	<stdio.h>
#include	<X11/Xos.h>
#include	"clientstr.h"
#include	"resource.h"
#include	"difsfontst.h"
#include	"fontstruct.h"
#include	"closestr.h"
#include	"globals.h"

FontPatternCachePtr fontPatternCache;

void
InitFonts()
{
    if (fontPatternCache)
	FreeFontPatternCache (fontPatternCache);
    fontPatternCache = MakeFontPatternCache ();

#ifdef FONT_PCF
    FontFileRegisterFpeFunctions();
#endif

#ifdef FONT_FS
    fs_register_fpe_functions();
#endif
}
