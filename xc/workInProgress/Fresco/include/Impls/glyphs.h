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

#ifndef Fresco_Impls_glyphs_h
#define Fresco_Impls_glyphs_h

#include <X11/Fresco/glyph.h>
#include <X11/Fresco/Impls/fobjects.h>
#include <X11/Fresco/OS/list.h>

class RegionImpl;
class TransformImpl;

declarePtrList(GlyphOffsetList,GlyphOffsetType)

//- GlyphImpl*
//+ GlyphImpl : GlyphType
class GlyphImpl : public GlyphType {
public:
    ~GlyphImpl();
    TypeObjId _tid();
    static GlyphImpl* _narrow(BaseObjectRef);
//+
    //. GlyphImpl is a default implementation for library
    //. glyph objects (those that live in the same address space
    //. as their creator).  GlyphImpl should be not used for
    //. aggregates, only leaf glyphs.
public:
    GlyphImpl();

    //+ Glyph::*
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
    //+

    //- init_requisition, default_requisition
    static void init_requisition(Glyph::Requisition& r);
    static void default_requisition(Glyph::Requisition& r);
	//. These operations initialize the given requisition.
	//. The init_requisition operation sets the requirements
	//. along each axis to be undefined.  The default_requisition
	//. operation sets each requirement to have a rigid, zero
	//. size and zero alignment.

    //- require
    static void require(
	Glyph::Requirement& r,
	Coord natural, Coord stretch, Coord shrink, float alignment
    );
	//. Set the given requirement to the given sizes and alignment.

    //- require_lead_trail
    static void require_lead_trail(
	Glyph::Requirement& r,
	Coord natural_lead, Coord max_lead, Coord min_lead,
	Coord natural_trail, Coord max_trail, Coord min_trail
    );
	//. Set the given requirement based on the given sizes that
	//. define the natural, maximum, and minimum sizes
	//. before (lead) and after (trail) the requirement's origin.

    //- requirement
    static Glyph::Requirement* requirement(Glyph::Requisition& r, Axis a);
	//. Return the requirement for the given requisition along
	//. the given axis.
protected:
    SharedFrescoObjectImpl object_;
    GlyphOffsetList parents_;
};

//- GlyphVisitorImpl
class GlyphVisitorImpl : public GlyphVisitorType {
    //. GlyphVisitorImpl provides a default implementation for
    //. GlyphVisitor.  Other implementations can subclass
    //. from GlyphVisitorImpl and just redefine visit.
public:
    GlyphVisitorImpl();
    ~GlyphVisitorImpl();

    //+ GlyphVisitor::*
    /* FrescoObject */
    Long ref__(Long references);
    Tag attach(FrescoObjectRef observer);
    void detach(Tag attach_tag);
    void disconnect();
    void notify_observers();
    void update();
    /* GlyphVisitor */
    Boolean visit(GlyphRef glyph, GlyphOffsetRef offset);
    //+
protected:
    SharedFrescoObjectImpl object_;
};

class MonoGlyph;

class MonoGlyphOffset : public GlyphOffsetType {
public:
    MonoGlyphOffset(GlyphRef);
    ~MonoGlyphOffset();

    //+ GlyphOffset::*
    /* FrescoObject */
    Long ref__(Long references);
    Tag attach(FrescoObjectRef observer);
    void detach(Tag attach_tag);
    void disconnect();
    void notify_observers();
    void update();
    /* GlyphOffset */
    GlyphRef _c_parent();
    GlyphRef _c_child();
    void allocations(Glyph::AllocationInfoList& a);
    GlyphOffsetRef _c_insert(GlyphRef g);
    void replace(GlyphRef g);
    void remove();
    void notify();
    //+

    GlyphRef parent_;
    GlyphRef child_;
    Tag remove_tag_;
};

//- MonoGlyph*
class MonoGlyph : public GlyphImpl {
    //. A MonoGlyph delegates some of its operations to another glyph,
    //. called the body.
public:
    MonoGlyph();
    ~MonoGlyph();

    void request(Glyph::Requisition& r); //+ Glyph::request
    void extension(const Glyph::AllocationInfo& a, RegionRef r); //+ Glyph::extension
    void shape(RegionRef r); //+ Glyph::shape
    void traverse(GlyphTraversalRef t); //+ Glyph::traverse
    void _c_body(GlyphRef); //+ Glyph::body=
    GlyphRef _c_body(); //+ Glyph::body?
    GlyphOffsetRef _c_append(GlyphRef g); //+ Glyph::append
    GlyphOffsetRef _c_prepend(GlyphRef g); //+ Glyph::prepend
    void visit_children(GlyphVisitorRef v); //+ Glyph::visit_children
    void visit_children_reversed(GlyphVisitorRef v); //+ Glyph::visit_children_reversed

    MonoGlyphOffset offset_;
};

//- DebugGlyph*
class DebugGlyph : public MonoGlyph {
    //. DebugGlyph is a glyph that prints information
    //. about Glyph operations to standard output and then
    //. forwards the operations to its body.  DebugGlyph is
    //. very useful for debugging layout management and
    //. screen update.
public:
    //- Flags
    enum Flags {
        trace_none = 0x0,
        trace_request = 0x1,
        trace_draw = 0x2,
        trace_pick = 0x4,
	trace_other = 0x8,
        trace_traverse = 0x1e,
        /* convenient shorthand */
	trace_request_traverse = 0x1f,
        trace_request_draw = 0x3,
        trace_request_pick = 0x5,
        trace_draw_pick = 0x6,
        trace_request_draw_pick = 0x7
    };
	//. The trace flags determine which operations will have
	//. information displayed.

    //- DebugGlyph
    DebugGlyph(
	GlyphRef g, const char* msg,
	Flags flags = DebugGlyph::trace_request_traverse
    );
	//. The constructor takes as parameters the body,
	//. a message header to print at the beginning of each
	//. line of information, and the flags that determine
	//. which operations are of interest.  The default flags
	//. are to record requests and all traversal operations.

    virtual ~DebugGlyph();

    void request(Glyph::Requisition& r); //+ Glyph::request
    void traverse(GlyphTraversalRef t); //+ Glyph::traverse

    //- print_requirement
    static void print_requirement(Glyph::Requirement&);
	//. Print out information about a requirement.
	//. If the requirement is undefined then "undef" is printed.
	//. Otherwise, the output format is "sizes @ alignment"
	//. where "@ alignment" is not displayed if the alignment
	//. is zero.  If the minimum and natural sizes are the same
	//. then only the natural size is displayed.  Similarly,
	//. if the natural and maximum sizes are the same
	//. then only the natural size is displayed.  So if the
	//. requirement is rigid, only the natural size will be displayed.

    //- print_region
    static void print_region(RegionRef);
	//. Print out information about an allocation.
	//. The format is "X(span), Y(span), Z(span)" where
	//. "span" is the output for the span along an axis.

    //- print_span
    static void print_span(const Region::BoundingSpan&);
	//. Print out information about the span of a region.
	//. The format is "begin,end @ align" where " @ align"
	//. is not displayed if the alignment is zero.
protected:
    const char* msg_;
    DebugGlyph::Flags flags_;

    void heading(const char*);
};

//- Allocator*
class Allocator : public MonoGlyph {
    //. An Allocator is a glyph that always gives its child
    //. an allocation that matches the child's requisition.
    //. This functionality is useful as a gateway between
    //. figure objects, which ignore their allocation, and
    //. layout objects.
public:
    Allocator();
    ~Allocator();

    void request(Glyph::Requisition& r); //+ Glyph::request
    void allocations(Glyph::AllocationInfoList& a); //+ Glyph::allocations
    void traverse(GlyphTraversalRef t); //+ Glyph::traverse
    void need_resize(); //+ Glyph::need_resize

    virtual void adjust_allocation(Glyph::AllocationInfo&);
protected:
    Boolean requested_;
    Glyph::Requisition req_;
    RegionImpl* nat_;

    void update_requisition(Glyph::Requisition& r);
};

//- TransformAllocator*
class TransformAllocator : public Allocator {
    //. A TransformAllocator maps its allocate to a translation
    //. during traversal and always gives its child the child's
    //. natural allocation.  This functionality is useful
    //. as a gateway between layout objects and figure objects
    //. (which ignore their allocation).
public:
    TransformAllocator(
	Alignment x_parent, Alignment y_parent, Alignment z_parent,
	Alignment x_child, Alignment y_child, Alignment z_child
    );
    ~TransformAllocator();

    void request(Glyph::Requisition& r); //+ Glyph::request
    void traverse(GlyphTraversalRef t); //+ Glyph::traverse

    void adjust_allocation(Glyph::AllocationInfo&);
protected:
    Alignment x_parent_, y_parent_, z_parent_;
    Alignment x_child_, y_child_, z_child_;
    TransformImpl* tx_;

    void compute_delta(
	const Vertex& lower, const Vertex& upper, Vertex& delta
    );
};

#endif
