/* Copyright International Business Machines,Corp. 1991
 * All Rights Reserved
 *
 * License, subject to the license given below, to use,
 * copy, modify, and distribute this software * and its
 * documentation for any purpose and without fee is hereby
 * granted, provided that the above copyright notice appear
 * in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation,
 * and that the name of IBM not be used in advertising or
 * publicity pertaining to distribution of the software
 * without specific, written prior permission.
 *
 * IBM PROVIDES THIS SOFTWARE "AS IS", WITHOUT ANY WARRANTIES
 * OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT
 * LIMITED TO ANY IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS.  THE ENTIRE RISK AS TO THE QUALITY AND
 * PERFORMANCE OF THE SOFTWARE, INCLUDING ANY DUTY TO SUPPORT
 * OR MAINTAIN, BELONGS TO THE LICENSEE.  SHOULD ANY PORTION OF
 * THE SOFTWARE PROVE DEFECTIVE, THE LICENSEE (NOT IBM) ASSUMES
 * THE ENTIRE COST OF ALL SERVICING, REPAIR AND CORRECTION.  IN
 * NO EVENT SHALL IBM BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
 * FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
 * Author: Jeffrey B. Lotspiech, IBM Almaden Research Center
 *   Modeled on spfuncs.c by Dave Lemke, Network Computing Devices, Inc
 *   which contains the following copyright and permission notices:
 *
 * Copyright 1990, 1991 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation and the
 * Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this protoype software
 * and its documentation to Members and Affiliates of the MIT X Consortium
 * any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Network Computing Devices, Digital or
 * MIT not be used in advertising or publicity pertaining to distribution of
 * the software without specific, written prior permission.
 *
 * NETWORK COMPUTING DEVICES, DIGITAL AND MIT DISCLAIM ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL NETWORK COMPUTING DEVICES, DIGITAL OR MIT BE
 * LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
 
#include    <string.h>
#include    "fontfilest.h"
#include    "FSproto.h"
#include    "t1intf.h"
 
#include "objects.h"
#include "spaces.h"
#include "regions.h"
 
int         Type1OpenScalable ();
static int  Type1GetGlyphs();
extern int  GenericGetExtents();
void        Type1CloseFont();
extern int  Type1GetInfoScalable ();
extern int  GenericGetBitmaps();
 
static int  Type1GetMetrics ();
 
static void fillrun();
 
char *ISO8859[] = {
 /* 32*/ "|space|",
 /* 33*/ "|exclam|",
 /* 34*/ "|quotedbl|",
 /* 35*/ "|numbersign|",
 /* 36*/ "|dollar|",
 /* 37*/ "|percent|",
 /* 38*/ "|ampersand|",
 /* 39*/ "|quoteright|",
 /* 40*/ "|parenleft|",
 /* 41*/ "|parenright|",
 /* 42*/ "|asterisk|",
 /* 43*/ "|plus|",
 /* 44*/ "|comma|",
 /* 45*/ "|minus|",
 /* 46*/ "|period|",
 /* 47*/ "|slash|",
 /* 48*/ "|zero|",
 /* 49*/ "|one|",
 /* 50*/ "|two|",
 /* 51*/ "|three|",
 /* 52*/ "|four|",
 /* 53*/ "|five|",
 /* 54*/ "|six|",
 /* 55*/ "|seven|",
 /* 56*/ "|eight|",
 /* 57*/ "|nine|",
 /* 58*/ "|colon|",
 /* 59*/ "|semicolon|",
 /* 60*/ "|less|",
 /* 61*/ "|equal|",
 /* 62*/ "|greater|",
 /* 63*/ "|question|",
 /* 64*/ "|at|",
 /* 65*/ "A",
 /* 66*/ "B",
 /* 67*/ "C",
 /* 68*/ "D",
 /* 69*/ "E",
 /* 70*/ "F",
 /* 71*/ "G",
 /* 72*/ "H",
 /* 73*/ "I",
 /* 74*/ "J",
 /* 75*/ "K",
 /* 76*/ "L",
 /* 77*/ "M",
 /* 78*/ "N",
 /* 79*/ "O",
 /* 80*/ "P",
 /* 81*/ "Q",
 /* 82*/ "R",
 /* 83*/ "S",
 /* 84*/ "T",
 /* 85*/ "U",
 /* 86*/ "V",
 /* 87*/ "W",
 /* 88*/ "X",
 /* 89*/ "Y",
 /* 90*/ "Z",
 /* 91*/ "|bracketleft|",
 /* 92*/ "|backslash|",
 /* 93*/ "|bracketright|",
 /* 94*/ "|asciicircum|",
 /* 95*/ "|underscore|",
 /* 96*/ "|quoteleft|",
 /* 97*/ "a",
 /* 98*/ "b",
 /* 99*/ "c",
 /*100*/ "d",
 /*101*/ "e",
 /*102*/ "f",
 /*103*/ "g",
 /*104*/ "h",
 /*105*/ "i",
 /*106*/ "j",
 /*107*/ "k",
 /*108*/ "l",
 /*109*/ "m",
 /*110*/ "n",
 /*111*/ "o",
 /*112*/ "p",
 /*113*/ "q",
 /*114*/ "r",
 /*115*/ "s",
 /*116*/ "t",
 /*117*/ "u",
 /*118*/ "v",
 /*119*/ "w",
 /*120*/ "x",
 /*121*/ "y",
 /*122*/ "z",
 /*123*/ "|braceleft|",
 /*124*/ "|bar|",
 /*125*/ "|braceright|",
 /*126*/ "|asciitilde|",
 /*127*/ NULL,
 /*128*/ NULL,
 /*129*/ NULL,
 /*130*/ NULL,
 /*131*/ NULL,
 /*132*/ NULL,
 /*133*/ NULL,
 /*134*/ NULL,
 /*135*/ NULL,
 /*136*/ NULL,
 /*137*/ NULL,
 /*138*/ NULL,
 /*139*/ NULL,
 /*140*/ NULL,
 /*141*/ NULL,
 /*142*/ NULL,
 /*143*/ NULL,
 /*144*/ NULL,
 /*145*/ NULL,
 /*146*/ NULL,
 /*147*/ NULL,
 /*148*/ NULL,
 /*149*/ NULL,
 /*150*/ NULL,
 /*151*/ NULL,
 /*152*/ NULL,
 /*153*/ NULL,
 /*154*/ NULL,
 /*155*/ NULL,
 /*156*/ NULL,
 /*157*/ NULL,
 /*158*/ NULL,
 /*159*/ NULL,
 /*160*/ NULL,
 /*161*/ "|exclamdown|",
 /*162*/ "|cent|",
 /*163*/ "|sterling|",
 /*164*/ "|currency|",
 /*165*/ "|yen|",
 /*166*/ "|brokenbar|",
 /*167*/ "|section|",
 /*168*/ "|dieresis|",
 /*169*/ "|copyright|",
 /*170*/ "|ordfeminine|",
 /*171*/ "|guillemotleft|",
 /*172*/ "|logicalnot|",
 /*173*/ "|hyphen|",
 /*174*/ "|registered|",
 /*175*/ "|macron|",
 /*176*/ "|degree|",
 /*177*/ "|plusminus|",
 /*178*/ "|twosuperior|",
 /*179*/ "|threesuperior|",
 /*180*/ "|acute|",
 /*181*/ "|mu|",
 /*182*/ "|paragraph|",
 /*183*/ "|periodcentered|",
 /*184*/ "|cedilla|",
 /*185*/ "|onesuperior|",
 /*186*/ "|ordmasculine|",
 /*187*/ "|guillemotright|",
 /*188*/ "|onequarter|",
 /*189*/ "|onehalf|",
 /*190*/ "|threequarters|",
 /*191*/ "|questiondown|",
 /*192*/ "|Agrave|",
 /*193*/ "|Aacute|",
 /*194*/ "|Acircumflex|",
 /*195*/ "|Atilde|",
 /*196*/ "|Adieresis|",
 /*197*/ "|Aring|",
 /*198*/ "|AE|",
 /*199*/ "|Ccedilla|",
 /*200*/ "|Egrave|",
 /*201*/ "|Eacute|",
 /*202*/ "|Ecircumflex|",
 /*203*/ "|Edieresis|",
 /*204*/ "|Igrave|",
 /*205*/ "|Iacute|",
 /*206*/ "|Icircumflex|",
 /*207*/ "|Idieresis|",
 /*208*/ "|Eth|",
 /*209*/ "|Ntilde|",
 /*210*/ "|Ograve|",
 /*211*/ "|Oacute|",
 /*212*/ "|Ocircumflex|",
 /*213*/ "|Otilde|",
 /*214*/ "|Odieresis|",
 /*215*/ "|multiply|",
 /*216*/ "|Oslash|",
 /*217*/ "|Ugrave|",
 /*218*/ "|Uacute|",
 /*219*/ "|Ucircumflex|",
 /*220*/ "|Udieresis|",
 /*221*/ "|Yacute|",
 /*222*/ "|Thorn|",
 /*223*/ "|germandbls|",
 /*224*/ "|agrave|",
 /*225*/ "|aacute|",
 /*226*/ "|acircumflex|",
 /*227*/ "|atilde|",
 /*228*/ "|adieresis|",
 /*229*/ "|aring|",
 /*230*/ "|ae|",
 /*231*/ "|ccedilla|",
 /*232*/ "|egrave|",
 /*233*/ "|eacute|",
 /*234*/ "|ecircumflex|",
 /*235*/ "|edieresis|",
 /*236*/ "|igrave|",
 /*237*/ "|iacute|",
 /*238*/ "|icircumflex|",
 /*239*/ "|idieresis|",
 /*240*/ "|eth|",
 /*241*/ "|ntilde|",
 /*242*/ "|ograve|",
 /*243*/ "|oacute|",
 /*244*/ "|ocircumflex|",
 /*245*/ "|otilde|",
 /*246*/ "|odieresis|",
 /*247*/ "|divide|",
 /*248*/ "|oslash|",
 /*249*/ "|ugrave|",
 /*250*/ "|uacute|",
 /*251*/ "|ucircumflex|",
 /*252*/ "|udieresis|",
 /*253*/ "|yacute|",
 /*254*/ "|thorn|",
 /*255*/ "|ydieresis|"
};
 
char *SYMBOL[] = {
/*   32  */ "|space|",
/*   33  */ "|exclam|",
/*   34  */ "|universal|",
/*   35  */ "|numbersign|",
/*   36  */ "|existential|",
/*   37  */ "|percent|",
/*   38  */ "|ampersand|",
/*   39  */ "|suchthat|",
/*   40  */ "|parenleft|",
/*   41  */ "|parenright|",
/*   42  */ "|asteriskmath|",
/*   43  */ "|plus|",
/*   44  */ "|comma|",
/*   45  */ "|minus|",
/*   46  */ "|period|",
/*   47  */ "|slash|",
/*   48  */ "|zero|",
/*   49  */ "|one|",
/*   50  */ "|two|",
/*   51  */ "|three|",
/*   52  */ "|four|",
/*   53  */ "|five|",
/*   54  */ "|six|",
/*   55  */ "|seven|",
/*   56  */ "|eight|",
/*   57  */ "|nine|",
/*   58  */ "|colon|",
/*   59  */ "|semicolon|",
/*   60  */ "|less|",
/*   61  */ "|equal|",
/*   62  */ "|greater|",
/*   63  */ "|question|",
/*   64  */ "|congruent|",
/*   65  */ "|Alpha|",
/*   66  */ "|Beta|",
/*   67  */ "|Chi|",
/*   68  */ "|Delta|",
/*   69  */ "|Epsilon|",
/*   70  */ "|Phi|",
/*   71  */ "|Gamma|",
/*   72  */ "|Eta|",
/*   73  */ "|Iota|",
/*   74  */ "|theta1|",
/*   75  */ "|Kappa|",
/*   76  */ "|Lambda|",
/*   77  */ "|Mu|",
/*   78  */ "|Nu|",
/*   79  */ "|Omicron|",
/*   80  */ "|Pi|",
/*   81  */ "|Theta|",
/*   82  */ "|Rho|",
/*   83  */ "|Sigma|",
/*   84  */ "|Tau|",
/*   85  */ "|Upsilon|",
/*   86  */ "|sigma1|",
/*   87  */ "|Omega|",
/*   88  */ "|Xi|",
/*   89  */ "|Psi|",
/*   90  */ "|Zeta|",
/*   91  */ "|bracketleft|",
/*   92  */ "|therefore|",
/*   93  */ "|bracketright|",
/*   94  */ "|perpendicular|",
/*   95  */ "|underscore|",
/*   96  */ "|radicalex|",
/*   97  */ "|alpha|",
/*   98  */ "|beta|",
/*   99  */ "|chi|",
/*  100  */ "|delta|",
/*  101  */ "|epsilon|",
/*  102  */ "|phi|",
/*  103  */ "|gamma|",
/*  104  */ "|eta|",
/*  105  */ "|iota|",
/*  106  */ "|phi1|",
/*  107  */ "|kappa|",
/*  108  */ "|lambda|",
/*  109  */ "|mu|",
/*  110  */ "|nu|",
/*  111  */ "|omicron|",
/*  112  */ "|pi|",
/*  113  */ "|theta|",
/*  114  */ "|rho|",
/*  115  */ "|sigma|",
/*  116  */ "|tau|",
/*  117  */ "|upsilon|",
/*  118  */ "|omega1|",
/*  119  */ "|omega|",
/*  120  */ "|xi|",
/*  121  */ "|psi|",
/*  122  */ "|zeta|",
/*  123  */ "|braceleft|",
/*  124  */ "|bar|",
/*  125  */ "|braceright|",
/*  126  */ "|similar|",
 /*127*/ NULL,
 /*128*/ NULL,
 /*129*/ NULL,
 /*130*/ NULL,
 /*131*/ NULL,
 /*132*/ NULL,
 /*133*/ NULL,
 /*134*/ NULL,
 /*135*/ NULL,
 /*136*/ NULL,
 /*137*/ NULL,
 /*138*/ NULL,
 /*139*/ NULL,
 /*140*/ NULL,
 /*141*/ NULL,
 /*142*/ NULL,
 /*143*/ NULL,
 /*144*/ NULL,
 /*145*/ NULL,
 /*146*/ NULL,
 /*147*/ NULL,
 /*148*/ NULL,
 /*149*/ NULL,
 /*150*/ NULL,
 /*151*/ NULL,
 /*152*/ NULL,
 /*153*/ NULL,
 /*154*/ NULL,
 /*155*/ NULL,
 /*156*/ NULL,
 /*157*/ NULL,
 /*158*/ NULL,
 /*159*/ NULL,
 /*160*/ NULL,
/*  161  */ "|Upsilon1|",
/*  162  */ "|minute|",
/*  163  */ "|lessequal|",
/*  164  */ "|fraction|",
/*  165  */ "|infinity|",
/*  166  */ "|florin|",
/*  167  */ "|club|",
/*  168  */ "|diamond|",
/*  169  */ "|heart|",
/*  170  */ "|spade|",
/*  171  */ "|arrowboth|",
/*  172  */ "|arrowleft|",
/*  173  */ "|arrowup|",
/*  174  */ "|arrowright|",
/*  175  */ "|arrowdown|",
/*  176  */ "|degree|",
/*  177  */ "|plusminus|",
/*  178  */ "|second|",
/*  179  */ "|greaterequal|",
/*  180  */ "|multiply|",
/*  181  */ "|proportional|",
/*  182  */ "|partialdiff|",
/*  183  */ "|bullet|",
/*  184  */ "|divide|",
/*  185  */ "|notequal|",
/*  186  */ "|equivalence|",
/*  187  */ "|approxequal|",
/*  188  */ "|ellipsis|",
/*  189  */ "|arrowvertex|",
/*  190  */ "|arrowhorizex|",
/*  191  */ "|carriagereturn|",
/*  192  */ "|aleph|",
/*  193  */ "|Ifraktur|",
/*  194  */ "|Rfraktur|",
/*  195  */ "|weierstrass|",
/*  196  */ "|circlemultiply|",
/*  197  */ "|circleplus|",
/*  198  */ "|emptyset|",
/*  199  */ "|intersection|",
/*  200  */ "|union|",
/*  201  */ "|propersuperset|",
/*  202  */ "|reflexsuperset|",
/*  203  */ "|notsubset|",
/*  204  */ "|propersubset|",
/*  205  */ "|reflexsubset|",
/*  206  */ "|element|",
/*  207  */ "|notelement|",
/*  208  */ "|angle|",
/*  209  */ "|gradient|",
/*  210  */ "|registerserif|",
/*  211  */ "|copyrightserif|",
/*  212  */ "|trademarkserif|",
/*  213  */ "|product|",
/*  214  */ "|radical|",
/*  215  */ "|dotmath|",
/*  216  */ "|logicalnot|",
/*  217  */ "|logicaland|",
/*  218  */ "|logicalor|",
/*  219  */ "|arrowdblboth|",
/*  220  */ "|arrowdblleft|",
/*  221  */ "|arrowdblup|",
/*  222  */ "|arrowdblright|",
/*  223  */ "|arrowdbldown|",
/*  224  */ "|lozenge|",
/*  225  */ "|angleleft|",
/*  226  */ "|registersans|",
/*  227  */ "|copyrightsans|",
/*  228  */ "|trademarksans|",
/*  229  */ "|summation|",
/*  230  */ "|parenlefttp|",
/*  231  */ "|parenleftex|",
/*  232  */ "|parenleftbt|",
/*  233  */ "|bracketlefttp|",
/*  234  */ "|bracketleftex|",
/*  235  */ "|bracketleftbt|",
/*  236  */ "|bracelefttp|",
/*  237  */ "|braceleftmid|",
/*  238  */ "|braceleftbt|",
/*  239  */ "|braceex|",
/*  240  */ "|apple|",
/*  241  */ "|angleright|",
/*  242  */ "|integral|",
/*  243  */ "|integraltp|",
/*  244  */ "|integralex|",
/*  245  */ "|integralbt|",
/*  246  */ "|parenrighttp|",
/*  247  */ "|parenrightex|",
/*  248  */ "|parenrightbt|",
/*  249  */ "|bracketrighttp|",
/*  250  */ "|bracketrightex|",
/*  251  */ "|bracketrightbt|",
/*  252  */ "|bracerighttp|",
/*  253  */ "|bracerightmid|",
/*  254  */ "|bracerightbt|",
/*  255  */ NULL
};
 
 
extern pointer Xalloc();
static void fill();
static void clearmemory();
 
int Type1OpenScalable (fpe, ppFont, flags, entry, fileName, vals, format, fmask)
    FontPathElementPtr  fpe;
    FontPtr             *ppFont;
    int                 flags;
    FontEntryPtr        entry;
    char                *fileName;
    FontScalablePtr     vals;
    fsBitmapFormat      format;
    fsBitmapFormatMask  fmask;
{
       extern struct XYspace *IDENTITY;
       extern struct region *fontfcnA();
 
 
       FontPtr     pFont;
       int         bit,
                   byte,
                   glyph,
                   scan,
                   image;
       int pad,wordsize;     /* scan & image in bits                         */
       void *pool;           /* memory pool for ximager objects              */
       int size;             /* for memory size calculations                 */
       struct XYspace *S;    /* coordinate space for character               */
       struct region *area;
       double scale;         /* scale factor for font                        */
       CharInfoRec *glyphs;
       register int i;
       int len,rc;
       struct type1font *type1;
       char *p;
       char **codepage;
 
       /* set up default values */
       FontDefaultFormat(&bit, &byte, &glyph, &scan);
       /* get any changes made from above */
       rc = CheckFSFormat(format, fmask, &bit, &byte, &scan, &glyph, &image);
       if (rc != Successful)
               return rc;
 
       pad                = glyph * 8;
       wordsize           = scan * 8;
 
#define  PAD(bits, pad)  (((bits)+(pad)-1)&-(pad))
 
       pFont = (FontPtr) xalloc(sizeof(FontRec));
       if (pFont == NULL)
           return AllocError;
 
       type1 = (struct type1font *)xalloc(sizeof(struct type1font));
       if (type1 == NULL) {
               xfree(pFont);
               return AllocError;
       }
       clearmemory(type1, sizeof(struct type1font));
 
       /* heuristic for "maximum" size of pool we'll need: */
       size = 20000 + 12 * vals->pixel * sizeof(short);
       if (size < 0 || NULL == (pool = xalloc(size))) {
               xfree(type1);
               xfree(pFont);
               return AllocError;
       }
 
       addmemory(pool, size);
 
       scale = (double) vals->pixel;
       S = Permanent(Scale(IDENTITY, scale, - scale));
 
       glyphs = type1->glyphs;
 
       for (p = entry->name.name, i = 13; --i >= 0 && p != NULL; p = strchr(p + 1, '-')) { ; }
 
       if (p != NULL && !strncmp(p, "-ADOBE-FONTSPECIFIC", 19))
               codepage = SYMBOL;
       else
               codepage = ISO8859;
 
 
       for (i=0; i < 256-FIRSTCOL; i++) {
               long h,w;
               long paddedW;
 
               if (codepage[i] == NULL)
                       continue;
               len = strlen(codepage[i]);
               rc = 0;
               area = fontfcnA(fileName, S, codepage[i], &len, &rc);
               if (rc < 0) {
                       rc = BadFontName;
                       break;
               }
               else if (rc > 0)
                       continue;
 
               if (area == NULL)
                       continue;
 
               h       = area->ymax - area->ymin;
               w       = area->xmax - area->xmin;
               paddedW = PAD(w, pad);
 
               if (h > 0 && w > 0) {
                       size = h * paddedW / 8;
                       glyphs[i].bits = (char *)xalloc(size);
                       if (glyphs[i].bits == NULL) {
                               rc = AllocError;
                               break;
                       }
               }
               else {
                       h = w = 0;
                       area->xmin = area->xmax = 0;
                       area->ymax = area->ymax = 0;
               }
 
               glyphs[i].metrics.leftSideBearing  = area->xmin;
               glyphs[i].metrics.characterWidth   = NEARESTPEL(area->ending.x - area->origin.x);
               glyphs[i].metrics.rightSideBearing = w + area->xmin;
               glyphs[i].metrics.descent          = area->ymax - NEARESTPEL(area->origin.x);
               glyphs[i].metrics.ascent           = h - glyphs[i].metrics.descent;
 
               if (h > 0 && w > 0) {
                       clearmemory(glyphs[i].bits, size);
                       fill(glyphs[i].bits, h, paddedW, area, byte, bit, wordsize );
               }
 
               Destroy(area);
       }
 
       delmemory();
       xfree(pool);
 
       if (i != 256 - FIRSTCOL) {
               for (i--; i >= 0; i--)
                       if (glyphs[i].bits != NULL)
                               xfree(glyphs[i].bits);
               xfree(type1);
               xfree(pFont);
               return rc;
       }
       type1->pDefault    = NULL;
 
       pFont->format      = format;
 
       pFont->bit         = bit;
       pFont->byte        = byte;
       pFont->glyph       = glyph;
       pFont->scan        = scan;
 
       pFont->info.firstCol = FIRSTCOL;
       pFont->info.lastCol  = 255;
       pFont->info.firstRow = 0;
       pFont->info.lastRow  = 0;
 
       pFont->get_bitmaps = GenericGetBitmaps;
       pFont->get_metrics = Type1GetMetrics;
       pFont->get_glyphs  = Type1GetGlyphs;
       pFont->get_extents = GenericGetExtents;
       pFont->unload_font = Type1CloseFont;
       pFont->refcnt = 0;
       pFont->maxPrivate = -1;
       pFont->devPrivates = 0;
 
       pFont->fontPrivate = (pointer) type1;
 
       T1FillFontInfo(pFont, vals, fileName, entry->name.name);
 
       *ppFont = pFont;
       return Successful;
}
 
static int
Type1GetGlyphs(pFont, count, chars, charEncoding, glyphCount, glyphs)
    FontPtr     pFont;
    unsigned long count;
    register unsigned char *chars;
    FontEncoding charEncoding;
    unsigned long *glyphCount;  /* RETURN */
    CharInfoPtr *glyphs;        /* RETURN */
{
    unsigned int firstRow;
    unsigned int numRows;
    CharInfoPtr *glyphsBase;
    register unsigned int c;
    register CharInfoPtr pci;
    unsigned int r;
    CharInfoPtr pDefault;
    register struct type1font *type1Font;
    register int firstCol;
 
    type1Font  = (struct type1font *) pFont->fontPrivate;
    firstCol   = pFont->info.firstCol;
    pDefault   = type1Font->pDefault;
    glyphsBase = glyphs;
 
    switch (charEncoding) {
 
    case Linear8Bit:
    case TwoD8Bit:
        if (pFont->info.firstRow > 0)
            break;
        while (count--) {
                c = (*chars++);
                if (c >= firstCol &&
                       (pci = &type1Font->glyphs[c-firstCol])->metrics.characterWidth != 0)
                    *glyphs++ = pci;
                else if (pDefault)
                    *glyphs++ = pDefault;
        }
        break;
    case Linear16Bit:
        while (count--) {
                c = *chars++ << 8;
                c = (c | *chars++);
                if (c < 256 && c >= firstCol &&
                        (pci = &type1Font->glyphs[c-firstCol])->metrics.characterWidth != 0)
                    *glyphs++ = pci;
                else if (pDefault)
                    *glyphs++ = pDefault;
        }
        break;
 
    case TwoD16Bit:
        firstRow = pFont->info.firstRow;
        numRows = pFont->info.lastRow - firstRow + 1;
        while (count--) {
            r = (*chars++) - firstRow;
            c = (*chars++);
            if (r < numRows && c < 256 && c >= firstCol &&
                    (pci = &type1Font->glyphs[(r << 8) + c - firstCol])->metrics.characterWidth != 0)
                *glyphs++ = pci;
            else if (pDefault)
                *glyphs++ = pDefault;
        }
        break;
    }
    *glyphCount = glyphs - glyphsBase;
    return Successful;
}
 
static int
Type1GetMetrics(pFont, count, chars, charEncoding, glyphCount, glyphs)
    FontPtr     pFont;
    unsigned long count;
    register unsigned char *chars;
    FontEncoding charEncoding;
    unsigned long *glyphCount;  /* RETURN */
    xCharInfo **glyphs;         /* RETURN */
{
    static CharInfoRec nonExistantChar;
 
    int         ret;
    struct type1font *type1Font;
    CharInfoPtr oldDefault;
 
    type1Font = (struct type1font *) pFont->fontPrivate;
    oldDefault = type1Font->pDefault;
    type1Font->pDefault = &nonExistantChar;
    ret = Type1GetGlyphs(pFont, count, chars, charEncoding, glyphCount, (CharInfoPtr *) glyphs);
    type1Font->pDefault = oldDefault;
    return ret;
}
 
void Type1CloseFont(pFont)
       FontPtr pFont;
{
       register int i;
       struct type1font *type1;
 
       type1 = (struct type1font *) pFont->fontPrivate;
       for (i=0; i < 256 - pFont->info.firstCol; i++)
               if (type1->glyphs[i].bits != NULL)
                        xfree(type1->glyphs[i].bits);
       xfree(type1);
       xfree(pFont->info.props);
       xfree(pFont->info.isStringProp);
       xfree(pFont);
}
 
 
 
static void clearmemory(addr, size)
       long *addr;
       int size;
{
       while (0 <= (size -= sizeof(long)))
               *addr++ = 0;
}
 
static void fill(dest, h, w, area, byte, bit, wordsize)
       register char *dest;  /* destination bitmap                           */
       int h,w;              /* dimensions of 'dest', w padded               */
       register struct region *area;  /* region to write to 'dest'           */
       int byte,bit;         /* flags; LSBFirst or MSBFirst                  */
       int wordsize;         /* number of bits per word for LSB/MSB purposes */
{
       register struct edgelist *edge;  /* for looping through edges         */
       register char *p;     /* current scan line in 'dest'                  */
       register int y;       /* for looping through scans                    */
       register int wbytes = w / 8;  /* number of bytes in width             */
       register pel *leftP,*rightP;  /* pointers to X values, left and right */
       int xmin = area->xmin;  /* upper left X                               */
       int ymin = area->ymin;  /* upper left Y                               */
 
       for (edge = area->anchor; VALIDEDGE(edge); edge = edge->link->link) {
 
               p = dest + (edge->ymin - ymin) * wbytes;
               leftP = edge->xvalues;
               rightP = edge->link->xvalues;
 
               for (y = edge->ymin; y < edge->ymax; y++) {
                       fillrun(p, *leftP++ - xmin, *rightP++ - xmin, bit);
                       p += wbytes;
               }
       }
/*
Now, as an afterthought, we'll go reorganize if odd byte order requires
it:
*/
       if (byte == LSBFirst && wordsize != 8) {
               register int i;
 
               switch (wordsize) {
                   case 16:
                   {
                       register unsigned short data,*p;
 
                       p = (unsigned short *) dest;
 
                       for (i = h * w /16; --i >= 0;) {
                               data = *p;
                               *p++ = (data << 8) + (data >> 8);
                       }
                       break;
                   }
                   case 64:
                   case 32:
                   {
                       register unsigned long data,*p;
 
                       p = (unsigned long *) dest;
 
                       for (i = h * w / 32; --i >= 0;) {
                               data = *p;
                               *p++ = (data << 24) + (data >> 24)
                                      + (0xFF00 & (data >> 8))
                                      + (0xFF0000 & (data << 8));
                       }
                       if (wordsize == 64) {
 
                               p = (unsigned long *) dest;
 
                               for (i = h * w / 64; --i >= 0;) {
                                       data = *p++;
                                       p[-1] = p[0];
                                       *p++ = data;
                               }
                       }
                       break;
                   }
                   default:
                       abort("xiFill: unknown format");
               }
       }
 
}
 
#define  ALLONES  0xFF
 
static void fillrun(p, x0, x1, bit)
       register char *p;     /* address of this scan line                    */
       pel x0,x1;            /* left and right X                             */
       int bit;              /* format:  LSBFirst or MSBFirst                */
{
       register int startmask,endmask;  /* bits to set in first and last char*/
       register int middle;  /* number of chars between start and end + 1    */
 
       if (x1 <= x0)
               return;
       middle = x1/8 - x0/8;
       p += x0/8;
       x0 &= 7;  x1 &= 7;
       if (bit == LSBFirst) {
               startmask = ALLONES << x0;
               endmask = ~(ALLONES << x1);
       }
       else {
               startmask = ALLONES >> x0;
               endmask = ~(ALLONES >> x1);
       }
       if (middle == 0)
               *p++ |= startmask & endmask;
       else {
               *p++ |= startmask;
               while (--middle > 0)
                       *p++ = ALLONES;
               *p |= endmask;
       }
}
 
 
static FontRendererRec renderers[] = {
  { ".pfa", 4, (int (*)()) 0, Type1OpenScalable,
        (int (*)()) 0, Type1GetInfoScalable, 0 },
  { ".pfb", 4, (int (*)()) 0, Type1OpenScalable,
        (int (*)()) 0, Type1GetInfoScalable, 0 }
};
 
Type1RegisterFontFileFunctions()
{
    int i;
 
    T1InitStdProps();
    for (i=0; i < sizeof(renderers) / sizeof(FontRendererRec); i++)
            FontFileRegisterRenderer(&renderers[i]);
}
 
/*
Synonym for xalloc() so type1 scanner doesn't have to know about X stuff:
*/
 
pointer type1alloc(size)
       unsigned size;
{
       return xalloc(size);
}
