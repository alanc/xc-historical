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
 * Layout - geometry management
 */

#include <X11/Fresco/drawing.h>
#include <X11/Fresco/fresco.h>
#include <X11/Fresco/layouts.h>
#include <X11/Fresco/Impls/fobjects.h>
#include <X11/Fresco/Impls/fresco-impl.h>
#include <X11/Fresco/Impls/glyphs.h>
#include <X11/Fresco/Impls/polyglyph.h>
#include <X11/Fresco/Impls/region.h>
#include <X11/Fresco/OS/math.h>

#define infinite_coord 10e6;

class DeckImpl : public DeckObjType, public PolyGlyph {
public:
    DeckImpl();
    ~DeckImpl();

    //+ DeckObj::*
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
    /* DeckObj */
    GlyphOffsetRef _c_card();
    void flip_to(GlyphOffsetRef off);
    //+

    void modified();
protected:
    GlyphOffsetRef top_;
    Boolean requested_;
#if defined(__DECCXX)
    /* workaround for DEC CXX bug */
    DeckObj::Requisition requisition_;
#else
    Glyph::Requisition requisition_;
#endif
};

/*
 * ScrollBox -- NOT IMPLEMENTED YET
 */

class LayoutKitImpl : public LayoutKitType {
public:
    LayoutKitImpl();
    ~LayoutKitImpl();

    //+ LayoutKit::*
    /* FrescoObject */
    Long ref__(Long references);
    Tag attach(FrescoObjectRef observer);
    void detach(Tag attach_tag);
    void disconnect();
    void notify_observers();
    void update();
    /* LayoutKit */
    Coord fil();
    void fil(Coord _p);
    GlyphRef _c_hbox();
    GlyphRef _c_vbox();
    GlyphRef _c_hbox_first_aligned();
    GlyphRef _c_vbox_first_aligned();
    ScrollBoxRef _c_vscrollbox();
    GlyphRef _c_overlay();
    DeckObjRef _c_deck();
    GlyphRef _c_back(GlyphRef g, GlyphRef under);
    GlyphRef _c_front(GlyphRef g, GlyphRef over);
    GlyphRef _c_between(GlyphRef g, GlyphRef under, GlyphRef over);
    GlyphRef _c_glue(Axis a, Coord natural, Coord stretch, Coord shrink, Alignment align);
    GlyphRef _c_glue_requisition(const Glyph::Requisition& r);
    GlyphRef _c_hfil();
    GlyphRef _c_hglue_fil(Coord natural);
    GlyphRef _c_hglue(Coord natural, Coord stretch, Coord shrink);
    GlyphRef _c_hglue_aligned(Coord natural, Coord stretch, Coord shrink, Alignment a);
    GlyphRef _c_hspace(Coord natural);
    GlyphRef _c_vfil();
    GlyphRef _c_vglue_fil(Coord natural);
    GlyphRef _c_vglue(Coord natural, Coord stretch, Coord shrink);
    GlyphRef _c_vglue_aligned(Coord natural, Coord stretch, Coord shrink, Alignment a);
    GlyphRef _c_vspace(Coord natural);
    GlyphRef _c_shape_of(GlyphRef g);
    GlyphRef _c_shape_of_xy(GlyphRef gx, GlyphRef gy);
    GlyphRef _c_shape_of_xyz(GlyphRef gx, GlyphRef gy, GlyphRef gz);
    GlyphRef _c_strut(FontRef f, Coord natural, Coord stretch, Coord shrink);
    GlyphRef _c_hstrut(Coord right_bearing, Coord left_bearing, Coord natural, Coord stretch, Coord shrink);
    GlyphRef _c_vstrut(Coord ascent, Coord descent, Coord natural, Coord stretch, Coord shrink);
    GlyphRef _c_spaces(Long count, Coord each, FontRef f, ColorRef c);
    GlyphRef _c_center(GlyphRef g);
    GlyphRef _c_center_aligned(GlyphRef g, Alignment x, Alignment y);
    GlyphRef _c_center_axis(GlyphRef g, Axis a, Alignment align);
    GlyphRef _c_hcenter(GlyphRef g);
    GlyphRef _c_hcenter_aligned(GlyphRef g, Alignment x);
    GlyphRef _c_vcenter(GlyphRef g);
    GlyphRef _c_vcenter_aligned(GlyphRef g, Alignment y);
    GlyphRef _c_fixed(GlyphRef g, Coord x, Coord y);
    GlyphRef _c_fixed_axis(GlyphRef g, Axis a, Coord size);
    GlyphRef _c_hfixed(GlyphRef g, Coord x);
    GlyphRef _c_vfixed(GlyphRef g, Coord y);
    GlyphRef _c_flexible(GlyphRef g, Coord stretch, Coord shrink);
    GlyphRef _c_flexible_fil(GlyphRef g);
    GlyphRef _c_flexible_axis(GlyphRef g, Axis a, Coord stretch, Coord shrink);
    GlyphRef _c_hflexible(GlyphRef g, Coord stretch, Coord shrink);
    GlyphRef _c_vflexible(GlyphRef g, Coord stretch, Coord shrink);
    GlyphRef _c_natural(GlyphRef g, Coord x, Coord y);
    GlyphRef _c_natural_axis(GlyphRef g, Axis a, Coord size);
    GlyphRef _c_hnatural(GlyphRef g, Coord x);
    GlyphRef _c_vnatural(GlyphRef g, Coord y);
    GlyphRef _c_margin(GlyphRef g, Coord all);
    GlyphRef _c_margin_lrbt(GlyphRef g, Coord lmargin, Coord rmargin, Coord bmargin, Coord tmargin);
    GlyphRef _c_margin_lrbt_flexible(GlyphRef g, Coord lmargin, Coord lstretch, Coord lshrink, Coord rmargin, Coord rstretch, Coord rshrink, Coord bmargin, Coord bstretch, Coord bshrink, Coord tmargin, Coord tstretch, Coord tshrink);
    GlyphRef _c_hmargin(GlyphRef g, Coord both);
    GlyphRef _c_hmargin_lr(GlyphRef g, Coord lmargin, Coord rmargin);
    GlyphRef _c_hmargin_lr_flexible(GlyphRef g, Coord lmargin, Coord lstretch, Coord lshrink, Coord rmargin, Coord rstretch, Coord rshrink);
    GlyphRef _c_vmargin(GlyphRef g, Coord both);
    GlyphRef _c_vmargin_bt(GlyphRef g, Coord bmargin, Coord tmargin);
    GlyphRef _c_vmargin_bt_flexible(GlyphRef g, Coord bmargin, Coord bstretch, Coord bshrink, Coord tmargin, Coord tstretch, Coord tshrink);
    GlyphRef _c_lmargin(GlyphRef g, Coord natural);
    GlyphRef _c_lmargin_flexible(GlyphRef g, Coord natural, Coord stretch, Coord shrink);
    GlyphRef _c_rmargin(GlyphRef g, Coord natural);
    GlyphRef _c_rmargin_flexible(GlyphRef g, Coord natural, Coord stretch, Coord shrink);
    GlyphRef _c_bmargin(GlyphRef g, Coord natural);
    GlyphRef _c_bmargin_flexible(GlyphRef g, Coord natural, Coord stretch, Coord shrink);
    GlyphRef _c_tmargin(GlyphRef g, Coord natural);
    GlyphRef _c_tmargin_flexible(GlyphRef g, Coord natural, Coord stretch, Coord shrink);
    //+
protected:
    Coord fil_;
    SharedFrescoObjectImpl object_;

    void init_types();
};

LayoutKitRef FrescoImpl::create_layout_kit() {
    return new LayoutKitImpl;
}

/* Helper classes for LayoutKitImpl */

/*
 * LayoutManager -- determine appropriate allocations for a set of requests
 */

class LayoutManager : public FrescoObjectType {
protected:
    LayoutManager();
    virtual ~LayoutManager();
public:
    typedef RegionImpl** Allocations;

    //+ FrescoObject::*
    /* FrescoObject */
    Long ref__(Long references);
    Tag attach(FrescoObjectRef observer);
    void detach(Tag attach_tag);
    void disconnect();
    void notify_observers();
    void update();
    //+

    virtual void request(
	long n, Glyph::Requisition* requests, Glyph::Requisition& result
    ) = 0;
    virtual void allocate(
	long n, Glyph::Requisition* requests, RegionRef given,
	LayoutManager::Allocations result
    ) = 0;

    static void set_span(
	RegionImpl* r, Axis a, Coord origin, Coord length, Alignment align
    );
protected:
    SharedFrescoObjectImpl object_;
};

LayoutManager::LayoutManager() { }
LayoutManager::~LayoutManager() { }

//+ LayoutManager(FrescoObject::=object_.)
Long LayoutManager::ref__(Long references) {
    return object_.ref__(references);
}
Tag LayoutManager::attach(FrescoObjectRef observer) {
    return object_.attach(observer);
}
void LayoutManager::detach(Tag attach_tag) {
    object_.detach(attach_tag);
}
void LayoutManager::disconnect() {
    object_.disconnect();
}
void LayoutManager::notify_observers() {
    object_.notify_observers();
}
void LayoutManager::update() {
    object_.update();
}
//+

void LayoutManager::set_span(
    RegionImpl* r, Axis a, Coord origin, Coord length, Alignment align
) {
    Coord begin = origin - length * align;
    Coord end = begin + length;
    switch (a) {
    case X_axis:
	r->lower_.x = begin;
	r->upper_.x = end;
	r->xalign_ = align;
	break;
    case Y_axis:
	r->lower_.y = begin;
	r->upper_.y = end;
	r->yalign_ = align;
	break;
    case Z_axis:
	r->lower_.z = begin;
	r->upper_.z = end;
	r->zalign_ = align;
	break;
    }
}

/*
 * LayoutAlign -- align positions along an axis
 */

class LayoutAlign : public LayoutManager {
public:
    LayoutAlign(Axis);
    virtual ~LayoutAlign();

    virtual void request(
	long n, Glyph::Requisition* requests, Glyph::Requisition& result
    );
    virtual void allocate(
	long n, Glyph::Requisition* requests, RegionRef given,
	LayoutManager::Allocations result
    );
private:
    Axis axis_;
};

/*
 * LayoutCenter -- center positions along an axis
 */

class LayoutCenter : public LayoutManager {
public:
    LayoutCenter(Axis, Alignment a);
    virtual ~LayoutCenter();

    virtual void request(
	long n, Glyph::Requisition* requests, Glyph::Requisition& result
    );
    virtual void allocate(
	long n, Glyph::Requisition* requests, RegionRef given,
	LayoutManager::Allocations result
    );
private:
    Axis axis_;
    Alignment alignment_;
};

/*
 * LayoutFixed -- set size along an axis
 */

class LayoutFixed : public LayoutManager {
public:
    LayoutFixed(Axis, Coord size);
    virtual ~LayoutFixed();

    virtual void request(
	long n, Glyph::Requisition* requests, Glyph::Requisition& result
    );
    virtual void allocate(
	long n, Glyph::Requisition* requests, RegionRef given,
	LayoutManager::Allocations result
    );
private:
    Axis axis_;
    Coord size_;
};

/*
 * LayoutVariable -- allow flexibility along an axis
 */

class LayoutVariable : public LayoutManager {
public:
    LayoutVariable(Axis, Coord stretch, Coord shrink);
    virtual ~LayoutVariable();

    virtual void request(
	long n, Glyph::Requisition* requests, Glyph::Requisition& result
    );
    virtual void allocate(
	long n, Glyph::Requisition* requests, RegionRef given,
	LayoutManager::Allocations result
    );
private:
    Axis axis_;
    Coord stretch_;
    Coord shrink_;
};

/*
 * LayoutNatural -- set the natural size along an axis
 */

class LayoutNatural : public LayoutManager {
public:
    LayoutNatural(Axis, Coord natural);
    virtual ~LayoutNatural();

    virtual void request(
	long n, Glyph::Requisition* requests, Glyph::Requisition& result
    );
    virtual void allocate(
	long n, Glyph::Requisition* requests, RegionRef given,
	LayoutManager::Allocations result
    );
private:
    Axis axis_;
    Coord natural_;
};

/*
 * LayoutMargin -- leave a margin around the sides
 */

class LayoutMargin : public LayoutManager {
public:
    LayoutMargin(Coord margin);
    LayoutMargin(Coord hmargin, Coord vmargin);
    LayoutMargin(Coord lmargin, Coord rmargin, Coord bmargin, Coord tmargin);
    LayoutMargin(
        Coord lmargin, Coord lstretch, Coord lshrink,
        Coord rmargin, Coord rstretch, Coord rshrink,
        Coord bmargin, Coord bstretch, Coord bshrink,
        Coord tmargin, Coord tstretch, Coord tshrink
    );
    virtual ~LayoutMargin();

    virtual void request(
	long n, Glyph::Requisition* requests, Glyph::Requisition& result
    );
    virtual void allocate(
	long n, Glyph::Requisition* requests, RegionRef given,
	LayoutManager::Allocations result
    );
private:
    Coord lnatural_, lstretch_, lshrink_;
    Coord rnatural_, rstretch_, rshrink_;
    Coord bnatural_, bstretch_, bshrink_;
    Coord tnatural_, tstretch_, tshrink_;
    Glyph::Requisition requisition_;

    void allocate_axis(
	Axis,
	Coord natural_lead, Coord stretch_lead, Coord shrink_lead,
	Coord natural_trail, Coord stretch_trail, Coord shrink_trail,
	LayoutManager::Allocations result
    );
    static Coord span(
	Coord span, Glyph::Requirement& total,
	Coord natural, Coord stretch, Coord shrink
    );
};

/*
 * LayoutSuperpose - composite layout manager
 */

class LayoutSuperpose : public LayoutManager {
public:
    LayoutSuperpose(LayoutManager*, LayoutManager*);
    LayoutSuperpose(LayoutManager*, LayoutManager*, LayoutManager*);
    virtual ~LayoutSuperpose();

    virtual void request(
	long n, Glyph::Requisition* requests, Glyph::Requisition& result
    );
    virtual void allocate(
	long n, Glyph::Requisition* requests, RegionRef given,
	LayoutManager::Allocations result
    );
private:
    LayoutManager* first_;
    LayoutManager* second_;
    LayoutManager* third_;
};

/*
 * LayoutTile -- side-by-side, first-to-last along an axis
 */

class LayoutTile : public LayoutManager {
public:
    LayoutTile(Axis);
    virtual ~LayoutTile();

    virtual void request(
	long n, Glyph::Requisition* requests, Glyph::Requisition& result
    );
    virtual void allocate(
	long n, Glyph::Requisition* requests, RegionRef given,
	LayoutManager::Allocations result
    );

    static void compute_request(
	Axis a, Alignment align,
	long n, Glyph::Requisition* requests, Glyph::Requisition& result
    );
    static void compute_allocations(
	Axis a, Glyph::Requisition& total, Boolean first_aligned,
	long n, Glyph::Requisition* requests, RegionRef given,
	LayoutManager::Allocations result
    );
    static Coord compute_length(
	const Glyph::Requirement& r, const Region::BoundingSpan& span
    );
    static float compute_squeeze(const Glyph::Requirement& r, Coord length);
private:
    Axis axis_;
    Glyph::Requisition requisition_;
};

/*
 * LayoutTileReversed -- side-by-side, last-to-first
 */

class LayoutTileReversed : public LayoutManager {
public:
    LayoutTileReversed(Axis);
    virtual ~LayoutTileReversed();

    virtual void request(
	long n, Glyph::Requisition* requests, Glyph::Requisition& result
    );
    virtual void allocate(
	long n, Glyph::Requisition* requests, RegionRef given,
	LayoutManager::Allocations result
    );
    static void compute_reversed_allocations(
	Axis a, Glyph::Requisition& total, Boolean first_aligned,
	long n, Glyph::Requisition* requests, RegionRef given,
	LayoutManager::Allocations result
    );
private:
    Axis axis_;
    Glyph::Requisition requisition_;
};

/*
 * LayoutTileFirstAligned -- like Tile but use first element's origin
 */

class LayoutTileFirstAligned : public LayoutManager {
public:
    LayoutTileFirstAligned(Axis);
    virtual ~LayoutTileFirstAligned();

    virtual void request(
	long n, Glyph::Requisition* requests, Glyph::Requisition& result
    );
    virtual void allocate(
	long n, Glyph::Requisition* requests, RegionRef given,
	LayoutManager::Allocations result
    );
    static void compute_request_first_aligned(
	Axis a,
	long n, Glyph::Requisition* requests, Glyph::Requisition& result
    );
private:
    Axis axis_;
    Glyph::Requisition requisition_;
};

/*
 * LayoutTileReversedFirstAligned -- like TileReversed
 *	but use first element's origin
 */

class LayoutTileReversedFirstAligned : public LayoutManager {
public:
    LayoutTileReversedFirstAligned(Axis);
    virtual ~LayoutTileReversedFirstAligned();

    virtual void request(
	long n, Glyph::Requisition* requests, Glyph::Requisition& result
    );
    virtual void allocate(
	long n, Glyph::Requisition* requests, RegionRef given,
	LayoutManager::Allocations result
    );
private:
    Axis axis_;
    Glyph::Requisition requisition_;
};

/*
 * Placement -- glyph that places its body
 */

class Placement : public MonoGlyph {
public:
    Placement(GlyphRef, LayoutManager*);
    virtual ~Placement();

    void request(Glyph::Requisition& r); //+ Glyph::request
    void extension(const Glyph::AllocationInfo& a, RegionRef r); //+ Glyph::extension
    void allocations(Glyph::AllocationInfoList& a); //+ Glyph::allocations
    void traverse(GlyphTraversalRef t); //+ Glyph::traverse
private:
    LayoutManager* layout_;
    RegionImpl* result_;
};

/*
 * LayoutLayer -- place glyph between two other glyphs
 */

class LayoutLayer : public MonoGlyph {
public:
    LayoutLayer(GlyphRef between, GlyphRef under, GlyphRef over);
    virtual ~LayoutLayer();

    void traverse(GlyphTraversalRef t); //+ Glyph::traverse
private:
    GlyphRef under_;
    GlyphRef over_;
};

/*
 * Box -- PolyGlyph with layout manager
 */

class Box : public PolyGlyph {
public:
    Box(LayoutManager*);
    virtual ~Box();

    void request(Glyph::Requisition& r); //+ Glyph::request
    void extension(const Glyph::AllocationInfo& a, RegionRef r); //+ Glyph::extension
    void traverse(GlyphTraversalRef t); //+ Glyph::traverse
    void need_resize(); //+ Glyph::need_resize

    void child_allocation(long index, Glyph::AllocationInfo& a);
    void modified();
private:
    static const int static_size_;
    LayoutManager* layout_;
    Boolean requested_;
    Glyph::Requisition requisition_;
};

const int Box::static_size_ = 10;

/*
 * Glue - filler glyph
 */

class Glue : public GlyphImpl {
public:
    Glue(Axis, Coord natural, Coord stretch, Coord shrink, Alignment a);
    Glue(const Glyph::Requisition&);
    virtual ~Glue();

    void request(Glyph::Requisition& r); //+ Glyph::request
private:
    Glyph::Requisition requisition_;
};

/*
 * ShapeOf -- request the same size(s) as another glyph
 */

class ShapeOf : public GlyphImpl {
public:
    ShapeOf(GlyphRef x, GlyphRef y, GlyphRef z);
    virtual ~ShapeOf();

    void request(Glyph::Requisition& r); //+ Glyph::request
private:
    GlyphRef x_;
    GlyphRef y_;
    GlyphRef z_;
};

/*
 * Space -- draw space with font metrics
 */

class Space : public GlyphImpl {
public:
    Space(long count, Coord each, FontRef, ColorRef);
    virtual ~Space();

    void request(Glyph::Requisition& r); //+ Glyph::request
    void draw(GlyphTraversalRef t); //+ Glyph::draw
    void pick(GlyphTraversalRef t); //+ Glyph::pick
private:
    long count_;
    Coord each_;
    FontRef font_;
    ColorRef color_;
    Coord width_;
    Coord height_;
    float alignment_;
};

/*
 * Strut -- prop open space aligned by a font
 */

class Strut : public GlyphImpl {
public:
    Strut(FontRef, Coord natural = 0, Coord stretch = 0, Coord shrink = 0);
    virtual ~Strut();

    void request(Glyph::Requisition& r); //+ Glyph::request
private:
    FontRef font_;
    Coord natural_;
    Coord stretch_;
    Coord shrink_;
    Coord height_;
    float alignment_;
};

/*
 * HStrut -- prop horizontal space
 */

class HStrut : public GlyphImpl {
public:
    HStrut(
        Coord right_bearing, Coord left_bearing = 0,
        Coord natural = 0, Coord stretch = 0, Coord shrink = 0
    );
    virtual ~HStrut();

    void request(Glyph::Requisition& r); //+ Glyph::request
private:
    Coord left_bearing_;
    Coord right_bearing_;
    Coord natural_;
    Coord stretch_;
    Coord shrink_;
};

/*
 * VStrut -- prop vertical space
 */

class VStrut : public GlyphImpl {
public:
    VStrut(
        Coord ascent, Coord descent = 0,
        Coord natural = 0, Coord stretch = 0, Coord shrink = 0
    );
    virtual ~VStrut();

    void request(Glyph::Requisition& r); //+ Glyph::request
private:
    Coord ascent_;
    Coord descent_;
    Coord natural_;
    Coord stretch_;
    Coord shrink_;
};

/* LayoutKitImpl operations */

LayoutKitImpl::LayoutKitImpl() {
    fil_ = infinite_coord;
}

LayoutKitImpl::~LayoutKitImpl() { }

//+ LayoutKitImpl(FrescoObject::=object_.)
Long LayoutKitImpl::ref__(Long references) {
    return object_.ref__(references);
}
Tag LayoutKitImpl::attach(FrescoObjectRef observer) {
    return object_.attach(observer);
}
void LayoutKitImpl::detach(Tag attach_tag) {
    object_.detach(attach_tag);
}
void LayoutKitImpl::disconnect() {
    object_.disconnect();
}
void LayoutKitImpl::notify_observers() {
    object_.notify_observers();
}
void LayoutKitImpl::update() {
    object_.update();
}
//+

//+ LayoutKitImpl(LayoutKit::fil=c)
void LayoutKitImpl::fil(Coord c) { fil_ = c; }

//+ LayoutKitImpl(LayoutKit::fil?)
Coord LayoutKitImpl::fil() { return fil_; }

//+ LayoutKitImpl(LayoutKit::hbox)
GlyphRef LayoutKitImpl::_c_hbox() {
    return new Box(
	new LayoutSuperpose(new LayoutTile(X_axis), new LayoutAlign(Y_axis))
    );
}

//+ LayoutKitImpl(LayoutKit::vbox)
GlyphRef LayoutKitImpl::_c_vbox() {
    return new Box(
	new LayoutSuperpose(
	    new LayoutTileReversed(Y_axis), new LayoutAlign(X_axis)
	)
    );
}

//+ LayoutKitImpl(LayoutKit::hbox_first_aligned)
GlyphRef LayoutKitImpl::_c_hbox_first_aligned() {
    return new Box(
	new LayoutSuperpose(
	    new LayoutTileFirstAligned(X_axis), new LayoutAlign(Y_axis)
	)
    );
}

//+ LayoutKitImpl(LayoutKit::vbox_first_aligned
GlyphRef LayoutKitImpl::_c_vbox_first_aligned() {
    return new Box(
	new LayoutSuperpose(
	    new LayoutTileReversedFirstAligned(Y_axis),
	    new LayoutAlign(X_axis)
	)
    );
}

//+ LayoutKitImpl(LayoutKit::vscrollbox)
ScrollBoxRef LayoutKitImpl::_c_vscrollbox() {
    /* NOT IMPLEMENTED */
    return nil;
}

//+ LayoutKitImpl(LayoutKit::overlay)
GlyphRef LayoutKitImpl::_c_overlay() {
    return new Box(
	new LayoutSuperpose(
	    new LayoutAlign(X_axis), new LayoutAlign(Y_axis)
	)
    );
}

//+ LayoutKitImpl(LayoutKit::deck)
DeckObjRef LayoutKitImpl::_c_deck() {
    return new DeckImpl;
}

//+ LayoutKitImpl(LayoutKit::back)
GlyphRef LayoutKitImpl::_c_back(GlyphRef g, GlyphRef under) {
    return new LayoutLayer(g, under, nil);
}

//+ LayoutKitImpl(LayoutKit::front)
GlyphRef LayoutKitImpl::_c_front(GlyphRef g, GlyphRef over) {
    return new LayoutLayer(g, nil, over);
}

//+ LayoutKitImpl(LayoutKit::between)
GlyphRef LayoutKitImpl::_c_between(GlyphRef g, GlyphRef under, GlyphRef over) {
    return new LayoutLayer(g, under, over);
}

//+ LayoutKitImpl(LayoutKit::glue)
GlyphRef LayoutKitImpl::_c_glue(Axis a, Coord natural, Coord stretch, Coord shrink, Alignment align) {
    return new Glue(a, natural, stretch, shrink, align);
}

//+ LayoutKitImpl(LayoutKit::glue_requisition)
GlyphRef LayoutKitImpl::_c_glue_requisition(const Glyph::Requisition& r) {
    return new Glue(r);
}

//+ LayoutKitImpl(LayoutKit::hfil)
GlyphRef LayoutKitImpl::_c_hfil() {
    return new Glue(X_axis, 0, fil_, 0, 0.0);
}

//+ LayoutKitImpl(LayoutKit::hglue_fil)
GlyphRef LayoutKitImpl::_c_hglue_fil(Coord natural) {
    return new Glue(X_axis, natural, fil_, 0, 0.0);
}

//+ LayoutKitImpl(LayoutKit::hglue)
GlyphRef LayoutKitImpl::_c_hglue(Coord natural, Coord stretch, Coord shrink) {
    return new Glue(X_axis, natural, stretch, shrink, 0.0);
}

//+ LayoutKitImpl(LayoutKit::hglue_aligned)
GlyphRef LayoutKitImpl::_c_hglue_aligned(Coord natural, Coord stretch, Coord shrink, Alignment a) {
    return new Glue(X_axis, natural, stretch, shrink, a);
}

//+ LayoutKitImpl(LayoutKit::hspace)
GlyphRef LayoutKitImpl::_c_hspace(Coord natural) {
    return new Glue(X_axis, natural, 0, 0, 0.0);
}

//+ LayoutKitImpl(LayoutKit::vfil)
GlyphRef LayoutKitImpl::_c_vfil() {
    return new Glue(Y_axis, 0, fil_, 0, 0.0);
}

//+ LayoutKitImpl(LayoutKit::vglue_fil)
GlyphRef LayoutKitImpl::_c_vglue_fil(Coord natural) {
    return new Glue(Y_axis, natural, fil_, 0, 0.0);
}

//+ LayoutKitImpl(LayoutKit::vglue)
GlyphRef LayoutKitImpl::_c_vglue(Coord natural, Coord stretch, Coord shrink) {
    return new Glue(Y_axis, natural, stretch, shrink, 0.0);
}

//+ LayoutKitImpl(LayoutKit::vglue_aligned)
GlyphRef LayoutKitImpl::_c_vglue_aligned(Coord natural, Coord stretch, Coord shrink, Alignment a) {
    return new Glue(Y_axis, natural, stretch, shrink, a);
}

//+ LayoutKitImpl(LayoutKit::vspace)
GlyphRef LayoutKitImpl::_c_vspace(Coord natural) {
    return new Glue(Y_axis, natural, 0, 0, 0.0);
}

//+ LayoutKitImpl(LayoutKit::shape_of)
GlyphRef LayoutKitImpl::_c_shape_of(GlyphRef g) {
    return new ShapeOf(g, nil, nil);
}

//+ LayoutKitImpl(LayoutKit::shape_of_xy)
GlyphRef LayoutKitImpl::_c_shape_of_xy(GlyphRef gx, GlyphRef gy) {
    return new ShapeOf(gx, gy, nil);
}

//+ LayoutKitImpl(LayoutKit::shape_of_xyz)
GlyphRef LayoutKitImpl::_c_shape_of_xyz(GlyphRef gx, GlyphRef gy, GlyphRef gz) {
    return new ShapeOf(gx, gy, gz);
}

//+ LayoutKitImpl(LayoutKit::strut)
GlyphRef LayoutKitImpl::_c_strut(FontRef f, Coord natural, Coord stretch, Coord shrink) {
    return new Strut(f, natural, stretch, shrink);
}

//+ LayoutKitImpl(LayoutKit::hstrut)
GlyphRef LayoutKitImpl::_c_hstrut(Coord right_bearing, Coord left_bearing, Coord natural, Coord stretch, Coord shrink) {
    return new HStrut(right_bearing, left_bearing, natural, stretch, shrink);
}

//+ LayoutKitImpl(LayoutKit::vstrut)
GlyphRef LayoutKitImpl::_c_vstrut(Coord ascent, Coord descent, Coord natural, Coord stretch, Coord shrink) {
    return new VStrut(ascent, descent, natural, stretch, shrink);
}

//+ LayoutKitImpl(LayoutKit::spaces)
GlyphRef LayoutKitImpl::_c_spaces(Long count, Coord each, FontRef f, ColorRef c) {
    return new Space(count, each, f, c);
}

//+ LayoutKitImpl(LayoutKit::center)
GlyphRef LayoutKitImpl::_c_center(GlyphRef g) {
    return _c_center_aligned(g, 0.5, 0.5);
}

//+ LayoutKitImpl(LayoutKit::center_aligned)
GlyphRef LayoutKitImpl::_c_center_aligned(GlyphRef g, Alignment x, Alignment y) {
    return new Placement(
	g,
	new LayoutSuperpose(
	    new LayoutCenter(X_axis, x), new LayoutCenter(Y_axis, y)
	)
    );
}

//+ LayoutKitImpl(LayoutKit::center_axis)
GlyphRef LayoutKitImpl::_c_center_axis(GlyphRef g, Axis a, Alignment align) {
    return new Placement(g, new LayoutCenter(a, align));
}

//+ LayoutKitImpl(LayoutKit::hcenter)
GlyphRef LayoutKitImpl::_c_hcenter(GlyphRef g) {
    return _c_center_axis(g, X_axis, 0.5);
}

//+ LayoutKitImpl(LayoutKit::hcenter_aligned)
GlyphRef LayoutKitImpl::_c_hcenter_aligned(GlyphRef g, Alignment x) {
    return _c_center_axis(g, X_axis, x);
}

//+ LayoutKitImpl(LayoutKit::vcenter)
GlyphRef LayoutKitImpl::_c_vcenter(GlyphRef g) {
    return _c_center_axis(g, Y_axis, 0.5);
}

//+ LayoutKitImpl(LayoutKit::vcenter_aligned)
GlyphRef LayoutKitImpl::_c_vcenter_aligned(GlyphRef g, Alignment y) {
    return _c_center_axis(g, Y_axis, y);
}

//+ LayoutKitImpl(LayoutKit::fixed)
GlyphRef LayoutKitImpl::_c_fixed(GlyphRef g, Coord x, Coord y) {
    return new Placement(
	g,
	new LayoutSuperpose(
	    new LayoutFixed(X_axis, x), new LayoutFixed(Y_axis, y)
	)
    );
}

//+ LayoutKitImpl(LayoutKit::fixed_axis)
GlyphRef LayoutKitImpl::_c_fixed_axis(GlyphRef g, Axis a, Coord size) {
    return new Placement(g, new LayoutFixed(a, size));
}

//+ LayoutKitImpl(LayoutKit::hfixed)
GlyphRef LayoutKitImpl::_c_hfixed(GlyphRef g, Coord x) {
    return _c_fixed_axis(g, X_axis, x);
}

//+ LayoutKitImpl(LayoutKit::vfixed)
GlyphRef LayoutKitImpl::_c_vfixed(GlyphRef g, Coord y) {
    return _c_fixed_axis(g, Y_axis, y);
}

//+ LayoutKitImpl(LayoutKit::flexible)
GlyphRef LayoutKitImpl::_c_flexible(GlyphRef g, Coord stretch, Coord shrink) {
    return new Placement(
	g,
	new LayoutSuperpose(
	    new LayoutVariable(X_axis, stretch, shrink),
	    new LayoutVariable(Y_axis, stretch, shrink)
	)
    );
}

//+ LayoutKitImpl(LayoutKit::flexible_fil)
GlyphRef LayoutKitImpl::_c_flexible_fil(GlyphRef g) {
    return _c_flexible(g, fil_, fil_);
}

//+ LayoutKitImpl(LayoutKit::flexible_axis)
GlyphRef LayoutKitImpl::_c_flexible_axis(GlyphRef g, Axis a, Coord stretch, Coord shrink) {
    return new Placement(g, new LayoutVariable(a, stretch, shrink));
}

//+ LayoutKitImpl(LayoutKit::hflexible)
GlyphRef LayoutKitImpl::_c_hflexible(GlyphRef g, Coord stretch, Coord shrink) {
    return _c_flexible_axis(g, X_axis, stretch, shrink);
}

//+ LayoutKitImpl(LayoutKit::vflexible)
GlyphRef LayoutKitImpl::_c_vflexible(GlyphRef g, Coord stretch, Coord shrink) {
    return _c_flexible_axis(g, Y_axis, stretch, shrink);
}

//+ LayoutKitImpl(LayoutKit::natural)
GlyphRef LayoutKitImpl::_c_natural(GlyphRef g, Coord x, Coord y) {
    return new Placement(
	g,
	new LayoutSuperpose(
	    new LayoutNatural(X_axis, x),
	    new LayoutNatural(Y_axis, y)
	)
    );
}

//+ LayoutKitImpl(LayoutKit::natural_axis)
GlyphRef LayoutKitImpl::_c_natural_axis(GlyphRef g, Axis a, Coord size) {
    return new Placement(g, new LayoutNatural(a, size));
}

//+ LayoutKitImpl(LayoutKit::hnatural)
GlyphRef LayoutKitImpl::_c_hnatural(GlyphRef g, Coord x) {
    return _c_natural_axis(g, X_axis, x);
}

//+ LayoutKitImpl(LayoutKit::vnatural)
GlyphRef LayoutKitImpl::_c_vnatural(GlyphRef g, Coord y) {
    return _c_natural_axis(g, Y_axis, y);
}

//+ LayoutKitImpl(LayoutKit::margin)
GlyphRef LayoutKitImpl::_c_margin(GlyphRef g, Coord all) {
    return new Placement(g, new LayoutMargin(all));
}

//+ LayoutKitImpl(LayoutKit::margin_lrbt)
GlyphRef LayoutKitImpl::_c_margin_lrbt(GlyphRef g, Coord lmargin, Coord rmargin, Coord bmargin, Coord tmargin) {
    return new Placement(
	g, new LayoutMargin(lmargin, rmargin, bmargin, tmargin)
    );
}

//+ LayoutKitImpl(LayoutKit::margin_lrbt_flexible)
GlyphRef LayoutKitImpl::_c_margin_lrbt_flexible(GlyphRef g, Coord lmargin, Coord lstretch, Coord lshrink, Coord rmargin, Coord rstretch, Coord rshrink, Coord bmargin, Coord bstretch, Coord bshrink, Coord tmargin, Coord tstretch, Coord tshrink) {
    return new Placement(
	g,
	new LayoutMargin(
	    lmargin, lstretch, lshrink, rmargin, rstretch, rshrink,
	    bmargin, bstretch, bshrink, tmargin, tstretch, tshrink
	)
    );
}

//+ LayoutKitImpl(LayoutKit::hmargin)
GlyphRef LayoutKitImpl::_c_hmargin(GlyphRef g, Coord both) {
    return _c_margin_lrbt(g, both, both, 0, 0);
}

//+ LayoutKitImpl(LayoutKit::hmargin_lr)
GlyphRef LayoutKitImpl::_c_hmargin_lr(GlyphRef g, Coord lmargin, Coord rmargin) {
    return _c_margin_lrbt(g, lmargin, rmargin, 0, 0);
}

//+ LayoutKitImpl(LayoutKit::hmargin_lr_flexible)
GlyphRef LayoutKitImpl::_c_hmargin_lr_flexible(GlyphRef g, Coord lmargin, Coord lstretch, Coord lshrink, Coord rmargin, Coord rstretch, Coord rshrink) {
    return _c_margin_lrbt_flexible(
	g,
	lmargin, lstretch, lshrink, rmargin, rstretch, rshrink,
	0, 0, 0, 0, 0, 0
    );
}

//+ LayoutKitImpl(LayoutKit::vmargin)
GlyphRef LayoutKitImpl::_c_vmargin(GlyphRef g, Coord both) {
    return _c_margin_lrbt(g, 0, 0, both, both);
}

//+ LayoutKitImpl(LayoutKit::vmargin_bt)
GlyphRef LayoutKitImpl::_c_vmargin_bt(GlyphRef g, Coord bmargin, Coord tmargin) {
    return _c_margin_lrbt(g, 0, 0, bmargin, tmargin);
}

//+ LayoutKitImpl(LayoutKit::vmargin_bt_flexible)
GlyphRef LayoutKitImpl::_c_vmargin_bt_flexible(GlyphRef g, Coord bmargin, Coord bstretch, Coord bshrink, Coord tmargin, Coord tstretch, Coord tshrink) {
    return _c_margin_lrbt_flexible(
	g,
	0, 0, 0, 0, 0, 0,
	bmargin, bstretch, bshrink, tmargin, tstretch, tshrink
    );
}

//+ LayoutKitImpl(LayoutKit::lmargin)
GlyphRef LayoutKitImpl::_c_lmargin(GlyphRef g, Coord natural) {
    return _c_margin_lrbt(g, natural, 0, 0, 0);
}

//+ LayoutKitImpl(LayoutKit::lmargin_flexible)
GlyphRef LayoutKitImpl::_c_lmargin_flexible(GlyphRef g, Coord natural, Coord stretch, Coord shrink) {
    return _c_margin_lrbt_flexible(
	g, natural, stretch, shrink, 0, 0, 0, 0, 0, 0, 0, 0, 0
    );
}

//+ LayoutKitImpl(LayoutKit::rmargin)
GlyphRef LayoutKitImpl::_c_rmargin(GlyphRef g, Coord natural) {
    return _c_margin_lrbt(g, 0, natural, 0, 0);
}

//+ LayoutKitImpl(LayoutKit::rmargin_flexible)
GlyphRef LayoutKitImpl::_c_rmargin_flexible(GlyphRef g, Coord natural, Coord stretch, Coord shrink) {
    return _c_margin_lrbt_flexible(
	g, 0, 0, 0, natural, stretch, shrink, 0, 0, 0, 0, 0, 0
    );
}

//+ LayoutKitImpl(LayoutKit::bmargin)
GlyphRef LayoutKitImpl::_c_bmargin(GlyphRef g, Coord natural) {
    return _c_margin_lrbt(g, 0, 0, natural, 0);
}

//+ LayoutKitImpl(LayoutKit::bmargin_flexible)
GlyphRef LayoutKitImpl::_c_bmargin_flexible(GlyphRef g, Coord natural, Coord stretch, Coord shrink) {
    return _c_margin_lrbt_flexible(
	g, 0, 0, 0, 0, 0, 0, natural, stretch, shrink, 0, 0, 0
    );
}

//+ LayoutKitImpl(LayoutKit::tmargin)
GlyphRef LayoutKitImpl::_c_tmargin(GlyphRef g, Coord natural) {
    return _c_margin_lrbt(g, 0, 0, 0, natural);
}

//+ LayoutKitImpl(LayoutKit::tmargin_flexible)
GlyphRef LayoutKitImpl::_c_tmargin_flexible(GlyphRef g, Coord natural, Coord stretch, Coord shrink) {
    return _c_margin_lrbt_flexible(
	g, 0, 0, 0, 0, 0, 0, 0, 0, 0, natural, stretch, shrink
    );
}

/* class LayoutAlign */

LayoutAlign::LayoutAlign(Axis a) {
    axis_ = a;
}

LayoutAlign::~LayoutAlign() { }

void LayoutAlign::request(
    long n, Glyph::Requisition* requests, Glyph::Requisition& result
) {
    Glyph::Requirement* r;
    Coord natural_lead = 0;
    Coord min_lead = -infinite_coord;
    Coord max_lead = infinite_coord;
    Coord natural_trail = 0;
    Coord min_trail = -infinite_coord;
    Coord max_trail = infinite_coord;
    for (long i = 0; i < n; i++) {
	r = GlyphImpl::requirement(requests[i], axis_);
	if (r->defined) {
	    Coord r_nat = r->natural;
	    Coord r_max = r->maximum;
	    Coord r_min = r->minimum;
	    Coord r_align = r->align;
	    Coord r_inv_align = 1.0 - r_align;
            natural_lead = Math::max(natural_lead, Coord(r_nat * r_align));
            max_lead = Math::min(max_lead, Coord(r_max * r_align));
            min_lead = Math::max(min_lead, Coord(r_min * r_align));
            natural_trail = Math::max(
                natural_trail, Coord(r_nat * r_inv_align)
            );
            max_trail = Math::min(max_trail, Coord(r_max * r_inv_align));
            min_trail = Math::max(min_trail, Coord(r_min * r_inv_align));
        }
    }
    r = GlyphImpl::requirement(result, axis_);
    GlyphImpl::require_lead_trail(
	*r, natural_lead, max_lead, min_lead,
	natural_trail, max_trail, min_trail
    );
}

void LayoutAlign::allocate(
    long n, Glyph::Requisition* requests, RegionRef given,
    LayoutManager::Allocations result
) {
    Glyph::Requirement* r;
    Region::BoundingSpan s;
    given->span(axis_, s);
    for (long i = 0; i < n; i++) {
        r = GlyphImpl::requirement(requests[i], axis_);
	if (r->defined) {
	    Coord length = Math::max(
		Math::min(LayoutTile::compute_length(*r, s), r->maximum),
		r->minimum
	    );
	    set_span(result[i], axis_, s.origin, length, r->align);
	} else {
	    set_span(result[i], axis_, s.origin, s.length, s.align);
	}
    }
}

/* class LayoutCenter */

LayoutCenter::LayoutCenter(Axis a, Alignment align) {
    axis_ = a;
    alignment_ = align;
}

LayoutCenter::~LayoutCenter() { }

void LayoutCenter::request(
    long, Glyph::Requisition*, Glyph::Requisition& result
) {
    Glyph::Requirement* r = GlyphImpl::requirement(result, axis_);
    r->defined = true;
    r->align = alignment_;
}

void LayoutCenter::allocate(
    long, Glyph::Requisition* requests, RegionRef,
    LayoutManager::Allocations result
) {
    Region::BoundingSpan s;
    result[0]->span(axis_, s);
    Alignment a = GlyphImpl::requirement(requests[0], axis_)->align;
    set_span(
	result[0], axis_, s.origin + (a - s.align) * s.length, s.length, a
    );
}

/* class LayoutFixed */

LayoutFixed::LayoutFixed(Axis a, Coord size) {
    axis_ = a;
    size_ = size;
}

LayoutFixed::~LayoutFixed() { }

void LayoutFixed::request(
    long, Glyph::Requisition*, Glyph::Requisition& result
) {
    Glyph::Requirement* r = GlyphImpl::requirement(result, axis_);
    r->defined = true;
    r->natural = size_;
    r->maximum = size_;
    r->minimum = size_;
}

void LayoutFixed::allocate(
    long, Glyph::Requisition* , RegionRef, LayoutManager::Allocations result
) {
    Region::BoundingSpan s;
    result[0]->span(axis_, s);
    set_span(result[0], axis_, s.origin, size_, s.align);
}

/* class LayoutMargin */

LayoutMargin::LayoutMargin(Coord margin) {
    lnatural_ = margin; lstretch_ = 0; lshrink_ = 0;
    rnatural_ = margin; rstretch_ = 0; rshrink_ = 0;
    bnatural_ = margin; bstretch_ = 0; bshrink_ = 0;
    tnatural_ = margin; tstretch_ = 0; tshrink_ = 0;
}

LayoutMargin::LayoutMargin(Coord hmargin, Coord vmargin) {
    lnatural_ = hmargin; lstretch_ = 0; lshrink_ = 0;
    rnatural_ = hmargin; rstretch_ = 0; rshrink_ = 0;
    bnatural_ = vmargin; bstretch_ = 0; bshrink_ = 0;
    tnatural_ = vmargin; tstretch_ = 0; tshrink_ = 0;
}

LayoutMargin::LayoutMargin(
    Coord lmargin, Coord rmargin, Coord bmargin, Coord tmargin
) {
    lnatural_ = lmargin; lstretch_ = 0; lshrink_ = 0;
    rnatural_ = rmargin; rstretch_ = 0; rshrink_ = 0;
    bnatural_ = bmargin; bstretch_ = 0; bshrink_ = 0;
    tnatural_ = tmargin; tstretch_ = 0; tshrink_ = 0;
}

LayoutMargin::LayoutMargin(
    Coord lmargin, Coord lstretch, Coord lshrink,
    Coord rmargin, Coord rstretch, Coord rshrink,
    Coord bmargin, Coord bstretch, Coord bshrink,
    Coord tmargin, Coord tstretch, Coord tshrink
) {
    lnatural_ = lmargin; lstretch_ = lstretch; lshrink_ = lshrink;
    rnatural_ = rmargin; rstretch_ = rstretch; rshrink_ = rshrink;
    bnatural_ = bmargin; bstretch_ = bstretch; bshrink_ = bshrink;
    tnatural_ = tmargin; tstretch_ = tstretch; tshrink_ = tshrink;
}

LayoutMargin::~LayoutMargin() { }

void LayoutMargin::request(
    long, Glyph::Requisition*, Glyph::Requisition& result
) {
    Glyph::Requirement& rx = result.x;
    if (rx.defined) {
	Coord dx = lnatural_ + rnatural_;
	rx.natural += dx;
	rx.maximum += dx + (lstretch_ + rstretch_);
	rx.minimum += dx - (lshrink_ + rshrink_);
    }
    Glyph::Requirement& ry = result.y;
    if (ry.defined) {
	Coord dy = bnatural_ + tnatural_;
	ry.natural += dy;
	ry.maximum += dy + (bstretch_ + tstretch_);
	ry.minimum += dy - (bshrink_ + tshrink_);
    }
    requisition_ = result;
}

void LayoutMargin::allocate(
    long, Glyph::Requisition*, RegionRef, LayoutManager::Allocations result
) {
    allocate_axis(
	X_axis,
	lnatural_, lstretch_, lshrink_, rnatural_, rstretch_, rshrink_,
	result
    );
    allocate_axis(
	Y_axis,
	bnatural_, bstretch_, bshrink_, tnatural_, tstretch_, tshrink_,
	result
    );
}

void LayoutMargin::allocate_axis(
    Axis a,
    Coord natural_lead, Coord stretch_lead, Coord shrink_lead,
    Coord natural_trail, Coord stretch_trail, Coord shrink_trail,
    LayoutManager::Allocations result
) {
    Region::BoundingSpan s;
    result[0]->span(a, s);
    Glyph::Requirement* r = GlyphImpl::requirement(requisition_, a);
    Coord lead = span(s.length, *r, natural_lead, stretch_lead, shrink_lead);
    Coord trail = span(
	s.length, *r, natural_trail, stretch_trail, shrink_trail
    );
    s.length -= (lead + trail);
    s.origin += ((1 - r->align) * lead - r->align * trail);
    set_span(result[0], a, s.origin, s.length, s.align);
}

Coord LayoutMargin::span(
    Coord span, Glyph::Requirement& total,
    Coord natural, Coord stretch, Coord shrink
) {
    Coord extra = span - total.natural;
    Coord result = natural;
    float ss = 0.0;
    Coord total_stretch = total.maximum - total.natural;
    Coord total_shrink = total.natural - total.minimum;
    if (extra > 0 && total_stretch > 0) {
        ss = stretch / total_stretch;
    } else if (extra < 0 && total_shrink > 0) {
        ss = shrink / total_shrink;
    }
    return result + ss * extra;
}

/* class LayoutNatural */

LayoutNatural::LayoutNatural(Axis a, Coord natural) {
    axis_ = a;
    natural_ = natural;
}

LayoutNatural::~LayoutNatural() { }

void LayoutNatural::request(
    long, Glyph::Requisition*, Glyph::Requisition& result
) {
    Glyph::Requirement* r = GlyphImpl::requirement(result, axis_);
    r->defined = true;
    r->natural = natural_;
}

void LayoutNatural::allocate(
    long, Glyph::Requisition*, RegionRef, LayoutManager::Allocations
) {
    /* leave it as is */
}

/* class LayoutSuperpose */

LayoutSuperpose::LayoutSuperpose(
    LayoutManager* first, LayoutManager* second
) {
    first_ = first;
    second_ = second;
    third_ = nil;
}

LayoutSuperpose::LayoutSuperpose(
    LayoutManager* first, LayoutManager* second, LayoutManager* third
) {
    first_ = first;
    second_ = second;
    third_ = third;
}

LayoutSuperpose::~LayoutSuperpose() {
    Fresco::unref(first_);
    Fresco::unref(second_);
    Fresco::unref(third_);
}

void LayoutSuperpose::request(
    long n, Glyph::Requisition* requests, Glyph::Requisition& result
) {
    if (is_not_nil(first_)) {
	first_->request(n, requests, result);
    }
    if (is_not_nil(second_)) {
	second_->request(n, requests, result);
    }
    if (is_not_nil(third_)) {
	third_->request(n, requests, result);
    }
}

void LayoutSuperpose::allocate(
    long n, Glyph::Requisition* requests, RegionRef given,
    LayoutManager::Allocations result
) {
    if (is_not_nil(first_)) {
	first_->allocate(n, requests, given, result);
    }
    if (is_not_nil(second_)) {
	second_->allocate(n, requests, given, result);
    }
    if (is_not_nil(third_)) {
	third_->allocate(n, requests, given, result);
    }
}

/* class LayoutTile */

LayoutTile::LayoutTile(Axis a) { axis_ = a; }
LayoutTile::~LayoutTile() { }

void LayoutTile::request(
    long n, Glyph::Requisition* requests, Glyph::Requisition& result
) {
    compute_request(axis_, 0.0, n, requests, result);
    requisition_ = result;
}

void LayoutTile::allocate(
    long n, Glyph::Requisition* requests, RegionRef given,
    LayoutManager::Allocations result
) {
    compute_allocations(
	axis_, requisition_, false, n, requests, given, result
    );
}

void LayoutTile::compute_request(
    Axis a, Alignment align,
    long n, Glyph::Requisition* requests, Glyph::Requisition& result
) {
    Glyph::Requirement* r;
    Coord natural = 0, min_size = 0, max_size = 0;
    for (long i = 0; i < n; i++) {
        r = GlyphImpl::requirement(requests[i], a);
        if (r->defined) {
	    Coord n = r->natural;
	    natural += n;
	    max_size += r->maximum;
	    min_size += r->minimum;
	}
    }
    r = GlyphImpl::requirement(result, a);
    r->defined = true;
    r->natural = natural;
    r->maximum = max_size;
    r->minimum = min_size;
    r->align = align;
}

void LayoutTile::compute_allocations(
    Axis a, Glyph::Requisition& total, Boolean first_aligned,
    long n, Glyph::Requisition* requests, RegionRef given,
    LayoutManager::Allocations result
) {
    Glyph::Requirement* r;
    Region::BoundingSpan s;
    r = GlyphImpl::requirement(total, a);
    given->span(a, s);
    Coord length = compute_length(*r, s);
    Boolean growing = length > r->natural;
    Boolean shrinking = length < r->natural;
    float f = compute_squeeze(*r, length);
    Coord p = s.origin;
    for (long i = 0; i < n; i++) {
        r = GlyphImpl::requirement(requests[i], a);
        if (r->defined) {
            Coord cspan = r->natural;
            if (growing) {
                cspan += f * (r->maximum - r->natural);
            } else if (shrinking) {
                cspan -= f * (r->natural - r->minimum);
            }
            if (first_aligned && i == 0) {
                p += (1 - r->align) * cspan;
            }
	    set_span(result[i], a, p + cspan * r->align, cspan, r->align);
            p += cspan;
        } else {
	    set_span(result[i], a, p, 0, 0);
        }
    }
}

Coord LayoutTile::compute_length(
    const Glyph::Requirement& r, const Region::BoundingSpan& s
) {
    Coord length = s.length;
    Coord s_a = s.align;
    Coord r_a = r.align;
    if (r_a == 0) {
	length *= (1 - s_a);
    } else if (r_a == 1) {
	length *= s_a;
    } else {
	length *= Math::min(s_a / r_a, (1 - s_a) / (1 - r_a));
    }
    return length;
}

float LayoutTile::compute_squeeze(const Glyph::Requirement& r, Coord length) {
    float f;
    Coord nat = r.natural;
    if (length > nat && r.maximum > nat) {
        f = (length - nat) / (r.maximum - nat);
    } else if (length < nat && r.minimum < nat) {
        f = (nat - length) / (nat - r.minimum);
    } else {
        f = 0;
    }
    return f;
}

/* class LayoutTileReversed */

LayoutTileReversed::LayoutTileReversed(Axis a) { axis_ = a; }
LayoutTileReversed::~LayoutTileReversed() { }

void LayoutTileReversed::request(
    long n, Glyph::Requisition* requests, Glyph::Requisition& result
) {
    LayoutTile::compute_request(axis_, 1.0, n, requests, result);
    requisition_ = result;
}

void LayoutTileReversed::allocate(
    long n, Glyph::Requisition* requests, RegionRef given,
    LayoutManager::Allocations result
) {
    compute_reversed_allocations(
	axis_, requisition_, false, n, requests, given, result
    );
}

void LayoutTileReversed::compute_reversed_allocations(
    Axis a, Glyph::Requisition& total, Boolean first_aligned,
    long n, Glyph::Requisition* requests, RegionRef given,
    LayoutManager::Allocations result
) {
    Glyph::Requirement* r;
    Region::BoundingSpan s;
    r = GlyphImpl::requirement(total, a);
    given->span(a, s);
    Coord length = LayoutTile::compute_length(*r, s);
    Boolean growing = length > r->natural;
    Boolean shrinking = length < r->natural;
    float f = LayoutTile::compute_squeeze(*r, length);
    Coord p = s.origin;
    for (long i = 0; i < n; i++) {
        r = GlyphImpl::requirement(requests[i], a);
        if (r->defined) {
            Coord cspan = r->natural;
            if (growing) {
                cspan += f * (r->maximum - r->natural);
            } else if (shrinking) {
                cspan -= f * (r->natural - r->minimum);
            }
            if (first_aligned && i == 0) {
                p += (1 - r->align) * cspan;
            }
            p -= cspan;
	    set_span(result[i], a, p + r->align * cspan, cspan, r->align);
        } else {
	    set_span(result[i], a, p, 0, 0);
        }
    }
}

/* class LayoutTileFirstAligned */

LayoutTileFirstAligned::LayoutTileFirstAligned(Axis a) { axis_ = a; }
LayoutTileFirstAligned::~LayoutTileFirstAligned() { }

void LayoutTileFirstAligned::request(
    long n, Glyph::Requisition* requests, Glyph::Requisition& result
) {
    compute_request_first_aligned(axis_, n, requests, result);
    requisition_ = result;
}

void LayoutTileFirstAligned::allocate(
    long n, Glyph::Requisition* requests, RegionRef given,
    LayoutManager::Allocations result
) {
    LayoutTile::compute_allocations(
	axis_, requisition_, true, n, requests, given, result
    );
}

void LayoutTileFirstAligned::compute_request_first_aligned(
    Axis a,
    long n, Glyph::Requisition* requests, Glyph::Requisition& result
) {
    Glyph::Requirement* r;
    Coord natural_lead = 0;
    Coord min_lead = 0;
    Coord max_lead = 0;
    Coord natural_trail = 0;
    Coord min_trail = 0;
    Coord max_trail = 0;
    for (long i = 0; i < n; i++) {
        r = GlyphImpl::requirement(requests[i], a);
        if (r->defined) {
            if (i == 0) {
		Alignment a = r->align;
		Alignment aa = 1 - a;
                natural_lead = a * r->natural;
		max_lead = a * r->maximum;
                min_lead = a * r->minimum;
                natural_trail = aa * r->natural;
                max_trail = aa * r->maximum;
                min_trail = aa * r->minimum;
            } else {
                natural_trail += r->natural;
                max_trail += r->maximum;
                min_trail += r->minimum;
            }
        }
    }
    r = GlyphImpl::requirement(result, a);
    GlyphImpl::require_lead_trail(
	*r, natural_lead, max_lead, min_lead,
	natural_trail, max_trail, min_trail
    );
}

/* class LayoutTileReversedFirstAligned */

LayoutTileReversedFirstAligned::LayoutTileReversedFirstAligned(Axis a) {
    axis_ = a;
}

LayoutTileReversedFirstAligned::~LayoutTileReversedFirstAligned() { }

void LayoutTileReversedFirstAligned::request(
    long n, Glyph::Requisition* requests, Glyph::Requisition& result
) {
    LayoutTileFirstAligned::compute_request_first_aligned(
	axis_, n, requests, result
    );
    requisition_ = result;
}

void LayoutTileReversedFirstAligned::allocate(
    long n, Glyph::Requisition* requests, RegionRef given,
    LayoutManager::Allocations result
) {
    LayoutTileReversed::compute_reversed_allocations(
	axis_, requisition_, true, n, requests, given, result
    );
}

/* class LayoutVariable */

LayoutVariable::LayoutVariable(Axis a, Coord stretch, Coord shrink) {
    axis_ = a;
    stretch_ = stretch;
    shrink_ = shrink;
}

LayoutVariable::~LayoutVariable() { }

void LayoutVariable::request(
    long, Glyph::Requisition*, Glyph::Requisition& result
) {
    Glyph::Requirement* r = GlyphImpl::requirement(result, axis_);
    r->defined = true;
    r->maximum = r->natural + stretch_;
    r->minimum = r->natural - shrink_;
}

void LayoutVariable::allocate(
    long, Glyph::Requisition*, RegionRef, LayoutManager::Allocations
) {
    /* leave it as is */
}

/* class Placement */

Placement::Placement(GlyphRef g, LayoutManager* layout) {
    body(g);
    layout_ = layout;
    result_ = new RegionImpl;
}

Placement::~Placement() {
    Fresco::unref(layout_);
    Fresco::unref(result_);
}

//+ Placement(Glyph::request)
void Placement::request(Glyph::Requisition& r) {
    MonoGlyph::request(r);
    layout_->request(0, nil, r);
}

//+ Placement(Glyph::extension)
void Placement::extension(const Glyph::AllocationInfo&, RegionRef) { }

//+ Placement(Glyph::allocations)
void Placement::allocations(Glyph::AllocationInfoList& a) {
    MonoGlyph::allocations(a);
    for (Long i = 0; i < a._length; i++) {
	RegionRef given = a._buffer[i].allocation;
	result_->copy(given);
	Glyph::Requisition r;
	GlyphImpl::init_requisition(r);
	MonoGlyph::request(r);
	layout_->allocate(1, &r, given, &result_);
	given->copy(result_);
    }
}

//+ Placement(Glyph::traverse)
void Placement::traverse(GlyphTraversalRef t) {
    Region given = t->allocation();
    if (is_not_nil(given)) {
	result_->copy(given);
	Glyph::Requisition r;
	GlyphImpl::init_requisition(r);
	MonoGlyph::request(r);
	layout_->allocate(1, &r, given, &result_);
	t->traverse_child(&offset_, result_);
    } else {
	MonoGlyph::traverse(t);
    }
}

/* class LayoutLayer */

LayoutLayer::LayoutLayer(
    GlyphRef between, GlyphRef under, GlyphRef over
) {
    body(between);
    under_ = under;
    over_ = over;
}

LayoutLayer::~LayoutLayer() {
    Fresco::unref(under_);
    Fresco::unref(over_);
}

//+ LayoutLayer(Glyph::traverse)
void LayoutLayer::traverse(GlyphTraversalRef t) {
    if (is_not_nil(under_)) {
	under_->traverse(t);
    }
    GlyphImpl::traverse(t);
    if (is_not_nil(over_)) {
	over_->traverse(t);
    }
}

/* class Box */

Box::Box(LayoutManager* layout) {
    layout_ = layout;
    requested_ = false;
}

Box::~Box() {
    Fresco::unref(layout_);
}

//+ Box(Glyph::request)
void Box::request(Glyph::Requisition& r) {
    if (!requested_) {
	GlyphImpl::init_requisition(requisition_);
	long n = children_.count();
	if (n > 0) {
	    Glyph::Requisition req[static_size_];
	    Glyph::Requisition* r = children_requests(req, static_size_);
	    layout_->request(n, r, requisition_);
	    if (r != req) {
		delete [] r;
	    }
	}
	requested_ = true;
    }
    r = requisition_;
}

//+ Box(Glyph::extension)
void Box::extension(const Glyph::AllocationInfo&, RegionRef) { }

/*
 * Both traverse and child_allocation could be made more efficient
 * by avoiding memory allocation using a fixed size array of regions
 * when the number of children is below a reasonable amount.
 */

//+ Box(Glyph::traverse)
void Box::traverse(GlyphTraversalRef t) {
    Region given = t->allocation();
    if (is_nil(given)) {
	for (ListItr(PolyGlyphOffsetList) i(children_); i.more(); i.next()) {
	    t->traverse_child(i.cur(), nil);
	}
    } else {
	long index;
	long n = children_.count();
	Glyph::Requisition req[static_size_];
	Glyph::Requisition* r = children_requests(req, static_size_);
	if (!requested_) {
	    layout_->request(n, r, requisition_);
	    requested_ = true;
	}
	RegionImpl** result = new RegionImpl*[n];
	for (index = 0; index < n; index++) {
	    result[index] = new RegionImpl;
	}
	layout_->allocate(n, r, given, result);
	if (r != req) {
	    delete [] r;
	}
	index = 0;
	for (ListItr(PolyGlyphOffsetList) i(children_); i.more(); i.next()) {
	    t->traverse_child(i.cur(), result[index]);
	    ++index;
	}
	delete [] result;
    }
}

//+ Box(Glyph::need_resize)
void Box::need_resize() {
    modified();
    PolyGlyph::need_resize();
}

void Box::child_allocation(long index, Glyph::AllocationInfo& a) {
    long i;
    RegionRef given = a.allocation;
    long n = children_.count();
    Glyph::Requisition req[static_size_];
    Glyph::Requisition* r = children_requests(req, static_size_);
    RegionImpl** result = new RegionImpl*[n];
    for (i = 0; i < n; i++) {
	result[i] = new RegionImpl;
    }
    layout_->allocate(n, r, given, result);
    if (r != req) {
	delete [] r;
    }
    given->copy(result[index]);
    for (i = 0; i < n; i++) {
	Fresco::unref(result[i]);
    }
    delete [] result;
}

void Box::modified() {
    requested_ = false;
}

/* class DeckImpl */

DeckImpl::DeckImpl() { top_ = nil; requested_ = false; }
DeckImpl::~DeckImpl() { Fresco::unref(top_); }

//+ DeckImpl(FrescoObject::=PolyGlyph::)
Long DeckImpl::ref__(Long references) {
    return PolyGlyph::ref__(references);
}
Tag DeckImpl::attach(FrescoObjectRef observer) {
    return PolyGlyph::attach(observer);
}
void DeckImpl::detach(Tag attach_tag) {
    PolyGlyph::detach(attach_tag);
}
void DeckImpl::disconnect() {
    PolyGlyph::disconnect();
}
void DeckImpl::notify_observers() {
    PolyGlyph::notify_observers();
}
void DeckImpl::update() {
    PolyGlyph::update();
}
//+

//+ DeckImpl(Glyph::style=s)
void DeckImpl::_c_style(StyleObjRef s) {
    PolyGlyph::style(s);
}

//+ DeckImpl(Glyph::style?)
StyleObjRef DeckImpl::_c_style() {
    return PolyGlyph::_c_style();
}

//+ DeckImpl(Glyph::transform)
TransformObjRef DeckImpl::_c_transform() {
    return PolyGlyph::_c_transform();
}

//+ DeckImpl(Glyph::request)
void DeckImpl::request(Glyph::Requisition& r) {
    if (!requested_) {
	GlyphImpl::init_requisition(requisition_);
	long n = children_.count();
	if (n > 0) {
#if defined(__DECCXX)
/* workaround for DEC CXX bug */
	    DeckObj::Requisition req[10];
	    DeckObj::Requisition* r = children_requests(req, 10);
#else
	    Glyph::Requisition req[10];
	    Glyph::Requisition* r = children_requests(req, 10);
#endif
	    LayoutAlign x(X_axis);
	    x.request(n, r, requisition_);
	    LayoutAlign y(Y_axis);
	    y.request(n, r, requisition_);
	    if (r != req) {
		delete [] r;
	    }
	}
	requested_ = true;
    }
    r = requisition_;
}

//+ DeckImpl(Glyph::extension)
void DeckImpl::extension(const Glyph::AllocationInfo& a, RegionRef r) {
    PolyGlyph::extension(a, r);
}

//+ DeckImpl(Glyph::shape)
void DeckImpl::shape(RegionRef r) {
    PolyGlyph::shape(r);
}

//+ DeckImpl(Glyph::allocations)
void DeckImpl::allocations(Glyph::AllocationInfoList& a) {
    PolyGlyph::allocations(a);
}

//+ DeckImpl(Glyph::traverse)
void DeckImpl::traverse(GlyphTraversalRef t) {
    if (is_not_nil(top_)) {
	t->traverse_child(top_, nil);
    }
}

//+ DeckImpl(Glyph::draw)
void DeckImpl::draw(GlyphTraversalRef t) {
    PolyGlyph::draw(t);
}

//+ DeckImpl(Glyph::pick)
void DeckImpl::pick(GlyphTraversalRef t) {
    PolyGlyph::pick(t);
}

//+ DeckImpl(Glyph::body=)
void DeckImpl::_c_body(GlyphRef) { }

//+ DeckImpl(Glyph::body?)
GlyphRef DeckImpl::_c_body() { return nil; }

//+ DeckImpl(Glyph::append)
GlyphOffsetRef DeckImpl::_c_append(GlyphRef g) {
    return PolyGlyph::_c_append(g);
}

//+ DeckImpl(Glyph::prepend)
GlyphOffsetRef DeckImpl::_c_prepend(GlyphRef g) {
    return PolyGlyph::_c_prepend(g);
}

//+ DeckImpl(Glyph::add_parent)
Tag DeckImpl::add_parent(GlyphOffsetRef parent_offset) {
    return PolyGlyph::add_parent(parent_offset);
}

//+ DeckImpl(Glyph::remove_parent)
void DeckImpl::remove_parent(Tag add_tag) {
    PolyGlyph::remove_parent(add_tag);
}

//+ DeckImpl(Glyph::visit_children)
void DeckImpl::visit_children(GlyphVisitorRef v) {
    PolyGlyph::visit_children(v);
}

//+ DeckImpl(Glyph::visit_children_reversed)
void DeckImpl::visit_children_reversed(GlyphVisitorRef v) {
    PolyGlyph::visit_children_reversed(v);
}

//+ DeckImpl(Glyph::visit_parents)
void DeckImpl::visit_parents(GlyphVisitorRef v) {
    PolyGlyph::visit_parents(v);
}

//+ DeckImpl(Glyph::need_redraw)
void DeckImpl::need_redraw() {
    PolyGlyph::need_redraw();
}

//+ DeckImpl(Glyph::need_redraw_region)
void DeckImpl::need_redraw_region(RegionRef r) {
    PolyGlyph::need_redraw_region(r);
}

//+ DeckImpl(Glyph::need_resize)
void DeckImpl::need_resize() {
    PolyGlyph::need_resize();
}

//+ DeckImpl(Glyph::clone_glyph)
GlyphRef DeckImpl::_c_clone_glyph() {
    return nil;
}

/*
 * Return the current top of the deck.
 */

//+ DeckImpl(DeckObj::card)
GlyphOffsetRef DeckImpl::_c_card() {
    return GlyphOffset::_duplicate(top_);
}

/*
 * Change the current top of the deck.
 */

//+ DeckImpl(DeckObj::flip_to)
void DeckImpl::flip_to(GlyphOffsetRef off) {
    Fresco::unref(top_);
    top_ = GlyphOffset::_duplicate(off);
    need_redraw();
}

void DeckImpl::modified() {
    requested_ = false;
}

/* class Glue */

Glue::Glue(
    Axis a, Coord natural, Coord stretch, Coord shrink, Alignment align
) {
    GlyphImpl::init_requisition(requisition_);
    Glyph::Requirement* r = GlyphImpl::requirement(requisition_, a);
    if (r != nil) {
	GlyphImpl::require(*r, natural, stretch, shrink, align);
    }
}

Glue::Glue(const Glyph::Requisition& r) {
    requisition_ = r;
}

Glue::~Glue() { }

//+ Glue(Glyph::request)
void Glue::request(Glyph::Requisition& r) {
    r = requisition_;
}

/* class Space */

Space::Space(long count, float each, FontRef f, ColorRef c) {
    count_ = count;
    each_ = each;
    font_ = Font::_duplicate(f);
    color_ = Color::_duplicate(c);
    if (is_not_nil(font_)) {
	Font::Info i;
	font_->font_info(i);
	Coord ascent = i.ascent;
	Coord descent = i.descent;
	font_->char_info(' ', i);
	width_ = i.width * each_ * count_;
	height_ = ascent + descent;
	alignment_ = (height_ == 0) ? 0 : descent / height_;
    } else {
	width_ = 0;
	height_ = 0;
	alignment_ = 0;
    }
}

Space::~Space() {
    Fresco::unref(font_);
    Fresco::unref(color_);
}

//+ Space(Glyph::request)
void Space::request(Glyph::Requisition& r) {
    require(r.x, width_, width_ * 4, width_ / 3, 0);
    require(r.y, height_, 0, 0, alignment_);
}

//+ Space(Glyph::draw)
void Space::draw(GlyphTraversalRef t) {
    if (count_ > 0) {
	Vertex v0, v1, v;
	t->bounds(v0, v1, v);
	Coord each = (v1.x - v0.x) / count_;
	PainterObj p = t->painter();
	p->font_attr(font_);
	p->color_attr(color_);
        for (long i = 0; i < count_; ++i) {
            p->character(' ', each, v.x, v.y);
            v.x += each;
        }
    }
}

//+ Space(Glyph::pick)
void Space::pick(GlyphTraversalRef) { }

/* class Strut */

Strut::Strut(FontRef font, Coord natural, Coord stretch, Coord shrink) {
    font_ = Font::_duplicate(font);
    if (is_not_nil(font_)) {
	Font::Info b;
	font_->font_info(b);
	height_ = b.ascent + b.descent;
	alignment_ = (height_ == 0) ? 0 : b.descent / height_;
    }
    natural_ = natural;
    stretch_ = stretch;
    shrink_ = shrink;
}

Strut::~Strut() {
    Fresco::unref(font_);
}

//+ Strut(Glyph::request)
void Strut::request(Glyph::Requisition& r) {
    require(r.x, natural_, stretch_, shrink_, 0);
    require(r.y, height_, 0, 0, alignment_);
}

/* class HStrut */

HStrut::HStrut(
    Coord right_bearing, Coord left_bearing,
    Coord natural, Coord stretch, Coord shrink
) {
    left_bearing_ = left_bearing;
    right_bearing_ = right_bearing;
    natural_ = natural;
    stretch_ = stretch;
    shrink_ = shrink;
}

HStrut::~HStrut() { }

//+ HStrut(Glyph::request)
void HStrut::request(Glyph::Requisition& r) {
    Coord width = left_bearing_ + right_bearing_;
    require(r.x, width, 0, 0, (width == 0) ? 0 : left_bearing_ / width);
    require(r.y, natural_, stretch_, shrink_, 0);
}

/* class VStrut */

VStrut::VStrut(
    Coord ascent, Coord descent, Coord natural, Coord stretch, Coord shrink
) {
    ascent_ = ascent;
    descent_ = descent;
    natural_ = natural;
    stretch_ = stretch;
    shrink_ = shrink;
}

VStrut::~VStrut() { }

//+ VStrut(Glyph::request)
void VStrut::request(Glyph::Requisition& r) {
    Coord height = ascent_ + descent_;
    require(r.x, natural_, stretch_, shrink_, 0);
    require(r.y, height, 0, 0, (height == 0) ? 0 : descent_ / height);
}

/* class ShapeOf */

ShapeOf::ShapeOf(GlyphRef x, GlyphRef y, GlyphRef z) {
    x_ = Glyph::_duplicate(x);
    y_ = Glyph::_duplicate(y);
    z_ = Glyph::_duplicate(z);
}

ShapeOf::~ShapeOf() {
    Fresco::unref(x_);
    Fresco::unref(y_);
    Fresco::unref(z_);
}

//+ ShapeOf(Glyph::request)
void ShapeOf::request(Glyph::Requisition& r) {
    if (is_nil(y_) && is_nil(z_)) {
	x_->request(r);
    } else {
	Glyph::Requisition req;
	if (is_not_nil(x_)) {
	    x_->request(req);
	    r.x = req.x;
	}
	if (is_not_nil(y_)) {
	    y_->request(req);
	    r.y = req.y;
	}
	if (is_not_nil(z_)) {
	    z_->request(req);
	    r.z = req.z;
	}
    }
}
