/* $XConsortium: math.c,v 1.2 89/05/04 12:15:21 jim Exp $ 
 *
 *  math.c  -  mathematics functions for a hand calculator under X
 *
 *  Author:    John H. Bradley, University of Pennsylvania
 *                (bradley@cis.upenn.edu)
 *                     March, 1987
 *
 *  RPN mode added and port to X11 by Mark Rosenstein, MIT Project Athena
 *
 *  Modified to be a client of the Xt toolkit and the Athena widget set by
 *  Donna Converse, MIT X Consortium.  This is all that remains of the 
 *  original calculator, and it still needs to be rewritten.  The HP
 *  functionality should be separated from the TI functionality. 
 *  Beware the HP functions: there are still errors here.
 */

#include <stdio.h>
#include <X11/Xos.h>
#include <math.h>
#include <signal.h>
#include <setjmp.h>
#include "xcalc.h"

#define PI          3.14159265358979
#define E           2.71828182845904
#define MAXDISP     11
#define DEG 0		/* DRG mode.  used for trig calculations */
#define RAD 1
#define GRAD 2
#define min(a,b) ((a) < (b) ? (a) : (b))
#define max(a,b) ((a) > (b) ? (a) : (b))
#define True	1
#define False   0

extern int	errno;
extern int	rpn;
extern char 	dispstr[];
extern void draw();
extern void ringbell();
extern void setflag();
extern void Quit();

#ifndef IEEE
    jmp_buf env;
#endif

 
/* This section is all of the state machine that implements the calculator
 * functions.  Much of it is shared between the infix and rpn modes.
 */

int 	flagINV, flagPAREN, flagM, drgmode;	/* display flags */

static double drg2rad=PI/180.0;  /* Conversion factors for trig funcs */
static double rad2drg=180.0/PI;
static int entered=1;  /* true if display contains a valid number.
                          if==2, then use 'dnum', rather than the string
                          stored in the display.  (for accuracy) 
                          if==3, then error occurred, only CLR & AC work */
/* entered seems to be overloaded - dmc */
static int lift_enabled = 0;	/* for rpn mode only */

static int CLR    =0;  /* CLR clears display.  if 1, clears acc, also */
static int OFF    =0;  /* once clears mem, twice quits */
static int Dpoint=0;  /* to prevent using decimal pt twice in a # */
static int clrdisp=1;  /* if true clears display before entering # */
static int accset =0;
static int lastop =kCLR;
static int memop  =kCLR;
static int exponent=0;
static double acc =0.0;
static double dnum=0.0;
static double mem[10] = { 0.0 };

/*
 * The following is to deal with the unfortunate assumption that if errno
 * is non-zero then an error has occurred.  On some systems (e.g. Ultrix), 
 * sscanf will call lower level routines that will set errno.
 */

void parse_double (src, fmt, dp)
    char *src;
    char *fmt;
    double *dp;
{
    int olderrno = errno;

    (void) sscanf (src, fmt, dp);
    errno = olderrno;
    return;
}

/*********************************/
void pre_op(keynum)
     int keynum;
{
    int i;
    int code;

    if (keynum==-1) return;
 
    errno = 0;			/* for non-IEEE machines */

    if ( (entered==3) && !(keynum==kCLR || keynum==kOFF)) {
      if (rpn) {
	clrdisp++;
      } else {
        ringbell();
        return;
      }
    }

    code = keynum;
    if (code != kCLR) CLR=0;
    if (code != kOFF) OFF=0;
    

#ifndef IEEE
    i=setjmp(env);
    if (i) {
        switch (i) {
#ifdef FPE_FLTDIV_TRAP
           case FPE_FLTDIV_TRAP:  strcpy(dispstr,"div by zero"); break;
#endif
#ifdef FPE_FLTDIV_FAULT
           case FPE_FLTDIV_FAULT: strcpy(dispstr,"div by zero"); break;
#endif
#ifdef FPE_FLTOVF_TRAP
           case FPE_FLTOVF_TRAP:  strcpy(dispstr,"overflow"); break;
#endif
#ifdef FPE_FLTOVF_FAULT
           case FPE_FLTOVF_FAULT: strcpy(dispstr,"overflow"); break;
#endif
#ifdef FPE_FLTUND_TRAP
           case FPE_FLTUND_TRAP:  strcpy(dispstr,"underflow"); break;
#endif
#ifdef FPE_FLTUND_FAULT
           case FPE_FLTUND_FAULT: strcpy(dispstr,"underflow"); break;
#endif
           default:               strcpy(dispstr,"error");
           }
        entered=3;
        DrawDisplay();
        return;
        }
#endif
}

void post_op()
{
#ifdef DEBUG
    showstack("\0");
#endif
#ifndef IEEE
    if (errno) {
        strcpy(dispstr,"error");
        DrawDisplay();
        entered=3;
        errno=0;
        }
#endif
}
/*-------------------------------------------------------------------------*/
DrawDisplay()
{
    if (strlen(dispstr)>12) {       /* strip out some decimal digits */
        char tmp[32];
        char *exp = index(dispstr,'e');  /* search for exponent part */
        if (!exp) dispstr[12]='\0';      /* no exp, just trunc. */
        else {
            if (strlen(exp)<=4) 
                sprintf(tmp,"%.8s",dispstr); /* leftmost 8 chars */
            else
                sprintf(tmp,"%.7s",dispstr); /* leftmost 7 chars */
            strcat (tmp,exp);            /* plus exponent */
            strcpy (dispstr,tmp);
            }
        }
    draw(dispstr);
    setflag(XCalc_MEMORY, (flagM));
    setflag(XCalc_INVERSE, (flagINV));
    setflag(XCalc_DEGREE, (drgmode==DEG));
    setflag(XCalc_RADIAN, (drgmode==RAD));
    setflag(XCalc_GRADAM, (drgmode==GRAD));
    setflag(XCalc_PAREN, (flagPAREN));
}

/*-------------------------------------------------------------------------*/
numeric(keynum)
     int keynum;
{
    char	st[2];
    int		cell;

  flagINV=0;

  if (rpn && (memop == kSTO || memop == kRCL || memop == kSUM)) {
      switch (keynum) {
	case kONE:	cell = 1; break;
	case kTWO:	cell = 2; break;
	case kTHREE:	cell = 3; break;
	case kFOUR:	cell = 4; break;
	case kFIVE:	cell = 5; break;
	case kSIX:	cell = 6; break;
	case kSEVEN:	cell = 7; break;
	case kEIGHT:	cell = 8; break;
	case kNINE:	cell = 9; break;
	case kZERO:	cell = 0; break;
      }
      switch (memop) {
      case kSTO:
	mem[cell] = dnum;
	lift_enabled = 1;
      entered = 2;
      clrdisp++;
	break;
      case kRCL:
	PushNum(dnum);
	dnum = mem[cell];
	sprintf(dispstr, "%.8g", dnum);
	lift_enabled = 1;
        entered = 1;
	clrdisp = 0;
	break;
      case kSUM:
	mem[cell] += dnum;
	lift_enabled = 1;
      entered = 2;
      clrdisp++;
	break;
      }
      memop = kCLR;
      DrawDisplay();
      return;
  }

  if (clrdisp) {
    dispstr[0]='\0';
    exponent=Dpoint=0;
/*    if (rpn && entered==2)
      PushNum(dnum);
 */
    if (rpn & lift_enabled)
	PushNum(dnum);
  }
  if (strlen(dispstr)>=MAXDISP)
    return;
    
    switch (keynum){
      case kONE:	st[0] = '1'; break;
      case kTWO:	st[0] = '2'; break;
      case kTHREE:	st[0] = '3'; break;
      case kFOUR:	st[0] = '4'; break;
      case kFIVE:	st[0] = '5'; break;
      case kSIX:	st[0] = '6'; break;
      case kSEVEN:	st[0] = '7'; break;
      case kEIGHT:	st[0] = '8'; break;
      case kNINE:	st[0] = '9'; break;
      case kZERO:	st[0] = '0'; break;
    }
    st[1] = '\0';
    strcat(dispstr,st);

  DrawDisplay();
  if (clrdisp && keynum != kZERO)
    clrdisp=0; /*no leading 0s*/
  entered=1;
  lift_enabled = 0;
}

bkspf()
{

  lift_enabled = 0;

  if (! flagINV)
  {
      if (entered!=1 || clrdisp)
	  return;
      if (strlen(dispstr) > 0)
	  dispstr[strlen(dispstr)-1] = 0;
      if (strlen(dispstr) == 0) {
	  strcat(dispstr, "0");
	  clrdisp++;
      }
  }
  else
  {
      strcpy(dispstr, "0");
      dnum = 0.0;
      clrdisp++;
      flagINV = 0;
  }
  DrawDisplay();
}

decf()
{
  flagINV=0;
  if (clrdisp) {
      if (rpn)
	PushNum(dnum);
      strcpy(dispstr,"0");
  }
  if (!Dpoint) {
    strcat(dispstr,".");
    DrawDisplay();
    Dpoint++;
  }
  clrdisp=0;
  entered=1;
}

eef()
{
  flagINV=0;
  if (clrdisp) {
      if (rpn && lift_enabled)
	PushNum(dnum);
      strcpy(dispstr, rpn ? "1" : "0");
  }
  if (!exponent) {
    strcat(dispstr,"E+");
    DrawDisplay();
    exponent=strlen(dispstr)-1;  /* where the '-' goes */
  }
  clrdisp=0;
  entered=1;
}

clearf()
{
  flagINV=0;
  if (CLR && !rpn) { /* clear all */
    ClearStacks();
    flagPAREN=0;
  }
  CLR++;
  exponent=Dpoint=0;
  clrdisp=1;
  entered=1;
  strcpy(dispstr,"0");
  DrawDisplay();
}

negf()
{
  flagINV=0;
  if (exponent) {       /* neg the exponent */
    if (dispstr[exponent]=='-')
      dispstr[exponent]='+';
    else
      dispstr[exponent]='-';
    DrawDisplay();
    return;
  }

  if (strcmp("0",dispstr)==0)
    return;			/* don't neg a zero */
  if (dispstr[0]=='-')	 	/* already neg-ed */
    strcpy(dispstr,dispstr+1);  /* move str left once */
  else {			/* not neg-ed.  add a '-' */
    char tmp[32];
    sprintf(tmp,"-%s",dispstr);
    strcpy(dispstr,tmp);
  }
  if (entered==2)
    dnum = -1.0 * dnum;
  DrawDisplay();
}

/* Two operand functions for infix calc */
twoop(keynum)
{
  double PopNum();

  if (flagINV) {
    flagINV=0;
    DrawDisplay();
  }

  if (!entered) {		/* something like "5+*" */
    if (!isopempty())
      (void) PopOp();			/* replace the prev op */
    PushOp(keynum);		/* with the new one */
    return;
  }
  
  if (entered==1)
    parse_double(dispstr,"%lf",&dnum);

  clrdisp=CLR=1;
  entered=Dpoint=exponent=0;

  if (!isopempty()) {  /* there was a previous op */
    lastop=PopOp();   /* get it */

    if (lastop==kLPAR) {  /* put it back */
      PushOp(kLPAR);
      PushOp(keynum);
      PushNum(dnum);
      return;
    }

    /* now, if the current op (keynum) is of
       higher priority than the lastop, the current
       op and number are just pushed on top 
       Priorities:  (Y^X) > *,/ > +,- */
    
    if (priority(keynum) > priority(lastop)) {
      PushNum(dnum);
      PushOp(lastop);
      PushOp(keynum);
    } else {  /* execute lastop on lastnum and dnum, push
	       result and current op on stack */
      acc=PopNum();
      switch (lastop) { /* perform the operation */
      case kADD: acc += dnum;  break;
      case kSUB: acc -= dnum;  break;
      case kMUL: acc *= dnum;  break;
      case kDIV: acc /= dnum;  break;
      case kPOW: acc =  pow(acc,dnum);  break;
	}
      PushNum(acc);
      PushOp(keynum);
      sprintf(dispstr,"%.8g",acc);
      DrawDisplay();
      dnum=acc;
    }
  }
  else { /* op stack is empty, push op and num */
    PushOp(keynum);
    PushNum(dnum);
  } 
}                      

/* Two operand functions for rpn calc */
twof(keynum)
{
  double PopNum();

  if (flagINV) {
    flagINV=0;
    DrawDisplay();
  }
  if (!entered)
    return;
  if (entered==1)
    parse_double(dispstr, "%lf", &dnum);
  acc = PopNum();
  switch(keynum) {
  case kADD: acc += dnum;  break;
  case kSUB: acc -= dnum;  break;
  case kMUL: acc *= dnum;  break;
  case kDIV: acc /= dnum;  break;
  case kPOW: acc =  pow(acc,dnum);  break;
  case kXXY: PushNum(dnum);
  }
  sprintf(dispstr, "%.8g", acc);
  DrawDisplay();
  clrdisp++;
  Dpoint = exponent = 0;
  entered = 2;
  lift_enabled = 1;
  dnum = acc;
}


entrf()
{
  flagINV=0;
  if (!entered)
    return;

  clrdisp=CLR=1;
  Dpoint=exponent=0;

  if (entered==1)
    parse_double(dispstr,"%lf",&dnum);
  entered=2;
  memop = kENTR;
  PushNum(dnum);
  lift_enabled = 0;
}

equf()
{
  double PopNum();

  flagINV=0;
  if (!entered)
    return;

  clrdisp=CLR=1;
  Dpoint=exponent=0;

  if (entered==1)
    parse_double(dispstr,"%lf",&dnum);
  entered=2;

  PushNum(dnum);

  while (!isopempty()) {  /* do all pending ops */
    dnum=PopNum();
    acc=PopNum();
    lastop=PopOp();
    switch (lastop) {
    case kADD:  acc += dnum;
		break;
    case kSUB:  acc -= dnum;
		break;
    case kMUL:  acc *= dnum;
		break;
    case kDIV:  acc /= dnum;
		break;
    case kPOW:  acc = pow(acc,dnum);
		break;
    case kLPAR:	flagPAREN--;
		PushNum(acc);
		break;
    }
    dnum=acc;
    PushNum(dnum);
  }

  sprintf(dispstr,"%.8g",dnum);
  DrawDisplay();
}
        
lparf()
{
  flagINV=0;
  PushOp(kLPAR);
  flagPAREN++;
  DrawDisplay();
}

rollf()
{
  double PopNum();

  if (!entered)
    return;
  if (entered==1)
    parse_double(dispstr, "%lf", &dnum);
  entered = 2;
  lift_enabled = 1;
  RollNum(flagINV);
  flagINV=0;
  clrdisp++;
  sprintf(dispstr, "%.8g", dnum);
  DrawDisplay();
}

rparf()
{
  double PopNum();

  flagINV=0;
  if (!entered)
    return;

  if (!flagPAREN)
    return;
  
  clrdisp++;
  Dpoint=exponent=0;

  if (entered==1)
    parse_double(dispstr,"%lf",&dnum);
  entered=2;

  PushNum(dnum);
  while (!isopempty() && (lastop=PopOp())!=kLPAR) {
    /* do all pending ops, back to left paren */
    dnum=PopNum();
    acc=PopNum();
    switch (lastop) {
    case kADD:  acc += dnum;
		break;
    case kSUB:  acc -= dnum;
		break;
    case kMUL:  acc *= dnum;
		break;
    case kDIV:  acc /= dnum;
		break;
    case kPOW:  acc = pow(acc,dnum);
		break;
    }
    dnum=acc;
    PushNum(dnum);
  }
  (void) PopNum();
  flagPAREN--;
  entered=2;
  sprintf(dispstr,"%.8g",dnum);
  DrawDisplay();
}

drgf()
{
  if (flagINV) {
    if (entered==1)
      parse_double(dispstr,"%lf",&dnum);
    switch (drgmode) {
    case DEG:  dnum=dnum*PI/180.0;    break;
    case RAD:  dnum=dnum*200.0/PI;    break;
    case GRAD: dnum=dnum*90.0/100.0;  break;
    }
    entered=2;
    clrdisp=1;
    flagINV=0;
    sprintf(dispstr,"%.8g",dnum);
  }
                         
  flagINV=0;
  drgmode = ++drgmode % 3;
  switch (drgmode) {
  case DEG:  drg2rad=PI / 180.0;
	     rad2drg=180.0 / PI;
	     break;
  case RAD:  drg2rad=1.0;
	     rad2drg=1.0;
	     break;
  case GRAD: drg2rad=PI / 200.0;
	     rad2drg=200.0 / PI;
	     break;
  }
  DrawDisplay();
}

invf()
{
  flagINV = ~flagINV;
  DrawDisplay();
}

memf(keynum)
{
    memop = keynum;
    if (entered==1)
      parse_double(dispstr,"%lf",&dnum);
    entered = 2;
    clrdisp++;
    lift_enabled = 0;
}

oneop(keynum)
{
  int i,j;
  double dtmp;

  if (entered==1)
    parse_double(dispstr,"%lf",&dnum);
  entered = 2;

  switch (keynum) {  /* do the actual math fn. */
  case kE:     if (rpn && memop != kENTR) PushNum(dnum); dnum=E;  break;
  case kPI:    if (rpn && memop != kENTR) PushNum(dnum); dnum=PI;  break;
  case kRECIP: dnum=1.0/dnum;  break;
  case kSQR:   flagINV = !flagINV; /* fall through to */
  case kSQRT:  if (flagINV) dnum=dnum*dnum;
	       else dnum=sqrt(dnum);
	       break;
  case k10X:   flagINV = !flagINV; /* fall through to */
  case kLOG:   if (flagINV) dnum=pow(10.0,dnum);
  	       else dnum=log10(dnum);
	       break;
  case kEXP:   flagINV = !flagINV; /* fall through to */
  case kLN:    if (flagINV) dnum=exp(dnum);
	       else dnum=log(dnum);
	       break;
  case kSIN:   if (flagINV) dnum=asin(dnum)*rad2drg;
	       else dnum=sin(dnum*drg2rad);
	       break;
  case kCOS:   if (flagINV) dnum=acos(dnum)*rad2drg;
	       else dnum=cos(dnum*drg2rad);
	       break;
  case kTAN:   if (flagINV) dnum=atan(dnum)*rad2drg;
	       else dnum=tan(dnum*drg2rad);
	       break;
  case kSTO:   mem[0]=dnum;  flagM=!(mem[0]==0.0);  break;
  case kRCL:   if (rpn && lift_enabled) PushNum(dnum);
               dnum=mem[0];  flagM=!(mem[0]==0.0);  break;
  case kSUM:   mem[0]+=dnum; flagM=!(mem[0]==0.0);  break;
  case kEXC:   dtmp=dnum; dnum=mem[0];  mem[0]=dtmp;
	       flagM=!(mem[0]==0.0);  break;
  case kFACT:  if (floor(dnum)!=dnum || dnum<0.0 || dnum>500.0) {
		 strcpy(dispstr,"error");
		 entered=3;
		 break;
	       }
	       i=(int) (floor(dnum));
	       for (j=1,dnum=1.0; j<=i; j++) 
		 dnum*=(float) j;
	       break;
  }
  
  if (entered==3) {  /* error */
    DrawDisplay();
    return;
  }

  entered=2;
  clrdisp=1;
  flagINV=0;
  lift_enabled = 1;
  sprintf(dispstr,"%.8g",dnum);
  DrawDisplay();
}

offf()
{
  /* full reset */
  ResetCalc();
  entered=clrdisp=1;
  lift_enabled = 0;
  dnum=mem[0]=0.0;
  accset=exponent=Dpoint=0;
  DrawDisplay();
}


#define STACKMAX 32
static int opstack[STACKMAX];
static int opsp;
static double numstack[STACKMAX];
static int numsp;


/*******/
PushOp(op)
   int op;
/*******/
{
  if (opsp==STACKMAX) {strcpy(dispstr,"stack error");  entered=3;}
  else opstack[opsp++]=op;
}

/*******/
int PopOp()
/*******/
{
  if (opsp==0) {
      strcpy(dispstr,"stack error");
      entered=3;
      return(kNOP);
  } else
    return(opstack[--opsp]);
}

/*******/
int isopempty()
/*******/
{
  return( opsp ? 0 : 1 );
}

#ifdef DEBUG
showstack(string)
    char	*string;
{
    fprintf(stderr, "%s: %lf %lf %lf\n", string, numstack[0], numstack[1],
	    numstack[2]);
}
#endif

/*******/
PushNum(num)
 double num;
/*******/
{
  if (rpn) {
      numstack[2] = numstack[1];
      numstack[1] = numstack[0];
      numstack[0] = num;
      return;
  }
  if (numsp==STACKMAX) {
      strcpy(dispstr,"stack error");
      entered=3;
  } else
    numstack[numsp++]=num;
}

/*******/
double PopNum()
/*******/
{
    if (rpn) {
	double tmp = numstack[0];
	numstack[0] = numstack[1];
	numstack[1] = numstack[2];
	return(tmp);
    }
    if (numsp==0) {
	strcpy(dispstr,"stack error");
	entered=3;
	return 0.0;
    } else
      return(numstack[--numsp]);
}

/*******/
RollNum(dir)
/*******/
{
    double tmp;

    if (dir) {				/* roll up */
	tmp         = dnum;
	dnum        = numstack[2];
	numstack[2] = numstack[1];
	numstack[1] = numstack[0];
	numstack[0] = tmp;
    } else {				/* roll down */
	tmp         = dnum;
	dnum        = numstack[0];
	numstack[0] = numstack[1];
	numstack[1] = numstack[2];
	numstack[2] = tmp;
    }
}


/*******/
ClearStacks()
/*******/
{
    if (rpn)
      numstack[0] = numstack[1] = numstack[2] = 0.;
    opsp=numsp=0;
}


/*******/
int priority(op)
         int op;
/*******/
{
    switch (op) {
        case kPOW: return(2);
        case kMUL:
        case kDIV: return(1);
        case kADD:
        case kSUB: return(0);
        }
    return 0;
}


/********/
ResetCalc()
/********/
{
    flagM=flagINV=flagPAREN=0;  drgmode=DEG;
    setflag(XCalc_MEMORY, False);
    setflag(XCalc_INVERSE, False);
    setflag(XCalc_PAREN, False);
    setflag(XCalc_RADIAN, False);
    setflag(XCalc_GRADAM, False);
    setflag(XCalc_DEGREE, True);
    strcpy(dispstr,"0");
    draw(dispstr);
    ClearStacks();
    drg2rad=PI/180.0;
    rad2drg=180.0/PI;
}


#ifndef IEEE
/******************/
/*ARGSUSED*/
void fperr(sig,code,scp)
  int sig,code;
  struct sigcontext *scp;
/******************/
{
    longjmp(env,code);
}
#endif
