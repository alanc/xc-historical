/* $XConsortium$ */

/*
 * Compare two areas of memory.
 */
int
memcmp(s1, s2, n)
char	*s1;
char	*s2;
int  n;
{
	return bcmp(s2, s1, n);
}
