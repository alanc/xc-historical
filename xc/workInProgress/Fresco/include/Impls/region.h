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

#ifndef Fresco_Impls_region_h
#define Fresco_Impls_region_h

#include <X11/Fresco/Impls/fobjects.h>

//- RegionImpl*
class RegionImpl : public RegionType {
    //. RegionImpl is a simple implementation of
    //. the Region interface, managing only the bounding box
    //. of the region.
public:
    //- RegionImpl
    RegionImpl();
	//. The constructor initializes an empty bounding box
	//. with all coordinates zero.

    ~RegionImpl();

    //+ Region::*
    /* FrescoObject */
    Long ref__(Long references);
    Tag attach(FrescoObjectRef observer);
    void detach(Tag attach_tag);
    void disconnect();
    void notify_observers();
    void update();
    /* Region */
    Boolean contains(const Vertex& v);
    Boolean contains_plane(const Vertex& v, Axis a);
    Boolean intersects(RegionRef r);
    void copy(RegionRef r);
    void merge_intersect(RegionRef r);
    void merge_union(RegionRef r);
    void subtract(RegionRef r);
    void transform(TransformObjRef t);
    void bounds(Vertex& lower, Vertex& upper);
    void origin(Vertex& v);
    void span(Axis a, Region::BoundingSpan& s);
    //+
protected:
    SharedFrescoObjectImpl object_;
public:
    Vertex lower_, upper_;
    Alignment xalign_, yalign_, zalign_;

    static void merge_min(Vertex& v0, const Vertex& v);
    static void merge_max(Vertex& v0, const Vertex& v);

    //- notify
    virtual void notify();
	//. Note that the region has been modified by the merge,
	//. subtract, or transform operations.
};

#endif
