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
 * Stubs for display interfaces
 */

#include <X11/Fresco/display.h>
#include <X11/Fresco/Impls/Xdisplay.h>
#include <X11/Fresco/Impls/Xscreen.h>
#include <X11/Fresco/Impls/Xwindow.h>
#include <X11/Fresco/Ox/request.h>
#include <X11/Fresco/Ox/stub.h>
#include <X11/Fresco/Ox/schema.h>

//+ Cursor::%init,type+dii,client
CursorType::CursorType() { }
CursorType::~CursorType() { }
void* CursorType::_this() { return this; }

extern TypeObj_Descriptor _Xfvoid_type;

TypeObj_OpData _XfCursor_methods[] = {
    { 0, 0, 0 }
};
TypeObj_ParamData _XfCursor_params[] = {
    { 0, 0, 0 }
};
extern TypeObj_Descriptor _XfFrescoObject_type;
TypeObj_Descriptor* _XfCursor_parents[] = { &_XfFrescoObject_type, nil };
extern TypeObjId _XfCursor_tid;
extern void _XfCursor_receive(BaseObjectRef, ULong, MarshalBuffer&);
TypeObj_Descriptor _XfCursor_type = {
    /* type */ 0,
    /* id */ &_XfCursor_tid,
    "Cursor",
    _XfCursor_parents, /* offsets */ nil, /* excepts */ nil,
    _XfCursor_methods, _XfCursor_params,
    &_XfCursor_receive
};

CursorRef Cursor::_narrow(BaseObjectRef o) {
    return (CursorRef)_BaseObject_tnarrow(
        o, _XfCursor_tid, &CursorStub::_create
    );
}
TypeObjId CursorType::_tid() { return _XfCursor_tid; }
void _XfCursor_receive(BaseObjectRef _object, ULong _m, MarshalBuffer& _b) {
    extern TypeObjId _XfCursor_tid;
    CursorRef _this = (CursorRef)_BaseObject_tcast(_object, _XfCursor_tid);
    switch (_m) {
        default:
            break;
    }
}
CursorStub::CursorStub(Exchange* e) { exch_ = e; }
CursorStub::~CursorStub() { }
BaseObjectRef CursorStub::_create(Exchange* e) {
    return (BaseObjectRef)(void*)new CursorStub(e);
}
Exchange* CursorStub::_exchange() {
    return exch_;
}
//+

//+ DisplayObj::%init,type+dii,client
DisplayObjType::DisplayObjType() { }
DisplayObjType::~DisplayObjType() { }
void* DisplayObjType::_this() { return this; }

extern TypeObj_Descriptor _XfDisplayStyle_type, _XfDrawingKit_type, 
    _XfScreenObj_type, _XfDisplayObj_ScreenNumber_type, _XfCursor_type, 
    _XfShort_type, _XfLong_array_16_type, _XfLong_array_16_type, _XfRaster_type, 
    _XfFont_type, _XfLong_type, _Xfvoid_type, _XfBoolean_type, _XfTag_type, 
    _XfGlyphTraversal_type, _XfWindow_type, _XfFloat_type;

TypeObj_OpData _XfDisplayObj_methods[] = {
    { "style", &_XfDisplayStyle_type, 0 },
    { "drawing_kit", &_XfDrawingKit_type, 0 },
    { "screen", &_XfScreenObj_type, 1 },
    { "number_of_screens", &_XfDisplayObj_ScreenNumber_type, 0 },
    { "default_screen", &_XfScreenObj_type, 0 },
    { "cursor_from_data", &_XfCursor_type, 4 },
    { "cursor_from_bitmap", &_XfCursor_type, 2 },
    { "cursor_from_font", &_XfCursor_type, 3 },
    { "cursor_from_index", &_XfCursor_type, 1 },
    { "run", &_Xfvoid_type, 1 },
    { "running", &_XfBoolean_type, 0 },
    { "add_filter", &_XfTag_type, 1 },
    { "remove_filter", &_Xfvoid_type, 1 },
    { "need_repair", &_Xfvoid_type, 1 },
    { "repair", &_Xfvoid_type, 0 },
    { "flush", &_Xfvoid_type, 0 },
    { "flush_and_wait", &_Xfvoid_type, 0 },
    { "ring_bell", &_Xfvoid_type, 1 },
    { "close", &_Xfvoid_type, 0 },
    { 0, 0, 0 }
};
TypeObj_ParamData _XfDisplayObj_params[] = {
    /* screen */
        { "n", 0, &_XfDisplayObj_ScreenNumber_type },
    /* cursor_from_data */
        { "x", 0, &_XfShort_type },
        { "y", 0, &_XfShort_type },
        { "pattern", 0, &_XfLong_array_16_type },
        { "mask", 0, &_XfLong_array_16_type },
    /* cursor_from_bitmap */
        { "b", 0, &_XfRaster_type },
        { "mask", 0, &_XfRaster_type },
    /* cursor_from_font */
        { "f", 0, &_XfFont_type },
        { "pattern", 0, &_XfLong_type },
        { "mask", 0, &_XfLong_type },
    /* cursor_from_index */
        { "index", 0, &_XfLong_type },
    /* run */
        { "b", 0, &_XfBoolean_type },
    /* add_filter */
        { "t", 0, &_XfGlyphTraversal_type },
    /* remove_filter */
        { "add_tag", 0, &_XfTag_type },
    /* need_repair */
        { "w", 0, &_XfWindow_type },
    /* ring_bell */
        { "pct_loudness", 0, &_XfFloat_type }
};
extern TypeObj_Descriptor _XfFrescoObject_type;
TypeObj_Descriptor* _XfDisplayObj_parents[] = { &_XfFrescoObject_type, nil };
extern TypeObjId _XfDisplayObj_tid;
extern void _XfDisplayObj_receive(BaseObjectRef, ULong, MarshalBuffer&);
TypeObj_Descriptor _XfDisplayObj_type = {
    /* type */ 0,
    /* id */ &_XfDisplayObj_tid,
    "DisplayObj",
    _XfDisplayObj_parents, /* offsets */ nil, /* excepts */ nil,
    _XfDisplayObj_methods, _XfDisplayObj_params,
    &_XfDisplayObj_receive
};

DisplayObjRef DisplayObj::_narrow(BaseObjectRef o) {
    return (DisplayObjRef)_BaseObject_tnarrow(
        o, _XfDisplayObj_tid, &DisplayObjStub::_create
    );
}
TypeObjId DisplayObjType::_tid() { return _XfDisplayObj_tid; }
void _XfDisplayObj_receive(BaseObjectRef _object, ULong _m, MarshalBuffer& _b) {
    extern TypeObjId _XfDisplayObj_tid;
    DisplayObjRef _this = (DisplayObjRef)_BaseObject_tcast(_object, _XfDisplayObj_tid);
    switch (_m) {
        case /* style */ 0: {
            extern MarshalBuffer::ArgInfo _XfDisplayObj_style_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_objref = _this->_c_style();
            _b.reply(_XfDisplayObj_style_pinfo, _arg);
            break;
        }
        case /* drawing_kit */ 1: {
            extern MarshalBuffer::ArgInfo _XfDisplayObj_drawing_kit_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_objref = _this->_c_drawing_kit();
            _b.reply(_XfDisplayObj_drawing_kit_pinfo, _arg);
            break;
        }
        case /* screen */ 2: {
            extern MarshalBuffer::ArgInfo _XfDisplayObj_screen_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            DisplayObj::ScreenNumber n;
            _arg[1].u_addr = &n;
            _b.receive(_XfDisplayObj_screen_pinfo, _arg);
            _arg[0].u_objref = _this->_c_screen(n);
            _b.reply(_XfDisplayObj_screen_pinfo, _arg);
            break;
        }
        case /* number_of_screens */ 3: {
            extern MarshalBuffer::ArgInfo _XfDisplayObj_number_of_screens_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_long = _this->number_of_screens();
            _b.reply(_XfDisplayObj_number_of_screens_pinfo, _arg);
            break;
        }
        case /* default_screen */ 4: {
            extern MarshalBuffer::ArgInfo _XfDisplayObj_default_screen_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_objref = _this->_c_default_screen();
            _b.reply(_XfDisplayObj_default_screen_pinfo, _arg);
            break;
        }
        case /* cursor_from_data */ 5: {
            extern MarshalBuffer::ArgInfo _XfDisplayObj_cursor_from_data_pinfo;
            MarshalBuffer::ArgValue _arg[5];
            Short x;
            _arg[1].u_addr = &x;
            Short y;
            _arg[2].u_addr = &y;
            Long pattern[16];
            _arg[3].u_addr = pattern;
            Long mask[16];
            _arg[4].u_addr = mask;
            _b.receive(_XfDisplayObj_cursor_from_data_pinfo, _arg);
            _arg[0].u_objref = _this->_c_cursor_from_data(x, y, pattern, mask);
            _b.reply(_XfDisplayObj_cursor_from_data_pinfo, _arg);
            break;
        }
        case /* cursor_from_bitmap */ 6: {
            extern MarshalBuffer::ArgInfo _XfDisplayObj_cursor_from_bitmap_pinfo;
            MarshalBuffer::ArgValue _arg[3];
            RasterRef b;
            _arg[1].u_addr = &b;
            RasterRef mask;
            _arg[2].u_addr = &mask;
            _b.receive(_XfDisplayObj_cursor_from_bitmap_pinfo, _arg);
            _arg[0].u_objref = _this->_c_cursor_from_bitmap(b, mask);
            _b.reply(_XfDisplayObj_cursor_from_bitmap_pinfo, _arg);
            break;
        }
        case /* cursor_from_font */ 7: {
            extern MarshalBuffer::ArgInfo _XfDisplayObj_cursor_from_font_pinfo;
            MarshalBuffer::ArgValue _arg[4];
            FontRef f;
            _arg[1].u_addr = &f;
            Long pattern;
            _arg[2].u_addr = &pattern;
            Long mask;
            _arg[3].u_addr = &mask;
            _b.receive(_XfDisplayObj_cursor_from_font_pinfo, _arg);
            _arg[0].u_objref = _this->_c_cursor_from_font(f, pattern, mask);
            _b.reply(_XfDisplayObj_cursor_from_font_pinfo, _arg);
            break;
        }
        case /* cursor_from_index */ 8: {
            extern MarshalBuffer::ArgInfo _XfDisplayObj_cursor_from_index_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            Long index;
            _arg[1].u_addr = &index;
            _b.receive(_XfDisplayObj_cursor_from_index_pinfo, _arg);
            _arg[0].u_objref = _this->_c_cursor_from_index(index);
            _b.reply(_XfDisplayObj_cursor_from_index_pinfo, _arg);
            break;
        }
        case /* run */ 9: {
            extern MarshalBuffer::ArgInfo _XfDisplayObj_run_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            Boolean b;
            _arg[1].u_addr = &b;
            _b.receive(_XfDisplayObj_run_pinfo, _arg);
            _this->run(b);
            _b.reply(_XfDisplayObj_run_pinfo, _arg);
            break;
        }
        case /* running */ 10: {
            extern MarshalBuffer::ArgInfo _XfDisplayObj_running_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_boolean = _this->running();
            _b.reply(_XfDisplayObj_running_pinfo, _arg);
            break;
        }
        case /* add_filter */ 11: {
            extern MarshalBuffer::ArgInfo _XfDisplayObj_add_filter_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            GlyphTraversalRef t;
            _arg[1].u_addr = &t;
            _b.receive(_XfDisplayObj_add_filter_pinfo, _arg);
            _arg[0].u_unsigned_long = _this->add_filter(t);
            _b.reply(_XfDisplayObj_add_filter_pinfo, _arg);
            break;
        }
        case /* remove_filter */ 12: {
            extern MarshalBuffer::ArgInfo _XfDisplayObj_remove_filter_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            Tag add_tag;
            _arg[1].u_addr = &add_tag;
            _b.receive(_XfDisplayObj_remove_filter_pinfo, _arg);
            _this->remove_filter(add_tag);
            _b.reply(_XfDisplayObj_remove_filter_pinfo, _arg);
            break;
        }
        case /* need_repair */ 13: {
            extern MarshalBuffer::ArgInfo _XfDisplayObj_need_repair_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            WindowRef w;
            _arg[1].u_addr = &w;
            _b.receive(_XfDisplayObj_need_repair_pinfo, _arg);
            _this->need_repair(w);
            _b.reply(_XfDisplayObj_need_repair_pinfo, _arg);
            break;
        }
        case /* repair */ 14: {
            extern MarshalBuffer::ArgInfo _XfDisplayObj_repair_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _this->repair();
            _b.reply(_XfDisplayObj_repair_pinfo, _arg);
            break;
        }
        case /* flush */ 15: {
            extern MarshalBuffer::ArgInfo _XfDisplayObj_flush_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _this->flush();
            _b.reply(_XfDisplayObj_flush_pinfo, _arg);
            break;
        }
        case /* flush_and_wait */ 16: {
            extern MarshalBuffer::ArgInfo _XfDisplayObj_flush_and_wait_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _this->flush_and_wait();
            _b.reply(_XfDisplayObj_flush_and_wait_pinfo, _arg);
            break;
        }
        case /* ring_bell */ 17: {
            extern MarshalBuffer::ArgInfo _XfDisplayObj_ring_bell_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            Float pct_loudness;
            _arg[1].u_addr = &pct_loudness;
            _b.receive(_XfDisplayObj_ring_bell_pinfo, _arg);
            _this->ring_bell(pct_loudness);
            _b.reply(_XfDisplayObj_ring_bell_pinfo, _arg);
            break;
        }
        case /* close */ 18: {
            extern MarshalBuffer::ArgInfo _XfDisplayObj_close_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _this->close();
            _b.reply(_XfDisplayObj_close_pinfo, _arg);
            break;
        }
    }
}
extern void _XfDisplayObj_cursor_from_data_pattern_put(
    MarshalBuffer&, const Long pattern[16]
);
extern void _XfDisplayObj_cursor_from_data_pattern_get(
    MarshalBuffer&, Long pattern[16]
);
extern void _XfDisplayObj_cursor_from_data_mask_put(
    MarshalBuffer&, const Long mask[16]
);
extern void _XfDisplayObj_cursor_from_data_mask_get(
    MarshalBuffer&, Long mask[16]
);

DisplayObjStub::DisplayObjStub(Exchange* e) { exch_ = e; }
DisplayObjStub::~DisplayObjStub() { }
BaseObjectRef DisplayObjStub::_create(Exchange* e) {
    return (BaseObjectRef)(void*)new DisplayObjStub(e);
}
Exchange* DisplayObjStub::_exchange() {
    return exch_;
}
MarshalBuffer::ArgDesc _XfDisplayObj_style_pdesc[2] = { 1, 60 };
MarshalBuffer::ArgMarshal _XfDisplayObj_style_pfunc[] = {
    &DisplayStyleStub::_create
};
MarshalBuffer::ArgInfo _XfDisplayObj_style_pinfo = {
    &_XfDisplayObj_tid, 0, _XfDisplayObj_style_pdesc, _XfDisplayObj_style_pfunc
};
DisplayStyle_tmp DisplayObjType::style() {
    return _c_style();
}
DisplayStyleRef DisplayObjType::_c_style() {
    MarshalBuffer _b;
    extern TypeObjId _XfDisplayObj_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfDisplayObj_style_pinfo, _arg);
    return (DisplayStyleRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfDisplayObj_drawing_kit_pdesc[2] = { 1, 60 };
MarshalBuffer::ArgMarshal _XfDisplayObj_drawing_kit_pfunc[] = {
    &DrawingKitStub::_create
};
MarshalBuffer::ArgInfo _XfDisplayObj_drawing_kit_pinfo = {
    &_XfDisplayObj_tid, 1, _XfDisplayObj_drawing_kit_pdesc, _XfDisplayObj_drawing_kit_pfunc
};
DrawingKitRef DisplayObjType::_c_drawing_kit() {
    MarshalBuffer _b;
    extern TypeObjId _XfDisplayObj_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfDisplayObj_drawing_kit_pinfo, _arg);
    return (DrawingKitRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfDisplayObj_screen_pdesc[3] = { 2, 60, 33 };
MarshalBuffer::ArgMarshal _XfDisplayObj_screen_pfunc[] = {
    &ScreenObjStub::_create
};
MarshalBuffer::ArgInfo _XfDisplayObj_screen_pinfo = {
    &_XfDisplayObj_tid, 2, _XfDisplayObj_screen_pdesc, _XfDisplayObj_screen_pfunc
};
ScreenObj_tmp DisplayObjType::screen(DisplayObj::ScreenNumber n) {
    return _c_screen(n);
}
ScreenObjRef DisplayObjType::_c_screen(DisplayObj::ScreenNumber n) {
    MarshalBuffer _b;
    extern TypeObjId _XfDisplayObj_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_long = n;
    _b.invoke(this, _XfDisplayObj_screen_pinfo, _arg);
    return (ScreenObjRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfDisplayObj_number_of_screens_pdesc[2] = { 1, 32 };
MarshalBuffer::ArgInfo _XfDisplayObj_number_of_screens_pinfo = {
    &_XfDisplayObj_tid, 3, _XfDisplayObj_number_of_screens_pdesc, 0
};
DisplayObj::ScreenNumber DisplayObjType::number_of_screens() {
    MarshalBuffer _b;
    extern TypeObjId _XfDisplayObj_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfDisplayObj_number_of_screens_pinfo, _arg);
    return _arg[0].u_long;
}
MarshalBuffer::ArgDesc _XfDisplayObj_default_screen_pdesc[2] = { 1, 60 };
MarshalBuffer::ArgMarshal _XfDisplayObj_default_screen_pfunc[] = {
    &ScreenObjStub::_create
};
MarshalBuffer::ArgInfo _XfDisplayObj_default_screen_pinfo = {
    &_XfDisplayObj_tid, 4, _XfDisplayObj_default_screen_pdesc, _XfDisplayObj_default_screen_pfunc
};
ScreenObj_tmp DisplayObjType::default_screen() {
    return _c_default_screen();
}
ScreenObjRef DisplayObjType::_c_default_screen() {
    MarshalBuffer _b;
    extern TypeObjId _XfDisplayObj_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfDisplayObj_default_screen_pinfo, _arg);
    return (ScreenObjRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfDisplayObj_cursor_from_data_pdesc[6] = { 5, 60, 25, 25, 33, 33 };
MarshalBuffer::ArgMarshal _XfDisplayObj_cursor_from_data_pfunc[] = {
    &_XfDisplayObj_cursor_from_data_pattern_put, &_XfDisplayObj_cursor_from_data_pattern_get,
    &_XfDisplayObj_cursor_from_data_mask_put, &_XfDisplayObj_cursor_from_data_mask_get,
    &CursorStub::_create
};
MarshalBuffer::ArgInfo _XfDisplayObj_cursor_from_data_pinfo = {
    &_XfDisplayObj_tid, 5, _XfDisplayObj_cursor_from_data_pdesc, _XfDisplayObj_cursor_from_data_pfunc
};
CursorRef DisplayObjType::_c_cursor_from_data(Short x, Short y, Long pattern[16], Long mask[16]) {
    MarshalBuffer _b;
    extern TypeObjId _XfDisplayObj_tid;
    MarshalBuffer::ArgValue _arg[5];
    _arg[1].u_short = x;
    _arg[2].u_short = y;
    _arg[3].u_addr = pattern;
    _arg[4].u_addr = mask;
    _b.invoke(this, _XfDisplayObj_cursor_from_data_pinfo, _arg);
    return (CursorRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfDisplayObj_cursor_from_bitmap_pdesc[4] = { 3, 60, 61, 61 };
MarshalBuffer::ArgMarshal _XfDisplayObj_cursor_from_bitmap_pfunc[] = {
    &RasterStub::_create,
    &RasterStub::_create,
    &CursorStub::_create
};
MarshalBuffer::ArgInfo _XfDisplayObj_cursor_from_bitmap_pinfo = {
    &_XfDisplayObj_tid, 6, _XfDisplayObj_cursor_from_bitmap_pdesc, _XfDisplayObj_cursor_from_bitmap_pfunc
};
CursorRef DisplayObjType::_c_cursor_from_bitmap(Raster_in b, Raster_in mask) {
    MarshalBuffer _b;
    extern TypeObjId _XfDisplayObj_tid;
    MarshalBuffer::ArgValue _arg[3];
    _arg[1].u_objref = b;
    _arg[2].u_objref = mask;
    _b.invoke(this, _XfDisplayObj_cursor_from_bitmap_pinfo, _arg);
    return (CursorRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfDisplayObj_cursor_from_font_pdesc[5] = { 4, 60, 61, 33, 33 };
MarshalBuffer::ArgMarshal _XfDisplayObj_cursor_from_font_pfunc[] = {
    &FontStub::_create,
    &CursorStub::_create
};
MarshalBuffer::ArgInfo _XfDisplayObj_cursor_from_font_pinfo = {
    &_XfDisplayObj_tid, 7, _XfDisplayObj_cursor_from_font_pdesc, _XfDisplayObj_cursor_from_font_pfunc
};
CursorRef DisplayObjType::_c_cursor_from_font(Font_in f, Long pattern, Long mask) {
    MarshalBuffer _b;
    extern TypeObjId _XfDisplayObj_tid;
    MarshalBuffer::ArgValue _arg[4];
    _arg[1].u_objref = f;
    _arg[2].u_long = pattern;
    _arg[3].u_long = mask;
    _b.invoke(this, _XfDisplayObj_cursor_from_font_pinfo, _arg);
    return (CursorRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfDisplayObj_cursor_from_index_pdesc[3] = { 2, 60, 33 };
MarshalBuffer::ArgMarshal _XfDisplayObj_cursor_from_index_pfunc[] = {
    &CursorStub::_create
};
MarshalBuffer::ArgInfo _XfDisplayObj_cursor_from_index_pinfo = {
    &_XfDisplayObj_tid, 8, _XfDisplayObj_cursor_from_index_pdesc, _XfDisplayObj_cursor_from_index_pfunc
};
CursorRef DisplayObjType::_c_cursor_from_index(Long index) {
    MarshalBuffer _b;
    extern TypeObjId _XfDisplayObj_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_long = index;
    _b.invoke(this, _XfDisplayObj_cursor_from_index_pinfo, _arg);
    return (CursorRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfDisplayObj_run_pdesc[3] = { 2, 4, 13 };
MarshalBuffer::ArgInfo _XfDisplayObj_run_pinfo = {
    &_XfDisplayObj_tid, 9, _XfDisplayObj_run_pdesc, 0
};
void DisplayObjType::run(Boolean b) {
    MarshalBuffer _b;
    extern TypeObjId _XfDisplayObj_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_boolean = b;
    _b.invoke(this, _XfDisplayObj_run_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfDisplayObj_running_pdesc[2] = { 1, 12 };
MarshalBuffer::ArgInfo _XfDisplayObj_running_pinfo = {
    &_XfDisplayObj_tid, 10, _XfDisplayObj_running_pdesc, 0
};
Boolean DisplayObjType::running() {
    MarshalBuffer _b;
    extern TypeObjId _XfDisplayObj_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfDisplayObj_running_pinfo, _arg);
    return _arg[0].u_boolean;
}
MarshalBuffer::ArgDesc _XfDisplayObj_add_filter_pdesc[3] = { 2, 36, 61 };
MarshalBuffer::ArgMarshal _XfDisplayObj_add_filter_pfunc[] = {
    &GlyphTraversalStub::_create,

};
MarshalBuffer::ArgInfo _XfDisplayObj_add_filter_pinfo = {
    &_XfDisplayObj_tid, 11, _XfDisplayObj_add_filter_pdesc, _XfDisplayObj_add_filter_pfunc
};
Tag DisplayObjType::add_filter(GlyphTraversal_in t) {
    MarshalBuffer _b;
    extern TypeObjId _XfDisplayObj_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = t;
    _b.invoke(this, _XfDisplayObj_add_filter_pinfo, _arg);
    return _arg[0].u_unsigned_long;
}
MarshalBuffer::ArgDesc _XfDisplayObj_remove_filter_pdesc[3] = { 2, 4, 37 };
MarshalBuffer::ArgInfo _XfDisplayObj_remove_filter_pinfo = {
    &_XfDisplayObj_tid, 12, _XfDisplayObj_remove_filter_pdesc, 0
};
void DisplayObjType::remove_filter(Tag add_tag) {
    MarshalBuffer _b;
    extern TypeObjId _XfDisplayObj_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_unsigned_long = add_tag;
    _b.invoke(this, _XfDisplayObj_remove_filter_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfDisplayObj_need_repair_pdesc[3] = { 2, 4, 61 };
MarshalBuffer::ArgMarshal _XfDisplayObj_need_repair_pfunc[] = {
    &WindowStub::_create,

};
MarshalBuffer::ArgInfo _XfDisplayObj_need_repair_pinfo = {
    &_XfDisplayObj_tid, 13, _XfDisplayObj_need_repair_pdesc, _XfDisplayObj_need_repair_pfunc
};
void DisplayObjType::need_repair(Window_in w) {
    MarshalBuffer _b;
    extern TypeObjId _XfDisplayObj_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = w;
    _b.invoke(this, _XfDisplayObj_need_repair_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfDisplayObj_repair_pdesc[2] = { 1, 4 };
MarshalBuffer::ArgInfo _XfDisplayObj_repair_pinfo = {
    &_XfDisplayObj_tid, 14, _XfDisplayObj_repair_pdesc, 0
};
void DisplayObjType::repair() {
    MarshalBuffer _b;
    extern TypeObjId _XfDisplayObj_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfDisplayObj_repair_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfDisplayObj_flush_pdesc[2] = { 1, 4 };
MarshalBuffer::ArgInfo _XfDisplayObj_flush_pinfo = {
    &_XfDisplayObj_tid, 15, _XfDisplayObj_flush_pdesc, 0
};
void DisplayObjType::flush() {
    MarshalBuffer _b;
    extern TypeObjId _XfDisplayObj_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfDisplayObj_flush_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfDisplayObj_flush_and_wait_pdesc[2] = { 1, 4 };
MarshalBuffer::ArgInfo _XfDisplayObj_flush_and_wait_pinfo = {
    &_XfDisplayObj_tid, 16, _XfDisplayObj_flush_and_wait_pdesc, 0
};
void DisplayObjType::flush_and_wait() {
    MarshalBuffer _b;
    extern TypeObjId _XfDisplayObj_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfDisplayObj_flush_and_wait_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfDisplayObj_ring_bell_pdesc[3] = { 2, 4, 49 };
MarshalBuffer::ArgInfo _XfDisplayObj_ring_bell_pinfo = {
    &_XfDisplayObj_tid, 17, _XfDisplayObj_ring_bell_pdesc, 0
};
void DisplayObjType::ring_bell(Float pct_loudness) {
    MarshalBuffer _b;
    extern TypeObjId _XfDisplayObj_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_float = pct_loudness;
    _b.invoke(this, _XfDisplayObj_ring_bell_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfDisplayObj_close_pdesc[2] = { 1, 4 };
MarshalBuffer::ArgInfo _XfDisplayObj_close_pinfo = {
    &_XfDisplayObj_tid, 18, _XfDisplayObj_close_pdesc, 0
};
void DisplayObjType::close() {
    MarshalBuffer _b;
    extern TypeObjId _XfDisplayObj_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfDisplayObj_close_pinfo, _arg);
}
void _XfDisplayObj_cursor_from_data_pattern_put(MarshalBuffer& _b, const Long _array[16]
) {
    for (int _i0 = 0; _i0 < 16; _i0++) {
        const Long& _tmp = _array[_i0];
        _b.put_long(_tmp);
    }
}
void _XfDisplayObj_cursor_from_data_pattern_get(MarshalBuffer& _b, Long _array[16]
) {
    for (int _i0 = 0; _i0 < 16; _i0++) {
        Long& _tmp = _array[_i0];
        _tmp = _b.get_long();
    }
}
void _XfDisplayObj_cursor_from_data_mask_put(MarshalBuffer& _b, const Long _array[16]
) {
    for (int _i0 = 0; _i0 < 16; _i0++) {
        const Long& _tmp = _array[_i0];
        _b.put_long(_tmp);
    }
}
void _XfDisplayObj_cursor_from_data_mask_get(MarshalBuffer& _b, Long _array[16]
) {
    for (int _i0 = 0; _i0 < 16; _i0++) {
        Long& _tmp = _array[_i0];
        _tmp = _b.get_long();
    }
}
//+

//+ DisplayImpl(DisplayObj)
extern TypeObj_Descriptor _XfDisplayObj_type;
TypeObj_Descriptor* _XfDisplayImpl_parents[] = { &_XfDisplayObj_type, nil };
extern TypeObjId _XfDisplayImpl_tid;
TypeObj_Descriptor _XfDisplayImpl_type = {
    /* type */ 0,
    /* id */ &_XfDisplayImpl_tid,
    "DisplayImpl",
    _XfDisplayImpl_parents, /* offsets */ nil, /* excepts */ nil,
    /* methods */ nil, /* params */ nil,
    /* receive */ nil
};

DisplayImpl* DisplayImpl::_narrow(BaseObjectRef o) {
    return (DisplayImpl*)_BaseObject_tnarrow(
        o, _XfDisplayImpl_tid, 0
    );
}
TypeObjId DisplayImpl::_tid() { return _XfDisplayImpl_tid; }
//+

//+ DisplayStyle::%init,type+dii,client
DisplayStyleType::DisplayStyleType() { }
DisplayStyleType::~DisplayStyleType() { }
void* DisplayStyleType::_this() { return this; }

extern TypeObj_Descriptor _Xfvoid_type;

TypeObj_OpData _XfDisplayStyle_methods[] = {
    { "_get_auto_repeat", &_XfBoolean_type, 0 },
    { "_set_auto_repeat", &_Xfvoid_type, 1 },
    { "_get_key_click_volume", &_XfFloat_type, 0 },
    { "_set_key_click_volume", &_Xfvoid_type, 1 },
    { "_get_pointer_acceleration", &_XfFloat_type, 0 },
    { "_set_pointer_acceleration", &_Xfvoid_type, 1 },
    { "_get_pointer_threshold", &_XfLong_type, 0 },
    { "_set_pointer_threshold", &_Xfvoid_type, 1 },
    { 0, 0, 0 }
};
TypeObj_ParamData _XfDisplayStyle_params[] = {
    /* auto_repeat */
        { "_p", 0, &_XfBoolean_type },
    /* key_click_volume */
        { "_p", 0, &_XfFloat_type },
    /* pointer_acceleration */
        { "_p", 0, &_XfFloat_type },
    /* pointer_threshold */
        { "_p", 0, &_XfLong_type }
};
extern TypeObj_Descriptor _XfStyleObj_type;
TypeObj_Descriptor* _XfDisplayStyle_parents[] = { &_XfStyleObj_type, nil };
extern TypeObjId _XfDisplayStyle_tid;
extern void _XfDisplayStyle_receive(BaseObjectRef, ULong, MarshalBuffer&);
TypeObj_Descriptor _XfDisplayStyle_type = {
    /* type */ 0,
    /* id */ &_XfDisplayStyle_tid,
    "DisplayStyle",
    _XfDisplayStyle_parents, /* offsets */ nil, /* excepts */ nil,
    _XfDisplayStyle_methods, _XfDisplayStyle_params,
    &_XfDisplayStyle_receive
};

DisplayStyleRef DisplayStyle::_narrow(BaseObjectRef o) {
    return (DisplayStyleRef)_BaseObject_tnarrow(
        o, _XfDisplayStyle_tid, &DisplayStyleStub::_create
    );
}
TypeObjId DisplayStyleType::_tid() { return _XfDisplayStyle_tid; }
void _XfDisplayStyle_receive(BaseObjectRef _object, ULong _m, MarshalBuffer& _b) {
    extern TypeObjId _XfDisplayStyle_tid;
    DisplayStyleRef _this = (DisplayStyleRef)_BaseObject_tcast(_object, _XfDisplayStyle_tid);
    switch (_m) {
        case /* _get_auto_repeat */ 0: {
            extern MarshalBuffer::ArgInfo _XfDisplayStyle__get_auto_repeat_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_boolean = _this->auto_repeat();
            _b.reply(_XfDisplayStyle__get_auto_repeat_pinfo, _arg);
            break;
        }
        case /* _set_auto_repeat */ 1: {
            extern MarshalBuffer::ArgInfo _XfDisplayStyle__set_auto_repeat_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            Boolean _p;
            _arg[1].u_addr = &_p;
            _b.receive(_XfDisplayStyle__set_auto_repeat_pinfo, _arg);
            _this->auto_repeat(_p);
            _b.reply(_XfDisplayStyle__set_auto_repeat_pinfo, _arg);
            break;
        }
        case /* _get_key_click_volume */ 2: {
            extern MarshalBuffer::ArgInfo _XfDisplayStyle__get_key_click_volume_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_float = _this->key_click_volume();
            _b.reply(_XfDisplayStyle__get_key_click_volume_pinfo, _arg);
            break;
        }
        case /* _set_key_click_volume */ 3: {
            extern MarshalBuffer::ArgInfo _XfDisplayStyle__set_key_click_volume_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            Float _p;
            _arg[1].u_addr = &_p;
            _b.receive(_XfDisplayStyle__set_key_click_volume_pinfo, _arg);
            _this->key_click_volume(_p);
            _b.reply(_XfDisplayStyle__set_key_click_volume_pinfo, _arg);
            break;
        }
        case /* _get_pointer_acceleration */ 4: {
            extern MarshalBuffer::ArgInfo _XfDisplayStyle__get_pointer_acceleration_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_float = _this->pointer_acceleration();
            _b.reply(_XfDisplayStyle__get_pointer_acceleration_pinfo, _arg);
            break;
        }
        case /* _set_pointer_acceleration */ 5: {
            extern MarshalBuffer::ArgInfo _XfDisplayStyle__set_pointer_acceleration_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            Float _p;
            _arg[1].u_addr = &_p;
            _b.receive(_XfDisplayStyle__set_pointer_acceleration_pinfo, _arg);
            _this->pointer_acceleration(_p);
            _b.reply(_XfDisplayStyle__set_pointer_acceleration_pinfo, _arg);
            break;
        }
        case /* _get_pointer_threshold */ 6: {
            extern MarshalBuffer::ArgInfo _XfDisplayStyle__get_pointer_threshold_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_long = _this->pointer_threshold();
            _b.reply(_XfDisplayStyle__get_pointer_threshold_pinfo, _arg);
            break;
        }
        case /* _set_pointer_threshold */ 7: {
            extern MarshalBuffer::ArgInfo _XfDisplayStyle__set_pointer_threshold_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            Long _p;
            _arg[1].u_addr = &_p;
            _b.receive(_XfDisplayStyle__set_pointer_threshold_pinfo, _arg);
            _this->pointer_threshold(_p);
            _b.reply(_XfDisplayStyle__set_pointer_threshold_pinfo, _arg);
            break;
        }
    }
}
DisplayStyleStub::DisplayStyleStub(Exchange* e) { exch_ = e; }
DisplayStyleStub::~DisplayStyleStub() { }
BaseObjectRef DisplayStyleStub::_create(Exchange* e) {
    return (BaseObjectRef)(void*)new DisplayStyleStub(e);
}
Exchange* DisplayStyleStub::_exchange() {
    return exch_;
}
MarshalBuffer::ArgDesc _XfDisplayStyle__get_auto_repeat_pdesc[2] = { 1, 12 };
MarshalBuffer::ArgInfo _XfDisplayStyle__get_auto_repeat_pinfo = {
    &_XfDisplayStyle_tid, 0, _XfDisplayStyle__get_auto_repeat_pdesc, 0
};
Boolean DisplayStyleType::auto_repeat() {
    MarshalBuffer _b;
    extern TypeObjId _XfDisplayStyle_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfDisplayStyle__get_auto_repeat_pinfo, _arg);
    return _arg[0].u_boolean;
}
MarshalBuffer::ArgDesc _XfDisplayStyle__set_auto_repeat_pdesc[3] = { 2, 4, 13 };
MarshalBuffer::ArgInfo _XfDisplayStyle__set_auto_repeat_pinfo = {
    &_XfDisplayStyle_tid, 1, _XfDisplayStyle__set_auto_repeat_pdesc, 0
};
void DisplayStyleType::auto_repeat(Boolean _p) {
    MarshalBuffer _b;
    extern TypeObjId _XfDisplayStyle_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_boolean = _p;
    _b.invoke(this, _XfDisplayStyle__set_auto_repeat_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfDisplayStyle__get_key_click_volume_pdesc[2] = { 1, 48 };
MarshalBuffer::ArgInfo _XfDisplayStyle__get_key_click_volume_pinfo = {
    &_XfDisplayStyle_tid, 2, _XfDisplayStyle__get_key_click_volume_pdesc, 0
};
Float DisplayStyleType::key_click_volume() {
    MarshalBuffer _b;
    extern TypeObjId _XfDisplayStyle_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfDisplayStyle__get_key_click_volume_pinfo, _arg);
    return _arg[0].u_float;
}
MarshalBuffer::ArgDesc _XfDisplayStyle__set_key_click_volume_pdesc[3] = { 2, 4, 49 };
MarshalBuffer::ArgInfo _XfDisplayStyle__set_key_click_volume_pinfo = {
    &_XfDisplayStyle_tid, 3, _XfDisplayStyle__set_key_click_volume_pdesc, 0
};
void DisplayStyleType::key_click_volume(Float _p) {
    MarshalBuffer _b;
    extern TypeObjId _XfDisplayStyle_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_float = _p;
    _b.invoke(this, _XfDisplayStyle__set_key_click_volume_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfDisplayStyle__get_pointer_acceleration_pdesc[2] = { 1, 48 };
MarshalBuffer::ArgInfo _XfDisplayStyle__get_pointer_acceleration_pinfo = {
    &_XfDisplayStyle_tid, 4, _XfDisplayStyle__get_pointer_acceleration_pdesc, 0
};
Float DisplayStyleType::pointer_acceleration() {
    MarshalBuffer _b;
    extern TypeObjId _XfDisplayStyle_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfDisplayStyle__get_pointer_acceleration_pinfo, _arg);
    return _arg[0].u_float;
}
MarshalBuffer::ArgDesc _XfDisplayStyle__set_pointer_acceleration_pdesc[3] = { 2, 4, 49 };
MarshalBuffer::ArgInfo _XfDisplayStyle__set_pointer_acceleration_pinfo = {
    &_XfDisplayStyle_tid, 5, _XfDisplayStyle__set_pointer_acceleration_pdesc, 0
};
void DisplayStyleType::pointer_acceleration(Float _p) {
    MarshalBuffer _b;
    extern TypeObjId _XfDisplayStyle_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_float = _p;
    _b.invoke(this, _XfDisplayStyle__set_pointer_acceleration_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfDisplayStyle__get_pointer_threshold_pdesc[2] = { 1, 32 };
MarshalBuffer::ArgInfo _XfDisplayStyle__get_pointer_threshold_pinfo = {
    &_XfDisplayStyle_tid, 6, _XfDisplayStyle__get_pointer_threshold_pdesc, 0
};
Long DisplayStyleType::pointer_threshold() {
    MarshalBuffer _b;
    extern TypeObjId _XfDisplayStyle_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfDisplayStyle__get_pointer_threshold_pinfo, _arg);
    return _arg[0].u_long;
}
MarshalBuffer::ArgDesc _XfDisplayStyle__set_pointer_threshold_pdesc[3] = { 2, 4, 33 };
MarshalBuffer::ArgInfo _XfDisplayStyle__set_pointer_threshold_pinfo = {
    &_XfDisplayStyle_tid, 7, _XfDisplayStyle__set_pointer_threshold_pdesc, 0
};
void DisplayStyleType::pointer_threshold(Long _p) {
    MarshalBuffer _b;
    extern TypeObjId _XfDisplayStyle_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_long = _p;
    _b.invoke(this, _XfDisplayStyle__set_pointer_threshold_pinfo, _arg);
}
//+

//+ ScreenObj::%init,type+dii,client
ScreenObjType::ScreenObjType() { }
ScreenObjType::~ScreenObjType() { }
void* ScreenObjType::_this() { return this; }

extern TypeObj_Descriptor _XfCoord_type, _XfDisplayObj_type, _XfPixelCoord_type, 
    _XfViewer_type;

TypeObj_OpData _XfScreenObj_methods[] = {
    { "_get_dpi", &_XfCoord_type, 0 },
    { "_set_dpi", &_Xfvoid_type, 1 },
    { "display", &_XfDisplayObj_type, 0 },
    { "width", &_XfCoord_type, 0 },
    { "height", &_XfCoord_type, 0 },
    { "to_pixels", &_XfPixelCoord_type, 1 },
    { "to_coord", &_XfCoord_type, 1 },
    { "to_pixels_coord", &_XfCoord_type, 1 },
    { "move_pointer", &_Xfvoid_type, 2 },
    { "application", &_XfWindow_type, 1 },
    { "top_level", &_XfWindow_type, 2 },
    { "transient", &_XfWindow_type, 2 },
    { "popup", &_XfWindow_type, 1 },
    { "icon", &_XfWindow_type, 1 },
    { 0, 0, 0 }
};
TypeObj_ParamData _XfScreenObj_params[] = {
    /* dpi */
        { "_p", 0, &_XfCoord_type },
    /* to_pixels */
        { "c", 0, &_XfCoord_type },
    /* to_coord */
        { "p", 0, &_XfPixelCoord_type },
    /* to_pixels_coord */
        { "c", 0, &_XfCoord_type },
    /* move_pointer */
        { "x", 0, &_XfCoord_type },
        { "y", 0, &_XfCoord_type },
    /* application */
        { "v", 0, &_XfViewer_type },
    /* top_level */
        { "v", 0, &_XfViewer_type },
        { "group_leader", 0, &_XfWindow_type },
    /* transient */
        { "v", 0, &_XfViewer_type },
        { "transient_for", 0, &_XfWindow_type },
    /* popup */
        { "v", 0, &_XfViewer_type },
    /* icon */
        { "v", 0, &_XfViewer_type }
};
extern TypeObj_Descriptor _XfFrescoObject_type;
TypeObj_Descriptor* _XfScreenObj_parents[] = { &_XfFrescoObject_type, nil };
extern TypeObjId _XfScreenObj_tid;
extern void _XfScreenObj_receive(BaseObjectRef, ULong, MarshalBuffer&);
TypeObj_Descriptor _XfScreenObj_type = {
    /* type */ 0,
    /* id */ &_XfScreenObj_tid,
    "ScreenObj",
    _XfScreenObj_parents, /* offsets */ nil, /* excepts */ nil,
    _XfScreenObj_methods, _XfScreenObj_params,
    &_XfScreenObj_receive
};

ScreenObjRef ScreenObj::_narrow(BaseObjectRef o) {
    return (ScreenObjRef)_BaseObject_tnarrow(
        o, _XfScreenObj_tid, &ScreenObjStub::_create
    );
}
TypeObjId ScreenObjType::_tid() { return _XfScreenObj_tid; }
void _XfScreenObj_receive(BaseObjectRef _object, ULong _m, MarshalBuffer& _b) {
    extern TypeObjId _XfScreenObj_tid;
    ScreenObjRef _this = (ScreenObjRef)_BaseObject_tcast(_object, _XfScreenObj_tid);
    switch (_m) {
        case /* _get_dpi */ 0: {
            extern MarshalBuffer::ArgInfo _XfScreenObj__get_dpi_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_float = _this->dpi();
            _b.reply(_XfScreenObj__get_dpi_pinfo, _arg);
            break;
        }
        case /* _set_dpi */ 1: {
            extern MarshalBuffer::ArgInfo _XfScreenObj__set_dpi_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            Coord _p;
            _arg[1].u_addr = &_p;
            _b.receive(_XfScreenObj__set_dpi_pinfo, _arg);
            _this->dpi(_p);
            _b.reply(_XfScreenObj__set_dpi_pinfo, _arg);
            break;
        }
        case /* display */ 2: {
            extern MarshalBuffer::ArgInfo _XfScreenObj_display_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_objref = _this->_c_display();
            _b.reply(_XfScreenObj_display_pinfo, _arg);
            break;
        }
        case /* width */ 3: {
            extern MarshalBuffer::ArgInfo _XfScreenObj_width_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_float = _this->width();
            _b.reply(_XfScreenObj_width_pinfo, _arg);
            break;
        }
        case /* height */ 4: {
            extern MarshalBuffer::ArgInfo _XfScreenObj_height_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_float = _this->height();
            _b.reply(_XfScreenObj_height_pinfo, _arg);
            break;
        }
        case /* to_pixels */ 5: {
            extern MarshalBuffer::ArgInfo _XfScreenObj_to_pixels_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            Coord c;
            _arg[1].u_addr = &c;
            _b.receive(_XfScreenObj_to_pixels_pinfo, _arg);
            _arg[0].u_long = _this->to_pixels(c);
            _b.reply(_XfScreenObj_to_pixels_pinfo, _arg);
            break;
        }
        case /* to_coord */ 6: {
            extern MarshalBuffer::ArgInfo _XfScreenObj_to_coord_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            PixelCoord p;
            _arg[1].u_addr = &p;
            _b.receive(_XfScreenObj_to_coord_pinfo, _arg);
            _arg[0].u_float = _this->to_coord(p);
            _b.reply(_XfScreenObj_to_coord_pinfo, _arg);
            break;
        }
        case /* to_pixels_coord */ 7: {
            extern MarshalBuffer::ArgInfo _XfScreenObj_to_pixels_coord_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            Coord c;
            _arg[1].u_addr = &c;
            _b.receive(_XfScreenObj_to_pixels_coord_pinfo, _arg);
            _arg[0].u_float = _this->to_pixels_coord(c);
            _b.reply(_XfScreenObj_to_pixels_coord_pinfo, _arg);
            break;
        }
        case /* move_pointer */ 8: {
            extern MarshalBuffer::ArgInfo _XfScreenObj_move_pointer_pinfo;
            MarshalBuffer::ArgValue _arg[3];
            Coord x;
            _arg[1].u_addr = &x;
            Coord y;
            _arg[2].u_addr = &y;
            _b.receive(_XfScreenObj_move_pointer_pinfo, _arg);
            _this->move_pointer(x, y);
            _b.reply(_XfScreenObj_move_pointer_pinfo, _arg);
            break;
        }
        case /* application */ 9: {
            extern MarshalBuffer::ArgInfo _XfScreenObj_application_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            ViewerRef v;
            _arg[1].u_addr = &v;
            _b.receive(_XfScreenObj_application_pinfo, _arg);
            _arg[0].u_objref = _this->_c_application(v);
            _b.reply(_XfScreenObj_application_pinfo, _arg);
            break;
        }
        case /* top_level */ 10: {
            extern MarshalBuffer::ArgInfo _XfScreenObj_top_level_pinfo;
            MarshalBuffer::ArgValue _arg[3];
            ViewerRef v;
            _arg[1].u_addr = &v;
            WindowRef group_leader;
            _arg[2].u_addr = &group_leader;
            _b.receive(_XfScreenObj_top_level_pinfo, _arg);
            _arg[0].u_objref = _this->_c_top_level(v, group_leader);
            _b.reply(_XfScreenObj_top_level_pinfo, _arg);
            break;
        }
        case /* transient */ 11: {
            extern MarshalBuffer::ArgInfo _XfScreenObj_transient_pinfo;
            MarshalBuffer::ArgValue _arg[3];
            ViewerRef v;
            _arg[1].u_addr = &v;
            WindowRef transient_for;
            _arg[2].u_addr = &transient_for;
            _b.receive(_XfScreenObj_transient_pinfo, _arg);
            _arg[0].u_objref = _this->_c_transient(v, transient_for);
            _b.reply(_XfScreenObj_transient_pinfo, _arg);
            break;
        }
        case /* popup */ 12: {
            extern MarshalBuffer::ArgInfo _XfScreenObj_popup_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            ViewerRef v;
            _arg[1].u_addr = &v;
            _b.receive(_XfScreenObj_popup_pinfo, _arg);
            _arg[0].u_objref = _this->_c_popup(v);
            _b.reply(_XfScreenObj_popup_pinfo, _arg);
            break;
        }
        case /* icon */ 13: {
            extern MarshalBuffer::ArgInfo _XfScreenObj_icon_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            ViewerRef v;
            _arg[1].u_addr = &v;
            _b.receive(_XfScreenObj_icon_pinfo, _arg);
            _arg[0].u_objref = _this->_c_icon(v);
            _b.reply(_XfScreenObj_icon_pinfo, _arg);
            break;
        }
    }
}
ScreenObjStub::ScreenObjStub(Exchange* e) { exch_ = e; }
ScreenObjStub::~ScreenObjStub() { }
BaseObjectRef ScreenObjStub::_create(Exchange* e) {
    return (BaseObjectRef)(void*)new ScreenObjStub(e);
}
Exchange* ScreenObjStub::_exchange() {
    return exch_;
}
MarshalBuffer::ArgDesc _XfScreenObj__get_dpi_pdesc[2] = { 1, 48 };
MarshalBuffer::ArgInfo _XfScreenObj__get_dpi_pinfo = {
    &_XfScreenObj_tid, 0, _XfScreenObj__get_dpi_pdesc, 0
};
Coord ScreenObjType::dpi() {
    MarshalBuffer _b;
    extern TypeObjId _XfScreenObj_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfScreenObj__get_dpi_pinfo, _arg);
    return _arg[0].u_float;
}
MarshalBuffer::ArgDesc _XfScreenObj__set_dpi_pdesc[3] = { 2, 4, 49 };
MarshalBuffer::ArgInfo _XfScreenObj__set_dpi_pinfo = {
    &_XfScreenObj_tid, 1, _XfScreenObj__set_dpi_pdesc, 0
};
void ScreenObjType::dpi(Coord _p) {
    MarshalBuffer _b;
    extern TypeObjId _XfScreenObj_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_float = _p;
    _b.invoke(this, _XfScreenObj__set_dpi_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfScreenObj_display_pdesc[2] = { 1, 60 };
MarshalBuffer::ArgMarshal _XfScreenObj_display_pfunc[] = {
    &DisplayObjStub::_create
};
MarshalBuffer::ArgInfo _XfScreenObj_display_pinfo = {
    &_XfScreenObj_tid, 2, _XfScreenObj_display_pdesc, _XfScreenObj_display_pfunc
};
DisplayObjRef ScreenObjType::_c_display() {
    MarshalBuffer _b;
    extern TypeObjId _XfScreenObj_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfScreenObj_display_pinfo, _arg);
    return (DisplayObjRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfScreenObj_width_pdesc[2] = { 1, 48 };
MarshalBuffer::ArgInfo _XfScreenObj_width_pinfo = {
    &_XfScreenObj_tid, 3, _XfScreenObj_width_pdesc, 0
};
Coord ScreenObjType::width() {
    MarshalBuffer _b;
    extern TypeObjId _XfScreenObj_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfScreenObj_width_pinfo, _arg);
    return _arg[0].u_float;
}
MarshalBuffer::ArgDesc _XfScreenObj_height_pdesc[2] = { 1, 48 };
MarshalBuffer::ArgInfo _XfScreenObj_height_pinfo = {
    &_XfScreenObj_tid, 4, _XfScreenObj_height_pdesc, 0
};
Coord ScreenObjType::height() {
    MarshalBuffer _b;
    extern TypeObjId _XfScreenObj_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfScreenObj_height_pinfo, _arg);
    return _arg[0].u_float;
}
MarshalBuffer::ArgDesc _XfScreenObj_to_pixels_pdesc[3] = { 2, 32, 49 };
MarshalBuffer::ArgInfo _XfScreenObj_to_pixels_pinfo = {
    &_XfScreenObj_tid, 5, _XfScreenObj_to_pixels_pdesc, 0
};
PixelCoord ScreenObjType::to_pixels(Coord c) {
    MarshalBuffer _b;
    extern TypeObjId _XfScreenObj_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_float = c;
    _b.invoke(this, _XfScreenObj_to_pixels_pinfo, _arg);
    return _arg[0].u_long;
}
MarshalBuffer::ArgDesc _XfScreenObj_to_coord_pdesc[3] = { 2, 48, 33 };
MarshalBuffer::ArgInfo _XfScreenObj_to_coord_pinfo = {
    &_XfScreenObj_tid, 6, _XfScreenObj_to_coord_pdesc, 0
};
Coord ScreenObjType::to_coord(PixelCoord p) {
    MarshalBuffer _b;
    extern TypeObjId _XfScreenObj_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_long = p;
    _b.invoke(this, _XfScreenObj_to_coord_pinfo, _arg);
    return _arg[0].u_float;
}
MarshalBuffer::ArgDesc _XfScreenObj_to_pixels_coord_pdesc[3] = { 2, 48, 49 };
MarshalBuffer::ArgInfo _XfScreenObj_to_pixels_coord_pinfo = {
    &_XfScreenObj_tid, 7, _XfScreenObj_to_pixels_coord_pdesc, 0
};
Coord ScreenObjType::to_pixels_coord(Coord c) {
    MarshalBuffer _b;
    extern TypeObjId _XfScreenObj_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_float = c;
    _b.invoke(this, _XfScreenObj_to_pixels_coord_pinfo, _arg);
    return _arg[0].u_float;
}
MarshalBuffer::ArgDesc _XfScreenObj_move_pointer_pdesc[4] = { 3, 4, 49, 49 };
MarshalBuffer::ArgInfo _XfScreenObj_move_pointer_pinfo = {
    &_XfScreenObj_tid, 8, _XfScreenObj_move_pointer_pdesc, 0
};
void ScreenObjType::move_pointer(Coord x, Coord y) {
    MarshalBuffer _b;
    extern TypeObjId _XfScreenObj_tid;
    MarshalBuffer::ArgValue _arg[3];
    _arg[1].u_float = x;
    _arg[2].u_float = y;
    _b.invoke(this, _XfScreenObj_move_pointer_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfScreenObj_application_pdesc[3] = { 2, 60, 61 };
MarshalBuffer::ArgMarshal _XfScreenObj_application_pfunc[] = {
    &ViewerStub::_create,
    &WindowStub::_create
};
MarshalBuffer::ArgInfo _XfScreenObj_application_pinfo = {
    &_XfScreenObj_tid, 9, _XfScreenObj_application_pdesc, _XfScreenObj_application_pfunc
};
Window_tmp ScreenObjType::application(Viewer_in v) {
    return _c_application(v);
}
WindowRef ScreenObjType::_c_application(Viewer_in v) {
    MarshalBuffer _b;
    extern TypeObjId _XfScreenObj_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = v;
    _b.invoke(this, _XfScreenObj_application_pinfo, _arg);
    return (WindowRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfScreenObj_top_level_pdesc[4] = { 3, 60, 61, 61 };
MarshalBuffer::ArgMarshal _XfScreenObj_top_level_pfunc[] = {
    &ViewerStub::_create,
    &WindowStub::_create,
    &WindowStub::_create
};
MarshalBuffer::ArgInfo _XfScreenObj_top_level_pinfo = {
    &_XfScreenObj_tid, 10, _XfScreenObj_top_level_pdesc, _XfScreenObj_top_level_pfunc
};
Window_tmp ScreenObjType::top_level(Viewer_in v, Window_in group_leader) {
    return _c_top_level(v, group_leader);
}
WindowRef ScreenObjType::_c_top_level(Viewer_in v, Window_in group_leader) {
    MarshalBuffer _b;
    extern TypeObjId _XfScreenObj_tid;
    MarshalBuffer::ArgValue _arg[3];
    _arg[1].u_objref = v;
    _arg[2].u_objref = group_leader;
    _b.invoke(this, _XfScreenObj_top_level_pinfo, _arg);
    return (WindowRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfScreenObj_transient_pdesc[4] = { 3, 60, 61, 61 };
MarshalBuffer::ArgMarshal _XfScreenObj_transient_pfunc[] = {
    &ViewerStub::_create,
    &WindowStub::_create,
    &WindowStub::_create
};
MarshalBuffer::ArgInfo _XfScreenObj_transient_pinfo = {
    &_XfScreenObj_tid, 11, _XfScreenObj_transient_pdesc, _XfScreenObj_transient_pfunc
};
Window_tmp ScreenObjType::transient(Viewer_in v, Window_in transient_for) {
    return _c_transient(v, transient_for);
}
WindowRef ScreenObjType::_c_transient(Viewer_in v, Window_in transient_for) {
    MarshalBuffer _b;
    extern TypeObjId _XfScreenObj_tid;
    MarshalBuffer::ArgValue _arg[3];
    _arg[1].u_objref = v;
    _arg[2].u_objref = transient_for;
    _b.invoke(this, _XfScreenObj_transient_pinfo, _arg);
    return (WindowRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfScreenObj_popup_pdesc[3] = { 2, 60, 61 };
MarshalBuffer::ArgMarshal _XfScreenObj_popup_pfunc[] = {
    &ViewerStub::_create,
    &WindowStub::_create
};
MarshalBuffer::ArgInfo _XfScreenObj_popup_pinfo = {
    &_XfScreenObj_tid, 12, _XfScreenObj_popup_pdesc, _XfScreenObj_popup_pfunc
};
Window_tmp ScreenObjType::popup(Viewer_in v) {
    return _c_popup(v);
}
WindowRef ScreenObjType::_c_popup(Viewer_in v) {
    MarshalBuffer _b;
    extern TypeObjId _XfScreenObj_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = v;
    _b.invoke(this, _XfScreenObj_popup_pinfo, _arg);
    return (WindowRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfScreenObj_icon_pdesc[3] = { 2, 60, 61 };
MarshalBuffer::ArgMarshal _XfScreenObj_icon_pfunc[] = {
    &ViewerStub::_create,
    &WindowStub::_create
};
MarshalBuffer::ArgInfo _XfScreenObj_icon_pinfo = {
    &_XfScreenObj_tid, 13, _XfScreenObj_icon_pdesc, _XfScreenObj_icon_pfunc
};
Window_tmp ScreenObjType::icon(Viewer_in v) {
    return _c_icon(v);
}
WindowRef ScreenObjType::_c_icon(Viewer_in v) {
    MarshalBuffer _b;
    extern TypeObjId _XfScreenObj_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = v;
    _b.invoke(this, _XfScreenObj_icon_pinfo, _arg);
    return (WindowRef)_arg[0].u_objref;
}
//+

//+ ScreenImpl(ScreenObj)
extern TypeObj_Descriptor _XfScreenObj_type;
TypeObj_Descriptor* _XfScreenImpl_parents[] = { &_XfScreenObj_type, nil };
extern TypeObjId _XfScreenImpl_tid;
TypeObj_Descriptor _XfScreenImpl_type = {
    /* type */ 0,
    /* id */ &_XfScreenImpl_tid,
    "ScreenImpl",
    _XfScreenImpl_parents, /* offsets */ nil, /* excepts */ nil,
    /* methods */ nil, /* params */ nil,
    /* receive */ nil
};

ScreenImpl* ScreenImpl::_narrow(BaseObjectRef o) {
    return (ScreenImpl*)_BaseObject_tnarrow(
        o, _XfScreenImpl_tid, 0
    );
}
TypeObjId ScreenImpl::_tid() { return _XfScreenImpl_tid; }
//+

//+ Window::%init,type+dii,client
WindowType::WindowType() { }
WindowType::~WindowType() { }
void* WindowType::_this() { return this; }

extern TypeObj_Descriptor _XfWindowStyle_type, _XfWindow_Placement_type, 
    _XfEvent_type;

TypeObj_OpData _XfWindow_methods[] = {
    { "screen", &_XfScreenObj_type, 0 },
    { "style", &_XfWindowStyle_type, 0 },
    { "main_viewer", &_XfViewer_type, 0 },
    { "configure", &_Xfvoid_type, 1 },
    { "get_configuration", &_Xfvoid_type, 2 },
    { "configure_notify", &_Xfvoid_type, 2 },
    { "move_notify", &_Xfvoid_type, 2 },
    { "map", &_Xfvoid_type, 0 },
    { "unmap", &_Xfvoid_type, 0 },
    { "is_mapped", &_XfBoolean_type, 0 },
    { "map_notify", &_Xfvoid_type, 0 },
    { "unmap_notify", &_Xfvoid_type, 0 },
    { "iconify", &_Xfvoid_type, 0 },
    { "deiconify", &_Xfvoid_type, 0 },
    { "raise", &_Xfvoid_type, 0 },
    { "lower", &_Xfvoid_type, 0 },
    { "redraw", &_Xfvoid_type, 4 },
    { "repair", &_Xfvoid_type, 0 },
    { "handle_event", &_Xfvoid_type, 1 },
    { "grab_pointer", &_Xfvoid_type, 1 },
    { "ungrab_pointer", &_Xfvoid_type, 0 },
    { 0, 0, 0 }
};
TypeObj_ParamData _XfWindow_params[] = {
    /* configure */
        { "p", 0, &_XfWindow_Placement_type },
    /* get_configuration */
        { "position", 0, &_XfBoolean_type },
        { "p", 1, &_XfWindow_Placement_type },
    /* configure_notify */
        { "width", 0, &_XfCoord_type },
        { "height", 0, &_XfCoord_type },
    /* move_notify */
        { "left", 0, &_XfCoord_type },
        { "bottom", 0, &_XfCoord_type },
    /* redraw */
        { "left", 0, &_XfCoord_type },
        { "bottom", 0, &_XfCoord_type },
        { "width", 0, &_XfCoord_type },
        { "height", 0, &_XfCoord_type },
    /* handle_event */
        { "e", 0, &_XfEvent_type },
    /* grab_pointer */
        { "c", 0, &_XfCursor_type }
};
extern TypeObj_Descriptor _XfFrescoObject_type;
TypeObj_Descriptor* _XfWindow_parents[] = { &_XfFrescoObject_type, nil };
extern TypeObjId _XfWindow_tid;
extern void _XfWindow_receive(BaseObjectRef, ULong, MarshalBuffer&);
TypeObj_Descriptor _XfWindow_type = {
    /* type */ 0,
    /* id */ &_XfWindow_tid,
    "Window",
    _XfWindow_parents, /* offsets */ nil, /* excepts */ nil,
    _XfWindow_methods, _XfWindow_params,
    &_XfWindow_receive
};

WindowRef Window::_narrow(BaseObjectRef o) {
    return (WindowRef)_BaseObject_tnarrow(
        o, _XfWindow_tid, &WindowStub::_create
    );
}
TypeObjId WindowType::_tid() { return _XfWindow_tid; }
void _XfWindow_receive(BaseObjectRef _object, ULong _m, MarshalBuffer& _b) {
    extern TypeObjId _XfWindow_tid;
    WindowRef _this = (WindowRef)_BaseObject_tcast(_object, _XfWindow_tid);
    switch (_m) {
        case /* screen */ 0: {
            extern MarshalBuffer::ArgInfo _XfWindow_screen_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_objref = _this->_c_screen();
            _b.reply(_XfWindow_screen_pinfo, _arg);
            break;
        }
        case /* style */ 1: {
            extern MarshalBuffer::ArgInfo _XfWindow_style_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_objref = _this->_c_style();
            _b.reply(_XfWindow_style_pinfo, _arg);
            break;
        }
        case /* main_viewer */ 2: {
            extern MarshalBuffer::ArgInfo _XfWindow_main_viewer_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_objref = _this->_c_main_viewer();
            _b.reply(_XfWindow_main_viewer_pinfo, _arg);
            break;
        }
        case /* configure */ 3: {
            extern MarshalBuffer::ArgInfo _XfWindow_configure_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            Window::Placement p;
            _arg[1].u_addr = &p;
            _b.receive(_XfWindow_configure_pinfo, _arg);
            _this->configure(p);
            _b.reply(_XfWindow_configure_pinfo, _arg);
            break;
        }
        case /* get_configuration */ 4: {
            extern MarshalBuffer::ArgInfo _XfWindow_get_configuration_pinfo;
            MarshalBuffer::ArgValue _arg[3];
            Boolean position;
            _arg[1].u_addr = &position;
            Window::Placement p;
            _arg[2].u_addr = &p;
            _b.receive(_XfWindow_get_configuration_pinfo, _arg);
            _this->get_configuration(position, p);
            _b.reply(_XfWindow_get_configuration_pinfo, _arg);
            break;
        }
        case /* configure_notify */ 5: {
            extern MarshalBuffer::ArgInfo _XfWindow_configure_notify_pinfo;
            MarshalBuffer::ArgValue _arg[3];
            Coord width;
            _arg[1].u_addr = &width;
            Coord height;
            _arg[2].u_addr = &height;
            _b.receive(_XfWindow_configure_notify_pinfo, _arg);
            _this->configure_notify(width, height);
            _b.reply(_XfWindow_configure_notify_pinfo, _arg);
            break;
        }
        case /* move_notify */ 6: {
            extern MarshalBuffer::ArgInfo _XfWindow_move_notify_pinfo;
            MarshalBuffer::ArgValue _arg[3];
            Coord left;
            _arg[1].u_addr = &left;
            Coord bottom;
            _arg[2].u_addr = &bottom;
            _b.receive(_XfWindow_move_notify_pinfo, _arg);
            _this->move_notify(left, bottom);
            _b.reply(_XfWindow_move_notify_pinfo, _arg);
            break;
        }
        case /* map */ 7: {
            extern MarshalBuffer::ArgInfo _XfWindow_map_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _this->map();
            _b.reply(_XfWindow_map_pinfo, _arg);
            break;
        }
        case /* unmap */ 8: {
            extern MarshalBuffer::ArgInfo _XfWindow_unmap_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _this->unmap();
            _b.reply(_XfWindow_unmap_pinfo, _arg);
            break;
        }
        case /* is_mapped */ 9: {
            extern MarshalBuffer::ArgInfo _XfWindow_is_mapped_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_boolean = _this->is_mapped();
            _b.reply(_XfWindow_is_mapped_pinfo, _arg);
            break;
        }
        case /* map_notify */ 10: {
            extern MarshalBuffer::ArgInfo _XfWindow_map_notify_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _this->map_notify();
            _b.reply(_XfWindow_map_notify_pinfo, _arg);
            break;
        }
        case /* unmap_notify */ 11: {
            extern MarshalBuffer::ArgInfo _XfWindow_unmap_notify_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _this->unmap_notify();
            _b.reply(_XfWindow_unmap_notify_pinfo, _arg);
            break;
        }
        case /* iconify */ 12: {
            extern MarshalBuffer::ArgInfo _XfWindow_iconify_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _this->iconify();
            _b.reply(_XfWindow_iconify_pinfo, _arg);
            break;
        }
        case /* deiconify */ 13: {
            extern MarshalBuffer::ArgInfo _XfWindow_deiconify_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _this->deiconify();
            _b.reply(_XfWindow_deiconify_pinfo, _arg);
            break;
        }
        case /* raise */ 14: {
            extern MarshalBuffer::ArgInfo _XfWindow_raise_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _this->raise();
            _b.reply(_XfWindow_raise_pinfo, _arg);
            break;
        }
        case /* lower */ 15: {
            extern MarshalBuffer::ArgInfo _XfWindow_lower_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _this->lower();
            _b.reply(_XfWindow_lower_pinfo, _arg);
            break;
        }
        case /* redraw */ 16: {
            extern MarshalBuffer::ArgInfo _XfWindow_redraw_pinfo;
            MarshalBuffer::ArgValue _arg[5];
            Coord left;
            _arg[1].u_addr = &left;
            Coord bottom;
            _arg[2].u_addr = &bottom;
            Coord width;
            _arg[3].u_addr = &width;
            Coord height;
            _arg[4].u_addr = &height;
            _b.receive(_XfWindow_redraw_pinfo, _arg);
            _this->redraw(left, bottom, width, height);
            _b.reply(_XfWindow_redraw_pinfo, _arg);
            break;
        }
        case /* repair */ 17: {
            extern MarshalBuffer::ArgInfo _XfWindow_repair_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _this->repair();
            _b.reply(_XfWindow_repair_pinfo, _arg);
            break;
        }
        case /* handle_event */ 18: {
            extern MarshalBuffer::ArgInfo _XfWindow_handle_event_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            EventRef e;
            _arg[1].u_addr = &e;
            _b.receive(_XfWindow_handle_event_pinfo, _arg);
            _this->handle_event(e);
            _b.reply(_XfWindow_handle_event_pinfo, _arg);
            break;
        }
        case /* grab_pointer */ 19: {
            extern MarshalBuffer::ArgInfo _XfWindow_grab_pointer_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            CursorRef c;
            _arg[1].u_addr = &c;
            _b.receive(_XfWindow_grab_pointer_pinfo, _arg);
            _this->grab_pointer(c);
            _b.reply(_XfWindow_grab_pointer_pinfo, _arg);
            break;
        }
        case /* ungrab_pointer */ 20: {
            extern MarshalBuffer::ArgInfo _XfWindow_ungrab_pointer_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _this->ungrab_pointer();
            _b.reply(_XfWindow_ungrab_pointer_pinfo, _arg);
            break;
        }
    }
}
extern void _XfWindow_Placement_put(
    MarshalBuffer&, const Window::Placement&
);
extern void _XfWindow_Placement_get(
    MarshalBuffer&, Window::Placement&
);
extern void _XfWindow_Placement_put(
    MarshalBuffer&, const Window::Placement&
);
extern void _XfWindow_Placement_get(
    MarshalBuffer&, Window::Placement&
);

WindowStub::WindowStub(Exchange* e) { exch_ = e; }
WindowStub::~WindowStub() { }
BaseObjectRef WindowStub::_create(Exchange* e) {
    return (BaseObjectRef)(void*)new WindowStub(e);
}
Exchange* WindowStub::_exchange() {
    return exch_;
}
MarshalBuffer::ArgDesc _XfWindow_screen_pdesc[2] = { 1, 60 };
MarshalBuffer::ArgMarshal _XfWindow_screen_pfunc[] = {
    &ScreenObjStub::_create
};
MarshalBuffer::ArgInfo _XfWindow_screen_pinfo = {
    &_XfWindow_tid, 0, _XfWindow_screen_pdesc, _XfWindow_screen_pfunc
};
ScreenObjRef WindowType::_c_screen() {
    MarshalBuffer _b;
    extern TypeObjId _XfWindow_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfWindow_screen_pinfo, _arg);
    return (ScreenObjRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfWindow_style_pdesc[2] = { 1, 60 };
MarshalBuffer::ArgMarshal _XfWindow_style_pfunc[] = {
    &WindowStyleStub::_create
};
MarshalBuffer::ArgInfo _XfWindow_style_pinfo = {
    &_XfWindow_tid, 1, _XfWindow_style_pdesc, _XfWindow_style_pfunc
};
WindowStyle_tmp WindowType::style() {
    return _c_style();
}
WindowStyleRef WindowType::_c_style() {
    MarshalBuffer _b;
    extern TypeObjId _XfWindow_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfWindow_style_pinfo, _arg);
    return (WindowStyleRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfWindow_main_viewer_pdesc[2] = { 1, 60 };
MarshalBuffer::ArgMarshal _XfWindow_main_viewer_pfunc[] = {
    &ViewerStub::_create
};
MarshalBuffer::ArgInfo _XfWindow_main_viewer_pinfo = {
    &_XfWindow_tid, 2, _XfWindow_main_viewer_pdesc, _XfWindow_main_viewer_pfunc
};
Viewer_tmp WindowType::main_viewer() {
    return _c_main_viewer();
}
ViewerRef WindowType::_c_main_viewer() {
    MarshalBuffer _b;
    extern TypeObjId _XfWindow_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfWindow_main_viewer_pinfo, _arg);
    return (ViewerRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfWindow_configure_pdesc[3] = { 2, 4, 1 };
MarshalBuffer::ArgMarshal _XfWindow_configure_pfunc[] = {
    &_XfWindow_Placement_put, &_XfWindow_Placement_get,

};
MarshalBuffer::ArgInfo _XfWindow_configure_pinfo = {
    &_XfWindow_tid, 3, _XfWindow_configure_pdesc, _XfWindow_configure_pfunc
};
void WindowType::configure(const Window::Placement& p) {
    MarshalBuffer _b;
    extern TypeObjId _XfWindow_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_addr = &p;
    _b.invoke(this, _XfWindow_configure_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfWindow_get_configuration_pdesc[4] = { 3, 4, 13, 2 };
MarshalBuffer::ArgMarshal _XfWindow_get_configuration_pfunc[] = {
    &_XfWindow_Placement_put, &_XfWindow_Placement_get,

};
MarshalBuffer::ArgInfo _XfWindow_get_configuration_pinfo = {
    &_XfWindow_tid, 4, _XfWindow_get_configuration_pdesc, _XfWindow_get_configuration_pfunc
};
void WindowType::get_configuration(Boolean position, Window::Placement& p) {
    MarshalBuffer _b;
    extern TypeObjId _XfWindow_tid;
    MarshalBuffer::ArgValue _arg[3];
    _arg[1].u_boolean = position;
    _arg[2].u_addr = &p;
    _b.invoke(this, _XfWindow_get_configuration_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfWindow_configure_notify_pdesc[4] = { 3, 4, 49, 49 };
MarshalBuffer::ArgInfo _XfWindow_configure_notify_pinfo = {
    &_XfWindow_tid, 5, _XfWindow_configure_notify_pdesc, 0
};
void WindowType::configure_notify(Coord width, Coord height) {
    MarshalBuffer _b;
    extern TypeObjId _XfWindow_tid;
    MarshalBuffer::ArgValue _arg[3];
    _arg[1].u_float = width;
    _arg[2].u_float = height;
    _b.invoke(this, _XfWindow_configure_notify_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfWindow_move_notify_pdesc[4] = { 3, 4, 49, 49 };
MarshalBuffer::ArgInfo _XfWindow_move_notify_pinfo = {
    &_XfWindow_tid, 6, _XfWindow_move_notify_pdesc, 0
};
void WindowType::move_notify(Coord left, Coord bottom) {
    MarshalBuffer _b;
    extern TypeObjId _XfWindow_tid;
    MarshalBuffer::ArgValue _arg[3];
    _arg[1].u_float = left;
    _arg[2].u_float = bottom;
    _b.invoke(this, _XfWindow_move_notify_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfWindow_map_pdesc[2] = { 1, 4 };
MarshalBuffer::ArgInfo _XfWindow_map_pinfo = {
    &_XfWindow_tid, 7, _XfWindow_map_pdesc, 0
};
void WindowType::map() {
    MarshalBuffer _b;
    extern TypeObjId _XfWindow_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfWindow_map_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfWindow_unmap_pdesc[2] = { 1, 4 };
MarshalBuffer::ArgInfo _XfWindow_unmap_pinfo = {
    &_XfWindow_tid, 8, _XfWindow_unmap_pdesc, 0
};
void WindowType::unmap() {
    MarshalBuffer _b;
    extern TypeObjId _XfWindow_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfWindow_unmap_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfWindow_is_mapped_pdesc[2] = { 1, 12 };
MarshalBuffer::ArgInfo _XfWindow_is_mapped_pinfo = {
    &_XfWindow_tid, 9, _XfWindow_is_mapped_pdesc, 0
};
Boolean WindowType::is_mapped() {
    MarshalBuffer _b;
    extern TypeObjId _XfWindow_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfWindow_is_mapped_pinfo, _arg);
    return _arg[0].u_boolean;
}
MarshalBuffer::ArgDesc _XfWindow_map_notify_pdesc[2] = { 1, 4 };
MarshalBuffer::ArgInfo _XfWindow_map_notify_pinfo = {
    &_XfWindow_tid, 10, _XfWindow_map_notify_pdesc, 0
};
void WindowType::map_notify() {
    MarshalBuffer _b;
    extern TypeObjId _XfWindow_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfWindow_map_notify_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfWindow_unmap_notify_pdesc[2] = { 1, 4 };
MarshalBuffer::ArgInfo _XfWindow_unmap_notify_pinfo = {
    &_XfWindow_tid, 11, _XfWindow_unmap_notify_pdesc, 0
};
void WindowType::unmap_notify() {
    MarshalBuffer _b;
    extern TypeObjId _XfWindow_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfWindow_unmap_notify_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfWindow_iconify_pdesc[2] = { 1, 4 };
MarshalBuffer::ArgInfo _XfWindow_iconify_pinfo = {
    &_XfWindow_tid, 12, _XfWindow_iconify_pdesc, 0
};
void WindowType::iconify() {
    MarshalBuffer _b;
    extern TypeObjId _XfWindow_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfWindow_iconify_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfWindow_deiconify_pdesc[2] = { 1, 4 };
MarshalBuffer::ArgInfo _XfWindow_deiconify_pinfo = {
    &_XfWindow_tid, 13, _XfWindow_deiconify_pdesc, 0
};
void WindowType::deiconify() {
    MarshalBuffer _b;
    extern TypeObjId _XfWindow_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfWindow_deiconify_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfWindow_raise_pdesc[2] = { 1, 4 };
MarshalBuffer::ArgInfo _XfWindow_raise_pinfo = {
    &_XfWindow_tid, 14, _XfWindow_raise_pdesc, 0
};
void WindowType::raise() {
    MarshalBuffer _b;
    extern TypeObjId _XfWindow_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfWindow_raise_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfWindow_lower_pdesc[2] = { 1, 4 };
MarshalBuffer::ArgInfo _XfWindow_lower_pinfo = {
    &_XfWindow_tid, 15, _XfWindow_lower_pdesc, 0
};
void WindowType::lower() {
    MarshalBuffer _b;
    extern TypeObjId _XfWindow_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfWindow_lower_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfWindow_redraw_pdesc[6] = { 5, 4, 49, 49, 49, 49 };
MarshalBuffer::ArgInfo _XfWindow_redraw_pinfo = {
    &_XfWindow_tid, 16, _XfWindow_redraw_pdesc, 0
};
void WindowType::redraw(Coord left, Coord bottom, Coord width, Coord height) {
    MarshalBuffer _b;
    extern TypeObjId _XfWindow_tid;
    MarshalBuffer::ArgValue _arg[5];
    _arg[1].u_float = left;
    _arg[2].u_float = bottom;
    _arg[3].u_float = width;
    _arg[4].u_float = height;
    _b.invoke(this, _XfWindow_redraw_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfWindow_repair_pdesc[2] = { 1, 4 };
MarshalBuffer::ArgInfo _XfWindow_repair_pinfo = {
    &_XfWindow_tid, 17, _XfWindow_repair_pdesc, 0
};
void WindowType::repair() {
    MarshalBuffer _b;
    extern TypeObjId _XfWindow_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfWindow_repair_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfWindow_handle_event_pdesc[3] = { 2, 4, 61 };
MarshalBuffer::ArgMarshal _XfWindow_handle_event_pfunc[] = {
    &EventStub::_create,

};
MarshalBuffer::ArgInfo _XfWindow_handle_event_pinfo = {
    &_XfWindow_tid, 18, _XfWindow_handle_event_pdesc, _XfWindow_handle_event_pfunc
};
void WindowType::handle_event(Event_in e) {
    MarshalBuffer _b;
    extern TypeObjId _XfWindow_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = e;
    _b.invoke(this, _XfWindow_handle_event_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfWindow_grab_pointer_pdesc[3] = { 2, 4, 61 };
MarshalBuffer::ArgMarshal _XfWindow_grab_pointer_pfunc[] = {
    &CursorStub::_create,

};
MarshalBuffer::ArgInfo _XfWindow_grab_pointer_pinfo = {
    &_XfWindow_tid, 19, _XfWindow_grab_pointer_pdesc, _XfWindow_grab_pointer_pfunc
};
void WindowType::grab_pointer(Cursor_in c) {
    MarshalBuffer _b;
    extern TypeObjId _XfWindow_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = c;
    _b.invoke(this, _XfWindow_grab_pointer_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfWindow_ungrab_pointer_pdesc[2] = { 1, 4 };
MarshalBuffer::ArgInfo _XfWindow_ungrab_pointer_pinfo = {
    &_XfWindow_tid, 20, _XfWindow_ungrab_pointer_pdesc, 0
};
void WindowType::ungrab_pointer() {
    MarshalBuffer _b;
    extern TypeObjId _XfWindow_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfWindow_ungrab_pointer_pinfo, _arg);
}
void _XfWindow_Placement_put(MarshalBuffer& _b, const Window::Placement& _this) {
    _b.put_float(_this.x);
    _b.put_float(_this.y);
    _b.put_float(_this.width);
    _b.put_float(_this.height);
    _b.put_float(_this.align_x);
    _b.put_float(_this.align_y);
}
void _XfWindow_Placement_get(MarshalBuffer& _b, Window::Placement& _this) {
    _this.x = _b.get_float();
    _this.y = _b.get_float();
    _this.width = _b.get_float();
    _this.height = _b.get_float();
    _this.align_x = _b.get_float();
    _this.align_y = _b.get_float();
}
//+

//+ WindowImpl(Window, DamageObj)
extern TypeObj_Descriptor _XfWindow_type, _XfDamageObj_type;
TypeObj_Descriptor* _XfWindowImpl_parents[] = { &_XfWindow_type, &_XfDamageObj_type, nil };
Long _XfWindowImpl_offsets[] = {
    Long((DamageObjType*)(WindowImpl*)8) - Long((WindowImpl*)8)
};
extern TypeObjId _XfWindowImpl_tid;
TypeObj_Descriptor _XfWindowImpl_type = {
    /* type */ 0,
    /* id */ &_XfWindowImpl_tid,
    "WindowImpl",
    _XfWindowImpl_parents, _XfWindowImpl_offsets, /* excepts */ nil,
    /* methods */ nil, /* params */ nil,
    /* receive */ nil
};

WindowImpl* WindowImpl::_narrow(BaseObjectRef o) {
    return (WindowImpl*)_BaseObject_tnarrow(
        o, _XfWindowImpl_tid, 0
    );
}
TypeObjId WindowImpl::_tid() { return _XfWindowImpl_tid; }
//+

//+ WindowStyle::%init,type+dii,client
WindowStyleType::WindowStyleType() { }
WindowStyleType::~WindowStyleType() { }
void* WindowStyleType::_this() { return this; }

extern TypeObj_Descriptor _XfColor_type, _XfCharString_type;

TypeObj_OpData _XfWindowStyle_methods[] = {
    { "_get_double_buffered", &_XfBoolean_type, 0 },
    { "_set_double_buffered", &_Xfvoid_type, 1 },
    { "_get_default_cursor", &_XfCursor_type, 0 },
    { "_set_default_cursor", &_Xfvoid_type, 1 },
    { "_get_cursor_foreground", &_XfColor_type, 0 },
    { "_set_cursor_foreground", &_Xfvoid_type, 1 },
    { "_get_cursor_background", &_XfColor_type, 0 },
    { "_set_cursor_background", &_Xfvoid_type, 1 },
    { "_get_geometry", &_XfCharString_type, 0 },
    { "_set_geometry", &_Xfvoid_type, 1 },
    { "_get_icon", &_XfWindow_type, 0 },
    { "_set_icon", &_Xfvoid_type, 1 },
    { "_get_icon_bitmap", &_XfRaster_type, 0 },
    { "_set_icon_bitmap", &_Xfvoid_type, 1 },
    { "_get_icon_mask", &_XfRaster_type, 0 },
    { "_set_icon_mask", &_Xfvoid_type, 1 },
    { "_get_icon_name", &_XfCharString_type, 0 },
    { "_set_icon_name", &_Xfvoid_type, 1 },
    { "_get_icon_geometry", &_XfCharString_type, 0 },
    { "_set_icon_geometry", &_Xfvoid_type, 1 },
    { "_get_iconic", &_XfBoolean_type, 0 },
    { "_set_iconic", &_Xfvoid_type, 1 },
    { "_get_title", &_XfCharString_type, 0 },
    { "_set_title", &_Xfvoid_type, 1 },
    { "_get_xor_pixel", &_XfLong_type, 0 },
    { "_set_xor_pixel", &_Xfvoid_type, 1 },
    { 0, 0, 0 }
};
TypeObj_ParamData _XfWindowStyle_params[] = {
    /* double_buffered */
        { "_p", 0, &_XfBoolean_type },
    /* default_cursor */
        { "_p", 0, &_XfCursor_type },
    /* cursor_foreground */
        { "_p", 0, &_XfColor_type },
    /* cursor_background */
        { "_p", 0, &_XfColor_type },
    /* geometry */
        { "_p", 0, &_XfCharString_type },
    /* icon */
        { "_p", 0, &_XfWindow_type },
    /* icon_bitmap */
        { "_p", 0, &_XfRaster_type },
    /* icon_mask */
        { "_p", 0, &_XfRaster_type },
    /* icon_name */
        { "_p", 0, &_XfCharString_type },
    /* icon_geometry */
        { "_p", 0, &_XfCharString_type },
    /* iconic */
        { "_p", 0, &_XfBoolean_type },
    /* title */
        { "_p", 0, &_XfCharString_type },
    /* xor_pixel */
        { "_p", 0, &_XfLong_type }
};
extern TypeObj_Descriptor _XfStyleObj_type;
TypeObj_Descriptor* _XfWindowStyle_parents[] = { &_XfStyleObj_type, nil };
extern TypeObjId _XfWindowStyle_tid;
extern void _XfWindowStyle_receive(BaseObjectRef, ULong, MarshalBuffer&);
TypeObj_Descriptor _XfWindowStyle_type = {
    /* type */ 0,
    /* id */ &_XfWindowStyle_tid,
    "WindowStyle",
    _XfWindowStyle_parents, /* offsets */ nil, /* excepts */ nil,
    _XfWindowStyle_methods, _XfWindowStyle_params,
    &_XfWindowStyle_receive
};

WindowStyleRef WindowStyle::_narrow(BaseObjectRef o) {
    return (WindowStyleRef)_BaseObject_tnarrow(
        o, _XfWindowStyle_tid, &WindowStyleStub::_create
    );
}
TypeObjId WindowStyleType::_tid() { return _XfWindowStyle_tid; }
void _XfWindowStyle_receive(BaseObjectRef _object, ULong _m, MarshalBuffer& _b) {
    extern TypeObjId _XfWindowStyle_tid;
    WindowStyleRef _this = (WindowStyleRef)_BaseObject_tcast(_object, _XfWindowStyle_tid);
    switch (_m) {
        case /* _get_double_buffered */ 0: {
            extern MarshalBuffer::ArgInfo _XfWindowStyle__get_double_buffered_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_boolean = _this->double_buffered();
            _b.reply(_XfWindowStyle__get_double_buffered_pinfo, _arg);
            break;
        }
        case /* _set_double_buffered */ 1: {
            extern MarshalBuffer::ArgInfo _XfWindowStyle__set_double_buffered_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            Boolean _p;
            _arg[1].u_addr = &_p;
            _b.receive(_XfWindowStyle__set_double_buffered_pinfo, _arg);
            _this->double_buffered(_p);
            _b.reply(_XfWindowStyle__set_double_buffered_pinfo, _arg);
            break;
        }
        case /* _get_default_cursor */ 2: {
            extern MarshalBuffer::ArgInfo _XfWindowStyle__get_default_cursor_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_objref = _this->_c_default_cursor();
            _b.reply(_XfWindowStyle__get_default_cursor_pinfo, _arg);
            break;
        }
        case /* _set_default_cursor */ 3: {
            extern MarshalBuffer::ArgInfo _XfWindowStyle__set_default_cursor_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            CursorRef _p;
            _arg[1].u_addr = &_p;
            _b.receive(_XfWindowStyle__set_default_cursor_pinfo, _arg);
            _this->_c_default_cursor(_p);
            _b.reply(_XfWindowStyle__set_default_cursor_pinfo, _arg);
            break;
        }
        case /* _get_cursor_foreground */ 4: {
            extern MarshalBuffer::ArgInfo _XfWindowStyle__get_cursor_foreground_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_objref = _this->_c_cursor_foreground();
            _b.reply(_XfWindowStyle__get_cursor_foreground_pinfo, _arg);
            break;
        }
        case /* _set_cursor_foreground */ 5: {
            extern MarshalBuffer::ArgInfo _XfWindowStyle__set_cursor_foreground_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            ColorRef _p;
            _arg[1].u_addr = &_p;
            _b.receive(_XfWindowStyle__set_cursor_foreground_pinfo, _arg);
            _this->_c_cursor_foreground(_p);
            _b.reply(_XfWindowStyle__set_cursor_foreground_pinfo, _arg);
            break;
        }
        case /* _get_cursor_background */ 6: {
            extern MarshalBuffer::ArgInfo _XfWindowStyle__get_cursor_background_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_objref = _this->_c_cursor_background();
            _b.reply(_XfWindowStyle__get_cursor_background_pinfo, _arg);
            break;
        }
        case /* _set_cursor_background */ 7: {
            extern MarshalBuffer::ArgInfo _XfWindowStyle__set_cursor_background_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            ColorRef _p;
            _arg[1].u_addr = &_p;
            _b.receive(_XfWindowStyle__set_cursor_background_pinfo, _arg);
            _this->_c_cursor_background(_p);
            _b.reply(_XfWindowStyle__set_cursor_background_pinfo, _arg);
            break;
        }
        case /* _get_geometry */ 8: {
            extern MarshalBuffer::ArgInfo _XfWindowStyle__get_geometry_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_objref = _this->_c_geometry();
            _b.reply(_XfWindowStyle__get_geometry_pinfo, _arg);
            break;
        }
        case /* _set_geometry */ 9: {
            extern MarshalBuffer::ArgInfo _XfWindowStyle__set_geometry_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            CharStringRef _p;
            _arg[1].u_addr = &_p;
            _b.receive(_XfWindowStyle__set_geometry_pinfo, _arg);
            _this->_c_geometry(_p);
            _b.reply(_XfWindowStyle__set_geometry_pinfo, _arg);
            break;
        }
        case /* _get_icon */ 10: {
            extern MarshalBuffer::ArgInfo _XfWindowStyle__get_icon_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_objref = _this->_c_icon();
            _b.reply(_XfWindowStyle__get_icon_pinfo, _arg);
            break;
        }
        case /* _set_icon */ 11: {
            extern MarshalBuffer::ArgInfo _XfWindowStyle__set_icon_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            WindowRef _p;
            _arg[1].u_addr = &_p;
            _b.receive(_XfWindowStyle__set_icon_pinfo, _arg);
            _this->_c_icon(_p);
            _b.reply(_XfWindowStyle__set_icon_pinfo, _arg);
            break;
        }
        case /* _get_icon_bitmap */ 12: {
            extern MarshalBuffer::ArgInfo _XfWindowStyle__get_icon_bitmap_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_objref = _this->_c_icon_bitmap();
            _b.reply(_XfWindowStyle__get_icon_bitmap_pinfo, _arg);
            break;
        }
        case /* _set_icon_bitmap */ 13: {
            extern MarshalBuffer::ArgInfo _XfWindowStyle__set_icon_bitmap_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            RasterRef _p;
            _arg[1].u_addr = &_p;
            _b.receive(_XfWindowStyle__set_icon_bitmap_pinfo, _arg);
            _this->_c_icon_bitmap(_p);
            _b.reply(_XfWindowStyle__set_icon_bitmap_pinfo, _arg);
            break;
        }
        case /* _get_icon_mask */ 14: {
            extern MarshalBuffer::ArgInfo _XfWindowStyle__get_icon_mask_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_objref = _this->_c_icon_mask();
            _b.reply(_XfWindowStyle__get_icon_mask_pinfo, _arg);
            break;
        }
        case /* _set_icon_mask */ 15: {
            extern MarshalBuffer::ArgInfo _XfWindowStyle__set_icon_mask_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            RasterRef _p;
            _arg[1].u_addr = &_p;
            _b.receive(_XfWindowStyle__set_icon_mask_pinfo, _arg);
            _this->_c_icon_mask(_p);
            _b.reply(_XfWindowStyle__set_icon_mask_pinfo, _arg);
            break;
        }
        case /* _get_icon_name */ 16: {
            extern MarshalBuffer::ArgInfo _XfWindowStyle__get_icon_name_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_objref = _this->_c_icon_name();
            _b.reply(_XfWindowStyle__get_icon_name_pinfo, _arg);
            break;
        }
        case /* _set_icon_name */ 17: {
            extern MarshalBuffer::ArgInfo _XfWindowStyle__set_icon_name_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            CharStringRef _p;
            _arg[1].u_addr = &_p;
            _b.receive(_XfWindowStyle__set_icon_name_pinfo, _arg);
            _this->_c_icon_name(_p);
            _b.reply(_XfWindowStyle__set_icon_name_pinfo, _arg);
            break;
        }
        case /* _get_icon_geometry */ 18: {
            extern MarshalBuffer::ArgInfo _XfWindowStyle__get_icon_geometry_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_objref = _this->_c_icon_geometry();
            _b.reply(_XfWindowStyle__get_icon_geometry_pinfo, _arg);
            break;
        }
        case /* _set_icon_geometry */ 19: {
            extern MarshalBuffer::ArgInfo _XfWindowStyle__set_icon_geometry_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            CharStringRef _p;
            _arg[1].u_addr = &_p;
            _b.receive(_XfWindowStyle__set_icon_geometry_pinfo, _arg);
            _this->_c_icon_geometry(_p);
            _b.reply(_XfWindowStyle__set_icon_geometry_pinfo, _arg);
            break;
        }
        case /* _get_iconic */ 20: {
            extern MarshalBuffer::ArgInfo _XfWindowStyle__get_iconic_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_boolean = _this->iconic();
            _b.reply(_XfWindowStyle__get_iconic_pinfo, _arg);
            break;
        }
        case /* _set_iconic */ 21: {
            extern MarshalBuffer::ArgInfo _XfWindowStyle__set_iconic_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            Boolean _p;
            _arg[1].u_addr = &_p;
            _b.receive(_XfWindowStyle__set_iconic_pinfo, _arg);
            _this->iconic(_p);
            _b.reply(_XfWindowStyle__set_iconic_pinfo, _arg);
            break;
        }
        case /* _get_title */ 22: {
            extern MarshalBuffer::ArgInfo _XfWindowStyle__get_title_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_objref = _this->_c_title();
            _b.reply(_XfWindowStyle__get_title_pinfo, _arg);
            break;
        }
        case /* _set_title */ 23: {
            extern MarshalBuffer::ArgInfo _XfWindowStyle__set_title_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            CharStringRef _p;
            _arg[1].u_addr = &_p;
            _b.receive(_XfWindowStyle__set_title_pinfo, _arg);
            _this->_c_title(_p);
            _b.reply(_XfWindowStyle__set_title_pinfo, _arg);
            break;
        }
        case /* _get_xor_pixel */ 24: {
            extern MarshalBuffer::ArgInfo _XfWindowStyle__get_xor_pixel_pinfo;
            MarshalBuffer::ArgValue _arg[1];
            _arg[0].u_long = _this->xor_pixel();
            _b.reply(_XfWindowStyle__get_xor_pixel_pinfo, _arg);
            break;
        }
        case /* _set_xor_pixel */ 25: {
            extern MarshalBuffer::ArgInfo _XfWindowStyle__set_xor_pixel_pinfo;
            MarshalBuffer::ArgValue _arg[2];
            Long _p;
            _arg[1].u_addr = &_p;
            _b.receive(_XfWindowStyle__set_xor_pixel_pinfo, _arg);
            _this->xor_pixel(_p);
            _b.reply(_XfWindowStyle__set_xor_pixel_pinfo, _arg);
            break;
        }
    }
}
WindowStyleStub::WindowStyleStub(Exchange* e) { exch_ = e; }
WindowStyleStub::~WindowStyleStub() { }
BaseObjectRef WindowStyleStub::_create(Exchange* e) {
    return (BaseObjectRef)(void*)new WindowStyleStub(e);
}
Exchange* WindowStyleStub::_exchange() {
    return exch_;
}
MarshalBuffer::ArgDesc _XfWindowStyle__get_double_buffered_pdesc[2] = { 1, 12 };
MarshalBuffer::ArgInfo _XfWindowStyle__get_double_buffered_pinfo = {
    &_XfWindowStyle_tid, 0, _XfWindowStyle__get_double_buffered_pdesc, 0
};
Boolean WindowStyleType::double_buffered() {
    MarshalBuffer _b;
    extern TypeObjId _XfWindowStyle_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfWindowStyle__get_double_buffered_pinfo, _arg);
    return _arg[0].u_boolean;
}
MarshalBuffer::ArgDesc _XfWindowStyle__set_double_buffered_pdesc[3] = { 2, 4, 13 };
MarshalBuffer::ArgInfo _XfWindowStyle__set_double_buffered_pinfo = {
    &_XfWindowStyle_tid, 1, _XfWindowStyle__set_double_buffered_pdesc, 0
};
void WindowStyleType::double_buffered(Boolean _p) {
    MarshalBuffer _b;
    extern TypeObjId _XfWindowStyle_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_boolean = _p;
    _b.invoke(this, _XfWindowStyle__set_double_buffered_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfWindowStyle__get_default_cursor_pdesc[2] = { 1, 60 };
MarshalBuffer::ArgMarshal _XfWindowStyle__get_default_cursor_pfunc[] = {
    &CursorStub::_create
};
MarshalBuffer::ArgInfo _XfWindowStyle__get_default_cursor_pinfo = {
    &_XfWindowStyle_tid, 2, _XfWindowStyle__get_default_cursor_pdesc, _XfWindowStyle__get_default_cursor_pfunc
};
CursorRef WindowStyleType::_c_default_cursor() {
    MarshalBuffer _b;
    extern TypeObjId _XfWindowStyle_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfWindowStyle__get_default_cursor_pinfo, _arg);
    return (CursorRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfWindowStyle__set_default_cursor_pdesc[3] = { 2, 4, 61 };
MarshalBuffer::ArgMarshal _XfWindowStyle__set_default_cursor_pfunc[] = {
    &CursorStub::_create,

};
MarshalBuffer::ArgInfo _XfWindowStyle__set_default_cursor_pinfo = {
    &_XfWindowStyle_tid, 3, _XfWindowStyle__set_default_cursor_pdesc, _XfWindowStyle__set_default_cursor_pfunc
};
void WindowStyleType::_c_default_cursor(Cursor_in _p) {
    MarshalBuffer _b;
    extern TypeObjId _XfWindowStyle_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = _p;
    _b.invoke(this, _XfWindowStyle__set_default_cursor_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfWindowStyle__get_cursor_foreground_pdesc[2] = { 1, 60 };
MarshalBuffer::ArgMarshal _XfWindowStyle__get_cursor_foreground_pfunc[] = {
    &ColorStub::_create
};
MarshalBuffer::ArgInfo _XfWindowStyle__get_cursor_foreground_pinfo = {
    &_XfWindowStyle_tid, 4, _XfWindowStyle__get_cursor_foreground_pdesc, _XfWindowStyle__get_cursor_foreground_pfunc
};
ColorRef WindowStyleType::_c_cursor_foreground() {
    MarshalBuffer _b;
    extern TypeObjId _XfWindowStyle_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfWindowStyle__get_cursor_foreground_pinfo, _arg);
    return (ColorRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfWindowStyle__set_cursor_foreground_pdesc[3] = { 2, 4, 61 };
MarshalBuffer::ArgMarshal _XfWindowStyle__set_cursor_foreground_pfunc[] = {
    &ColorStub::_create,

};
MarshalBuffer::ArgInfo _XfWindowStyle__set_cursor_foreground_pinfo = {
    &_XfWindowStyle_tid, 5, _XfWindowStyle__set_cursor_foreground_pdesc, _XfWindowStyle__set_cursor_foreground_pfunc
};
void WindowStyleType::_c_cursor_foreground(Color_in _p) {
    MarshalBuffer _b;
    extern TypeObjId _XfWindowStyle_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = _p;
    _b.invoke(this, _XfWindowStyle__set_cursor_foreground_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfWindowStyle__get_cursor_background_pdesc[2] = { 1, 60 };
MarshalBuffer::ArgMarshal _XfWindowStyle__get_cursor_background_pfunc[] = {
    &ColorStub::_create
};
MarshalBuffer::ArgInfo _XfWindowStyle__get_cursor_background_pinfo = {
    &_XfWindowStyle_tid, 6, _XfWindowStyle__get_cursor_background_pdesc, _XfWindowStyle__get_cursor_background_pfunc
};
ColorRef WindowStyleType::_c_cursor_background() {
    MarshalBuffer _b;
    extern TypeObjId _XfWindowStyle_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfWindowStyle__get_cursor_background_pinfo, _arg);
    return (ColorRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfWindowStyle__set_cursor_background_pdesc[3] = { 2, 4, 61 };
MarshalBuffer::ArgMarshal _XfWindowStyle__set_cursor_background_pfunc[] = {
    &ColorStub::_create,

};
MarshalBuffer::ArgInfo _XfWindowStyle__set_cursor_background_pinfo = {
    &_XfWindowStyle_tid, 7, _XfWindowStyle__set_cursor_background_pdesc, _XfWindowStyle__set_cursor_background_pfunc
};
void WindowStyleType::_c_cursor_background(Color_in _p) {
    MarshalBuffer _b;
    extern TypeObjId _XfWindowStyle_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = _p;
    _b.invoke(this, _XfWindowStyle__set_cursor_background_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfWindowStyle__get_geometry_pdesc[2] = { 1, 60 };
MarshalBuffer::ArgMarshal _XfWindowStyle__get_geometry_pfunc[] = {
    &CharStringStub::_create
};
MarshalBuffer::ArgInfo _XfWindowStyle__get_geometry_pinfo = {
    &_XfWindowStyle_tid, 8, _XfWindowStyle__get_geometry_pdesc, _XfWindowStyle__get_geometry_pfunc
};
CharStringRef WindowStyleType::_c_geometry() {
    MarshalBuffer _b;
    extern TypeObjId _XfWindowStyle_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfWindowStyle__get_geometry_pinfo, _arg);
    return (CharStringRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfWindowStyle__set_geometry_pdesc[3] = { 2, 4, 61 };
MarshalBuffer::ArgMarshal _XfWindowStyle__set_geometry_pfunc[] = {
    &CharStringStub::_create,

};
MarshalBuffer::ArgInfo _XfWindowStyle__set_geometry_pinfo = {
    &_XfWindowStyle_tid, 9, _XfWindowStyle__set_geometry_pdesc, _XfWindowStyle__set_geometry_pfunc
};
void WindowStyleType::_c_geometry(CharString_in _p) {
    MarshalBuffer _b;
    extern TypeObjId _XfWindowStyle_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = _p;
    _b.invoke(this, _XfWindowStyle__set_geometry_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfWindowStyle__get_icon_pdesc[2] = { 1, 60 };
MarshalBuffer::ArgMarshal _XfWindowStyle__get_icon_pfunc[] = {
    &WindowStub::_create
};
MarshalBuffer::ArgInfo _XfWindowStyle__get_icon_pinfo = {
    &_XfWindowStyle_tid, 10, _XfWindowStyle__get_icon_pdesc, _XfWindowStyle__get_icon_pfunc
};
WindowRef WindowStyleType::_c_icon() {
    MarshalBuffer _b;
    extern TypeObjId _XfWindowStyle_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfWindowStyle__get_icon_pinfo, _arg);
    return (WindowRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfWindowStyle__set_icon_pdesc[3] = { 2, 4, 61 };
MarshalBuffer::ArgMarshal _XfWindowStyle__set_icon_pfunc[] = {
    &WindowStub::_create,

};
MarshalBuffer::ArgInfo _XfWindowStyle__set_icon_pinfo = {
    &_XfWindowStyle_tid, 11, _XfWindowStyle__set_icon_pdesc, _XfWindowStyle__set_icon_pfunc
};
void WindowStyleType::_c_icon(Window_in _p) {
    MarshalBuffer _b;
    extern TypeObjId _XfWindowStyle_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = _p;
    _b.invoke(this, _XfWindowStyle__set_icon_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfWindowStyle__get_icon_bitmap_pdesc[2] = { 1, 60 };
MarshalBuffer::ArgMarshal _XfWindowStyle__get_icon_bitmap_pfunc[] = {
    &RasterStub::_create
};
MarshalBuffer::ArgInfo _XfWindowStyle__get_icon_bitmap_pinfo = {
    &_XfWindowStyle_tid, 12, _XfWindowStyle__get_icon_bitmap_pdesc, _XfWindowStyle__get_icon_bitmap_pfunc
};
RasterRef WindowStyleType::_c_icon_bitmap() {
    MarshalBuffer _b;
    extern TypeObjId _XfWindowStyle_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfWindowStyle__get_icon_bitmap_pinfo, _arg);
    return (RasterRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfWindowStyle__set_icon_bitmap_pdesc[3] = { 2, 4, 61 };
MarshalBuffer::ArgMarshal _XfWindowStyle__set_icon_bitmap_pfunc[] = {
    &RasterStub::_create,

};
MarshalBuffer::ArgInfo _XfWindowStyle__set_icon_bitmap_pinfo = {
    &_XfWindowStyle_tid, 13, _XfWindowStyle__set_icon_bitmap_pdesc, _XfWindowStyle__set_icon_bitmap_pfunc
};
void WindowStyleType::_c_icon_bitmap(Raster_in _p) {
    MarshalBuffer _b;
    extern TypeObjId _XfWindowStyle_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = _p;
    _b.invoke(this, _XfWindowStyle__set_icon_bitmap_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfWindowStyle__get_icon_mask_pdesc[2] = { 1, 60 };
MarshalBuffer::ArgMarshal _XfWindowStyle__get_icon_mask_pfunc[] = {
    &RasterStub::_create
};
MarshalBuffer::ArgInfo _XfWindowStyle__get_icon_mask_pinfo = {
    &_XfWindowStyle_tid, 14, _XfWindowStyle__get_icon_mask_pdesc, _XfWindowStyle__get_icon_mask_pfunc
};
RasterRef WindowStyleType::_c_icon_mask() {
    MarshalBuffer _b;
    extern TypeObjId _XfWindowStyle_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfWindowStyle__get_icon_mask_pinfo, _arg);
    return (RasterRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfWindowStyle__set_icon_mask_pdesc[3] = { 2, 4, 61 };
MarshalBuffer::ArgMarshal _XfWindowStyle__set_icon_mask_pfunc[] = {
    &RasterStub::_create,

};
MarshalBuffer::ArgInfo _XfWindowStyle__set_icon_mask_pinfo = {
    &_XfWindowStyle_tid, 15, _XfWindowStyle__set_icon_mask_pdesc, _XfWindowStyle__set_icon_mask_pfunc
};
void WindowStyleType::_c_icon_mask(Raster_in _p) {
    MarshalBuffer _b;
    extern TypeObjId _XfWindowStyle_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = _p;
    _b.invoke(this, _XfWindowStyle__set_icon_mask_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfWindowStyle__get_icon_name_pdesc[2] = { 1, 60 };
MarshalBuffer::ArgMarshal _XfWindowStyle__get_icon_name_pfunc[] = {
    &CharStringStub::_create
};
MarshalBuffer::ArgInfo _XfWindowStyle__get_icon_name_pinfo = {
    &_XfWindowStyle_tid, 16, _XfWindowStyle__get_icon_name_pdesc, _XfWindowStyle__get_icon_name_pfunc
};
CharStringRef WindowStyleType::_c_icon_name() {
    MarshalBuffer _b;
    extern TypeObjId _XfWindowStyle_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfWindowStyle__get_icon_name_pinfo, _arg);
    return (CharStringRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfWindowStyle__set_icon_name_pdesc[3] = { 2, 4, 61 };
MarshalBuffer::ArgMarshal _XfWindowStyle__set_icon_name_pfunc[] = {
    &CharStringStub::_create,

};
MarshalBuffer::ArgInfo _XfWindowStyle__set_icon_name_pinfo = {
    &_XfWindowStyle_tid, 17, _XfWindowStyle__set_icon_name_pdesc, _XfWindowStyle__set_icon_name_pfunc
};
void WindowStyleType::_c_icon_name(CharString_in _p) {
    MarshalBuffer _b;
    extern TypeObjId _XfWindowStyle_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = _p;
    _b.invoke(this, _XfWindowStyle__set_icon_name_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfWindowStyle__get_icon_geometry_pdesc[2] = { 1, 60 };
MarshalBuffer::ArgMarshal _XfWindowStyle__get_icon_geometry_pfunc[] = {
    &CharStringStub::_create
};
MarshalBuffer::ArgInfo _XfWindowStyle__get_icon_geometry_pinfo = {
    &_XfWindowStyle_tid, 18, _XfWindowStyle__get_icon_geometry_pdesc, _XfWindowStyle__get_icon_geometry_pfunc
};
CharStringRef WindowStyleType::_c_icon_geometry() {
    MarshalBuffer _b;
    extern TypeObjId _XfWindowStyle_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfWindowStyle__get_icon_geometry_pinfo, _arg);
    return (CharStringRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfWindowStyle__set_icon_geometry_pdesc[3] = { 2, 4, 61 };
MarshalBuffer::ArgMarshal _XfWindowStyle__set_icon_geometry_pfunc[] = {
    &CharStringStub::_create,

};
MarshalBuffer::ArgInfo _XfWindowStyle__set_icon_geometry_pinfo = {
    &_XfWindowStyle_tid, 19, _XfWindowStyle__set_icon_geometry_pdesc, _XfWindowStyle__set_icon_geometry_pfunc
};
void WindowStyleType::_c_icon_geometry(CharString_in _p) {
    MarshalBuffer _b;
    extern TypeObjId _XfWindowStyle_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = _p;
    _b.invoke(this, _XfWindowStyle__set_icon_geometry_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfWindowStyle__get_iconic_pdesc[2] = { 1, 12 };
MarshalBuffer::ArgInfo _XfWindowStyle__get_iconic_pinfo = {
    &_XfWindowStyle_tid, 20, _XfWindowStyle__get_iconic_pdesc, 0
};
Boolean WindowStyleType::iconic() {
    MarshalBuffer _b;
    extern TypeObjId _XfWindowStyle_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfWindowStyle__get_iconic_pinfo, _arg);
    return _arg[0].u_boolean;
}
MarshalBuffer::ArgDesc _XfWindowStyle__set_iconic_pdesc[3] = { 2, 4, 13 };
MarshalBuffer::ArgInfo _XfWindowStyle__set_iconic_pinfo = {
    &_XfWindowStyle_tid, 21, _XfWindowStyle__set_iconic_pdesc, 0
};
void WindowStyleType::iconic(Boolean _p) {
    MarshalBuffer _b;
    extern TypeObjId _XfWindowStyle_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_boolean = _p;
    _b.invoke(this, _XfWindowStyle__set_iconic_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfWindowStyle__get_title_pdesc[2] = { 1, 60 };
MarshalBuffer::ArgMarshal _XfWindowStyle__get_title_pfunc[] = {
    &CharStringStub::_create
};
MarshalBuffer::ArgInfo _XfWindowStyle__get_title_pinfo = {
    &_XfWindowStyle_tid, 22, _XfWindowStyle__get_title_pdesc, _XfWindowStyle__get_title_pfunc
};
CharStringRef WindowStyleType::_c_title() {
    MarshalBuffer _b;
    extern TypeObjId _XfWindowStyle_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfWindowStyle__get_title_pinfo, _arg);
    return (CharStringRef)_arg[0].u_objref;
}
MarshalBuffer::ArgDesc _XfWindowStyle__set_title_pdesc[3] = { 2, 4, 61 };
MarshalBuffer::ArgMarshal _XfWindowStyle__set_title_pfunc[] = {
    &CharStringStub::_create,

};
MarshalBuffer::ArgInfo _XfWindowStyle__set_title_pinfo = {
    &_XfWindowStyle_tid, 23, _XfWindowStyle__set_title_pdesc, _XfWindowStyle__set_title_pfunc
};
void WindowStyleType::_c_title(CharString_in _p) {
    MarshalBuffer _b;
    extern TypeObjId _XfWindowStyle_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_objref = _p;
    _b.invoke(this, _XfWindowStyle__set_title_pinfo, _arg);
}
MarshalBuffer::ArgDesc _XfWindowStyle__get_xor_pixel_pdesc[2] = { 1, 32 };
MarshalBuffer::ArgInfo _XfWindowStyle__get_xor_pixel_pinfo = {
    &_XfWindowStyle_tid, 24, _XfWindowStyle__get_xor_pixel_pdesc, 0
};
Long WindowStyleType::xor_pixel() {
    MarshalBuffer _b;
    extern TypeObjId _XfWindowStyle_tid;
    MarshalBuffer::ArgValue _arg[1];
    _b.invoke(this, _XfWindowStyle__get_xor_pixel_pinfo, _arg);
    return _arg[0].u_long;
}
MarshalBuffer::ArgDesc _XfWindowStyle__set_xor_pixel_pdesc[3] = { 2, 4, 33 };
MarshalBuffer::ArgInfo _XfWindowStyle__set_xor_pixel_pinfo = {
    &_XfWindowStyle_tid, 25, _XfWindowStyle__set_xor_pixel_pdesc, 0
};
void WindowStyleType::xor_pixel(Long _p) {
    MarshalBuffer _b;
    extern TypeObjId _XfWindowStyle_tid;
    MarshalBuffer::ArgValue _arg[2];
    _arg[1].u_long = _p;
    _b.invoke(this, _XfWindowStyle__set_xor_pixel_pinfo, _arg);
}
//+
