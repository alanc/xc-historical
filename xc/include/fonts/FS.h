/* $XConsortium: FS.h,v 1.8 94/04/17 20:11:05 dpw Exp $ */

/*
 * Copyright 1990, 1991 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation 
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Network Computing Devices or Digital
 * not be used in advertising or publicity pertaining to distribution
 * of the software without specific, written prior permission.
 * Network Computing Devices and Digital make no representations 
 * about the suitability of this software for any purpose.  It is provided 
 * "as is" without express or implied warranty.
 *
 * NETWORK COMPUTING DEVICES AND DIGITAL DISCLAIM ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL NETWORK COMPUTING DEVICES
 * OR DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 * THIS SOFTWARE.
 */

/*

Portions Copyright (c) 1987, 1994  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.

*/

#ifndef _FS_H_
#define	_FS_H_

#include <fsmasks.h>

#define	FS_PROTOCOL		2
#define	FS_PROTOCOL_MINOR	0

typedef unsigned long FSID;

#ifndef X_PROTOCOL
/* protocol familes */
#define FamilyInternet          0
#define FamilyDECnet            1
#define FamilyChaos             2

typedef unsigned long Mask;

typedef FSID	Font;
typedef FSID	AccContext;

typedef unsigned int    FSDrawDirection;
#endif

#ifndef None
#define	None		0L
#endif

#define	LeftToRightDrawDirection	0
#define	RightToLeftDrawDirection	1

/* font info flags */
#define	FontInfoAllCharsExist		(1L << 0)
#define	FontInfoInkInside		(1L << 1)
#define	FontInfoHorizontalOverlap	(1L << 2)

/* auth status flags */
#define	AuthSuccess	0
#define	AuthContinue	1
#define	AuthBusy	2
#define	AuthDenied	3

/* property types */
#define	PropTypeString		0
#define	PropTypeUnsigned	1
#define	PropTypeSigned		2

#ifndef LSBFirst
/* byte order */
#define LSBFirst                0
#define MSBFirst                1
#endif

/* event masks */
#define	CatalogueChangeNotifyMask	(1L << 0)
#define	FontChangeNotifyMask		(1L << 1)

/* errors */
#define	FSSuccess		-1
#define	FSBadRequest		0
#define	FSBadFormat		1
#define	FSBadFont		2
#define	FSBadRange		3
#define	FSBadEventMask		4
#define	FSBadAccessContext	5
#define	FSBadIDChoice		6
#define	FSBadName		7
#define	FSBadResolution		8
#define	FSBadAlloc		9
#define	FSBadLength		10
#define	FSBadImplementation	11

#define	FirstExtensionError	128
#define	LastExtensionError	255

/* events */
#define	KeepAlive		0
#define	CatalogueChangeNotify	1
#define	FontChangeNotify	2
#define FSLASTEvent		3

#endif				/* _FS_H_ */
