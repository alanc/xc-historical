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
#include	"mc.h"

#define	F_BANNER	"mepbanner.tmc"
#define	F_EXPAND	"mepexpand.tmc"

static	FILE	*FpBanner;
static	FILE	*FpExp;

void
mepcopyright(fp, buf)
FILE	*fp;
char	*buf;
{
static int 	firsttime = 1;

	while (newline(fp, buf) != NULL && !SECSTART(buf)) {
		if (strncmp(buf, " */", 3) == 0)
			strcpy(buf, " * \n");
		if (firsttime || strinstr(buf, "SCCS"))
			fputs(buf, FpBanner);
	}
	firsttime = 0;
}

void
mepecho(fp, buf)
FILE	*fp;
char	*buf;
{
	echo(fp, buf, FpExp);
}

/* Hooks */
/*ARGSUSED*/
void
mepstart(buf)
char	*buf;
{
	FpExp = cretmpfile(F_EXPAND);
	FpBanner = cretmpfile(F_BANNER);
}

/*ARGSUSED*/
void
mepend(buf)
char	*buf;
{
	(void) fprintf(FpBanner, " */\n");
	outfile(FpBanner);
	outfile(FpExp);
	fclose(FpExp);
}

void
mepset(buf)
char	*buf;
{
	fprintf(FpExp, ">>SET %s", buf);
}

void
mepcomment(buf)
char	*buf;
{
	fprintf(FpExp, "%s", buf);
}
