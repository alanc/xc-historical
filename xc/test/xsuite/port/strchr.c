/* $XConsortium$ */
/*
 * Emulations of strchr() and strrchr() for BSD systems without
 * them
 */

char	*index();
char	*rindex();

char *
strchr(s1, c)
char	*s1;
int 	c;
{
	return(index(s1, c));
}

char *
strrchr(s1, c)
char	*s1;
int 	c;
{
	return(rindex(s1, c));
}
