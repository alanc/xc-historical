/*
 * file.c
 */

# include	"dm.h"
# include	"buf.h"
# include	<signal.h>

ReadDisplay (file, acceptableTypes, sockaddr)
struct buffer	*file;
DisplayType	*acceptableTypes;
char		*sockaddr;
{
	int		c;
	char		**args;
	struct display	*d;
	DisplayType	type;
	char		word[1024];
	char		typeName[1024];
	int		i;

	c = readWord (file, word, sizeof (word));
	if (word[0] != '\0') {
		if (c == EOB || c == '\n') {
			LogError ("missing display type for display %s", word);
			return c;
		}
		c = readWord (file, typeName, sizeof (typeName));
		type = parseDisplayType (typeName);
		while (*acceptableTypes != unknown)
			if (*acceptableTypes++ == type)
				goto acceptable;
		LogError ("unacceptable display type %s for display %s\n", typeName, word);
		return c;
acceptable:;
		switch (type) {
		case unknown:
			break;
		case remove:
			if (d = FindDisplayByName (word))
				if (d->displayType != secure) {
					Debug ("removing display %s\n", d->name);
					TerminateDisplay (d);
				}
			Debug ("not removing display %s\n", word);
			break;
		default:
			if (d = FindDisplayByName (word)) {
				LogError ("Attempt to start running display %s\n",
					word);
				break;
			}
			d = NewDisplay (word);
			d->displayType = type;
#ifdef UDP_SOCKET
			if (sockaddr)
				d->addr = *(struct sockaddr_in *) sockaddr;
#endif
			i = 0;
			args = (char **) malloc (sizeof (char *));
			if (!args)
				LogPanic ("out of memory\n");
			while (c != EOB && c != '\n') {
				c = readWord (file, word, sizeof (word));
				if (word[0] != '\0') {
					args[i] = strcpy (malloc (strlen (word) + 1), word);
					i++;
					args = (char **) 
					    realloc ((char *) args, (i+1) * sizeof (char **));
				}
			}
			args[i] = 0;
			d->argv = args;
		}
		while (c != EOB && c != '\n')
			c = readWord (file, word, sizeof (word));
	}
	return c;
}

# define isbreak(c)	((c) == ' ' || (c) == '\t' || (c) == '\n')

int
readWord (file, word, len)
struct buffer	*file;
char	*word;
int	len;
{
	int	c;
	int	i;

	while ((c = bufc (file)) != EOB)
		if (!isbreak (c))
			break;
	if (c == EOB) {
		word[0] = '\0';
		return EOB;
	}
	word[0] = c;
	i = 1;
	while ((c = bufc (file)) != EOB && !isbreak (c)) {
		if (i < len-1 && c != '\r') {
			word[i] = c;
			++i;
		}
	}
	word[i] = '\0';
	return c;
}

static struct displayMatch {
	char		*name;
	DisplayType	type;
} displayTypes[] = {
	"secure",	secure,
	"insecure",	insecure,
	"foreign",	foreign,
	"transient",	transient,
	"remove",	remove,
	0,		unknown
};

DisplayType
parseDisplayType (string)
	char	*string;
{
	struct displayMatch	*d;

	for (d = displayTypes; d->name; d++)
		if (!strcmp (d->name, string))
			return d->type;
	return d->type;
}
