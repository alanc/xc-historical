#include <X11/Fresco/display.h>
#include <X11/Fresco/drawing.h>
#include <X11/Fresco/figures.h>
#include <X11/Fresco/fresco.h>
#include <X11/Fresco/layouts.h>
#include <X11/Fresco/Impls/action.h>
#include <X11/Fresco/Impls/glyphs.h>
#include <X11/Fresco/OS/thread.h>

#ifdef sgi

extern "C" {
    int sginap(long);
}

static Boolean delay(float seconds) {
    long n = long(100.0 * seconds + 0.5);
    if (n <= 2) {
	n = 3;
    }
    return sginap(n) == 0;
}

#else

#include <X11/Fresco/OS/types.h>
#include <sys/time.h>

extern "C" {
    int select(int, fd_set*, fd_set*, fd_set*, struct timeval*);
}

static Boolean delay(float seconds) {
    struct timeval tv;
    tv.tv_sec = time_t(seconds);
    tv.tv_usec = time_t(1000000.0 * (seconds - float(tv.tv_sec)) + 0.5);
    return select(0, nil, nil, nil, &tv) == 0;
}

#endif

static Option options[] = {
    { "-d", "*delta", Option::value },
    { "-d1", "*zoomer1*delta", Option::value },
    { "-d2", "*zoomer2*delta", Option::value },
    { "-delta", "*delta", Option::value },
    { "-delta1", "*zoomer1*delta", Option::value },
    { "-delta2", "*zoomer2*delta", Option::value },
    { "-p", "*pause", Option::value },
    { "-p1", "*zoomer1*pause", Option::value },
    { "-p2", "*zoomer2*pause", Option::value },
    { "-pause", "*pause", Option::value },
    { "-pause1", "*zoomer1*pause", Option::value },
    { "-pause2", "*zoomer2*pause", Option::value },
    { "-v", "*verbose", Option::implicit, "on" },
    { "-verbose", "*verbose", Option::implicit, "on" },
    { nil }
};

class Zoomer : public MonoGlyph {
public:
    Zoomer(Fresco*, GlyphRef, StyleObjRef);
    virtual ~Zoomer();

    void main();

    void traverse(GlyphTraversalRef t); //+ Glyph::traverse
private:
    TransformObj matrix_;
    float scale_;
    float delta_;
    float pause_;
    ThreadObj zoom_;
};

declareActionCallback(Zoomer)
implementActionCallback(Zoomer)

Zoomer::Zoomer(Fresco* f, GlyphRef g, StyleObjRef s) {
    body(g);
    matrix_ = f->drawing_kit()->identity_transform();
    scale_ = 1.0;
    delta_ = 0.02;
    pause_ = 0.1;
    double value;
    StyleValue a = s->resolve(Fresco::string_ref("delta"));
    if (is_not_nil(a)) {
	if (a->read_real(value)) {
	    delta_ = float(value);
	}
    }
    a = s->resolve(Fresco::string_ref("pause"));
    if (is_not_nil(a)) {
	if (a->read_real(value)) {
	    pause_ = float(value);
	}
    }
    zoom_ = f->thread_kit()->thread(
	Action(new ActionCallback(Zoomer)(this, &Zoomer::main))
    );
    zoom_->run();
}

Zoomer::~Zoomer() {
    zoom_->terminate();
}

void Zoomer::main() {
    for (;;) {
	if (delay(pause_)) {
	    scale_ -= delta_;
	    if (scale_ < 0.05 || scale_ > 0.999999) {
		delta_ = -delta_;
	    }
	    need_redraw();
	}
    }
}

//+ Zoomer(Glyph::traverse)
void Zoomer::traverse(GlyphTraversalRef t) {
    PainterObj p = t->painter();
    if (is_not_nil(p)) {
	p->push_matrix();
	Vertex v;
	v.x = scale_; v.y = scale_; v.z = scale_;
	matrix_->load_identity();
	matrix_->scale(v);
	p->transform(matrix_);
	MonoGlyph::traverse(t);
	p->pop_matrix();
    }
}

int main(int argc, char** argv) {
    Fresco* f = Fresco_open("MTest", argc, argv, options);
    FigureKit figures = f->figure_kit();
    Glyph g = figures->figure_root(
	figures->circle(
	    FigureKit::fill, figures->default_style(), 0.0, 0.0, 100.0
	)
    );
    StyleObj s = f->style();
    Boolean verbose = s->is_on(Fresco::string_ref("verbose"));
    if (verbose) {
	g = new DebugGlyph(g, "circle", DebugGlyph::trace_request_traverse);
    }
    StyleObj s1 = s->new_style();
    s1->name(Fresco::string_ref("zoomer1"));
    s1->link_parent(s);
    Glyph g1 = new Zoomer(f, g, s1);
    if (verbose) {
	g1 = new DebugGlyph(
	    g1, "zoomer 1", DebugGlyph::trace_request_traverse
	);
    }
    StyleObj s2 = s->new_style();
    s2->name(Fresco::string_ref("zoomer2"));
    s2->link_parent(s);
    Glyph g2 = new Zoomer(f, g, s2);
    if (verbose) {
	g2 = new DebugGlyph(
	    g2, "zoomer 2", DebugGlyph::trace_request_traverse
	);
    }
    LayoutKit layouts = f->layout_kit();
    g = layouts->hbox();
    g->append(layouts->fixed(g1, 200, 200));
    g->append(layouts->hfil());
    g->append(layouts->fixed(g2, 200, 200));
    if (verbose) {
	g = new DebugGlyph(g, "hbox", DebugGlyph::trace_request_traverse);
    }
    f->main(nil, g);
    Fresco::unref(f);
    return 0;
}
