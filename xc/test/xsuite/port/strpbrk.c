/* $XConsortium$ */
/* A version of strpbrk for systems without it. */

/* Return pointer to first occurence of any character from `bset' */
/* in `string'. NULL is returned if none exists. */

#ifndef	NULL
#define	NULL	(char *) 0
#endif

char *
strpbrk(str, bset)
char *str;
char *bset;
{
	char *ptr;

	do {
		for(ptr=bset; *ptr != '\0' && *ptr != *str; ++ptr)
			;
		if(*ptr != '\0')
			return(str);
	}
	while(*str++);
	return(NULL);
}
