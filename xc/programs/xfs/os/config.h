/* $XConsortium: config.h,v 1.2 91/05/13 16:50:35 gildea Exp $ */
/*
 * Copyright 1990, 1991 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation and the
 * Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of M.I.T., Network Computing Devices,
 * or Digital not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. Network Computing Devices, or Digital
 * make no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * NETWORK COMPUTING DEVICES, DIGITAL AND MIT DISCLAIM ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL NETWORK COMPUTING DEVICES, DIGITAL OR MIT BE
 * LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * @(#)config.h	4.1	91/05/02
 *
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_

/* max size in bytes of config file */
#define	CONFIG_MAX_FILESIZE	32767

/* error codes */
/* these should be in the same order as the error strings in config.c */
#define	CONFIG_ERR_MEMORY	1
#define	CONFIG_ERR_OPEN		2
#define	CONFIG_ERR_READ		3
#define	CONFIG_ERR_VALUE	4
#define	CONFIG_ERR_UNKNOWN	5
#define	CONFIG_ERR_NOEQUALS	6
#define	CONFIG_ERR_RANGE	7
#define	CONFIG_ERR_SYNTAX	8
#define	CONFIG_ERR_NOVALUE	9
#define	CONFIG_ERR_EXTRAVALUE	10
#endif				/* _CONFIG_H_ */
