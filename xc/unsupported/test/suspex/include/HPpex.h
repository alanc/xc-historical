/* $XConsortium$ */

/*
 * Copyright (c) 1992 by Sun Microsystems, Inc.
 * All Rights Reserved
 */

/***********************************************************
Copyright 1989, 1990 by Sun Microsystems, Inc. and the X Consortium.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Sun Microsystems,
the X Consortium, and MIT not be used in advertising or publicity 
pertaining to distribution of the software without specific, written 
prior permission.  

SUN MICROSYSTEMS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, 
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT 
SHALL SUN MICROSYSTEMS BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/


#ifndef HPPEX_H
#define HPPEX_H

#include "X11/PEX5/PEX.h"

/* The HP "draw to front buffer" escape */
#define HP_ESCAPE_ET_DFRONT             0x8701
#define HP_ESCAPE_ETM_DFRONT            "HP_ESCAPE_DFRONT"
#define HP_ESCAPE_DFRONT		0x80070001

#define HP_RENDER_TO_BACK_BUFFER	0
#define HP_RENDER_TO_FRONT_BUFFER	1

#endif /* HPpex.h */

