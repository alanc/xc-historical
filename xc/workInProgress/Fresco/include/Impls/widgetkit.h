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

/*
 * Kit for common widgets
 */

#ifndef Fresco_Impls_widgetkit_h
#define Fresco_Impls_widgetkit_h

#include <X11/Fresco/widgets.h>
#include <X11/Fresco/Impls/fobjects.h>
#include <X11/Fresco/Impls/viewers.h>

//- ButtonImpl*
class ButtonImpl : public ButtonType, public ActiveViewer {
    //. This class implements the Button interface using
    //. the ActiveViewer implementation for noticing when
    //. motion enters or leaves the button.  The button
    //. modifies the telltale state in response to input, and
    //. observes the telltale state to cause a redraw whenever
    //. the state changes.
public:
    ButtonImpl(Fresco*, TelltaleRef, ActionRef);
    ~ButtonImpl();

    //+ Button::*
    /* FrescoObject */
    Long ref__(Long references);
    Tag attach(FrescoObject_in observer);
    void detach(Tag attach_tag);
    void disconnect();
    void notify_observers();
    void update();
    /* Glyph */
    GlyphRef _c_clone_glyph();
    StyleObjRef _c_style();
    void _c_style(StyleObj_in _p);
    TransformObjRef _c_transform();
    void request(Glyph::Requisition& r);
    void extension(const Glyph::AllocationInfo& a, Region_in r);
    RegionRef _c_shape();
    void traverse(GlyphTraversal_in t);
    void draw(GlyphTraversal_in t);
    void pick(GlyphTraversal_in t);
    GlyphRef _c_body();
    void _c_body(Glyph_in _p);
    GlyphOffsetRef _c_append(Glyph_in g);
    GlyphOffsetRef _c_prepend(Glyph_in g);
    Tag add_parent(GlyphOffset_in parent_offset);
    void remove_parent(Tag add_tag);
    void visit_children(GlyphVisitor_in v);
    void visit_children_reversed(GlyphVisitor_in v);
    void visit_parents(GlyphVisitor_in v);
    void allocations(Glyph::AllocationInfoList& a);
    void need_redraw();
    void need_redraw_region(Region_in r);
    void need_resize();
    Boolean restore_trail(GlyphTraversal_in t);
    /* Viewer */
    ViewerRef _c_parent_viewer();
    ViewerRef _c_next_viewer();
    ViewerRef _c_prev_viewer();
    ViewerRef _c_first_viewer();
    ViewerRef _c_last_viewer();
    void append_viewer(Viewer_in v);
    void prepend_viewer(Viewer_in v);
    void insert_viewer(Viewer_in v);
    void replace_viewer(Viewer_in v);
    void remove_viewer();
    void set_viewer_links(Viewer_in parent, Viewer_in prev, Viewer_in next);
    void set_first_viewer(Viewer_in v);
    void set_last_viewer(Viewer_in v);
    FocusRef _c_request_focus(Viewer_in requestor, Boolean temporary);
    Boolean receive_focus(Focus_in f, Boolean primary);
    void lose_focus(Boolean temporary);
    Boolean first_focus();
    Boolean last_focus();
    Boolean next_focus();
    Boolean prev_focus();
    Boolean handle(GlyphTraversal_in t, Event_in e);
    void close();
    /* Button */
    TelltaleRef _c_state();
    void _c_state(Telltale_in _p);
    ActionRef _c_click_action();
    void _c_click_action(Action_in _p);
    //+

    Boolean press(GlyphTraversalRef, EventRef);
    Boolean release(GlyphTraversalRef, EventRef);
    Boolean inside(GlyphTraversalRef);

    void enter();
    void leave();

    virtual void click();
protected:
    TelltaleRef state_;
    ActionRef action_;
    Tag tag_;

    void detach_state();
};

//- TelltaleImpl*
class TelltaleImpl : public TelltaleType {
    //. The TelltaleImpl class implements the Telltale interface.
public:
    //- FlagSet
    typedef short FlagSet;
    enum {
	off = 0,
	enabled_bit = 1 << Telltale::enabled,
	visible_bit = 1 << Telltale::visible,
	active_bit = 1 << Telltale::active,
	chosen_bit = 1 << Telltale::chosen,
	running_bit = 1 << Telltale::running,
	stepping_bit = 1 << Telltale::stepping,
	choosable_bit = 1 << Telltale::choosable,
	toggle_bit = 1 << Telltale::toggle
    };
	//. We use a short to represent the set of possible telltale flags,
	//. meaning we must define the enumeration as a separate type
	//. (because C++ enumerations are always 32 bits).
	//. The encoding of bits here assumes that the Telltale::Flag
	//. enumeration defines constants that start at 0 and increment by 1.

    TelltaleImpl(FlagSet f = TelltaleImpl::off);
    ~TelltaleImpl();

    //+ Telltale::*
    /* FrescoObject */
    Long ref__(Long references);
    Tag attach(FrescoObject_in observer);
    void detach(Tag attach_tag);
    void disconnect();
    void notify_observers();
    void update();
    /* Telltale */
    void set(Telltale::Flag f);
    void clear(Telltale::Flag f);
    Boolean test(Telltale::Flag f);
    TelltaleRef _c_current();
    void _c_current(Telltale_in _p);
    //+

    //- flags
    TelltaleImpl::FlagSet flags();
	//. Return the current set of "visibility" flags, which only
	//. includes enabled, visible, active, and chosen.  This function
	//. is useful for indexing an array of colors, one per set
	//. of states.
protected:
    SharedFrescoObjectImpl object_;
    TelltaleImpl::FlagSet flags_;
    TelltaleRef current_;

    void modify(Telltale::Flag f, Boolean on);
};

inline TelltaleImpl::FlagSet TelltaleImpl::flags() {
    return flags_ & 0xf;
}

//- Beveler*
class Beveler : public MonoGlyph {
    //. The beveler class draws a frame around its body and provides
    //. convenience operations for drawing bevelled objects.
public:
    //- Beveler
    Beveler(
	Coord thickness, Alignment x = 0.0, Alignment y = 0.0,
	Boolean hmargin = true, Boolean vmargin = true
    );
	//. The constructor is given the frame thickness,
	//. optional x and y alignments, and optional margin flags.
	//. If the margin flags are true, then the beveler will
	//. allocate its body inside a frame of the given thickness;
	//. otherwise the body will have the same allocation
	//. as the beveler.

    virtual ~Beveler();

    void request(Glyph::Requisition& r); //+ Glyph::request
    void extension(const Glyph::AllocationInfo& a, Region_in r); //+ Glyph::extension
    void traverse(GlyphTraversal_in t); //+ Glyph::traverse

    RegionRef allocate_body(GlyphTraversalRef t);
    void allocate_span(
	Glyph::Requirement&, Region::BoundingSpan&, Coord margin, Alignment
    );

    //- set_color
    static void set_color(PainterObjRef p, ColorRef c);
	//. Short-hand for ref'ing the color and then setting
	//. the painter's color.

    //- fill_path
    static void fill_path(PainterObjRef p, Vertex* v, long n);
	//. Short-hand for filling a path containing the given vertices.

    //- rect
    static void rect(
	PainterObjRef p, Coord thickness,
	ColorRef light, ColorRef medium, ColorRef dark,
	Coord left, Coord bottom, Coord right, Coord top
    );
	//. Draw a beveled rectangle with the given thickness,
	//. upper-left color (light), middle color (medium), and
	//. lower-right color (dark).  If medium is nil, then
	//. no drawing is done on the interior of the rectangle.

    //- left_arrow
    static void left_arrow(
	PainterObjRef p, Coord thickness,
	ColorRef light, ColorRef medium, ColorRef dark,
	Coord left, Coord bottom, Coord right, Coord top
    );
	//. Draw a beveled triangle with base parallel to the Y-axis
	//. on the right and sides meeting in the middle on the left.
	//. This function is a bit misnamed, but such a triangle is
	//. typically used to represent a left-arrow.

    //- right_arrow
    static void right_arrow(
	PainterObjRef p, Coord thickness,
	ColorRef light, ColorRef medium, ColorRef dark,
	Coord left, Coord bottom, Coord right, Coord top
    );
	//. Draw a beveled triangle with base parallel to the Y-axis
	//. on the left and sides meeting in the middle on the right.
	//. This function is a bit misnamed, but such a triangle is
	//. typically used to represent a right-arrow.

    //- up_arrow
    static void up_arrow(
	PainterObjRef p, Coord thickness,
	ColorRef light, ColorRef medium, ColorRef dark,
	Coord left, Coord bottom, Coord right, Coord top
    );
	//. Draw a beveled triangle with base parallel to the X-axis
	//. on the bottom and sides meeting in the middle on the top.
	//. This function is a bit misnamed, but such a triangle is
	//. typically used to represent an up-arrow.

    //- down_arrow
    static void down_arrow(
	PainterObjRef p, Coord thickness,
	ColorRef light, ColorRef medium, ColorRef dark,
	Coord left, Coord bottom, Coord right, Coord top
    );
	//. Draw a beveled triangle with base parallel to the X-axis
	//. on the top and sides meeting in the middle on the bottom.
	//. This function is a bit misnamed, but such a triangle is
	//. typically used to represent a down-arrow.

    //- diamond
    static void diamond(
	PainterObjRef p, Coord thickness,
	ColorRef light, ColorRef medium, ColorRef dark,
	Coord left, Coord bottom, Coord right, Coord top
    );
	//. Draw a diamond with sides that meet in the middle
	//. on all four edges.
protected:
    Coord thickness_;
    Alignment xalign_;
    Alignment yalign_;
    Boolean hmargin_ : 1, vmargin_ : 1;
    RegionImpl* allocation_;

    Coord thickness(PainterObjRef);
};

//- WidgetKitImpl*
class WidgetKitImpl : public WidgetKitType {
    //. WidgetKitImpl provides an implementation of the WidgetKit
    //. interface that creates objects with a Motif-like look and feel.
    //. The bevelled look is slightly enhanced by using several levels
    //. of shading instead of a single color change.
public:
    WidgetKitImpl(Fresco*);
    ~WidgetKitImpl();

    //+ WidgetKit::*
    /* FrescoObject */
    Long ref__(Long references);
    Tag attach(FrescoObject_in observer);
    void detach(Tag attach_tag);
    void disconnect();
    void notify_observers();
    void update();
    /* WidgetKit */
    GlyphRef _c_inset_frame(Glyph_in g);
    GlyphRef _c_outset_frame(Glyph_in g);
    GlyphRef _c_bright_inset_frame(Glyph_in g);
    ViewerRef _c_label(CharString_in s);
    MenuRef _c_menubar();
    MenuRef _c_pulldown();
    MenuRef _c_pullright();
    MenuItemRef _c_menubar_item(Glyph_in g);
    MenuItemRef _c_menu_item(Glyph_in g);
    MenuItemRef _c_check_menu_item(Glyph_in g);
    MenuItemRef _c_radio_menu_item(Glyph_in g, Telltale_in group);
    MenuItemRef _c_menu_item_separator();
    TelltaleRef _c_telltale_group();
    ButtonRef _c_push_button(Glyph_in g, Action_in a);
    ButtonRef _c_default_button(Glyph_in g, Action_in a);
    ButtonRef _c_palette_button(Glyph_in g, Action_in a);
    ButtonRef _c_check_box(Glyph_in g, Action_in a);
    ButtonRef _c_radio_button(Glyph_in g, Action_in a, Telltale_in group);
    ViewerRef _c_slider(Axis a, Adjustment_in adj);
    ViewerRef _c_scroll_bar(Axis a, Adjustment_in adj);
    ViewerRef _c_panner(Adjustment_in x, Adjustment_in y);
    ButtonRef _c_zoomer(Coord scale, Adjustment_in x, Adjustment_in y, Adjustment_in z);
    ButtonRef _c_up_mover(Adjustment_in a);
    ButtonRef _c_down_mover(Adjustment_in a);
    ButtonRef _c_left_mover(Adjustment_in a);
    ButtonRef _c_right_mover(Adjustment_in a);
    //+

    struct Info {
	Coord thickness;
	float check_size;
	float radio_scale;
	Coord mover_size;
	Coord slider_size;
	Coord min_button_width;
	long num_colors;
	FontRef font;
	ColorRef* color;
	RasterRef checkmark;
	ColorRef checkmark_color;
	RasterRef shadow1;
	RasterRef shadow2;
    };

    ColorRef brightness(ColorRef c, float adjust);
protected:
    SharedFrescoObjectImpl object_;
    Fresco* fresco_;
    StyleObjRef style_;
    WidgetKitImpl::Info info_;

    void load();
    RasterRef make_bitmap(
	DrawingKitRef, const char*, unsigned int width, unsigned int height
    );
    void load_coord(const char*, Coord&);
    void load_float(const char*, float&);
    void unload();
};

#endif
