/*
 * parse.c
 *
 * parse dvi input
 */

#include <X11/Xos.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <stdio.h>
#include <ctype.h>
#include "DviP.h"

static int StopSeen = 0;

#define HorizontalMove(dw, delta)	((dw)->dvi.state->x += (delta))

#define charWidth(fi,c)	((fi)->per_char[(c) - (fi)->min_char_or_byte2].width)

ParseInput(dw)
register DviWidget	dw;
{
	int		n, k;
	register int	c;
	char		Buffer[BUFSIZ];
	register FILE	*fp = dw->dvi.file;
	int		NextPage;
	int		prevFont;

	StopSeen = 0;

	for (;;) {
		switch (c = getc (fp)) {
		case '\n':	
			break;
		case ' ':	/* when input is text */
		case 0:		/* occasional noise creeps in */
			break;
		case '{':	/* push down current environment */
			push_env(dw);
			break;
		case '}':
			pop_env(dw);
			break;
		/*
		 * two motion digits plus a character
		 */
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			HorizontalMove(dw, (c-'0')*10 + getc(fp)-'0');
			/* fall through */
		case 'c':	/* single ascii character */
			c = getc (fp);
			/*
			 * ugly as sin, but a big win when
			 * inlined...
			 */
			prevFont = -1;
	PutCharacter:	;
			/*
			 * quick and dirty extents calculation:
			 */
			if (dw->dvi.state->y + 24 >= dw->dvi.extents.y1 &&
			    dw->dvi.state->y - 24 <= dw->dvi.extents.y2 &&
			    dw->dvi.state->x + 24 >= dw->dvi.extents.x1 &&
			    dw->dvi.state->x - 24 <= dw->dvi.extents.x2)
			{
	    	    	    register XFontStruct	*font;
			    register XTextItem		*text;
    	    	    
	    	    	    if (!dw->dvi.display_enable)
		    	    	    break;
    	    	    
		    	    if (c == ' ')
				break;

	    	    	    if (dw->dvi.cache.char_index == 0 ||
	    	    	    	dw->dvi.state->y != dw->dvi.cache.y ||
	    	    	    	dw->dvi.cache.char_index >= DVI_CHAR_CACHE_SIZE ||
				dw->dvi.cache.index >= DVI_TEXT_CACHE_SIZE - 1)
	    	    	    {
		    	    	    if (dw->dvi.cache.char_index != 0)
					FlushCharCache (dw);
		    	    	    dw->dvi.cache.start_x = dw->dvi.cache.x = dw->dvi.state->x;
		    	    	    dw->dvi.cache.start_y = dw->dvi.cache.y = dw->dvi.state->y;
			    }
			    if (dw->dvi.cache.font_size != dw->dvi.state->font_size ||
		    	       	dw->dvi.cache.font_number != dw->dvi.state->font_number)
		    	    {
			        dw->dvi.cache.font_size = dw->dvi.state->font_size;
			        dw->dvi.cache.font_number = dw->dvi.state->font_number;
			        dw->dvi.cache.font = QueryFont (dw,
						      dw->dvi.cache.font_number,
						      dw->dvi.cache.font_size);
				if (dw->dvi.cache.cache[dw->dvi.cache.index].nchars != 0) {
				    ++dw->dvi.cache.index;
				    dw->dvi.cache.cache[dw->dvi.cache.index].nchars = 0;
				}
	    	    	    }
			    if (dw->dvi.cache.x != dw->dvi.state->x) {
				if (dw->dvi.cache.cache[dw->dvi.cache.index].nchars != 0) {
				    ++dw->dvi.cache.index;
				    dw->dvi.cache.cache[dw->dvi.cache.index].nchars = 0;
				}
			    }
	    	    	    font = dw->dvi.cache.font;
			    text = &dw->dvi.cache.cache[dw->dvi.cache.index];
			    if (text->nchars == 0) {
				text->chars = &dw->dvi.cache.char_cache[dw->dvi.cache.char_index];
				text->delta = dw->dvi.state->x - dw->dvi.cache.x;
				if (dw->dvi.cache.index == 0 ||
 				    font->fid != dw->dvi.cache.cache[dw->dvi.cache.index-1].font)
				{
				    text->font = font->fid;
				} else
				    text->font = None;
				dw->dvi.cache.x += text->delta;
			    }
			    dw->dvi.cache.char_cache[dw->dvi.cache.char_index++] = (char) c;
			    ++text->nchars;
			    dw->dvi.cache.x += charWidth(font,c);
			}
			if (prevFont != -1)
			    dw->dvi.state->font_number = prevFont;
			break;
		case 'C':
			GetWord(fp, Buffer, BUFSIZ);
			{
	    	    	    DviCharNameMap	*map;
			    int			i;
    	    	    
	    	    	    map = QueryFontMap (dw, dw->dvi.state->font_number);
	    	    	    if (map) {
		    	    	    c = DviCharIndex (map, Buffer);
		    	    	    if (c == -1) {
					for (i = 1; map = QueryFontMap (dw, i); i++)
					    if (map->special)
						if ((c = DviCharIndex (map, Buffer)) != -1) {
						    prevFont = dw->dvi.state->font_number;
						    dw->dvi.state->font_number = i;
						    break;
						}
				    }
				    if (c != -1)
					goto PutCharacter;
	    	    	    }
			}
			break;
		case 'D':	/* draw function */
			GetLine(fp, Buffer, BUFSIZ);
			ParseDrawFunction(dw, Buffer);
			break;
		case 's':	/* ignore fractional sizes */
			n = GetNumber(fp);
			dw->dvi.state->font_size = n;
			break;
		case 'f':
			n = GetNumber(fp);
			dw->dvi.state->font_number = n;
			break;
		case 'H':	/* absolute horizontal motion */
			k = GetNumber(fp);
			HorizontalGoto(dw, k);
			break;
		case 'h':	/* relative horizontal motion */
			k = GetNumber(fp);
			HorizontalMove(dw, k);
			break;
		case 'w':	/* word space */
			break;
		case 'V':
			n = GetNumber(fp);
			VerticalGoto(dw, n);
			break;
		case 'v':
			n = GetNumber(fp);
			VerticalMove(dw, n);
			break;
		case 'P':	/* new spread */
			break;
		case 'p':	/* new page */
			(void) GetNumber(fp);
			NextPage = dw->dvi.current_page + 1;
			RememberPagePosition(dw, NextPage);
			return(NextPage);
		case 'n':	/* end of line */
			GetNumber(fp);
			GetNumber(fp);
			HorizontalGoto(dw, 0);
			break;
		case '#':	/* comment */
			GetLine(fp, NULL, 0);
			break;
		case 'x':	/* device control */
			ParseDeviceControl(dw);
			break;
		case EOF:
			dw->dvi.last_page = dw->dvi.current_page;
			return dw->dvi.current_page;
		default:
			break;
		}
	}
}

static
push_env(dw)
	DviWidget	dw;
{
	DviState	*new;

	new = (DviState *) malloc (sizeof (*new));
	if (dw->dvi.state)
		*new = *(dw->dvi.state);
	else {
		new->font_size = 10;
		new->font_number = 1;
		new->line_style = 0;
		new->line_width = 1;
		new->x = 0;
		new->y = 0;
	}
	new->next = dw->dvi.state;
	dw->dvi.state = new;
}

static
pop_env(dw)
	DviWidget	dw;
{
	DviState	*old;

	old = dw->dvi.state;
	dw->dvi.state = old->next;
	free ((char *) old);
}

static
InitTypesetter (dw)
	DviWidget	dw;
{
	while (dw->dvi.state)
		pop_env (dw);
	push_env (dw);
}

static
ParseDrawFunction(dw, buf)
	DviWidget	dw;
	char		*buf;
{
	int	n, m, n1, m1;
	FILE	*fp = dw->dvi.file;

	switch (buf[0]) {
	case 'l':				/* draw a line */
		sscanf(buf+1, "%d %d", &n, &m);
		DrawLine(dw, n, m);
		break;
	case 'c':				/* circle */
		sscanf(buf+1, "%d", &n);
		DrawCircle(dw, n);
		break;
	case 'e':				/* ellipse */
		sscanf(buf+1, "%d %d", &m, &n);
		DrawEllipse(dw, m, n);
		break;
	case 'a':				/* arc */
		sscanf(buf+1, "%d %d %d %d", &n, &m, &n1, &m1);
		DrawArc(dw, n, m, n1, m1);
		break;
	case '~':				/* wiggly line */
		DrawSpline(dw, buf+1,1);
		break;
	default:
/*		warning("unknown drawing function %s", buf); */
		break;
	}
} 

static
ParseDeviceControl(dw)				/* Parse the x commands */
	DviWidget	dw;
{
        char str[20], str1[50], buf[50];
	int c, n;
	extern int LastPage, CurrentPage;
	FILE	*fp = dw->dvi.file;

	GetWord (fp, str, 20);
	switch (str[0]) {			/* crude for now */
	case 'T':				/* output device */
		GetWord(fp, str, 20);
		break;
	case 'i':				/* initialize */
		InitTypesetter(dw);
		break;
	case 't':				/* trailer */
		break;
	case 'p':				/* pause -- can restart */
		break;
	case 's':				/* stop */
		StopSeen = 1;
		return;
	case 'r':				/* resolution when prepared */
		(void) GetNumber(fp);
		break;
	case 'f':				/* font used */
		n = GetNumber(fp);
		GetWord(fp, str, 20);
		GetLine(fp, str1, 50);
		SetFontPosition(dw, n, str, str1);
		break;
	case 'H':				/* char height */
		break;
	case 'S':				/* slant */
		break;
	}
	while ((c = getc(fp)) != '\n')		/* skip rest of input line */
		if (c == EOF)
			return;
	return;
}
