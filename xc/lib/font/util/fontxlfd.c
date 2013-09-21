/* $XConsortium: fontxlfd.c,v 1.19 94/06/05 14:42:48 rws Exp $ */

/*

Copyright (c) 1990  X Consortium

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the X Consortium.

*/

/*
 * Author:  Keith Packard, MIT X Consortium
 */

#include	"fontmisc.h"
#include	"fontstruct.h"
#include	"fontxlfd.h"
#include	<X11/Xos.h>
#include	<math.h>
#ifndef X_NOT_STDC_ENV
#include	<stdlib.h>
#endif
#if defined(X_NOT_STDC_ENV) || (defined(sony) && !defined(SYSTYPE_SYSV) && !defined(_SYSTYPE_SYSV))
#define NO_LOCALE
#endif
#ifndef NO_LOCALE
#include	<locale.h>
#endif
#include	<ctype.h>

static char *
GetInt(ptr, val)
    char       *ptr;
    int        *val;
{
    if (*ptr == '*') {
	*val = -1;
	ptr++;
    } else
	for (*val = 0; *ptr >= '0' && *ptr <= '9';)
	    *val = *val * 10 + *ptr++ - '0';
    if (*ptr == '-')
	return ptr;
    return (char *) 0;
}

#define minchar(p) ((p).min_char_low + ((p).min_char_high << 8))
#define maxchar(p) ((p).max_char_low + ((p).max_char_high << 8))


#ifndef NO_LOCALE
static struct lconv *locale = 0;
#endif
static char *radix = ".", *plus = "+", *minus = "-";

static char *
readreal(ptr, result)
char *ptr;
double *result;
{
    char buffer[80], *p1, *p2;
    int count;

#ifndef NO_LOCALE
    /* Figure out what symbols apply in this locale */

    if (!locale)
    {
	locale = localeconv();
	if (locale->decimal_point && *locale->decimal_point)
	    radix = locale->decimal_point;
	if (locale->positive_sign && *locale->positive_sign)
	    plus = locale->positive_sign;
	if (locale->negative_sign && *locale->negative_sign)
	    minus = locale->negative_sign;
    }
#endif
    /* Copy the first 80 chars of ptr into our local buffer, changing
       symbols as needed. */
    for (p1 = ptr, p2 = buffer;
	 *p1 && (p2 - buffer) < sizeof(buffer) - 1;
	 p1++, p2++)
    {
	switch(*p1)
	{
	    case '~': *p2 = *minus; break;
	    case '+': *p2 = *plus; break;
	    case '.': *p2 = *radix; break;
	    default: *p2 = *p1;
	}
    }
    *p2 = 0;

    /* Now we have something that strtod() can interpret... do it. */
#ifndef X_NOT_STDC_ENV
    *result = strtod(buffer, &p1);
    /* Return NULL if failure, pointer past number if success */
    return (p1 == buffer) ? (char *)0 : (ptr + (p1 - buffer));
#else
    for (p1 = buffer; isspace(*p1); p1++)
	;
    if (sscanf(p1, "%lf", result) != 1)
	return (char *)0;
    while (!isspace(*p1))
	p1++;
    return ptr + (p1 - buffer);
#endif
}

static char *
xlfd_double_to_text(value, buffer, space_required)
double value;
char *buffer;
int space_required;
{
    char formatbuf[40];
    register char *p1;
    int ndigits, exponent;

#ifndef NO_LOCALE
    if (!locale)
    {
	locale = localeconv();
	if (locale->decimal_point && *locale->decimal_point)
	    radix = locale->decimal_point;
	if (locale->positive_sign && *locale->positive_sign)
	    plus = locale->positive_sign;
	if (locale->negative_sign && *locale->negative_sign)
	    minus = locale->negative_sign;
    }
#endif
    /* Compute a format to use to render the number */
    sprintf(formatbuf, "%%.%dle", XLFD_NDIGITS);

    if (space_required)
	*buffer++ = ' ';

    /* Render the number using printf's idea of formatting */
    sprintf(buffer, formatbuf, value);

    /* Find and read the exponent value */
    for (p1 = buffer + strlen(buffer);
	*p1-- != 'e' && p1[1] != 'E';);
    exponent = atoi(p1 + 2);
    if (value == 0.0) exponent = 0;

    /* Figure out how many digits are significant */
    while (p1 >= buffer && (!isdigit(*p1) || *p1 == '0')) p1--;
    ndigits = 0;
    while (p1 >= buffer) if (isdigit(*p1--)) ndigits++;

    /* Figure out notation to use */
    if (exponent >= XLFD_NDIGITS || ndigits - exponent > XLFD_NDIGITS + 1)
    {
	/* Scientific */
	sprintf(formatbuf, "%%.%dle", ndigits - 1);
	sprintf(buffer, formatbuf, value);
    }
    else
    {
	/* Fixed */
	ndigits -= exponent + 1;
	if (ndigits < 0) ndigits = 0;
	sprintf(formatbuf, "%%.%dlf", ndigits);
	sprintf(buffer, formatbuf, value);
	if (exponent < 0)
	{
	    p1 = buffer;
	    while (*p1 && *p1 != '0') p1++;
	    while (*p1++) p1[-1] = *p1;
	}
    }

    /* Last step, convert the locale-specific sign and radix characters
       to our own. */
    for (p1 = buffer; *p1; p1++)
    {
	if (*p1 == *minus) *p1 = '~';
	else if (*p1 == *plus) *p1 = '+';
	else if (*p1 == *radix) *p1 = '.';
    }

    return buffer - space_required;
}

double
xlfd_round_double(x)
double x;
{
    /* Utility for XLFD users to round numbers to XLFD_NDIGITS
       significant digits.  How do you round to n significant digits on
       a binary machine?  Let printf() do it for you.  */
    char formatbuf[40], buffer[40];

    sprintf(formatbuf, "%%.%dlg", XLFD_NDIGITS);
    sprintf(buffer, formatbuf, x);
    return atof(buffer);
}

static char *
GetMatrix(ptr, vals, which)
char *ptr;
FontScalablePtr vals;
int which;
{
    double *matrix;

    if (which == PIXELSIZE_MASK)
	matrix = vals->pixel_matrix;
    else if (which == POINTSIZE_MASK)
	matrix = vals->point_matrix;
    else return (char *)0;

    while (isspace(*ptr)) ptr++;
    if (*ptr == '[')
    {
	/* This is a matrix containing real numbers.  It would be nice
	   to use strtod() or sscanf() to read the numbers, but those
	   don't handle '~' for minus and we cannot force them to use a
	   "."  for the radix.  We'll have to do the hard work ourselves
	   (in readreal()).  */

	if ((ptr = readreal(++ptr, matrix + 0)) &&
	    (ptr = readreal(ptr, matrix + 1)) &&
	    (ptr = readreal(ptr, matrix + 2)) &&
	    (ptr = readreal(ptr, matrix + 3)))
	{
	    while (isspace(*ptr)) ptr++;
	    if (*ptr != ']')
		ptr = (char *)0;
	    else
	    {
		ptr++;
		while (isspace(*ptr)) ptr++;
		if (*ptr == '-')
		{
		    if (which == POINTSIZE_MASK)
			vals->values_supplied |= POINTSIZE_ARRAY;
		    else
			vals->values_supplied |= PIXELSIZE_ARRAY;
		}
		else ptr = (char *)0;
	    }
	}
    }
    else
    {
	int value;
	if (ptr = GetInt(ptr, &value))
	{
	    vals->values_supplied &= ~which;
	    if (value > 0)
	    {
		matrix[3] = (double)value;
		if (which == POINTSIZE_MASK)
		{
		    matrix[3] /= 10.0;
		    vals->values_supplied |= POINTSIZE_SCALAR;
		}
		else
		    vals->values_supplied |= PIXELSIZE_SCALAR;
		/* If we're concocting the pixelsize array from a scalar,
		   we will need to normalize element 0 for the pixel shape.
		   This is done in FontFileCompleteXLFD(). */
		matrix[0] = matrix[3];
		matrix[1] = matrix[2] = 0.0;
	    }
	    else if (value < 0)
	    {
		if (which == POINTSIZE_MASK)
		    vals->values_supplied |= POINTSIZE_WILDCARD;
		else
		    vals->values_supplied |= PIXELSIZE_WILDCARD;
	    }
	}
    }
    return ptr;
}


static void append_ranges(fname, nranges, ranges)
char *fname;
int nranges;
fsRange *ranges;
{
    if (nranges)
    {
        int i;

        strcat(fname, "[");
        for (i = 0; i < nranges && strlen(fname) < 1010; i++)
        {
	    if (i) strcat(fname, " ");
	    sprintf(fname + strlen(fname), "%d",
		    minchar(ranges[i]));
	    if (ranges[i].min_char_low ==
	        ranges[i].max_char_low &&
	        ranges[i].min_char_high ==
	        ranges[i].max_char_high) continue;
	    sprintf(fname + strlen(fname), "_%d",
		    maxchar(ranges[i]));
        }
        strcat(fname, "]");
    }
}

Bool
FontParseXLFDName(fname, vals, subst)
    char       *fname;
    FontScalablePtr vals;
    int         subst;
{
    register char *ptr;
    register char *ptr1,
               *ptr2,
               *ptr3,
               *ptr4;
    register char *ptr5;
    FontScalableRec tmpvals;
    char        replaceChar = '0';
    char        tmpBuf[1024];
    int         spacingLen;
    int		l;
    char	*p;

    bzero(&tmpvals, sizeof(tmpvals));
    if (subst != FONT_XLFD_REPLACE_VALUE)
	*vals = tmpvals;

    if (!(*(ptr = fname) == '-' || *ptr++ == '*' && *ptr == '-') ||  /* fndry */
	    !(ptr = strchr(ptr + 1, '-')) ||	/* family_name */
	    !(ptr1 = ptr = strchr(ptr + 1, '-')) ||	/* weight_name */
	    !(ptr = strchr(ptr + 1, '-')) ||	/* slant */
	    !(ptr = strchr(ptr + 1, '-')) ||	/* setwidth_name */
	    !(ptr = strchr(ptr + 1, '-')) ||	/* add_style_name */
	    !(ptr = strchr(ptr + 1, '-')) ||	/* pixel_size */
	    !(ptr = GetMatrix(ptr + 1, &tmpvals, PIXELSIZE_MASK)) ||
	    !(ptr2 = ptr = GetMatrix(ptr + 1, &tmpvals, POINTSIZE_MASK)) ||
	    !(ptr = GetInt(ptr + 1, &tmpvals.x)) ||	/* resolution_x */
	    !(ptr3 = ptr = GetInt(ptr + 1, &tmpvals.y)) ||  /* resolution_y */
	    !(ptr4 = ptr = strchr(ptr + 1, '-')) ||	/* spacing */
	    !(ptr5 = ptr = GetInt(ptr + 1, &tmpvals.width)) || /* average_width */
	    !(ptr = strchr(ptr + 1, '-')) ||	/* charset_registry */
	    strchr(ptr + 1, '-'))/* charset_encoding */
	return FALSE;

    /* Lop off HP charset subsetting enhancement.  Interpreting this
       field requires allocating some space in which to return the
       results.  So, to prevent memory leaks, this procedure will simply
       lop off and ignore charset subsetting, and initialize the
       relevant vals fields to zero.  It's up to the caller to make its
       own call to FontParseRanges() if it's interested in the charset
       subsetting.  */

    if (subst != FONT_XLFD_REPLACE_NONE &&
	(p = strchr(strrchr(fname, '-'), '[')))
    {
	tmpvals.values_supplied |= CHARSUBSET_SPECIFIED;
	*p = '\0';
    }

    /* Fill in deprecated fields for the benefit of rasterizers that care
       about them. */
    tmpvals.pixel = (tmpvals.pixel_matrix[3] >= 0) ?
		    (int)(tmpvals.pixel_matrix[3] + .5) :
		    (int)(tmpvals.pixel_matrix[3] - .5);
    tmpvals.point = (tmpvals.point_matrix[3] >= 0) ?
                    (int)(tmpvals.point_matrix[3] * 10 + .5) :
                    (int)(tmpvals.point_matrix[3] * 10 - .5);

    spacingLen = ptr4 - ptr3 + 1;

    switch (subst) {
    case FONT_XLFD_REPLACE_NONE:
	*vals = tmpvals;
	break;
    case FONT_XLFD_REPLACE_STAR:
	replaceChar = '*';
    case FONT_XLFD_REPLACE_ZERO:
	strcpy(tmpBuf, ptr2);
	ptr5 = tmpBuf + (ptr5 - ptr2);
	ptr3 = tmpBuf + (ptr3 - ptr2);
	ptr2 = tmpBuf;
	ptr = ptr1 + 1;

	ptr = strchr(ptr, '-') + 1;		/* skip weight */
	ptr = strchr(ptr, '-') + 1;		/* skip slant */
	ptr = strchr(ptr, '-') + 1;		/* skip setwidth_name */
	ptr = strchr(ptr, '-') + 1;		/* skip add_style_name */

	if ((ptr - fname) + spacingLen + strlen(ptr5) + 10 >= (unsigned)1024)
	    return FALSE;
	*ptr++ = replaceChar;
	*ptr++ = '-';
	*ptr++ = replaceChar;
	*ptr++ = '-';
	*ptr++ = '*';
	*ptr++ = '-';
	*ptr++ = '*';
	if (spacingLen > 2)
	{
	    memmove(ptr, ptr3, spacingLen);
	    ptr += spacingLen;
	}
	else
	{
	    *ptr++ = '-';
	    *ptr++ = '*';
	    *ptr++ = '-';
	}
	*ptr++ = replaceChar;
	strcpy(ptr, ptr5);
	*vals = tmpvals;
	break;
    case FONT_XLFD_REPLACE_VALUE:
	if (vals->values_supplied & PIXELSIZE_MASK)
	{
	    tmpvals.values_supplied =
		(tmpvals.values_supplied & ~PIXELSIZE_MASK) |
		(vals->values_supplied & PIXELSIZE_MASK);
	    tmpvals.pixel_matrix[0] = vals->pixel_matrix[0];
	    tmpvals.pixel_matrix[1] = vals->pixel_matrix[1];
	    tmpvals.pixel_matrix[2] = vals->pixel_matrix[2];
	    tmpvals.pixel_matrix[3] = vals->pixel_matrix[3];
	}
	if (vals->values_supplied & POINTSIZE_MASK)
	{
	    tmpvals.values_supplied =
		(tmpvals.values_supplied & ~POINTSIZE_MASK) |
		(vals->values_supplied & POINTSIZE_MASK);
	    tmpvals.point_matrix[0] = vals->point_matrix[0];
	    tmpvals.point_matrix[1] = vals->point_matrix[1];
	    tmpvals.point_matrix[2] = vals->point_matrix[2];
	    tmpvals.point_matrix[3] = vals->point_matrix[3];
	}
	if (vals->x >= 0)
	    tmpvals.x = vals->x;
	if (vals->y >= 0)
	    tmpvals.y = vals->y;
	if (vals->width >= 0)
	    tmpvals.width = vals->width;
	else if (vals->width < -1)	/* overload: -1 means wildcard */
	    tmpvals.width = -vals->width;


	p = ptr1 + 1;				/* weight field */
	l = strchr(p, '-') - p;
	sprintf(tmpBuf, "%*.*s", l, l, p);

	p += l + 1;				/* slant field */
	l = strchr(p, '-') - p;
	sprintf(tmpBuf + strlen(tmpBuf), "-%*.*s", l, l, p);

	p += l + 1;				/* setwidth_name */
	l = strchr(p, '-') - p;
	sprintf(tmpBuf + strlen(tmpBuf), "-%*.*s", l, l, p);

	p += l + 1;				/* add_style_name field */
	l = strchr(p, '-') - p;
	sprintf(tmpBuf + strlen(tmpBuf), "-%*.*s", l, l, p);

	strcat(tmpBuf, "-");
	if ((tmpvals.values_supplied & PIXELSIZE_MASK) == PIXELSIZE_ARRAY)
	{
	    char buffer[80];
	    strcat(tmpBuf, "[");
	    strcat(tmpBuf, xlfd_double_to_text(tmpvals.pixel_matrix[0],
		   buffer, 0));
	    strcat(tmpBuf, xlfd_double_to_text(tmpvals.pixel_matrix[1],
		   buffer, 1));
	    strcat(tmpBuf, xlfd_double_to_text(tmpvals.pixel_matrix[2],
		   buffer, 1));
	    strcat(tmpBuf, xlfd_double_to_text(tmpvals.pixel_matrix[3],
		   buffer, 1));
	    strcat(tmpBuf, "]");
	}
	else
	{
	    sprintf(tmpBuf + strlen(tmpBuf), "%d",
		    (int)(tmpvals.pixel_matrix[3] + .5));
	}
	strcat(tmpBuf, "-");
	if ((tmpvals.values_supplied & POINTSIZE_MASK) == POINTSIZE_ARRAY)
	{
	    char buffer[80];
	    strcat(tmpBuf, "[");
	    strcat(tmpBuf, xlfd_double_to_text(tmpvals.point_matrix[0],
		   buffer, 0));
	    strcat(tmpBuf, xlfd_double_to_text(tmpvals.point_matrix[1],
		   buffer, 1));
	    strcat(tmpBuf, xlfd_double_to_text(tmpvals.point_matrix[2],
		   buffer, 1));
	    strcat(tmpBuf, xlfd_double_to_text(tmpvals.point_matrix[3],
		   buffer, 1));
	    strcat(tmpBuf, "]");
	}
	else
	{
	    sprintf(tmpBuf + strlen(tmpBuf), "%d",
		    (int)(tmpvals.point_matrix[3] * 10.0 + .5));
	}
	sprintf(tmpBuf + strlen(tmpBuf), "-%d-%d%*.*s%d%s",
		tmpvals.x, tmpvals.y,
		spacingLen, spacingLen, ptr3, tmpvals.width, ptr5);
	strcpy(ptr1 + 1, tmpBuf);
	if ((vals->values_supplied & CHARSUBSET_SPECIFIED) && !vals->nranges)
	    strcat(fname, "[]");
	else
	    append_ranges(fname, vals->nranges, vals->ranges);
	break;
    }
    return TRUE;
}

fsRange *FontParseRanges(name, nranges)
char *name;
int *nranges;
{
    int n;
    unsigned long l;
    char *p1, *p2;
    fsRange *result = (fsRange *)0;

    name = strchr(name, '-');
    for (n = 1; name && n < 14; n++)
	name = strchr(name + 1, '-');

    *nranges = 0;
    if (!name || !(p1 = strchr(name, '['))) return (fsRange *)0;
    p1++;

    while (*p1 && *p1 != ']')
    {
	fsRange thisrange;

	l = strtol(p1, &p2, 0);
	if (p2 == p1 || l > 0xffff) break;
	thisrange.max_char_low = thisrange.min_char_low = l & 0xff;
	thisrange.max_char_high = thisrange.min_char_high = l >> 8;

	p1 = p2;
	if (*p1 == ']' || *p1 == ' ')
	{
	    while (*p1 == ' ') p1++;
	    if (add_range(&thisrange, nranges, &result, TRUE) != Successful)
		break;
	}
	else if (*p1 == '_')
	{
	    l = strtol(++p1, &p2, 0);
	    if (p2 == p1 || l > 0xffff) break;
	    thisrange.max_char_low = l & 0xff;
	    thisrange.max_char_high = l >> 8;
	    p1 = p2;
	    if (*p1 == ']' || *p1 == ' ')
	    {
		while (*p1 == ' ') p1++;
		if (add_range(&thisrange, nranges, &result, TRUE) != Successful)
		    break;
	    }
	}
	else break;
    }

    return result;
}
