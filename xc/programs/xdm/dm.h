/*
 * dm.h
 *
 * public interfaces for greet/verify functionality
 */

#define	UDP_SOCKET

typedef enum displayStatus { running, notRunning } DisplayStatus;

struct display {
	struct display	*next;
	char		*name;		/* DISPLAY name */
	char		**argv;		/* program name and arguments */
	DisplayStatus	status;		/* current status */
	int		pid;		/* process id of child */
	char		*resources;	/* resource file */
	char		*xrdb;		/* xrdb program */
	char		*startup;	/* Xstartup program */
	char		*reset;		/* Xreset program */
	char		*session;	/* Xsession program */
	int		openDelay;	/* open delay time */
	int		openRepeat;	/* open attempts to make */
	int		terminateServer;/* restart this server */
};

struct greet_info {
	char		*name;		/* user name */
	char		*password;	/* user password */
	char		*string;	/* random string */
};

struct verify_info {
	int		uid;		/* user id */
#ifdef NGROUPS
	int		groups[NGROUPS];/* group list */
	int		ngroups;	/* number of elements in groups */
#else
	int		gid;		/* group id */
#endif
	char		**argv;		/* arguments to session */
	char		**environ;	/* environment for session */
};

extern char	*servers;
extern int	request_port;
extern char	*errorLogFile;
extern char	*validProgramsFile;

extern struct display	*FindDisplayByName (),
			*FindDisplayByPid (),
			*NewDisplay (),
			*ReadDisplay ();

extern char	*malloc (), *realloc (), *strcpy ();

#ifdef UDP_SOCKET

# define POLL_PROVIDERS "polling for available display managers\n"
# define ADVERTISE	"advertising a display manager\n"

#endif
