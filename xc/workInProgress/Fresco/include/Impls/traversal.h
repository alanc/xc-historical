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

#ifndef Fresco_Impls_traversal_h
#define Fresco_Impls_traversal_h

#include <X11/Fresco/display.h>
#include <X11/Fresco/glyph.h>
#include <X11/Fresco/Impls/fobjects.h>

class GTStack;

class GlyphTraversalImpl : public GlyphTraversalType {
public:
    GlyphTraversalImpl(GlyphTraversal::Operation, WindowRef, DamageObjRef);
    GlyphTraversalImpl(const GlyphTraversalImpl&);
    ~GlyphTraversalImpl();

    //+ GlyphTraversal::*
    /* FrescoObject */
    Long ref__(Long references);
    Tag attach(FrescoObject_in observer);
    void detach(Tag attach_tag);
    void disconnect();
    void notify_observers();
    void update();
    /* GlyphTraversal */
    GlyphTraversal::Operation op();
    GlyphTraversal::Operation swap_op(GlyphTraversal::Operation op);
    void begin_trail(Viewer_in v);
    void end_trail();
    void traverse_child(GlyphOffset_in o, Region_in allocation);
    void visit();
    GlyphTraversalRef _c_trail();
    GlyphRef _c_current_glyph();
    GlyphOffsetRef _c_current_offset();
    ViewerRef _c_current_viewer();
    Boolean forward();
    Boolean backward();
    PainterObjRef _c_painter();
    void _c_painter(PainterObj_in _p);
    DisplayObjRef _c_display();
    ScreenObjRef _c_screen();
    RegionRef _c_allocation();
    Boolean bounds(Vertex& lower, Vertex& upper, Vertex& origin);
    Boolean origin(Vertex& origin);
    Boolean span(Axis a, Region::BoundingSpan& s);
    TransformObjRef _c_transform();
    DamageObjRef _c_damage();
    void hit();
    Long hit_info();
    void hit_info(Long _p);
    GlyphTraversalRef _c_picked();
    void clear();
    //+

    struct Info {
	ViewerRef viewer;
	GlyphRef glyph;
	GlyphOffsetRef offset;
	RegionRef allocation;
	TransformObjRef transform;
    };

    void push(
	ViewerRef v, GlyphRef g, GlyphOffsetRef o,
	RegionRef allocation, TransformObjRef transform
    );
    void pop();
protected:
    SharedFrescoObjectImpl object_;
    GlyphTraversal::Operation op_;
    GTStack* stack_;
    long index_;
    PainterObjRef painter_;
    WindowRef window_;
    DamageObjRef damage_;
    GlyphTraversalImpl* picked_;
    long hit_info_;

    GlyphTraversalImpl::Info* top();
    GlyphTraversalImpl::Info* cur();
};

#endif
