/*
 * main.cxx
 */

#include <X11/Fresco/fresco.h>
#include "figviewer.h"
#include "fdraw.h"
#include "globals.h"

static Option options[] = {
    { nil }
};

int main(int argc, char** argv) {
     Fresco* f = Fresco_open("Fdraw", argc, argv, options);
     global_init(f);
     Fresco::ref(f);
     FDraw* fdraw = new FDraw(f);
     f->main(fdraw, fdraw);
     return 0;
}
