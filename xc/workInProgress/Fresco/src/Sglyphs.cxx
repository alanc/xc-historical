/*
 * $XConsortium$
 */

/*
 * Copyright (c) 1992-93 Silicon Graphics, Inc.
 * Copyright (c) 1993 Fujitsu, Ltd.
 *
 * Permission to use, copy, modify, distribute, and sell this software and 
 * its documentation for any purpose is hereby granted without fee, provided
 * that (i) the above copyright notices and this permission notice appear in
 * all copies of the software and related documentation, and (ii) the name of
 * Silicon Graphics and Fujitsu may not be used in any advertising or
 * publicity relating to the software without the specific, prior written
 * permission of Silicon Graphics and Fujitsu.
 *
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY 
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  
 *
 * IN NO EVENT SHALL SILICON GRAPHICS OR FUJITSU BE LIABLE FOR
 * ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF 
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE 
 * OF THIS SOFTWARE.
 */

/*
 * Stubs for Glyph and related interfaces
 */

#include <X11/Fresco/glyph.h>
#include <X11/Fresco/viewer.h>
#include <X11/Fresco/Impls/glyphs.h>
#include <X11/Fresco/Ox/request.h>
#include <X11/Fresco/Ox/stub.h>
#include <X11/Fresco/Ox/schema.h>

//+ DamageObj::%init,type+dii,client
DamageObjType::DamageObjType() { }
DamageObjType::~DamageObjType() { }
void* DamageObjType::_this() { return this; }

extern TypeObj_Descriptor _Xfvoid_type, _XfRegion_type;

TypeObj_OpData _XfDamageObj_methods[] = {
    { "incur", &_Xfvoid_type, 0 },
    { "extend", &_Xfvoid_type, 1 },
    { "current", &_XfRegion_type, 0 },
    { 0, 0, 0 }
};
TypeObj_ParamData _XfDamageObj_params[] = {
    /* extend */
        { "r", 0, &_XfRegion_type }
};
extern TypeObj_Descriptor _XfFrescoObject_type;
TypeObj_Descriptor* _XfDamageObj_parents[] = { &_XfFrescoObject_type, nil };
extern TypeObjId _XfDamageObj_tid;
extern void _XfDamageObj_receive(BaseObjectRef, ULong, MarshalBuffer&);
TypeObj_Descriptor _XfDamageObj_type = {
    /* type */ 0,
    /* id */ &_XfDamageObj_tid,
    "DamageObj",
    _XfDamageObj_parents, /* offsets */ nil, /* excepts */ nil,
    _XfDamageObj_methods, _XfDamageObj_params,
    &_XfDamageObj_receive
};

DamageObjRef DamageObj::_narrow(BaseObjectRef o) {
    return (DamageObjRef)_BaseObject_tnarrow(
        o, _XfDamageObj_tid, &DamageObjStub::_create
    );
}
TypeObjId DamageObjType::_tid() { return _XfDamageObj_tid; }
void _XfDamageObj_receive(BaseObjectRef _object, ULong _m, MarshalBuffer& _b) {
    extern TypeObjId _XfDamageObj_tid;
    DamageObjRef _this = (DamageObjRef)_BaseObject_tcast(_object, _XfDamageObj_tid);
    switch (_m) {
        case /* incur */ 0: {
            extern MarshalBuffer::ArgInfo _XfDamageObj_incur_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _this->incur();
            _b.reply(_XfDamageObj_incur_pinfo, _arg);
            break;
        }
        case /* extend */ 1: {
            extern MarshalBuffer::ArgInfo _XfDamageObj_extend_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            RegionRef r;
            _arg[1].u_addr = &r;
            _b.receive(_XfDamageObj_extend_pinfo, _arg);
            _this->extend(r);
            _b.reply(_XfDamageObj_extend_pinfo, _arg);
            break;
        }
        case /* current */ 2: {
            extern MarshalBuffer::ArgInfo _XfDamageObj_current_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_objref = _this->_c_current();
            _b.reply(_XfDamageObj_current_pinfo, _arg);
            break;
        }
    }
}
DamageObjStub::DamageObjStub(Exchange* e) { exch_ = e; }
DamageObjStub::~DamageObjStub() { }
BaseObjectRef DamageObjStub::_create(Exchange* e) {
    return (BaseObjectRef)(void*)new DamageObjStub(e);
}
Exchange* DamageObjStub::_exchange() {
    return exch_;
}
MarshalBuffer::ArgDesc _XfDamageObj_incur_pdesc[2] = { 1, 4 };
MarshalBuffer::ArgInfo _XfDamageObj_incur_pinfo = {
    &_XfDamageObj_tid, 0, _XfDamageObj_incur_pdesc, 0
};
void DamageObjType::incur() {
    MarshalBuffer _b;
    extern TypeObjId _XfDamageObj_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfDamageObj_incur_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfDamageObj_extend_pdesc[3] = { 2, 4, 61 };
MarshalBuffer::ArgMarshal _XfDamageObj_extend_pfunc[] = {
    &RegionStub::_create,

};
MarshalBuffer::ArgInfo _XfDamageObj_extend_pinfo = {
    &_XfDamageObj_tid, 1, _XfDamageObj_extend_pdesc, _XfDamageObj_extend_pfunc
};
void DamageObjType::extend(Region_in r) {
    MarshalBuffer _b;
    extern TypeObjId _XfDamageObj_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = r;
    _b.invoke(this, _XfDamageObj_extend_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfDamageObj_current_pdesc[2] = { 1, 60 };
MarshalBuffer::ArgMarshal _XfDamageObj_current_pfunc[] = {
    &RegionStub::_create
};
MarshalBuffer::ArgInfo _XfDamageObj_current_pinfo = {
    &_XfDamageObj_tid, 2, _XfDamageObj_current_pdesc, _XfDamageObj_current_pfunc
};
RegionRef DamageObjType::_c_current() {
    MarshalBuffer _b;
    extern TypeObjId _XfDamageObj_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfDamageObj_current_pinfo, _arg);
    return (RegionRef)_arg[0].u_objref;
}
//+

//+ Glyph::%init,type+dii,client
Glyph::AllocationInfoList& Glyph::AllocationInfoList::operator =(const AllocationInfoList& _s) {
    delete [] _buffer;
    _maximum = _s._maximum;
    _length = _s._length;
    _buffer = _maximum == 0 ? 0 : new AllocationInfo[_maximum];
    for (int i = 0; i < _length; i++) {
        _buffer[i] = _s._buffer[i];
    }
    return *this;
}

GlyphType::GlyphType() { }
GlyphType::~GlyphType() { }
void* GlyphType::_this() { return this; }

extern TypeObj_Descriptor _XfGlyph_type, _XfStyleObj_type, _XfTransformObj_type, 
    _XfGlyph_Requisition_type, _XfGlyph_AllocationInfo_type, _XfGlyphTraversal_type, 
    _XfGlyphOffset_type, _XfTag_type, _XfGlyphVisitor_type, _XfGlyph_AllocationInfoList_type, 
    _XfBoolean_type;

TypeObj_OpData _XfGlyph_methods[] = {
    { "clone_glyph", &_XfGlyph_type, 0 },
    { "_get_style", &_XfStyleObj_type, 0 },
    { "_set_style", &_Xfvoid_type, 1 },
    { "transform", &_XfTransformObj_type, 0 },
    { "request", &_Xfvoid_type, 1 },
    { "extension", &_Xfvoid_type, 2 },
    { "shape", &_XfRegion_type, 0 },
    { "traverse", &_Xfvoid_type, 1 },
    { "draw", &_Xfvoid_type, 1 },
    { "pick", &_Xfvoid_type, 1 },
    { "_get_body", &_XfGlyph_type, 0 },
    { "_set_body", &_Xfvoid_type, 1 },
    { "append", &_XfGlyphOffset_type, 1 },
    { "prepend", &_XfGlyphOffset_type, 1 },
    { "add_parent", &_XfTag_type, 1 },
    { "remove_parent", &_Xfvoid_type, 1 },
    { "visit_children", &_Xfvoid_type, 1 },
    { "visit_children_reversed", &_Xfvoid_type, 1 },
    { "visit_parents", &_Xfvoid_type, 1 },
    { "allocations", &_Xfvoid_type, 1 },
    { "need_redraw", &_Xfvoid_type, 0 },
    { "need_redraw_region", &_Xfvoid_type, 1 },
    { "need_resize", &_Xfvoid_type, 0 },
    { "restore_trail", &_XfBoolean_type, 1 },
    { 0, 0, 0 }
};
TypeObj_ParamData _XfGlyph_params[] = {
    /* style */
        { "_p", 0, &_XfStyleObj_type },
    /* request */
        { "r", 1, &_XfGlyph_Requisition_type },
    /* extension */
        { "a", 0, &_XfGlyph_AllocationInfo_type },
        { "r", 0, &_XfRegion_type },
    /* traverse */
        { "t", 0, &_XfGlyphTraversal_type },
    /* draw */
        { "t", 0, &_XfGlyphTraversal_type },
    /* pick */
        { "t", 0, &_XfGlyphTraversal_type },
    /* body */
        { "_p", 0, &_XfGlyph_type },
    /* append */
        { "g", 0, &_XfGlyph_type },
    /* prepend */
        { "g", 0, &_XfGlyph_type },
    /* add_parent */
        { "parent_offset", 0, &_XfGlyphOffset_type },
    /* remove_parent */
        { "add_tag", 0, &_XfTag_type },
    /* visit_children */
        { "v", 0, &_XfGlyphVisitor_type },
    /* visit_children_reversed */
        { "v", 0, &_XfGlyphVisitor_type },
    /* visit_parents */
        { "v", 0, &_XfGlyphVisitor_type },
    /* allocations */
        { "a", 1, &_XfGlyph_AllocationInfoList_type },
    /* need_redraw_region */
        { "r", 0, &_XfRegion_type },
    /* restore_trail */
        { "t", 0, &_XfGlyphTraversal_type }
};
extern TypeObj_Descriptor _XfFrescoObject_type;
TypeObj_Descriptor* _XfGlyph_parents[] = { &_XfFrescoObject_type, nil };
extern TypeObjId _XfGlyph_tid;
extern void _XfGlyph_receive(BaseObjectRef, ULong, MarshalBuffer&);
TypeObj_Descriptor _XfGlyph_type = {
    /* type */ 0,
    /* id */ &_XfGlyph_tid,
    "Glyph",
    _XfGlyph_parents, /* offsets */ nil, /* excepts */ nil,
    _XfGlyph_methods, _XfGlyph_params,
    &_XfGlyph_receive
};

GlyphRef Glyph::_narrow(BaseObjectRef o) {
    return (GlyphRef)_BaseObject_tnarrow(
        o, _XfGlyph_tid, &GlyphStub::_create
    );
}
TypeObjId GlyphType::_tid() { return _XfGlyph_tid; }
void _XfGlyph_receive(BaseObjectRef _object, ULong _m, MarshalBuffer& _b) {
    extern TypeObjId _XfGlyph_tid;
    GlyphRef _this = (GlyphRef)_BaseObject_tcast(_object, _XfGlyph_tid);
    switch (_m) {
        case /* clone_glyph */ 0: {
            extern MarshalBuffer::ArgInfo _XfGlyph_clone_glyph_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_objref = _this->_c_clone_glyph();
            _b.reply(_XfGlyph_clone_glyph_pinfo, _arg);
            break;
        }
        case /* _get_style */ 1: {
            extern MarshalBuffer::ArgInfo _XfGlyph__get_style_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_objref = _this->_c_style();
            _b.reply(_XfGlyph__get_style_pinfo, _arg);
            break;
        }
        case /* _set_style */ 2: {
            extern MarshalBuffer::ArgInfo _XfGlyph__set_style_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            StyleObjRef _p;
            _arg[1].u_addr = &_p;
            _b.receive(_XfGlyph__set_style_pinfo, _arg);
            _this->_c_style(_p);
            _b.reply(_XfGlyph__set_style_pinfo, _arg);
            break;
        }
        case /* transform */ 3: {
            extern MarshalBuffer::ArgInfo _XfGlyph_transform_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_objref = _this->_c_transform();
            _b.reply(_XfGlyph_transform_pinfo, _arg);
            break;
        }
        case /* request */ 4: {
            extern MarshalBuffer::ArgInfo _XfGlyph_request_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            Glyph::Requisition r;
            _arg[1].u_addr = &r;
            _b.receive(_XfGlyph_request_pinfo, _arg);
            _this->request(r);
            _b.reply(_XfGlyph_request_pinfo, _arg);
            break;
        }
        case /* extension */ 5: {
            extern MarshalBuffer::ArgInfo _XfGlyph_extension_pinfo;
            MarshalBuffer::ArgValue _arg[3];
            Glyph::AllocationInfo a;
            _arg[1].u_addr = &a;
            RegionRef r;
            _arg[2].u_addr = &r;
            _b.receive(_XfGlyph_extension_pinfo, _arg);
            _this->extension(a, r);
            _b.reply(_XfGlyph_extension_pinfo, _arg);
            break;
        }
        case /* shape */ 6: {
            extern MarshalBuffer::ArgInfo _XfGlyph_shape_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_objref = _this->_c_shape();
            _b.reply(_XfGlyph_shape_pinfo, _arg);
            break;
        }
        case /* traverse */ 7: {
            extern MarshalBuffer::ArgInfo _XfGlyph_traverse_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            GlyphTraversalRef t;
            _arg[1].u_addr = &t;
            _b.receive(_XfGlyph_traverse_pinfo, _arg);
            _this->traverse(t);
            _b.reply(_XfGlyph_traverse_pinfo, _arg);
            break;
        }
        case /* draw */ 8: {
            extern MarshalBuffer::ArgInfo _XfGlyph_draw_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            GlyphTraversalRef t;
            _arg[1].u_addr = &t;
            _b.receive(_XfGlyph_draw_pinfo, _arg);
            _this->draw(t);
            _b.reply(_XfGlyph_draw_pinfo, _arg);
            break;
        }
        case /* pick */ 9: {
            extern MarshalBuffer::ArgInfo _XfGlyph_pick_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            GlyphTraversalRef t;
            _arg[1].u_addr = &t;
            _b.receive(_XfGlyph_pick_pinfo, _arg);
            _this->pick(t);
            _b.reply(_XfGlyph_pick_pinfo, _arg);
            break;
        }
        case /* _get_body */ 10: {
            extern MarshalBuffer::ArgInfo _XfGlyph__get_body_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_objref = _this->_c_body();
            _b.reply(_XfGlyph__get_body_pinfo, _arg);
            break;
        }
        case /* _set_body */ 11: {
            extern MarshalBuffer::ArgInfo _XfGlyph__set_body_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            GlyphRef _p;
            _arg[1].u_addr = &_p;
            _b.receive(_XfGlyph__set_body_pinfo, _arg);
            _this->_c_body(_p);
            _b.reply(_XfGlyph__set_body_pinfo, _arg);
            break;
        }
        case /* append */ 12: {
            extern MarshalBuffer::ArgInfo _XfGlyph_append_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            GlyphRef g;
            _arg[1].u_addr = &g;
            _b.receive(_XfGlyph_append_pinfo, _arg);
            _arg[0].u_objref = _this->_c_append(g);
            _b.reply(_XfGlyph_append_pinfo, _arg);
            break;
        }
        case /* prepend */ 13: {
            extern MarshalBuffer::ArgInfo _XfGlyph_prepend_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            GlyphRef g;
            _arg[1].u_addr = &g;
            _b.receive(_XfGlyph_prepend_pinfo, _arg);
            _arg[0].u_objref = _this->_c_prepend(g);
            _b.reply(_XfGlyph_prepend_pinfo, _arg);
            break;
        }
        case /* add_parent */ 14: {
            extern MarshalBuffer::ArgInfo _XfGlyph_add_parent_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            GlyphOffsetRef parent_offset;
            _arg[1].u_addr = &parent_offset;
            _b.receive(_XfGlyph_add_parent_pinfo, _arg);
            _arg[0].u_unsigned_long = _this->add_parent(parent_offset);
            _b.reply(_XfGlyph_add_parent_pinfo, _arg);
            break;
        }
        case /* remove_parent */ 15: {
            extern MarshalBuffer::ArgInfo _XfGlyph_remove_parent_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            Tag add_tag;
            _arg[1].u_addr = &add_tag;
            _b.receive(_XfGlyph_remove_parent_pinfo, _arg);
            _this->remove_parent(add_tag);
            _b.reply(_XfGlyph_remove_parent_pinfo, _arg);
            break;
        }
        case /* visit_children */ 16: {
            extern MarshalBuffer::ArgInfo _XfGlyph_visit_children_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            GlyphVisitorRef v;
            _arg[1].u_addr = &v;
            _b.receive(_XfGlyph_visit_children_pinfo, _arg);
            _this->visit_children(v);
            _b.reply(_XfGlyph_visit_children_pinfo, _arg);
            break;
        }
        case /* visit_children_reversed */ 17: {
            extern MarshalBuffer::ArgInfo _XfGlyph_visit_children_reversed_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            GlyphVisitorRef v;
            _arg[1].u_addr = &v;
            _b.receive(_XfGlyph_visit_children_reversed_pinfo, _arg);
            _this->visit_children_reversed(v);
            _b.reply(_XfGlyph_visit_children_reversed_pinfo, _arg);
            break;
        }
        case /* visit_parents */ 18: {
            extern MarshalBuffer::ArgInfo _XfGlyph_visit_parents_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            GlyphVisitorRef v;
            _arg[1].u_addr = &v;
            _b.receive(_XfGlyph_visit_parents_pinfo, _arg);
            _this->visit_parents(v);
            _b.reply(_XfGlyph_visit_parents_pinfo, _arg);
            break;
        }
        case /* allocations */ 19: {
            extern MarshalBuffer::ArgInfo _XfGlyph_allocations_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            Glyph::AllocationInfoList a;
            _arg[1].u_addr = &a;
            _b.receive(_XfGlyph_allocations_pinfo, _arg);
            _this->allocations(a);
            _b.reply(_XfGlyph_allocations_pinfo, _arg);
            break;
        }
        case /* need_redraw */ 20: {
            extern MarshalBuffer::ArgInfo _XfGlyph_need_redraw_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _this->need_redraw();
            _b.reply(_XfGlyph_need_redraw_pinfo, _arg);
            break;
        }
        case /* need_redraw_region */ 21: {
            extern MarshalBuffer::ArgInfo _XfGlyph_need_redraw_region_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            RegionRef r;
            _arg[1].u_addr = &r;
            _b.receive(_XfGlyph_need_redraw_region_pinfo, _arg);
            _this->need_redraw_region(r);
            _b.reply(_XfGlyph_need_redraw_region_pinfo, _arg);
            break;
        }
        case /* need_resize */ 22: {
            extern MarshalBuffer::ArgInfo _XfGlyph_need_resize_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _this->need_resize();
            _b.reply(_XfGlyph_need_resize_pinfo, _arg);
            break;
        }
        case /* restore_trail */ 23: {
            extern MarshalBuffer::ArgInfo _XfGlyph_restore_trail_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            GlyphTraversalRef t;
            _arg[1].u_addr = &t;
            _b.receive(_XfGlyph_restore_trail_pinfo, _arg);
            _arg[0].u_boolean = _this->restore_trail(t);
            _b.reply(_XfGlyph_restore_trail_pinfo, _arg);
            break;
        }
    }
}
extern void _XfGlyph_Requirement_put(
    MarshalBuffer&, const Glyph::Requirement&
);
extern void _XfGlyph_Requirement_get(
    MarshalBuffer&, Glyph::Requirement&
);
extern void _XfGlyph_AllocationInfo_put(
    MarshalBuffer&, const Glyph::AllocationInfo&
);
extern void _XfGlyph_AllocationInfo_get(
    MarshalBuffer&, Glyph::AllocationInfo&
);
extern void _XfGlyph_AllocationInfoList_put(
    MarshalBuffer&, const Glyph::AllocationInfoList&
);
extern void _XfGlyph_AllocationInfoList_get(
    MarshalBuffer&, Glyph::AllocationInfoList&
);
extern void _XfGlyph_Requisition_put(
    MarshalBuffer&, const Glyph::Requisition&
);
extern void _XfGlyph_Requisition_get(
    MarshalBuffer&, Glyph::Requisition&
);
extern void _XfGlyph_AllocationInfo_put(
    MarshalBuffer&, const Glyph::AllocationInfo&
);
extern void _XfGlyph_AllocationInfo_get(
    MarshalBuffer&, Glyph::AllocationInfo&
);
extern void _XfGlyph_AllocationInfo_put(
    MarshalBuffer&, const Glyph::AllocationInfo&
);
extern void _XfGlyph_AllocationInfo_get(
    MarshalBuffer&, Glyph::AllocationInfo&
);
extern void _XfGlyph_AllocationInfoList_put(
    MarshalBuffer&, const Glyph::AllocationInfoList&
);
extern void _XfGlyph_AllocationInfoList_get(
    MarshalBuffer&, Glyph::AllocationInfoList&
);

GlyphStub::GlyphStub(Exchange* e) { exch_ = e; }
GlyphStub::~GlyphStub() { }
BaseObjectRef GlyphStub::_create(Exchange* e) {
    return (BaseObjectRef)(void*)new GlyphStub(e);
}
Exchange* GlyphStub::_exchange() {
    return exch_;
}
MarshalBuffer::ArgDesc _XfGlyph_clone_glyph_pdesc[2] = { 1, 60 };
MarshalBuffer::ArgMarshal _XfGlyph_clone_glyph_pfunc[] = {
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfGlyph_clone_glyph_pinfo = {
    &_XfGlyph_tid, 0, _XfGlyph_clone_glyph_pdesc, _XfGlyph_clone_glyph_pfunc
};
GlyphRef GlyphType::_c_clone_glyph() {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyph_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfGlyph_clone_glyph_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfGlyph__get_style_pdesc[2] = { 1, 60 };
MarshalBuffer::ArgMarshal _XfGlyph__get_style_pfunc[] = {
    &StyleObjStub::_create
};
MarshalBuffer::ArgInfo _XfGlyph__get_style_pinfo = {
    &_XfGlyph_tid, 1, _XfGlyph__get_style_pdesc, _XfGlyph__get_style_pfunc
};
StyleObjRef GlyphType::_c_style() {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyph_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfGlyph__get_style_pinfo, _arg);
    return (StyleObjRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfGlyph__set_style_pdesc[3] = { 2, 4, 61 };
MarshalBuffer::ArgMarshal _XfGlyph__set_style_pfunc[] = {
    &StyleObjStub::_create,

};
MarshalBuffer::ArgInfo _XfGlyph__set_style_pinfo = {
    &_XfGlyph_tid, 2, _XfGlyph__set_style_pdesc, _XfGlyph__set_style_pfunc
};
void GlyphType::_c_style(StyleObj_in _p) {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyph_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = _p;
    _b.invoke(this, _XfGlyph__set_style_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfGlyph_transform_pdesc[2] = { 1, 60 };
MarshalBuffer::ArgMarshal _XfGlyph_transform_pfunc[] = {
    &TransformObjStub::_create
};
MarshalBuffer::ArgInfo _XfGlyph_transform_pinfo = {
    &_XfGlyph_tid, 3, _XfGlyph_transform_pdesc, _XfGlyph_transform_pfunc
};
TransformObjRef GlyphType::_c_transform() {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyph_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfGlyph_transform_pinfo, _arg);
    return (TransformObjRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfGlyph_request_pdesc[3] = { 2, 4, 2 };
MarshalBuffer::ArgMarshal _XfGlyph_request_pfunc[] = {
    &_XfGlyph_Requisition_put, &_XfGlyph_Requisition_get,

};
MarshalBuffer::ArgInfo _XfGlyph_request_pinfo = {
    &_XfGlyph_tid, 4, _XfGlyph_request_pdesc, _XfGlyph_request_pfunc
};
void GlyphType::request(Glyph::Requisition& r) {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyph_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_addr = &r;
    _b.invoke(this, _XfGlyph_request_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfGlyph_extension_pdesc[4] = { 3, 4, 1, 61 };
MarshalBuffer::ArgMarshal _XfGlyph_extension_pfunc[] = {
    &_XfGlyph_AllocationInfo_put, &_XfGlyph_AllocationInfo_get,
    &RegionStub::_create,

};
MarshalBuffer::ArgInfo _XfGlyph_extension_pinfo = {
    &_XfGlyph_tid, 5, _XfGlyph_extension_pdesc, _XfGlyph_extension_pfunc
};
void GlyphType::extension(const Glyph::AllocationInfo& a, Region_in r) {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyph_tid;
    MarshalBuffer::ArgValue _arg[3];
    _arg[1].u_addr = &a;
    _arg[2].u_objref = r;
    _b.invoke(this, _XfGlyph_extension_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfGlyph_shape_pdesc[2] = { 1, 60 };
MarshalBuffer::ArgMarshal _XfGlyph_shape_pfunc[] = {
    &RegionStub::_create
};
MarshalBuffer::ArgInfo _XfGlyph_shape_pinfo = {
    &_XfGlyph_tid, 6, _XfGlyph_shape_pdesc, _XfGlyph_shape_pfunc
};
RegionRef GlyphType::_c_shape() {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyph_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfGlyph_shape_pinfo, _arg);
    return (RegionRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfGlyph_traverse_pdesc[3] = { 2, 4, 61 };
MarshalBuffer::ArgMarshal _XfGlyph_traverse_pfunc[] = {
    &GlyphTraversalStub::_create,

};
MarshalBuffer::ArgInfo _XfGlyph_traverse_pinfo = {
    &_XfGlyph_tid, 7, _XfGlyph_traverse_pdesc, _XfGlyph_traverse_pfunc
};
void GlyphType::traverse(GlyphTraversal_in t) {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyph_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = t;
    _b.invoke(this, _XfGlyph_traverse_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfGlyph_draw_pdesc[3] = { 2, 4, 61 };
MarshalBuffer::ArgMarshal _XfGlyph_draw_pfunc[] = {
    &GlyphTraversalStub::_create,

};
MarshalBuffer::ArgInfo _XfGlyph_draw_pinfo = {
    &_XfGlyph_tid, 8, _XfGlyph_draw_pdesc, _XfGlyph_draw_pfunc
};
void GlyphType::draw(GlyphTraversal_in t) {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyph_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = t;
    _b.invoke(this, _XfGlyph_draw_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfGlyph_pick_pdesc[3] = { 2, 4, 61 };
MarshalBuffer::ArgMarshal _XfGlyph_pick_pfunc[] = {
    &GlyphTraversalStub::_create,

};
MarshalBuffer::ArgInfo _XfGlyph_pick_pinfo = {
    &_XfGlyph_tid, 9, _XfGlyph_pick_pdesc, _XfGlyph_pick_pfunc
};
void GlyphType::pick(GlyphTraversal_in t) {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyph_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = t;
    _b.invoke(this, _XfGlyph_pick_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfGlyph__get_body_pdesc[2] = { 1, 60 };
MarshalBuffer::ArgMarshal _XfGlyph__get_body_pfunc[] = {
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfGlyph__get_body_pinfo = {
    &_XfGlyph_tid, 10, _XfGlyph__get_body_pdesc, _XfGlyph__get_body_pfunc
};
GlyphRef GlyphType::_c_body() {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyph_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfGlyph__get_body_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfGlyph__set_body_pdesc[3] = { 2, 4, 61 };
MarshalBuffer::ArgMarshal _XfGlyph__set_body_pfunc[] = {
    &GlyphStub::_create,

};
MarshalBuffer::ArgInfo _XfGlyph__set_body_pinfo = {
    &_XfGlyph_tid, 11, _XfGlyph__set_body_pdesc, _XfGlyph__set_body_pfunc
};
void GlyphType::_c_body(Glyph_in _p) {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyph_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = _p;
    _b.invoke(this, _XfGlyph__set_body_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfGlyph_append_pdesc[3] = { 2, 60, 61 };
MarshalBuffer::ArgMarshal _XfGlyph_append_pfunc[] = {
    &GlyphStub::_create,
    &GlyphOffsetStub::_create
};
MarshalBuffer::ArgInfo _XfGlyph_append_pinfo = {
    &_XfGlyph_tid, 12, _XfGlyph_append_pdesc, _XfGlyph_append_pfunc
};
GlyphOffset_tmp GlyphType::append(Glyph_in g) {
    return _c_append(g);
}
GlyphOffsetRef GlyphType::_c_append(Glyph_in g) {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyph_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = g;
    _b.invoke(this, _XfGlyph_append_pinfo, _arg);
    return (GlyphOffsetRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfGlyph_prepend_pdesc[3] = { 2, 60, 61 };
MarshalBuffer::ArgMarshal _XfGlyph_prepend_pfunc[] = {
    &GlyphStub::_create,
    &GlyphOffsetStub::_create
};
MarshalBuffer::ArgInfo _XfGlyph_prepend_pinfo = {
    &_XfGlyph_tid, 13, _XfGlyph_prepend_pdesc, _XfGlyph_prepend_pfunc
};
GlyphOffset_tmp GlyphType::prepend(Glyph_in g) {
    return _c_prepend(g);
}
GlyphOffsetRef GlyphType::_c_prepend(Glyph_in g) {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyph_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = g;
    _b.invoke(this, _XfGlyph_prepend_pinfo, _arg);
    return (GlyphOffsetRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfGlyph_add_parent_pdesc[3] = { 2, 36, 61 };
MarshalBuffer::ArgMarshal _XfGlyph_add_parent_pfunc[] = {
    &GlyphOffsetStub::_create,

};
MarshalBuffer::ArgInfo _XfGlyph_add_parent_pinfo = {
    &_XfGlyph_tid, 14, _XfGlyph_add_parent_pdesc, _XfGlyph_add_parent_pfunc
};
Tag GlyphType::add_parent(GlyphOffset_in parent_offset) {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyph_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = parent_offset;
    _b.invoke(this, _XfGlyph_add_parent_pinfo, _arg);
    return _arg[0].u_unsigned_long;
}
MarshalBuffer::ArgDesc _XfGlyph_remove_parent_pdesc[3] = { 2, 4, 37 };
MarshalBuffer::ArgInfo _XfGlyph_remove_parent_pinfo = {
    &_XfGlyph_tid, 15, _XfGlyph_remove_parent_pdesc, 0
};
void GlyphType::remove_parent(Tag add_tag) {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyph_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_unsigned_long = add_tag;
    _b.invoke(this, _XfGlyph_remove_parent_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfGlyph_visit_children_pdesc[3] = { 2, 4, 61 };
MarshalBuffer::ArgMarshal _XfGlyph_visit_children_pfunc[] = {
    &GlyphVisitorStub::_create,

};
MarshalBuffer::ArgInfo _XfGlyph_visit_children_pinfo = {
    &_XfGlyph_tid, 16, _XfGlyph_visit_children_pdesc, _XfGlyph_visit_children_pfunc
};
void GlyphType::visit_children(GlyphVisitor_in v) {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyph_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = v;
    _b.invoke(this, _XfGlyph_visit_children_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfGlyph_visit_children_reversed_pdesc[3] = { 2, 4, 61 };
MarshalBuffer::ArgMarshal _XfGlyph_visit_children_reversed_pfunc[] = {
    &GlyphVisitorStub::_create,

};
MarshalBuffer::ArgInfo _XfGlyph_visit_children_reversed_pinfo = {
    &_XfGlyph_tid, 17, _XfGlyph_visit_children_reversed_pdesc, _XfGlyph_visit_children_reversed_pfunc
};
void GlyphType::visit_children_reversed(GlyphVisitor_in v) {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyph_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = v;
    _b.invoke(this, _XfGlyph_visit_children_reversed_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfGlyph_visit_parents_pdesc[3] = { 2, 4, 61 };
MarshalBuffer::ArgMarshal _XfGlyph_visit_parents_pfunc[] = {
    &GlyphVisitorStub::_create,

};
MarshalBuffer::ArgInfo _XfGlyph_visit_parents_pinfo = {
    &_XfGlyph_tid, 18, _XfGlyph_visit_parents_pdesc, _XfGlyph_visit_parents_pfunc
};
void GlyphType::visit_parents(GlyphVisitor_in v) {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyph_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = v;
    _b.invoke(this, _XfGlyph_visit_parents_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfGlyph_allocations_pdesc[3] = { 2, 4, 2 };
MarshalBuffer::ArgMarshal _XfGlyph_allocations_pfunc[] = {
    &_XfGlyph_AllocationInfoList_put, &_XfGlyph_AllocationInfoList_get,

};
MarshalBuffer::ArgInfo _XfGlyph_allocations_pinfo = {
    &_XfGlyph_tid, 19, _XfGlyph_allocations_pdesc, _XfGlyph_allocations_pfunc
};
void GlyphType::allocations(Glyph::AllocationInfoList& a) {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyph_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_addr = &a;
    _b.invoke(this, _XfGlyph_allocations_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfGlyph_need_redraw_pdesc[2] = { 1, 4 };
MarshalBuffer::ArgInfo _XfGlyph_need_redraw_pinfo = {
    &_XfGlyph_tid, 20, _XfGlyph_need_redraw_pdesc, 0
};
void GlyphType::need_redraw() {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyph_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfGlyph_need_redraw_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfGlyph_need_redraw_region_pdesc[3] = { 2, 4, 61 };
MarshalBuffer::ArgMarshal _XfGlyph_need_redraw_region_pfunc[] = {
    &RegionStub::_create,

};
MarshalBuffer::ArgInfo _XfGlyph_need_redraw_region_pinfo = {
    &_XfGlyph_tid, 21, _XfGlyph_need_redraw_region_pdesc, _XfGlyph_need_redraw_region_pfunc
};
void GlyphType::need_redraw_region(Region_in r) {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyph_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = r;
    _b.invoke(this, _XfGlyph_need_redraw_region_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfGlyph_need_resize_pdesc[2] = { 1, 4 };
MarshalBuffer::ArgInfo _XfGlyph_need_resize_pinfo = {
    &_XfGlyph_tid, 22, _XfGlyph_need_resize_pdesc, 0
};
void GlyphType::need_resize() {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyph_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfGlyph_need_resize_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfGlyph_restore_trail_pdesc[3] = { 2, 12, 61 };
MarshalBuffer::ArgMarshal _XfGlyph_restore_trail_pfunc[] = {
    &GlyphTraversalStub::_create,

};
MarshalBuffer::ArgInfo _XfGlyph_restore_trail_pinfo = {
    &_XfGlyph_tid, 23, _XfGlyph_restore_trail_pdesc, _XfGlyph_restore_trail_pfunc
};
Boolean GlyphType::restore_trail(GlyphTraversal_in t) {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyph_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = t;
    _b.invoke(this, _XfGlyph_restore_trail_pinfo, _arg);
    return _arg[0].u_boolean;
}
void _XfGlyph_Requirement_put(MarshalBuffer& _b, const Glyph::Requirement& _this) {
    _b.put_boolean(_this.defined);
    _b.put_float(_this.natural);
    _b.put_float(_this.maximum);
    _b.put_float(_this.minimum);
    _b.put_float(_this.align);
}
void _XfGlyph_Requirement_get(MarshalBuffer& _b, Glyph::Requirement& _this) {
    _this.defined = _b.get_boolean();
    _this.natural = _b.get_float();
    _this.maximum = _b.get_float();
    _this.minimum = _b.get_float();
    _this.align = _b.get_float();
}
void _XfGlyph_Requisition_put(MarshalBuffer& _b, const Glyph::Requisition& _this) {
    _XfGlyph_Requirement_put(_b, _this.x);
    _XfGlyph_Requirement_put(_b, _this.y);
    _XfGlyph_Requirement_put(_b, _this.z);
    _b.put_boolean(_this.preserve_aspect);
}
void _XfGlyph_Requisition_get(MarshalBuffer& _b, Glyph::Requisition& _this) {
    _XfGlyph_Requirement_get(_b, _this.x);
    _XfGlyph_Requirement_get(_b, _this.y);
    _XfGlyph_Requirement_get(_b, _this.z);
    _this.preserve_aspect = _b.get_boolean();
}
void _XfGlyph_AllocationInfo_put(MarshalBuffer& _b, const Glyph::AllocationInfo& _this) {
    _b.put_object(_this.allocation);
    _b.put_object(_this.transform);
    _b.put_object(_this.damage);
}
void _XfGlyph_AllocationInfo_get(MarshalBuffer& _b, Glyph::AllocationInfo& _this) {
    _this.allocation = (RegionRef)_b.get_object(&RegionStub::_create);
    _this.transform = (TransformObjRef)_b.get_object(&TransformObjStub::_create);
    _this.damage = (DamageObjRef)_b.get_object(&DamageObjStub::_create);
}
void _XfGlyph_AllocationInfoList_put(MarshalBuffer& _b, const Glyph::AllocationInfoList& _this) {
    long _i;
    _b.put_seq_hdr(&_this);
    for (_i = 0; _i < _this._length; _i++) {
        _XfGlyph_AllocationInfo_put(_b, _this._buffer[_i]);
    }
}
void _XfGlyph_AllocationInfoList_get(MarshalBuffer& _b, Glyph::AllocationInfoList& _this) {
    Long _i;
    _b.get_seq_hdr(&_this);
    _this._buffer = (_this._maximum == 0) ? 0 : new Glyph::AllocationInfo[_this._maximum];
    for (_i = 0; _i < _this._length; _i++) {
        _XfGlyph_AllocationInfo_get(_b, _this._buffer[_i]);
    }
}
//+

//+ GlyphImpl(Glyph)
extern TypeObj_Descriptor _XfGlyph_type;
TypeObj_Descriptor* _XfGlyphImpl_parents[] = { &_XfGlyph_type, nil };
extern TypeObjId _XfGlyphImpl_tid;
TypeObj_Descriptor _XfGlyphImpl_type = {
    /* type */ 0,
    /* id */ &_XfGlyphImpl_tid,
    "GlyphImpl",
    _XfGlyphImpl_parents, /* offsets */ nil, /* excepts */ nil,
    /* methods */ nil, /* params */ nil,
    /* receive */ nil
};

GlyphImpl* GlyphImpl::_narrow(BaseObjectRef o) {
    return (GlyphImpl*)_BaseObject_tnarrow(
        o, _XfGlyphImpl_tid, 0
    );
}
TypeObjId GlyphImpl::_tid() { return _XfGlyphImpl_tid; }
//+

//+ GlyphOffset::%init,type+dii,client
GlyphOffsetType::GlyphOffsetType() { }
GlyphOffsetType::~GlyphOffsetType() { }
void* GlyphOffsetType::_this() { return this; }

extern TypeObj_Descriptor _Xfvoid_type;

TypeObj_OpData _XfGlyphOffset_methods[] = {
    { "parent", &_XfGlyph_type, 0 },
    { "child", &_XfGlyph_type, 0 },
    { "allocations", &_Xfvoid_type, 1 },
    { "insert", &_XfGlyphOffset_type, 1 },
    { "replace", &_Xfvoid_type, 1 },
    { "remove", &_Xfvoid_type, 0 },
    { "notify", &_Xfvoid_type, 0 },
    { "visit_trail", &_Xfvoid_type, 1 },
    { "child_allocate", &_Xfvoid_type, 1 },
    { 0, 0, 0 }
};
TypeObj_ParamData _XfGlyphOffset_params[] = {
    /* allocations */
        { "a", 1, &_XfGlyph_AllocationInfoList_type },
    /* insert */
        { "g", 0, &_XfGlyph_type },
    /* replace */
        { "g", 0, &_XfGlyph_type },
    /* visit_trail */
        { "t", 0, &_XfGlyphTraversal_type },
    /* child_allocate */
        { "a", 2, &_XfGlyph_AllocationInfo_type }
};
extern TypeObj_Descriptor _XfFrescoObject_type;
TypeObj_Descriptor* _XfGlyphOffset_parents[] = { &_XfFrescoObject_type, nil };
extern TypeObjId _XfGlyphOffset_tid;
extern void _XfGlyphOffset_receive(BaseObjectRef, ULong, MarshalBuffer&);
TypeObj_Descriptor _XfGlyphOffset_type = {
    /* type */ 0,
    /* id */ &_XfGlyphOffset_tid,
    "GlyphOffset",
    _XfGlyphOffset_parents, /* offsets */ nil, /* excepts */ nil,
    _XfGlyphOffset_methods, _XfGlyphOffset_params,
    &_XfGlyphOffset_receive
};

GlyphOffsetRef GlyphOffset::_narrow(BaseObjectRef o) {
    return (GlyphOffsetRef)_BaseObject_tnarrow(
        o, _XfGlyphOffset_tid, &GlyphOffsetStub::_create
    );
}
TypeObjId GlyphOffsetType::_tid() { return _XfGlyphOffset_tid; }
void _XfGlyphOffset_receive(BaseObjectRef _object, ULong _m, MarshalBuffer& _b) {
    extern TypeObjId _XfGlyphOffset_tid;
    GlyphOffsetRef _this = (GlyphOffsetRef)_BaseObject_tcast(_object, _XfGlyphOffset_tid);
    switch (_m) {
        case /* parent */ 0: {
            extern MarshalBuffer::ArgInfo _XfGlyphOffset_parent_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_objref = _this->_c_parent();
            _b.reply(_XfGlyphOffset_parent_pinfo, _arg);
            break;
        }
        case /* child */ 1: {
            extern MarshalBuffer::ArgInfo _XfGlyphOffset_child_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_objref = _this->_c_child();
            _b.reply(_XfGlyphOffset_child_pinfo, _arg);
            break;
        }
        case /* allocations */ 2: {
            extern MarshalBuffer::ArgInfo _XfGlyphOffset_allocations_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            Glyph::AllocationInfoList a;
            _arg[1].u_addr = &a;
            _b.receive(_XfGlyphOffset_allocations_pinfo, _arg);
            _this->allocations(a);
            _b.reply(_XfGlyphOffset_allocations_pinfo, _arg);
            break;
        }
        case /* insert */ 3: {
            extern MarshalBuffer::ArgInfo _XfGlyphOffset_insert_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            GlyphRef g;
            _arg[1].u_addr = &g;
            _b.receive(_XfGlyphOffset_insert_pinfo, _arg);
            _arg[0].u_objref = _this->_c_insert(g);
            _b.reply(_XfGlyphOffset_insert_pinfo, _arg);
            break;
        }
        case /* replace */ 4: {
            extern MarshalBuffer::ArgInfo _XfGlyphOffset_replace_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            GlyphRef g;
            _arg[1].u_addr = &g;
            _b.receive(_XfGlyphOffset_replace_pinfo, _arg);
            _this->replace(g);
            _b.reply(_XfGlyphOffset_replace_pinfo, _arg);
            break;
        }
        case /* remove */ 5: {
            extern MarshalBuffer::ArgInfo _XfGlyphOffset_remove_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _this->remove();
            _b.reply(_XfGlyphOffset_remove_pinfo, _arg);
            break;
        }
        case /* notify */ 6: {
            extern MarshalBuffer::ArgInfo _XfGlyphOffset_notify_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _this->notify();
            _b.reply(_XfGlyphOffset_notify_pinfo, _arg);
            break;
        }
        case /* visit_trail */ 7: {
            extern MarshalBuffer::ArgInfo _XfGlyphOffset_visit_trail_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            GlyphTraversalRef t;
            _arg[1].u_addr = &t;
            _b.receive(_XfGlyphOffset_visit_trail_pinfo, _arg);
            _this->visit_trail(t);
            _b.reply(_XfGlyphOffset_visit_trail_pinfo, _arg);
            break;
        }
        case /* child_allocate */ 8: {
            extern MarshalBuffer::ArgInfo _XfGlyphOffset_child_allocate_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            Glyph::AllocationInfo a;
            _arg[1].u_addr = &a;
            _b.receive(_XfGlyphOffset_child_allocate_pinfo, _arg);
            _this->child_allocate(a);
            _b.reply(_XfGlyphOffset_child_allocate_pinfo, _arg);
            break;
        }
    }
}
extern void _XfGlyph_AllocationInfo_put(
    MarshalBuffer&, const Glyph::AllocationInfo&
);
extern void _XfGlyph_AllocationInfo_get(
    MarshalBuffer&, Glyph::AllocationInfo&
);
extern void _XfGlyph_AllocationInfoList_put(
    MarshalBuffer&, const Glyph::AllocationInfoList&
);
extern void _XfGlyph_AllocationInfoList_get(
    MarshalBuffer&, Glyph::AllocationInfoList&
);
extern void _XfGlyph_AllocationInfo_put(
    MarshalBuffer&, const Glyph::AllocationInfo&
);
extern void _XfGlyph_AllocationInfo_get(
    MarshalBuffer&, Glyph::AllocationInfo&
);

GlyphOffsetStub::GlyphOffsetStub(Exchange* e) { exch_ = e; }
GlyphOffsetStub::~GlyphOffsetStub() { }
BaseObjectRef GlyphOffsetStub::_create(Exchange* e) {
    return (BaseObjectRef)(void*)new GlyphOffsetStub(e);
}
Exchange* GlyphOffsetStub::_exchange() {
    return exch_;
}
MarshalBuffer::ArgDesc _XfGlyphOffset_parent_pdesc[2] = { 1, 60 };
MarshalBuffer::ArgMarshal _XfGlyphOffset_parent_pfunc[] = {
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfGlyphOffset_parent_pinfo = {
    &_XfGlyphOffset_tid, 0, _XfGlyphOffset_parent_pdesc, _XfGlyphOffset_parent_pfunc
};
GlyphRef GlyphOffsetType::_c_parent() {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyphOffset_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfGlyphOffset_parent_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfGlyphOffset_child_pdesc[2] = { 1, 60 };
MarshalBuffer::ArgMarshal _XfGlyphOffset_child_pfunc[] = {
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfGlyphOffset_child_pinfo = {
    &_XfGlyphOffset_tid, 1, _XfGlyphOffset_child_pdesc, _XfGlyphOffset_child_pfunc
};
GlyphRef GlyphOffsetType::_c_child() {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyphOffset_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfGlyphOffset_child_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfGlyphOffset_allocations_pdesc[3] = { 2, 4, 2 };
MarshalBuffer::ArgMarshal _XfGlyphOffset_allocations_pfunc[] = {
    &_XfGlyph_AllocationInfoList_put, &_XfGlyph_AllocationInfoList_get,

};
MarshalBuffer::ArgInfo _XfGlyphOffset_allocations_pinfo = {
    &_XfGlyphOffset_tid, 2, _XfGlyphOffset_allocations_pdesc, _XfGlyphOffset_allocations_pfunc
};
void GlyphOffsetType::allocations(Glyph::AllocationInfoList& a) {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyphOffset_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_addr = &a;
    _b.invoke(this, _XfGlyphOffset_allocations_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfGlyphOffset_insert_pdesc[3] = { 2, 60, 61 };
MarshalBuffer::ArgMarshal _XfGlyphOffset_insert_pfunc[] = {
    &GlyphStub::_create,
    &GlyphOffsetStub::_create
};
MarshalBuffer::ArgInfo _XfGlyphOffset_insert_pinfo = {
    &_XfGlyphOffset_tid, 3, _XfGlyphOffset_insert_pdesc, _XfGlyphOffset_insert_pfunc
};
GlyphOffsetRef GlyphOffsetType::_c_insert(Glyph_in g) {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyphOffset_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = g;
    _b.invoke(this, _XfGlyphOffset_insert_pinfo, _arg);
    return (GlyphOffsetRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfGlyphOffset_replace_pdesc[3] = { 2, 4, 61 };
MarshalBuffer::ArgMarshal _XfGlyphOffset_replace_pfunc[] = {
    &GlyphStub::_create,

};
MarshalBuffer::ArgInfo _XfGlyphOffset_replace_pinfo = {
    &_XfGlyphOffset_tid, 4, _XfGlyphOffset_replace_pdesc, _XfGlyphOffset_replace_pfunc
};
void GlyphOffsetType::replace(Glyph_in g) {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyphOffset_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = g;
    _b.invoke(this, _XfGlyphOffset_replace_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfGlyphOffset_remove_pdesc[2] = { 1, 4 };
MarshalBuffer::ArgInfo _XfGlyphOffset_remove_pinfo = {
    &_XfGlyphOffset_tid, 5, _XfGlyphOffset_remove_pdesc, 0
};
void GlyphOffsetType::remove() {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyphOffset_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfGlyphOffset_remove_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfGlyphOffset_notify_pdesc[2] = { 1, 4 };
MarshalBuffer::ArgInfo _XfGlyphOffset_notify_pinfo = {
    &_XfGlyphOffset_tid, 6, _XfGlyphOffset_notify_pdesc, 0
};
void GlyphOffsetType::notify() {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyphOffset_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfGlyphOffset_notify_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfGlyphOffset_visit_trail_pdesc[3] = { 2, 4, 61 };
MarshalBuffer::ArgMarshal _XfGlyphOffset_visit_trail_pfunc[] = {
    &GlyphTraversalStub::_create,

};
MarshalBuffer::ArgInfo _XfGlyphOffset_visit_trail_pinfo = {
    &_XfGlyphOffset_tid, 7, _XfGlyphOffset_visit_trail_pdesc, _XfGlyphOffset_visit_trail_pfunc
};
void GlyphOffsetType::visit_trail(GlyphTraversal_in t) {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyphOffset_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = t;
    _b.invoke(this, _XfGlyphOffset_visit_trail_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfGlyphOffset_child_allocate_pdesc[3] = { 2, 4, 3 };
MarshalBuffer::ArgMarshal _XfGlyphOffset_child_allocate_pfunc[] = {
    &_XfGlyph_AllocationInfo_put, &_XfGlyph_AllocationInfo_get,

};
MarshalBuffer::ArgInfo _XfGlyphOffset_child_allocate_pinfo = {
    &_XfGlyphOffset_tid, 8, _XfGlyphOffset_child_allocate_pdesc, _XfGlyphOffset_child_allocate_pfunc
};
void GlyphOffsetType::child_allocate(Glyph::AllocationInfo& a) {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyphOffset_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_addr = &a;
    _b.invoke(this, _XfGlyphOffset_child_allocate_pinfo, _arg);
}
//+

//+ GlyphTraversal::%init,type+dii,client
GlyphTraversalType::GlyphTraversalType() { }
GlyphTraversalType::~GlyphTraversalType() { }
void* GlyphTraversalType::_this() { return this; }

extern TypeObj_Descriptor _XfGlyphTraversal_Operation_type, _XfViewer_type, 
    _XfPainterObj_type, _XfDisplayObj_type, _XfScreenObj_type, _XfVertex_type, 
    _XfAxis_type, _XfRegion_BoundingSpan_type, _XfDamageObj_type, 
    _XfLong_type;

TypeObj_OpData _XfGlyphTraversal_methods[] = {
    { "op", &_XfGlyphTraversal_Operation_type, 0 },
    { "swap_op", &_XfGlyphTraversal_Operation_type, 1 },
    { "begin_trail", &_Xfvoid_type, 1 },
    { "end_trail", &_Xfvoid_type, 0 },
    { "traverse_child", &_Xfvoid_type, 2 },
    { "visit", &_Xfvoid_type, 0 },
    { "trail", &_XfGlyphTraversal_type, 0 },
    { "current_glyph", &_XfGlyph_type, 0 },
    { "current_offset", &_XfGlyphOffset_type, 0 },
    { "current_viewer", &_XfViewer_type, 0 },
    { "forward", &_XfBoolean_type, 0 },
    { "backward", &_XfBoolean_type, 0 },
    { "_get_painter", &_XfPainterObj_type, 0 },
    { "_set_painter", &_Xfvoid_type, 1 },
    { "display", &_XfDisplayObj_type, 0 },
    { "screen", &_XfScreenObj_type, 0 },
    { "allocation", &_XfRegion_type, 0 },
    { "bounds", &_XfBoolean_type, 3 },
    { "origin", &_XfBoolean_type, 1 },
    { "span", &_XfBoolean_type, 2 },
    { "transform", &_XfTransformObj_type, 0 },
    { "damage", &_XfDamageObj_type, 0 },
    { "hit", &_Xfvoid_type, 0 },
    { "_get_hit_info", &_XfLong_type, 0 },
    { "_set_hit_info", &_Xfvoid_type, 1 },
    { "picked", &_XfGlyphTraversal_type, 0 },
    { "clear", &_Xfvoid_type, 0 },
    { 0, 0, 0 }
};
TypeObj_ParamData _XfGlyphTraversal_params[] = {
    /* swap_op */
        { "op", 0, &_XfGlyphTraversal_Operation_type },
    /* begin_trail */
        { "v", 0, &_XfViewer_type },
    /* traverse_child */
        { "o", 0, &_XfGlyphOffset_type },
        { "allocation", 0, &_XfRegion_type },
    /* painter */
        { "_p", 0, &_XfPainterObj_type },
    /* bounds */
        { "lower", 1, &_XfVertex_type },
        { "upper", 1, &_XfVertex_type },
        { "origin", 1, &_XfVertex_type },
    /* origin */
        { "origin", 1, &_XfVertex_type },
    /* span */
        { "a", 0, &_XfAxis_type },
        { "s", 1, &_XfRegion_BoundingSpan_type },
    /* hit_info */
        { "_p", 0, &_XfLong_type }
};
extern TypeObj_Descriptor _XfFrescoObject_type;
TypeObj_Descriptor* _XfGlyphTraversal_parents[] = { &_XfFrescoObject_type, nil };
extern TypeObjId _XfGlyphTraversal_tid;
extern void _XfGlyphTraversal_receive(BaseObjectRef, ULong, MarshalBuffer&);
TypeObj_Descriptor _XfGlyphTraversal_type = {
    /* type */ 0,
    /* id */ &_XfGlyphTraversal_tid,
    "GlyphTraversal",
    _XfGlyphTraversal_parents, /* offsets */ nil, /* excepts */ nil,
    _XfGlyphTraversal_methods, _XfGlyphTraversal_params,
    &_XfGlyphTraversal_receive
};

GlyphTraversalRef GlyphTraversal::_narrow(BaseObjectRef o) {
    return (GlyphTraversalRef)_BaseObject_tnarrow(
        o, _XfGlyphTraversal_tid, &GlyphTraversalStub::_create
    );
}
TypeObjId GlyphTraversalType::_tid() { return _XfGlyphTraversal_tid; }
void _XfGlyphTraversal_receive(BaseObjectRef _object, ULong _m, MarshalBuffer& _b) {
    extern TypeObjId _XfGlyphTraversal_tid;
    GlyphTraversalRef _this = (GlyphTraversalRef)_BaseObject_tcast(_object, _XfGlyphTraversal_tid);
    switch (_m) {
        case /* op */ 0: {
            extern MarshalBuffer::ArgInfo _XfGlyphTraversal_op_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_long = _this->op();
            _b.reply(_XfGlyphTraversal_op_pinfo, _arg);
            break;
        }
        case /* swap_op */ 1: {
            extern MarshalBuffer::ArgInfo _XfGlyphTraversal_swap_op_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            GlyphTraversal::Operation op;
            _arg[1].u_addr = &op;
            _b.receive(_XfGlyphTraversal_swap_op_pinfo, _arg);
            _arg[0].u_long = _this->swap_op(op);
            _b.reply(_XfGlyphTraversal_swap_op_pinfo, _arg);
            break;
        }
        case /* begin_trail */ 2: {
            extern MarshalBuffer::ArgInfo _XfGlyphTraversal_begin_trail_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            ViewerRef v;
            _arg[1].u_addr = &v;
            _b.receive(_XfGlyphTraversal_begin_trail_pinfo, _arg);
            _this->begin_trail(v);
            _b.reply(_XfGlyphTraversal_begin_trail_pinfo, _arg);
            break;
        }
        case /* end_trail */ 3: {
            extern MarshalBuffer::ArgInfo _XfGlyphTraversal_end_trail_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _this->end_trail();
            _b.reply(_XfGlyphTraversal_end_trail_pinfo, _arg);
            break;
        }
        case /* traverse_child */ 4: {
            extern MarshalBuffer::ArgInfo _XfGlyphTraversal_traverse_child_pinfo;
            MarshalBuffer::ArgValue _arg[3];
            GlyphOffsetRef o;
            _arg[1].u_addr = &o;
            RegionRef allocation;
            _arg[2].u_addr = &allocation;
            _b.receive(_XfGlyphTraversal_traverse_child_pinfo, _arg);
            _this->traverse_child(o, allocation);
            _b.reply(_XfGlyphTraversal_traverse_child_pinfo, _arg);
            break;
        }
        case /* visit */ 5: {
            extern MarshalBuffer::ArgInfo _XfGlyphTraversal_visit_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _this->visit();
            _b.reply(_XfGlyphTraversal_visit_pinfo, _arg);
            break;
        }
        case /* trail */ 6: {
            extern MarshalBuffer::ArgInfo _XfGlyphTraversal_trail_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_objref = _this->_c_trail();
            _b.reply(_XfGlyphTraversal_trail_pinfo, _arg);
            break;
        }
        case /* current_glyph */ 7: {
            extern MarshalBuffer::ArgInfo _XfGlyphTraversal_current_glyph_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_objref = _this->_c_current_glyph();
            _b.reply(_XfGlyphTraversal_current_glyph_pinfo, _arg);
            break;
        }
        case /* current_offset */ 8: {
            extern MarshalBuffer::ArgInfo _XfGlyphTraversal_current_offset_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_objref = _this->_c_current_offset();
            _b.reply(_XfGlyphTraversal_current_offset_pinfo, _arg);
            break;
        }
        case /* current_viewer */ 9: {
            extern MarshalBuffer::ArgInfo _XfGlyphTraversal_current_viewer_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_objref = _this->_c_current_viewer();
            _b.reply(_XfGlyphTraversal_current_viewer_pinfo, _arg);
            break;
        }
        case /* forward */ 10: {
            extern MarshalBuffer::ArgInfo _XfGlyphTraversal_forward_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_boolean = _this->forward();
            _b.reply(_XfGlyphTraversal_forward_pinfo, _arg);
            break;
        }
        case /* backward */ 11: {
            extern MarshalBuffer::ArgInfo _XfGlyphTraversal_backward_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_boolean = _this->backward();
            _b.reply(_XfGlyphTraversal_backward_pinfo, _arg);
            break;
        }
        case /* _get_painter */ 12: {
            extern MarshalBuffer::ArgInfo _XfGlyphTraversal__get_painter_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_objref = _this->_c_painter();
            _b.reply(_XfGlyphTraversal__get_painter_pinfo, _arg);
            break;
        }
        case /* _set_painter */ 13: {
            extern MarshalBuffer::ArgInfo _XfGlyphTraversal__set_painter_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            PainterObjRef _p;
            _arg[1].u_addr = &_p;
            _b.receive(_XfGlyphTraversal__set_painter_pinfo, _arg);
            _this->_c_painter(_p);
            _b.reply(_XfGlyphTraversal__set_painter_pinfo, _arg);
            break;
        }
        case /* display */ 14: {
            extern MarshalBuffer::ArgInfo _XfGlyphTraversal_display_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_objref = _this->_c_display();
            _b.reply(_XfGlyphTraversal_display_pinfo, _arg);
            break;
        }
        case /* screen */ 15: {
            extern MarshalBuffer::ArgInfo _XfGlyphTraversal_screen_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_objref = _this->_c_screen();
            _b.reply(_XfGlyphTraversal_screen_pinfo, _arg);
            break;
        }
        case /* allocation */ 16: {
            extern MarshalBuffer::ArgInfo _XfGlyphTraversal_allocation_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_objref = _this->_c_allocation();
            _b.reply(_XfGlyphTraversal_allocation_pinfo, _arg);
            break;
        }
        case /* bounds */ 17: {
            extern MarshalBuffer::ArgInfo _XfGlyphTraversal_bounds_pinfo;
            MarshalBuffer::ArgValue _arg[4];
            Vertex lower;
            _arg[1].u_addr = &lower;
            Vertex upper;
            _arg[2].u_addr = &upper;
            Vertex origin;
            _arg[3].u_addr = &origin;
            _b.receive(_XfGlyphTraversal_bounds_pinfo, _arg);
            _arg[0].u_boolean = _this->bounds(lower, upper, origin);
            _b.reply(_XfGlyphTraversal_bounds_pinfo, _arg);
            break;
        }
        case /* origin */ 18: {
            extern MarshalBuffer::ArgInfo _XfGlyphTraversal_origin_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            Vertex origin;
            _arg[1].u_addr = &origin;
            _b.receive(_XfGlyphTraversal_origin_pinfo, _arg);
            _arg[0].u_boolean = _this->origin(origin);
            _b.reply(_XfGlyphTraversal_origin_pinfo, _arg);
            break;
        }
        case /* span */ 19: {
            extern MarshalBuffer::ArgInfo _XfGlyphTraversal_span_pinfo;
            MarshalBuffer::ArgValue _arg[3];
            Axis a;
            _arg[1].u_addr = &a;
            Region::BoundingSpan s;
            _arg[2].u_addr = &s;
            _b.receive(_XfGlyphTraversal_span_pinfo, _arg);
            _arg[0].u_boolean = _this->span(a, s);
            _b.reply(_XfGlyphTraversal_span_pinfo, _arg);
            break;
        }
        case /* transform */ 20: {
            extern MarshalBuffer::ArgInfo _XfGlyphTraversal_transform_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_objref = _this->_c_transform();
            _b.reply(_XfGlyphTraversal_transform_pinfo, _arg);
            break;
        }
        case /* damage */ 21: {
            extern MarshalBuffer::ArgInfo _XfGlyphTraversal_damage_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_objref = _this->_c_damage();
            _b.reply(_XfGlyphTraversal_damage_pinfo, _arg);
            break;
        }
        case /* hit */ 22: {
            extern MarshalBuffer::ArgInfo _XfGlyphTraversal_hit_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _this->hit();
            _b.reply(_XfGlyphTraversal_hit_pinfo, _arg);
            break;
        }
        case /* _get_hit_info */ 23: {
            extern MarshalBuffer::ArgInfo _XfGlyphTraversal__get_hit_info_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_long = _this->hit_info();
            _b.reply(_XfGlyphTraversal__get_hit_info_pinfo, _arg);
            break;
        }
        case /* _set_hit_info */ 24: {
            extern MarshalBuffer::ArgInfo _XfGlyphTraversal__set_hit_info_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            Long _p;
            _arg[1].u_addr = &_p;
            _b.receive(_XfGlyphTraversal__set_hit_info_pinfo, _arg);
            _this->hit_info(_p);
            _b.reply(_XfGlyphTraversal__set_hit_info_pinfo, _arg);
            break;
        }
        case /* picked */ 25: {
            extern MarshalBuffer::ArgInfo _XfGlyphTraversal_picked_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_objref = _this->_c_picked();
            _b.reply(_XfGlyphTraversal_picked_pinfo, _arg);
            break;
        }
        case /* clear */ 26: {
            extern MarshalBuffer::ArgInfo _XfGlyphTraversal_clear_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _this->clear();
            _b.reply(_XfGlyphTraversal_clear_pinfo, _arg);
            break;
        }
    }
}
extern void _XfVertex_put(
    MarshalBuffer&, const Vertex&
);
extern void _XfVertex_get(
    MarshalBuffer&, Vertex&
);
extern void _XfVertex_put(
    MarshalBuffer&, const Vertex&
);
extern void _XfVertex_get(
    MarshalBuffer&, Vertex&
);
extern void _XfVertex_put(
    MarshalBuffer&, const Vertex&
);
extern void _XfVertex_get(
    MarshalBuffer&, Vertex&
);
extern void _XfVertex_put(
    MarshalBuffer&, const Vertex&
);
extern void _XfVertex_get(
    MarshalBuffer&, Vertex&
);
extern void _XfRegion_BoundingSpan_put(
    MarshalBuffer&, const Region::BoundingSpan&
);
extern void _XfRegion_BoundingSpan_get(
    MarshalBuffer&, Region::BoundingSpan&
);

GlyphTraversalStub::GlyphTraversalStub(Exchange* e) { exch_ = e; }
GlyphTraversalStub::~GlyphTraversalStub() { }
BaseObjectRef GlyphTraversalStub::_create(Exchange* e) {
    return (BaseObjectRef)(void*)new GlyphTraversalStub(e);
}
Exchange* GlyphTraversalStub::_exchange() {
    return exch_;
}
MarshalBuffer::ArgDesc _XfGlyphTraversal_op_pdesc[2] = { 1, 32 };
MarshalBuffer::ArgInfo _XfGlyphTraversal_op_pinfo = {
    &_XfGlyphTraversal_tid, 0, _XfGlyphTraversal_op_pdesc, 0
};
GlyphTraversal::Operation GlyphTraversalType::op() {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyphTraversal_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfGlyphTraversal_op_pinfo, _arg);
    return (GlyphTraversal::Operation)_arg[0].u_long;
}
MarshalBuffer::ArgDesc _XfGlyphTraversal_swap_op_pdesc[3] = { 2, 32, 33 };
MarshalBuffer::ArgInfo _XfGlyphTraversal_swap_op_pinfo = {
    &_XfGlyphTraversal_tid, 1, _XfGlyphTraversal_swap_op_pdesc, 0
};
GlyphTraversal::Operation GlyphTraversalType::swap_op(GlyphTraversal::Operation op) {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyphTraversal_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_long = op;
    _b.invoke(this, _XfGlyphTraversal_swap_op_pinfo, _arg);
    return (GlyphTraversal::Operation)_arg[0].u_long;
}
MarshalBuffer::ArgDesc _XfGlyphTraversal_begin_trail_pdesc[3] = { 2, 4, 61 };
MarshalBuffer::ArgMarshal _XfGlyphTraversal_begin_trail_pfunc[] = {
    &ViewerStub::_create,

};
MarshalBuffer::ArgInfo _XfGlyphTraversal_begin_trail_pinfo = {
    &_XfGlyphTraversal_tid, 2, _XfGlyphTraversal_begin_trail_pdesc, _XfGlyphTraversal_begin_trail_pfunc
};
void GlyphTraversalType::begin_trail(Viewer_in v) {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyphTraversal_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = v;
    _b.invoke(this, _XfGlyphTraversal_begin_trail_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfGlyphTraversal_end_trail_pdesc[2] = { 1, 4 };
MarshalBuffer::ArgInfo _XfGlyphTraversal_end_trail_pinfo = {
    &_XfGlyphTraversal_tid, 3, _XfGlyphTraversal_end_trail_pdesc, 0
};
void GlyphTraversalType::end_trail() {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyphTraversal_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfGlyphTraversal_end_trail_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfGlyphTraversal_traverse_child_pdesc[4] = { 3, 4, 61, 61 };
MarshalBuffer::ArgMarshal _XfGlyphTraversal_traverse_child_pfunc[] = {
    &GlyphOffsetStub::_create,
    &RegionStub::_create,

};
MarshalBuffer::ArgInfo _XfGlyphTraversal_traverse_child_pinfo = {
    &_XfGlyphTraversal_tid, 4, _XfGlyphTraversal_traverse_child_pdesc, _XfGlyphTraversal_traverse_child_pfunc
};
void GlyphTraversalType::traverse_child(GlyphOffset_in o, Region_in allocation) {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyphTraversal_tid;
    MarshalBuffer::ArgValue _arg[3];
    _arg[1].u_objref = o;
    _arg[2].u_objref = allocation;
    _b.invoke(this, _XfGlyphTraversal_traverse_child_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfGlyphTraversal_visit_pdesc[2] = { 1, 4 };
MarshalBuffer::ArgInfo _XfGlyphTraversal_visit_pinfo = {
    &_XfGlyphTraversal_tid, 5, _XfGlyphTraversal_visit_pdesc, 0
};
void GlyphTraversalType::visit() {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyphTraversal_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfGlyphTraversal_visit_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfGlyphTraversal_trail_pdesc[2] = { 1, 60 };
MarshalBuffer::ArgMarshal _XfGlyphTraversal_trail_pfunc[] = {
    &GlyphTraversalStub::_create
};
MarshalBuffer::ArgInfo _XfGlyphTraversal_trail_pinfo = {
    &_XfGlyphTraversal_tid, 6, _XfGlyphTraversal_trail_pdesc, _XfGlyphTraversal_trail_pfunc
};
GlyphTraversalRef GlyphTraversalType::_c_trail() {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyphTraversal_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfGlyphTraversal_trail_pinfo, _arg);
    return (GlyphTraversalRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfGlyphTraversal_current_glyph_pdesc[2] = { 1, 60 };
MarshalBuffer::ArgMarshal _XfGlyphTraversal_current_glyph_pfunc[] = {
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfGlyphTraversal_current_glyph_pinfo = {
    &_XfGlyphTraversal_tid, 7, _XfGlyphTraversal_current_glyph_pdesc, _XfGlyphTraversal_current_glyph_pfunc
};
GlyphRef GlyphTraversalType::_c_current_glyph() {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyphTraversal_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfGlyphTraversal_current_glyph_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfGlyphTraversal_current_offset_pdesc[2] = { 1, 60 };
MarshalBuffer::ArgMarshal _XfGlyphTraversal_current_offset_pfunc[] = {
    &GlyphOffsetStub::_create
};
MarshalBuffer::ArgInfo _XfGlyphTraversal_current_offset_pinfo = {
    &_XfGlyphTraversal_tid, 8, _XfGlyphTraversal_current_offset_pdesc, _XfGlyphTraversal_current_offset_pfunc
};
GlyphOffsetRef GlyphTraversalType::_c_current_offset() {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyphTraversal_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfGlyphTraversal_current_offset_pinfo, _arg);
    return (GlyphOffsetRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfGlyphTraversal_current_viewer_pdesc[2] = { 1, 60 };
MarshalBuffer::ArgMarshal _XfGlyphTraversal_current_viewer_pfunc[] = {
    &ViewerStub::_create
};
MarshalBuffer::ArgInfo _XfGlyphTraversal_current_viewer_pinfo = {
    &_XfGlyphTraversal_tid, 9, _XfGlyphTraversal_current_viewer_pdesc, _XfGlyphTraversal_current_viewer_pfunc
};
Viewer_tmp GlyphTraversalType::current_viewer() {
    return _c_current_viewer();
}
ViewerRef GlyphTraversalType::_c_current_viewer() {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyphTraversal_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfGlyphTraversal_current_viewer_pinfo, _arg);
    return (ViewerRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfGlyphTraversal_forward_pdesc[2] = { 1, 12 };
MarshalBuffer::ArgInfo _XfGlyphTraversal_forward_pinfo = {
    &_XfGlyphTraversal_tid, 10, _XfGlyphTraversal_forward_pdesc, 0
};
Boolean GlyphTraversalType::forward() {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyphTraversal_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfGlyphTraversal_forward_pinfo, _arg);
    return _arg[0].u_boolean;
}
MarshalBuffer::ArgDesc _XfGlyphTraversal_backward_pdesc[2] = { 1, 12 };
MarshalBuffer::ArgInfo _XfGlyphTraversal_backward_pinfo = {
    &_XfGlyphTraversal_tid, 11, _XfGlyphTraversal_backward_pdesc, 0
};
Boolean GlyphTraversalType::backward() {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyphTraversal_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfGlyphTraversal_backward_pinfo, _arg);
    return _arg[0].u_boolean;
}
MarshalBuffer::ArgDesc _XfGlyphTraversal__get_painter_pdesc[2] = { 1, 60 };
MarshalBuffer::ArgMarshal _XfGlyphTraversal__get_painter_pfunc[] = {
    &PainterObjStub::_create
};
MarshalBuffer::ArgInfo _XfGlyphTraversal__get_painter_pinfo = {
    &_XfGlyphTraversal_tid, 12, _XfGlyphTraversal__get_painter_pdesc, _XfGlyphTraversal__get_painter_pfunc
};
PainterObjRef GlyphTraversalType::_c_painter() {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyphTraversal_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfGlyphTraversal__get_painter_pinfo, _arg);
    return (PainterObjRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfGlyphTraversal__set_painter_pdesc[3] = { 2, 4, 61 };
MarshalBuffer::ArgMarshal _XfGlyphTraversal__set_painter_pfunc[] = {
    &PainterObjStub::_create,

};
MarshalBuffer::ArgInfo _XfGlyphTraversal__set_painter_pinfo = {
    &_XfGlyphTraversal_tid, 13, _XfGlyphTraversal__set_painter_pdesc, _XfGlyphTraversal__set_painter_pfunc
};
void GlyphTraversalType::_c_painter(PainterObj_in _p) {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyphTraversal_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = _p;
    _b.invoke(this, _XfGlyphTraversal__set_painter_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfGlyphTraversal_display_pdesc[2] = { 1, 60 };
MarshalBuffer::ArgMarshal _XfGlyphTraversal_display_pfunc[] = {
    &DisplayObjStub::_create
};
MarshalBuffer::ArgInfo _XfGlyphTraversal_display_pinfo = {
    &_XfGlyphTraversal_tid, 14, _XfGlyphTraversal_display_pdesc, _XfGlyphTraversal_display_pfunc
};
DisplayObjRef GlyphTraversalType::_c_display() {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyphTraversal_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfGlyphTraversal_display_pinfo, _arg);
    return (DisplayObjRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfGlyphTraversal_screen_pdesc[2] = { 1, 60 };
MarshalBuffer::ArgMarshal _XfGlyphTraversal_screen_pfunc[] = {
    &ScreenObjStub::_create
};
MarshalBuffer::ArgInfo _XfGlyphTraversal_screen_pinfo = {
    &_XfGlyphTraversal_tid, 15, _XfGlyphTraversal_screen_pdesc, _XfGlyphTraversal_screen_pfunc
};
ScreenObjRef GlyphTraversalType::_c_screen() {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyphTraversal_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfGlyphTraversal_screen_pinfo, _arg);
    return (ScreenObjRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfGlyphTraversal_allocation_pdesc[2] = { 1, 60 };
MarshalBuffer::ArgMarshal _XfGlyphTraversal_allocation_pfunc[] = {
    &RegionStub::_create
};
MarshalBuffer::ArgInfo _XfGlyphTraversal_allocation_pinfo = {
    &_XfGlyphTraversal_tid, 16, _XfGlyphTraversal_allocation_pdesc, _XfGlyphTraversal_allocation_pfunc
};
RegionRef GlyphTraversalType::_c_allocation() {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyphTraversal_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfGlyphTraversal_allocation_pinfo, _arg);
    return (RegionRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfGlyphTraversal_bounds_pdesc[5] = { 4, 12, 2, 2, 2 };
MarshalBuffer::ArgMarshal _XfGlyphTraversal_bounds_pfunc[] = {
    &_XfVertex_put, &_XfVertex_get,
    &_XfVertex_put, &_XfVertex_get,
    &_XfVertex_put, &_XfVertex_get,

};
MarshalBuffer::ArgInfo _XfGlyphTraversal_bounds_pinfo = {
    &_XfGlyphTraversal_tid, 17, _XfGlyphTraversal_bounds_pdesc, _XfGlyphTraversal_bounds_pfunc
};
Boolean GlyphTraversalType::bounds(Vertex& lower, Vertex& upper, Vertex& origin) {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyphTraversal_tid;
    MarshalBuffer::ArgValue _arg[4];
    _arg[1].u_addr = &lower;
    _arg[2].u_addr = &upper;
    _arg[3].u_addr = &origin;
    _b.invoke(this, _XfGlyphTraversal_bounds_pinfo, _arg);
    return _arg[0].u_boolean;
}
MarshalBuffer::ArgDesc _XfGlyphTraversal_origin_pdesc[3] = { 2, 12, 2 };
MarshalBuffer::ArgMarshal _XfGlyphTraversal_origin_pfunc[] = {
    &_XfVertex_put, &_XfVertex_get,

};
MarshalBuffer::ArgInfo _XfGlyphTraversal_origin_pinfo = {
    &_XfGlyphTraversal_tid, 18, _XfGlyphTraversal_origin_pdesc, _XfGlyphTraversal_origin_pfunc
};
Boolean GlyphTraversalType::origin(Vertex& origin) {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyphTraversal_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_addr = &origin;
    _b.invoke(this, _XfGlyphTraversal_origin_pinfo, _arg);
    return _arg[0].u_boolean;
}
MarshalBuffer::ArgDesc _XfGlyphTraversal_span_pdesc[4] = { 3, 12, 33, 2 };
MarshalBuffer::ArgMarshal _XfGlyphTraversal_span_pfunc[] = {
    &_XfRegion_BoundingSpan_put, &_XfRegion_BoundingSpan_get,

};
MarshalBuffer::ArgInfo _XfGlyphTraversal_span_pinfo = {
    &_XfGlyphTraversal_tid, 19, _XfGlyphTraversal_span_pdesc, _XfGlyphTraversal_span_pfunc
};
Boolean GlyphTraversalType::span(Axis a, Region::BoundingSpan& s) {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyphTraversal_tid;
    MarshalBuffer::ArgValue _arg[3];
    _arg[1].u_long = a;
    _arg[2].u_addr = &s;
    _b.invoke(this, _XfGlyphTraversal_span_pinfo, _arg);
    return _arg[0].u_boolean;
}
MarshalBuffer::ArgDesc _XfGlyphTraversal_transform_pdesc[2] = { 1, 60 };
MarshalBuffer::ArgMarshal _XfGlyphTraversal_transform_pfunc[] = {
    &TransformObjStub::_create
};
MarshalBuffer::ArgInfo _XfGlyphTraversal_transform_pinfo = {
    &_XfGlyphTraversal_tid, 20, _XfGlyphTraversal_transform_pdesc, _XfGlyphTraversal_transform_pfunc
};
TransformObjRef GlyphTraversalType::_c_transform() {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyphTraversal_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfGlyphTraversal_transform_pinfo, _arg);
    return (TransformObjRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfGlyphTraversal_damage_pdesc[2] = { 1, 60 };
MarshalBuffer::ArgMarshal _XfGlyphTraversal_damage_pfunc[] = {
    &DamageObjStub::_create
};
MarshalBuffer::ArgInfo _XfGlyphTraversal_damage_pinfo = {
    &_XfGlyphTraversal_tid, 21, _XfGlyphTraversal_damage_pdesc, _XfGlyphTraversal_damage_pfunc
};
DamageObjRef GlyphTraversalType::_c_damage() {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyphTraversal_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfGlyphTraversal_damage_pinfo, _arg);
    return (DamageObjRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfGlyphTraversal_hit_pdesc[2] = { 1, 4 };
MarshalBuffer::ArgInfo _XfGlyphTraversal_hit_pinfo = {
    &_XfGlyphTraversal_tid, 22, _XfGlyphTraversal_hit_pdesc, 0
};
void GlyphTraversalType::hit() {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyphTraversal_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfGlyphTraversal_hit_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfGlyphTraversal__get_hit_info_pdesc[2] = { 1, 32 };
MarshalBuffer::ArgInfo _XfGlyphTraversal__get_hit_info_pinfo = {
    &_XfGlyphTraversal_tid, 23, _XfGlyphTraversal__get_hit_info_pdesc, 0
};
Long GlyphTraversalType::hit_info() {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyphTraversal_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfGlyphTraversal__get_hit_info_pinfo, _arg);
    return _arg[0].u_long;
}
MarshalBuffer::ArgDesc _XfGlyphTraversal__set_hit_info_pdesc[3] = { 2, 4, 33 };
MarshalBuffer::ArgInfo _XfGlyphTraversal__set_hit_info_pinfo = {
    &_XfGlyphTraversal_tid, 24, _XfGlyphTraversal__set_hit_info_pdesc, 0
};
void GlyphTraversalType::hit_info(Long _p) {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyphTraversal_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_long = _p;
    _b.invoke(this, _XfGlyphTraversal__set_hit_info_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfGlyphTraversal_picked_pdesc[2] = { 1, 60 };
MarshalBuffer::ArgMarshal _XfGlyphTraversal_picked_pfunc[] = {
    &GlyphTraversalStub::_create
};
MarshalBuffer::ArgInfo _XfGlyphTraversal_picked_pinfo = {
    &_XfGlyphTraversal_tid, 25, _XfGlyphTraversal_picked_pdesc, _XfGlyphTraversal_picked_pfunc
};
GlyphTraversalRef GlyphTraversalType::_c_picked() {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyphTraversal_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfGlyphTraversal_picked_pinfo, _arg);
    return (GlyphTraversalRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfGlyphTraversal_clear_pdesc[2] = { 1, 4 };
MarshalBuffer::ArgInfo _XfGlyphTraversal_clear_pinfo = {
    &_XfGlyphTraversal_tid, 26, _XfGlyphTraversal_clear_pdesc, 0
};
void GlyphTraversalType::clear() {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyphTraversal_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfGlyphTraversal_clear_pinfo, _arg);
}
//+

//+ GlyphVisitor::%init,type+dii,client
GlyphVisitorType::GlyphVisitorType() { }
GlyphVisitorType::~GlyphVisitorType() { }
void* GlyphVisitorType::_this() { return this; }

extern TypeObj_Descriptor _Xfvoid_type;

TypeObj_OpData _XfGlyphVisitor_methods[] = {
    { "visit", &_XfBoolean_type, 2 },
    { 0, 0, 0 }
};
TypeObj_ParamData _XfGlyphVisitor_params[] = {
    /* visit */
        { "glyph", 0, &_XfGlyph_type },
        { "offset", 0, &_XfGlyphOffset_type }
};
extern TypeObj_Descriptor _XfFrescoObject_type;
TypeObj_Descriptor* _XfGlyphVisitor_parents[] = { &_XfFrescoObject_type, nil };
extern TypeObjId _XfGlyphVisitor_tid;
extern void _XfGlyphVisitor_receive(BaseObjectRef, ULong, MarshalBuffer&);
TypeObj_Descriptor _XfGlyphVisitor_type = {
    /* type */ 0,
    /* id */ &_XfGlyphVisitor_tid,
    "GlyphVisitor",
    _XfGlyphVisitor_parents, /* offsets */ nil, /* excepts */ nil,
    _XfGlyphVisitor_methods, _XfGlyphVisitor_params,
    &_XfGlyphVisitor_receive
};

GlyphVisitorRef GlyphVisitor::_narrow(BaseObjectRef o) {
    return (GlyphVisitorRef)_BaseObject_tnarrow(
        o, _XfGlyphVisitor_tid, &GlyphVisitorStub::_create
    );
}
TypeObjId GlyphVisitorType::_tid() { return _XfGlyphVisitor_tid; }
void _XfGlyphVisitor_receive(BaseObjectRef _object, ULong _m, MarshalBuffer& _b) {
    extern TypeObjId _XfGlyphVisitor_tid;
    GlyphVisitorRef _this = (GlyphVisitorRef)_BaseObject_tcast(_object, _XfGlyphVisitor_tid);
    switch (_m) {
        case /* visit */ 0: {
            extern MarshalBuffer::ArgInfo _XfGlyphVisitor_visit_pinfo;
            MarshalBuffer::ArgValue _arg[3];
            GlyphRef glyph;
            _arg[1].u_addr = &glyph;
            GlyphOffsetRef offset;
            _arg[2].u_addr = &offset;
            _b.receive(_XfGlyphVisitor_visit_pinfo, _arg);
            _arg[0].u_boolean = _this->visit(glyph, offset);
            _b.reply(_XfGlyphVisitor_visit_pinfo, _arg);
            break;
        }
    }
}
GlyphVisitorStub::GlyphVisitorStub(Exchange* e) { exch_ = e; }
GlyphVisitorStub::~GlyphVisitorStub() { }
BaseObjectRef GlyphVisitorStub::_create(Exchange* e) {
    return (BaseObjectRef)(void*)new GlyphVisitorStub(e);
}
Exchange* GlyphVisitorStub::_exchange() {
    return exch_;
}
MarshalBuffer::ArgDesc _XfGlyphVisitor_visit_pdesc[4] = { 3, 12, 61, 61 };
MarshalBuffer::ArgMarshal _XfGlyphVisitor_visit_pfunc[] = {
    &GlyphStub::_create,
    &GlyphOffsetStub::_create,

};
MarshalBuffer::ArgInfo _XfGlyphVisitor_visit_pinfo = {
    &_XfGlyphVisitor_tid, 0, _XfGlyphVisitor_visit_pdesc, _XfGlyphVisitor_visit_pfunc
};
Boolean GlyphVisitorType::visit(Glyph_in glyph, GlyphOffset_in offset) {
    MarshalBuffer _b;
    extern TypeObjId _XfGlyphVisitor_tid;
    MarshalBuffer::ArgValue _arg[3];
    _arg[1].u_objref = glyph;
    _arg[2].u_objref = offset;
    _b.invoke(this, _XfGlyphVisitor_visit_pinfo, _arg);
    return _arg[0].u_boolean;
}
//+
