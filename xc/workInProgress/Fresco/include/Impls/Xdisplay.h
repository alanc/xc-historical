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

#ifndef Fresco_Impls_Xdisplay_h
#define Fresco_Impls_Xdisplay_h

#include <X11/Fresco/display.h>
#include <X11/Fresco/Impls/fobjects.h>
#include <X11/Fresco/Impls/styles.h>
#include <X11/Fresco/Impls/transform.h>
#include <X11/Fresco/Impls/Xlib.h>
#include <X11/Fresco/OS/thread.h>

class DisplayImplDamage;
class DisplayImplFilters;
class DisplayImplRemoveFilter;
class DisplayImplWindowTable;
class DisplayStyleImpl;
class EventImpl;
class ScreenImpl;

//+ DisplayImpl : DisplayObjType
class DisplayImpl : public DisplayObjType {
public:
    ~DisplayImpl();
    TypeObjId _tid();
    static DisplayImpl* _narrow(BaseObjectRef);
//+
public:
    DisplayImpl(Fresco*, XDisplay*);

    //+ DisplayObj::*
    /* FrescoObject */
    Long ref__(Long references);
    Tag attach(FrescoObject_in observer);
    void detach(Tag attach_tag);
    void disconnect();
    void notify_observers();
    void update();
    /* DisplayObj */
    DisplayStyleRef _c_style();
    DrawingKitRef _c_drawing_kit();
    ScreenObjRef _c_screen(DisplayObj::ScreenNumber n);
    DisplayObj::ScreenNumber number_of_screens();
    ScreenObjRef _c_default_screen();
    CursorRef _c_cursor_from_data(Short x, Short y, Long pattern[16], Long mask[16]);
    CursorRef _c_cursor_from_bitmap(Raster_in b, Raster_in mask);
    CursorRef _c_cursor_from_font(Font_in f, Long pattern, Long mask);
    CursorRef _c_cursor_from_index(Long index);
    void run(Boolean b);
    Boolean running();
    Tag add_filter(GlyphTraversal_in t);
    void remove_filter(Tag add_tag);
    void need_repair(Window_in w);
    void repair();
    void flush();
    void flush_and_wait();
    void ring_bell(Float pct_loudness);
    void close();
    //+

    struct FilterInfo {
	long tag;
	TransformImpl inverse;
	GlyphTraversalRef traversal;
    };

    Fresco* fresco() { return fresco_; }
    XDisplay* xdisplay();
    Atom wm_protocols_atom();
    Atom wm_delete_atom();
    void bind(XWindow, WindowRef);
    void unbind(XWindow);

    Boolean prepare_to_read();
    void read_event(EventImpl*);
    void after_read();
    void prepare_to_request();
    void after_requesting();
protected:
    friend class DisplayImplRemoveFilter;

    SharedFrescoObjectImpl object_;
    Fresco* fresco_;
    LockObj lock_;
    Boolean running_;
    XDisplay* xdisplay_;
    XWindow xwindow_;
    DisplayStyleImpl* style_;
    Long nscreens_;
    ScreenImpl** screen_;
    Boolean trace_events_;
    Boolean reading_;
    Long waiting_to_read_;
    Boolean requesting_;
    Long waiting_to_request_;
    ConditionVariable wait_to_read_;
    ConditionVariable wait_to_request_;
    ThreadObj redisplay_;
    LockObj damage_lock_;
    ConditionVariable damage_pending_;
    DisplayImplDamage* damaged_;
    DisplayImplFilters* filters_;
    long filter_tag_;
    DisplayImplWindowTable* wtable_;
    Atom wm_protocols_;
    Atom wm_delete_;

    void init_style();
    void load_path(const char* head, const char* tail, long priority);
    void load_path(
	const char* head, const char* middle, const char* tail, long priority
    );
    const char* home();

    void dispatch(EventImpl*);
    Boolean filtered(EventImpl*);
    void expose(WindowRef, const XExposeEvent&);
    void configure(WindowRef, const XConfigureEvent&);

    void redisplay_thread();
    void notify_redisplay();
    void do_repairs();
};

class DisplayStyleImpl : public DisplayStyleType {
public:
    DisplayStyleImpl(Fresco*, DisplayImpl*);
    ~DisplayStyleImpl();

    //+ DisplayStyle::*
    /* FrescoObject */
    Long ref__(Long references);
    Tag attach(FrescoObject_in observer);
    void detach(Tag attach_tag);
    void disconnect();
    void notify_observers();
    void update();
    /* StyleObj */
    StyleObjRef _c_new_style();
    StyleObjRef _c_parent_style();
    void link_parent(StyleObj_in parent);
    void unlink_parent();
    Tag link_child(StyleObj_in child);
    void unlink_child(Tag link_tag);
    void merge(StyleObj_in s);
    CharStringRef _c_name();
    void _c_name(CharString_in _p);
    void alias(CharString_in s);
    Boolean is_on(CharString_in name);
    StyleValueRef _c_bind(CharString_in name);
    void unbind(CharString_in name);
    StyleValueRef _c_resolve(CharString_in name);
    StyleValueRef _c_resolve_wildcard(CharString_in name, StyleObj_in start);
    Long match(CharString_in name);
    void visit_aliases(StyleVisitor_in v);
    void visit_attributes(StyleVisitor_in v);
    void visit_styles(StyleVisitor_in v);
    void lock();
    void unlock();
    /* DisplayStyle */
    Boolean auto_repeat();
    void auto_repeat(Boolean _p);
    Float key_click_volume();
    void key_click_volume(Float _p);
    Float pointer_acceleration();
    void pointer_acceleration(Float _p);
    Long pointer_threshold();
    void pointer_threshold(Long _p);
    //+

    SharedStyleImpl* impl() { return &impl_; }
protected:
    SharedFrescoObjectImpl object_;
    SharedStyleImpl impl_;
    DisplayImpl* display_;
};

#endif
