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

#ifndef Fresco_Impls_viewer_h
#define Fresco_Impls_viewer_h

#include <X11/Fresco/viewer.h>
#include <X11/Fresco/Impls/glyphs.h>
#include <X11/Fresco/Impls/styles.h>

class RegionImpl;

//- ViewerImpl*
class ViewerImpl : public ViewerType, public StyleObjType {
    //. ViewerImpl is an implementation of the viewer interface.
    //. Because every viewer has one and only one distinct style,
    //. ViewerImpl also implements the StyleObj interface directly.
    //. This approach means there is only one object for a viewer and
    //. its style.
    //.
    //. ViewerImpl also adds some implementation-specific virtual functions
    //. for input handling.  ViewerImpl implements Viewer::handle
    //. by inspecting the event type and calling the appropriate
    //. virtual function.
public:
    ViewerImpl(Fresco*);
    ~ViewerImpl();

    //+ Viewer::*
    /* FrescoObject */
    Long ref__(Long references);
    Tag attach(FrescoObjectRef observer);
    void detach(Tag attach_tag);
    void disconnect();
    void notify_observers();
    void update();
    /* Glyph */
    GlyphRef _c_clone_glyph();
    StyleObjRef _c_style();
    void _c_style(StyleObjRef _p);
    TransformObjRef _c_transform();
    void request(Glyph::Requisition& r);
    void extension(const Glyph::AllocationInfo& a, RegionRef r);
    void shape(RegionRef r);
    void traverse(GlyphTraversalRef t);
    void draw(GlyphTraversalRef t);
    void pick(GlyphTraversalRef t);
    GlyphRef _c_body();
    void _c_body(GlyphRef _p);
    GlyphOffsetRef _c_append(GlyphRef g);
    GlyphOffsetRef _c_prepend(GlyphRef g);
    Tag add_parent(GlyphOffsetRef parent_offset);
    void remove_parent(Tag add_tag);
    void visit_children(GlyphVisitorRef v);
    void visit_children_reversed(GlyphVisitorRef v);
    void visit_parents(GlyphVisitorRef v);
    void allocations(Glyph::AllocationInfoList& a);
    void need_redraw();
    void need_redraw_region(RegionRef r);
    void need_resize();
    /* Viewer */
    ViewerRef _c_parent_viewer();
    ViewerRef _c_next_viewer();
    ViewerRef _c_prev_viewer();
    ViewerRef _c_first_viewer();
    ViewerRef _c_last_viewer();
    void append_viewer(ViewerRef v);
    void prepend_viewer(ViewerRef v);
    void insert_viewer(ViewerRef v);
    void replace_viewer(ViewerRef v);
    void remove_viewer();
    void set_viewer_links(ViewerRef parent, ViewerRef prev, ViewerRef next);
    void set_first_viewer(ViewerRef v);
    void set_last_viewer(ViewerRef v);
    FocusRef _c_request_focus(ViewerRef requestor, Boolean temporary);
    Boolean receive_focus(FocusRef f, Boolean primary);
    void lose_focus(Boolean temporary);
    Boolean first_focus();
    Boolean last_focus();
    Boolean next_focus();
    Boolean prev_focus();
    Boolean handle(GlyphTraversalRef t, EventRef e);
    void close();
    //+

    //+ StyleObj::=
    StyleObjRef _c_new_style();
    StyleObjRef _c_parent_style();
    void link_parent(StyleObjRef parent);
    void unlink_parent();
    Tag link_child(StyleObjRef child);
    void unlink_child(Tag link_tag);
    void merge(StyleObjRef s);
    CharStringRef _c_name();
    void _c_name(CharStringRef _p);
    void alias(CharStringRef s);
    Boolean is_on(CharStringRef name);
    StyleValueRef _c_bind(CharStringRef name);
    void unbind(CharStringRef name);
    StyleValueRef _c_resolve(CharStringRef name);
    StyleValueRef _c_resolve_wildcard(CharStringRef name, StyleObjRef start);
    Long match(CharStringRef name);
    void visit_aliases(StyleVisitorRef v);
    void visit_attributes(StyleVisitorRef v);
    void visit_styles(StyleVisitorRef v);
    void lock();
    void unlock();
    //+

    //- position_event
    virtual Boolean position_event(GlyphTraversalRef t, EventRef e);
	//. Handle a event for a positional device, including as pointer
	//. motion, button press, and button release.  The default
	//. implementation does a pick to see if the event
	//. is contained in a nested viewer; if so, then the event
	//. is passed to the nested viewer.  Otherwise, this operation
	//. calls handle_position_event.

    //- handle_position_event
    virtual Boolean handle_position_event(GlyphTraversalRef t, EventRef e);
	//. This operation looks at the type of event and calls
	//. move, press, drag, or whatever the appropriate function is.
	//. The chosen function depends on some state as well
	//. as the event type.  If a pointer button has been pressed,
	//. (and is still down), then a motion event will call drag,
	//. otherwise it will call move.  A button-down event
	//. will be ignored if a button is already down, otherwise
	//. it will call press and grab.  A button-up event
	//. will call ungrab and either release or double_click,
	//. depending on whether the timestamp of the up event
	//. is within a preset threshold.

    //- focus_event
    virtual Boolean focus_event(GlyphTraversalRef t, EventRef e);
	//. Handle an event from a focusable device, such as a keyboard.
	//. This operation looks at the type of event and calls
	//. key_press, key_release, or whatever the appropriate function is.

    //- move
    virtual Boolean move(GlyphTraversalRef t, EventRef e);
	//. Respond to a pointer motion event.  The default
	//. is to do nothing and return false.

    //- press
    virtual Boolean press(GlyphTraversalRef t, EventRef e);
	//. Respond to a button press event.  The default
	//. is to do nothing and return false.

    //- drag
    virtual Boolean drag(GlyphTraversalRef t, EventRef e);
	//. Respond to a pointer motion event while a button
	//. is pressed.  The default is to do nothing and return false.

    //- release
    virtual Boolean release(GlyphTraversalRef t, EventRef e);
	//. Respond to a button up event.  The default
	//. is to do nothing and return false.

    //- double_click
    virtual Boolean double_click(GlyphTraversalRef t, EventRef e);
	//. Respond to a button up event that rapidly follows
	//. a previous up event.  The default is to call release and
	//. return the value from release.

    //- key_press
    virtual Boolean key_press(GlyphTraversalRef t, EventRef e);
	//. Respond to a key being pressed.  The default is to do
	//. nothing and return false.

    //- key_release
    virtual Boolean key_release(GlyphTraversalRef t, EventRef e);
	//. Respond to a key being released.  The default is to do
	//. nothing and return false.

    //- other
    virtual Boolean other(GlyphTraversalRef t, EventRef e);
	//. Respond to an implementation-dependent event.  The default
	//. is to do nothing and return false.

    //- inside
    virtual Boolean inside(GlyphTraversalRef t);
	//. Test whether the given traversal state has clipping set
	//. that intersects with this viewer.  This operation is useful
	//. for testing if a positional event is inside a viewer
	//. when the positional event is the result of a grab
	//. (other positional events will necessarily be inside the viewer,
	//. otherwise it wouldn't be given the event to handle).

    //- grab
    virtual void grab(GlyphTraversalRef t);
	//. Set up a filter on the display (see Display::add_filter)
	//. so that further positional events will be passed directly
	//. to this viewer's handle_position_event operation.

    //- ungrab
    virtual void ungrab();
	//. Remove the filter for this viewer.  If the same viewer
	//. calls several grabs, then only one filter will be created
	//. but it will not be removed until a corresponding number
	//. of ungrab calls are made.
protected:
    SharedFrescoObjectImpl object_;
    SharedStyleImpl style_;
    GlyphOffsetList glyph_parents_;
    MonoGlyphOffset offset_;
    ViewerRef parent_;
    ViewerRef next_;
    ViewerRef prev_;
    ViewerRef first_;
    ViewerRef last_;

    Boolean entered_;
    Boolean pressed_;
    Event::ButtonIndex button_;
    Boolean recorded_time_;
    unsigned long click_time_;
    unsigned long threshold_;
    DisplayObjRef display_;
    Tag filter_;
};

//- ActiveViewer*
class ActiveViewer : public ViewerImpl {
    //. The ActiveViewer class checks motion events by implementing
    //. ViewerImpl::move and ViewerImpl::drag to notice when
    //. the pointer enters or leaves the viewer.
public:
    ActiveViewer(Fresco*);
    ~ActiveViewer();

    Boolean move(GlyphTraversalRef, EventRef);
    Boolean drag(GlyphTraversalRef, EventRef);

    //- enter
    virtual void enter();
	//. Notice that the pointer is now inside the viewer.  The default
	//. is to do nothing.

    //- leave
    virtual void leave();
	//. Notice that the pointer is now outside the viewer.  The default
	//. is to do nothing.
};

class FocusImpl : public FocusType {
public:
    FocusImpl(Fresco*);
    ~FocusImpl();

    //+ Focus::*
    /* FrescoObject */
    Long ref__(Long references);
    Tag attach(FrescoObjectRef observer);
    void detach(Tag attach_tag);
    void disconnect();
    void notify_observers();
    void update();
    /* Focus */
    void add_focus_interest(ViewerRef v);
    void receive_focus_below(ViewerRef v, Boolean temporary);
    void lose_focus_below(ViewerRef v, Boolean temporary);
    void map_keystroke(Event::KeySym k, ActionRef a);
    void map_keychord(const Event::KeyChord& k, ActionRef a);
    //+
protected:
    SharedFrescoObjectImpl object_;
};

class MainViewer : public ViewerImpl {
public:
    MainViewer(Fresco*);
    ~MainViewer();

    void notify_observers(); //+ FrescoObject::notify_observers
    void allocations(Glyph::AllocationInfoList& a); //+ Glyph::allocations
    void traverse(GlyphTraversalRef t); //+ Glyph::traverse
    void draw(GlyphTraversalRef t); //+ Glyph::draw
    void need_redraw(); //+ Glyph::need_redraw
    void need_resize(); //+ Glyph::need_resize
    FocusRef _c_request_focus(ViewerRef requestor, Boolean temporary); //+ Viewer::request_focus
    void close(); //+ Viewer::close

    WindowRef window_;
    RegionImpl* allocation_;
protected:
    FocusImpl* focus_;
    Boolean valid_;
    ColorRef background_;
    DamageObjRef damage_;

    void cache();
    void invalidate();
};

#endif
