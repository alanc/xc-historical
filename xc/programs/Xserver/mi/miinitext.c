/***********************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/
/* $XConsortium: miinitext.c,v 1.24 93/09/03 15:56:08 dpw Exp $ */

#include "misc.h"

#ifdef NOPEXEXT /* sleaze for Solaris cpp building XsunMono */
#undef PEXEXT
#endif

extern Bool noTestExtensions;

#ifdef BEZIER
extern void BezierExtensionInit();
#endif
#ifdef XTESTEXT1
extern void XTestExtension1Init();
#endif
#ifdef SHAPE
extern void ShapeExtensionInit();
#endif
#ifdef MITSHM
extern void ShmExtensionInit();
#endif
#ifdef PEXEXT
extern void PexExtensionInit();
#endif
#ifdef MULTIBUFFER
extern void MultibufferExtensionInit();
#endif
#ifdef XINPUT
extern void XInputExtensionInit();
#endif
#ifdef XTEST
extern void XTestExtensionInit();
#endif
#ifdef BIGREQS
extern void BigReqExtensionInit();
#endif
#ifdef MITMISC
extern void MITMiscExtensionInit();
#endif
#ifdef XIDLE
extern void XIdleExtensionInit();
#endif
#ifdef XTRAP
extern void DEC_XTRAPInit();
#endif
#ifdef SCREENSAVER
extern void ScreenSaverExtensionInit ();
#endif
#ifdef XV
extern void XvExtensionInit();
#endif
#ifdef XIE
extern void XieInit();
#endif
#ifdef XSYNC
extern void SyncExtensionInit();
#endif

/*ARGSUSED*/
void
InitExtensions(argc, argv)
    int		argc;
    char	*argv[];
{
#ifdef BEZIER
    BezierExtensionInit();
#endif
#ifdef XTESTEXT1
    if (!noTestExtensions) XTestExtension1Init();
#endif
#ifdef SHAPE
    ShapeExtensionInit();
#endif
#ifdef MITSHM
    ShmExtensionInit();
#endif
#ifdef PEXEXT
    PexExtensionInit();
#endif
#ifdef MULTIBUFFER
    MultibufferExtensionInit();
#endif
#ifdef XINPUT
    XInputExtensionInit();
#endif
#ifdef XTEST
    if (!noTestExtensions) XTestExtensionInit();
#endif
#ifdef BIGREQS
    BigReqExtensionInit();
#endif
#ifdef MITMISC
    MITMiscExtensionInit();
#endif
#ifdef XIDLE
    XIdleExtensionInit();
#endif
#ifdef XTRAP
    if (!noTestExtensions) DEC_XTRAPInit();
#endif
#ifdef SCREENSAVER
    ScreenSaverExtensionInit ();
#endif
#ifdef XV
    XvExtensionInit();
#endif
#ifdef XIE
    XieInit();
#endif
#ifdef XSYNC
    SyncExtensionInit();
#endif
}
