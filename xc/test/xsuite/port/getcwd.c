/* $XConsortium$ */

#include <stdlib.h>
#include <errno.h>
#include <sys/param.h>

/*
 * A simple emulation of getcwd() to allow the tcc to run on BSD systems
 * that don't provide this.
 * getcwd() differs from the BSD getwd() by specifing a size.
 * 
 * Do not use if the system provides getcwd()
 * 
 */
char *
getcwd(buf, size)
char	*buf;
int 	size;
{
char	*bp;
char	*getwd();

	if (size <= 0) {
		errno = EINVAL;
		return((char*) 0);
	}

	/*
	 * get a buffer that hold the max possible BSD pathname.
	 */
	bp = (char *)malloc((size_t)(MAXPATHLEN+1));
	if (bp == (char *)0) {
		errno = ENOMEM;
		return((char *) 0);
	}

	if (getwd(bp) == (char *)0) {
		/* errno should have been set */
		return((char *) 0);
	}

	if (strlen(bp) >= size) {
		errno = ERANGE;
		return((char *) 0);
	}

	strcpy(buf, bp);

	free(bp);

	return(buf);
	
}
