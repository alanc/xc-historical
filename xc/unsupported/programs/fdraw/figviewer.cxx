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

FigViewer::FigViewer(Fresco* f) : ViewerImpl(f) {
    curtool_ = nil;
    active_ = nil;

    sel_ = new Selection(10);
    root_ = new PolyManip(new PolyFigure);

    allocation_ = new RegionImpl;
    damage_ = nil;
    transform_ = new TransformImpl;
    body(root_);
}
 
FigViewer::~FigViewer() {
    delete sel_;
}

//+ FigViewer(Glyph::allocations)
void FigViewer::allocations(Glyph::AllocationInfoList& a) {
    if (a._length >= a._maximum) {
	long n = a._maximum == 0 ? 10 : a._maximum + a._maximum;
	Glyph::AllocationInfo* buffer = new Glyph::AllocationInfo[n];
	if (a._maximum > 0) {
            Memory::copy(
                a._buffer, buffer, a._maximum * sizeof(Glyph::AllocationInfo)
            );
	}
	/* should free elems? */
	a._buffer = buffer;
	a._maximum = n;
    }
    Glyph::AllocationInfo& i = a._buffer[a._length];
    i.allocation = new RegionImpl;
    i.allocation->copy(allocation_);
    i.transform = new TransformImpl;
    i.transform->load(transform_);
    i.damage = DamageObj::_duplicate(damage_);
    ++a._length;
}

//+ FigViewer(Glyph::traverse)
void FigViewer::traverse(GlyphTraversal_in t) {
    switch(t->op()) {
    case GlyphTraversal::pick_top:
    case GlyphTraversal::pick_all:
    case GlyphTraversal::pick_any:

// This is inefficient for motion events
        ViewerImpl::traverse(t);
        if (
            is_nil(t->picked()) && t->painter()->is_visible(t->allocation())
        ) {
            t->begin_trail(ViewerRef(this));
            t->hit();
            t->end_trail();
        }
        break;
    case GlyphTraversal::draw:
        {
            allocation_->copy(t->allocation());
            PainterObjRef po = t->painter();
            transform_->load(po->matrix());
            damage_ = t->damage();
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
    LayoutKit layouts = fresco_instance()->layout_kit();
    Coord fil = layouts->fil();
    r.x.natural = 0; r.x.maximum = fil, r.x.minimum = 0, r.x.align = 0.5;
    r.y.natural = 0; r.y.maximum = fil, r.y.minimum = 0, r.y.align = 0.5;
}

Tool* FigViewer::current_tool() { return curtool_; }
void FigViewer::current_tool(Tool* t) { curtool_ = t; }

//+ FigViewer(Glyph::transform)
TransformObjRef FigViewer::_c_transform() {
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

Boolean FigViewer::press(GlyphTraversalRef t, EventRef e) {
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

Boolean FigViewer::move(GlyphTraversalRef t, EventRef e) {
    return drag(t, e);
}

Boolean FigViewer::drag(GlyphTraversalRef, EventRef e) {
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

Boolean FigViewer::release(GlyphTraversalRef, EventRef e) {
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

