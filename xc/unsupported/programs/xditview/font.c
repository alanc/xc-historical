/*
 * font.c
 *
 * map dvi fonts to X fonts
 */

#include <X11/Xos.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <stdio.h>
#include <ctype.h>
#include "DviP.h"
#include "XFontName.h"

extern char	*malloc ();

static char *
savestr (s)
	char	*s;
{
	char	*n;

	if (!s)
		return 0;
	n = malloc (strlen (s) + 1);
	if (n)
		strcpy (n, s);
	return n;
}

static DviFontList *
LookupFontByPosition (dw, position)
	DviWidget	dw;
	int		position;
{
	DviFontList	*f;

	for (f = dw->dvi.fonts; f; f=f->next)
		if (f->dvi_number == position)
			break;
	return f;
}

static DviFontSizeList *
LookupFontSizeBySize (dw, f, size)
    DviWidget	dw;
    DviFontList	*f;
    int		size;
{
    DviFontSizeList *fs, *best = 0;
    int		    bestsize = 0;
    char	    fontNameString[2048];
    XFontName	    fontName;
    unsigned int    fontNameAttributes;

    if (f->scalable)
    {
	for (best = f->sizes; best; best = best->next)
	    if (best->size == size)
		return best;
	best = (DviFontSizeList *) malloc (sizeof *best);
	best->next = f->sizes;
	best->size = size;
	XParseFontName (f->x_name, &fontName, &fontNameAttributes);
	fontNameAttributes &= ~(FontNamePixelSize|FontNameAverageWidth);
	fontNameAttributes |= FontNameResolutionX;
	fontNameAttributes |= FontNameResolutionY;
	fontNameAttributes |= FontNamePointSize;
	fontName.ResolutionX = dw->dvi.screen_resolution;
	fontName.ResolutionY = dw->dvi.screen_resolution;
	fontName.PointSize = size * 10;
	XFormatFontName (&fontName, fontNameAttributes, fontNameString);
	best->x_name = savestr (fontNameString);
	best->doesnt_exist = 0;
	best->font = 0;
	f->sizes = best;
    }
    else
    {
    	for (fs = f->sizes; fs; fs=fs->next) {
	    	if (fs->size <= size && fs->size >= bestsize) {
		    	best = fs;
		    	bestsize = fs->size;
	    	}
    	}
    }
    return best;
}

static char *
SkipFontNameElement (n)
	char	*n;
{
	while (*n != '-')
		if (!*++n)
			return 0;
	return n+1;
}

# define SizePosition		8
# define EncodingPosition	13

static
ConvertFontNameToSize (n)
	char	*n;
{
	int	i, size;

	for (i = 0; i < SizePosition; i++) {
		n = SkipFontNameElement (n);
		if (!n)
			return -1;
	}
	size = atoi (n);
	return size/10;
}

static char *
ConvertFontNameToEncoding (n)
	char	*n;
{
        int i;
	for (i = 0; i < EncodingPosition; i++) {
		n = SkipFontNameElement (n);
		if (!n)
			return 0;
	}
	return n;
}

DviFontSizeList *
InstallFontSizes (dw, x_name, scalablep)
	DviWidget	dw;
	char		*x_name;
	int		*scalablep;
{
    char	    fontNameString[2048];
    char	    **fonts;
    int		    i, count;
    int		    size;
    DviFontSizeList *sizes, *new;
    XFontName	    fontName;
    unsigned int    fontNameAttributes;

    if (!XParseFontName (x_name, &fontName, &fontNameAttributes))
	return;
    fontNameAttributes &= ~(FontNamePixelSize|FontNamePointSize);
    fontNameAttributes |= FontNameResolutionX;
    fontNameAttributes |= FontNameResolutionY;
    fontName.ResolutionX = dw->dvi.screen_resolution;
    fontName.ResolutionY = dw->dvi.screen_resolution;
    XFormatFontName (&fontName, fontNameAttributes, fontNameString);
    fonts = XListFonts (XtDisplay (dw), fontNameString, 10000000, &count);
    sizes = 0;
    *scalablep = FALSE;
    for (i = 0; i < count; i++) {
	size = ConvertFontNameToSize (fonts[i]);
	if (size == 0)
	{
	    DisposeFontSizes (sizes);
	    *scalablep = TRUE;
	    sizes = 0;
	    break;
	}
	if (size != -1) {
	    new = (DviFontSizeList *) malloc (sizeof *new);
	    new->next = sizes;
	    new->size = size;
	    new->x_name = savestr (fonts[i]);
	    new->doesnt_exist = 0;
	    new->font = 0;
	    sizes = new;
	}
    }
    XFreeFontNames (fonts);
    return sizes;
}

static
DisposeFontSizes (fs)
	DviFontSizeList	*fs;
{
	DviFontSizeList	*next;

	for (; fs; fs=next) {
		next = fs->next;
		if (fs->x_name)
			free (fs->x_name);
		if (fs->font)
			XFree ((char *)fs->font);
		free ((char *) fs);
	}
}

static DviFontList *
InstallFont (dw, position, dvi_name, x_name)
	DviWidget	dw;
	int		position;
	char		*dvi_name;
	char		*x_name;
{
    DviFontList	*f;
    DviFontSizeList	*sizes;
    char		*encoding;
    Boolean		scalable;

    if (f = LookupFontByPosition (dw, position)) {
	/*
	 * ignore gratuitous font loading
	 */
	if (!strcmp (f->dvi_name, dvi_name) && !strcmp (f->x_name, x_name))
	    return;

	sizes = InstallFontSizes (dw, x_name, &scalable);
	if (!sizes && !scalable)
	    return f;

	DisposeFontSizes (f->sizes);
	if (f->dvi_name)
	    free (f->dvi_name);
	if (f->x_name)
	    free (f->x_name);
    } else {
	sizes = InstallFontSizes (dw, x_name, &scalable);
	if (!sizes && !scalable)
	    return 0;
	f = (DviFontList *) malloc (sizeof (*f));
	f->next = dw->dvi.fonts;
	dw->dvi.fonts = f;
    }
    f->dvi_name = savestr (dvi_name);
    f->x_name = savestr (x_name);
    f->dvi_number = position;
    f->sizes = sizes;
    f->scalable = scalable;
    if (f->x_name) {
	encoding = ConvertFontNameToEncoding (f->x_name);
	f->char_map = DviFindMap (encoding);
    } else
	f->char_map = 0;
    /* 
     * force requery of fonts
     */
    dw->dvi.font = 0;
    dw->dvi.font_number = -1;
    dw->dvi.cache.font = 0;
    dw->dvi.cache.font_number = -1;
    return f;
}

static char *
MapDviNameToXName (dw, dvi_name)
	DviWidget	dw;
	char		*dvi_name;
{
    DviFontMap	*fm;
    
    for (fm = dw->dvi.font_map; fm; fm=fm->next)
	if (!strcmp (fm->dvi_name, dvi_name))
	    return fm->x_name;
    for (fm = dw->dvi.font_map; fm; fm=fm->next)
	if (!strcmp (fm->dvi_name, "R"))
	    return fm->x_name;
    return dw->dvi.font_map->x_name;
}

static char *
MapXNameToDviName (dw, x_name)
	DviWidget	dw;
	char		*x_name;
{
    DviFontMap	*fm;
    
    for (fm = dw->dvi.font_map; fm; fm=fm->next)
	if (!strcmp (fm->x_name, x_name))
	    return fm->dvi_name;
    return 0;
}

ParseFontMap (dw)
	DviWidget	dw;
{
    char		dvi_name[1024];
    char		x_name[2048];
    char		*m, *s;
    char		*encoding;
    DviFontMap	*fm, *new;

    if (dw->dvi.font_map)
	    DestroyFontMap (dw->dvi.font_map);
    fm = 0;
    m = dw->dvi.font_map_string;
    while (*m) {
	s = m;
	while (*m && !isspace (*m))
	    ++m;
	strncpy (dvi_name, s, m-s);
	dvi_name[m-s] = '\0';
	while (isspace (*m))
	    ++m;
	s = m;
	while (*m && *m != '\n')
	    ++m;
	strncpy (x_name, s, m-s);
	x_name[m-s] = '\0';
	new = (DviFontMap *) malloc (sizeof *new);
	new->x_name = savestr (x_name);
	new->dvi_name = savestr (dvi_name);
	new->next = fm;
	fm = new;
	++m;
    }
    dw->dvi.font_map = fm;
}

DestroyFontMap (font_map)
    DviFontMap	*font_map;
{
    DviFontMap	*next;

    for (; font_map; font_map = next) {
	next = font_map->next;
	if (font_map->x_name)
	    free (font_map->x_name);
	if (font_map->dvi_name)
	    free (font_map->dvi_name);
	free ((char *) font_map);
    }
}

SetFontPosition (dw, position, dvi_name, extra)
    DviWidget	dw;
    int		position;
    char	*dvi_name;
    char	*extra;	/* unused */
{
    char	*x_name;

    x_name = MapDviNameToXName (dw, dvi_name);
    (void) InstallFont (dw, position, dvi_name, x_name);
}

XFontStruct *
QueryFont (dw, position, size)
    DviWidget	dw;
    int		position;
    int		size;
{
    DviFontList	*f;
    DviFontSizeList	*fs;

    f = LookupFontByPosition (dw, position);
    if (!f)
	return dw->dvi.default_font;
    fs = LookupFontSizeBySize (dw, f, size);
    if (!fs->font) {
	if (fs->x_name)
	    fs->font = XLoadQueryFont (XtDisplay (dw), fs->x_name);
	if (!fs->font)
	    fs->font = dw->dvi.default_font;
    }
    return fs->font;
}

DviCharNameMap *
QueryFontMap (dw, position)
	DviWidget	dw;
	int		position;
{
	DviFontList	*f;
	char		*encoding;

	f = LookupFontByPosition (dw, position);
	if (f)
	    return f->char_map;
	else
	    return 0;
}

char *
DviCharIsLigature (map, name)
    DviCharNameMap  *map;
    char	    *name;
{
    int	    i;

    for (i = 0; i < DVI_MAX_LIGATURES; i++) {
	if (!map->ligatures[i][0])
	    break;
	if (!strcmp (name, map->ligatures[i][0]))
	    return map->ligatures[i][1];
    }
    return 0;
}

LoadFont (dw, position, size)
	DviWidget	dw;
	int		position;
	int		size;
{
	XFontStruct	*font;

	font = QueryFont (dw, position, size);
	dw->dvi.font_number = position;
	dw->dvi.font_size = size;
	dw->dvi.font = font;
	XSetFont (XtDisplay (dw), dw->dvi.normal_GC, font->fid);
	return;
}
