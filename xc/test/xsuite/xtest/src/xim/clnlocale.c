/* $XConsortium$ */
/*
 * Copyright 1993 by Sun Microsystems, Inc. Mountain View, CA.
 *
 *                   All Rights Reserved
 *
 * Permission  to  use,  copy,  modify,  and  distribute   this
 * software  and  its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright no-
 * tice  appear  in all copies and that both that copyright no-
 * tice and this permission notice appear in  supporting  docu-
 * mentation,  and  that the names of Sun or MIT not be used in
 * advertising or publicity pertaining to distribution  of  the
 * software  without specific prior written permission. Sun and
 * M.I.T. make no representations about the suitability of this
 * software for any purpose. It is provided "as is" without any
 * express or implied warranty.
 *
 * SUN DISCLAIMS ALL WARRANTIES WITH REGARD TO  THIS  SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-
 * NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SUN BE  LI-
 * ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
 * PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
 * THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include        "xtest.h"
#include        "Xlib.h"
#include        "Xutil.h"
#include        "Xresource.h"
#include        "xtestlib.h"
#include        "tet_api.h"
#include        "pixval.h"
#include        <string.h>
#include        "ximtest.h"

extern	Display	*Dsp;

void
cleanup_locale(style,fs,im,db)
        XIMStyles *style;
        XFontSet fs;
        XIM im;
        XrmDatabase db;
{
        if(style != NULL)
                XFree(style);
        if(fs != NULL)
                XFreeFontSet(Dsp,fs);
        if(im != NULL)
        XCloseIM(im);
        if(db  != NULL)
                XrmDestroyDatabase(db);
}