/*
 * $XConsortium$
 */

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

/*
 * X-dependent drawing operations
 */

#include <X11/Fresco/display.h>
#include <X11/Fresco/drawing.h>
#include <X11/Fresco/fresco.h>
#include <X11/Fresco/Impls/charstr.h>
#include <X11/Fresco/Impls/fresco-impl.h>
#include <X11/Fresco/Impls/region.h>
#include <X11/Fresco/Impls/transform.h>
#include <X11/Fresco/Impls/Xdisplay.h>
#include <X11/Fresco/Impls/Xdrawing.h>
#include <X11/Fresco/Impls/Xpainter.h>
#include <X11/Fresco/Impls/Xraster.h>
#include <X11/Fresco/Impls/Xwindow.h>
#include <X11/Fresco/Impls/Xlib.h>
#include <X11/Fresco/OS/math.h>
#include <X11/Fresco/OS/memory.h>
#include <X11/Fresco/OS/table.h>
#include <X11/Xatom.h>
#include <ctype.h>
#include <limits.h>

#include <stdio.h>

/*
 * This rather disgusting macro encapsulates the tedium of testing whether
 * an operation to set a paint object, such as brush or color, in fact
 * changes the current paint state.  The usage is "same(new,old)" and
 * returns true if the new is nil or equal to old.
 */

#define same(a,b) ( \
    is_nil(a) ? true : is_not_nil(b) && (Fresco::ref(b), a->equal(b)) \
)

/*
 * Two useful array macros: determine the number of elements and
 * return the address of the end of the array (after last element).
 * We assume the compiler will fold any constant arithmetic.
 */

#define array_nelements(array) (sizeof(array) / sizeof(array[0]))
#define array_end(array) (&array[array_nelements(array)])

/*
 * This function tests whether a matrix is only a translation.
 * Something like this probably should be provided directly
 * by the TransformObj interface.
 */
static Boolean is_translation(TransformRef t) {
    TransformObj::Matrix m;
    t->store_matrix(m);
    return m[0][0] == 1 && m[0][1] == 0 && m[1][0] == 0 && m[1][1] == 1;
}

BrushImpl::BrushImpl(Coord w) { init(nil, 0, w); }
BrushImpl::BrushImpl(const long* p, long c, Coord w) { init(p, c, w); }

BrushImpl::BrushImpl(long pat, Coord w) {
    long dash[16];
    long count;

    calc_dashes(pat, dash, count);
    init(dash, count, w);
}

BrushImpl::~BrushImpl() {
    delete dash_list_;
}

//+ BrushImpl(FrescoObject::=object_.)
Long BrushImpl::ref__(Long references) {
    return object_.ref__(references);
}
Tag BrushImpl::attach(FrescoObject_in observer) {
    return object_.attach(observer);
}
void BrushImpl::detach(Tag attach_tag) {
    object_.detach(attach_tag);
}
void BrushImpl::disconnect() {
    object_.disconnect();
}
void BrushImpl::notify_observers() {
    object_.notify_observers();
}
void BrushImpl::update() {
    object_.update();
}
//+

//+ BrushImpl(Brush::hash)
ULong BrushImpl::hash() {
    return 0;
}

//+ BrushImpl(Brush::equal)
Boolean BrushImpl::equal(Brush_in b) {
    if (is_nil(b) || hash() != b->hash()) {
	return false;
    }
    /* should compare values */
    return this == b;
}

void BrushImpl::info(Coord& w, char*& dash_list, long& count) {
    w = width_;
    dash_list = dash_list_;
    count = dash_count_;
}

void BrushImpl::init(const long* pattern, long count, Coord w) {
    width_ = w;
    dash_count_ = count;
    if (count > 0) {
        dash_list_ = new char[count];
        for (long i = 0; i < count; i++) {
            dash_list_[i] = char(pattern[i]);
        }
    } else {
        dash_list_ = nil;
    }
}

void BrushImpl::calc_dashes(long pat, long* dash, long& count) {
    unsigned long p = pat & 0xffff;

    if (p == 0 || p == 0xffff) {
        count = 0;
    } else {
        const unsigned long MSB = 1 << 15;
        while ((p & MSB) == 0) {
            p <<= 1;
        }

        if (p == 0x5555 || p == 0xaaaa) {
            dash[0] = 1;
            dash[1] = 3;
            count = 2;
        } else if (p == 0xaaaa) {
            dash[0] = 1;
            dash[1] = 1;
            count = 2;
        } else if (p == 0xcccc) {
            dash[0] = 2;
            dash[1] = 2;
            count = 2;
        } else if (p == 0xeeee) {
            dash[0] = 3;
            dash[1] = 1;
            count = 2;
        } else {
           unsigned long m = MSB;
           long index = 0;
           while (m != 0) {
                /* count the consecutive one bits */
                int length = 0;
                while (m != 0 && (p & m) != 0) {
                    ++length;
                    m >>= 1;
                }
                dash[index++] = length;

                /* count the consecutive zero bits */
                length = 0;
                while (m != 0 && (p & m) == 0) {
                    ++length;
                    m >>= 1;
                }
                if (length > 0) {
                    dash[index++] = length;
                }
            }
            count = index;
        }
    }
}

ColorImpl::ColorImpl(
    Color::Intensity r, Color::Intensity g, Color::Intensity b
) {
    red_ = r;
    green_ = g;
    blue_ = b;
}

ColorImpl::~ColorImpl() { }

//+ ColorImpl(FrescoObject::=object_.)
Long ColorImpl::ref__(Long references) {
    return object_.ref__(references);
}
Tag ColorImpl::attach(FrescoObject_in observer) {
    return object_.attach(observer);
}
void ColorImpl::detach(Tag attach_tag) {
    object_.detach(attach_tag);
}
void ColorImpl::disconnect() {
    object_.disconnect();
}
void ColorImpl::notify_observers() {
    object_.notify_observers();
}
void ColorImpl::update() {
    object_.update();
}
//+

//+ ColorImpl(Color::rgb)
void ColorImpl::rgb(Color::Intensity& r, Color::Intensity& g, Color::Intensity& b) {
    r = red_;
    g = green_;
    b = blue_;
}

/*
 * Hash function generates 18 distinct bits, 6 each from rgb.
 * The idea is to "promote" collisions a little bit, so that colors
 * that are indistinguishable will match.
 */

//+ ColorImpl(Color::hash)
ULong ColorImpl::hash() {
    long r = long(64 * red_) & 0x3f;
    long g = long(64 * green_) & 0x3f;
    long b = long(64 * blue_) & 0x3f;
    return (r << 12) | (g << 6) | b;
}

//+ ColorImpl(Color::equal)
Boolean ColorImpl::equal(Color_in c) {
    if (is_nil(c)) {
	return false;
    }
    Color::Intensity r, g, b;
    c->rgb(r, g, b);
    float tol = 1e-2;
    /* convoluted testing to workaround compiler inline problem */
    Boolean result = Math::equal(red_, r, tol);
    if (result) {
	result = Math::equal(green_, g, tol);
	if (result) {
	    result = Math::equal(blue_, b, tol);
	}
    }
    return result;
}

static inline ULong num_chars(XFontStruct* fs) {
    return (
	(fs->max_byte1 - fs->min_byte1 + 1) *
	(fs->max_char_or_byte2 - fs->min_char_or_byte2 + 1)
    );
}

static ULong char_index(CharCode c, XFontStruct* fs) {
    if (fs->min_byte1 == fs->max_byte1) {
	return ULong(c - fs->min_char_or_byte2);
    }
    ULong width = fs->max_char_or_byte2 - fs->min_char_or_byte2 + 1;
    return (
	((c / width) - fs->min_byte1) * width +
	(c % width) - fs->min_char_or_byte2
    );
}

FontImpl::FontImpl(DisplayImpl* d, const char* name) {
    name_ = new CharStringImpl(name);
    encoding_ = nil;
    point_size_ = 0;
    scale_ = 1.0;
    widths_ = nil;
    xdisplay_ = d->xdisplay();
    xfont_ = nil;
    rasters_ = nil;
}

FontImpl::~FontImpl() {
    Fresco::unref(name_);
    Fresco::unref(encoding_);
    delete [] widths_;
    if (rasters_ != nil) {
	for (Long i = num_chars(xfont_); i >= 0; i--) {
	    Fresco::unref(rasters_[i]);
	}
    }
    if (xfont_ != nil) {
	XFreeFont(xdisplay_, xfont_);
    }
}

//+ FontImpl(FrescoObject::=object_.)
Long FontImpl::ref__(Long references) {
    return object_.ref__(references);
}
Tag FontImpl::attach(FrescoObject_in observer) {
    return object_.attach(observer);
}
void FontImpl::detach(Tag attach_tag) {
    object_.detach(attach_tag);
}
void FontImpl::disconnect() {
    object_.disconnect();
}
void FontImpl::notify_observers() {
    object_.notify_observers();
}
void FontImpl::update() {
    object_.update();
}
//+

//+ FontImpl(Font::hash)
ULong FontImpl::hash() {
    return name_->hash();
}

//+ FontImpl(Font::equal)
Boolean FontImpl::equal(Font_in f) {
    return f == this || name_->equal(f->name());
}

//+ FontImpl(Font::name)
CharStringRef FontImpl::_c_name() {
    return CharString::_duplicate(name_);
}

//+ FontImpl(Font::encoding)
CharStringRef FontImpl::_c_encoding() {
    return CharString::_duplicate(encoding_);
}

//+ FontImpl(Font::point_size)
Coord FontImpl::point_size() {
    load();
    return point_size_;
}

//+ FontImpl(Font::font_info)
void FontImpl::font_info(Font::Info& i) {
    load();
    XFontStruct* xf = xfont_;
    Coord s = scale_;
    i.left_bearing = to_coord(xf->max_bounds.lbearing);
    i.right_bearing = to_coord(xf->max_bounds.rbearing);
    i.width = to_coord(xf->max_bounds.width);
    i.ascent = to_coord(xf->ascent);
    i.descent = to_coord(xf->descent);
    i.font_ascent = i.ascent;
    i.font_descent = i.descent;
}

//+ FontImpl(Font::char_info)
void FontImpl::char_info(CharCode c, Font::Info& i) {
    load();
    Coord s = scale_;
    XCharStruct xc;
    XChar2b xc2b;
    xc2b.byte1 = (unsigned char)((c & 0xff00) >> 8);
    xc2b.byte2 = (unsigned char)(c & 0xff);
    int dir, asc, des;
    XTextExtents16(xfont_, &xc2b, 1, &dir, &asc, &des, &xc);
    i.left_bearing = to_coord(-xc.lbearing);
    i.right_bearing = to_coord(xc.rbearing);
    i.ascent = to_coord(xc.ascent);
    i.descent = to_coord(xc.descent);
    i.font_ascent = to_coord(xfont_->ascent);
    i.font_descent = to_coord(xfont_->descent);
    if (widths_ != nil) {
	i.width = widths_[c];
    } else {
	i.width = to_coord(XTextWidth16(xfont_, &xc2b, 1));
    }
}

//+ FontImpl(Font::string_info)
void FontImpl::string_info(CharString_in s, Font::Info& i) {
    load();
    CharStringBuffer buf(s);
    const char* cp = buf.string();
    long n = buf.length();
    XCharStruct xc;
    int dir, asc, des;
    XTextExtents(xfont_, cp, int(n), &dir, &asc, &des, &xc);
    i.left_bearing = to_coord(-xc.lbearing);
    i.right_bearing = to_coord(xc.rbearing);
    i.ascent = to_coord(xc.ascent);
    i.descent = to_coord(xc.descent);
    i.font_ascent = to_coord(xfont_->ascent);
    i.font_descent = to_coord(xfont_->descent);
    if (widths_ != nil) {
	Coord w = 0;
	for (long p = 0; p < n; p++) {
	    w += widths_[cp[p]];
	}
	i.width = w;
    } else {
	i.width = to_coord(XTextWidth(xfont_, cp, int(n)));
    }
}

void FontImpl::load() {
    if (xfont_ != nil) {
	return;
    }
    CharStringBuffer buf(name_);
    xfont_ = XLoadQueryFont(xdisplay_, buf.string());
    if (xfont_ == nil) {
	/* raise exception? */
	return;
    }
    unsigned long value;
    if (XGetFontProperty(xfont_, XA_POINT_SIZE, &value)) {
	point_size_ = Coord(value) / 10.0;
    }
    if (XGetFontProperty(xfont_, XA_RESOLUTION, &value)) {
	scale_ = (100.0 * 72.0 / 72.27) / Coord(value);
    }
}

RasterRef FontImpl::bitmap(CharCode c) {
    load();
    ULong index = char_index(c, xfont_);
    if (rasters_ == nil) {
	ULong size = num_chars(xfont_);
	rasters_ = new RasterRef[size];
	Memory::zero(rasters_, size * sizeof(RasterRef));
    } else {
	RasterRef r = rasters_[index];
	if (is_not_nil(r)) {
	    Fresco::ref(r);
	    return r;
	}
    }
    Coord s = scale_;
    XCharStruct xc;
    XChar2b xc2b;
    xc2b.byte1 = (unsigned char)((c & 0xff00) >> 8);
    xc2b.byte2 = (unsigned char)(c & 0xff);
    int dir, asc, des;
    XTextExtents16(xfont_, &xc2b, 1, &dir, &asc, &des, &xc);

    int cwidth = xc.rbearing - xc.lbearing;
    int cheight = xc.ascent + xc.descent;
    Pixmap pixmap = XCreatePixmap(
	xdisplay_, DefaultRootWindow(xdisplay_), cwidth, cheight, 1
    );
    XGCValues values;
    values.font = xfont_->fid;
    values.background = 0;
    values.foreground = 0;
    GC xgc = XCreateGC(
	xdisplay_, pixmap, GCForeground | GCBackground | GCFont, &values
    );
    XFillRectangle(xdisplay_, pixmap, xgc, 0, 0, cwidth, cheight);
    XSetForeground(xdisplay_, xgc, 1);
    XDrawString16(xdisplay_, pixmap, xgc, -xc.lbearing, xc.ascent, &xc2b, 1);
    XFreeGC(xdisplay_, xgc);
    DrawingKit::Data data;
    data._buffer = nil;
    RasterBitmap* rb = new RasterBitmap(
	data, cheight, cwidth, xc.descent, -xc.lbearing, scale_
    );
    rb->read_drawable(xdisplay_, pixmap, 0, 0);
    XFreePixmap(xdisplay_, pixmap);
    rasters_[index] = rb;
    Fresco::ref(rb);
    return rb;
}

Coord FontImpl::to_coord(XCoord c) {
    return scale_ * Coord(c);
}

/*
 * We only need this silly MatrixStackElement because the compiler
 * (language) can't deal with functions that return arrays.
 */

struct MatrixStackElement {
    TransformObj::Matrix matrix;
};

declareList(MatrixStack,MatrixStackElement)
implementList(MatrixStack,MatrixStackElement)

declarePtrList(ClippingStack,RegionImpl)
implementPtrList(ClippingStack,RegionImpl)

DefaultPainterImpl::DefaultPainterImpl() {
    brush_ = nil;
    color_ = nil;
    font_ = nil;
    matrix_ = new TransformImpl;
    transforms_ = new MatrixStack;
    transformed_ = false;
    clipping_ = nil;
    clippers_ = new ClippingStack;
    free_clipper_head_ = 0;
    free_clipper_tail_ = 0;
}

DefaultPainterImpl::~DefaultPainterImpl() {
    Fresco::unref(brush_);
    Fresco::unref(color_);
    Fresco::unref(font_);
    delete transforms_;
    Fresco::unref(clipping_);
    for (ListItr(ClippingStack) j(*clippers_); j.more(); j.next()) {
	Fresco::unref(j.cur());
    }
    delete clippers_;
}

//+ DefaultPainterImpl(FrescoObject::=object_.)
Long DefaultPainterImpl::ref__(Long references) {
    return object_.ref__(references);
}
Tag DefaultPainterImpl::attach(FrescoObject_in observer) {
    return object_.attach(observer);
}
void DefaultPainterImpl::detach(Tag attach_tag) {
    object_.detach(attach_tag);
}
void DefaultPainterImpl::disconnect() {
    object_.disconnect();
}
void DefaultPainterImpl::notify_observers() {
    object_.notify_observers();
}
void DefaultPainterImpl::update() {
    object_.update();
}
//+

//+ DefaultPainterImpl(PainterObj::to_coord)
Coord DefaultPainterImpl::to_coord(PixelCoord p) { return Coord(p); }

//+ DefaultPainterImpl(PainterObj::to_pixels)
PixelCoord DefaultPainterImpl::to_pixels(Coord c) { return PixelCoord(c); }

//+ DefaultPainterImpl(PainterObj::to_pixels_coord)
Coord DefaultPainterImpl::to_pixels_coord(Coord c) { return c; }

//+ DefaultPainterImpl(PainterObj::begin_path)
void DefaultPainterImpl::begin_path() { }

//+ DefaultPainterImpl(PainterObj::move_to)
void DefaultPainterImpl::move_to(Coord, Coord) { }

//+ DefaultPainterImpl(PainterObj::line_to)
void DefaultPainterImpl::line_to(Coord, Coord) { }

//+ DefaultPainterImpl(PainterObj::curve_to)
void DefaultPainterImpl::curve_to(Coord, Coord, Coord, Coord, Coord, Coord) { }

//+ DefaultPainterImpl(PainterObj::close_path)
void DefaultPainterImpl::close_path() { }

//+ DefaultPainterImpl(PainterObj::brush_attr=b)
void DefaultPainterImpl::_c_brush_attr(Brush_in b) {
    brush_ = Brush::_duplicate(b);
}

//+ DefaultPainterImpl(PainterObj::brush_attr?)
BrushRef DefaultPainterImpl::_c_brush_attr() {
    return Brush::_duplicate(brush_);
}

//+ DefaultPainterImpl(PainterObj::color_attr=c)
void DefaultPainterImpl::_c_color_attr(Color_in c) {
    color_ = Color::_duplicate(c);
}

//+ DefaultPainterImpl(PainterObj::color_attr?)
ColorRef DefaultPainterImpl::_c_color_attr() {
    return Color::_duplicate(color_);
}

//+ DefaultPainterImpl(PainterObj::font_attr=f)
void DefaultPainterImpl::_c_font_attr(Font_in f) {
    font_ = Font::_duplicate(f);
}

//+ DefaultPainterImpl(PainterObj::font_attr?)
FontRef DefaultPainterImpl::_c_font_attr() {
    return Font::_duplicate(font_);
}

//+ DefaultPainterImpl(PainterObj::stroke)
void DefaultPainterImpl::stroke() { }

//+ DefaultPainterImpl(PainterObj::fill)
void DefaultPainterImpl::fill() { }

//+ DefaultPainterImpl(PainterObj::line)
void DefaultPainterImpl::line(Coord, Coord, Coord, Coord) { }

//+ DefaultPainterImpl(PainterObj::rect)
void DefaultPainterImpl::rect(Coord, Coord, Coord, Coord) { }

//+ DefaultPainterImpl(PainterObj::fill_rect)
void DefaultPainterImpl::fill_rect(Coord, Coord, Coord, Coord) { }

//+ DefaultPainterImpl(PainterObj::character)
void DefaultPainterImpl::character(CharCode, Coord, Coord, Coord) { }

//+ DefaultPainterImpl(PainterObj::image)
void DefaultPainterImpl::image(Raster_in, Coord, Coord) { }

//+ DefaultPainterImpl(PainterObj::stencil)
void DefaultPainterImpl::stencil(Raster_in, Coord, Coord) { }

//+ DefaultPainterImpl(PainterObj::matrix=t)
void DefaultPainterImpl::_c_matrix(TransformObj_in t) {
    flush_text();
    matrix_->load(t);
    transformed_ = !matrix_->identity();
}

//+ DefaultPainterImpl(PainterObj::matrix?)
TransformObjRef DefaultPainterImpl::_c_matrix() {
    return TransformObj::_duplicate(matrix_);
}

//+ DefaultPainterImpl(PainterObj::push_matrix)
void DefaultPainterImpl::push_matrix() {
    transforms_->prepend(*((MatrixStackElement*)(matrix_->matrix())));
}

//+ DefaultPainterImpl(PainterObj::pop_matrix)
void DefaultPainterImpl::pop_matrix() {
    MatrixStack* s = transforms_;
    if (s->count() == 0) {
	/*
	 * Underflow -- should raise an exception.
	 */
	return;
    }
    flush_text();
    matrix_->load_matrix(s->item_ref(0).matrix);
    transformed_ = !matrix_->identity();
    s->remove(0);
}

//+ DefaultPainterImpl(PainterObj::transform)
void DefaultPainterImpl::transform(TransformObj_in t) {
    flush_text();
    matrix_->premultiply(t);
    transformed_ = !matrix_->identity();
}

//+ DefaultPainterImpl(PainterObj::clip)
void DefaultPainterImpl::clip() {
    /* not implemented */
}

//+ DefaultPainterImpl(PainterObj::clip_rect)
void DefaultPainterImpl::clip_rect(Coord x0, Coord y0, Coord x1, Coord y1) {
    RegionImpl region;
    RegionImpl* r = (clipping_ == nil) ? new_clip() : &region;
    r->lower_.x = x0; r->lower_.y = y0; r->lower_.z = 0.0;
    r->upper_.x = x1; r->upper_.y = y1; r->upper_.z = 0.0;
    if (transformed_) {
	r->transform(matrix_);
    }
    if (clipping_ == nil) {
	clipping_ = r;
    } else {
	clipping_->merge_intersect(r);
    }
    set_clip();
}

//+ DefaultPainterImpl(PainterObj::push_clipping)
void DefaultPainterImpl::push_clipping() {
//printf("push_clipping\n");
    clippers_->prepend(clipping_);
    if (is_not_nil(clipping_)) {
	RegionImpl* r = new_clip();
	r->lower_ = clipping_->lower_;
	r->upper_ = clipping_->upper_;
	clipping_ = r;
    }
}

inline Long DefaultPainterImpl::clip_index(Long i) {
    return i % (sizeof(free_clippers_) / sizeof(free_clippers_[0]));
}

RegionImpl* DefaultPainterImpl::new_clip() {
    RegionImpl* r;
    if (free_clipper_head_ != free_clipper_tail_) {
	r = free_clippers_[free_clipper_head_];
	free_clipper_head_ = clip_index(free_clipper_head_ + 1);
    } else {
	r = new RegionImpl;
    }
    return r;
}

//+ DefaultPainterImpl(PainterObj::pop_clipping)
void DefaultPainterImpl::pop_clipping() {
    ClippingStack* s = clippers_;
    if (s->count() == 0) {
	/*
	 * Underflow -- should raise an exception.
	 */
	return;
    }
//printf("pop_clipping\n");
    flush_text();
    free_clip(clipping_);
    clipping_ = s->item(0);
    s->remove(0);
    reset_clip();
}

void DefaultPainterImpl::free_clip(RegionImpl* r) {
    Long new_tail = clip_index(free_clipper_tail_ + 1);
    if (new_tail != free_clipper_head_) {
	free_clippers_[free_clipper_tail_] = r;
	free_clipper_tail_ = new_tail;
    } else {
	Fresco::unref(r);
    }
}

/*
 * Determine if the given region is (at least partially) visible
 * given the current clipping region.
 */

//+ DefaultPainterImpl(PainterObj::is_visible)
Boolean DefaultPainterImpl::is_visible(Region_in r) {
    Boolean b = is_nil(clipping_);
    if (!b) {
	RegionRef rr;
	RegionImpl bounds;
	if (matrix_->identity()) {
	    rr = r;
	} else {
	    rr = &bounds;
	    rr->copy(r);
	    rr->transform(matrix_);
	}
	b = clipping_->intersects(rr);
    }
    return b;
}

//+ DefaultPainterImpl(PainterObj::visible)
RegionRef DefaultPainterImpl::_c_visible() {
    return Region::_duplicate(clipping_);
}

/*
 * The comment and page_number operations are typically
 * only relevent for a printer.
 */

//+ DefaultPainterImpl(PainterObj::comment)
void DefaultPainterImpl::comment(CharString_in) { }

//+ DefaultPainterImpl(PainterObj::page_number)
void DefaultPainterImpl::page_number(CharString_in) { }

void DefaultPainterImpl::set_clip() { }
void DefaultPainterImpl::reset_clip() { }
void DefaultPainterImpl::flush_text() { }

/* class XPainterImpl */

XPainterImpl::XPainterImpl(WindowImpl* w, ScreenImpl* s) {
    window_ = w;
    screen_ = s;
    xdisplay_ = w->display()->xdisplay();
    double_buffered_ = false;
    xdrawable_ = nil;
    xfrontbuffer_ = nil;
    xbackbuffer_ = nil;
    xgc_ = nil;
    xfrontgc_ = nil;
    pixels_ = s->to_coord(1);
    points_ = 1 / pixels_;
    smoothness_ = 10.0;
    point_ = new XPoint[25];
    cur_point_ = point_;
    end_point_ = point_ + 25;
    subpath_ = new SubPathInfo[5];
    cur_subpath_ = subpath_;
    end_subpath_ = subpath_ + 5;
    init_fonts();
    init_items();
    tx0_ = 0;
    ty0_ = 0;
    tx_ = 0.0;
    ty_ = 0.0;
    pwidth_ = 0;
    pheight_ = 0;
}

/*
 * The assumption here is the we were constructred without being given
 * a refcount for the WindowImpl and ScreenImpl, so we don't need
 * to unref them.  We also must re-fetch the xdisplay_ data member,
 * as the display may already have been closed.
 */

XPainterImpl::~XPainterImpl() {
    xdisplay_ = window_->display()->xdisplay();
    cleanup();
    close_fonts();
}

//+ XPainterImpl(PainterObj::to_coord)
Coord XPainterImpl::to_coord(PixelCoord p) {
    return inline_to_coord(p);
}

//+ XPainterImpl(PainterObj::to_pixels)
PixelCoord XPainterImpl::to_pixels(Coord c) {
    return inline_to_pixels(c);
}

//+ XPainterImpl(PainterObj::to_pixels_coord)
Coord XPainterImpl::to_pixels_coord(Coord c) {
    return inline_to_pixels_coord(c);
}

Boolean XPainterImpl::straight(
    Coord x0, Coord y0, Coord x1, Coord y1,
    Coord x2, Coord y2, Coord x3, Coord y3
) {
    float f = (
        (x1 + x2) * (y0 - y3) + (y1 + y2) * (x3 - x0) +
        2 * (x0 * y3 - y0 * x3)
    );
    return (f * f) < smoothness_;
}

inline Coord XPainterImpl::mid(Coord a, Coord b) {
    return (a + b) * 0.5;
}

//+ XPainterImpl(PainterObj::begin_path)
void XPainterImpl::begin_path() {
    path_cur_x_ = 0;
    path_cur_y_ = 0;
    XPoint* xp = point_;
    xp->x = 0;
    xp->y = 0;
    cur_point_ = xp;
    SubPathInfo* sp = subpath_;
    cur_subpath_ = sp;
    cur_subpath_->start = xp - point_;
    cur_subpath_->end = cur_subpath_->start;
    cur_subpath_->closed = false;
}

//+ XPainterImpl(PainterObj::move_to)
void XPainterImpl::move_to(Coord x, Coord y) {
    path_cur_x_ = x;
    path_cur_y_ = y;
    Vertex v;
    v.x = x; v.y = y; v.z = 0;
    if (transformed_) {
	matrix_->transform(v);
    }

    XPoint* xp;
    /* Test for two move_to on the row */
    if (cur_point_ - point_ == cur_subpath_->start + 1) {
	cur_point_ = point_ + cur_subpath_->start;
	xp = cur_point_;
    } else if (cur_point_ != point_) {
	/* start a new subpath */
	if (!cur_subpath_->closed) {
	    /* do some work to close the path */
	    cur_subpath_->end = cur_point_ - point_;
	    close_path();
	    cur_subpath_->closed = false;
	}
	next_subpath();
	xp = next_point();
	cur_subpath_->start = xp - point_;
	cur_subpath_->end = 0; /* means not terminated yet */
	cur_subpath_->closed = false;
    } else {
	xp = next_point();
    }

    xp->x = short(inline_to_pixels(v.x));
    xp->y = short(pheight_ - inline_to_pixels(v.y));
    cur_point_ = xp + 1;
}

static void constrain_point(XPoint* xp, PixelCoord x, PixelCoord y) {
    if (x < SHRT_MIN) {
	xp->x = SHRT_MIN;
    } else if (x > SHRT_MAX) {
	xp->x = SHRT_MAX;
    } else {
	xp->x = XCoord(x);
    }
    if (y < SHRT_MIN) {
	xp->y = SHRT_MIN;
    } else if (y > SHRT_MAX) {
	xp->y = SHRT_MAX;
    } else {
	xp->y = XCoord(y);
    }
}

//+ XPainterImpl(PainterObj::line_to)
void XPainterImpl::line_to(Coord x, Coord y) {
    if (cur_subpath_->closed) {
	return;
    }

    path_cur_x_ = x;
    path_cur_y_ = y;
    Vertex v;
    v.x = x; v.y = y; v.z = 0;
    if (transformed_) {
	matrix_->transform(v);
    }
    PixelCoord px = inline_to_pixels(v.x);
    PixelCoord py = pheight_ - inline_to_pixels(v.y);
    constrain_point(next_point(), px, py);
}

//+ XPainterImpl(PainterObj::curve_to)
void XPainterImpl::curve_to(Coord x, Coord y, Coord x1, Coord y1, Coord x2, Coord y2) {
    Vertex v, v1, v2, pv;
    v.x = x;
    v.y = y;
    v1.x = x1;
    v1.y = y1;
    v2.x = x2;
    v2.y = y2;
    pv.x = path_cur_x_;
    pv.y = path_cur_y_;
    if (transformed_) {
	matrix_->transform(v);
	matrix_->transform(v1);
	matrix_->transform(v2);
	matrix_->transform(pv);
    }
    curve_pt(pv.x, pv.y, v.x, v.y, v1.x, v1.y, v2.x, v2.y);
    path_cur_x_ = x;
    path_cur_y_ = y;
}

//+ XPainterImpl(PainterObj::close_path)
void XPainterImpl::close_path() {
    if (cur_subpath_->closed) {
	return;
    }

    XPoint* start = point_ + cur_subpath_->start;
    XPoint* xp = next_point();
    *xp = *start;

    cur_subpath_->end = cur_point_ - point_;
    if (cur_subpath_ != subpath_) {
	/* return to the first point of the first subpath */
	xp = next_point();
	*xp = *point_;
    }
    cur_subpath_->closed = true;
}

//+ XPainterImpl(PainterObj::line)
void XPainterImpl::line(Coord x0, Coord y0, Coord x1, Coord y1) {
    begin_path();
    move_to(x0, y0);
    line_to(x1, y1);
    stroke();
}

//+ XPainterImpl(PainterObj::rect)
void XPainterImpl::rect(Coord x0, Coord y0, Coord x1, Coord y1) {
    begin_path();
    move_to(x0, y0);
    line_to(x0, y1);
    line_to(x1, y1);
    line_to(x1, y0);
    close_path();
    stroke();
}

//+ XPainterImpl(PainterObj::fill_rect)
void XPainterImpl::fill_rect(Coord x0, Coord y0, Coord x1, Coord y1) {
    begin_path();
    move_to(x0, y0);
    line_to(x0, y1);
    line_to(x1, y1);
    line_to(x1, y0);
    close_path();
    fill();
}

//+ XPainterImpl(PainterObj::brush_attr=b)
void XPainterImpl::_c_brush_attr(Brush_in b) {
    if (same(b, brush_)) {
	return;
    }
    Fresco::unref(brush_);
    brush_ = Brush::_duplicate(b);
    /*
     * Should do (checked) narrow instead of cast.
     */
    BrushImpl* br = (BrushImpl*)b;
    Coord w;
    char* dashes;
    long ndashes;
    br->info(w, dashes, ndashes);
    short p = short(inline_to_pixels(w));
    XDisplay* dpy = xdisplay_;
    GC gc = xgc_;
    if (dashes == nil) {
	XSetLineAttributes(dpy, gc, p, LineSolid, CapButt, JoinMiter);
    } else {
	XSetLineAttributes(dpy, gc, p, LineOnOffDash, CapButt, JoinMiter);
	XSetDashes(dpy, gc, 0, dashes, int(ndashes));
    }
}

//+ XPainterImpl(PainterObj::brush_attr?)
BrushRef XPainterImpl::_c_brush_attr() {
    return Brush::_duplicate(brush_);
}

//+ XPainterImpl(PainterObj::color_attr=c)
void XPainterImpl::_c_color_attr(Color_in c) {
    if (same(c, color_)) {
	return;
    }
    flush_text();
    Fresco::unref(color_);
    color_ = Color::_duplicate(c);
    // need a table here from Color -> pixel
    XColor xc;
    Color::Intensity r, g, b;
    c->rgb(r, g, b);
    xc.red = ColorImpl::to_short(r);
    xc.green = ColorImpl::to_short(g);
    xc.blue = ColorImpl::to_short(b);
    screen_->find_color(*window_->visual(), xc);
    XSetForeground(xdisplay_, xgc_, xc.pixel);
}

//+ XPainterImpl(PainterObj::color_attr?)
ColorRef XPainterImpl::_c_color_attr() {
    return Color::_duplicate(color_);
}

//+ XPainterImpl(PainterObj::font_attr=f)
void XPainterImpl::_c_font_attr(Font_in f) {
    if (same(f, fontinfo_->font)) {
	return;
    }
    XPainterImpl::FontInfo* info = open_font(f);
    if (info->charsize != fontinfo_->charsize) {
	flush_text();
    }
    info->age = 0;
    add_item();
    item_->font = info->xfont;
    fontinfo_ = info;
}

//+ XPainterImpl(PainterObj::font_attr?)
FontRef XPainterImpl::_c_font_attr() {
    return Font::_duplicate(fontinfo_->font);
}

//+ XPainterImpl(PainterObj::fill)
void XPainterImpl::fill() {
    int n = int(cur_point_ - point_);
    if (n <= 2) {
	return;
    }
    XDisplay* dpy = xdisplay_;
    XDrawable d = xdrawable_;
    GC gc = xgc_;
    XPoint* xp = point_;
    if (n == 5 && cur_subpath_ == subpath_ && rectangular(xp)) {
	int x = Math::min(xp[0].x, xp[2].x);
	int y = Math::min(xp[0].y, xp[2].y);
	int w = Math::abs(xp[2].x - xp[0].x);
	int h = Math::abs(xp[2].y - xp[0].y);
	XFillRectangle(dpy, d, gc, x, y, w, h);
    } else {
	XFillPolygon(dpy, d, gc, xp, n, Complex, CoordModeOrigin);
    }
}

//+ XPainterImpl(PainterObj::stroke)
void XPainterImpl::stroke() {
    XDisplay* dpy = xdisplay_;
    XDrawable d = xdrawable_;
    GC gc = xgc_;
    for (SubPathInfo* sp = subpath_; sp <= cur_subpath_; sp++) {
	int n = sp->end - sp->start;
	if (n <= 0) {
	    /* not terminated, stroke to the end */
	    n = int((cur_point_ - point_) + sp->start);
	}
	XPoint* xp = point_ + sp->start;
	if (n == 2) {
	    XDrawLine(dpy, d, gc, xp[0].x, xp[0].y, xp[1].x, xp[1].y);
	} else if (n == 5 && rectangular(xp)) {
	    int x = Math::min(xp[0].x, xp[2].x);
	    int y = Math::min(xp[0].y, xp[2].y);
	    int w = Math::abs(xp[2].x - xp[0].x);
	    int h = Math::abs(xp[2].y - xp[0].y);
	    XDrawRectangle(dpy, d, gc, x, y, w, h);
	} else if (n > 2) {
	    XDrawLines(dpy, d, gc, xp, n, CoordModeOrigin);
	}
    }
}

//+ XPainterImpl(PainterObj::character)
void XPainterImpl::character(CharCode ch, Coord width, Coord x, Coord y) {
    if (transformed_) {
	if (!is_translation(matrix_)) {
	    stencil(fontinfo_->font->bitmap(ch), x, y);
	    return;
	} else {
	    Vertex v;
	    v.x = x;
	    v.y = y;
	    v.z = 0;
	    matrix_->transform(v);
	    x = v.x;
	    y = v.y;
	}
    }
    if (!Math::equal(y, ty_, float(1e-2)) ||
	char_ + fontinfo_->charsize >= array_end(chars_)
    ) {
	flush_text();
    }
    if (char_ == chars_) {
	tx0_ = inline_to_pixels(x);
	ty0_ = pheight_ - inline_to_pixels(y);
	tx_ = x;
	ty_ = y;
    } else {
	PixelCoord px = inline_to_pixels(x);
	PixelCoord ptx = inline_to_pixels(tx_);
	int delta = XCoord(px - ptx);
	if (delta != 0) {
	    add_item();
	    item_->delta = delta;
	}
    }
    item_->nchars += fontinfo_->charsize;
    tx_ = x + width;
    add_char(ch);
}

//+ XPainterImpl(PainterObj::image)
void XPainterImpl::image(Raster_in r, Coord x, Coord y) {
    RasterImpl* raster = RasterImpl::_narrow(r);
    if (raster == nil) {
	/*
	 * Not a known kind of image -- should extract colors
	 * with peek and build a RasterImpl.
	 */
	return;
    }

    // Before calling RasterImpl::lookup, we remove the translation
    // part of the transform. We'll add it afterwards.
    Vertex v;
    v.x = 0; v.y = 0; v.z = 0;
    matrix_->transform(v);
    TransformRef t;
    if (v.x == 0 && v.y == 0 && v.z == 0) {
	Fresco::ref(matrix_);
	t = matrix_;
    } else {
	TransformObj::Matrix m;
	matrix_->store_matrix(m);
	t = new TransformImpl(m);
	v.x = - v.x;
	v.y = - v.y;
	v.z = - v.z;
	t->translate(v);
    }

    RasterImpl::PerScreenData* psd = raster->lookup(window_, t);
    if (psd == nil || psd->pixmap == nil) {
	return;
    }
    XDisplay* dpy = xdisplay_;

    v.x = x; v.y = y; v.z = 0;
    matrix_->transform(v);

    PixelCoord px, py;
    px = Math::round(inline_to_pixels(v.x) + psd->origin_x);
    py = Math::round(pheight_ - inline_to_pixels(v.y) + psd->origin_y);

    /*
     * We assume that graphics exposures are off in the gc.
     */
    XCopyArea(
	dpy, psd->pixmap, xdrawable_, xgc_,
	0, 0, psd->pwidth, psd->pheight, int(px), int(py)
    );
}

//+ XPainterImpl(PainterObj::stencil)
void XPainterImpl::stencil(Raster_in r, Coord x, Coord y) {
    RasterImpl* raster = RasterImpl::_narrow(r);
    if (raster == nil) {
	/*
	 * Not a known kind of image -- should extract colors
	 * with peek and build a RasterImpl.
	 */
	return;
    }

    // Before calling RasterImpl::lookup, we remove the translation
    // part of the transform. We'll add it afterwards.
    Vertex v;
    v.x = 0; v.y = 0; v.z = 0;
    matrix_->transform(v);
    TransformRef t;
    if (v.x == 0 && v.y == 0 && v.z == 0) {
	t = matrix_;
    } else {
	TransformObj::Matrix m;
	matrix_->store_matrix(m);
	t = new TransformImpl(m);
	v.x = - v.x;
	v.y = - v.y;
	v.z = - v.z;
	t->translate(v);
    }

    RasterImpl::PerScreenData* psd = raster->lookup(window_, t);
    if (psd == nil || psd->pixmap == nil) {
	return;
    }
    XDisplay* dpy = xdisplay_;

    v.x = x; v.y = y; v.z = 0;
    matrix_->transform(v);

    PixelCoord px = Math::round(inline_to_pixels(v.x) + psd->origin_x);
    PixelCoord py = Math::round(
	pheight_ - inline_to_pixels(v.y) + psd->origin_y
    );

    XGCValues gcv;
    unsigned long valuemask = 0;

    valuemask |= GCFunction;
    gcv.function = GXand;
    valuemask |= GCForeground;
    gcv.foreground = 0;
    valuemask |= GCBackground;
    gcv.background = AllPlanes;
    valuemask |= GCGraphicsExposures;
    gcv.graphics_exposures = False;

    XDrawable d = xdrawable_;
    GC xgc = XCreateGC(dpy, d, valuemask, &gcv);
    XCopyGC(dpy, xgc_, GCClipMask, xgc);

    XCopyPlane(
	dpy, psd->pixmap, d, xgc,
	0, 0, psd->pwidth, psd->pheight, XCoord(px), XCoord(py), 1
    );

    gcv.function = GXxor;
    gcv.background = 0;
    valuemask &= ~GCGraphicsExposures;
    XChangeGC(dpy, xgc, valuemask, &gcv);
    XCopyGC(dpy, xgc_, GCForeground, xgc);
    XCopyPlane(
	dpy, psd->pixmap, d, xgc,
	0, 0, psd->pwidth, psd->pheight, int(px), int(py), 1
    );
    XFreeGC(dpy, xgc);
}

static inline void restrict(PixelCoord& c, PixelCoord a, PixelCoord b) {
    if (c < a) {
	c = a;
    } else if (c > b) {
	c = b;
    }
}

void XPainterImpl::set_clip() {
    PixelCoord left = inline_to_pixels(clipping_->lower_.x);
    PixelCoord bottom = inline_to_pixels(clipping_->lower_.y);
    PixelCoord right = inline_to_pixels(clipping_->upper_.x);
    PixelCoord top = inline_to_pixels(clipping_->upper_.y);
    restrict(left, 0, pwidth_);
    restrict(bottom, 0, pheight_);
    restrict(right, 0, pwidth_);
    restrict(top, 0, pheight_);
    xclip_.x = XCoord(left);
    xclip_.y = XCoord(pheight_ - top);
    xclip_.width = XCoord(right - left);
    xclip_.height = XCoord(top - bottom);
    XSetClipRectangles(xdisplay_, xgc_, 0, 0, &xclip_, 1, YXBanded);
}

void XPainterImpl::reset_clip() {
    if (clipping_ == nil) {
	XSetClipMask(xdisplay_, xgc_, None);
    } else {
	set_clip();
    }
}

/*
 * Prepare the painter for X drawing by allocating a graphics context.
 */

void XPainterImpl::prepare(Boolean double_buffered) {
    cleanup();
    double_buffered_ = double_buffered;
    XGCValues gcv;
    gcv.graphics_exposures = False;
    gcv.fill_style = FillSolid;
    xfrontbuffer_ = window_->xwindow();
    if (double_buffered) {
	xfrontgc_ = XCreateGC(
	    xdisplay_, xfrontbuffer_, GCGraphicsExposures, &gcv
	);
	xbackbuffer_ = XCreatePixmap(
	    xdisplay_, xfrontbuffer_,
	    XCoord(window_->pwidth()), XCoord(window_->pheight()),
	    int(window_->visual()->depth)
	);
	xdrawable_ = xbackbuffer_;
    } else {
	xfrontbuffer_ = window_->xwindow();
	xfrontgc_ = nil;
	xbackbuffer_ = nil;
	xdrawable_ = xfrontbuffer_;
    }
    xgc_ = XCreateGC(
	xdisplay_, xdrawable_, GCGraphicsExposures | GCFillStyle, &gcv
    );
    pwidth_ = window_->pwidth();
    pheight_ = window_->pheight();
    Fresco::unref(brush_);
    brush_ = nil;
    Fresco::unref(color_);
    color_ = nil;
    Fresco::unref(fontinfo_->font);
    fontinfo_->font = nil;
}

void XPainterImpl::frontbuffer() {
    xdrawable_ = xfrontbuffer_;
}

void XPainterImpl::backbuffer() {
    if (double_buffered_) {
	xdrawable_ = xbackbuffer_;
    }
}

void XPainterImpl::swapbuffers() {
    flush_text();
    if (double_buffered_) {
	XCopyArea(
	    xdisplay_, xbackbuffer_, xfrontbuffer_, xfrontgc_,
	    xclip_.x, xclip_.y, xclip_.width, xclip_.height, xclip_.x, xclip_.y
	);
    }
}

void XPainterImpl::init_fonts() {
    FontInfo* i;
    for (i = fontinfos_; i < array_end(fontinfos_); i++) {
	i->font = nil;
	i->charsize = 0;
	i->age = 0;
    }
    fontinfo_ = fontinfos_;
}

void XPainterImpl::init_items() {
    char_ = chars_;
    item_ = items_;
    item_->chars = chars_;
    item_->nchars = 0;
    item_->delta = 0;
    item_->font = None;
}

XPainterImpl::FontInfo* XPainterImpl::open_font(FontRef f) {
    FontInfo* i = find_font(f);
    if (i->font != nil) {
	return i;
    }
    /*
     * Should use narrow
     */
    FontImpl* fi = (FontImpl*)f;
    i->font = fi;
    XFontStruct* xf = fi->xfont();
    if (xf->min_byte1 > 0 || xf->max_byte1 > 0) {
	i->charsize = 2;
    } else {
	i->charsize = 1;
    }
    if (fi->xdisplay() == xdisplay_) {
	i->xfont = xf->fid;
	i->scaled = !Math::equal(fi->to_coord(1), to_coord(1), 0.001f);
    } else {
	/* Use temporary variable to workaround DEC CXX bug */
	CharString fname = fi->name();
	CharStringBuffer s(fname);
	i->xfont = XLoadFont(xdisplay_, s.string());
	i->scaled = true;
    }
    return i;
}

XPainterImpl::FontInfo* XPainterImpl::find_font(FontRef f) {
    FontInfo* i = nil;
    FontInfo* oldest = nil;
    FontInfo* cur = fontinfo_;
    for (;;) {
	++cur;
	if (cur >= array_end(fontinfos_)) {
	    cur = fontinfos_;
	}
	if (cur == fontinfo_) {
	    break;
	}
	if (i == nil) {
	    if (is_nil(cur->font)) {
		oldest = cur;
	    } else {
		if (f->equal(cur->font)) {
		    i = cur;
		} else if (
		    oldest == nil || (
			is_not_nil(oldest->font) && cur->age > oldest->age
		    )
		) {
		    oldest = cur;
		}
	    }
	}
	cur->age += 1;
    }
    if (i == nil) {
	i = oldest;
	if (i->font != nil) {
	    Fresco::unref(i->font);
	    i->font = nil;
	}
    }
    return i;
}

void XPainterImpl::close_fonts() {
    FontInfo* i;
    for (i = fontinfos_; i < array_end(fontinfos_); i++) {
	if (xdisplay_ != nil &&
	    i->font != nil && i->font->xdisplay() != xdisplay_
	) {
	    XUnloadFont(xdisplay_, i->xfont);
	}
	i->xfont = nil;
    }
}

void XPainterImpl::cleanup() {
    if (xdisplay_ != nil) {
	if (xgc_ != nil) {
	    XFreeGC(xdisplay_, xgc_);
	}
	if (double_buffered_) {
	    if (xfrontgc_ != nil) {
		XFreeGC(xdisplay_, xfrontgc_);
	    }
	    if (xbackbuffer_ != nil) {
		XFreePixmap(xdisplay_, xbackbuffer_);
	    }
	}
    }
}

XPoint* XPainterImpl::next_point() {
    if (cur_point_ == end_point_) {
	long old_size = cur_point_ - point_;
	long new_size = old_size + old_size;
	XPoint* new_path = new XPoint[new_size];
	for (long i = 0; i < old_size; i++) {
	    new_path[i] = point_[i];
	}
	delete point_;
	point_ = new_path;
	cur_point_ = point_ + old_size;
	end_point_ = point_ + new_size;
    }
    XPoint* xp = cur_point_;
    cur_point_ = xp + 1;
    return xp;
}

XPainterImpl::SubPathInfo* XPainterImpl::next_subpath() {
    ++cur_subpath_;
    if (cur_subpath_ == end_subpath_) {
	long old_size = cur_subpath_ - subpath_;
	long new_size = old_size + old_size;
	SubPathInfo* new_subpath = new SubPathInfo[new_size];
	for (long i = 0; i < old_size; i++) {
	    new_subpath[i] = subpath_[i];
	}
	delete subpath_;
	subpath_ = new_subpath;
	cur_subpath_ = subpath_ + old_size;
	end_subpath_ = subpath_ + new_size;
    }
    return cur_subpath_;
}

Boolean XPainterImpl::rectangular(XPoint* p) {
    int x0 = p[0].x, y0 = p[0].y;
    if (x0 == p[4].x && y0 == p[4].y) {
	int x1 = p[1].x, y1 = p[1].y;
	int x2 = p[2].x, y2 = p[2].y;
	int x3 = p[3].x, y3 = p[3].y;
	return (
	    (x0 == x1 && y1 == y2 && x2 == x3 && y3 == y0) ||
	    (x0 == x3 && y3 == y2 && x2 == x1 && y1 == y0)
	);
    }
    return false;
}

void XPainterImpl::curve_pt(
    Coord px, Coord py, Coord x, Coord y,
    Coord x1, Coord y1, Coord x2, Coord y2
) {
    if (straight(px, py, x1, y1, x2, y2, x, y)) {
	XPoint* xp = next_point();
	xp->x = short(inline_to_pixels(x));
	xp->y = short(pheight_ - inline_to_pixels(y));
    } else {
	Coord xx = mid(x1, x2);
	Coord yy = mid(y1, y2);
	Coord x11 = mid(px, x1);
	Coord y11 = mid(py, y1);
	Coord x22 = mid(x2, x);
	Coord y22 = mid(y2, y);
	Coord x12 = mid(x11, xx);
	Coord y12 = mid(y11, yy);
	Coord x21 = mid(xx, x22);
	Coord y21 = mid(yy, y22);
	Coord cx = mid(x12, x21);
	Coord cy = mid(y12, y21);

	curve_pt(px, py, cx, cy, x11, y11, x12, y12);
	curve_pt(cx, cy, x, y, x21, y21, x22, y22);
    }
}

/*
 * Allocate another XTextItem in the text buffer.
 */

void XPainterImpl::add_item() {
    XTextItem* i = item_;
    if (i->nchars != 0) {
	++i;
	if (i >= array_end(items_)) {
	    flush_text();
	    i = item_;
	} else {
	    item_ = i;
	}
	i->chars = char_;
	i->nchars = 0;
	i->delta = 0;
	i->font = None;
    }
}

/*
 * Add a character code into the text buffer.
 */

void XPainterImpl::add_char(CharCode ch) {
    char* cp = char_;
    switch (fontinfo_->charsize) {
    case 4:
	*cp++ = char((ch & 0xff000000) >> 24);
	/* goto case 3 */
    case 3:
	*cp++ = char((ch & 0xff0000) >> 16);
	/* goto case 2 */
    case 2:
	*cp++ = char((ch & 0xff00) >> 8);
	/* goto case 1 */
    case 1:
	*cp++ = char(ch & 0xff);
    }
    char_ = cp;
}

/*
 * Force drawing of any buffered text.
 */

void XPainterImpl::flush_text() {
    if (char_ == chars_) {
	return;
    }
    int n = item_ - items_ + 1;
    XDisplay* dpy = xdisplay_;
    XDrawable d = xdrawable_;
    GC gc = xgc_;
    XCoord x = XCoord(tx0_);
    XCoord y = XCoord(ty0_);
    switch (fontinfo_->charsize) {
    case 1:
	XDrawText(dpy, d, gc, x, y, items_, n);
	break;
    case 2:
	XDrawText16(dpy, d, gc, x, y, (XTextItem16*)items_, n);
	break;
    }
    init_items();
    item_->font = fontinfo_->xfont;
}

/*
 * DrawingKit -- create drawing objects
 */

declareTable(DrawingKitColorTable,CharStringRef,ColorRef)
implementTable(DrawingKitColorTable,CharStringRef,ColorRef)

declareTable(DrawingKitFontTable,CharStringRef,FontRef)
implementTable(DrawingKitFontTable,CharStringRef,FontRef)

DrawingKitImpl::DrawingKitImpl(DisplayImpl* d) {
    Fresco::ref(d);
    display_ = d;
    colors_ = new DrawingKitColorTable(20);
    fonts_ = new DrawingKitFontTable(20);
    foreground_str_ = Fresco::string_ref("foreground");
    Foreground_str_ = Fresco::string_ref("Foreground");
    background_str_ = Fresco::string_ref("background");
    Background_str_ = Fresco::string_ref("Background");
    font_str_ = Fresco::string_ref("font");
    Font_str_ = Fresco::string_ref("Font");
}

DrawingKitImpl::~DrawingKitImpl() {
    Fresco::unref(display_);
    delete colors_;
    delete fonts_;
}

//+ DrawingKitImpl(FrescoObject::=object_.)
Long DrawingKitImpl::ref__(Long references) {
    return object_.ref__(references);
}
Tag DrawingKitImpl::attach(FrescoObject_in observer) {
    return object_.attach(observer);
}
void DrawingKitImpl::detach(Tag attach_tag) {
    object_.detach(attach_tag);
}
void DrawingKitImpl::disconnect() {
    object_.disconnect();
}
void DrawingKitImpl::notify_observers() {
    object_.notify_observers();
}
void DrawingKitImpl::update() {
    object_.update();
}
//+

//+ DrawingKitImpl(DrawingKit::style)
StyleObjRef DrawingKitImpl::_c_style() {
    return display_->_c_style();
}

//+ DrawingKitImpl(DrawingKit::simple_brush)
BrushRef DrawingKitImpl::_c_simple_brush(Coord width) {
    return new BrushImpl(width);
}

//+ DrawingKitImpl(DrawingKit::dither_brush)
BrushRef DrawingKitImpl::_c_dither_brush(Coord width, Long pattern) {
    return new BrushImpl(pattern, width);
}

//+ DrawingKitImpl(DrawingKit::patterned_brush)
BrushRef DrawingKitImpl::_c_patterned_brush(Coord width, const DrawingKit::Data& pattern) {
    return new BrushImpl(pattern._buffer, pattern._length, width);
}

//+ DrawingKitImpl(DrawingKit::color_rgb)
ColorRef DrawingKitImpl::_c_color_rgb(Color::Intensity r, Color::Intensity g, Color::Intensity b) {
    return new ColorImpl(r, g, b);
}

//+ DrawingKitImpl(DrawingKit::find_color)
ColorRef DrawingKitImpl::_c_find_color(CharString_in name) {
    ColorRef c = nil;
    if (!colors_->find(c, name)) {
	XColor xc;
	CharStringBuffer cs(name);
	XDisplay* dpy = display_->xdisplay();
	if (XParseColor(
	    dpy, DefaultColormap(dpy, DefaultScreen(dpy)), cs.string(), &xc
	)) {
	    float range = float(0xffff);
	    Color::Intensity r = float(xc.red) / range;
	    Color::Intensity g = float(xc.green) / range;
	    Color::Intensity b = float(xc.blue) / range;
	    c = new ColorImpl(r, g, b);
	    colors_->insert(name, c);
	}
    }
    return Color::_duplicate(c);
}

//+ DrawingKitImpl(DrawingKit::resolve_color)
ColorRef DrawingKitImpl::_c_resolve_color(StyleObj_in s, CharString_in name) {
    ColorRef c = nil;
    StyleValue a = s->resolve(name);
    if (is_not_nil(a)) {
	CharString v;
	if (a->read_string(v)) {
	    c = _c_find_color(v);
	}
    }
    return c;
}

//+ DrawingKitImpl(DrawingKit::foreground)
ColorRef DrawingKitImpl::_c_foreground(StyleObj_in s) {
    return resolve_colors(s, foreground_str_, Foreground_str_);
}

//+ DrawingKitImpl(DrawingKit::background)
ColorRef DrawingKitImpl::_c_background(StyleObj_in s) {
    return resolve_colors(s, background_str_, Background_str_);
}

ColorRef DrawingKitImpl::resolve_colors(
    StyleObjRef s, CharString_in name1, CharString_in name2
) {
    ColorRef c = _c_resolve_color(s, name1);
    if (is_nil(c)) {
	c = _c_resolve_color(s, name2);
    }
    return c;
}

//+ DrawingKitImpl(DrawingKit::find_font)
FontRef DrawingKitImpl::_c_find_font(CharString_in fullname) {
    FontRef f = nil;
    if (!fonts_->find(f, fullname)) {
	CharStringBuffer cs(fullname);
	int n;
	char** names = XListFonts(display_->xdisplay(), cs.string(), 1, &n);
	if (n == 1) {
	    f = new FontImpl(display_, names[0]);
	    fonts_->insert(fullname, f);
	}
	XFreeFontNames(names);
    }
    return Font::_duplicate(f);
}

//+ DrawingKitImpl(DrawingKit::find_font_match)
FontRef DrawingKitImpl::_c_find_font_match(CharString_in family, CharString_in style, Coord ptsize) {
    return nil;
}

//+ DrawingKitImpl(DrawingKit::resolve_font)
FontRef DrawingKitImpl::_c_resolve_font(StyleObj_in s, CharString_in name) {
    FontRef f = nil;
    StyleValue a = s->resolve(name);
    if (is_not_nil(a)) {
	CharString v;
	if (a->read_string(v)) {
	    f = _c_find_font(v);
	}
    }
    return f;
}

//+ DrawingKitImpl(DrawingKit::default_font)
FontRef DrawingKitImpl::_c_default_font(StyleObj_in s) {
    FontRef f = _c_resolve_font(s, font_str_);
    if (is_nil(f)) {
	f = _c_resolve_font(s, Font_str_);
    }
    return f;
}

//+ DrawingKitImpl(DrawingKit::bitmap_file)
RasterRef DrawingKitImpl::_c_bitmap_file(CharString_in filename) {
    if (is_nil(filename)) {
	return nil;
    }
    CharStringBuffer cs(filename);
    unsigned int width, height;
    int x, y;
    Pixmap pixmap;

    if (BitmapOpenFailed == XReadBitmapFile(
	display_->xdisplay(), DefaultRootWindow(display_->xdisplay()),
	cs.string(), &width, &height, &pixmap, &x, &y
    )) {
	return nil;
    }
    if (x == -1) {
	x = 0;
    }
    if (y == -1) {
	y = height;
    }
    DrawingKit::Data data;
    data._buffer = nil;
    RasterBitmap* rb = new RasterBitmap(data, height, width, height - y, x, 0);
    rb->read_drawable(display_->xdisplay(), pixmap, 0, 0);
    XFreePixmap(display_->xdisplay(), pixmap);
    return rb;
}

//+ DrawingKitImpl(DrawingKit::bitmap_data)
RasterRef DrawingKitImpl::_c_bitmap_data(const DrawingKit::Data& data, Raster::Index rows, Raster::Index columns, Raster::Index origin_row, Raster::Index origin_column) {
    return new RasterBitmap(data, rows, columns, origin_row, origin_column);
}

//+ DrawingKitImpl(DrawingKit::bitmap_char)
RasterRef DrawingKitImpl::_c_bitmap_char(Font_in f, CharCode c) {
    FontImpl* fi = (FontImpl*)f;
    return fi->bitmap(c);
}

//+ DrawingKitImpl(DrawingKit::raster_tiff)
RasterRef DrawingKitImpl::_c_raster_tiff(CharString_in filename) {
    return nil;
}

//+ DrawingKitImpl(DrawingKit::identity_transform)
TransformObjRef DrawingKitImpl::_c_identity_transform() {
    return new TransformImpl;
}

//+ DrawingKitImpl(DrawingKit::transform)
TransformObjRef DrawingKitImpl::_c_transform(TransformObj::Matrix m) {
    return new TransformImpl(m);
}

//+ DrawingKitImpl(DrawingKit::printer)
PainterObjRef DrawingKitImpl::_c_printer(CharString_in output) {
    return nil;
}
