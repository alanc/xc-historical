/*
 * resource.c
 */

# include "dm.h"
# include <X11/Xlib.h>
# include <X11/Xresource.h>

# define DM_RESOURCES	"/usr/lib/X11/xdm/Xdm-config"

char	*servers;
int	request_port;
char	*errorLogFile;
char	*validProgramsFile;

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
				"/usr/lib/xdm/Xservers",
"requestPort",	"RequestPort",	DM_INT,		(char **) &request_port,
				"0",
"errorLogFile",	"ErrorLogFile",	DM_STRING,	&errorLogFile,
				"/usr/adm/Xdm-errors",
"validPrograms","ValidPrograms",DM_STRING,	&validProgramsFile,
				"/usr/lib/xdm/Xprograms",
};

# define NUM_DM_RESOURCES	(sizeof DmResources / sizeof DmResources[0])

# define boffset(f)	((char *) &(((struct display *) 0)->f) - (char *) 0)

char	terminateDefault[10] = "false";
char	multipleDefault[10] = "true";

struct displayResources {
	char	*name, *class;
	int	type;
	int	offset;
	char	*default_value;
} DisplayResources[] = {
"resources",	"Resources",	DM_STRING,	boffset(resources),
				"/usr/lib/xdm/Xresources",
"xrdb",		"Xrdb",		DM_STRING,	boffset(xrdb),
				"/usr/bin/X11/xrdb",
"startup",	"Startup",	DM_STRING,	boffset(startup),
				"/usr/lib/xdm/Xstartup",
"reset",	"Reset",	DM_STRING,	boffset(reset),
				"/usr/lib/xdm/Xreset",
"session",	"Session",	DM_STRING,	boffset(session),
				"/usr/lib/xdm/Xsession",
"openDelay",	"OpenDelay",	DM_INT,		boffset(openDelay),
				"5",
"openRepeat",	"OpenRepeat",	DM_INT,		boffset(openRepeat),
				"5",
"terminateServer","TerminateServer",DM_BOOL,	boffset(terminateServer),
				terminateDefault,
"multipleSessions","MultipleSessions",DM_BOOL,	boffset(multipleSessions),
				multipleDefault,
};

# define NUM_DISPLAY_RESOURCES	(sizeof DisplayResources/\
				 sizeof DisplayResources[0])

XrmDatabase	DmResourceDB;

SetDefaults (terminate, multiple)
char	*terminate, *multiple;
{
	strcpy (terminateDefault, terminate);
	strcpy (multipleDefault, multiple);
}

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

InitResources ()
{
	DmResourceDB = XrmGetFileDatabase ( DM_RESOURCES );
	if (!DmResourceDB)
		LogError ("Can't open resource file %s\n", DM_RESOURCES);
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
