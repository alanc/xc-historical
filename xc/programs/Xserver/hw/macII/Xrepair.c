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

/*
 * Under A/UX >=2.0 struct video has been "Macintized" and now incorporates
 * a structure called AuxDCE which is defined once and for all in
 * /usr/include/mac. Alas the definition for AuxDCE requires wheeling in
 * lots of Mac stuff including QuickDraw. This is a headache as there are
 * a variety of clashes between X and QuickDraw (they both do windows after
 * all). So we define just what we need here and avoid pulling in the Mac
 * includes. Of course if this ever changes ...
 */

#if !defined(__mac_types_h) && !defined(__TYPES__)
#define __mac_types_h
typedef unsigned char Boolean;
typedef long (*ProcPtr)();
typedef short OSErr;
typedef char *Ptr;

struct Rect {
    short top;
    short left;
    short bottom;
    short right;
};

typedef struct Rect Rect;
#endif /* __mac_types_h */

#if !defined(__mac_quickdraw_h)
#define __mac_quickdraw_h
struct RGBColor {
    unsigned short red;
    unsigned short green;
    unsigned short blue;
};

typedef struct RGBColor RGBColor;

struct ColorSpec {
    short value;
    RGBColor rgb;
};

typedef struct ColorSpec ColorSpec;
#endif /* __mac_quickdraw_h */

#if !defined(__mac_osutils_h)
#define __mac_osutils_h
struct QElem {
    struct QElem *qLink;
    short qType;
    short qData[1];
};

typedef struct QElem QElem;

typedef QElem *QElemPtr;

struct QHdr {
    short qFlags;
    QElemPtr qHead;
    QElemPtr qTail;
};

typedef struct QHdr QHdr;
#endif /* __mac_osutils_h */

#if !defined(__mac_files_h)
#define __mac_files_h
struct CntrlParam {
	QElem *qLink;
	short qType;
	short ioTrap;
	Ptr ioCmdAddr;
	ProcPtr ioCompletion;
	OSErr ioResult;
	unsigned char *ioNamePtr;
	short ioVRefNum;
	short ioCRefNum;
	short csCode;
	short csParam[11];
};
#endif /* __mac_files_h */

#if !defined(__mac_devices_h)
#define __mac_devices_h
struct AuxDCE {
    Ptr dCtlDriver;
    short dCtlFlags;
    QHdr dCtlQHdr;
    long dCtlPosition;
    Ptr *dCtlStorage;
    short dCtlRefNum;
    long dCtlCurTicks;
    Ptr dCtlWindow;
    short dCtlDelay;
    short dCtlEMask;
    short dCtlMenu;
    char dCtlSlot;
    char dCtlSlotId;
    long dCtlDevBase;
    Ptr dCtlOwner;
    char dCtlExtDev;
    char fillByte;
};

typedef struct AuxDCE AuxDCE;

typedef AuxDCE *AuxDCEPtr;
#endif /* __mac_devices_h */

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
	struct strioctl ctl;
	char buff[FMNAMESZ+1];
	int errors = 0;

	if ((fd = open("/dev/console", O_RDWR)) < 0) {
	    printf("Xrepair: can't open /dev/console\n");
	} else if (ioctl(fd, I_FIND, "line") == 0) {
#ifdef CONS_UNDIRECT
            ctl.ic_len = 0;
            ctl.ic_cmd = CONS_UNDIRECT;
            if (ioctl(fd, I_STR, &ctl) < 0) {
                errors++;
                printf("Failed to ioctl I_STR CONS_UNDIRECT.\r\n");
            }
#endif
	    iarg = 0;
	    if (ioctl(fd, FIONBIO, &iarg) < 0) {
		errors++;
		printf("Could not ioctl FIONBIO. \r\n");
	    }
	    
	    iarg = 0;
	    if (ioctl(fd, FIOASYNC, &iarg) < 0) {
		errors++;
		printf("Could not ioctl FIOASYNC. \r\n");
	    }
	    
	    if (ioctl(fd, I_FLUSH, FLUSHRW) < 0) {
		errors++;
		printf("Failed to ioctl I_FLUSH FLUSHRW.\r\n");
	    }
	    
	    ctl.ic_len = 0;
	    ctl.ic_cmd = VIDEO_NOMOUSE;
	    if (ioctl(fd, I_STR, &ctl) < 0) {
		errors++;
		printf("Failed to ioctl I_STR VIDEO_NOMOUSE.\r\n");
	    }
	    
#ifdef VIDEO_MAC
	    ctl.ic_len = 0;
	    ctl.ic_cmd = VIDEO_MAC; /* For A/UX 2.0 and later */
	    if (ioctl(fd, I_STR, &ctl) < 0) {
	        ctl.ic_len = 0;
	        ctl.ic_cmd = VIDEO_ASCII; /* A/UX 1.* */
	        if (ioctl(fd, I_STR, &ctl) < 0) {
		    errors++;
		    printf("Failed to ioctl I_STR VIDEO_MAC VIDEO_ASCII.\r\n");
	        }
	    }
#else
	    ctl.ic_len = 0;
	    ctl.ic_cmd = VIDEO_ASCII;
	    if (ioctl(fd, I_STR, &ctl) < 0) {
		errors++;
		printf("Failed to ioctl I_STR VIDEO_ASCII.\r\n");
	    }
#endif

	    if(ioctl(fd, I_PUSH, "line") < 0) {
		errors++;
		printf("Failed to ioctl I_PUSH.\r\n");
	    }

	    if (ioctl(fd, TCSETA, &d_tio) < 0) {
		errors++;
		printf("Failed to ioctl TCSETA.\r\n");
	    }
	}
    exit (errors);
}
