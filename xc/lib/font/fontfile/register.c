/*
 * $XConsortium: register.c,v 1.1 94/02/04 11:13:47 gildea Exp $
 *
 * Copyright 1994 Massachusetts Institute of Technology
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
 * This is in a separate source file so that small programs
 * such as mkfontdir that want to use the fontfile utilities don't
 * end up dragging in code from all the renderers, which is not small.
 */

FontFileRegisterFpeFunctions()
{
    BitmapRegisterFontFileFunctions ();
#ifndef CRAY
    SpeedoRegisterFontFileFunctions ();
    Type1RegisterFontFileFunctions();
#endif

    FontFileRegisterLocalFpeFunctions ();
}
