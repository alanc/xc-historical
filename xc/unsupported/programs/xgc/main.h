XbenchData FunctionData[MAXCHOICES] = {
  {"clear",        "clear",        GXclear},
  {"and",          "and",          GXand},
  {"andReverse",   "andReverse",   GXandReverse},
  {"copy",         "copy",         GXcopy},
  {"andInverted",  "andInverted",  GXandInverted},
  {"noop",         "noop",         GXnoop},
  {"xor",          "xor",          GXxor},
  {"or",           "or",            GXor},
  {"nor",          "nor",          GXnor},
  {"equiv",        "equiv",        GXequiv},
  {"invert",       "invert",       GXinvert},
  {"orReverse",    "orReverse",    GXorReverse},
  {"copyInverted", "copyInverted", GXcopyInverted},
  {"orInverted",   "orInverted",   GXorInverted},
  {"nand",         "nand",         GXnand},
  {"set",          "set",          GXset}
}; 

XbenchStuff FunctionStuff = {
  {"Function","function",16,4},
  FunctionData
};

XbenchData TestData[MAXCHOICES] = {
  {"Copy Area",          "CopyArea",      CopyArea},
  {"Copy Plane",         "CopyPlane",     CopyPlane},
  {"Points",             "PolyPoint",     PolyPoint},
  {"Lines",              "PolyLine",      PolyLine},
  {"Segments",           "PolySegment",   PolySegment},
  {"Rectangles",         "PolyRectangle", PolyRectangle},
  {"Arcs",               "PolyArc",       PolyArc},
  {"(Filled Polygons)",  "FillPolygon",   FillPolygon},
  {"Filled Rectangles",  "PolyFillRect",  PolyFillRect},
  {"Filled Arcs",        "PolyFillArc",   PolyFillArc},
  {"Put Image",          "PutImage",      PutImage},
  {"(Get Image)",        "GetImage",      GetImage},
  {"Text 8",             "PolyText8",     PolyText8},
  {"Image Text 8",       "ImageText8",    ImageText8},
  {"(Text 16)",          "PolyText16",    PolyText16},
  {"(Image Text 16)",    "ImageText16",   ImageText16}
};

XbenchStuff TestStuff = {
  {"Test","test",16,2},
  TestData
};

XbenchData LinestyleData[MAXCHOICES] = {
  {"Solid",      "Solid",       LineSolid},
  {"OnOffDash",  "OnOffDash",   LineOnOffDash},
  {"DoubleDash", "DoubleDash",  LineDoubleDash}
};

XbenchStuff LinestyleStuff = {
  {"LineStyle","linestyle",3,0},
  LinestyleData
};

XbenchData CapstyleData[MAXCHOICES] = {
  {"NotLast",    "NotLast",     CapNotLast},
  {"Butt",       "Butt",        CapButt},
  {"Round",      "Round",       CapRound},
  {"Projecting", "Projecting",  CapProjecting}
};

XbenchStuff CapstyleStuff = {
  {"CapStyle","capstyle",4,2},
  CapstyleData
};

XbenchData JoinstyleData[MAXCHOICES] = {
  {"Miter",   "Miter",   JoinMiter},
  {"Round",   "Round",   JoinRound},
  {"Bevel",   "Bevel",   JoinBevel}
};

XbenchStuff JoinstyleStuff = {
  {"JoinStyle","joinstyle",3,0},
  JoinstyleData
};

XbenchData FillstyleData[MAXCHOICES] = {
  {"Solid",          "Solid",          FillSolid},
  {"Tiled",          "Tiled",          FillTiled},
  {"Stippled",       "Stippled",       FillStippled},
  {"OpaqueStippled", "OpaqueStippled", FillOpaqueStippled}
};

XbenchStuff FillstyleStuff = {
  {"FillStyle","fillstyle",4,2},
  FillstyleData
};

XbenchData FillruleData[MAXCHOICES] = {
  {"EvenOdd",  "EvenOdd",  EvenOddRule},
  {"Winding",  "Winding",  WindingRule}
};

XbenchStuff FillruleStuff = {
  {"FillRule","fillrule",2,0},
  FillruleData
};

XbenchData ArcmodeData[MAXCHOICES] = {
  {"Chord",    "Chord",    ArcChord},
 {"PieSlice", "PieSlice", ArcPieSlice}
};

XbenchStuff ArcmodeStuff = {
  {"ArcMode","arcmode",2,0},
  ArcmodeData
};

static XbenchStuff *Everything[8] = {
  &FunctionStuff,
  &LinestyleStuff,
  &CapstyleStuff,
  &JoinstyleStuff,
  &FillstyleStuff,
  &FillruleStuff,
  &ArcmodeStuff,
  &TestStuff
};
