/* $XConsortium$ */


/*
 * A version of strtok for systems that do not have it
 */
char *
strtok(s1, sep)
char	*s1;
char	*sep;
{
static	char	*savpos;
int 	n;

	if (s1)
		savpos = s1;
	else
		s1 = savpos;

	/* Skip over leading separators */
	n = strspn(s1, sep);
	s1 += n;

	n = strcspn(s1, sep);
	if (n == 0)
		return((char *) 0);

	savpos = s1 + n;
	if (*savpos != '\0')
		*savpos++ = '\0';

	return(s1);
}

#ifdef test
#include <stdio.h>
main()
{
char *s;
char	*strtok();

	s = strtok("this is   :, . string", " :,");
	do {
		printf("%s\n", s);
		fflush(stdout);
	} while ((s = strtok((char*)0, " :,")) != 0);

}
#endif
