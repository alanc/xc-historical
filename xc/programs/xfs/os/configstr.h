/* $XConsortium: configstr.h,v 1.2 91/05/13 16:50:40 gildea Exp $ */
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
 * @(#)configstr.h	4.1	91/05/02
 *
 */
#ifndef _CONFIGSTR_H_
#define _CONFIGSTR_H_
#include	"config.h"

typedef struct _config_options {
    char       *parm_name;
    char       *(*set_func) ();
}           ConfigOptionRec, *ConfigOptionPtr;

#endif				/* _CONFIGSTR_H_ */
