/*
 * Copyright (c) 1987-91 Stanford University
 * Copyright (c) 1991-93 Silicon Graphics, Inc.
 * Copyright (c) 1993 Fujitsu, Ltd.
 *
 * Permission to use, copy, modify, distribute, and sell this software and 
 * its documentation for any purpose is hereby granted without fee, provided
 * that (i) the above copyright notices and this permission notice appear in
 * all copies of the software and related documentation, and (ii) the names
 * of Stanford, Silicon Graphics, and Fujitsu may not be used in any
 * advertising or publicity relating to the software without the specific,
 * prior written permission of Stanford, Silicon Graphics, and Fujitsu.
 * 
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY 
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  
 *
 * IN NO EVENT SHALL STANFORD, SILICON GRAPHICS, OR FUJITSU BE LIABLE FOR
 * ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF 
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE 
 * OF THIS SOFTWARE.
 */

/*
 * FigureKit -- structured graphics
 */

#include <X11/Fresco/drawing.h>
#include <X11/Fresco/figures.h>
#include <X11/Fresco/fresco.h>
#include <X11/Fresco/Impls/charstr.h>
#include <X11/Fresco/Impls/fobjects.h>
#include <X11/Fresco/Impls/glyphs.h>
#include <X11/Fresco/Impls/polyglyph.h>
#include <X11/Fresco/Impls/region.h>
#include <X11/Fresco/Impls/styles.h>
#include <X11/Fresco/Impls/transform.h>
#include <X11/Fresco/OS/math.h>
#include <X11/Fresco/OS/threads.h>
#include <string.h>

class FigureStyleImpl;

class FigureKitImpl : public FigureKitType {
public:
    FigureKitImpl(Fresco*);
    ~FigureKitImpl();

    //+ FigureKit::*
    /* FrescoObject */
    Long ref__(Long references);
    Tag attach(FrescoObjectRef observer);
    void detach(Tag attach_tag);
    void disconnect();
    void notify_observers();
    void update();
    /* FigureKit */
    FigureStyleRef _c_default_style();
    FigureStyleRef _c_new_style(StyleObjRef parent);
    GlyphRef _c_figure_root(GlyphRef child);
    GlyphRef _c_label(FigureStyleRef s, CharStringRef str);
    GlyphRef _c_point(FigureStyleRef s, Coord x, Coord y);
    GlyphRef _c_line(FigureStyleRef s, Coord x0, Coord y0, Coord x1, Coord y1);
    GlyphRef _c_rectangle(FigureKit::Mode m, FigureStyleRef s, Coord left, Coord bottom, Coord right, Coord top);
    GlyphRef _c_circle(FigureKit::Mode m, FigureStyleRef s, Coord x, Coord y, Coord r);
    GlyphRef _c_ellipse(FigureKit::Mode m, FigureStyleRef s, Coord x, Coord y, Coord r1, Coord r2);
    GlyphRef _c_open_bspline(FigureKit::Mode m, FigureStyleRef s, const FigureKit::Vertices& v);
    GlyphRef _c_closed_bspline(FigureKit::Mode m, FigureStyleRef s, const FigureKit::Vertices& v);
    GlyphRef _c_multiline(FigureKit::Mode m, FigureStyleRef s, const FigureKit::Vertices& v);
    GlyphRef _c_polygon(FigureKit::Mode m, FigureStyleRef s, const FigureKit::Vertices& v);
    GlyphRef _c_fitter(GlyphRef g);
    GlyphRef _c_group();
    //+
protected:
    SharedFrescoObjectImpl object_;
    Fresco* fresco_;
    FigureStyle style_;
};

class Figure : public GlyphImpl {
public:
    Figure(
	FigureKit::Mode, FigureStyleRef,
        Boolean closed, Boolean curved, long coords
    );
    ~Figure();

    TransformObjRef _c_transform(); //+ Glyph::transform
    void request(Glyph::Requisition& r); //+ Glyph::request
    void extension(const Glyph::AllocationInfo& a, RegionRef r); //+ Glyph::extension
    void draw(GlyphTraversalRef t); //+ Glyph::draw
    void pick(GlyphTraversalRef t); //+ Glyph::pick

    void add_point(Coord x, Coord y);
    void add_curve(Coord x, Coord y, Coord x1, Coord y1, Coord x2, Coord y2);
    void Bspline_move_to(
        Coord x, Coord y, Coord x1, Coord y1, Coord x2, Coord y2
    );
    void Bspline_curve_to(
        Coord x, Coord y, Coord x1, Coord y1, Coord x2, Coord y2
    );
    void get_bbox(RegionImpl*);
private:
    FigureKit::Mode mode_;
    TransformRef tx_;
    FigureStyleRef style_;
    Boolean closed_ : 1;
    Boolean curved_ : 1;
    long count_;
    Vertex* v_;
    Vertex vmin_;
    Vertex vmax_;
};

class PolyFigure : public PolyGlyph {
public:
    PolyFigure();
    ~PolyFigure();

    void request(Glyph::Requisition& r); //+ Glyph::request
    void extension(const Glyph::AllocationInfo& a, RegionRef r); //+ Glyph::extension
    void traverse(GlyphTraversalRef t); //+ Glyph::traverse
    TransformObjRef _c_transform(); //+ Glyph::transform
    void need_resize(); //+ Glyph::need_resize

    void child_allocation(long, Glyph::AllocationInfo& a);

    static void corners(
	TransformRef t, Coord& left, Coord& bottom, Coord& right, Coord& top
    );
protected:
    TransformRef tx_;
    Boolean bbox_cached_;
    RegionImpl* bbox_;

    void update_bbox();
};

class FigureLabel : public GlyphImpl {
public:
    FigureLabel(FigureStyleRef, CharStringRef);
    ~FigureLabel();

    void request(Glyph::Requisition& r); //+ Glyph::request
    void draw(GlyphTraversalRef t); //+ Glyph::draw
    void pick(GlyphTraversalRef t); //+ Glyph::pick
private:
    FigureStyleRef style_;
    CharString text_;
    Font::Info info_;
};

class FigureStyleImpl : public FigureStyleType {
public:
    FigureStyleImpl(Fresco*);
    ~FigureStyleImpl();

    //+ FigureStyle::*
    /* FrescoObject */
    Long ref__(Long references);
    Tag attach(FrescoObjectRef observer);
    void detach(Tag attach_tag);
    void disconnect();
    void notify_observers();
    void update();
    /* StyleObj */
    StyleObjRef _c_new_style();
    StyleObjRef _c_parent_style();
    void link_parent(StyleObjRef parent);
    void unlink_parent();
    Tag link_child(StyleObjRef child);
    void unlink_child(Tag link_tag);
    void merge(StyleObjRef s);
    CharStringRef _c_name();
    void _c_name(CharStringRef _p);
    void alias(CharStringRef s);
    Boolean is_on(CharStringRef name);
    StyleValueRef _c_bind(CharStringRef name);
    void unbind(CharStringRef name);
    StyleValueRef _c_resolve(CharStringRef name);
    StyleValueRef _c_resolve_wildcard(CharStringRef name, StyleObjRef start);
    Long match(CharStringRef name);
    void visit_aliases(StyleVisitorRef v);
    void visit_attributes(StyleVisitorRef v);
    void visit_styles(StyleVisitorRef v);
    void lock();
    void unlock();
    /* FigureStyle */
    ColorRef _c_background();
    void _c_background(ColorRef _p);
    BrushRef _c_brush_attr();
    void _c_brush_attr(BrushRef _p);
    FontRef _c_font_attr();
    void _c_font_attr(FontRef _p);
    ColorRef _c_foreground();
    void _c_foreground(ColorRef _p);
    //+
private:
    LockedFrescoObjectImpl object_;
    SharedStyleImpl style_;
    Color background_;
    Brush brush_;
    Font font_;
    Color foreground_;
};

FigureKitRef FrescoImpl::create_figure_kit() {
    return new FigureKitImpl(this);
}

FigureKitImpl::FigureKitImpl(Fresco* f) {
    fresco_ = f;
    style_ = new_style(f->drawing_kit()->style());
}

FigureKitImpl::~FigureKitImpl() { }

//+ FigureKitImpl(FrescoObject::=object_.)
Long FigureKitImpl::ref__(Long references) {
    return object_.ref__(references);
}
Tag FigureKitImpl::attach(FrescoObjectRef observer) {
    return object_.attach(observer);
}
void FigureKitImpl::detach(Tag attach_tag) {
    object_.detach(attach_tag);
}
void FigureKitImpl::disconnect() {
    object_.disconnect();
}
void FigureKitImpl::notify_observers() {
    object_.notify_observers();
}
void FigureKitImpl::update() {
    object_.update();
}
//+

//+ FigureKitImpl(FigureKit::default_style)
FigureStyleRef FigureKitImpl::_c_default_style() {
    return FigureStyle::_duplicate(FigureStyleRef(style_));
}

//+ FigureKitImpl(FigureKit::new_style)
FigureStyleRef FigureKitImpl::_c_new_style(StyleObjRef parent) {
    FigureStyleImpl* fs = new FigureStyleImpl(fresco_);
    fs->link_parent(parent);
    return fs;
}

//+ FigureKitImpl(FigureKit::figure_root)
GlyphRef FigureKitImpl::_c_figure_root(GlyphRef child) {
    GlyphRef g = new TransformAllocator(0.5, 0.5, 0.5, 0.5, 0.5, 0.5);
    g->body(child);
    return g;
}

//+ FigureKitImpl(FigureKit::label)
GlyphRef FigureKitImpl::_c_label(FigureStyleRef s, CharStringRef str) {
    return new FigureLabel(s, str);
}

//+ FigureKitImpl(FigureKit::point)
GlyphRef FigureKitImpl::_c_point(FigureStyleRef s, Coord x, Coord y) {
    return line(s, x, y, x, y);
}

//+ FigureKitImpl(FigureKit::line)
GlyphRef FigureKitImpl::_c_line(FigureStyleRef s, Coord x0, Coord y0, Coord x1, Coord y1) {
    Figure* f = new Figure(FigureKit::stroke, s, false, false, 2);
    f->add_point(x0, y0);
    f->add_point(x1, y1);
    return f;
}

//+ FigureKitImpl(FigureKit::rectangle)
GlyphRef FigureKitImpl::_c_rectangle(FigureKit::Mode m, FigureStyleRef s, Coord left, Coord bottom, Coord right, Coord top) {
    Figure* f = new Figure(m, s, true, false, 4);
    f->add_point(left, bottom);
    f->add_point(left, top);
    f->add_point(right, top);
    f->add_point(right, bottom);
    return f;
}

static const float magic = 0.5522847498307934; // 4/3 * (sqrt(2) - 1)

//+ FigureKitImpl(FigureKit::circle)
GlyphRef FigureKitImpl::_c_circle(FigureKit::Mode m, FigureStyleRef s, Coord x, Coord y, Coord r) {
    float r0 = magic * r;

    Figure* f = new Figure(m, s, true, true, 25);
    f->add_point(x + r, y);
    f->add_curve(x, y - r, x + r, y - r0, x + r0, y - r);
    f->add_curve(x - r, y, x - r0, y - r, x - r, y - r0);
    f->add_curve(x, y + r, x - r, y + r0, x - r0, y + r);
    f->add_curve(x + r, y, x + r0, y + r, x + r, y + r0);
    return f;
}

static const float p0 = 1.00000000;
static const float p1 = 0.89657547;   // cos 30 * sqrt(1 + tan 15 * tan 15)
static const float p2 = 0.70710678;   // cos 45
static const float p3 = 0.51763809;   // cos 60 * sqrt(1 + tan 15 * tan 15)
static const float p4 = 0.26794919;   // tan 15

//+ FigureKitImpl(FigureKit::ellipse)
GlyphRef FigureKitImpl::_c_ellipse(FigureKit::Mode m, FigureStyleRef s, Coord x, Coord y, Coord r1, Coord r2) {
    float px0 = p0 * r1, py0 = p0 * r2;
    float px1 = p1 * r1, py1 = p1 * r2;
    float px2 = p2 * r1, py2 = p2 * r2;
    float px3 = p3 * r1, py3 = p3 * r2;
    float px4 = p4 * r1, py4 = p4 * r2;

    Figure* f = new Figure(m, s, true, true, 25);
    f->add_point(x + r1, y);
    f->add_curve(x + px2, y + py2, x + px0, y + py4, x + px1, y + py3);
    f->add_curve(x, y + r2, x + px3, y + py1, x + px4, y + py0);
    f->add_curve(x - px2, y + py2, x - px4, y + py0, x - px3, y + py1);
    f->add_curve(x - r1, y, x - px1, y + py3, x - px0, y + py4);
    f->add_curve(x - px2, y - py2, x - px0, y - py4, x - px1, y - py3);
    f->add_curve(x, y - r2, x - px3, y - py1, x - px4, y - py0);
    f->add_curve(x + px2, y - py2, x + px4, y - py0, x + px3, y - py1);
    f->add_curve(x + r1, y, x + px1, y - py3, x + px0, y - py4);
    return f;
}

//+ FigureKitImpl(FigureKit::open_bspline)
GlyphRef FigureKitImpl::_c_open_bspline(FigureKit::Mode m, FigureStyleRef s, const FigureKit::Vertices& v) {
    long n = v._length;
    Vertex* vv = v._buffer;
    Figure* f = new Figure(m, s, false, true, (n + 2) * 3 + 1);
    f->Bspline_move_to(vv[0].x, vv[0].y, vv[0].x, vv[0].y, vv[0].x, vv[0].y);
    f->Bspline_curve_to(vv[0].x, vv[0].y, vv[0].x, vv[0].y, vv[1].x, vv[1].y);
    for (long i = 1; i < n - 1; ++i) {
        f->Bspline_curve_to(
	    vv[i].x, vv[i].y, vv[i-1].x, vv[i-1].y, vv[i+1].x, vv[i+1].y
	);
    }
    f->Bspline_curve_to(
	vv[n-1].x, vv[n-1].y, vv[n-2].x, vv[n-2].y, vv[n-1].x, vv[n-1].y
    );
    f->Bspline_curve_to(
	vv[n-1].x, vv[n-1].y, vv[n-1].x, vv[n-1].y, vv[n-1].x, vv[n-1].y
    );
    return f;
}

//+ FigureKitImpl(FigureKit::closed_bspline)
GlyphRef FigureKitImpl::_c_closed_bspline(FigureKit::Mode m, FigureStyleRef s, const FigureKit::Vertices& v) {
    long n = v._length;
    Vertex* vv = v._buffer;
    Figure* f = new Figure(m, s, true, true, n * 3 + 1);
    f->Bspline_move_to(
	vv[0].x, vv[0].y, vv[n-1].x, vv[n-1].y, vv[1].x, vv[1].y
    );
    for (long i = 1; i < n - 1; ++i) {
        f->Bspline_curve_to(
	    vv[i].x, vv[i].y, vv[i-1].x, vv[i-1].y, vv[i+1].x, vv[i+1].y
	);
    }
    f->Bspline_curve_to(
	vv[n-1].x, vv[n-1].y, vv[n-2].x, vv[n-2].y, vv[0].x, vv[0].y
    );
    f->Bspline_curve_to(
	vv[0].x, vv[0].y, vv[n-1].x, vv[n-1].y, vv[1].x, vv[1].y
    );
    return f;
}

//+ FigureKitImpl(FigureKit::multiline)
GlyphRef FigureKitImpl::_c_multiline(FigureKit::Mode m, FigureStyleRef s, const FigureKit::Vertices& v) {
    long n = v._length;
    Vertex* vv = v._buffer;
    Figure* f = new Figure(m, s, false, false, n);
    f->add_point(vv[0].x, vv[0].y);
    for (long i = 1; i < n; ++i) {
        f->add_point(vv[i].x, vv[i].y);
    }
    return f;
}

//+ FigureKitImpl(FigureKit::polygon)
GlyphRef FigureKitImpl::_c_polygon(FigureKit::Mode m, FigureStyleRef s, const FigureKit::Vertices& v) {
    long n = v._length;
    Vertex* vv = v._buffer;
    Figure* f = new Figure(m, s, true, false, n);
    f->add_point(vv[0].x, vv[0].y);
    for (long i = 1; i < n; ++i) {
        f->add_point(vv[i].x, vv[i].y);
    }
    return f;
}

//+ FigureKitImpl(FigureKit::fitter)
GlyphRef FigureKitImpl::_c_fitter(GlyphRef g) {
    /* unimplemented */
    return g;
}

//+ FigureKitImpl(FigureKit::group)
GlyphRef FigureKitImpl::_c_group() {
    return new PolyFigure;
}

/* class Figure */

Figure::Figure(
    FigureKit::Mode mode, FigureStyleRef style,
    Boolean closed, Boolean curved, long coords
) {
    mode_ = mode;
    tx_ = new TransformImpl;
    style_ = FigureStyle::_duplicate(style);
    closed_ = closed;
    curved_ = curved;
    count_ = 0;
    v_ = new Vertex[coords];
}

Figure::~Figure() {
    Fresco::unref(tx_);
    Fresco::unref(style_);
    delete v_;
}

void Figure::add_point(Coord x, Coord y) {
    if (count_ == 0) {
	vmin_.x = x;
	vmax_.x = x;
	vmin_.y = y;
	vmax_.y = y;
	vmin_.z = 0;
	vmax_.z = 0;
    } else {
        vmin_.x = Math::min(vmin_.x, x);
        vmax_.x = Math::max(vmax_.x, x);
        vmin_.y = Math::min(vmin_.y, y);
        vmax_.y = Math::max(vmax_.y, y);
    }
    Vertex* v = &v_[count_];
    v->x = x;
    v->y = y;
    ++count_;
}

void Figure::add_curve(
    Coord x, Coord y, Coord x1, Coord y1, Coord x2, Coord y2
) {
    add_point(x1, y1);
    add_point(x2, y2);
    add_point(x, y);
}

void Figure::Bspline_move_to(
    Coord x, Coord y, Coord x1, Coord y1, Coord x2, Coord y2
) {
    Coord p1x = (x + x + x1) / 3;
    Coord p1y = (y + y + y1) / 3;
    Coord p2x = (x + x + x2) / 3;
    Coord p2y = (y + y + y2) / 3;
    add_point((p1x + p2x) / 2, (p1y + p2y) / 2);
}

void Figure::Bspline_curve_to(
    Coord x, Coord y, Coord x1, Coord y1, Coord x2, Coord y2
) {
    Coord p1x = (x + x + x1) / 3;
    Coord p1y = (y + y + y1) / 3;
    Coord p2x = (x + x + x2) / 3;
    Coord p2y = (y + y + y2) / 3;
    Coord p3x = (x1 + x1 + x) / 3;
    Coord p3y = (y1 + y1 + y) / 3;
    add_curve((p1x + p2x) / 2, (p1y + p2y) / 2, p3x, p3y, p1x, p1y);
}

void Figure::get_bbox(RegionImpl* bbox) {
    bbox->lower_ = vmin_;
    bbox->upper_ = vmax_;
    bbox->xalign_ = 0;
    bbox->yalign_ = 0;
    bbox->zalign_ = 0;
}

//+ Figure(Glyph::transform)
TransformObjRef Figure::_c_transform() {
    return TransformObj::_duplicate(tx_);
}

//+ Figure(Glyph::request)
void Figure::request(Glyph::Requisition& r) {
    if (count_ > 0) {
	RegionImpl region;
	region.lower_ = vmin_;
	region.upper_ = vmax_;
	region.xalign_ = region.yalign_ = region.zalign_ = 0;
	Fresco::ref(tx_);
	region.transform(tx_);
	Coord x_lead = -region.lower_.x, x_trail = region.upper_.x;
	Coord y_lead = -region.lower_.y, y_trail = region.upper_.y;
	GlyphImpl::require_lead_trail(
	    r.x, x_lead, x_lead, x_lead, x_trail, x_trail, x_trail
	);
	GlyphImpl::require_lead_trail(
	    r.y, y_lead, y_lead, y_lead, y_trail, y_trail, y_trail
	);
    } else {
	r.x.defined = false;
	r.y.defined = false;
    }
}

//+ Figure(Glyph::extension)
void Figure::extension(const Glyph::AllocationInfo& a, RegionRef r) {
    RegionImpl region;
    if (count_ > 0) {
	TransformImpl t;
	if (is_not_nil(a.transform)) {
	    t.load(a.transform);
	}
	t.premultiply(tx_);

	region.lower_ = vmin_;
	region.upper_ = vmax_;
	region.xalign_ = region.yalign_ = region.zalign_ = 0;
	region.transform(&t);
    } else {
	region.lower_.x = 1e6;
	region.lower_.y = region.lower_.x;
	region.upper_.x = -region.lower_.x;
	region.upper_.y = -region.lower_.y;
    }
    r->copy(&region);
}

//+ Figure(Glyph::draw)
void Figure::draw(GlyphTraversalRef t) {
    if (count_ > 0) {
	PainterObj p = t->painter();
	p->push_matrix();
	p->transform(tx_);

	/*
	 * Should do bounding box culling here.  The bounding box is
	 * described in object coordinates by vmin_, vmax_.
	 * Then we should just do p->is_visible(bbox) for the test.
	 */

        p->begin_path();
        p->move_to(v_[0].x, v_[0].y);
        if (curved_) {
            for (long i = 1; i < count_; i += 3) {
		long i1 = i + 1;
		long i2 = i1 + 1;
                p->curve_to(
                    v_[i2].x, v_[i2].y, v_[i].x, v_[i].y, v_[i1].x, v_[i1].y
                );
            }
        } else {
            for (long i = 1; i < count_; ++i) {
                p->line_to(v_[i].x, v_[i].y);
            }
        }
        if (closed_) {
            p->close_path();
        }
	if (mode_ == FigureKit::stroke || mode_ == FigureKit::fill_stroke) {
	    p->brush_attr(style_->brush_attr());
	}
	if (mode_ == FigureKit::fill || mode_ == FigureKit::stroke) {
	    p->color_attr(style_->foreground());
	}
	switch (mode_) {
	case FigureKit::fill:
	    p->fill();
	    break;
	case FigureKit::stroke:
	    p->stroke();
	    break;
	case FigureKit::fill_stroke:
	    p->color_attr(style_->background());
	    p->fill();
	    p->color_attr(style_->foreground());
	    p->stroke();
	    break;
	}
	p->pop_matrix();
    }
}

/*
 * Picking just does a bounding box test for now.
 */

//+ Figure(Glyph::pick)
void Figure::pick(GlyphTraversalRef t) {
    if (count_ > 0) {
	PainterObj p = t->painter();
	p->push_matrix();
	p->transform(tx_);
	RegionImpl* bbox = new RegionImpl;
	get_bbox(bbox);
	bbox->transform(p->matrix());
	if (p->is_visible(bbox)) {
	    t->hit();
	}
	p->pop_matrix();
    }
}

/* class PolyFigure */

PolyFigure::PolyFigure() {
    tx_ = new TransformImpl;
    bbox_ = new RegionImpl;
    bbox_cached_ = false;
}

PolyFigure::~PolyFigure() {
    Fresco::unref(tx_);
    Fresco::unref(bbox_);
}

void PolyFigure::update_bbox() {
    if (!bbox_cached_) {
	long_int n = children_.count();
	if (n > 0) {
	    Glyph::AllocationInfo a;
	    a.transform = nil;
	    a.allocation = nil;
	    a.damage = nil;
	    children_.item(0)->child_->extension(a, bbox_);
	    RegionImpl region;
	    for (long_int i = 1; i < n; i++) {
		children_.item(i)->child_->extension(a, &region);
		bbox_->merge_union(&region);
	    }
	} else {
	    bbox_->lower_.x = 1e6;
	    bbox_->upper_.x = -bbox_->lower_.x;
	    bbox_->lower_.y = bbox_->lower_.x;
	    bbox_->upper_.y = -bbox_->lower_.y;
	}
	bbox_cached_ = true;
    }
}

//+ PolyFigure(Glyph::request)
void PolyFigure::request(Glyph::Requisition& r) {
    if (children_.count() != 0) {
	Glyph::AllocationInfo a;
	a.transform = nil;
	a.allocation = nil;
	a.damage = nil;
	RegionImpl region;
	extension(a, &region);
	Coord x_lead = -region.lower_.x, x_trail = region.upper_.x;
	Coord y_lead = -region.lower_.y, y_trail = region.upper_.y;
	GlyphImpl::require_lead_trail(
	    r.x, x_lead, x_lead, x_lead, x_trail, x_trail, x_trail
	);
	GlyphImpl::require_lead_trail(
	    r.y, y_lead, y_lead, y_lead, y_trail, y_trail, y_trail
	);
    } else {
	r.x.defined = false;
	r.y.defined = false;
    }
}

//+ PolyFigure(Glyph::extension)
void PolyFigure::extension(const Glyph::AllocationInfo& a, RegionRef r) {
    update_bbox();
    r->copy(bbox_);
    if (children_.count() != 0) {
	r->transform(tx_);
    }
}

//+ PolyFigure(Glyph::traverse)
void PolyFigure::traverse(GlyphTraversalRef t) {
    PainterObj p = t->painter();
    if (is_not_nil(p)) {
	p->push_matrix();
	p->transform(tx_);
	for (ListItr(PolyGlyphOffsetList) i(children_); i.more(); i.next()) {
	    t->traverse_child(i.cur(), nil);
	}
	p->pop_matrix();
    }
}

//+ PolyFigure(Glyph::transform)
TransformObjRef PolyFigure::_c_transform() {
    return TransformObj::_duplicate(tx_);
}

//+ PolyFigure(Glyph::need_resize)
void PolyFigure::need_resize() {
    bbox_cached_ = false;
    PolyGlyph::need_resize();
}

void PolyFigure::child_allocation(long, Glyph::AllocationInfo& a) {
    a.transform->premultiply(tx_);
}

/* class FigureLabel */

FigureLabel::FigureLabel(FigureStyleRef s, CharStringRef str) {
    style_ = FigureStyle::_duplicate(s);
    text_ = CharString::_duplicate(str);
    style_->font_attr()->string_info(text_, info_);
}

FigureLabel::~FigureLabel() {
    Fresco::unref(style_);
}

//+ FigureLabel(Glyph::request)
void FigureLabel::request(Glyph::Requisition& r) {
    Coord h = info_.font_ascent + info_.font_descent;
    Alignment a;
    if (Math::equal(h, float(0), float(1e-2))) {
	a = 0;
    } else {
	a = info_.font_descent / h;
    }
    GlyphImpl::require(r.x, info_.width, 0, 0, 0);
    GlyphImpl::require(r.y, h, 0, 0, a);
}

//+ FigureLabel(Glyph::draw)
void FigureLabel::draw(GlyphTraversalRef t) {
    PainterObj p = t->painter();
    p->color_attr(style_->foreground());
    Font f = style_->font_attr();
    p->font_attr(f);
    Vertex v;
    t->origin(v);
    Font::Info info;
    CharStringBuffer buf(text_);
    const char* cur = buf.string();
    const char* end = &cur[buf.length()];
    for (; cur < end; cur++) {
	f->char_info(*cur, info);
	p->character(*cur, info.width, v.x, v.y);
	v.x += info.width;
    }
}

//+ FigureLabel(Glyph::pick)
void FigureLabel::pick(GlyphTraversalRef) { }

/* class FigureStyleImpl */

FigureStyleImpl::FigureStyleImpl(Fresco* f) : style_(f) {
    ThreadKit t = f->thread_kit();
    object_.lock_ = t->lock();
    style_.style_ = this;
    style_.lock_ = object_.lock_;
    background_ = nil;
    brush_ = nil;
    font_ = nil;
    foreground_ = nil;
}

FigureStyleImpl::~FigureStyleImpl() { }

//+ FigureStyleImpl(FrescoObject::=object_.)
Long FigureStyleImpl::ref__(Long references) {
    return object_.ref__(references);
}
Tag FigureStyleImpl::attach(FrescoObjectRef observer) {
    return object_.attach(observer);
}
void FigureStyleImpl::detach(Tag attach_tag) {
    object_.detach(attach_tag);
}
void FigureStyleImpl::disconnect() {
    object_.disconnect();
}
void FigureStyleImpl::notify_observers() {
    object_.notify_observers();
}
void FigureStyleImpl::update() {
    object_.update();
}
//+

//+ FigureStyleImpl(StyleObj::=style_.)
StyleObjRef FigureStyleImpl::_c_new_style() {
    return style_._c_new_style();
}
StyleObjRef FigureStyleImpl::_c_parent_style() {
    return style_._c_parent_style();
}
void FigureStyleImpl::link_parent(StyleObjRef parent) {
    style_.link_parent(parent);
}
void FigureStyleImpl::unlink_parent() {
    style_.unlink_parent();
}
Tag FigureStyleImpl::link_child(StyleObjRef child) {
    return style_.link_child(child);
}
void FigureStyleImpl::unlink_child(Tag link_tag) {
    style_.unlink_child(link_tag);
}
void FigureStyleImpl::merge(StyleObjRef s) {
    style_.merge(s);
}
CharStringRef FigureStyleImpl::_c_name() {
    return style_._c_name();
}
void FigureStyleImpl::_c_name(CharStringRef _p) {
    style_._c_name(_p);
}
void FigureStyleImpl::alias(CharStringRef s) {
    style_.alias(s);
}
Boolean FigureStyleImpl::is_on(CharStringRef name) {
    return style_.is_on(name);
}
StyleValueRef FigureStyleImpl::_c_bind(CharStringRef name) {
    return style_._c_bind(name);
}
void FigureStyleImpl::unbind(CharStringRef name) {
    style_.unbind(name);
}
StyleValueRef FigureStyleImpl::_c_resolve(CharStringRef name) {
    return style_._c_resolve(name);
}
StyleValueRef FigureStyleImpl::_c_resolve_wildcard(CharStringRef name, StyleObjRef start) {
    return style_._c_resolve_wildcard(name, start);
}
Long FigureStyleImpl::match(CharStringRef name) {
    return style_.match(name);
}
void FigureStyleImpl::visit_aliases(StyleVisitorRef v) {
    style_.visit_aliases(v);
}
void FigureStyleImpl::visit_attributes(StyleVisitorRef v) {
    style_.visit_attributes(v);
}
void FigureStyleImpl::visit_styles(StyleVisitorRef v) {
    style_.visit_styles(v);
}
void FigureStyleImpl::lock() {
    style_.lock();
}
void FigureStyleImpl::unlock() {
    style_.unlock();
}
//+

//+ FigureStyleImpl(FigureStyle::background=c)
void FigureStyleImpl::_c_background(ColorRef c) {
    background_ = Color::_duplicate(c);
}

//+ FigureStyleImpl(FigureStyle::background?)
ColorRef FigureStyleImpl::_c_background() {
    ColorRef c = background_;
    if (is_nil(c)) {
	c = style_.fresco_->drawing_kit()->_c_background(this);
    }
    return Color::_duplicate(c);
}

//+ FigureStyleImpl(FigureStyle::brush_attr=b)
void FigureStyleImpl::_c_brush_attr(BrushRef b) {
    brush_ = Brush::_duplicate(b);
}

//+ FigureStyleImpl(FigureStyle::brush_attr?)
BrushRef FigureStyleImpl::_c_brush_attr() {
    BrushRef b = brush_;
    return Brush::_duplicate(b);
}

//+ FigureStyleImpl(FigureStyle::font_attr=f)
void FigureStyleImpl::_c_font_attr(FontRef f) {
    font_ = Font::_duplicate(f);
}

//+ FigureStyleImpl(FigureStyle::font_attr?)
FontRef FigureStyleImpl::_c_font_attr() {
    FontRef f = font_;
    if (is_nil(f)) {
	f = style_.fresco_->drawing_kit()->_c_default_font(this);
    }
    return Font::_duplicate(f);
}

//+ FigureStyleImpl(FigureStyle::foreground=c)
void FigureStyleImpl::_c_foreground(ColorRef c) {
    foreground_ = Color::_duplicate(c);
}

//+ FigureStyleImpl(FigureStyle::foreground?)
ColorRef FigureStyleImpl::_c_foreground() {
    ColorRef c = foreground_;
    if (is_nil(c)) {
	c = style_.fresco_->drawing_kit()->foreground(this);
    }
    return Color::_duplicate(c);
}