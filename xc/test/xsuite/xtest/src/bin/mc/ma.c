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
macopyright(fp, buf)
FILE	*fp;
char	*buf;
{
static	int 	firsttime = 1;

	while (newline(fp, buf) != NULL && !SECSTART(buf)) {
	int 	off = 3;

		if (strncmp(buf, " */", 3) == 0)
			strcpy(buf, " * \n");
		if (firsttime || strinstr(buf, "SCCS")) {
			fputs("'\\\" ", FpBanner);
			if (buf[off] == '\0')
				off--;
			fputs(buf+off, FpBanner);
		}
	}
	firsttime = 0;
}

void
maheader(fp, buf)
FILE	*fp;
char	*buf;
{
	(void) fprintf(FpText, ".TH %s %s\n", State.name, State.chap);
	skip(fp, buf);
}

void
maassertion(fp, buf)
FILE	*fp;
char	*buf;
{
	(void) fprintf(FpText, ".TI ");
	if (State.category != CAT_NONE)
		(void) fprintf(FpText, "%c ", (char)State.category);
	(void) fprintf(FpText, "\\\" %s-%d\n",
		State.name, State.assertion);
	echon(fp, buf, FpText);
}

void
madefassertion(fp, buf)
FILE	*fp;
char	*buf;
{
	(void) fprintf(FpText, ".TI def \\\" %s-%d\n", State.name, State.assertion);
	echon(fp, buf, FpText);
}

/* Hooks */
/*ARGSUSED*/
void
mastart(buf)
char	*buf;
{
	FpBanner = cretmpfile(F_BANNER);
	FpText = cretmpfile(F_TEXT);
}

/*ARGSUSED*/
void
maend(buf)
char	*buf;
{
	fputs("'\\\"\n", FpBanner);
	outfile(FpBanner);
	if (hflag) {
		if (sflag) {
			FpHeader = cretmpfile(F_HEADER);
			(void) fprintf(FpHeader, ".so head.t\n");
			outfile(FpHeader);
		} else {
			outcopy(F_STDHEADER);
		}
	}
	outfile(FpText);
}

void
macomment(buf)
char	*buf;
{
extern	int 	pflag;

	if (pflag) {
		fputs(".NS\n", FpText);
		fputs(buf, FpText);
		fputs(".NE\n", FpText);
	}
}
