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
 * $XConsortium$
 *
 * Author: Steve Ratcliffe, UniSoft Ltd.
 */


#include	<stdio.h>
#include	<string.h>

#include	"mc.h"

static	struct	gclookup {
	char	*gccompname;
	char	*include;
} gclookup[] = {
	/*
	 * Note:  If there are gccompname's that are initial strings of others
	 * then they must be afterwards.
	 */
	{"function", "function"},
	{"plane-mask", "plane-mask"},
	{"foreground", "foreground"},
	{"background", "background"},
	{"line-width", "line-width"},
	{"line-style", "line-style"},
	{"cap-style", "cap-style"},
	{"join-style", "join-style"},
	{"fill-style", "fill-style"},
	{"fill-rule", "fill-rule"},
	{"arc-mode", "arc-mode"},
	{"tile-stipple-x-origin", "ts-x-origin"},
	{"tile-stipple-y-origin", "ts-y-origin"},
	{"ts-x-origin", "ts-x-origin"},
	{"ts-y-origin", "ts-y-origin"},
	{"tile", "tile"},
	{"stipple", "stipple"},
	{"font", "font"},
	{"subwindow-mode", "subwindow-mode"},
	{"graphics-exposures", "graphics-exposures"},
	{"clip-x-origin", "clip-x-origin"},
	{"clip-y-origin", "clip-y-origin"},
	{"clip-mask", "clip-mask"},
	{"dash-offset", "dash-offset"},
	{"dash-list", "dash-list"},
	{"dashes", "dash-list"},
};

#define	NGCCOMP	(sizeof(gclookup)/sizeof(struct gclookup))

gccomps(fp, buf)
FILE	*fp;
char	*buf;
{
struct	gclookup	*lp;
char	*cp;

	while (newline(fp, buf) != NULL && !SECSTART(buf)) {
		if (strncmp(buf, ".M", 2) != 0)
			continue;

		cp = buf+3;
		while (*cp && *cp == ' ' || *cp == '\t')
			cp++;

		for (lp = gclookup; lp < gclookup+NGCCOMP; lp++) {
			if (strncmp(cp, lp->gccompname, strlen(lp->gccompname)) == 0)
				break;
		}

		if (lp == gclookup+NGCCOMP) {
			err("");
			(void) fprintf(stderr, "Unrecognised gc component name %s\n", cp);
			errexit();
		}

		(void) sprintf(buf, "gc/%.9s.mc\n", lp->include);
		includefile(buf, buf);
	}
}
