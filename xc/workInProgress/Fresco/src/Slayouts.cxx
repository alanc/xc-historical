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
 * Stubs for LayoutKit
 */

#include <X11/Fresco/layouts.h>
#include <X11/Fresco/Ox/request.h>
#include <X11/Fresco/Ox/stub.h>
#include <X11/Fresco/Ox/schema.h>

//+ DeckObj::%init,type+dii,client
DeckObjType::DeckObjType() { }
DeckObjType::~DeckObjType() { }
void* DeckObjType::_this() { return this; }

extern TypeObj_Descriptor _XfGlyphOffset_type, _Xfvoid_type;

TypeObj_OpData _XfDeckObj_methods[] = {
    { "card", &_XfGlyphOffset_type, 0 },
    { "flip_to", &_Xfvoid_type, 1 },
    { 0, 0, 0 }
};
TypeObj_ParamData _XfDeckObj_params[] = {
    /* flip_to */
        { "off", 0, &_XfGlyphOffset_type }
};
extern TypeObj_Descriptor _XfGlyph_type;
TypeObj_Descriptor* _XfDeckObj_parents[] = { &_XfGlyph_type, nil };
extern TypeObjId _XfDeckObj_tid;
extern void _XfDeckObj_receive(BaseObjectRef, ULong, MarshalBuffer&);
TypeObj_Descriptor _XfDeckObj_type = {
    /* type */ 0,
    /* id */ &_XfDeckObj_tid,
    "DeckObj",
    _XfDeckObj_parents, /* offsets */ nil, /* excepts */ nil,
    _XfDeckObj_methods, _XfDeckObj_params,
    &_XfDeckObj_receive
};

DeckObjRef DeckObj::_narrow(BaseObjectRef o) {
    return (DeckObjRef)_BaseObject_tnarrow(
        o, _XfDeckObj_tid, &DeckObjStub::_create
    );
}
TypeObjId DeckObjType::_tid() { return _XfDeckObj_tid; }
void _XfDeckObj_receive(BaseObjectRef _object, ULong _m, MarshalBuffer& _b) {
    extern TypeObjId _XfDeckObj_tid;
    DeckObjRef _this = (DeckObjRef)_BaseObject_tcast(_object, _XfDeckObj_tid);
    switch (_m) {
        case /* card */ 0: {
            extern MarshalBuffer::ArgInfo _XfDeckObj_card_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_objref = _this->_c_card();
            _b.reply(_XfDeckObj_card_pinfo, _arg);
            break;
        }
        case /* flip_to */ 1: {
            extern MarshalBuffer::ArgInfo _XfDeckObj_flip_to_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            GlyphOffsetRef off;
            _arg[1].u_addr = &off;
            _b.receive(_XfDeckObj_flip_to_pinfo, _arg);
            _this->flip_to(off);
            _b.reply(_XfDeckObj_flip_to_pinfo, _arg);
            break;
        }
    }
}
DeckObjStub::DeckObjStub(Exchange* e) { exch_ = e; }
DeckObjStub::~DeckObjStub() { }
BaseObjectRef DeckObjStub::_create(Exchange* e) {
    return (BaseObjectRef)(void*)new DeckObjStub(e);
}
Exchange* DeckObjStub::_exchange() {
    return exch_;
}
MarshalBuffer::ArgDesc _XfDeckObj_card_pdesc[2] = { 1, 60 };
MarshalBuffer::ArgMarshal _XfDeckObj_card_pfunc[] = {
    &GlyphOffsetStub::_create
};
MarshalBuffer::ArgInfo _XfDeckObj_card_pinfo = {
    &_XfDeckObj_tid, 0, _XfDeckObj_card_pdesc, _XfDeckObj_card_pfunc
};
GlyphOffsetRef DeckObjType::_c_card() {
    MarshalBuffer _b;
    extern TypeObjId _XfDeckObj_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfDeckObj_card_pinfo, _arg);
    return (GlyphOffsetRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfDeckObj_flip_to_pdesc[3] = { 2, 4, 61 };
MarshalBuffer::ArgMarshal _XfDeckObj_flip_to_pfunc[] = {
    &GlyphOffsetStub::_create,

};
MarshalBuffer::ArgInfo _XfDeckObj_flip_to_pinfo = {
    &_XfDeckObj_tid, 1, _XfDeckObj_flip_to_pdesc, _XfDeckObj_flip_to_pfunc
};
void DeckObjType::flip_to(GlyphOffset_in off) {
    MarshalBuffer _b;
    extern TypeObjId _XfDeckObj_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = off;
    _b.invoke(this, _XfDeckObj_flip_to_pinfo, _arg);
}
//+

//+ LayoutKit::%init,type+dii,client
LayoutKitType::LayoutKitType() { }
LayoutKitType::~LayoutKitType() { }
void* LayoutKitType::_this() { return this; }

extern TypeObj_Descriptor _XfCoord_type, _XfGlyph_type, _XfScrollBox_type, 
    _XfDeckObj_type, _XfAxis_type, _XfAlignment_type, _XfGlyph_Requisition_type, 
    _XfFont_type, _XfLong_type, _XfColor_type;

TypeObj_OpData _XfLayoutKit_methods[] = {
    { "_get_fil", &_XfCoord_type, 0 },
    { "_set_fil", &_Xfvoid_type, 1 },
    { "hbox", &_XfGlyph_type, 0 },
    { "vbox", &_XfGlyph_type, 0 },
    { "hbox_first_aligned", &_XfGlyph_type, 0 },
    { "vbox_first_aligned", &_XfGlyph_type, 0 },
    { "vscrollbox", &_XfScrollBox_type, 0 },
    { "overlay", &_XfGlyph_type, 0 },
    { "deck", &_XfDeckObj_type, 0 },
    { "back", &_XfGlyph_type, 2 },
    { "front", &_XfGlyph_type, 2 },
    { "between", &_XfGlyph_type, 3 },
    { "glue", &_XfGlyph_type, 5 },
    { "glue_requisition", &_XfGlyph_type, 1 },
    { "hfil", &_XfGlyph_type, 0 },
    { "hglue_fil", &_XfGlyph_type, 1 },
    { "hglue", &_XfGlyph_type, 3 },
    { "hglue_aligned", &_XfGlyph_type, 4 },
    { "hspace", &_XfGlyph_type, 1 },
    { "vfil", &_XfGlyph_type, 0 },
    { "vglue_fil", &_XfGlyph_type, 1 },
    { "vglue", &_XfGlyph_type, 3 },
    { "vglue_aligned", &_XfGlyph_type, 4 },
    { "vspace", &_XfGlyph_type, 1 },
    { "shape_of", &_XfGlyph_type, 1 },
    { "shape_of_xy", &_XfGlyph_type, 2 },
    { "shape_of_xyz", &_XfGlyph_type, 3 },
    { "strut", &_XfGlyph_type, 4 },
    { "hstrut", &_XfGlyph_type, 5 },
    { "vstrut", &_XfGlyph_type, 5 },
    { "spaces", &_XfGlyph_type, 4 },
    { "center", &_XfGlyph_type, 1 },
    { "center_aligned", &_XfGlyph_type, 3 },
    { "center_axis", &_XfGlyph_type, 3 },
    { "hcenter", &_XfGlyph_type, 1 },
    { "hcenter_aligned", &_XfGlyph_type, 2 },
    { "vcenter", &_XfGlyph_type, 1 },
    { "vcenter_aligned", &_XfGlyph_type, 2 },
    { "fixed", &_XfGlyph_type, 3 },
    { "fixed_axis", &_XfGlyph_type, 3 },
    { "hfixed", &_XfGlyph_type, 2 },
    { "vfixed", &_XfGlyph_type, 2 },
    { "flexible", &_XfGlyph_type, 3 },
    { "flexible_fil", &_XfGlyph_type, 1 },
    { "flexible_axis", &_XfGlyph_type, 4 },
    { "hflexible", &_XfGlyph_type, 3 },
    { "vflexible", &_XfGlyph_type, 3 },
    { "natural", &_XfGlyph_type, 3 },
    { "natural_axis", &_XfGlyph_type, 3 },
    { "hnatural", &_XfGlyph_type, 2 },
    { "vnatural", &_XfGlyph_type, 2 },
    { "margin", &_XfGlyph_type, 2 },
    { "margin_lrbt", &_XfGlyph_type, 5 },
    { "margin_lrbt_flexible", &_XfGlyph_type, 13 },
    { "hmargin", &_XfGlyph_type, 2 },
    { "hmargin_lr", &_XfGlyph_type, 3 },
    { "hmargin_lr_flexible", &_XfGlyph_type, 7 },
    { "vmargin", &_XfGlyph_type, 2 },
    { "vmargin_bt", &_XfGlyph_type, 3 },
    { "vmargin_bt_flexible", &_XfGlyph_type, 7 },
    { "lmargin", &_XfGlyph_type, 2 },
    { "lmargin_flexible", &_XfGlyph_type, 4 },
    { "rmargin", &_XfGlyph_type, 2 },
    { "rmargin_flexible", &_XfGlyph_type, 4 },
    { "bmargin", &_XfGlyph_type, 2 },
    { "bmargin_flexible", &_XfGlyph_type, 4 },
    { "tmargin", &_XfGlyph_type, 2 },
    { "tmargin_flexible", &_XfGlyph_type, 4 },
    { 0, 0, 0 }
};
TypeObj_ParamData _XfLayoutKit_params[] = {
    /* fil */
        { "_p", 0, &_XfCoord_type },
    /* back */
        { "g", 0, &_XfGlyph_type },
        { "under", 0, &_XfGlyph_type },
    /* front */
        { "g", 0, &_XfGlyph_type },
        { "over", 0, &_XfGlyph_type },
    /* between */
        { "g", 0, &_XfGlyph_type },
        { "under", 0, &_XfGlyph_type },
        { "over", 0, &_XfGlyph_type },
    /* glue */
        { "a", 0, &_XfAxis_type },
        { "natural", 0, &_XfCoord_type },
        { "stretch", 0, &_XfCoord_type },
        { "shrink", 0, &_XfCoord_type },
        { "align", 0, &_XfAlignment_type },
    /* glue_requisition */
        { "r", 0, &_XfGlyph_Requisition_type },
    /* hglue_fil */
        { "natural", 0, &_XfCoord_type },
    /* hglue */
        { "natural", 0, &_XfCoord_type },
        { "stretch", 0, &_XfCoord_type },
        { "shrink", 0, &_XfCoord_type },
    /* hglue_aligned */
        { "natural", 0, &_XfCoord_type },
        { "stretch", 0, &_XfCoord_type },
        { "shrink", 0, &_XfCoord_type },
        { "a", 0, &_XfAlignment_type },
    /* hspace */
        { "natural", 0, &_XfCoord_type },
    /* vglue_fil */
        { "natural", 0, &_XfCoord_type },
    /* vglue */
        { "natural", 0, &_XfCoord_type },
        { "stretch", 0, &_XfCoord_type },
        { "shrink", 0, &_XfCoord_type },
    /* vglue_aligned */
        { "natural", 0, &_XfCoord_type },
        { "stretch", 0, &_XfCoord_type },
        { "shrink", 0, &_XfCoord_type },
        { "a", 0, &_XfAlignment_type },
    /* vspace */
        { "natural", 0, &_XfCoord_type },
    /* shape_of */
        { "g", 0, &_XfGlyph_type },
    /* shape_of_xy */
        { "gx", 0, &_XfGlyph_type },
        { "gy", 0, &_XfGlyph_type },
    /* shape_of_xyz */
        { "gx", 0, &_XfGlyph_type },
        { "gy", 0, &_XfGlyph_type },
        { "gz", 0, &_XfGlyph_type },
    /* strut */
        { "f", 0, &_XfFont_type },
        { "natural", 0, &_XfCoord_type },
        { "stretch", 0, &_XfCoord_type },
        { "shrink", 0, &_XfCoord_type },
    /* hstrut */
        { "right_bearing", 0, &_XfCoord_type },
        { "left_bearing", 0, &_XfCoord_type },
        { "natural", 0, &_XfCoord_type },
        { "stretch", 0, &_XfCoord_type },
        { "shrink", 0, &_XfCoord_type },
    /* vstrut */
        { "ascent", 0, &_XfCoord_type },
        { "descent", 0, &_XfCoord_type },
        { "natural", 0, &_XfCoord_type },
        { "stretch", 0, &_XfCoord_type },
        { "shrink", 0, &_XfCoord_type },
    /* spaces */
        { "count", 0, &_XfLong_type },
        { "each", 0, &_XfCoord_type },
        { "f", 0, &_XfFont_type },
        { "c", 0, &_XfColor_type },
    /* center */
        { "g", 0, &_XfGlyph_type },
    /* center_aligned */
        { "g", 0, &_XfGlyph_type },
        { "x", 0, &_XfAlignment_type },
        { "y", 0, &_XfAlignment_type },
    /* center_axis */
        { "g", 0, &_XfGlyph_type },
        { "a", 0, &_XfAxis_type },
        { "align", 0, &_XfAlignment_type },
    /* hcenter */
        { "g", 0, &_XfGlyph_type },
    /* hcenter_aligned */
        { "g", 0, &_XfGlyph_type },
        { "x", 0, &_XfAlignment_type },
    /* vcenter */
        { "g", 0, &_XfGlyph_type },
    /* vcenter_aligned */
        { "g", 0, &_XfGlyph_type },
        { "y", 0, &_XfAlignment_type },
    /* fixed */
        { "g", 0, &_XfGlyph_type },
        { "x", 0, &_XfCoord_type },
        { "y", 0, &_XfCoord_type },
    /* fixed_axis */
        { "g", 0, &_XfGlyph_type },
        { "a", 0, &_XfAxis_type },
        { "size", 0, &_XfCoord_type },
    /* hfixed */
        { "g", 0, &_XfGlyph_type },
        { "x", 0, &_XfCoord_type },
    /* vfixed */
        { "g", 0, &_XfGlyph_type },
        { "y", 0, &_XfCoord_type },
    /* flexible */
        { "g", 0, &_XfGlyph_type },
        { "stretch", 0, &_XfCoord_type },
        { "shrink", 0, &_XfCoord_type },
    /* flexible_fil */
        { "g", 0, &_XfGlyph_type },
    /* flexible_axis */
        { "g", 0, &_XfGlyph_type },
        { "a", 0, &_XfAxis_type },
        { "stretch", 0, &_XfCoord_type },
        { "shrink", 0, &_XfCoord_type },
    /* hflexible */
        { "g", 0, &_XfGlyph_type },
        { "stretch", 0, &_XfCoord_type },
        { "shrink", 0, &_XfCoord_type },
    /* vflexible */
        { "g", 0, &_XfGlyph_type },
        { "stretch", 0, &_XfCoord_type },
        { "shrink", 0, &_XfCoord_type },
    /* natural */
        { "g", 0, &_XfGlyph_type },
        { "x", 0, &_XfCoord_type },
        { "y", 0, &_XfCoord_type },
    /* natural_axis */
        { "g", 0, &_XfGlyph_type },
        { "a", 0, &_XfAxis_type },
        { "size", 0, &_XfCoord_type },
    /* hnatural */
        { "g", 0, &_XfGlyph_type },
        { "x", 0, &_XfCoord_type },
    /* vnatural */
        { "g", 0, &_XfGlyph_type },
        { "y", 0, &_XfCoord_type },
    /* margin */
        { "g", 0, &_XfGlyph_type },
        { "all", 0, &_XfCoord_type },
    /* margin_lrbt */
        { "g", 0, &_XfGlyph_type },
        { "lmargin", 0, &_XfCoord_type },
        { "rmargin", 0, &_XfCoord_type },
        { "bmargin", 0, &_XfCoord_type },
        { "tmargin", 0, &_XfCoord_type },
    /* margin_lrbt_flexible */
        { "g", 0, &_XfGlyph_type },
        { "lmargin", 0, &_XfCoord_type },
        { "lstretch", 0, &_XfCoord_type },
        { "lshrink", 0, &_XfCoord_type },
        { "rmargin", 0, &_XfCoord_type },
        { "rstretch", 0, &_XfCoord_type },
        { "rshrink", 0, &_XfCoord_type },
        { "bmargin", 0, &_XfCoord_type },
        { "bstretch", 0, &_XfCoord_type },
        { "bshrink", 0, &_XfCoord_type },
        { "tmargin", 0, &_XfCoord_type },
        { "tstretch", 0, &_XfCoord_type },
        { "tshrink", 0, &_XfCoord_type },
    /* hmargin */
        { "g", 0, &_XfGlyph_type },
        { "both", 0, &_XfCoord_type },
    /* hmargin_lr */
        { "g", 0, &_XfGlyph_type },
        { "lmargin", 0, &_XfCoord_type },
        { "rmargin", 0, &_XfCoord_type },
    /* hmargin_lr_flexible */
        { "g", 0, &_XfGlyph_type },
        { "lmargin", 0, &_XfCoord_type },
        { "lstretch", 0, &_XfCoord_type },
        { "lshrink", 0, &_XfCoord_type },
        { "rmargin", 0, &_XfCoord_type },
        { "rstretch", 0, &_XfCoord_type },
        { "rshrink", 0, &_XfCoord_type },
    /* vmargin */
        { "g", 0, &_XfGlyph_type },
        { "both", 0, &_XfCoord_type },
    /* vmargin_bt */
        { "g", 0, &_XfGlyph_type },
        { "bmargin", 0, &_XfCoord_type },
        { "tmargin", 0, &_XfCoord_type },
    /* vmargin_bt_flexible */
        { "g", 0, &_XfGlyph_type },
        { "bmargin", 0, &_XfCoord_type },
        { "bstretch", 0, &_XfCoord_type },
        { "bshrink", 0, &_XfCoord_type },
        { "tmargin", 0, &_XfCoord_type },
        { "tstretch", 0, &_XfCoord_type },
        { "tshrink", 0, &_XfCoord_type },
    /* lmargin */
        { "g", 0, &_XfGlyph_type },
        { "natural", 0, &_XfCoord_type },
    /* lmargin_flexible */
        { "g", 0, &_XfGlyph_type },
        { "natural", 0, &_XfCoord_type },
        { "stretch", 0, &_XfCoord_type },
        { "shrink", 0, &_XfCoord_type },
    /* rmargin */
        { "g", 0, &_XfGlyph_type },
        { "natural", 0, &_XfCoord_type },
    /* rmargin_flexible */
        { "g", 0, &_XfGlyph_type },
        { "natural", 0, &_XfCoord_type },
        { "stretch", 0, &_XfCoord_type },
        { "shrink", 0, &_XfCoord_type },
    /* bmargin */
        { "g", 0, &_XfGlyph_type },
        { "natural", 0, &_XfCoord_type },
    /* bmargin_flexible */
        { "g", 0, &_XfGlyph_type },
        { "natural", 0, &_XfCoord_type },
        { "stretch", 0, &_XfCoord_type },
        { "shrink", 0, &_XfCoord_type },
    /* tmargin */
        { "g", 0, &_XfGlyph_type },
        { "natural", 0, &_XfCoord_type },
    /* tmargin_flexible */
        { "g", 0, &_XfGlyph_type },
        { "natural", 0, &_XfCoord_type },
        { "stretch", 0, &_XfCoord_type },
        { "shrink", 0, &_XfCoord_type }
};
extern TypeObj_Descriptor _XfFrescoObject_type;
TypeObj_Descriptor* _XfLayoutKit_parents[] = { &_XfFrescoObject_type, nil };
extern TypeObjId _XfLayoutKit_tid;
extern void _XfLayoutKit_receive(BaseObjectRef, ULong, MarshalBuffer&);
TypeObj_Descriptor _XfLayoutKit_type = {
    /* type */ 0,
    /* id */ &_XfLayoutKit_tid,
    "LayoutKit",
    _XfLayoutKit_parents, /* offsets */ nil, /* excepts */ nil,
    _XfLayoutKit_methods, _XfLayoutKit_params,
    &_XfLayoutKit_receive
};

LayoutKitRef LayoutKit::_narrow(BaseObjectRef o) {
    return (LayoutKitRef)_BaseObject_tnarrow(
        o, _XfLayoutKit_tid, &LayoutKitStub::_create
    );
}
TypeObjId LayoutKitType::_tid() { return _XfLayoutKit_tid; }
void _XfLayoutKit_receive(BaseObjectRef _object, ULong _m, MarshalBuffer& _b) {
    extern TypeObjId _XfLayoutKit_tid;
    LayoutKitRef _this = (LayoutKitRef)_BaseObject_tcast(_object, _XfLayoutKit_tid);
    switch (_m) {
        case /* _get_fil */ 0: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit__get_fil_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_float = _this->fil();
            _b.reply(_XfLayoutKit__get_fil_pinfo, _arg);
            break;
        }
        case /* _set_fil */ 1: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit__set_fil_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            Coord _p;
            _arg[1].u_addr = &_p;
            _b.receive(_XfLayoutKit__set_fil_pinfo, _arg);
            _this->fil(_p);
            _b.reply(_XfLayoutKit__set_fil_pinfo, _arg);
            break;
        }
        case /* hbox */ 2: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_hbox_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_objref = _this->_c_hbox();
            _b.reply(_XfLayoutKit_hbox_pinfo, _arg);
            break;
        }
        case /* vbox */ 3: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_vbox_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_objref = _this->_c_vbox();
            _b.reply(_XfLayoutKit_vbox_pinfo, _arg);
            break;
        }
        case /* hbox_first_aligned */ 4: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_hbox_first_aligned_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_objref = _this->_c_hbox_first_aligned();
            _b.reply(_XfLayoutKit_hbox_first_aligned_pinfo, _arg);
            break;
        }
        case /* vbox_first_aligned */ 5: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_vbox_first_aligned_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_objref = _this->_c_vbox_first_aligned();
            _b.reply(_XfLayoutKit_vbox_first_aligned_pinfo, _arg);
            break;
        }
        case /* vscrollbox */ 6: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_vscrollbox_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_objref = _this->_c_vscrollbox();
            _b.reply(_XfLayoutKit_vscrollbox_pinfo, _arg);
            break;
        }
        case /* overlay */ 7: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_overlay_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_objref = _this->_c_overlay();
            _b.reply(_XfLayoutKit_overlay_pinfo, _arg);
            break;
        }
        case /* deck */ 8: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_deck_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_objref = _this->_c_deck();
            _b.reply(_XfLayoutKit_deck_pinfo, _arg);
            break;
        }
        case /* back */ 9: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_back_pinfo;
            MarshalBuffer::ArgValue _arg[3];
            GlyphRef g;
            _arg[1].u_addr = &g;
            GlyphRef under;
            _arg[2].u_addr = &under;
            _b.receive(_XfLayoutKit_back_pinfo, _arg);
            _arg[0].u_objref = _this->_c_back(g, under);
            _b.reply(_XfLayoutKit_back_pinfo, _arg);
            break;
        }
        case /* front */ 10: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_front_pinfo;
            MarshalBuffer::ArgValue _arg[3];
            GlyphRef g;
            _arg[1].u_addr = &g;
            GlyphRef over;
            _arg[2].u_addr = &over;
            _b.receive(_XfLayoutKit_front_pinfo, _arg);
            _arg[0].u_objref = _this->_c_front(g, over);
            _b.reply(_XfLayoutKit_front_pinfo, _arg);
            break;
        }
        case /* between */ 11: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_between_pinfo;
            MarshalBuffer::ArgValue _arg[4];
            GlyphRef g;
            _arg[1].u_addr = &g;
            GlyphRef under;
            _arg[2].u_addr = &under;
            GlyphRef over;
            _arg[3].u_addr = &over;
            _b.receive(_XfLayoutKit_between_pinfo, _arg);
            _arg[0].u_objref = _this->_c_between(g, under, over);
            _b.reply(_XfLayoutKit_between_pinfo, _arg);
            break;
        }
        case /* glue */ 12: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_glue_pinfo;
            MarshalBuffer::ArgValue _arg[6];
            Axis a;
            _arg[1].u_addr = &a;
            Coord natural;
            _arg[2].u_addr = &natural;
            Coord stretch;
            _arg[3].u_addr = &stretch;
            Coord shrink;
            _arg[4].u_addr = &shrink;
            Alignment align;
            _arg[5].u_addr = &align;
            _b.receive(_XfLayoutKit_glue_pinfo, _arg);
            _arg[0].u_objref = _this->_c_glue(a, natural, stretch, shrink, align);
            _b.reply(_XfLayoutKit_glue_pinfo, _arg);
            break;
        }
        case /* glue_requisition */ 13: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_glue_requisition_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            Glyph::Requisition r;
            _arg[1].u_addr = &r;
            _b.receive(_XfLayoutKit_glue_requisition_pinfo, _arg);
            _arg[0].u_objref = _this->_c_glue_requisition(r);
            _b.reply(_XfLayoutKit_glue_requisition_pinfo, _arg);
            break;
        }
        case /* hfil */ 14: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_hfil_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_objref = _this->_c_hfil();
            _b.reply(_XfLayoutKit_hfil_pinfo, _arg);
            break;
        }
        case /* hglue_fil */ 15: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_hglue_fil_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            Coord natural;
            _arg[1].u_addr = &natural;
            _b.receive(_XfLayoutKit_hglue_fil_pinfo, _arg);
            _arg[0].u_objref = _this->_c_hglue_fil(natural);
            _b.reply(_XfLayoutKit_hglue_fil_pinfo, _arg);
            break;
        }
        case /* hglue */ 16: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_hglue_pinfo;
            MarshalBuffer::ArgValue _arg[4];
            Coord natural;
            _arg[1].u_addr = &natural;
            Coord stretch;
            _arg[2].u_addr = &stretch;
            Coord shrink;
            _arg[3].u_addr = &shrink;
            _b.receive(_XfLayoutKit_hglue_pinfo, _arg);
            _arg[0].u_objref = _this->_c_hglue(natural, stretch, shrink);
            _b.reply(_XfLayoutKit_hglue_pinfo, _arg);
            break;
        }
        case /* hglue_aligned */ 17: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_hglue_aligned_pinfo;
            MarshalBuffer::ArgValue _arg[5];
            Coord natural;
            _arg[1].u_addr = &natural;
            Coord stretch;
            _arg[2].u_addr = &stretch;
            Coord shrink;
            _arg[3].u_addr = &shrink;
            Alignment a;
            _arg[4].u_addr = &a;
            _b.receive(_XfLayoutKit_hglue_aligned_pinfo, _arg);
            _arg[0].u_objref = _this->_c_hglue_aligned(natural, stretch, shrink, a);
            _b.reply(_XfLayoutKit_hglue_aligned_pinfo, _arg);
            break;
        }
        case /* hspace */ 18: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_hspace_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            Coord natural;
            _arg[1].u_addr = &natural;
            _b.receive(_XfLayoutKit_hspace_pinfo, _arg);
            _arg[0].u_objref = _this->_c_hspace(natural);
            _b.reply(_XfLayoutKit_hspace_pinfo, _arg);
            break;
        }
        case /* vfil */ 19: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_vfil_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_objref = _this->_c_vfil();
            _b.reply(_XfLayoutKit_vfil_pinfo, _arg);
            break;
        }
        case /* vglue_fil */ 20: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_vglue_fil_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            Coord natural;
            _arg[1].u_addr = &natural;
            _b.receive(_XfLayoutKit_vglue_fil_pinfo, _arg);
            _arg[0].u_objref = _this->_c_vglue_fil(natural);
            _b.reply(_XfLayoutKit_vglue_fil_pinfo, _arg);
            break;
        }
        case /* vglue */ 21: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_vglue_pinfo;
            MarshalBuffer::ArgValue _arg[4];
            Coord natural;
            _arg[1].u_addr = &natural;
            Coord stretch;
            _arg[2].u_addr = &stretch;
            Coord shrink;
            _arg[3].u_addr = &shrink;
            _b.receive(_XfLayoutKit_vglue_pinfo, _arg);
            _arg[0].u_objref = _this->_c_vglue(natural, stretch, shrink);
            _b.reply(_XfLayoutKit_vglue_pinfo, _arg);
            break;
        }
        case /* vglue_aligned */ 22: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_vglue_aligned_pinfo;
            MarshalBuffer::ArgValue _arg[5];
            Coord natural;
            _arg[1].u_addr = &natural;
            Coord stretch;
            _arg[2].u_addr = &stretch;
            Coord shrink;
            _arg[3].u_addr = &shrink;
            Alignment a;
            _arg[4].u_addr = &a;
            _b.receive(_XfLayoutKit_vglue_aligned_pinfo, _arg);
            _arg[0].u_objref = _this->_c_vglue_aligned(natural, stretch, shrink, a);
            _b.reply(_XfLayoutKit_vglue_aligned_pinfo, _arg);
            break;
        }
        case /* vspace */ 23: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_vspace_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            Coord natural;
            _arg[1].u_addr = &natural;
            _b.receive(_XfLayoutKit_vspace_pinfo, _arg);
            _arg[0].u_objref = _this->_c_vspace(natural);
            _b.reply(_XfLayoutKit_vspace_pinfo, _arg);
            break;
        }
        case /* shape_of */ 24: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_shape_of_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            GlyphRef g;
            _arg[1].u_addr = &g;
            _b.receive(_XfLayoutKit_shape_of_pinfo, _arg);
            _arg[0].u_objref = _this->_c_shape_of(g);
            _b.reply(_XfLayoutKit_shape_of_pinfo, _arg);
            break;
        }
        case /* shape_of_xy */ 25: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_shape_of_xy_pinfo;
            MarshalBuffer::ArgValue _arg[3];
            GlyphRef gx;
            _arg[1].u_addr = &gx;
            GlyphRef gy;
            _arg[2].u_addr = &gy;
            _b.receive(_XfLayoutKit_shape_of_xy_pinfo, _arg);
            _arg[0].u_objref = _this->_c_shape_of_xy(gx, gy);
            _b.reply(_XfLayoutKit_shape_of_xy_pinfo, _arg);
            break;
        }
        case /* shape_of_xyz */ 26: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_shape_of_xyz_pinfo;
            MarshalBuffer::ArgValue _arg[4];
            GlyphRef gx;
            _arg[1].u_addr = &gx;
            GlyphRef gy;
            _arg[2].u_addr = &gy;
            GlyphRef gz;
            _arg[3].u_addr = &gz;
            _b.receive(_XfLayoutKit_shape_of_xyz_pinfo, _arg);
            _arg[0].u_objref = _this->_c_shape_of_xyz(gx, gy, gz);
            _b.reply(_XfLayoutKit_shape_of_xyz_pinfo, _arg);
            break;
        }
        case /* strut */ 27: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_strut_pinfo;
            MarshalBuffer::ArgValue _arg[5];
            FontRef f;
            _arg[1].u_addr = &f;
            Coord natural;
            _arg[2].u_addr = &natural;
            Coord stretch;
            _arg[3].u_addr = &stretch;
            Coord shrink;
            _arg[4].u_addr = &shrink;
            _b.receive(_XfLayoutKit_strut_pinfo, _arg);
            _arg[0].u_objref = _this->_c_strut(f, natural, stretch, shrink);
            _b.reply(_XfLayoutKit_strut_pinfo, _arg);
            break;
        }
        case /* hstrut */ 28: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_hstrut_pinfo;
            MarshalBuffer::ArgValue _arg[6];
            Coord right_bearing;
            _arg[1].u_addr = &right_bearing;
            Coord left_bearing;
            _arg[2].u_addr = &left_bearing;
            Coord natural;
            _arg[3].u_addr = &natural;
            Coord stretch;
            _arg[4].u_addr = &stretch;
            Coord shrink;
            _arg[5].u_addr = &shrink;
            _b.receive(_XfLayoutKit_hstrut_pinfo, _arg);
            _arg[0].u_objref = _this->_c_hstrut(right_bearing, left_bearing, natural, stretch, shrink);
            _b.reply(_XfLayoutKit_hstrut_pinfo, _arg);
            break;
        }
        case /* vstrut */ 29: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_vstrut_pinfo;
            MarshalBuffer::ArgValue _arg[6];
            Coord ascent;
            _arg[1].u_addr = &ascent;
            Coord descent;
            _arg[2].u_addr = &descent;
            Coord natural;
            _arg[3].u_addr = &natural;
            Coord stretch;
            _arg[4].u_addr = &stretch;
            Coord shrink;
            _arg[5].u_addr = &shrink;
            _b.receive(_XfLayoutKit_vstrut_pinfo, _arg);
            _arg[0].u_objref = _this->_c_vstrut(ascent, descent, natural, stretch, shrink);
            _b.reply(_XfLayoutKit_vstrut_pinfo, _arg);
            break;
        }
        case /* spaces */ 30: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_spaces_pinfo;
            MarshalBuffer::ArgValue _arg[5];
            Long count;
            _arg[1].u_addr = &count;
            Coord each;
            _arg[2].u_addr = &each;
            FontRef f;
            _arg[3].u_addr = &f;
            ColorRef c;
            _arg[4].u_addr = &c;
            _b.receive(_XfLayoutKit_spaces_pinfo, _arg);
            _arg[0].u_objref = _this->_c_spaces(count, each, f, c);
            _b.reply(_XfLayoutKit_spaces_pinfo, _arg);
            break;
        }
        case /* center */ 31: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_center_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            GlyphRef g;
            _arg[1].u_addr = &g;
            _b.receive(_XfLayoutKit_center_pinfo, _arg);
            _arg[0].u_objref = _this->_c_center(g);
            _b.reply(_XfLayoutKit_center_pinfo, _arg);
            break;
        }
        case /* center_aligned */ 32: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_center_aligned_pinfo;
            MarshalBuffer::ArgValue _arg[4];
            GlyphRef g;
            _arg[1].u_addr = &g;
            Alignment x;
            _arg[2].u_addr = &x;
            Alignment y;
            _arg[3].u_addr = &y;
            _b.receive(_XfLayoutKit_center_aligned_pinfo, _arg);
            _arg[0].u_objref = _this->_c_center_aligned(g, x, y);
            _b.reply(_XfLayoutKit_center_aligned_pinfo, _arg);
            break;
        }
        case /* center_axis */ 33: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_center_axis_pinfo;
            MarshalBuffer::ArgValue _arg[4];
            GlyphRef g;
            _arg[1].u_addr = &g;
            Axis a;
            _arg[2].u_addr = &a;
            Alignment align;
            _arg[3].u_addr = &align;
            _b.receive(_XfLayoutKit_center_axis_pinfo, _arg);
            _arg[0].u_objref = _this->_c_center_axis(g, a, align);
            _b.reply(_XfLayoutKit_center_axis_pinfo, _arg);
            break;
        }
        case /* hcenter */ 34: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_hcenter_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            GlyphRef g;
            _arg[1].u_addr = &g;
            _b.receive(_XfLayoutKit_hcenter_pinfo, _arg);
            _arg[0].u_objref = _this->_c_hcenter(g);
            _b.reply(_XfLayoutKit_hcenter_pinfo, _arg);
            break;
        }
        case /* hcenter_aligned */ 35: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_hcenter_aligned_pinfo;
            MarshalBuffer::ArgValue _arg[3];
            GlyphRef g;
            _arg[1].u_addr = &g;
            Alignment x;
            _arg[2].u_addr = &x;
            _b.receive(_XfLayoutKit_hcenter_aligned_pinfo, _arg);
            _arg[0].u_objref = _this->_c_hcenter_aligned(g, x);
            _b.reply(_XfLayoutKit_hcenter_aligned_pinfo, _arg);
            break;
        }
        case /* vcenter */ 36: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_vcenter_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            GlyphRef g;
            _arg[1].u_addr = &g;
            _b.receive(_XfLayoutKit_vcenter_pinfo, _arg);
            _arg[0].u_objref = _this->_c_vcenter(g);
            _b.reply(_XfLayoutKit_vcenter_pinfo, _arg);
            break;
        }
        case /* vcenter_aligned */ 37: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_vcenter_aligned_pinfo;
            MarshalBuffer::ArgValue _arg[3];
            GlyphRef g;
            _arg[1].u_addr = &g;
            Alignment y;
            _arg[2].u_addr = &y;
            _b.receive(_XfLayoutKit_vcenter_aligned_pinfo, _arg);
            _arg[0].u_objref = _this->_c_vcenter_aligned(g, y);
            _b.reply(_XfLayoutKit_vcenter_aligned_pinfo, _arg);
            break;
        }
        case /* fixed */ 38: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_fixed_pinfo;
            MarshalBuffer::ArgValue _arg[4];
            GlyphRef g;
            _arg[1].u_addr = &g;
            Coord x;
            _arg[2].u_addr = &x;
            Coord y;
            _arg[3].u_addr = &y;
            _b.receive(_XfLayoutKit_fixed_pinfo, _arg);
            _arg[0].u_objref = _this->_c_fixed(g, x, y);
            _b.reply(_XfLayoutKit_fixed_pinfo, _arg);
            break;
        }
        case /* fixed_axis */ 39: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_fixed_axis_pinfo;
            MarshalBuffer::ArgValue _arg[4];
            GlyphRef g;
            _arg[1].u_addr = &g;
            Axis a;
            _arg[2].u_addr = &a;
            Coord size;
            _arg[3].u_addr = &size;
            _b.receive(_XfLayoutKit_fixed_axis_pinfo, _arg);
            _arg[0].u_objref = _this->_c_fixed_axis(g, a, size);
            _b.reply(_XfLayoutKit_fixed_axis_pinfo, _arg);
            break;
        }
        case /* hfixed */ 40: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_hfixed_pinfo;
            MarshalBuffer::ArgValue _arg[3];
            GlyphRef g;
            _arg[1].u_addr = &g;
            Coord x;
            _arg[2].u_addr = &x;
            _b.receive(_XfLayoutKit_hfixed_pinfo, _arg);
            _arg[0].u_objref = _this->_c_hfixed(g, x);
            _b.reply(_XfLayoutKit_hfixed_pinfo, _arg);
            break;
        }
        case /* vfixed */ 41: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_vfixed_pinfo;
            MarshalBuffer::ArgValue _arg[3];
            GlyphRef g;
            _arg[1].u_addr = &g;
            Coord y;
            _arg[2].u_addr = &y;
            _b.receive(_XfLayoutKit_vfixed_pinfo, _arg);
            _arg[0].u_objref = _this->_c_vfixed(g, y);
            _b.reply(_XfLayoutKit_vfixed_pinfo, _arg);
            break;
        }
        case /* flexible */ 42: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_flexible_pinfo;
            MarshalBuffer::ArgValue _arg[4];
            GlyphRef g;
            _arg[1].u_addr = &g;
            Coord stretch;
            _arg[2].u_addr = &stretch;
            Coord shrink;
            _arg[3].u_addr = &shrink;
            _b.receive(_XfLayoutKit_flexible_pinfo, _arg);
            _arg[0].u_objref = _this->_c_flexible(g, stretch, shrink);
            _b.reply(_XfLayoutKit_flexible_pinfo, _arg);
            break;
        }
        case /* flexible_fil */ 43: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_flexible_fil_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            GlyphRef g;
            _arg[1].u_addr = &g;
            _b.receive(_XfLayoutKit_flexible_fil_pinfo, _arg);
            _arg[0].u_objref = _this->_c_flexible_fil(g);
            _b.reply(_XfLayoutKit_flexible_fil_pinfo, _arg);
            break;
        }
        case /* flexible_axis */ 44: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_flexible_axis_pinfo;
            MarshalBuffer::ArgValue _arg[5];
            GlyphRef g;
            _arg[1].u_addr = &g;
            Axis a;
            _arg[2].u_addr = &a;
            Coord stretch;
            _arg[3].u_addr = &stretch;
            Coord shrink;
            _arg[4].u_addr = &shrink;
            _b.receive(_XfLayoutKit_flexible_axis_pinfo, _arg);
            _arg[0].u_objref = _this->_c_flexible_axis(g, a, stretch, shrink);
            _b.reply(_XfLayoutKit_flexible_axis_pinfo, _arg);
            break;
        }
        case /* hflexible */ 45: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_hflexible_pinfo;
            MarshalBuffer::ArgValue _arg[4];
            GlyphRef g;
            _arg[1].u_addr = &g;
            Coord stretch;
            _arg[2].u_addr = &stretch;
            Coord shrink;
            _arg[3].u_addr = &shrink;
            _b.receive(_XfLayoutKit_hflexible_pinfo, _arg);
            _arg[0].u_objref = _this->_c_hflexible(g, stretch, shrink);
            _b.reply(_XfLayoutKit_hflexible_pinfo, _arg);
            break;
        }
        case /* vflexible */ 46: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_vflexible_pinfo;
            MarshalBuffer::ArgValue _arg[4];
            GlyphRef g;
            _arg[1].u_addr = &g;
            Coord stretch;
            _arg[2].u_addr = &stretch;
            Coord shrink;
            _arg[3].u_addr = &shrink;
            _b.receive(_XfLayoutKit_vflexible_pinfo, _arg);
            _arg[0].u_objref = _this->_c_vflexible(g, stretch, shrink);
            _b.reply(_XfLayoutKit_vflexible_pinfo, _arg);
            break;
        }
        case /* natural */ 47: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_natural_pinfo;
            MarshalBuffer::ArgValue _arg[4];
            GlyphRef g;
            _arg[1].u_addr = &g;
            Coord x;
            _arg[2].u_addr = &x;
            Coord y;
            _arg[3].u_addr = &y;
            _b.receive(_XfLayoutKit_natural_pinfo, _arg);
            _arg[0].u_objref = _this->_c_natural(g, x, y);
            _b.reply(_XfLayoutKit_natural_pinfo, _arg);
            break;
        }
        case /* natural_axis */ 48: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_natural_axis_pinfo;
            MarshalBuffer::ArgValue _arg[4];
            GlyphRef g;
            _arg[1].u_addr = &g;
            Axis a;
            _arg[2].u_addr = &a;
            Coord size;
            _arg[3].u_addr = &size;
            _b.receive(_XfLayoutKit_natural_axis_pinfo, _arg);
            _arg[0].u_objref = _this->_c_natural_axis(g, a, size);
            _b.reply(_XfLayoutKit_natural_axis_pinfo, _arg);
            break;
        }
        case /* hnatural */ 49: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_hnatural_pinfo;
            MarshalBuffer::ArgValue _arg[3];
            GlyphRef g;
            _arg[1].u_addr = &g;
            Coord x;
            _arg[2].u_addr = &x;
            _b.receive(_XfLayoutKit_hnatural_pinfo, _arg);
            _arg[0].u_objref = _this->_c_hnatural(g, x);
            _b.reply(_XfLayoutKit_hnatural_pinfo, _arg);
            break;
        }
        case /* vnatural */ 50: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_vnatural_pinfo;
            MarshalBuffer::ArgValue _arg[3];
            GlyphRef g;
            _arg[1].u_addr = &g;
            Coord y;
            _arg[2].u_addr = &y;
            _b.receive(_XfLayoutKit_vnatural_pinfo, _arg);
            _arg[0].u_objref = _this->_c_vnatural(g, y);
            _b.reply(_XfLayoutKit_vnatural_pinfo, _arg);
            break;
        }
        case /* margin */ 51: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_margin_pinfo;
            MarshalBuffer::ArgValue _arg[3];
            GlyphRef g;
            _arg[1].u_addr = &g;
            Coord all;
            _arg[2].u_addr = &all;
            _b.receive(_XfLayoutKit_margin_pinfo, _arg);
            _arg[0].u_objref = _this->_c_margin(g, all);
            _b.reply(_XfLayoutKit_margin_pinfo, _arg);
            break;
        }
        case /* margin_lrbt */ 52: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_margin_lrbt_pinfo;
            MarshalBuffer::ArgValue _arg[6];
            GlyphRef g;
            _arg[1].u_addr = &g;
            Coord lmargin;
            _arg[2].u_addr = &lmargin;
            Coord rmargin;
            _arg[3].u_addr = &rmargin;
            Coord bmargin;
            _arg[4].u_addr = &bmargin;
            Coord tmargin;
            _arg[5].u_addr = &tmargin;
            _b.receive(_XfLayoutKit_margin_lrbt_pinfo, _arg);
            _arg[0].u_objref = _this->_c_margin_lrbt(g, lmargin, rmargin, bmargin, tmargin);
            _b.reply(_XfLayoutKit_margin_lrbt_pinfo, _arg);
            break;
        }
        case /* margin_lrbt_flexible */ 53: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_margin_lrbt_flexible_pinfo;
            MarshalBuffer::ArgValue _arg[14];
            GlyphRef g;
            _arg[1].u_addr = &g;
            Coord lmargin;
            _arg[2].u_addr = &lmargin;
            Coord lstretch;
            _arg[3].u_addr = &lstretch;
            Coord lshrink;
            _arg[4].u_addr = &lshrink;
            Coord rmargin;
            _arg[5].u_addr = &rmargin;
            Coord rstretch;
            _arg[6].u_addr = &rstretch;
            Coord rshrink;
            _arg[7].u_addr = &rshrink;
            Coord bmargin;
            _arg[8].u_addr = &bmargin;
            Coord bstretch;
            _arg[9].u_addr = &bstretch;
            Coord bshrink;
            _arg[10].u_addr = &bshrink;
            Coord tmargin;
            _arg[11].u_addr = &tmargin;
            Coord tstretch;
            _arg[12].u_addr = &tstretch;
            Coord tshrink;
            _arg[13].u_addr = &tshrink;
            _b.receive(_XfLayoutKit_margin_lrbt_flexible_pinfo, _arg);
            _arg[0].u_objref = _this->_c_margin_lrbt_flexible(g, lmargin, lstretch, lshrink, rmargin, rstretch, rshrink, bmargin, bstretch, bshrink, tmargin, tstretch, tshrink);
            _b.reply(_XfLayoutKit_margin_lrbt_flexible_pinfo, _arg);
            break;
        }
        case /* hmargin */ 54: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_hmargin_pinfo;
            MarshalBuffer::ArgValue _arg[3];
            GlyphRef g;
            _arg[1].u_addr = &g;
            Coord both;
            _arg[2].u_addr = &both;
            _b.receive(_XfLayoutKit_hmargin_pinfo, _arg);
            _arg[0].u_objref = _this->_c_hmargin(g, both);
            _b.reply(_XfLayoutKit_hmargin_pinfo, _arg);
            break;
        }
        case /* hmargin_lr */ 55: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_hmargin_lr_pinfo;
            MarshalBuffer::ArgValue _arg[4];
            GlyphRef g;
            _arg[1].u_addr = &g;
            Coord lmargin;
            _arg[2].u_addr = &lmargin;
            Coord rmargin;
            _arg[3].u_addr = &rmargin;
            _b.receive(_XfLayoutKit_hmargin_lr_pinfo, _arg);
            _arg[0].u_objref = _this->_c_hmargin_lr(g, lmargin, rmargin);
            _b.reply(_XfLayoutKit_hmargin_lr_pinfo, _arg);
            break;
        }
        case /* hmargin_lr_flexible */ 56: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_hmargin_lr_flexible_pinfo;
            MarshalBuffer::ArgValue _arg[8];
            GlyphRef g;
            _arg[1].u_addr = &g;
            Coord lmargin;
            _arg[2].u_addr = &lmargin;
            Coord lstretch;
            _arg[3].u_addr = &lstretch;
            Coord lshrink;
            _arg[4].u_addr = &lshrink;
            Coord rmargin;
            _arg[5].u_addr = &rmargin;
            Coord rstretch;
            _arg[6].u_addr = &rstretch;
            Coord rshrink;
            _arg[7].u_addr = &rshrink;
            _b.receive(_XfLayoutKit_hmargin_lr_flexible_pinfo, _arg);
            _arg[0].u_objref = _this->_c_hmargin_lr_flexible(g, lmargin, lstretch, lshrink, rmargin, rstretch, rshrink);
            _b.reply(_XfLayoutKit_hmargin_lr_flexible_pinfo, _arg);
            break;
        }
        case /* vmargin */ 57: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_vmargin_pinfo;
            MarshalBuffer::ArgValue _arg[3];
            GlyphRef g;
            _arg[1].u_addr = &g;
            Coord both;
            _arg[2].u_addr = &both;
            _b.receive(_XfLayoutKit_vmargin_pinfo, _arg);
            _arg[0].u_objref = _this->_c_vmargin(g, both);
            _b.reply(_XfLayoutKit_vmargin_pinfo, _arg);
            break;
        }
        case /* vmargin_bt */ 58: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_vmargin_bt_pinfo;
            MarshalBuffer::ArgValue _arg[4];
            GlyphRef g;
            _arg[1].u_addr = &g;
            Coord bmargin;
            _arg[2].u_addr = &bmargin;
            Coord tmargin;
            _arg[3].u_addr = &tmargin;
            _b.receive(_XfLayoutKit_vmargin_bt_pinfo, _arg);
            _arg[0].u_objref = _this->_c_vmargin_bt(g, bmargin, tmargin);
            _b.reply(_XfLayoutKit_vmargin_bt_pinfo, _arg);
            break;
        }
        case /* vmargin_bt_flexible */ 59: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_vmargin_bt_flexible_pinfo;
            MarshalBuffer::ArgValue _arg[8];
            GlyphRef g;
            _arg[1].u_addr = &g;
            Coord bmargin;
            _arg[2].u_addr = &bmargin;
            Coord bstretch;
            _arg[3].u_addr = &bstretch;
            Coord bshrink;
            _arg[4].u_addr = &bshrink;
            Coord tmargin;
            _arg[5].u_addr = &tmargin;
            Coord tstretch;
            _arg[6].u_addr = &tstretch;
            Coord tshrink;
            _arg[7].u_addr = &tshrink;
            _b.receive(_XfLayoutKit_vmargin_bt_flexible_pinfo, _arg);
            _arg[0].u_objref = _this->_c_vmargin_bt_flexible(g, bmargin, bstretch, bshrink, tmargin, tstretch, tshrink);
            _b.reply(_XfLayoutKit_vmargin_bt_flexible_pinfo, _arg);
            break;
        }
        case /* lmargin */ 60: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_lmargin_pinfo;
            MarshalBuffer::ArgValue _arg[3];
            GlyphRef g;
            _arg[1].u_addr = &g;
            Coord natural;
            _arg[2].u_addr = &natural;
            _b.receive(_XfLayoutKit_lmargin_pinfo, _arg);
            _arg[0].u_objref = _this->_c_lmargin(g, natural);
            _b.reply(_XfLayoutKit_lmargin_pinfo, _arg);
            break;
        }
        case /* lmargin_flexible */ 61: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_lmargin_flexible_pinfo;
            MarshalBuffer::ArgValue _arg[5];
            GlyphRef g;
            _arg[1].u_addr = &g;
            Coord natural;
            _arg[2].u_addr = &natural;
            Coord stretch;
            _arg[3].u_addr = &stretch;
            Coord shrink;
            _arg[4].u_addr = &shrink;
            _b.receive(_XfLayoutKit_lmargin_flexible_pinfo, _arg);
            _arg[0].u_objref = _this->_c_lmargin_flexible(g, natural, stretch, shrink);
            _b.reply(_XfLayoutKit_lmargin_flexible_pinfo, _arg);
            break;
        }
        case /* rmargin */ 62: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_rmargin_pinfo;
            MarshalBuffer::ArgValue _arg[3];
            GlyphRef g;
            _arg[1].u_addr = &g;
            Coord natural;
            _arg[2].u_addr = &natural;
            _b.receive(_XfLayoutKit_rmargin_pinfo, _arg);
            _arg[0].u_objref = _this->_c_rmargin(g, natural);
            _b.reply(_XfLayoutKit_rmargin_pinfo, _arg);
            break;
        }
        case /* rmargin_flexible */ 63: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_rmargin_flexible_pinfo;
            MarshalBuffer::ArgValue _arg[5];
            GlyphRef g;
            _arg[1].u_addr = &g;
            Coord natural;
            _arg[2].u_addr = &natural;
            Coord stretch;
            _arg[3].u_addr = &stretch;
            Coord shrink;
            _arg[4].u_addr = &shrink;
            _b.receive(_XfLayoutKit_rmargin_flexible_pinfo, _arg);
            _arg[0].u_objref = _this->_c_rmargin_flexible(g, natural, stretch, shrink);
            _b.reply(_XfLayoutKit_rmargin_flexible_pinfo, _arg);
            break;
        }
        case /* bmargin */ 64: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_bmargin_pinfo;
            MarshalBuffer::ArgValue _arg[3];
            GlyphRef g;
            _arg[1].u_addr = &g;
            Coord natural;
            _arg[2].u_addr = &natural;
            _b.receive(_XfLayoutKit_bmargin_pinfo, _arg);
            _arg[0].u_objref = _this->_c_bmargin(g, natural);
            _b.reply(_XfLayoutKit_bmargin_pinfo, _arg);
            break;
        }
        case /* bmargin_flexible */ 65: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_bmargin_flexible_pinfo;
            MarshalBuffer::ArgValue _arg[5];
            GlyphRef g;
            _arg[1].u_addr = &g;
            Coord natural;
            _arg[2].u_addr = &natural;
            Coord stretch;
            _arg[3].u_addr = &stretch;
            Coord shrink;
            _arg[4].u_addr = &shrink;
            _b.receive(_XfLayoutKit_bmargin_flexible_pinfo, _arg);
            _arg[0].u_objref = _this->_c_bmargin_flexible(g, natural, stretch, shrink);
            _b.reply(_XfLayoutKit_bmargin_flexible_pinfo, _arg);
            break;
        }
        case /* tmargin */ 66: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_tmargin_pinfo;
            MarshalBuffer::ArgValue _arg[3];
            GlyphRef g;
            _arg[1].u_addr = &g;
            Coord natural;
            _arg[2].u_addr = &natural;
            _b.receive(_XfLayoutKit_tmargin_pinfo, _arg);
            _arg[0].u_objref = _this->_c_tmargin(g, natural);
            _b.reply(_XfLayoutKit_tmargin_pinfo, _arg);
            break;
        }
        case /* tmargin_flexible */ 67: {
            extern MarshalBuffer::ArgInfo _XfLayoutKit_tmargin_flexible_pinfo;
            MarshalBuffer::ArgValue _arg[5];
            GlyphRef g;
            _arg[1].u_addr = &g;
            Coord natural;
            _arg[2].u_addr = &natural;
            Coord stretch;
            _arg[3].u_addr = &stretch;
            Coord shrink;
            _arg[4].u_addr = &shrink;
            _b.receive(_XfLayoutKit_tmargin_flexible_pinfo, _arg);
            _arg[0].u_objref = _this->_c_tmargin_flexible(g, natural, stretch, shrink);
            _b.reply(_XfLayoutKit_tmargin_flexible_pinfo, _arg);
            break;
        }
    }
}
extern void _XfGlyph_Requisition_put(
    MarshalBuffer&, const Glyph::Requisition&
);
extern void _XfGlyph_Requisition_get(
    MarshalBuffer&, Glyph::Requisition&
);

LayoutKitStub::LayoutKitStub(Exchange* e) { exch_ = e; }
LayoutKitStub::~LayoutKitStub() { }
BaseObjectRef LayoutKitStub::_create(Exchange* e) {
    return (BaseObjectRef)(void*)new LayoutKitStub(e);
}
Exchange* LayoutKitStub::_exchange() {
    return exch_;
}
MarshalBuffer::ArgDesc _XfLayoutKit__get_fil_pdesc[2] = { 1, 48 };
MarshalBuffer::ArgInfo _XfLayoutKit__get_fil_pinfo = {
    &_XfLayoutKit_tid, 0, _XfLayoutKit__get_fil_pdesc, 0
};
Coord LayoutKitType::fil() {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfLayoutKit__get_fil_pinfo, _arg);
    return _arg[0].u_float;
}
MarshalBuffer::ArgDesc _XfLayoutKit__set_fil_pdesc[3] = { 2, 4, 49 };
MarshalBuffer::ArgInfo _XfLayoutKit__set_fil_pinfo = {
    &_XfLayoutKit_tid, 1, _XfLayoutKit__set_fil_pdesc, 0
};
void LayoutKitType::fil(Coord _p) {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_float = _p;
    _b.invoke(this, _XfLayoutKit__set_fil_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfLayoutKit_hbox_pdesc[2] = { 1, 60 };
MarshalBuffer::ArgMarshal _XfLayoutKit_hbox_pfunc[] = {
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_hbox_pinfo = {
    &_XfLayoutKit_tid, 2, _XfLayoutKit_hbox_pdesc, _XfLayoutKit_hbox_pfunc
};
GlyphRef LayoutKitType::_c_hbox() {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfLayoutKit_hbox_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_vbox_pdesc[2] = { 1, 60 };
MarshalBuffer::ArgMarshal _XfLayoutKit_vbox_pfunc[] = {
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_vbox_pinfo = {
    &_XfLayoutKit_tid, 3, _XfLayoutKit_vbox_pdesc, _XfLayoutKit_vbox_pfunc
};
GlyphRef LayoutKitType::_c_vbox() {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfLayoutKit_vbox_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_hbox_first_aligned_pdesc[2] = { 1, 60 };
MarshalBuffer::ArgMarshal _XfLayoutKit_hbox_first_aligned_pfunc[] = {
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_hbox_first_aligned_pinfo = {
    &_XfLayoutKit_tid, 4, _XfLayoutKit_hbox_first_aligned_pdesc, _XfLayoutKit_hbox_first_aligned_pfunc
};
GlyphRef LayoutKitType::_c_hbox_first_aligned() {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfLayoutKit_hbox_first_aligned_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_vbox_first_aligned_pdesc[2] = { 1, 60 };
MarshalBuffer::ArgMarshal _XfLayoutKit_vbox_first_aligned_pfunc[] = {
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_vbox_first_aligned_pinfo = {
    &_XfLayoutKit_tid, 5, _XfLayoutKit_vbox_first_aligned_pdesc, _XfLayoutKit_vbox_first_aligned_pfunc
};
GlyphRef LayoutKitType::_c_vbox_first_aligned() {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfLayoutKit_vbox_first_aligned_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_vscrollbox_pdesc[2] = { 1, 60 };
MarshalBuffer::ArgMarshal _XfLayoutKit_vscrollbox_pfunc[] = {
    &ScrollBoxStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_vscrollbox_pinfo = {
    &_XfLayoutKit_tid, 6, _XfLayoutKit_vscrollbox_pdesc, _XfLayoutKit_vscrollbox_pfunc
};
ScrollBoxRef LayoutKitType::_c_vscrollbox() {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfLayoutKit_vscrollbox_pinfo, _arg);
    return (ScrollBoxRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_overlay_pdesc[2] = { 1, 60 };
MarshalBuffer::ArgMarshal _XfLayoutKit_overlay_pfunc[] = {
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_overlay_pinfo = {
    &_XfLayoutKit_tid, 7, _XfLayoutKit_overlay_pdesc, _XfLayoutKit_overlay_pfunc
};
GlyphRef LayoutKitType::_c_overlay() {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfLayoutKit_overlay_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_deck_pdesc[2] = { 1, 60 };
MarshalBuffer::ArgMarshal _XfLayoutKit_deck_pfunc[] = {
    &DeckObjStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_deck_pinfo = {
    &_XfLayoutKit_tid, 8, _XfLayoutKit_deck_pdesc, _XfLayoutKit_deck_pfunc
};
DeckObjRef LayoutKitType::_c_deck() {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfLayoutKit_deck_pinfo, _arg);
    return (DeckObjRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_back_pdesc[4] = { 3, 60, 61, 61 };
MarshalBuffer::ArgMarshal _XfLayoutKit_back_pfunc[] = {
    &GlyphStub::_create,
    &GlyphStub::_create,
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_back_pinfo = {
    &_XfLayoutKit_tid, 9, _XfLayoutKit_back_pdesc, _XfLayoutKit_back_pfunc
};
GlyphRef LayoutKitType::_c_back(Glyph_in g, Glyph_in under) {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[3];
    _arg[1].u_objref = g;
    _arg[2].u_objref = under;
    _b.invoke(this, _XfLayoutKit_back_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_front_pdesc[4] = { 3, 60, 61, 61 };
MarshalBuffer::ArgMarshal _XfLayoutKit_front_pfunc[] = {
    &GlyphStub::_create,
    &GlyphStub::_create,
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_front_pinfo = {
    &_XfLayoutKit_tid, 10, _XfLayoutKit_front_pdesc, _XfLayoutKit_front_pfunc
};
GlyphRef LayoutKitType::_c_front(Glyph_in g, Glyph_in over) {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[3];
    _arg[1].u_objref = g;
    _arg[2].u_objref = over;
    _b.invoke(this, _XfLayoutKit_front_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_between_pdesc[5] = { 4, 60, 61, 61, 61 };
MarshalBuffer::ArgMarshal _XfLayoutKit_between_pfunc[] = {
    &GlyphStub::_create,
    &GlyphStub::_create,
    &GlyphStub::_create,
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_between_pinfo = {
    &_XfLayoutKit_tid, 11, _XfLayoutKit_between_pdesc, _XfLayoutKit_between_pfunc
};
GlyphRef LayoutKitType::_c_between(Glyph_in g, Glyph_in under, Glyph_in over) {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[4];
    _arg[1].u_objref = g;
    _arg[2].u_objref = under;
    _arg[3].u_objref = over;
    _b.invoke(this, _XfLayoutKit_between_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_glue_pdesc[7] = { 6, 60, 33, 49, 49, 49, 49 };
MarshalBuffer::ArgMarshal _XfLayoutKit_glue_pfunc[] = {
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_glue_pinfo = {
    &_XfLayoutKit_tid, 12, _XfLayoutKit_glue_pdesc, _XfLayoutKit_glue_pfunc
};
GlyphRef LayoutKitType::_c_glue(Axis a, Coord natural, Coord stretch, Coord shrink, Alignment align) {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[6];
    _arg[1].u_long = a;
    _arg[2].u_float = natural;
    _arg[3].u_float = stretch;
    _arg[4].u_float = shrink;
    _arg[5].u_float = align;
    _b.invoke(this, _XfLayoutKit_glue_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_glue_requisition_pdesc[3] = { 2, 60, 1 };
MarshalBuffer::ArgMarshal _XfLayoutKit_glue_requisition_pfunc[] = {
    &_XfGlyph_Requisition_put, &_XfGlyph_Requisition_get,
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_glue_requisition_pinfo = {
    &_XfLayoutKit_tid, 13, _XfLayoutKit_glue_requisition_pdesc, _XfLayoutKit_glue_requisition_pfunc
};
GlyphRef LayoutKitType::_c_glue_requisition(const Glyph::Requisition& r) {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_addr = &r;
    _b.invoke(this, _XfLayoutKit_glue_requisition_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_hfil_pdesc[2] = { 1, 60 };
MarshalBuffer::ArgMarshal _XfLayoutKit_hfil_pfunc[] = {
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_hfil_pinfo = {
    &_XfLayoutKit_tid, 14, _XfLayoutKit_hfil_pdesc, _XfLayoutKit_hfil_pfunc
};
GlyphRef LayoutKitType::_c_hfil() {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfLayoutKit_hfil_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_hglue_fil_pdesc[3] = { 2, 60, 49 };
MarshalBuffer::ArgMarshal _XfLayoutKit_hglue_fil_pfunc[] = {
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_hglue_fil_pinfo = {
    &_XfLayoutKit_tid, 15, _XfLayoutKit_hglue_fil_pdesc, _XfLayoutKit_hglue_fil_pfunc
};
GlyphRef LayoutKitType::_c_hglue_fil(Coord natural) {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_float = natural;
    _b.invoke(this, _XfLayoutKit_hglue_fil_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_hglue_pdesc[5] = { 4, 60, 49, 49, 49 };
MarshalBuffer::ArgMarshal _XfLayoutKit_hglue_pfunc[] = {
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_hglue_pinfo = {
    &_XfLayoutKit_tid, 16, _XfLayoutKit_hglue_pdesc, _XfLayoutKit_hglue_pfunc
};
GlyphRef LayoutKitType::_c_hglue(Coord natural, Coord stretch, Coord shrink) {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[4];
    _arg[1].u_float = natural;
    _arg[2].u_float = stretch;
    _arg[3].u_float = shrink;
    _b.invoke(this, _XfLayoutKit_hglue_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_hglue_aligned_pdesc[6] = { 5, 60, 49, 49, 49, 49 };
MarshalBuffer::ArgMarshal _XfLayoutKit_hglue_aligned_pfunc[] = {
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_hglue_aligned_pinfo = {
    &_XfLayoutKit_tid, 17, _XfLayoutKit_hglue_aligned_pdesc, _XfLayoutKit_hglue_aligned_pfunc
};
GlyphRef LayoutKitType::_c_hglue_aligned(Coord natural, Coord stretch, Coord shrink, Alignment a) {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[5];
    _arg[1].u_float = natural;
    _arg[2].u_float = stretch;
    _arg[3].u_float = shrink;
    _arg[4].u_float = a;
    _b.invoke(this, _XfLayoutKit_hglue_aligned_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_hspace_pdesc[3] = { 2, 60, 49 };
MarshalBuffer::ArgMarshal _XfLayoutKit_hspace_pfunc[] = {
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_hspace_pinfo = {
    &_XfLayoutKit_tid, 18, _XfLayoutKit_hspace_pdesc, _XfLayoutKit_hspace_pfunc
};
GlyphRef LayoutKitType::_c_hspace(Coord natural) {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_float = natural;
    _b.invoke(this, _XfLayoutKit_hspace_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_vfil_pdesc[2] = { 1, 60 };
MarshalBuffer::ArgMarshal _XfLayoutKit_vfil_pfunc[] = {
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_vfil_pinfo = {
    &_XfLayoutKit_tid, 19, _XfLayoutKit_vfil_pdesc, _XfLayoutKit_vfil_pfunc
};
GlyphRef LayoutKitType::_c_vfil() {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfLayoutKit_vfil_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_vglue_fil_pdesc[3] = { 2, 60, 49 };
MarshalBuffer::ArgMarshal _XfLayoutKit_vglue_fil_pfunc[] = {
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_vglue_fil_pinfo = {
    &_XfLayoutKit_tid, 20, _XfLayoutKit_vglue_fil_pdesc, _XfLayoutKit_vglue_fil_pfunc
};
GlyphRef LayoutKitType::_c_vglue_fil(Coord natural) {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_float = natural;
    _b.invoke(this, _XfLayoutKit_vglue_fil_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_vglue_pdesc[5] = { 4, 60, 49, 49, 49 };
MarshalBuffer::ArgMarshal _XfLayoutKit_vglue_pfunc[] = {
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_vglue_pinfo = {
    &_XfLayoutKit_tid, 21, _XfLayoutKit_vglue_pdesc, _XfLayoutKit_vglue_pfunc
};
GlyphRef LayoutKitType::_c_vglue(Coord natural, Coord stretch, Coord shrink) {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[4];
    _arg[1].u_float = natural;
    _arg[2].u_float = stretch;
    _arg[3].u_float = shrink;
    _b.invoke(this, _XfLayoutKit_vglue_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_vglue_aligned_pdesc[6] = { 5, 60, 49, 49, 49, 49 };
MarshalBuffer::ArgMarshal _XfLayoutKit_vglue_aligned_pfunc[] = {
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_vglue_aligned_pinfo = {
    &_XfLayoutKit_tid, 22, _XfLayoutKit_vglue_aligned_pdesc, _XfLayoutKit_vglue_aligned_pfunc
};
GlyphRef LayoutKitType::_c_vglue_aligned(Coord natural, Coord stretch, Coord shrink, Alignment a) {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[5];
    _arg[1].u_float = natural;
    _arg[2].u_float = stretch;
    _arg[3].u_float = shrink;
    _arg[4].u_float = a;
    _b.invoke(this, _XfLayoutKit_vglue_aligned_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_vspace_pdesc[3] = { 2, 60, 49 };
MarshalBuffer::ArgMarshal _XfLayoutKit_vspace_pfunc[] = {
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_vspace_pinfo = {
    &_XfLayoutKit_tid, 23, _XfLayoutKit_vspace_pdesc, _XfLayoutKit_vspace_pfunc
};
GlyphRef LayoutKitType::_c_vspace(Coord natural) {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_float = natural;
    _b.invoke(this, _XfLayoutKit_vspace_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_shape_of_pdesc[3] = { 2, 60, 61 };
MarshalBuffer::ArgMarshal _XfLayoutKit_shape_of_pfunc[] = {
    &GlyphStub::_create,
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_shape_of_pinfo = {
    &_XfLayoutKit_tid, 24, _XfLayoutKit_shape_of_pdesc, _XfLayoutKit_shape_of_pfunc
};
GlyphRef LayoutKitType::_c_shape_of(Glyph_in g) {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = g;
    _b.invoke(this, _XfLayoutKit_shape_of_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_shape_of_xy_pdesc[4] = { 3, 60, 61, 61 };
MarshalBuffer::ArgMarshal _XfLayoutKit_shape_of_xy_pfunc[] = {
    &GlyphStub::_create,
    &GlyphStub::_create,
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_shape_of_xy_pinfo = {
    &_XfLayoutKit_tid, 25, _XfLayoutKit_shape_of_xy_pdesc, _XfLayoutKit_shape_of_xy_pfunc
};
GlyphRef LayoutKitType::_c_shape_of_xy(Glyph_in gx, Glyph_in gy) {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[3];
    _arg[1].u_objref = gx;
    _arg[2].u_objref = gy;
    _b.invoke(this, _XfLayoutKit_shape_of_xy_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_shape_of_xyz_pdesc[5] = { 4, 60, 61, 61, 61 };
MarshalBuffer::ArgMarshal _XfLayoutKit_shape_of_xyz_pfunc[] = {
    &GlyphStub::_create,
    &GlyphStub::_create,
    &GlyphStub::_create,
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_shape_of_xyz_pinfo = {
    &_XfLayoutKit_tid, 26, _XfLayoutKit_shape_of_xyz_pdesc, _XfLayoutKit_shape_of_xyz_pfunc
};
GlyphRef LayoutKitType::_c_shape_of_xyz(Glyph_in gx, Glyph_in gy, Glyph_in gz) {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[4];
    _arg[1].u_objref = gx;
    _arg[2].u_objref = gy;
    _arg[3].u_objref = gz;
    _b.invoke(this, _XfLayoutKit_shape_of_xyz_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_strut_pdesc[6] = { 5, 60, 61, 49, 49, 49 };
MarshalBuffer::ArgMarshal _XfLayoutKit_strut_pfunc[] = {
    &FontStub::_create,
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_strut_pinfo = {
    &_XfLayoutKit_tid, 27, _XfLayoutKit_strut_pdesc, _XfLayoutKit_strut_pfunc
};
GlyphRef LayoutKitType::_c_strut(Font_in f, Coord natural, Coord stretch, Coord shrink) {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[5];
    _arg[1].u_objref = f;
    _arg[2].u_float = natural;
    _arg[3].u_float = stretch;
    _arg[4].u_float = shrink;
    _b.invoke(this, _XfLayoutKit_strut_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_hstrut_pdesc[7] = { 6, 60, 49, 49, 49, 49, 49 };
MarshalBuffer::ArgMarshal _XfLayoutKit_hstrut_pfunc[] = {
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_hstrut_pinfo = {
    &_XfLayoutKit_tid, 28, _XfLayoutKit_hstrut_pdesc, _XfLayoutKit_hstrut_pfunc
};
GlyphRef LayoutKitType::_c_hstrut(Coord right_bearing, Coord left_bearing, Coord natural, Coord stretch, Coord shrink) {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[6];
    _arg[1].u_float = right_bearing;
    _arg[2].u_float = left_bearing;
    _arg[3].u_float = natural;
    _arg[4].u_float = stretch;
    _arg[5].u_float = shrink;
    _b.invoke(this, _XfLayoutKit_hstrut_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_vstrut_pdesc[7] = { 6, 60, 49, 49, 49, 49, 49 };
MarshalBuffer::ArgMarshal _XfLayoutKit_vstrut_pfunc[] = {
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_vstrut_pinfo = {
    &_XfLayoutKit_tid, 29, _XfLayoutKit_vstrut_pdesc, _XfLayoutKit_vstrut_pfunc
};
GlyphRef LayoutKitType::_c_vstrut(Coord ascent, Coord descent, Coord natural, Coord stretch, Coord shrink) {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[6];
    _arg[1].u_float = ascent;
    _arg[2].u_float = descent;
    _arg[3].u_float = natural;
    _arg[4].u_float = stretch;
    _arg[5].u_float = shrink;
    _b.invoke(this, _XfLayoutKit_vstrut_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_spaces_pdesc[6] = { 5, 60, 33, 49, 61, 61 };
MarshalBuffer::ArgMarshal _XfLayoutKit_spaces_pfunc[] = {
    &FontStub::_create,
    &ColorStub::_create,
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_spaces_pinfo = {
    &_XfLayoutKit_tid, 30, _XfLayoutKit_spaces_pdesc, _XfLayoutKit_spaces_pfunc
};
GlyphRef LayoutKitType::_c_spaces(Long count, Coord each, Font_in f, Color_in c) {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[5];
    _arg[1].u_long = count;
    _arg[2].u_float = each;
    _arg[3].u_objref = f;
    _arg[4].u_objref = c;
    _b.invoke(this, _XfLayoutKit_spaces_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_center_pdesc[3] = { 2, 60, 61 };
MarshalBuffer::ArgMarshal _XfLayoutKit_center_pfunc[] = {
    &GlyphStub::_create,
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_center_pinfo = {
    &_XfLayoutKit_tid, 31, _XfLayoutKit_center_pdesc, _XfLayoutKit_center_pfunc
};
GlyphRef LayoutKitType::_c_center(Glyph_in g) {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = g;
    _b.invoke(this, _XfLayoutKit_center_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_center_aligned_pdesc[5] = { 4, 60, 61, 49, 49 };
MarshalBuffer::ArgMarshal _XfLayoutKit_center_aligned_pfunc[] = {
    &GlyphStub::_create,
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_center_aligned_pinfo = {
    &_XfLayoutKit_tid, 32, _XfLayoutKit_center_aligned_pdesc, _XfLayoutKit_center_aligned_pfunc
};
GlyphRef LayoutKitType::_c_center_aligned(Glyph_in g, Alignment x, Alignment y) {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[4];
    _arg[1].u_objref = g;
    _arg[2].u_float = x;
    _arg[3].u_float = y;
    _b.invoke(this, _XfLayoutKit_center_aligned_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_center_axis_pdesc[5] = { 4, 60, 61, 33, 49 };
MarshalBuffer::ArgMarshal _XfLayoutKit_center_axis_pfunc[] = {
    &GlyphStub::_create,
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_center_axis_pinfo = {
    &_XfLayoutKit_tid, 33, _XfLayoutKit_center_axis_pdesc, _XfLayoutKit_center_axis_pfunc
};
GlyphRef LayoutKitType::_c_center_axis(Glyph_in g, Axis a, Alignment align) {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[4];
    _arg[1].u_objref = g;
    _arg[2].u_long = a;
    _arg[3].u_float = align;
    _b.invoke(this, _XfLayoutKit_center_axis_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_hcenter_pdesc[3] = { 2, 60, 61 };
MarshalBuffer::ArgMarshal _XfLayoutKit_hcenter_pfunc[] = {
    &GlyphStub::_create,
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_hcenter_pinfo = {
    &_XfLayoutKit_tid, 34, _XfLayoutKit_hcenter_pdesc, _XfLayoutKit_hcenter_pfunc
};
GlyphRef LayoutKitType::_c_hcenter(Glyph_in g) {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = g;
    _b.invoke(this, _XfLayoutKit_hcenter_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_hcenter_aligned_pdesc[4] = { 3, 60, 61, 49 };
MarshalBuffer::ArgMarshal _XfLayoutKit_hcenter_aligned_pfunc[] = {
    &GlyphStub::_create,
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_hcenter_aligned_pinfo = {
    &_XfLayoutKit_tid, 35, _XfLayoutKit_hcenter_aligned_pdesc, _XfLayoutKit_hcenter_aligned_pfunc
};
GlyphRef LayoutKitType::_c_hcenter_aligned(Glyph_in g, Alignment x) {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[3];
    _arg[1].u_objref = g;
    _arg[2].u_float = x;
    _b.invoke(this, _XfLayoutKit_hcenter_aligned_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_vcenter_pdesc[3] = { 2, 60, 61 };
MarshalBuffer::ArgMarshal _XfLayoutKit_vcenter_pfunc[] = {
    &GlyphStub::_create,
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_vcenter_pinfo = {
    &_XfLayoutKit_tid, 36, _XfLayoutKit_vcenter_pdesc, _XfLayoutKit_vcenter_pfunc
};
GlyphRef LayoutKitType::_c_vcenter(Glyph_in g) {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = g;
    _b.invoke(this, _XfLayoutKit_vcenter_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_vcenter_aligned_pdesc[4] = { 3, 60, 61, 49 };
MarshalBuffer::ArgMarshal _XfLayoutKit_vcenter_aligned_pfunc[] = {
    &GlyphStub::_create,
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_vcenter_aligned_pinfo = {
    &_XfLayoutKit_tid, 37, _XfLayoutKit_vcenter_aligned_pdesc, _XfLayoutKit_vcenter_aligned_pfunc
};
GlyphRef LayoutKitType::_c_vcenter_aligned(Glyph_in g, Alignment y) {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[3];
    _arg[1].u_objref = g;
    _arg[2].u_float = y;
    _b.invoke(this, _XfLayoutKit_vcenter_aligned_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_fixed_pdesc[5] = { 4, 60, 61, 49, 49 };
MarshalBuffer::ArgMarshal _XfLayoutKit_fixed_pfunc[] = {
    &GlyphStub::_create,
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_fixed_pinfo = {
    &_XfLayoutKit_tid, 38, _XfLayoutKit_fixed_pdesc, _XfLayoutKit_fixed_pfunc
};
GlyphRef LayoutKitType::_c_fixed(Glyph_in g, Coord x, Coord y) {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[4];
    _arg[1].u_objref = g;
    _arg[2].u_float = x;
    _arg[3].u_float = y;
    _b.invoke(this, _XfLayoutKit_fixed_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_fixed_axis_pdesc[5] = { 4, 60, 61, 33, 49 };
MarshalBuffer::ArgMarshal _XfLayoutKit_fixed_axis_pfunc[] = {
    &GlyphStub::_create,
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_fixed_axis_pinfo = {
    &_XfLayoutKit_tid, 39, _XfLayoutKit_fixed_axis_pdesc, _XfLayoutKit_fixed_axis_pfunc
};
GlyphRef LayoutKitType::_c_fixed_axis(Glyph_in g, Axis a, Coord size) {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[4];
    _arg[1].u_objref = g;
    _arg[2].u_long = a;
    _arg[3].u_float = size;
    _b.invoke(this, _XfLayoutKit_fixed_axis_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_hfixed_pdesc[4] = { 3, 60, 61, 49 };
MarshalBuffer::ArgMarshal _XfLayoutKit_hfixed_pfunc[] = {
    &GlyphStub::_create,
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_hfixed_pinfo = {
    &_XfLayoutKit_tid, 40, _XfLayoutKit_hfixed_pdesc, _XfLayoutKit_hfixed_pfunc
};
GlyphRef LayoutKitType::_c_hfixed(Glyph_in g, Coord x) {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[3];
    _arg[1].u_objref = g;
    _arg[2].u_float = x;
    _b.invoke(this, _XfLayoutKit_hfixed_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_vfixed_pdesc[4] = { 3, 60, 61, 49 };
MarshalBuffer::ArgMarshal _XfLayoutKit_vfixed_pfunc[] = {
    &GlyphStub::_create,
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_vfixed_pinfo = {
    &_XfLayoutKit_tid, 41, _XfLayoutKit_vfixed_pdesc, _XfLayoutKit_vfixed_pfunc
};
GlyphRef LayoutKitType::_c_vfixed(Glyph_in g, Coord y) {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[3];
    _arg[1].u_objref = g;
    _arg[2].u_float = y;
    _b.invoke(this, _XfLayoutKit_vfixed_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_flexible_pdesc[5] = { 4, 60, 61, 49, 49 };
MarshalBuffer::ArgMarshal _XfLayoutKit_flexible_pfunc[] = {
    &GlyphStub::_create,
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_flexible_pinfo = {
    &_XfLayoutKit_tid, 42, _XfLayoutKit_flexible_pdesc, _XfLayoutKit_flexible_pfunc
};
GlyphRef LayoutKitType::_c_flexible(Glyph_in g, Coord stretch, Coord shrink) {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[4];
    _arg[1].u_objref = g;
    _arg[2].u_float = stretch;
    _arg[3].u_float = shrink;
    _b.invoke(this, _XfLayoutKit_flexible_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_flexible_fil_pdesc[3] = { 2, 60, 61 };
MarshalBuffer::ArgMarshal _XfLayoutKit_flexible_fil_pfunc[] = {
    &GlyphStub::_create,
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_flexible_fil_pinfo = {
    &_XfLayoutKit_tid, 43, _XfLayoutKit_flexible_fil_pdesc, _XfLayoutKit_flexible_fil_pfunc
};
GlyphRef LayoutKitType::_c_flexible_fil(Glyph_in g) {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = g;
    _b.invoke(this, _XfLayoutKit_flexible_fil_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_flexible_axis_pdesc[6] = { 5, 60, 61, 33, 49, 49 };
MarshalBuffer::ArgMarshal _XfLayoutKit_flexible_axis_pfunc[] = {
    &GlyphStub::_create,
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_flexible_axis_pinfo = {
    &_XfLayoutKit_tid, 44, _XfLayoutKit_flexible_axis_pdesc, _XfLayoutKit_flexible_axis_pfunc
};
GlyphRef LayoutKitType::_c_flexible_axis(Glyph_in g, Axis a, Coord stretch, Coord shrink) {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[5];
    _arg[1].u_objref = g;
    _arg[2].u_long = a;
    _arg[3].u_float = stretch;
    _arg[4].u_float = shrink;
    _b.invoke(this, _XfLayoutKit_flexible_axis_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_hflexible_pdesc[5] = { 4, 60, 61, 49, 49 };
MarshalBuffer::ArgMarshal _XfLayoutKit_hflexible_pfunc[] = {
    &GlyphStub::_create,
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_hflexible_pinfo = {
    &_XfLayoutKit_tid, 45, _XfLayoutKit_hflexible_pdesc, _XfLayoutKit_hflexible_pfunc
};
GlyphRef LayoutKitType::_c_hflexible(Glyph_in g, Coord stretch, Coord shrink) {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[4];
    _arg[1].u_objref = g;
    _arg[2].u_float = stretch;
    _arg[3].u_float = shrink;
    _b.invoke(this, _XfLayoutKit_hflexible_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_vflexible_pdesc[5] = { 4, 60, 61, 49, 49 };
MarshalBuffer::ArgMarshal _XfLayoutKit_vflexible_pfunc[] = {
    &GlyphStub::_create,
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_vflexible_pinfo = {
    &_XfLayoutKit_tid, 46, _XfLayoutKit_vflexible_pdesc, _XfLayoutKit_vflexible_pfunc
};
GlyphRef LayoutKitType::_c_vflexible(Glyph_in g, Coord stretch, Coord shrink) {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[4];
    _arg[1].u_objref = g;
    _arg[2].u_float = stretch;
    _arg[3].u_float = shrink;
    _b.invoke(this, _XfLayoutKit_vflexible_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_natural_pdesc[5] = { 4, 60, 61, 49, 49 };
MarshalBuffer::ArgMarshal _XfLayoutKit_natural_pfunc[] = {
    &GlyphStub::_create,
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_natural_pinfo = {
    &_XfLayoutKit_tid, 47, _XfLayoutKit_natural_pdesc, _XfLayoutKit_natural_pfunc
};
GlyphRef LayoutKitType::_c_natural(Glyph_in g, Coord x, Coord y) {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[4];
    _arg[1].u_objref = g;
    _arg[2].u_float = x;
    _arg[3].u_float = y;
    _b.invoke(this, _XfLayoutKit_natural_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_natural_axis_pdesc[5] = { 4, 60, 61, 33, 49 };
MarshalBuffer::ArgMarshal _XfLayoutKit_natural_axis_pfunc[] = {
    &GlyphStub::_create,
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_natural_axis_pinfo = {
    &_XfLayoutKit_tid, 48, _XfLayoutKit_natural_axis_pdesc, _XfLayoutKit_natural_axis_pfunc
};
GlyphRef LayoutKitType::_c_natural_axis(Glyph_in g, Axis a, Coord size) {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[4];
    _arg[1].u_objref = g;
    _arg[2].u_long = a;
    _arg[3].u_float = size;
    _b.invoke(this, _XfLayoutKit_natural_axis_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_hnatural_pdesc[4] = { 3, 60, 61, 49 };
MarshalBuffer::ArgMarshal _XfLayoutKit_hnatural_pfunc[] = {
    &GlyphStub::_create,
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_hnatural_pinfo = {
    &_XfLayoutKit_tid, 49, _XfLayoutKit_hnatural_pdesc, _XfLayoutKit_hnatural_pfunc
};
GlyphRef LayoutKitType::_c_hnatural(Glyph_in g, Coord x) {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[3];
    _arg[1].u_objref = g;
    _arg[2].u_float = x;
    _b.invoke(this, _XfLayoutKit_hnatural_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_vnatural_pdesc[4] = { 3, 60, 61, 49 };
MarshalBuffer::ArgMarshal _XfLayoutKit_vnatural_pfunc[] = {
    &GlyphStub::_create,
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_vnatural_pinfo = {
    &_XfLayoutKit_tid, 50, _XfLayoutKit_vnatural_pdesc, _XfLayoutKit_vnatural_pfunc
};
GlyphRef LayoutKitType::_c_vnatural(Glyph_in g, Coord y) {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[3];
    _arg[1].u_objref = g;
    _arg[2].u_float = y;
    _b.invoke(this, _XfLayoutKit_vnatural_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_margin_pdesc[4] = { 3, 60, 61, 49 };
MarshalBuffer::ArgMarshal _XfLayoutKit_margin_pfunc[] = {
    &GlyphStub::_create,
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_margin_pinfo = {
    &_XfLayoutKit_tid, 51, _XfLayoutKit_margin_pdesc, _XfLayoutKit_margin_pfunc
};
GlyphRef LayoutKitType::_c_margin(Glyph_in g, Coord all) {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[3];
    _arg[1].u_objref = g;
    _arg[2].u_float = all;
    _b.invoke(this, _XfLayoutKit_margin_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_margin_lrbt_pdesc[7] = { 6, 60, 61, 49, 49, 49, 49 };
MarshalBuffer::ArgMarshal _XfLayoutKit_margin_lrbt_pfunc[] = {
    &GlyphStub::_create,
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_margin_lrbt_pinfo = {
    &_XfLayoutKit_tid, 52, _XfLayoutKit_margin_lrbt_pdesc, _XfLayoutKit_margin_lrbt_pfunc
};
GlyphRef LayoutKitType::_c_margin_lrbt(Glyph_in g, Coord lmargin, Coord rmargin, Coord bmargin, Coord tmargin) {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[6];
    _arg[1].u_objref = g;
    _arg[2].u_float = lmargin;
    _arg[3].u_float = rmargin;
    _arg[4].u_float = bmargin;
    _arg[5].u_float = tmargin;
    _b.invoke(this, _XfLayoutKit_margin_lrbt_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_margin_lrbt_flexible_pdesc[15] = { 14, 60, 61, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49 };
MarshalBuffer::ArgMarshal _XfLayoutKit_margin_lrbt_flexible_pfunc[] = {
    &GlyphStub::_create,
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_margin_lrbt_flexible_pinfo = {
    &_XfLayoutKit_tid, 53, _XfLayoutKit_margin_lrbt_flexible_pdesc, _XfLayoutKit_margin_lrbt_flexible_pfunc
};
GlyphRef LayoutKitType::_c_margin_lrbt_flexible(Glyph_in g, Coord lmargin, Coord lstretch, Coord lshrink, Coord rmargin, Coord rstretch, Coord rshrink, Coord bmargin, Coord bstretch, Coord bshrink, Coord tmargin, Coord tstretch, Coord tshrink) {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[14];
    _arg[1].u_objref = g;
    _arg[2].u_float = lmargin;
    _arg[3].u_float = lstretch;
    _arg[4].u_float = lshrink;
    _arg[5].u_float = rmargin;
    _arg[6].u_float = rstretch;
    _arg[7].u_float = rshrink;
    _arg[8].u_float = bmargin;
    _arg[9].u_float = bstretch;
    _arg[10].u_float = bshrink;
    _arg[11].u_float = tmargin;
    _arg[12].u_float = tstretch;
    _arg[13].u_float = tshrink;
    _b.invoke(this, _XfLayoutKit_margin_lrbt_flexible_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_hmargin_pdesc[4] = { 3, 60, 61, 49 };
MarshalBuffer::ArgMarshal _XfLayoutKit_hmargin_pfunc[] = {
    &GlyphStub::_create,
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_hmargin_pinfo = {
    &_XfLayoutKit_tid, 54, _XfLayoutKit_hmargin_pdesc, _XfLayoutKit_hmargin_pfunc
};
GlyphRef LayoutKitType::_c_hmargin(Glyph_in g, Coord both) {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[3];
    _arg[1].u_objref = g;
    _arg[2].u_float = both;
    _b.invoke(this, _XfLayoutKit_hmargin_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_hmargin_lr_pdesc[5] = { 4, 60, 61, 49, 49 };
MarshalBuffer::ArgMarshal _XfLayoutKit_hmargin_lr_pfunc[] = {
    &GlyphStub::_create,
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_hmargin_lr_pinfo = {
    &_XfLayoutKit_tid, 55, _XfLayoutKit_hmargin_lr_pdesc, _XfLayoutKit_hmargin_lr_pfunc
};
GlyphRef LayoutKitType::_c_hmargin_lr(Glyph_in g, Coord lmargin, Coord rmargin) {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[4];
    _arg[1].u_objref = g;
    _arg[2].u_float = lmargin;
    _arg[3].u_float = rmargin;
    _b.invoke(this, _XfLayoutKit_hmargin_lr_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_hmargin_lr_flexible_pdesc[9] = { 8, 60, 61, 49, 49, 49, 49, 49, 49 };
MarshalBuffer::ArgMarshal _XfLayoutKit_hmargin_lr_flexible_pfunc[] = {
    &GlyphStub::_create,
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_hmargin_lr_flexible_pinfo = {
    &_XfLayoutKit_tid, 56, _XfLayoutKit_hmargin_lr_flexible_pdesc, _XfLayoutKit_hmargin_lr_flexible_pfunc
};
GlyphRef LayoutKitType::_c_hmargin_lr_flexible(Glyph_in g, Coord lmargin, Coord lstretch, Coord lshrink, Coord rmargin, Coord rstretch, Coord rshrink) {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[8];
    _arg[1].u_objref = g;
    _arg[2].u_float = lmargin;
    _arg[3].u_float = lstretch;
    _arg[4].u_float = lshrink;
    _arg[5].u_float = rmargin;
    _arg[6].u_float = rstretch;
    _arg[7].u_float = rshrink;
    _b.invoke(this, _XfLayoutKit_hmargin_lr_flexible_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_vmargin_pdesc[4] = { 3, 60, 61, 49 };
MarshalBuffer::ArgMarshal _XfLayoutKit_vmargin_pfunc[] = {
    &GlyphStub::_create,
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_vmargin_pinfo = {
    &_XfLayoutKit_tid, 57, _XfLayoutKit_vmargin_pdesc, _XfLayoutKit_vmargin_pfunc
};
GlyphRef LayoutKitType::_c_vmargin(Glyph_in g, Coord both) {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[3];
    _arg[1].u_objref = g;
    _arg[2].u_float = both;
    _b.invoke(this, _XfLayoutKit_vmargin_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_vmargin_bt_pdesc[5] = { 4, 60, 61, 49, 49 };
MarshalBuffer::ArgMarshal _XfLayoutKit_vmargin_bt_pfunc[] = {
    &GlyphStub::_create,
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_vmargin_bt_pinfo = {
    &_XfLayoutKit_tid, 58, _XfLayoutKit_vmargin_bt_pdesc, _XfLayoutKit_vmargin_bt_pfunc
};
GlyphRef LayoutKitType::_c_vmargin_bt(Glyph_in g, Coord bmargin, Coord tmargin) {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[4];
    _arg[1].u_objref = g;
    _arg[2].u_float = bmargin;
    _arg[3].u_float = tmargin;
    _b.invoke(this, _XfLayoutKit_vmargin_bt_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_vmargin_bt_flexible_pdesc[9] = { 8, 60, 61, 49, 49, 49, 49, 49, 49 };
MarshalBuffer::ArgMarshal _XfLayoutKit_vmargin_bt_flexible_pfunc[] = {
    &GlyphStub::_create,
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_vmargin_bt_flexible_pinfo = {
    &_XfLayoutKit_tid, 59, _XfLayoutKit_vmargin_bt_flexible_pdesc, _XfLayoutKit_vmargin_bt_flexible_pfunc
};
GlyphRef LayoutKitType::_c_vmargin_bt_flexible(Glyph_in g, Coord bmargin, Coord bstretch, Coord bshrink, Coord tmargin, Coord tstretch, Coord tshrink) {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[8];
    _arg[1].u_objref = g;
    _arg[2].u_float = bmargin;
    _arg[3].u_float = bstretch;
    _arg[4].u_float = bshrink;
    _arg[5].u_float = tmargin;
    _arg[6].u_float = tstretch;
    _arg[7].u_float = tshrink;
    _b.invoke(this, _XfLayoutKit_vmargin_bt_flexible_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_lmargin_pdesc[4] = { 3, 60, 61, 49 };
MarshalBuffer::ArgMarshal _XfLayoutKit_lmargin_pfunc[] = {
    &GlyphStub::_create,
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_lmargin_pinfo = {
    &_XfLayoutKit_tid, 60, _XfLayoutKit_lmargin_pdesc, _XfLayoutKit_lmargin_pfunc
};
GlyphRef LayoutKitType::_c_lmargin(Glyph_in g, Coord natural) {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[3];
    _arg[1].u_objref = g;
    _arg[2].u_float = natural;
    _b.invoke(this, _XfLayoutKit_lmargin_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_lmargin_flexible_pdesc[6] = { 5, 8, 61, 49, 49, 49 };
MarshalBuffer::ArgMarshal _XfLayoutKit_lmargin_flexible_pfunc[] = {
    &GlyphStub::_create,
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_lmargin_flexible_pinfo = {
    &_XfLayoutKit_tid, 61, _XfLayoutKit_lmargin_flexible_pdesc, _XfLayoutKit_lmargin_flexible_pfunc
};
GlyphRef LayoutKitType::_c_lmargin_flexible(Glyph_in g, Coord natural, Coord stretch, Coord shrink) {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[5];
    _arg[1].u_objref = g;
    _arg[2].u_float = natural;
    _arg[3].u_float = stretch;
    _arg[4].u_float = shrink;
    _b.invoke(this, _XfLayoutKit_lmargin_flexible_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_rmargin_pdesc[4] = { 3, 60, 61, 49 };
MarshalBuffer::ArgMarshal _XfLayoutKit_rmargin_pfunc[] = {
    &GlyphStub::_create,
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_rmargin_pinfo = {
    &_XfLayoutKit_tid, 62, _XfLayoutKit_rmargin_pdesc, _XfLayoutKit_rmargin_pfunc
};
GlyphRef LayoutKitType::_c_rmargin(Glyph_in g, Coord natural) {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[3];
    _arg[1].u_objref = g;
    _arg[2].u_float = natural;
    _b.invoke(this, _XfLayoutKit_rmargin_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_rmargin_flexible_pdesc[6] = { 5, 60, 61, 49, 49, 49 };
MarshalBuffer::ArgMarshal _XfLayoutKit_rmargin_flexible_pfunc[] = {
    &GlyphStub::_create,
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_rmargin_flexible_pinfo = {
    &_XfLayoutKit_tid, 63, _XfLayoutKit_rmargin_flexible_pdesc, _XfLayoutKit_rmargin_flexible_pfunc
};
GlyphRef LayoutKitType::_c_rmargin_flexible(Glyph_in g, Coord natural, Coord stretch, Coord shrink) {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[5];
    _arg[1].u_objref = g;
    _arg[2].u_float = natural;
    _arg[3].u_float = stretch;
    _arg[4].u_float = shrink;
    _b.invoke(this, _XfLayoutKit_rmargin_flexible_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_bmargin_pdesc[4] = { 3, 60, 61, 49 };
MarshalBuffer::ArgMarshal _XfLayoutKit_bmargin_pfunc[] = {
    &GlyphStub::_create,
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_bmargin_pinfo = {
    &_XfLayoutKit_tid, 64, _XfLayoutKit_bmargin_pdesc, _XfLayoutKit_bmargin_pfunc
};
GlyphRef LayoutKitType::_c_bmargin(Glyph_in g, Coord natural) {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[3];
    _arg[1].u_objref = g;
    _arg[2].u_float = natural;
    _b.invoke(this, _XfLayoutKit_bmargin_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_bmargin_flexible_pdesc[6] = { 5, 60, 61, 49, 49, 49 };
MarshalBuffer::ArgMarshal _XfLayoutKit_bmargin_flexible_pfunc[] = {
    &GlyphStub::_create,
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_bmargin_flexible_pinfo = {
    &_XfLayoutKit_tid, 65, _XfLayoutKit_bmargin_flexible_pdesc, _XfLayoutKit_bmargin_flexible_pfunc
};
GlyphRef LayoutKitType::_c_bmargin_flexible(Glyph_in g, Coord natural, Coord stretch, Coord shrink) {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[5];
    _arg[1].u_objref = g;
    _arg[2].u_float = natural;
    _arg[3].u_float = stretch;
    _arg[4].u_float = shrink;
    _b.invoke(this, _XfLayoutKit_bmargin_flexible_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_tmargin_pdesc[4] = { 3, 60, 61, 49 };
MarshalBuffer::ArgMarshal _XfLayoutKit_tmargin_pfunc[] = {
    &GlyphStub::_create,
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_tmargin_pinfo = {
    &_XfLayoutKit_tid, 66, _XfLayoutKit_tmargin_pdesc, _XfLayoutKit_tmargin_pfunc
};
GlyphRef LayoutKitType::_c_tmargin(Glyph_in g, Coord natural) {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[3];
    _arg[1].u_objref = g;
    _arg[2].u_float = natural;
    _b.invoke(this, _XfLayoutKit_tmargin_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfLayoutKit_tmargin_flexible_pdesc[6] = { 5, 60, 61, 49, 49, 49 };
MarshalBuffer::ArgMarshal _XfLayoutKit_tmargin_flexible_pfunc[] = {
    &GlyphStub::_create,
    &GlyphStub::_create
};
MarshalBuffer::ArgInfo _XfLayoutKit_tmargin_flexible_pinfo = {
    &_XfLayoutKit_tid, 67, _XfLayoutKit_tmargin_flexible_pdesc, _XfLayoutKit_tmargin_flexible_pfunc
};
GlyphRef LayoutKitType::_c_tmargin_flexible(Glyph_in g, Coord natural, Coord stretch, Coord shrink) {
    MarshalBuffer _b;
    extern TypeObjId _XfLayoutKit_tid;
    MarshalBuffer::ArgValue _arg[5];
    _arg[1].u_objref = g;
    _arg[2].u_float = natural;
    _arg[3].u_float = stretch;
    _arg[4].u_float = shrink;
    _b.invoke(this, _XfLayoutKit_tmargin_flexible_pinfo, _arg);
    return (GlyphRef)_arg[0].u_objref;
}
//+

//+ ScrollBox::%init,type+dii,client
ScrollBoxType::ScrollBoxType() { }
ScrollBoxType::~ScrollBoxType() { }
void* ScrollBoxType::_this() { return this; }

extern TypeObj_Descriptor _XfBoolean_type;

TypeObj_OpData _XfScrollBox_methods[] = {
    { "shown", &_XfBoolean_type, 1 },
    { "first_shown", &_XfGlyphOffset_type, 0 },
    { "last_shown", &_XfGlyphOffset_type, 0 },
    { 0, 0, 0 }
};
TypeObj_ParamData _XfScrollBox_params[] = {
    /* shown */
        { "off", 0, &_XfGlyphOffset_type }
};
extern TypeObj_Descriptor _XfGlyph_type;
TypeObj_Descriptor* _XfScrollBox_parents[] = { &_XfGlyph_type, nil };
extern TypeObjId _XfScrollBox_tid;
extern void _XfScrollBox_receive(BaseObjectRef, ULong, MarshalBuffer&);
TypeObj_Descriptor _XfScrollBox_type = {
    /* type */ 0,
    /* id */ &_XfScrollBox_tid,
    "ScrollBox",
    _XfScrollBox_parents, /* offsets */ nil, /* excepts */ nil,
    _XfScrollBox_methods, _XfScrollBox_params,
    &_XfScrollBox_receive
};

ScrollBoxRef ScrollBox::_narrow(BaseObjectRef o) {
    return (ScrollBoxRef)_BaseObject_tnarrow(
        o, _XfScrollBox_tid, &ScrollBoxStub::_create
    );
}
TypeObjId ScrollBoxType::_tid() { return _XfScrollBox_tid; }
void _XfScrollBox_receive(BaseObjectRef _object, ULong _m, MarshalBuffer& _b) {
    extern TypeObjId _XfScrollBox_tid;
    ScrollBoxRef _this = (ScrollBoxRef)_BaseObject_tcast(_object, _XfScrollBox_tid);
    switch (_m) {
        case /* shown */ 0: {
            extern MarshalBuffer::ArgInfo _XfScrollBox_shown_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            GlyphOffsetRef off;
            _arg[1].u_addr = &off;
            _b.receive(_XfScrollBox_shown_pinfo, _arg);
            _arg[0].u_boolean = _this->shown(off);
            _b.reply(_XfScrollBox_shown_pinfo, _arg);
            break;
        }
        case /* first_shown */ 1: {
            extern MarshalBuffer::ArgInfo _XfScrollBox_first_shown_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_objref = _this->_c_first_shown();
            _b.reply(_XfScrollBox_first_shown_pinfo, _arg);
            break;
        }
        case /* last_shown */ 2: {
            extern MarshalBuffer::ArgInfo _XfScrollBox_last_shown_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_objref = _this->_c_last_shown();
            _b.reply(_XfScrollBox_last_shown_pinfo, _arg);
            break;
        }
    }
}
ScrollBoxStub::ScrollBoxStub(Exchange* e) { exch_ = e; }
ScrollBoxStub::~ScrollBoxStub() { }
BaseObjectRef ScrollBoxStub::_create(Exchange* e) {
    return (BaseObjectRef)(void*)new ScrollBoxStub(e);
}
Exchange* ScrollBoxStub::_exchange() {
    return exch_;
}
MarshalBuffer::ArgDesc _XfScrollBox_shown_pdesc[3] = { 2, 12, 61 };
MarshalBuffer::ArgMarshal _XfScrollBox_shown_pfunc[] = {
    &GlyphOffsetStub::_create,

};
MarshalBuffer::ArgInfo _XfScrollBox_shown_pinfo = {
    &_XfScrollBox_tid, 0, _XfScrollBox_shown_pdesc, _XfScrollBox_shown_pfunc
};
Boolean ScrollBoxType::shown(GlyphOffset_in off) {
    MarshalBuffer _b;
    extern TypeObjId _XfScrollBox_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = off;
    _b.invoke(this, _XfScrollBox_shown_pinfo, _arg);
    return _arg[0].u_boolean;
}
MarshalBuffer::ArgDesc _XfScrollBox_first_shown_pdesc[2] = { 1, 60 };
MarshalBuffer::ArgMarshal _XfScrollBox_first_shown_pfunc[] = {
    &GlyphOffsetStub::_create
};
MarshalBuffer::ArgInfo _XfScrollBox_first_shown_pinfo = {
    &_XfScrollBox_tid, 1, _XfScrollBox_first_shown_pdesc, _XfScrollBox_first_shown_pfunc
};
GlyphOffsetRef ScrollBoxType::_c_first_shown() {
    MarshalBuffer _b;
    extern TypeObjId _XfScrollBox_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfScrollBox_first_shown_pinfo, _arg);
    return (GlyphOffsetRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfScrollBox_last_shown_pdesc[2] = { 1, 60 };
MarshalBuffer::ArgMarshal _XfScrollBox_last_shown_pfunc[] = {
    &GlyphOffsetStub::_create
};
MarshalBuffer::ArgInfo _XfScrollBox_last_shown_pinfo = {
    &_XfScrollBox_tid, 2, _XfScrollBox_last_shown_pdesc, _XfScrollBox_last_shown_pfunc
};
GlyphOffsetRef ScrollBoxType::_c_last_shown() {
    MarshalBuffer _b;
    extern TypeObjId _XfScrollBox_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfScrollBox_last_shown_pinfo, _arg);
    return (GlyphOffsetRef)_arg[0].u_objref;
}
//+
