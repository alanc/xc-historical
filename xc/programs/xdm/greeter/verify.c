/*
 * standard unix verification routine
 *
 * This section should live in a seperate os-dependent file
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
	char		**env, **setEnv ();
	char		*shell;
	int		i;
	static char	*argv[3];

	Debug ("Verify %s %s\n", greet->name, greet->password);
	argv[0] = d->session;
	p = getpwnam (greet->name);
	if (!p)
		p = &joeblow;
	if (!strcmp (crypt (greet->password, p->pw_passwd), p->pw_passwd)) {
		verify->uid = p->pw_uid;
#ifdef NGROUPS
		verify->groups = parseGroups (greet->name, p->pw_gid);
#else
		verify->gid = p->pw_gid;
#endif
		argv[1] = greet->string;
		verify->argv = argv;
		verify->environ = setEnv (greet->name, p->pw_dir, d->name, p->pw_shell);
		Debug ("verify succeeded\n");
		return 1;
	}
	Debug ("verify failed\n");
	return 0;
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

char	stdPath[] = ".:/bin:/usr/bin:/usr/bin/X11:/usr/ucb";

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
setEnv (user, home, display, shell)
char	*user, *home, *display, *shell;
{
	environment[DISPLAY] = makeEnv (DISPLAY, display);
	environment[HOME] = makeEnv (HOME, home);
	environment[USER] = makeEnv (USER, user);
	environment[PATH] = makeEnv (PATH, stdPath);
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
