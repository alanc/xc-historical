/* $XConsortium: Berklib.c,v 1.4 90/08/27 15:29:40 swick Exp $ */

/*
 * This file is used by System V based systems.
 */

#include <sys/types.h>

/*
 * These are routines found in BSD and not found in many SysV's.  They are
 * included so that some clients can compile.
 */

bcopy (b1, b2, length)
register unsigned char *b1, *b2;
register length;
{
    if (b1 < b2) {
	b2 += length;
	b1 += length;
	while (length--) {
	    *--b2 = *--b1;
	}
    }
    else {
	while (length--) {
	    *b2++ = *b1++;
	}
    }
}

bcmp (b1, b2, length)
register unsigned char *b1, *b2;
register length;
{
    while (length--) {
	if (*b1++ != *b2++) return 1;
    }
    return 0;
}

bzero (b, length)
register unsigned char *b;
register length;
{
    while (length--) {
	*b++ = '\0';
    }
}


/* Find the first set bit
 * i.e. least signifigant 1 bit:
 * 0 => 0
 * 1 => 1
 * 2 => 2
 * 3 => 1
 * 4 => 3
 */

int
ffs(mask)
unsigned int	mask;
{
    register i;

    if ( ! mask ) return 0;
    i = 1;
    while (! (mask & 1)) {
	i++;
	mask = mask >> 1;
    }
    return i;
}

/*
 * insque, remque - insert/remove element from a queue
 *
 * DESCRIPTION
 *      Insque and remque manipulate queues built from doubly linked
 *      lists.  Each element in the queue must in the form of
 *      ``struct qelem''.  Insque inserts elem in a queue immedi-
 *      ately after pred; remque removes an entry elem from a queue.
 *
 * SEE ALSO
 *      ``VAX Architecture Handbook'', pp. 228-235.
 */

struct qelem {
    struct    qelem *q_forw;
    struct    qelem *q_back;
    char *q_data;
    };

insque(elem, pred)
register struct qelem *elem, *pred;
{
    register struct qelem *q;
    /* Insert locking code here */
    if ( elem->q_forw = q = (pred ? pred->q_forw : pred) )
	q->q_back = elem;
    if ( elem->q_back = pred )
	pred->q_forw = elem;
    /* Insert unlocking code here */
}

remque(elem)
register struct qelem *elem;
{
    register struct qelem *q;
    if ( ! elem ) return;
    /* Insert locking code here */

    if ( q = elem->q_back ) q->q_forw = elem->q_forw;
    if ( q = elem->q_forw ) q->q_back = elem->q_back;

    /* insert unlocking code here */
}


/*
 * Berkeley random()
 *
 * We simulate via System V's rand()
 */

int
random()
{
   return (rand());
}

/*
 * Berkeley srandom()
 *
 * We simulate via System V's rand()
 */

int
srandom(seed)
int seed;
{
   return (srand(seed));
}


#ifdef hpux

/** on hpux 5.n, readv/writev don't work on sockets;
 ** Even on 6.0, we'll keep these routines around for doing
 ** extra large writes; (> 4000); (this caused the Bezier
 ** demo to blow up.)
 **/

#include <sys/uio.h>

#define min(x,y) ((x)>(y)?(y):(x))

int swWritev(fildes, iov, iovcnt)
int fildes;
register struct iovec *iov;
register int iovcnt;
{
    while (iovcnt && iov->iov_len == 0)
	iovcnt--, iov++;

    if (iovcnt)
	return(write(fildes,iov->iov_base,min(iov->iov_len,4000)));
    else
	return(0);
}

int swReadv(fildes, iov, iovcnt)
int fildes;
register struct iovec *iov;
register int iovcnt;
{
    while (iovcnt && iov->iov_len == 0)
	iovcnt--, iov++;

    if (iovcnt)
	return(read(fildes,iov->iov_base,iov->iov_len));
    else
	return(0);
}

#endif /* hpux */

/*
 * gettimeofday emulation
 * Caution -- emulation is incomplete
 *  - has only second, not microsecond, resolution.
 *  - does not return timezone info.
 */

#if defined(USG) && !defined(CRAY)
int gettimeofday (tvp, tzp)
    struct timeval *tvp;
    struct timezone *tzp;
{
    time (&tvp->tv_sec);
    tvp->tv_usec = 0L;

    if (tzp) {
	fprintf( stderr,
		 "Warning: gettimeofday() emulation does not return timezone\n"
		);
    }
}
#endif
