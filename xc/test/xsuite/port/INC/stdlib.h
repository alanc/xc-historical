/* $XConsortium$ */

#define EXIT_SUCCESS	0
#define EXIT_FAILURE	1

#ifndef NULL
#define NULL 0

#ifndef _SIZE_T
#define _SIZE_T
	typedef unsigned int size_t;

extern	void	*malloc();
extern	void	*calloc();
extern	char	*getenv();
