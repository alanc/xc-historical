/* $XConsortium$ */

#include <stdlib.h>
#include <string.h>

/*
 * An emulation of putenv() which is required for BSD4.2 systems
 * and perhaps pure POSIX.1 systems.
 * 
 * Do not use if the system provides putenv()
 * 
 */
int
putenv(envstr)
char *envstr;
{
	/*
	 * This routine mimics putenv(), and is provided
	 * because putenv() is not in POSIX.1
	 */

	char **newenv, **cur, *envname;
	int n, count = 0;
	static char **allocp = NULL;
	extern	char	**environ;

	if (environ == NULL)
	{
		newenv = (char **)malloc((size_t)(2*sizeof(char *)));
		if (newenv == NULL) 
			return -1;

		newenv[0] = envstr;
		newenv[1] = NULL;
		environ = newenv;
		allocp = newenv;
		return 0;
	}

	cur = environ;

	while (*cur != NULL)
	{
		count++;
		envname = *cur;
		n = strcspn(envstr, "=");
		if (strncmp(envname, envstr, (size_t) n) || envname[n] != '=')
			cur++;
		else
		{
			*cur = envstr;
			return 0;
		}
	}
	
	/*
	 * If we previously allocated this environment enlarge it using
	 * realloc(), otherwise allocate a new one and copy it over.
	 * Note that only the last malloc()/realloc() pointer is saved, so
	 * if environ has since been changed the old space will be wasted.
	 */

	if (environ == allocp)
		newenv = (char **) realloc((void *) environ, 
				(size_t) ((count+2)*sizeof(char *)));
	else
		newenv = (char **) malloc((size_t) ((count+2)*sizeof(char *)));

	if (newenv == NULL) 
		return -1;
	
	if (environ != allocp)
	{
		for (n = 0; environ[n] != NULL; n++)
			newenv[n] = environ[n];
		allocp = newenv;
	}
	newenv[count] = envstr;
	newenv[count+1] = NULL;
	environ = newenv;

	return 0;
}
