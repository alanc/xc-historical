/* $XConsortium: cfbpolypnt.c,v 5.16 94/03/16 16:53:04 dpw Exp $ */

/*
 * Copyright 1988 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */

/*
 * special socket routine for hp
 */

#include <sys/types.h>
#include <sys/socket.h>

int
set_socket_option (socket_id, option)
int socket_id;
char option;
{
	int optlen = 1;
	char optval = 0x0;

	getsockopt (socket_id, SOL_SOCKET, option, &optval, &optlen);

	optval |= option;

	setsockopt (socket_id, SOL_SOCKET, option, &optval, 1);
}


int
unset_socket_option (socket_id, option)
int socket_id;
char option;
{
	int optlen = 1;
	char optval = 0x0;

	getsockopt (socket_id, SOL_SOCKET, option, &optval, &optlen);

	optval &= ~option;

	setsockopt (socket_id, SOL_SOCKET, option, &optval, 1);
}
