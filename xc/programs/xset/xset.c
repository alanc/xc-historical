/* 
 * $Header: xset.c,v 1.4 87/05/08 18:51:42 dkk Locked $ 
 * $Locker: dkk $ 
 */
#include <X11/copyright.h>

/* Copyright    Massachusetts Institute of Technology    1985	*/

#ifndef lint
static char *rcsid_xset_c = "$Header: xset.c,v 1.4 87/05/08 18:51:42 dkk Locked $";
#endif

#include <X11/X.h>      /*  Should be transplanted to X11/Xlibwm.h     %*/
#include <X11/Xlib.h>
/*  #include <X11/Xlibwm.h>  [Doesn't exist yet  -dkk]  %*/
#include <sys/types.h>  /*  Unnecessary.  Aready exists in X11/Xlib.h  %*/
#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <strings.h>
#include <ctype.h>

#define TRUE 1
#define FALSE 0

XKeyboardControl values;
unsigned long  value_mask;  /*  Was int but ulong according to Xlib  %%*/

#define	nextarg(i, argv) \
	argv[i]; \
	if (i >= argc) \
		break; \

main(argc, argv)
int argc;
char **argv;
{
	char *disp = "\0";
	Display	*dpy;
	register char *arg;
	register int i;
	int screen;
/*
 *  These are for XSetScreenSaver:
 */
	int prefer_blank, allow_exp, timeout, interval;
	int repeat = -1;
	int dosaver = 0;
	int blank, expose;
	int dummy1, dummy2;
	int discard = TRUE;
	int pixels[512];
	caddr_t colors[512];
/*
 *  These next two are for mouse (pointer) control.
 */
	unsigned char map[];
	int nmap;
	int numpixels = 0;
	XColor def;  /* was Color, but only XColor, Colormap exist %%*/
	value_mask = 0;          /*  initialize mask  %*/

	if (argc == 1)  usage(argv[0]);

	for (i = 1; i < argc; ) {
		arg = argv[i++];
		if (strcmp(arg, "-c") == 0 || strcmp(arg, "-click") == 0) {
			values.key_click_percent = 0;
			value_mask |= KBKeyClickPercent;
		} 
		else if (strcmp(arg, "c") == 0 || strcmp(arg, "click") == 0) {
			values.key_click_percent = -1;
			value_mask |= KBKeyClickPercent;

			arg = nextarg(i, argv);
			if (strcmp(arg, "on") == 0) {
				i++;
			} 
			else if (strcmp(arg, "off") == 0) {
				values.key_click_percent = 0;
				i++;
			} 
			else if (isnumber(arg, 100)) {
				values.key_click_percent = atoi(arg);
				i++;
			}
		} 
		else if (strcmp(arg, "-b") == 0 || strcmp(arg, "-bell") == 0) {
			values.bell_percent = 0;
			value_mask |= KBBellPercent;
		} 
		else if (strcmp(arg, "b") == 0 || strcmp(arg, "bell") == 0) {
			values.bell_percent = -1;
			value_mask |= KBBellPercent;
			arg = nextarg(i, argv);

			if (strcmp(arg, "on") == 0) {
				i++;
			} 
			else if (strcmp(arg, "off") == 0) {
				values.bell_percent = 0;
				i++;
			} 
			/* volume */
			else if (isnumber(arg, 100)) {
			    values.bell_percent = atoi(arg);
			    i++;

			    /* pitch */
			    arg = nextarg(i, argv);
			    if (isnumber(arg, 20000)) {
				values.bell_pitch = atoi(arg);
				value_mask |= KBBellPitch;
				i++;

				/* duration */
				arg = nextarg(i, argv);
				if (isnumber(arg, 1000)) {
				    values.bell_duration = atoi(arg);
				    value_mask |= KBBellDuration;
				    i++;
				}
			    }
			}
		} 
		else if (strcmp(arg, "-led") == 0) {
			values.led = 0;
			values.led_mode = 0;
			value_mask |= KBLedMode;

		        arg = nextarg(i, argv);
			if (atoi(arg) == 0)
			         i++;
			else if (isnumber(arg, 32) == 0) {
			         values.led = atoi(arg);
				 value_mask |= KBLed;
				 i++;
			}
		} 

		else if (strcmp(arg, "led") == 0) {
		        values.led = 0;
			values.led_mode = ~0L;
			value_mask |= KBLedMode;

			arg = nextarg(i, argv);
			if (strcmp(arg, "on") == 0) {
				i++;
			} 
			else if (strcmp(arg, "off") == 0) {
				values.led_mode = 0;
				i++;
			}
			else if (isnumber(arg, 32) == 0) {
			        values.led = atoi(arg);
			        i++;
			}
		}
		else if (strcmp(arg, "m") == 0 || strcmp(arg, "mouse") == 0) {
		        nmap = 6;
		}
/*			acc = 4;
 *			thresh = 2;
 *			if (i >= argc)
 *				break;
 *			arg = argv[i];
 *			if (strcmp(arg, "default") == 0) {
 *				i++;
			} 
 *			else if (*arg >= '0' && *arg <= '9') {
				acc = atoi(arg);
 *				i++;
				if (i >= argc)
 *					break;
				arg = argv[i];
 *				if (*arg >= '0' && *arg <= '9') {
					thresh = atoi(arg);
 *					i++;
				}
 *			}
 *		} 
 %*/

		else if (strcmp(arg, "s") == 0 || strcmp(arg, "saver") == 0 ||
		    strcmp(arg, "v") == 0 || strcmp(arg, "video") == 0) {
			timeout = 10;
			interval = 60;
			blank = (*arg == ('s' ? DontPreferBlanking :
				 PreferBlanking));
			dosaver = TRUE;
			if (i >= argc)
				break;
			arg = argv[i];
			if (strcmp(arg, "default") == 0) {
			        blank = DefaultBlanking;
				i++;
			} 
			else if (*arg >= '0' && *arg <= '9') {
				timeout = atoi(arg);
				i++;
				if (i >= argc)
					break;
				arg = argv[i];
				if (*arg >= '0' && *arg <= '9') {
					interval = atoi(arg);
					i++;
				}
			}
		} 
		else if (strcmp(arg, "-r") == 0 || strcmp(arg, "-repeat") == 0) {
			repeat = FALSE;
		} 
		else if (strcmp(arg, "r") == 0 || strcmp(arg, "repeat") == 0) {
			repeat = TRUE;
			if (i >= argc)
				break;
			arg = argv[i];
			if (strcmp(arg, "on") == 0) {
				i++;
			} 
			else if (strcmp(arg, "off") == 0) {
				repeat = FALSE;
				i++;
 			}
		} 

/*		else if (strcmp(arg, "p") == 0 || strcmp(arg, "pixel") == 0) {
			if (i + 1 >= argc)
				usage(argv[0]);
 *			arg = argv[i];
			if (*arg >= '0' && *arg <= '9')
				pixels[numpixels] = atoi(arg);
 *			else
				usage(argv[0]);
			i++;
 *			colors[numpixels] = argv[i];
			i++;
			numpixels++;
		} 
%*/

		else if (index(arg, ':')) {
			disp = arg;
		} 
		else
			usage(argv[0]);
	}
	if (!value_mask && !repeat)
		usage(argv[0]);

	dpy = XOpenDisplay(disp);
	
	if (dpy == NULL) {
		fprintf(stderr, "%s: Can't open display '%s'\n",
		argv[0], XDisplayName(disp ? disp : "\0"));
		exit(1);
	}

	XChangeKeyboardControl(dpy, value_mask, &values);

	if (value_mask){
	XGetKeyboardControl(dpy, &values);

	printf ("Display: %d \n", *dpy);
	printf ("Key Click Percent: %d \n", values.key_click_percent);
	printf ("Bell Percent: %d \n", values.bell_percent);
	printf ("Bell Pitch (Hz): %d \n", values.bell_pitch);
	printf ("Bell Duration (msec): %d \n", values.bell_duration);
	printf ("LED: %d \n", values.led);
	printf ("LED Mode: %d \n", values.led_mode);
	printf ("Key: %d \n", values.key);
	printf ("Auto Repeat: %d \n", values.auto_repeat_mode);
        }

	if (nmap){
	  XGetPointerMapping(dpy, map, nmap);
	  printf ("nmap: %d \n", nmap);
	  printf ("map: %c, %c, %c, %c, %c, %c, %c", map[0], map[1],
		  map[2], map[3], map[4], map[5], map[6]);
	}

/*
 *      OBSOLETE -- TO BE DELETED
 *	if (doclick) XKeyClickControl(click);
 *	if (dolock) {
 *		if (lock)  XLockToggle();
 *		else XLockUpDown();
 *	}
 */
	if (repeat == TRUE)
	        XAutoRepeatOn(dpy);
	else if (repeat == FALSE)
		XAutoRepeatOff(dpy);

/*
 *     OBSOLETE -- DELETE
 *	if (dobell) XFeepControl(bell);
 *	if (domouse) XMouseControl(acc, thresh);
 */
/*
 *    The following function sets the ScreenSaver parameters to their
 *  pre-xset values; so if they aren't specified in xset, they don't
 *  get changed.
 */
/*	XGetScreenSaver(dpy, dummy1, dummy2, &prefer_blank, &allow_exp); %%*/

	if (dosaver) XSetScreenSaver(dpy, timeout, interval, 
				     prefer_blank, allow_exp);

/*	screen = DefaultScreen(dpy);
	if (DisplayCells(dpy, screen) >= 2) {
		while (--numpixels >= 0) {
			def.pixel = pixels[numpixels];
			if (XParseColor(colors[numpixels], &def))
				XStoreColor(&def);
			else
				fprintf(stderr, "%s: No such color\n", colors[numpixels]);
		}
	}
 %%*/
	XSync(dpy, discard);
	exit(0);
}

isnumber(arg, maximum)
	char	*arg;
	int maximum;
{
	register char	*p;

	if (arg[0] == '-' && arg[1] == '1' && arg[2] == '\0')
		return(1);
	for (p=arg; isdigit(*p); p++) ;
	if (*p || atoi(arg) > maximum)
		return(0); 
	return(1);
}

usage(prog)
char *prog;
{
	printf("usage: %s option [option ...] [host:vs]\n", prog);
	printf("    To turn bell off:\n");
	printf("\t-b                b off               b 0\n");
	printf("    To set bell volume, pitch and duration:\n");
	printf("\t b [vol [pitch [dur]]]          b on\n");
	printf("    To turn keyclick off:\n");
	printf("\t-c                c off               c 0\n");
	printf("    To set keyclick volume:\n");
	printf("\t c [0-100]        c on\n");
	printf("    To set LED states off or on:\n");
	printf("\t-led [0-31]         led off\n");
	printf("\t led [0-31]         led on\n");
	printf("    To set mouse acceleration and threshold:\n");
	printf("\t m [acc [thr]]    m default\n");
	printf("    To set pixel colors:\n");
	printf("\t p pixel_value color_name\n");
	printf("    To turn auto-repeat off or on:\n");
	printf("\t-r                r off\n");
	printf("\t r                r on\n");
	printf("    To make the screen-saver display a pattern:\n");
	printf("\t s [timeout [cycle]]  s default\n");
	printf("    To make the screen-saver blank the video:\n");
	printf("\t v [timeout [cycle]]  v default\n");
	exit(0);
}
