#include <stdio.h>
#ifndef VMS
#include <X11/Xatom.h>
#include <sys/time.h>
#else
#include <decw$include/Xatom.h>
#endif
#include "x11perf.h"

extern void DoAtom();

extern void InitGetProp();
extern void DoGetProp();

extern void InitRects();
extern void DoRects();
extern void EndRects();

extern void InitValGC();
extern void DoValGC();
extern void EndValGC();

extern void InitSegs1();
extern void InitSegs10();
extern void InitSegs100();
extern void InitSegs500();
extern void DoSegs();
extern void EndSegs();

extern void InitLines1();
extern void InitLines10();
extern void InitLines100();
extern void InitLines500();
extern void InitDashedLines100();
extern void DoLines();
extern void EndLines();

extern void InitCircles1();
extern void InitCircles10();
extern void InitCircles100();
extern void InitCircles500();
extern void InitEllipses1();
extern void InitEllipses10();
extern void InitEllipses100();
extern void InitEllipses500();
extern void DoArcs();
extern void EndArcs();

extern void InitDots();
extern void DoDots();
extern void EndDots();

extern void InitWins();
extern void DoWins();
extern void DeleteSubs();
extern void EndWins();

extern void InitWins2();
extern void DoWins2();
extern void UnmapWin();

extern void InitMoveWins();
extern void DoMoveWins();
extern void EndMoveWins();

extern void InitCircWins();
extern void DoCircWins();

extern void InitText();
extern void DoText();
extern void DoImageText();
extern void DoPolyText();
extern void ClearTextWin();
extern void EndText();

extern void InitCopyPlane();
extern void DoCopyPlane();
extern void EndCopyPlane();

extern void InitPopups();
extern void DoPopUps();
extern void EndPopups();

extern void InitScrolling();
extern void DoScrolling();
extern void MidScroll();
extern void EndScrolling();

extern void InitCopyArea();
extern void DoCopyArea();
extern void MidCopyArea();
extern void EndCopyArea();

extern void InitCopyArea2();
extern void DoCopyArea2();
extern void MidCopyArea2();
extern void EndCopyArea2();

extern void InitTriangles();
extern void DoTriangles();
extern void EndTriangles();

extern void InitTraps();
extern void DoTraps();
extern void EndTraps();

extern void InitComplexPoly();
extern void DoComplexPoly();
extern void EndComplexPoly();

extern void InitTileWins();
extern void DoTileWins();
extern void EndTileWins();

/*
 * some test must be done a certain multiple of times. That multiple is
 * in the second half of the products below. You can edit the multiplier,
 * just not the multiplicand.
 */
int subs[] = {4, 16, 25, 50, 75, 100, 200, 0};

Test test[] = {
  {"-atoms", "GetAtomName",
		NullProc, DoAtom, NullProc, NullProc, False, False,
		{2000, 1}},
  {"-props", "GetProperty",
		InitGetProp, DoGetProp, NullProc, NullProc, False, False,
		{2000, 1}},
  {"-gc", "Graphics context validation",
		InitValGC, DoValGC, NullProc, EndValGC, False, False,
		{5000, 4}},
  {"-dots", "Dot",
		InitDots, DoDots, NullProc, EndDots, False, False,
		{500, POLY}},
  {"-smallrects", "1x1 Rectangle",
		InitRects, DoRects, NullProc, EndRects, False, False,
		{200, POLY, 1, NULL, NULL, FillSolid}},
  {"-rects", "10x10 Rectangle",
		InitRects, DoRects, NullProc, EndRects, False, False,
		{100, POLY, 10, NULL, NULL, FillSolid}},
  {"-bigrects", "50x50 Rectangle",
		InitRects, DoRects, NullProc, EndRects, False, False,
		{100, POLY/10, 50, NULL, NULL, FillSolid}},
  {"-smalltsrects", "1x1 Transparent Stippled Rectangle",
		InitRects, DoRects, NullProc, EndRects, False, False,
		{200, POLY, 1, NULL, NULL, FillStippled}},
  {"-tsrects", "10x10 Transparent Stippled Rectangle",
		InitRects, DoRects, NullProc, EndRects, False, False,
		{100, POLY, 10, NULL, NULL, FillStippled}},
  {"-bigtsrects", "50x50 Transparent Stippled Rectangle",
		InitRects, DoRects, NullProc, EndRects, False, False,
		{100, POLY/10, 50, NULL, NULL, FillStippled}},
  {"-smallosrects", "1x1 Opaque Stippled Rectangle",
		InitRects, DoRects, NullProc, EndRects, False, False,
		{100, POLY, 1, NULL, NULL, FillOpaqueStippled}},
  {"-osrects", "10x10 Opaque Stippled Rectangle",
		InitRects, DoRects, NullProc, EndRects, False, False,
		{50, POLY, 10, NULL, NULL, FillOpaqueStippled}},
  {"-bigosrects", "50x50 Opaque Stippled Rectangle",
		InitRects, DoRects, NullProc, EndRects, False, False,
		{50, POLY/10, 50, NULL, NULL, FillOpaqueStippled}},
  {"-smalltilerects", "1x1 4x4 Tiled Rectangle",
		InitRects, DoRects, NullProc, EndRects, False, False,
		{100, POLY, 1, NULL, NULL, FillTiled}},
  {"-tilerects", "10x10 4x4 Tiled Rectangle",
		InitRects, DoRects, NullProc, EndRects, False, False,
		{50, POLY, 10, NULL, NULL, FillTiled}},
  {"-bigtilerects", "50x50 4x4 Tiled Rectangle",
		InitRects, DoRects, NullProc, EndRects, False, False,
		{50, POLY/10, 50, NULL, NULL, FillTiled}},
  {"-segs1", "1-pixel line segment",
		InitSegs1, DoSegs, NullProc, EndSegs, False, False,
		{100, POLY, 0}},
  {"-segs10", "10-pixel line segment",
		InitSegs10, DoSegs, NullProc, EndSegs, False, False,
		{100, POLY, 0}},
  {"-segs100", "100-pixel line segment",
		InitSegs100, DoSegs, NullProc, EndSegs, False, False,
		{50, POLY, 0}},
  {"-segs500", "500-pixel line segment",
		InitSegs500, DoSegs, NullProc, EndSegs, False, False,
		{20, POLY, 0}},
  {"-segs100c1", "100-pixel line segment (1 occluding window)",
		InitSegs100, DoSegs, NullProc, EndSegs, False, False,
		{50, POLY, 1}},
  {"-segs100c2", "100-pixel line segment (2 occluding windows)",
		InitSegs100, DoSegs, NullProc, EndSegs, False, False,
		{50, POLY, 2}},
  {"-segs100c3", "100-pixel line segment (3 occluding windows)",
		InitSegs100, DoSegs, NullProc, EndSegs, False, False,
		{50, POLY, 3}},
  {"-lines1", "1-pixel line",
		InitLines1, DoLines, NullProc, EndLines, False, False,
		{100, POLY, 0}},
  {"-lines10", "10-pixel line",
		InitLines10, DoLines, NullProc, EndLines, False, False,
		{100, POLY, 0}},
  {"-lines100", "100-pixel line",
		InitLines100, DoLines, NullProc, EndLines, False, False,
		{50, POLY, 0}},
  {"-lines500", "500-pixel line",
		InitLines500, DoLines, NullProc, EndLines, False, False,
		{20, POLY, 0}},
  {"-dlines", "100-pixel dashed line",
		InitDashedLines100, DoLines, NullProc, EndLines, False, False,
		{10, POLY, 0}},
  {"-circles1", "1-pixel diameter circle",
		InitCircles1, DoArcs, NullProc, EndArcs, False, False,
		{20, POLY/2, 0}},
  {"-circles10", "10-pixel diameter circle",
		InitCircles10, DoArcs, NullProc, EndArcs, False, False,
		{20, POLY/4, 0}},
  {"-circles100", "100-pixel diameter circle",
		InitCircles100, DoArcs, NullProc, EndArcs, False, False,
		{20, POLY/40, 0}},
  {"-circles500", "500-pixel diameter circle",
		InitCircles500, DoArcs, NullProc, EndArcs, False, False,
		{20, POLY/100, 0}},
  {"-ellipses1", "1-pixel major-axis diameter ellipse",
		InitEllipses1, DoArcs, NullProc, EndArcs, False, False,
		{20, POLY/2, 0}},
  {"-ellipses10", "10-pixel major-axis diameter ellipse",
		InitEllipses10, DoArcs, NullProc, EndArcs, False, False,
		{20, POLY/4, 0}},
  {"-ellipses100", "100-pixel major-axis diameter ellipse",
		InitEllipses100, DoArcs, NullProc, EndArcs, False, False,
		{20, POLY/40, 0}},
  {"-ellipses500", "500-pixel major-axis diameter ellipse",
		InitEllipses500, DoArcs, NullProc, EndArcs, False, False,
		{10, POLY/100, 0}},
  {"-ftext", "100-character line/'fixed'",
		InitText, DoText, ClearTextWin, EndText, False, False,
		{100, 1, False, "fixed", NULL}},
  {"-tr10text", "100 character line/'Times Roman 10'",
		InitText, DoText, ClearTextWin, EndText, False, False,
		{100, 1, False, "times_roman10", NULL}},
  {"-tr24text", "100 character line/'Times Roman 24'",
		InitText, DoText, ClearTextWin, EndText, False, False,
		{100, 1, False, "times_roman24", NULL}},
  {"-fitext", "100 character image line/'fixed'",
		InitText, DoImageText, ClearTextWin, EndText, False, False,
		{100, 1, False, "fixed", NULL}},
  {"-tr10itext", "100 character image line/'Times Roman 10'",
		InitText, DoImageText, ClearTextWin, EndText, False, False,
		{100, 1, False, "times_roman10", NULL}},
  {"-tr24itext", "100 character image line/'Times Roman 24'",
		InitText, DoImageText, ClearTextWin, EndText, False, False,
		{100, 1, False, "times_roman24", NULL}},
  {"-polytext1", "poly text/'8x13', '8x13bold'",
		InitText, DoPolyText, ClearTextWin, EndText, False, False,
		{100, 1, True, "8x13", "8x13bold"}},
  {"-polytext2", "poly text/'6x10', '9x15'",
		InitText, DoPolyText, ClearTextWin, EndText, False, False,
		{100, 1, True, "6x10", "9x15"}},
  {"-polytext3", "poly text/'times_roman24', 'times_bold24'",
		InitText, DoPolyText, ClearTextWin, EndText, False, False,
		{100, 1, True, "times_roman24", "times_bold24"}},
  {"-copyplane", "Copy 1-bit plane",
		InitCopyPlane, DoCopyPlane, NULL, EndCopyPlane, False, False,
		{50, 1, False, "fixed", NULL}},
  {"-scroll", "Scrolling",
		InitScrolling, DoScrolling, MidScroll,
		EndScrolling, False, False,
		{50, 1, False}},
  {"-copyarea", "Copyarea",
		InitCopyArea, DoCopyArea, MidCopyArea,
		EndCopyArea, False, False,
		{200, 1, False}},
  {"-copyarea2", "Copyarea from pixmap",
		InitCopyArea2, DoCopyArea2, MidCopyArea2,
		EndCopyArea2, False, False,
		{200, 1, False}},
  {"-triangle", "Fill Polygon, triangles",
		InitTriangles, DoTriangles, NullProc, EndTriangles, 
                False, False,
		{10, 100, 0}},
  {"-trap", "Fill Polygon, trapezoids",
		InitTraps, DoTraps, NullProc, EndTraps, False, False,
		{10, 100, 0}},
  {"-complex", "Fill Random Complex Polygons",
		InitComplexPoly, DoComplexPoly, NullProc, EndComplexPoly, 
                False, False,
		{5, 50, 0}},
  {"-tile", 	"Fill 4x4 Tiled windows",
		InitTileWins, DoTileWins, NullProc, EndTileWins, 
                False, False,
		{100, 0, 0}},
  {"-windows", "Create Window, MapSubwindows",
		InitWins, DoWins, DeleteSubs, EndWins, True, False,
		{1, 4, True}},
  {"-windows2", "Create Window, Map Parent",
		InitWins2, DoWins2, UnmapWin, EndWins, True, False,
		{1, 4, True}},
  {"-uwindows", "Create Unmapped Window",
		InitWins, DoWins, DeleteSubs, EndWins, True, False,
		{1, 4, False}},
  {"-move", "Move Window",
		InitMoveWins, DoMoveWins, NullProc, EndMoveWins, True, False,
		{400, 4, True}},
  {"-umove", "Moved Unmapped Window",
		InitMoveWins, DoMoveWins, NullProc, EndMoveWins, True, False,
		{400, 4, False}},
  {"-circulate", "Circulate Window",
		InitCircWins, DoCircWins, NullProc, EndMoveWins, True, False,
		{400, 4, True}},
  {"-ucirculate", "Circulate Unmapped Window",
		InitCircWins, DoCircWins, NullProc, EndMoveWins, True, False,
		{400, 4, False}},
  {"-popup", "Popup Window",
		InitPopups, DoPopUps, NULL, EndPopups, True, False,
		{10, 4, True}},
  { NULL, NULL,
		NULL, NULL, NULL, NULL, False, False,
		{0, 0, False, NULL, NULL}}
};
