/*
 * fdraw.h
 */

#ifndef _fdraw_h
#define _fdraw_h

#include <X11/Fresco/widgets.h>
#include <X11/Fresco/Impls/viewers.h>

class Command;
class FigViewer;

class TelltaleType;
typedef TelltaleType* TelltaleRef;

class Flipper;
class Fresco;
class Patch;
class Tool;

class FDraw : public ViewerImpl {
public:
    FDraw(Fresco*);
    ~FDraw();

    virtual void traverse(GlyphTraversalRef);
    virtual Boolean key_press(GlyphTraversalRef, EventRef);
private:
    FigViewer* viewer_;
    ViewerImpl* subviews_;
private:
    Glyph_tmp interior();
    Glyph_tmp tools();
    Glyph_tmp commands();
   
    Button_tmp tool_button(GlyphRef, Tool*, TelltaleRef);
    Glyph_tmp command_button(const char*, Command*);
private:
    Patch* patch_;
    Flipper* flipper_;
    Boolean editing_;
};


#endif
