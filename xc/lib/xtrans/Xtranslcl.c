/*
 *
 * The connection code/ideas in lib/X and server/os for SVR4/Intel 
 * environments was contributed by the following companies/groups:
 *
 *	MetroLink Inc
 *	NCR
 *	Pittsburgh Powercomputing Corporation (PPc)/Quarterdeck Office Systems
 *	SGCS
 *	Unix System Laboratories (USL) / Novell
 *	XFree86
 *
 * The goal is to have common connection code among all SVR4/Intel vendors.
 *
 * ALL THE ABOVE COMPANIES DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS 
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, 
 * IN NO EVENT SHALL THESE COMPANIES * BE LIABLE FOR ANY SPECIAL, INDIRECT 
 * OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS 
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE 
 * OR PERFORMANCE OF THIS SOFTWARE.
 */
#include <errno.h>
#include <sys/signal.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#if 0
#include <sys/ptms.h> /* Maybe for SVR4 only?? */
#endif
#ifdef SVR4
#include <sys/filio.h>
#endif
#include <sys/stropts.h>
#include <sys/wait.h>

/*
 * The local transports should be treated the same as a UNIX domain socket
 * wrt authentication, etc. Because of this, we will use struct sockaddr_un
 * for the address format. This will simplify the code in other places like
 * The X Server.
 */
#include <sys/socket.h>
#include <sys/un.h>

/*
 * These functions actually implement the local connection mechanisms.
 */

/* Type Not Supported */
static int
TRANS(Fail)(ciptr, port)
XtransConnInfo	ciptr;
char		*port;
{
return -1;
}

/* PTS */

#ifdef SYSV
#define SIGNAL_T int
#else
#define SIGNAL_T void
#endif /* SYSV */
typedef SIGNAL_T (*PFV)();
extern PFV signal();

extern char *ptsname(int);

static void _dummy(sig)
int sig;
{
}

#define X_STREAMS_DIR	"/dev/X"
#define DEV_PTMX	"/dev/ptmx"
#define DEV_SPX		"/dev/spx"

#if defined(X11_t)
#define PTSNODENAME "/dev/X/server."
#define NAMEDNODENAME "/dev/X/Nserver."
/*
 * ICS and SCO are only defined for X11 since they are there for
 * backwards binary compatability only.
 */
#define X_ISC_DIR	"/dev/X"
#define ISCDEVNODENAME	"/dev/X/ICSCONN/X%s"
#define ISCTMPNODENAME	"/tmp/.X11-unix/X%s"
#define SCORNODENAME	"/dev/X/%1sR"
#define SCOSNODENAME	"/dev/X/%1sS"
#endif
#if defined(XIM_t)
#define PTSNODENAME	"/dev/X/XIM."
#define NAMEDNODENAME	"/dev/X/NXIM."
#endif
#if defined(FS_t)
/*
 * USL has already defined something here. We need to check with them
 * and see if their choice is usable here.
 */
#define PTSNODENAME	"/dev/X/fontserver."
#define NAMEDNODENAME	"/dev/X/Nfontserver."
#endif
#if defined(ICE_t)
#define PTSNODENAME	"/dev/X/ICE."
#define NAMEDNODENAME	"/dev/X/NICE."
#endif
#if defined(TEST_t)
#define PTSNODENAME	"/dev/X/transtest."
#define NAMEDNODENAME	"/dev/X/Ntranstest."
#endif

static int
TRANS(PTSOpenClient)(ciptr, port)
XtransConnInfo	ciptr;
char		*port;
{
int			fd,server,exitval,alarm_time,ret;
char			server_path[64];
char			*slave, namelen;
char			buf[20]; /* MAX_PATH_LEN?? */
PFV			savef;
struct sockaddr_un	*sunaddr;

PRMSG(2,"TRANS(PTSOpenClient)(%s)\n", port, 0,0 );

#if !defined(PTSNODENAME)
PRMSG(1,"Protocol is not supported by a pts connection\n", 0,0,0);
return -1;
#else
if (port && *port ) {
	if( *port == '/' ) { /* A full pathname */
		(void) sprintf(server_path, "%s", port);
		} else {
		(void) sprintf(server_path, "%s%s", PTSNODENAME, port);
		}
	} else {
	(void) sprintf(server_path, "%s%d", PTSNODENAME, getpid());
	}
#endif /* !PTSNODENAME */

/*
 * Open the node the on which the server is listening.
 */

if ((server = open (server_path, O_RDWR)) < 0) {
	PRMSG(1,"TRANS(PTSOpenClient)() failed to open %s\n", server_path, 0,0);
	return -1;
	}

/*
 * Open the streams based pipe that will be this connection.
 */

if ((fd = open("/dev/ptmx", O_RDWR)) < 0) {
	PRMSG(1,"TRANS(PTSOpenClient)() failed to open /dev/ptmx\n", 0,0,0);
	return -1;
	}

(void) grantpt(fd);
(void) unlockpt(fd);

slave = ptsname(fd); /* get name */

if( slave == NULL ) {
	PRMSG(1,"TRANS(PTSOpenClient)() failed to get ptsname()\n", 0,0,0);
	close(fd);
	close(server);
	return -1;
	}

/*
 * This is neccesary for the case where a program is setuid to non-root.
 * grantpt() calls /usr/lib/pt_chmod which is set-uid root. This program will
 * set the owner of the pt device incorrectly if the uid is not restored
 * before it is called. The problem is that once it gets restored, it
 * cannot be changed back to its original condition, hence the fork().
 */

if( !fork()) {
	uid_t       saved_euid;

	saved_euid = geteuid();
	setuid( getuid() ); /** sets the euid to the actual/real uid **/
	if( chown( slave, saved_euid, -1 ) < 0 ) {
		exit( 1 );
		}

	exit( 0 );
	}

wait( &exitval );

if (chmod(slave, 0666) < 0) {
	close(fd);
	close(server);
	PRMSG(1,"Cannot chmod %s\n", slave, 0,0);
	return(-1);
	}

/*
 * write slave name to server
 */

namelen = strlen(slave);
buf[0] = namelen;
(void) sprintf(&buf[1], slave);
(void) write(server, buf, namelen+1);
(void) close(server);

/*
 * wait for server to respond
 */

savef = signal(SIGALRM, _dummy);
alarm_time = alarm (30); /* CONNECT_TIMEOUT */

ret = read(fd, buf, 1);

(void) alarm(alarm_time);
(void) signal(SIGALRM, savef);

if (ret != 1) {
	PRMSG(1,
	"TRANS(PTSOpenClient)() failed to get acknoledgement from server\n",
									0,0,0);
	(void) close(fd);
	fd = -1;
	}

/*
 * Everything looks good: fill in the XtransConnInfo structure.
 */

ciptr->family=AF_UNIX;
ciptr->addrlen=sizeof(struct sockaddr_un);
if( (sunaddr=(struct sockaddr_un *)malloc(ciptr->addrlen)) == NULL )
	{
	PRMSG(1,"TRANS(PTSOpenClient)() failed to allocate memory for addr\n",
									0,0,0);
	close(fd);
	return -1;
	}

sunaddr->sun_family=AF_UNIX;
strcpy(sunaddr->sun_path,slave);
#ifdef BSD44SOCKETS
sunaddr->sun_len=strlen(sunaddr->sun_path);
#endif

ciptr->addr=(char *)sunaddr;

ciptr->peeraddrlen=sizeof(struct sockaddr_un);
if( (sunaddr=(struct sockaddr_un *)malloc(ciptr->peeraddrlen)) == NULL )
	{
	PRMSG(1,
	   "TRANS(PTSOpenClient)() failed to allocate memory for peer addr\n",
									0,0,0);
	close(fd);
	return -1;
	}

sunaddr->sun_family=AF_UNIX;
strcpy(sunaddr->sun_path,server_path);
#ifdef BSD44SOCKETS
sunaddr->sun_len=strlen(sunaddr->sun_path);
#endif

ciptr->peeraddr=(char *)sunaddr;

return(fd);
}

static int
TRANS(PTSOpenServer)(ciptr, port)
XtransConnInfo	ciptr;
char		*port;
{
int fd, server;
char server_path[64], *slave;
struct	sockaddr_un	*sunaddr;

PRMSG(2,"TRANS(PTSOpenServer)(%s)\n", port, 0,0 );

#if !defined(PTSNODENAME)
PRMSG(1,"Protocol is not supported by a pts connection\n", 0,0,0);
return -1;
#else
if (port && *port ) {
	if( *port == '/' ) { /* A full pathname */
		(void) sprintf(server_path, "%s", port);
		} else {
		(void) sprintf(server_path, "%s%s", PTSNODENAME, port);
		}
	} else {
	(void) sprintf(server_path, "%s%d", PTSNODENAME, getpid());
	}
#endif /* !PTSNODENAME */

mkdir(X_STREAMS_DIR, 0777);
chmod(X_STREAMS_DIR, 0777);

if( (fd=open(server_path, O_RDWR)) >= 0 ) {
	PRMSG(1, "An X server is already running for display %s\n", port, 0,0 );
	PRMSG(1, "Remove %s if this is incorrect.\n", server_path, 0,0 );
	close(fd);
	return(-1);
	}

unlink(server_path);

if( (fd=open(DEV_PTMX, O_RDWR)) < 0) {
	PRMSG(1, "Unable to open %s\n", DEV_PTMX, 0,0 );
	return(-1);
	}

grantpt(fd);
unlockpt(fd);

if( (slave=ptsname(fd)) == NULL) {
	PRMSG(1, "Unable to get slave device name\n", 0,0,0 );
	close(fd);
	return(-1);
	}

if( link(slave,server_path) < 0 ) {
	PRMSG(1, "Unable to link %s to %s\n", slave, server_path,0 );
	close(fd);
	return(-1);
	}

if( chmod(server_path, 0666) < 0 ) {
	PRMSG(1, "Unable to chmod %s to 0666\n", server_path,0,0 );
	close(fd);
	return(-1);
	}

if( (server=open(server_path, O_RDWR)) < 0 ) {
	PRMSG(1, "Unable to open server device %s\n", server_path,0,0 );
	close(fd);
	return(-1);
	}

close(server);

/*
 * Everything looks good: fill in the XtransConnInfo structure.
 */

ciptr->family=AF_UNIX;
ciptr->addrlen=sizeof(struct sockaddr_un);
if( (sunaddr=(struct sockaddr_un *)malloc(ciptr->addrlen)) == NULL )
	{
	PRMSG(1,"TRANS(PTSOpenServer)() failed to allocate memory for addr\n",
									0,0,0);
	close(fd);
	return -1;
	}

sunaddr->sun_family=AF_UNIX;
strcpy(sunaddr->sun_path,server_path);
#ifdef BSD44SOCKETS
sunaddr->sun_len=strlen(sunaddr->sun_path);
#endif

ciptr->addr=(char *)sunaddr;

return fd;
}

static int
TRANS(PTSAccept)(ciptr, newciptr)
XtransConnInfo	ciptr;
XtransConnInfo	newciptr;
{
int		newfd;
int		in;
unsigned char	length;
char		buf[256];
struct	sockaddr_un	*sunaddr;

PRMSG(2,"TRANS(PTSAccept)(%x->%d)\n",ciptr,ciptr->fd,0);

if( (in=read(ciptr->fd,&length,1)) <= 0 ){
	if( !in ) {
		PRMSG(1,
		"TRANS(PTSAccept)() Incoming connection closed\n",0,0,0);
		}
	else {
		PRMSG(1,
	"TRANS(PTSAccept)() Error reading incoming connection. errno=%d \n",
								errno,0,0);
		}
	return -1;
	}

if( (in=read(ciptr->fd,buf,length)) <= 0 ){
	if( !in ) {
		PRMSG(1,
		"TRANS(PTSAccept)() Incoming connection closed\n",0,0,0);
		}
	else {
		PRMSG(1,
"TRANS(PTSAccept)() Error reading device name for new connection. errno=%d \n",
								errno,0,0);
		}
	return -1;
	}

buf[length] = '\0';

if( (newfd=open(buf,O_RDWR)) < 0 ) {
	PRMSG(1, "TRANS(PTSAccept)() Failed to open %s\n",buf,0,0);
	return -1;
	}

write(newfd,"1",1);

/*
 * Everything looks good: fill in the XtransConnInfo structure.
 */

newciptr->addrlen=ciptr->addrlen;
if( (newciptr->addr=(char *)malloc(newciptr->addrlen)) == NULL ) {
	PRMSG(1,"TRANS(PTSAccept)() failed to allocate memory for peer addr\n",
									0,0,0);
	close(newfd);
	return -1;
	}

memcpy(newciptr->addr,ciptr->addr,newciptr->addrlen);

newciptr->peeraddrlen=sizeof(struct sockaddr_un);
if( (sunaddr=(struct sockaddr_un *)malloc(newciptr->peeraddrlen)) == NULL ) {
	PRMSG(1,"TRANS(PTSAccept)() failed to allocate memory for peer addr\n",
									0,0,0);
	free(newciptr->addr);
	close(newfd);
	return -1;
	}

sunaddr->sun_family=AF_UNIX;
strcpy(sunaddr->sun_path,buf);
#ifdef BSD44SOCKETS
sunaddr->sun_len=strlen(sunaddr->sun_path);
#endif

newciptr->peeraddr=(char *)sunaddr;

return newfd;
}

#ifdef SVR4
/* NAMED */
static int
TRANS(NAMEDOpenClient)(ciptr, port)
XtransConnInfo	ciptr;
char		*port;
{
int			fd;
char			server_path[64];
struct stat		filestat;
struct sockaddr_un	*sunaddr;
extern int		isastream();

PRMSG(2,"TRANS(NAMEDOpenClient)(%s)\n", port, 0,0 );

#if !defined(NAMEDNODENAME)
PRMSG(1,"Protocol is not supported by a NAMED connection\n", 0,0,0);
return -1;
#else
if ( port && *port ) {
	if( *port == '/' ) { /* A full pathname */
		(void) sprintf(server_path, "%s", port);
		} else {
		(void) sprintf(server_path, "%s%s", NAMEDNODENAME, port);
		}
	} else {
	(void) sprintf(server_path, "%s%d", NAMEDNODENAME, getpid());
	}
#endif

if (stat(server_path, &filestat) < 0 ) {
	PRMSG(1,"No device %s for NAMED connection\n", server_path, 0,0 );
	return -1;
	}

if ((filestat.st_mode & S_IFMT) != S_IFIFO) {
	PRMSG(1,"Device %s is not a FIFO\n", server_path, 0,0 );
	/* Is this really a failure? */
	return -1;
	}

if ((fd = open(server_path, O_RDWR)) < 0) {
	PRMSG(1,"Cannot open %s for NAMED connection\n", server_path, 0,0 );
	return -1;
	}

#if defined(I_BIGPIPE)
ioctl( fd, I_BIGPIPE, &str );
#endif /* I_BIGPIPE */

if (isastream(fd) <= 0) {
	PRMSG(1,"%s is not a streams device\n", server_path, 0,0 );
	(void) close(fd);
	return -1;
	}

/*
 * Everything looks good: fill in the XtransConnInfo structure.
 */

ciptr->family=AF_UNIX;
ciptr->addrlen=sizeof(struct sockaddr_un);
if( (sunaddr=(struct sockaddr_un *)malloc(ciptr->addrlen)) == NULL ) {
	PRMSG(1,"TRANS(NAMEDOpenClient)() failed to allocate memory for addr\n",
									0,0,0);
	close(fd);
	return -1;
	}

sunaddr->sun_family=AF_UNIX;
strcpy(sunaddr->sun_path,server_path);
#ifdef BSD44SOCKETS
sunaddr->sun_len=strlen(sunaddr->sun_path);
#endif

ciptr->addr=(char *)sunaddr;

ciptr->peeraddrlen=sizeof(struct sockaddr_un);
if( (sunaddr=(struct sockaddr_un *)malloc(ciptr->peeraddrlen)) == NULL ) {
	PRMSG(1,
	   "TRANS(NAMEDOpenClient)() failed to allocate memory for peer addr\n",
									0,0,0);
	close(fd);
	return -1;
	}

sunaddr->sun_family=AF_UNIX;
strcpy(sunaddr->sun_path,server_path);
#ifdef BSD44SOCKETS
sunaddr->sun_len=strlen(sunaddr->sun_path);
#endif

ciptr->peeraddr=(char *)sunaddr;

return(fd);
}

static int
TRANS(NAMEDOpenServer)(ciptr, port)
XtransConnInfo	ciptr;
char		*port;
{
int			fd, pipefd[2];
char			server_path[64];
struct stat		sbuf;
struct sockaddr_un	*sunaddr;

PRMSG(2,"TRANS(NAMEDOpenServer)(%s)\n", port, 0,0 );

#if !defined(NAMEDNODENAME)
PRMSG(1,"Protocol is not supported by a NAMED connection\n", 0,0,0);
return -1;
#else
if ( port && *port ) {
	if( *port == '/' ) { /* A full pathname */
		(void) sprintf(server_path, "%s", port);
		} else {
		(void) sprintf(server_path, "%s%s", NAMEDNODENAME, port);
		}
	} else {
	(void) sprintf(server_path, "%s%d", NAMEDNODENAME, getpid());
	}
#endif /* !NAMEDNODENAME */

mkdir(X_STREAMS_DIR, 0777);
chmod(X_STREAMS_DIR, 0777);

if(stat(server_path, &sbuf) != 0) {
	if (errno == ENOENT) {
		if ((fd = creat(server_path, (mode_t)0666)) == -1) {
			PRMSG(1, "Can't open %s\n", server_path, 0,0 );
			return(-1);
			}
		close(fd);
		if (chmod(server_path, (mode_t)0666) < 0) {
			PRMSG(1, "Can't open %s\n", server_path, 0,0 );
			return(-1);
			}
		} else {
			PRMSG(1, "stat on %s failed\n", server_path, 0,0 );
			return(-1);
			}
	}

if( pipe(pipefd) != 0) {
	PRMSG(1, "pipe() failed, errno=%d\n",errno, 0,0 );
	return(-1);
	}

if( ioctl(pipefd[0], I_PUSH, "connld") != 0) {
	PRMSG(1, "ioctl(I_PUSH,\"connld\") failed, errno=%d\n",errno, 0,0 );
	close(pipefd[0]);
	close(pipefd[1]);
	return(-1);
	}

if( fattach(pipefd[0], server_path) != 0) {
	PRMSG(1, "fattach(%s) failed, errno=%d\n", server_path,errno, 0 );
	close(pipefd[0]);
	close(pipefd[1]);
	return(-1);
	}

/*
 * Everything looks good: fill in the XtransConnInfo structure.
 */

ciptr->family=AF_UNIX;
ciptr->addrlen=sizeof(struct sockaddr_un);
if( (sunaddr=(struct sockaddr_un *)malloc(ciptr->addrlen)) == NULL ) {
	PRMSG(1,"TRANS(NAMEDOpenClient)() failed to allocate memory for addr\n",
									0,0,0);
	close(fd);
	return -1;
	}

sunaddr->sun_family=AF_UNIX;
strcpy(sunaddr->sun_path,server_path);
#ifdef BSD44SOCKETS
sunaddr->sun_len=strlen(sunaddr->sun_path);
#endif

ciptr->addr=(char *)sunaddr;

ciptr->peeraddrlen=sizeof(struct sockaddr_un);
if( (sunaddr=(struct sockaddr_un *)malloc(ciptr->peeraddrlen)) == NULL )
	{
	PRMSG(1,
	   "TRANS(NAMEDOpenClient)() failed to allocate memory for peer addr\n",
									0,0,0);
	close(fd);
	return -1;
	}

sunaddr->sun_family=AF_UNIX;
strcpy(sunaddr->sun_path,server_path);
#ifdef BSD44SOCKETS
sunaddr->sun_len=strlen(sunaddr->sun_path);
#endif

ciptr->peeraddr=(char *)sunaddr;

return(pipefd[1]);
}

static int
TRANS(NAMEDAccept)(ciptr, newciptr)
XtransConnInfo	ciptr;
XtransConnInfo	newciptr;
{
struct strrecvfd str;

PRMSG(2,"TRANS(NAMEDAccept)(%x->%d)\n", ciptr, ciptr->fd, 0 );

if( ioctl(ciptr->fd, I_RECVFD, &str ) < 0 ) {
	PRMSG(1, "ioctl(I_RECVFD) failed, errno=%d\n", errno, 0,0 );
	return(-1);
	}

/*
 * Everything looks good: fill in the XtransConnInfo structure.
 */

newciptr->addrlen=ciptr->addrlen;
if( (newciptr->addr=(char *)malloc(newciptr->addrlen)) == NULL ) {
	PRMSG(1,
	"TRANS(NAMEDAccept)() failed to allocate memory for peer addr\n",
									0,0,0);
	close(str.fd);
	return -1;
	}

memcpy(newciptr->addr,ciptr->addr,newciptr->addrlen);

newciptr->peeraddrlen=newciptr->addrlen;
if( (newciptr->peeraddr=(char *)malloc(newciptr->peeraddrlen)) == NULL ) {
	PRMSG(1,
	"TRANS(NAMEDAccept)() failed to allocate memory for peer addr\n",
									0,0,0);
	free(newciptr->addr);
	close(str.fd);
	return -1;
	}

memcpy(newciptr->peeraddr,newciptr->addr,newciptr->peeraddrlen);

return str.fd;
}
#endif /* SVR4 */

/*
 * connect_spipe is used by both the SCO and ISC connection types.
 */
static int
connect_spipe(fd1, fd2)
int	fd1, fd2;
{
long temp;
struct strfdinsert sbuf;

sbuf.databuf.maxlen = -1;
sbuf.databuf.len = -1;
sbuf.databuf.buf = NULL;
sbuf.ctlbuf.maxlen = sizeof(long);
sbuf.ctlbuf.len = sizeof(long);
sbuf.ctlbuf.buf = (caddr_t)&temp;
sbuf.offset = 0;
sbuf.fildes = fd2;
sbuf.flags = 0;

if( ioctl(fd1, I_FDINSERT, &sbuf) < 0 )
	return(-1);

return(0);
}

/*
 * connect_spipe is used by both the SCO and ISC connection types.
 */
static int
named_spipe(fd, path)
int	fd;
char	*path;
{
int oldUmask, ret;
struct stat sbuf;

oldUmask = umask(0);

(void) fstat(fd, &sbuf);
ret = mknod(path, 0020666, sbuf.st_rdev);

umask(oldUmask);

if (ret < 0) {
	ret = -1;
} else {
	ret = fd;
}

return(ret);
}


/* ISC */

static int
TRANS(ISCOpenClient)(ciptr, port)
XtransConnInfo	ciptr;
char		*port;
{
int	fd,fds,server;
char	server_path[64];
char	server_dev_path[64];
struct strfdinsert buf;
long	temp;
o_mode_t spmode;
struct stat filestat;
struct sockaddr_un	*sunaddr;

PRMSG(2,"TRANS(ICSOpenClient)(%s)\n", port, 0,0 );

#if !defined(ISCDEVNODENAME)
PRMSG(1,"Protocol is not supported by a ISC connection\n", 0,0,0);
return -1;
#else
(void) sprintf(server_path, ISCTMPNODENAME, port);
(void) sprintf(server_dev_path, ISCDEVNODENAME, port);
#endif /* !ISCDEVNODE */

fd = fds = server = -1;

if (stat(DEV_SPX, &filestat) == -1) {
	PRMSG(1, "stat(%s) failed, errno=%d\n", DEV_SPX, errno, 0 );
	return(-1);
	}

spmode = (filestat.st_mode & S_IFMT);

if (stat(server_path, &filestat) != -1) {
	if ((filestat.st_mode & S_IFMT) == spmode) {
		if ((server = open(server_path, O_RDWR)) < 0) {
			PRMSG(1,"TRANS(ICSOpenClient): failed to open %s\n",
							server_path, 0,0 );
		}
	}
}

if (server < 0) {
	/* try the alternate path */
	if (stat(server_dev_path, &filestat) != -1) {
		if ((filestat.st_mode & S_IFMT) == spmode) {
			if ((server = open(server_dev_path, O_RDWR)) < 0) {
			PRMSG(1,"TRANS(ICSOpenClient): failed to open %s\n",
							server_dev_path, 0,0 );
			}
		}
	}
}

if (server < 0) {
	PRMSG(1,"TRANS(ICSOpenClient): can't open either device %s or %s\n",
					server_path, server_dev_path, 0 );
	return -1;
}

if ((fds = open(DEV_SPX, O_RDWR)) < 0 ||
    (fd  = open(DEV_SPX, O_RDWR)) < 0) {
	/* Failed to open all of the devices */
	PRMSG(1,"TRANS(ICSOpenClient): can't open %s\n", DEV_SPX, 0,0 );
	(void) close(server);
	if (fds != -1)
		(void) close(fds);
	if (fd != -1)
		(void) close(fd);
	return -1;
}

/* make a STREAMS-pipe */

buf.databuf.maxlen = -1;
buf.databuf.len = -1;
buf.databuf.buf = NULL;
buf.ctlbuf.maxlen = sizeof(long);
buf.ctlbuf.len = sizeof(long);
buf.ctlbuf.buf = (caddr_t)&temp;
buf.offset = 0;
buf.fildes = fd;
buf.flags = 0;

if (ioctl(fds, I_FDINSERT, &buf) < 0 ||
    ioctl(server, I_SENDFD, fds) < 0) {
	PRMSG(1,"TRANS(ICSOpenClient): ioctl(I_FDINSERT or I_SENDFD) failed\n",
								0,0,0 );
	(void) close(server);
	(void) close(fds);
	(void) close(fd);
	return -1;
}

/*
 * Everything looks good: fill in the XtransConnInfo structure.
 */

ciptr->family=AF_UNIX;
ciptr->addrlen=sizeof(struct sockaddr_un);
if( (sunaddr=(struct sockaddr_un *)malloc(ciptr->addrlen)) == NULL ) {
	PRMSG(1,"TRANS(NAMEDOpenClient)() failed to allocate memory for addr\n",
									0,0,0);
	close(fd);
	return -1;
	}

sunaddr->sun_family=AF_UNIX;
strcpy(sunaddr->sun_path,server_path);
#ifdef BSD44SOCKETS
sunaddr->sun_len=strlen(sunaddr->sun_path);
#endif

ciptr->addr=(char *)sunaddr;

ciptr->peeraddrlen=sizeof(struct sockaddr_un);
if( (sunaddr=(struct sockaddr_un *)malloc(ciptr->peeraddrlen)) == NULL )
	{
	PRMSG(1,
	   "TRANS(NAMEDOpenClient)() failed to allocate memory for peer addr\n",
									0,0,0);
	close(fd);
	return -1;
	}

sunaddr->sun_family=AF_UNIX;
strcpy(sunaddr->sun_path,server_path);
#ifdef BSD44SOCKETS
sunaddr->sun_len=strlen(sunaddr->sun_path);
#endif

ciptr->peeraddr=(char *)sunaddr;

return (fd);
}

static int
TRANS(ISCOpenServer)(ciptr, port)
XtransConnInfo	ciptr;
char		*port;
{
int	fd = -1,fds = -1;
char	server_path[64],server_unix_path[64];
struct sockaddr_un	*sunaddr;

PRMSG(2,"TRANS(ISCOpenServer)(%s)\n", port, 0,0 );

#if !defined(ISCDEVNODENAME)
PRMSG(1,"Protocol is not supported by a ISC connection\n", 0,0,0);
return -1;
#else
(void) sprintf(server_path, ISCDEVNODENAME, port);
(void) sprintf(server_unix_path, ISCTMPNODENAME, port);

mkdir(X_STREAMS_DIR, 0777); /* "/dev/X" */
chmod(X_STREAMS_DIR, 0777);
mkdir(X_ISC_DIR, 0777); /* "/dev/X/ISCCONN" */
chmod(X_ISC_DIR, 0777);
#endif /* !ISCDEVNODE */

unlink(server_path);

if( ((fds=open(DEV_SPX, O_RDWR)) < 0) ||
    ((fd =open(DEV_SPX, O_RDWR)) < 0)) {
	PRMSG(1,"TRANS(ISCOpenServer): failed to open %s\n", DEV_SPX, 0,0 );
	return -1;
}

if( (connect_spipe(fds, fd) < 0) ||
    (named_spipe(fds, server_path) < 0)) {
	PRMSG(1,"TRANS(ISCOpenServer): failed connect pipes\n", 0,0,0 );
	close(fd);
	close(fds);
	return -1;
}

#if !defined(UNIXCONN)
/*
 * If the UNIX Domain socket transport is now being used, then link this
 * device to the path /tmp/.X11-unix/X path.
 */
mkdir(X_UNIX_DIR, 0777); /* "/tmp/.X11-unix/X" */
chmod(X_UNIX_DIR, 0777);

unlink(server_unix_path);

#ifdef SVR4
/* we prefer symbolic links because hard links can't cross file systems */
if( symlink(server_path, server_unix_path) < 0 )
	PRMSG(1,"TRANS(ISCOpenServer): failed to link %s to %s\n",
					server_path, server_unix_path, 0 );
	/*
	 * Don't make this failure fatal since the listener 
	 * is already established, and this just for compatability
	 */
#else
if( link(server_path, server_unix_path) < 0 )
	PRMSG(1,"TRANS(ISCOpenServer): failed to link %s to %s\n",
					server_path, server_unix_path, 0 );
	/*
	 * Don't make this failure fatal since the listener 
	 * is already established, and this just for compatability
	 */
#endif /* SVR4 */
#endif /* !UNIXCONN */
 
return fd;
}

static int
TRANS(ISCAccept)(ciptr, newciptr)
XtransConnInfo	ciptr;
XtransConnInfo	newciptr;
{
struct strrecvfd str;

PRMSG(2,"TRANS(ISCAccept)(%d)\n", ciptr->fd, 0,0 );

while (ioctl(ciptr->fd, I_RECVFD, &str) < 0) {
	if (errno != EAGAIN) {
		PRMSG(1,"TRANS(ISCAccept): Can't read fildes", 0,0,0 );
		return(-1);
	}
}

/*
 * Everything looks good: fill in the XtransConnInfo structure.
 */

newciptr->addrlen=ciptr->addrlen;
if( (newciptr->addr=(char *)malloc(newciptr->addrlen)) == NULL ) {
	PRMSG(1,
	"TRANS(ISCAccept)() failed to allocate memory for peer addr\n",
									0,0,0);
	close(str.fd);
	return -1;
	}

memcpy(newciptr->addr,ciptr->addr,newciptr->addrlen);

newciptr->peeraddrlen=newciptr->addrlen;
if( (newciptr->peeraddr=(char *)malloc(newciptr->peeraddrlen)) == NULL ) {
	PRMSG(1,
	"TRANS(ISCAccept)() failed to allocate memory for peer addr\n",
									0,0,0);
	free(newciptr->addr);
	close(str.fd);
	return -1;
	}

memcpy(newciptr->peeraddr,newciptr->addr,newciptr->peeraddrlen);

return(str.fd);
}

/* SCO */
static int
TRANS(SCOOpenClient)(ciptr, port)
XtransConnInfo	ciptr;
char		*port;
{
int		fd, server, fl, ret;
char		server_path[64];
struct strbuf	ctlbuf;
unsigned long	alarm_time;
void		(*savef)();
long		temp;
struct sockaddr_un	*sunaddr;
extern int	getmsg(), putmsg();

PRMSG(2,"TRANS(SCOOpenClient)(%s)\n", port, 0,0 );

#if !defined(SCORNODENAME)
PRMSG(1,"Protocol is not supported by a SCO connection\n", 0,0,0);
return -1;
#else
(void) sprintf(server_path, SCORNODENAME, port);
#endif

if ((server = open(server_path, O_RDWR)) < 0) {
	PRMSG(1,"TRANS(SCOOpenClient) failed to open %s\n", server_path, 0,0 );
	return -1;
	}

if ((fd = open(DEV_SPX, O_RDWR)) >= 0) {
	PRMSG(1,"TRANS(SCOOpenClient) failed to open %s\n", DEV_SPX, 0,0 );
	close(server);
	return -1;
	}
	
(void) write(server, &server, 1);
ctlbuf.len = 0;
ctlbuf.maxlen = sizeof(long);
ctlbuf.buf = (caddr_t)&temp;
fl = 0;

savef = signal(SIGALRM, _dummy);
alarm_time = alarm(10);

ret = getmsg(server, &ctlbuf, 0, &fl);

(void) alarm(alarm_time);
(void) signal(SIGALRM, savef);

if (ret < 0) {
	PRMSG(1,"TRANS(SCOOpenClient) error from getmsg\n", 0,0,0 );
	close(fd);
	close(server);
	return -1;
	}

/* The msg we got via getmsg is the result of an
 * I_FDINSERT, so if we do a putmsg with whatever
 * we recieved, we're doing another I_FDINSERT ...
 */
(void) putmsg(fd, &ctlbuf, 0, 0);
(void) fcntl(fd,F_SETFL,fcntl(fd,F_GETFL,0)|O_NDELAY);

(void) close(server);

/*
 * Everything looks good: fill in the XtransConnInfo structure.
 */

ciptr->family=AF_UNIX;
ciptr->addrlen=sizeof(struct sockaddr_un);
if( (sunaddr=(struct sockaddr_un *)malloc(ciptr->addrlen)) == NULL ) {
	PRMSG(1,"TRANS(SCOOpenClient)() failed to allocate memory for addr\n",
									0,0,0);
	close(fd);
	return -1;
	}

sunaddr->sun_family=AF_UNIX;
strcpy(sunaddr->sun_path,server_path);
#ifdef BSD44SOCKETS
sunaddr->sun_len=strlen(sunaddr->sun_path);
#endif

ciptr->addr=(char *)sunaddr;

ciptr->peeraddrlen=sizeof(struct sockaddr_un);
if( (sunaddr=(struct sockaddr_un *)malloc(ciptr->peeraddrlen)) == NULL )
	{
	PRMSG(1,
	   "TRANS(SCOOpenClient)() failed to allocate memory for peer addr\n",
									0,0,0);
	close(fd);
	return -1;
	}

sunaddr->sun_family=AF_UNIX;
strcpy(sunaddr->sun_path,server_path);
#ifdef BSD44SOCKETS
sunaddr->sun_len=strlen(sunaddr->sun_path);
#endif

ciptr->peeraddr=(char *)sunaddr;

return(fd);
}

static int
TRANS(SCOOpenServer)(ciptr, port)
XtransConnInfo	ciptr;
char		*port;
{
char		serverR_path[64];
char		serverS_path[64];
int	fdr = -1;
int	fds = -1;
struct sockaddr_un	*sunaddr;

PRMSG(2,"TRANS(SCOOpenServer)(%s)\n", port, 0,0 );

#if !defined(SCORNODENAME)
PRMSG(1,"Protocol is not supported by a SCO connection\n", 0,0,0);
return -1;
#else
(void) sprintf(serverR_path, SCORNODENAME, port);
(void) sprintf(serverS_path, SCOSNODENAME, port);
#endif /* SCORNODENAME */

unlink(serverR_path);
unlink(serverS_path);

if ((fds = open(DEV_SPX, O_RDWR)) < 0 ||
    (fdr = open(DEV_SPX, O_RDWR)) < 0 ) {
	PRMSG(2,"TRANS(SCOOpenServer) failed to open %s\n", DEV_SPX, 0,0 );
	return -1;
}

if (connect_spipe(fds, fdr) != -1 &&
    named_spipe(fds, serverS_path) != -1 &&
    named_spipe(fdr, serverR_path) != -1) {
	PRMSG(2,"TRANS(SCOOpenServer) connect pipes\n", 0,0,0 );
	close(fds);
	close(fdr);
	return -1;
}
	
/*
 * Everything looks good: fill in the XtransConnInfo structure.
 */

ciptr->family=AF_UNIX;
ciptr->addrlen=sizeof(struct sockaddr_un);
if( (sunaddr=(struct sockaddr_un *)malloc(ciptr->addrlen)) == NULL ) {
	PRMSG(1,"TRANS(SCOOpenClient)() failed to allocate memory for addr\n",
									0,0,0);
	close(fds);
	close(fdr);
	return -1;
	}

sunaddr->sun_family=AF_UNIX;
strcpy(sunaddr->sun_path,serverS_path);
#ifdef BSD44SOCKETS
sunaddr->sun_len=strlen(sunaddr->sun_path);
#endif

ciptr->addr=(char *)sunaddr;

ciptr->peeraddrlen=sizeof(struct sockaddr_un);
if( (sunaddr=(struct sockaddr_un *)malloc(ciptr->peeraddrlen)) == NULL )
	{
	PRMSG(1,
	   "TRANS(SCOOpenClient)() failed to allocate memory for peer addr\n",
									0,0,0);
	close(fds);
	close(fdr);
	return -1;
	}

sunaddr->sun_family=AF_UNIX;
strcpy(sunaddr->sun_path,serverR_path);
#ifdef BSD44SOCKETS
sunaddr->sun_len=strlen(sunaddr->sun_path);
#endif

ciptr->peeraddr=(char *)sunaddr;

return(fds);
}

static int
TRANS(SCOAccept)(ciptr, newciptr)
XtransConnInfo	ciptr;
XtransConnInfo	newciptr;
{
char	c;
int	fd;

PRMSG(2,"TRANS(SCOAccept)(%d)\n", ciptr->fd, 0,0 );

if (read(ciptr->fd, &c, 1) < 0) {
	PRMSG(1,"TRANS(SCOAccept): can't read from client",0,0,0);
	return(-1);
}

if( (fd = open(DEV_SPX, O_RDWR)) < 0 ) {
	PRMSG(1,"TRANS(SCOAccept)(): can't open \"%s\"",DEV_SPX, 0,0 );
	return(-1);
}

if (connect_spipe(ciptr->fd, fd) < 0) {
	PRMSG(1,"TRANS(SCOAccept)(): can't connect pipes", 0,0,0 );
	(void) close(fd);
	return(-1);
}

/*
 * Everything looks good: fill in the XtransConnInfo structure.
 */

newciptr->addrlen=ciptr->addrlen;
if( (newciptr->addr=(char *)malloc(newciptr->addrlen)) == NULL ) {
	PRMSG(1,
	"TRANS(SCOAccept)() failed to allocate memory for peer addr\n",
									0,0,0);
	close(fd);
	return -1;
	}

memcpy(newciptr->addr,ciptr->addr,newciptr->addrlen);

newciptr->peeraddrlen=newciptr->addrlen;
if( (newciptr->peeraddr=(char *)malloc(newciptr->peeraddrlen)) == NULL ) {
	PRMSG(1,
	"TRANS(SCOAccept)() failed to allocate memory for peer addr\n",
									0,0,0);
	free(newciptr->addr);
	close(fd);
	return -1;
	}

memcpy(newciptr->peeraddr,newciptr->addr,newciptr->peeraddrlen);

return(fd);
}

/*
 * This table contains all of the entry points for the different local
 * connection mechanisms.
 */

typedef struct _LOCALtrans2dev {
	char	*transname;
	int	(*devcotsopenclient)(XtransConnInfo ,char * /*port*/);
	int	(*devcotsopenserver)(XtransConnInfo ,char * /*port*/);
	int	(*devcltsopenclient)(XtransConnInfo ,char * /*port*/);
	int	(*devcltsopenserver)(XtransConnInfo ,char * /*port*/);
	int	(*devaccept)(XtransConnInfo,XtransConnInfo);
	} LOCALtrans2dev;

static LOCALtrans2dev LOCALtrans2devtab[] = {
{"",TRANS(PTSOpenClient),TRANS(PTSOpenServer),
		TRANS(Fail),TRANS(Fail),TRANS(PTSAccept)},
{"local",TRANS(PTSOpenClient),TRANS(PTSOpenServer),
		TRANS(Fail),TRANS(Fail),TRANS(PTSAccept)},
{"pts",TRANS(PTSOpenClient),TRANS(PTSOpenServer),
		TRANS(Fail),TRANS(Fail),TRANS(PTSAccept)},
#ifdef SVR4
{"named",TRANS(NAMEDOpenClient),TRANS(NAMEDOpenServer),
		TRANS(Fail),TRANS(Fail),TRANS(NAMEDAccept)},
#endif /* SVR4 */
{"isc",TRANS(ISCOpenClient),TRANS(ISCOpenServer),
		TRANS(Fail),TRANS(Fail),TRANS(ISCAccept)},
{"sco",TRANS(SCOOpenClient),TRANS(SCOOpenServer),
		TRANS(Fail),TRANS(Fail),TRANS(SCOAccept)},
};

#define NUMTRANSPORTS	(sizeof(LOCALtrans2devtab)/sizeof(LOCALtrans2dev))

static	char	*XLOCAL=NULL;
static	char	*workingXLOCAL=NULL;
static	char	*freeXLOCAL=NULL;

static void
TRANS(LocalInitTransports)(char *protocol)
{
PRMSG(3,"TRANS(LocalInitTransports)(%s)\n", protocol, 0,0 );

if( strcmp(protocol,"local") && strcmp(protocol,"LOCAL") )
{
	workingXLOCAL=freeXLOCAL=(char *)malloc (strlen (protocol) + 1);
	if (workingXLOCAL)
	    strcpy (workingXLOCAL, protocol);
    }
else {
	XLOCAL=(char *)getenv("XLOCAL");
	if(XLOCAL==NULL)
		XLOCAL="UNIX:PTS:NAMED:ISC:SCO";
	workingXLOCAL=freeXLOCAL=(char *)malloc (strlen (XLOCAL) + 1);
	if (workingXLOCAL)
	    strcpy (workingXLOCAL, XLOCAL);
	}
}

static void
TRANS(LocalEndTransports)()
{
PRMSG(3,"TRANS(LocalEndTransports)()\n", 0,0,0 );
free(freeXLOCAL);
}

static LOCALtrans2dev *
TRANS(LocalGetNextTransport)()
{
int	i,j;
char	*typetocheck;
#define TYPEBUFSIZE	32
char	typebuf[TYPEBUFSIZE];
PRMSG(3,"TRANS(LocalGetNextTransport)()\n", 0,0,0 );

while(1)
	{
	if( workingXLOCAL == NULL || *workingXLOCAL == '\0' )
		return NULL;

	typetocheck=workingXLOCAL;
	workingXLOCAL=strchr(workingXLOCAL,':');
	if(*workingXLOCAL)
		*workingXLOCAL++='\0';

	for(i=0;i<NUMTRANSPORTS;i++)
		{
		/*
		 * This is equivilent to a case insensitive strcmp(),
		 * but should be more portable.
		 */
		strncpy(typebuf,typetocheck,TYPEBUFSIZE);
		for(j=0;j<TYPEBUFSIZE;j++)
			typebuf[j]=tolower(typebuf[j]);

		/* Now, see if they match */
		if(!strcmp(LOCALtrans2devtab[i].transname,typebuf))
			return &LOCALtrans2devtab[i];
		}
	}
/*NOTREACHED*/
return NULL;
}

static XtransConnInfo
TRANS(LocalOpenClient)(int type, char *protocol, char *host, char *port)
{
int	fd = -1;
LOCALtrans2dev *transptr;
XtransConnInfo ciptr;

PRMSG(3,"TRANS(LocalOpenClient)()\n", 0,0,0 );

#if defined(X11_t)
/*
 * X has a well known port, that is transport dependant. It is easier
 * to handle it here, than try and come up with a transport independent
 * representation that can be passed in and resolved the usual way.
 *
 * The port that is passed here is really a string containing the idisplay
 * from ConnectDisplay(). Since that is what we want for the local transports,
 * we don't have to do anything special.
 */
#endif /* X11_t */

if( (ciptr=(XtransConnInfo)calloc(1,sizeof(struct _XtransConnInfo))) == NULL )
	{
	PRMSG(1,"TRANS(LocalOpenClient)() calloc(1,%d) failed\n",
					sizeof(struct _XtransConnInfo),0,0 );
	return NULL;
	}

ciptr->fd = -1;

TRANS(LocalInitTransports)(protocol);

for(transptr=TRANS(LocalGetNextTransport)();
	transptr!=NULL;transptr=TRANS(LocalGetNextTransport)())
	{
	switch( type )
       		{
       		case XTRANS_OPEN_COTS_CLIENT:
               		ciptr->fd=transptr->devcotsopenclient(ciptr,port);
               		break;
       		case XTRANS_OPEN_CLTS_CLIENT:
               		ciptr->fd=transptr->devcltsopenclient(ciptr,port);
               		break;
       		case XTRANS_OPEN_COTS_SERVER:
       		case XTRANS_OPEN_CLTS_SERVER:
        		PRMSG(1,
"TRANS(LocalOpenClient): Should not be opening a server with this function\n",
								0,0,0);
               		break;
       		default:
        		PRMSG(1,
			"TRANS(LocalOpenClient): Unknown Open type %d\n",
				type, 0,0 );
       		}
	if( ciptr->fd >= 0 )
		break;
       	}

TRANS(LocalEndTransports)();

if( ciptr->fd < 0 )
	{
	free(ciptr);
	return NULL;
	}

ciptr->priv=(char *)transptr;

return ciptr;
}

static XtransConnInfo
TRANS(LocalOpenServer)(int type, char *protocol, char *host, char *port)
{
int	i,fd = -1;
XtransConnInfo ciptr;

PRMSG(2,"TRANS(LocalOpenServer)(%d,%s,%s)\n", type, protocol, port);

#if defined(X11_t)
/*
 * For X11, the port will be in the format xserverN where N is the
 * display number. All of the local connections just need to know
 * the display number because they don't do any name resolution on
 * the port. This just truncates port to the display portion.
 */
#endif /* X11_t */

if( (ciptr=(XtransConnInfo)calloc(1,sizeof(struct _XtransConnInfo))) == NULL )
	{
	PRMSG(1,"TRANS(LocalOpenServer)() calloc(1,%d) failed\n",
					sizeof(struct _XtransConnInfo),0,0 );
	return NULL;
	}

for(i=1;i<NUMTRANSPORTS;i++)
	{
	if( strcmp(protocol,LOCALtrans2devtab[i].transname) != 0 )
		continue;
	switch( type )
       	    {
       	    case XTRANS_OPEN_COTS_CLIENT:
       	    case XTRANS_OPEN_CLTS_CLIENT:
        	PRMSG(1,
"TRANS(LocalOpenServer): Should not be opening a client with this function\n",
								0,0,0);
               	break;
       	    case XTRANS_OPEN_COTS_SERVER:
               	ciptr->fd=LOCALtrans2devtab[i].devcotsopenserver(ciptr,port);
               	break;
       	    case XTRANS_OPEN_CLTS_SERVER:
               	ciptr->fd=LOCALtrans2devtab[i].devcltsopenserver(ciptr,port);
               	break;
       	    default:
        	PRMSG(1,"TRANS(LocalOpenServer): Unknown Open type %d\n",
								type ,0,0);
       	    }
	if( ciptr->fd >= 0 ) {
		ciptr->priv=(char *)&LOCALtrans2devtab[i];
		ciptr->flags=1;
		return ciptr;
		}
       	}

free(ciptr);
return NULL;
}

/*
 * This is the Local implementation of the X Transport service layer
 */

static XtransConnInfo
TRANS(LocalOpenCOTSClient)(Xtransport *thistrans, char *protocol, char *host, char *port)
{
PRMSG(2,"TRANS(LocalOpenCOTSClient)(%s,%s,%s)\n",protocol,host,port);

return TRANS(LocalOpenClient)(XTRANS_OPEN_COTS_CLIENT, protocol, host, port);
}

static XtransConnInfo
TRANS(LocalOpenCOTSServer)(Xtransport *thistrans, char *protocol, char *host, char *port)
{
PRMSG(2,"TRANS(LocalOpenCOTSServer)(%s,%s,%s)\n",protocol,host,port);

return TRANS(LocalOpenServer)(XTRANS_OPEN_COTS_SERVER, protocol, host, port);
}

static XtransConnInfo
TRANS(LocalOpenCLTSClient)(Xtransport *thistrans, char *protocol, char *host, char *port)
{
PRMSG(2,"TRANS(LocalOpenCLTSClient)(%s,%s,%s)\n",protocol,host,port);

return TRANS(LocalOpenClient)(XTRANS_OPEN_CLTS_CLIENT, protocol, host, port);
}

static XtransConnInfo
TRANS(LocalOpenCLTSServer)(Xtransport *thistrans, char *protocol, char *host, char *port)
{
PRMSG(2,"TRANS(LocalOpenCLTSServer)(%s,%s,%s)\n",protocol,host,port);

return TRANS(LocalOpenServer)(XTRANS_OPEN_CLTS_SERVER, protocol, host, port);
}

static
TRANS(LocalSetOption)(XtransConnInfo ciptr, int option, int arg)
{
PRMSG(2,"TRANS(LocalSetOption)(%d,%d,%d)\n",ciptr->fd,option,arg);

return -1;
}

static
TRANS(LocalCreateListener)(XtransConnInfo ciptr, char *port)
{
PRMSG(2,"TRANS(LocalCreateListener)(%x->%d,%s)\n",ciptr,ciptr->fd,port);

return 0;
}

static XtransConnInfo
TRANS(LocalAccept)(XtransConnInfo ciptr)
{
XtransConnInfo	newciptr;
LOCALtrans2dev	*transptr;

PRMSG(2,"TRANS(LocalAccept)(%x->%d)\n", ciptr, ciptr->fd,0);

transptr=(LOCALtrans2dev *)ciptr->priv;

if( (newciptr=(XtransConnInfo)calloc(1,sizeof(struct _XtransConnInfo)))==NULL )
	{
	PRMSG(1,"TRANS(LocalAccept)() calloc(1,%d) failed\n",
					sizeof(struct _XtransConnInfo),0,0 );
	return NULL;
	}

newciptr->fd=transptr->devaccept(ciptr,newciptr);

if( newciptr->fd < 0 )
	{
	free(newciptr);
	return NULL;
	}

newciptr->priv=(char *)transptr;

return newciptr;
}

static
TRANS(LocalConnect)(XtransConnInfo ciptr, char *host, char *port)
{
PRMSG(2,"TRANS(LocalConnect)(%x->%d,%s)\n", ciptr, ciptr->fd, port);

return 0;
}

static int
TRANS(LocalBytesReadable)(XtransConnInfo ciptr, BytesReadable_t *pend )
{
PRMSG(2,"TRANS(LocalBytesReadable)(%x->%d,%x)\n", ciptr, ciptr->fd, pend);

return ioctl(ciptr->fd, FIONREAD, (char *)pend);
}

static int
TRANS(LocalRead)(XtransConnInfo ciptr, char *buf, int size)
{
PRMSG(2,"TRANS(LocalRead)(%d,%x,%d)\n", ciptr->fd, buf, size );

return read(ciptr->fd,buf,size);
}

static int
TRANS(LocalWrite)(XtransConnInfo ciptr, char *buf, int size)
{
PRMSG(2,"TRANS(LocalWrite)(%d,%x,%d)\n", ciptr->fd, buf, size );

return write(ciptr->fd,buf,size);
}

static int
TRANS(LocalReadv)(XtransConnInfo ciptr, struct iovec *buf, int size)
{
PRMSG(2,"TRANS(LocalReadv)(%d,%x,%d)\n", ciptr->fd, buf, size );

return READV(ciptr,buf,size);
}

static int
TRANS(LocalWritev)(XtransConnInfo ciptr, struct iovec *buf, int size)
{
PRMSG(2,"TRANS(LocalWritev)(%d,%x,%d)\n", ciptr->fd, buf, size );

return WRITEV(ciptr,buf,size);
}

static int
TRANS(LocalDisconnect)(XtransConnInfo ciptr)
{
PRMSG(2,"TRANS(LocalDisconnect)(%x->%d)\n", ciptr, ciptr->fd, 0);

return 0;
}

static int
TRANS(LocalClose)(XtransConnInfo ciptr)
{
struct sockaddr_un      *sockname=(struct sockaddr_un *) ciptr->addr;
char    path[200]; /* > sizeof sun_path +1 */
int	ret;

PRMSG(2,"TRANS(LocalClose)(%x->%d)\n", ciptr, ciptr->fd ,0);

ret=close(ciptr->fd);

if(ciptr->flags
   && sockname
   && sockname->sun_family == AF_UNIX
     && sockname->sun_path[0] )
	{
	strncpy(path,sockname->sun_path,
			ciptr->addrlen-sizeof(sockname->sun_family));
	unlink(path);
	}

return ret;
}

static
TRANS(LocalNameToAddr)(XtransConnInfo ciptr /*???what else???*/ )
{
}

static
TRANS(LocalAddrToName)(XtransConnInfo ciptr /*???what else???*/ )
{
}

/*
 * MakeAllCOTSServerListeners() will go through the entire Xtransports[]
 * array defined in Xtrans.c and try to OpenCOTSServer() for each entry.
 * We will add duplicate entries to that table so that the OpenCOTSServer()
 * function will get called once for each type of local transport. 
 *
 * The TransName is in lowercase, so it will never match during a normal
 * call to SelectTransport() in Xtrans.c.
 */

Xtransport	TRANS(LocalFuncs) = {
	/* Local Interface */
	"local",
	TRANS_ALIAS,
	TRANS(LocalOpenCOTSClient),
	TRANS(LocalOpenCOTSServer),
	TRANS(LocalOpenCLTSClient),
	TRANS(LocalOpenCLTSServer),
	TRANS(LocalSetOption),
	TRANS(LocalCreateListener),
	NULL,					/* ResetListener */
	TRANS(LocalAccept),
	TRANS(LocalConnect),
	TRANS(LocalBytesReadable),
	TRANS(LocalRead),
	TRANS(LocalWrite),
	TRANS(LocalReadv),
	TRANS(LocalWritev),
	TRANS(LocalDisconnect),
	TRANS(LocalClose),
	TRANS(LocalNameToAddr),
	TRANS(LocalAddrToName),
};

Xtransport	TRANS(PTSFuncs) = {
	/* Local Interface */
	"pts",
	0,
	TRANS(LocalOpenCOTSClient),
	TRANS(LocalOpenCOTSServer),
	TRANS(LocalOpenCLTSClient),
	TRANS(LocalOpenCLTSServer),
	TRANS(LocalSetOption),
	TRANS(LocalCreateListener),
	NULL,					/* ResetListener */
	TRANS(LocalAccept),
	TRANS(LocalConnect),
	TRANS(LocalBytesReadable),
	TRANS(LocalRead),
	TRANS(LocalWrite),
	TRANS(LocalReadv),
	TRANS(LocalWritev),
	TRANS(LocalDisconnect),
	TRANS(LocalClose),
	TRANS(LocalNameToAddr),
	TRANS(LocalAddrToName),
};

Xtransport	TRANS(NAMEDFuncs) = {
	/* Local Interface */
	"named",
	0,
	TRANS(LocalOpenCOTSClient),
	TRANS(LocalOpenCOTSServer),
	TRANS(LocalOpenCLTSClient),
	TRANS(LocalOpenCLTSServer),
	TRANS(LocalSetOption),
	TRANS(LocalCreateListener),
	NULL,					/* ResetListener */
	TRANS(LocalAccept),
	TRANS(LocalConnect),
	TRANS(LocalBytesReadable),
	TRANS(LocalRead),
	TRANS(LocalWrite),
	TRANS(LocalReadv),
	TRANS(LocalWritev),
	TRANS(LocalDisconnect),
	TRANS(LocalClose),
	TRANS(LocalNameToAddr),
	TRANS(LocalAddrToName),
};

Xtransport	TRANS(ISCFuncs) = {
	/* Local Interface */
	"isc",
	0,
	TRANS(LocalOpenCOTSClient),
	TRANS(LocalOpenCOTSServer),
	TRANS(LocalOpenCLTSClient),
	TRANS(LocalOpenCLTSServer),
	TRANS(LocalSetOption),
	TRANS(LocalCreateListener),
	NULL,					/* ResetListener */
	TRANS(LocalAccept),
	TRANS(LocalConnect),
	TRANS(LocalBytesReadable),
	TRANS(LocalRead),
	TRANS(LocalWrite),
	TRANS(LocalReadv),
	TRANS(LocalWritev),
	TRANS(LocalDisconnect),
	TRANS(LocalClose),
	TRANS(LocalNameToAddr),
	TRANS(LocalAddrToName),
};
Xtransport	TRANS(SCOFuncs) = {
	/* Local Interface */
	"sco",
	0,
	TRANS(LocalOpenCOTSClient),
	TRANS(LocalOpenCOTSServer),
	TRANS(LocalOpenCLTSClient),
	TRANS(LocalOpenCLTSServer),
	TRANS(LocalSetOption),
	TRANS(LocalCreateListener),
	NULL,					/* ResetListener */
	TRANS(LocalAccept),
	TRANS(LocalConnect),
	TRANS(LocalBytesReadable),
	TRANS(LocalRead),
	TRANS(LocalWrite),
	TRANS(LocalReadv),
	TRANS(LocalWritev),
	TRANS(LocalDisconnect),
	TRANS(LocalClose),
	TRANS(LocalNameToAddr),
	TRANS(LocalAddrToName),
};
