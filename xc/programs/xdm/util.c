/*
 * xdm - display manager daemon
 *
 * $XConsortium: verify.c,v 1.6 88/11/17 19:13:52 keith Exp $
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
 * util.c
 *
 * various utility routines
 */

printEnv (e)
char	**e;
{
	while (*e)
		Debug ("%s\n", *e++);
}

static char *
makeEnv (name, value)
char	*name;
char	*value;
{
	char	*result, *malloc ();

	result = malloc (strlen (name) + strlen (value) + 2);
	sprintf (result, "%s=%s", name, value);
	return result;
}

char *
getEnv (e, name)
	char	**e;
	char	*name;
{
	int	l = strlen (name);

	while (*e) {
		if (strlen (*e) > l && !strncmp (*e, name, l) &&
			(*e)[l] == '=')
			return (*e) + l + 1;
		++e;
	}
	return 0;
}

char **
setEnv (e, name, value)
	char	**e;
	char	*name;
	char	*value;
{
	char	**new, **old;
	int	envsize;
	int	i;
	int	l;
	char	*malloc (), *realloc ();

	l = strlen (name);
	if (e) {
		for (old = e; *old; old++)
			if (strlen (*old) > l && !strncmp (*old, name, l) && (*old)[l] == '=')
				break;
		if (*old) {
			free (*old);
			*old = makeEnv (name, value);
			return e;
		}
		envsize = old - e;
		new = (char **) realloc (e, (envsize + 2) * sizeof (char *));
	} else {
		envsize = 0;
		new = (char **) malloc (2 * sizeof (char *));
	}
	new[envsize] = makeEnv (name, value);
	new[envsize+1] = 0;
	return new;
}


# define isblank(c)	((c) == ' ' || c == '\t')

char **
parseArgs (argv, string)
char	**argv;
char	*string;
{
	char	*word;
	char	*save;
	int	i;
	char	*malloc (), *realloc (), *strcpy (), *strncpy ();

	i = 0;
	while (argv && argv[i])
		++i;
	if (!argv)
		argv = (char **) malloc (sizeof (char *));
	word = string;
	for (;;) {
		if (!*string || isblank (*string)) {
			if (word != string) {
				argv = (char **) realloc ((char *) argv, (i + 2) * sizeof (char *));
				argv[i] = strncpy (malloc (string - word + 1), word, string-word);
				argv[i][string-word] = '\0';
				i++;
			}
			if (!*string)
				break;
			word = string + 1;
		}
		++string;
	}
	argv[i] = 0;
	return argv;
}
