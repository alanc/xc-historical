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

#include	"stdio.h"
#include	"string.h"

char	*strtok();

#include	"mc.h"

extern	struct	state	State;
extern	int 	hflag;
extern	int 	sflag;

#define	F_BANNER	"mabanner.tmc"
#define	F_STDHEADER	"maheader.mc"
#define	F_HEADER	"maheader.tmc"
#define	F_TEXT		"matext.tmc"

static	FILE	*FpBanner;
static	FILE	*FpHeader;
static	FILE	*FpText;

void
mascopyright(fp, buf)
FILE	*fp;
char	*buf;
{
static	int 	firsttime = 1;

	while (newline(fp, buf) != NULL && !SECSTART(buf)) {
	int 	off = 3;

		if (strncmp(buf, " */", 3) == 0)
			strcpy(buf, " * \n");
		if (firsttime || strinstr(buf, "SCCS")) {
			fputs(">># ", FpBanner);
			if (buf[off] == '\0')
				off--;
			fputs(buf+off, FpBanner);
		}
	}
	firsttime = 0;
}

void
masheader(fp, buf)
FILE	*fp;
char	*buf;
{
	(void) fprintf(FpText, ">>TITLE %s %s\n", State.name, State.chap);
	skip(fp, buf);
}

void
masassertion(fp, buf)
FILE	*fp;
char	*buf;
{
	(void) fprintf(FpText, ">>ASSERTION ");
	if (State.category != CAT_NONE)
		(void) fprintf(FpText, "%c ", (char)State.category);
	(void) fprintf(FpText, "%s-%d\n",
		State.name, State.assertion);
	assertfill(fp, buf, FpText, "");
	(void) fprintf(FpText, "\n");
}

void
masstrategy(fp, buf)
FILE	*fp;
char	*buf;
{
	(void) fprintf(FpText, ">>STRATEGY\n");
	echon(fp, buf, FpText);
}

void
masdefassertion(fp, buf)
FILE	*fp;
char	*buf;
{
	(void) fprintf(FpText, ">>ASSERTION def %s-%d\n",
		State.name, State.assertion);
	echon(fp, buf, FpText);
}

/* Hooks */
/*ARGSUSED*/
void
masstart(buf)
char	*buf;
{
	FpBanner = cretmpfile(F_BANNER);
	FpText = cretmpfile(F_TEXT);
}

/*ARGSUSED*/
void
masend(buf)
char	*buf;
{
	fputs(">>#\n", FpBanner);
	outfile(FpBanner);
	outfile(FpText);
}

