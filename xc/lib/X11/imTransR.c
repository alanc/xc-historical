/* $XConsortium: imTransR.c,v 1.1 93/09/17 13:29:30 rws Exp $ */
/******************************************************************

              Copyright 1992 by Sun Microsystems, Inc.
              Copyright 1992, 1993 by FUJITSU LIMITED

Permission to use, copy, modify, distribute, and sell this software
and its documentation for any purpose is hereby granted without fee,
provided that the above copyright notice appear in all copies and
that both that copyright notice and this permission notice appear
in supporting documentation, and that the name of Sun Microsystems, Inc.
and FUJITSU LIMITED not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior permission.
Sun Microsystems, Inc. and FUJITSU LIMITED makes no representations about
the suitability of this software for any purpose.
It is provided "as is" without express or implied warranty.

Sun Microsystems Inc. AND FUJITSU LIMITED DISCLAIMS ALL WARRANTIES WITH
REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS, IN NO EVENT SHALL Sun Microsystems, Inc. AND FUJITSU LIMITED
BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

  Author: Hideki Hiura (hhiura@Sun.COM) Sun Microsystems, Inc.
          Takashi Fujiwara     FUJITSU LIMITED 
                               fujiwara@a80.tech.yk.fujitsu.co.jp

******************************************************************/

#include "Xlibint.h"
#include "Xlcint.h"
#include "XimTrInt.h"
#include "Ximint.h"

extern Bool	_XimXConf(
#if NeedFunctionPrototypes
    Xim		 im,
    char	*address
#endif
);

#ifdef TCPCONN
extern Bool	_XimTcpInetConf(
#if NeedFunctionPrototypes
    Xim		 im,
    char	*address
#endif
);

extern Bool	_XimTcpInternalConf(
#if NeedFunctionPrototypes
    Xim		 im,
    char	*address
#endif
);
#endif /* TCPCONN */

#ifdef DNETCONN
extern Bool	_XimDecnetConf(
#if NeedFunctionPrototypes
    Xim		 im,
    char	*address
#endif
);
#endif /* DNETCONN */

#ifdef STREAMSCONN
extern Bool	_XimStreamsConf(
#if NeedFunctionPrototypes
    Xim		 im,
    char	*address
#endif
);
#endif /* STREAMSCONN */

Public TransportSW _XimTransportRec[] = {
    "X",          1, _XimXConf,  /* 1st entry must be X. 
					This will be a fallback */
#ifdef TCPCONN
    "tcp",        3, _XimTcpInetConf,
    "local",      4, _XimTcpInternalConf,
#endif /* TCPCONN */
#ifdef DNETCONN
    "decnet",     6, _XimDecnetConf,
#endif /* DNETCONN */
#ifdef STREAMSCONN
    "streams",    6, _XimStreamsConf,
#endif /* STREAMSCONN */
    (char *)NULL, 0, (Bool (*)())NULL,
};
