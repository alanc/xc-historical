#include <X11/Fresco/display.h>
#include <X11/Fresco/figures.h>
#include <X11/Fresco/fresco.h>
#include <stdio.h>

int main(int argc, char** argv) {
    Fresco* f = Fresco_open("Test", argc, argv);
    FigureKit figures = f->figure_kit();
    StyleObj s = figures->default_style();
    FigureStyle fs = FigureStyle::_narrow(s);
    if (is_not_nil(fs)) {
	printf("s narrowed to FigureStyle\n");
    }
    DisplayStyle ds = DisplayStyle::_narrow(s);
    if (is_nil(ds)) {
	printf("failed to narrow to DisplayStyle\n");
    }
    Fresco::unref(f);
    return 0;
}
