/*
 * xdm - display manager daemon
 *
 * $XConsortium: file.c,v 1.8 89/01/16 17:11:07 keith Exp $
 *
 * Copyright 1988 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

/*
 * file.c
 */

# include	"dm.h"
# include	"buf.h"
# include	<signal.h>

extern void	free (), bcopy ();

DisplayTypeMatch (d1, d2)
DisplayType	d1, d2;
{
	return d1.location == d2.location &&
	       d1.lifetime == d2.lifetime &&
	       d1.origin == d2.origin;
}

ReadDisplay (file, acceptableTypes, numAcceptable)
struct buffer	*file;
DisplayType	*acceptableTypes;
int		numAcceptable;
{
	int		c;
	char		**args, **newargs;
	struct display	*d;
	DisplayType	type;
	char		word[1024];
	char		typeText[1024];
	int		i;

	c = readWord (file, word, sizeof (word));
	if (word[0] != '\0') {
		if (c == EOB || c == '\n') {
			LogError ("missing display type for display %s", word);
			return c;
		}
		if (d = FindDisplayByName (word)) {
			d->state = OldEntry;
		} else {
			c = readWord (file, typeText, sizeof (typeText));
			Debug ("read display %s type %s\n", word, typeText);
			type = parseDisplayType (typeText);
			while (numAcceptable--)
				if (DisplayTypeMatch (*acceptableTypes++, type))
					goto acceptable;
			LogError ("unacceptable display type %s for display %s\n", typeText, word);
		}
		while (c != EOB && c != '\n')
			c = bufc (file);
		return c;
acceptable:;
		d = NewDisplay (word, (char *) 0);
		if (!d) {
			LogOutOfMem ("ReadDisplay");
			goto skipLine;
		}
		Debug ("new display %s\n", d->name);
		d->displayType = type;
		d->state = NewEntry;
#ifdef UDP_SOCKET
		if (sockaddr)
			d->addr = *(struct sockaddr_in *) sockaddr;
		else
			bzero (d->addr, sizeof (d->addr));
#endif
		i = 0;
		args = (char **) malloc (sizeof (char *));
		if (!args) {
			LogOutOfMem ("ReadDisplay");
			d->argv = 0;
			goto skipLine;
		}
		while (c != EOB && c != '\n') {
			c = readWord (file, word, sizeof (word));
			if (word[0] != '\0') {
				args[i] = malloc ((unsigned) (strlen (word) + 1));
				if (!args[i]) {
					LogOutOfMem ("ReadDisplay");
					break;
				}
				strcpy (args[i], word);
				newargs = (char **) 
				    malloc ((unsigned) ((i+2) * sizeof (char **)));
				if (!newargs) {
					LogOutOfMem ("ReadDisplay");
					break;
				}
				bcopy ((char *) args, (char *) newargs,
				       (i+1) * sizeof (char **));
				free ((char *) args);
				args = newargs;
				i++;
			}
		}
		args[i] = 0;
		d->argv = args;
skipLine:	;
		while (c != EOB && c != '\n')
			c = readWord (file, word, sizeof (word));
	}
	return c;
}

# define isbreak(c)	((c) == ' ' || (c) == '\t' || (c) == '\n')

int
readWord (file, word, len)
struct buffer	*file;
char	*word;
int	len;
{
	int	c;
	int	i;

	while ((c = bufc (file)) != EOB)
		if (!isbreak (c))
			break;
	if (c == EOB) {
		word[0] = '\0';
		return EOB;
	}
	word[0] = c;
	i = 1;
	while ((c = bufc (file)) != EOB && !isbreak (c)) {
		if (i < len-1 && c != '\r') {
			word[i] = c;
			++i;
		}
	}
	word[i] = '\0';
	return c;
}

static struct displayMatch {
	char		*name;
	DisplayType	type;
} displayTypes[] = {
	"local",		{ Local, Permanent, FromFile },
	"foreign",		{ Foreign, Permanent, FromFile },
	"transient",		{ Foreign, Transient, FromFile },
	"localTransient",	{ Local,  Transient, FromFile },
	0,			{ Local, Permanent, FromFile },
};

DisplayType
parseDisplayType (string)
	char	*string;
{
	struct displayMatch	*d;

	for (d = displayTypes; d->name; d++)
		if (!strcmp (d->name, string))
			return d->type;
	return d->type;
}
