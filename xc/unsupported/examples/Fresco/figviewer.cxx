#include <X11/Fresco/figures.h>
#include <X11/Fresco/fresco.h>
#include <X11/Fresco/layouts.h>
#include <X11/Fresco/widgets.h>
#include <X11/Fresco/Impls/viewers.h>
#include <X11/Fresco/Impls/transform.h>
#include <X11/Fresco/Impls/region.h>
#include <stdio.h>
#include <stdlib.h>

class PageFigure : public GlyphImpl { 
public:
    PageFigure(Coord width, Coord height);
    virtual ~PageFigure();

    void request(Glyph::Requisition& r);
    void extension(const Glyph::AllocationInfo& a, RegionRef r);
    void draw(GlyphTraversalRef t);
private:
    Coord w_, h_;
};

PageFigure::PageFigure(Coord w, Coord h) {
    w_ = w;
    h_ = h;
}

PageFigure::~PageFigure () {}

void PageFigure::draw(GlyphTraversalRef) {}

void PageFigure::request(Glyph::Requisition& r) {
    r.x.natural = r.x.maximum = r.x.minimum = w_;
    r.y.natural = r.y.maximum = r.y.minimum = h_;
    r.x.align = r.y.align = 0.5;
    r.z.natural = 0;
}

void PageFigure::extension(const Glyph::AllocationInfo& a, RegionRef r) {
    RegionImpl region;
    region.lower_.x = 0;
    region.lower_.y = 0;
    region.lower_.z = 0;
    region.upper_.x = w_;
    region.upper_.y = h_;
    region.upper_.z = 0;
    region.xalign_ = region.yalign_ = 0.5;
    region.zalign_ = 0;
    if (is_not_nil(a.transform)) {
	region.transform(a.transform);
    }
    r->copy(&region);
}

class FigureViewer : public ViewerImpl {
public:
    FigureViewer(Fresco* f);
    ~FigureViewer();

    void need_resize();
    Boolean press(GlyphTraversalRef, EventRef);
    Boolean drag(GlyphTraversalRef, EventRef);
    Boolean release(GlyphTraversalRef, EventRef);
    void add(Coord x, Coord y);
protected:
    FigureKit figures_;
    FigureStyle def_style_;
    Glyph root_, figroot_;
    Coord width_, height_;
    enum { move_tool, create_tool } curtool_;
    GlyphRef target_;
    Coord start_x_, start_y_;
};

FigureViewer::FigureViewer(Fresco* f) : ViewerImpl(f) {
    width_ = 300; height_ = 300;
    figures_ = f->figure_kit();
    def_style_ = figures_->default_style();
    LayoutKit layouts = f->layout_kit();

    figroot_ = figures_->group();
    Glyph g = new TransformAllocator(0.5, 0.5, 0, 0.5, 0.5, 0);
    g->body(figroot_);
    root_ = layouts->fixed(g, width_, height_);
    body(root_);
    figroot_->append(new PageFigure(width_ * 10, height_ * 10));
}

FigureViewer::~FigureViewer() { }

void FigureViewer::need_resize() { }

void FigureViewer::add(Coord x, Coord y) {
    static int toggle = 0;
    Glyph fig;
    if (++toggle%2) {
	const float r = 36;
	fig = figures_->circle(FigureKit::fill, def_style_, x, y, r);
    } else {
	const Coord hh=25, hw=25;  // half-height,width
	fig = figures_->rectangle(
	    FigureKit::stroke, def_style_, x-hw,y-hh, x+hw, y+hh
	);
    }
    Glyph::AllocationInfoList al;
    figroot_->allocations(al);
    if (al._length < 1) {
	fprintf(stderr, "%d allocations!\n", al._length);
	exit(1);
    }
    Glyph::AllocationInfo& ai = al._buffer[0];
    TransformObj figtr = fig->transform();
    figtr->postmultiply(ai.transform);
    figtr->invert();
    figroot_->append(fig);
    fig->need_redraw();
    fig->need_resize();
    Fresco::unref(ai.allocation);
}
    
Boolean FigureViewer::press(GlyphTraversalRef t, EventRef e) {
    const Coord slop = 4;
    start_x_ = e->pointer_x();
    start_y_ = e->pointer_y();
    GlyphTraversal::Operation orig_op;
    orig_op = t->swap_op(GlyphTraversal::pick_top);
    t->begin_trail(this);
    PainterObj p = t->painter();
    p->push_clipping();
    p->clip_rect(start_x_-slop, start_y_-slop, start_x_+slop, start_y_+slop);
    root_->traverse(t);  
    
    GlyphTraversal picked = t->picked();
    if (is_not_nil(picked)) {
	target_ = picked->_c_current_glyph();
	curtool_ = move_tool;
    } else {
	curtool_ = create_tool;
    }
    p->pop_clipping();
    t->end_trail();
    return true;
}

Boolean FigureViewer::drag(GlyphTraversalRef, EventRef e) {
    if (curtool_ == move_tool) {
	TransformObj tx = target_->transform();
	if (is_not_nil(tx)) {
	    Vertex v;
	    v.x = e->pointer_x() - start_x_;
	    v.y = e->pointer_y() - start_y_;
	    start_x_ = e->pointer_x();
	    start_y_ = e->pointer_y();
	    target_->need_redraw();
	    tx->translate(v);
	    target_->need_redraw();
	    target_->need_resize();
	}
    }
    return true;
}

Boolean FigureViewer::release(GlyphTraversalRef, EventRef) {
    if (curtool_ == create_tool) {
	add(start_x_, start_y_);
    } else {
	Fresco::unref(target_);
    }
    return true;
}

int main(int argc, char** argv) {
     Fresco* f = Fresco_open("Test", argc, argv);
     LayoutKit layouts = f->layout_kit();
     Viewer fv = new FigureViewer(f);
     f->main(fv, layouts->margin(fv, 10.0));
     Fresco::unref(f);
     return 0;
}
