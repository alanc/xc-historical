/* 
 * $Header: xset.c,v 1.13 87/06/29 13:56:25 dkk Exp $ 
 * $Locker:  $ 
 */
#include <X11/copyright.h>

/* Copyright    Massachusetts Institute of Technology    1985	*/

#ifndef lint
static char *rcsid_xset_c = "$Header: xset.c,v 1.13 87/06/29 13:56:25 dkk Exp $";
#endif

#include <X11/X.h>      /*  Should be transplanted to X11/Xlibwm.h     %*/
#include <X11/Xlib.h>
/*  #include <X11/Xlibwm.h>  [Doesn't exist yet  5-14-87]  %*/
#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <strings.h>
#include <ctype.h>

#define ON 1
#define OFF 0

#define	nextarg(i, argv) \
	argv[i]; \
	if (i >= argc) \
		break; \

#define DO_ACCEL 1
#define DO_THRESH 2
#define DO_SAVER 4
#define REPEAT_OFF 8
#define REPEAT_ON 16
#define NEW_PIXELS 32
#define DO_QUERY 64

struct Setinfo {
  XKeyboardControl *values;
  unsigned long value_mask;
  int do_what;
  int acc_num;
  int acc_denom;
  int thresh;
  int timeout;
  int interval;
  int prefer_blank;
  int allow_exp;
  int pixels[512];
  caddr_t colors[512];
  int numpixels;
} *set;

main(argc, argv)
int argc;
char **argv;
{
	Display	*dpy;

	XColor def;
	Display *process_input();
	set = (struct Setinfo *) malloc(sizeof(struct Setinfo));
	set->values = (XKeyboardControl *) malloc(sizeof(XKeyboardControl));

	dpy = process_input(argc, argv, set);

	if (!set->value_mask && !set->do_what) {
		printf("No changes made.\n");
		usage(argv[0]);
	}

/*	XSynchronize(dpy, 1); %%*/


	if (set->value_mask) {
	        XChangeKeyboardControl(dpy, set->value_mask, set->values);
	}

	if (set->do_what & DO_QUERY) {
	        query(dpy);
	}

	if (set->do_what & DO_ACCEL){
	  XChangePointerControl(dpy, True, 
				set->do_what & DO_THRESH / DO_THRESH,
				set->acc_num, set->acc_denom, set->thresh);
	}

	if (set->do_what & REPEAT_ON) {
	        XAutoRepeatOn(dpy);
		XFlush(dpy);
	}
	else if (set->do_what & REPEAT_OFF)
		XAutoRepeatOff(dpy);

	if (set->do_what & DO_SAVER)
	  XSetScreenSaver(dpy, set->timeout, set->interval, 
				     set->prefer_blank, set->allow_exp);

	if (set->do_what & NEW_PIXELS &&
	    DisplayCells(dpy, DefaultScreen(dpy)) >= 2) {
		while (--set->numpixels >= 0) {
			def.pixel = set->pixels[set->numpixels];
			if (XParseColor(set->colors[set->numpixels], &def))
				XStoreColor(&def);
			else
				fprintf(stderr, "%s: No such color\n", set->colors[set->numpixels]);
		}
	}
	XSync(dpy, 0);
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

/*  This is the command-line argument processing function.  The Setinfo
 *  structure is used to return data from the command line to the rest
 *  of the program.
 */
Display *process_input(argc, argv, cmd)
int argc;
char **argv;
struct Setinfo *cmd;
{
register char *arg;
register int i;
char *disp = '\0';
Display *dpy;
if (argc == 1)  usage(argv[0]);
for (i = 1; i < argc; ) {
  arg = argv[i++];
  if (strcmp(arg, "-c") == 0 || strcmp(arg, "-click") == 0) {
    cmd->values->key_click_percent = 0;
    cmd->value_mask |= KBKeyClickPercent;
  } 
  else if (strcmp(arg, "c") == 0 || strcmp(arg, "click") == 0) {
    cmd->values->key_click_percent = -1;
    cmd->value_mask |= KBKeyClickPercent;
    arg = nextarg(i, argv);
    if (strcmp(arg, "on") == 0) {
      i++;
    } 
    else if (strcmp(arg, "off") == 0) {
      cmd->values->key_click_percent = 0;
      i++;
    } 
    else if (isnumber(arg, 100)) {
      cmd->values->key_click_percent = atoi(arg);
      i++;
    }
  } 
  else if (strcmp(arg, "-b") == 0 || strcmp(arg, "-bell") == 0) {
    cmd->values->bell_percent = 0;
    cmd->value_mask |= KBBellPercent;
  } 
  else if (strcmp(arg, "b") == 0 || strcmp(arg, "bell") == 0) {
    cmd->values->bell_percent = -1;
    cmd->value_mask |= KBBellPercent;
    arg = nextarg(i, argv);
    if (strcmp(arg, "on") == 0) {
      i++;
    } 
    else if (strcmp(arg, "off") == 0) {
      cmd->values->bell_percent = 0;
      i++;
    } 
    /* volume */
    else if (isnumber(arg, 100)) {
      cmd->values->bell_percent = atoi(arg);
      i++;
      /* pitch */
      arg = nextarg(i, argv);
      if (isnumber(arg, 20000)) {
	cmd->values->bell_pitch = atoi(arg);
	cmd->value_mask |= KBBellPitch;
	i++;
	/* duration */
	arg = nextarg(i, argv);
	if (isnumber(arg, 1000)) {
	  cmd->values->bell_duration = atoi(arg);
	  cmd->value_mask |= KBBellDuration;
	  i++;
	}
      }
    }
  } 
  else if (strcmp(arg, "-led") == 0) {
    cmd->values->led_mode = OFF;
    cmd->value_mask |= KBLedMode;
    arg = nextarg(i, argv);
    if (isnumber(arg, 32) && atoi(arg) > 0) {
      cmd->values->led = atoi(arg);
      cmd->value_mask |= KBLed;
      i++;
    }
  } 
  else if (strcmp(arg, "led") == 0) {
    cmd->values->led_mode = ON;
    cmd->value_mask |= KBLedMode;
    arg = nextarg(i, argv);
    if (strcmp(arg, "on") == 0) {
      i++;
    } 
    else if (strcmp(arg, "off") == 0) {
      cmd->values->led_mode = OFF;
      i++;
    }
    else if (isnumber(arg, 32) && atoi(arg) > 0) {
      cmd->values->led = atoi(arg);
      cmd->value_mask |= KBLed;
      i++;
    }
  }
  else if (strcmp(arg, "m") == 0 || strcmp(arg, "mouse") == 0) {
    cmd->acc_num = -1;
    cmd->acc_denom = -1;
    cmd->thresh = -1;
    if (i >= argc){
      cmd->do_what & DO_ACCEL;
      cmd->do_what & DO_THRESH;
      break;
    }
    arg = argv[i];
    if (strcmp(arg, "default") == 0) {
      cmd->do_what & DO_ACCEL;
      cmd->do_what & DO_THRESH;
      i++;
    } 
    else if (*arg >= '0' && *arg <= '9') {
      cmd->acc_num = atoi(arg);
      cmd->do_what & DO_ACCEL;
      i++;
      if (i >= argc)
	break;
      arg = argv[i];
      if (*arg >= '0' && *arg <= '9') {
	cmd->thresh = atoi(arg);
	cmd->do_what & DO_THRESH;
	i++;
      }
    }
  } 
  else if (strcmp(arg, "s") == 0 || strcmp(arg, "saver") == 0) {
    cmd->timeout = -1;
    cmd->interval = -1;
    cmd->prefer_blank = DefaultBlanking;
    cmd->do_what & DO_SAVER;
    if (i >= argc)
      break;
    arg = argv[i];
    if (strcmp(arg, "blank") == 0) {
      cmd->prefer_blank = PreferBlanking;
      i++;
    }
    if (strcmp(arg, "noblank") == 0) {
      cmd->prefer_blank = DontPreferBlanking;
      i++;
    }
    if (strcmp(arg, "off") == 0) {
      cmd->timeout = 0;
      i++;
      if (i >= argc)
	break;
      arg = argv[i];
      if (strcmp(arg, "off") == 0) {
	cmd->interval = 0;
	i++;
      }
    }
    if (strcmp(arg, "default") == 0) {
      i++;
    } 
    else if (*arg >= '0' && *arg <= '9') {
      cmd->timeout = atoi(arg);
      i++;
      if (i >= argc)
	break;
      arg = argv[i];
      if (*arg >= '0' && *arg <= '9') {
	cmd->interval = atoi(arg);
	i++;
      }
    }
  } 
  else if(strcmp(arg, "-r") == 0 || strcmp(arg, "-repeat") == 0){
    cmd->do_what & REPEAT_OFF;
  } 
  else if (strcmp(arg, "r") == 0 || strcmp(arg, "repeat") == 0) {
    cmd->do_what & REPEAT_ON;
    if (i > argc)
      break;
    arg = argv[i];
    if (strcmp(arg, "on") == 0) {
      i++;
    } 
    else if (strcmp(arg, "off") == 0) {
    cmd->do_what & REPEAT_OFF;
      i++;
    }
  } 
  else if (strcmp(arg, "p") == 0 || strcmp(arg, "pixel") == 0) {
    cmd->do_what & NEW_PIXELS;
    if (i + 1 >= argc)
      usage(argv[0]);
    arg = argv[i];
    if (*arg >= '0' && *arg <= '9')
      cmd->pixels[cmd->numpixels] = atoi(arg);
    else
      usage(argv[0]);
    i++;
    cmd->colors[cmd->numpixels] = argv[i];
    i++;
    cmd->numpixels++;
  } 
  else if (index(arg, ':')) {
    disp = arg;
  } 
  else if (*arg == 'q') {
    cmd->do_what & DO_QUERY;
  }
  else
    usage(argv[0]);
}
	dpy = XOpenDisplay(disp);
	if (dpy == NULL) {
		fprintf(stderr, "%s: Can't open display '%s'\n",
		argv[0], XDisplayName(disp ? disp : "\0"));
		exit(1);
	}
return(dpy);

}

/*  This is the information-getting function for telling the user what the
 *  current "xsettings" are.
 */
query(dpy)
Display *dpy;
{
int acc_num, acc_denom, thresh;
int timeout, interval, prefer_blank, allow_exp;

XGetKeyboardControl(dpy, set->values);
XGetPointerControl(dpy, &acc_num, &acc_denom, &thresh);
XGetScreenSaver(dpy, &timeout, &interval, &prefer_blank, &allow_exp);

printf ("Keyboard Control Values:\n");
printf ("Auto Repeat: %d \t\t", set->values->auto_repeat_mode);
/*printf ("Key: %d \n\n", set->values->key);     %%*/
printf ("Key Click Volume (%%): %d \n", set->values->key_click_percent);
printf ("Bell Volume (%%): %d \t", set->values->bell_percent);
printf ("Bell Pitch (Hz): %d \t", set->values->bell_pitch);
printf ("Bell Duration (msec): %d \n", set->values->bell_duration);
/*printf ("LED: %d \t\t\t", set->values->led);
printf ("LED Mode: %o \t\t", set->values->led_mode);         %%*/

printf ("Pointer (Mouse) Control Values:\n");
printf ("Acceleration: %d \t", acc_num / acc_denom);
printf ("Threshold: %d \n\n", thresh);
printf ("Screen Saver: (yes = %d, no = %d, default = %d)\n",
	PreferBlanking, DontPreferBlanking, DefaultBlanking);
printf ("Prefer Blanking: %d \t", prefer_blank);
printf ("Time-out: %d \t Cycle: %d\n", timeout, interval);
}

/*  This is the usage function */

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
