#include	<stdio.h>
#include	<ctype.h>

char *
GetLine(fp, Buffer, Length)
	FILE	*fp;
	char	*Buffer;
	int	Length;
{
	int 	i = 0, c;
	char	*p = Buffer;
	
	Length--;			    /* Save room for final NULL */
	
	while (i < Length && (c = getc (fp)) != EOF && c != '\n')
		if (p)
			*p++ = c;
	if (c == '\n' && p)		    /* Retain the newline like fgets */
		*p++ = c;
	if (c == '\n')
		ungetc(c, fp);
	if (p)	
		*p = NULL;
	return (Buffer);
} 

char *
GetWord(fp, Buffer, Length)
	FILE	*fp;
	char	*Buffer;
	int	Length;
{
	int 	i = 0, c;
	char	*p = Buffer;
	
	Length--;			    /* Save room for final NULL */
	while ((c = getc (fp)) != EOF && isspace(c))
		;
	if (c != EOF)
		ungetc(c, fp);
	while (i < Length && (c = getc (fp)) != EOF && !isspace(c))
		if (p)
			*p++ = c;
	if (c != EOF)
		ungetc(c, fp);
	if (p)
		*p = NULL;
	return (Buffer);
} 

GetNumber(fp)
	FILE	*fp;
{
	int	i = 0,  c;

	while ((c = getc (fp)) != EOF && isspace(c))
		;
	if (c != EOF)
		ungetc(c, fp);
	while ((c = getc (fp)) != EOF && isdigit(c))
		i = i*10 + c - '0';
	if (c != EOF)
		ungetc(c, fp);
	return (i);
}
	
	    
