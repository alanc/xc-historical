/*
 * xdm - display manager daemon
 *
 * $XConsortium: $
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
 * verify.c
 *
 * typical unix verification routine.
 */

# include	<pwd.h>
# include	"dm.h"

struct passwd joeblow = {
	"Nobody", "***************"
};

Verify (d, greet, verify)
struct display		*d;
struct greet_info	*greet;
struct verify_info	*verify;
{
	struct passwd	*p;
	char		*crypt ();
	char		**setEnv (), **parseArgs ();
	char		*shell, *home;
	char		**argv;

	p = getpwnam (greet->name);
	if (!p)
		p = &joeblow;
	Debug ("Verify %s %s\n", greet->name, greet->password);
	if (strcmp (crypt (greet->password, p->pw_passwd), p->pw_passwd)) {
		Debug ("verify failed\n");
		return 0;
	}
	Debug ("verify succeeded\n");
	verify->uid = p->pw_uid;
#ifdef NGROUPS
	verify->groups = parseGroups (greet->name, p->pw_gid);
#else
	verify->gid = p->pw_gid;
#endif
	home = p->pw_dir;
	shell = p->pw_shell;
	argv = parseArgs ((char **) 0, d->session);
	argv = parseArgs (argv, greet->string);
	verify->argv = argv;
	verify->environ = setEnv (d, greet->name, home, shell);
	return 1;
}

/*
 * build an execution environment
 */

# define DISPLAY	0
# define HOME		1
# define PATH		2
# define USER		3
# define SHELL		4

# define NENV		5

char	*environment[NENV + 1];

char	*envname[NENV] = {
 	"DISPLAY",
	"HOME",
 	"PATH",
 	"USER",
	"SHELL",
};

char *
makeEnv (num, value)
char	*value;
{
	char	*result, *malloc ();

	result = malloc (strlen (envname[num]) + strlen (value) + 2);
	sprintf (result, "%s=%s", envname[num], value);
	return result;
}

char **
setEnv (d, user, home, shell)
struct display	*d;
char	*user, *home, *shell;
{
	environment[DISPLAY] = makeEnv (DISPLAY, d->name);
	environment[HOME] = makeEnv (HOME, home);
	environment[USER] = makeEnv (USER, user);
	environment[PATH] = makeEnv (PATH, d->unixPath);
	environment[SHELL] = makeEnv (SHELL, shell);
	return environment;
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

#ifdef NGROUPS
groupMember (name, members)
char	*name;
char	**members;
{
	while (*members) {
		if (!strcmp (name, *members))
			return 1;
		++members;
	}
	return 0;
}

getGroups (name, verify)
char			*name;
struct verify_info	*verify;
{
	int		ngroups;
	struct group	*g;

	ngroups = 0;
	verify->groups[ngroups++] = gid;
	setgrent ();
	while (g = getgrent()) {
		if (groupMember (name, g->gr_mem)) {
			if (ngroups >= NGROUPS)
				error ("too many groups");
			else
				verify->groups[ngroups++] = g->gr_gid;
		}
	}
	verify->ngroups = ngroups;
	endgrent ();
}
#endif

# define isblank(c)	((c) == ' ' || c == '\t')

char **
parseArgs (argv, string)
char	**argv;
char	*string;
{
	char	*word;
	char	*save;
	int	i;
	char	*malloc (), *realloc (), *strcpy ();;

	i = 0;
	while (argv && argv[i])
		++i;
	if (!argv)
		argv = (char **) malloc (sizeof (char *));
	word = string;
	for (;;) {
		if (!*string || isblank (*string)) {
			if (word != string) {
				argv = (char **) realloc (argv, (i + 1) * sizeof (char *));
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
