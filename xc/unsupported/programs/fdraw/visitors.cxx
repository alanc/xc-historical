/*
 * vistitors.cxx
 */

#include "globals.h"
#include "commands.h"
#include "manipulators.h"
#include "visitors.h"
#include <X11/Fresco/fresco.h>
#include <X11/Fresco/viewer.h>
#include <X11/Fresco/Impls/region.h>
#include <X11/Fresco/Impls/transform.h>
#include <stdlib.h> 

implementPtrList(AllocationList, RegionImpl);
implementPtrList(TransformList, TransformImpl);

Appender::Appender(GlyphRef glyph) {
    glyph_ = glyph;
}

Boolean Appender::visit(GlyphRef parent, GlyphOffsetRef) {
    parent->append(glyph_);
    return true;
}

Appender::~Appender () {}

Remover::Remover() {}

Remover::~Remover() {
    for (ListItr(GlyphOffsetList) i(list_); i.more(); i.next()) {
	GlyphOffsetRef offset = i.cur();
	offset->notify_observers();
	offset->remove();
    }
}

Boolean Remover::visit(GlyphRef, GlyphOffsetRef offset) {
    list_.append(offset);
    return true;
}

Counter::Counter() {
    count_ = 0;
}

Boolean Counter::visit(GlyphRef, GlyphOffsetRef) {
    count_++;
    return true;
}

CmdVisitor::CmdVisitor (Command* cmd, Boolean e) {
    cmd_ = cmd;
    execute_ = e;
}

CmdVisitor::~CmdVisitor () {}

Boolean CmdVisitor::visit (GlyphRef g, GlyphOffsetRef) {
    Manipulator* m = Manipulator::_narrow(g);
    if (is_not_nil(m)) {
        if (execute_) {
            m->execute(cmd_);
        } else {
            m->unexecute(cmd_);
        }
    }
    return true;
}
    
ManipCopier::ManipCopier (Boolean s) {
    maniplist_ = new ManipList;
    shallow_ = s;
}

ManipCopier::~ManipCopier () {
    delete maniplist_;
}

Boolean ManipCopier::visit (GlyphRef g, GlyphOffsetRef) {
    Manipulator* m = Manipulator::_narrow(g);
    if (is_not_nil(m)) {
        if (shallow_) {
            maniplist_->append(m->shallow_copy());
        } else {
            maniplist_->append(m->deep_copy());
        }
    }
    return true;
}

TAManipCopier::TAManipCopier (RegionRef a, Boolean s) {
    alist_ = new AllocationList;
    tlist_ = new TransformList;
    a_ = a;
}

TAManipCopier::~TAManipCopier () {
    for (long i = 0; i < alist_->count(); i++) {
        Fresco::unref(alist_->item(i));
    }
    delete alist_;
    for (long j = 0; i < tlist_->count(); j++) {
        Fresco::unref(tlist_->item(j));
    }
    delete tlist_;
}

Boolean TAManipCopier::visit (GlyphRef g, GlyphOffsetRef go) {
    ManipCopier::visit(g, go);
    RegionImpl* r = new RegionImpl;
    TransformImpl* t = new TransformImpl;
    r->copy(a_);

    Glyph::AllocationInfo a;
    a.allocation = r;
    a.transform = t;
    a.damage = nil;
    
    go->child_allocate(a);
    
    alist_->append(r);
    tlist_->append(t);

    return true;
}

OffsetVisitor::OffsetVisitor () {
    glist_ = new GlyphOffsetList;
}

OffsetVisitor::~OffsetVisitor () {
    delete glist_;
}

Boolean OffsetVisitor::visit (GlyphRef, GlyphOffsetRef go) {
    glist_->append(go);
    return true;
}

GlyphOffsetRef OffsetVisitor::offset (long i) {
    GlyphOffsetRef go = nil;
    if (i < glist_->count()) {
        go = glist_->item(i);
    }
    return go;
}
