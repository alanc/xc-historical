/* $XConsortium: restart.c,v 1.15 94/08/02 18:46:37 mor Exp mor $ */
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


/*
 * Until XSMP provides a better way to know which clients are "managers",
 * we have to hard code the list.
 */

Bool
CheckIsManager (program)

char *program;

{
    return (strcmp (program, "twm") == 0);
}



/*
 * GetRestartInfo() will determine which method should be used to
 * restart a client.
 *
 * 'restart_service_prop' is a property set by the client, or NULL.
 * The format is "remote_start_protocol/remote_start_data".  An
 * example is "rstart-rsh/hostname".  This is a non-standard property,
 * which is the whole reason we need this function in order to determine
 * the restart method.  The proxy uses this property to over-ride the
 * 'client_host_name' from the ICE connection (the proxy is connected to
 * the SM via a local connection, but the proxy may be acting as a proxy
 * for a remote client).
 *
 * 'client_host_name' is the connection info obtained from the ICE
 * connection.  It's format is "transport/host_info".  An example
 * is "tcp/machine:port".
 *
 * If 'restart_service_prop' is NULL, we use 'client_host_name' to
 * determine the restart method.  If the transport is "local", we
 * do a local restart.  Otherwise, we use the default "rstart-rsh" method.
 *
 * If 'restart_service_prop' is non-NULL, we check the remote_start_protocol
 * field.  "local" means a local restart.  Currently, the only remote
 * protocol we recognize is "rstart-rsh".  If the remote protocol is
 * "rstart-rsh" but the hostname in the 'restart_service_prop' matches
 * 'client_host_name', we do a local restart.
 *
 * On return, set the run_local flag, restart_protocol and restart_machine.
 */

void
GetRestartInfo (restart_service_prop, client_host_name,
    run_local, restart_protocol, restart_machine)

char *restart_service_prop;
char *client_host_name;
Bool *run_local;
char **restart_protocol;
char **restart_machine;

{
    char hostnamebuf[80];
    char *temp;

    *run_local = False;
    *restart_protocol = NULL;
    *restart_machine = NULL;

    if (restart_service_prop)
    {
	gethostname (hostnamebuf, sizeof hostnamebuf);

	if ((temp = (char *) strchr (
	    restart_service_prop, '/')) == NULL)
	{
	    *restart_protocol = (char *) XtNewString ("rstart-rsh");
	    *restart_machine = (char *) XtNewString (restart_service_prop);
	}
	else
	{
	    *restart_protocol = (char *) XtNewString (restart_service_prop);
	    (*restart_protocol)[temp - restart_service_prop] = '\0';
	    *restart_machine = (char *) XtNewString (temp + 1);
	}

	if (strcmp (*restart_machine, hostnamebuf) == 0 ||
	    strcmp (*restart_protocol, "local") == 0)
	{
	    *run_local = True;
	}
    }
    else
    {
	if (strncmp (client_host_name, "tcp/", 4) != 0 &&
	    strncmp (client_host_name, "decnet/", 7) != 0)
	{
	    *run_local = True;
	}
	else
	{
	    *restart_protocol = (char *) XtNewString ("rstart-rsh");

	    if ((temp = (char *) strchr (
		client_host_name, '/')) == NULL)
	    {
		*restart_machine = (char *) XtNewString (client_host_name);
	    }
	    else
	    {
		*restart_machine = (char *) XtNewString (temp + 1);
	    }
	}
    }
}



/*
 * Restart clients.  The flag indicates RESTART_MANAGERS or
 * RESTART_REST_OF_CLIENTS.
 */

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
    char	*p;
    char	*restart_service_prop;
    char	*restart_protocol;
    char	*restart_machine;
    Bool	run_local;
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
	restart_service_prop=NULL;

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
	    } else if(!strcmp(prop->name, "_XC_RestartService")) {
		vl = ListFirst(prop->values);
		if(vl) restart_service_prop =
		    ((PendingValue *)vl->thing)->value;
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

	    GetRestartInfo (restart_service_prop, c->clientHostname,
    		&run_local, &restart_protocol, &restart_machine);

	    if (run_local)
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

		remote_start (restart_protocol, restart_machine,
		    program, args, cwd, env,
		    non_local_display_env, non_local_session_env);
	    }

	    if (restart_protocol)
		XtFree (restart_protocol);

	    if (restart_machine)
		XtFree (restart_machine);

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



/*
 * Clone a client
 */

void
Clone (client, useSavedState)

ClientRec *client;
Bool useSavedState;

{
    char	*cwd;
    char	*program;
    char	**args;
    char	**env;
    char	**pp;
    int		i, j;
    extern char **environ;
    char	*p;
    char	*restart_service_prop;
    char	*restart_protocol;
    char	*restart_machine;
    Bool	run_local;

    if (verbose)
    {
	printf ("Cloning id '%s', useSavedState = %d...\n",
		client->clientId, useSavedState);
	printf ("Host = %s\n", client->clientHostname);
    }

    cwd = ".";
    env = NULL;
    program = NULL;
    args = NULL;
    restart_service_prop = NULL;

    for (i = 0; i < client->numProps; i++)
    {
	SmProp *prop = client->props[i];

	if (strcmp (prop->name, SmProgram) == 0)
	    program = (char *) prop->vals[0].value;
	else if (strcmp (prop->name, SmCurrentDirectory) == 0)
	    cwd = (char *) prop->vals[0].value;
	else if (strcmp (prop->name, "_XC_RestartService") == 0)
	    restart_service_prop = (char *) prop->vals[0].value;
	else if (
	    (!useSavedState && strcmp(prop->name, SmCloneCommand) == 0) ||
	    (useSavedState && strcmp(prop->name, SmRestartCommand) == 0))
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

	GetRestartInfo (restart_service_prop, client->clientHostname,
    	    &run_local, &restart_protocol, &restart_machine);

	if (run_local)
	{
	    /*
	     * The client is being cloned on the local machine.
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
		   "Can't remote clone client ID '%s': only local supported\n",
			client->clientId);
	}
	else
	{
	    /*
	     * The client is being cloned on a remote machine.
	     */

	    remote_start (restart_protocol, restart_machine,
		program, args, cwd, env,
		non_local_display_env, non_local_session_env);
	}

	if (restart_protocol)
	    XtFree (restart_protocol);

	if (restart_machine)
	    XtFree (restart_machine);

    }
    else
    {
	XBell (XtDisplay (topLevel), 0);

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
	if (buf[0] == '!')
	    continue;		/* a comment */

	if (p = strchr (buf, '\n'))
	    *p = '\0';

	len = strlen (buf);

	buf[len] = '&';
	buf[len+1] = '\0';

	/* let the shell parse the stupid args */

	system (buf);
    }
}
