#include <X11/Fresco/drawing.h>
#include <X11/Fresco/figures.h>
#include <X11/Fresco/fresco.h>

static Option options[] = {
    { "-s", "*string", Option::value, "on" },
    { "-string", "*string", Option::value, "on" },
    { nil }
};

int main(int argc, char** argv) {
    Fresco* f = Fresco_open("Test", argc, argv, options);
    CharString hello = Fresco::string_ref("hello goodbye");
    StyleValue a = f->style()->resolve(Fresco::string_ref("string"));
    if (is_not_nil(a)) {
	a->read_string(hello);
    }
    FigureKit figures = f->figure_kit();
    f->main(nil, figures->label(figures->default_style(), hello));
    Fresco::unref(f);
    return 0;
}
