/* $XConsortium: closestr.h,v 1.1 94/03/17 11:35:08 dpw Exp $ */

/*
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
 */

#ifndef CLOSESTR_H
#define CLOSESTR_H

#define	NEED_REPLIES
#include "Xproto.h"
#include "closure.h"
#include "dix.h"
#include "misc.h"
#include "gcstruct.h"

/* closure structures */
typedef struct _OFclosure {
    ClientPtr   client;
    short       current_fpe;
    short       num_fpes;
    FontPathElementPtr *fpe_list;
    Mask        flags;
    Bool        slept;

/* XXX -- get these from request buffer instead? */
    char       *origFontName;
    int		origFontNameLen;
    XID         fontid;
    char       *fontname;
    int         fnamelen;
    FontPtr	non_cachable_font;
}           OFclosureRec;

typedef struct _LFWIstate {
    char	*pattern;
    int		patlen;
    int		current_fpe;
    int		max_names;
    Bool	list_started;
    pointer	private;
} LFWIstateRec, *LFWIstatePtr;

typedef struct _LFWIclosure {
    ClientPtr		client;
    int			num_fpes;
    FontPathElementPtr	*fpe_list;
    xListFontsWithInfoReply *reply;
    int			length;
    LFWIstateRec	current;
    LFWIstateRec	saved;
    int			savedNumFonts;
    Bool		haveSaved;
    Bool		slept;
    char		*savedName;
} LFWIclosureRec;

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

typedef struct _PTclosure {
    ClientPtr		client;
    DrawablePtr		pDraw;
    GC			*pGC;
    unsigned char	*pElt;
    unsigned char	*endReq;
    unsigned char	*data;
    int			xorg;
    int			yorg;
    CARD8		reqType;
    int			(* polyText)();
    int			itemSize;
    XID			did;
    int			err;
    Bool		slept;
} PTclosureRec;

typedef struct _ITclosure {
    ClientPtr		client;
    DrawablePtr		pDraw;
    GC			*pGC;
    BYTE		nChars;
    unsigned char	*data;
    int			xorg;
    int			yorg;
    CARD8		reqType;
    void		(* imageText)();
    int			itemSize;
    XID			did;
    Bool		slept;
} ITclosureRec;
#endif				/* CLOSESTR_H */
