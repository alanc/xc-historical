/*
 * xdm - display manager daemon
 *
 * $XConsortium: file.c,v 1.9 89/08/31 11:34:56 keith Exp $
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
# include	<ctype.h>
# include	<signal.h>

extern void	free ();
extern char	*malloc (), *realloc ();

DisplayTypeMatch (d1, d2)
DisplayType	d1, d2;
{
	return d1.location == d2.location &&
	       d1.lifetime == d2.lifetime &&
	       d1.origin == d2.origin;
}

static void
freeArgs (args)
    char    **args;
{
    char    **a;

    for (a = args; *a; a++)
	free (*a);
    free ((char *) args);
}

static char **
splitIntoWords (s)
    char    *s;
{
    char    **args, **newargs;
    char    *wordStart;
    int	    nargs;

    args = 0;
    nargs = 0;
    while (*s)
    {
	while (*s && isspace (*s))
	    ++s;
	if (!*s || *s == '#')
	    break;
	wordStart = s;
	while (*s && *s != '#' && !isspace (*s))
	    ++s;
	if (!args)
	{
    	    args = (char **) malloc (2 * sizeof (char *));
    	    if (!args)
	    	return NULL;
	}
	else
	{
	    newargs = (char **) realloc ((char *) args,
					 (nargs+2)*sizeof (char *));
	    if (!newargs)
	    {
	    	freeArgs (args);
	    	return NULL;
	    }
	    args = newargs;
	}
	args[nargs] = malloc (s - wordStart);
	if (!args[nargs])
	{
	    freeArgs (args);
	    return NULL;
	}
	strncpy (args[nargs], wordStart, s - wordStart);
	args[nargs][s-wordStart] = '\0';
	++nargs;
	args[nargs] = NULL;
    }
    return args;
}

static char **
copyArgs (args)
    char    **args;
{
    char    **a, **new, **n;
    int	    i;

    for (a = args; *a; a++)
	;
    new = (char **) malloc ((a - args + 1) * sizeof (char *));
    if (!new)
	return NULL;
    n = new;
    a = args;
    while (*n++ = *a++)
	;
    return new;
}

freeSomeArgs (args, n)
    char    **args;
    int	    n;
{
    char    **a;

    a = args;
    while (n--)
	free (*a++);
    free ((char *) args);
}

ParseDisplay (source, acceptableTypes, numAcceptable)
char		*source;
DisplayType	*acceptableTypes;
int		numAcceptable;
{
    char		**args, **a;
    struct display	*d;
    DisplayType	type;

    args = splitIntoWords (source);
    if (!args)
	return;
    if (!args[0])
    {
	LogError ("Missing display name in servers file\n");
	freeArgs (args);
	return;
    }
    if (!args[1])
    {
	LogError ("Missing display type for %s\n", args[0]);
	freeArgs (args);
	return;
    }
    d = FindDisplayByName (args[0]);
    type = parseDisplayType (args[1]);
    while (numAcceptable)
    {
	if (DisplayTypeMatch (*acceptableTypes, type))
	    break;
	--numAcceptable;
	++acceptableTypes;
    }
    if (!numAcceptable)
    {
	LogError ("Unacceptable display type %s for display %s\n",
		  args[1], args[0]);
    }
    if (d)
    {
	d->state = OldEntry;
	Debug ("Found existing display:  %s %s", d->name, args[1]);
	freeArgs (d->argv);
    }
    else
    {
	d = NewDisplay (args[0], (char *) 0);
	Debug ("Found new display:  %s %s", d->name, args[1]);
    }
    d->displayType = type;
    d->argv = copyArgs (args+2);
    for (a = d->argv; a && *a; a++)
	Debug (" %s", *a);
    Debug ("\n");
    freeSomeArgs (args, 2);
}

static struct displayMatch {
	char		*name;
	DisplayType	type;
} displayTypes[] = {
	"local",		{ Local, Permanent, FromFile },
	"foreign",		{ Foreign, Permanent, FromFile },
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
