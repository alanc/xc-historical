/*
kbd_mode: libcps.a kbd_mode.o
	cc -o $@ $@.o libcps.a -lm
kbd_mode.o: kbd_mode.c
=========================kbd_mode.c=========================
 * NeWS is a product of Sun Microsystems, Inc. and is provided for
 * unrestricted use provided that this legend is included on all tape
 * media and as a part of the software program in whole or part.  Users
 * may copy or modify NeWS without charge, but are not authorized to
 * license or distribute it to anyone else except as part of a product
 * or program developed by the user.
 *
 * NEWS IS PROVIDED AS IS WITH NO WARRANTIES OF ANY KIND INCLUDING THE
 * WARRANTIES OF DESIGN, MERCHANTIBILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE, OR ARISING FROM A COURSE OF DEALING, USAGE OR TRADE PRACTICE.
 *
 * NeWS is provided with no support and without any obligation on the
 * part of Sun Microsystems, Inc. to assist in its use, correction,
 * modification or enhancement.
 *
 * SUN MICROSYSTEMS, INC. SHALL HAVE NO LIABILITY WITH RESPECT TO THE
 * INFRINGEMENT OF COPYRIGHTS, TRADE SECRETS OR ANY PATENTS BY NEWS
 * OR ANY PART THEREOF.
 *
 * In no event will Sun Microsystems, Inc. be liable for any lost revenue
 * or profits or other special, indirect and consequential damages, even
 * if Sun has been advised of the possibility of such damages.
 *
 * Sun Microsystems, Inc.
 * 2550 Garcia Avenue
 * Mountain View, California  94043
 */

#ifndef lint
static  char sccsid[] = "@(#)kbd_mode.c 7.1 87/04/13";
#endif

/*
 * Copyright (c) 1986 by Sun Microsystems, Inc.
 *
 *      kbd_mode:       set keyboard encoding mode
 */

#include <sys/types.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sundev/kbio.h>
#include <sundev/kbd.h>
#include <stdio.h>

static void         die(), usage();
static int          kbd_fd;

main(argc, argv)
    int             argc;
    char          **argv;
{
    int             code, translate;

    if ((kbd_fd = open("/dev/kbd", O_RDONLY, 0)) < 0) {
	die("Couldn't open /dev/kbd");
    }
    argc--; argv++;
    if (argc-- && **argv == '-') {
	code = *(++*argv);
    } else {
	usage();
    }
    switch (code) {
      case 'a':
      case 'A':
	translate = TR_ASCII;
	break;
      case 'e':
      case 'E':
	translate = TR_EVENT;
	break;
      case 'n':
      case 'N':
	translate = TR_NONE;
	break;
      case 'u':
      case 'U':
	translate = TR_UNTRANS_EVENT;
	break;
      default:
	usage();
    }
    if (ioctl(kbd_fd, KIOCTRANS, (caddr_t) &translate)) {
	die("Couldn't initialize translation to Event");
    }
    exit(0);
}

static void
die(msg)
    char        *msg;
{
    fprintf(stderr, "%s\n", msg);
    exit(1);
}

static void
usage()
{
    int             translate;

    if (ioctl(kbd_fd, KIOCGTRANS, (caddr_t) &translate)) {
	die("Couldn't inquire current translation");
     }
    fprintf(stderr, "kbd_mode {-a | -e | -n | -u }\n");
    fprintf(stderr, "\tfor ascii, encoded (normal) SunView events,\n");
    fprintf(stderr, " \tnon-encoded, or unencoded SunView events, resp.\n");
    fprintf(stderr, "Current mode is %s.\n",
		(   translate == 0 ?    "n (non-translated bytes)"      :
		 (  translate == 1 ?    "a (ascii bytes)"               :
		  ( translate == 2 ?    "e (encoded events)"            :
		  /* translate == 3 */  "u (unencoded events)"))));
    exit(1);
}


