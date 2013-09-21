/* $XConsortium: lbxbuf.h,v 1.5 95/01/23 21:25:52 gildea Exp $ */
/*
 * Copyright 1988, 1989, 1990, 1994 Network Computing Devices, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name Network Computing Devices, Inc. not be
 * used in advertising or publicity pertaining to distribution of this 
 * software without specific, written prior permission.
 * 
 * THIS SOFTWARE IS PROVIDED `AS-IS'.  NETWORK COMPUTING DEVICES, INC.,
 * DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING WITHOUT
 * LIMITATION ALL IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, OR NONINFRINGEMENT.  IN NO EVENT SHALL NETWORK
 * COMPUTING DEVICES, INC., BE LIABLE FOR ANY DAMAGES WHATSOEVER, INCLUDING
 * SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES, INCLUDING LOSS OF USE,
 * DATA, OR PROFITS, EVEN IF ADVISED OF THE POSSIBILITY THEREOF, AND
 * REGARDLESS OF WHETHER IN AN ACTION IN CONTRACT, TORT OR NEGLIGENCE,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 * 
 * $NCDId: @(#)lbxbuf.h,v 1.1 1994/02/16 18:57:32 lemke Exp $
 */

#ifndef _BUFFER_H_
#define _BUFFER_H_

#define INBUFFER_SIZE		(1 << 13)	/* 8K */
#define OUTBUFFER_SIZE		(1 << 12)	/* 4K */

#ifndef TRUE
#define TRUE    1
#endif
#ifndef FALSE
#define FALSE   0
#endif

typedef struct _lzwbuffer  *LzwBufferPtr;

extern int  InitLzwBuffer();
extern void FreeLzwBuffer();
extern char *ReserveOutBuf();

#endif	/* _BUFFER_H_ */
