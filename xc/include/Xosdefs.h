/*
 * O/S-dependent (mis)feature macro definitions
 *
 * $XConsortium$
 *
 * Copyright 1991 Massachusetts Institute of Technology
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
 */

/*
 * X_NOT_STDC_ENV means does not have ANSI C header files.  Lack of this
 * symbol does NOT mean that the system has stdarg.h.
 *
 * X_NOT_POSIX means does not have POSIX header files.  Lack of this
 * symbol does NOT mean that the POSIX environment is the default.
 * You may still have to define _POSIX_SOURCE to get it.
 */

#ifdef bsd43
#define X_NOT_POSIX
#define X_NOT_STDC_ENV
#endif

#ifdef UTEK
#define X_NOT_POSIX
#define X_NOT_STDC_ENV
#endif

#ifdef CRAY
#define X_NOT_POSIX
#endif

#ifdef vax
#ifndef ultrix			/* assume vanilla BSD */
#define X_NOT_POSIX
#define X_NOT_STDC_ENV
#endif
#endif
