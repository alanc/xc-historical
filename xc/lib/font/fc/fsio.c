/* $XConsortium$ */
/*
 * Copyright 1990 Network Computing Devices
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Network Computing Devices not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  Network Computing
 * Devices makes no representations about the suitability of this software
 * for any purpose.  It is provided "as is" without express or implied
 * warranty.
 *
 * NETWORK COMPUTING DEVICES DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
 * IN NO EVENT SHALL NETWORK COMPUTING DEVICES BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE
 * OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  	Dave Lemke, Network Computing Devices, Inc
 *
 */
/*
 * font server i/o routines
 */

#include	<X11/Xos.h>
#include	"fontmisc.h"
#include	"FS.h"
#include	"FSproto.h"
#include	<stdio.h>
#include	<sys/socket.h>
#include	<sys/param.h>
#include	<netinet/tcp.h>
#include	<errno.h>
#include	"FSlibos.h"
#include	"fsio.h"

/* check for both EAGAIN and EWOULDBLOCK, because some supposedly POSIX
 * systems are broken and return EWOULDBLOCK when they should return EAGAIN
 */
#if defined(EAGAIN) && defined(EWOULDBLOCK)
#define ETEST(err) (err == EAGAIN || err == EWOULDBLOCK)
#else
#ifdef EAGAIN
#define ETEST(err) (err == EAGAIN)
#else
#define ETEST(err) (err == EWOULDBLOCK)
#endif
#endif

extern int  errno;
#ifdef DEBUG
extern char *sys_errlist[];
#endif

static int  padlength[4] = {0, 3, 2, 1};
unsigned long fs_fd_mask[MSKCNT];

static int  _fs_wait_for_readable();

_fs_name_to_address(servername, inaddr)
    char       *servername;
    struct sockaddr_in *inaddr;
{
    int         servernum = 0;
    char        hostname[256];
    char       *sp;
    unsigned long hostinetaddr;
    struct hostent *hp;

    /* XXX - do any service name lookup to get a hostname */

    /* XXX - hack to convert ':' name to normal one */
    (void) strncpy(hostname, servername, sizeof(hostname));
    if ((sp = index(hostname, ':')) == NULL)
	return -1;

    *(sp++) = '\0';

    /* missing server number */
    if (*sp == '\0')
	return -1;
    servernum = atoi(sp);

    /* find host address */
    sp = hostname;
    if (*sp == ':')
	sp++;

/* XXX -- this is all TCP specific.  other transports need to hack */
    hostinetaddr = inet_addr(sp);
    if (hostinetaddr == -1) {
	if ((hp = gethostbyname(sp)) == NULL) {
	    /* no such host */
	    errno = EINVAL;
	    return -1;
	}
	inaddr->sin_family = hp->h_addrtype;
	bcopy((char *) hp->h_addr, (char *) &inaddr->sin_addr,
	      sizeof(inaddr->sin_addr));
    } else {
	inaddr->sin_addr.s_addr = hostinetaddr;
	inaddr->sin_family = AF_INET;
    }
    inaddr->sin_port = servernum;
    inaddr->sin_port += FS_TCP_PORT;
    inaddr->sin_port = htons(inaddr->sin_port);

    return 1;
}

static int
_fs_connect(servername)
    char       *servername;
{
    int         fd;
    struct sockaddr *addr;
    struct sockaddr_in inaddr;


    if (_fs_name_to_address(servername, &inaddr) < 0)
	return -1;

    addr = (struct sockaddr *) & inaddr;
    if ((fd = socket((int) addr->sa_family, SOCK_STREAM, 0)) < 0) {
	return -1;
    }
    if (connect(fd, addr, sizeof(struct sockaddr_in)) == -1) {
	(void) close(fd);
	return -1;
    }

    /* ultrix reads hang on Unix sockets, hpux reads fail */
#if defined(O_NONBLOCK) && (!defined(ultrix) && !defined(hpux))
    (void) fcntl (fd, F_SETFL, O_NONBLOCK);
#else
#ifdef FIOSNBIO
    {
	int arg = 1;
	ioctl (fd, FIOSNBIO, &arg);
    }
#else
    (void) fcntl (fd, F_SETFL, FNDELAY);
#endif
#endif

    return fd;
}

FSFpePtr
_fs_open_server(servername)
    char       *servername;
{
    FSFpePtr    conn;
    fsConnClientPrefix prefix;
    fsConnSetup rep;
    int         setuplength;
    fsConnSetupAccept conn_accept;
    int         endian;
    char       *auth_data = NULL,
               *vendor_string = NULL,
               *alt_data = NULL;

    conn = (FSFpePtr) xalloc(sizeof(FSFpeRec));
    if (!conn) {
	errno = ENOMEM;
	return conn;
    }
    bzero((char *) conn, sizeof(FSFpeRec));
    conn->fs_fd = _fs_connect(servername);
    if (conn->fs_fd < 0) {
	xfree((char *) conn);
	return (FSFpePtr) NULL;
    }
    /* send setup prefix */
    endian = 1;
    if (*(char *) &endian)
	prefix.byteOrder = 'l';
    else
	prefix.byteOrder = 'B';

    prefix.major_version = FS_PROTOCOL;
    prefix.minor_version = FS_PROTOCOL_MINOR;

/* XXX add some auth info here */
    prefix.num_auths = 0;
    prefix.auth_len = 0;

    _fs_write(conn, (char *) &prefix, sizeof(fsConnClientPrefix));

    /* read setup info */
    _fs_read(conn, (char *) &rep, sizeof(fsConnSetup));

    setuplength = rep.alternate_len << 2;
    if (setuplength &&
	    !(alt_data = (char *) xalloc((unsigned int) setuplength))) {
	errno = ENOMEM;
	xfree((char *) conn);
	return (FSFpePtr) NULL;
    }
    _fs_read(conn, (char *) alt_data, setuplength);
/* XXX take apart delegate info */

    setuplength = rep.auth_len << 2;
    if (setuplength &&
	    !(auth_data = (char *) xalloc((unsigned int) setuplength))) {
	errno = ENOMEM;
	xfree((char *) conn);
	return (FSFpePtr) NULL;
    }
    _fs_read(conn, (char *) auth_data, setuplength);

    if (rep.status != AuthSuccess) {
/* XXX -- try a delegate */
	xfree(alt_data);
	xfree(auth_data);
	xfree((char *) conn);
	return (FSFpePtr) NULL;
    }
    /* get rest */
    _fs_read(conn, (char *) &conn_accept, (long) sizeof(fsConnSetupAccept));

    if ((vendor_string = (char *)
	 xalloc((unsigned) conn_accept.vendor_len + 1)) == NULL) {
	errno = ENOMEM;
	xfree((char *) conn);
	return (FSFpePtr) NULL;
    }
    _fs_read_pad(conn, (char *) vendor_string, conn_accept.vendor_len);

    xfree(alt_data);
    xfree(auth_data);
    xfree(vendor_string);

    return conn;
}

/*
 * expects everything to be here.  *not* to be called when reading huge
 * numbers of replies, but rather to get each chunk
 */
_fs_read(conn, data, size)
    FSFpePtr    conn;
    char       *data;
    unsigned long size;
{
    long        bytes_read;

    if (size == 0) {

#ifdef DEBUG
	fprintf(stderr, "tried to read 0 bytes \n");
#endif

	return 0;
    }
    while ((bytes_read = read(conn->fs_fd, data, (int) size)) != size) {
	if (bytes_read > 0) {
	    size -= bytes_read;
	    data += bytes_read;
	} else if (ETEST(errno)) {
	    /* this shouldn't happen */
	    if (_fs_wait_for_readable(conn) == -1)	/* check for error */
		return -1;
	} else if (bytes_read == 0) {	/* EOF */
	    errno = EPIPE;
	    return -1;
	} else {
	    if (errno != EINTR)
		continue;

#ifdef DEBUG
	    fprintf(stderr, "read failed -- %s?\n", sys_errlist[errno]);
#endif

	    return -1;
	}
    }
    return 0;
}

_fs_write(conn, data, size)
    FSFpePtr    conn;
    char       *data;
    unsigned long size;
{
    long        bytes_written;

    if (size == 0) {

#ifdef DEBUG
	fprintf(stderr, "tried to write 0 bytes \n");
#endif

	return 0;
    }
    while ((bytes_written = write(conn->fs_fd, data, (int) size)) != size) {
	if (bytes_written > 0) {
	    size -= bytes_written;
	    data += bytes_written;
	} else if (ETEST(errno)) {
	    /* XXX -- we assume this can't happen */

#ifdef DEBUG
	    fprintf(stderr, "fs_write blocking\n");
#endif
	} else if (bytes_written == 0) {	/* EOF */
	    errno = EPIPE;
	    return -1;
	} else {
	    if (errno != EINTR)
		continue;

#ifdef DEBUG
	    fprintf(stderr, "write failed -- %s?\n", sys_errlist[errno]);
#endif

	    return -1;
	}
    }
    return 0;
}

_fs_read_pad(conn, data, len)
    FSFpePtr    conn;
    char       *data;
    int         len;
{
    char        pad[3];

    _fs_read(conn, data, len);

    /* read the junk */
    if (padlength[len & 3]) {
	_fs_read(conn, pad, padlength[len & 3]);
    }
}

_fs_write_pad(conn, data, len)
    FSFpePtr    conn;
    char       *data;
    int         len;
{
    static char pad[3];

    _fs_write(conn, data, len);

    /* write the pad */
    if (padlength[len & 3]) {
	_fs_write(conn, pad, padlength[len & 3]);
    }
}

/*
 * returns the amount of data waiting to be read
 */
int
_fs_data_ready(conn)
    FSFpePtr    conn;
{
    long        readable;

    if (ioctl(conn->fs_fd, FIONREAD, &readable) < 0)
	return -1;
    return readable;
}

static int
_fs_wait_for_readable(conn)
    FSFpePtr    conn;
{
    unsigned long r_mask[MSKCNT];
    unsigned long e_mask[MSKCNT];
    int         result;

#ifdef DEBUG
    fprintf(stderr, "read would block\n");
#endif

    CLEARBITS(r_mask);
    CLEARBITS(e_mask);
    do {
	BITSET(r_mask, conn->fs_fd);
	BITSET(e_mask, conn->fs_fd);
	result = select(conn->fs_fd + 1, r_mask, NULL, e_mask, NULL);
	if (result == -1) {
	    if (errno != EINTR)
	        return -1;
	    else continue;
	}
	if (result && _fs_any_bit_set(e_mask))
	    return -1;
    } while (result <= 0);

    return 0;
}

int
_fs_set_bit(mask, fd)
    unsigned long *mask;
    int         fd;
{
    return BITSET(mask, fd);
}

int
_fs_is_bit_set(mask, fd)
    unsigned long *mask;
    int         fd;
{
    return GETBIT(mask, fd);
}

void
_fs_bit_clear(mask, fd)
    unsigned long *mask;
    int         fd;
{
    BITCLEAR(mask, fd);
}

int
_fs_any_bit_set(mask)
    unsigned long *mask;
{
#ifdef _FSANYSET
    return _FSANYSET(mask);
#else
    int i;

    for (i=0; i<MSKCNT; i++)
	if (mask[i])
	    return (1);
    return (0);
#endif
}

int
_fs_or_bits(dst, m1, m2)
    unsigned long *dst,
               *m1,
               *m2;
{
    ORBITS(dst, m1, m2);
}

_fs_eat_rest_of_error(conn, err)
    FSFpePtr    conn;
    fsError    *err;
{
    char        buf[128];
    int         len = (err->length - (sizeof(fsReplyHeader) >> 2)) << 2;

    while (len > 0) {
        _fs_read(conn, buf, len < 128 ? len : 128);
	len -= 128;
    }
}
