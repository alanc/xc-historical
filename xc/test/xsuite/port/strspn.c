/* $XConsortium$ */
/*
 * returns the number of contiguous characters from the beginning
 * of str that are in set.
 * 
 * A simple emulation provided for (eg BSD) systems that do not provide
 * this routine.
 */
int
strspn(str, set)
char	*str;
char	*set;
{
char	*cp, *sp;

	for (cp = str; *cp; cp++) {
		for (sp = set; *sp && *cp != *sp; sp++)
			;

		if (*sp == '\0')
			break;

	}
	return(cp - str);
}

/*
 * returns the number of contiguous characters from the beginning
 * of str that are not in set.
 * 
 * A simple emulation provided for (eg BSD) systems that do not provide
 * this routine.
 */
int
strcspn(str, set)
char	*str;
char	*set;
{
char	*cp, *sp;

	for (cp = str; *cp; cp++) {
		for (sp = set; *sp && *cp != *sp; sp++)
			;

		if (*sp != '\0')
			break;

	}
	return(cp - str);
}
