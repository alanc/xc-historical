/*
 * Example program using Painter subpaths.
 */

#include <X11/Fresco/drawing.h>
#include <X11/Fresco/figures.h>
#include <X11/Fresco/fresco.h>
#include <X11/Fresco/layouts.h>
#include <X11/Fresco/Impls/fresco-impl.h>
#include <X11/Fresco/Impls/glyphs.h>
#include <X11/Fresco/Impls/styles.h>
#include <X11/Fresco/OS/math.h>

class Rects : public GlyphImpl {
public:
    Rects(Coord width, Coord height, FigureKit::Mode, FigureStyleRef style);
    virtual ~Rects();

    void request(Glyph::Requisition& r); //+ Glyph::request
    void draw(GlyphTraversalRef t); //+ Glyph::draw
private:
    Coord width_, height_;
    FigureKit::Mode mode_;
    FigureStyle style_;
};

Rects::Rects(
    Coord width, Coord height, FigureKit::Mode mode, FigureStyleRef style
) {
    width_ = width;
    height_ = height;
    style_ = FigureStyle::_duplicate(style);
    mode_ = mode;
}

Rects::~Rects() { }

void Rects::request(Glyph::Requisition& r) {
    GlyphImpl::require_lead_trail(
	r.x, 0, 0, 0, width_, width_, width_
    );
    GlyphImpl::require_lead_trail(
	r.y, 0, 0, 0, height_, height_, height_
    );
}

void Rects::draw(GlyphTraversalRef t) {
    Vertex o;

    t->origin(o);

    PainterObj p = t->painter();

    // Draws two rectangles one inside the other.
    p->begin_path();
    p->move_to(o.x, o.y);
    p->line_to(o.x + width_, o.y);
    p->line_to(o.x + width_, o.y + height_);
    p->line_to(o.x, o.y + height_);
    p->close_path();

    Coord nw = width_ / 2;
    Coord nh = height_ / 2;
    o.x += nw / 2;
    o.y += nh / 2;
    // turn in the other direction
    p->move_to(o.x, o.y);
    p->line_to(o.x, o.y + nh);
    p->line_to(o.x + nw, o.y + nh);
    p->line_to(o.x + nw, o.y);
    p->close_path();
    
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
}

int main(int argc, char** argv) {
    Fresco* f = Fresco_open("Test", argc, argv);
    FigureKit figures = f->figure_kit();

    FigureStyle style = figures->new_style(figures->default_style());
    style->background(f->drawing_kit()->find_color(f->string_ref("blue")));

    Glyph group = f->layout_kit()->overlay();
    group->append(
	figures->circle(
	    FigureKit::fill, figures->default_style(), 100, 100, 50
	)
    );
    group->append(Glyph(new Rects(100, 100, FigureKit::fill_stroke, style)));
    f->main(nil, group);
    Fresco::unref(f);
    return 0;
}
