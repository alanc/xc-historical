/* $XConsortium$ */
/******************************************************************************

Copyright 1993 by the Massachusetts Institute of Technology,

                        All Rights Reserved

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that copyright
notice and this permission notice appear in supporting documentation, and that
the name of M.I.T. not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior permission.
M.I.T. makes no representations about the suitability of this software for any
purpose.  It is provided "as is" without express or implied warranty.

Author: Ralph Mor, X Consortium
******************************************************************************/

#include <X11/SM/SMlib.h>
#include <X11/SM/SMlibint.h>
#include <X11/Xtrans.h>
#include <stdio.h>

#ifndef WIN32

#ifdef TCPCONN
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#endif

#else /* WIN32 */

#define BOOL wBOOL
#undef Status
#define Status wStatus
#include <winsock.h>
#undef Status
#define Status int
#undef BOOL
#include <X11/Xw32defs.h>
#undef close
#define close closesocket

#endif /* WIN32 */


static char *hex_table[] = {	/* for generating client IDs */
    "00", "01", "02", "03", "04", "05", "06", "07", 
    "08", "09", "0a", "0b", "0c", "0d", "0e", "0f", 
    "10", "11", "12", "13", "14", "15", "16", "17", 
    "18", "19", "1a", "1b", "1c", "1d", "1e", "1f", 
    "20", "21", "22", "23", "24", "25", "26", "27", 
    "28", "29", "2a", "2b", "2c", "2d", "2e", "2f", 
    "30", "31", "32", "33", "34", "35", "36", "37", 
    "38", "39", "3a", "3b", "3c", "3d", "3e", "3f", 
    "40", "41", "42", "43", "44", "45", "46", "47", 
    "48", "49", "4a", "4b", "4c", "4d", "4e", "4f", 
    "50", "51", "52", "53", "54", "55", "56", "57", 
    "58", "59", "5a", "5b", "5c", "5d", "5e", "5f", 
    "60", "61", "62", "63", "64", "65", "66", "67", 
    "68", "69", "6a", "6b", "6c", "6d", "6e", "6f", 
    "70", "71", "72", "73", "74", "75", "76", "77", 
    "78", "79", "7a", "7b", "7c", "7d", "7e", "7f", 
    "80", "81", "82", "83", "84", "85", "86", "87", 
    "88", "89", "8a", "8b", "8c", "8d", "8e", "8f", 
    "90", "91", "92", "93", "94", "95", "96", "97", 
    "98", "99", "9a", "9b", "9c", "9d", "9e", "9f", 
    "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", 
    "a8", "a9", "aa", "ab", "ac", "ad", "ae", "af", 
    "b0", "b1", "b2", "b3", "b4", "b5", "b6", "b7", 
    "b8", "b9", "ba", "bb", "bc", "bd", "be", "bf", 
    "c0", "c1", "c2", "c3", "c4", "c5", "c6", "c7", 
    "c8", "c9", "ca", "cb", "cc", "cd", "ce", "cf", 
    "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", 
    "d8", "d9", "da", "db", "dc", "dd", "de", "df", 
    "e0", "e1", "e2", "e3", "e4", "e5", "e6", "e7", 
    "e8", "e9", "ea", "eb", "ec", "ed", "ee", "ef", 
    "f0", "f1", "f2", "f3", "f4", "f5", "f6", "f7", 
    "f8", "f9", "fa", "fb", "fc", "fd", "fe", "ff", 
};



char *
SmsGenerateClientID (smsConn)

SmsConn smsConn;

{
    char hostname[256];
    char address[14];
    char temp[256];
    char *id;
    static int sequence = 0;
    struct timeval time;

    if (gethostname (hostname, sizeof (hostname)))
	return (NULL);

#ifdef TCPCONN
    {
    struct hostent *tcp_hostent = gethostbyname (hostname);
    char *inet_addr = (char *) inet_ntoa (
	*(struct in_addr *)(tcp_hostent->h_addr));
    char temp[4], *ptr1, *ptr2;
    unsigned char decimal[4];
    int i, len;

    for (i = 0, ptr1 = inet_addr; i < 3; i++)
    {
	ptr2 = strchr (ptr1, '.');
	len = ptr2 - ptr1;
	if (!ptr2 || len > 3)
	    return (NULL);
	strncpy (temp, ptr1, len);
	temp[len] = '\0';
	decimal[i] = atoi (temp);
	ptr1 = ptr2 + 1;
    }

    decimal[3] = atoi (ptr1);

    address[0] = '1';
    address[1] = '\0';
    for (i = 0; i < 4; i++)
	strcat (address, hex_table[decimal[i]]);
    }
#else
    return (NULL);
#endif

#if defined(SVR4) || defined(WIN32) || defined(VMS)
    gettimeofday (&time);
#else
    gettimeofday (&time, NULL);
#endif

    sprintf (temp, "1%s%.13ld%.10d%.4d", address, time.tv_sec,
        getpid (), sequence);

    if (++sequence > 9999)
	sequence = 0;

    if ((id = malloc (strlen (temp) + 1)) != NULL)
	strcpy (id, temp);

    return (id);
}
