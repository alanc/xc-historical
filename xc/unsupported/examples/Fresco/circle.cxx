#include <X11/Fresco/figures.h>
#include <X11/Fresco/fresco.h>

int main(int argc, char** argv) {
    Fresco* f = Fresco_open("Test", argc, argv);
    FigureKit figures = f->figure_kit();
    f->main(
	nil,
	figures->figure_root(
	    figures->circle(
		FigureKit::fill, figures->default_style(), 0.0, 0.0, 100.0
	    )
	)
    );
    Fresco::unref(f);
    return 0;
}
