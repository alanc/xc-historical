/* $XConsortium: keycodes.h,v 1.4 93/09/28 20:16:45 rws Exp $ */
/************************************************************
 Copyright (c) 1994 by Silicon Graphics Computer Systems, Inc.

 Permission to use, copy, modify, and distribute this
 software and its documentation for any purpose and without
 fee is hereby granted, provided that the above copyright
 notice appear in all copies and that both that copyright
 notice and this permission notice appear in supporting
 documentation, and that the name of Silicon Graphics not be 
 used in advertising or publicity pertaining to distribution 
 of the software without specific prior written permission.
 Silicon Graphics makes no representation about the suitability 
 of this software for any purpose. It is provided "as is"
 without any express or implied warranty.
 
 SILICON GRAPHICS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS 
 SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY 
 AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SILICON
 GRAPHICS BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
 DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
 DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
 THE USE OR PERFORMANCE OF THIS SOFTWARE.

 ********************************************************/

#ifndef KEYCODES_H
#define KEYCODES_H 1

#define	KeyNameToLong(n)	((n[0]<<24)|(n[1]<<16)|(n[2]<<8)|n[3])

extern char * longText(
#if NeedFunctionPrototypes
    unsigned long /* val */
#endif
);

extern char * keyNameText(
#if NeedFunctionPrototypes
    char *	/* name */
#endif
);

extern void LongToKeyName(
#if NeedFunctionPrototypes
	unsigned long	/* val */,
	char *		/* name_rtrn */
#endif
);

#endif /* KEYCODES_H */
