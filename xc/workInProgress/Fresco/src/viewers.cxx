/*
 * Copyright (c) 1987-91 Stanford University
 * Copyright (c) 1991-93 Silicon Graphics, Inc.
 * Copyright (c) 1993 Fujitsu, Ltd.
 *
 * Permission to use, copy, modify, distribute, and sell this software and 
 * its documentation for any purpose is hereby granted without fee, provided
 * that (i) the above copyright notices and this permission notice appear in
 * all copies of the software and related documentation, and (ii) the names
 * of Stanford, Silicon Graphics, and Fujitsu may not be used in any
 * advertising or publicity relating to the software without the specific,
 * prior written permission of Stanford, Silicon Graphics, and Fujitsu.
 * 
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY 
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  
 *
 * IN NO EVENT SHALL STANFORD, SILICON GRAPHICS, OR FUJITSU BE LIABLE FOR
 * ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF 
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE 
 * OF THIS SOFTWARE.
 */

/*
 * Common viewer implementations
 */

#include <X11/Fresco/display.h>
#include <X11/Fresco/drawing.h>
#include <X11/Fresco/Impls/region.h>
#include <X11/Fresco/Impls/transform.h>
#include <X11/Fresco/Impls/viewers.h>
#include <X11/Fresco/OS/memory.h>

ViewerImpl::ViewerImpl(Fresco* f) : style_(f), offset_(this) {
    style_.style_ = this;
    style_.lock_ = nil;
    offset_.child_ = nil;
    parent_ = nil;
    next_ = nil;
    prev_ = nil;
    first_ = nil;
    last_ = nil;
    entered_ = false;
    pressed_ = false;
    recorded_time_ = false;
    threshold_ = 250;
    display_ = nil;
    filter_ = 0;
    alias(Fresco::string_ref("Viewer"));
    StyleValue a = style_._c_bind(Fresco::string_ref("click_delay"));
    if (is_not_nil(a)) {
	long n;
	if (a->read_integer(n)) {
	    threshold_ = n;
	}
    }
}

ViewerImpl::~ViewerImpl() {
    Fresco::unref(offset_.child_);
    ViewerRef v, nextv;
    for (v = first_; is_not_nil(v); v = nextv) {
	nextv = v->_c_next_viewer();
	v->remove_viewer();
    }
}

//+ ViewerImpl(FrescoObject::=object_.)
Long ViewerImpl::ref__(Long references) {
    return object_.ref__(references);
}
Tag ViewerImpl::attach(FrescoObjectRef observer) {
    return object_.attach(observer);
}
void ViewerImpl::detach(Tag attach_tag) {
    object_.detach(attach_tag);
}
void ViewerImpl::disconnect() {
    object_.disconnect();
}
void ViewerImpl::notify_observers() {
    object_.notify_observers();
}
void ViewerImpl::update() {
    object_.update();
}
//+

//+ ViewerImpl(Glyph::style=s)
void ViewerImpl::_c_style(StyleObjRef s) { link_parent(s); }

//+ ViewerImpl(Glyph::style?)
StyleObjRef ViewerImpl::_c_style() {
    return StyleObj::_duplicate(StyleObjRef(this));
}

//+ ViewerImpl(Glyph::transform)
TransformObjRef ViewerImpl::_c_transform() { return nil; }

//+ ViewerImpl(Glyph::request)
void ViewerImpl::request(Glyph::Requisition& r) {
    offset_.child_->request(r);
}

//+ ViewerImpl(Glyph::extension)
void ViewerImpl::extension(const Glyph::AllocationInfo& a, RegionRef r) {
    offset_.child_->extension(a, r);
}

//+ ViewerImpl(Glyph::shape)
void ViewerImpl::shape(RegionRef r) {
    offset_.child_->shape(r);
}

//+ ViewerImpl(Glyph::allocations)
void ViewerImpl::allocations(Glyph::AllocationInfoList& a) {
    if (is_not_nil(parent_)) {
	parent_->allocations(a);
    }
}

//+ ViewerImpl(Glyph::traverse)
void ViewerImpl::traverse(GlyphTraversalRef t) {
    t->begin_trail(ViewerRef(this));
    switch (t->op()) {
    case GlyphTraversal::pick_top:
    case GlyphTraversal::pick_any:
    case GlyphTraversal::pick_all:
	if (t->painter()->is_visible(t->allocation())) {
	    t->hit();
	}
	break;
    default:
	t->traverse_child(&offset_, nil);
	break;
    }
    t->end_trail();
}

//+ ViewerImpl(Glyph::draw)
void ViewerImpl::draw(GlyphTraversalRef) { }

//+ ViewerImpl(Glyph::pick)
void ViewerImpl::pick(GlyphTraversalRef) { }

//+ ViewerImpl(Glyph::body=g)
void ViewerImpl::_c_body(GlyphRef g) {
    if (is_not_nil(offset_.child_)) {
	offset_.child_->remove_parent(offset_.remove_tag_);
    }
    Fresco::unref(offset_.child_);
    offset_.child_ = Glyph::_duplicate(g);
    offset_.remove_tag_ = g->add_parent(&offset_);
}

//+ ViewerImpl(Glyph::body?)
GlyphRef ViewerImpl::_c_body() {
    return Glyph::_duplicate(offset_.child_);
}

//+ ViewerImpl(Glyph::append)
GlyphOffsetRef ViewerImpl::_c_append(GlyphRef g) {
    return offset_.child_->append(g);
}

//+ ViewerImpl(Glyph::prepend)
GlyphOffsetRef ViewerImpl::_c_prepend(GlyphRef g) {
    return offset_.child_->prepend(g);
}

//+ ViewerImpl(Glyph::add_parent)
Tag ViewerImpl::add_parent(GlyphOffsetRef parent_offset) {
    long n = glyph_parents_.count();
    glyph_parents_.append(parent_offset);
    return n;
}

//+ ViewerImpl(Glyph::remove_parent)
void ViewerImpl::remove_parent(Tag add_tag) {
    long n = long(add_tag);
    if (n >= 0 && n < glyph_parents_.count()) {
	glyph_parents_.remove(n);
    }
}

//+ ViewerImpl(Glyph::visit_children)
void ViewerImpl::visit_children(GlyphVisitorRef v) {
    offset_.child_->visit_children(v);
}

//+ ViewerImpl(Glyph::visit_children_reversed)
void ViewerImpl::visit_children_reversed(GlyphVisitorRef v) {
    offset_.child_->visit_children_reversed(v);
}

//+ ViewerImpl(Glyph::visit_parents)
void ViewerImpl::visit_parents(GlyphVisitorRef v) {
    GlyphOffsetRef g;
    for (ListItr(GlyphOffsetList) i(glyph_parents_); i.more(); i.next()) {
	g = i.cur();
	v->visit(g->parent(), g);
    }
}

//+ ViewerImpl(Glyph::need_redraw)
void ViewerImpl::need_redraw() {
    if (is_not_nil(parent_)) {
	parent_->need_redraw();
    }
}

/*
 * Currently, we ignore the region and redraw the entire glyph.
 */

//+ ViewerImpl(Glyph::need_redraw_region)
void ViewerImpl::need_redraw_region(RegionRef r) {
    if (is_not_nil(parent_)) {
	parent_->need_redraw();
    }
}

//+ ViewerImpl(Glyph::need_resize)
void ViewerImpl::need_resize() {
    if (is_not_nil(parent_)) {
	parent_->need_resize();
    }
}

//+ ViewerImpl(Glyph::clone_glyph)
GlyphRef ViewerImpl::_c_clone_glyph() {
    return nil;
}

//+ ViewerImpl(Viewer::parent_viewer)
ViewerRef ViewerImpl::_c_parent_viewer() {
    return Viewer::_duplicate(parent_);
}

//+ ViewerImpl(Viewer::next_viewer)
ViewerRef ViewerImpl::_c_next_viewer() { return Viewer::_duplicate(next_); }

//+ ViewerImpl(Viewer::prev_viewer)
ViewerRef ViewerImpl::_c_prev_viewer() { return Viewer::_duplicate(prev_); }

//+ ViewerImpl(Viewer::first_viewer)
ViewerRef ViewerImpl::_c_first_viewer() { return Viewer::_duplicate(first_); }

//+ ViewerImpl(Viewer::last_viewer)
ViewerRef ViewerImpl::_c_last_viewer() { return Viewer::_duplicate(last_); }

//+ ViewerImpl(Viewer::append_viewer)
void ViewerImpl::append_viewer(ViewerRef v) {
    ViewerRef nv = Viewer::_duplicate(v);
    nv->set_viewer_links(this, last_, nil);
    last_ = nv;
    if (is_nil(first_)) {
	first_ = last_;
    }
}

//+ ViewerImpl(Viewer::prepend_viewer)
void ViewerImpl::prepend_viewer(ViewerRef v) {
    ViewerRef nv = Viewer::_duplicate(v);
    nv->set_viewer_links(this, nil, first_);
    first_ = nv;
    if (is_nil(last_)) {
	last_ = nv;
    }
}

//+ ViewerImpl(Viewer::set_viewer_links)
void ViewerImpl::set_viewer_links(ViewerRef parent, ViewerRef prev, ViewerRef next) {
    if (is_not_nil(parent)) {
	parent_ = parent;
	prev_ = prev;
	if (is_not_nil(prev)) {
	    prev->set_viewer_links(nil, nil, this);
	}
	next_ = next;
	if (is_not_nil(next)) {
	    next->set_viewer_links(nil, this, nil);
	}
    } else {
	if (is_not_nil(prev)) {
	    prev_ = prev;
	}
	if (is_not_nil(next)) {
	    next_ = next;
	}
    }
}

//+ ViewerImpl(Viewer::insert_viewer)
void ViewerImpl::insert_viewer(ViewerRef v) {
    ViewerRef nv = Viewer::_duplicate(v);
    nv->set_viewer_links(parent_, prev_, this);
    if (prev_ == nil) {
	parent_->set_first_viewer(nv);
    }
    prev_ = nv;
}

//+ ViewerImpl(Viewer::replace_viewer)
void ViewerImpl::replace_viewer(ViewerRef v) {
    v->set_viewer_links(parent_, prev_, next_);
    if (is_nil(prev_)) {
	parent_->set_first_viewer(v);
    }
    if (is_nil(next_)) {
	parent_->set_last_viewer(v);
    }
    prev_ = nil;
    next_ = nil;
    Fresco::unref(ViewerRef(this));
}

//+ ViewerImpl(Viewer::remove_viewer)
void ViewerImpl::remove_viewer() {
    if (is_nil(prev_)) {
	parent_->set_first_viewer(next_);
    } else if (is_nil(next_)) {
	parent_->set_last_viewer(prev_);
    }
    parent_ = nil;
    prev_ = nil;
    next_ = nil;
    Fresco::unref(ViewerRef(this));
}

//+ ViewerImpl(Viewer::set_first_viewer)
void ViewerImpl::set_first_viewer(ViewerRef v) {
    first_ = v;
    if (is_nil(v)) {
	last_ = nil;
    }
}

//+ ViewerImpl(Viewer::set_last_viewer)
void ViewerImpl::set_last_viewer(ViewerRef v) {
    last_ = v;
    if (is_nil(v)) {
	first_ = nil;
    }
}

//+ ViewerImpl(Viewer::request_focus)
FocusRef ViewerImpl::_c_request_focus(ViewerRef requestor, Boolean temporary) {
    return (
	is_nil(parent_) ? nil : parent_->_c_request_focus(requestor, temporary)
    );
}

//+ ViewerImpl(Viewer::receive_focus)
Boolean ViewerImpl::receive_focus(FocusRef f, Boolean primary) {
    return nil;
}

//+ ViewerImpl(Viewer::lose_focus)
void ViewerImpl::lose_focus(Boolean) { }

//+ ViewerImpl(Viewer::first_focus)
Boolean ViewerImpl::first_focus() {
    return false;
}

//+ ViewerImpl(Viewer::last_focus)
Boolean ViewerImpl::last_focus() {
    return false;
}

//+ ViewerImpl(Viewer::next_focus)
Boolean ViewerImpl::next_focus() {
    return false;
}

//+ ViewerImpl(Viewer::prev_focus)
Boolean ViewerImpl::prev_focus() {
    return false;
}

//+ ViewerImpl(Viewer::handle)
Boolean ViewerImpl::handle(GlyphTraversalRef t, EventRef e) {
    Boolean b = false;
    switch (e->type()) {
    case Event::motion:
    case Event::enter:
    case Event::leave:
    case Event::down:
    case Event::up:
	b = position_event(t, e);
	break;
    case Event::other:
	b = other(t, e);
	break;
    default:
	b = focus_event(t, e);
	break;
    }
    return b;
}

//+ ViewerImpl(Viewer::close)
void ViewerImpl::close() { }

Boolean ViewerImpl::position_event(GlyphTraversalRef t, EventRef e) {
    if (!entered_ && !pressed_ && is_not_nil(first_)) {
	GlyphTraversal::Operation op = t->swap_op(GlyphTraversal::pick_all);
	t->traverse_child(&offset_, nil);
	t->swap_op(op);
	GlyphTraversal p = t->picked();
	if (is_nil(p)) {
	    return false;
	}
	do {
	    if (is_nil(p->current_glyph())) {
		Viewer v = p->current_viewer();
		if (is_not_nil(v)) {
		    if (v->handle(p, e)) {
			return true;
		    }
		}
	    }
	    p = p->picked();
	} while (is_not_nil(p));
    }
    return handle_position_event(t, e);
}

Boolean ViewerImpl::handle_position_event(GlyphTraversalRef t, EventRef e) {
    Boolean b = false;
    switch (e->type()) {
    case Event::motion:
    case Event::enter:
    case Event::leave:
	b = pressed_ ? drag(t, e) : move(t, e);
	break;
    case Event::down:
	if (!pressed_ && press(t, e)) {
	    b = true;
	    pressed_ = true;
	    button_ = e->pointer_button();
	    grab(t);
	}
	break;
    case Event::up:
	if (pressed_ && e->pointer_button() == button_) {
	    pressed_ = false;
	    ungrab();
	    unsigned long time = e->time();
	    if (recorded_time_ && time - click_time_ < threshold_) {
		b = double_click(t, e);
	    } else {
		b = release(t, e);
	    }
	    click_time_ = time;
	    recorded_time_ = true;
	}
	break;
    default:
	b = other(t, e);
	break;
    }
    return b;
}

Boolean ViewerImpl::focus_event(GlyphTraversalRef t, EventRef e) {
    Boolean b = false;
    switch (e->type()) {
    case Event::key_press:
	b = key_press(t, e);
	break;
    case Event::key_release:
	b = key_release(t, e);
	break;
    default:
	break;
    }
    return b;
}

Boolean ViewerImpl::move(GlyphTraversalRef, EventRef) { return false; }
Boolean ViewerImpl::press(GlyphTraversalRef, EventRef) { return false; }
Boolean ViewerImpl::drag(GlyphTraversalRef, EventRef) { return false; }
Boolean ViewerImpl::release(GlyphTraversalRef, EventRef) { return false; }
Boolean ViewerImpl::double_click(GlyphTraversalRef t, EventRef e) {
    return release(t, e);
}

Boolean ViewerImpl::key_press(GlyphTraversalRef, EventRef) { return false; }
Boolean ViewerImpl::key_release(GlyphTraversalRef, EventRef) { return false; }
Boolean ViewerImpl::other(GlyphTraversalRef, EventRef) { return false; }

Boolean ViewerImpl::inside(GlyphTraversalRef t) {
    Boolean b = false;
    if (t->painter()->is_visible(t->allocation())) {
	GlyphTraversal::Operation op = t->swap_op(GlyphTraversal::pick_all);
	t->traverse_child(&offset_, nil);
	t->swap_op(op);
	b = is_not_nil(t->picked());
    }
    return b;
}

void ViewerImpl::grab(GlyphTraversalRef t) {
    if (is_nil(display_)) {
	display_ = t->_c_display();
	filter_ = display_->add_filter(ViewerRef(this), t);
    }
}

void ViewerImpl::ungrab() {
    if (!entered_ && !pressed_ && is_not_nil(display_)) {
	display_->remove_filter(filter_);
	Fresco::unref(display_);
	display_ = nil;
    }
}

//+ ViewerImpl(StyleObj::=style_.)
StyleObjRef ViewerImpl::_c_new_style() {
    return style_._c_new_style();
}
StyleObjRef ViewerImpl::_c_parent_style() {
    return style_._c_parent_style();
}
void ViewerImpl::link_parent(StyleObjRef parent) {
    style_.link_parent(parent);
}
void ViewerImpl::unlink_parent() {
    style_.unlink_parent();
}
Tag ViewerImpl::link_child(StyleObjRef child) {
    return style_.link_child(child);
}
void ViewerImpl::unlink_child(Tag link_tag) {
    style_.unlink_child(link_tag);
}
void ViewerImpl::merge(StyleObjRef s) {
    style_.merge(s);
}
CharStringRef ViewerImpl::_c_name() {
    return style_._c_name();
}
void ViewerImpl::_c_name(CharStringRef _p) {
    style_._c_name(_p);
}
void ViewerImpl::alias(CharStringRef s) {
    style_.alias(s);
}
Boolean ViewerImpl::is_on(CharStringRef name) {
    return style_.is_on(name);
}
StyleValueRef ViewerImpl::_c_bind(CharStringRef name) {
    return style_._c_bind(name);
}
void ViewerImpl::unbind(CharStringRef name) {
    style_.unbind(name);
}
StyleValueRef ViewerImpl::_c_resolve(CharStringRef name) {
    return style_._c_resolve(name);
}
StyleValueRef ViewerImpl::_c_resolve_wildcard(CharStringRef name, StyleObjRef start) {
    return style_._c_resolve_wildcard(name, start);
}
Long ViewerImpl::match(CharStringRef name) {
    return style_.match(name);
}
void ViewerImpl::visit_aliases(StyleVisitorRef v) {
    style_.visit_aliases(v);
}
void ViewerImpl::visit_attributes(StyleVisitorRef v) {
    style_.visit_attributes(v);
}
void ViewerImpl::visit_styles(StyleVisitorRef v) {
    style_.visit_styles(v);
}
void ViewerImpl::lock() {
    style_.lock();
}
void ViewerImpl::unlock() {
    style_.unlock();
}
//+

/* ActiveViewer */

ActiveViewer::ActiveViewer(Fresco* f) : ViewerImpl(f) { }
ActiveViewer::~ActiveViewer() { }

/*
 * Detect enter/leave into a viewer.  This is a little tricky
 * because it needs to work in conjunction with the grabbing
 * done by button press and release.  We assume that the
 * grab and ungrab operations take care of ensuring a single grab
 * is active, but we need to watch for re-entering a viewer
 * with the button pressed.  When the button is pressed and
 * we got the event while not inside the viewer, we must check
 * to see if the new event is now inside the viewer.
 */

Boolean ActiveViewer::move(GlyphTraversalRef t, EventRef e) {
    if (!entered_) {
	if (!pressed_ || inside(t)) {
	    entered_ = true;
	    grab(t);
	    enter();
	}
    } else if (e->type() == Event::leave || !inside(t)) {
	entered_ = false;
	leave();
	ungrab();
    }
    return true;
}

Boolean ActiveViewer::drag(GlyphTraversalRef t, EventRef e) {
    return move(t, e);
}

void ActiveViewer::enter() { }
void ActiveViewer::leave() { }

FocusImpl::FocusImpl(Fresco*) { }
FocusImpl::~FocusImpl() { }

//+ FocusImpl(FrescoObject::=object_.)
Long FocusImpl::ref__(Long references) {
    return object_.ref__(references);
}
Tag FocusImpl::attach(FrescoObjectRef observer) {
    return object_.attach(observer);
}
void FocusImpl::detach(Tag attach_tag) {
    object_.detach(attach_tag);
}
void FocusImpl::disconnect() {
    object_.disconnect();
}
void FocusImpl::notify_observers() {
    object_.notify_observers();
}
void FocusImpl::update() {
    object_.update();
}
//+

//+ FocusImpl(Focus::add_focus_interest)
void FocusImpl::add_focus_interest(ViewerRef) { }

//+ FocusImpl(Focus::receive_focus_below)
void FocusImpl::receive_focus_below(ViewerRef, Boolean) { }

//+ FocusImpl(Focus::lose_focus_below)
void FocusImpl::lose_focus_below(ViewerRef, Boolean) { }

//+ FocusImpl(Focus::map_keystroke)
void FocusImpl::map_keystroke(Event::KeySym, ActionRef) { }

//+ FocusImpl(Focus::map_keychord)
void FocusImpl::map_keychord(const Event::KeyChord&, ActionRef) { }

/* class MainViewer */

/*
 * We assume the window holds a reference to a main viewer and
 * will be destroyed about the same time, so a main viewer doesn't
 * hold a reference back to the window.
 */

MainViewer::MainViewer(Fresco* f) : ViewerImpl(f) {
    window_ = nil;
    focus_ = new FocusImpl(f);
    background_ = nil;
    damage_ = nil;
    invalidate();
}

MainViewer::~MainViewer() {
    Fresco::unref(focus_);
    Fresco::unref(background_);
}

//+ MainViewer(FrescoObject::notify_observers)
void MainViewer::notify_observers() {
    invalidate();
    ViewerImpl::notify_observers();
}

//+ MainViewer(Glyph::allocations)
void MainViewer::allocations(Glyph::AllocationInfoList& a) {
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
    i.damage = DamageObj::_duplicate(damage_);
    ++a._length;
}

//+ MainViewer(Glyph::traverse)
void MainViewer::traverse(GlyphTraversalRef t) {
    t->begin_trail(ViewerRef(this));
    if (t->op() == GlyphTraversal::draw) {
	draw(t);
    }
    ViewerImpl::traverse(t);
    t->end_trail();
    damage_ = t->_c_damage();
    Fresco::unref(damage_);
}

//+ MainViewer(Glyph::draw)
void MainViewer::draw(GlyphTraversalRef t) {
    if (!valid_) {
	cache();
    }
    Vertex lower, upper;
    t->allocation()->bounds(lower, upper);
    PainterObj p = t->painter();
    p->color_attr(background_);
    p->fill_rect(lower.x, lower.y, upper.x, upper.y);
}

//+ MainViewer(Glyph::need_redraw)
void MainViewer::need_redraw() {
    if (is_not_nil(damage_)) {
	damage_->incur();
    }
}

//+ MainViewer(Glyph::need_resize)
void MainViewer::need_resize() {
    need_redraw();
}

//+ MainViewer(Viewer::request_focus)
FocusRef MainViewer::_c_request_focus(ViewerRef requestor, Boolean temporary) {
    // should note who has focus here
    return focus_;
}

/*
 * Default close action is to exit the display run loop.
 */

void MainViewer::close() {
    if (is_not_nil(window_)) {
	window_->screen()->display()->run(false);
    }
}

void MainViewer::cache() {
    background_ = style_.fresco_->drawing_kit()->_c_background(style());
    valid_ = true;
}

void MainViewer::invalidate() {
    valid_ = false;
    Fresco::unref(background_);
    background_ = nil;
}
