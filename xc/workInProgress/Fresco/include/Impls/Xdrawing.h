/*
 * Copyright (c) 1987-91 Stanford University
 * Copyright (c) 1991-93 Silicon Graphics, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and 
 * its documentation for any purpose is hereby granted without fee, provided
 * that (i) the above copyright notices and this permission notice appear in
 * all copies of the software and related documentation, and (ii) the names of
 * Stanford and Silicon Graphics may not be used in any advertising or
 * publicity relating to the software without the specific, prior written
 * permission of Stanford and Silicon Graphics.
 * 
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY 
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  
 *
 * IN NO EVENT SHALL STANFORD OR SILICON GRAPHICS BE LIABLE FOR
 * ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF 
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE 
 * OF THIS SOFTWARE.
 */

#ifndef Fresco_Impls_Xdrawing_h
#define Fresco_Impls_Xdrawing_h

#include <X11/Fresco/drawing.h>
#include <X11/Fresco/Impls/fobjects.h>
#include <X11/Fresco/Impls/Xlib.h>

class DisplayImpl;
class DrawingKitColorTable;
class DrawingKitFontTable;

class DrawingKitImpl : public DrawingKitType {
public:
    DrawingKitImpl(DisplayImpl*);
    ~DrawingKitImpl();

    //+ DrawingKit::*
    /* FrescoObject */
    Long ref__(Long references);
    Tag attach(FrescoObject_in observer);
    void detach(Tag attach_tag);
    void disconnect();
    void notify_observers();
    void update();
    /* DrawingKit */
    StyleObjRef _c_style();
    BrushRef _c_simple_brush(Coord width);
    BrushRef _c_dither_brush(Coord width, Long pattern);
    BrushRef _c_patterned_brush(Coord width, const DrawingKit::Data& pattern);
    ColorRef _c_color_rgb(Color::Intensity r, Color::Intensity g, Color::Intensity b);
    ColorRef _c_find_color(CharString_in name);
    ColorRef _c_resolve_color(StyleObj_in s, CharString_in name);
    ColorRef _c_foreground(StyleObj_in s);
    ColorRef _c_background(StyleObj_in s);
    FontRef _c_find_font(CharString_in fullname);
    FontRef _c_find_font_match(CharString_in family, CharString_in style, Coord ptsize);
    FontRef _c_resolve_font(StyleObj_in s, CharString_in name);
    FontRef _c_default_font(StyleObj_in s);
    RasterRef _c_bitmap_file(CharString_in filename);
    RasterRef _c_bitmap_data(const DrawingKit::Data& data, Raster::Index rows, Raster::Index columns, Raster::Index origin_row, Raster::Index origin_column);
    RasterRef _c_bitmap_char(Font_in f, CharCode c);
    RasterRef _c_raster_tiff(CharString_in filename);
    TransformObjRef _c_transform(TransformObj::Matrix m);
    TransformObjRef _c_identity_transform();
    PainterObjRef _c_printer(CharString_in output);
    //+
protected:
    SharedFrescoObjectImpl object_;
    DisplayImpl* display_;
    DrawingKitColorTable* colors_;
    DrawingKitFontTable* fonts_;
    CharString foreground_str_;
    CharString Foreground_str_;
    CharString background_str_;
    CharString Background_str_;
    CharString font_str_;
    CharString Font_str_;

    ColorRef resolve_colors(
	StyleObjRef s, CharString_in name1, CharString_in name2
    );
};

//+ BrushImpl : BrushType
class BrushImpl : public BrushType {
public:
    ~BrushImpl();
    TypeObjId _tid();
    static BrushImpl* _narrow(BaseObjectRef);
//+
public:
    BrushImpl(Coord width);
    BrushImpl(const long* p, long c, Coord w);
    BrushImpl(long pat, Coord w);

    //+ Brush::*
    /* FrescoObject */
    Long ref__(Long references);
    Tag attach(FrescoObject_in observer);
    void detach(Tag attach_tag);
    void disconnect();
    void notify_observers();
    void update();
    /* Brush */
    Boolean equal(Brush_in b);
    ULong hash();
    //+

    void info(Coord& width, char*& dash_list, long& dash_count);
protected:
    SharedFrescoObjectImpl object_;
    Coord width_;
    char* dash_list_;
    long dash_count_;

    void init(const long* p, long c, Coord w);
    void calc_dashes(long pat, long* dash, long& count);
};

//+ ColorImpl : ColorType
class ColorImpl : public ColorType {
public:
    ~ColorImpl();
    TypeObjId _tid();
    static ColorImpl* _narrow(BaseObjectRef);
//+
public:
    ColorImpl(Color::Intensity r, Color::Intensity g, Color::Intensity b);

    //+ Color::*
    /* FrescoObject */
    Long ref__(Long references);
    Tag attach(FrescoObject_in observer);
    void detach(Tag attach_tag);
    void disconnect();
    void notify_observers();
    void update();
    /* Color */
    void rgb(Color::Intensity& r, Color::Intensity& g, Color::Intensity& b);
    Boolean equal(Color_in c);
    ULong hash();
    //+

    static unsigned short to_short(Color::Intensity);
    static Color::Intensity to_intensity(unsigned short);
protected:
    SharedFrescoObjectImpl object_;
    Color::Intensity red_;
    Color::Intensity green_;
    Color::Intensity blue_;
};

/*
 * The assumption here is that intensities are positive,
 * so the roundoff doesn't need to do a sign check.
 */

inline unsigned short ColorImpl::to_short(Color::Intensity i) {
    return (unsigned short)(i * float(0xffff) + 0.5);
}

inline Color::Intensity ColorImpl::to_intensity(unsigned short s) {
    return float(s) / float(0xffff);
}

//+ FontImpl : FontType
class FontImpl : public FontType {
public:
    ~FontImpl();
    TypeObjId _tid();
    static FontImpl* _narrow(BaseObjectRef);
//+
public:
    FontImpl(DisplayImpl*, const char* name);

    //+ Font::*
    /* FrescoObject */
    Long ref__(Long references);
    Tag attach(FrescoObject_in observer);
    void detach(Tag attach_tag);
    void disconnect();
    void notify_observers();
    void update();
    /* Font */
    Boolean equal(Font_in f);
    ULong hash();
    CharStringRef _c_name();
    CharStringRef _c_encoding();
    Coord point_size();
    void font_info(Font::Info& i);
    void char_info(CharCode c, Font::Info& i);
    void string_info(CharString_in s, Font::Info& i);
    //+

    void load();
    XDisplay* xdisplay();
    XFontStruct* xfont();
    RasterRef bitmap(CharCode);
    Coord to_coord(int);
protected:
    SharedFrescoObjectImpl object_;
    CharStringRef name_;
    CharStringRef encoding_;
    Coord point_size_;
    Coord scale_;
    Coord* widths_;
    XDisplay* xdisplay_;
    XFontStruct* xfont_;
    RasterRef* rasters_;
};

inline XDisplay* FontImpl::xdisplay() { return xdisplay_; }
inline XFontStruct* FontImpl::xfont() { return xfont_; }

#endif
