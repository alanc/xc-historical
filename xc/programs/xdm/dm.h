/*
 * dm.h
 *
 * public interfaces for greet/verify functionality
 */

# include	<X11/Xos.h>

#define	UDP_SOCKET

#ifdef SYSV
# define waitCode(w)	((w) & 0xff)
# define waitSig(w)	(((w) >> 8) & 0xff)
typedef int		waitType;
#else
# include	<sys/wait.h>
# define waitCode(w)	((w).w_T.w_Retcode)
# define waitSig(w)	((w).w_T.w_Termsig)
typedef union wait	waitType;
#endif

#ifdef UDP_SOCKET
#include	<sys/types.h>
#include	<netinet/in.h>
#endif

# define waitVal(w)	(waitSig(w) ? -1 : waitCode (w))

typedef enum displayStatus { running, notRunning } DisplayStatus;

/*
 * local     - server runs on local host
 * foreign   - server runs on remote host
 * removable - 'remove' message can delete entry
 * permanent - session restarted when it exits
 * transient - session not restarted when it exits
 */

typedef enum displayType {
	secure,		/* local permanent non-removable server */
 	insecure,	/* local permanent removable server */
 	foreign,	/* foreign permanent server */
 	transient,	/* foreign non-permanent server */
 	remove,		/* command to remove a server */
	unknown		/* not any of the above */
} DisplayType;

extern DisplayType parseDisplayType ();

# define restartType(t)	((t) == secure || (t) == insecure || (t) == foreign)

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
	int		terminateServer;/* restart for each session */
	DisplayType	displayType;	/* method to handle with */
#ifdef UDP_SOCKET
	struct sockaddr_in	addr;	/* address used in connection */
#endif
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

/* session exit status definitions. */

# define OBEYTERM_DISPLAY	0	/* obey terminateServer resource */
# define RESTART_DISPLAY	1	/* force session restart */
# define ABORT_DISPLAY		2	/* force server restart */
# define DISABLE_DISPLAY	3	/* unmanage this display */

/* display manager exit status definitions */

# define OBEYSESS_DISPLAY	0	/* obey multipleSessions resource */
# define REMANAGE_DISPLAY	1	/* force remanage */
# define UNMANAGE_DISPLAY	2	/* force deletion */

extern char	*servers;
extern int	request_port;
extern char	*errorLogFile;
extern char	*validProgramsFile;

extern struct display	*FindDisplayByName (),
			*FindDisplayByPid (),
			*NewDisplay ();

extern char	*malloc (), *realloc (), *strcpy ();

#ifdef UDP_SOCKET

# define START		"START"
# define TERMINATE	"TERMINATE"
# define RESTART	"RESTART"

# define POLL_PROVIDERS "POLL"
# define ADVERTISE	"ADVERTISE"

#endif
