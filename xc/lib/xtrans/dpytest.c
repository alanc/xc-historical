#include <stdio.h>
#include <X11/Xlib.h>

char	*displays[] = {
	":0",
	":1",
	":2",
	"unix:0",
	"unix:1",
	"unix:2",
	"local:0",
	"local:1",
	"local:2",
	THISHOST":0",
	THISHOST":1",
	THISHOST":2",
	"153.78.17.16:0",
	"153.78.17.16:1",
	"local/:0",
	"local/:1",
	"local/:2",
	"local/"THISHOST":0",
	"local/"THISHOST":1",
	"local/"THISHOST":2",
	"tcp/"THISHOST":0",
	"tcp/"THISHOST":1",
	"tcp/"THISHOST":2",
	"inet/"THISHOST":0",
	"inet/"THISHOST":1",
#ifdef TLICONN
	"tli/"THISHOST":0",
	"tli/"THISHOST":1",
#endif
#ifdef DNETCONN
	"::0",
	"::1",
	THISHOST"::0",
	THISHOST"::1",
	"local/::0",
	"local/::1",
	"decnet/"THISHOST"::0",
	"decnet/"THISHOST"::1",
	"decnet/"THISHOST":0",
	"decnet/"THISHOST":1",
#endif
#ifdef LOCALCONN
	"pts/"THISHOST":0",
	"pts/"THISHOST":1",
	"named/"THISHOST":0",
	"named/"THISHOST":1",
	"ics/"THISHOST":0",
	"ics/"THISHOST":1",
	"sco/"THISHOST":0",
	"sco/"THISHOST":1",
#endif
	};

#define NUMDISPLAYS	(sizeof(displays)/sizeof(char *))

int	numpassed=0;
int	numfailed=0;

main()
{
int	i;
Display	*disp;

for( i=0; i<NUMDISPLAYS; i++ )
	{
/*
	fprintf(stderr,"** Opening %s\n",displays[i]);
*/
	disp=XOpenDisplay(displays[i]);
	if( disp == NULL )
		{
		fprintf(stderr,"%%%% Failed to open %s\n",displays[i]);
		numfailed++;
		}
	else
		{
		numpassed++;
/*
		fprintf(stderr,"** Successful\n");
*/
		XCloseDisplay(disp);
		}
	}
fprintf(stderr,"Pass: %d\tFail: %d\tTotal: %d\n",
		numpassed, numfailed, numpassed+numfailed );
}
