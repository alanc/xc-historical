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

#ifndef Fresco_lib_polyglyph_h
#define Fresco_lib_polyglyph_h

#include <X11/Fresco/Impls/glyphs.h>
#include <X11/Fresco/OS/list.h>

class PolyGlyphOffset;

declarePtrList(PolyGlyphOffsetList,PolyGlyphOffset)

//- PolyGlyph*
class PolyGlyph : public GlyphImpl {
    //. PolyGlyph is a default implementation for aggregate glyphs.
    //. PolyGlyph inherits most operations from GlyphImpl, redefining
    //. only those operations relating to aggregates.  PolyGlyph
    //. represents children as a list of PolyGlyphOffset objects
    //. that denote the child and the child's index into the list.
public:
    PolyGlyph();
    virtual ~PolyGlyph();

    GlyphOffsetRef _c_append(GlyphRef g); //+ Glyph::append
    GlyphOffsetRef _c_prepend(GlyphRef g); //+ Glyph::prepend
    void visit_children(GlyphVisitorRef v); //+ Glyph::visit_children
    void visit_children_reversed(GlyphVisitorRef v); //+ Glyph::visit_children_reversed

    PolyGlyphOffsetList children_;

    //- children_requests
    Glyph::Requisition* children_requests(Glyph::Requisition* req, long n);
	//. Return an array of requisitions for the polyglyph's children.
	//. If the given integer is as large as the number of children,
	//. then the given pointer will be used for the result array.
	//. Otherwise, an array will be dynamically-allocated that
	//. the caller should delete.

    //- child_allocation
    virtual void child_allocation(long index, Glyph::AllocationInfo& a);
	//. Determine the allocation for the given child given the
	//. allocation information for the parent.

    //- fixup
    virtual void fixup(long start, long delta);
	//. This operation is called when a list item is added
	//. or removed, meaning that the indices in all offsets
	//. must be updated.

    //- change
    virtual void change(long index);
	//. Note that the request of the indexed child
	//. may have changed.

    //- modified
    virtual void modified();
	//. Note that the child list has been modified.  A polyglyph
	//. will normally cache its requisition; if so this operation
	//. should invalidate the cache.
};

class PolyGlyphOffset : public GlyphOffsetType {
public:
    PolyGlyphOffset(PolyGlyph* parent, long index, GlyphRef child);
    ~PolyGlyphOffset();

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

    SharedFrescoObjectImpl object_;
    PolyGlyph* parent_;
    long index_;
    GlyphRef child_;
    Tag remove_tag_;
};

#endif