/* DO NOT EDIT -- Automatically generated from Interfaces/drawing.idl */

#ifndef Interfaces_drawing_h
#define Interfaces_drawing_h

#include <X11/Fresco/_enter.h>
#include <X11/Fresco/types.h>

class BrushType;
typedef BrushType* BrushRef;
class Brush;
class _BrushExpr;
class _BrushElem;

class Brush {
public:
    BrushRef _obj;

    Brush() { _obj = 0; }
    Brush(BrushRef p) { _obj = p; }
    Brush& operator =(BrushRef p);
    Brush(const Brush&);
    Brush& operator =(const Brush& r);
    Brush(const _BrushExpr&);
    Brush& operator =(const _BrushExpr&);
    Brush(const _BrushElem&);
    Brush& operator =(const _BrushElem&);
    ~Brush();

    operator BrushRef() const { return _obj; }
    BrushRef operator ->() { return _obj; }

    operator FrescoObject() const;
    static BrushRef _narrow(BaseObjectRef p);
    static _BrushExpr _narrow(const BaseObject& r);

    static BrushRef _duplicate(BrushRef obj);
    static _BrushExpr _duplicate(const Brush& r);
};

class _BrushExpr : public Brush {
public:
    _BrushExpr(BrushRef p) { _obj = p; }
    _BrushExpr(const Brush& r) { _obj = r._obj; }
    _BrushExpr(const _BrushExpr& r) { _obj = r._obj; }
    ~_BrushExpr();
};

class _BrushElem {
public:
    BrushRef _obj;

    _BrushElem(BrushRef p) { _obj = p; }
    operator BrushRef() const { return _obj; }
    BrushRef operator ->() { return _obj; }
};

class BrushType : public FrescoObjectType {
protected:
    BrushType();
    virtual ~BrushType();
public:
    virtual Boolean equal(BrushRef b);
    virtual ULong hash();

    _BrushExpr _ref();
    virtual TypeObjId _tid();
};

class BrushStub : public BrushType {
public:
    BrushStub(Exchange*);
    ~BrushStub();

    static BaseObjectRef _create(Exchange*);
    Exchange* _exchange();
protected:
    Exchange* exch_;
};

inline BrushRef Brush::_duplicate(BrushRef obj) {
    return (BrushRef)_BaseObject__duplicate(obj, &BrushStub::_create);
}
inline Brush& Brush::operator =(BrushRef p) {
    _BaseObject__release(_obj);
    _obj = Brush::_duplicate(p);
    return *this;
}
inline Brush::Brush(const Brush& r) {
    _obj = Brush::_duplicate(r._obj);
}
inline Brush& Brush::operator =(const Brush& r) {
    _BaseObject__release(_obj);
    _obj = Brush::_duplicate(r._obj);
    return *this;
}
inline Brush::Brush(const _BrushExpr& r) {
    _obj = r._obj;
    ((_BrushExpr*)&r)->_obj = 0;
}
inline Brush& Brush::operator =(const _BrushExpr& r) {
    _BaseObject__release(_obj);
    _obj = r._obj;
    ((_BrushExpr*)&r)->_obj = 0;
    return *this;
}
inline Brush::Brush(const _BrushElem& e) {
    _obj = Brush::_duplicate(e._obj);
}
inline Brush& Brush::operator =(const _BrushElem& e) {
    _BaseObject__release(_obj);
    _obj = Brush::_duplicate(e._obj);
    return *this;
}
inline Brush::~Brush() {
    _BaseObject__release(_obj);
}
inline _BrushExpr Brush::_narrow(const BaseObject& r) {
    return _narrow(r._obj);
}
inline _BrushExpr Brush::_duplicate(const Brush& r) {
    return _duplicate(r._obj);
}
inline Brush::operator FrescoObject() const {
    return _FrescoObjectExpr((FrescoObjectRef)_BaseObject__duplicate(_obj, &FrescoObjectStub::_create));
}
inline _BrushExpr::~_BrushExpr() { }
inline _BrushExpr BrushType::_ref() { return this; }

class ColorType;
typedef ColorType* ColorRef;
class Color;
class _ColorExpr;
class _ColorElem;

class Color {
public:
    ColorRef _obj;

    Color() { _obj = 0; }
    Color(ColorRef p) { _obj = p; }
    Color& operator =(ColorRef p);
    Color(const Color&);
    Color& operator =(const Color& r);
    Color(const _ColorExpr&);
    Color& operator =(const _ColorExpr&);
    Color(const _ColorElem&);
    Color& operator =(const _ColorElem&);
    ~Color();

    operator ColorRef() const { return _obj; }
    ColorRef operator ->() { return _obj; }

    operator FrescoObject() const;
    static ColorRef _narrow(BaseObjectRef p);
    static _ColorExpr _narrow(const BaseObject& r);

    static ColorRef _duplicate(ColorRef obj);
    static _ColorExpr _duplicate(const Color& r);
    typedef Float Intensity;
};

class _ColorExpr : public Color {
public:
    _ColorExpr(ColorRef p) { _obj = p; }
    _ColorExpr(const Color& r) { _obj = r._obj; }
    _ColorExpr(const _ColorExpr& r) { _obj = r._obj; }
    ~_ColorExpr();
};

class _ColorElem {
public:
    ColorRef _obj;

    _ColorElem(ColorRef p) { _obj = p; }
    operator ColorRef() const { return _obj; }
    ColorRef operator ->() { return _obj; }
};

class ColorType : public FrescoObjectType {
protected:
    ColorType();
    virtual ~ColorType();
public:
    virtual void rgb(Color::Intensity& r, Color::Intensity& g, Color::Intensity& b);
    virtual Boolean equal(ColorRef c);
    virtual ULong hash();

    _ColorExpr _ref();
    virtual TypeObjId _tid();
};

class ColorStub : public ColorType {
public:
    ColorStub(Exchange*);
    ~ColorStub();

    static BaseObjectRef _create(Exchange*);
    Exchange* _exchange();
protected:
    Exchange* exch_;
};

inline ColorRef Color::_duplicate(ColorRef obj) {
    return (ColorRef)_BaseObject__duplicate(obj, &ColorStub::_create);
}
inline Color& Color::operator =(ColorRef p) {
    _BaseObject__release(_obj);
    _obj = Color::_duplicate(p);
    return *this;
}
inline Color::Color(const Color& r) {
    _obj = Color::_duplicate(r._obj);
}
inline Color& Color::operator =(const Color& r) {
    _BaseObject__release(_obj);
    _obj = Color::_duplicate(r._obj);
    return *this;
}
inline Color::Color(const _ColorExpr& r) {
    _obj = r._obj;
    ((_ColorExpr*)&r)->_obj = 0;
}
inline Color& Color::operator =(const _ColorExpr& r) {
    _BaseObject__release(_obj);
    _obj = r._obj;
    ((_ColorExpr*)&r)->_obj = 0;
    return *this;
}
inline Color::Color(const _ColorElem& e) {
    _obj = Color::_duplicate(e._obj);
}
inline Color& Color::operator =(const _ColorElem& e) {
    _BaseObject__release(_obj);
    _obj = Color::_duplicate(e._obj);
    return *this;
}
inline Color::~Color() {
    _BaseObject__release(_obj);
}
inline _ColorExpr Color::_narrow(const BaseObject& r) {
    return _narrow(r._obj);
}
inline _ColorExpr Color::_duplicate(const Color& r) {
    return _duplicate(r._obj);
}
inline Color::operator FrescoObject() const {
    return _FrescoObjectExpr((FrescoObjectRef)_BaseObject__duplicate(_obj, &FrescoObjectStub::_create));
}
inline _ColorExpr::~_ColorExpr() { }
inline _ColorExpr ColorType::_ref() { return this; }

class FontType;
typedef FontType* FontRef;
class Font;
class _FontExpr;
class _FontElem;

class Font {
public:
    FontRef _obj;

    Font() { _obj = 0; }
    Font(FontRef p) { _obj = p; }
    Font& operator =(FontRef p);
    Font(const Font&);
    Font& operator =(const Font& r);
    Font(const _FontExpr&);
    Font& operator =(const _FontExpr&);
    Font(const _FontElem&);
    Font& operator =(const _FontElem&);
    ~Font();

    operator FontRef() const { return _obj; }
    FontRef operator ->() { return _obj; }

    operator FrescoObject() const;
    static FontRef _narrow(BaseObjectRef p);
    static _FontExpr _narrow(const BaseObject& r);

    static FontRef _duplicate(FontRef obj);
    static _FontExpr _duplicate(const Font& r);
    struct Info {
        Coord width, height;
        Coord left_bearing, right_bearing;
        Coord ascent, descent;
        Coord font_ascent, font_descent;
    };
};

class _FontExpr : public Font {
public:
    _FontExpr(FontRef p) { _obj = p; }
    _FontExpr(const Font& r) { _obj = r._obj; }
    _FontExpr(const _FontExpr& r) { _obj = r._obj; }
    ~_FontExpr();
};

class _FontElem {
public:
    FontRef _obj;

    _FontElem(FontRef p) { _obj = p; }
    operator FontRef() const { return _obj; }
    FontRef operator ->() { return _obj; }
};

class FontType : public FrescoObjectType {
protected:
    FontType();
    virtual ~FontType();
public:
    virtual Boolean equal(FontRef f);
    virtual ULong hash();
    _CharStringExpr name() {
        return _c_name();
    }
    virtual CharStringRef _c_name();
    _CharStringExpr encoding() {
        return _c_encoding();
    }
    virtual CharStringRef _c_encoding();
    virtual Coord point_size();
    virtual void font_info(Font::Info& i);
    virtual void char_info(CharCode c, Font::Info& i);
    virtual void string_info(CharStringRef s, Font::Info& i);

    _FontExpr _ref();
    virtual TypeObjId _tid();
};

class FontStub : public FontType {
public:
    FontStub(Exchange*);
    ~FontStub();

    static BaseObjectRef _create(Exchange*);
    Exchange* _exchange();
protected:
    Exchange* exch_;
};

inline FontRef Font::_duplicate(FontRef obj) {
    return (FontRef)_BaseObject__duplicate(obj, &FontStub::_create);
}
inline Font& Font::operator =(FontRef p) {
    _BaseObject__release(_obj);
    _obj = Font::_duplicate(p);
    return *this;
}
inline Font::Font(const Font& r) {
    _obj = Font::_duplicate(r._obj);
}
inline Font& Font::operator =(const Font& r) {
    _BaseObject__release(_obj);
    _obj = Font::_duplicate(r._obj);
    return *this;
}
inline Font::Font(const _FontExpr& r) {
    _obj = r._obj;
    ((_FontExpr*)&r)->_obj = 0;
}
inline Font& Font::operator =(const _FontExpr& r) {
    _BaseObject__release(_obj);
    _obj = r._obj;
    ((_FontExpr*)&r)->_obj = 0;
    return *this;
}
inline Font::Font(const _FontElem& e) {
    _obj = Font::_duplicate(e._obj);
}
inline Font& Font::operator =(const _FontElem& e) {
    _BaseObject__release(_obj);
    _obj = Font::_duplicate(e._obj);
    return *this;
}
inline Font::~Font() {
    _BaseObject__release(_obj);
}
inline _FontExpr Font::_narrow(const BaseObject& r) {
    return _narrow(r._obj);
}
inline _FontExpr Font::_duplicate(const Font& r) {
    return _duplicate(r._obj);
}
inline Font::operator FrescoObject() const {
    return _FrescoObjectExpr((FrescoObjectRef)_BaseObject__duplicate(_obj, &FrescoObjectStub::_create));
}
inline _FontExpr::~_FontExpr() { }
inline _FontExpr FontType::_ref() { return this; }

class RasterType;
typedef RasterType* RasterRef;
class Raster;
class _RasterExpr;
class _RasterElem;

class Raster {
public:
    RasterRef _obj;

    Raster() { _obj = 0; }
    Raster(RasterRef p) { _obj = p; }
    Raster& operator =(RasterRef p);
    Raster(const Raster&);
    Raster& operator =(const Raster& r);
    Raster(const _RasterExpr&);
    Raster& operator =(const _RasterExpr&);
    Raster(const _RasterElem&);
    Raster& operator =(const _RasterElem&);
    ~Raster();

    operator RasterRef() const { return _obj; }
    RasterRef operator ->() { return _obj; }

    operator FrescoObject() const;
    static RasterRef _narrow(BaseObjectRef p);
    static _RasterExpr _narrow(const BaseObject& r);

    static RasterRef _duplicate(RasterRef obj);
    static _RasterExpr _duplicate(const Raster& r);
    typedef Long Index;
    struct Element {
        Boolean on;
        ColorRef pixel;
        Float blend;
    };
};

class _RasterExpr : public Raster {
public:
    _RasterExpr(RasterRef p) { _obj = p; }
    _RasterExpr(const Raster& r) { _obj = r._obj; }
    _RasterExpr(const _RasterExpr& r) { _obj = r._obj; }
    ~_RasterExpr();
};

class _RasterElem {
public:
    RasterRef _obj;

    _RasterElem(RasterRef p) { _obj = p; }
    operator RasterRef() const { return _obj; }
    RasterRef operator ->() { return _obj; }
};

class RasterType : public FrescoObjectType {
protected:
    RasterType();
    virtual ~RasterType();
public:
    virtual Boolean equal(RasterRef r);
    virtual ULong hash();
    virtual Raster::Index rows();
    virtual Raster::Index columns();
    virtual Raster::Index origin_x();
    virtual Raster::Index origin_y();
    virtual void peek(Raster::Index row, Raster::Index column, Raster::Element& e);
    virtual void poke(Raster::Index row, Raster::Index column, const Raster::Element& e);
    virtual Coord scale();
    virtual void scale(Coord _p);

    _RasterExpr _ref();
    virtual TypeObjId _tid();
};

class RasterStub : public RasterType {
public:
    RasterStub(Exchange*);
    ~RasterStub();

    static BaseObjectRef _create(Exchange*);
    Exchange* _exchange();
protected:
    Exchange* exch_;
};

inline RasterRef Raster::_duplicate(RasterRef obj) {
    return (RasterRef)_BaseObject__duplicate(obj, &RasterStub::_create);
}
inline Raster& Raster::operator =(RasterRef p) {
    _BaseObject__release(_obj);
    _obj = Raster::_duplicate(p);
    return *this;
}
inline Raster::Raster(const Raster& r) {
    _obj = Raster::_duplicate(r._obj);
}
inline Raster& Raster::operator =(const Raster& r) {
    _BaseObject__release(_obj);
    _obj = Raster::_duplicate(r._obj);
    return *this;
}
inline Raster::Raster(const _RasterExpr& r) {
    _obj = r._obj;
    ((_RasterExpr*)&r)->_obj = 0;
}
inline Raster& Raster::operator =(const _RasterExpr& r) {
    _BaseObject__release(_obj);
    _obj = r._obj;
    ((_RasterExpr*)&r)->_obj = 0;
    return *this;
}
inline Raster::Raster(const _RasterElem& e) {
    _obj = Raster::_duplicate(e._obj);
}
inline Raster& Raster::operator =(const _RasterElem& e) {
    _BaseObject__release(_obj);
    _obj = Raster::_duplicate(e._obj);
    return *this;
}
inline Raster::~Raster() {
    _BaseObject__release(_obj);
}
inline _RasterExpr Raster::_narrow(const BaseObject& r) {
    return _narrow(r._obj);
}
inline _RasterExpr Raster::_duplicate(const Raster& r) {
    return _duplicate(r._obj);
}
inline Raster::operator FrescoObject() const {
    return _FrescoObjectExpr((FrescoObjectRef)_BaseObject__duplicate(_obj, &FrescoObjectStub::_create));
}
inline _RasterExpr::~_RasterExpr() { }
inline _RasterExpr RasterType::_ref() { return this; }

class PainterObjType;
typedef PainterObjType* PainterObjRef;
class PainterObj;
class _PainterObjExpr;
class _PainterObjElem;

class PainterObj {
public:
    PainterObjRef _obj;

    PainterObj() { _obj = 0; }
    PainterObj(PainterObjRef p) { _obj = p; }
    PainterObj& operator =(PainterObjRef p);
    PainterObj(const PainterObj&);
    PainterObj& operator =(const PainterObj& r);
    PainterObj(const _PainterObjExpr&);
    PainterObj& operator =(const _PainterObjExpr&);
    PainterObj(const _PainterObjElem&);
    PainterObj& operator =(const _PainterObjElem&);
    ~PainterObj();

    operator PainterObjRef() const { return _obj; }
    PainterObjRef operator ->() { return _obj; }

    operator FrescoObject() const;
    static PainterObjRef _narrow(BaseObjectRef p);
    static _PainterObjExpr _narrow(const BaseObject& r);

    static PainterObjRef _duplicate(PainterObjRef obj);
    static _PainterObjExpr _duplicate(const PainterObj& r);
};

class _PainterObjExpr : public PainterObj {
public:
    _PainterObjExpr(PainterObjRef p) { _obj = p; }
    _PainterObjExpr(const PainterObj& r) { _obj = r._obj; }
    _PainterObjExpr(const _PainterObjExpr& r) { _obj = r._obj; }
    ~_PainterObjExpr();
};

class _PainterObjElem {
public:
    PainterObjRef _obj;

    _PainterObjElem(PainterObjRef p) { _obj = p; }
    operator PainterObjRef() const { return _obj; }
    PainterObjRef operator ->() { return _obj; }
};

class PainterObjType : public FrescoObjectType {
protected:
    PainterObjType();
    virtual ~PainterObjType();
public:
    virtual Coord to_coord(PixelCoord p);
    virtual PixelCoord to_pixels(Coord c);
    virtual Coord to_pixels_coord(Coord c);
    virtual void begin_path();
    virtual void move_to(Coord x, Coord y);
    virtual void line_to(Coord x, Coord y);
    virtual void curve_to(Coord x, Coord y, Coord x1, Coord y1, Coord x2, Coord y2);
    virtual void close_path();
    _BrushExpr brush_attr() {
        return _c_brush_attr();
    }
    virtual BrushRef _c_brush_attr();
    void brush_attr(BrushRef _p) {
        _c_brush_attr(_p);
    }
    virtual void _c_brush_attr(BrushRef _p);
    _ColorExpr color_attr() {
        return _c_color_attr();
    }
    virtual ColorRef _c_color_attr();
    void color_attr(ColorRef _p) {
        _c_color_attr(_p);
    }
    virtual void _c_color_attr(ColorRef _p);
    _FontExpr font_attr() {
        return _c_font_attr();
    }
    virtual FontRef _c_font_attr();
    void font_attr(FontRef _p) {
        _c_font_attr(_p);
    }
    virtual void _c_font_attr(FontRef _p);
    virtual void stroke();
    virtual void fill();
    virtual void line(Coord x0, Coord y0, Coord x1, Coord y1);
    virtual void rect(Coord x0, Coord y0, Coord x1, Coord y1);
    virtual void fill_rect(Coord x0, Coord y0, Coord x1, Coord y1);
    virtual void character(CharCode ch, Coord width, Coord x, Coord y);
    virtual void image(RasterRef r, Coord x, Coord y);
    virtual void stencil(RasterRef r, Coord x, Coord y);
    _TransformObjExpr matrix() {
        return _c_matrix();
    }
    virtual TransformObjRef _c_matrix();
    void matrix(TransformObjRef _p) {
        _c_matrix(_p);
    }
    virtual void _c_matrix(TransformObjRef _p);
    virtual void push_matrix();
    virtual void pop_matrix();
    virtual void transform(TransformObjRef t);
    virtual void clip();
    virtual void clip_rect(Coord x0, Coord y0, Coord x1, Coord y1);
    virtual void push_clipping();
    virtual void pop_clipping();
    virtual Boolean is_visible(RegionRef r);
    _RegionExpr visible() {
        return _c_visible();
    }
    virtual RegionRef _c_visible();
    virtual void comment(CharStringRef s);
    virtual void page_number(CharStringRef s);

    _PainterObjExpr _ref();
    virtual TypeObjId _tid();
};

class PainterObjStub : public PainterObjType {
public:
    PainterObjStub(Exchange*);
    ~PainterObjStub();

    static BaseObjectRef _create(Exchange*);
    Exchange* _exchange();
protected:
    Exchange* exch_;
};

inline PainterObjRef PainterObj::_duplicate(PainterObjRef obj) {
    return (PainterObjRef)_BaseObject__duplicate(obj, &PainterObjStub::_create);
}
inline PainterObj& PainterObj::operator =(PainterObjRef p) {
    _BaseObject__release(_obj);
    _obj = PainterObj::_duplicate(p);
    return *this;
}
inline PainterObj::PainterObj(const PainterObj& r) {
    _obj = PainterObj::_duplicate(r._obj);
}
inline PainterObj& PainterObj::operator =(const PainterObj& r) {
    _BaseObject__release(_obj);
    _obj = PainterObj::_duplicate(r._obj);
    return *this;
}
inline PainterObj::PainterObj(const _PainterObjExpr& r) {
    _obj = r._obj;
    ((_PainterObjExpr*)&r)->_obj = 0;
}
inline PainterObj& PainterObj::operator =(const _PainterObjExpr& r) {
    _BaseObject__release(_obj);
    _obj = r._obj;
    ((_PainterObjExpr*)&r)->_obj = 0;
    return *this;
}
inline PainterObj::PainterObj(const _PainterObjElem& e) {
    _obj = PainterObj::_duplicate(e._obj);
}
inline PainterObj& PainterObj::operator =(const _PainterObjElem& e) {
    _BaseObject__release(_obj);
    _obj = PainterObj::_duplicate(e._obj);
    return *this;
}
inline PainterObj::~PainterObj() {
    _BaseObject__release(_obj);
}
inline _PainterObjExpr PainterObj::_narrow(const BaseObject& r) {
    return _narrow(r._obj);
}
inline _PainterObjExpr PainterObj::_duplicate(const PainterObj& r) {
    return _duplicate(r._obj);
}
inline PainterObj::operator FrescoObject() const {
    return _FrescoObjectExpr((FrescoObjectRef)_BaseObject__duplicate(_obj, &FrescoObjectStub::_create));
}
inline _PainterObjExpr::~_PainterObjExpr() { }
inline _PainterObjExpr PainterObjType::_ref() { return this; }

class DrawingKitType;
typedef DrawingKitType* DrawingKitRef;
class DrawingKit;
class _DrawingKitExpr;
class _DrawingKitElem;

class DrawingKit {
public:
    DrawingKitRef _obj;

    DrawingKit() { _obj = 0; }
    DrawingKit(DrawingKitRef p) { _obj = p; }
    DrawingKit& operator =(DrawingKitRef p);
    DrawingKit(const DrawingKit&);
    DrawingKit& operator =(const DrawingKit& r);
    DrawingKit(const _DrawingKitExpr&);
    DrawingKit& operator =(const _DrawingKitExpr&);
    DrawingKit(const _DrawingKitElem&);
    DrawingKit& operator =(const _DrawingKitElem&);
    ~DrawingKit();

    operator DrawingKitRef() const { return _obj; }
    DrawingKitRef operator ->() { return _obj; }

    operator FrescoObject() const;
    static DrawingKitRef _narrow(BaseObjectRef p);
    static _DrawingKitExpr _narrow(const BaseObject& r);

    static DrawingKitRef _duplicate(DrawingKitRef obj);
    static _DrawingKitExpr _duplicate(const DrawingKit& r);
    class Data {
    public:
        long _maximum, _length; Long* _buffer;

        Data() { _maximum = _length = 0; _buffer = 0; }
        Data(long m, long n, Long* e) {
            _maximum = m; _length = n; _buffer = e;
        }
        Data(const Data& _s) { _buffer = 0; *this = _s; }
        Data& operator =(const Data&);
        ~Data() { delete [] _buffer; }
    };
};

class _DrawingKitExpr : public DrawingKit {
public:
    _DrawingKitExpr(DrawingKitRef p) { _obj = p; }
    _DrawingKitExpr(const DrawingKit& r) { _obj = r._obj; }
    _DrawingKitExpr(const _DrawingKitExpr& r) { _obj = r._obj; }
    ~_DrawingKitExpr();
};

class _DrawingKitElem {
public:
    DrawingKitRef _obj;

    _DrawingKitElem(DrawingKitRef p) { _obj = p; }
    operator DrawingKitRef() const { return _obj; }
    DrawingKitRef operator ->() { return _obj; }
};

class DrawingKitType : public FrescoObjectType {
protected:
    DrawingKitType();
    virtual ~DrawingKitType();
public:
    _StyleObjExpr style() {
        return _c_style();
    }
    virtual StyleObjRef _c_style();
    _BrushExpr simple_brush(Coord width) {
        return _c_simple_brush(width);
    }
    virtual BrushRef _c_simple_brush(Coord width);
    _BrushExpr dither_brush(Coord width, Long pattern) {
        return _c_dither_brush(width, pattern);
    }
    virtual BrushRef _c_dither_brush(Coord width, Long pattern);
    _BrushExpr patterned_brush(Coord width, const DrawingKit::Data& pattern) {
        return _c_patterned_brush(width, pattern);
    }
    virtual BrushRef _c_patterned_brush(Coord width, const DrawingKit::Data& pattern);
    _ColorExpr color_rgb(Color::Intensity r, Color::Intensity g, Color::Intensity b) {
        return _c_color_rgb(r, g, b);
    }
    virtual ColorRef _c_color_rgb(Color::Intensity r, Color::Intensity g, Color::Intensity b);
    _ColorExpr find_color(CharStringRef name) {
        return _c_find_color(name);
    }
    virtual ColorRef _c_find_color(CharStringRef name);
    _ColorExpr resolve_color(StyleObjRef s, CharStringRef name) {
        return _c_resolve_color(s, name);
    }
    virtual ColorRef _c_resolve_color(StyleObjRef s, CharStringRef name);
    _ColorExpr foreground(StyleObjRef s) {
        return _c_foreground(s);
    }
    virtual ColorRef _c_foreground(StyleObjRef s);
    _ColorExpr background(StyleObjRef s) {
        return _c_background(s);
    }
    virtual ColorRef _c_background(StyleObjRef s);
    _FontExpr find_font(CharStringRef fullname) {
        return _c_find_font(fullname);
    }
    virtual FontRef _c_find_font(CharStringRef fullname);
    _FontExpr find_font_match(CharStringRef family, CharStringRef style, Coord ptsize) {
        return _c_find_font_match(family, style, ptsize);
    }
    virtual FontRef _c_find_font_match(CharStringRef family, CharStringRef style, Coord ptsize);
    _FontExpr resolve_font(StyleObjRef s, CharStringRef name) {
        return _c_resolve_font(s, name);
    }
    virtual FontRef _c_resolve_font(StyleObjRef s, CharStringRef name);
    _FontExpr default_font(StyleObjRef s) {
        return _c_default_font(s);
    }
    virtual FontRef _c_default_font(StyleObjRef s);
    _RasterExpr bitmap_file(CharStringRef filename) {
        return _c_bitmap_file(filename);
    }
    virtual RasterRef _c_bitmap_file(CharStringRef filename);
    _RasterExpr bitmap_data(const DrawingKit::Data& data, Raster::Index rows, Raster::Index columns, Raster::Index origin_row, Raster::Index origin_column) {
        return _c_bitmap_data(data, rows, columns, origin_row, origin_column);
    }
    virtual RasterRef _c_bitmap_data(const DrawingKit::Data& data, Raster::Index rows, Raster::Index columns, Raster::Index origin_row, Raster::Index origin_column);
    _RasterExpr bitmap_char(FontRef f, CharCode c) {
        return _c_bitmap_char(f, c);
    }
    virtual RasterRef _c_bitmap_char(FontRef f, CharCode c);
    _RasterExpr raster_tiff(CharStringRef filename) {
        return _c_raster_tiff(filename);
    }
    virtual RasterRef _c_raster_tiff(CharStringRef filename);
    _TransformObjExpr transform(TransformObj::Matrix m) {
        return _c_transform(m);
    }
    virtual TransformObjRef _c_transform(TransformObj::Matrix m);
    _TransformObjExpr identity_transform() {
        return _c_identity_transform();
    }
    virtual TransformObjRef _c_identity_transform();
    _PainterObjExpr printer(CharStringRef output) {
        return _c_printer(output);
    }
    virtual PainterObjRef _c_printer(CharStringRef output);

    _DrawingKitExpr _ref();
    virtual TypeObjId _tid();
};

class DrawingKitStub : public DrawingKitType {
public:
    DrawingKitStub(Exchange*);
    ~DrawingKitStub();

    static BaseObjectRef _create(Exchange*);
    Exchange* _exchange();
protected:
    Exchange* exch_;
};

inline DrawingKitRef DrawingKit::_duplicate(DrawingKitRef obj) {
    return (DrawingKitRef)_BaseObject__duplicate(obj, &DrawingKitStub::_create);
}
inline DrawingKit& DrawingKit::operator =(DrawingKitRef p) {
    _BaseObject__release(_obj);
    _obj = DrawingKit::_duplicate(p);
    return *this;
}
inline DrawingKit::DrawingKit(const DrawingKit& r) {
    _obj = DrawingKit::_duplicate(r._obj);
}
inline DrawingKit& DrawingKit::operator =(const DrawingKit& r) {
    _BaseObject__release(_obj);
    _obj = DrawingKit::_duplicate(r._obj);
    return *this;
}
inline DrawingKit::DrawingKit(const _DrawingKitExpr& r) {
    _obj = r._obj;
    ((_DrawingKitExpr*)&r)->_obj = 0;
}
inline DrawingKit& DrawingKit::operator =(const _DrawingKitExpr& r) {
    _BaseObject__release(_obj);
    _obj = r._obj;
    ((_DrawingKitExpr*)&r)->_obj = 0;
    return *this;
}
inline DrawingKit::DrawingKit(const _DrawingKitElem& e) {
    _obj = DrawingKit::_duplicate(e._obj);
}
inline DrawingKit& DrawingKit::operator =(const _DrawingKitElem& e) {
    _BaseObject__release(_obj);
    _obj = DrawingKit::_duplicate(e._obj);
    return *this;
}
inline DrawingKit::~DrawingKit() {
    _BaseObject__release(_obj);
}
inline _DrawingKitExpr DrawingKit::_narrow(const BaseObject& r) {
    return _narrow(r._obj);
}
inline _DrawingKitExpr DrawingKit::_duplicate(const DrawingKit& r) {
    return _duplicate(r._obj);
}
inline DrawingKit::operator FrescoObject() const {
    return _FrescoObjectExpr((FrescoObjectRef)_BaseObject__duplicate(_obj, &FrescoObjectStub::_create));
}
inline _DrawingKitExpr::~_DrawingKitExpr() { }
inline _DrawingKitExpr DrawingKitType::_ref() { return this; }

#endif
