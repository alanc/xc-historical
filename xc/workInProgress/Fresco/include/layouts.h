/* DO NOT EDIT -- Automatically generated from Interfaces/layouts.idl */

#ifndef Interfaces_layouts_h
#define Interfaces_layouts_h

#include <X11/Fresco/_enter.h>
#include <X11/Fresco/glyph.h>

class ColorType;
typedef ColorType* ColorRef;
typedef ColorRef Color_in;
class Color;
class Color_tmp;
class Color_var;

class FontType;
typedef FontType* FontRef;
typedef FontRef Font_in;
class Font;
class Font_tmp;
class Font_var;

class DeckObjType;
typedef DeckObjType* DeckObjRef;
typedef DeckObjRef DeckObj_in;
class DeckObj;
class DeckObj_tmp;
class DeckObj_var;

class DeckObj {
public:
    DeckObjRef _obj_;

    DeckObj() { _obj_ = 0; }
    DeckObj(DeckObjRef p) { _obj_ = p; }
    DeckObj& operator =(DeckObjRef p);
    DeckObj(const DeckObj&);
    DeckObj& operator =(const DeckObj& r);
    DeckObj(const DeckObj_tmp&);
    DeckObj& operator =(const DeckObj_tmp&);
    DeckObj(const DeckObj_var&);
    DeckObj& operator =(const DeckObj_var&);
    ~DeckObj();

    DeckObjRef operator ->() { return _obj_; }

    operator DeckObj_in() const { return _obj_; }
    operator Glyph() const;
    operator FrescoObject() const;
    static DeckObjRef _narrow(BaseObjectRef p);
    static DeckObj_tmp _narrow(const BaseObject& r);

    static DeckObjRef _duplicate(DeckObjRef obj);
    static DeckObj_tmp _duplicate(const DeckObj& r);
};

class DeckObj_tmp : public DeckObj {
public:
    DeckObj_tmp(DeckObjRef p) { _obj_ = p; }
    DeckObj_tmp(const DeckObj& r);
    DeckObj_tmp(const DeckObj_tmp& r);
    ~DeckObj_tmp();
};

class DeckObj_var {
public:
    DeckObjRef _obj_;

    DeckObj_var(DeckObjRef p) { _obj_ = p; }
    operator DeckObjRef() const { return _obj_; }
    DeckObjRef operator ->() { return _obj_; }
};

class DeckObjType : public GlyphType {
protected:
    DeckObjType();
    virtual ~DeckObjType();
public:
    GlyphOffset_tmp card() {
        return _c_card();
    }
    virtual GlyphOffsetRef _c_card();
    virtual void flip_to(GlyphOffset_in off);
    DeckObjRef _obj() { return this; }
    void* _this();
    virtual TypeObjId _tid();
};

class DeckObjStub : public DeckObjType {
public:
    DeckObjStub(Exchange*);
    ~DeckObjStub();

    static BaseObjectRef _create(Exchange*);
    Exchange* _exchange();
protected:
    Exchange* exch_;
};

class ScrollBoxType;
typedef ScrollBoxType* ScrollBoxRef;
typedef ScrollBoxRef ScrollBox_in;
class ScrollBox;
class ScrollBox_tmp;
class ScrollBox_var;

class ScrollBox {
public:
    ScrollBoxRef _obj_;

    ScrollBox() { _obj_ = 0; }
    ScrollBox(ScrollBoxRef p) { _obj_ = p; }
    ScrollBox& operator =(ScrollBoxRef p);
    ScrollBox(const ScrollBox&);
    ScrollBox& operator =(const ScrollBox& r);
    ScrollBox(const ScrollBox_tmp&);
    ScrollBox& operator =(const ScrollBox_tmp&);
    ScrollBox(const ScrollBox_var&);
    ScrollBox& operator =(const ScrollBox_var&);
    ~ScrollBox();

    ScrollBoxRef operator ->() { return _obj_; }

    operator ScrollBox_in() const { return _obj_; }
    operator Glyph() const;
    operator FrescoObject() const;
    static ScrollBoxRef _narrow(BaseObjectRef p);
    static ScrollBox_tmp _narrow(const BaseObject& r);

    static ScrollBoxRef _duplicate(ScrollBoxRef obj);
    static ScrollBox_tmp _duplicate(const ScrollBox& r);
};

class ScrollBox_tmp : public ScrollBox {
public:
    ScrollBox_tmp(ScrollBoxRef p) { _obj_ = p; }
    ScrollBox_tmp(const ScrollBox& r);
    ScrollBox_tmp(const ScrollBox_tmp& r);
    ~ScrollBox_tmp();
};

class ScrollBox_var {
public:
    ScrollBoxRef _obj_;

    ScrollBox_var(ScrollBoxRef p) { _obj_ = p; }
    operator ScrollBoxRef() const { return _obj_; }
    ScrollBoxRef operator ->() { return _obj_; }
};

class ScrollBoxType : public GlyphType {
protected:
    ScrollBoxType();
    virtual ~ScrollBoxType();
public:
    virtual Boolean shown(GlyphOffset_in off);
    GlyphOffset_tmp first_shown() {
        return _c_first_shown();
    }
    virtual GlyphOffsetRef _c_first_shown();
    GlyphOffset_tmp last_shown() {
        return _c_last_shown();
    }
    virtual GlyphOffsetRef _c_last_shown();
    ScrollBoxRef _obj() { return this; }
    void* _this();
    virtual TypeObjId _tid();
};

class ScrollBoxStub : public ScrollBoxType {
public:
    ScrollBoxStub(Exchange*);
    ~ScrollBoxStub();

    static BaseObjectRef _create(Exchange*);
    Exchange* _exchange();
protected:
    Exchange* exch_;
};

class LayoutKitType;
typedef LayoutKitType* LayoutKitRef;
typedef LayoutKitRef LayoutKit_in;
class LayoutKit;
class LayoutKit_tmp;
class LayoutKit_var;

class LayoutKit {
public:
    LayoutKitRef _obj_;

    LayoutKit() { _obj_ = 0; }
    LayoutKit(LayoutKitRef p) { _obj_ = p; }
    LayoutKit& operator =(LayoutKitRef p);
    LayoutKit(const LayoutKit&);
    LayoutKit& operator =(const LayoutKit& r);
    LayoutKit(const LayoutKit_tmp&);
    LayoutKit& operator =(const LayoutKit_tmp&);
    LayoutKit(const LayoutKit_var&);
    LayoutKit& operator =(const LayoutKit_var&);
    ~LayoutKit();

    LayoutKitRef operator ->() { return _obj_; }

    operator LayoutKit_in() const { return _obj_; }
    operator FrescoObject() const;
    static LayoutKitRef _narrow(BaseObjectRef p);
    static LayoutKit_tmp _narrow(const BaseObject& r);

    static LayoutKitRef _duplicate(LayoutKitRef obj);
    static LayoutKit_tmp _duplicate(const LayoutKit& r);
};

class LayoutKit_tmp : public LayoutKit {
public:
    LayoutKit_tmp(LayoutKitRef p) { _obj_ = p; }
    LayoutKit_tmp(const LayoutKit& r);
    LayoutKit_tmp(const LayoutKit_tmp& r);
    ~LayoutKit_tmp();
};

class LayoutKit_var {
public:
    LayoutKitRef _obj_;

    LayoutKit_var(LayoutKitRef p) { _obj_ = p; }
    operator LayoutKitRef() const { return _obj_; }
    LayoutKitRef operator ->() { return _obj_; }
};

class LayoutKitType : public FrescoObjectType {
protected:
    LayoutKitType();
    virtual ~LayoutKitType();
public:
    virtual Coord fil();
    virtual void fil(Coord _p);
    Glyph_tmp hbox() {
        return _c_hbox();
    }
    virtual GlyphRef _c_hbox();
    Glyph_tmp vbox() {
        return _c_vbox();
    }
    virtual GlyphRef _c_vbox();
    Glyph_tmp hbox_first_aligned() {
        return _c_hbox_first_aligned();
    }
    virtual GlyphRef _c_hbox_first_aligned();
    Glyph_tmp vbox_first_aligned() {
        return _c_vbox_first_aligned();
    }
    virtual GlyphRef _c_vbox_first_aligned();
    ScrollBox_tmp vscrollbox() {
        return _c_vscrollbox();
    }
    virtual ScrollBoxRef _c_vscrollbox();
    Glyph_tmp overlay() {
        return _c_overlay();
    }
    virtual GlyphRef _c_overlay();
    DeckObj_tmp deck() {
        return _c_deck();
    }
    virtual DeckObjRef _c_deck();
    Glyph_tmp back(Glyph_in g, Glyph_in under) {
        return _c_back(g, under);
    }
    virtual GlyphRef _c_back(Glyph_in g, Glyph_in under);
    Glyph_tmp front(Glyph_in g, Glyph_in over) {
        return _c_front(g, over);
    }
    virtual GlyphRef _c_front(Glyph_in g, Glyph_in over);
    Glyph_tmp between(Glyph_in g, Glyph_in under, Glyph_in over) {
        return _c_between(g, under, over);
    }
    virtual GlyphRef _c_between(Glyph_in g, Glyph_in under, Glyph_in over);
    Glyph_tmp glue(Axis a, Coord natural, Coord stretch, Coord shrink, Alignment align) {
        return _c_glue(a, natural, stretch, shrink, align);
    }
    virtual GlyphRef _c_glue(Axis a, Coord natural, Coord stretch, Coord shrink, Alignment align);
    Glyph_tmp glue_requisition(const Glyph::Requisition& r) {
        return _c_glue_requisition(r);
    }
    virtual GlyphRef _c_glue_requisition(const Glyph::Requisition& r);
    Glyph_tmp hfil() {
        return _c_hfil();
    }
    virtual GlyphRef _c_hfil();
    Glyph_tmp hglue_fil(Coord natural) {
        return _c_hglue_fil(natural);
    }
    virtual GlyphRef _c_hglue_fil(Coord natural);
    Glyph_tmp hglue(Coord natural, Coord stretch, Coord shrink) {
        return _c_hglue(natural, stretch, shrink);
    }
    virtual GlyphRef _c_hglue(Coord natural, Coord stretch, Coord shrink);
    Glyph_tmp hglue_aligned(Coord natural, Coord stretch, Coord shrink, Alignment a) {
        return _c_hglue_aligned(natural, stretch, shrink, a);
    }
    virtual GlyphRef _c_hglue_aligned(Coord natural, Coord stretch, Coord shrink, Alignment a);
    Glyph_tmp hspace(Coord natural) {
        return _c_hspace(natural);
    }
    virtual GlyphRef _c_hspace(Coord natural);
    Glyph_tmp vfil() {
        return _c_vfil();
    }
    virtual GlyphRef _c_vfil();
    Glyph_tmp vglue_fil(Coord natural) {
        return _c_vglue_fil(natural);
    }
    virtual GlyphRef _c_vglue_fil(Coord natural);
    Glyph_tmp vglue(Coord natural, Coord stretch, Coord shrink) {
        return _c_vglue(natural, stretch, shrink);
    }
    virtual GlyphRef _c_vglue(Coord natural, Coord stretch, Coord shrink);
    Glyph_tmp vglue_aligned(Coord natural, Coord stretch, Coord shrink, Alignment a) {
        return _c_vglue_aligned(natural, stretch, shrink, a);
    }
    virtual GlyphRef _c_vglue_aligned(Coord natural, Coord stretch, Coord shrink, Alignment a);
    Glyph_tmp vspace(Coord natural) {
        return _c_vspace(natural);
    }
    virtual GlyphRef _c_vspace(Coord natural);
    Glyph_tmp shape_of(Glyph_in g) {
        return _c_shape_of(g);
    }
    virtual GlyphRef _c_shape_of(Glyph_in g);
    Glyph_tmp shape_of_xy(Glyph_in gx, Glyph_in gy) {
        return _c_shape_of_xy(gx, gy);
    }
    virtual GlyphRef _c_shape_of_xy(Glyph_in gx, Glyph_in gy);
    Glyph_tmp shape_of_xyz(Glyph_in gx, Glyph_in gy, Glyph_in gz) {
        return _c_shape_of_xyz(gx, gy, gz);
    }
    virtual GlyphRef _c_shape_of_xyz(Glyph_in gx, Glyph_in gy, Glyph_in gz);
    Glyph_tmp strut(Font_in f, Coord natural, Coord stretch, Coord shrink) {
        return _c_strut(f, natural, stretch, shrink);
    }
    virtual GlyphRef _c_strut(Font_in f, Coord natural, Coord stretch, Coord shrink);
    Glyph_tmp hstrut(Coord right_bearing, Coord left_bearing, Coord natural, Coord stretch, Coord shrink) {
        return _c_hstrut(right_bearing, left_bearing, natural, stretch, shrink);
    }
    virtual GlyphRef _c_hstrut(Coord right_bearing, Coord left_bearing, Coord natural, Coord stretch, Coord shrink);
    Glyph_tmp vstrut(Coord ascent, Coord descent, Coord natural, Coord stretch, Coord shrink) {
        return _c_vstrut(ascent, descent, natural, stretch, shrink);
    }
    virtual GlyphRef _c_vstrut(Coord ascent, Coord descent, Coord natural, Coord stretch, Coord shrink);
    Glyph_tmp spaces(Long count, Coord each, Font_in f, Color_in c) {
        return _c_spaces(count, each, f, c);
    }
    virtual GlyphRef _c_spaces(Long count, Coord each, Font_in f, Color_in c);
    Glyph_tmp center(Glyph_in g) {
        return _c_center(g);
    }
    virtual GlyphRef _c_center(Glyph_in g);
    Glyph_tmp center_aligned(Glyph_in g, Alignment x, Alignment y) {
        return _c_center_aligned(g, x, y);
    }
    virtual GlyphRef _c_center_aligned(Glyph_in g, Alignment x, Alignment y);
    Glyph_tmp center_axis(Glyph_in g, Axis a, Alignment align) {
        return _c_center_axis(g, a, align);
    }
    virtual GlyphRef _c_center_axis(Glyph_in g, Axis a, Alignment align);
    Glyph_tmp hcenter(Glyph_in g) {
        return _c_hcenter(g);
    }
    virtual GlyphRef _c_hcenter(Glyph_in g);
    Glyph_tmp hcenter_aligned(Glyph_in g, Alignment x) {
        return _c_hcenter_aligned(g, x);
    }
    virtual GlyphRef _c_hcenter_aligned(Glyph_in g, Alignment x);
    Glyph_tmp vcenter(Glyph_in g) {
        return _c_vcenter(g);
    }
    virtual GlyphRef _c_vcenter(Glyph_in g);
    Glyph_tmp vcenter_aligned(Glyph_in g, Alignment y) {
        return _c_vcenter_aligned(g, y);
    }
    virtual GlyphRef _c_vcenter_aligned(Glyph_in g, Alignment y);
    Glyph_tmp fixed(Glyph_in g, Coord x, Coord y) {
        return _c_fixed(g, x, y);
    }
    virtual GlyphRef _c_fixed(Glyph_in g, Coord x, Coord y);
    Glyph_tmp fixed_axis(Glyph_in g, Axis a, Coord size) {
        return _c_fixed_axis(g, a, size);
    }
    virtual GlyphRef _c_fixed_axis(Glyph_in g, Axis a, Coord size);
    Glyph_tmp hfixed(Glyph_in g, Coord x) {
        return _c_hfixed(g, x);
    }
    virtual GlyphRef _c_hfixed(Glyph_in g, Coord x);
    Glyph_tmp vfixed(Glyph_in g, Coord y) {
        return _c_vfixed(g, y);
    }
    virtual GlyphRef _c_vfixed(Glyph_in g, Coord y);
    Glyph_tmp flexible(Glyph_in g, Coord stretch, Coord shrink) {
        return _c_flexible(g, stretch, shrink);
    }
    virtual GlyphRef _c_flexible(Glyph_in g, Coord stretch, Coord shrink);
    Glyph_tmp flexible_fil(Glyph_in g) {
        return _c_flexible_fil(g);
    }
    virtual GlyphRef _c_flexible_fil(Glyph_in g);
    Glyph_tmp flexible_axis(Glyph_in g, Axis a, Coord stretch, Coord shrink) {
        return _c_flexible_axis(g, a, stretch, shrink);
    }
    virtual GlyphRef _c_flexible_axis(Glyph_in g, Axis a, Coord stretch, Coord shrink);
    Glyph_tmp hflexible(Glyph_in g, Coord stretch, Coord shrink) {
        return _c_hflexible(g, stretch, shrink);
    }
    virtual GlyphRef _c_hflexible(Glyph_in g, Coord stretch, Coord shrink);
    Glyph_tmp vflexible(Glyph_in g, Coord stretch, Coord shrink) {
        return _c_vflexible(g, stretch, shrink);
    }
    virtual GlyphRef _c_vflexible(Glyph_in g, Coord stretch, Coord shrink);
    Glyph_tmp natural(Glyph_in g, Coord x, Coord y) {
        return _c_natural(g, x, y);
    }
    virtual GlyphRef _c_natural(Glyph_in g, Coord x, Coord y);
    Glyph_tmp natural_axis(Glyph_in g, Axis a, Coord size) {
        return _c_natural_axis(g, a, size);
    }
    virtual GlyphRef _c_natural_axis(Glyph_in g, Axis a, Coord size);
    Glyph_tmp hnatural(Glyph_in g, Coord x) {
        return _c_hnatural(g, x);
    }
    virtual GlyphRef _c_hnatural(Glyph_in g, Coord x);
    Glyph_tmp vnatural(Glyph_in g, Coord y) {
        return _c_vnatural(g, y);
    }
    virtual GlyphRef _c_vnatural(Glyph_in g, Coord y);
    Glyph_tmp margin(Glyph_in g, Coord all) {
        return _c_margin(g, all);
    }
    virtual GlyphRef _c_margin(Glyph_in g, Coord all);
    Glyph_tmp margin_lrbt(Glyph_in g, Coord lmargin, Coord rmargin, Coord bmargin, Coord tmargin) {
        return _c_margin_lrbt(g, lmargin, rmargin, bmargin, tmargin);
    }
    virtual GlyphRef _c_margin_lrbt(Glyph_in g, Coord lmargin, Coord rmargin, Coord bmargin, Coord tmargin);
    Glyph_tmp margin_lrbt_flexible(Glyph_in g, Coord lmargin, Coord lstretch, Coord lshrink, Coord rmargin, Coord rstretch, Coord rshrink, Coord bmargin, Coord bstretch, Coord bshrink, Coord tmargin, Coord tstretch, Coord tshrink) {
        return _c_margin_lrbt_flexible(g, lmargin, lstretch, lshrink, rmargin, rstretch, rshrink, bmargin, bstretch, bshrink, tmargin, tstretch, tshrink);
    }
    virtual GlyphRef _c_margin_lrbt_flexible(Glyph_in g, Coord lmargin, Coord lstretch, Coord lshrink, Coord rmargin, Coord rstretch, Coord rshrink, Coord bmargin, Coord bstretch, Coord bshrink, Coord tmargin, Coord tstretch, Coord tshrink);
    Glyph_tmp hmargin(Glyph_in g, Coord both) {
        return _c_hmargin(g, both);
    }
    virtual GlyphRef _c_hmargin(Glyph_in g, Coord both);
    Glyph_tmp hmargin_lr(Glyph_in g, Coord lmargin, Coord rmargin) {
        return _c_hmargin_lr(g, lmargin, rmargin);
    }
    virtual GlyphRef _c_hmargin_lr(Glyph_in g, Coord lmargin, Coord rmargin);
    Glyph_tmp hmargin_lr_flexible(Glyph_in g, Coord lmargin, Coord lstretch, Coord lshrink, Coord rmargin, Coord rstretch, Coord rshrink) {
        return _c_hmargin_lr_flexible(g, lmargin, lstretch, lshrink, rmargin, rstretch, rshrink);
    }
    virtual GlyphRef _c_hmargin_lr_flexible(Glyph_in g, Coord lmargin, Coord lstretch, Coord lshrink, Coord rmargin, Coord rstretch, Coord rshrink);
    Glyph_tmp vmargin(Glyph_in g, Coord both) {
        return _c_vmargin(g, both);
    }
    virtual GlyphRef _c_vmargin(Glyph_in g, Coord both);
    Glyph_tmp vmargin_bt(Glyph_in g, Coord bmargin, Coord tmargin) {
        return _c_vmargin_bt(g, bmargin, tmargin);
    }
    virtual GlyphRef _c_vmargin_bt(Glyph_in g, Coord bmargin, Coord tmargin);
    Glyph_tmp vmargin_bt_flexible(Glyph_in g, Coord bmargin, Coord bstretch, Coord bshrink, Coord tmargin, Coord tstretch, Coord tshrink) {
        return _c_vmargin_bt_flexible(g, bmargin, bstretch, bshrink, tmargin, tstretch, tshrink);
    }
    virtual GlyphRef _c_vmargin_bt_flexible(Glyph_in g, Coord bmargin, Coord bstretch, Coord bshrink, Coord tmargin, Coord tstretch, Coord tshrink);
    Glyph_tmp lmargin(Glyph_in g, Coord natural) {
        return _c_lmargin(g, natural);
    }
    virtual GlyphRef _c_lmargin(Glyph_in g, Coord natural);
    Glyph_tmp lmargin_flexible(Glyph_in g, Coord natural, Coord stretch, Coord shrink) {
        return _c_lmargin_flexible(g, natural, stretch, shrink);
    }
    virtual GlyphRef _c_lmargin_flexible(Glyph_in g, Coord natural, Coord stretch, Coord shrink);
    Glyph_tmp rmargin(Glyph_in g, Coord natural) {
        return _c_rmargin(g, natural);
    }
    virtual GlyphRef _c_rmargin(Glyph_in g, Coord natural);
    Glyph_tmp rmargin_flexible(Glyph_in g, Coord natural, Coord stretch, Coord shrink) {
        return _c_rmargin_flexible(g, natural, stretch, shrink);
    }
    virtual GlyphRef _c_rmargin_flexible(Glyph_in g, Coord natural, Coord stretch, Coord shrink);
    Glyph_tmp bmargin(Glyph_in g, Coord natural) {
        return _c_bmargin(g, natural);
    }
    virtual GlyphRef _c_bmargin(Glyph_in g, Coord natural);
    Glyph_tmp bmargin_flexible(Glyph_in g, Coord natural, Coord stretch, Coord shrink) {
        return _c_bmargin_flexible(g, natural, stretch, shrink);
    }
    virtual GlyphRef _c_bmargin_flexible(Glyph_in g, Coord natural, Coord stretch, Coord shrink);
    Glyph_tmp tmargin(Glyph_in g, Coord natural) {
        return _c_tmargin(g, natural);
    }
    virtual GlyphRef _c_tmargin(Glyph_in g, Coord natural);
    Glyph_tmp tmargin_flexible(Glyph_in g, Coord natural, Coord stretch, Coord shrink) {
        return _c_tmargin_flexible(g, natural, stretch, shrink);
    }
    virtual GlyphRef _c_tmargin_flexible(Glyph_in g, Coord natural, Coord stretch, Coord shrink);
    LayoutKitRef _obj() { return this; }
    void* _this();
    virtual TypeObjId _tid();
};

class LayoutKitStub : public LayoutKitType {
public:
    LayoutKitStub(Exchange*);
    ~LayoutKitStub();

    static BaseObjectRef _create(Exchange*);
    Exchange* _exchange();
protected:
    Exchange* exch_;
};

inline DeckObjRef DeckObj::_duplicate(DeckObjRef obj) {
    return (DeckObjRef)_BaseObject__duplicate(obj, &DeckObjStub::_create);
}
inline DeckObj& DeckObj::operator =(DeckObjRef p) {
    _BaseObject__release(_obj_);
    _obj_ = DeckObj::_duplicate(p);
    return *this;
}
inline DeckObj::DeckObj(const DeckObj& r) {
    _obj_ = DeckObj::_duplicate(r._obj_);
}
inline DeckObj& DeckObj::operator =(const DeckObj& r) {
    _BaseObject__release(_obj_);
    _obj_ = DeckObj::_duplicate(r._obj_);
    return *this;
}
inline DeckObj::DeckObj(const DeckObj_tmp& r) {
    _obj_ = r._obj_;
    ((DeckObj_tmp*)&r)->_obj_ = 0;
}
inline DeckObj& DeckObj::operator =(const DeckObj_tmp& r) {
    _BaseObject__release(_obj_);
    _obj_ = r._obj_;
    ((DeckObj_tmp*)&r)->_obj_ = 0;
    return *this;
}
inline DeckObj::DeckObj(const DeckObj_var& e) {
    _obj_ = DeckObj::_duplicate(e._obj_);
}
inline DeckObj& DeckObj::operator =(const DeckObj_var& e) {
    _BaseObject__release(_obj_);
    _obj_ = DeckObj::_duplicate(e._obj_);
    return *this;
}
inline DeckObj::~DeckObj() {
    _BaseObject__release(_obj_);
}
inline DeckObj_tmp DeckObj::_narrow(const BaseObject& r) {
    return _narrow(r._obj_);
}
inline DeckObj_tmp DeckObj::_duplicate(const DeckObj& r) {
    return _duplicate(r._obj_);
}
inline DeckObj::operator Glyph() const {
    return Glyph_tmp((GlyphRef)_BaseObject__duplicate((GlyphRef)_obj_, &GlyphStub::_create));
}
inline DeckObj::operator FrescoObject() const {
    return FrescoObject_tmp((FrescoObjectRef)_BaseObject__duplicate((GlyphRef)(FrescoObjectRef)_obj_, &FrescoObjectStub::_create));
}
inline DeckObj_tmp::DeckObj_tmp(const DeckObj& r) {
    _obj_ = DeckObj::_duplicate(r._obj_);
}
inline DeckObj_tmp::DeckObj_tmp(const DeckObj_tmp& r) {
    _obj_ = r._obj_;
    ((DeckObj_tmp*)&r)->_obj_ = 0;
}
inline DeckObj_tmp::~DeckObj_tmp() { }

inline ScrollBoxRef ScrollBox::_duplicate(ScrollBoxRef obj) {
    return (ScrollBoxRef)_BaseObject__duplicate(obj, &ScrollBoxStub::_create);
}
inline ScrollBox& ScrollBox::operator =(ScrollBoxRef p) {
    _BaseObject__release(_obj_);
    _obj_ = ScrollBox::_duplicate(p);
    return *this;
}
inline ScrollBox::ScrollBox(const ScrollBox& r) {
    _obj_ = ScrollBox::_duplicate(r._obj_);
}
inline ScrollBox& ScrollBox::operator =(const ScrollBox& r) {
    _BaseObject__release(_obj_);
    _obj_ = ScrollBox::_duplicate(r._obj_);
    return *this;
}
inline ScrollBox::ScrollBox(const ScrollBox_tmp& r) {
    _obj_ = r._obj_;
    ((ScrollBox_tmp*)&r)->_obj_ = 0;
}
inline ScrollBox& ScrollBox::operator =(const ScrollBox_tmp& r) {
    _BaseObject__release(_obj_);
    _obj_ = r._obj_;
    ((ScrollBox_tmp*)&r)->_obj_ = 0;
    return *this;
}
inline ScrollBox::ScrollBox(const ScrollBox_var& e) {
    _obj_ = ScrollBox::_duplicate(e._obj_);
}
inline ScrollBox& ScrollBox::operator =(const ScrollBox_var& e) {
    _BaseObject__release(_obj_);
    _obj_ = ScrollBox::_duplicate(e._obj_);
    return *this;
}
inline ScrollBox::~ScrollBox() {
    _BaseObject__release(_obj_);
}
inline ScrollBox_tmp ScrollBox::_narrow(const BaseObject& r) {
    return _narrow(r._obj_);
}
inline ScrollBox_tmp ScrollBox::_duplicate(const ScrollBox& r) {
    return _duplicate(r._obj_);
}
inline ScrollBox::operator Glyph() const {
    return Glyph_tmp((GlyphRef)_BaseObject__duplicate((GlyphRef)_obj_, &GlyphStub::_create));
}
inline ScrollBox::operator FrescoObject() const {
    return FrescoObject_tmp((FrescoObjectRef)_BaseObject__duplicate((GlyphRef)(FrescoObjectRef)_obj_, &FrescoObjectStub::_create));
}
inline ScrollBox_tmp::ScrollBox_tmp(const ScrollBox& r) {
    _obj_ = ScrollBox::_duplicate(r._obj_);
}
inline ScrollBox_tmp::ScrollBox_tmp(const ScrollBox_tmp& r) {
    _obj_ = r._obj_;
    ((ScrollBox_tmp*)&r)->_obj_ = 0;
}
inline ScrollBox_tmp::~ScrollBox_tmp() { }

inline LayoutKitRef LayoutKit::_duplicate(LayoutKitRef obj) {
    return (LayoutKitRef)_BaseObject__duplicate(obj, &LayoutKitStub::_create);
}
inline LayoutKit& LayoutKit::operator =(LayoutKitRef p) {
    _BaseObject__release(_obj_);
    _obj_ = LayoutKit::_duplicate(p);
    return *this;
}
inline LayoutKit::LayoutKit(const LayoutKit& r) {
    _obj_ = LayoutKit::_duplicate(r._obj_);
}
inline LayoutKit& LayoutKit::operator =(const LayoutKit& r) {
    _BaseObject__release(_obj_);
    _obj_ = LayoutKit::_duplicate(r._obj_);
    return *this;
}
inline LayoutKit::LayoutKit(const LayoutKit_tmp& r) {
    _obj_ = r._obj_;
    ((LayoutKit_tmp*)&r)->_obj_ = 0;
}
inline LayoutKit& LayoutKit::operator =(const LayoutKit_tmp& r) {
    _BaseObject__release(_obj_);
    _obj_ = r._obj_;
    ((LayoutKit_tmp*)&r)->_obj_ = 0;
    return *this;
}
inline LayoutKit::LayoutKit(const LayoutKit_var& e) {
    _obj_ = LayoutKit::_duplicate(e._obj_);
}
inline LayoutKit& LayoutKit::operator =(const LayoutKit_var& e) {
    _BaseObject__release(_obj_);
    _obj_ = LayoutKit::_duplicate(e._obj_);
    return *this;
}
inline LayoutKit::~LayoutKit() {
    _BaseObject__release(_obj_);
}
inline LayoutKit_tmp LayoutKit::_narrow(const BaseObject& r) {
    return _narrow(r._obj_);
}
inline LayoutKit_tmp LayoutKit::_duplicate(const LayoutKit& r) {
    return _duplicate(r._obj_);
}
inline LayoutKit::operator FrescoObject() const {
    return FrescoObject_tmp((FrescoObjectRef)_BaseObject__duplicate((FrescoObjectRef)_obj_, &FrescoObjectStub::_create));
}
inline LayoutKit_tmp::LayoutKit_tmp(const LayoutKit& r) {
    _obj_ = LayoutKit::_duplicate(r._obj_);
}
inline LayoutKit_tmp::LayoutKit_tmp(const LayoutKit_tmp& r) {
    _obj_ = r._obj_;
    ((LayoutKit_tmp*)&r)->_obj_ = 0;
}
inline LayoutKit_tmp::~LayoutKit_tmp() { }

#endif
