/* DO NOT EDIT -- Automatically generated from Interfaces/figures.idl */

#ifndef Interfaces_figures_h
#define Interfaces_figures_h

#include <X11/Fresco/_enter.h>
#include <X11/Fresco/drawing.h>
#include <X11/Fresco/glyph.h>

class FigureStyleType;
typedef FigureStyleType* FigureStyleRef;
typedef FigureStyleRef FigureStyle_in;
class FigureStyle;
class FigureStyle_tmp;
class FigureStyle_var;

class FigureStyle {
public:
    FigureStyleRef _obj_;

    FigureStyle() { _obj_ = 0; }
    FigureStyle(FigureStyleRef p) { _obj_ = p; }
    FigureStyle& operator =(FigureStyleRef p);
    FigureStyle(const FigureStyle&);
    FigureStyle& operator =(const FigureStyle& r);
    FigureStyle(const FigureStyle_tmp&);
    FigureStyle& operator =(const FigureStyle_tmp&);
    FigureStyle(const FigureStyle_var&);
    FigureStyle& operator =(const FigureStyle_var&);
    ~FigureStyle();

    FigureStyleRef operator ->() { return _obj_; }

    operator FigureStyle_in() const { return _obj_; }
    operator StyleObj() const;
    operator FrescoObject() const;
    static FigureStyleRef _narrow(BaseObjectRef p);
    static FigureStyle_tmp _narrow(const BaseObject& r);

    static FigureStyleRef _duplicate(FigureStyleRef obj);
    static FigureStyle_tmp _duplicate(const FigureStyle& r);
};

class FigureStyle_tmp : public FigureStyle {
public:
    FigureStyle_tmp(FigureStyleRef p) { _obj_ = p; }
    FigureStyle_tmp(const FigureStyle& r);
    FigureStyle_tmp(const FigureStyle_tmp& r);
    ~FigureStyle_tmp();
};

class FigureStyle_var {
public:
    FigureStyleRef _obj_;

    FigureStyle_var(FigureStyleRef p) { _obj_ = p; }
    operator FigureStyleRef() const { return _obj_; }
    FigureStyleRef operator ->() { return _obj_; }
};

class FigureStyleType : public StyleObjType {
protected:
    FigureStyleType();
    virtual ~FigureStyleType();
public:
    Color_tmp background() {
        return _c_background();
    }
    virtual ColorRef _c_background();
    void background(Color_in _p) {
        _c_background(_p);
    }
    virtual void _c_background(Color_in _p);
    Brush_tmp brush_attr() {
        return _c_brush_attr();
    }
    virtual BrushRef _c_brush_attr();
    void brush_attr(Brush_in _p) {
        _c_brush_attr(_p);
    }
    virtual void _c_brush_attr(Brush_in _p);
    Font_tmp font_attr() {
        return _c_font_attr();
    }
    virtual FontRef _c_font_attr();
    void font_attr(Font_in _p) {
        _c_font_attr(_p);
    }
    virtual void _c_font_attr(Font_in _p);
    Color_tmp foreground() {
        return _c_foreground();
    }
    virtual ColorRef _c_foreground();
    void foreground(Color_in _p) {
        _c_foreground(_p);
    }
    virtual void _c_foreground(Color_in _p);
    FigureStyleRef _obj() { return this; }
    void* _this();
    virtual TypeObjId _tid();
};

class FigureStyleStub : public FigureStyleType {
public:
    FigureStyleStub(Exchange*);
    ~FigureStyleStub();

    static BaseObjectRef _create(Exchange*);
    Exchange* _exchange();
protected:
    Exchange* exch_;
};

class FigureKitType;
typedef FigureKitType* FigureKitRef;
typedef FigureKitRef FigureKit_in;
class FigureKit;
class FigureKit_tmp;
class FigureKit_var;

class FigureKit {
public:
    FigureKitRef _obj_;

    FigureKit() { _obj_ = 0; }
    FigureKit(FigureKitRef p) { _obj_ = p; }
    FigureKit& operator =(FigureKitRef p);
    FigureKit(const FigureKit&);
    FigureKit& operator =(const FigureKit& r);
    FigureKit(const FigureKit_tmp&);
    FigureKit& operator =(const FigureKit_tmp&);
    FigureKit(const FigureKit_var&);
    FigureKit& operator =(const FigureKit_var&);
    ~FigureKit();

    FigureKitRef operator ->() { return _obj_; }

    operator FigureKit_in() const { return _obj_; }
    operator FrescoObject() const;
    static FigureKitRef _narrow(BaseObjectRef p);
    static FigureKit_tmp _narrow(const BaseObject& r);

    static FigureKitRef _duplicate(FigureKitRef obj);
    static FigureKit_tmp _duplicate(const FigureKit& r);
    class Vertices {
    public:
        long _maximum, _length; Vertex* _buffer;

        Vertices() { _maximum = _length = 0; _buffer = 0; }
        Vertices(long m, long n, Vertex* e) {
            _maximum = m; _length = n; _buffer = e;
        }
        Vertices(const Vertices& _s) { _buffer = 0; *this = _s; }
        Vertices& operator =(const Vertices&);
        ~Vertices() { delete [] _buffer; }
    };
    enum Mode {
        fill, stroke, fill_stroke
    };
};

class FigureKit_tmp : public FigureKit {
public:
    FigureKit_tmp(FigureKitRef p) { _obj_ = p; }
    FigureKit_tmp(const FigureKit& r);
    FigureKit_tmp(const FigureKit_tmp& r);
    ~FigureKit_tmp();
};

class FigureKit_var {
public:
    FigureKitRef _obj_;

    FigureKit_var(FigureKitRef p) { _obj_ = p; }
    operator FigureKitRef() const { return _obj_; }
    FigureKitRef operator ->() { return _obj_; }
};

class FigureKitType : public FrescoObjectType {
protected:
    FigureKitType();
    virtual ~FigureKitType();
public:
    FigureStyle_tmp default_style() {
        return _c_default_style();
    }
    virtual FigureStyleRef _c_default_style();
    FigureStyle_tmp new_style(StyleObj_in parent) {
        return _c_new_style(parent);
    }
    virtual FigureStyleRef _c_new_style(StyleObj_in parent);
    Glyph_tmp figure_root(Glyph_in child) {
        return _c_figure_root(child);
    }
    virtual GlyphRef _c_figure_root(Glyph_in child);
    Glyph_tmp label(FigureStyle_in s, CharString_in str) {
        return _c_label(s, str);
    }
    virtual GlyphRef _c_label(FigureStyle_in s, CharString_in str);
    Glyph_tmp point(FigureStyle_in s, Coord x, Coord y) {
        return _c_point(s, x, y);
    }
    virtual GlyphRef _c_point(FigureStyle_in s, Coord x, Coord y);
    Glyph_tmp line(FigureStyle_in s, Coord x0, Coord y0, Coord x1, Coord y1) {
        return _c_line(s, x0, y0, x1, y1);
    }
    virtual GlyphRef _c_line(FigureStyle_in s, Coord x0, Coord y0, Coord x1, Coord y1);
    Glyph_tmp rectangle(FigureKit::Mode m, FigureStyle_in s, Coord left, Coord bottom, Coord right, Coord top) {
        return _c_rectangle(m, s, left, bottom, right, top);
    }
    virtual GlyphRef _c_rectangle(FigureKit::Mode m, FigureStyle_in s, Coord left, Coord bottom, Coord right, Coord top);
    Glyph_tmp circle(FigureKit::Mode m, FigureStyle_in s, Coord x, Coord y, Coord r) {
        return _c_circle(m, s, x, y, r);
    }
    virtual GlyphRef _c_circle(FigureKit::Mode m, FigureStyle_in s, Coord x, Coord y, Coord r);
    Glyph_tmp ellipse(FigureKit::Mode m, FigureStyle_in s, Coord x, Coord y, Coord r1, Coord r2) {
        return _c_ellipse(m, s, x, y, r1, r2);
    }
    virtual GlyphRef _c_ellipse(FigureKit::Mode m, FigureStyle_in s, Coord x, Coord y, Coord r1, Coord r2);
    Glyph_tmp open_bspline(FigureKit::Mode m, FigureStyle_in s, const FigureKit::Vertices& v) {
        return _c_open_bspline(m, s, v);
    }
    virtual GlyphRef _c_open_bspline(FigureKit::Mode m, FigureStyle_in s, const FigureKit::Vertices& v);
    Glyph_tmp closed_bspline(FigureKit::Mode m, FigureStyle_in s, const FigureKit::Vertices& v) {
        return _c_closed_bspline(m, s, v);
    }
    virtual GlyphRef _c_closed_bspline(FigureKit::Mode m, FigureStyle_in s, const FigureKit::Vertices& v);
    Glyph_tmp multiline(FigureKit::Mode m, FigureStyle_in s, const FigureKit::Vertices& v) {
        return _c_multiline(m, s, v);
    }
    virtual GlyphRef _c_multiline(FigureKit::Mode m, FigureStyle_in s, const FigureKit::Vertices& v);
    Glyph_tmp polygon(FigureKit::Mode m, FigureStyle_in s, const FigureKit::Vertices& v) {
        return _c_polygon(m, s, v);
    }
    virtual GlyphRef _c_polygon(FigureKit::Mode m, FigureStyle_in s, const FigureKit::Vertices& v);
    Glyph_tmp fitter(Glyph_in g) {
        return _c_fitter(g);
    }
    virtual GlyphRef _c_fitter(Glyph_in g);
    Glyph_tmp group() {
        return _c_group();
    }
    virtual GlyphRef _c_group();
    FigureKitRef _obj() { return this; }
    void* _this();
    virtual TypeObjId _tid();
};

class FigureKitStub : public FigureKitType {
public:
    FigureKitStub(Exchange*);
    ~FigureKitStub();

    static BaseObjectRef _create(Exchange*);
    Exchange* _exchange();
protected:
    Exchange* exch_;
};

inline FigureStyleRef FigureStyle::_duplicate(FigureStyleRef obj) {
    return (FigureStyleRef)_BaseObject__duplicate(obj, &FigureStyleStub::_create);
}
inline FigureStyle& FigureStyle::operator =(FigureStyleRef p) {
    _BaseObject__release(_obj_);
    _obj_ = FigureStyle::_duplicate(p);
    return *this;
}
inline FigureStyle::FigureStyle(const FigureStyle& r) {
    _obj_ = FigureStyle::_duplicate(r._obj_);
}
inline FigureStyle& FigureStyle::operator =(const FigureStyle& r) {
    _BaseObject__release(_obj_);
    _obj_ = FigureStyle::_duplicate(r._obj_);
    return *this;
}
inline FigureStyle::FigureStyle(const FigureStyle_tmp& r) {
    _obj_ = r._obj_;
    ((FigureStyle_tmp*)&r)->_obj_ = 0;
}
inline FigureStyle& FigureStyle::operator =(const FigureStyle_tmp& r) {
    _BaseObject__release(_obj_);
    _obj_ = r._obj_;
    ((FigureStyle_tmp*)&r)->_obj_ = 0;
    return *this;
}
inline FigureStyle::FigureStyle(const FigureStyle_var& e) {
    _obj_ = FigureStyle::_duplicate(e._obj_);
}
inline FigureStyle& FigureStyle::operator =(const FigureStyle_var& e) {
    _BaseObject__release(_obj_);
    _obj_ = FigureStyle::_duplicate(e._obj_);
    return *this;
}
inline FigureStyle::~FigureStyle() {
    _BaseObject__release(_obj_);
}
inline FigureStyle_tmp FigureStyle::_narrow(const BaseObject& r) {
    return _narrow(r._obj_);
}
inline FigureStyle_tmp FigureStyle::_duplicate(const FigureStyle& r) {
    return _duplicate(r._obj_);
}
inline FigureStyle::operator StyleObj() const {
    return StyleObj_tmp((StyleObjRef)_BaseObject__duplicate((StyleObjRef)_obj_, &StyleObjStub::_create));
}
inline FigureStyle::operator FrescoObject() const {
    return FrescoObject_tmp((FrescoObjectRef)_BaseObject__duplicate((StyleObjRef)(FrescoObjectRef)_obj_, &FrescoObjectStub::_create));
}
inline FigureStyle_tmp::FigureStyle_tmp(const FigureStyle& r) {
    _obj_ = FigureStyle::_duplicate(r._obj_);
}
inline FigureStyle_tmp::FigureStyle_tmp(const FigureStyle_tmp& r) {
    _obj_ = r._obj_;
    ((FigureStyle_tmp*)&r)->_obj_ = 0;
}
inline FigureStyle_tmp::~FigureStyle_tmp() { }

inline FigureKitRef FigureKit::_duplicate(FigureKitRef obj) {
    return (FigureKitRef)_BaseObject__duplicate(obj, &FigureKitStub::_create);
}
inline FigureKit& FigureKit::operator =(FigureKitRef p) {
    _BaseObject__release(_obj_);
    _obj_ = FigureKit::_duplicate(p);
    return *this;
}
inline FigureKit::FigureKit(const FigureKit& r) {
    _obj_ = FigureKit::_duplicate(r._obj_);
}
inline FigureKit& FigureKit::operator =(const FigureKit& r) {
    _BaseObject__release(_obj_);
    _obj_ = FigureKit::_duplicate(r._obj_);
    return *this;
}
inline FigureKit::FigureKit(const FigureKit_tmp& r) {
    _obj_ = r._obj_;
    ((FigureKit_tmp*)&r)->_obj_ = 0;
}
inline FigureKit& FigureKit::operator =(const FigureKit_tmp& r) {
    _BaseObject__release(_obj_);
    _obj_ = r._obj_;
    ((FigureKit_tmp*)&r)->_obj_ = 0;
    return *this;
}
inline FigureKit::FigureKit(const FigureKit_var& e) {
    _obj_ = FigureKit::_duplicate(e._obj_);
}
inline FigureKit& FigureKit::operator =(const FigureKit_var& e) {
    _BaseObject__release(_obj_);
    _obj_ = FigureKit::_duplicate(e._obj_);
    return *this;
}
inline FigureKit::~FigureKit() {
    _BaseObject__release(_obj_);
}
inline FigureKit_tmp FigureKit::_narrow(const BaseObject& r) {
    return _narrow(r._obj_);
}
inline FigureKit_tmp FigureKit::_duplicate(const FigureKit& r) {
    return _duplicate(r._obj_);
}
inline FigureKit::operator FrescoObject() const {
    return FrescoObject_tmp((FrescoObjectRef)_BaseObject__duplicate((FrescoObjectRef)_obj_, &FrescoObjectStub::_create));
}
inline FigureKit_tmp::FigureKit_tmp(const FigureKit& r) {
    _obj_ = FigureKit::_duplicate(r._obj_);
}
inline FigureKit_tmp::FigureKit_tmp(const FigureKit_tmp& r) {
    _obj_ = r._obj_;
    ((FigureKit_tmp*)&r)->_obj_ = 0;
}
inline FigureKit_tmp::~FigureKit_tmp() { }

#endif
