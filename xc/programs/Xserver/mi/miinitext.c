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
/* $XConsortium: miinitext.c,v 1.5 89/03/30 08:55:11 rws Exp $ */

#ifdef ZOID
extern void ZoidExtensionInit();
#endif
#ifdef BEZIER
extern void BezierExtensionInit();
#endif
#ifdef XTESTEXT1
extern void XTestExtension1Init();
#endif
#ifdef SHAPE
extern void ShapeExtensionInit();
#endif

/*ARGSUSED*/
void
InitExtensions(argc, argv)
    int		argc;
    char	*argv[];
{
#ifdef ZOID
    ZoidExtensionInit();
#endif
#ifdef BEZIER
    BezierExtensionInit();
#endif
#ifdef XTESTEXT1
    XTestExtension1Init();
#endif
#ifdef SHAPE
    ShapeExtensionInit();
#endif
}
