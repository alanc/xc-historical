/* $XConsortium$ */
/******************************************************************

          Copyright 1991, 1992 by FUJITSU LIMITED
          Copyright 1993 by Digital Equipment Corporation

Permission to use, copy, modify, distribute, and sell this software
and its documentation for any purpose is hereby granted without fee,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of FUJITSU LIMITED and
Digital Equipment Corporation not be used in advertising or publicity
pertaining to distribution of the software without specific, written
prior permission.  FUJITSU LIMITED and Digital Equipment Corporation
makes no representations about the suitability of this software for
any purpose.  It is provided "as is" without express or implied
warranty.

FUJITSU LIMITED AND DIGITAL EQUIPMENT CORPORATION DISCLAIM ALL 
WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL 
FUJITSU LIMITED AND DIGITAL EQUIPMENT CORPORATION BE LIABLE FOR 
ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES 
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER 
IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, 
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF 
THIS SOFTWARE.

  Author:    Takashi Fujiwara     FUJITSU LIMITED 
                               	  fujiwara@a80.tech.yk.fujitsu.co.jp
  Modifier:  Franky Ling          Digital Equipment Corporation
	                          frankyling@hgrd01.enet.dec.com

******************************************************************/

#ifndef _XIMINTL_H
#define _XIMINTL_H

#define	COMPOSE_FILE	"Compose"

/*
 * Data Structure for Local Processing
 */
typedef struct _DefTree {
    struct _DefTree *next; 		/* another Key definition */
    struct _DefTree *succession;	/* successive Key Sequence */
					/* Key definitions */
    unsigned         modifier_mask;
    unsigned         modifier;
    KeySym           keysym;		/* leaf only */
    char            *mb;
    wchar_t         *wc;		/* make from mb */
} DefTree;

typedef struct _XimLocalPrivateRec {
	XIMStyles	*styles;
	XIC		current_ic;
	DefTree		*top;
} XimLocalPrivateRec;

typedef struct _XicThaiPart {
	int		comp_state;
	KeySym		keysym;
	int		input_mode;
} XicThaiPart;

typedef struct _XicLocalPrivateRec {
	long			 value_mask;
	DefTree			*context;
	DefTree			*composed;
	XicThaiPart		 thai;
} XicLocalPrivateRec;

#define XIM_CREATEIC		0
#define XIM_SETICVALUES		1

#define XIM_INPUTSTYLE		(1L << 0)
#define XIM_CLIENTWINDOW	(1L << 1)
#define XIM_FOCUSWINDOW		(1L << 2)
#define XIM_RESOURCENAME	(1L << 3)
#define XIM_RESOURCECLASS	(1L << 4)
#define XIM_GEOMETRYCALLBACK	(1L << 5)
#define XIM_PREEDITCALLBACK	(1L << 6)
#define XIM_STATUSCALLBACK	(1L << 7)

#endif /* _XIMINTL_H */
