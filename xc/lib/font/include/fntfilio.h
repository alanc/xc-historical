/*
 * $XConsortium: fontfileio.h,v 1.1 91/09/07 11:59:55 keith Exp $
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
 *
 * Author:  Keith Packard, MIT X Consortium
 */

#include    <bufio.h>

typedef BufFilePtr  FontFilePtr;

#define FontFileGetc(f)	    BufFileGet(f)
#define FontFilePutc(c,f)   BufFilePut(c,f)
#define FontFileRead(f,b,n) BufFileRead(f,b,n)
#define FontFileWrite(f,b,n)	BufFileWrite(f,b,n)
#define FontFileSkip(f,n)   (BufFileSkip (f, n) != BUFFILEEOF)
#define FontFileSeek(f,n)   (BufFileSeek (f,n,0) != BUFFILEEOF)

#define FontFileEOF	BUFFILEEOF

extern FontFilePtr  FontFileOpen ();
extern FontFilePtr  FontFileOpenWrite ();
extern FontFilePtr  FontFileOpenFd ();
extern FontFilePtr  FontFileOpenWriteFd ();
extern int	    FontFileClose ();
