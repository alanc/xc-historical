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

#ifndef Fresco_Impls_Xpainter_h
#define Fresco_Impls_Xpainter_h

#include <X11/Fresco/drawing.h>
#include <X11/Fresco/Impls/fobjects.h>
#include <X11/Fresco/Impls/Xlib.h>
#include <X11/Fresco/Impls/Xutil.h>
#include <X11/Fresco/OS/list.h>

class ClippingStack;
class FontImpl;
class MatrixStack;
class RegionImpl;
class ScreenImpl;
class TransformImpl;
class WindowImpl;

class DefaultPainterImpl : public PainterObjType {
public:
    DefaultPainterImpl();
    ~DefaultPainterImpl();

    //+ PainterObj::*
    /* FrescoObject */
    Long ref__(Long references);
    Tag attach(FrescoObject_in observer);
    void detach(Tag attach_tag);
    void disconnect();
    void notify_observers();
    void update();
    /* PainterObj */
    Coord to_coord(PixelCoord p);
    PixelCoord to_pixels(Coord c);
    Coord to_pixels_coord(Coord c);
    void begin_path();
    void move_to(Coord x, Coord y);
    void line_to(Coord x, Coord y);
    void curve_to(Coord x, Coord y, Coord x1, Coord y1, Coord x2, Coord y2);
    void close_path();
    BrushRef _c_brush_attr();
    void _c_brush_attr(Brush_in _p);
    ColorRef _c_color_attr();
    void _c_color_attr(Color_in _p);
    FontRef _c_font_attr();
    void _c_font_attr(Font_in _p);
    void stroke();
    void fill();
    void line(Coord x0, Coord y0, Coord x1, Coord y1);
    void rect(Coord x0, Coord y0, Coord x1, Coord y1);
    void fill_rect(Coord x0, Coord y0, Coord x1, Coord y1);
    void character(CharCode ch, Coord width, Coord x, Coord y);
    void image(Raster_in r, Coord x, Coord y);
    void stencil(Raster_in r, Coord x, Coord y);
    TransformObjRef _c_matrix();
    void _c_matrix(TransformObj_in _p);
    void push_matrix();
    void pop_matrix();
    void transform(TransformObj_in t);
    void clip();
    void clip_rect(Coord x0, Coord y0, Coord x1, Coord y1);
    void push_clipping();
    void pop_clipping();
    Boolean is_visible(Region_in r);
    RegionRef _c_visible();
    void comment(CharString_in s);
    void page_number(CharString_in s);
    //+

    virtual void set_clip();
    virtual void reset_clip();
    virtual void flush_text();
protected:
    SharedFrescoObjectImpl object_;
    BrushRef brush_;
    ColorRef color_;
    FontRef font_;
    Boolean transformed_;
    TransformImpl* matrix_;
    MatrixStack* transforms_;
    RegionImpl* clipping_;
    ClippingStack* clippers_;
    RegionImpl* free_clippers_[10];
    Long free_clipper_head_;
    Long free_clipper_tail_;
    XRectangle xclip_;

    RegionImpl* new_clip();
    void free_clip(RegionImpl*);
    Long clip_index(Long);
};

//+ XPainterImpl : DefaultPainterImpl
class XPainterImpl : public DefaultPainterImpl {
public:
    ~XPainterImpl();
    TypeObjId _tid();
    static XPainterImpl* _narrow(BaseObjectRef);
//+
public:
    XPainterImpl(WindowImpl*, ScreenImpl*);

    Coord to_coord(PixelCoord p); //+ PainterObj::to_coord
    PixelCoord to_pixels(Coord c); //+ PainterObj::to_pixels
    Coord to_pixels_coord(Coord c); //+ PainterObj::to_pixels_coord
    void begin_path(); //+ PainterObj::begin_path
    void move_to(Coord x, Coord y); //+ PainterObj::move_to
    void line_to(Coord x, Coord y); //+ PainterObj::line_to
    void curve_to(Coord x, Coord y, Coord x1, Coord y1, Coord x2, Coord y2); //+ PainterObj::curve_to
    void close_path(); //+ PainterObj::close_path
    void _c_brush_attr(Brush_in); //+ PainterObj::brush_attr=
    BrushRef _c_brush_attr(); //+ PainterObj::brush_attr?
    void _c_color_attr(Color_in); //+ PainterObj::color_attr=
    ColorRef _c_color_attr(); //+ PainterObj::color_attr?
    void _c_font_attr(Font_in); //+ PainterObj::font_attr=
    FontRef _c_font_attr(); //+ PainterObj::font_attr?
    void stroke(); //+ PainterObj::stroke
    void fill(); //+ PainterObj::fill
    void line(Coord x0, Coord y0, Coord x1, Coord y1); //+ PainterObj::line
    void rect(Coord x0, Coord y0, Coord x1, Coord y1); //+ PainterObj::rect
    void fill_rect(Coord x0, Coord y0, Coord x1, Coord y1); //+ PainterObj::fill_rect
    void character(CharCode ch, Coord width, Coord x, Coord y); //+ PainterObj::character
    void image(Raster_in r, Coord x, Coord y); //+ PainterObj::image
    void stencil(Raster_in r, Coord x, Coord y); //+ PainterObj::stencil

    struct FontInfo {
	FontImpl* font;
	XFont xfont;
	Boolean scaled;
	float scale;
	short charsize;
	short age;
    };

    /*
     * SubPathInfo structure holds information to emulate PostScript
     * subpath behavior under X, meaning a path may contain holes.
     * Each subpath is started and terminated with the same vertex.
     * This vertex can be chosen anywhere but the first vertex of the
     * first subpath is handy since we must return to it when the first
     * subpath is closed.  When we do this, XFillPolygon works as is.
     * For stroking, we simply send each subpath one after the other.
     */

    struct SubPathInfo {
	unsigned int start;
	unsigned int end;
	Boolean closed;
    };

    void prepare(Boolean double_buffered);
    void frontbuffer();
    void backbuffer();
    void swapbuffers();
    PixelCoord inline_to_pixels(Coord c);
    Coord inline_to_coord(PixelCoord p);
    Coord inline_to_pixels_coord(Coord c);
    void smoothness(float s);
    float smoothness();
protected:
    WindowImpl* window_;
    ScreenImpl* screen_;
    XDisplay* xdisplay_;
    Boolean double_buffered_;
    XDrawable xdrawable_;
    XDrawable xfrontbuffer_;
    XDrawable xbackbuffer_;
    GC xgc_;
    GC xfrontgc_;
    PixelCoord pwidth_;
    PixelCoord pheight_;
    Coord pixels_;
    Coord points_;
    float smoothness_;

    FontInfo* fontinfo_;
    FontInfo fontinfos_[10];
    PixelCoord tx0_, ty0_;
    Coord tx_, ty_;
    char* char_;
    char chars_[200];
    XTextItem* item_;
    XTextItem items_[100];

    Coord path_cur_x_;
    Coord path_cur_y_;
    XPoint* point_;
    XPoint* cur_point_;
    XPoint* end_point_;
    SubPathInfo* subpath_;
    SubPathInfo* cur_subpath_;
    SubPathInfo* end_subpath_;

    void init_fonts();
    void init_items();
    XPainterImpl::FontInfo* open_font(FontRef);
    XPainterImpl::FontInfo* find_font(FontRef);
    void close_fonts();
    void cleanup();
    XPoint* next_point();
    SubPathInfo* next_subpath();

    Boolean straight(
	Coord x0, Coord y0, Coord x1, Coord y1,
	Coord x2, Coord y2, Coord x3, Coord y3
    );
    Boolean rectangular(XPoint*);
    Coord mid(Coord a, Coord b);
    void curve_pt(
	Coord px, Coord py, Coord x, Coord y,
	Coord x1, Coord y1, Coord x2, Coord y2
    );
    void add_item();
    void add_char(CharCode);
    void flush_text();

    void set_clip();
    void reset_clip();
};

inline PixelCoord XPainterImpl::inline_to_pixels(Coord c) {
    return PixelCoord( c * points_ + ((c > 0) ? 0.5 : -0.5) );
}

inline Coord XPainterImpl::inline_to_coord(PixelCoord p) {
    return Coord(p) * pixels_;
}

inline Coord XPainterImpl::inline_to_pixels_coord(Coord c) {
    return inline_to_coord(inline_to_pixels(c));
}

inline void XPainterImpl::smoothness(float s) { smoothness_ = s; }
inline float XPainterImpl::smoothness() { return smoothness_; }

#endif
