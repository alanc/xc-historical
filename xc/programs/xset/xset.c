/* 
 * $Header: xset.c,v 1.10 87/05/18 09:32:18 dkk Locked $ 
 * $Locker: dkk $ 
 */
#include <X11/copyright.h>

/* Copyright    Massachusetts Institute of Technology    1985	*/

#ifndef lint
static char *rcsid_xset_c = "$Header: xset.c,v 1.10 87/05/18 09:32:18 dkk Locked $";
#endif

#include <X11/X.h>      /*  Should be transplanted to X11/Xlibwm.h     %*/
#include <X11/Xlib.h>
/*  #include <X11/Xlibwm.h>  [Doesn't exist yet  5-14-87]  %*/
/*  #include <sys/types.h>  /*  Unnecessary.  Aready exists in X11/Xlib.h  %*/
#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <strings.h>
#include <ctype.h>

#define TRUE 1
#define FALSE 0

XKeyboardControl values;
unsigned long  value_mask;

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
	int dosaver = FALSE;
/*	int blank, expose;    %%*/
	int *dummy1, *dummy2;
	int discard = FALSE;
	int pixels[512];
	caddr_t colors[512];
/*
 *  These next two lines are for mouse (pointer) control.
 */
	int do_acc, do_thresh;     /* Boolean */
	int acc_num, acc_denom, thresh;
	int status = FALSE;
	int numpixels = 0;
	int newpixels = FALSE;
	XColor def;  /* was Color, but only XColor, Colormap exist %%*/
	value_mask = 0;          /*  initialize mask for LED changes */
	do_acc = do_thresh = FALSE;

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

			acc_num = -1;
			acc_denom = -1;
			thresh = -1;
			if (i >= argc){
			        do_acc = do_thresh = TRUE;
				break;
			}
		        arg = argv[i];
			if (strcmp(arg, "default") == 0) {
				do_acc = do_thresh = TRUE;
				i++;
			} 
			else if (*arg >= '0' && *arg <= '9') {
				acc_num = atoi(arg);
				do_acc = TRUE;
				i++;
				if (i >= argc)
					break;
				arg = argv[i];
				if (*arg >= '0' && *arg <= '9') {
				        thresh = atoi(arg);
				        do_thresh = TRUE;
					i++;
				}
			}
		} 
		else if (strcmp(arg, "s") == 0 || strcmp(arg, "saver") == 0) {
			timeout = -1;
			interval = -1;
			prefer_blank = DefaultBlanking;
			dosaver = TRUE;
			if (i >= argc)
				break;
			arg = argv[i];
			if (strcmp(arg, "blank") == 0) {
			        prefer_blank = PreferBlanking;
			        i++;
			}
			if (strcmp(arg, "noblank") == 0) {
			        prefer_blank = DontPreferBlanking;
			        i++;
			}
			if (strcmp(arg, "off") == 0) {
			        timeout = 0;
			        i++;
				if (i >= argc)
				        break;
				arg = argv[i];
				if (strcmp(arg, "off") == 0) {
				        interval = 0;
					i++;
				}
			}
			if (strcmp(arg, "default") == 0) {
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
		else if(strcmp(arg, "-r") == 0 || strcmp(arg, "-repeat") == 0){
			repeat = FALSE;
		} 
		else if (strcmp(arg, "r") == 0 || strcmp(arg, "repeat") == 0) {
			repeat = TRUE;
			if (i > argc)
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

		else if (strcmp(arg, "p") == 0 || strcmp(arg, "pixel") == 0) {
		        newpixels = TRUE;
			if (i + 1 >= argc)
				usage(argv[0]);
		       		arg = argv[i];
			if (*arg >= '0' && *arg <= '9')
				pixels[numpixels] = atoi(arg);
			else
				usage(argv[0]);
			i++;
			colors[numpixels] = argv[i];
			i++;
			numpixels++;
		} 


		else if (index(arg, ':')) {
			disp = arg;
		} 
		else if (*arg == 'q') {
		        status = TRUE;
		}
		else
			usage(argv[0]);
	}
	if (!value_mask && !status && !dosaver && !do_acc && !newpixels 
	    && (repeat < 0))
		usage(argv[0]);

	dpy = XOpenDisplay(disp);
	
	if (dpy == NULL) {
		fprintf(stderr, "%s: Can't open display '%s'\n",
		argv[0], XDisplayName(disp ? disp : "\0"));
		exit(1);
	}

	if (value_mask) {
	        XChangeKeyboardControl(dpy, value_mask, &values);
	}

	if (status){
	XGetKeyboardControl(dpy, &values);
	XGetPointerControl(dpy, &acc_num, &acc_denom, &thresh);
	XGetScreenSaver(dpy, &timeout, &interval, &prefer_blank, &allow_exp);

	printf ("Keyboard Control Values:\n");
	printf ("Auto Repeat: %d \t\t", values.auto_repeat_mode);
/* 	printf ("Key: %d \n\n", values.key);     %%*/
	printf ("Key Click Volume (%%): %d \n", values.key_click_percent);
	printf ("Bell Volume (%%): %d \t", values.bell_percent);
	printf ("Bell Pitch (Hz): %d \t", values.bell_pitch);
	printf ("Bell Duration (msec): %d \n", values.bell_duration);
/*	printf ("LED: %d \t\t\t", values.led);
	printf ("LED Mode: %o \t\t", values.led_mode);         %%*/


	printf ("Pointer (Mouse) Control Values:\n");
	printf ("Acceleration: %d \t", acc_num / acc_denom);
	printf ("Threshold: %d \n\n", thresh);

	printf ("Screen Saver: (yes = %d, no = %d, default = %d)\n",
		PreferBlanking, DontPreferBlanking, DefaultBlanking);
	printf ("Prefer Blanking: %d \t", prefer_blank);
	printf ("Time-out: %d \t Cycle: %d\n", timeout, interval);

        }

	if (do_acc || do_thresh){
	  XChangePointerControl(dpy, do_acc, do_thresh, acc_num, 
				acc_denom, thresh);
	}

	if (repeat == TRUE) {
	        XAutoRepeatOn(dpy);
		printf ("Auto Repeat On\n");  /* %%*/
		XFlush(dpy);
	}
	else if (repeat == FALSE)
		XAutoRepeatOff(dpy);

	if (dosaver) XSetScreenSaver(dpy, timeout, interval, 
				     prefer_blank, allow_exp);

/*	screen = DefaultScreen(dpy);  %%*/
	if (newpixels && DisplayCells(dpy, screen = DefaultScreen(dpy)) >= 2) {
		while (--numpixels >= 0) {
			def.pixel = pixels[numpixels];
			if (XParseColor(colors[numpixels], &def))
				XStoreColor(&def);
			else
				fprintf(stderr, "%s: No such color\n", colors[numpixels]);
		}
	}

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
	printf("\t-led [1-32]         led off\n");
	printf("\t led [1-32]         led on\n");
	printf("    To set mouse acceleration and threshold:\n");
	printf("\t m [acc [thr]]    m default\n");
	printf("    To set pixel colors:\n");
	printf("\t p pixel_value color_name\n");
	printf("    To turn auto-repeat off or on:\n");
	printf("\t-r     r off        r    r on\n");
	printf("    For screen-saver control:\n");
	printf("\t s [timeout [cycle]]  s default\n");
	printf("\t s blank              s noblank\n");
	printf("    For status information:  q   or  query\n");
	exit(0);
}
