#include <sys/stropts.h>
#include <sys/termio.h>
#include <sys/video.h>
#include <fcntl.h>
#include <sys/ioctl.h>

static struct termio d_tio = {
	(BRKINT|IGNPAR|ISTRIP|ICRNL|IXON)&(~IGNBRK)&(~PARMRK)&(~INPCK)&(~INLCR)&
	(~IGNCR)&(~IUCLC)&(~IXANY)&(~IXOFF),
	(OPOST|ONLCR)&(~OLCUC)&(~OCRNL)&(~ONOCR)&(~ONLRET)&(~OFILL)&(~OFDEL),
	(B9600|CS7|CREAD)&(~CSTOPB)&(~PARENB)&(~PARODD)&(~HUPCL)&(~CLOCAL)&(~LOBLK),
	(ISIG|ICANON|ECHO|ECHOE|ECHOK)&(~XCASE)&(~ECHONL)&(~NOFLSH),
	0,
	{CINTR, CQUIT, CERASE, CKILL, CEOF, CNUL, CNUL, CNUL}
};

main()
{
	int fd; int line;
	struct strioctl s;

	fd = open("/dev/console", O_RDWR);

	line = ioctl(fd, I_POP, 0); /* remove line discipline and stash it */
	ioctl(fd, I_FLUSH, FLUSHRW); /* flush input to put us in known state */
    
	s.ic_len = 0;
	s.ic_cmd = VIDEO_NOMOUSE;
	ioctl(fd, I_STR, &s);
	s.ic_len = 0;
	s.ic_cmd = VIDEO_ASCII;
	ioctl(fd, I_STR, &s);
	ioctl(fd, I_PUSH, "line");

	ioctl(fd, TCSETA, &d_tio);

}
