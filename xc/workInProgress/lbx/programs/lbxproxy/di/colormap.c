/*
 * Copyright 1988-1993 Network Computing Devices, Inc.  All rights reserved.
 * An unpublished work.
 *
 * $NCDId: @(#)colormap.c,v 1.1 1993/12/01 23:46:29 lemke Exp $
 */

/*
 * XXX
 *
 * known glitches:
 *
 * LookupColor
 *	- only default cmap supported.  others possible, but need some
 *		data structure reorg.  perhaps hang RGBEntry cache off
 *		each cmap instead
 *
 * Alloc{Named}Color
 *	- no support for DirectColor or TrueColor visuals
 *		should just be a matter of adding more code all
 *		over the place
 *
 * FreeColors
 *	- not supporting planemask argument
 *
 * XXX
 */

#include	<stdio.h>
#include	<X11/X.h>	/* for KeymapNotify */
#include	<X11/Xproto.h>
#include	"lbxdata.h"
#include	"assert.h"
#include	"resource.h"
#include	"colormap.h"
#include	"cmapst.h"
#include	"util.h"
#include	"lbx.h"

#define	NBUCKETS	16

typedef struct _RGBEntry {
    struct _RGBEntry *next;
    RGBEntryRec color;
}           RGBCacheEntryRec, *RGBCacheEntryPtr;

static RGBCacheEntryPtr rgb_cache[NBUCKETS];

/*
 * colormap cache code
 *
 * handles RGB database and AllocColor
 *
 */


/*
 * colorname cache
 *
 * assumes that hashing on the name makes the most sense.
 *
 *
 * XXX this gets real messy with colormaps other than the default,
 * since then we need to track the buggers to make sure we flush
 * entries when the cmap goes away.  we may simply want to cheat
 * and ignore non-default cmaps
 */


static int
Hash(name, len)
    char       *name;
    int         len;
{
    int         hash = 0;

    while (len--)
	hash = (hash << 1) ^ *name++;
    if (hash < 0)
	hash = -hash;
    return hash;
}

RGBEntryPtr
FindColorName(name, len, cmap)
    char       *name;
    int         len;
    Colormap    cmap;
{
    RGBCacheEntryPtr ce;
    int         hash;

    hash = Hash(name, len) % NBUCKETS;

    ce = rgb_cache[hash];

    while (ce) {
	if ((ce->color.cmap == cmap) &&
		(ce->color.namelen == len) &&
		!strncmp(ce->color.name, name, len)) {
	    return &ce->color;
	}
	ce = ce->next;
    }
    return (RGBEntryPtr) NULL;
}

Bool
AddColorName(name, len, rgbe)
    char       *name;
    int         len;
    RGBEntryRec rgbe;
{
    RGBCacheEntryPtr ce,
                new;
    int         hash;

    hash = Hash(name, len) % NBUCKETS;

    ce = rgb_cache[hash];

    new = (RGBCacheEntryPtr) xalloc(sizeof(RGBCacheEntryRec));
    if (!new)
	return FALSE;
    if (!ce) {
	rgb_cache[hash] = new;
    } else {
	while (ce->next) {

#ifdef DEBUG
	    if ((rgbe.cmap == ce->color.cmap) &&
		    (len == ce->color.namelen) &&
		    !strncmp(name, ce->color.name, len)) {
		fprintf(stderr, "resetting color name\n");
	    }
#endif

	    ce = ce->next;
	}
	ce->next = new;
    }
    new->color = rgbe;
    new->color.name = strnalloc(name, len);
    new->color.namelen = len;
    new->next = NULL;
    return TRUE;
}

/*****************************************************************************/

typedef struct _visual {
    int         class;
    VisualID    id;
    int         depth;
}           VisualRec, *VisualPtr;

static int  num_visuals = 0;

VisualPtr  *visuals;

typedef struct {
    Colormap    mid;
    int         client;
}           colorResource;

int
CreateVisual(depth, vis)
    int         depth;
    xVisualType *vis;
{
    VisualPtr   pvis;

    pvis = (VisualPtr) xalloc(sizeof(VisualRec));
    if (!pvis)
	return 0;
    pvis->id = vis->visualID;
    pvis->class = vis->class;
    pvis->depth = depth;
    visuals = (VisualPtr *) xrealloc(visuals, 
    	(num_visuals + 1) * sizeof(VisualPtr));
    if (!visuals) {
	xfree(pvis);
	return 0;
    }
    visuals[num_visuals++] = pvis;

    return 1;
}

static      VisualPtr
get_visual(vid)
    VisualID    vid;
{
    VisualPtr   pvis;
    int         i;

    for (i = 0; i < num_visuals; i++) {
	pvis = visuals[i];
	if (pvis->id == vid)
	    return pvis;
    }
    assert(0);
    return (VisualPtr) 0;
}

static int
find_matching_pixel(pent, num, red, green, blue, pe)
    Entry      *pent;
    int         num;
    int         red,
                green,
                blue;
    Entry     **pe;
{
    int         i;

    if (!pent)
	return 0;

    *pe = (Entry *) 0;
    for (i = 0; i < num; i++) {
	if (pent[i].refcnt == 0)
	    continue;
	if ((pent[i].red == red) &&
		(pent[i].blue == blue) && (pent[i].green == green)) {
	    *pe = &pent[i];
	    return 1;
	}
    }
    return 0;
}

static int
find_named_pixel(pent, num, name, namelen, pe)
    Entry      *pent;
    int         num;
    char       *name;
    int         namelen;
    Entry     **pe;
{
    int         i;

    if (!pent)
	return 0;

    *pe = (Entry *) 0;
    for (i = 0; i < num; i++) {
	if (pent[i].refcnt == 0)
	    continue;
	if (pent[i].len == namelen && !strncmp(pent[i].name, name, namelen)) {
	    *pe = &pent[i];
	    return 1;
	}
    }
    return 0;
}

int
FindPixel(client, cmap, red, green, blue, pent)
    ClientPtr   client;
    Colormap    cmap;
    int         red,
                green,
                blue;
    Entry     **pent;
{
    ColormapPtr pmap;
    int         i;
    Entry      *pe;

    pmap = (ColormapPtr) LookupIDByType(cmap, RT_COLORMAP);
    if (!pmap) {
	return 0;
    }
    *pent = (Entry *) 0;
    switch (pmap->class) {
    case StaticGray:
    case StaticColor:
	if (find_matching_pixel(pmap->red, pmap->size,
				red, green, blue, &pe)) {
	    *pent = pe;
	    return 1;
	}
	return 0;
	break;
    case PseudoColor:
    case GrayScale:
	if (find_matching_pixel(pmap->red, pmap->size,
				red, green, blue, &pe)) {
	    *pent = pe;
	    return 1;
	}
	return 0;
	break;
    case TrueColor:
    case DirectColor:
	/* XXX */
	return 0;
	break;
    default:
	assert(0);
	break;
    }
}

int
FindNamedPixel(client, cmap, name, namelen, pent)
    ClientPtr   client;
    Colormap    cmap;
    char       *name;
    int         namelen;
    Entry     **pent;
{
    Entry      *pe;
    ColormapPtr pmap;
    int         i;
    Pixel       pix;

    pmap = (ColormapPtr) LookupIDByType(cmap, RT_COLORMAP);
    if (!pmap) {
	return 0;
    }
    *pent = (Entry *) 0;
    switch (pmap->class) {
    case StaticGray:
    case StaticColor:
	if (find_named_pixel(pmap->red, pmap->size, name, namelen, &pe)) {
	    *pent = pe;
	    return 1;
	}
	return 0;
	break;
    case PseudoColor:
    case GrayScale:
	if (find_named_pixel(pmap->red, pmap->size, name, namelen, &pe)) {
	    *pent = pe;
	    return 1;
	}
	return 0;
	break;
    case TrueColor:
    case DirectColor:
	/* XXX */
	return 0;
	break;
    default:
	assert(0);
	break;
    }
}

int
StorePixel(client, cmap, red, green, blue, rep_red, rep_green, rep_blue, pixel)
    ClientPtr   client;
    Colormap    cmap;
    int         red,
                green,
                blue;
    int         rep_red,
                rep_green,
                rep_blue;
    Pixel       pixel;
{
    ColormapPtr pmap;
    Entry      *pent;
    int         size;

    pmap = (ColormapPtr) LookupIDByType(cmap, RT_COLORMAP);
    if (!pmap) {
	return 0;
    }
    switch (pmap->class) {
    case StaticGray:
    case StaticColor:
    case PseudoColor:
    case GrayScale:
	pent = pmap->red;
	if (pent[pixel].refcnt)
	    fprintf(stderr, "Overwriting existing pixel\n");

	pent[pixel].red = red;
	pent[pixel].green = green;
	pent[pixel].blue = blue;
	pent[pixel].rep_red = rep_red;
	pent[pixel].rep_green = rep_green;
	pent[pixel].rep_blue = rep_blue;
	pent[pixel].pixel = pixel;

	IncrementPixel(client, cmap, &pent[pixel]);

	return 1;
	break;
    case TrueColor:
    case DirectColor:
	/* XXX */
	return 0;
	break;
    default:
	assert(0);
	break;
    }
}

int
StoreNamedPixel(client, cmap, name, namelen, xred, xgreen, xblue,
		vred, vgreen, vblue, pixel)
    ClientPtr   client;
    Colormap    cmap;
    char       *name;
    int         namelen;
    int         xred,
                xgreen,
                xblue;
    int         vred,
                vgreen,
                vblue;
    Pixel       pixel;
{
    ColormapPtr pmap;
    Entry      *pent;
    RGBEntryRec rgbe;
    int         size;

    pmap = (ColormapPtr) LookupIDByType(cmap, RT_COLORMAP);
    if (!pmap) {
	return 0;
    }
    switch (pmap->class) {
    case StaticGray:
    case StaticColor:
    case PseudoColor:
    case GrayScale:
	pent = pmap->red;

	if (pent[pixel].refcnt)
	    fprintf(stderr, "Overwriting existing pixel name\n");

	pent[pixel].red = xred;
	pent[pixel].green = xgreen;
	pent[pixel].blue = xblue;
	pent[pixel].rep_red = vred;
	pent[pixel].rep_green = vgreen;
	pent[pixel].rep_blue = vblue;
	pent[pixel].pixel = pixel;

	pent[pixel].name = strnalloc(name, namelen);
	pent[pixel].len = namelen;

	IncrementPixel(client, cmap, &pent[pixel]);
	break;
    case TrueColor:
    case DirectColor:
	/* XXX */
	break;
    default:
	assert(0);
	break;
    }
    /* stash RGB value in RGB database as well */
    rgbe.xred = xred;
    rgbe.xblue = xblue;
    rgbe.xgreen = xgreen;
    rgbe.vred = vred;
    rgbe.vblue = vblue;
    rgbe.vgreen = vgreen;
    rgbe.cmap = cmap;
    AddColorName(name, namelen, &rgbe);
}

static void
FreeCell(pmap, pixel, channel)
    ColormapPtr pmap;
    Pixel       pixel;
    int         channel;
{
    Entry      *pent;

    /* XXX ingoring all but red/pseudo channel */
    pent = (Entry *) & pmap->red[pixel];
    if (pent->refcnt > 1)
	pent->refcnt--;
    else {
	pent->refcnt = 0;
	xfree(pent->name);
	bzero((char *) pent, sizeof(Entry));
    }
}

int
FreeAllClientPixels(pmap, client)
    ColormapPtr pmap;
    int         client;
{
    Pixel      *ppix,
               *ppst;
    int         n;

    ppst = pmap->clientPixelsRed[client];
    if (pmap->class & DynamicClass) {
	for (ppix = ppst, n = pmap->numPixelsRed[client]; --n >= 0;) {
	    FreeCell(pmap, *ppix++, 1);
	}
    }
    xfree(ppst);
    pmap->clientPixelsRed[client] = (Pixel *) NULL;
    pmap->numPixelsRed[client] = 0;

    /* XXX add Direct and True color stuff */
}

int
FreeClientPixels(pcr, id)
    colorResource *pcr;

{
    ColormapPtr pmap;

    pmap = (ColormapPtr) LookupIDByType(pcr->mid, RT_COLORMAP);
    if (pmap) {
	FreeAllClientPixels(pmap, pcr->client);
    }
    xfree(pcr);
}

IncrementPixel(pclient, cmap, pent)
    ClientPtr   pclient;
    Colormap    cmap;
    Entry      *pent;
{
    colorResource *pcr;
    ColormapPtr pmap;
    int         npix;
    Pixel      *ppix;
    int         client = pclient->index;

    pmap = (ColormapPtr) LookupIDByType(cmap, RT_COLORMAP);
    if (!pmap) {
	return 0;
    }
    npix = pmap->numPixelsRed[client];
    ppix = (Pixel *) xrealloc(pmap->clientPixelsRed[client],
			      (npix + 1) * sizeof(Pixel));
    if (!ppix)
	return 0;
    ppix[npix] = pent->pixel;
    pmap->clientPixelsRed[client] = ppix;
    pmap->numPixelsRed[client]++;

    pent->refcnt++;

    if ((CLIENT_ID(cmap) != client) && (pmap->numPixelsRed[client] == 1)) {
	pcr = (colorResource *) xalloc(sizeof(colorResource));
	if (!pcr)
	    return 0;
	pcr->mid = cmap;
	pcr->client = client;

	AddResource(FakeClientID(client), RT_CMAPENTRY, (pointer) pcr);
    }
    return 1;
}

int
FreePixels(client, cmap, num, pixels)
    ClientPtr   client;
    Colormap    cmap;
    int         num;
    Pixel       pixels[];
{
    ColormapPtr pmap;
    Entry      *pent;
    Pixel       pix;
    int         i,
                zapped,
                npix,
                npixnew;
    Pixel      *cptr,
               *pptr,
               *ppixClient;
    int         idx = client->index;

/* XXX doesn't handle plane-mask properly */
    pmap = (ColormapPtr) LookupIDByType(cmap, RT_COLORMAP);
    if (!pmap) {
	return 0;
    }
    switch (pmap->class) {
    case StaticGray:
    case StaticColor:
    case PseudoColor:
    case GrayScale:
	if (!pent->red)
	    return 0;
	for (i = 0, pix = pixels[i]; i < num; i++) {
	    for (cptr = pmap->clientPixelsRed[idx],
		    npix = pmap->numPixelsRed[idx];
		    --npix >= 0 && *cptr != pix;
		    cptr++);
	    if (npix >= 0) {
		if (pmap->class & DynamicClass)
		    FreeCell(pmap, pix, 1);
		*cptr = ~((Pixel) 0);
		zapped++;
	    }
	}
	if (zapped) {
	    npixnew = pmap->numPixelsRed[idx] - zapped;
	    if (npixnew) {
		pptr = cptr = pmap->clientPixelsRed[idx];
		for (npix = 0; npix <= npixnew; cptr++) {
		    if (*cptr != ~((Pixel) 0)) {
			*pptr++ = *cptr;
			npix++;
		    }
		}
		ppixClient = pmap->clientPixelsRed[idx];
		pptr = (Pixel *) xrealloc(ppixClient, npixnew * sizeof(Pixel));
		if (pptr)
		    ppixClient = pptr;
		pmap->numPixelsRed[idx] = npixnew;
		pmap->clientPixelsRed[idx] = ppixClient;
	    } else {
		pmap->numPixelsRed[idx] = 0;
		xfree(pmap->clientPixelsRed[idx]);
		pmap->clientPixelsRed[idx] = (Pixel *) NULL;
	    }
	}
	return 1;
	break;
    case TrueColor:
    case DirectColor:
	/* XXX */
	break;
    default:
	assert(0);
	break;
    }
}

static      ColormapPtr
create_colormap(cmap, win, visual)
    Colormap    cmap;
    Window      win;
    VisualID    visual;
{
    ColormapPtr pmap;
    VisualPtr   pvis;
    int         tsize,
                size;
    Pixel     **pptr;

    pvis = get_visual(visual);
    size = (1 << pvis->depth);
    tsize = sizeof(ColormapRec) + (size * sizeof(Entry)) +
	(MAXCLIENTS * sizeof(Pixel *)) + (MAXCLIENTS * sizeof(int));

    pmap = (ColormapPtr) xalloc(tsize);
    if (!pmap)
	return pmap;
    pmap->size = size;
    pmap->id = cmap;
    pmap->window = win;
    pmap->visual = visual;
    pmap->class = pvis->class;
    pmap->red = (Entry *) ((char *) pmap + sizeof(ColormapRec));

    pmap->clientPixelsRed = (Pixel **)
	((char *) pmap->red + (pmap->size * sizeof(Entry)));
    pmap->numPixelsRed = (int *) ((char *) pmap->clientPixelsRed +
				  (MAXCLIENTS * sizeof(Pixel *)));

    bzero((char *) pmap->red, (pmap->size * sizeof(Entry)));
    bzero((char *) pmap->numPixelsRed, (MAXCLIENTS * sizeof(int)));
    for (pptr = &pmap->clientPixelsRed[MAXCLIENTS]; --pptr >= pmap->clientPixelsRed;)
	*pptr = (Pixel *) NULL;

    /* XXX still need to handle Direct and True */

    return pmap;
}

int
CreateColormap(client, cmap, win, visual)
    ClientPtr   client;
    Colormap    cmap;
    Window      win;
    VisualID    visual;
{
    ColormapPtr pmap;

    pmap = create_colormap(cmap, win, visual);
    if (!pmap)
	return 0;
    if (!AddResource(cmap, RT_COLORMAP, (pointer) pmap)) {
	return 0;
    }
    return 1;
}

DestroyColormap(pmap, id)
    ColormapPtr pmap;
    Colormap    id;
{
    xfree(pmap->red);
    xfree(pmap->blue);
    xfree(pmap->green);
    xfree(pmap);
}

int
FreeColormap(client, cmap)
    ClientPtr   client;
    Colormap    cmap;
{
    ColormapPtr pmap;

    pmap = (ColormapPtr) LookupIDByType(cmap, RT_COLORMAP);
    if (!pmap) {
	return 0;
    }
    FreeResource(cmap, RT_NONE);
}

/*
 * this one is real ugly.  just nuke everything for now
 */
int
CopyAndFreeColormap(client, new, old)
    ClientPtr   client;
    Colormap    new,
                old;
{
    ColormapPtr pmap,
                newmap;

    pmap = (ColormapPtr) LookupIDByType(old, RT_COLORMAP);
    if (!pmap) {
	return 0;
    }
    /* trash existing cache values */
    xfree(pmap->red);
    xfree(pmap->blue);
    xfree(pmap->green);
    pmap->red = pmap->blue = pmap->green = (Entry *) 0;

    /* create a new cmap of the same type */
    newmap = create_colormap(new, pmap->window, pmap->visual);
    if (!AddResource(new, RT_COLORMAP, (pointer) newmap)) {
	return 0;
    }
}
