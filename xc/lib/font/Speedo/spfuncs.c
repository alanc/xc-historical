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
 * Author: Dave Lemke, Network Computing Devices, Inc
 *
 */

#include	<X11/Xos.h>
#include	"fontfilest.h"
#include	"spint.h"

SpeedoOpenScalable (fpe, pFont, flags, entry, fileName, vals, format, fmask)
    FontPathElementPtr	fpe;
    FontPtr		*pFont;
    int			flags;
    FontEntryPtr	entry;
    char		*fileName;
    FontScalablePtr	vals;
    fsBitmapFormat	format;
    fsBitmapFormatMask	fmask;
{
    char	fullName[MAXFONTNAMELEN];
    int		ret;

    strcpy (fullName, entry->name.name);
    FontParseXLFDName (fullName, vals, FONT_XLFD_REPLACE_VALUE);
    return SpeedoFontLoad (pFont, fullName, fileName, entry,
			    format, fmask, flags);
}

SpeedoGetInfo ()
{
    /* XXX TBD */
}

static FontRendererRec renderer = {
    ".spd", 4, (int (*)()) 0, SpeedoOpenScalable, SpeedoGetInfo, 0
};
    
SpeedoRegisterFontFileFunctions()
{
    make_sp_standard_props();
    sp_reset();
    FontFileRegisterRenderer (&renderer);
}
