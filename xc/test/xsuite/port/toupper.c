/* $XConsortium$ */


/*
 * A function version of toupper().
 * The BSD4.2 version of toupper in ctype.h does
 * not leave non-lowercase letters unchanged.
 */
toupper(c)
int 	c;
{
	return(upcase(c));
}

tolower(c)
int 	c;
{
	return(locase(c));
}


#include <ctype.h>

upcase(c)
int 	c;
{
	if (islower(c))
		return(toupper(c));
	else
		return(c);
}

locase(c)
int 	c;
{
	if (isupper(c))
		return(tolower(c));
	else
		return(c);
}
