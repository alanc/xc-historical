/*
 * $XConsortium: actions.c,v 1.2 89/05/08 16:22:27 converse Exp $
 *
 * actions.c - externally available procedures for xcalc
 * 
 * Copyright 1989 by the Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of M.I.T. not be used in advertising
 * or publicity pertaining to distribution of the software without specific, 
 * written prior permission. M.I.T. makes no representations about the 
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Donna Converse, MIT X Consortium
 */

#include <X11/Intrinsic.h>
#include "xcalc.h"
extern int rpn;
extern void pre_op(), post_op(), Quit(), ringbell(), do_select();

/*ARGSUSED*/
void add(w, e, vector, count)
    Widget	w;
    XEvent	*e;
    String	*vector;
    Cardinal	*count;
{
    pre_op(kADD);
    rpn ? twof(kADD) : twoop(kADD);
    post_op();
}

/*ARGSUSED*/
void back(w, e, vector, count)
    Widget	w;
    XEvent	*e;
    String	*vector;
    Cardinal	*count;
{
    pre_op(kBKSP);
    bkspf();
    post_op();
}

/*ARGSUSED*/
void bell(w, e, vector, count)
    Widget	w;
    XEvent	*e;
    String	*vector;
    Cardinal	*count;
{
    ringbell();
}

/*ARGSUSED*/
void clear(w, e, vector, count)
    Widget	w;
    XEvent	*e;
    String	*vector;
    Cardinal	*count;
{
    pre_op(kCLR);
    clearf();
    post_op();
}
   
/*ARGSUSED*/
void cosine(w, e, vector, count)
    Widget	w;
    XEvent	*e;
    String	*vector;
    Cardinal	*count;
{
    pre_op(kCOS);
    oneop(kCOS);
    post_op();
}

/*ARGSUSED*/
void decimal(w, e, vector, count)
    Widget	w;
    XEvent	*e;
    String	*vector;
    Cardinal	*count;
{
    pre_op(kDEC);
    decf();
    post_op();
}

/*ARGSUSED*/
void degree(w, e, vector, count)
    Widget	w;
    XEvent	*e;
    String	*vector;
    Cardinal	*count;
{
    pre_op(kDRG);
    drgf();
    post_op();
}

/*ARGSUSED*/
void digit(w, e, vector, count)
    Widget	w;
    XEvent	*e;
    String	*vector;
    Cardinal	*count;
{
    switch (vector[0][0])
    {
      case '1':	pre_op(kONE); numeric(kONE); break;
      case '2': pre_op(kTWO); numeric(kTWO); break;
      case '3': pre_op(kTHREE); numeric(kTHREE); break;
      case '4': pre_op(kFOUR); numeric(kFOUR); break;
      case '5': pre_op(kFIVE); numeric(kFIVE); break;
      case '6': pre_op(kSIX); numeric(kSIX); break;
      case '7': pre_op(kSEVEN); numeric(kSEVEN); break;
      case '8': pre_op(kEIGHT); numeric(kEIGHT); break;
      case '9': pre_op(kNINE); numeric(kNINE); break;
      case '0': pre_op(kZERO); numeric(kZERO); break;
    }
    post_op();
}

/*ARGSUSED*/
void divide(w, e, vector, count)
    Widget	w;
    XEvent	*e;
    String	*vector;
    Cardinal	*count;
{
    pre_op(kDIV);
    rpn  ? twof(kDIV) : twoop(kDIV);
    post_op();
}

/*ARGSUSED*/
void e(w, e, vector, count)
    Widget	w;
    XEvent	*e;
    String	*vector;
    Cardinal	*count;
{
    pre_op(kE);
    oneop(kE);
    post_op();
}

/*ARGSUSED*/
void enter(w, e, vector, count)
    Widget	w;
    XEvent	*e;
    String	*vector;
    Cardinal	*count;
{
    pre_op(kENTR);
    entrf();
    post_op();
}

/*ARGSUSED*/
void epower(w, e, vector, count)
    Widget	w;
    XEvent	*e;
    String	*vector;
    Cardinal	*count;
{
    pre_op(kEXP);
    oneop(kEXP);
    post_op();
}

/*ARGSUSED*/
void equal(w, e, vector, count)
    Widget	w;
    XEvent	*e;
    String	*vector;
    Cardinal	*count;
{
    pre_op(kEQU);
    equf();
    post_op();
}

/*ARGSUSED*/
void exchange(w, e, vector, count)
    Widget	w;
    XEvent	*e;
    String	*vector;
    Cardinal	*count;
{
    pre_op(kEXC);
    oneop(kEXC);
    post_op();
}
   
/*ARGSUSED*/
void factorial(w, e, vector, count)
    Widget	w;
    XEvent	*e;
    String	*vector;
    Cardinal	*count;
{
    pre_op(kFACT);
    oneop(kFACT);
    post_op();
}

/*ARGSUSED*/
void inverse(w, e, vector, count)
    Widget	w;
    XEvent	*e;
    String	*vector;
    Cardinal	*count;
{
    pre_op(kINV);
    invf();
    post_op();
}
   
/*ARGSUSED*/
void leftParen(w, e, vector, count)
    Widget	w;
    XEvent	*e;
    String	*vector;
    Cardinal	*count;
{
    pre_op(kLPAR);
    lparf();
    post_op();
}
   
/*ARGSUSED*/
void logarithm(w, e, vector, count)
    Widget	w;
    XEvent	*e;
    String	*vector;
    Cardinal	*count;
{
    pre_op(kLOG);
    oneop(kLOG);
    post_op();
}

/*ARGSUSED*/
void multiply(w, e, vector, count)
    Widget	w;
    XEvent	*e;
    String	*vector;
    Cardinal	*count;
{
    pre_op(kMUL);
    rpn ? twof(kMUL) : twoop(kMUL);
    post_op();
}
   
/*ARGSUSED*/
void naturalLog(w, e, vector, count)
    Widget	w;
    XEvent	*e;
    String	*vector;
    Cardinal	*count;
{
    pre_op(kLN);
    oneop(kLN);
    post_op();
}
   
/*ARGSUSED*/
void negate(w, e, vector, count)
    Widget	w;
    XEvent	*e;
    String	*vector;
    Cardinal	*count;
{
    pre_op(kNEG);
    negf();
    post_op();
}

/*ARGSUSED*/
void nop(w, e, vector, count)
    Widget	w;
    XEvent	*e;
    String	*vector;
    Cardinal	*count;
{
    ringbell();
}

/*ARGSUSED*/
void off(w, e, vector, count)
    Widget	w;
    XEvent	*e;
    String	*vector;
    Cardinal	*count;
{
    pre_op(kOFF);
    offf();
    post_op();
}
   
/*ARGSUSED*/
void pi(w, e, vector, count)
    Widget	w;
    XEvent	*e;
    String	*vector;
    Cardinal	*count;
{
    pre_op(kPI);
    oneop(kPI);
    post_op();
}
   
/*ARGSUSED*/
void power(w, e, vector, count)
    Widget	w;
    XEvent	*e;
    String	*vector;
    Cardinal	*count;
{
    pre_op(kPOW);
    rpn ? twof(kPOW) : twoop(kPOW);
    post_op();
}

/*ARGSUSED*/
void quit(w, e, vector, count)
    Widget	w;
    XEvent	*e;
    String	*vector;
    Cardinal	*count;
{
    Quit();
}

/*ARGSUSED*/
void recall(w, e, vector, count)
    Widget	w;
    XEvent	*e;
    String	*vector;
    Cardinal	*count;
{
    pre_op(kRCL);
    rpn ? memf(kRCL) : oneop(kRCL);
    post_op();
}
	
/*ARGSUSED*/
void reciprocal(w, e, vector, count)
    Widget	w;
    XEvent	*e;
    String	*vector;
    Cardinal	*count;
{
    pre_op(kRECIP);
    oneop(kRECIP);
    post_op();
}
   
/*ARGSUSED*/
void rightParen(w, e, vector, count)
    Widget	w;
    XEvent	*e;
    String	*vector;
    Cardinal	*count;
{
    pre_op(kRPAR);
    rparf();
    post_op();
}
   
/*ARGSUSED*/
void roll(w, e, vector, count)
    Widget	w;
    XEvent	*e;
    String	*vector;
    Cardinal	*count;
{
    pre_op(kROLL);
    rollf();
    post_op();
}

/*ARGSUSED*/
void scientific(w, e, vector, count)
    Widget	w;
    XEvent	*e;
    String	*vector;
    Cardinal	*count;
{
    pre_op(kEE);
    eef();
    post_op();
}

/*ARGSUSED*/
void selection(w, e, vector, count)
    Widget	w;
    XEvent	*e;
    String	*vector;
    Cardinal	*count;
{
    do_select(((XButtonReleasedEvent *)e)->time);
}

/*ARGSUSED*/
void sine(w, e, vector, count)
    Widget	w;
    XEvent	*e;
    String	*vector;
    Cardinal	*count;
{
    pre_op(kSIN);
    oneop(kSIN);
    post_op();
}

/*ARGSUSED*/
void square(w, e, vector, count)
    Widget	w;
    XEvent	*e;
    String	*vector;
    Cardinal	*count;
{
    pre_op(kSQR);
    oneop(kSQR);
    post_op();
}

/*ARGSUSED*/
void squareRoot(w, e, vector, count)
    Widget	w;
    XEvent	*e;
    String	*vector;
    Cardinal	*count;
{
    pre_op(kSQRT);
    oneop(kSQRT);
    post_op();
}
   
/*ARGSUSED*/
void store(w, e, vector, count)
    Widget	w;
    XEvent	*e;
    String	*vector;
    Cardinal	*count;
{
    pre_op(kSTO);
    rpn ? memf(kSTO) : oneop(kSTO);
    post_op();
}

/*ARGSUSED*/
void subtract(w, e, vector, count)
    Widget	w;
    XEvent	*e;
    String	*vector;
    Cardinal	*count;
{
    pre_op(kSUB);
    rpn ? twof(kSUB) : twoop(kSUB);
    post_op();
}
   
/*ARGSUSED*/
void sum(w, e, vector, count)
    Widget	w;
    XEvent	*e;
    String	*vector;
    Cardinal	*count;
{
    pre_op(kSUM);
    rpn ? memf(kSUM) : oneop(kSUM);
    post_op();
}
   
/*ARGSUSED*/
void tangent(w, e, vector, count)
    Widget	w;
    XEvent	*e;
    String	*vector;
    Cardinal	*count;
{
    pre_op(kTAN);
    oneop(kTAN);
    post_op();
}
   
/*ARGSUSED*/
void tenpower(w, e, vector, count)
    Widget	w;
    XEvent	*e;
    String	*vector;
    Cardinal	*count;
{
    pre_op(k10X);
    oneop(k10X);
    post_op();
}
   
/*ARGSUSED*/
void XexchangeY(w, e, vector, count)
    Widget	w;
    XEvent	*e;
    String	*vector;
    Cardinal	*count;
{
    pre_op(kXXY);
    twof(kXXY);
    post_op();
}
