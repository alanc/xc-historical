#include <stdio.h>
#include <X11/Xtrans.h>

/*
 * parsetest
 *
 * This test programs will invoke the main parseing routine on a list of
 * addresses to determine if the parsing algorith is working correctly.
 * Of course, this test is only as good as the list of addresses supplied,
 * so the list below should probably continue to grow.
 */

typedef	struct	{
	char	*addr;
	char	*protocol;
	char	*host;
	char	*port;
	} addr;

addr	address[] = {
	{"","local","local",""},
	{":","local","local",""},
	{"/:","local","local",""},
	{"/"THISHOST":","inet",THISHOST,""},
	{"local/"THISHOST":","local",THISHOST,""},
	{"local/:","local","local",""},
	{"inet/"THISHOST":","inet",THISHOST,""},
	{"inet/:","inet","local",""},
	{"::","decnet","local",""},
	{THISHOST"::0","decnet",THISHOST,"0"},
	{"/::","decnet","local",""},
	{"/"THISHOST"::0","decnet",THISHOST,"0"},
	{"decnet/:","decnet","local",""},
	{"decnet/::","decnet","local",""},
	{":0","local","local","0"},
	{":/tmp/.ICE/test","local","local","/tmp/.ICE/test"},
	{"/:0","local","local","0"},
	{"/:/tmp/.ICE/test","local","local","/tmp/.ICE/test"},
	{"pts/:0","pts","local","0"},
	{"pts/:/tmp/.ICE/test","pts","local","/tmp/.ICE/test"},
	{"unix/:0","unix","local","0"},
	{"unix/:/tmp/.ICE/test","unix","local","/tmp/.ICE/test"},
	{"inet/:0","inet","local","0"},
	{"inet/"THISHOST":0","inet",THISHOST,"0"},
	{"tli/:0","tli","local","0"},
	{"tli/"THISHOST":0","tli",THISHOST,"0"},
	{"decnet/"THISHOST":0","decnet",THISHOST,"0"},
	};

#define NUMADDRESS	(sizeof(address)/sizeof(addr))

main()
{
int	i;
char	*protocol;
char	*host;
char	*port;

for(i=0;i<NUMADDRESS;i++)
	{
	if(_TESTTransParseAddress(address[i].addr,&protocol,&host,&port) == 0)
		{
		fprintf(stderr,"%%%%Failed to parse \"%s\"\n", address[i].addr );
		continue;
		}
	fprintf(stderr,"%-20s becomes %s/%s:%s\n",
		address[i].addr, protocol, host, port );
	/*
	 * validate the result against what is expected.
	 */
	if( strcmp(address[i].protocol,protocol) )
		{
		fprintf(stderr,"%%%%Parse error: %s doesn't match %s\n",
						address[i].protocol, protocol );
		}
	if( strcmp(address[i].host,host) )
		{
		fprintf(stderr,"%%%%Parse error: %s doesn't match %s\n",
						address[i].host, host );
		}
	if( strcmp(address[i].port,port) )
		{
		fprintf(stderr,"%%%%Parse error: %s doesn't match %s\n",
						address[i].port, port );
		}
	}
}
