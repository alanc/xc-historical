/* $XConsortium$ */
/*
 * Copyright 1990, 1991 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation and the
 * Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this protoype software
 * and its documentation to Members and Affiliates of the MIT X Consortium
 * any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Network Computing Devices, Digital or
 * MIT not be used in advertising or publicity pertaining to distribution of
 * the software without specific, written prior permission.
 *
 * NETWORK COMPUTING DEVICES, DIGITAL AND MIT DISCLAIM ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL NETWORK COMPUTING DEVICES, DIGITAL OR MIT BE
 * LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * @(#)closestr.h	4.1	91/05/02
 *
 */

#ifndef CLOSESTR_H
#define CLOSESTR_H

#include	"FSproto.h"
#include	"closure.h"
#include	"misc.h"
#include	"font.h"

/* closure structures */
typedef struct _OFclosure {
    ClientPtr   client;
    short       current_fpe;
    short       num_fpes;
    FontPathElementPtr *fpe_list;
    Mask        flags;
    fsBitmapFormat format;
    fsBitmapFormatMask format_mask;
    Bool        slept;
    FSID        fontid;
    char       *fontname;
    int         fnamelen;
}           OFclosureRec;

typedef struct _QEclosure {
    ClientPtr   client;
    int         nranges;
    fsRange    *range;
    FontPtr     pfont;
    Mask        flags;
    Bool        slept;
}           QEclosureRec;

typedef struct _QBclosure {
    ClientPtr   client;
    int         nranges;
    fsRange    *range;
    FontPtr     pfont;
    fsBitmapFormat format;
    Mask        flags;
    Bool        slept;
}           QBclosureRec;

typedef struct _LFclosure {
    ClientPtr   client;
    short       current_fpe;
    short       num_fpes;
    FontPathElementPtr *fpe_list;
    FontNamesPtr names;
    char       *pattern;
    int         maxnames;
    int         patlen;
    Bool        slept;
}           LFclosureRec;

typedef struct _LFWXIclosure {
    ClientPtr   client;
    FontNamesPtr names;
    int         current_name;
    short       current_fpe;
    short       num_fpes;
    FontPathElementPtr *fpe_list;
    fsListFontsWithXInfoReply *reply;
    int         length;
    char       *name;
    int         namelen;
    Bool        list_started;
    Bool        slept;
    pointer     private;
}           LFWXIclosureRec;

#endif				/* CLOSESTR_H */
