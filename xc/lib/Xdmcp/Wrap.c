/*
 * $XConsortium: Encrypt.c,v 1.4 91/01/02 16:28:13 gildea Exp $
 *
 * Copyright 1989 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
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

#include <X11/Xos.h>
#include <X11/X.h>
#include <X11/Xmd.h>
#include <X11/Xdmcp.h>

#ifdef HASDES

/*
 * The following function exists only to demonstrate the
 * desired functional interface for this routine.  You will
 * need to add the appropriate algorithm if you wish to
 * use XDM-AUTHENTICATION-1/XDM-AUTHORIZATION-1.
 *
 * Examine the XDMCP specification for the correct algorithm
 */

#include "des.h"

void
XdmcpEncrypt (plain, key, crypto, bytes)
    unsigned char	*plain, *crypto;
    unsigned char	*key;
    int			bytes;
{
    int			i, j;
    int			len;
    unsigned char	tmp[8];
    unsigned char	expand_key[8];
    des_key_schedule	schedule;

    XdmcpKeyToOddParityKey (key, expand_key);
    des_set_key (key, schedule);
    for (j = 0; j < bytes; j += 8)
    {
	len = 8;
	if (bytes - j < len)
	    len = bytes - j;
	/* block chaining */
	for (i = 0; i < len; i++)
	{
	    if (j == 0)
		tmp[i] = plain[i];
	    else
		tmp[i] = plain[j + i] ^ crypto[j - 8 + i];
	}
	for (; i < 8; i++)
	{
	    if (j == 0)
		tmp[i] = 0;
	    else
		tmp[i] = 0 ^ crypto[j - 8 + i];
	}
	des_ecb_encrypt (tmp, (crypto + j), schedule, 1);
    }
}

/*
 * Given a 56 bit key in XDMCP format, create a 56
 * bit key in 7-bits + odd parity format
 */

static int
OddParity (c)
    unsigned char   c;
{
    c = c ^ (c >> 4);
    c = c ^ (c >> 2);
    c = c ^ (c >> 1);
    return ~c & 0x1;
}

/*
 * Spread the 56 bit key among 8 bytes, using the upper 7 bits
 * of each byte, and storing an odd parity bit in the low bit
 */

void
XdmcpKeyToOddParityKey (in, out)
    unsigned char   *in, *out;
{
    int		    ashift, bshift;
    int		    i;
    unsigned char   c;

    ashift = 7;
    bshift = 1;
    for (i = 0; i < 7; i++)
    {
	c = ((in[i] << ashift) | (in[i+1] >> bshift)) & 0x7f;
	out[i] = (c << 1) | OddParity (c);
	ashift--;
	bshift++;
    }
    c = in[i];
    out[i] = (c << 1) | OddParity(c);
}

#endif
