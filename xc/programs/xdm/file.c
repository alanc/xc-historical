/*
 * file.c
 */

# include	"dm.h"
# include	"buf.h"

struct display *
ReadDisplay (file)
struct buffer	*file;
{
	int		c;
	char		**args;
	struct display	*d;
	char		word[1024];
	int		i;

	int		isNew = 0;

	c = readWord (file, word, sizeof (word));
	if (word[0] != '\0') {
		d = FindDisplayByName (word);
		if (!d) {
			d = NewDisplay (word);
			isNew = 1;
		}
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
		return d;
	}
	return 0;
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
