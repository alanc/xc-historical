/* $XConsortium: free.c,v 1.1 93/07/19 11:39:21 mor Exp $ */

/******************************************************************************
Copyright 1993 by the Massachusetts Institute of Technology

                        All Rights Reserved

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation, and that the name of M.I.T. not be used in advertising or
publicity pertaining to distribution of the software without specific,
written prior permission.  M.I.T. makes no representations about the
suitability of this software for any purpose.  It is provided "as is"
without express or implied warranty.
******************************************************************************/

#include "XIElibint.h"

#define CHECK_AND_FREE(_ptr) if (_ptr) Xfree (_ptr)


void
XieFreeTechniques (techs, count)

XieTechnique 	*techs;
unsigned int	count;

{
    int i;

    if (techs)
    {
	for (i = 0; i < count; i++)
	    CHECK_AND_FREE (techs[i].name);

	Xfree ((char *) techs);
    }
}


void
XieFreePhotofloGraph (elements, count)

XiePhotoElement *elements;
unsigned int    count;

{
    /*
     * NOTE: We do not free the technique parameters here.
     * Most of the technique parameters should be freed by the
     * client using Xfree (exception: EncodeJPEGBaseline and
     * EncodeJPEGLossless, see functions below).  This is so
     * the client can reuse technique parameters between photoflos.
     */

    int i;

    if (!elements)
	return;

    for (i = 0; i < count; i++)
    {
	switch (elements[i].elemType)
	{
	case xieElemConvolve:
	    CHECK_AND_FREE ((char *) elements[i].data.Convolve.kernel);
	    break;
	case xieElemPasteUp:
	    CHECK_AND_FREE ((char *) elements[i].data.PasteUp.tiles);
	    break;
	default:
	    break;
	}
    }

    Xfree ((char *) elements);
}


void
XieFreeEncodeJPEGBaseline (param)

XieEncodeJPEGBaselineParam *param;

{
    if (param)
    {
	CHECK_AND_FREE ((char *) param->q_table);
	CHECK_AND_FREE ((char *) param->ac_table);
	CHECK_AND_FREE ((char *) param->dc_table);
	Xfree ((char *) param);
    }
}


void
XieFreeEncodeJPEGLossless (param)

XieEncodeJPEGLosslessParam *param;

{
    if (param)
    {
	CHECK_AND_FREE ((char *) param->table);
	Xfree ((char *) param);
    }
}


void 
XieFreePasteUpTiles (element)

XiePhotoElement	*element;

{
    XieTile *tiles= element->data.PasteUp.tiles;

    if (tiles)
    {
	Xfree (tiles);
	element->data.PasteUp.tiles=NULL;
    }
}
