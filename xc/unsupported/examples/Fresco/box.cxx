#include <X11/Fresco/drawing.h>
#include <X11/Fresco/figures.h>
#include <X11/Fresco/fresco.h>
#include <X11/Fresco/layouts.h>
#include <X11/Fresco/Impls/glyphs.h>

static Option options[] = {
    { "-s1", "*string1", Option::value, "on" },
    { "-s2", "*string2", Option::value, "on" },
    { "-string1", "*string1", Option::value, "on" },
    { "-string2", "*string2", Option::value, "on" },
    { "-v", "*verbose", Option::implicit, "on" },
    { "-verbose", "*verbose", Option::implicit, "on" },
    { nil }
};

static Boolean verbose;

static GlyphRef debug(GlyphRef g, const char* name) {
    return (
	verbose ?
	    new DebugGlyph(g, name, DebugGlyph::trace_request_traverse) : g
    );
}

int main(int argc, char** argv) {
    Fresco* f = Fresco_open("Test", argc, argv, options);
    StyleObj s = f->style();
    CharString hello = Fresco::string_ref("hello");
    StyleValue a = s->resolve(Fresco::string_ref("string1"));
    if (is_not_nil(a)) {
	a->read_string(hello);
    }
    CharString goodbye = Fresco::string_ref("goodbye");
    a = s->resolve(Fresco::string_ref("string2"));
    if (is_not_nil(a)) {
	a->read_string(goodbye);
    }
    verbose = s->is_on(Fresco::string_ref("verbose"));
    FigureKit figures = f->figure_kit();
    LayoutKit layouts = f->layout_kit();
    FigureStyle fs = figures->default_style();
    Glyph g = layouts->hbox();
    g->append(debug(figures->label(fs, hello), "label 1"));
    g->append(layouts->hfil());
    g->append(debug(figures->label(fs, goodbye), "label 2"));
    Glyph b = layouts->vbox();
    b->append(debug(g, "hbox instance 1"));
    b->append(layouts->vfil());
    b->append(debug(g, "hbox instance 2"));
    g = debug(b, "vbox");
    f->main(nil, g);
    Fresco::unref(f);
    return 0;
}
