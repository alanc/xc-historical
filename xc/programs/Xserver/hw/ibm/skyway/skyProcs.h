/*
 * $XConsortium: skyProcs.h,v 1.1 91/05/10 09:09:08 jap Exp $
 *
 * Copyright IBM Corporation 1987,1988,1989,1990,1991
 *
 * All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of IBM not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * IBM DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * IBM BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
*/

/*
 * skyProcs.h - function calls
 */

#ifndef _SKYPROCS_H
#define _SKYPROCS_H

extern int  skyProbe();
extern void skySetColor();
extern Bool skyScreenInit();
extern int  skyHdwrInit();
extern Bool skyScreenIO();
extern void skySaveState();
extern void skyRestoreState();
extern Bool skyScreenClose();

extern void skyQueryBestSize();
extern void skyStoreColors();
extern int  skyListInstalledColormaps();
extern void skyInstallColormap();
extern void skyUninstallColormap();
extern void skyRefreshColormaps();

extern void SkywaySetColor();
extern void SkywayBitBlt();
extern void SkywayFillSolid();
extern void SkywayTileRect();
extern void skywayReadColorImage();
extern void skywayDrawColorImage();

extern Bool cfbScreenInit();

#endif /* _SKYPROC_H */
