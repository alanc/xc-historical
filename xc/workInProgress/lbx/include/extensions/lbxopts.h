/* $XConsortium: lbxopts.h,v 1.1 94/12/01 20:04:44 mor Exp $ */
/*
 * Copyright 1994 Network Computing Devices, Inc.
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
 * SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES, INCLUDING LOSS OF USE, DATA,
 * OR PROFITS, EVEN IF ADVISED OF THE POSSIBILITY THEREOF, AND REGARDLESS OF
 * WHETHER IN AN ACTION IN CONTRACT, TORT OR NEGLIGENCE, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * $NCDId: @(#)lbxopts.h,v 1.1 1994/09/15 22:46:03 dct Exp $
 */

#ifndef _LBX_OPTS_H_
#define _LBX_OPTS_H_

/*
 * Negotiable configuration options
 */
#define LBX_OPT_DELTA_PROXY		0
#define LBX_OPT_DELTA_SERVER		1
#define LBX_OPT_STREAM_COMP		2
#define LBX_OPT_BITMAP_COMP		3
#define LBX_OPT_PIXMAP_COMP		4
#define LBX_OPT_MSG_COMP		5	/* aka "squishing" */
#define LBX_OPT_ENCRYPT			6	/* ???? */
#define LBX_OPT_EXTENSION		255

/*
 * Length fields are encoded in 1 or 3 bytes
 */
#define LBX_OPT_SMALLLEN_SIZE		1
#define LBX_OPT_BIGLEN_SIZE		3
#define LBX_OPT_BIGLEN_MIN		256

#define LBX_OPT_DECODE_LEN(p, len, size) \
    if ((p)[0]) { \
	(len) = (p)[0]; \
	(size) = LBX_OPT_SMALLLEN_SIZE; \
    } \
    else { \
	(len) = LBX_OPT_BIGLEN_MIN + (p)[1] << 8 | (p)[2]; \
	(size) = LBX_OPT_BIGLEN_SIZE; \
    }

/*
 * Option header lengths
 */
#define LBX_OPT_SMALLHDR_LEN		(1 + LBX_OPT_SMALLLEN_SIZE)
#define LBX_OPT_BIGHDR_LEN		(1 + LBX_OPT_BIGLEN_SIZE)

/*
 * Delta compression parameters
 */
#define LBX_OPT_DELTA_REQLEN		6
#define LBX_OPT_DELTA_REPLYLEN		2
#define LBX_OPT_DELTA_NCACHE_DFLT	16
#define LBX_OPT_DELTA_MSGLEN_MIN	32
#define LBX_OPT_DELTA_MSGLEN_DFLT	64

#endif /* _LBX_OPTS_H_ */
