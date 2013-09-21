/* $XConsortium: cards.c,v 1.4 95/06/09 20:57:00 gildea Exp $ */
/* $XFree86: xc/programs/Xserver/hw/xfree86/xf86config/cards.c,v 3.2 1995/01/23 01:33:07 dawes Exp $ */

/*
 *  Functions to manipulate card database.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cards.h"

/*
 * Database format:
 *
 * NAME <name of card>
 * CHIPSET <chipset description>
 * SERVER <server name>	
 *
 * Optional keywords:
 * RAMDAC <ramdac identifier>
 * CLOCKCHIP <clockchip identifier>
 * DACSPEED <dacspeed>
 * NOCLOCKPROBE
 * UNSUPPORTED
 *
 * SEE <name of card> refers to another card definition; parameters that
 * are already defined are not overridden.
 *
 * <server name> is one of Mono, VGA16, SVGA, S3, Mach32, Mach8, 8514,
 * P9000, AGX, W32.
 *
 * A useful additional keywords may be CLOCKS.
 */



/* Database vars. */

int lastcard;

Card card[MAX_CARDS];

void sort_database();


static int getline(f, l)
FILE *f;
char *l;
{
	if (fgets(l, 128, f) == NULL)
		return -1;
	return 0;
}

static void appendstring(destp, src)
	char **destp;
	char *src;
{
	char *newstr;
	newstr = malloc(strlen(*destp) + strlen(src) + 1);
	strcpy(newstr, *destp);
	strcat(newstr, src);
	if (strlen(*destp) > 0)
		free(*destp);
	*destp = newstr;
}

int lookupcard( char *name ) {
	int i;
	for (i = 0; i <= lastcard; i++)
		if (strcmp(name, card[i].name) == 0)
			return i;
	return -1;
}

static char *s3_comment =
"# Use Option \"nolinear\" if the server doesn't start up correctly\n"
"# (this avoids the linear framebuffer probe). If that fails try\n"
"# option \"nomemaccess\".\n"
"#\n"
"# Use Option \"sw_cursor\" if the server completely locked up\n"
"# several times while you're moving the mouse.\n"
"#\n"
"# Refer to /usr/X11R6/lib/doc/README.S3, and the XF86_S3 man page.\n";

static char *cirrus_comment =
"# Use Option \"no_bitblt\" if you have graphics problems. If that fails\n"
"# try Option \"noaccel\".\n"
"# Refer to /usr/X11R6/lib/doc/README.cirrus.\n"
"# To allow linear addressing, uncomment the Option line and the\n"
"# address that the card maps the framebuffer to.\n";

int parse_database() {
	FILE *f;
	char buf[128];
	int i, lineno;
	f = fopen(CARD_DATABASE_FILE, "r");
	if (f == NULL)
		return -1;

	lastcard = -1;
	lineno = 0;

	for (;;) {
		if (getline(f, buf))
			break;
		lineno++;
		if (buf[0] == '#')
			/* Comment. */
			continue;
		if (strncmp(buf, "END", 3) == 0)
			/* End of database. */
			break;
		if (strncmp(buf, "LINE", 4) == 0 && lastcard>=0) {
			/* Line of Device comment. */
			char *lines;
			/* Append to existing lines. */
			appendstring(&card[lastcard].lines, buf + 5);
			continue;
		}
		/*
		 * The following keywords require the trailing newline
		 * to be deleted.
		 */
		i = strlen(buf);
		buf[--i] = '\0';

		/* remove trailing spaces or tabs */
		for(--i; i>=0 && (buf[i] == ' ' || buf[i] == '\011'); i--) ;
		if (i>=0)
		   buf[i+1] = '\0';
		else 
		   continue; /* skip empty lines */

		if (strncmp(buf, "NAME", 4) == 0) {
			/* New entry. */
			lastcard++;
			card[lastcard].name = malloc(strlen(buf + 5) + 1);
			strcpy(card[lastcard].name, buf + 5);
			card[lastcard].chipset = NULL;
			card[lastcard].ramdac = NULL;
			card[lastcard].clockchip = NULL;
			card[lastcard].dacspeed = NULL;
			card[lastcard].flags = 0;
			card[lastcard].lines = "";
			continue;
		}
		if (lastcard < 0)  /* no NAME line found yet */
		   continue; 
		if (strncmp(buf, "SEE", 3) == 0) {
			/* Reference to another entry. */
			int i;
			i = lookupcard(buf + 4);
			if (i == -1) {
				printf("Error in database, invalid reference: %s.\n",
					buf + 4);
				continue;
			}
			if (card[lastcard].chipset == NULL)
				card[lastcard].chipset = card[i].chipset;
			if (card[lastcard].server == NULL)
				card[lastcard].server = card[i].server;
			if (card[lastcard].ramdac == NULL)
				card[lastcard].ramdac = card[i].ramdac;
			if (card[lastcard].clockchip == NULL)
				card[lastcard].clockchip = card[i].clockchip;
			if (card[lastcard].dacspeed == NULL)
				card[lastcard].dacspeed = card[i].dacspeed;
			card[lastcard].flags |= card[i].flags;
			appendstring(&card[lastcard].lines, card[i].lines);
			continue;
		}
		if (strncmp(buf, "CHIPSET", 7) == 0) {
			/* Chipset description. */
			card[lastcard].chipset = malloc(strlen(buf + 8) + 1);
			strcpy(card[lastcard].chipset, buf + 8);
			continue;
		}
		if (strncmp(buf, "SERVER", 6) == 0) {
			/* Server identifier. */
			card[lastcard].server = malloc(strlen(buf + 7) + 1);
			strcpy(card[lastcard].server, buf + 7);
			continue;
		}
		if (strncmp(buf, "RAMDAC", 6) == 0) {
			/* Ramdac indentifier. */
			card[lastcard].ramdac = malloc(strlen(buf + 7) + 1);
			strcpy(card[lastcard].ramdac, buf + 7);
			continue;
		}
		if (strncmp(buf, "CLOCKCHIP", 9) == 0) {
			/* Clockchip indentifier. */
			card[lastcard].clockchip = malloc(strlen(buf + 10) + 1);
			strcpy(card[lastcard].clockchip, buf + 10);
			card[lastcard].flags |= NOCLOCKPROBE;
			continue;
		}
		if (strncmp(buf, "DACSPEED", 8) == 0) {
			/* Clockchip indentifier. */
			card[lastcard].dacspeed = malloc(strlen(buf + 9) + 1);
			strcpy(card[lastcard].dacspeed, buf + 9);
			continue;
		}
		if (strncmp(buf, "NOCLOCKPROBE", 12) == 0) {
			card[lastcard].flags |= NOCLOCKPROBE;
			continue;
		}
		if (strncmp(buf, "UNSUPPORTED", 12) == 0) {
			card[lastcard].flags |= UNSUPPORTED;
			continue;
		}
		/* test for missing required fields */
		if (card[lastcard].server == NULL) {
		    fprintf(stderr, "Warning SERVER specification missing "
			    "in Card database entry %s (line %d).\n", 
			    card[lastcard].name, lineno);
		    keypress();
		       card[lastcard].server = "unknown";
		}
		if (card[lastcard].chipset == NULL) {
		    fprintf(stderr, "Warning CHIPSET specification missing "
			    "in Card database entry %s (line %d).\n", 
			    card[lastcard].name, lineno);
		    keypress();
		    card[lastcard].chipset = "unknown";
		}
	    }

	fclose(f);

	/*
	 * Add general comments.
	 */
	for (i = 0; i <= lastcard; i++) {
		if (strcmp(card[i].server, "S3") == 0)
			appendstring(&card[i].lines, s3_comment);
		if (strncmp(card[i].chipset, "CL-GD", 5) == 0)
			appendstring(&card[i].lines, cirrus_comment);
	}

	sort_database();

	return 0;
}

#ifdef __STDC__
#define CONST const
#else
#define CONST
#endif

static int compare_card(e1, e2)
	CONST void *e1;
	CONST void *e2;
{
	return strcmp(((Card *)e1)->name, ((Card *)e2)->name);
}

void sort_database() {
	/* Each element is a bunch of words, but nothing too bad. */
	qsort(card, lastcard + 1, sizeof(Card), compare_card);
}
