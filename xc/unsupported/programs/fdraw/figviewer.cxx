/*
 * figviewer.cxx
 */

#include "commands.h"
#include "figviewer.h"
#include "globals.h"
#include "manipulators.h"
#include "selection.h"
#include "tools.h"
#include "visitors.h"
#include <X11/Fresco/Impls/glyphs.h>
#include <X11/Fresco/Impls/region.h>
#include <X11/Fresco/Impls/transform.h>
#include <X11/Fresco/OS/memory.h>

FigViewer::FigViewer(Fresco* f, Boolean* editing) : ViewerImpl(f, true) {
    curtool_ = nil;
    active_ = nil;
    editing_ = editing;

    sel_ = new Selection(10);
    PolyFigure* pf = new PolyFigure;
    root_ = new PolyManip(pf);
    Fresco::unref(pf);

    allocation_ = new RegionImpl;
    damage_ = nil;
    transform_ = new TransformImpl;
    body(root_);
}
 
FigViewer::~FigViewer() {
    delete sel_;
}

//+ FigViewer(Glyph::allocations)
void FigViewer::allocations(Glyph::AllocationInfoSeq& a) {
    a._maximum = 1;
    a._length = 1;
    a._buffer = new Glyph::AllocationInfo[1];

    Glyph::AllocationInfo& i = a._buffer[0];
    i.allocation = new RegionImpl;
    i.allocation->copy(allocation_);
    i.transformation = new TransformImpl;
    i.transformation->load(transform_);
    i.damaged = Damage::_duplicate(damage_);
}

//+ FigViewer(Glyph::traverse)
void FigViewer::traverse(GlyphTraversal_in t) {
    switch (t->op()) {
    case GlyphTraversal::pick_top:
    case GlyphTraversal::pick_all:
    case GlyphTraversal::pick_any:

// This is inefficient for motion events
        ViewerImpl::traverse(t);
        if (is_nil(_tmp(t->picked())) && t->allocation_is_visible()) {
            t->begin_viewer(this);
            t->hit();
            t->end_viewer();
        }
        break;
    case GlyphTraversal::draw:
        {
            allocation_->copy(_tmp(t->allocation()));
            Painter_var po = t->current_painter();
            transform_->load(_tmp(po->current_matrix()));
            damage_ = Damage::_return_ref(t->damaged());
            ViewerImpl::traverse(t);
        }
        break;
    default:
        ViewerImpl::traverse(t);
        break;
    }
}

Manipulator* FigViewer::root () { 
    return root_; 
}

Selection* FigViewer::selection() {
    return sel_;
}

//+ FigViewer(Glyph::request)
void FigViewer::request(Glyph::Requisition& r) {
    LayoutKit_var layouts = fresco_instance()->layout_kit();
    Coord fil = layouts->fil();
    r.x.natural = 0; r.x.maximum = fil, r.x.minimum = 0, r.x.align = 0.5;
    r.y.natural = 0; r.y.maximum = fil, r.y.minimum = 0, r.y.align = 0.5;
}

Tool* FigViewer::current_tool() { return curtool_; }
void FigViewer::current_tool(Tool* t) { curtool_ = t; }

//+ FigViewer(Glyph::transformation)
Transform_return FigViewer::transformation() {
    Fresco::ref(transform_);
    return transform_;
}

//+ FigViewer(Glyph::need_resize)
void FigViewer::need_resize() {
    /*
     * Do nothing because we assume that we've already handled
     * a resize appropriately.
     */
}

Boolean FigViewer::press(GlyphTraversal_in t, EventRef e) {
    if (!*editing_) {
        return false;
    }
    Boolean ok = false;
    if (is_nil(active_)) {
        SelectInfo* si = curtool_->create_manipulator(t, e, this);
        active_ = (si != nil) ? si->m_ : nil;
        if (is_not_nil(active_)) {
            ok = active_->grasp(*curtool_, *this, si, e);
        } else {
            ok = false;
        }
        if (ok) {
            grab(t);
        }
    } else {
        ok = active_->manipulate(e);
    }
    if(!ok && is_not_nil(active_)) {
        Command* command = active_->effect(e);
        if (is_not_nil(command) && command->reversible()) {
            command->log();
        }
        active_ = nil;
    }
    return true;
}

Boolean FigViewer::move(GlyphTraversal_in t, EventRef e) {
    if (!*editing_) {
        return false;
    }
    return drag(t, e);
}

Boolean FigViewer::drag(GlyphTraversal_in, EventRef e) {
    if (!*editing_) {
        return false;
    }
    Boolean ok = false;
    if (is_not_nil(active_)) {
        ok = active_->manipulate(e);
        if (!ok) {
            Command* command = active_->effect(e);
            if (is_not_nil(command) && command->reversible()) {
                command->log();
            }
            active_ = nil;
            ungrab();
        }
    }
    return true;
}

Boolean FigViewer::release(GlyphTraversal_in, EventRef e) {
    if (!*editing_) {
        return false;
    }
    Boolean ok = false;
    if (is_not_nil(active_)) {
        ok = active_->manipulate(e);
        if (!ok) {
            Command* command = active_->effect(e);
            if (is_not_nil(command) && command->reversible()) {
                command->log();
            }
            active_ = nil;
            ungrab();
        }
    }
    return true;
}

