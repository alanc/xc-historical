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
 * Stubs for drawing interfaces
 */

#include <X11/Fresco/drawing.h>
#include <X11/Fresco/Impls/Xdrawing.h>
#include <X11/Fresco/Impls/Xpainter.h>
#include <X11/Fresco/Impls/Xraster.h>
#include <X11/Fresco/Ox/request.h>
#include <X11/Fresco/Ox/stub.h>
#include <X11/Fresco/Ox/schema.h>

//+ Brush::%init,type+dii,client
BrushType::BrushType() { }
BrushType::~BrushType() { }
void* BrushType::_this() { return this; }

extern TypeObj_Descriptor _XfBoolean_type, _XfBrush_type, _XfULong_type;

TypeObj_OpData _XfBrush_methods[] = {
    { "equal", &_XfBoolean_type, 1 },
    { "hash", &_XfULong_type, 0 },
    { 0, 0, 0 }
};
TypeObj_ParamData _XfBrush_params[] = {
    /* equal */
        { "b", 0, &_XfBrush_type }
};
extern TypeObj_Descriptor _XfFrescoObject_type;
TypeObj_Descriptor* _XfBrush_parents[] = { &_XfFrescoObject_type, nil };
extern TypeObjId _XfBrush_tid;
extern void _XfBrush_receive(BaseObjectRef, ULong, MarshalBuffer&);
TypeObj_Descriptor _XfBrush_type = {
    /* type */ 0,
    /* id */ &_XfBrush_tid,
    "Brush",
    _XfBrush_parents, /* offsets */ nil, /* excepts */ nil,
    _XfBrush_methods, _XfBrush_params,
    &_XfBrush_receive
};

BrushRef Brush::_narrow(BaseObjectRef o) {
    return (BrushRef)_BaseObject_tnarrow(
        o, _XfBrush_tid, &BrushStub::_create
    );
}
TypeObjId BrushType::_tid() { return _XfBrush_tid; }
void _XfBrush_receive(BaseObjectRef _object, ULong _m, MarshalBuffer& _b) {
    extern TypeObjId _XfBrush_tid;
    BrushRef _this = (BrushRef)_BaseObject_tcast(_object, _XfBrush_tid);
    switch (_m) {
        case /* equal */ 0: {
            extern MarshalBuffer::ArgInfo _XfBrush_equal_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            BrushRef b;
            _arg[1].u_addr = &b;
            _b.receive(_XfBrush_equal_pinfo, _arg);
            _arg[0].u_boolean = _this->equal(b);
            _b.reply(_XfBrush_equal_pinfo, _arg);
            break;
        }
        case /* hash */ 1: {
            extern MarshalBuffer::ArgInfo _XfBrush_hash_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_unsigned_long = _this->hash();
            _b.reply(_XfBrush_hash_pinfo, _arg);
            break;
        }
    }
}
BrushStub::BrushStub(Exchange* e) { exch_ = e; }
BrushStub::~BrushStub() { }
BaseObjectRef BrushStub::_create(Exchange* e) {
    return (BaseObjectRef)(void*)new BrushStub(e);
}
Exchange* BrushStub::_exchange() {
    return exch_;
}
MarshalBuffer::ArgDesc _XfBrush_equal_pdesc[3] = { 2, 12, 61 };
MarshalBuffer::ArgMarshal _XfBrush_equal_pfunc[] = {
    &BrushStub::_create,

};
MarshalBuffer::ArgInfo _XfBrush_equal_pinfo = {
    &_XfBrush_tid, 0, _XfBrush_equal_pdesc, _XfBrush_equal_pfunc
};
Boolean BrushType::equal(Brush_in b) {
    MarshalBuffer _b;
    extern TypeObjId _XfBrush_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = b;
    _b.invoke(this, _XfBrush_equal_pinfo, _arg);
    return _arg[0].u_boolean;
}
MarshalBuffer::ArgDesc _XfBrush_hash_pdesc[2] = { 1, 36 };
MarshalBuffer::ArgInfo _XfBrush_hash_pinfo = {
    &_XfBrush_tid, 1, _XfBrush_hash_pdesc, 0
};
ULong BrushType::hash() {
    MarshalBuffer _b;
    extern TypeObjId _XfBrush_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfBrush_hash_pinfo, _arg);
    return _arg[0].u_unsigned_long;
}
//+

//+ BrushImpl(Brush)
extern TypeObj_Descriptor _XfBrush_type;
TypeObj_Descriptor* _XfBrushImpl_parents[] = { &_XfBrush_type, nil };
extern TypeObjId _XfBrushImpl_tid;
TypeObj_Descriptor _XfBrushImpl_type = {
    /* type */ 0,
    /* id */ &_XfBrushImpl_tid,
    "BrushImpl",
    _XfBrushImpl_parents, /* offsets */ nil, /* excepts */ nil,
    /* methods */ nil, /* params */ nil,
    /* receive */ nil
};

BrushImpl* BrushImpl::_narrow(BaseObjectRef o) {
    return (BrushImpl*)_BaseObject_tnarrow(
        o, _XfBrushImpl_tid, 0
    );
}
TypeObjId BrushImpl::_tid() { return _XfBrushImpl_tid; }
//+

//+ Color::%init,type+dii,client
ColorType::ColorType() { }
ColorType::~ColorType() { }
void* ColorType::_this() { return this; }

extern TypeObj_Descriptor _Xfvoid_type, _XfColor_Intensity_type, _XfColor_type;

TypeObj_OpData _XfColor_methods[] = {
    { "rgb", &_Xfvoid_type, 3 },
    { "equal", &_XfBoolean_type, 1 },
    { "hash", &_XfULong_type, 0 },
    { 0, 0, 0 }
};
TypeObj_ParamData _XfColor_params[] = {
    /* rgb */
        { "r", 1, &_XfColor_Intensity_type },
        { "g", 1, &_XfColor_Intensity_type },
        { "b", 1, &_XfColor_Intensity_type },
    /* equal */
        { "c", 0, &_XfColor_type }
};
extern TypeObj_Descriptor _XfFrescoObject_type;
TypeObj_Descriptor* _XfColor_parents[] = { &_XfFrescoObject_type, nil };
extern TypeObjId _XfColor_tid;
extern void _XfColor_receive(BaseObjectRef, ULong, MarshalBuffer&);
TypeObj_Descriptor _XfColor_type = {
    /* type */ 0,
    /* id */ &_XfColor_tid,
    "Color",
    _XfColor_parents, /* offsets */ nil, /* excepts */ nil,
    _XfColor_methods, _XfColor_params,
    &_XfColor_receive
};

ColorRef Color::_narrow(BaseObjectRef o) {
    return (ColorRef)_BaseObject_tnarrow(
        o, _XfColor_tid, &ColorStub::_create
    );
}
TypeObjId ColorType::_tid() { return _XfColor_tid; }
void _XfColor_receive(BaseObjectRef _object, ULong _m, MarshalBuffer& _b) {
    extern TypeObjId _XfColor_tid;
    ColorRef _this = (ColorRef)_BaseObject_tcast(_object, _XfColor_tid);
    switch (_m) {
        case /* rgb */ 0: {
            extern MarshalBuffer::ArgInfo _XfColor_rgb_pinfo;
            MarshalBuffer::ArgValue _arg[4];
            Color::Intensity r;
            _arg[1].u_addr = &r;
            Color::Intensity g;
            _arg[2].u_addr = &g;
            Color::Intensity b;
            _arg[3].u_addr = &b;
            _b.receive(_XfColor_rgb_pinfo, _arg);
            _this->rgb(r, g, b);
            _b.reply(_XfColor_rgb_pinfo, _arg);
            break;
        }
        case /* equal */ 1: {
            extern MarshalBuffer::ArgInfo _XfColor_equal_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            ColorRef c;
            _arg[1].u_addr = &c;
            _b.receive(_XfColor_equal_pinfo, _arg);
            _arg[0].u_boolean = _this->equal(c);
            _b.reply(_XfColor_equal_pinfo, _arg);
            break;
        }
        case /* hash */ 2: {
            extern MarshalBuffer::ArgInfo _XfColor_hash_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_unsigned_long = _this->hash();
            _b.reply(_XfColor_hash_pinfo, _arg);
            break;
        }
    }
}
ColorStub::ColorStub(Exchange* e) { exch_ = e; }
ColorStub::~ColorStub() { }
BaseObjectRef ColorStub::_create(Exchange* e) {
    return (BaseObjectRef)(void*)new ColorStub(e);
}
Exchange* ColorStub::_exchange() {
    return exch_;
}
MarshalBuffer::ArgDesc _XfColor_rgb_pdesc[5] = { 4, 4, 50, 50, 50 };
MarshalBuffer::ArgInfo _XfColor_rgb_pinfo = {
    &_XfColor_tid, 0, _XfColor_rgb_pdesc, 0
};
void ColorType::rgb(Color::Intensity& r, Color::Intensity& g, Color::Intensity& b) {
    MarshalBuffer _b;
    extern TypeObjId _XfColor_tid;
    MarshalBuffer::ArgValue _arg[4];
    _arg[1].u_addr = &r;
    _arg[2].u_addr = &g;
    _arg[3].u_addr = &b;
    _b.invoke(this, _XfColor_rgb_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfColor_equal_pdesc[3] = { 2, 12, 61 };
MarshalBuffer::ArgMarshal _XfColor_equal_pfunc[] = {
    &ColorStub::_create,

};
MarshalBuffer::ArgInfo _XfColor_equal_pinfo = {
    &_XfColor_tid, 1, _XfColor_equal_pdesc, _XfColor_equal_pfunc
};
Boolean ColorType::equal(Color_in c) {
    MarshalBuffer _b;
    extern TypeObjId _XfColor_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = c;
    _b.invoke(this, _XfColor_equal_pinfo, _arg);
    return _arg[0].u_boolean;
}
MarshalBuffer::ArgDesc _XfColor_hash_pdesc[2] = { 1, 36 };
MarshalBuffer::ArgInfo _XfColor_hash_pinfo = {
    &_XfColor_tid, 2, _XfColor_hash_pdesc, 0
};
ULong ColorType::hash() {
    MarshalBuffer _b;
    extern TypeObjId _XfColor_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfColor_hash_pinfo, _arg);
    return _arg[0].u_unsigned_long;
}
//+

//+ ColorImpl(Color)
extern TypeObj_Descriptor _XfColor_type;
TypeObj_Descriptor* _XfColorImpl_parents[] = { &_XfColor_type, nil };
extern TypeObjId _XfColorImpl_tid;
TypeObj_Descriptor _XfColorImpl_type = {
    /* type */ 0,
    /* id */ &_XfColorImpl_tid,
    "ColorImpl",
    _XfColorImpl_parents, /* offsets */ nil, /* excepts */ nil,
    /* methods */ nil, /* params */ nil,
    /* receive */ nil
};

ColorImpl* ColorImpl::_narrow(BaseObjectRef o) {
    return (ColorImpl*)_BaseObject_tnarrow(
        o, _XfColorImpl_tid, 0
    );
}
TypeObjId ColorImpl::_tid() { return _XfColorImpl_tid; }
//+

//+ Font::%init,type+dii,client
FontType::FontType() { }
FontType::~FontType() { }
void* FontType::_this() { return this; }

extern TypeObj_Descriptor _XfFont_type, _XfCharString_type, _XfCoord_type, 
    _XfFont_Info_type, _XfCharCode_type;

TypeObj_OpData _XfFont_methods[] = {
    { "equal", &_XfBoolean_type, 1 },
    { "hash", &_XfULong_type, 0 },
    { "name", &_XfCharString_type, 0 },
    { "encoding", &_XfCharString_type, 0 },
    { "point_size", &_XfCoord_type, 0 },
    { "font_info", &_Xfvoid_type, 1 },
    { "char_info", &_Xfvoid_type, 2 },
    { "string_info", &_Xfvoid_type, 2 },
    { 0, 0, 0 }
};
TypeObj_ParamData _XfFont_params[] = {
    /* equal */
        { "f", 0, &_XfFont_type },
    /* font_info */
        { "i", 1, &_XfFont_Info_type },
    /* char_info */
        { "c", 0, &_XfCharCode_type },
        { "i", 1, &_XfFont_Info_type },
    /* string_info */
        { "s", 0, &_XfCharString_type },
        { "i", 1, &_XfFont_Info_type }
};
extern TypeObj_Descriptor _XfFrescoObject_type;
TypeObj_Descriptor* _XfFont_parents[] = { &_XfFrescoObject_type, nil };
extern TypeObjId _XfFont_tid;
extern void _XfFont_receive(BaseObjectRef, ULong, MarshalBuffer&);
TypeObj_Descriptor _XfFont_type = {
    /* type */ 0,
    /* id */ &_XfFont_tid,
    "Font",
    _XfFont_parents, /* offsets */ nil, /* excepts */ nil,
    _XfFont_methods, _XfFont_params,
    &_XfFont_receive
};

FontRef Font::_narrow(BaseObjectRef o) {
    return (FontRef)_BaseObject_tnarrow(
        o, _XfFont_tid, &FontStub::_create
    );
}
TypeObjId FontType::_tid() { return _XfFont_tid; }
void _XfFont_receive(BaseObjectRef _object, ULong _m, MarshalBuffer& _b) {
    extern TypeObjId _XfFont_tid;
    FontRef _this = (FontRef)_BaseObject_tcast(_object, _XfFont_tid);
    switch (_m) {
        case /* equal */ 0: {
            extern MarshalBuffer::ArgInfo _XfFont_equal_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            FontRef f;
            _arg[1].u_addr = &f;
            _b.receive(_XfFont_equal_pinfo, _arg);
            _arg[0].u_boolean = _this->equal(f);
            _b.reply(_XfFont_equal_pinfo, _arg);
            break;
        }
        case /* hash */ 1: {
            extern MarshalBuffer::ArgInfo _XfFont_hash_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_unsigned_long = _this->hash();
            _b.reply(_XfFont_hash_pinfo, _arg);
            break;
        }
        case /* name */ 2: {
            extern MarshalBuffer::ArgInfo _XfFont_name_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_objref = _this->_c_name();
            _b.reply(_XfFont_name_pinfo, _arg);
            break;
        }
        case /* encoding */ 3: {
            extern MarshalBuffer::ArgInfo _XfFont_encoding_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_objref = _this->_c_encoding();
            _b.reply(_XfFont_encoding_pinfo, _arg);
            break;
        }
        case /* point_size */ 4: {
            extern MarshalBuffer::ArgInfo _XfFont_point_size_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_float = _this->point_size();
            _b.reply(_XfFont_point_size_pinfo, _arg);
            break;
        }
        case /* font_info */ 5: {
            extern MarshalBuffer::ArgInfo _XfFont_font_info_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            Font::Info i;
            _arg[1].u_addr = &i;
            _b.receive(_XfFont_font_info_pinfo, _arg);
            _this->font_info(i);
            _b.reply(_XfFont_font_info_pinfo, _arg);
            break;
        }
        case /* char_info */ 6: {
            extern MarshalBuffer::ArgInfo _XfFont_char_info_pinfo;
            MarshalBuffer::ArgValue _arg[3];
            CharCode c;
            _arg[1].u_addr = &c;
            Font::Info i;
            _arg[2].u_addr = &i;
            _b.receive(_XfFont_char_info_pinfo, _arg);
            _this->char_info(c, i);
            _b.reply(_XfFont_char_info_pinfo, _arg);
            break;
        }
        case /* string_info */ 7: {
            extern MarshalBuffer::ArgInfo _XfFont_string_info_pinfo;
            MarshalBuffer::ArgValue _arg[3];
            CharStringRef s;
            _arg[1].u_addr = &s;
            Font::Info i;
            _arg[2].u_addr = &i;
            _b.receive(_XfFont_string_info_pinfo, _arg);
            _this->string_info(s, i);
            _b.reply(_XfFont_string_info_pinfo, _arg);
            break;
        }
    }
}
extern void _XfFont_Info_put(
    MarshalBuffer&, const Font::Info&
);
extern void _XfFont_Info_get(
    MarshalBuffer&, Font::Info&
);
extern void _XfFont_Info_put(
    MarshalBuffer&, const Font::Info&
);
extern void _XfFont_Info_get(
    MarshalBuffer&, Font::Info&
);
extern void _XfFont_Info_put(
    MarshalBuffer&, const Font::Info&
);
extern void _XfFont_Info_get(
    MarshalBuffer&, Font::Info&
);

FontStub::FontStub(Exchange* e) { exch_ = e; }
FontStub::~FontStub() { }
BaseObjectRef FontStub::_create(Exchange* e) {
    return (BaseObjectRef)(void*)new FontStub(e);
}
Exchange* FontStub::_exchange() {
    return exch_;
}
MarshalBuffer::ArgDesc _XfFont_equal_pdesc[3] = { 2, 12, 61 };
MarshalBuffer::ArgMarshal _XfFont_equal_pfunc[] = {
    &FontStub::_create,

};
MarshalBuffer::ArgInfo _XfFont_equal_pinfo = {
    &_XfFont_tid, 0, _XfFont_equal_pdesc, _XfFont_equal_pfunc
};
Boolean FontType::equal(Font_in f) {
    MarshalBuffer _b;
    extern TypeObjId _XfFont_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = f;
    _b.invoke(this, _XfFont_equal_pinfo, _arg);
    return _arg[0].u_boolean;
}
MarshalBuffer::ArgDesc _XfFont_hash_pdesc[2] = { 1, 36 };
MarshalBuffer::ArgInfo _XfFont_hash_pinfo = {
    &_XfFont_tid, 1, _XfFont_hash_pdesc, 0
};
ULong FontType::hash() {
    MarshalBuffer _b;
    extern TypeObjId _XfFont_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfFont_hash_pinfo, _arg);
    return _arg[0].u_unsigned_long;
}
MarshalBuffer::ArgDesc _XfFont_name_pdesc[2] = { 1, 60 };
MarshalBuffer::ArgMarshal _XfFont_name_pfunc[] = {
    &CharStringStub::_create
};
MarshalBuffer::ArgInfo _XfFont_name_pinfo = {
    &_XfFont_tid, 2, _XfFont_name_pdesc, _XfFont_name_pfunc
};
CharStringRef FontType::_c_name() {
    MarshalBuffer _b;
    extern TypeObjId _XfFont_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfFont_name_pinfo, _arg);
    return (CharStringRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfFont_encoding_pdesc[2] = { 1, 60 };
MarshalBuffer::ArgMarshal _XfFont_encoding_pfunc[] = {
    &CharStringStub::_create
};
MarshalBuffer::ArgInfo _XfFont_encoding_pinfo = {
    &_XfFont_tid, 3, _XfFont_encoding_pdesc, _XfFont_encoding_pfunc
};
CharStringRef FontType::_c_encoding() {
    MarshalBuffer _b;
    extern TypeObjId _XfFont_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfFont_encoding_pinfo, _arg);
    return (CharStringRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfFont_point_size_pdesc[2] = { 1, 48 };
MarshalBuffer::ArgInfo _XfFont_point_size_pinfo = {
    &_XfFont_tid, 4, _XfFont_point_size_pdesc, 0
};
Coord FontType::point_size() {
    MarshalBuffer _b;
    extern TypeObjId _XfFont_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfFont_point_size_pinfo, _arg);
    return _arg[0].u_float;
}
MarshalBuffer::ArgDesc _XfFont_font_info_pdesc[3] = { 2, 4, 2 };
MarshalBuffer::ArgMarshal _XfFont_font_info_pfunc[] = {
    &_XfFont_Info_put, &_XfFont_Info_get,

};
MarshalBuffer::ArgInfo _XfFont_font_info_pinfo = {
    &_XfFont_tid, 5, _XfFont_font_info_pdesc, _XfFont_font_info_pfunc
};
void FontType::font_info(Font::Info& i) {
    MarshalBuffer _b;
    extern TypeObjId _XfFont_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_addr = &i;
    _b.invoke(this, _XfFont_font_info_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfFont_char_info_pdesc[4] = { 3, 4, 33, 2 };
MarshalBuffer::ArgMarshal _XfFont_char_info_pfunc[] = {
    &_XfFont_Info_put, &_XfFont_Info_get,

};
MarshalBuffer::ArgInfo _XfFont_char_info_pinfo = {
    &_XfFont_tid, 6, _XfFont_char_info_pdesc, _XfFont_char_info_pfunc
};
void FontType::char_info(CharCode c, Font::Info& i) {
    MarshalBuffer _b;
    extern TypeObjId _XfFont_tid;
    MarshalBuffer::ArgValue _arg[3];
    _arg[1].u_long = c;
    _arg[2].u_addr = &i;
    _b.invoke(this, _XfFont_char_info_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfFont_string_info_pdesc[4] = { 3, 4, 61, 2 };
MarshalBuffer::ArgMarshal _XfFont_string_info_pfunc[] = {
    &CharStringStub::_create,
    &_XfFont_Info_put, &_XfFont_Info_get,

};
MarshalBuffer::ArgInfo _XfFont_string_info_pinfo = {
    &_XfFont_tid, 7, _XfFont_string_info_pdesc, _XfFont_string_info_pfunc
};
void FontType::string_info(CharString_in s, Font::Info& i) {
    MarshalBuffer _b;
    extern TypeObjId _XfFont_tid;
    MarshalBuffer::ArgValue _arg[3];
    _arg[1].u_objref = s;
    _arg[2].u_addr = &i;
    _b.invoke(this, _XfFont_string_info_pinfo, _arg);
}
void _XfFont_Info_put(MarshalBuffer& _b, const Font::Info& _this) {
    _b.put_float(_this.width);
    _b.put_float(_this.height);
    _b.put_float(_this.left_bearing);
    _b.put_float(_this.right_bearing);
    _b.put_float(_this.ascent);
    _b.put_float(_this.descent);
    _b.put_float(_this.font_ascent);
    _b.put_float(_this.font_descent);
}
void _XfFont_Info_get(MarshalBuffer& _b, Font::Info& _this) {
    _this.width = _b.get_float();
    _this.height = _b.get_float();
    _this.left_bearing = _b.get_float();
    _this.right_bearing = _b.get_float();
    _this.ascent = _b.get_float();
    _this.descent = _b.get_float();
    _this.font_ascent = _b.get_float();
    _this.font_descent = _b.get_float();
}
//+

//+ FontImpl(Font)
extern TypeObj_Descriptor _XfFont_type;
TypeObj_Descriptor* _XfFontImpl_parents[] = { &_XfFont_type, nil };
extern TypeObjId _XfFontImpl_tid;
TypeObj_Descriptor _XfFontImpl_type = {
    /* type */ 0,
    /* id */ &_XfFontImpl_tid,
    "FontImpl",
    _XfFontImpl_parents, /* offsets */ nil, /* excepts */ nil,
    /* methods */ nil, /* params */ nil,
    /* receive */ nil
};

FontImpl* FontImpl::_narrow(BaseObjectRef o) {
    return (FontImpl*)_BaseObject_tnarrow(
        o, _XfFontImpl_tid, 0
    );
}
TypeObjId FontImpl::_tid() { return _XfFontImpl_tid; }
//+

//+ DrawingKit::%init,type+dii,client
DrawingKit::Data& DrawingKit::Data::operator =(const Data& _s) {
    delete [] _buffer;
    _maximum = _s._maximum;
    _length = _s._length;
    _buffer = _maximum == 0 ? 0 : new Long[_maximum];
    for (int i = 0; i < _length; i++) {
        _buffer[i] = _s._buffer[i];
    }
    return *this;
}

DrawingKitType::DrawingKitType() { }
DrawingKitType::~DrawingKitType() { }
void* DrawingKitType::_this() { return this; }

extern TypeObj_Descriptor _XfStyleObj_type, _XfLong_type, _XfDrawingKit_Data_type, 
    _XfRaster_type, _XfRaster_Index_type, _XfTransformObj_type, _XfTransformObj_Matrix_type, 
    _XfPainterObj_type;

TypeObj_OpData _XfDrawingKit_methods[] = {
    { "style", &_XfStyleObj_type, 0 },
    { "simple_brush", &_XfBrush_type, 1 },
    { "dither_brush", &_XfBrush_type, 2 },
    { "patterned_brush", &_XfBrush_type, 2 },
    { "color_rgb", &_XfColor_type, 3 },
    { "find_color", &_XfColor_type, 1 },
    { "resolve_color", &_XfColor_type, 2 },
    { "foreground", &_XfColor_type, 1 },
    { "background", &_XfColor_type, 1 },
    { "find_font", &_XfFont_type, 1 },
    { "find_font_match", &_XfFont_type, 3 },
    { "resolve_font", &_XfFont_type, 2 },
    { "default_font", &_XfFont_type, 1 },
    { "bitmap_file", &_XfRaster_type, 1 },
    { "bitmap_data", &_XfRaster_type, 5 },
    { "bitmap_char", &_XfRaster_type, 2 },
    { "raster_tiff", &_XfRaster_type, 1 },
    { "transform", &_XfTransformObj_type, 1 },
    { "identity_transform", &_XfTransformObj_type, 0 },
    { "printer", &_XfPainterObj_type, 1 },
    { 0, 0, 0 }
};
TypeObj_ParamData _XfDrawingKit_params[] = {
    /* simple_brush */
        { "width", 0, &_XfCoord_type },
    /* dither_brush */
        { "width", 0, &_XfCoord_type },
        { "pattern", 0, &_XfLong_type },
    /* patterned_brush */
        { "width", 0, &_XfCoord_type },
        { "pattern", 0, &_XfDrawingKit_Data_type },
    /* color_rgb */
        { "r", 0, &_XfColor_Intensity_type },
        { "g", 0, &_XfColor_Intensity_type },
        { "b", 0, &_XfColor_Intensity_type },
    /* find_color */
        { "name", 0, &_XfCharString_type },
    /* resolve_color */
        { "s", 0, &_XfStyleObj_type },
        { "name", 0, &_XfCharString_type },
    /* foreground */
        { "s", 0, &_XfStyleObj_type },
    /* background */
        { "s", 0, &_XfStyleObj_type },
    /* find_font */
        { "fullname", 0, &_XfCharString_type },
    /* find_font_match */
        { "family", 0, &_XfCharString_type },
        { "style", 0, &_XfCharString_type },
        { "ptsize", 0, &_XfCoord_type },
    /* resolve_font */
        { "s", 0, &_XfStyleObj_type },
        { "name", 0, &_XfCharString_type },
    /* default_font */
        { "s", 0, &_XfStyleObj_type },
    /* bitmap_file */
        { "filename", 0, &_XfCharString_type },
    /* bitmap_data */
        { "data", 0, &_XfDrawingKit_Data_type },
        { "rows", 0, &_XfRaster_Index_type },
        { "columns", 0, &_XfRaster_Index_type },
        { "origin_row", 0, &_XfRaster_Index_type },
        { "origin_column", 0, &_XfRaster_Index_type },
    /* bitmap_char */
        { "f", 0, &_XfFont_type },
        { "c", 0, &_XfCharCode_type },
    /* raster_tiff */
        { "filename", 0, &_XfCharString_type },
    /* transform */
        { "m", 0, &_XfTransformObj_Matrix_type },
    /* printer */
        { "output", 0, &_XfCharString_type }
};
extern TypeObj_Descriptor _XfFrescoObject_type;
TypeObj_Descriptor* _XfDrawingKit_parents[] = { &_XfFrescoObject_type, nil };
extern TypeObjId _XfDrawingKit_tid;
extern void _XfDrawingKit_receive(BaseObjectRef, ULong, MarshalBuffer&);
TypeObj_Descriptor _XfDrawingKit_type = {
    /* type */ 0,
    /* id */ &_XfDrawingKit_tid,
    "DrawingKit",
    _XfDrawingKit_parents, /* offsets */ nil, /* excepts */ nil,
    _XfDrawingKit_methods, _XfDrawingKit_params,
    &_XfDrawingKit_receive
};

DrawingKitRef DrawingKit::_narrow(BaseObjectRef o) {
    return (DrawingKitRef)_BaseObject_tnarrow(
        o, _XfDrawingKit_tid, &DrawingKitStub::_create
    );
}
TypeObjId DrawingKitType::_tid() { return _XfDrawingKit_tid; }
void _XfDrawingKit_receive(BaseObjectRef _object, ULong _m, MarshalBuffer& _b) {
    extern TypeObjId _XfDrawingKit_tid;
    DrawingKitRef _this = (DrawingKitRef)_BaseObject_tcast(_object, _XfDrawingKit_tid);
    switch (_m) {
        case /* style */ 0: {
            extern MarshalBuffer::ArgInfo _XfDrawingKit_style_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_objref = _this->_c_style();
            _b.reply(_XfDrawingKit_style_pinfo, _arg);
            break;
        }
        case /* simple_brush */ 1: {
            extern MarshalBuffer::ArgInfo _XfDrawingKit_simple_brush_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            Coord width;
            _arg[1].u_addr = &width;
            _b.receive(_XfDrawingKit_simple_brush_pinfo, _arg);
            _arg[0].u_objref = _this->_c_simple_brush(width);
            _b.reply(_XfDrawingKit_simple_brush_pinfo, _arg);
            break;
        }
        case /* dither_brush */ 2: {
            extern MarshalBuffer::ArgInfo _XfDrawingKit_dither_brush_pinfo;
            MarshalBuffer::ArgValue _arg[3];
            Coord width;
            _arg[1].u_addr = &width;
            Long pattern;
            _arg[2].u_addr = &pattern;
            _b.receive(_XfDrawingKit_dither_brush_pinfo, _arg);
            _arg[0].u_objref = _this->_c_dither_brush(width, pattern);
            _b.reply(_XfDrawingKit_dither_brush_pinfo, _arg);
            break;
        }
        case /* patterned_brush */ 3: {
            extern MarshalBuffer::ArgInfo _XfDrawingKit_patterned_brush_pinfo;
            MarshalBuffer::ArgValue _arg[3];
            Coord width;
            _arg[1].u_addr = &width;
            DrawingKit::Data pattern;
            _arg[2].u_addr = &pattern;
            _b.receive(_XfDrawingKit_patterned_brush_pinfo, _arg);
            _arg[0].u_objref = _this->_c_patterned_brush(width, pattern);
            _b.reply(_XfDrawingKit_patterned_brush_pinfo, _arg);
            break;
        }
        case /* color_rgb */ 4: {
            extern MarshalBuffer::ArgInfo _XfDrawingKit_color_rgb_pinfo;
            MarshalBuffer::ArgValue _arg[4];
            Color::Intensity r;
            _arg[1].u_addr = &r;
            Color::Intensity g;
            _arg[2].u_addr = &g;
            Color::Intensity b;
            _arg[3].u_addr = &b;
            _b.receive(_XfDrawingKit_color_rgb_pinfo, _arg);
            _arg[0].u_objref = _this->_c_color_rgb(r, g, b);
            _b.reply(_XfDrawingKit_color_rgb_pinfo, _arg);
            break;
        }
        case /* find_color */ 5: {
            extern MarshalBuffer::ArgInfo _XfDrawingKit_find_color_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            CharStringRef name;
            _arg[1].u_addr = &name;
            _b.receive(_XfDrawingKit_find_color_pinfo, _arg);
            _arg[0].u_objref = _this->_c_find_color(name);
            _b.reply(_XfDrawingKit_find_color_pinfo, _arg);
            break;
        }
        case /* resolve_color */ 6: {
            extern MarshalBuffer::ArgInfo _XfDrawingKit_resolve_color_pinfo;
            MarshalBuffer::ArgValue _arg[3];
            StyleObjRef s;
            _arg[1].u_addr = &s;
            CharStringRef name;
            _arg[2].u_addr = &name;
            _b.receive(_XfDrawingKit_resolve_color_pinfo, _arg);
            _arg[0].u_objref = _this->_c_resolve_color(s, name);
            _b.reply(_XfDrawingKit_resolve_color_pinfo, _arg);
            break;
        }
        case /* foreground */ 7: {
            extern MarshalBuffer::ArgInfo _XfDrawingKit_foreground_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            StyleObjRef s;
            _arg[1].u_addr = &s;
            _b.receive(_XfDrawingKit_foreground_pinfo, _arg);
            _arg[0].u_objref = _this->_c_foreground(s);
            _b.reply(_XfDrawingKit_foreground_pinfo, _arg);
            break;
        }
        case /* background */ 8: {
            extern MarshalBuffer::ArgInfo _XfDrawingKit_background_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            StyleObjRef s;
            _arg[1].u_addr = &s;
            _b.receive(_XfDrawingKit_background_pinfo, _arg);
            _arg[0].u_objref = _this->_c_background(s);
            _b.reply(_XfDrawingKit_background_pinfo, _arg);
            break;
        }
        case /* find_font */ 9: {
            extern MarshalBuffer::ArgInfo _XfDrawingKit_find_font_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            CharStringRef fullname;
            _arg[1].u_addr = &fullname;
            _b.receive(_XfDrawingKit_find_font_pinfo, _arg);
            _arg[0].u_objref = _this->_c_find_font(fullname);
            _b.reply(_XfDrawingKit_find_font_pinfo, _arg);
            break;
        }
        case /* find_font_match */ 10: {
            extern MarshalBuffer::ArgInfo _XfDrawingKit_find_font_match_pinfo;
            MarshalBuffer::ArgValue _arg[4];
            CharStringRef family;
            _arg[1].u_addr = &family;
            CharStringRef style;
            _arg[2].u_addr = &style;
            Coord ptsize;
            _arg[3].u_addr = &ptsize;
            _b.receive(_XfDrawingKit_find_font_match_pinfo, _arg);
            _arg[0].u_objref = _this->_c_find_font_match(family, style, ptsize);
            _b.reply(_XfDrawingKit_find_font_match_pinfo, _arg);
            break;
        }
        case /* resolve_font */ 11: {
            extern MarshalBuffer::ArgInfo _XfDrawingKit_resolve_font_pinfo;
            MarshalBuffer::ArgValue _arg[3];
            StyleObjRef s;
            _arg[1].u_addr = &s;
            CharStringRef name;
            _arg[2].u_addr = &name;
            _b.receive(_XfDrawingKit_resolve_font_pinfo, _arg);
            _arg[0].u_objref = _this->_c_resolve_font(s, name);
            _b.reply(_XfDrawingKit_resolve_font_pinfo, _arg);
            break;
        }
        case /* default_font */ 12: {
            extern MarshalBuffer::ArgInfo _XfDrawingKit_default_font_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            StyleObjRef s;
            _arg[1].u_addr = &s;
            _b.receive(_XfDrawingKit_default_font_pinfo, _arg);
            _arg[0].u_objref = _this->_c_default_font(s);
            _b.reply(_XfDrawingKit_default_font_pinfo, _arg);
            break;
        }
        case /* bitmap_file */ 13: {
            extern MarshalBuffer::ArgInfo _XfDrawingKit_bitmap_file_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            CharStringRef filename;
            _arg[1].u_addr = &filename;
            _b.receive(_XfDrawingKit_bitmap_file_pinfo, _arg);
            _arg[0].u_objref = _this->_c_bitmap_file(filename);
            _b.reply(_XfDrawingKit_bitmap_file_pinfo, _arg);
            break;
        }
        case /* bitmap_data */ 14: {
            extern MarshalBuffer::ArgInfo _XfDrawingKit_bitmap_data_pinfo;
            MarshalBuffer::ArgValue _arg[6];
            DrawingKit::Data data;
            _arg[1].u_addr = &data;
            Raster::Index rows;
            _arg[2].u_addr = &rows;
            Raster::Index columns;
            _arg[3].u_addr = &columns;
            Raster::Index origin_row;
            _arg[4].u_addr = &origin_row;
            Raster::Index origin_column;
            _arg[5].u_addr = &origin_column;
            _b.receive(_XfDrawingKit_bitmap_data_pinfo, _arg);
            _arg[0].u_objref = _this->_c_bitmap_data(data, rows, columns, origin_row, origin_column);
            _b.reply(_XfDrawingKit_bitmap_data_pinfo, _arg);
            break;
        }
        case /* bitmap_char */ 15: {
            extern MarshalBuffer::ArgInfo _XfDrawingKit_bitmap_char_pinfo;
            MarshalBuffer::ArgValue _arg[3];
            FontRef f;
            _arg[1].u_addr = &f;
            CharCode c;
            _arg[2].u_addr = &c;
            _b.receive(_XfDrawingKit_bitmap_char_pinfo, _arg);
            _arg[0].u_objref = _this->_c_bitmap_char(f, c);
            _b.reply(_XfDrawingKit_bitmap_char_pinfo, _arg);
            break;
        }
        case /* raster_tiff */ 16: {
            extern MarshalBuffer::ArgInfo _XfDrawingKit_raster_tiff_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            CharStringRef filename;
            _arg[1].u_addr = &filename;
            _b.receive(_XfDrawingKit_raster_tiff_pinfo, _arg);
            _arg[0].u_objref = _this->_c_raster_tiff(filename);
            _b.reply(_XfDrawingKit_raster_tiff_pinfo, _arg);
            break;
        }
        case /* transform */ 17: {
            extern MarshalBuffer::ArgInfo _XfDrawingKit_transform_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            TransformObj::Matrix m;
            _arg[1].u_addr = m;
            _b.receive(_XfDrawingKit_transform_pinfo, _arg);
            _arg[0].u_objref = _this->_c_transform(m);
            _b.reply(_XfDrawingKit_transform_pinfo, _arg);
            break;
        }
        case /* identity_transform */ 18: {
            extern MarshalBuffer::ArgInfo _XfDrawingKit_identity_transform_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_objref = _this->_c_identity_transform();
            _b.reply(_XfDrawingKit_identity_transform_pinfo, _arg);
            break;
        }
        case /* printer */ 19: {
            extern MarshalBuffer::ArgInfo _XfDrawingKit_printer_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            CharStringRef output;
            _arg[1].u_addr = &output;
            _b.receive(_XfDrawingKit_printer_pinfo, _arg);
            _arg[0].u_objref = _this->_c_printer(output);
            _b.reply(_XfDrawingKit_printer_pinfo, _arg);
            break;
        }
    }
}
extern void _XfDrawingKit_Data_put(
    MarshalBuffer&, const DrawingKit::Data&
);
extern void _XfDrawingKit_Data_get(
    MarshalBuffer&, DrawingKit::Data&
);
extern void _XfDrawingKit_Data_put(
    MarshalBuffer&, const DrawingKit::Data&
);
extern void _XfDrawingKit_Data_get(
    MarshalBuffer&, DrawingKit::Data&
);
extern void _XfDrawingKit_Data_put(
    MarshalBuffer&, const DrawingKit::Data&
);
extern void _XfDrawingKit_Data_get(
    MarshalBuffer&, DrawingKit::Data&
);
extern void _XfTransformObj_Matrix_put(
    MarshalBuffer&, const Coord Matrix[4][4]
);
extern void _XfTransformObj_Matrix_get(
    MarshalBuffer&, Coord Matrix[4][4]
);

DrawingKitStub::DrawingKitStub(Exchange* e) { exch_ = e; }
DrawingKitStub::~DrawingKitStub() { }
BaseObjectRef DrawingKitStub::_create(Exchange* e) {
    return (BaseObjectRef)(void*)new DrawingKitStub(e);
}
Exchange* DrawingKitStub::_exchange() {
    return exch_;
}
MarshalBuffer::ArgDesc _XfDrawingKit_style_pdesc[2] = { 1, 60 };
MarshalBuffer::ArgMarshal _XfDrawingKit_style_pfunc[] = {
    &StyleObjStub::_create
};
MarshalBuffer::ArgInfo _XfDrawingKit_style_pinfo = {
    &_XfDrawingKit_tid, 0, _XfDrawingKit_style_pdesc, _XfDrawingKit_style_pfunc
};
StyleObjRef DrawingKitType::_c_style() {
    MarshalBuffer _b;
    extern TypeObjId _XfDrawingKit_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfDrawingKit_style_pinfo, _arg);
    return (StyleObjRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfDrawingKit_simple_brush_pdesc[3] = { 2, 60, 49 };
MarshalBuffer::ArgMarshal _XfDrawingKit_simple_brush_pfunc[] = {
    &BrushStub::_create
};
MarshalBuffer::ArgInfo _XfDrawingKit_simple_brush_pinfo = {
    &_XfDrawingKit_tid, 1, _XfDrawingKit_simple_brush_pdesc, _XfDrawingKit_simple_brush_pfunc
};
BrushRef DrawingKitType::_c_simple_brush(Coord width) {
    MarshalBuffer _b;
    extern TypeObjId _XfDrawingKit_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_float = width;
    _b.invoke(this, _XfDrawingKit_simple_brush_pinfo, _arg);
    return (BrushRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfDrawingKit_dither_brush_pdesc[4] = { 3, 60, 49, 33 };
MarshalBuffer::ArgMarshal _XfDrawingKit_dither_brush_pfunc[] = {
    &BrushStub::_create
};
MarshalBuffer::ArgInfo _XfDrawingKit_dither_brush_pinfo = {
    &_XfDrawingKit_tid, 2, _XfDrawingKit_dither_brush_pdesc, _XfDrawingKit_dither_brush_pfunc
};
BrushRef DrawingKitType::_c_dither_brush(Coord width, Long pattern) {
    MarshalBuffer _b;
    extern TypeObjId _XfDrawingKit_tid;
    MarshalBuffer::ArgValue _arg[3];
    _arg[1].u_float = width;
    _arg[2].u_long = pattern;
    _b.invoke(this, _XfDrawingKit_dither_brush_pinfo, _arg);
    return (BrushRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfDrawingKit_patterned_brush_pdesc[4] = { 3, 60, 49, 1 };
MarshalBuffer::ArgMarshal _XfDrawingKit_patterned_brush_pfunc[] = {
    &_XfDrawingKit_Data_put, &_XfDrawingKit_Data_get,
    &BrushStub::_create
};
MarshalBuffer::ArgInfo _XfDrawingKit_patterned_brush_pinfo = {
    &_XfDrawingKit_tid, 3, _XfDrawingKit_patterned_brush_pdesc, _XfDrawingKit_patterned_brush_pfunc
};
BrushRef DrawingKitType::_c_patterned_brush(Coord width, const DrawingKit::Data& pattern) {
    MarshalBuffer _b;
    extern TypeObjId _XfDrawingKit_tid;
    MarshalBuffer::ArgValue _arg[3];
    _arg[1].u_float = width;
    _arg[2].u_addr = &pattern;
    _b.invoke(this, _XfDrawingKit_patterned_brush_pinfo, _arg);
    return (BrushRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfDrawingKit_color_rgb_pdesc[5] = { 4, 60, 49, 49, 49 };
MarshalBuffer::ArgMarshal _XfDrawingKit_color_rgb_pfunc[] = {
    &ColorStub::_create
};
MarshalBuffer::ArgInfo _XfDrawingKit_color_rgb_pinfo = {
    &_XfDrawingKit_tid, 4, _XfDrawingKit_color_rgb_pdesc, _XfDrawingKit_color_rgb_pfunc
};
ColorRef DrawingKitType::_c_color_rgb(Color::Intensity r, Color::Intensity g, Color::Intensity b) {
    MarshalBuffer _b;
    extern TypeObjId _XfDrawingKit_tid;
    MarshalBuffer::ArgValue _arg[4];
    _arg[1].u_float = r;
    _arg[2].u_float = g;
    _arg[3].u_float = b;
    _b.invoke(this, _XfDrawingKit_color_rgb_pinfo, _arg);
    return (ColorRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfDrawingKit_find_color_pdesc[3] = { 2, 60, 61 };
MarshalBuffer::ArgMarshal _XfDrawingKit_find_color_pfunc[] = {
    &CharStringStub::_create,
    &ColorStub::_create
};
MarshalBuffer::ArgInfo _XfDrawingKit_find_color_pinfo = {
    &_XfDrawingKit_tid, 5, _XfDrawingKit_find_color_pdesc, _XfDrawingKit_find_color_pfunc
};
ColorRef DrawingKitType::_c_find_color(CharString_in name) {
    MarshalBuffer _b;
    extern TypeObjId _XfDrawingKit_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = name;
    _b.invoke(this, _XfDrawingKit_find_color_pinfo, _arg);
    return (ColorRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfDrawingKit_resolve_color_pdesc[4] = { 3, 60, 61, 61 };
MarshalBuffer::ArgMarshal _XfDrawingKit_resolve_color_pfunc[] = {
    &StyleObjStub::_create,
    &CharStringStub::_create,
    &ColorStub::_create
};
MarshalBuffer::ArgInfo _XfDrawingKit_resolve_color_pinfo = {
    &_XfDrawingKit_tid, 6, _XfDrawingKit_resolve_color_pdesc, _XfDrawingKit_resolve_color_pfunc
};
ColorRef DrawingKitType::_c_resolve_color(StyleObj_in s, CharString_in name) {
    MarshalBuffer _b;
    extern TypeObjId _XfDrawingKit_tid;
    MarshalBuffer::ArgValue _arg[3];
    _arg[1].u_objref = s;
    _arg[2].u_objref = name;
    _b.invoke(this, _XfDrawingKit_resolve_color_pinfo, _arg);
    return (ColorRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfDrawingKit_foreground_pdesc[3] = { 2, 60, 61 };
MarshalBuffer::ArgMarshal _XfDrawingKit_foreground_pfunc[] = {
    &StyleObjStub::_create,
    &ColorStub::_create
};
MarshalBuffer::ArgInfo _XfDrawingKit_foreground_pinfo = {
    &_XfDrawingKit_tid, 7, _XfDrawingKit_foreground_pdesc, _XfDrawingKit_foreground_pfunc
};
ColorRef DrawingKitType::_c_foreground(StyleObj_in s) {
    MarshalBuffer _b;
    extern TypeObjId _XfDrawingKit_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = s;
    _b.invoke(this, _XfDrawingKit_foreground_pinfo, _arg);
    return (ColorRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfDrawingKit_background_pdesc[3] = { 2, 60, 61 };
MarshalBuffer::ArgMarshal _XfDrawingKit_background_pfunc[] = {
    &StyleObjStub::_create,
    &ColorStub::_create
};
MarshalBuffer::ArgInfo _XfDrawingKit_background_pinfo = {
    &_XfDrawingKit_tid, 8, _XfDrawingKit_background_pdesc, _XfDrawingKit_background_pfunc
};
ColorRef DrawingKitType::_c_background(StyleObj_in s) {
    MarshalBuffer _b;
    extern TypeObjId _XfDrawingKit_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = s;
    _b.invoke(this, _XfDrawingKit_background_pinfo, _arg);
    return (ColorRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfDrawingKit_find_font_pdesc[3] = { 2, 60, 61 };
MarshalBuffer::ArgMarshal _XfDrawingKit_find_font_pfunc[] = {
    &CharStringStub::_create,
    &FontStub::_create
};
MarshalBuffer::ArgInfo _XfDrawingKit_find_font_pinfo = {
    &_XfDrawingKit_tid, 9, _XfDrawingKit_find_font_pdesc, _XfDrawingKit_find_font_pfunc
};
FontRef DrawingKitType::_c_find_font(CharString_in fullname) {
    MarshalBuffer _b;
    extern TypeObjId _XfDrawingKit_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = fullname;
    _b.invoke(this, _XfDrawingKit_find_font_pinfo, _arg);
    return (FontRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfDrawingKit_find_font_match_pdesc[5] = { 4, 60, 61, 61, 49 };
MarshalBuffer::ArgMarshal _XfDrawingKit_find_font_match_pfunc[] = {
    &CharStringStub::_create,
    &CharStringStub::_create,
    &FontStub::_create
};
MarshalBuffer::ArgInfo _XfDrawingKit_find_font_match_pinfo = {
    &_XfDrawingKit_tid, 10, _XfDrawingKit_find_font_match_pdesc, _XfDrawingKit_find_font_match_pfunc
};
FontRef DrawingKitType::_c_find_font_match(CharString_in family, CharString_in style, Coord ptsize) {
    MarshalBuffer _b;
    extern TypeObjId _XfDrawingKit_tid;
    MarshalBuffer::ArgValue _arg[4];
    _arg[1].u_objref = family;
    _arg[2].u_objref = style;
    _arg[3].u_float = ptsize;
    _b.invoke(this, _XfDrawingKit_find_font_match_pinfo, _arg);
    return (FontRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfDrawingKit_resolve_font_pdesc[4] = { 3, 60, 61, 61 };
MarshalBuffer::ArgMarshal _XfDrawingKit_resolve_font_pfunc[] = {
    &StyleObjStub::_create,
    &CharStringStub::_create,
    &FontStub::_create
};
MarshalBuffer::ArgInfo _XfDrawingKit_resolve_font_pinfo = {
    &_XfDrawingKit_tid, 11, _XfDrawingKit_resolve_font_pdesc, _XfDrawingKit_resolve_font_pfunc
};
FontRef DrawingKitType::_c_resolve_font(StyleObj_in s, CharString_in name) {
    MarshalBuffer _b;
    extern TypeObjId _XfDrawingKit_tid;
    MarshalBuffer::ArgValue _arg[3];
    _arg[1].u_objref = s;
    _arg[2].u_objref = name;
    _b.invoke(this, _XfDrawingKit_resolve_font_pinfo, _arg);
    return (FontRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfDrawingKit_default_font_pdesc[3] = { 2, 60, 61 };
MarshalBuffer::ArgMarshal _XfDrawingKit_default_font_pfunc[] = {
    &StyleObjStub::_create,
    &FontStub::_create
};
MarshalBuffer::ArgInfo _XfDrawingKit_default_font_pinfo = {
    &_XfDrawingKit_tid, 12, _XfDrawingKit_default_font_pdesc, _XfDrawingKit_default_font_pfunc
};
FontRef DrawingKitType::_c_default_font(StyleObj_in s) {
    MarshalBuffer _b;
    extern TypeObjId _XfDrawingKit_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = s;
    _b.invoke(this, _XfDrawingKit_default_font_pinfo, _arg);
    return (FontRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfDrawingKit_bitmap_file_pdesc[3] = { 2, 60, 61 };
MarshalBuffer::ArgMarshal _XfDrawingKit_bitmap_file_pfunc[] = {
    &CharStringStub::_create,
    &RasterStub::_create
};
MarshalBuffer::ArgInfo _XfDrawingKit_bitmap_file_pinfo = {
    &_XfDrawingKit_tid, 13, _XfDrawingKit_bitmap_file_pdesc, _XfDrawingKit_bitmap_file_pfunc
};
RasterRef DrawingKitType::_c_bitmap_file(CharString_in filename) {
    MarshalBuffer _b;
    extern TypeObjId _XfDrawingKit_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = filename;
    _b.invoke(this, _XfDrawingKit_bitmap_file_pinfo, _arg);
    return (RasterRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfDrawingKit_bitmap_data_pdesc[7] = { 6, 60, 1, 33, 33, 33, 33 };
MarshalBuffer::ArgMarshal _XfDrawingKit_bitmap_data_pfunc[] = {
    &_XfDrawingKit_Data_put, &_XfDrawingKit_Data_get,
    &RasterStub::_create
};
MarshalBuffer::ArgInfo _XfDrawingKit_bitmap_data_pinfo = {
    &_XfDrawingKit_tid, 14, _XfDrawingKit_bitmap_data_pdesc, _XfDrawingKit_bitmap_data_pfunc
};
RasterRef DrawingKitType::_c_bitmap_data(const DrawingKit::Data& data, Raster::Index rows, Raster::Index columns, Raster::Index origin_row, Raster::Index origin_column) {
    MarshalBuffer _b;
    extern TypeObjId _XfDrawingKit_tid;
    MarshalBuffer::ArgValue _arg[6];
    _arg[1].u_addr = &data;
    _arg[2].u_long = rows;
    _arg[3].u_long = columns;
    _arg[4].u_long = origin_row;
    _arg[5].u_long = origin_column;
    _b.invoke(this, _XfDrawingKit_bitmap_data_pinfo, _arg);
    return (RasterRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfDrawingKit_bitmap_char_pdesc[4] = { 3, 60, 61, 33 };
MarshalBuffer::ArgMarshal _XfDrawingKit_bitmap_char_pfunc[] = {
    &FontStub::_create,
    &RasterStub::_create
};
MarshalBuffer::ArgInfo _XfDrawingKit_bitmap_char_pinfo = {
    &_XfDrawingKit_tid, 15, _XfDrawingKit_bitmap_char_pdesc, _XfDrawingKit_bitmap_char_pfunc
};
RasterRef DrawingKitType::_c_bitmap_char(Font_in f, CharCode c) {
    MarshalBuffer _b;
    extern TypeObjId _XfDrawingKit_tid;
    MarshalBuffer::ArgValue _arg[3];
    _arg[1].u_objref = f;
    _arg[2].u_long = c;
    _b.invoke(this, _XfDrawingKit_bitmap_char_pinfo, _arg);
    return (RasterRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfDrawingKit_raster_tiff_pdesc[3] = { 2, 60, 61 };
MarshalBuffer::ArgMarshal _XfDrawingKit_raster_tiff_pfunc[] = {
    &CharStringStub::_create,
    &RasterStub::_create
};
MarshalBuffer::ArgInfo _XfDrawingKit_raster_tiff_pinfo = {
    &_XfDrawingKit_tid, 16, _XfDrawingKit_raster_tiff_pdesc, _XfDrawingKit_raster_tiff_pfunc
};
RasterRef DrawingKitType::_c_raster_tiff(CharString_in filename) {
    MarshalBuffer _b;
    extern TypeObjId _XfDrawingKit_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = filename;
    _b.invoke(this, _XfDrawingKit_raster_tiff_pinfo, _arg);
    return (RasterRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfDrawingKit_transform_pdesc[3] = { 2, 60, 1 };
MarshalBuffer::ArgMarshal _XfDrawingKit_transform_pfunc[] = {
    &_XfTransformObj_Matrix_put, &_XfTransformObj_Matrix_get,
    &TransformObjStub::_create
};
MarshalBuffer::ArgInfo _XfDrawingKit_transform_pinfo = {
    &_XfDrawingKit_tid, 17, _XfDrawingKit_transform_pdesc, _XfDrawingKit_transform_pfunc
};
TransformObjRef DrawingKitType::_c_transform(TransformObj::Matrix m) {
    MarshalBuffer _b;
    extern TypeObjId _XfDrawingKit_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_addr = m;
    _b.invoke(this, _XfDrawingKit_transform_pinfo, _arg);
    return (TransformObjRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfDrawingKit_identity_transform_pdesc[2] = { 1, 60 };
MarshalBuffer::ArgMarshal _XfDrawingKit_identity_transform_pfunc[] = {
    &TransformObjStub::_create
};
MarshalBuffer::ArgInfo _XfDrawingKit_identity_transform_pinfo = {
    &_XfDrawingKit_tid, 18, _XfDrawingKit_identity_transform_pdesc, _XfDrawingKit_identity_transform_pfunc
};
TransformObjRef DrawingKitType::_c_identity_transform() {
    MarshalBuffer _b;
    extern TypeObjId _XfDrawingKit_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfDrawingKit_identity_transform_pinfo, _arg);
    return (TransformObjRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfDrawingKit_printer_pdesc[3] = { 2, 60, 61 };
MarshalBuffer::ArgMarshal _XfDrawingKit_printer_pfunc[] = {
    &CharStringStub::_create,
    &PainterObjStub::_create
};
MarshalBuffer::ArgInfo _XfDrawingKit_printer_pinfo = {
    &_XfDrawingKit_tid, 19, _XfDrawingKit_printer_pdesc, _XfDrawingKit_printer_pfunc
};
PainterObjRef DrawingKitType::_c_printer(CharString_in output) {
    MarshalBuffer _b;
    extern TypeObjId _XfDrawingKit_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = output;
    _b.invoke(this, _XfDrawingKit_printer_pinfo, _arg);
    return (PainterObjRef)_arg[0].u_objref;
}
void _XfDrawingKit_Data_put(MarshalBuffer& _b, const DrawingKit::Data& _this) {
    _b.put_seq(&_this, sizeof(Long));
}
void _XfDrawingKit_Data_get(MarshalBuffer& _b, DrawingKit::Data& _this) {
    _b.get_seq(&_this, sizeof(Long));
}
//+

//+ PainterObj::%init,type+dii,client
PainterObjType::PainterObjType() { }
PainterObjType::~PainterObjType() { }
void* PainterObjType::_this() { return this; }

extern TypeObj_Descriptor _XfPixelCoord_type, _XfRegion_type;

TypeObj_OpData _XfPainterObj_methods[] = {
    { "to_coord", &_XfCoord_type, 1 },
    { "to_pixels", &_XfPixelCoord_type, 1 },
    { "to_pixels_coord", &_XfCoord_type, 1 },
    { "begin_path", &_Xfvoid_type, 0 },
    { "move_to", &_Xfvoid_type, 2 },
    { "line_to", &_Xfvoid_type, 2 },
    { "curve_to", &_Xfvoid_type, 6 },
    { "close_path", &_Xfvoid_type, 0 },
    { "_get_brush_attr", &_XfBrush_type, 0 },
    { "_set_brush_attr", &_Xfvoid_type, 1 },
    { "_get_color_attr", &_XfColor_type, 0 },
    { "_set_color_attr", &_Xfvoid_type, 1 },
    { "_get_font_attr", &_XfFont_type, 0 },
    { "_set_font_attr", &_Xfvoid_type, 1 },
    { "stroke", &_Xfvoid_type, 0 },
    { "fill", &_Xfvoid_type, 0 },
    { "line", &_Xfvoid_type, 4 },
    { "rect", &_Xfvoid_type, 4 },
    { "fill_rect", &_Xfvoid_type, 4 },
    { "character", &_Xfvoid_type, 4 },
    { "image", &_Xfvoid_type, 3 },
    { "stencil", &_Xfvoid_type, 3 },
    { "_get_matrix", &_XfTransformObj_type, 0 },
    { "_set_matrix", &_Xfvoid_type, 1 },
    { "push_matrix", &_Xfvoid_type, 0 },
    { "pop_matrix", &_Xfvoid_type, 0 },
    { "transform", &_Xfvoid_type, 1 },
    { "clip", &_Xfvoid_type, 0 },
    { "clip_rect", &_Xfvoid_type, 4 },
    { "push_clipping", &_Xfvoid_type, 0 },
    { "pop_clipping", &_Xfvoid_type, 0 },
    { "is_visible", &_XfBoolean_type, 1 },
    { "visible", &_XfRegion_type, 0 },
    { "comment", &_Xfvoid_type, 1 },
    { "page_number", &_Xfvoid_type, 1 },
    { 0, 0, 0 }
};
TypeObj_ParamData _XfPainterObj_params[] = {
    /* to_coord */
        { "p", 0, &_XfPixelCoord_type },
    /* to_pixels */
        { "c", 0, &_XfCoord_type },
    /* to_pixels_coord */
        { "c", 0, &_XfCoord_type },
    /* move_to */
        { "x", 0, &_XfCoord_type },
        { "y", 0, &_XfCoord_type },
    /* line_to */
        { "x", 0, &_XfCoord_type },
        { "y", 0, &_XfCoord_type },
    /* curve_to */
        { "x", 0, &_XfCoord_type },
        { "y", 0, &_XfCoord_type },
        { "x1", 0, &_XfCoord_type },
        { "y1", 0, &_XfCoord_type },
        { "x2", 0, &_XfCoord_type },
        { "y2", 0, &_XfCoord_type },
    /* brush_attr */
        { "_p", 0, &_XfBrush_type },
    /* color_attr */
        { "_p", 0, &_XfColor_type },
    /* font_attr */
        { "_p", 0, &_XfFont_type },
    /* line */
        { "x0", 0, &_XfCoord_type },
        { "y0", 0, &_XfCoord_type },
        { "x1", 0, &_XfCoord_type },
        { "y1", 0, &_XfCoord_type },
    /* rect */
        { "x0", 0, &_XfCoord_type },
        { "y0", 0, &_XfCoord_type },
        { "x1", 0, &_XfCoord_type },
        { "y1", 0, &_XfCoord_type },
    /* fill_rect */
        { "x0", 0, &_XfCoord_type },
        { "y0", 0, &_XfCoord_type },
        { "x1", 0, &_XfCoord_type },
        { "y1", 0, &_XfCoord_type },
    /* character */
        { "ch", 0, &_XfCharCode_type },
        { "width", 0, &_XfCoord_type },
        { "x", 0, &_XfCoord_type },
        { "y", 0, &_XfCoord_type },
    /* image */
        { "r", 0, &_XfRaster_type },
        { "x", 0, &_XfCoord_type },
        { "y", 0, &_XfCoord_type },
    /* stencil */
        { "r", 0, &_XfRaster_type },
        { "x", 0, &_XfCoord_type },
        { "y", 0, &_XfCoord_type },
    /* matrix */
        { "_p", 0, &_XfTransformObj_type },
    /* transform */
        { "t", 0, &_XfTransformObj_type },
    /* clip_rect */
        { "x0", 0, &_XfCoord_type },
        { "y0", 0, &_XfCoord_type },
        { "x1", 0, &_XfCoord_type },
        { "y1", 0, &_XfCoord_type },
    /* is_visible */
        { "r", 0, &_XfRegion_type },
    /* comment */
        { "s", 0, &_XfCharString_type },
    /* page_number */
        { "s", 0, &_XfCharString_type }
};
extern TypeObj_Descriptor _XfFrescoObject_type;
TypeObj_Descriptor* _XfPainterObj_parents[] = { &_XfFrescoObject_type, nil };
extern TypeObjId _XfPainterObj_tid;
extern void _XfPainterObj_receive(BaseObjectRef, ULong, MarshalBuffer&);
TypeObj_Descriptor _XfPainterObj_type = {
    /* type */ 0,
    /* id */ &_XfPainterObj_tid,
    "PainterObj",
    _XfPainterObj_parents, /* offsets */ nil, /* excepts */ nil,
    _XfPainterObj_methods, _XfPainterObj_params,
    &_XfPainterObj_receive
};

PainterObjRef PainterObj::_narrow(BaseObjectRef o) {
    return (PainterObjRef)_BaseObject_tnarrow(
        o, _XfPainterObj_tid, &PainterObjStub::_create
    );
}
TypeObjId PainterObjType::_tid() { return _XfPainterObj_tid; }
void _XfPainterObj_receive(BaseObjectRef _object, ULong _m, MarshalBuffer& _b) {
    extern TypeObjId _XfPainterObj_tid;
    PainterObjRef _this = (PainterObjRef)_BaseObject_tcast(_object, _XfPainterObj_tid);
    switch (_m) {
        case /* to_coord */ 0: {
            extern MarshalBuffer::ArgInfo _XfPainterObj_to_coord_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            PixelCoord p;
            _arg[1].u_addr = &p;
            _b.receive(_XfPainterObj_to_coord_pinfo, _arg);
            _arg[0].u_float = _this->to_coord(p);
            _b.reply(_XfPainterObj_to_coord_pinfo, _arg);
            break;
        }
        case /* to_pixels */ 1: {
            extern MarshalBuffer::ArgInfo _XfPainterObj_to_pixels_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            Coord c;
            _arg[1].u_addr = &c;
            _b.receive(_XfPainterObj_to_pixels_pinfo, _arg);
            _arg[0].u_long = _this->to_pixels(c);
            _b.reply(_XfPainterObj_to_pixels_pinfo, _arg);
            break;
        }
        case /* to_pixels_coord */ 2: {
            extern MarshalBuffer::ArgInfo _XfPainterObj_to_pixels_coord_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            Coord c;
            _arg[1].u_addr = &c;
            _b.receive(_XfPainterObj_to_pixels_coord_pinfo, _arg);
            _arg[0].u_float = _this->to_pixels_coord(c);
            _b.reply(_XfPainterObj_to_pixels_coord_pinfo, _arg);
            break;
        }
        case /* begin_path */ 3: {
            extern MarshalBuffer::ArgInfo _XfPainterObj_begin_path_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _this->begin_path();
            _b.reply(_XfPainterObj_begin_path_pinfo, _arg);
            break;
        }
        case /* move_to */ 4: {
            extern MarshalBuffer::ArgInfo _XfPainterObj_move_to_pinfo;
            MarshalBuffer::ArgValue _arg[3];
            Coord x;
            _arg[1].u_addr = &x;
            Coord y;
            _arg[2].u_addr = &y;
            _b.receive(_XfPainterObj_move_to_pinfo, _arg);
            _this->move_to(x, y);
            _b.reply(_XfPainterObj_move_to_pinfo, _arg);
            break;
        }
        case /* line_to */ 5: {
            extern MarshalBuffer::ArgInfo _XfPainterObj_line_to_pinfo;
            MarshalBuffer::ArgValue _arg[3];
            Coord x;
            _arg[1].u_addr = &x;
            Coord y;
            _arg[2].u_addr = &y;
            _b.receive(_XfPainterObj_line_to_pinfo, _arg);
            _this->line_to(x, y);
            _b.reply(_XfPainterObj_line_to_pinfo, _arg);
            break;
        }
        case /* curve_to */ 6: {
            extern MarshalBuffer::ArgInfo _XfPainterObj_curve_to_pinfo;
            MarshalBuffer::ArgValue _arg[7];
            Coord x;
            _arg[1].u_addr = &x;
            Coord y;
            _arg[2].u_addr = &y;
            Coord x1;
            _arg[3].u_addr = &x1;
            Coord y1;
            _arg[4].u_addr = &y1;
            Coord x2;
            _arg[5].u_addr = &x2;
            Coord y2;
            _arg[6].u_addr = &y2;
            _b.receive(_XfPainterObj_curve_to_pinfo, _arg);
            _this->curve_to(x, y, x1, y1, x2, y2);
            _b.reply(_XfPainterObj_curve_to_pinfo, _arg);
            break;
        }
        case /* close_path */ 7: {
            extern MarshalBuffer::ArgInfo _XfPainterObj_close_path_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _this->close_path();
            _b.reply(_XfPainterObj_close_path_pinfo, _arg);
            break;
        }
        case /* _get_brush_attr */ 8: {
            extern MarshalBuffer::ArgInfo _XfPainterObj__get_brush_attr_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_objref = _this->_c_brush_attr();
            _b.reply(_XfPainterObj__get_brush_attr_pinfo, _arg);
            break;
        }
        case /* _set_brush_attr */ 9: {
            extern MarshalBuffer::ArgInfo _XfPainterObj__set_brush_attr_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            BrushRef _p;
            _arg[1].u_addr = &_p;
            _b.receive(_XfPainterObj__set_brush_attr_pinfo, _arg);
            _this->_c_brush_attr(_p);
            _b.reply(_XfPainterObj__set_brush_attr_pinfo, _arg);
            break;
        }
        case /* _get_color_attr */ 10: {
            extern MarshalBuffer::ArgInfo _XfPainterObj__get_color_attr_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_objref = _this->_c_color_attr();
            _b.reply(_XfPainterObj__get_color_attr_pinfo, _arg);
            break;
        }
        case /* _set_color_attr */ 11: {
            extern MarshalBuffer::ArgInfo _XfPainterObj__set_color_attr_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            ColorRef _p;
            _arg[1].u_addr = &_p;
            _b.receive(_XfPainterObj__set_color_attr_pinfo, _arg);
            _this->_c_color_attr(_p);
            _b.reply(_XfPainterObj__set_color_attr_pinfo, _arg);
            break;
        }
        case /* _get_font_attr */ 12: {
            extern MarshalBuffer::ArgInfo _XfPainterObj__get_font_attr_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_objref = _this->_c_font_attr();
            _b.reply(_XfPainterObj__get_font_attr_pinfo, _arg);
            break;
        }
        case /* _set_font_attr */ 13: {
            extern MarshalBuffer::ArgInfo _XfPainterObj__set_font_attr_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            FontRef _p;
            _arg[1].u_addr = &_p;
            _b.receive(_XfPainterObj__set_font_attr_pinfo, _arg);
            _this->_c_font_attr(_p);
            _b.reply(_XfPainterObj__set_font_attr_pinfo, _arg);
            break;
        }
        case /* stroke */ 14: {
            extern MarshalBuffer::ArgInfo _XfPainterObj_stroke_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _this->stroke();
            _b.reply(_XfPainterObj_stroke_pinfo, _arg);
            break;
        }
        case /* fill */ 15: {
            extern MarshalBuffer::ArgInfo _XfPainterObj_fill_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _this->fill();
            _b.reply(_XfPainterObj_fill_pinfo, _arg);
            break;
        }
        case /* line */ 16: {
            extern MarshalBuffer::ArgInfo _XfPainterObj_line_pinfo;
            MarshalBuffer::ArgValue _arg[5];
            Coord x0;
            _arg[1].u_addr = &x0;
            Coord y0;
            _arg[2].u_addr = &y0;
            Coord x1;
            _arg[3].u_addr = &x1;
            Coord y1;
            _arg[4].u_addr = &y1;
            _b.receive(_XfPainterObj_line_pinfo, _arg);
            _this->line(x0, y0, x1, y1);
            _b.reply(_XfPainterObj_line_pinfo, _arg);
            break;
        }
        case /* rect */ 17: {
            extern MarshalBuffer::ArgInfo _XfPainterObj_rect_pinfo;
            MarshalBuffer::ArgValue _arg[5];
            Coord x0;
            _arg[1].u_addr = &x0;
            Coord y0;
            _arg[2].u_addr = &y0;
            Coord x1;
            _arg[3].u_addr = &x1;
            Coord y1;
            _arg[4].u_addr = &y1;
            _b.receive(_XfPainterObj_rect_pinfo, _arg);
            _this->rect(x0, y0, x1, y1);
            _b.reply(_XfPainterObj_rect_pinfo, _arg);
            break;
        }
        case /* fill_rect */ 18: {
            extern MarshalBuffer::ArgInfo _XfPainterObj_fill_rect_pinfo;
            MarshalBuffer::ArgValue _arg[5];
            Coord x0;
            _arg[1].u_addr = &x0;
            Coord y0;
            _arg[2].u_addr = &y0;
            Coord x1;
            _arg[3].u_addr = &x1;
            Coord y1;
            _arg[4].u_addr = &y1;
            _b.receive(_XfPainterObj_fill_rect_pinfo, _arg);
            _this->fill_rect(x0, y0, x1, y1);
            _b.reply(_XfPainterObj_fill_rect_pinfo, _arg);
            break;
        }
        case /* character */ 19: {
            extern MarshalBuffer::ArgInfo _XfPainterObj_character_pinfo;
            MarshalBuffer::ArgValue _arg[5];
            CharCode ch;
            _arg[1].u_addr = &ch;
            Coord width;
            _arg[2].u_addr = &width;
            Coord x;
            _arg[3].u_addr = &x;
            Coord y;
            _arg[4].u_addr = &y;
            _b.receive(_XfPainterObj_character_pinfo, _arg);
            _this->character(ch, width, x, y);
            _b.reply(_XfPainterObj_character_pinfo, _arg);
            break;
        }
        case /* image */ 20: {
            extern MarshalBuffer::ArgInfo _XfPainterObj_image_pinfo;
            MarshalBuffer::ArgValue _arg[4];
            RasterRef r;
            _arg[1].u_addr = &r;
            Coord x;
            _arg[2].u_addr = &x;
            Coord y;
            _arg[3].u_addr = &y;
            _b.receive(_XfPainterObj_image_pinfo, _arg);
            _this->image(r, x, y);
            _b.reply(_XfPainterObj_image_pinfo, _arg);
            break;
        }
        case /* stencil */ 21: {
            extern MarshalBuffer::ArgInfo _XfPainterObj_stencil_pinfo;
            MarshalBuffer::ArgValue _arg[4];
            RasterRef r;
            _arg[1].u_addr = &r;
            Coord x;
            _arg[2].u_addr = &x;
            Coord y;
            _arg[3].u_addr = &y;
            _b.receive(_XfPainterObj_stencil_pinfo, _arg);
            _this->stencil(r, x, y);
            _b.reply(_XfPainterObj_stencil_pinfo, _arg);
            break;
        }
        case /* _get_matrix */ 22: {
            extern MarshalBuffer::ArgInfo _XfPainterObj__get_matrix_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_objref = _this->_c_matrix();
            _b.reply(_XfPainterObj__get_matrix_pinfo, _arg);
            break;
        }
        case /* _set_matrix */ 23: {
            extern MarshalBuffer::ArgInfo _XfPainterObj__set_matrix_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            TransformObjRef _p;
            _arg[1].u_addr = &_p;
            _b.receive(_XfPainterObj__set_matrix_pinfo, _arg);
            _this->_c_matrix(_p);
            _b.reply(_XfPainterObj__set_matrix_pinfo, _arg);
            break;
        }
        case /* push_matrix */ 24: {
            extern MarshalBuffer::ArgInfo _XfPainterObj_push_matrix_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _this->push_matrix();
            _b.reply(_XfPainterObj_push_matrix_pinfo, _arg);
            break;
        }
        case /* pop_matrix */ 25: {
            extern MarshalBuffer::ArgInfo _XfPainterObj_pop_matrix_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _this->pop_matrix();
            _b.reply(_XfPainterObj_pop_matrix_pinfo, _arg);
            break;
        }
        case /* transform */ 26: {
            extern MarshalBuffer::ArgInfo _XfPainterObj_transform_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            TransformObjRef t;
            _arg[1].u_addr = &t;
            _b.receive(_XfPainterObj_transform_pinfo, _arg);
            _this->transform(t);
            _b.reply(_XfPainterObj_transform_pinfo, _arg);
            break;
        }
        case /* clip */ 27: {
            extern MarshalBuffer::ArgInfo _XfPainterObj_clip_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _this->clip();
            _b.reply(_XfPainterObj_clip_pinfo, _arg);
            break;
        }
        case /* clip_rect */ 28: {
            extern MarshalBuffer::ArgInfo _XfPainterObj_clip_rect_pinfo;
            MarshalBuffer::ArgValue _arg[5];
            Coord x0;
            _arg[1].u_addr = &x0;
            Coord y0;
            _arg[2].u_addr = &y0;
            Coord x1;
            _arg[3].u_addr = &x1;
            Coord y1;
            _arg[4].u_addr = &y1;
            _b.receive(_XfPainterObj_clip_rect_pinfo, _arg);
            _this->clip_rect(x0, y0, x1, y1);
            _b.reply(_XfPainterObj_clip_rect_pinfo, _arg);
            break;
        }
        case /* push_clipping */ 29: {
            extern MarshalBuffer::ArgInfo _XfPainterObj_push_clipping_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _this->push_clipping();
            _b.reply(_XfPainterObj_push_clipping_pinfo, _arg);
            break;
        }
        case /* pop_clipping */ 30: {
            extern MarshalBuffer::ArgInfo _XfPainterObj_pop_clipping_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _this->pop_clipping();
            _b.reply(_XfPainterObj_pop_clipping_pinfo, _arg);
            break;
        }
        case /* is_visible */ 31: {
            extern MarshalBuffer::ArgInfo _XfPainterObj_is_visible_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            RegionRef r;
            _arg[1].u_addr = &r;
            _b.receive(_XfPainterObj_is_visible_pinfo, _arg);
            _arg[0].u_boolean = _this->is_visible(r);
            _b.reply(_XfPainterObj_is_visible_pinfo, _arg);
            break;
        }
        case /* visible */ 32: {
            extern MarshalBuffer::ArgInfo _XfPainterObj_visible_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_objref = _this->_c_visible();
            _b.reply(_XfPainterObj_visible_pinfo, _arg);
            break;
        }
        case /* comment */ 33: {
            extern MarshalBuffer::ArgInfo _XfPainterObj_comment_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            CharStringRef s;
            _arg[1].u_addr = &s;
            _b.receive(_XfPainterObj_comment_pinfo, _arg);
            _this->comment(s);
            _b.reply(_XfPainterObj_comment_pinfo, _arg);
            break;
        }
        case /* page_number */ 34: {
            extern MarshalBuffer::ArgInfo _XfPainterObj_page_number_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            CharStringRef s;
            _arg[1].u_addr = &s;
            _b.receive(_XfPainterObj_page_number_pinfo, _arg);
            _this->page_number(s);
            _b.reply(_XfPainterObj_page_number_pinfo, _arg);
            break;
        }
    }
}
PainterObjStub::PainterObjStub(Exchange* e) { exch_ = e; }
PainterObjStub::~PainterObjStub() { }
BaseObjectRef PainterObjStub::_create(Exchange* e) {
    return (BaseObjectRef)(void*)new PainterObjStub(e);
}
Exchange* PainterObjStub::_exchange() {
    return exch_;
}
MarshalBuffer::ArgDesc _XfPainterObj_to_coord_pdesc[3] = { 2, 48, 33 };
MarshalBuffer::ArgInfo _XfPainterObj_to_coord_pinfo = {
    &_XfPainterObj_tid, 0, _XfPainterObj_to_coord_pdesc, 0
};
Coord PainterObjType::to_coord(PixelCoord p) {
    MarshalBuffer _b;
    extern TypeObjId _XfPainterObj_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_long = p;
    _b.invoke(this, _XfPainterObj_to_coord_pinfo, _arg);
    return _arg[0].u_float;
}
MarshalBuffer::ArgDesc _XfPainterObj_to_pixels_pdesc[3] = { 2, 32, 49 };
MarshalBuffer::ArgInfo _XfPainterObj_to_pixels_pinfo = {
    &_XfPainterObj_tid, 1, _XfPainterObj_to_pixels_pdesc, 0
};
PixelCoord PainterObjType::to_pixels(Coord c) {
    MarshalBuffer _b;
    extern TypeObjId _XfPainterObj_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_float = c;
    _b.invoke(this, _XfPainterObj_to_pixels_pinfo, _arg);
    return _arg[0].u_long;
}
MarshalBuffer::ArgDesc _XfPainterObj_to_pixels_coord_pdesc[3] = { 2, 48, 49 };
MarshalBuffer::ArgInfo _XfPainterObj_to_pixels_coord_pinfo = {
    &_XfPainterObj_tid, 2, _XfPainterObj_to_pixels_coord_pdesc, 0
};
Coord PainterObjType::to_pixels_coord(Coord c) {
    MarshalBuffer _b;
    extern TypeObjId _XfPainterObj_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_float = c;
    _b.invoke(this, _XfPainterObj_to_pixels_coord_pinfo, _arg);
    return _arg[0].u_float;
}
MarshalBuffer::ArgDesc _XfPainterObj_begin_path_pdesc[2] = { 1, 4 };
MarshalBuffer::ArgInfo _XfPainterObj_begin_path_pinfo = {
    &_XfPainterObj_tid, 3, _XfPainterObj_begin_path_pdesc, 0
};
void PainterObjType::begin_path() {
    MarshalBuffer _b;
    extern TypeObjId _XfPainterObj_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfPainterObj_begin_path_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfPainterObj_move_to_pdesc[4] = { 3, 4, 49, 49 };
MarshalBuffer::ArgInfo _XfPainterObj_move_to_pinfo = {
    &_XfPainterObj_tid, 4, _XfPainterObj_move_to_pdesc, 0
};
void PainterObjType::move_to(Coord x, Coord y) {
    MarshalBuffer _b;
    extern TypeObjId _XfPainterObj_tid;
    MarshalBuffer::ArgValue _arg[3];
    _arg[1].u_float = x;
    _arg[2].u_float = y;
    _b.invoke(this, _XfPainterObj_move_to_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfPainterObj_line_to_pdesc[4] = { 3, 4, 49, 49 };
MarshalBuffer::ArgInfo _XfPainterObj_line_to_pinfo = {
    &_XfPainterObj_tid, 5, _XfPainterObj_line_to_pdesc, 0
};
void PainterObjType::line_to(Coord x, Coord y) {
    MarshalBuffer _b;
    extern TypeObjId _XfPainterObj_tid;
    MarshalBuffer::ArgValue _arg[3];
    _arg[1].u_float = x;
    _arg[2].u_float = y;
    _b.invoke(this, _XfPainterObj_line_to_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfPainterObj_curve_to_pdesc[8] = { 7, 4, 49, 49, 49, 49, 49, 49 };
MarshalBuffer::ArgInfo _XfPainterObj_curve_to_pinfo = {
    &_XfPainterObj_tid, 6, _XfPainterObj_curve_to_pdesc, 0
};
void PainterObjType::curve_to(Coord x, Coord y, Coord x1, Coord y1, Coord x2, Coord y2) {
    MarshalBuffer _b;
    extern TypeObjId _XfPainterObj_tid;
    MarshalBuffer::ArgValue _arg[7];
    _arg[1].u_float = x;
    _arg[2].u_float = y;
    _arg[3].u_float = x1;
    _arg[4].u_float = y1;
    _arg[5].u_float = x2;
    _arg[6].u_float = y2;
    _b.invoke(this, _XfPainterObj_curve_to_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfPainterObj_close_path_pdesc[2] = { 1, 4 };
MarshalBuffer::ArgInfo _XfPainterObj_close_path_pinfo = {
    &_XfPainterObj_tid, 7, _XfPainterObj_close_path_pdesc, 0
};
void PainterObjType::close_path() {
    MarshalBuffer _b;
    extern TypeObjId _XfPainterObj_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfPainterObj_close_path_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfPainterObj__get_brush_attr_pdesc[2] = { 1, 60 };
MarshalBuffer::ArgMarshal _XfPainterObj__get_brush_attr_pfunc[] = {
    &BrushStub::_create
};
MarshalBuffer::ArgInfo _XfPainterObj__get_brush_attr_pinfo = {
    &_XfPainterObj_tid, 8, _XfPainterObj__get_brush_attr_pdesc, _XfPainterObj__get_brush_attr_pfunc
};
BrushRef PainterObjType::_c_brush_attr() {
    MarshalBuffer _b;
    extern TypeObjId _XfPainterObj_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfPainterObj__get_brush_attr_pinfo, _arg);
    return (BrushRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfPainterObj__set_brush_attr_pdesc[3] = { 2, 4, 61 };
MarshalBuffer::ArgMarshal _XfPainterObj__set_brush_attr_pfunc[] = {
    &BrushStub::_create,

};
MarshalBuffer::ArgInfo _XfPainterObj__set_brush_attr_pinfo = {
    &_XfPainterObj_tid, 9, _XfPainterObj__set_brush_attr_pdesc, _XfPainterObj__set_brush_attr_pfunc
};
void PainterObjType::_c_brush_attr(Brush_in _p) {
    MarshalBuffer _b;
    extern TypeObjId _XfPainterObj_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = _p;
    _b.invoke(this, _XfPainterObj__set_brush_attr_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfPainterObj__get_color_attr_pdesc[2] = { 1, 60 };
MarshalBuffer::ArgMarshal _XfPainterObj__get_color_attr_pfunc[] = {
    &ColorStub::_create
};
MarshalBuffer::ArgInfo _XfPainterObj__get_color_attr_pinfo = {
    &_XfPainterObj_tid, 10, _XfPainterObj__get_color_attr_pdesc, _XfPainterObj__get_color_attr_pfunc
};
ColorRef PainterObjType::_c_color_attr() {
    MarshalBuffer _b;
    extern TypeObjId _XfPainterObj_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfPainterObj__get_color_attr_pinfo, _arg);
    return (ColorRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfPainterObj__set_color_attr_pdesc[3] = { 2, 4, 61 };
MarshalBuffer::ArgMarshal _XfPainterObj__set_color_attr_pfunc[] = {
    &ColorStub::_create,

};
MarshalBuffer::ArgInfo _XfPainterObj__set_color_attr_pinfo = {
    &_XfPainterObj_tid, 11, _XfPainterObj__set_color_attr_pdesc, _XfPainterObj__set_color_attr_pfunc
};
void PainterObjType::_c_color_attr(Color_in _p) {
    MarshalBuffer _b;
    extern TypeObjId _XfPainterObj_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = _p;
    _b.invoke(this, _XfPainterObj__set_color_attr_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfPainterObj__get_font_attr_pdesc[2] = { 1, 60 };
MarshalBuffer::ArgMarshal _XfPainterObj__get_font_attr_pfunc[] = {
    &FontStub::_create
};
MarshalBuffer::ArgInfo _XfPainterObj__get_font_attr_pinfo = {
    &_XfPainterObj_tid, 12, _XfPainterObj__get_font_attr_pdesc, _XfPainterObj__get_font_attr_pfunc
};
FontRef PainterObjType::_c_font_attr() {
    MarshalBuffer _b;
    extern TypeObjId _XfPainterObj_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfPainterObj__get_font_attr_pinfo, _arg);
    return (FontRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfPainterObj__set_font_attr_pdesc[3] = { 2, 4, 61 };
MarshalBuffer::ArgMarshal _XfPainterObj__set_font_attr_pfunc[] = {
    &FontStub::_create,

};
MarshalBuffer::ArgInfo _XfPainterObj__set_font_attr_pinfo = {
    &_XfPainterObj_tid, 13, _XfPainterObj__set_font_attr_pdesc, _XfPainterObj__set_font_attr_pfunc
};
void PainterObjType::_c_font_attr(Font_in _p) {
    MarshalBuffer _b;
    extern TypeObjId _XfPainterObj_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = _p;
    _b.invoke(this, _XfPainterObj__set_font_attr_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfPainterObj_stroke_pdesc[2] = { 1, 4 };
MarshalBuffer::ArgInfo _XfPainterObj_stroke_pinfo = {
    &_XfPainterObj_tid, 14, _XfPainterObj_stroke_pdesc, 0
};
void PainterObjType::stroke() {
    MarshalBuffer _b;
    extern TypeObjId _XfPainterObj_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfPainterObj_stroke_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfPainterObj_fill_pdesc[2] = { 1, 4 };
MarshalBuffer::ArgInfo _XfPainterObj_fill_pinfo = {
    &_XfPainterObj_tid, 15, _XfPainterObj_fill_pdesc, 0
};
void PainterObjType::fill() {
    MarshalBuffer _b;
    extern TypeObjId _XfPainterObj_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfPainterObj_fill_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfPainterObj_line_pdesc[6] = { 5, 4, 49, 49, 49, 49 };
MarshalBuffer::ArgInfo _XfPainterObj_line_pinfo = {
    &_XfPainterObj_tid, 16, _XfPainterObj_line_pdesc, 0
};
void PainterObjType::line(Coord x0, Coord y0, Coord x1, Coord y1) {
    MarshalBuffer _b;
    extern TypeObjId _XfPainterObj_tid;
    MarshalBuffer::ArgValue _arg[5];
    _arg[1].u_float = x0;
    _arg[2].u_float = y0;
    _arg[3].u_float = x1;
    _arg[4].u_float = y1;
    _b.invoke(this, _XfPainterObj_line_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfPainterObj_rect_pdesc[6] = { 5, 4, 49, 49, 49, 49 };
MarshalBuffer::ArgInfo _XfPainterObj_rect_pinfo = {
    &_XfPainterObj_tid, 17, _XfPainterObj_rect_pdesc, 0
};
void PainterObjType::rect(Coord x0, Coord y0, Coord x1, Coord y1) {
    MarshalBuffer _b;
    extern TypeObjId _XfPainterObj_tid;
    MarshalBuffer::ArgValue _arg[5];
    _arg[1].u_float = x0;
    _arg[2].u_float = y0;
    _arg[3].u_float = x1;
    _arg[4].u_float = y1;
    _b.invoke(this, _XfPainterObj_rect_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfPainterObj_fill_rect_pdesc[6] = { 5, 4, 49, 49, 49, 49 };
MarshalBuffer::ArgInfo _XfPainterObj_fill_rect_pinfo = {
    &_XfPainterObj_tid, 18, _XfPainterObj_fill_rect_pdesc, 0
};
void PainterObjType::fill_rect(Coord x0, Coord y0, Coord x1, Coord y1) {
    MarshalBuffer _b;
    extern TypeObjId _XfPainterObj_tid;
    MarshalBuffer::ArgValue _arg[5];
    _arg[1].u_float = x0;
    _arg[2].u_float = y0;
    _arg[3].u_float = x1;
    _arg[4].u_float = y1;
    _b.invoke(this, _XfPainterObj_fill_rect_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfPainterObj_character_pdesc[6] = { 5, 4, 33, 49, 49, 49 };
MarshalBuffer::ArgInfo _XfPainterObj_character_pinfo = {
    &_XfPainterObj_tid, 19, _XfPainterObj_character_pdesc, 0
};
void PainterObjType::character(CharCode ch, Coord width, Coord x, Coord y) {
    MarshalBuffer _b;
    extern TypeObjId _XfPainterObj_tid;
    MarshalBuffer::ArgValue _arg[5];
    _arg[1].u_long = ch;
    _arg[2].u_float = width;
    _arg[3].u_float = x;
    _arg[4].u_float = y;
    _b.invoke(this, _XfPainterObj_character_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfPainterObj_image_pdesc[5] = { 4, 4, 61, 49, 49 };
MarshalBuffer::ArgMarshal _XfPainterObj_image_pfunc[] = {
    &RasterStub::_create,

};
MarshalBuffer::ArgInfo _XfPainterObj_image_pinfo = {
    &_XfPainterObj_tid, 20, _XfPainterObj_image_pdesc, _XfPainterObj_image_pfunc
};
void PainterObjType::image(Raster_in r, Coord x, Coord y) {
    MarshalBuffer _b;
    extern TypeObjId _XfPainterObj_tid;
    MarshalBuffer::ArgValue _arg[4];
    _arg[1].u_objref = r;
    _arg[2].u_float = x;
    _arg[3].u_float = y;
    _b.invoke(this, _XfPainterObj_image_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfPainterObj_stencil_pdesc[5] = { 4, 4, 61, 49, 49 };
MarshalBuffer::ArgMarshal _XfPainterObj_stencil_pfunc[] = {
    &RasterStub::_create,

};
MarshalBuffer::ArgInfo _XfPainterObj_stencil_pinfo = {
    &_XfPainterObj_tid, 21, _XfPainterObj_stencil_pdesc, _XfPainterObj_stencil_pfunc
};
void PainterObjType::stencil(Raster_in r, Coord x, Coord y) {
    MarshalBuffer _b;
    extern TypeObjId _XfPainterObj_tid;
    MarshalBuffer::ArgValue _arg[4];
    _arg[1].u_objref = r;
    _arg[2].u_float = x;
    _arg[3].u_float = y;
    _b.invoke(this, _XfPainterObj_stencil_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfPainterObj__get_matrix_pdesc[2] = { 1, 60 };
MarshalBuffer::ArgMarshal _XfPainterObj__get_matrix_pfunc[] = {
    &TransformObjStub::_create
};
MarshalBuffer::ArgInfo _XfPainterObj__get_matrix_pinfo = {
    &_XfPainterObj_tid, 22, _XfPainterObj__get_matrix_pdesc, _XfPainterObj__get_matrix_pfunc
};
TransformObjRef PainterObjType::_c_matrix() {
    MarshalBuffer _b;
    extern TypeObjId _XfPainterObj_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfPainterObj__get_matrix_pinfo, _arg);
    return (TransformObjRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfPainterObj__set_matrix_pdesc[3] = { 2, 4, 61 };
MarshalBuffer::ArgMarshal _XfPainterObj__set_matrix_pfunc[] = {
    &TransformObjStub::_create,

};
MarshalBuffer::ArgInfo _XfPainterObj__set_matrix_pinfo = {
    &_XfPainterObj_tid, 23, _XfPainterObj__set_matrix_pdesc, _XfPainterObj__set_matrix_pfunc
};
void PainterObjType::_c_matrix(TransformObj_in _p) {
    MarshalBuffer _b;
    extern TypeObjId _XfPainterObj_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = _p;
    _b.invoke(this, _XfPainterObj__set_matrix_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfPainterObj_push_matrix_pdesc[2] = { 1, 4 };
MarshalBuffer::ArgInfo _XfPainterObj_push_matrix_pinfo = {
    &_XfPainterObj_tid, 24, _XfPainterObj_push_matrix_pdesc, 0
};
void PainterObjType::push_matrix() {
    MarshalBuffer _b;
    extern TypeObjId _XfPainterObj_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfPainterObj_push_matrix_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfPainterObj_pop_matrix_pdesc[2] = { 1, 4 };
MarshalBuffer::ArgInfo _XfPainterObj_pop_matrix_pinfo = {
    &_XfPainterObj_tid, 25, _XfPainterObj_pop_matrix_pdesc, 0
};
void PainterObjType::pop_matrix() {
    MarshalBuffer _b;
    extern TypeObjId _XfPainterObj_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfPainterObj_pop_matrix_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfPainterObj_transform_pdesc[3] = { 2, 4, 61 };
MarshalBuffer::ArgMarshal _XfPainterObj_transform_pfunc[] = {
    &TransformObjStub::_create,

};
MarshalBuffer::ArgInfo _XfPainterObj_transform_pinfo = {
    &_XfPainterObj_tid, 26, _XfPainterObj_transform_pdesc, _XfPainterObj_transform_pfunc
};
void PainterObjType::transform(TransformObj_in t) {
    MarshalBuffer _b;
    extern TypeObjId _XfPainterObj_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = t;
    _b.invoke(this, _XfPainterObj_transform_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfPainterObj_clip_pdesc[2] = { 1, 4 };
MarshalBuffer::ArgInfo _XfPainterObj_clip_pinfo = {
    &_XfPainterObj_tid, 27, _XfPainterObj_clip_pdesc, 0
};
void PainterObjType::clip() {
    MarshalBuffer _b;
    extern TypeObjId _XfPainterObj_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfPainterObj_clip_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfPainterObj_clip_rect_pdesc[6] = { 5, 4, 49, 49, 49, 49 };
MarshalBuffer::ArgInfo _XfPainterObj_clip_rect_pinfo = {
    &_XfPainterObj_tid, 28, _XfPainterObj_clip_rect_pdesc, 0
};
void PainterObjType::clip_rect(Coord x0, Coord y0, Coord x1, Coord y1) {
    MarshalBuffer _b;
    extern TypeObjId _XfPainterObj_tid;
    MarshalBuffer::ArgValue _arg[5];
    _arg[1].u_float = x0;
    _arg[2].u_float = y0;
    _arg[3].u_float = x1;
    _arg[4].u_float = y1;
    _b.invoke(this, _XfPainterObj_clip_rect_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfPainterObj_push_clipping_pdesc[2] = { 1, 8 };
MarshalBuffer::ArgInfo _XfPainterObj_push_clipping_pinfo = {
    &_XfPainterObj_tid, 29, _XfPainterObj_push_clipping_pdesc, 0
};
void PainterObjType::push_clipping() {
    MarshalBuffer _b;
    extern TypeObjId _XfPainterObj_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfPainterObj_push_clipping_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfPainterObj_pop_clipping_pdesc[2] = { 1, 4 };
MarshalBuffer::ArgInfo _XfPainterObj_pop_clipping_pinfo = {
    &_XfPainterObj_tid, 30, _XfPainterObj_pop_clipping_pdesc, 0
};
void PainterObjType::pop_clipping() {
    MarshalBuffer _b;
    extern TypeObjId _XfPainterObj_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfPainterObj_pop_clipping_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfPainterObj_is_visible_pdesc[3] = { 2, 12, 61 };
MarshalBuffer::ArgMarshal _XfPainterObj_is_visible_pfunc[] = {
    &RegionStub::_create,

};
MarshalBuffer::ArgInfo _XfPainterObj_is_visible_pinfo = {
    &_XfPainterObj_tid, 31, _XfPainterObj_is_visible_pdesc, _XfPainterObj_is_visible_pfunc
};
Boolean PainterObjType::is_visible(Region_in r) {
    MarshalBuffer _b;
    extern TypeObjId _XfPainterObj_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = r;
    _b.invoke(this, _XfPainterObj_is_visible_pinfo, _arg);
    return _arg[0].u_boolean;
}
MarshalBuffer::ArgDesc _XfPainterObj_visible_pdesc[2] = { 1, 60 };
MarshalBuffer::ArgMarshal _XfPainterObj_visible_pfunc[] = {
    &RegionStub::_create
};
MarshalBuffer::ArgInfo _XfPainterObj_visible_pinfo = {
    &_XfPainterObj_tid, 32, _XfPainterObj_visible_pdesc, _XfPainterObj_visible_pfunc
};
RegionRef PainterObjType::_c_visible() {
    MarshalBuffer _b;
    extern TypeObjId _XfPainterObj_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfPainterObj_visible_pinfo, _arg);
    return (RegionRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfPainterObj_comment_pdesc[3] = { 2, 4, 61 };
MarshalBuffer::ArgMarshal _XfPainterObj_comment_pfunc[] = {
    &CharStringStub::_create,

};
MarshalBuffer::ArgInfo _XfPainterObj_comment_pinfo = {
    &_XfPainterObj_tid, 33, _XfPainterObj_comment_pdesc, _XfPainterObj_comment_pfunc
};
void PainterObjType::comment(CharString_in s) {
    MarshalBuffer _b;
    extern TypeObjId _XfPainterObj_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = s;
    _b.invoke(this, _XfPainterObj_comment_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfPainterObj_page_number_pdesc[3] = { 2, 4, 61 };
MarshalBuffer::ArgMarshal _XfPainterObj_page_number_pfunc[] = {
    &CharStringStub::_create,

};
MarshalBuffer::ArgInfo _XfPainterObj_page_number_pinfo = {
    &_XfPainterObj_tid, 34, _XfPainterObj_page_number_pdesc, _XfPainterObj_page_number_pfunc
};
void PainterObjType::page_number(CharString_in s) {
    MarshalBuffer _b;
    extern TypeObjId _XfPainterObj_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = s;
    _b.invoke(this, _XfPainterObj_page_number_pinfo, _arg);
}
//+

//+ XPainterImpl(PainterObj)
extern TypeObj_Descriptor _XfPainterObj_type;
TypeObj_Descriptor* _XfXPainterImpl_parents[] = { &_XfPainterObj_type, nil };
extern TypeObjId _XfXPainterImpl_tid;
TypeObj_Descriptor _XfXPainterImpl_type = {
    /* type */ 0,
    /* id */ &_XfXPainterImpl_tid,
    "XPainterImpl",
    _XfXPainterImpl_parents, /* offsets */ nil, /* excepts */ nil,
    /* methods */ nil, /* params */ nil,
    /* receive */ nil
};

XPainterImpl* XPainterImpl::_narrow(BaseObjectRef o) {
    return (XPainterImpl*)_BaseObject_tnarrow(
        o, _XfXPainterImpl_tid, 0
    );
}
TypeObjId XPainterImpl::_tid() { return _XfXPainterImpl_tid; }
//+

//+ Raster::%init,type+dii,client
RasterType::RasterType() { }
RasterType::~RasterType() { }
void* RasterType::_this() { return this; }

extern TypeObj_Descriptor _XfRaster_Element_type;

TypeObj_OpData _XfRaster_methods[] = {
    { "equal", &_XfBoolean_type, 1 },
    { "hash", &_XfULong_type, 0 },
    { "rows", &_XfRaster_Index_type, 0 },
    { "columns", &_XfRaster_Index_type, 0 },
    { "origin_x", &_XfRaster_Index_type, 0 },
    { "origin_y", &_XfRaster_Index_type, 0 },
    { "peek", &_Xfvoid_type, 3 },
    { "poke", &_Xfvoid_type, 3 },
    { "_get_scale", &_XfCoord_type, 0 },
    { "_set_scale", &_Xfvoid_type, 1 },
    { 0, 0, 0 }
};
TypeObj_ParamData _XfRaster_params[] = {
    /* equal */
        { "r", 0, &_XfRaster_type },
    /* peek */
        { "row", 0, &_XfRaster_Index_type },
        { "column", 0, &_XfRaster_Index_type },
        { "e", 1, &_XfRaster_Element_type },
    /* poke */
        { "row", 0, &_XfRaster_Index_type },
        { "column", 0, &_XfRaster_Index_type },
        { "e", 0, &_XfRaster_Element_type },
    /* scale */
        { "_p", 0, &_XfCoord_type }
};
extern TypeObj_Descriptor _XfFrescoObject_type;
TypeObj_Descriptor* _XfRaster_parents[] = { &_XfFrescoObject_type, nil };
extern TypeObjId _XfRaster_tid;
extern void _XfRaster_receive(BaseObjectRef, ULong, MarshalBuffer&);
TypeObj_Descriptor _XfRaster_type = {
    /* type */ 0,
    /* id */ &_XfRaster_tid,
    "Raster",
    _XfRaster_parents, /* offsets */ nil, /* excepts */ nil,
    _XfRaster_methods, _XfRaster_params,
    &_XfRaster_receive
};

RasterRef Raster::_narrow(BaseObjectRef o) {
    return (RasterRef)_BaseObject_tnarrow(
        o, _XfRaster_tid, &RasterStub::_create
    );
}
TypeObjId RasterType::_tid() { return _XfRaster_tid; }
void _XfRaster_receive(BaseObjectRef _object, ULong _m, MarshalBuffer& _b) {
    extern TypeObjId _XfRaster_tid;
    RasterRef _this = (RasterRef)_BaseObject_tcast(_object, _XfRaster_tid);
    switch (_m) {
        case /* equal */ 0: {
            extern MarshalBuffer::ArgInfo _XfRaster_equal_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            RasterRef r;
            _arg[1].u_addr = &r;
            _b.receive(_XfRaster_equal_pinfo, _arg);
            _arg[0].u_boolean = _this->equal(r);
            _b.reply(_XfRaster_equal_pinfo, _arg);
            break;
        }
        case /* hash */ 1: {
            extern MarshalBuffer::ArgInfo _XfRaster_hash_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_unsigned_long = _this->hash();
            _b.reply(_XfRaster_hash_pinfo, _arg);
            break;
        }
        case /* rows */ 2: {
            extern MarshalBuffer::ArgInfo _XfRaster_rows_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_long = _this->rows();
            _b.reply(_XfRaster_rows_pinfo, _arg);
            break;
        }
        case /* columns */ 3: {
            extern MarshalBuffer::ArgInfo _XfRaster_columns_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_long = _this->columns();
            _b.reply(_XfRaster_columns_pinfo, _arg);
            break;
        }
        case /* origin_x */ 4: {
            extern MarshalBuffer::ArgInfo _XfRaster_origin_x_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_long = _this->origin_x();
            _b.reply(_XfRaster_origin_x_pinfo, _arg);
            break;
        }
        case /* origin_y */ 5: {
            extern MarshalBuffer::ArgInfo _XfRaster_origin_y_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_long = _this->origin_y();
            _b.reply(_XfRaster_origin_y_pinfo, _arg);
            break;
        }
        case /* peek */ 6: {
            extern MarshalBuffer::ArgInfo _XfRaster_peek_pinfo;
            MarshalBuffer::ArgValue _arg[4];
            Raster::Index row;
            _arg[1].u_addr = &row;
            Raster::Index column;
            _arg[2].u_addr = &column;
            Raster::Element e;
            _arg[3].u_addr = &e;
            _b.receive(_XfRaster_peek_pinfo, _arg);
            _this->peek(row, column, e);
            _b.reply(_XfRaster_peek_pinfo, _arg);
            break;
        }
        case /* poke */ 7: {
            extern MarshalBuffer::ArgInfo _XfRaster_poke_pinfo;
            MarshalBuffer::ArgValue _arg[4];
            Raster::Index row;
            _arg[1].u_addr = &row;
            Raster::Index column;
            _arg[2].u_addr = &column;
            Raster::Element e;
            _arg[3].u_addr = &e;
            _b.receive(_XfRaster_poke_pinfo, _arg);
            _this->poke(row, column, e);
            _b.reply(_XfRaster_poke_pinfo, _arg);
            break;
        }
        case /* _get_scale */ 8: {
            extern MarshalBuffer::ArgInfo _XfRaster__get_scale_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_float = _this->scale();
            _b.reply(_XfRaster__get_scale_pinfo, _arg);
            break;
        }
        case /* _set_scale */ 9: {
            extern MarshalBuffer::ArgInfo _XfRaster__set_scale_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            Coord _p;
            _arg[1].u_addr = &_p;
            _b.receive(_XfRaster__set_scale_pinfo, _arg);
            _this->scale(_p);
            _b.reply(_XfRaster__set_scale_pinfo, _arg);
            break;
        }
    }
}
extern void _XfRaster_Element_put(
    MarshalBuffer&, const Raster::Element&
);
extern void _XfRaster_Element_get(
    MarshalBuffer&, Raster::Element&
);
extern void _XfRaster_Element_put(
    MarshalBuffer&, const Raster::Element&
);
extern void _XfRaster_Element_get(
    MarshalBuffer&, Raster::Element&
);

RasterStub::RasterStub(Exchange* e) { exch_ = e; }
RasterStub::~RasterStub() { }
BaseObjectRef RasterStub::_create(Exchange* e) {
    return (BaseObjectRef)(void*)new RasterStub(e);
}
Exchange* RasterStub::_exchange() {
    return exch_;
}
MarshalBuffer::ArgDesc _XfRaster_equal_pdesc[3] = { 2, 12, 61 };
MarshalBuffer::ArgMarshal _XfRaster_equal_pfunc[] = {
    &RasterStub::_create,

};
MarshalBuffer::ArgInfo _XfRaster_equal_pinfo = {
    &_XfRaster_tid, 0, _XfRaster_equal_pdesc, _XfRaster_equal_pfunc
};
Boolean RasterType::equal(Raster_in r) {
    MarshalBuffer _b;
    extern TypeObjId _XfRaster_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = r;
    _b.invoke(this, _XfRaster_equal_pinfo, _arg);
    return _arg[0].u_boolean;
}
MarshalBuffer::ArgDesc _XfRaster_hash_pdesc[2] = { 1, 36 };
MarshalBuffer::ArgInfo _XfRaster_hash_pinfo = {
    &_XfRaster_tid, 1, _XfRaster_hash_pdesc, 0
};
ULong RasterType::hash() {
    MarshalBuffer _b;
    extern TypeObjId _XfRaster_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfRaster_hash_pinfo, _arg);
    return _arg[0].u_unsigned_long;
}
MarshalBuffer::ArgDesc _XfRaster_rows_pdesc[2] = { 1, 32 };
MarshalBuffer::ArgInfo _XfRaster_rows_pinfo = {
    &_XfRaster_tid, 2, _XfRaster_rows_pdesc, 0
};
Raster::Index RasterType::rows() {
    MarshalBuffer _b;
    extern TypeObjId _XfRaster_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfRaster_rows_pinfo, _arg);
    return _arg[0].u_long;
}
MarshalBuffer::ArgDesc _XfRaster_columns_pdesc[2] = { 1, 32 };
MarshalBuffer::ArgInfo _XfRaster_columns_pinfo = {
    &_XfRaster_tid, 3, _XfRaster_columns_pdesc, 0
};
Raster::Index RasterType::columns() {
    MarshalBuffer _b;
    extern TypeObjId _XfRaster_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfRaster_columns_pinfo, _arg);
    return _arg[0].u_long;
}
MarshalBuffer::ArgDesc _XfRaster_origin_x_pdesc[2] = { 1, 32 };
MarshalBuffer::ArgInfo _XfRaster_origin_x_pinfo = {
    &_XfRaster_tid, 4, _XfRaster_origin_x_pdesc, 0
};
Raster::Index RasterType::origin_x() {
    MarshalBuffer _b;
    extern TypeObjId _XfRaster_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfRaster_origin_x_pinfo, _arg);
    return _arg[0].u_long;
}
MarshalBuffer::ArgDesc _XfRaster_origin_y_pdesc[2] = { 1, 32 };
MarshalBuffer::ArgInfo _XfRaster_origin_y_pinfo = {
    &_XfRaster_tid, 5, _XfRaster_origin_y_pdesc, 0
};
Raster::Index RasterType::origin_y() {
    MarshalBuffer _b;
    extern TypeObjId _XfRaster_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfRaster_origin_y_pinfo, _arg);
    return _arg[0].u_long;
}
MarshalBuffer::ArgDesc _XfRaster_peek_pdesc[5] = { 4, 4, 33, 33, 2 };
MarshalBuffer::ArgMarshal _XfRaster_peek_pfunc[] = {
    &_XfRaster_Element_put, &_XfRaster_Element_get,

};
MarshalBuffer::ArgInfo _XfRaster_peek_pinfo = {
    &_XfRaster_tid, 6, _XfRaster_peek_pdesc, _XfRaster_peek_pfunc
};
void RasterType::peek(Raster::Index row, Raster::Index column, Raster::Element& e) {
    MarshalBuffer _b;
    extern TypeObjId _XfRaster_tid;
    MarshalBuffer::ArgValue _arg[4];
    _arg[1].u_long = row;
    _arg[2].u_long = column;
    _arg[3].u_addr = &e;
    _b.invoke(this, _XfRaster_peek_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfRaster_poke_pdesc[5] = { 4, 4, 33, 33, 1 };
MarshalBuffer::ArgMarshal _XfRaster_poke_pfunc[] = {
    &_XfRaster_Element_put, &_XfRaster_Element_get,

};
MarshalBuffer::ArgInfo _XfRaster_poke_pinfo = {
    &_XfRaster_tid, 7, _XfRaster_poke_pdesc, _XfRaster_poke_pfunc
};
void RasterType::poke(Raster::Index row, Raster::Index column, const Raster::Element& e) {
    MarshalBuffer _b;
    extern TypeObjId _XfRaster_tid;
    MarshalBuffer::ArgValue _arg[4];
    _arg[1].u_long = row;
    _arg[2].u_long = column;
    _arg[3].u_addr = &e;
    _b.invoke(this, _XfRaster_poke_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfRaster__get_scale_pdesc[2] = { 1, 48 };
MarshalBuffer::ArgInfo _XfRaster__get_scale_pinfo = {
    &_XfRaster_tid, 8, _XfRaster__get_scale_pdesc, 0
};
Coord RasterType::scale() {
    MarshalBuffer _b;
    extern TypeObjId _XfRaster_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfRaster__get_scale_pinfo, _arg);
    return _arg[0].u_float;
}
MarshalBuffer::ArgDesc _XfRaster__set_scale_pdesc[3] = { 2, 4, 49 };
MarshalBuffer::ArgInfo _XfRaster__set_scale_pinfo = {
    &_XfRaster_tid, 9, _XfRaster__set_scale_pdesc, 0
};
void RasterType::scale(Coord _p) {
    MarshalBuffer _b;
    extern TypeObjId _XfRaster_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_float = _p;
    _b.invoke(this, _XfRaster__set_scale_pinfo, _arg);
}
void _XfRaster_Element_put(MarshalBuffer& _b, const Raster::Element& _this) {
    _b.put_boolean(_this.on);
    _b.put_object(_this.pixel);
    _b.put_float(_this.blend);
}
void _XfRaster_Element_get(MarshalBuffer& _b, Raster::Element& _this) {
    _this.on = _b.get_boolean();
    _this.pixel = (ColorRef)_b.get_object(&ColorStub::_create);
    _this.blend = _b.get_float();
}
//+

//+ RasterImpl(Raster)
extern TypeObj_Descriptor _XfRaster_type;
TypeObj_Descriptor* _XfRasterImpl_parents[] = { &_XfRaster_type, nil };
extern TypeObjId _XfRasterImpl_tid;
TypeObj_Descriptor _XfRasterImpl_type = {
    /* type */ 0,
    /* id */ &_XfRasterImpl_tid,
    "RasterImpl",
    _XfRasterImpl_parents, /* offsets */ nil, /* excepts */ nil,
    /* methods */ nil, /* params */ nil,
    /* receive */ nil
};

RasterImpl* RasterImpl::_narrow(BaseObjectRef o) {
    return (RasterImpl*)_BaseObject_tnarrow(
        o, _XfRasterImpl_tid, 0
    );
}
TypeObjId RasterImpl::_tid() { return _XfRasterImpl_tid; }
//+
