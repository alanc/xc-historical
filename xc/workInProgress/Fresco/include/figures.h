/* DO NOT EDIT -- Automatically generated from Interfaces/figures.idl */

#ifndef Interfaces_figures_h
#define Interfaces_figures_h

#include <X11/Fresco/_enter.h>
#include <X11/Fresco/drawing.h>
#include <X11/Fresco/glyph.h>

class FigureStyleType;
typedef FigureStyleType* FigureStyleRef;
class FigureStyle;
class _FigureStyleExpr;
class _FigureStyleElem;

class FigureStyle {
public:
    FigureStyleRef _obj;

    FigureStyle() { _obj = 0; }
    FigureStyle(FigureStyleRef p) { _obj = p; }
    FigureStyle& operator =(FigureStyleRef p);
    FigureStyle(const FigureStyle&);
    FigureStyle& operator =(const FigureStyle& r);
    FigureStyle(const _FigureStyleExpr&);
    FigureStyle& operator =(const _FigureStyleExpr&);
    FigureStyle(const _FigureStyleElem&);
    FigureStyle& operator =(const _FigureStyleElem&);
    ~FigureStyle();

    operator FigureStyleRef() const { return _obj; }
    FigureStyleRef operator ->() { return _obj; }

    operator StyleObj() const;
    operator FrescoObject() const;
    static FigureStyleRef _narrow(BaseObjectRef p);
    static _FigureStyleExpr _narrow(const BaseObject& r);

    static FigureStyleRef _duplicate(FigureStyleRef obj);
    static _FigureStyleExpr _duplicate(const FigureStyle& r);
};

class _FigureStyleExpr : public FigureStyle {
public:
    _FigureStyleExpr(FigureStyleRef p) { _obj = p; }
    _FigureStyleExpr(const FigureStyle& r) { _obj = r._obj; }
    _FigureStyleExpr(const _FigureStyleExpr& r) { _obj = r._obj; }
    ~_FigureStyleExpr();
};

class _FigureStyleElem {
public:
    FigureStyleRef _obj;

    _FigureStyleElem(FigureStyleRef p) { _obj = p; }
    operator FigureStyleRef() const { return _obj; }
    FigureStyleRef operator ->() { return _obj; }
};

class FigureStyleType : public StyleObjType {
protected:
    FigureStyleType();
    virtual ~FigureStyleType();
public:
    _ColorExpr background() {
        return _c_background();
    }
    virtual ColorRef _c_background();
    void background(ColorRef _p) {
        _c_background(_p);
    }
    virtual void _c_background(ColorRef _p);
    _BrushExpr brush_attr() {
        return _c_brush_attr();
    }
    virtual BrushRef _c_brush_attr();
    void brush_attr(BrushRef _p) {
        _c_brush_attr(_p);
    }
    virtual void _c_brush_attr(BrushRef _p);
    _FontExpr font_attr() {
        return _c_font_attr();
    }
    virtual FontRef _c_font_attr();
    void font_attr(FontRef _p) {
        _c_font_attr(_p);
    }
    virtual void _c_font_attr(FontRef _p);
    _ColorExpr foreground() {
        return _c_foreground();
    }
    virtual ColorRef _c_foreground();
    void foreground(ColorRef _p) {
        _c_foreground(_p);
    }
    virtual void _c_foreground(ColorRef _p);

    _FigureStyleExpr _ref();
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

inline FigureStyleRef FigureStyle::_duplicate(FigureStyleRef obj) {
    return (FigureStyleRef)_BaseObject__duplicate(obj, &FigureStyleStub::_create);
}
inline FigureStyle& FigureStyle::operator =(FigureStyleRef p) {
    _BaseObject__release(_obj);
    _obj = FigureStyle::_duplicate(p);
    return *this;
}
inline FigureStyle::FigureStyle(const FigureStyle& r) {
    _obj = FigureStyle::_duplicate(r._obj);
}
inline FigureStyle& FigureStyle::operator =(const FigureStyle& r) {
    _BaseObject__release(_obj);
    _obj = FigureStyle::_duplicate(r._obj);
    return *this;
}
inline FigureStyle::FigureStyle(const _FigureStyleExpr& r) {
    _obj = r._obj;
    ((_FigureStyleExpr*)&r)->_obj = 0;
}
inline FigureStyle& FigureStyle::operator =(const _FigureStyleExpr& r) {
    _BaseObject__release(_obj);
    _obj = r._obj;
    ((_FigureStyleExpr*)&r)->_obj = 0;
    return *this;
}
inline FigureStyle::FigureStyle(const _FigureStyleElem& e) {
    _obj = FigureStyle::_duplicate(e._obj);
}
inline FigureStyle& FigureStyle::operator =(const _FigureStyleElem& e) {
    _BaseObject__release(_obj);
    _obj = FigureStyle::_duplicate(e._obj);
    return *this;
}
inline FigureStyle::~FigureStyle() {
    _BaseObject__release(_obj);
}
inline _FigureStyleExpr FigureStyle::_narrow(const BaseObject& r) {
    return _narrow(r._obj);
}
inline _FigureStyleExpr FigureStyle::_duplicate(const FigureStyle& r) {
    return _duplicate(r._obj);
}
inline FigureStyle::operator StyleObj() const {
    return _StyleObjExpr((StyleObjRef)_BaseObject__duplicate(_obj, &StyleObjStub::_create));
}
inline FigureStyle::operator FrescoObject() const {
    return _FrescoObjectExpr((FrescoObjectRef)_BaseObject__duplicate((StyleObjRef)_obj, &FrescoObjectStub::_create));
}
inline _FigureStyleExpr::~_FigureStyleExpr() { }
inline _FigureStyleExpr FigureStyleType::_ref() { return this; }

class FigureKitType;
typedef FigureKitType* FigureKitRef;
class FigureKit;
class _FigureKitExpr;
class _FigureKitElem;

class FigureKit {
public:
    FigureKitRef _obj;

    FigureKit() { _obj = 0; }
    FigureKit(FigureKitRef p) { _obj = p; }
    FigureKit& operator =(FigureKitRef p);
    FigureKit(const FigureKit&);
    FigureKit& operator =(const FigureKit& r);
    FigureKit(const _FigureKitExpr&);
    FigureKit& operator =(const _FigureKitExpr&);
    FigureKit(const _FigureKitElem&);
    FigureKit& operator =(const _FigureKitElem&);
    ~FigureKit();

    operator FigureKitRef() const { return _obj; }
    FigureKitRef operator ->() { return _obj; }

    operator FrescoObject() const;
    static FigureKitRef _narrow(BaseObjectRef p);
    static _FigureKitExpr _narrow(const BaseObject& r);

    static FigureKitRef _duplicate(FigureKitRef obj);
    static _FigureKitExpr _duplicate(const FigureKit& r);
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

class _FigureKitExpr : public FigureKit {
public:
    _FigureKitExpr(FigureKitRef p) { _obj = p; }
    _FigureKitExpr(const FigureKit& r) { _obj = r._obj; }
    _FigureKitExpr(const _FigureKitExpr& r) { _obj = r._obj; }
    ~_FigureKitExpr();
};

class _FigureKitElem {
public:
    FigureKitRef _obj;

    _FigureKitElem(FigureKitRef p) { _obj = p; }
    operator FigureKitRef() const { return _obj; }
    FigureKitRef operator ->() { return _obj; }
};

class FigureKitType : public FrescoObjectType {
protected:
    FigureKitType();
    virtual ~FigureKitType();
public:
    _FigureStyleExpr default_style() {
        return _c_default_style();
    }
    virtual FigureStyleRef _c_default_style();
    _FigureStyleExpr new_style(StyleObjRef parent) {
        return _c_new_style(parent);
    }
    virtual FigureStyleRef _c_new_style(StyleObjRef parent);
    _GlyphExpr figure_root(GlyphRef child) {
        return _c_figure_root(child);
    }
    virtual GlyphRef _c_figure_root(GlyphRef child);
    _GlyphExpr label(FigureStyleRef s, CharStringRef str) {
        return _c_label(s, str);
    }
    virtual GlyphRef _c_label(FigureStyleRef s, CharStringRef str);
    _GlyphExpr point(FigureStyleRef s, Coord x, Coord y) {
        return _c_point(s, x, y);
    }
    virtual GlyphRef _c_point(FigureStyleRef s, Coord x, Coord y);
    _GlyphExpr line(FigureStyleRef s, Coord x0, Coord y0, Coord x1, Coord y1) {
        return _c_line(s, x0, y0, x1, y1);
    }
    virtual GlyphRef _c_line(FigureStyleRef s, Coord x0, Coord y0, Coord x1, Coord y1);
    _GlyphExpr rectangle(FigureKit::Mode m, FigureStyleRef s, Coord left, Coord bottom, Coord right, Coord top) {
        return _c_rectangle(m, s, left, bottom, right, top);
    }
    virtual GlyphRef _c_rectangle(FigureKit::Mode m, FigureStyleRef s, Coord left, Coord bottom, Coord right, Coord top);
    _GlyphExpr circle(FigureKit::Mode m, FigureStyleRef s, Coord x, Coord y, Coord r) {
        return _c_circle(m, s, x, y, r);
    }
    virtual GlyphRef _c_circle(FigureKit::Mode m, FigureStyleRef s, Coord x, Coord y, Coord r);
    _GlyphExpr ellipse(FigureKit::Mode m, FigureStyleRef s, Coord x, Coord y, Coord r1, Coord r2) {
        return _c_ellipse(m, s, x, y, r1, r2);
    }
    virtual GlyphRef _c_ellipse(FigureKit::Mode m, FigureStyleRef s, Coord x, Coord y, Coord r1, Coord r2);
    _GlyphExpr open_bspline(FigureKit::Mode m, FigureStyleRef s, const FigureKit::Vertices& v) {
        return _c_open_bspline(m, s, v);
    }
    virtual GlyphRef _c_open_bspline(FigureKit::Mode m, FigureStyleRef s, const FigureKit::Vertices& v);
    _GlyphExpr closed_bspline(FigureKit::Mode m, FigureStyleRef s, const FigureKit::Vertices& v) {
        return _c_closed_bspline(m, s, v);
    }
    virtual GlyphRef _c_closed_bspline(FigureKit::Mode m, FigureStyleRef s, const FigureKit::Vertices& v);
    _GlyphExpr multiline(FigureKit::Mode m, FigureStyleRef s, const FigureKit::Vertices& v) {
        return _c_multiline(m, s, v);
    }
    virtual GlyphRef _c_multiline(FigureKit::Mode m, FigureStyleRef s, const FigureKit::Vertices& v);
    _GlyphExpr polygon(FigureKit::Mode m, FigureStyleRef s, const FigureKit::Vertices& v) {
        return _c_polygon(m, s, v);
    }
    virtual GlyphRef _c_polygon(FigureKit::Mode m, FigureStyleRef s, const FigureKit::Vertices& v);
    _GlyphExpr fitter(GlyphRef g) {
        return _c_fitter(g);
    }
    virtual GlyphRef _c_fitter(GlyphRef g);
    _GlyphExpr group() {
        return _c_group();
    }
    virtual GlyphRef _c_group();

    _FigureKitExpr _ref();
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

inline FigureKitRef FigureKit::_duplicate(FigureKitRef obj) {
    return (FigureKitRef)_BaseObject__duplicate(obj, &FigureKitStub::_create);
}
inline FigureKit& FigureKit::operator =(FigureKitRef p) {
    _BaseObject__release(_obj);
    _obj = FigureKit::_duplicate(p);
    return *this;
}
inline FigureKit::FigureKit(const FigureKit& r) {
    _obj = FigureKit::_duplicate(r._obj);
}
inline FigureKit& FigureKit::operator =(const FigureKit& r) {
    _BaseObject__release(_obj);
    _obj = FigureKit::_duplicate(r._obj);
    return *this;
}
inline FigureKit::FigureKit(const _FigureKitExpr& r) {
    _obj = r._obj;
    ((_FigureKitExpr*)&r)->_obj = 0;
}
inline FigureKit& FigureKit::operator =(const _FigureKitExpr& r) {
    _BaseObject__release(_obj);
    _obj = r._obj;
    ((_FigureKitExpr*)&r)->_obj = 0;
    return *this;
}
inline FigureKit::FigureKit(const _FigureKitElem& e) {
    _obj = FigureKit::_duplicate(e._obj);
}
inline FigureKit& FigureKit::operator =(const _FigureKitElem& e) {
    _BaseObject__release(_obj);
    _obj = FigureKit::_duplicate(e._obj);
    return *this;
}
inline FigureKit::~FigureKit() {
    _BaseObject__release(_obj);
}
inline _FigureKitExpr FigureKit::_narrow(const BaseObject& r) {
    return _narrow(r._obj);
}
inline _FigureKitExpr FigureKit::_duplicate(const FigureKit& r) {
    return _duplicate(r._obj);
}
inline FigureKit::operator FrescoObject() const {
    return _FrescoObjectExpr((FrescoObjectRef)_BaseObject__duplicate(_obj, &FrescoObjectStub::_create));
}
inline _FigureKitExpr::~_FigureKitExpr() { }
inline _FigureKitExpr FigureKitType::_ref() { return this; }

#endif
