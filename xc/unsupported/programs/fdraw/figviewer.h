/*
 * figviewer.h
 */

#ifndef _figviewer_h
#define _figviewer_h

#include <X11/Fresco/figures.h>
#include <X11/Fresco/layouts.h>
#include <X11/Fresco/Impls/viewers.h>

class Manipulator;
class Selection;
class Tool;
class TransformImpl;

class FigViewer;
typedef FigViewer* FigViewerRef;

class FigViewer : public ViewerImpl {
public:
    FigViewer(Fresco*);
    ~FigViewer();
    
    void allocations(Glyph::AllocationInfoList& a); //+ Glyph::allocations
    void traverse(GlyphTraversal_in t); //+ Glyph::traverse
    void request(Glyph::Requisition& r); //+ Glyph::request
    TransformObjRef _c_transform(); //+ Glyph::transform
    void need_resize(); //+ Glyph::need_resize

    Tool* current_tool();
    void current_tool(Tool*);

    Boolean press(GlyphTraversalRef, EventRef);
    Boolean drag(GlyphTraversalRef, EventRef);
    Boolean move(GlyphTraversalRef, EventRef);
    Boolean release(GlyphTraversalRef, EventRef);
    
    Manipulator* root();
    Selection* selection();

protected:
    Tool* curtool_;
    Manipulator* root_;
    Selection* sel_;
    Manipulator* active_;
    RegionImpl* allocation_;
    DamageObjRef damage_;
    TransformImpl* transform_;
};

#endif
