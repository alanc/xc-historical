/* $XConsortium: t1info.c,v 1.13 94/02/04 17:07:16 gildea Exp $ */
/* Copyright International Business Machines,Corp. 1991
 * All Rights Reserved
 *
 * License, subject to the license given below, to use,
 * copy, modify, and distribute this software * and its
 * documentation for any purpose and without fee is hereby
 * granted, provided that the above copyright notice appear
 * in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation,
 * and that the name of IBM not be used in advertising or
 * publicity pertaining to distribution of the software
 * without specific, written prior permission.
 *
 * IBM PROVIDES THIS SOFTWARE "AS IS", WITHOUT ANY WARRANTIES
 * OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT
 * LIMITED TO ANY IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS.  THE ENTIRE RISK AS TO THE QUALITY AND
 * PERFORMANCE OF THE SOFTWARE, INCLUDING ANY DUTY TO SUPPORT
 * OR MAINTAIN, BELONGS TO THE LICENSEE.  SHOULD ANY PORTION OF
 * THE SOFTWARE PROVE DEFECTIVE, THE LICENSEE (NOT IBM) ASSUMES
 * THE ENTIRE COST OF ALL SERVICING, REPAIR AND CORRECTION.  IN
 * NO EVENT SHALL IBM BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
 * FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
 * Author: Carol H. Thompson  IBM Almaden Research Center
 *   Modeled on spinfo.c by Dave Lemke, Network Computing Devices, Inc
 *   which contains the following copyright and permission notices:
 *
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
 */

#include <stdio.h> 
#include "fntfilst.h"
#include "FSproto.h"
#include "t1intf.h"
#include <math.h>
 
#define DECIPOINTSPERINCH 722.7
#define DEFAULTRES 75
#define DEFAULTPOINTSIZE 120
 
enum scaleType {
    atom, truncate_atom, pixel_size, point_size, resolution_x,
    resolution_y, average_width
};
 
typedef struct _fontProp {
    char       *name;
    long        atom;
    enum scaleType type;
}           fontProp;
 
static fontProp fontNamePropTable[] = {  /* Example: */
    "FOUNDRY", 0, atom,                  /* adobe */
    "FAMILY_NAME", 0, atom,              /* times roman */
    "WEIGHT_NAME", 0, truncate_atom,     /* bold */
    "SLANT", 0, atom,			 /* i */
    "SETWIDTH_NAME", 0, atom,            /* normal */
    "ADD_STYLE_NAME", 0, atom,		 /* */
    "PIXEL_SIZE", 0, pixel_size,         /* 18 */
    "POINT_SIZE", 0, point_size,         /* 180 */
    "RESOLUTION_X", 0, resolution_x,     /* 72 */
    "RESOLUTION_Y", 0, resolution_y,     /* 72 */
    "SPACING", 0, atom,                  /* p */
    "AVERAGE_WIDTH", 0, average_width,   /* 0 */
    "CHARSET_REGISTRY", 0, atom,         /* ISO8859 */
    "CHARSET_ENCODING", 0, truncate_atom, /* 1 */
};
 
/* NOTICE: Following array is closely related to the sequence of defines
   following it. */
static fontProp extraProps[] = {
    "FONT", 0, 0,
    "COPYRIGHT", 0, 0,
    "RAW_PIXELSIZE", 0, 0,
    "RAW_POINTSIZE", 0, 0,
    "RAW_ASCENT", 0, 0,
    "RAW_DESCENT", 0, 0,
    "RAW_AVERAGE_WIDTH", 0, 0,
    "FACE_NAME", 0, 0,
};
 
/* this is a bit kludgy */
#define FONTPROP        0
#define COPYRIGHTPROP   1
#define RAWPIXELPROP	2
#define RAWPOINTPROP	3
#define RAWASCENTPROP	4
#define RAWDESCENTPROP	5
#define RAWWIDTHPROP	6
#define FACE_NAMEPROP	7

#define NNAMEPROPS (sizeof(fontNamePropTable) / sizeof(fontProp))
#define NEXTRAPROPS (sizeof(extraProps) / sizeof(fontProp))
 
#define NPROPS  (NNAMEPROPS + NEXTRAPROPS)
 
/*ARGSUSED*/
static void
FillHeader(pInfo, Vals)
    FontInfoPtr         pInfo;
    FontScalablePtr     Vals;
{
    /* OpenScalable in T1FUNCS sets the following:
    pInfo->firstCol,
    pInfo->firstRow,
    pInfo->lastCol, and
    pInfo->lastRow. */
    /* the following are ununsed
    pInfo->pad. */
 
    /* Items we should handle better someday +++ */
    pInfo->defaultCh = 0;
    pInfo->drawDirection = LeftToRight;
    if (Vals->point_matrix[0] == Vals->point_matrix[3])
	pInfo->anamorphic = 0;
    else
	pInfo->anamorphic = 1;
    pInfo->inkMetrics = 0;  /* no ink metrics here */
    pInfo->cachable = 1;    /* no licensing (yet) */
}
 
static void
adjust_min_max(minc, maxc, tmp)
    xCharInfo  *minc,
               *maxc,
               *tmp;
{
#define MINMAX(field,ci) \
        if (minc->field > (ci)->field) \
             minc->field = (ci)->field; \
        if (maxc->field < (ci)->field) \
            maxc->field = (ci)->field;
 
    MINMAX(ascent, tmp);
    MINMAX(descent, tmp);
    MINMAX(leftSideBearing, tmp);
    MINMAX(rightSideBearing, tmp);
    MINMAX(characterWidth, tmp);

    /* Do MINMAX for attributes field.  Since that field is CARD16,
       we'll cast to a signed integer */
    if ((INT16)minc->attributes > (INT16)tmp->attributes)
         minc->attributes = tmp->attributes;
    if ((INT16)maxc->attributes < (INT16)tmp->attributes)
        maxc->attributes = tmp->attributes;
 
#undef  MINMAX
}
 
static void
ComputeBounds(pInfo, pChars, Vals)
    FontInfoPtr         pInfo;
    CharInfoPtr         pChars;
    FontScalablePtr     Vals;
{
    int i;
    xCharInfo minchar, maxchar;
    long width = 0;
    int numchars = 0;
    int totchars;
    int overlap;
    int maxlap;
 
    minchar.ascent = minchar.descent =
        minchar.leftSideBearing = minchar.rightSideBearing =
        minchar.characterWidth = minchar.attributes = 32767;
    maxchar.ascent = maxchar.descent =
        maxchar.leftSideBearing = maxchar.rightSideBearing =
        maxchar.characterWidth = maxchar.attributes = -32767;
 
    maxlap = -32767;
    totchars = pInfo->lastCol - pInfo->firstCol + 1;
    pChars += pInfo->firstCol - FIRSTCOL;
    pInfo->allExist = 1;
    for (i = 0; i < totchars; i++,pChars++) {
        xCharInfo *pmetrics = &pChars->metrics;
 
        if (pmetrics->attributes ||
	    pmetrics->ascent != -pmetrics->descent ||
	    pmetrics->leftSideBearing != pmetrics->rightSideBearing) {
            width += pmetrics->characterWidth;
            numchars++;
            adjust_min_max(&minchar, &maxchar, pmetrics);
            overlap = pmetrics->rightSideBearing - pmetrics->characterWidth;
            if (overlap > maxlap) maxlap = overlap;
        }
        else pInfo->allExist = 0;
    }
 
    if (numchars == 0)
	Vals->width = 0;
    else
	Vals->width = (width * 10 + (width > 0 ? numchars : -numchars) / 2)
		      / numchars;
    /* (We think the above average width value should be put into
        the Vals structure.  This may be wrong, and the proper
        behavior might be to regard the values in Vals as sacred,
        and for us to squirrel the computed number in a static, and
        then use that static in ComputeStdProps.) */
    pInfo->maxbounds = maxchar;
    pInfo->minbounds = minchar;
    pInfo->ink_maxbounds = maxchar;
    pInfo->ink_minbounds = minchar;
    pInfo->maxOverlap = maxlap + -(minchar.leftSideBearing);
 
    /* Set the pInfo flags */
    /* Properties set by FontComputeInfoAccelerators:
        pInfo->noOverlap;
        pInfo->terminalFont;
        pInfo->constantMetrics;
        pInfo->constantWidth;
        pInfo->inkInside;
 
    */
    FontComputeInfoAccelerators (pInfo);
}
 
static void
ComputeProps(pInfo, Vals, Filename, sAscent, sDescent)
    FontInfoPtr         pInfo;
    FontScalablePtr     Vals;
    char                *Filename;
    long		*sAscent;
    long		*sDescent;
{
    int infoint;
    int infoBBox[4];
    int rc;
 
    QueryFontLib(Filename, "isFixedPitch", &infoint, &rc);
    if (!rc) {
        pInfo->constantWidth = infoint;
    }
    QueryFontLib((char *)0, "FontBBox", infoBBox, &rc);
    if (!rc) {
	pInfo->fontAscent =
	    (int)((double)infoBBox[3] * Vals->pixel_matrix[3] +
		  (infoBBox[3] > 0 ? 500 : -500)) / 1000;
	pInfo->fontDescent =
	    -(int)((double)infoBBox[1] * Vals->pixel_matrix[3] +
		   (infoBBox[1] > 0 ? 500 : -500)) / 1000;
	*sAscent = infoBBox[3];
	*sDescent = -infoBBox[1];
    }
}
 
static void
ComputeStdProps(pInfo, Vals, Filename, Fontname, sAscent, sDescent, sWidth)
    FontInfoPtr         pInfo;
    FontScalablePtr     Vals;
    char                *Filename;
    char                *Fontname;
    long		sAscent;
    long		sDescent;
    long		sWidth;
{
    FontPropPtr pp;
    int         i,
                nprops;
    fontProp   *fpt;
    char       *is_str;
    char       *ptr1,
               *ptr2;
    char       *ptr3;
    char *infostrP;
    long rc;
    char      scaledName[MAXFONTNAMELEN];
 
    strcpy (scaledName, Fontname);
    /* Fill in our copy of the fontname from the Vals structure */
    FontParseXLFDName (scaledName, Vals, FONT_XLFD_REPLACE_VALUE);
 
    /* This form of the properties is used by the X-client; the X-server
       doesn't care what they are. */
    nprops = pInfo->nprops = NPROPS;
    pInfo->isStringProp = (char *) xalloc(sizeof(char) * nprops);
    pInfo->props = (FontPropPtr) xalloc(sizeof(FontPropRec) * nprops);
    if (!pInfo->isStringProp || !pInfo->props) {
        xfree(pInfo->isStringProp);
        pInfo->isStringProp = (char *) 0;
        xfree(pInfo->props);
        pInfo->props = (FontPropPtr) 0;
        return;
    }
    bzero(pInfo->isStringProp, (sizeof(char) * nprops));
 
    ptr2 = scaledName;
    for (i = NNAMEPROPS, pp = pInfo->props, fpt = fontNamePropTable, is_str = pInfo->isStringProp;
            i;
            i--, pp++, fpt++, is_str++) {

	if (*ptr2)
	{
	    ptr1 = ptr2 + 1;
	    if (!(ptr2 = strchr(ptr1, '-'))) ptr2 = strchr(ptr1, '\0');
	}

        pp->name = fpt->atom;
        switch (fpt->type) {
         case atom:  /* Just copy info from scaledName */
            *is_str = TRUE;
            pp->value = MakeAtom(ptr1, ptr2 - ptr1, TRUE);
            break;
	case truncate_atom:
            *is_str = TRUE;
	    for (ptr3 = ptr1; *ptr3; ptr3++)
		if (*ptr3 == '=' || *ptr3 == '-' ||
		    *ptr3 == '+' || *ptr3 == '~')
		    break;
	    pp->value = MakeAtom(ptr1, ptr3 - ptr1, TRUE);
	    break;
         case pixel_size:
            pp->value = (int)(fabs(Vals->pixel_matrix[3]) + .5);
            break;
         case point_size:
            pp->value = (int)(fabs(Vals->point_matrix[3]) * 10.0 + .5);
            break;
         case resolution_x:
            pp->value = Vals->x;
            break;
         case resolution_y:
            pp->value = Vals->y;
            break;
         case average_width:
            pp->value = Vals->width;
            break;
        }
    }
 
    for (i = 0, fpt = extraProps;
          i < NEXTRAPROPS;
          i++, is_str++, pp++, fpt++) {
        pp->name = fpt->atom;
        switch (i) {
         case FONTPROP:
            *is_str = TRUE;
            pp->value = MakeAtom(scaledName, strlen(scaledName), TRUE);
            break;
         case COPYRIGHTPROP:
            *is_str = TRUE;
            QueryFontLib(Filename, "Notice", &infostrP, &rc);
            if (rc || !infostrP) {
                infostrP = "Copyright Notice not available";
            }
            pp->value = MakeAtom(infostrP, strlen(infostrP), TRUE);
            break;
         case FACE_NAMEPROP:
            *is_str = TRUE;
            QueryFontLib(Filename, "FontName", &infostrP, &rc);
            if (rc || !infostrP) {
                infostrP = "(unknown)"; 
            }
            pp->value = MakeAtom(infostrP, strlen(infostrP), TRUE);
            break;
         case RAWPIXELPROP:
            *is_str = FALSE;
            pp->value = 1000;
	    break;
         case RAWPOINTPROP:
            *is_str = FALSE;
            pp->value = (long)(72270.0 / (double)Vals->y + .5);
	    break;
         case RAWASCENTPROP:
            *is_str = FALSE;
            pp->value = sAscent;
	    break;
         case RAWDESCENTPROP:
            *is_str = FALSE;
            pp->value = sDescent;
	    break;
         case RAWWIDTHPROP:
            *is_str = FALSE;
            pp->value = sWidth;
	    break;
        }
    }
}
 
/*ARGSUSED*/
int
Type1GetInfoScalable(fpe, pInfo, entry, fontName, fileName, Vals)
    FontPathElementPtr  fpe;
    FontInfoPtr         pInfo;
    FontEntryPtr        entry;
    FontNamePtr         fontName;
    char                *fileName;
    FontScalablePtr     Vals;
{
    FontPtr pfont;
    int flags = 0;
    long format = 0;  /* It doesn't matter what format for just info */
    long fmask = 0;
    int ret;
 
    ret = Type1OpenScalable(fpe, &pfont, flags, entry, fileName, Vals, format, fmask);
    if (ret != Successful)
	return ret;
    *pInfo = pfont->info;

    /* XXX - Set pointers in pfont->info to NULL so they are not freed. */
    pfont->info.props = NULL;
    pfont->info.isStringProp = NULL;

    Type1CloseFont(pfont);
    return Successful;
}
 
void
T1FillFontInfo(pFont, Vals, Filename, Fontname, sWidth)
    FontPtr             pFont;
    FontScalablePtr     Vals;
    char                *Filename;
    char                *Fontname;
    long		sWidth;
{
    FontInfoPtr         pInfo = &pFont->info;
    struct type1font *p = (struct type1font *)pFont->fontPrivate;
    long sAscent, sDescent;	/* Scalable 1000-pixel values */
 
    FillHeader(pInfo, Vals);
 
    ComputeBounds(pInfo, p->glyphs, Vals);
 
    ComputeProps(pInfo, Vals, Filename, &sAscent, &sDescent);
    ComputeStdProps(pInfo, Vals, Filename, Fontname, sAscent, sDescent, sWidth);
}
 
/* Called once, at renderer registration time */
void
T1InitStdProps()
{
    int         i;
    fontProp   *t;
 
    i = sizeof(fontNamePropTable) / sizeof(fontProp);
    for (t = fontNamePropTable; i; i--, t++)
        t->atom = MakeAtom(t->name, (unsigned) strlen(t->name), TRUE);
    i = sizeof(extraProps) / sizeof(fontProp);
    for (t = extraProps; i; i--, t++)
        t->atom = MakeAtom(t->name, (unsigned) strlen(t->name), TRUE);
}
