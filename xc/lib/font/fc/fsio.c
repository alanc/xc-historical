/* $XConsortium: fsio.c,v 1.28 93/09/12 20:08:19 rws Exp $ */
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
 */
/*
 * font server i/o routines
 */

#ifdef WIN32
#define _WILLWINSOCK_
#endif
#include	<X11/Xos.h>

#ifdef NCD
#include	<fcntl.h>
#endif

#include	"FS.h"
#include	"FSproto.h"
#include	<stdio.h>
#include	<signal.h>
#include	<sys/types.h>
#ifndef WIN32
#include	<sys/socket.h>
#endif
#include	<errno.h>
#ifdef X_NOT_STDC_ENV
extern int errno;
#endif 
#include	"FSlibos.h"
#include	"fontmisc.h"
#include	"fsio.h"
#ifdef WIN32
#define EWOULDBLOCK WSAEWOULDBLOCK
#undef EINTR
#define EINTR WSAEINTR
#endif

/* check for both EAGAIN and EWOULDBLOCK, because some supposedly POSIX
 * systems are broken and return EWOULDBLOCK when they should return EAGAIN
 */
#ifdef WIN32
#define ETEST() (WSAGetLastError() == WSAEWOULDBLOCK)
#else
#if defined(EAGAIN) && defined(EWOULDBLOCK)
#define ETEST() (errno == EAGAIN || errno == EWOULDBLOCK)
#else
#ifdef EAGAIN
#define ETEST() (errno == EAGAIN)
#else
#define ETEST() (errno == EWOULDBLOCK)
#endif
#endif
#endif
#ifdef WIN32
#define ECHECK(err) (WSAGetLastError() == err)
#define ESET(val) WSASetLastError(val)
#else
#define ECHECK(err) (errno == err)
#define ESET(val) errno = val
#endif

static int  padlength[4] = {0, 3, 2, 1};
FdSet _fs_fd_mask;

int  _fs_wait_for_readable();

static int
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

    (void) strncpy(hostname, servername, sizeof(hostname));

    /* get port */
    if ((sp = index(hostname, ':')) == NULL)
	return -1;

    *(sp++) = '\0';

    /* missing server number */
    if (*sp == '\0')
	return -1;
    servernum = atoi(sp);

    /* find host address */
    sp = hostname;
    if (!strncmp(hostname, "tcp/", 4)) {
	sp += 4;
    }
/* XXX -- this is all TCP specific.  other transports need to hack */
    hostinetaddr = inet_addr(sp);
    if (hostinetaddr == -1) {
	if ((hp = gethostbyname(sp)) == NULL) {
	    /* no such host */
	    errno = EINVAL;
	    return -1;
	}
	inaddr->sin_family = hp->h_addrtype;
	memcpy(&inaddr->sin_addr, hp->h_addr,
	       sizeof(inaddr->sin_addr));
    } else {
	inaddr->sin_addr.s_addr = hostinetaddr;
	inaddr->sin_family = AF_INET;
    }
    inaddr->sin_port = servernum;
    inaddr->sin_port = htons(inaddr->sin_port);
#ifdef BSD44SOCKETS
    inaddr->sin_len = sizeof(*inaddr);
#endif

    return 1;
}

#ifdef SIGNALRETURNSINT
#define SIGNAL_T int
#else
#define SIGNAL_T void
#endif

/* ARGSUSED */
static      SIGNAL_T
_fs_alarm(foo)
    int         foo;
{
    return;
}

static int
_fs_connect(servername, timeout)
    char       *servername;
    int         timeout;
{
    int         fd;
    struct sockaddr *addr;
    struct sockaddr_in inaddr;
#ifdef SIGALRM
    unsigned    oldTime;

    SIGNAL_T(*oldAlarm) ();
#endif
    int         ret;
#ifdef WIN32
    static WSADATA wsadata;
#endif

#ifdef WIN32
    if (!wsadata.wVersion && WSAStartup(MAKEWORD(1,1), &wsadata))
	return -1;
#endif
    if (_fs_name_to_address(servername, &inaddr) < 0)
	return -1;

    addr = (struct sockaddr *) & inaddr;
    if ((fd = socket((int) addr->sa_family, SOCK_STREAM, 0)) < 0)
	return -1;

#ifdef SIGALRM
    oldTime = alarm((unsigned) 0);
    oldAlarm = signal(SIGALRM, _fs_alarm);
    alarm((unsigned) timeout);
#endif
    ret = connect(fd, addr, sizeof(struct sockaddr_in));
#ifdef SIGALRM
    alarm((unsigned) 0);
    signal(SIGALRM, oldAlarm);
    alarm(oldTime);
#endif
    if (ret == -1) {
	(void) close(fd);
	return -1;
    }

    /*
     * Set the connection non-blocking since we use select() to block.
     */
    /* ultrix reads hang on Unix sockets, hpux reads fail */
#if defined(O_NONBLOCK) && (!defined(ultrix) && !defined(hpux) && !defined(AIXV3) && !defined(uniosu))
    (void) fcntl (fd, F_SETFL, O_NONBLOCK);
#else
#ifdef FIOSNBIO
    {
	int arg = 1;
	ioctl (fd, FIOSNBIO, &arg);
    }
#else
#if (defined(AIXV3) || defined(uniosu) || defined(WIN32)) && defined(FIONBIO)
    {
	int arg;
	arg = 1;
	ioctl(fd, FIONBIO, &arg);
    }
#else
    (void) fcntl (fd, F_SETFL, FNDELAY);
#endif
#endif
#endif
    return fd;
}

static int  generationCount;

/* ARGSUSED */
static Bool
_fs_setup_connection(conn, servername, timeout)
    FSFpePtr    conn;
    char       *servername;
    int         timeout;
{
    fsConnClientPrefix prefix;
    fsConnSetup rep;
    int         setuplength;
    fsConnSetupAccept conn_accept;
    int         endian;
    int         i;
    int         alt_len;
    char       *auth_data = NULL,
               *vendor_string = NULL,
               *alt_data = NULL,
               *alt_dst;
    FSFpeAltPtr alts;
    int         nalts;

    conn->fs_fd = _fs_connect(servername, 5);
    if (conn->fs_fd < 0)
	return FALSE;

    conn->generation = ++generationCount;

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

    if (_fs_write(conn, (char *) &prefix, SIZEOF(fsConnClientPrefix)) == -1)
	return FALSE;

    /* read setup info */
    if (_fs_read(conn, (char *) &rep, SIZEOF(fsConnSetup)) == -1)
	return FALSE;

    conn->fsMajorVersion = rep.major_version;
    if (rep.major_version > FS_PROTOCOL)
	return FALSE;

    alts = 0;
    /* parse alternate list */
    if (nalts = rep.num_alternates) {
	setuplength = rep.alternate_len << 2;
	alts = (FSFpeAltPtr) xalloc(nalts * sizeof(FSFpeAltRec) +
				    setuplength);
	if (!alts) {
	    close(conn->fs_fd);
	    errno = ENOMEM;
	    return FALSE;
	}
	alt_data = (char *) (alts + nalts);
	if (_fs_read(conn, (char *) alt_data, setuplength) == -1) {
	    xfree(alts);
	    return FALSE;
	}
	alt_dst = alt_data;
	for (i = 0; i < nalts; i++) {
	    alts[i].subset = alt_data[0];
	    alt_len = alt_data[1];
	    alts[i].name = alt_dst;
	    memmove(alt_dst, alt_data + 2, alt_len);
	    alt_dst[alt_len] = '\0';
	    alt_dst += (alt_len + 1);
	    alt_data += (2 + alt_len + padlength[(2 + alt_len) & 3]);
	}
    }
    if (conn->alts)
	xfree(conn->alts);
    conn->alts = alts;
    conn->numAlts = nalts;

    setuplength = rep.auth_len << 2;
    if (setuplength &&
	    !(auth_data = (char *) xalloc((unsigned int) setuplength))) {
	close(conn->fs_fd);
	errno = ENOMEM;
	return FALSE;
    }
    if (_fs_read(conn, (char *) auth_data, setuplength) == -1) {
	xfree(auth_data);
	return FALSE;
    }
    if (rep.status != AuthSuccess) {
	xfree(auth_data);
	close(conn->fs_fd);
	errno = EPERM;
	return FALSE;
    }
    /* get rest */
    if (_fs_read(conn, (char *) &conn_accept, (long) SIZEOF(fsConnSetupAccept)) == -1) {
	xfree(auth_data);
	return FALSE;
    }
    if ((vendor_string = (char *)
	 xalloc((unsigned) conn_accept.vendor_len + 1)) == NULL) {
	xfree(auth_data);
	close(conn->fs_fd);
	errno = ENOMEM;
	return FALSE;
    }
    if (_fs_read_pad(conn, (char *) vendor_string, conn_accept.vendor_len) == -1) {
	xfree(vendor_string);
	xfree(auth_data);
	return FALSE;
    }
    xfree(auth_data);
    xfree(vendor_string);

    conn->servername = (char *) xalloc(strlen(servername) + 1);
    if (conn->servername == NULL)
	return FALSE;
    strcpy(conn->servername, servername);

    return TRUE;
}

static Bool
_fs_try_alternates(conn, timeout)
    FSFpePtr    conn;
    int         timeout;
{
    int         i;

    for (i = 0; i < conn->numAlts; i++)
	if (_fs_setup_connection(conn, conn->alts[i].name, timeout))
	    return TRUE;
    return FALSE;
}

#define FS_OPEN_TIMEOUT	    30
#define FS_REOPEN_TIMEOUT   10

FSFpePtr
_fs_open_server(servername)
    char       *servername;
{
    FSFpePtr    conn;

    conn = (FSFpePtr) xalloc(sizeof(FSFpeRec));
    if (!conn) {
	errno = ENOMEM;
	return (FSFpePtr) NULL;
    }
    bzero((char *) conn, sizeof(FSFpeRec));
    if (!_fs_setup_connection(conn, servername, FS_OPEN_TIMEOUT)) {
	if (!_fs_try_alternates(conn, FS_OPEN_TIMEOUT)) {
	    xfree(conn->alts);
	    xfree(conn);
	    return (FSFpePtr) NULL;
	}
    }
    return conn;
}

Bool
_fs_reopen_server(conn)
    FSFpePtr    conn;
{
    if (_fs_setup_connection(conn, conn->servername, FS_REOPEN_TIMEOUT))
	return TRUE;
    if (_fs_try_alternates(conn, FS_REOPEN_TIMEOUT))
	return TRUE;
    return FALSE;
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
    ESET(0);
    while ((bytes_read = read(conn->fs_fd, data, (int) size)) != size) {
	if (bytes_read > 0) {
	    size -= bytes_read;
	    data += bytes_read;
	} else if (ETEST()) {
	    /* in a perfect world, this shouldn't happen */
	    /* ... but then, its less than perfect... */
	    if (_fs_wait_for_readable(conn) == -1) {	/* check for error */
		_fs_connection_died(conn);
		ESET(EPIPE);
		return -1;
	    }
	    ESET(0);
	} else if (ECHECK(EINTR)) {
	    continue;
	} else {		/* something bad happened */
	    if (conn->fs_fd > 0)
		_fs_connection_died(conn);
	    ESET(EPIPE);
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
    ESET(0);
    while ((bytes_written = write(conn->fs_fd, data, (int) size)) != size) {
	if (bytes_written > 0) {
	    size -= bytes_written;
	    data += bytes_written;
	} else if (ETEST()) {
	    /* XXX -- we assume this can't happen */

#ifdef DEBUG
	    fprintf(stderr, "fs_write blocking\n");
#endif
	} else if (ECHECK(EINTR)) {
	    continue;
	} else {		/* something bad happened */
	    _fs_connection_died(conn);
	    ESET(EPIPE);
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

    if (_fs_read(conn, data, len) == -1)
	return -1;

    /* read the junk */
    if (padlength[len & 3]) {
	return _fs_read(conn, pad, padlength[len & 3]);
    }
    return 0;
}

_fs_write_pad(conn, data, len)
    FSFpePtr    conn;
    char       *data;
    int         len;
{
    static char pad[3];

    if (_fs_write(conn, data, len) == -1)
	return -1;

    /* write the pad */
    if (padlength[len & 3]) {
	return _fs_write(conn, pad, padlength[len & 3]);
    }
    return 0;
}

/*
 * returns the amount of data waiting to be read
 */
int
_fs_data_ready(conn)
    FSFpePtr    conn;
{
    long        readable;

    if (BytesReadable(conn->fs_fd, &readable) < 0)
	return -1;
    return readable;
}

int
_fs_wait_for_readable(conn)
    FSFpePtr    conn;
{
    FdSet r_mask;
    FdSet e_mask;
    int         result;

#ifdef DEBUG
    fprintf(stderr, "read would block\n");
#endif

    CLEARBITS(r_mask);
    CLEARBITS(e_mask);
    do {
	BITSET(r_mask, conn->fs_fd);
	BITSET(e_mask, conn->fs_fd);
#ifdef WIN32
	result = select(0, &r_mask, NULL, &e_mask, NULL);
#else
	result = select(conn->fs_fd + 1, r_mask, NULL, e_mask, NULL);
#endif
	if (result == -1) {
	    if (!ECHECK(EINTR))
		return -1;
	    else
		continue;
	}
	if (result && _fs_any_bit_set(e_mask))
	    return -1;
    } while (result <= 0);

    return 0;
}

int
_fs_set_bit(mask, fd)
    FdSetPtr mask;
    int         fd;
{
    BITSET(mask, fd);
    return fd;
}

int
_fs_is_bit_set(mask, fd)
    FdSetPtr mask;
    int         fd;
{
    return GETBIT(mask, fd);
}

void
_fs_bit_clear(mask, fd)
    FdSetPtr mask;
    int         fd;
{
    BITCLEAR(mask, fd);
}

int
_fs_any_bit_set(mask)
    FdSetPtr mask;
{

#ifdef ANYSET
    return ANYSET(mask);
#else
    int         i;

    for (i = 0; i < MSKCNT; i++)
	if (mask[i])
	    return (1);
    return (0);
#endif
}

int
_fs_or_bits(dst, m1, m2)
    FdSetPtr dst, m1, m2;
{
#ifdef WIN32
    int i;
    if (dst != m1) {
	for (i = m1->fd_count; --i >= 0; ) {
	    if (!FD_ISSET(m1->fd_array[i], dst))
		FD_SET(m1->fd_array[i], dst);
	}
    }
    if (dst != m2) {
	for (i = m2->fd_count; --i >= 0; ) {
	    if (!FD_ISSET(m2->fd_array[i], dst))
		FD_SET(m2->fd_array[i], dst);
	}
    }
#else
    ORBITS(dst, m1, m2);
#endif
}

_fs_drain_bytes(conn, len)
    FSFpePtr    conn;
    int         len;
{
    char        buf[128];

#ifdef DEBUG
    fprintf(stderr, "draining wire\n");
#endif

    while (len > 0) {
	if (_fs_read(conn, buf, (len < 128) ? len : 128) < 0)
	    return -1;
	len -= 128;
    }
    return 0;
}

_fs_drain_bytes_pad(conn, len)
    FSFpePtr    conn;
    int         len;
{
    _fs_drain_bytes(conn, len);

    /* read the junk */
    if (padlength[len & 3]) {
	_fs_drain_bytes(conn, padlength[len & 3]);
    }
}

_fs_eat_rest_of_error(conn, err)
    FSFpePtr    conn;
    fsError    *err;
{
    int         len = (err->length - (SIZEOF(fsGenericReply) >> 2)) << 2;

#ifdef DEBUG
    fprintf(stderr, "clearing error\n");
#endif

    _fs_drain_bytes(conn, len);
}
