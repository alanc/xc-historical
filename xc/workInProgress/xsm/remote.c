/* $XConsortium$ */
/******************************************************************************
Copyright 1993 by the Massachusetts Institute of Technology,

                        All Rights Reserved

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that copyright
notice and this permission notice appear in supporting documentation, and that
the name of M.I.T. not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior permission.
M.I.T. makes no representations about the suitability of this software for any
purpose.  It is provided "as is" without express or implied warranty.
******************************************************************************/

/*
 * We use the rstart protocol to restart clients on remote machines.
 */

#include "xsm.h"

static char 		*spaces_to_octal();

extern IceAuthDataEntry	*authDataEntries;
extern int		numTransports;
extern void		fprintfhex ();


void
remote_start (client_host, program, args, cwd, env,
    non_local_display_env, non_local_session_env)

char	*client_host;
char	*program;
char	**args;
char	*cwd;
char	**env;
char	*non_local_display_env;
char	*non_local_session_env;

{
    FILE *fp;
    char *hostname;
    char *tmp;
    int	 pipefd[2];
    int  i;

    if ((tmp = strchr (client_host, '/')) == NULL)
	hostname = client_host;
    else
	hostname = tmp + 1;

    if (app_resources.verbose)
    {
	printf ("Attempting to restart remote client on %s\n", hostname);
    }

    if (pipe (pipefd) < 0)
    {
	perror ("pipe error");
    }
    else
    {
	switch(fork())
	{
	case -1:

	    perror("fork");
	    break;

	case 0:		/* kid */

	    close (pipefd[1]);
	    close (0);
	    dup (pipefd[0]);
	    close (pipefd[0]);

	    execlp (RSHCMD, hostname, "rstartd", (char *) 0);

	    perror("Failed to start remote client with rstart protocol");
	    _exit(255);

	default:		/* parent */

	    close (pipefd[0]);
	    fp = fdopen (pipefd[1], "w");

	    fprintf (fp, "CONTEXT X\n");
	    fprintf (fp, "DIR %s\n", cwd);

	    for (i = 0; env[i]; i++)
	    {
		/*
		 * There may be spaces inside some of the environment
		 * values, and rstartd will barf on spaces.  Need
		 * to replace space characters with their equivalent
		 * octal escape sequences.
		 */

		char *temp = spaces_to_octal (env[i]);
		fprintf (fp, "MISC X %s\n", temp);
		if (temp != env[i])
		    free (temp);
	    }

	    fprintf (fp, "MISC X %s\n", non_local_display_env);
	    fprintf (fp, "MISC X %s\n", non_local_session_env);

	    /*
	     * Pass the authentication data.
	     * Each transport has auth data for ICE and XSMP.
	     * Don't pass local auth data.
	     */

	    for (i = 0; i < numTransports * 2; i++)
	    {
		if (Strstr (authDataEntries[i].address, "local/"))
		    continue;

		fprintf (fp, "AUTH ICE %s \"\" %s %s ",
		    authDataEntries[i].protocol_name,
		    authDataEntries[i].address,
		    authDataEntries[i].auth_name);
		
		fprintfhex (fp,
		    authDataEntries[i].auth_data_length,
		    authDataEntries[i].auth_data);
		
		fprintf (fp, "\n");
	    }

	    /*
	     * And execute the program
	     */

	    fprintf (fp, "EXEC %s %s", program, program);
	    for (i = 1; args[i]; i++)
		fprintf (fp, " %s", args[i]);
	    fprintf (fp, "\n\n");
	    fclose (fp);
	    break;
	}
    }
}



/*
 * rstart requires that any spaces inside of a string be represented by
 * octal escape sequences.
 */

static char *
spaces_to_octal (str)

char *str;

{
    int space_count = 0;
    char *temp = str;

    while (*temp != '\0')
    {
	if (*temp == ' ')
	    space_count++;
	temp++;
    }

    if (space_count == 0)
	return (str);
    else
    {
	int len = strlen (str) + 1 + (space_count * 4);
	char *ret = malloc (len);
	char *ptr = ret;

	temp = str;
	while (*temp != '\0')
	{
	    if (*temp != ' ')
		*(ptr++) = *temp;
	    else
	    {
		ptr[0] = '\\'; ptr[1] = '0'; ptr[2] = '4'; ptr[3] = '0';
		ptr += 4;
	    }
	    temp++;
	}

	*ptr = '\0';
	return (ret);
    }
}
