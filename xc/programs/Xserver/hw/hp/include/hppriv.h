/* $XConsortium$ */
/*************************************************************************
 * 
 * (c)Copyright 1992 Hewlett-Packard Co.,  All Rights Reserved.
 * 
 *                          RESTRICTED RIGHTS LEGEND
 * Use, duplication, or disclosure by the U.S. Government is subject to
 * restrictions as set forth in sub-paragraph (c)(1)(ii) of the Rights in
 * Technical Data and Computer Software clause in DFARS 252.227-7013.
 * 
 *                          Hewlett-Packard Company
 *                          3000 Hanover Street
 *                          Palo Alto, CA 94304 U.S.A.
 * 
 * Rights for non-DOD U.S. Government Departments and Agencies are as set
 * forth in FAR 52.227-19(c)(1,2).
 *
 *************************************************************************/

/**************************************************
Copyright (c) 1988 by Hewlett-Packard Company
Copyright (c) 1987, 1988 by Digital Equipment Corporation, Maynard, 
              Massachusetts, and the Massachusetts Institute of Technology, 
              Cambridge, Massachusetts

Permission to use, copy, modify, and distribute this software 
and its documentation for any purpose and without fee is hereby 
granted, provided that the above copyright notice appear in all 
copies and that both that copyright notice and this permission 
notice appear in supporting documentation, and that the names of 
Hewlett-Packard, Digital or  M.I.T.  not be used in advertising or 
publicity pertaining to distribution of the software without specific, 
written prior permission.

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

HEWLETT-PACKARD MAKES NO WARRANTY OF ANY KIND WITH REGARD
TO THIS SOFWARE, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE.  Hewlett-Packard shall not be liable for errors
contained herein or direct, indirect, special, incidental or
consequential damages in connection with the furnishing,
performance, or use of this material.

***********************************************/


/* 
 * Number of depths that we can possible have on our hardware per
 * screen in the foreseeable future
 */

#ifndef		MAX_PATH_NAME
#define		MAX_PATH_NAME	256
#endif		/* MAX_PATH_NAME */

#define		MAX_DRIVER_NAME	32
#define		MAX_DEPTHS	16


/* private field for XOS displays */
typedef struct _hpPriv {
    
    /* Start up information from X*screens file */
    char	StandardDevice[MAX_PATH_NAME];
    char	StandardDriver[MAX_DRIVER_NAME];
    int		StandardFd;
    int		StandardNumDepths;
    int		StandardDepths[ MAX_DEPTHS ];
    Bool	StandardDoubleBuffer;
    int		StandardDoubleBufferDepth;
    int		LineNumber;
    int		MonitorDiagonal;	/* in units of .001 inches */
    int		depth;			/* Device depth.  Defaults to 8, */
					/*  can be set via Xnscreens file */

    /* Hooks for the input driver to communicate with the output driver */
    void	(*MoveMouse)();
    void	(*CursorOff)();
    void	(*ChangeScreen)();
    Bool	isSaved;
} hpPriv;

typedef hpPriv *hpPrivPtr;
