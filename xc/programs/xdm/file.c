/* $XConsortium: file.c,v 1.15 91/02/13 19:13:21 rws Exp $ */
/*

Copyright (c) 1988  X Consortium

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
 * xdm - display manager daemon
 * Author:  Keith Packard, MIT X Consortium
 *
 * file.c
 */

# include	"dm.h"
# include	<ctype.h>

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
	args[nargs] = malloc (s - wordStart + 1);
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

    for (a = args; *a; a++)
	/* SUPPRESS 530 */
	;
    new = (char **) malloc ((a - args + 1) * sizeof (char *));
    if (!new)
	return NULL;
    n = new;
    a = args;
    /* SUPPRESS 560 */
    while (*n++ = *a++)
	/* SUPPRESS 530 */
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
    char		**args, **argv, **a;
    char		*name, *class, *type;
    struct display	*d;
    int			usedDefault;
    DisplayType		displayType;

    args = splitIntoWords (source);
    if (!args)
	return;
    if (!args[0])
    {
	LogError ("Missing display name in servers file\n");
	freeArgs (args);
	return;
    }
    name = args[0];
    if (!args[1])
    {
	LogError ("Missing display type for %s\n", args[0]);
	freeArgs (args);
	return;
    }
    displayType = parseDisplayType (args[1], &usedDefault);
    class = NULL;
    type = args[1];
    argv = args + 2;
    /*
     * extended syntax; if the second argument doesn't
     * exactly match a legal display type and the third
     * argument does, use the second argument as the
     * display class string
     */
    if (usedDefault && args[2])
    {
	displayType = parseDisplayType (args[2], &usedDefault);
	if (!usedDefault)
	{
	    class = args[1];
	    type = args[2];
	    argv = args + 3;
	}
    }
    while (numAcceptable)
    {
	if (DisplayTypeMatch (*acceptableTypes, displayType))
	    break;
	--numAcceptable;
	++acceptableTypes;
    }
    if (!numAcceptable)
    {
	LogError ("Unacceptable display type %s for display %s\n",
		  type, name);
    }
    d = FindDisplayByName (name);
    if (d)
    {
	d->state = OldEntry;
	if (class && strcmp (d->class, class))
	{
	    char    *newclass;

	    newclass = malloc ((unsigned) (strlen (class) + 1));
	    if (newclass)
	    {
		free (d->class);
		strcpy (newclass, class);
		d->class = newclass;
	    }
	}
	Debug ("Found existing display:  %s %s %s", d->name, d->class, type);
	freeArgs (d->argv);
    }
    else
    {
	d = NewDisplay (name, class);
	Debug ("Found new display:  %s %s %s", d->name, d->class, type);
    }
    d->displayType = displayType;
    d->argv = copyArgs (argv);
    for (a = d->argv; a && *a; a++)
	Debug (" %s", *a);
    Debug ("\n");
    freeSomeArgs (args, argv - args);
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
parseDisplayType (string, usedDefault)
	char	*string;
	int	*usedDefault;
{
	struct displayMatch	*d;

	for (d = displayTypes; d->name; d++)
		if (!strcmp (d->name, string))
		{
			*usedDefault = 0;
			return d->type;
		}
	*usedDefault = 1;
	return d->type;
}
