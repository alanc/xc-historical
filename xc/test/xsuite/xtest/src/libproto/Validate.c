/*
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology and
 * UniSoft Group Limited.
 * 
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the names of MIT and UniSoft not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  MIT and UniSoft
 * make no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * $XConsortium: Validate.c,v 1.5 92/06/11 15:53:24 rws Exp $
 */
/*
 * ***************************************************************************
 *  Copyright 1989 by Sequent Computer Systems, Inc., Portland, Oregon       *
 *                                                                           *
 *                                                                           *
 *                          All Rights Reserved                              *
 *                                                                           *
 *  Permission to use, copy, modify, and distribute this software and its    *
 *  documentation for any purpose and without fee is hereby granted,         *
 *  provided that the above copyright notice appears in all copies and that  *
 *  both that copyright notice and this permission notice appear in          *
 *  supporting documentation, and that the name of Sequent not be used       *
 *  in advertising or publicity pertaining to distribution or use of the     *
 *  software without specific, written prior permission.                     *
 *                                                                           *
 *  SEQUENT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING *
 *  ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL *
 *  SEQUENT BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR  *
 *  ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,      *
 *  WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,   *
 *  ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS      *
 *  SOFTWARE.                                                                *
 * ***************************************************************************
 */

#include "Xstlib.h"
#include <X11/X.h>


/* 
   intent:	 return True if the client's visual is of a class that
                 permits allocating color cells
   input:	 client - integer from 0 to MAX_CLIENTS
   output:	 
   global input:
   side effects: 
   methods:	 
*/

int
Allocatable (client)
int client;
{
    int visual_class;
	XstDisplay	*dsp;

	dsp = Get_Display(client);
    visual_class = Get_Visual_Class(Get_Visual(client,XstDefaultScreen(dsp)));

    return ((visual_class == GrayScale) || (visual_class == PseudoColor) || (visual_class == DirectColor));
}
