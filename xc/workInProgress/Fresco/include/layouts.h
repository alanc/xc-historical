/* DO NOT EDIT -- Automatically generated from Interfaces/layouts.idl */

#ifndef Interfaces_layouts_h
#define Interfaces_layouts_h

#include <X11/Fresco/_enter.h>
#include <X11/Fresco/glyph.h>

class ColorType;
typedef ColorType* ColorRef;
class Color;
class _ColorExpr;
class _ColorElem;

class FontType;
typedef FontType* FontRef;
class Font;
class _FontExpr;
class _FontElem;

class DeckObjType;
typedef DeckObjType* DeckObjRef;
class DeckObj;
class _DeckObjExpr;
class _DeckObjElem;

class DeckObj {
public:
    DeckObjRef _obj;

    DeckObj() { _obj = 0; }
    DeckObj(DeckObjRef p) { _obj = p; }
    DeckObj& operator =(DeckObjRef p);
    DeckObj(const DeckObj&);
    DeckObj& operator =(const DeckObj& r);
    DeckObj(const _DeckObjExpr&);
    DeckObj& operator =(const _DeckObjExpr&);
    DeckObj(const _DeckObjElem&);
    DeckObj& operator =(const _DeckObjElem&);
    ~DeckObj();

    operator DeckObjRef() const { return _obj; }
    DeckObjRef operator ->() { return _obj; }

    operator Glyph() const;
    operator FrescoObject() const;
    static DeckObjRef _narrow(BaseObjectRef p);
    static _DeckObjExpr _narrow(const BaseObject& r);

    static DeckObjRef _duplicate(DeckObjRef obj);
    static _DeckObjExpr _duplicate(const DeckObj& r);
};

class _DeckObjExpr : public DeckObj {
public:
    _DeckObjExpr(DeckObjRef p) { _obj = p; }
    _DeckObjExpr(const DeckObj& r) { _obj = r._obj; }
    _DeckObjExpr(const _DeckObjExpr& r) { _obj = r._obj; }
    ~_DeckObjExpr();
};

class _DeckObjElem {
public:
    DeckObjRef _obj;

    _DeckObjElem(DeckObjRef p) { _obj = p; }
    operator DeckObjRef() const { return _obj; }
    DeckObjRef operator ->() { return _obj; }
};

class DeckObjType : public GlyphType {
protected:
    DeckObjType();
    virtual ~DeckObjType();
public:
    _GlyphOffsetExpr card() {
        return _c_card();
    }
    virtual GlyphOffsetRef _c_card();
    virtual void flip_to(GlyphOffsetRef off);

    _DeckObjExpr _ref();
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

inline DeckObjRef DeckObj::_duplicate(DeckObjRef obj) {
    return (DeckObjRef)_BaseObject__duplicate(obj, &DeckObjStub::_create);
}
inline DeckObj& DeckObj::operator =(DeckObjRef p) {
    _BaseObject__release(_obj);
    _obj = DeckObj::_duplicate(p);
    return *this;
}
inline DeckObj::DeckObj(const DeckObj& r) {
    _obj = DeckObj::_duplicate(r._obj);
}
inline DeckObj& DeckObj::operator =(const DeckObj& r) {
    _BaseObject__release(_obj);
    _obj = DeckObj::_duplicate(r._obj);
    return *this;
}
inline DeckObj::DeckObj(const _DeckObjExpr& r) {
    _obj = r._obj;
    ((_DeckObjExpr*)&r)->_obj = 0;
}
inline DeckObj& DeckObj::operator =(const _DeckObjExpr& r) {
    _BaseObject__release(_obj);
    _obj = r._obj;
    ((_DeckObjExpr*)&r)->_obj = 0;
    return *this;
}
inline DeckObj::DeckObj(const _DeckObjElem& e) {
    _obj = DeckObj::_duplicate(e._obj);
}
inline DeckObj& DeckObj::operator =(const _DeckObjElem& e) {
    _BaseObject__release(_obj);
    _obj = DeckObj::_duplicate(e._obj);
    return *this;
}
inline DeckObj::~DeckObj() {
    _BaseObject__release(_obj);
}
inline _DeckObjExpr DeckObj::_narrow(const BaseObject& r) {
    return _narrow(r._obj);
}
inline _DeckObjExpr DeckObj::_duplicate(const DeckObj& r) {
    return _duplicate(r._obj);
}
inline DeckObj::operator Glyph() const {
    return _GlyphExpr((GlyphRef)_BaseObject__duplicate(_obj, &GlyphStub::_create));
}
inline DeckObj::operator FrescoObject() const {
    return _FrescoObjectExpr((FrescoObjectRef)_BaseObject__duplicate((GlyphRef)_obj, &FrescoObjectStub::_create));
}
inline _DeckObjExpr::~_DeckObjExpr() { }
inline _DeckObjExpr DeckObjType::_ref() { return this; }

class ScrollBoxType;
typedef ScrollBoxType* ScrollBoxRef;
class ScrollBox;
class _ScrollBoxExpr;
class _ScrollBoxElem;

class ScrollBox {
public:
    ScrollBoxRef _obj;

    ScrollBox() { _obj = 0; }
    ScrollBox(ScrollBoxRef p) { _obj = p; }
    ScrollBox& operator =(ScrollBoxRef p);
    ScrollBox(const ScrollBox&);
    ScrollBox& operator =(const ScrollBox& r);
    ScrollBox(const _ScrollBoxExpr&);
    ScrollBox& operator =(const _ScrollBoxExpr&);
    ScrollBox(const _ScrollBoxElem&);
    ScrollBox& operator =(const _ScrollBoxElem&);
    ~ScrollBox();

    operator ScrollBoxRef() const { return _obj; }
    ScrollBoxRef operator ->() { return _obj; }

    operator Glyph() const;
    operator FrescoObject() const;
    static ScrollBoxRef _narrow(BaseObjectRef p);
    static _ScrollBoxExpr _narrow(const BaseObject& r);

    static ScrollBoxRef _duplicate(ScrollBoxRef obj);
    static _ScrollBoxExpr _duplicate(const ScrollBox& r);
};

class _ScrollBoxExpr : public ScrollBox {
public:
    _ScrollBoxExpr(ScrollBoxRef p) { _obj = p; }
    _ScrollBoxExpr(const ScrollBox& r) { _obj = r._obj; }
    _ScrollBoxExpr(const _ScrollBoxExpr& r) { _obj = r._obj; }
    ~_ScrollBoxExpr();
};

class _ScrollBoxElem {
public:
    ScrollBoxRef _obj;

    _ScrollBoxElem(ScrollBoxRef p) { _obj = p; }
    operator ScrollBoxRef() const { return _obj; }
    ScrollBoxRef operator ->() { return _obj; }
};

class ScrollBoxType : public GlyphType {
protected:
    ScrollBoxType();
    virtual ~ScrollBoxType();
public:
    virtual Boolean shown(GlyphOffsetRef off);
    _GlyphOffsetExpr first_shown() {
        return _c_first_shown();
    }
    virtual GlyphOffsetRef _c_first_shown();
    _GlyphOffsetExpr last_shown() {
        return _c_last_shown();
    }
    virtual GlyphOffsetRef _c_last_shown();

    _ScrollBoxExpr _ref();
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

inline ScrollBoxRef ScrollBox::_duplicate(ScrollBoxRef obj) {
    return (ScrollBoxRef)_BaseObject__duplicate(obj, &ScrollBoxStub::_create);
}
inline ScrollBox& ScrollBox::operator =(ScrollBoxRef p) {
    _BaseObject__release(_obj);
    _obj = ScrollBox::_duplicate(p);
    return *this;
}
inline ScrollBox::ScrollBox(const ScrollBox& r) {
    _obj = ScrollBox::_duplicate(r._obj);
}
inline ScrollBox& ScrollBox::operator =(const ScrollBox& r) {
    _BaseObject__release(_obj);
    _obj = ScrollBox::_duplicate(r._obj);
    return *this;
}
inline ScrollBox::ScrollBox(const _ScrollBoxExpr& r) {
    _obj = r._obj;
    ((_ScrollBoxExpr*)&r)->_obj = 0;
}
inline ScrollBox& ScrollBox::operator =(const _ScrollBoxExpr& r) {
    _BaseObject__release(_obj);
    _obj = r._obj;
    ((_ScrollBoxExpr*)&r)->_obj = 0;
    return *this;
}
inline ScrollBox::ScrollBox(const _ScrollBoxElem& e) {
    _obj = ScrollBox::_duplicate(e._obj);
}
inline ScrollBox& ScrollBox::operator =(const _ScrollBoxElem& e) {
    _BaseObject__release(_obj);
    _obj = ScrollBox::_duplicate(e._obj);
    return *this;
}
inline ScrollBox::~ScrollBox() {
    _BaseObject__release(_obj);
}
inline _ScrollBoxExpr ScrollBox::_narrow(const BaseObject& r) {
    return _narrow(r._obj);
}
inline _ScrollBoxExpr ScrollBox::_duplicate(const ScrollBox& r) {
    return _duplicate(r._obj);
}
inline ScrollBox::operator Glyph() const {
    return _GlyphExpr((GlyphRef)_BaseObject__duplicate(_obj, &GlyphStub::_create));
}
inline ScrollBox::operator FrescoObject() const {
    return _FrescoObjectExpr((FrescoObjectRef)_BaseObject__duplicate((GlyphRef)_obj, &FrescoObjectStub::_create));
}
inline _ScrollBoxExpr::~_ScrollBoxExpr() { }
inline _ScrollBoxExpr ScrollBoxType::_ref() { return this; }

class LayoutKitType;
typedef LayoutKitType* LayoutKitRef;
class LayoutKit;
class _LayoutKitExpr;
class _LayoutKitElem;

class LayoutKit {
public:
    LayoutKitRef _obj;

    LayoutKit() { _obj = 0; }
    LayoutKit(LayoutKitRef p) { _obj = p; }
    LayoutKit& operator =(LayoutKitRef p);
    LayoutKit(const LayoutKit&);
    LayoutKit& operator =(const LayoutKit& r);
    LayoutKit(const _LayoutKitExpr&);
    LayoutKit& operator =(const _LayoutKitExpr&);
    LayoutKit(const _LayoutKitElem&);
    LayoutKit& operator =(const _LayoutKitElem&);
    ~LayoutKit();

    operator LayoutKitRef() const { return _obj; }
    LayoutKitRef operator ->() { return _obj; }

    operator FrescoObject() const;
    static LayoutKitRef _narrow(BaseObjectRef p);
    static _LayoutKitExpr _narrow(const BaseObject& r);

    static LayoutKitRef _duplicate(LayoutKitRef obj);
    static _LayoutKitExpr _duplicate(const LayoutKit& r);
};

class _LayoutKitExpr : public LayoutKit {
public:
    _LayoutKitExpr(LayoutKitRef p) { _obj = p; }
    _LayoutKitExpr(const LayoutKit& r) { _obj = r._obj; }
    _LayoutKitExpr(const _LayoutKitExpr& r) { _obj = r._obj; }
    ~_LayoutKitExpr();
};

class _LayoutKitElem {
public:
    LayoutKitRef _obj;

    _LayoutKitElem(LayoutKitRef p) { _obj = p; }
    operator LayoutKitRef() const { return _obj; }
    LayoutKitRef operator ->() { return _obj; }
};

class LayoutKitType : public FrescoObjectType {
protected:
    LayoutKitType();
    virtual ~LayoutKitType();
public:
    virtual Coord fil();
    virtual void fil(Coord _p);
    _GlyphExpr hbox() {
        return _c_hbox();
    }
    virtual GlyphRef _c_hbox();
    _GlyphExpr vbox() {
        return _c_vbox();
    }
    virtual GlyphRef _c_vbox();
    _GlyphExpr hbox_first_aligned() {
        return _c_hbox_first_aligned();
    }
    virtual GlyphRef _c_hbox_first_aligned();
    _GlyphExpr vbox_first_aligned() {
        return _c_vbox_first_aligned();
    }
    virtual GlyphRef _c_vbox_first_aligned();
    _ScrollBoxExpr vscrollbox() {
        return _c_vscrollbox();
    }
    virtual ScrollBoxRef _c_vscrollbox();
    _GlyphExpr overlay() {
        return _c_overlay();
    }
    virtual GlyphRef _c_overlay();
    _DeckObjExpr deck() {
        return _c_deck();
    }
    virtual DeckObjRef _c_deck();
    _GlyphExpr back(GlyphRef g, GlyphRef under) {
        return _c_back(g, under);
    }
    virtual GlyphRef _c_back(GlyphRef g, GlyphRef under);
    _GlyphExpr front(GlyphRef g, GlyphRef over) {
        return _c_front(g, over);
    }
    virtual GlyphRef _c_front(GlyphRef g, GlyphRef over);
    _GlyphExpr between(GlyphRef g, GlyphRef under, GlyphRef over) {
        return _c_between(g, under, over);
    }
    virtual GlyphRef _c_between(GlyphRef g, GlyphRef under, GlyphRef over);
    _GlyphExpr glue(Axis a, Coord natural, Coord stretch, Coord shrink, Alignment align) {
        return _c_glue(a, natural, stretch, shrink, align);
    }
    virtual GlyphRef _c_glue(Axis a, Coord natural, Coord stretch, Coord shrink, Alignment align);
    _GlyphExpr glue_requisition(const Glyph::Requisition& r) {
        return _c_glue_requisition(r);
    }
    virtual GlyphRef _c_glue_requisition(const Glyph::Requisition& r);
    _GlyphExpr hfil() {
        return _c_hfil();
    }
    virtual GlyphRef _c_hfil();
    _GlyphExpr hglue_fil(Coord natural) {
        return _c_hglue_fil(natural);
    }
    virtual GlyphRef _c_hglue_fil(Coord natural);
    _GlyphExpr hglue(Coord natural, Coord stretch, Coord shrink) {
        return _c_hglue(natural, stretch, shrink);
    }
    virtual GlyphRef _c_hglue(Coord natural, Coord stretch, Coord shrink);
    _GlyphExpr hglue_aligned(Coord natural, Coord stretch, Coord shrink, Alignment a) {
        return _c_hglue_aligned(natural, stretch, shrink, a);
    }
    virtual GlyphRef _c_hglue_aligned(Coord natural, Coord stretch, Coord shrink, Alignment a);
    _GlyphExpr hspace(Coord natural) {
        return _c_hspace(natural);
    }
    virtual GlyphRef _c_hspace(Coord natural);
    _GlyphExpr vfil() {
        return _c_vfil();
    }
    virtual GlyphRef _c_vfil();
    _GlyphExpr vglue_fil(Coord natural) {
        return _c_vglue_fil(natural);
    }
    virtual GlyphRef _c_vglue_fil(Coord natural);
    _GlyphExpr vglue(Coord natural, Coord stretch, Coord shrink) {
        return _c_vglue(natural, stretch, shrink);
    }
    virtual GlyphRef _c_vglue(Coord natural, Coord stretch, Coord shrink);
    _GlyphExpr vglue_aligned(Coord natural, Coord stretch, Coord shrink, Alignment a) {
        return _c_vglue_aligned(natural, stretch, shrink, a);
    }
    virtual GlyphRef _c_vglue_aligned(Coord natural, Coord stretch, Coord shrink, Alignment a);
    _GlyphExpr vspace(Coord natural) {
        return _c_vspace(natural);
    }
    virtual GlyphRef _c_vspace(Coord natural);
    _GlyphExpr shape_of(GlyphRef g) {
        return _c_shape_of(g);
    }
    virtual GlyphRef _c_shape_of(GlyphRef g);
    _GlyphExpr shape_of_xy(GlyphRef gx, GlyphRef gy) {
        return _c_shape_of_xy(gx, gy);
    }
    virtual GlyphRef _c_shape_of_xy(GlyphRef gx, GlyphRef gy);
    _GlyphExpr shape_of_xyz(GlyphRef gx, GlyphRef gy, GlyphRef gz) {
        return _c_shape_of_xyz(gx, gy, gz);
    }
    virtual GlyphRef _c_shape_of_xyz(GlyphRef gx, GlyphRef gy, GlyphRef gz);
    _GlyphExpr strut(FontRef f, Coord natural, Coord stretch, Coord shrink) {
        return _c_strut(f, natural, stretch, shrink);
    }
    virtual GlyphRef _c_strut(FontRef f, Coord natural, Coord stretch, Coord shrink);
    _GlyphExpr hstrut(Coord right_bearing, Coord left_bearing, Coord natural, Coord stretch, Coord shrink) {
        return _c_hstrut(right_bearing, left_bearing, natural, stretch, shrink);
    }
    virtual GlyphRef _c_hstrut(Coord right_bearing, Coord left_bearing, Coord natural, Coord stretch, Coord shrink);
    _GlyphExpr vstrut(Coord ascent, Coord descent, Coord natural, Coord stretch, Coord shrink) {
        return _c_vstrut(ascent, descent, natural, stretch, shrink);
    }
    virtual GlyphRef _c_vstrut(Coord ascent, Coord descent, Coord natural, Coord stretch, Coord shrink);
    _GlyphExpr spaces(Long count, Coord each, FontRef f, ColorRef c) {
        return _c_spaces(count, each, f, c);
    }
    virtual GlyphRef _c_spaces(Long count, Coord each, FontRef f, ColorRef c);
    _GlyphExpr center(GlyphRef g) {
        return _c_center(g);
    }
    virtual GlyphRef _c_center(GlyphRef g);
    _GlyphExpr center_aligned(GlyphRef g, Alignment x, Alignment y) {
        return _c_center_aligned(g, x, y);
    }
    virtual GlyphRef _c_center_aligned(GlyphRef g, Alignment x, Alignment y);
    _GlyphExpr center_axis(GlyphRef g, Axis a, Alignment align) {
        return _c_center_axis(g, a, align);
    }
    virtual GlyphRef _c_center_axis(GlyphRef g, Axis a, Alignment align);
    _GlyphExpr hcenter(GlyphRef g) {
        return _c_hcenter(g);
    }
    virtual GlyphRef _c_hcenter(GlyphRef g);
    _GlyphExpr hcenter_aligned(GlyphRef g, Alignment x) {
        return _c_hcenter_aligned(g, x);
    }
    virtual GlyphRef _c_hcenter_aligned(GlyphRef g, Alignment x);
    _GlyphExpr vcenter(GlyphRef g) {
        return _c_vcenter(g);
    }
    virtual GlyphRef _c_vcenter(GlyphRef g);
    _GlyphExpr vcenter_aligned(GlyphRef g, Alignment y) {
        return _c_vcenter_aligned(g, y);
    }
    virtual GlyphRef _c_vcenter_aligned(GlyphRef g, Alignment y);
    _GlyphExpr fixed(GlyphRef g, Coord x, Coord y) {
        return _c_fixed(g, x, y);
    }
    virtual GlyphRef _c_fixed(GlyphRef g, Coord x, Coord y);
    _GlyphExpr fixed_axis(GlyphRef g, Axis a, Coord size) {
        return _c_fixed_axis(g, a, size);
    }
    virtual GlyphRef _c_fixed_axis(GlyphRef g, Axis a, Coord size);
    _GlyphExpr hfixed(GlyphRef g, Coord x) {
        return _c_hfixed(g, x);
    }
    virtual GlyphRef _c_hfixed(GlyphRef g, Coord x);
    _GlyphExpr vfixed(GlyphRef g, Coord y) {
        return _c_vfixed(g, y);
    }
    virtual GlyphRef _c_vfixed(GlyphRef g, Coord y);
    _GlyphExpr flexible(GlyphRef g, Coord stretch, Coord shrink) {
        return _c_flexible(g, stretch, shrink);
    }
    virtual GlyphRef _c_flexible(GlyphRef g, Coord stretch, Coord shrink);
    _GlyphExpr flexible_fil(GlyphRef g) {
        return _c_flexible_fil(g);
    }
    virtual GlyphRef _c_flexible_fil(GlyphRef g);
    _GlyphExpr flexible_axis(GlyphRef g, Axis a, Coord stretch, Coord shrink) {
        return _c_flexible_axis(g, a, stretch, shrink);
    }
    virtual GlyphRef _c_flexible_axis(GlyphRef g, Axis a, Coord stretch, Coord shrink);
    _GlyphExpr hflexible(GlyphRef g, Coord stretch, Coord shrink) {
        return _c_hflexible(g, stretch, shrink);
    }
    virtual GlyphRef _c_hflexible(GlyphRef g, Coord stretch, Coord shrink);
    _GlyphExpr vflexible(GlyphRef g, Coord stretch, Coord shrink) {
        return _c_vflexible(g, stretch, shrink);
    }
    virtual GlyphRef _c_vflexible(GlyphRef g, Coord stretch, Coord shrink);
    _GlyphExpr natural(GlyphRef g, Coord x, Coord y) {
        return _c_natural(g, x, y);
    }
    virtual GlyphRef _c_natural(GlyphRef g, Coord x, Coord y);
    _GlyphExpr natural_axis(GlyphRef g, Axis a, Coord size) {
        return _c_natural_axis(g, a, size);
    }
    virtual GlyphRef _c_natural_axis(GlyphRef g, Axis a, Coord size);
    _GlyphExpr hnatural(GlyphRef g, Coord x) {
        return _c_hnatural(g, x);
    }
    virtual GlyphRef _c_hnatural(GlyphRef g, Coord x);
    _GlyphExpr vnatural(GlyphRef g, Coord y) {
        return _c_vnatural(g, y);
    }
    virtual GlyphRef _c_vnatural(GlyphRef g, Coord y);
    _GlyphExpr margin(GlyphRef g, Coord all) {
        return _c_margin(g, all);
    }
    virtual GlyphRef _c_margin(GlyphRef g, Coord all);
    _GlyphExpr margin_lrbt(GlyphRef g, Coord lmargin, Coord rmargin, Coord bmargin, Coord tmargin) {
        return _c_margin_lrbt(g, lmargin, rmargin, bmargin, tmargin);
    }
    virtual GlyphRef _c_margin_lrbt(GlyphRef g, Coord lmargin, Coord rmargin, Coord bmargin, Coord tmargin);
    _GlyphExpr margin_lrbt_flexible(GlyphRef g, Coord lmargin, Coord lstretch, Coord lshrink, Coord rmargin, Coord rstretch, Coord rshrink, Coord bmargin, Coord bstretch, Coord bshrink, Coord tmargin, Coord tstretch, Coord tshrink) {
        return _c_margin_lrbt_flexible(g, lmargin, lstretch, lshrink, rmargin, rstretch, rshrink, bmargin, bstretch, bshrink, tmargin, tstretch, tshrink);
    }
    virtual GlyphRef _c_margin_lrbt_flexible(GlyphRef g, Coord lmargin, Coord lstretch, Coord lshrink, Coord rmargin, Coord rstretch, Coord rshrink, Coord bmargin, Coord bstretch, Coord bshrink, Coord tmargin, Coord tstretch, Coord tshrink);
    _GlyphExpr hmargin(GlyphRef g, Coord both) {
        return _c_hmargin(g, both);
    }
    virtual GlyphRef _c_hmargin(GlyphRef g, Coord both);
    _GlyphExpr hmargin_lr(GlyphRef g, Coord lmargin, Coord rmargin) {
        return _c_hmargin_lr(g, lmargin, rmargin);
    }
    virtual GlyphRef _c_hmargin_lr(GlyphRef g, Coord lmargin, Coord rmargin);
    _GlyphExpr hmargin_lr_flexible(GlyphRef g, Coord lmargin, Coord lstretch, Coord lshrink, Coord rmargin, Coord rstretch, Coord rshrink) {
        return _c_hmargin_lr_flexible(g, lmargin, lstretch, lshrink, rmargin, rstretch, rshrink);
    }
    virtual GlyphRef _c_hmargin_lr_flexible(GlyphRef g, Coord lmargin, Coord lstretch, Coord lshrink, Coord rmargin, Coord rstretch, Coord rshrink);
    _GlyphExpr vmargin(GlyphRef g, Coord both) {
        return _c_vmargin(g, both);
    }
    virtual GlyphRef _c_vmargin(GlyphRef g, Coord both);
    _GlyphExpr vmargin_bt(GlyphRef g, Coord bmargin, Coord tmargin) {
        return _c_vmargin_bt(g, bmargin, tmargin);
    }
    virtual GlyphRef _c_vmargin_bt(GlyphRef g, Coord bmargin, Coord tmargin);
    _GlyphExpr vmargin_bt_flexible(GlyphRef g, Coord bmargin, Coord bstretch, Coord bshrink, Coord tmargin, Coord tstretch, Coord tshrink) {
        return _c_vmargin_bt_flexible(g, bmargin, bstretch, bshrink, tmargin, tstretch, tshrink);
    }
    virtual GlyphRef _c_vmargin_bt_flexible(GlyphRef g, Coord bmargin, Coord bstretch, Coord bshrink, Coord tmargin, Coord tstretch, Coord tshrink);
    _GlyphExpr lmargin(GlyphRef g, Coord natural) {
        return _c_lmargin(g, natural);
    }
    virtual GlyphRef _c_lmargin(GlyphRef g, Coord natural);
    _GlyphExpr lmargin_flexible(GlyphRef g, Coord natural, Coord stretch, Coord shrink) {
        return _c_lmargin_flexible(g, natural, stretch, shrink);
    }
    virtual GlyphRef _c_lmargin_flexible(GlyphRef g, Coord natural, Coord stretch, Coord shrink);
    _GlyphExpr rmargin(GlyphRef g, Coord natural) {
        return _c_rmargin(g, natural);
    }
    virtual GlyphRef _c_rmargin(GlyphRef g, Coord natural);
    _GlyphExpr rmargin_flexible(GlyphRef g, Coord natural, Coord stretch, Coord shrink) {
        return _c_rmargin_flexible(g, natural, stretch, shrink);
    }
    virtual GlyphRef _c_rmargin_flexible(GlyphRef g, Coord natural, Coord stretch, Coord shrink);
    _GlyphExpr bmargin(GlyphRef g, Coord natural) {
        return _c_bmargin(g, natural);
    }
    virtual GlyphRef _c_bmargin(GlyphRef g, Coord natural);
    _GlyphExpr bmargin_flexible(GlyphRef g, Coord natural, Coord stretch, Coord shrink) {
        return _c_bmargin_flexible(g, natural, stretch, shrink);
    }
    virtual GlyphRef _c_bmargin_flexible(GlyphRef g, Coord natural, Coord stretch, Coord shrink);
    _GlyphExpr tmargin(GlyphRef g, Coord natural) {
        return _c_tmargin(g, natural);
    }
    virtual GlyphRef _c_tmargin(GlyphRef g, Coord natural);
    _GlyphExpr tmargin_flexible(GlyphRef g, Coord natural, Coord stretch, Coord shrink) {
        return _c_tmargin_flexible(g, natural, stretch, shrink);
    }
    virtual GlyphRef _c_tmargin_flexible(GlyphRef g, Coord natural, Coord stretch, Coord shrink);

    _LayoutKitExpr _ref();
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

inline LayoutKitRef LayoutKit::_duplicate(LayoutKitRef obj) {
    return (LayoutKitRef)_BaseObject__duplicate(obj, &LayoutKitStub::_create);
}
inline LayoutKit& LayoutKit::operator =(LayoutKitRef p) {
    _BaseObject__release(_obj);
    _obj = LayoutKit::_duplicate(p);
    return *this;
}
inline LayoutKit::LayoutKit(const LayoutKit& r) {
    _obj = LayoutKit::_duplicate(r._obj);
}
inline LayoutKit& LayoutKit::operator =(const LayoutKit& r) {
    _BaseObject__release(_obj);
    _obj = LayoutKit::_duplicate(r._obj);
    return *this;
}
inline LayoutKit::LayoutKit(const _LayoutKitExpr& r) {
    _obj = r._obj;
    ((_LayoutKitExpr*)&r)->_obj = 0;
}
inline LayoutKit& LayoutKit::operator =(const _LayoutKitExpr& r) {
    _BaseObject__release(_obj);
    _obj = r._obj;
    ((_LayoutKitExpr*)&r)->_obj = 0;
    return *this;
}
inline LayoutKit::LayoutKit(const _LayoutKitElem& e) {
    _obj = LayoutKit::_duplicate(e._obj);
}
inline LayoutKit& LayoutKit::operator =(const _LayoutKitElem& e) {
    _BaseObject__release(_obj);
    _obj = LayoutKit::_duplicate(e._obj);
    return *this;
}
inline LayoutKit::~LayoutKit() {
    _BaseObject__release(_obj);
}
inline _LayoutKitExpr LayoutKit::_narrow(const BaseObject& r) {
    return _narrow(r._obj);
}
inline _LayoutKitExpr LayoutKit::_duplicate(const LayoutKit& r) {
    return _duplicate(r._obj);
}
inline LayoutKit::operator FrescoObject() const {
    return _FrescoObjectExpr((FrescoObjectRef)_BaseObject__duplicate(_obj, &FrescoObjectStub::_create));
}
inline _LayoutKitExpr::~_LayoutKitExpr() { }
inline _LayoutKitExpr LayoutKitType::_ref() { return this; }

#endif
