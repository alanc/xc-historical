/* $XConsortium: restart.c,v 1.11 94/07/21 15:59:06 mor Exp $ */
/******************************************************************************

Copyright (c) 1993  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.
******************************************************************************/

#include "xsm.h"

#if (defined(SYSV) || defined(macII)) && !defined(hpux)
#define vfork() fork()
#endif

extern List *PendingList;
extern void remote_start ();


Bool
CheckIsManager (program)

char *program;

{
    return (strcmp (program, "twm") == 0);
}



Status
Restart (flag)

int flag;

{
    List 	*cl, *pl, *vl;
    PendingClient *c;
    PendingProp *prop;
    PendingValue *v;
    char	*cwd;
    char	*program;
    char	**args;
    char	**env;
    char	**pp;
    int		cnt;
    extern char **environ;
    char	*p, *temp;
    Bool	is_manager;
    Bool	ran_manager = 0;

    for(cl = ListFirst(PendingList); cl; cl = ListNext(cl)) {
	c = (PendingClient *)cl->thing;

	if (verbose) {
	    printf("Restarting id '%s'...\n", c->clientId);
	    printf("Host = %s\n", c->clientHostname);
	}
	cwd = ".";
	env = NULL;
	program=NULL;
	args=NULL;

	is_manager = 0;

	for(pl = ListFirst(c->props); pl; pl = ListNext(pl)) {
	    prop = (PendingProp *)pl->thing;
	    if(!strcmp(prop->name, SmProgram)) {
		vl = ListFirst(prop->values);
		if(vl) program = ((PendingValue *)vl->thing)->value;
		if (CheckIsManager (program))
		    is_manager = 1;
	    } else if(!strcmp(prop->name, SmCurrentDirectory)) {
		vl = ListFirst(prop->values);
		if(vl) cwd = ((PendingValue *)vl->thing)->value;
	    } else if(!strcmp(prop->name, SmRestartCommand)) {
		cnt = ListCount(prop->values);
		args = (char **)malloc((cnt+1) * sizeof(char *));
		pp = args;
		for(vl = ListFirst(prop->values); vl; vl = ListNext(vl)) {
		    *pp++ = ((PendingValue *)vl->thing)->value;
		}
		*pp = NULL;
	    } else if(!strcmp(prop->name, SmEnvironment)) {
		cnt = ListCount(prop->values);
		env = (char **)malloc((cnt+3+1) * sizeof(char *));
		pp = env;
		for(vl = ListFirst(prop->values); vl; vl = ListNext(vl)) {
		    p = ((PendingValue *)vl->thing)->value;
		    if((display_env && strbw(p, "DISPLAY="))
		    || (session_env && strbw(p, "SESSION_MANAGER="))
		    || (audio_env && strbw(p, "AUDIOSERVER="))
		        ) continue;
		    *pp++ = p;
		}
		if(display_env) *pp++ = display_env;
		if(session_env) *pp++ = session_env;
		if(audio_env) *pp++ = audio_env;
		*pp = NULL;
	    }
	}

	if(program && args) {
	    if ((flag == RESTART_MANAGERS && !is_manager) ||
	        (flag == RESTART_REST_OF_CLIENTS && is_manager))
	    {
		if(args) free((char *)args);
		if(env) free((char *)env);
		continue;
	    }

	    if (flag == RESTART_MANAGERS && is_manager)
		ran_manager = 1;

	    if (verbose) {
		printf("\t%s\n", program);
		printf("\t");
		for(pp = args; *pp; pp++) printf("%s ", *pp);
		printf("\n");
	    }

	    if (strncmp(c->clientHostname, "tcp/", 4) != 0 &&
		strncmp(c->clientHostname, "decnet/", 7) != 0)
	    {
		/*
		 * The client is being restarted on the local machine.
		 */

		switch(vfork()) {
		case -1:
		    perror("vfork");
		    break;
		case 0:		/* kid */
		    chdir(cwd);
		    if(env) environ = env;
		    execvp(program, args);
		    perror("execve");
		    _exit(255);
		default:	/* parent */
		    break;
		}
	    }
	    else if (!remote_allowed)
	    {
		fprintf(stderr,
		   "Can't remote start client ID '%s': only local supported\n",
			c->clientId);
	    }
	    else
	    {
		/*
		 * The client is being restarted on a remote machine.
		 */

		remote_start (c->clientHostname, program, args, cwd, env,
		    non_local_display_env, non_local_session_env);
	    }
	} else {
	    fprintf(stderr, "Can't restart ID '%s':  no program or no args\n",
		c->clientId);
	}
	if(args) free((char *)args);
	if(env) free((char *)env);
    }

    if (flag == RESTART_MANAGERS && !ran_manager)
	return (0);
    else
	return (1);
}



void
Clone (client)

ClientRec *client;

{
    char	*cwd;
    char	*program;
    char	**args;
    char	**env;
    char	**pp;
    int		i, j;
    extern char **environ;
    char	*p, *temp;

    if (verbose)
    {
	printf ("Cloning id '%s'...\n", client->clientId);
	printf ("Host = %s\n", client->clientHostname);
    }

    cwd = ".";
    env = NULL;
    program = NULL;
    args = NULL;

    for (i = 0; i < client->numProps; i++)
    {
	SmProp *prop = client->props[i];

	if (strcmp (prop->name, SmProgram) == 0)
	    program = (char *) prop->vals[0].value;
	else if (strcmp (prop->name, SmCurrentDirectory) == 0)
	    cwd = (char *) prop->vals[0].value;
	else if (strcmp(prop->name, SmCloneCommand) == 0)
	{
	    args = (char **) malloc ((prop->num_vals + 1) * sizeof (char *));
	    pp = args;
	    for (j = 0; j < prop->num_vals; j++)
		*pp++ = (char *) prop->vals[j].value;
	    *pp = NULL;
	}
	else if (strcmp (prop->name, SmEnvironment) == 0)
	{
	    env = (char **) malloc (
		(prop->num_vals + 3 + 1) * sizeof (char *));
	    pp = env;
	    for (j = 0; j < prop->num_vals; j++)
	    {
		p = (char *) prop->vals[j].value;

		if ((display_env && strbw (p, "DISPLAY="))
	         || (session_env && strbw (p, "SESSION_MANAGER="))
		 || (audio_env && strbw (p, "AUDIOSERVER=")))
		    continue;

		*pp++ = p;
	    }

	    if (display_env)
		*pp++ = display_env;
	    if (session_env)
		*pp++ = session_env;
	    if (audio_env)
		*pp++ = audio_env;

	    *pp = NULL;
	}
    }

    if (program && args)
    {
	if (verbose)
	{
	    printf("\t%s\n", program);
	    printf("\t");
	    for (pp = args; *pp; pp++)
		printf ("%s ", *pp);
	    printf("\n");
	}

	if (strncmp (client->clientHostname, "tcp/", 4) != 0 &&
	    strncmp (client->clientHostname, "decnet/", 7) != 0)
	{
	    /*
	     * The client is being restarted on the local machine.
	     */

	    switch(vfork()) {
	    case -1:
		perror("vfork");
		break;
	    case 0:		/* kid */
		chdir (cwd);
		if (env) environ = env;
		execvp (program, args);
		perror ("execve");
		_exit (255);
	    default:	/* parent */
		break;
	    }
	}
	else if (!remote_allowed)
	{
	    fprintf (stderr,
		   "Can't remote clone client ID '%s': only local supported\n",
		     client->clientId);
	}
	else
	{
	    /*
	     * The client is being restarted on a remote machine.
	     */

	    remote_start (client->clientHostname, program, args, cwd, env,
		non_local_display_env, non_local_session_env);
	}
    } else {
	fprintf(stderr, "Can't restart ID '%s':  no program or no args\n",
		client->clientId);
    }

    if (args)
	free ((char *)args);
    if (env)
	free ((char *)env);
}



#if defined(sun) && defined(SVR4)
static int System (s)
    char *s;
{
    int pid, status;
    if ((pid = vfork ()) == 0) {
	(void) setpgrp();
	execl ("/bin/sh", "sh", "-c", s, 0);
    } else
	waitpid (pid, &status, 0);
    return status;
}
#define system(s) System(s)
#endif



void
StartDefaultApps ()

{
    FILE *f;
    char *buf, *p, *home, filename[128];
    int buflen, len;

    /*
     * First try ~/.xsmstartup, then system.xsm
     */

    home = (char *) getenv ("HOME");
    if (!home)
	home = ".";
    sprintf (filename, "%s/.xsmstartup", home);

    f = fopen (filename, "r");

    if (!f)
    {
	f = fopen (SYSTEM_INIT_FILE, "r");
	if (!f)
	{
	    printf ("Could not find default apps file.  Make sure you did\n");
	    printf ("a 'make install' in the xsm build directory.\n");
	    exit (1);
	}
    }

    buf = NULL;
    buflen = 0;

    while (getline(&buf, &buflen, f))
    {
	if (p = strchr (buf, '\n'))
	    *p = '\0';

	len = strlen (buf);

	buf[len] = '&';
	buf[len+1] = '\0';

	/* let the shell parse the stupid args */

	system (buf);
    }
}
