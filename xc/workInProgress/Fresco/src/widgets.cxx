/*
 * $XConsortium$
 */

/*
 * Copyright (c) 1987-91 Stanford University
 * Copyright (c) 1991-93 Silicon Graphics, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and 
 * its documentation for any purpose is hereby granted without fee, provided
 * that (i) the above copyright notices and this permission notice appear in
 * all copies of the software and related documentation, and (ii) the names of
 * Stanford and Silicon Graphics may not be used in any advertising or
 * publicity relating to the software without the specific, prior written
 * permission of Stanford and Silicon Graphics.
 * 
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY 
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  
 *
 * IN NO EVENT SHALL STANFORD OR SILICON GRAPHICS BE LIABLE FOR
 * ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF 
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE 
 * OF THIS SOFTWARE.
 */

#include <X11/Fresco/display.h>
#include <X11/Fresco/drawing.h>
#include <X11/Fresco/fresco.h>
#include <X11/Fresco/layouts.h>
#include <X11/Fresco/types.h>
#include <X11/Fresco/widgets.h>
#include <X11/Fresco/Impls/region.h>
#include <X11/Fresco/Impls/widgetkit.h>
#include <X11/Fresco/OS/list.h>
#include <math.h>

ButtonImpl::ButtonImpl(
    Fresco* f, TelltaleRef t, ActionRef a
) : ActiveViewer(f) {
    state_ = nil;
    action_ = nil;
    tag_ = 0;
    state(t);
    click_action(a);
    alias(Fresco::string_ref("Button"));
}

ButtonImpl::~ButtonImpl() {
    detach_state();
    Fresco::unref(action_);
}

//+ ButtonImpl(FrescoObject::ref__)
Long ButtonImpl::ref__(Long references) {
    return object_.ref__(references);
}

//+ ButtonImpl(FrescoObject::attach)
Tag ButtonImpl::attach(FrescoObject_in observer) {
    return ActiveViewer::attach(observer);
}

//+ ButtonImpl(FrescoObject::detach)
void ButtonImpl::detach(Tag attach_tag) { ActiveViewer::detach(attach_tag); }

//+ ButtonImpl(FrescoObject::disconnect)
void ButtonImpl::disconnect() {
    state_ = nil;
}

//+ ButtonImpl(FrescoObject::notify_observers)
void ButtonImpl::notify_observers() { ActiveViewer::notify_observers(); }

//+ ButtonImpl(FrescoObject::update)
void ButtonImpl::update() { need_redraw(); }

//+ ButtonImpl(Glyph::=ActiveViewer::)
GlyphRef ButtonImpl::_c_clone_glyph() {
    return ActiveViewer::_c_clone_glyph();
}
StyleObjRef ButtonImpl::_c_style() {
    return ActiveViewer::_c_style();
}
void ButtonImpl::_c_style(StyleObj_in _p) {
    ActiveViewer::_c_style(_p);
}
TransformObjRef ButtonImpl::_c_transform() {
    return ActiveViewer::_c_transform();
}
void ButtonImpl::request(Glyph::Requisition& r) {
    ActiveViewer::request(r);
}
void ButtonImpl::extension(const Glyph::AllocationInfo& a, Region_in r) {
    ActiveViewer::extension(a, r);
}
RegionRef ButtonImpl::_c_shape() {
    return ActiveViewer::_c_shape();
}
void ButtonImpl::traverse(GlyphTraversal_in t) {
    ActiveViewer::traverse(t);
}
void ButtonImpl::draw(GlyphTraversal_in t) {
    ActiveViewer::draw(t);
}
void ButtonImpl::pick(GlyphTraversal_in t) {
    ActiveViewer::pick(t);
}
GlyphRef ButtonImpl::_c_body() {
    return ActiveViewer::_c_body();
}
void ButtonImpl::_c_body(Glyph_in _p) {
    ActiveViewer::_c_body(_p);
}
GlyphOffsetRef ButtonImpl::_c_append(Glyph_in g) {
    return ActiveViewer::_c_append(g);
}
GlyphOffsetRef ButtonImpl::_c_prepend(Glyph_in g) {
    return ActiveViewer::_c_prepend(g);
}
Tag ButtonImpl::add_parent(GlyphOffset_in parent_offset) {
    return ActiveViewer::add_parent(parent_offset);
}
void ButtonImpl::remove_parent(Tag add_tag) {
    ActiveViewer::remove_parent(add_tag);
}
void ButtonImpl::visit_children(GlyphVisitor_in v) {
    ActiveViewer::visit_children(v);
}
void ButtonImpl::visit_children_reversed(GlyphVisitor_in v) {
    ActiveViewer::visit_children_reversed(v);
}
void ButtonImpl::visit_parents(GlyphVisitor_in v) {
    ActiveViewer::visit_parents(v);
}
void ButtonImpl::allocations(Glyph::AllocationInfoList& a) {
    ActiveViewer::allocations(a);
}
void ButtonImpl::need_redraw() {
    ActiveViewer::need_redraw();
}
void ButtonImpl::need_redraw_region(Region_in r) {
    ActiveViewer::need_redraw_region(r);
}
void ButtonImpl::need_resize() {
    ActiveViewer::need_resize();
}
Boolean ButtonImpl::restore_trail(GlyphTraversal_in t) {
    return ActiveViewer::restore_trail(t);
}
//+

//+ ButtonImpl(Viewer::=ActiveViewer::)
ViewerRef ButtonImpl::_c_parent_viewer() {
    return ActiveViewer::_c_parent_viewer();
}
ViewerRef ButtonImpl::_c_next_viewer() {
    return ActiveViewer::_c_next_viewer();
}
ViewerRef ButtonImpl::_c_prev_viewer() {
    return ActiveViewer::_c_prev_viewer();
}
ViewerRef ButtonImpl::_c_first_viewer() {
    return ActiveViewer::_c_first_viewer();
}
ViewerRef ButtonImpl::_c_last_viewer() {
    return ActiveViewer::_c_last_viewer();
}
void ButtonImpl::append_viewer(Viewer_in v) {
    ActiveViewer::append_viewer(v);
}
void ButtonImpl::prepend_viewer(Viewer_in v) {
    ActiveViewer::prepend_viewer(v);
}
void ButtonImpl::insert_viewer(Viewer_in v) {
    ActiveViewer::insert_viewer(v);
}
void ButtonImpl::replace_viewer(Viewer_in v) {
    ActiveViewer::replace_viewer(v);
}
void ButtonImpl::remove_viewer() {
    ActiveViewer::remove_viewer();
}
void ButtonImpl::set_viewer_links(Viewer_in parent, Viewer_in prev, Viewer_in next) {
    ActiveViewer::set_viewer_links(parent, prev, next);
}
void ButtonImpl::set_first_viewer(Viewer_in v) {
    ActiveViewer::set_first_viewer(v);
}
void ButtonImpl::set_last_viewer(Viewer_in v) {
    ActiveViewer::set_last_viewer(v);
}
FocusRef ButtonImpl::_c_request_focus(Viewer_in requestor, Boolean temporary) {
    return ActiveViewer::_c_request_focus(requestor, temporary);
}
Boolean ButtonImpl::receive_focus(Focus_in f, Boolean primary) {
    return ActiveViewer::receive_focus(f, primary);
}
void ButtonImpl::lose_focus(Boolean temporary) {
    ActiveViewer::lose_focus(temporary);
}
Boolean ButtonImpl::first_focus() {
    return ActiveViewer::first_focus();
}
Boolean ButtonImpl::last_focus() {
    return ActiveViewer::last_focus();
}
Boolean ButtonImpl::next_focus() {
    return ActiveViewer::next_focus();
}
Boolean ButtonImpl::prev_focus() {
    return ActiveViewer::prev_focus();
}
Boolean ButtonImpl::handle(GlyphTraversal_in t, Event_in e) {
    return ActiveViewer::handle(t, e);
}
void ButtonImpl::close() {
    ActiveViewer::close();
}
//+

//+ ButtonImpl(Button::state=t)
void ButtonImpl::_c_state(Telltale_in t) {
    detach_state();
    state_ = Telltale::_duplicate(t);
    tag_ = state_->attach(ButtonRef(this));
}

//+ ButtonImpl(Button::state?)
TelltaleRef ButtonImpl::_c_state() {
    return Telltale::_duplicate(state_);
}

//+ ButtonImpl(Button::click_action=a)
void ButtonImpl::_c_click_action(Action_in a) {
    action_ = Action::_duplicate(a);
}

//+ ButtonImpl(Button::click_action?)
ActionRef ButtonImpl::_c_click_action() {
    return Action::_duplicate(action_);
}

void ButtonImpl::enter() {
    if (state_->test(Telltale::enabled)) {
	state_->set(Telltale::visible);
    }
}

void ButtonImpl::leave() {
    if (state_->test(Telltale::enabled)) {
	state_->clear(Telltale::visible);
    }
}

Boolean ButtonImpl::press(GlyphTraversalRef, EventRef) {
    if (state_->test(Telltale::enabled)) {
	state_->set(Telltale::active);
    }
    return true;
}

Boolean ButtonImpl::release(GlyphTraversalRef t, EventRef) {
    TelltaleRef s = state_;
    if (s->test(Telltale::enabled)) {
	s->clear(Telltale::active);
	if (inside(t)) {
	    Boolean chosen = s->test(Telltale::chosen);
	    if (s->test(Telltale::toggle)) {
		if (chosen) {
		    s->clear(Telltale::chosen);
		} else {
		    s->set(Telltale::chosen);
		}
		click();
	    } else {
		if (s->test(Telltale::choosable)) {
		    s->set(Telltale::chosen);
		}
		if (!chosen) {
		    click();
		}
	    }
	}
    }
    return true;
}

/*
 * Unlike ViewerImpl, which does a pick on its contents to see if
 * the traversal area intersects with the viewer's appearance,
 * ButtonImpl simply checks to see if the traversal area intersects
 * with the button's allocation.
 */

Boolean ButtonImpl::inside(GlyphTraversalRef t) {
    return t->painter()->is_visible(t->allocation());
}

void ButtonImpl::click() {
    if (is_not_nil(action_)) {
	state_->set(Telltale::running);
	action_->execute();
	state_->clear(Telltale::running);
    }
}

void ButtonImpl::detach_state() {
    if (is_not_nil(state_)) {
	state_->detach(tag_);
	Fresco::unref(state_);
    }
}

TelltaleImpl::TelltaleImpl(TelltaleImpl::FlagSet f) {
    flags_ = f;
    current_ = nil;
}

TelltaleImpl::~TelltaleImpl() { }

//+ TelltaleImpl(FrescoObject::=object_.)
Long TelltaleImpl::ref__(Long references) {
    return object_.ref__(references);
}
Tag TelltaleImpl::attach(FrescoObject_in observer) {
    return object_.attach(observer);
}
void TelltaleImpl::detach(Tag attach_tag) {
    object_.detach(attach_tag);
}
void TelltaleImpl::disconnect() {
    object_.disconnect();
}
void TelltaleImpl::notify_observers() {
    object_.notify_observers();
}
void TelltaleImpl::update() {
    object_.update();
}
//+

//+ TelltaleImpl(Telltale::set)
void TelltaleImpl::set(Telltale::Flag f) { modify(f, true); }

//+ TelltaleImpl(Telltale::clear)
void TelltaleImpl::clear(Telltale::Flag f) { modify(f, false); }

//+ TelltaleImpl(Telltale::test)
Boolean TelltaleImpl::test(Telltale::Flag f) {
    return (flags_ & (1 << f)) != 0;
}

//+ TelltaleImpl(Telltale::current=g)
void TelltaleImpl::_c_current(Telltale_in g) {
    Fresco::unref(current_);
    current_ = g;
}

//+ TelltaleImpl(Telltale::current?)
TelltaleRef TelltaleImpl::_c_current() {
    return Telltale::_duplicate(current_);
}

void TelltaleImpl::modify(Telltale::Flag f, Boolean on) {
    TelltaleImpl::FlagSet fs = (1 << f);
    TelltaleImpl::FlagSet newflags = on ? (flags_ | fs) : (flags_ & ~fs);
    if (newflags != flags_) {
	flags_ = newflags;
	notify_observers();
	if (on && f == Telltale::chosen && is_not_nil(current_)) {
	    Telltale t = current_->current();
	    if (is_not_nil(t)) {
		t->clear(Telltale::chosen);
	    }
	    current_->current(this);
	}
    }
}

WidgetKitRef FrescoImpl::create_widget_kit() {
    return new WidgetKitImpl(this);
}

#define button_border 4
#define arrow_border 6

static const int black = 0;
static const int very_dark_gray = 1;
static const int dark_gray = 2;
static const int medium_gray = 3;
static const int light_gray = 4;
static const int very_light_gray = 5;
static const int white = 6;
static const int gray_out = 7;
static const int shadow = 8;
static const int yellow = 9;
static const int light_yellow = 10;
static const int dark_yellow = 11;
static const int medium_yellow = 12;

static const unsigned int checkmark_width = 32;
static const unsigned int checkmark_height = 12;

static char checkmark_bits[] = {
    0x00, 0x00, 0x3e, 0x00, 0x00, 0xc0, 0x0f, 0x00,
    0x04, 0xf0, 0x03, 0x00, 0x1e, 0xf8, 0x00, 0x00,
    0x3f, 0x3e, 0x00, 0x00, 0xbf, 0x1f, 0x00, 0x00,
    0xf8, 0x07, 0x00, 0x00, 0xf0, 0x03, 0x00, 0x00,
    0xf0, 0x01, 0x00, 0x00, 0xe0, 0x00, 0x00, 0x00,
    0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const unsigned int shadow1_width = 32;
static const unsigned int shadow1_height = 13;

static char shadow1_bits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xc0, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x00,
    0x07, 0x38, 0x00, 0x00, 0x0f, 0x1c, 0x00, 0x00,
    0x08, 0x06, 0x00, 0x00, 0x10, 0x03, 0x00, 0x00,
    0x90, 0x01, 0x00, 0x00, 0xe0, 0x00, 0x00, 0x00,
    0x60, 0x00, 0x00, 0x00
};

static const unsigned int shadow2_width = 32;
static const unsigned int shadow2_height = 5;

static char shadow2_bits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00,
    0x00, 0x00, 0x3c, 0x00, 0x00, 0x00, 0x0f, 0x00,
    0x00, 0x00, 0x03, 0x00
};

class WKFrame : public Beveler {
public:
    WKFrame(
	GlyphRef, TelltaleImpl*, WidgetKitImpl::Info*, Coord thickness,
	float xalign = 0.5, float yalign = 0.5,
	Boolean hmargin = true, Boolean vmargin = true
    );
    ~WKFrame();

    void draw(GlyphTraversal_in t); //+ Glyph::draw
    void pick(GlyphTraversal_in t); //+ Glyph::pick
    virtual void draw_frame(GlyphTraversalRef t);

    static void fill_rect(
	PainterObjRef p, const Vertex& lower, const Vertex& upper, ColorRef
    );
    static void shade(
	PainterObjRef p, const Vertex& lower, const Vertex& upper,
	WidgetKitImpl::Info* info, const int* colors, int ncolors, Coord* t
    );
protected:
    TelltaleImpl* state_;
    WidgetKitImpl::Info* info_;
};

class WKButtonFrame : public WKFrame {
public:
    WKButtonFrame(
	GlyphRef, TelltaleImpl*, WidgetKitImpl::Info*,
	Coord thickness = button_border, float xalign = 0.5, float yalign = 0.5
    );
    ~WKButtonFrame();

    virtual void draw_frame(GlyphTraversalRef t);
};

class WKPushButtonFrame : public WKButtonFrame {
public:
    WKPushButtonFrame(
	GlyphRef, TelltaleImpl*, WidgetKitImpl::Info*,
	Coord thickness = button_border, float xalign = 0.5, float yalign = 0.5
    );
    ~WKPushButtonFrame();

    void request(Glyph::Requisition& r); //+ Glyph::request
};

class WKMenuItemFrame : public WKButtonFrame {
public:
    WKMenuItemFrame(GlyphRef, TelltaleImpl*, WidgetKitImpl::Info*);
    ~WKMenuItemFrame();

    virtual void draw_frame(GlyphTraversalRef t);
};

class WKGlyph : public GlyphImpl {
public:
    WKGlyph(WidgetKitImpl::Info*);
    ~WKGlyph();
protected:
    WidgetKitImpl::Info* info_;
};

class WKCheckmark : public WKGlyph {
public:
    WKCheckmark(WidgetKitImpl::Info*, TelltaleImpl*);
    ~WKCheckmark();

    void request(Glyph::Requisition& r); //+ Glyph::request
    void draw(GlyphTraversal_in t); //+ Glyph::draw
protected:
    TelltaleImpl* state_;
};

class WKIndicator : public WKGlyph {
public:
    WKIndicator(WidgetKitImpl::Info*, TelltaleImpl*);
    ~WKIndicator();

    void request(Glyph::Requisition& r); //+ Glyph::request
    void draw(GlyphTraversal_in t); //+ Glyph::draw
protected:
    TelltaleImpl* state_;
};

class WKRadioFlag : public WKGlyph {
public:
    WKRadioFlag(WidgetKitImpl::Info*, TelltaleImpl*, FontRef);
    ~WKRadioFlag();

    void request(Glyph::Requisition& r); //+ Glyph::request
    void draw(GlyphTraversal_in t); //+ Glyph::draw
protected:
    TelltaleImpl* state_;
    FontRef font_;
};

class WKRadioItem : public WKRadioFlag {
public:
    WKRadioItem(WidgetKitImpl::Info*, TelltaleImpl*, FontRef);
    ~WKRadioItem();

    void draw(GlyphTraversal_in t); //+ Glyph::draw
};

class WKDefaultArrow : public WKGlyph {
public:
    WKDefaultArrow(WidgetKitImpl::Info*, FontRef);
    ~WKDefaultArrow();

    void request(Glyph::Requisition& r); //+ Glyph::request
    void draw(GlyphTraversal_in t); //+ Glyph::draw
protected:
    Coord width_;
    Coord height_;
    Coord ascent_;
    Alignment align_;
};

class WKThumb : public WKGlyph {
public:
    WKThumb(WidgetKitImpl::Info*, Axis, long ridges, TelltaleImpl*);
    ~WKThumb();

    void draw(GlyphTraversal_in t); //+ Glyph::draw
protected:
    Axis axis_;
    long ridges_;
    TelltaleImpl* state_;
};

struct StyleData {
    const char* path;
    const char* value;
};

static StyleData kit_attributes[] = {
    { "*WidgetKit*background", "#aaaaaa" },
    { "*WidgetKit*checkScale", "0.7" },
    { "*WidgetKit*frameThickness", "2.0" },
    { "*WidgetKit*moverSize", "20.0" },
    { "*WidgetKit*radioScale", "0.9" },
    { "*WidgetKit*sliderSize", "20.0" },
    { "*WidgetKit*FileChooser*filter", "off" },
    { "*WidgetKit*FieldEditor*beveled", "on" },
    { "*WidgetKit*FieldEditor*background", "#b88d8d" },
    { "*WidgetKit*FieldEditor*flat", "#b88d8d" },
    { "*WidgetKit*MenuBar*font", "*-helvetica-bold-o-normal--14-140-*" },
    { "*WidgetKit*MenuItem*font", "*-helvetica-bold-o-normal--14-140-*" },
    { "*WidgetKit*minButtonWidth", "72.0" },
    { "*WidgetKit*minThumbSize", "28.0" },
    { "*WidgetKit*thumbRidges", "3" },
    { "*Slider*thumbRidges", "1" },
    { nil }
};

/*
 * We don't hold a reference to the fresco object to avoid
 * a circularity.
 */

WidgetKitImpl::WidgetKitImpl(Fresco* f) {
    fresco_ = f;
    StyleObj s = f->style();
    style_ = s->_c_new_style();
    style_->alias(Fresco::string_ref("WidgetKit"));
    style_->link_parent(s);
    for (StyleData* d = kit_attributes; d->path != nil; d++) {
	StyleValue a = s->bind(Fresco::string_ref(d->path));
	if (a->uninitialized() || a->priority() <= -10) {
	    a->write_string(Fresco::string_ref(d->value));
	}
    }
    load();
}

WidgetKitImpl::~WidgetKitImpl() {
    Fresco::unref(style_);
}

//+ WidgetKitImpl(FrescoObject::=object_.)
Long WidgetKitImpl::ref__(Long references) {
    return object_.ref__(references);
}
Tag WidgetKitImpl::attach(FrescoObject_in observer) {
    return object_.attach(observer);
}
void WidgetKitImpl::detach(Tag attach_tag) {
    object_.detach(attach_tag);
}
void WidgetKitImpl::disconnect() {
    object_.disconnect();
}
void WidgetKitImpl::notify_observers() {
    object_.notify_observers();
}
void WidgetKitImpl::update() {
    object_.update();
}
//+

//+ WidgetKitImpl(WidgetKit::outset_frame)
GlyphRef WidgetKitImpl::_c_outset_frame(Glyph_in g) {
    return new WKFrame(
	g,
	new TelltaleImpl(TelltaleImpl::enabled_bit | TelltaleImpl::active_bit),
	&info_, info_.thickness
    );
}

//+ WidgetKitImpl(WidgetKit::inset_frame)
GlyphRef WidgetKitImpl::_c_inset_frame(Glyph_in g) {
    return new WKFrame(
	g,
	new TelltaleImpl(TelltaleImpl::enabled_bit | TelltaleImpl::chosen_bit),
	&info_, info_.thickness
    );
}

//+ WidgetKitImpl(WidgetKit::bright_inset_frame)
GlyphRef WidgetKitImpl::_c_bright_inset_frame(Glyph_in g) {
    return new WKFrame(
	g,
	new TelltaleImpl(
	    TelltaleImpl::enabled_bit | TelltaleImpl::visible_bit |
	    TelltaleImpl::active_bit
	),
	&info_, 0.5 * info_.thickness, 0.0, 0.0, false, false
    );
}

//+ WidgetKitImpl(WidgetKit::label)
ViewerRef WidgetKitImpl::_c_label(CharString_in s) {
    return nil;
}

//+ WidgetKitImpl(WidgetKit::menubar)
MenuRef WidgetKitImpl::_c_menubar() {
    return nil;
}

//+ WidgetKitImpl(WidgetKit::pulldown)
MenuRef WidgetKitImpl::_c_pulldown() {
    return nil;
}

//+ WidgetKitImpl(WidgetKit::pullright)
MenuRef WidgetKitImpl::_c_pullright() { return nil; }

//+ WidgetKitImpl(WidgetKit::menubar_item)
MenuItemRef WidgetKitImpl::_c_menubar_item(Glyph_in g) { return nil; }

//+ WidgetKitImpl(WidgetKit::menu_item)
MenuItemRef WidgetKitImpl::_c_menu_item(Glyph_in g) { return nil; }

//+ WidgetKitImpl(WidgetKit::check_menu_item)
MenuItemRef WidgetKitImpl::_c_check_menu_item(Glyph_in g) { return nil; }

//+ WidgetKitImpl(WidgetKit::radio_menu_item)
MenuItemRef WidgetKitImpl::_c_radio_menu_item(Glyph_in g, Telltale_in group) { return nil; }

//+ WidgetKitImpl(WidgetKit::menu_item_separator)
MenuItemRef WidgetKitImpl::_c_menu_item_separator() { return nil; }

//+ WidgetKitImpl(WidgetKit::telltale_group)
TelltaleRef WidgetKitImpl::_c_telltale_group() {
    return new TelltaleImpl;
}

//+ WidgetKitImpl(WidgetKit::push_button)
ButtonRef WidgetKitImpl::_c_push_button(Glyph_in g, Action_in a) {
    TelltaleImpl* t = new TelltaleImpl(TelltaleImpl::enabled_bit);
    ButtonRef b = new ButtonImpl(fresco_, t, a);
    b->body(new WKPushButtonFrame(g, t, &info_));
    return b;
}

//+ WidgetKitImpl(WidgetKit::default_button)
ButtonRef WidgetKitImpl::_c_default_button(Glyph_in g, Action_in a) {
    LayoutKit layouts = fresco_->layout_kit();
    FontRef f = info_.font;
    Glyph hbox = layouts->hbox();
    hbox->append(g);
    hbox->append(layouts->hspace(3.0));
    hbox->append(new WKDefaultArrow(&info_, f));
    return _c_push_button(hbox, a);
}

//+ WidgetKitImpl(WidgetKit::check_box)
ButtonRef WidgetKitImpl::_c_check_box(Glyph_in g, Action_in a) {
    TelltaleImpl* t = new TelltaleImpl(
	TelltaleImpl::enabled_bit | TelltaleImpl::toggle_bit
    );
    ButtonRef b = new ButtonImpl(fresco_, t, a);
    LayoutKit layouts = fresco_->layout_kit();
    Glyph box = layouts->hbox();
    box->append(
	layouts->vcenter(
	    Glyph(
		new WKButtonFrame(
		    Glyph(new WKCheckmark(&info_, t)), t, &info_
		)
	    )
	)
    );
    box->append(layouts->hspace(6.0));
    box->append(layouts->vcenter(g));
    b->body(box);
    return b;
}

//+ WidgetKitImpl(WidgetKit::palette_button)
ButtonRef WidgetKitImpl::_c_palette_button(Glyph_in g, Action_in a) {
    TelltaleImpl* t = new TelltaleImpl(
	TelltaleImpl::enabled_bit | TelltaleImpl::toggle_bit
    );
    ButtonRef b = new ButtonImpl(fresco_, t, a);
    LayoutKit layouts = fresco_->layout_kit();
    Glyph box = layouts->hbox();
    box->append(layouts->hspace(3.0));
    box->append(Glyph(new WKIndicator(&info_, t)));
    box->append(layouts->hspace(6.0));
    box->append(g);
    b->body(
	Glyph(new WKPushButtonFrame(box, t, &info_, button_border, 0.0, 0.5))
    );
    return b;
}

//+ WidgetKitImpl(WidgetKit::radio_button)
ButtonRef WidgetKitImpl::_c_radio_button(Glyph_in g, Action_in a, Telltale_in group) {
    TelltaleImpl* t = new TelltaleImpl(
	TelltaleImpl::enabled_bit | TelltaleImpl::choosable_bit
    );
    t->current(group);
    ButtonRef b = new ButtonImpl(fresco_, t, a);
    LayoutKit layouts = fresco_->layout_kit();
    Glyph box = layouts->hbox();
    FontRef f = info_.font;
    box->append(layouts->vcenter(Glyph(new WKRadioFlag(&info_, t, f))));
    box->append(layouts->hspace(6.0));
    box->append(layouts->vcenter(g));
    b->body(box);
    return b;
}

//+ WidgetKitImpl(WidgetKit::slider)
ViewerRef WidgetKitImpl::_c_slider(Axis a, Adjustment_in adj) {
    return nil;
}

//+ WidgetKitImpl(WidgetKit::scroll_bar)
ViewerRef WidgetKitImpl::_c_scroll_bar(Axis a, Adjustment_in adj) { return nil; }

//+ WidgetKitImpl(WidgetKit::panner)
ViewerRef WidgetKitImpl::_c_panner(Adjustment_in x, Adjustment_in y) { return nil; }

//+ WidgetKitImpl(WidgetKit::zoomer)
ButtonRef WidgetKitImpl::_c_zoomer(Coord scale, Adjustment_in x, Adjustment_in y, Adjustment_in z) { return nil; }

//+ WidgetKitImpl(WidgetKit::up_mover)
ButtonRef WidgetKitImpl::_c_up_mover(Adjustment_in a) { return nil; }

//+ WidgetKitImpl(WidgetKit::down_mover)
ButtonRef WidgetKitImpl::_c_down_mover(Adjustment_in a) { return nil; }

//+ WidgetKitImpl(WidgetKit::left_mover)
ButtonRef WidgetKitImpl::_c_left_mover(Adjustment_in a) { return nil; }

//+ WidgetKitImpl(WidgetKit::right_mover)
ButtonRef WidgetKitImpl::_c_right_mover(Adjustment_in a) { return nil; }

ColorRef WidgetKitImpl::brightness(ColorRef c, float adjust) {
    Color::Intensity r, g, b;
    c->rgb(r, g, b);
    if (adjust >= 0) {
	r += (1 - r) * adjust;
	g += (1 - g) * adjust;
	b += (1 - b) * adjust;
    } else {
	float f = adjust + 1.0;
	r *= f;
	g *= f;
	b *= f;
    }
    return fresco_->drawing_kit()->_c_color_rgb(r, g, b);
}

void WidgetKitImpl::load() {
    WidgetKitImpl::Info* i = &info_;
    load_coord("frameThickness", i->thickness);
    load_float("checkScale", i->check_size);
    load_float("radioScale", i->radio_scale);
    load_coord("moverSize", i->mover_size);
    load_coord("sliderSize", i->slider_size);
    load_coord("minButtonWidth", i->min_button_width);

    DrawingKit d = fresco_->drawing_kit();
    i->font = d->_c_default_font(style_);
    ColorRef bg = d->_c_background(style_);
    if (is_nil(bg)) {
	bg = d->_c_color_rgb(0.7, 0.7, 0.7);
    }
    i->num_colors = 13;
    ColorRef* c = new ColorRef[i->num_colors];
    i->color = c;
    c[black] = brightness(bg, -0.85);
    c[very_dark_gray] = brightness(bg, -0.66);
    c[dark_gray] = brightness(bg, -0.5);
    c[medium_gray] = brightness(bg, -0.33);
    c[light_gray] = bg;
    c[very_light_gray] = brightness(bg, 0.45);
    c[white] = brightness(bg, 0.7);
    /* gray out not implemented correctly */
    c[gray_out] = Color::_duplicate(bg);
    /* shadow not implemented correctly */
    c[shadow] = Color::_duplicate(c[black]);
    c[yellow] = d->_c_color_rgb(1.0, 1.0, 0.0);
    c[light_yellow] = d->_c_color_rgb(1.0, 1.0, 0.875);
    c[medium_yellow] = brightness(i->color[yellow], -0.3);
    c[dark_yellow] = brightness(i->color[yellow], -0.5);

    i->checkmark_color = d->_c_resolve_color(
	style_, Fresco::string_ref("checkmarkColor")
    );
    if (is_nil(i->checkmark_color)) {
	i->checkmark_color = d->_c_color_rgb(0.9, 0.0, 0.0);
    }
    i->checkmark = make_bitmap(
	d, checkmark_bits, checkmark_width, checkmark_height
    );
    i->shadow1 = make_bitmap(
	d, shadow1_bits, shadow1_width, shadow1_height
    );
    i->shadow2 = make_bitmap(
	d, shadow2_bits, shadow2_width, shadow2_height
    );
}

RasterRef WidgetKitImpl::make_bitmap(
    DrawingKitRef d, const char* data,
    unsigned int width, unsigned int height
) {
    DrawingKit::Data seq;

    /* This length computation is wrong, but matches RasterBitmap */
    seq._length = (width * height + sizeof(long) - 1) / sizeof(long);
    seq._maximum = seq._length;
    seq._buffer = (long*)data;
    RasterRef r = d->_c_bitmap_data(seq, height, width, 13, 0);

    /* Don't let destructor free data */
    seq._buffer = nil;

    return r;
}

void WidgetKitImpl::load_coord(const char* name, Coord& c) {
    StyleValue a = style_->resolve(Fresco::string_ref(name));
    if (is_not_nil(a)) {
	a->read_coord(c);
    }
}

void WidgetKitImpl::load_float(const char* name, float& f) {
    StyleValue a = style_->resolve(Fresco::string_ref(name));
    if (is_not_nil(a)) {
	double d;
	if (a->read_real(d)) {
	    f = float(d);
	}
    }
}

void WidgetKitImpl::unload() {
    WidgetKitImpl::Info* i = &info_;
    for (long c = 0; c < i->num_colors; c++) {
	Fresco::unref(i->color[c]);
    }
    delete [] i->color;
    Fresco::unref(i->checkmark);
    Fresco::unref(i->checkmark_color);
    Fresco::unref(i->shadow1);
    Fresco::unref(i->shadow2);
}

/* class WKFrame */

WKFrame::WKFrame(
    GlyphRef g, TelltaleImpl* t, WidgetKitImpl::Info* info, Coord thickness,
    float xalign, float yalign, Boolean hmargin, Boolean vmargin
) : Beveler(thickness, xalign, yalign, hmargin, vmargin) {
    body(g);
    state_ = t;
    info_ = info;
}

WKFrame::~WKFrame() { }

//+ WKFrame(Glyph::draw)
void WKFrame::draw(GlyphTraversal_in t) {
    draw_frame(t);
    if (!state_->test(Telltale::enabled)) {
	Coord th = info_->thickness;
	PainterObj p = t->painter();
	Vertex v[3];
	t->bounds(v[0], v[1], v[2]);
	v[0].x += th; v[0].y += th;
	v[1].x -= th; v[1].y -= th;
	// need fill patterns or alpha blending here
	fill_rect(p, v[0], v[1], info_->color[gray_out]);
    }
}

//+ WKFrame(Glyph::pick)
void WKFrame::pick(GlyphTraversal_in t) {
    if (t->painter()->is_visible(t->allocation())) {
	t->hit();
    }
}

static int inset_colors[] = {
    dark_gray, very_dark_gray, light_gray, very_light_gray, white
};

static int bright_inset_colors[] = {
    dark_gray, very_light_gray, white
};

static int outset_colors[] = {
    black, white, light_gray, medium_gray, black
};

static int trough_colors[] = {
    dark_gray, very_light_gray, light_gray, dark_gray, very_dark_gray
};

static int visible_trough_colors[] = {
    medium_gray, white, very_light_gray, medium_gray, dark_gray
};

static int old_thumb_colors[] = {
    very_dark_gray, dark_gray, medium_gray, light_gray, very_light_gray
};

static int* frame_colors[] = {
    /* 0 */ nil,
    /* is_enabled */ trough_colors,
    /* is_visible */ nil,
    /* is_enabled_visible */ visible_trough_colors,
    /* is_active */ nil,
    /* is_enabled_active */ outset_colors,
    /* is_visible_active */ nil,
    /* is_enabled_visible_active */ bright_inset_colors,
    /* is_chosen */ nil,
    /* is_enabled_chosen */ inset_colors,
    /* is_visible_chosen */ nil,
    /* is_enabled_visible_chosen */ nil,
    /* is_active_chosen */ nil,
    /* is_enabled_active_chosen */ old_thumb_colors,
    /* is_visible_active_chosen */ nil,
    /* is_enabled_visible_active_chosen */ nil
};

void WKFrame::draw_frame(GlyphTraversalRef t) {
    PainterObj p = t->painter();
    Vertex v[3];
    t->bounds(v[0], v[1], v[2]);
    int* colors = frame_colors[state_->flags()];
    if (colors == nil) {
	fill_rect(p, v[0], v[1], info_->color[light_gray]);
    } else {
	int ncolors;
	Coord thickness = p->to_pixels_coord(info_->thickness);
	Coord th[2];
	if (colors == bright_inset_colors) {
	    ncolors = 3;
	    th[0] = thickness;
	} else {
	    ncolors = 5;
	    Coord tt = 0.5 * thickness;
	    th[0] = tt;
	    th[1] = tt;
	}
	shade(p, v[0], v[1], info_, colors, ncolors, th);
    }
}

void WKFrame::fill_rect(
    PainterObjRef p, const Vertex& lower, const Vertex& upper, ColorRef c
) {
    Beveler::set_color(p, c);
    p->fill_rect(lower.x, lower.y, upper.x, upper.y);
}

void WKFrame::shade(
    PainterObjRef p, const Vertex& lower, const Vertex& upper,
    WidgetKitImpl::Info* info, const int* colors, int ncolors, Coord* th
) {
    Coord x0 = lower.x, y0 = lower.y, x1 = upper.x, y1 = upper.y;
    int nbands = (ncolors - 1) >> 1;
    int b = nbands - 1;
    int n = ncolors - 1;
    for (int i = 0; i < b; i++) {
	Coord t1 = th[i];
	Beveler::rect(
	    p, t1, info->color[colors[i]], nil, info->color[colors[n - i]],
	    x0, y0, x1, y1
	);
	x0 += t1; y0 += t1;
	x1 -= t1; y1 -= t1;
    }
    Beveler::rect(
	p, th[b], info->color[colors[b]], info->color[colors[nbands]],
	info->color[colors[nbands + 1]], x0, y0, x1, y1
    );
}

WKButtonFrame::WKButtonFrame(
    GlyphRef g, TelltaleImpl* t, WidgetKitImpl::Info* info, Coord thickness,
    float xalign, float yalign
) : WKFrame(g, t, info, thickness, xalign, yalign, true, true) { }

WKButtonFrame::~WKButtonFrame() { }

static int disabled_button_colors[] = {
    medium_gray, light_gray, light_gray, light_gray,
    light_gray, light_gray, dark_gray
};

static int enabled_button_colors[] = {
    dark_gray, white, very_light_gray, light_gray,
    medium_gray, dark_gray, very_dark_gray
};

static int visible_button_colors[] = {
    dark_gray, white, white, very_light_gray,
    light_gray, dark_gray, very_dark_gray
};

static int active_button_colors[] = {
    very_dark_gray, dark_gray, very_light_gray, light_gray,
    light_gray, black, white
};

static int visible_active_button_colors[] = {
    very_dark_gray, dark_gray, white, very_light_gray,
    light_gray, black, white
};

static int* button_colors[] = {
    /* 0 */ disabled_button_colors,
    /* is_enabled */ enabled_button_colors,
    /* is_visible */ disabled_button_colors,
    /* is_enabled_visible */ visible_button_colors,
    /* is_active */ disabled_button_colors,
    /* is_enabled_active */ active_button_colors,
    /* is_visible_active */ disabled_button_colors,
    /* is_enabled_visible_active */ visible_active_button_colors,
    /* is_chosen */ disabled_button_colors,
    /* is_enabled_chosen */ enabled_button_colors,
    /* is_visible_chosen */ disabled_button_colors,
    /* is_enabled_visible_chosen */ visible_button_colors,
    /* is_active_chosen */ disabled_button_colors,
    /* is_enabled_active_chosen */ active_button_colors,
    /* is_visible_active_chosen */ disabled_button_colors,
    /* is_enabled_visible_active_chosen */ visible_active_button_colors
};

void WKButtonFrame::draw_frame(GlyphTraversalRef t) {
    PainterObj p = t->painter();
    Vertex v[3];
    t->bounds(v[0], v[1], v[2]);
    Coord t1 = p->to_pixels_coord(1.0);
    Coord th[3];
    th[0] = t1; th[1] = t1; th[2] = t1 + t1;
    WKFrame::shade(
	p, v[0], v[1], info_, button_colors[state_->flags()], 7, th
    );
}

/* class WKPushButtonFrame */

WKPushButtonFrame::WKPushButtonFrame(
    GlyphRef g, TelltaleImpl* t, WidgetKitImpl::Info* info, Coord thickness,
    float xalign, float yalign
) : WKButtonFrame(g, t, info, thickness, xalign, yalign) { }

WKPushButtonFrame::~WKPushButtonFrame() { }

//+ WKPushButtonFrame(Glyph::request)
void WKPushButtonFrame::request(Glyph::Requisition& r) {
    WKButtonFrame::request(r);
    if (r.x.natural < info_->min_button_width) {
	r.x.natural = info_->min_button_width;
	if (r.x.maximum < r.x.natural) {
	    r.x.maximum = r.x.natural;
	}
    }
    r.x.natural += button_border;
    r.x.maximum += button_border;
    r.x.minimum += button_border;
    r.y.natural += button_border;
    r.y.maximum += button_border;
    r.y.minimum += button_border;
}

/* class WKMenuItemFrame */

WKMenuItemFrame::WKMenuItemFrame(
    GlyphRef g, TelltaleImpl* t, WidgetKitImpl::Info* info
) : WKButtonFrame(g, t, info, info->thickness) { }

WKMenuItemFrame::~WKMenuItemFrame() { }

void WKMenuItemFrame::draw_frame(GlyphTraversalRef t) {
    PainterObj p = t->painter();
    Vertex v[3];
    t->bounds(v[0], v[1], v[2]);
    if (state_->test(Telltale::active) || state_->test(Telltale::running)) {
	int* colors = outset_colors;
	Coord th[2];
	Coord tt = 0.5 * info_->thickness;
	th[0] = tt;
	th[1] = tt;
	WKFrame::shade(p, v[0], v[1], info_, colors, 5, th);
    } else {
	WKFrame::fill_rect(p, v[0], v[1], info_->color[light_gray]);
    }
}

/* class WKGlyph */

WKGlyph::WKGlyph(WidgetKitImpl::Info* info) {
    info_ = info;
}

WKGlyph::~WKGlyph() { }

/* class WKCheckmark */

WKCheckmark::WKCheckmark(
    WidgetKitImpl::Info* i, TelltaleImpl* t
) : WKGlyph(i) {
    state_ = t;
}

WKCheckmark::~WKCheckmark() { }

//+ WKCheckmark(Glyph::request)
void WKCheckmark::request(Glyph::Requisition& r) {
    WidgetKitImpl::Info* i = info_;
    Font::Info b;
    i->font->font_info(b);
    Coord h = b.font_ascent + b.font_descent;
    Coord size = h * i->check_size;
    Alignment a = (h == 0) ? 0 : b.font_descent / h;
    GlyphImpl::require(r.x, size, 0, 0, 0);
    GlyphImpl::require(r.y, size, 0, 0, a);
}

//+ WKCheckmark(Glyph::draw)
void WKCheckmark::draw(GlyphTraversal_in t) {
    if (state_->test(Telltale::chosen)) {
	WidgetKitImpl::Info* i = info_;
	Vertex o;
	t->origin(o);
	PainterObj p = t->painter();
	Beveler::set_color(p, i->color[dark_gray]);
	p->stencil(i->shadow1, o.x, o.y);
	p->stencil(i->shadow2, o.x, o.y);
	Beveler::set_color(p, i->checkmark_color);
	p->stencil(i->checkmark, o.x, o.y);
    }
}

/* class WKIndicator */

WKIndicator::WKIndicator(
    WidgetKitImpl::Info* i, TelltaleImpl* t
) : WKGlyph(i) {
    state_ = t;
}

WKIndicator::~WKIndicator() {
    Fresco::unref(TelltaleRef(state_));
}

//+ WKIndicator(Glyph::request)
void WKIndicator::request(Glyph::Requisition& r) {
    GlyphImpl::require(r.x, 6.0, 0, 0, 0);
}

static int disabled_indicator_colors[] = {
    dark_gray, medium_gray, white
};

static int disabled_chosen_indicator_colors[] = {
    dark_gray, light_yellow, white
};

static int enabled_indicator_colors[] = {
    black, dark_yellow, white
};

static int active_indicator_colors[] = {
    black, yellow, white
};

static int visible_indicator_colors[] = {
    black, medium_yellow, white
};

static int* indicator_colors[] = {
    /* 0 */ disabled_indicator_colors,
    /* is_enabled */ enabled_indicator_colors,
    /* is_visible */ disabled_indicator_colors,
    /* is_enabled_visible */ visible_indicator_colors,
    /* is_active */ disabled_indicator_colors,
    /* is_enabled_active */ enabled_indicator_colors,
    /* is_visible_active */ disabled_indicator_colors,
    /* is_enabled_visible_active */ visible_indicator_colors,
    /* is_chosen */ disabled_chosen_indicator_colors,
    /* is_enabled_chosen */ active_indicator_colors,
    /* is_visible_chosen */ disabled_chosen_indicator_colors,
    /* is_enabled_visible_chosen */ active_indicator_colors,
    /* is_active_chosen */ disabled_chosen_indicator_colors,
    /* is_enabled_active_chosen */ active_indicator_colors,
    /* is_visible_active_chosen */ disabled_chosen_indicator_colors,
    /* is_enabled_visible_active_chosen */ active_indicator_colors
};

//+ WKIndicator(Glyph::draw)
void WKIndicator::draw(GlyphTraversal_in t) {
    WidgetKitImpl::Info* i = info_;
    PainterObj p = t->painter();
    int* c = indicator_colors[state_->flags()];
    Coord t1 = p->to_pixels_coord(1.0);
    Coord t2 = t1 + t1;
    Vertex v0, v1, v;
    t->bounds(v0, v1, v);
    Beveler::rect(
	p, t1, i->color[c[0]], i->color[c[1]], i->color[c[2]],
	v0.x, v0.y + t2, v1.x, v1.y - t2
    );
}

/* class WKRadioFlag */

WKRadioFlag::WKRadioFlag(
    WidgetKitImpl::Info* i, TelltaleImpl* t, FontRef f
) : WKGlyph(i) {
    state_ = t;
    font_ = f;
}

WKRadioFlag::~WKRadioFlag() {
    Fresco::unref(TelltaleRef(state_));
    Fresco::unref(font_);
}

//+ WKRadioFlag(Glyph::request)
void WKRadioFlag::request(Glyph::Requisition& r) {
    Font::Info b;
    font_->char_info('M', b);
    Coord size = b.width * info_->radio_scale;
    GlyphImpl::require(r.x, size, 0, 0, 0);
    GlyphImpl::require(r.y, size, 0, 0, 0);
}

static int outset_radio_colors[] = {
    very_light_gray, light_gray, dark_gray
};

static int visible_outset_radio_colors[] = {
    white, very_light_gray, medium_gray
};

static int inset_radio_colors[] = {
    dark_gray, light_gray, very_light_gray
};

static int visible_inset_radio_colors[] = {
    medium_gray, very_light_gray, white
};
    
static int* radio_colors[] = {
    /* 0 */ outset_radio_colors,
    /* is_enabled */ outset_radio_colors,
    /* is_visible */ outset_radio_colors,
    /* is_enabled_visible */ visible_outset_radio_colors,
    /* is_active */ outset_radio_colors,
    /* is_enabled_active */ inset_radio_colors,
    /* is_visible_active */ outset_radio_colors,
    /* is_enabled_visible_active */ visible_inset_radio_colors,
    /* is_chosen */ inset_radio_colors,
    /* is_enabled_chosen */ inset_radio_colors,
    /* is_visible_chosen */ inset_radio_colors,
    /* is_enabled_visible_chosen */ visible_inset_radio_colors,
    /* is_active_chosen */ inset_radio_colors,
    /* is_enabled_active_chosen */ inset_radio_colors,
    /* is_visible_active_chosen */ inset_radio_colors,
    /* is_enabled_visible_active_chosen */ visible_inset_radio_colors
};

//+ WKRadioFlag(Glyph::draw)
void WKRadioFlag::draw(GlyphTraversal_in t) {
    WidgetKitImpl::Info* i = info_;
    PainterObj p = t->painter();
    int* colors = radio_colors[state_->flags()];
    Vertex v0, v1, v;
    t->bounds(v0, v1, v);
    Beveler::diamond(
	p, i->thickness,
	i->color[colors[0]], i->color[colors[1]], i->color[colors[2]],
	v0.x, v0.y, v1.x, v1.y
    );
}

/* class WKRadioItem */

WKRadioItem::WKRadioItem(
    WidgetKitImpl::Info* i, TelltaleImpl* t, FontRef f
) : WKRadioFlag(i, t, f) { }

WKRadioItem::~WKRadioItem() { }

//+ WKRadioItem(Glyph::draw)
void WKRadioItem::draw(GlyphTraversal_in t) {
    if (state_->test(Telltale::chosen)) {
	WKRadioFlag::draw(t);
    }
}

/* class WidgetKitImplThumb */

WKThumb::WKThumb(
    WidgetKitImpl::Info* info, Axis a, long ridges, TelltaleImpl* t
) : WKGlyph(info) {
    axis_ = a;
    ridges_ = ridges;
    state_ = t;
}

WKThumb::~WKThumb() {
    Fresco::unref(TelltaleRef(state_));
}

static int thumb_colors[] = {
    very_light_gray, very_light_gray, light_gray, medium_gray, dark_gray
};

static int visible_thumb_colors[] = {
    white, white, very_light_gray, light_gray, dark_gray
};

//+ WKThumb(Glyph::draw)
void WKThumb::draw(GlyphTraversal_in t) {
    WidgetKitImpl::Info* i = info_;
    int* colors = (
	state_->test(Telltale::visible) ? visible_thumb_colors : thumb_colors
    );
    ColorRef c[6];
    for (int c_i = 0; c_i < 5; c_i++) {
	c[c_i] = i->color[colors[c_i]];
    }
    Vertex v0, v1, v;
    t->bounds(v0, v1, v);
    PainterObj p = t->painter();
    Coord p1 = p->to_pixels_coord(1.0);
    Coord p2 = p1 + p1, p3 = p2 + p1, p4 = p3 + p1, p5 = p4 + p1;
    Beveler::set_color(p, i->color[very_dark_gray]);
    switch (axis_) {
    case X_axis:
	p->fill_rect(v0.x, v0.y, v0.x + p1, v1.y);
	p->fill_rect(v1.x - p1, v0.y, v1.x, v1.y);
	v0.x += p1;
	v1.x -= p1;
	break;
    case Y_axis:
	p->fill_rect(v0.x, v1.y - p1, v1.x, v1.y);
	p->fill_rect(v0.x, v0.y, v1.x, v0.y + p1);
	v0.y += p1;
	v1.y -= p1;
	break;
    }
    Beveler::rect(p, p1, c[0], nil, c[4], v0.x, v0.y, v1.x, v1.y);
    Beveler::rect(
	p, p1, c[1], c[2], c[3], v0.x + p1, v0.y + p1, v1.x - p1, v1.y - p1
    );

    Coord mid;
    Coord left[6], bottom[6], right[6], top[6];
    long r;
    switch (axis_) {
    case X_axis:
	mid = p->to_pixels_coord(0.5 * (v0.x + v1.x));
	v0.y += p1; v1.y -= p1;
	c[0] = i->color[very_dark_gray];
	c[1] = i->color[white];
	for (r = 0; r < 6; r++) {
	    bottom[r] = v0.y; top[r] = v1.y;
	}
	left[0] = mid + p4; right[0] = mid + p5;
	left[1] = mid + p3; right[1] = mid + p4;
	left[2] = mid; right[2] = mid + p1;
	left[3] = mid - p1; right[3] = mid;
	left[4] = mid - p4; right[4] = mid - p3;
	left[5] = mid - p5; right[5] = mid - p4;
	break;
    case Y_axis:
	mid = p->to_pixels_coord(0.5 * (v0.y + v1.y));
	v0.x += p1; v1.x -= p1;
	c[0] = i->color[white];
	c[1] = i->color[very_dark_gray];
	for (r = 0; r < 6; r++) {
	    left[0] = v0.x; right[0] = v1.x;
	}
	bottom[0] = mid + p4; top[0] = mid + p5;
	bottom[1] = mid + p3; top[1] = mid + p4;
	bottom[2] = mid; top[2] = mid + p1;
	bottom[3] = mid - p1; top[3] = mid;
	bottom[4] = mid - p4; top[4] = mid - p3;
	bottom[5] = mid - p5; top[5] = mid - p4;
	break;
    default:
	return;
    }
    c[2] = c[0];
    c[3] = c[1];
    c[4] = c[0];
    c[5] = c[1];
    for (r = 3 - ridges_; r < 3 + ridges_; r++) {
	Beveler::set_color(p, c[r]);
	p->fill_rect(left[r], bottom[r], right[r], top[r]);
    }
}

/* class WKDefaultArrow */

WKDefaultArrow::WKDefaultArrow(
    WidgetKitImpl::Info* i, FontRef f
) : WKGlyph(i) {
    Font::Info b;
    f->char_info('M', b);
    width_ = b.width;
    height_ = b.font_ascent + b.font_descent;
    ascent_ = b.ascent;
    align_ = (height_ == 0) ? 0 : b.font_descent / height_;
}

WKDefaultArrow::~WKDefaultArrow() { }

//+ WKDefaultArrow(Glyph::request)
void WKDefaultArrow::request(Glyph::Requisition& r) {
    GlyphImpl::require(r.x, 1.25 * width_, 0, 0, 0);
    GlyphImpl::require(r.y, height_, 0, 0, align_);
}

//+ WKDefaultArrow(Glyph::draw)
void WKDefaultArrow::draw(GlyphTraversal_in t) {
    WidgetKitImpl::Info* i = info_;
    Vertex v0, v1, v;
    t->bounds(v0, v1, v);

    Coord left = v0.x, right = v1.x;
    Coord x0 = left + 0.4 * (right - left);
    Coord bottom = v.y;
    Coord top = bottom + 0.9 * ascent_;
    Coord y0 = 0.5 * (bottom + top);

    PainterObj p = t->painter();
    Beveler::set_color(p, i->color[black]);
    p->begin_path();
    p->move_to(left, y0);
    p->line_to(x0, top);
    p->line_to(x0, bottom);
    p->close_path();
    p->fill();

    Coord th = i->thickness;
    y0 -= 0.5 * th;
    Coord y1 = y0 + th;
    p->fill_rect(x0, y0, right, y1);
    p->fill_rect(right - th, y1, right, top);
}

/* class Beveler */

Beveler::Beveler(
    Coord thickness, Alignment x, Alignment y, Boolean h, Boolean v
) {
    thickness_ = thickness;
    xalign_ = x;
    yalign_ = y;
    hmargin_ = h;
    vmargin_ = v;
    allocation_ = new RegionImpl;
}

Beveler::~Beveler() {
    Fresco::unref(allocation_);
}

//+ Beveler(Glyph::request)
void Beveler::request(Glyph::Requisition& r) {
    MonoGlyph::request(r);
    if (hmargin_ || vmargin_) {
	Coord t = thickness_ + thickness_;
	if (hmargin_ && r.x.defined) {
	    r.x.natural += t;
	    r.x.maximum += t;
	    r.x.minimum += t;
	}
	if (vmargin_ && r.y.defined) {
	    r.y.natural += t;
	    r.y.maximum += t;
	    r.y.minimum += t;
	}
    }
}

/*
 * To compute our extension quickly, we make two simplifying
 * assumptions.  First, we assume that a beveler does not have
 * a transform.  Second, we assume that the glyph inside a beveler
 * does not draw outside the beveler.  This assumption is not checked,
 * but for the kinds of objects we normally have in a beveler
 * should be safe.
 */

//+ Beveler(Glyph::extension)
void Beveler::extension(const Glyph::AllocationInfo& a, Region_in r) {
    if (is_not_nil(a.allocation)) {
	if (is_nil(a.transform)) {
	    r->merge_union(a.allocation);
	} else {
	    RegionImpl tmp;
	    tmp.copy(a.allocation);
	    tmp.transform(a.transform);
	    r->merge_union(&tmp);
	}
    }
}

//+ Beveler(Glyph::traverse)
void Beveler::traverse(GlyphTraversal_in t) {
    switch (t->op()) {
    case GlyphTraversal::draw:
	draw(t);
	break;
    case GlyphTraversal::pick_top:
    case GlyphTraversal::pick_any:
    case GlyphTraversal::pick_all:
	pick(t);
	break;
    default:
	break;
    }
    if (is_not_nil(offset_.child_)) {
	if (hmargin_ || vmargin_) {
	    t->traverse_child(&offset_, allocate_body(t));
	} else {
	    MonoGlyph::traverse(t);
	}
    }
}

RegionRef Beveler::allocate_body(GlyphTraversalRef t) {
    Glyph::Requisition req;
    MonoGlyph::request(req);
    RegionImpl* a = allocation_;
    Region::BoundingSpan s;

    t->span(X_axis, s);
    allocate_span(req.x, s, hmargin_ ? thickness_ : 0, xalign_);
    a->lower_.x = s.begin;
    a->upper_.x = s.end;
    a->xalign_ = s.align;

    t->span(Y_axis, s);
    allocate_span(req.y, s, vmargin_ ? thickness_ : 0, yalign_);
    a->lower_.y = s.begin;
    a->upper_.y = s.end;
    a->yalign_ = s.align;
    return Region::_duplicate(a);
}

void Beveler::allocate_span(
    Glyph::Requirement& r, Region::BoundingSpan& s, Coord margin, Alignment a
) {
    s.length -= (margin + margin);
    Coord offset = margin;
    if (r.defined) {
	if (s.length > r.maximum) {
	    offset += a * (s.length - r.maximum);
	    s.length = r.maximum;
	}
    }
    s.origin += offset * (1 - s.align - s.align);
    s.begin = s.origin - s.align * s.length;
    s.end = s.begin + s.length;
}

void Beveler::set_color(PainterObjRef p, ColorRef c) {
    p->color_attr(c);
}

void Beveler::fill_path(PainterObjRef p, Vertex* v, long n) {
    p->begin_path();
    p->move_to(v[0].x, v[0].y);
    for (long i = 1; i < n; i++) {
	p->line_to(v[i].x, v[i].y);
    }
    p->close_path();
    p->fill();
}

void Beveler::rect(
    PainterObjRef p, Coord thickness,
    ColorRef light, ColorRef medium, ColorRef dark,
    Coord left, Coord bottom, Coord right, Coord top
) {
    if (is_not_nil(medium)) {
	set_color(p, medium);
	p->fill_rect(left, bottom, right, top);
    }

    Coord left_inside = left + thickness;
    Coord bottom_inside = bottom + thickness;
    Coord right_inside = right - thickness;
    Coord top_inside = top - thickness;

    Vertex v[10];

    /* left edge */
    set_color(p, light);
    v[0].x = left; v[0].y = bottom;
    v[1].x = left; v[1].y = top;
    v[2].x = left_inside; v[2].y = top;
    v[3].x = left_inside; v[3].y = bottom;
    fill_path(p, v, 4);

    /* top edge */
    v[0].x = left_inside; v[0].y = top_inside;
    v[1].x = left_inside; v[1].y = top;
    v[2].x = right; v[2].y = top;
    v[3].x = right; v[3].y = top_inside;
    fill_path(p, v, 4);

    /* right edge */
    set_color(p, dark);
    v[0].x = right_inside; v[0].y = bottom_inside;
    v[1].x = right_inside; v[1].y = top_inside;
    v[2].x = right; v[2].y = top;
    v[3].x = right; v[3].y = bottom;

    /* bottom edge (as part of same path) */
    v[4].x = left; v[4].y = bottom;
    v[5].x = left_inside; v[5].y = bottom_inside;
    v[6].x = right_inside; v[6].y = bottom_inside;
    fill_path(p, v, 7);
}

/*
 * A bit of a misnomer to call these arrows; they are really beveled
 * triangles.  The only tricky part is dropping the bevel down and to the
 * right of the original triangle, which requires finding delta x,y for
 * a given thickness and matching the slope of the triangle.
 */

void Beveler::left_arrow(
    PainterObjRef p, Coord thickness,
    ColorRef light, ColorRef medium, ColorRef dark,
    Coord left, Coord bottom, Coord right, Coord top
) {
    Coord center_y = (bottom + top) * 0.5;
    float slope = ((top - bottom) / (right - left)) * 0.5;
    float delta_x = thickness / sqrt(slope * slope + 1);
    float delta_y = slope * delta_x;

    Vertex v[10];

    set_color(p, medium);
    v[0].x = left; v[0].y = center_y;
    v[1].x = right; v[1].y = top;
    v[2].x = right; v[2].y = bottom;
    fill_path(p, v, 3);

    set_color(p, dark);
    v[0].x = right - thickness; v[0].y = bottom + thickness;
    v[1].x = v[0].x; v[1].y = top - thickness;
    v[2].x = right; v[2].y = top;
    v[3].x = right; v[3].y = bottom;
    fill_path(p, v, 4);

    v[0].x = left; v[1].y = center_y;
    v[1].x = left + thickness; v[1].y = center_y;
    v[2].x = right - thickness; v[2].y = bottom + thickness;
    v[3].x = right; v[3].y = bottom;
    fill_path(p, v, 4);

    set_color(p, light);
    v[0].x = left + delta_x; v[0].y = center_y - delta_y;
    v[1].x = left; v[1].y = center_y;
    v[2].x = right; v[2].y = top;
    v[3].x = right; v[3].y = top - thickness;
    fill_path(p, v, 4);
}

void Beveler::right_arrow(
    PainterObjRef p, Coord thickness,
    ColorRef light, ColorRef medium, ColorRef dark,
    Coord left, Coord bottom, Coord right, Coord top
) {
    Coord center_y = (bottom + top) * 0.5;
    float slope = ((top - bottom) / (right - left)) * 0.5;
    float delta_x = thickness / sqrt(slope * slope + 1);
    float delta_y = slope * delta_x;

    Vertex v[10];

    set_color(p, medium);
    v[0].x = left; v[0].y = bottom;
    v[1].x = left; v[1].y = top;
    v[2].x = right; v[2].y = center_y;
    fill_path(p, v, 3);

    set_color(p, dark);
    v[0].x = left; v[0].y = bottom;
    v[1].x = left + thickness; v[1].y = bottom + thickness;
    v[2].x = right - thickness; v[2].y = center_y;
    v[3].x = right; v[3].y = center_y;
    fill_path(p, v, 4);

    set_color(p, light);
    v[0].x = left; v[0].y = bottom;
    v[1].x = left; v[1].y = top;
    v[2].x = left + thickness; v[2].y = top - thickness;
    v[3].x = left + delta_x; v[3].y = bottom + delta_y;
    fill_path(p, v, 4);

    v[0].x = left + thickness; v[0].y = top - thickness;
    v[1].x = left; v[1].y = top;
    v[2].x = right; v[2].y = center_y;
    v[3].x = right - delta_x; v[3].y = center_y - delta_y;
    fill_path(p, v, 4);
}

void Beveler::up_arrow(
    PainterObjRef p, Coord thickness,
    ColorRef light, ColorRef medium, ColorRef dark,
    Coord left, Coord bottom, Coord right, Coord top
) {
    Coord center_x = (left + right) * 0.5;
    float slope = 2 * ((top - bottom) / (right - left));
    float delta_x = thickness / sqrt(slope * slope + 1);
    float delta_y = slope * delta_x;

    Vertex v[10];

    set_color(p, medium);
    v[0].x = left; v[0].y = bottom;
    v[1].x = center_x; v[1].y = top;
    v[2].x = right; v[2].y = bottom;
    fill_path(p, v, 3);

    set_color(p, dark);
    v[0].x = left; v[0].y = bottom;
    v[1].x = right; v[1].y = bottom;
    v[2].x = right - thickness; v[2].y = bottom + thickness;
    v[3].x = left + thickness; v[3].y = v[2].y;
    fill_path(p, v, 4);

    v[0].x = center_x; v[0].y = top - thickness;
    v[1].x = center_x; v[1].y = top;
    v[2].x = right; v[2].y = bottom;
    v[3].x = right - thickness; v[3].y = bottom + thickness;
    fill_path(p, v, 4);

    set_color(p, light);
    v[0].x = left; v[0].y = bottom;
    v[1].x = left + delta_x; v[1].y = bottom;
    v[2].x = center_x + delta_x; v[2].y = top - delta_y;
    v[3].x = center_x; v[3].y = top;
    fill_path(p, v, 4);
}

void Beveler::down_arrow(
    PainterObjRef p, Coord thickness,
    ColorRef light, ColorRef medium, ColorRef dark,
    Coord left, Coord bottom, Coord right, Coord top
) {
    Coord center_x = (left + right) * 0.5;
    float slope = 2* ((top - bottom) / (right - left));
    float delta_x = thickness / sqrt(slope * slope + 1);
    float delta_y = slope * delta_x;

    Vertex v[10];

    set_color(p, medium);
    v[0].x = left; v[0].y = top;
    v[1].x = right; v[1].y = top;
    v[2].x = center_x; v[2].y = bottom;
    fill_path(p, v, 3);

    set_color(p, dark);
    v[0].x = center_x; v[0].y = bottom;
    v[1].x = center_x; v[1].y = bottom + thickness;
    v[2].x = right - thickness; v[2].y = top - thickness;
    v[3].x = right; v[3].y = top;
    fill_path(p, v, 4);

    set_color(p, light);
    v[0].x = left; v[0].y = top;
    v[1].x = center_x; v[1].y = bottom;
    v[2].x = center_x + delta_x; v[2].y = bottom + delta_y;
    v[3].x = left + thickness; v[3].y = top - thickness;
    fill_path(p, v, 4);

    v[0].x = left; v[0].y = top;
    v[1].x = right; v[1].y = top;
    v[2].x = right - delta_x; v[2].y = top - delta_y;
    v[3].x = left + thickness; v[3].y = top - thickness;
    fill_path(p, v, 4);
}

void Beveler::diamond(
    PainterObjRef p, Coord thickness,
    ColorRef light, ColorRef medium, ColorRef dark,
    Coord left, Coord bottom, Coord right, Coord top
) {
    Coord x_mid = (left + right) * 0.5;
    Coord y_mid = (bottom + top) * 0.5;
    Coord left_inside = left + thickness;
    Coord top_inside = top - thickness;
    Coord right_inside = right - thickness;
    Coord bottom_inside = bottom + thickness;

    Vertex v[10];

    /* interior of diamond */
    set_color(p, medium);
    v[0].x = left; v[0].y = y_mid;
    v[1].x = x_mid; v[1].y = top;
    v[2].x = right; v[2].y = y_mid;
    v[3].x = x_mid; v[3].y = bottom;
    fill_path(p, v, 4);

    /* lower half */
    set_color(p, dark);
    v[0].x = left; v[0].y = y_mid;
    v[1].x = x_mid; v[1].y = bottom;
    v[2].x = right; v[2].y = y_mid;
    v[3].x = right_inside; v[3].y = y_mid;
    v[4].x = x_mid; v[4].y = bottom_inside;
    v[5].x = left_inside; v[5].y = y_mid;
    fill_path(p, v, 6);

    /* upper half */
    set_color(p, light);
    v[0].x = left; v[0].y = y_mid;
    v[1].x = x_mid; v[1].y = top;
    v[2].x = right; v[2].y = y_mid;
    v[3].x = right_inside; v[3].y = y_mid;
    v[4].x = x_mid; v[4].y = top_inside;
    v[5].x = left_inside; v[5].y = y_mid;
    fill_path(p, v, 6);
}
