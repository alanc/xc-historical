/*****************************************************************************
Copyright 1988, 1989 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************************/

#include <stdio.h>
#ifndef VMS
#include <X11/Xatom.h>
#include <X11/Xos.h>
#else
#include <decw$include/Xatom.h>
#endif
#include "x11perf.h"

extern void DoAtom();

extern void DoNoOp();

extern Bool InitGetProp();
extern void DoGetProp();

extern Bool InitRects();
extern void DoRects();
extern void EndRects();

extern Bool InitValGC();
extern void DoValGC();
extern void EndValGC();

extern Bool InitSegs();
extern Bool InitDashedSegs();
extern Bool InitDoubleDashedSegs();
extern void DoSegs();
extern void EndSegs();

extern Bool InitLines();
extern Bool InitWideLines();
extern Bool InitDashedLines();
extern Bool InitWideDashedLines();
extern Bool InitDoubleDashedLines();
extern Bool InitWideDoubleDashedLines();
extern void DoLines();
extern void EndLines();


extern Bool InitCircles();
extern Bool InitWideCircles();
extern Bool InitDashedCircles();
extern Bool InitWideDashedCircles();
extern Bool InitDoubleDashedCircles();
extern Bool InitWideDoubleDashedCircles();
extern Bool InitEllipses();
extern Bool InitWideEllipses();
extern Bool InitDashedEllipses();
extern Bool InitWideDashedEllipses();
extern Bool InitDoubleDashedEllipses();
extern Bool InitWideDoubleDashedEllipses();
extern void DoArcs();
extern void DoFilledArcs();
extern void EndArcs();

extern Bool InitDots();
extern void DoDots();
extern void EndDots();

extern Bool InitCreate();
extern void CreateChildren();
extern void DestroyChildren();
extern void EndCreate();

extern Bool InitMap();
extern void MapParents();
extern void UnmapParents();

extern Bool InitDestroy();
extern void DestroyParents();
extern void RenewParents();

extern Bool InitMoveWins();
extern void DoMoveWins();
extern void EndMoveWins();

extern void DoResizeWins();

extern Bool InitCircWins();
extern void DoCircWins();
extern void EndCircWins();

extern Bool InitMoveTree();
extern void DoMoveTree();
extern void EndMoveTree();

extern Bool InitText();
extern void DoText();
extern void DoImageText();
extern void DoPolyText();
extern void ClearTextWin();
extern void EndText();

extern Bool InitCopyPlane();
extern void DoCopyPlane();
extern void EndCopyPlane();

extern Bool InitPopups();
extern void DoPopUps();
extern void EndPopups();

extern Bool InitScrolling();
extern void DoScrolling();
extern void MidScroll();
extern void EndScrolling();

extern Bool InitCopyArea();
extern void DoCopyArea();
extern void EndCopyArea();

extern Bool InitCopyPix();
extern void MidCopyPix();
extern void DoCopyPix();
extern void EndCopyPix();

extern Bool InitGetImage();
extern void DoGetImage();
extern void EndGetImage();

extern Bool InitPutImage();
extern void DoPutImage();

#ifdef MITSHM
extern Bool InitShmPutImage();
extern void DoShmPutImage();
extern void EndShmPutImage();
#endif

extern Bool InitTriangles();
extern void DoTriangles();
extern void EndTriangles();

extern Bool InitTrapezoids();
extern void DoTrapezoids();
extern void EndTrapezoids();

extern Bool InitComplexPoly();
extern void DoComplexPoly();
extern void EndComplexPoly();

/*
 * some test must be done a certain multiple of times. That multiple is
 * in the second half of the products below. You can edit the multiplier,
 * just not the multiplicand.
 */
int subs[] = {4, 16, 25, 50, 75, 100, 200, 0};

Test test[] = {
  {"-dot",      "Dot",
		InitDots, DoDots, NullProc, EndDots,    False, 0,
		{500, POLY}},
  {"-rect1",    "1x1 rectangle",
		InitRects, DoRects, NullProc, EndRects, False, 0,
		{500, POLY, 1, NULL, NULL, FillSolid}},
  {"-rect10",   "10x10 rectangle",
		InitRects, DoRects, NullProc, EndRects, False, 0,
		{200, POLY, 10, NULL, NULL, FillSolid}},
  {"-rect100",  "100x100 rectangle",
		InitRects, DoRects, NullProc, EndRects, False, 0,
		{200, 30, 100, NULL, NULL, FillSolid}},
  {"-rect500",  "500x500 rectangle",
		InitRects, DoRects, NullProc, EndRects, False, 0,
		{200, 1, 500, NULL, NULL, FillSolid}},
  {"-tsrect1",  "1x1 transparent stippled rectangle",
		InitRects, DoRects, NullProc, EndRects, False, 0,
		{500, POLY, 1, NULL, NULL, FillStippled}},
  {"-tsrect10", "10x10 transparent stippled rectangle",
		InitRects, DoRects, NullProc, EndRects, False, 0,
		{200, POLY, 10, NULL, NULL, FillStippled}},
  {"-tsrect100", "100x100 transparent stippled rectangle",
		InitRects, DoRects, NullProc, EndRects, False, 0,
		{200, 30, 100, NULL, NULL, FillStippled}},
  {"-tsrect500", "500x500 transparent stippled rectangle",
		InitRects, DoRects, NullProc, EndRects, False, 0,
		{200, 1, 500, NULL, NULL, FillStippled}},
  {"-osrect",  "1x1 opaque stippled rectangle",
		InitRects, DoRects, NullProc, EndRects, False, 0,
		{500, POLY, 1, NULL, NULL, FillOpaqueStippled}},
  {"-osrect10", "10x10 opaque stippled rectangle",
		InitRects, DoRects, NullProc, EndRects, False, 0,
		{200, POLY, 10, NULL, NULL, FillOpaqueStippled}},
  {"-osrect100", "100x100 opaque stippled rectangle",
		InitRects, DoRects, NullProc, EndRects, False, 0,
		{200, 30, 100, NULL, NULL, FillOpaqueStippled}},
  {"-osrect500", "500x500 opaque stippled rectangle",
		InitRects, DoRects, NullProc, EndRects, False, 0,
		{200, 1, 500, NULL, NULL, FillOpaqueStippled}},
  {"-tilerect1", "1x1 4x4 tiled rectangle",
		InitRects, DoRects, NullProc, EndRects, False, 0,
		{500, POLY, 1, NULL, NULL, FillTiled}},
  {"-tilerect10", "10x10 4x4 tiled rectangle",
		InitRects, DoRects, NullProc, EndRects, False, 0,
		{200, POLY, 10, NULL, NULL, FillTiled}},
  {"-tilerect100", "100x100 4x4 tiled rectangle",
		InitRects, DoRects, NullProc, EndRects, False, 0,
		{200, 30, 100, NULL, NULL, FillTiled}},
  {"-tilerect500", "500x500 4x4 tiled rectangle",
		InitRects, DoRects, NullProc, EndRects, False, 0,
		{200, 1, 500, NULL, NULL, FillTiled}},
  {"-bigtilerect1", "1x1 161x145 tiled rectangle",
		InitRects, DoRects, NullProc, EndRects, False, 0,
		{200, POLY, 1, "big", NULL, FillTiled}},
  {"-bigtilerect10", "10x10 161x145 tiled rectangle",
		InitRects, DoRects, NullProc, EndRects, False, 0,
		{200, POLY, 10, "big", NULL, FillTiled}},
  {"-bigtilerect100", "100x100 161x145 tiled rectangle",
		InitRects, DoRects, NullProc, EndRects, False, 0,
		{200, 30, 100, "big", NULL, FillTiled}},
  {"-bigtilerect500", "500x500 161x145 tiled rectangle",
		InitRects, DoRects, NullProc, EndRects, False, 0,
		{200, 1, 500, "big", NULL, FillTiled}},
  {"-seg1",     "1-pixel line segment",
		InitSegs, DoSegs, NullProc, EndSegs,    False, 0,
		{200, POLY, 1}},
  {"-seg10",    "10-pixel line segment",
		InitSegs, DoSegs, NullProc, EndSegs,    False, 0,
		{200, POLY, 10}},
  {"-seg100",   "100-pixel line segment",
		InitSegs, DoSegs, NullProc, EndSegs,    False, 0,
		{100, POLY, 100}},
  {"-seg500",   "500-pixel line segment",
		InitSegs, DoSegs, NullProc, EndSegs,    False, 0,
		{20, POLY, 500}},
  {"-seg100c1", "100-pixel line segment (1 clip rect)",
		InitSegs, DoSegs, NullProc, EndSegs,    False, 1,
		{100, POLY, 100}},
  {"-seg100c2", "100-pixel line segment (2 clip rects)",
		InitSegs, DoSegs, NullProc, EndSegs,    False, 2,
		{100, POLY, 100}},
  {"-seg100c3", "100-pixel line segment (3 clip rects)",
		InitSegs, DoSegs, NullProc, EndSegs,    False, 3,
		{100, POLY, 100}},
  {"-dseg10",   "10-pixel dashed segment",
		InitDashedSegs, DoSegs, NullProc, EndSegs, False, 0,
		{10, POLY, 10}},
  {"-dseg100", "100-pixel dashed segment",
		InitDashedSegs, DoSegs, NullProc, EndSegs, False, 0,
		{10, POLY, 100}},
  {"-ddseg100", "100-pixel double-dashed segment",
		InitDoubleDashedSegs, DoSegs, NullProc, EndSegs, False, 0,
		{10, POLY, 100}},
  {"-line1",   "1-pixel line",
		InitLines, DoLines, NullProc, EndLines, False, 0,
		{200, POLY, 1}},
  {"-line10",   "10-pixel line",
		InitLines, DoLines, NullProc, EndLines, False, 0,
		{200, POLY, 10}},
  {"-line100",  "100-pixel line",
		InitLines, DoLines, NullProc, EndLines, False, 0,
		{100, POLY, 100}},
  {"-line500", "500-pixel line",
		InitLines, DoLines, NullProc, EndLines, False, 0,
		{20, POLY, 500}},
  {"-dline10",  "10-pixel dashed line",
		InitDashedLines, DoLines, NullProc, EndLines, False, 0,
		{10, POLY, 10}},
  {"-dline100", "100-pixel dashed line",
		InitDashedLines, DoLines, NullProc, EndLines, False, 0,
		{10, POLY, 100}},
  {"-ddline100", "100-pixel double-dashed line",
		InitDoubleDashedLines, DoLines, NullProc, EndLines, False, 0,
		{10, POLY, 100}},
  {"-wline10",   "10x1 wide line",
		InitWideLines, DoLines, NullProc, EndLines, False, 0,
		{200, 100, 10}},
  {"-wline100",  "100x10 wide line",
		InitWideLines, DoLines, NullProc, EndLines, False, 0,
		{100, 100, 100}},
  {"-wline500", "500x50 wide line",
		InitWideLines, DoLines, NullProc, EndLines, False, 0,
		{20, 50, 500}},
  {"-wdline100",  "100x10 wide dashed line",
		InitWideDashedLines, DoLines, NullProc, EndLines,
		False, 0,
		{100, 100, 100}},
  {"-wddline100",  "100x10 wide double-dashed line",
		InitWideDoubleDashedLines, DoLines, NullProc, EndLines,
		False, 0,
		{100, 100, 100}},
  {"-circle1",  "1-pixel circle",
		InitCircles, DoArcs, NullProc, EndArcs, False, 0,
		{20, POLY, 1}},
  {"-circle10", "10-pixel circle",
		InitCircles, DoArcs, NullProc, EndArcs, False, 0,
		{10, POLY, 10}},
  {"-circle100", "100-pixel circle",
		InitCircles, DoArcs, NullProc, EndArcs, False, 0,
		{40, 100, 100}},
  {"-circle500", "500-pixel circle",
		InitCircles, DoArcs, NullProc, EndArcs, False, 0,
		{20, 25, 500}},
  {"-dcircle100", "100-pixel dashed circle",
		InitDashedCircles, DoArcs, NullProc, EndArcs, False, 0,
		{40, 100, 100}},
  {"-ddcircle100", "100-pixel double-dashed circle",
		InitDoubleDashedCircles, DoArcs, NullProc, EndArcs, False, 0,
		{40, 100, 100}},
  {"-wcircle10", "10-pixel wide circle",
		InitWideCircles, DoArcs, NullProc, EndArcs, False, 0,
		{10, POLY, 10}},
  {"-wcircle100", "100-pixel wide circle",
		InitWideCircles, DoArcs, NullProc, EndArcs, False, 0,
		{40, 100, 100}},
  {"-wcircle500", "500-pixel wide circle",
		InitWideCircles, DoArcs, NullProc, EndArcs, False, 0,
		{20, 25, 500}},
  {"-wdcircle100", "100-pixel wide dashed circle",
		InitWideDashedCircles, DoArcs, NullProc, EndArcs, False, 0,
		{40, 100, 100}},
  {"-wddcircle100", "100-pixel wide double-dashed circle",
		InitWideDoubleDashedCircles, DoArcs, NullProc, EndArcs,
		False, 0,
		{40, 100, 100}},
  {"-fcircle1",  "1-pixel solid circle",
		InitCircles, DoFilledArcs, NullProc, EndArcs, False, 0,
		{20, POLY, 1}},
  {"-fcircle10", "10-pixel solid circle",
		InitCircles, DoFilledArcs, NullProc, EndArcs, False, 0,
		{10, POLY, 10}},
  {"-fcircle100", "100-pixel solid circle",
		InitCircles, DoFilledArcs, NullProc, EndArcs, False, 0,
		{40, 25, 100}},
  {"-fcircle500", "500-pixel solid circle",
		InitCircles, DoFilledArcs, NullProc, EndArcs, False, 0,
		{20, 1, 500}},
  {"-ellipse10", "10-pixel ellipse",
		InitEllipses, DoArcs, NullProc, EndArcs, False, 0,
		{20, 200, 10}},
  {"-ellipse100", "100-pixel ellipse",
		InitEllipses, DoArcs, NullProc, EndArcs, False, 0,
		{20, 25, 100}},
  {"-ellipse500", "500-pixel ellipse",
		InitEllipses, DoArcs, NullProc, EndArcs, False, 0,
		{10, 20, 500}},
  {"-dellipse100", "100-pixel dashed ellipse",
		InitDashedEllipses, DoArcs, NullProc, EndArcs, False, 0,
		{20, 25, 100}},
  {"-ddellipse100", "100-pixel double-dashed ellipse",
		InitDoubleDashedEllipses, DoArcs, NullProc, EndArcs, False, 0,
		{20, 25, 100}},
  {"-wellipse10", "10-pixel wide ellipse",
		InitWideEllipses, DoArcs, NullProc, EndArcs, False, 0,
		{20, 200, 10}},
  {"-wellipse100", "100-pixel wide ellipse",
		InitWideEllipses, DoArcs, NullProc, EndArcs, False, 0,
		{20, 25, 100}},
  {"-wellipse500", "500-pixel wide ellipse",
		InitWideEllipses, DoArcs, NullProc, EndArcs, False, 0,
		{10, 20, 500}},
  {"-wdellipse100", "100-pixel wide dashed ellipse",
		InitWideDashedEllipses, DoArcs, NullProc, EndArcs, False, 0,
		{20, 25, 100}},
  {"-wddellipse100", "100-pixel wide double-dashed ellipse",
		InitWideDoubleDashedEllipses, DoArcs, NullProc, EndArcs,
		False, 0,
		{20, 25, 100}},
  {"-fellipse10", "10-pixel filled ellipse",
		InitEllipses, DoFilledArcs, NullProc, EndArcs, False, 0,
		{20, 200, 10}},
  {"-fellipse100", "100-pixel filled ellipse",
		InitEllipses, DoFilledArcs, NullProc, EndArcs, False, 0,
		{20, 25, 100}},
  {"-fellipse500", "500-pixel filled ellipse",
		InitEllipses, DoFilledArcs, NullProc, EndArcs, False, 0,
		{10, 10, 500}},
  {"-triangle1", "Fill 1-pixel/side triangle",
		InitTriangles, DoTriangles, NullProc, EndTriangles, 
                False, 0,
		{50, POLY, 1}},
  {"-triangle10", "Fill 10-pixel/side triangle",
		InitTriangles, DoTriangles, NullProc, EndTriangles, 
                False, 0,
		{50, POLY, 10}},
  {"-triangle100", "Fill 100-pixel/side triangle",
		InitTriangles, DoTriangles, NullProc, EndTriangles, 
                False, 0,
		{50, 100, 100}},
  {"-trap10", "Fill 10x10 trapezoid",
		InitTrapezoids, DoTrapezoids, NullProc, EndTrapezoids,
		False, 0,
		{50, POLY, 10}},
  {"-trap100", "Fill 100x100 trapezoid",
		InitTrapezoids, DoTrapezoids, NullProc, EndTrapezoids,
		False, 0,
		{50, POLY/10, 100}},
  {"-complex10", "Fill 10-pixel/side complex polygon",
		InitComplexPoly, DoComplexPoly, NullProc, EndComplexPoly, 
                False, 0,
		{20, POLY, 10}},
  {"-complex100", "Fill 100-pixel/side complex polygons",
		InitComplexPoly, DoComplexPoly, NullProc, EndComplexPoly, 
                False, 0,
		{20, POLY/10, 100}},
  {"-ftext",    "Char in 80-char line (6x13)",
		InitText, DoText, ClearTextWin, EndText, False, 0,
		{200000, 80, False, "6x13", NULL}},
  {"-tr10text", "Char in 80-char line (TR 10)",
		InitText, DoText, ClearTextWin, EndText, False, 0,
		{2000, 80, False, 
		"-adobe-times-medium-r-normal--10-100-75-75-p-54-iso8859-1", 
		NULL}},
  {"-tr24text", "Char in 30-char line (TR 24)",
		InitText, DoText, ClearTextWin, EndText, False, 0,
		{2000, 30, False, 
		"-adobe-times-medium-r-normal--24-240-75-75-p-124-iso8859-1",
 		NULL}},
  {"-polytext", "Char in 20/40/20 line (6x13, TR 10)",
		InitText, DoPolyText, ClearTextWin, EndText, False, 0,
		{2000, 80, True, "6x13", 
		"-adobe-times-medium-r-normal--10-100-75-75-p-54-iso8859-1"}},
  {"-fitext",   "Char in 80-char image line (6x13)",
		InitText, DoImageText, ClearTextWin, EndText, False, 0,
		{2000, 80, False, "6x13", NULL}},
  {"-tr10itext", "Char in 80-char image line (TR 10)",
		InitText, DoImageText, ClearTextWin, EndText, False, 0,
		{2000, 80, False, 
		"-adobe-times-medium-r-normal--10-100-75-75-p-54-iso8859-1", 
		NULL}},
  {"-tr24itext", "Char in 30-char image line (TR 24)",
		InitText, DoImageText, ClearTextWin, EndText, False, 0,
		{2000, 30, False, 
		"-adobe-times-medium-r-normal--24-240-75-75-p-124-iso8859-1",
 		NULL}},
  {"-scroll10", "Scroll 10x10 pixels",
		InitScrolling, DoScrolling, MidScroll, EndScrolling, False, 0,
		{100, 1, 10}},
  {"-scroll100", "Scroll 100x100 pixels",
		InitScrolling, DoScrolling, MidScroll, EndScrolling, False, 0,
		{100, 1, 100}},
  {"-scroll500", "Scroll 500x500 pixels",
		InitScrolling, DoScrolling, MidScroll, EndScrolling, False, 0,
		{100, 1, 500}},
  {"-copyarea10", "Copy 10x10 square",
		InitCopyArea, DoCopyArea, MidScroll, EndCopyArea, False, 0,
		{2500, 4, 10}},
  {"-copyarea100", "Copy 100x100 square",
		InitCopyArea, DoCopyArea, MidScroll, EndCopyArea, False, 0,
		{500, 4, 100}},
  {"-copyarea500", "Copy 500x500 square",
		InitCopyArea, DoCopyArea, MidScroll, EndCopyArea, False, 0,
		{25, 4, 500}},
  {"-copypix10", "Copy 10x10 square from pixmap",
		InitCopyPix, DoCopyPix, MidCopyPix, EndCopyPix, False, 0,
		{2500, 4, 10}},
  {"-copypix100", "Copy 100x100 square from pixmap",
		InitCopyPix, DoCopyPix, MidCopyPix, EndCopyPix, False, 0,
		{500, 4, 100}},
  {"-copypix500", "Copy 500x500 square from pixmap",
		InitCopyPix, DoCopyPix, MidCopyPix, EndCopyPix, False, 0,
		{25, 4, 500}},
  {"-copyplane10", "Copy 10x10 1-bit deep plane",
		InitCopyPlane, DoCopyPlane, MidCopyPix, EndCopyPix, False, 0,
		{50, 4, 10}},
  {"-copyplane100", "Copy 100x100 1-bit deep plane",
		InitCopyPlane, DoCopyPlane, MidCopyPix, EndCopyPix, False, 0,
		{50, 4, 100}},
  {"-copyplane500", "Copy 500x500 1-bit deep plane",
		InitCopyPlane, DoCopyPlane, MidCopyPix, EndCopyPix, False, 0,
		{50, 4, 500}},
  {"-putimage10", "PutImage 10x10 square",
		InitPutImage, DoPutImage, MidCopyPix, EndGetImage, False, 0,
		{2500, 4, 10}},
  {"-putimage100", "PutImage 100x100 square",
		InitPutImage, DoPutImage, MidCopyPix, EndGetImage, False, 0,
		{500, 4, 100}},
  {"-putimage500", "PutImage 500x500 square",
		InitPutImage, DoPutImage, MidCopyPix, EndGetImage, False, 0,
		{25, 4, 500}},
#ifdef MITSHM
  {"-shmput10", "ShmPutImage 10x10 square",
		InitShmPutImage, DoShmPutImage, MidCopyPix, EndGetImage,
		False, 0,
		{2500, 4, 10}},
  {"-shmput100", "ShmPutImage 100x100 square",
		InitShmPutImage, DoShmPutImage, MidCopyPix, EndGetImage,
		False, 0,
		{500, 4, 100}},
  {"-shmput500", "ShmPutImage 500x500 square",
		InitShmPutImage, DoShmPutImage, MidCopyPix, EndGetImage,
		False, 0,
		{25, 4, 500}},
#endif
  {"-getimage10", "GetImage 10x10 square",
		InitGetImage, DoGetImage, NullProc, EndGetImage, False, 0,
		{2500, 4, 10}},
  {"-getimage100", "GetImage 100x100 square",
		InitGetImage, DoGetImage, NullProc, EndGetImage, False, 0,
		{500, 4, 100}},
  {"-getimage500", "GetImage 500x500 square",
		InitGetImage, DoGetImage, NullProc, EndGetImage, False, 0,
		{25, 4, 500}},
  {"-noop",     "X protocol NoOperation",
		NullInitProc, DoNoOp, NullProc, NullProc, False, 0,
		{5000, 1}},
  {"-atom",     "GetAtomName",
		NullInitProc, DoAtom, NullProc, NullProc, False, 0,
		{5000, 1}},
  {"-prop",     "GetProperty",
		InitGetProp, DoGetProp, NullProc, NullProc, False, 0,
		{5000, 1}},
  {"-gc",       "Graphics context validation",
		InitValGC, DoValGC, NullProc, EndValGC, False, 0,
		{125000, 4}},
  {"-create",   "Create and map subwindows",
		InitCreate, CreateChildren, DestroyChildren, EndCreate,
		True, 0,
		{0, 0, True}},
  {"-ucreate",  "Create unmapped window",
		InitCreate, CreateChildren, DestroyChildren, EndCreate,
		True, 0,
		{0, 0, False}},
  {"-map",      "Map window via parent",
		InitMap, MapParents, UnmapParents, EndCreate, True, 0,
		{0, 0, True}},
  {"-unmap",    "Unmap window via parent",
		InitDestroy, UnmapParents, MapParents, EndCreate, True, 0,
		{0, 0, True}},
  {"-destroy",  "Destroy window via parent",
		InitDestroy, DestroyParents, RenewParents, EndCreate,
		True, 0,
		{0, 0, True}},
  {"-popup",    "Hide/expose window via popup",
		InitPopups, DoPopUps, NullProc, EndPopups, True, 0,
		{6000, 0, True}},
  {"-move",     "Move window",
		InitMoveWins, DoMoveWins, NullProc, EndMoveWins, True, 0,
		{1200, 0, True}},
  {"-umove",    "Moved unmapped window",
		InitMoveWins, DoMoveWins, NullProc, EndMoveWins, True, 0,
		{12000, 0, False}},
  {"-movetree", "Move window via parent",
		InitMoveTree, DoMoveTree, NullProc, EndMoveTree, True, 0,
		{1200, 4, True}},
  {"-resize",   "Resize window",
		InitMoveWins, DoResizeWins, NullProc, EndMoveWins, True, 0,
		{1200, 4, True}},
  {"-uresize",  "Resize unmapped window",
		InitMoveWins, DoResizeWins, NullProc, EndMoveWins, True, 0,
		{12000, 4, False}},
  {"-circulate", "Circulate window",
		InitCircWins, DoCircWins, NullProc, EndCircWins, True, 0,
		{1200, 4, True}},
  {"-ucirculate", "Circulate Unmapped window",
		InitCircWins, DoCircWins, NullProc, EndCircWins, True, 0,
		{12000, 4, False}},
  { NULL, NULL,
		NULL, NULL, NULL, NULL, False, 0,
		{0, 0, False, NULL, NULL}}
};
