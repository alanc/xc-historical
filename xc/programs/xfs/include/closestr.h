/* $XConsortium: closestr.h,v 1.4 94/02/03 10:07:21 gildea Exp $ */
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
 * $NCDId: @(#)closestr.h,v 4.1 1991/05/02 04:15:46 lemke Exp $
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
    char       *orig_name;
    int         orig_len;
    FontPtr	non_cachable_font;
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

typedef struct _LFWIstate {
    char       *pattern;
    int         patlen;
    int         current_fpe;
    int         max_names;
    Bool        list_started;
    pointer     private;
}           LFWIstateRec, *LFWIstatePtr;


typedef struct _LFWXIclosure {
    ClientPtr   client;
    int         num_fpes;
    FontPathElementPtr *fpe_list;
    fsListFontsWithXInfoReply *reply;
    int         length;
    LFWIstateRec current;
    LFWIstateRec saved;
    int         savedNumFonts;
    Bool        haveSaved;
    Bool        slept;
    char       *savedName;
}           LFWXIclosureRec;

typedef struct _LFclosure {
    ClientPtr   client;
    int         num_fpes;
    FontPathElementPtr *fpe_list;
    FontNamesPtr names;
    LFWIstateRec current;
    LFWIstateRec saved;
    Bool        haveSaved;
    Bool        slept;
    char	*savedName;
    int		savedNameLen;
}	LFclosureRec;

#endif				/* CLOSESTR_H */
