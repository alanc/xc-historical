#include <stdio.h>
#ifndef VMS
#include <X11/Xatom.h>
#include <sys/time.h>
#else
#include <decw$include/Xatom.h>
#endif
#include "x11perf.h"

extern void DoAtom();

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
extern void DoSegs();
extern void EndSegs();

extern Bool InitLines();
extern Bool InitDashedLines();
extern void DoLines();
extern void EndLines();

extern Bool InitCircles();
extern Bool InitEllipses();
extern void DoArcs();
extern void EndArcs();

extern Bool InitDots();
extern void DoDots();
extern void EndDots();

extern Bool InitCreate();
extern void CreateChildren();
extern void DeleteChildren();
extern void EndCreate();

extern Bool InitMap();
extern void MapParents();
extern void UnmapParents();

extern Bool InitMoveWins();
extern void DoMoveWins();
extern void EndMoveWins();

extern void DoResizeWins();

extern Bool InitCircWins();
extern void DoCircWins();

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

extern Bool InitCopyArea2();
extern void MidCopyArea2();
extern void DoCopyArea2();
extern void EndCopyArea2();

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
  {"-dots",     "Dot",
		InitDots, DoDots, NullProc, EndDots,    False, 0,
		{500, POLY}},
  {"-rects1",   "1x1 Rectangle",
		InitRects, DoRects, NullProc, EndRects, False, 0,
		{500, POLY, 1, NULL, NULL, FillSolid}},
  {"-rects10",  "10x10 Rectangle",
		InitRects, DoRects, NullProc, EndRects, False, 0,
		{200, POLY, 10, NULL, NULL, FillSolid}},
  {"-rects50",  "50x50 Rectangle",
		InitRects, DoRects, NullProc, EndRects, False, 0,
		{200, POLY/10, 50, NULL, NULL, FillSolid}},
  {"-tsrects1", "1x1 Transparent Stippled Rectangle",
		InitRects, DoRects, NullProc, EndRects, False, 0,
		{500, POLY, 1, NULL, NULL, FillStippled}},
  {"-tsrects10", "10x10 Transparent Stippled Rectangle",
		InitRects, DoRects, NullProc, EndRects, False, 0,
		{200, POLY, 10, NULL, NULL, FillStippled}},
  {"-tsrects50", "50x50 Transparent Stippled Rectangle",
		InitRects, DoRects, NullProc, EndRects, False, 0,
		{200, POLY/10, 50, NULL, NULL, FillStippled}},
  {"-osrects1", "1x1 Opaque Stippled Rectangle",
		InitRects, DoRects, NullProc, EndRects, False, 0,
		{500, POLY, 1, NULL, NULL, FillOpaqueStippled}},
  {"-osrects10", "10x10 Opaque Stippled Rectangle",
		InitRects, DoRects, NullProc, EndRects, False, 0,
		{200, POLY, 10, NULL, NULL, FillOpaqueStippled}},
  {"-osrects50", "50x50 Opaque Stippled Rectangle",
		InitRects, DoRects, NullProc, EndRects, False, 0,
		{200, POLY/10, 50, NULL, NULL, FillOpaqueStippled}},
  {"-tilerects1", "1x1 4x4 Tiled Rectangle",
		InitRects, DoRects, NullProc, EndRects, False, 0,
		{500, POLY, 1, NULL, NULL, FillTiled}},
  {"-tilerects10", "10x10 4x4 Tiled Rectangle",
		InitRects, DoRects, NullProc, EndRects, False, 0,
		{200, POLY, 10, NULL, NULL, FillTiled}},
  {"-tilerects50", "50x50 4x4 Tiled Rectangle",
		InitRects, DoRects, NullProc, EndRects, False, 0,
		{200, POLY/10, 50, NULL, NULL, FillTiled}},
  {"-tilerects500", "500x500 4x4 Tiled Rectangle",
		InitRects, DoRects, NullProc, EndRects, False, 0,
		{200, 1, 500, NULL, NULL, FillTiled}},
  {"-segs1", "1-pixel line segment",
		InitSegs, DoSegs, NullProc, EndSegs,    False, 0,
		{200, POLY, 1}},
  {"-segs10", "10-pixel line segment",
		InitSegs, DoSegs, NullProc, EndSegs,    False, 0,
		{200, POLY, 10}},
  {"-segs100", "100-pixel line segment",
		InitSegs, DoSegs, NullProc, EndSegs,    False, 0,
		{100, POLY, 100}},
  {"-segs500", "500-pixel line segment",
		InitSegs, DoSegs, NullProc, EndSegs,    False, 0,
		{20, POLY, 500}},
  {"-segs100c1", "100-pixel line segment (1 clip window)",
		InitSegs, DoSegs, NullProc, EndSegs,    False, 1,
		{100, POLY, 100}},
  {"-segs100c2", "100-pixel line segment (2 clip windows)",
		InitSegs, DoSegs, NullProc, EndSegs,    False, 2,
		{100, POLY, 100}},
  {"-segs100c3", "100-pixel line segment (3 clip windows)",
		InitSegs, DoSegs, NullProc, EndSegs,    False, 3,
		{100, POLY, 100}},
  {"-dsegs100", "100-pixel dashed segment",
		InitDashedSegs, DoSegs, NullProc, EndSegs, False, 0,
		{10, POLY, 100}},
  {"-lines1", "1-pixel line",
		InitLines, DoLines, NullProc, EndLines, False, 0,
		{200, POLY, 1}},
  {"-lines10", "10-pixel line",
		InitLines, DoLines, NullProc, EndLines, False, 0,
		{200, POLY, 10}},
  {"-lines100", "100-pixel line",
		InitLines, DoLines, NullProc, EndLines, False, 0,
		{100, POLY, 100}},
  {"-lines500", "500-pixel line",
		InitLines, DoLines, NullProc, EndLines, False, 0,
		{20, POLY, 500}},
  {"-dlines100", "100-pixel dashed line",
		InitDashedLines, DoLines, NullProc, EndLines, False, 0,
		{10, POLY, 100}},
  {"-circles1", "1-pixel diameter circle",
		InitCircles, DoArcs, NullProc, EndArcs, False, 0,
		{20, POLY, 1}},
  {"-circles10", "10-pixel diameter circle",
		InitCircles, DoArcs, NullProc, EndArcs, False, 0,
		{10, POLY, 10}},
  {"-circles100", "100-pixel diameter circle",
		InitCircles, DoArcs, NullProc, EndArcs, False, 0,
		{40, POLY/40, 100}},
  {"-circles500", "500-pixel diameter circle",
		InitCircles, DoArcs, NullProc, EndArcs, False, 0,
		{20, POLY/100, 500}},
  {"-ellipses1", "1-pixel major-axis diameter ellipse",
		InitEllipses, DoArcs, NullProc, EndArcs, False, 0,
		{20, POLY, 1}},
  {"-ellipses10", "10-pixel major-axis diameter ellipse",
		InitEllipses, DoArcs, NullProc, EndArcs, False, 0,
		{20, POLY/4, 10}},
  {"-ellipses100", "100-pixel major-axis diameter ellipse",
		InitEllipses, DoArcs, NullProc, EndArcs, False, 0,
		{20, POLY/40, 100}},
  {"-ellipses500", "500-pixel major-axis diameter ellipse",
		InitEllipses, DoArcs, NullProc, EndArcs, False, 0,
		{10, POLY/100, 500}},
  {"-ftext",    "Char in 80-char line (6x13)",
		InitText, DoText, ClearTextWin, EndText, False, 0,
		{200000, 80, False, "6x13", NULL}},
  {"-tr10text", "Char in 80-char line (TR 10)",
		InitText, DoText, ClearTextWin, EndText, False, 0,
		{2000, 80, False, "times_roman10", NULL}},
  {"-tr24text", "Char in 30-char line (TR 24)",
		InitText, DoText, ClearTextWin, EndText, False, 0,
		{2000, 30, False, "times_roman24", NULL}},
  {"-polytext", "Char in 20/40/20 line (6x13, TR 10)",
		InitText, DoPolyText, ClearTextWin, EndText, False, 0,
		{2000, 80, True, "6x13", "times_roman10"}},
  {"-fitext",   "Char in 80-char image line (6x13)",
		InitText, DoImageText, ClearTextWin, EndText, False, 0,
		{2000, 80, False, "6x13", NULL}},
  {"-tr10itext", "Char in 80-char image line (TR 10)",
		InitText, DoImageText, ClearTextWin, EndText, False, 0,
		{2000, 80, False, "times_roman10", NULL}},
  {"-tr24itext", "Char in 30-char image line (TR 24)",
		InitText, DoImageText, ClearTextWin, EndText, False, 0,
		{2000, 30, False, "times_roman24", NULL}},
  {"-copyplane10", "Copy 10x10 1-bit deep plane",
		InitCopyPlane, DoCopyPlane, MidCopyArea2, EndCopyArea2,
		False, 0,
		{50, 4, 10}},
  {"-copyplane100", "Copy 100x100 1-bit deep plane",
		InitCopyPlane, DoCopyPlane, MidCopyArea2, EndCopyArea2,
		False, 0,
		{50, 4, 100}},
  {"-copyplane500", "Copy 500x500 1-bit deep plane",
		InitCopyPlane, DoCopyPlane, MidCopyArea2, EndCopyArea2,
		False, 0,
		{50, 4, 500}},
  {"-scroll", "Scroll 600x599 pixels",
		InitScrolling, DoScrolling, MidScroll,
		EndScrolling, False, 0,
		{100, 1, False}},
  {"-copyarea10", "Copyarea 10x10 square",
		InitCopyArea, DoCopyArea, MidScroll,
		EndCopyArea, False, 0,
		{2500, 4, 10}},
  {"-copyarea100", "Copyarea 100x100 square",
		InitCopyArea, DoCopyArea, MidScroll,
		EndCopyArea, False, 0,
		{500, 4, 100}},
  {"-copyarea500", "Copyarea 500x500 square",
		InitCopyArea, DoCopyArea, MidScroll,
		EndCopyArea, False, 0,
		{25, 4, 500}},
  {"-copypix10", "Copyarea 10x10 square from pixmap",
		InitCopyArea2, DoCopyArea2, MidCopyArea2,
		EndCopyArea2, False, 0,
		{2500, 4, 10}},
  {"-copypix100", "Copyarea 100x100 square from pixmap",
		InitCopyArea2, DoCopyArea2, MidCopyArea2,
		EndCopyArea2, False, 0,
		{500, 4, 100}},
  {"-copypix500", "Copyarea 500x500 square from pixmap",
		InitCopyArea2, DoCopyArea2, MidCopyArea2,
		EndCopyArea2, False, 0,
		{25, 4, 500}},
  {"-triangle1", "Fill 1-pixel/side equilateral triangle",
		InitTriangles, DoTriangles, NullProc, EndTriangles, 
                False, 0,
		{50, POLY, 1}},
  {"-triangle10", "Fill 10-pixel/side equilateral triangle",
		InitTriangles, DoTriangles, NullProc, EndTriangles, 
                False, 0,
		{50, POLY, 10}},
  {"-triangle100", "Fill 100-pixel/side equilateral triangle",
		InitTriangles, DoTriangles, NullProc, EndTriangles, 
                False, 0,
		{50, POLY/10, 100}},
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
  {"-atoms", "GetAtomName",
		NULL, DoAtom, NullProc, NullProc, False, 0,
		{5000, 1}},
  {"-props", "GetProperty",
		InitGetProp, DoGetProp, NullProc, NullProc, False, 0,
		{5000, 1}},
  {"-gc", "Graphics context validation",
		InitValGC, DoValGC, NullProc, EndValGC, False, 0,
		{125000, 4}},
  {"-create",   "Create and map subwindows",
		InitCreate, CreateChildren, DeleteChildren, EndCreate,
		True, 0,
		{0, 0, True}},
  {"-map",      "Map window by mapping parent",
		InitMap, MapParents, UnmapParents, EndCreate, True, 0,
		{0, 0, True}},
  {"-ucreate",  "Create Unmapped Window",
		InitCreate, CreateChildren, DeleteChildren, EndCreate,
		True, 0,
		{0, 0, False}},
  {"-popup",    "Popup Window",
		InitPopups, DoPopUps, NULL, EndPopups, True, 0,
		{6000, 0, True}},
  {"-move",     "Move Window",
		InitMoveWins, DoMoveWins, NullProc, EndMoveWins, True, 0,
		{1200, 0, True}},
  {"-umove",    "Moved Unmapped Window",
		InitMoveWins, DoMoveWins, NullProc, EndMoveWins, True, 0,
		{12000, 0, False}},
  {"-resize",   "Resize Window",
		InitMoveWins, DoResizeWins, NullProc, EndMoveWins, True, 0,
		{1200, 4, True}},
  {"-uresize",  "Resize Unmapped Window",
		InitMoveWins, DoResizeWins, NullProc, EndMoveWins, True, 0,
		{12000, 4, False}},
  {"-circulate", "Circulate Window",
		InitCircWins, DoCircWins, NullProc, EndMoveWins, True, 0,
		{1200, 4, True}},
  {"-ucirculate", "Circulate Unmapped Window",
		InitCircWins, DoCircWins, NullProc, EndMoveWins, True, 0,
		{12000, 4, False}},
  { NULL, NULL,
		NULL, NULL, NULL, NULL, False, 0,
		{0, 0, False, NULL, NULL}}
};
