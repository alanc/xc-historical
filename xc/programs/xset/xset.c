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

/*  These are bit identifiers for "set->do_what" to tell us what to do: */
#define DO_ACCEL 1     /*  Alter the pointer (mouse) acceleration.      */
#define DO_THRESH 2    /*  Alter the pointer threshold (of acceleration).*/
#define DO_SAVER 4     /*  Alter the screen saver settings. */
#define REPEAT_OFF 8   /*  Turn auto repeat off. */
#define REPEAT_ON 16   /*  Turn auto repeat on.  */
#define NEW_PIXELS 32  /*  Define new pixel color values.  */
#define DO_QUERY 64    /*  Answer a query.  (Give info about xsettings.) */

/*  The Setinfo structure is for communication between the argument parsing
 *  function, process_input(), and main.
 */
struct Setinfo {
  XKeyboardControl *values;  /*  An argument for XChangeKeyboardControl() */
  unsigned long value_mask;  /*  An argument for XChangeKeyboardControl() */
  int do_what;       /*  Tells us what to change.  See #defines, above.   */
  int acc_num;       /*  These two specify the mouse acceleration for use */
  int acc_denom;     /*    by XChangeKeyboardControl().                   */
  int thresh;        /*  Specifies threshold above which pointer speed is */
                     /*    increased by a factor of accel_num/accel_denom.*/
  int timeout;       /*  Specifies idle time until screen saver kicks in. */
  int interval;      /*  Time between redrawings of screen saver pattern. */
  int prefer_blank;  /*  Either PreferBlanking, DontPrefer.. or Default.. */
  int allow_exp;     /*  Not yet implemented.                             */
  int pixels[512];
  caddr_t colors[512];
  int numpixels;
} *set;

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
	if (!set->value_mask && !set->do_what) {
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
Display *process_input(argc, argv, cmd)  /* Returns what XOpenDisplay does*/
int argc;       /*  These two are passed from main. */
char **argv;     
struct Setinfo *cmd;  /*  The structure is filled with info for main.  */
{
register char *arg;
register int i;
char *disp = '\0';
Display *dpy;
cmd->do_what = 0;
if (argc == 1)  usage(argv[0]);
for (i = 1; i < argc; ) {
  arg = argv[i++];
  if (*arg == '-' && *(arg + 1) == 'c') {  /* Does arg start with "-c"?  */
    cmd->values->key_click_percent = 0;    /* If so, turn click off and  */
    cmd->value_mask |= KBKeyClickPercent;  /*    set proper bit in mask. */
  } 
  else if (*arg == 'c') {         /* Well, does it start with "c", then? */
    cmd->values->key_click_percent = -1;   /* Default click volume.      */
    cmd->value_mask |= KBKeyClickPercent;
    arg = nextarg(i, argv);
    if (strcmp(arg, "on") == 0) {               /* Let click be default. */
      i++;
    } 
    else if (strcmp(arg, "off") == 0) {  
      cmd->values->key_click_percent = 0;       /* Turn it off.          */
      i++;
    } 
    else if (isnumber(arg, 100)) {
      cmd->values->key_click_percent = atoi(arg);  /* Set to spec. volume */
      i++;
    }
  } 
  else if (*arg == '-' && *(arg + 1) == 'b') {  /* Does arg start w/ "-b" */
    cmd->values->bell_percent = 0;              /* Then turn off bell.    */
    cmd->value_mask |= KBBellPercent;
  } 
  else if (*arg == 'b') {                       /* Does it start w/ "b".  */
    cmd->values->bell_percent = -1;             /* Set bell to default.   */
    cmd->value_mask |= KBBellPercent;
    arg = nextarg(i, argv);
    if (strcmp(arg, "on") == 0) {               /* Let it stay that way.  */
      i++;
    } 
    else if (strcmp(arg, "off") == 0) {
      cmd->values->bell_percent = 0;            /* Turn the bell off.     */
      i++;
    } 

    else if (isnumber(arg, 100)) {              /* If volume is given:    */
      cmd->values->bell_percent = atoi(arg);    /* set bell appropriately.*/
      i++;
      arg = nextarg(i, argv);

      if (isnumber(arg, 20000)) {               /* If pitch is given:     */
	cmd->values->bell_pitch = atoi(arg);    /* set the bell.           */
	cmd->value_mask |= KBBellPitch;
	i++;

	arg = nextarg(i, argv);
	if (isnumber(arg, 1000)) {              /* If duration is given:  */
	  cmd->values->bell_duration = atoi(arg);  /*  set the bell.      */
	  cmd->value_mask |= KBBellDuration;
	  i++;
	}
      }
    }
  } 
  else if (strcmp(arg, "-led") == 0) {         /* Turn off one or all LEDs */
    cmd->values->led_mode = OFF;
    cmd->value_mask |= KBLedMode;
    arg = nextarg(i, argv);
    if (isnumber(arg, 32) && atoi(arg) > 0) {
      cmd->values->led = atoi(arg);
      cmd->value_mask |= KBLed;
      i++;
    }
  } 
  else if (strcmp(arg, "led") == 0) {         /* Turn on one or all LEDs  */
    cmd->values->led_mode = ON;
    cmd->value_mask |= KBLedMode;
    arg = nextarg(i, argv);
    if (strcmp(arg, "on") == 0) {
      i++;
    } 
    else if (strcmp(arg, "off") == 0) {       /*  ...except in this case. */
      cmd->values->led_mode = OFF;
      i++;
    }
    else if (isnumber(arg, 32) && atoi(arg) > 0) {
      cmd->values->led = atoi(arg);
      cmd->value_mask |= KBLed;
      i++;
    }
  }
/*  Set pointer (mouse) settings:  Acceleration and Threshold. */
  else if (strcmp(arg, "m") == 0 || strcmp(arg, "mouse") == 0) {
    cmd->acc_num = -1;
    cmd->acc_denom = -1;     /*  Defaults */
    cmd->thresh = -1;
    if (i >= argc){
      cmd->do_what |= DO_ACCEL;    /*  Let the defaults stand.  */
      cmd->do_what |= DO_THRESH;
      break;
    }
    arg = argv[i];
    if (strcmp(arg, "default") == 0) {
      cmd->do_what |= DO_ACCEL;    /* Let defaults stand here, too.  */
      cmd->do_what |= DO_THRESH;
      i++;
    } 
    else if (*arg >= '0' && *arg <= '9') {
      cmd->acc_num = atoi(arg);  /* Set acceleration to user's tastes.  */
      cmd->do_what |= DO_ACCEL;
      i++;
      if (i >= argc)
	break;
      arg = argv[i];
      if (*arg >= '0' && *arg <= '9') {
	cmd->thresh = atoi(arg);  /* Set threshold as user specified.  */
	cmd->do_what |= DO_THRESH;
	i++;
      }
    }
  } 
  else if (*arg == 's') {   /*  If arg starts with "s".  */
    cmd->timeout = -1;      /*  Set defaults.            */
    cmd->interval = -1;
    cmd->prefer_blank = DefaultBlanking;
    cmd->do_what |= DO_SAVER;
    if (i >= argc)
      break;
    arg = argv[i];
    if (strcmp(arg, "blank") == 0) {       /* Alter blanking preference. */
      cmd->prefer_blank = PreferBlanking;
      i++;
    }
    if (strcmp(arg, "noblank") == 0) {     /*  Ditto.  */
      cmd->prefer_blank = DontPreferBlanking;
      i++;
    }
    if (strcmp(arg, "off") == 0) {
      cmd->timeout = 0;                    /*  Turn off screen saver.  */
      i++;
      if (i >= argc)
	break;
      arg = argv[i];
      if (strcmp(arg, "off") == 0) {
	cmd->interval = 0;
	i++;
      }
    }
    if (strcmp(arg, "default") == 0) {    /*  Leave as default.       */
      i++;
    } 
    else if (*arg >= '0' && *arg <= '9') {  /*  Set as user wishes.   */
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
  else if(*arg == '-' && *(arg + 1) == 'r'){ /* If arg starts w/ "-r" */
    cmd->do_what |= REPEAT_OFF;
  } 
  else if (*arg == 'r') {            /*  If it starts with "r"        */
    cmd->do_what |= REPEAT_ON;
    if (i > argc)
      break;
    arg = argv[i];                   /*  Check next argument.         */
    if (strcmp(arg, "on") == 0) {
      i++;
    } 
    else if (strcmp(arg, "off") == 0) {
    cmd->do_what |= REPEAT_OFF;
      i++;
    }
  } 
  else if (*arg == 'p') {           /*  If arg starts with "p"       */
    cmd->do_what |= NEW_PIXELS;
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
  else if (index(arg, ':')) {     /*  Set display name given by user.  */
    disp = arg;
  } 
  else if (*arg == 'q') {         /*  Give status to user.             */
    cmd->do_what |= DO_QUERY;
  }
  else
    usage(argv[0]);
}
	dpy = XOpenDisplay(disp);  /*  Open display and check for success */
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
