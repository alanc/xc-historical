/*
 * Allegro CL dependent C helper routines for CLX
 */

/*
 * This code requires select.  This means you probably need BSD, or a version
 * of Unix with select and interval timers added.
 * I only select on 32 file descriptors, so if your fd is higher than that you
 * lose.
 */

#include <sys/types.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <signal.h>

#define ERROR -1
#define INTERRUPT -2

#define min(x,y) (((x) > (y)) ? (y) : (x))

extern int errno;

static struct timeval zerotimeout = {0, 0};


/*
 * Slightly misnamed for backwards compatibility.  This function returns 0
 * on interrupt or if there are no bytes available, ERROR on error, and 1
 * if bytes are available.  (Note 1 is also returned if the socket becomes
 * disconnected.  Thus it is up to the read to detect the eof)
 */
static int c_howmany_bytes(fd)
    register int fd;
{
    register int i;
    int readfds = 1 << fd;

    i = select(32, &readfds, (int *)0, (int *)0, &zerotimeout);
    if (i < 0)
      /* error condition */
      if (errno == EINTR)
	return (0);
      else
	return (ERROR);
    return (i);
}


/*
 * Tries to read length characters into array at position start.  This function
 * will either 1: return 0 if there are no characters available on the socket
 *	       2: return ERROR on eof or error
 *	       3: succeed in reading all length bytes and return length
 *
 * Note that this implicitly assumes there will be enough bytes available if
 * there are any bytes available.  This is safe (ie: won't block) unless CLX
 * gets out of sync.
 */
int c_read_bytes(fd, array, start, length)
    register int fd, start, length;
    register unsigned char *array;

{
    register int numread, avail, totread;

    totread = length;

    avail = c_howmany_bytes(fd);

    if (avail <= 0) {
	return (avail);
    } else {
	while (length > 0) {
	    numread = read(fd, (char *)&array[start], length);
	    if (numread <= 0) {
		return (ERROR);
	    } else {
		length -= numread;
		start += numread;
	    }
	}
    }
    return (totread);
}


/*
 * When the scheduler is not running we must provide a way for the user
 * to interrupt the read from the X socket.  So we provide a separate
 * reading function, which returns INTERRUPT if it was interrupted.
 */
int c_read_bytes_interruptible(fd, array, start, length)
    register int fd, start, length;
    register unsigned char *array;

{
    register int i, numread, totread;
    int readfds;

    totread = length;
    readfds = 1 << fd;

    i = select(32, &readfds, (int *)0, (int *)0, (struct timeval *)0);
    if (i < 0)
      /* error condition */
      if (errno == EINTR)
	return (INTERRUPT);
      else
	return (ERROR);

    while (length > 0) {
	numread = read(fd, (char *)&array[start], length);
	if (numread <= 0) {
	    return (ERROR);
	} else {
	    length -= numread;
	    start += numread;
	}
    }
    return (totread);
}
