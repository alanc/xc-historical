/*
 * $XConsortium: bufio.c,v 1.5 93/09/12 18:01:35 rws Exp $
 *
 * Copyright 1991 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Network Computing Devices, Digital or
 * M.I.T. not be used in advertising or publicity pertaining to distribution
 * of the software without specific, written prior permission.
 *
 * NETWORK COMPUTING DEVICES, DIGITAL AND M.I.T. DISCLAIM ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL NETWORK COMPUTING DEVICES,
 * DIGITAL OR M.I.T. BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 * THIS SOFTWARE.
 *
 * Author:  Keith Packard, MIT X Consortium
 */


#include    <X11/Xos.h>
#include    <fontmisc.h>
#include    <bufio.h>
#include    <errno.h>
extern int errno;

BufFilePtr
BufFileCreate (private, io, skip, close)
    char    *private;
    int	    (*io)();
    int	    (*skip)();
    int	    (*close)();
{
    BufFilePtr	f;

    f = (BufFilePtr) xalloc (sizeof *f);
    if (!f)
	return 0;
    f->private = private;
    f->bufp = f->buffer;
    f->left = 0;
    f->io = io;
    f->skip = skip;
    f->close = close;
    return f;
}

#define FileDes(f)  ((int) (f)->private)

static int
BufFileRawFill (f)
    BufFilePtr	f;
{
    int	left;

    left = read (FileDes(f), (char *)f->buffer, BUFFILESIZE);
    if (left <= 0) {
	f->left = 0;
	return BUFFILEEOF;
    }
    f->left = left - 1;
    f->bufp = f->buffer + 1;
    return f->buffer[0];
}

static int
BufFileRawSkip (f, count)
    BufFilePtr	f;
    int		count;
{
    int	    curoff;
    int	    fileoff;
    int	    todo;

    curoff = f->bufp - f->buffer;
    fileoff = curoff + f->left;
    if (curoff + count <= fileoff) {
	f->bufp += count;
	f->left -= count;
    } else {
	todo = count - (fileoff - curoff);
	if (lseek (FileDes(f), todo, 1) == -1) {
	    if (errno != ESPIPE)
		return BUFFILEEOF;
	    while (todo) {
		curoff = BUFFILESIZE;
		if (curoff > todo)
		    curoff = todo;
		fileoff = read (FileDes(f), (char *)f->buffer, curoff);
		if (fileoff <= 0)
		    return BUFFILEEOF;
		todo -= fileoff;
	    }
	}
	f->left = 0;
    }
    return count;
}

static int
BufFileRawClose (f, doClose)
    BufFilePtr	f;
{
    if (doClose)
	close (FileDes (f));
    return 1;
}

BufFilePtr
BufFileOpenRead (fd)
    int	fd;
{
    return BufFileCreate ((char *) fd, BufFileRawFill, BufFileRawSkip, BufFileRawClose);
}

static
BufFileRawFlush (c, f)
    int		c;
    BufFilePtr	f;
{
    int	cnt;

    if (c != BUFFILEEOF)
	*f->bufp++ = c;
    cnt = f->bufp - f->buffer;
    f->bufp = f->buffer;
    f->left = BUFFILESIZE;
    if (write (FileDes(f), (char *)f->buffer, cnt) != cnt)
	return BUFFILEEOF;
    return c;
}

BufFilePtr
BufFileOpenWrite (fd)
    int	fd;
{
    BufFilePtr	f;

    f = BufFileCreate ((char *) fd, BufFileRawFlush, 0, BufFileFlush);
    f->bufp = f->buffer;
    f->left = BUFFILESIZE;
    return f;
}

BufFileRead (f, b, n)
    BufFilePtr	f;
    char	*b;
    int		n;
{
    int	    c, cnt;
    cnt = n;
    while (cnt--) {
	c = BufFileGet (f);
	if (c == BUFFILEEOF)
	    break;
	*b++ = c;
    }
    return n - cnt - 1;
}

BufFileWrite (f, b, n)
    BufFilePtr	f;
    char	*b;
    int		n;
{
    int	    cnt;
    cnt = n;
    while (cnt--) {
	if (BufFilePut (*b++, f) == BUFFILEEOF)
	    return BUFFILEEOF;
    }
    return n;
}

int
BufFileFlush (f)
    BufFilePtr	f;
{
    if (f->bufp != f->buffer)
	(*f->io) (BUFFILEEOF, f);
}

int
BufFileClose (f, doClose)
    BufFilePtr	f;
{
    (void) (*f->close) (f, doClose);
    xfree (f);
}

int
BufFileFree (f)
    BufFilePtr	f;
{
    xfree (f);
}
