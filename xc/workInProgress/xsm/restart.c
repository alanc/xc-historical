/* $XConsortium: restart.c,v 1.2 94/02/22 15:44:18 mor Exp $ */
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

#include "xsm.h"

extern List *PendingList;
extern void remote_start ();


void
restart_everything()
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

	if (app_resources.verbose) {
	    printf("Restarting id '%s'...\n", c->clientId);
	    printf("Host = %s\n", c->clientHostname);
	}
	cwd = ".";
	env = NULL;
	program=NULL;
	args=NULL;

	for(pl = ListFirst(c->props); pl; pl = ListNext(pl)) {
	    prop = (PendingProp *)pl->thing;
	    if(!strcmp(prop->name, "Program")) {
		vl = ListFirst(prop->values);
		if(vl) program = ((PendingValue *)vl->thing)->value;
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
	    if (app_resources.verbose) {
		printf("\t%s\n", program);
		printf("\t");
		for(pp = args; *pp; pp++) printf("%s ", *pp);
		printf("\n");
	    }

	    if (!strncmp(c->clientHostname, "local/", 6))
	    {
		/*
		 * The client is being restarted on the local machine.
		 */

		switch(fork()) {
		case -1:
		    perror("fork");
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
}
