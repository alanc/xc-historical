/*
 * $XConsortium: saver.h,v 1.3 92/02/28 18:09:07 keith Exp $
 *
 * Copyright 1992 Massachusetts Institute of Technology
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

#ifndef _SAVER_H_
#define _SAVER_H_

#define ScreenSaverName	"MIT-SCREEN-SAVER"
#define ScreenSaverPropertyName "_MIT_SCREEN_SAVER_ID"

#define ScreenSaverNotifyMask	0x00000001
#define ScreenSaverCycleMask	0x00000002

#define ScreenSaverMajorVersion	1
#define ScreenSaverMinorVersion	0

#define ScreenSaverOff		0
#define ScreenSaverOn		1
#define ScreenSaverCycle	2
#define ScreenSaverDisabled	3

#define ScreenSaverBlanked	0
#define ScreenSaverInternal	1
#define ScreenSaverExternal	2

#define ScreenSaverNotify	0
#define ScreenSaverNumberEvents	1

#endif /* _SAVER_H_ */
