/*
 * dm.h
 *
 * public interfaces for greet/verify functionality
 */

# include	<X11/Xos.h>

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

# define waitVal(w)	(waitSig(w) ? -1 : waitCode (w))

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
	int		terminateServer;/* restart for each session */
	int		multipleSessions;/* keep a session going */
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
			*NewDisplay (),
			*ReadDisplay ();

extern char	*malloc (), *realloc (), *strcpy ();

#ifdef UDP_SOCKET

# define POLL_PROVIDERS "polling for available display managers\n"
# define ADVERTISE	"advertising a display manager\n"

#endif
