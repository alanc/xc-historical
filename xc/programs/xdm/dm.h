/*
 * dm.h
 *
 * public interfaces for greet/verify functionality
 */

typedef enum displayStatus { running, notRunning } DisplayStatus;

struct display {
	struct display	*next;
	char		*name;		/* DISPLAY name */
	char		**argv;		/* program name and arguments */
	DisplayStatus	status;		/* current status */
	int		pid;		/* process id of child */
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

extern char	*session;
extern char	*errorLogFile;

# define POLL_PROVIDERS "polling for available display managers\n"
# define ADVERTISE	"advertising a display manager\n"
