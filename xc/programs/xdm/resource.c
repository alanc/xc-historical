/*
 * xdm - display manager daemon
 *
 * $XConsortium: resource.c,v 1.5 88/10/15 19:12:30 keith Exp $
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
 * resource.c
 */

# include "dm.h"
# include <X11/Xlib.h>
# include <X11/Xresource.h>

/* XtOffset() hack for ibmrt BandAidCompiler */

char	*servers;
int	request_port;
int	debugLevel;
char	*errorLogFile;
int	daemonMode;

# define DM_STRING	0
# define DM_INT		1
# define DM_BOOL	2

struct dmResources {
	char	*name, *class;
	int	type;
	char	**dm_value;
	char	*default_value;
} DmResources[] = {
"servers",	"Servers", 	DM_STRING,	&servers,
				":0 secure /usr/bin/X11/X :0",
"requestPort",	"RequestPort",	DM_INT,		(char **) &request_port,
				"0",
"debugLevel",	"DebugLevel",	DM_INT,		(char **) &debugLevel,
				"0",
"errorLogFile",	"ErrorLogFile",	DM_STRING,	&errorLogFile,
				"",
"daemonMode",	"DaemonMode",	DM_BOOL,	(char **) &daemonMode,
				"true"
};

# define NUM_DM_RESOURCES	(sizeof DmResources / sizeof DmResources[0])

# define boffset(f)	((char *) &(((struct display *) 0)->f) - (char *) 0)

struct displayResources {
	char	*name, *class;
	int	type;
	int	offset;
	char	*default_value;
} DisplayResources[] = {
"resources",	"Resources",	DM_STRING,	boffset(resources),
				"",
"xrdb",		"Xrdb",		DM_STRING,	boffset(xrdb),
				"/usr/bin/X11/xrdb",
"startup",	"Startup",	DM_STRING,	boffset(startup),
				"",
"reset",	"Reset",	DM_STRING,	boffset(reset),
				"",
"session",	"Session",	DM_STRING,	boffset(session),
				"/usr/bin/X11/xterm -ls",
"openDelay",	"OpenDelay",	DM_INT,		boffset(openDelay),
				"5",
"openRepeat",	"OpenRepeat",	DM_INT,		boffset(openRepeat),
				"5",
"terminateServer","TerminateServer",DM_BOOL,	boffset(terminateServer),
				"false",
"userPath",	"Path",		DM_STRING,	boffset(userPath),
				":/bin:/usr/bin:/usr/bin/X11:/usr/ucb",
"systemPath",	"Path",		DM_STRING,	boffset(systemPath),
				"/etc:/bin:/usr/bin:/usr/bin/X11:/usr/ucb",
"systemShell",	"Shell",	DM_STRING,	boffset(systemShell),
				"/bin/sh",
"failsafeClient","FailsafeClient",	DM_STRING,	boffset(failsafeClient),
				"/usr/bin/X11/xterm",
};

# define NUM_DISPLAY_RESOURCES	(sizeof DisplayResources/\
				 sizeof DisplayResources[0])

XrmDatabase	DmResourceDB;

GetResource (name, class, valueType, valuep, default_value)
char	*name, *class;
int	valueType;
char	**valuep;
char	*default_value;
{
	char	*type;
	XrmValue	value;
	char	*string, *strncpy (), *malloc ();
	int	len;

	if (DmResourceDB && XrmGetResource (DmResourceDB,
		name, class,
		&type, &value))
	{
		string = value.addr;
		len = value.size;
	} else {
		string = default_value;
		len = strlen (string);
	}
	string = strncpy (malloc (len+1), string, len);
	string[len] = '\0';
	Debug ("resource %s value %s\n", name, string);
	switch (valueType) {
	case DM_STRING:
		*(valuep) = string;
		break;
	case DM_INT:
		*((int *) valuep) = atoi (string);
		free (string);
		break;
	case DM_BOOL:
		XmuCopyISOLatin1Lowered (string, string);
		if (!strcmp (string, "true") ||
		    !strcmp (string, "on") ||
		    !strcmp (string, "yes"))
			*((int *) valuep) = 1;
		else if (!strcmp (string, "false") ||
			 !strcmp (string, "off") ||
			 !strcmp (string, "no"))
			*((int *) valuep) = 0;
		free (string);
		break;
	}
}

XrmOptionDescRec optionTable [] = {
{"-server",	".servers",		XrmoptionSepArg,	(caddr_t) NULL },
{"-udpPort",	".requestPort",		XrmoptionSepArg,	(caddr_t) NULL },
{"-error",	".errorLogFile",	XrmoptionSepArg,	(caddr_t) NULL },
{"-resources",	"*resources",		XrmoptionSepArg,	(caddr_t) NULL },
{"-session",	"*session",		XrmoptionSepArg,	(caddr_t) NULL },
{"-debug",	"*debugLevel",		XrmoptionSepArg,	(caddr_t) NULL },
{"-config",	NULL,			XrmoptionSkipArg,	(caddr_t) NULL },
{"-xrm",	NULL,			XrmoptionResArg,	(caddr_t) NULL },
{"-daemon",	".daemonMode",		XrmoptionNoArg,		"true"         },
{"-nodaemon",	".daemonMode",		XrmoptionNoArg,		"false"        },
};

#ifndef DEFAULT_XDM_CONFIG
# define DEFAULT_XDM_CONFIG "/usr/lib/X11/xdm/xdm-config"
#endif

InitResources (argc, argv)
int	argc;
char	**argv;
{
	char	**a;
	char	*config = 0;

	XrmInitialize ();
	for (a = argv+1; *a; a++) {
		if (!strcmp (*a, "-config")) {
			if (!a[1])
				LogError ("missing config file argument\n");
			else
				config = a[1];
			break;
		}
	}
	if (!config) {
		config = DEFAULT_XDM_CONFIG;
		if (access (config, 4) == -1)
			config = 0;
	}
	if (config) {
		DmResourceDB = XrmGetFileDatabase ( config );
		if (!DmResourceDB)
			LogError ("Can't open resource file %s\n", config );
	}
	XrmParseCommand (&DmResourceDB, optionTable,
 			 sizeof (optionTable) / sizeof (optionTable[0]),
			 "DisplayManager", &argc, argv);
			 
}

LoadDMResources ()
{
	int	i;
	char	name[1024], class[1024];

	for (i = 0; i < NUM_DM_RESOURCES; i++) {
		sprintf (name, "DisplayManager.%s", DmResources[i].name);
		sprintf (class, "DisplayManager.%s", DmResources[i].class);
		GetResource (name, class, DmResources[i].type,
			      DmResources[i].dm_value,
			      DmResources[i].default_value);
	}
}

LoadDisplayResources (d)
struct display	*d;
{
	int	i;
	char	name[1024], class[1024];

	for (i = 0; i < NUM_DISPLAY_RESOURCES; i++) {
		sprintf (name, "DisplayManager.%s.%s", 
			d->name, DisplayResources[i].name);
		sprintf (class, "DisplayManager.%s.%s",
			d->name, DisplayResources[i].class);
		GetResource (name, class, DisplayResources[i].type,
			      ((char *) d) + DisplayResources[i].offset,
			      DisplayResources[i].default_value);
	}
}
