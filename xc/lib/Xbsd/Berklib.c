/* $XConsortium: Berklib.c,v 1.8 91/01/10 20:21:56 rws Exp $ */

/*
 * These are routines found in BSD but not on all other systems.  The core
 * MIT distribution does not use them except for ffs in the server.  You
 * should enable only the ones that you need for your system.  Use Xfuncs.h
 * in clients to avoid using the slow versions of bcopy, bcmp, and bzero
 * provided here.
 */

#include <sys/types.h>

#ifdef hpux
#define WANT_BFUNCS
#define WANT_FFS
#define WANT_RANDOM
#endif

#ifdef macII
#define WANT_RANDOM
#endif


#ifdef WANT_BFUNCS

/* you should use Xfuncs.h instead of this in most cases */
bcopy (b1, b2, length)
    register char *b1, *b2;
    register length;
{
    if (b1 < b2) {
	b2 += length;
	b1 += length;
	while (length--)
	    *--b2 = *--b1;
    } else {
	while (length--)
	    *b2++ = *b1++;
    }
}

/* you should use Xfuncs.h instead of this in most cases */
bcmp (b1, b2, length)
    register char *b1, *b2;
    register length;
{
    while (length--) {
	if (*b1++ != *b2++) return 1;
    }
    return 0;
}
#endif

/* you should use Xfuncs.h instead of this in most cases */
bzero (b, length)
    register char *b;
    register length;
{
    while (length--)
	*b++ = '\0';
}

#endif /* WANT_BFUNCS */

/* Find the first set bit
 * i.e. least signifigant 1 bit:
 * 0 => 0
 * 1 => 1
 * 2 => 2
 * 3 => 1
 * 4 => 3
 */

#ifdef WANT_FFS
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
#endif

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

#ifdef WANT_QUE
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
#endif /* WANT_QUE */

/*
 * Berkeley random()
 *
 * We simulate via System V's rand()
 */

#ifdef WANT_RANDOM
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
#endif /* WANT_RANDOM */

/*
 * gettimeofday emulation
 * Caution -- emulation is incomplete
 *  - has only second, not microsecond, resolution.
 *  - does not return timezone info.
 */

#if WANT_GTOD
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
#endif /* WANT_GTOD */

