/* 
 * $Header: xset.c,v 1.17 87/07/11 05:06:34 dkk Locked $ 
 * $Locker: dkk $ 
 */
#include <X11/copyright.h>

/* Copyright    Massachusetts Institute of Technology    1985	*/

#ifndef lint
static char *rcsid_xset_c = "$Header: xset.c,v 1.17 87/07/11 05:06:34 dkk Locked $";
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

#define DONT_CHANGE -2

#define TIMEOUT 1
#define INTERVAL 2
#define PREFER_BLANK 3
#define ALLOW_EXP 4

#define	nextarg(i, argv) \
	argv[i]; \
	if (i >= argc) \
		break; \

main(argc, argv)
int argc;
char **argv;
{
	Display	*dpy;    /*  Pointer to the display structure.   */
	XColor def;
	Display *process_input();  /*  Command parsing function. */

	set = (struct Setinfo *) malloc(sizeof(struct Setinfo));
	set->values = (XKeyboardControl *) malloc(sizeof(XKeyboardControl));

	dpy = process_input(argc, argv, set);  /*  Sets set structure. */
/*%%%%*/	if (!set->value_mask && !set->do_what) {
/*  That is, if nothing has been flagged for change by XKeyboardControl()
 *  (by value_mask) or for change by another X function (by do_what).
 */
		usage(argv[0]);
	}
	XSynchronize(dpy, ON);

	if (set->value_mask) {
	        XChangeKeyboardControl(dpy, set->value_mask, set->values);
	}
	if (set->do_what & DO_QUERY) {
	        query(dpy);   /* Returns info to user.  */
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
	else if (set->do_what & REPEAT_OFF) {
		XAutoRepeatOff(dpy);
	}
	if (set->do_what & DO_SAVER) {
	  set->allow_exp = DefaultExposures;
	  XSetScreenSaver(dpy, set->timeout, set->interval, 
				     set->prefer_blank, set->allow_exp);
	}
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
	XFlush(dpy);
	exit(0);    /*  Done.  We can go home now.  */
}

isnumber(arg, maximum)
	char *arg;
	int maximum;
{
	register char *p;

	if (arg[0] == '-' && arg[1] == '1' && arg[2] == '\0')
		return(1);
	for (p=arg; isdigit(*p); p++);
	if (*p || atoi(arg) > maximum)
		return(0); 
	return(1);
}

/*  This is the command-line argument processing function.  The Setinfo
 *  structure is used to return data from the command line to the rest
 *  of the program.
 */
Display *process_input(argc, argv)  /* Returns what XOpenDisplay does*/
int argc;       /*  These two are passed from main. */
char **argv;     
struct Setinfo *cmd;  /*  The structure is filled with info for main.  */
{
register char *arg;
register int i;
char *disp = '\0';
Display *dpy;
if (argc == 1)  usage(argv[0]); /* To be replaced by window-interface */
for (i = 1; i < argc; ) {
  arg = argv[i++];
  if (index(arg, ':')) {     /*  Set display name if given by user.  */
    disp = arg;
  } 
}
dpy = XOpenDisplay(disp);  /*  Open display and check for success */
if (dpy == NULL) {
  fprintf(stderr, "%s: Can't open display '%s'\n",
	  argv[0], XDisplayName(disp ? disp : "\0"));
  exit(1);
}
for (i = 1; i < argc; ) {
  arg = argv[i++];

  else if (*arg == '-' && *(arg + 1) == 'c') { /* Does arg start with "-c"? */
    values->key_click_percent = 0;    /* If so, turn click off and  */
/* call:  set_click %%%%*/
  } 
  else if (*arg == 'c') {         /* Well, does it start with "c", then? */
    values->key_click_percent = -1;   /* Default click volume.      */
    arg = nextarg(i, argv);
    if (strcmp(arg, "on") == 0) {               /* Let click be default. */
      i++;
    } 
    else if (strcmp(arg, "off") == 0) {  
      values->key_click_percent = 0;       /* Turn it off.          */
      i++;
    } 
    else if (isnumber(arg, 100)) {
      values->key_click_percent = atoi(arg);  /* Set to spec. volume */
      i++;
    }
/* call:  set_click %%%%*/
  } 
  else if (*arg == '-' && *(arg + 1) == 'b') {  /* Does arg start w/ "-b" */
    values->bell_percent = 0;              /* Then turn off bell.    */
/* call:  set_bell %%%%*/
  } 
  else if (*arg == 'b') {                       /* Does it start w/ "b".  */
    values->bell_percent = -1;             /* Set bell to default.   */
    arg = nextarg(i, argv);
    if (strcmp(arg, "on") == 0) {               /* Let it stay that way.  */
      i++;
    } 
    else if (strcmp(arg, "off") == 0) {
      values->bell_percent = 0;            /* Turn the bell off.     */
      i++;
    } 

    else if (isnumber(arg, 100)) {              /* If volume is given:    */
      values->bell_percent = atoi(arg);    /* set bell appropriately.*/
      i++;
      arg = nextarg(i, argv);

      if (isnumber(arg, 20000)) {               /* If pitch is given:     */
	values->bell_pitch = atoi(arg);    /* set the bell.           */
	i++;

	arg = nextarg(i, argv);
	if (isnumber(arg, 1000)) {              /* If duration is given:  */
	  values->bell_duration = atoi(arg);  /*  set the bell.      */
	  i++;
	}
      }
    }
/* call:  set_bell %%%%*/
  } 
  else if (strcmp(arg, "-led") == 0) {         /* Turn off one or all LEDs */
    values->led_mode = OFF;
    arg = nextarg(i, argv);
    if (isnumber(arg, 32) && atoi(arg) > 0) {
      values->led = atoi(arg);
      i++;
    }
/* call:  set_led %%%%*/
  } 
  else if (strcmp(arg, "led") == 0) {         /* Turn on one or all LEDs  */
    values->led_mode = ON;
    arg = nextarg(i, argv);
    if (strcmp(arg, "on") == 0) {
      i++;
    } 
    else if (strcmp(arg, "off") == 0) {       /*  ...except in this case. */
      values->led_mode = OFF;
      i++;
    }
    else if (isnumber(arg, 32) && atoi(arg) > 0) {
      values->led = atoi(arg);
      i++;
    }
/* call:  set_led %%%%*/
  }
/*  Set pointer (mouse) settings:  Acceleration and Threshold. */
  else if (strcmp(arg, "m") == 0 || strcmp(arg, "mouse") == 0) {
    acc_num = -1;
    acc_denom = -1;     /*  Defaults */
    threshold = -1;
    if (i >= argc){
      set_mouse(dpy, acc_num, acc_denom, threshold);
      break;
    }
    arg = argv[i];
    if (strcmp(arg, "default") == 0) {
      i++;
    } 
    else if (*arg >= '0' && *arg <= '9') {
      acc_num = atoi(arg);  /* Set acceleration to user's tastes.  */
      i++;
      if (i >= argc) {
	set_mouse(dpy, acc_num, acc_denom, threshold);
	break;
      }
      arg = argv[i];
      if (*arg >= '0' && *arg <= '9') {
	thresh = atoi(arg);  /* Set threshold as user specified.  */
	i++;
      }
      set_mouse(dpy, acc_num, acc_denom, threshold);
    }
/* call:  set_mouse %%%%*/
  } 
  else if (*arg == 's') {   /*  If arg starts with "s".  */
    timeout = -1;      /*  Set defaults.            */
    interval = -1;
    prefer_blank = DefaultBlanking;
    allow_exp = DefaultExposures;
    if (i >= argc)
      break;
    arg = argv[i];
    if (strcmp(arg, "blank") == 0) {       /* Alter blanking preference. */
      prefer_blank = PreferBlanking;
      i++;
    }
    if (strcmp(arg, "noblank") == 0) {     /*  Ditto.  */
      prefer_blank = DontPreferBlanking;
      i++;
    }
    if (strcmp(arg, "off") == 0) {
      timeout = 0;                    /*  Turn off screen saver.  */
      i++;
      if (i >= argc)
	break;
      arg = argv[i];
      if (strcmp(arg, "off") == 0) {
	interval = 0;
	i++;
      }
    }
    if (strcmp(arg, "default") == 0) {    /*  Leave as default.       */
      i++;
    } 
    else if (*arg >= '0' && *arg <= '9') {  /*  Set as user wishes.   */
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
/* call:  set_saver %%%%*/
  } 
  else if(*arg == '-' && *(arg + 1) == 'r'){ /* If arg starts w/ "-r" */
/* call:  set_repeat %%%%*/
  } 
  else if (*arg == 'r') {            /*  If it starts with "r"        */
    if (i > argc)
      break;
    arg = argv[i];                   /*  Check next argument.         */
    if (strcmp(arg, "on") == 0) {
      i++;
    } 
    else if (strcmp(arg, "off") == 0) {
      i++;
    }
/* call:  set_repeat %%%%*/
  } 
  else if (*arg == 'p') {           /*  If arg starts with "p"       */
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
/* call:  set_pixels %%%%*/
  } 
  else if (*arg == 'q') {         /*  Give status to user.             */
/* call: do_query %%%%*/
  }
  else
    usage(argv[0]);
}

return(dpy);

}

/*  These next few functions do the real work (xsetting things).
 */
set_click(dpy, percent)
Display *dpy;
int percent;
{
XKeyboardControl values;
values.key_click_percent = percent;
XChangeKeyboardControl(dpy, KBKeyClickPercent, &values);
}

set_bell_vol(dpy, percent )
Display *dpy;
int percent;
{
XKeyboardControl values;
values.bell_percent = percent;
XChangeKeyboardControl(dpy, KBBellPercent, &values);
}

set_bell_pitch(dpy, pitch)
Display *dpy;
int pitch;
{
XKeyboardControl values;
values.bell_pitch = pitch;
XChangeKeyboardControl(dpy, KBBellPitch, &values);
return;
}

set_bell_dur(dpy, duration)
Display *dpy;
int duration;
{
XKeyboardControl values;
values.bell_duration = duration;
XChangeKeyboardControl(dpy, KBBellDuration, &values);
return;
}

set_mouse(dpy, acc_num, acc_denom, threshold)
Display *dpy;
int acc_num, acc_denom, threshold;
{
int do_accel = True, do_threshold = True;
if (acc_num == DONT_CHANGE)
  do_accel = False;
if (threshold == DONT_CHANGE)
  do_threshold = False;
XChangePointerControl(dpy, do_accel, do_threshold, acc_num,
		      acc_denom, threshold);
return;
}

set_saver(dpy, mask, value)
Display *dpy;
int mask, value;
{
int timeout, interval, prefer_blank, allow_exp;
XGetScreenSaver(dpy, &timeout, &interval, &prefer_blank, 
		&allow_exp);
if (mask == TIMEOUT) timeout = value;
if (mask == INTERVAL) interval = value;
if (mask == PREFER_BLANK) prefer_blank = value;
if (mask == ALLOW_EXP) allow_exp = value;
XSetScreenSaver(dpy, timeout, interval, prefer_blank, 
		allow_exp);
return;
}

set_repeat(dpy, )

set_pixels(dpy, )

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
