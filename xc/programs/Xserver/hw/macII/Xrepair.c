/************************************************************ 
Copyright 1988 by Apple Computer, Inc, Cupertino, California
			All Rights Reserved

Permission to use, copy, modify, and distribute this software
for any purpose and without fee is hereby granted, provided
that the above copyright notice appear in all copies.

APPLE MAKES NO WARRANTY OR REPRESENTATION, EITHER EXPRESS,
OR IMPLIED, WITH RESPECT TO THIS SOFTWARE, ITS QUALITY,
PERFORMANCE, MERCHANABILITY, OR FITNESS FOR A PARTICULAR
PURPOSE. AS A RESULT, THIS SOFTWARE IS PROVIDED "AS IS,"
AND YOU THE USER ARE ASSUMING THE ENTIRE RISK AS TO ITS
QUALITY AND PERFORMANCE. IN NO EVENT WILL APPLE BE LIABLE 
FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
DAMAGES RESULTING FROM ANY DEFECT IN THE SOFTWARE.

THE WARRANTY AND REMEDIES SET FORTH ABOVE ARE EXCLUSIVE
AND IN LIEU OF ALL OTHERS, ORAL OR WRITTEN, EXPRESS OR
IMPLIED.

************************************************************/
#include <sys/stropts.h>
#include <sys/termio.h>
#include <sys/video.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>

static struct termio d_tio = {
	(BRKINT|IGNPAR|ISTRIP|ICRNL|IXON)&(~IGNBRK)&(~PARMRK)&(~INPCK)&(~INLCR)&
	(~IGNCR)&(~IUCLC)&(~IXANY)&(~IXOFF),
	(OPOST|ONLCR)&(~OLCUC)&(~OCRNL)&(~ONOCR)&(~ONLRET)&(~OFILL)&(~OFDEL),
	(B9600|CS7|CREAD)&(~CSTOPB)&(~PARENB)&(~PARODD)&(~HUPCL)&(~CLOCAL)&(~LOBLK),
	(ISIG|ICANON|ECHO|ECHOE|ECHOK)&(~XCASE)&(~ECHONL)&(~NOFLSH),
	0,
	{CINTR, CQUIT, CERASE, CKILL, CEOF, CNUL, CNUL, CNUL}
};

/*
 * Check to see that the server restored the "line" streams module
 * on /dev/console. If so, we'll presume all is well. If not, clear 
 * all stacked modules, push "line", and establish workable stty values.
 */
main()
{
	int fd; int line;
	int iarg;
	struct strioctl s;
	char buff[FMNAMESZ+1];

	if ((fd = open("/dev/console", O_RDWR)) < 0) {
	    printf("Xrepair: can't open /dev/console\n");
	} else if (ioctl(fd, I_FIND, "line") == 0) {
	    errno = 0;
	    ioctl(fd, I_LOOK, buff);
	    while (errno != EINVAL) {
		if(ioctl(fd, I_POP, 0) < 0) {
			printf("Failed to ioctl I_POP %s.\n", buff);
		}
		ioctl(fd, I_LOOK, buff);
	    }

	    ioctl(fd, I_FLUSH, FLUSHRW); 
    
	    iarg = 0;
	    if (ioctl(fd, FIONBIO, &iarg) < 0) {
		printf("Failed to FIONBIO.\r\n");
	    }

	    s.ic_len = 0;
	    s.ic_cmd = VIDEO_NOMOUSE;
	    ioctl(fd, I_STR, &s);
	    s.ic_len = 0;
	    s.ic_cmd = VIDEO_ASCII;
	    ioctl(fd, I_STR, &s);
	    ioctl(fd, I_PUSH, "line");
    
	    ioctl(fd, TCSETA, &d_tio);
	}
    
}
