/*
 * xdm - display manager daemon
 *
 * $XConsortium: cryptokey.c,v 1.5 90/09/14 17:51:53 keith Exp $
 *
 * Copyright 1988 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

# include   <X11/Xauth.h>
# include   <X11/Xos.h>
# include   "dm.h"

/*
 * crypto key.  Generate cryptographically secure random numbers
 */

static unsigned char	key[8];

#ifdef HASDES
# define USE_XDMCP_DES

typedef unsigned char des_cblock[8];	/* crypto-block size */
typedef struct des_ks_struct { des_cblock _; } des_key_schedule[16];

static
longtochars (l, c)
    long	    l;
    unsigned char    *c;
{
    c[0] = (l >> 24) & 0xff;
    c[1] = (l >> 16) & 0xff;
    c[2] = (l >> 8) & 0xff;
    c[3] = l & 0xff;
}


# define FILE_LIMIT	1024	/* no more than this many buffers */

static
sumFile (name, sum)
char	*name;
long	sum[2];
{
    long    buf[1024*2];
    int	    cnt;
    int	    fd;
    int	    loops;
    int	    reads;
    int	    i;

    fd = open (name, 0);
    if (fd < 0)
	return 0;
    reads = FILE_LIMIT;
    while ((cnt = read (fd, buf, sizeof (buf))) > 0 && --reads > 0) {
	loops = cnt / (2 * sizeof (long));
	for (i = 0; i < loops; i+= 2) {
	    sum[0] += buf[i];
	    sum[1] += buf[i+1];
	}
    }
    close (fd);
    return 1;
}

static
InitCryptoKey ()
{
    long	    sum[2];
    unsigned char   tmpkey[8];
    
    if (!sumFile (randomFile, sum)) {
	sum[0] = time ((long *) 0);
	sum[1] = time ((long *) 0);
    }
    longtochars (sum[0], tmpkey+0);
    longtochars (sum[1], tmpkey+4);
    tmpkey[0] = 0;
    XdmcpKeyToOddParityKey (tmpkey, key);
}

#endif

GenerateCryptoKey (auth, len)
char	*auth;
int	len;
{
    long	    ldata[2];

#ifdef ITIMER_REAL
    {
	struct timeval  now;
	struct timezone zone;
	gettimeofday (&now, &zone);
	ldata[0] = now.tv_sec;
	ldata[1] = now.tv_usec;
    }
#else
    {
	long    time ();

	ldata[0] = time ((long *) 0);
	ldata[1] = getpid ();
    }
#endif
#ifdef USE_XDMCP_DES
    {
    	int		    bit;
    	int		    i;
	des_key_schedule    schedule;
	unsigned char	    data[8];
	static int	    cryptoInited;
    
	longtochars (ldata[0], data+0);
	longtochars (ldata[1], data+4);
	if (!cryptoInited)
	{
	    InitCryptoKey();
	    cryptoInited = 1;
	}
	des_set_key (key, schedule);
    	for (i = 0; i < len; i++) {
	    auth[i] = 0;
	    for (bit = 1; bit < 256; bit <<= 1) {
	    	des_ecb_encrypt (data, data, schedule, 1);
	    	if (data[0] & 0x01)
		    auth[i] |= bit;
	    }
    	}
    }
#else
    {
    	int	    seed;
    	int	    value;
    	int	    i;
    
    	seed = (ldata[0]) + (ldata[1] << 16);
    	srand (seed);
    	for (i = 0; i < len; i++)
    	{
	    value = rand ();
	    auth[i] = value & 0xff;
    	}
	value = len;
	if (value > sizeof (key))
	    value = sizeof (key);
    	bcopy (auth, (char *) key, value);
    }
#endif
}
