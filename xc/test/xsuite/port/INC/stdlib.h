/* $XConsortium: stdlib.h,v 1.1 92/06/11 15:29:49 rws Exp $ */

#define EXIT_SUCCESS	0
#define EXIT_FAILURE	1

#ifndef NULL
#define NULL 0
#endif

#ifndef _SIZE_T
#define _SIZE_T
	typedef unsigned int size_t;
#endif

extern	void	*malloc();
extern	void	*calloc();
extern	char	*getenv();
