#include <X11/Fresco/figures.h>
#include <X11/Fresco/fresco.h>
#include <X11/Fresco/layouts.h>
#include <X11/Fresco/widgets.h>
#include <X11/Fresco/Impls/action.h>
#include <stdio.h>

class App {
public:
    App() { }
    ~App() { }

    int main(int argc, char** argv);
private:
    void push_me();
};

declareActionCallback(App)
implementActionCallback(App)

int main(int argc, char** argv) {
    App a;
    return a.main(argc, argv);
}

int App::main(int argc, char** argv) {
    Fresco* f = Fresco_open("Test", argc, argv);
    FigureKit figures = f->figure_kit();
    Button b = f->widget_kit()->check_box(
	figures->label(
	    figures->default_style(), Fresco::string_ref("Push me")
	),
	Action(new ActionCallback(App)(this, &App::push_me))
    );
    f->main(b, f->layout_kit()->margin(b, 10.0));
    Fresco::unref(f);
    return 0;
}

void App::push_me() {
    printf("pushed\n");
}
