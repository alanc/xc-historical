/*
 * a simple linked list of known displays
 */

# include "dm.h"

struct display	*displays;

StartDisplays ()
{
	struct display	*d;

	for (d = displays; d; d = d->next)
		if (d->status == notRunning)
			StartDisplay (d);
}

struct display *
FindDisplayByName (name)
char	*name;
{
	struct display	*d;

	for (d = displays; d; d = d->next)
		if (!strcmp (name, d->name))
			return d;
	return 0;
}

struct display *
FindDisplayByPid (pid)
int	pid;
{
	struct display	*d;

	for (d = displays; d; d = d->next)
		if (pid == d->pid)
			return d;
	return 0;
}

RemoveDisplay (old)
struct display	*old;
{
	struct display	*d, *p;

	p = 0;
	for (d = displays; d; d = d->next)
		if (d == old) {
			if (p)
				p->next = d->next;
			else
				displays = d->next;
			free (d);
			break;
		}
}

struct display *
NewDisplay (name)
char	*name;
{
	struct display	*d;

	d = (struct display *) malloc (sizeof (struct display));
	if (!d)
		LogPanic ("out of memory\n");
	d->next = displays;
	d->name = strcpy (malloc (strlen (name) + 1), name);
	if (!d->name)
		LogPanic ("out of memory\n");
	d->argv = 0;
	d->status = notRunning;
	d->pid = -1;
	displays = d;
	LoadDisplayResources (d);
	return d;
}
