/* $XConsortium: restart.c,v 1.10 94/07/15 14:12:30 mor Exp $ */
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
    List *cl;
    List *pl;
    List *vl;
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
    static char	envDISPLAY[]="DISPLAY";
    static char	envSESSION_MANAGER[]="SESSION_MANAGER";
    static char	envAUDIOSERVER[]="AUDIOSERVER";
    char	*display_env, *non_local_display_env;
    char	*session_env, *non_local_session_env;
    char	*audio_env;
    int		remote_allowed = 1;
    Bool	is_manager;
    Bool	ran_manager = 0;

    display_env = NULL;
    if(p = (char *) getenv(envDISPLAY)) {
	display_env = (char *) malloc(strlen(envDISPLAY)+1+strlen(p)+1);
	if(!display_env) nomem();
	sprintf(display_env, "%s=%s", envDISPLAY, p);

	/*
	 * When we restart a remote client, we have to make sure the
	 * display environment we give it has the SM's hostname.
	 */

	if ((temp = strchr (p, '/')) == 0)
	    temp = p;
	else
	    temp++;

	if (*temp != ':')
	{
	    /* we have a host name */

	    non_local_display_env = (char *) malloc (strlen (display_env) + 1);
	    if (!non_local_display_env) nomem();

	    strcpy (non_local_display_env, display_env);
	}
	else
	{
	    char hostnamebuf[256];

	    gethostname (hostnamebuf, sizeof hostnamebuf);
	    non_local_display_env = (char *) malloc (strlen (envDISPLAY) + 1 +
		strlen (hostnamebuf) + strlen (temp) + 1);
	    if (!non_local_display_env) nomem();
	    sprintf(non_local_display_env, "%s=%s%s",
		envDISPLAY, hostnamebuf, temp);
	}
    }

    session_env = NULL;
    if(p = (char *) getenv(envSESSION_MANAGER)) {
	session_env = (char *) malloc(
	    strlen(envSESSION_MANAGER)+1+strlen(p)+1);
	if(!session_env) nomem();
	sprintf(session_env, "%s=%s", envSESSION_MANAGER, p);

	/*
	 * When we restart a remote client, we have to make sure the
	 * session environment does not have the SM's local connection port.
	 */

	non_local_session_env = (char *) malloc (strlen (session_env) + 1);
	if (!non_local_session_env) nomem();
	strcpy (non_local_session_env, session_env);

	if ((temp = Strstr (non_local_session_env, "local/")) != NULL)
	{
	    char *delim = strchr (temp, ',');
	    if (delim == NULL)
	    {
		if (temp == non_local_session_env +
		    strlen (envSESSION_MANAGER) + 1)
		{
		    *temp = '\0';
		    remote_allowed = 0;
		}
		else
		    *(temp - 1) = '\0';
	    }
	    else
	    {
		int bytes = strlen (delim + 1);
		memmove (temp, delim + 1, bytes);
		*(temp + bytes) = '\0';
	    }
	}
    }

    audio_env = NULL;
    if(p = (char *) getenv(envAUDIOSERVER)) {
	audio_env = (char *) malloc(strlen(envAUDIOSERVER)+1+strlen(p)+1);
	if(!audio_env) nomem();
	sprintf(audio_env, "%s=%s", envAUDIOSERVER, p);
    }

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
	    if(!strcmp(prop->name, "Program")) {
		vl = ListFirst(prop->values);
		if(vl) program = ((PendingValue *)vl->thing)->value;
		if (CheckIsManager (program))
		    is_manager = 1;
	    } else if(!strcmp(prop->name, "CurrentDirectory")) {
		vl = ListFirst(prop->values);
		if(vl) cwd = ((PendingValue *)vl->thing)->value;
	    } else if(!strcmp(prop->name, "RestartCommand")) {
		cnt = ListCount(prop->values);
		args = (char **)malloc((cnt+1) * sizeof(char *));
		pp = args;
		for(vl = ListFirst(prop->values); vl; vl = ListNext(vl)) {
		    *pp++ = ((PendingValue *)vl->thing)->value;
		}
		*pp = NULL;
	    } else if(!strcmp(prop->name, "Environment")) {
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
    if(display_env) free(display_env);
    if(session_env) free(session_env);
    if(non_local_display_env) free(non_local_display_env);
    if(non_local_session_env) free(non_local_session_env);
    if(audio_env) free(audio_env);

    if (flag == RESTART_MANAGERS && !ran_manager)
	return (0);
    else
	return (1);
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
