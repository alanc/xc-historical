/* DO NOT EDIT -- Automatically generated from Interfaces/drawing.idl */

#ifndef Interfaces_drawing_h
#define Interfaces_drawing_h

#include <X11/Fresco/_enter.h>
#include <X11/Fresco/types.h>

class BrushType;
typedef BrushType* BrushRef;
typedef BrushRef Brush_in;
class Brush;
class Brush_tmp;
class Brush_var;

class Brush {
public:
    BrushRef _obj_;

    Brush() { _obj_ = 0; }
    Brush(BrushRef p) { _obj_ = p; }
    Brush& operator =(BrushRef p);
    Brush(const Brush&);
    Brush& operator =(const Brush& r);
    Brush(const Brush_tmp&);
    Brush& operator =(const Brush_tmp&);
    Brush(const Brush_var&);
    Brush& operator =(const Brush_var&);
    ~Brush();

    BrushRef operator ->() { return _obj_; }

    operator Brush_in() const { return _obj_; }
    operator FrescoObject() const;
    static BrushRef _narrow(BaseObjectRef p);
    static Brush_tmp _narrow(const BaseObject& r);

    static BrushRef _duplicate(BrushRef obj);
    static Brush_tmp _duplicate(const Brush& r);
};

class Brush_tmp : public Brush {
public:
    Brush_tmp(BrushRef p) { _obj_ = p; }
    Brush_tmp(const Brush& r);
    Brush_tmp(const Brush_tmp& r);
    ~Brush_tmp();
};

class Brush_var {
public:
    BrushRef _obj_;

    Brush_var(BrushRef p) { _obj_ = p; }
    operator BrushRef() const { return _obj_; }
    BrushRef operator ->() { return _obj_; }
};

class BrushType : public FrescoObjectType {
protected:
    BrushType();
    virtual ~BrushType();
public:
    virtual Boolean equal(Brush_in b);
    virtual ULong hash();
    BrushRef _obj() { return this; }
    void* _this();
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

class ColorType;
typedef ColorType* ColorRef;
typedef ColorRef Color_in;
class Color;
class Color_tmp;
class Color_var;

class Color {
public:
    ColorRef _obj_;

    Color() { _obj_ = 0; }
    Color(ColorRef p) { _obj_ = p; }
    Color& operator =(ColorRef p);
    Color(const Color&);
    Color& operator =(const Color& r);
    Color(const Color_tmp&);
    Color& operator =(const Color_tmp&);
    Color(const Color_var&);
    Color& operator =(const Color_var&);
    ~Color();

    ColorRef operator ->() { return _obj_; }

    operator Color_in() const { return _obj_; }
    operator FrescoObject() const;
    static ColorRef _narrow(BaseObjectRef p);
    static Color_tmp _narrow(const BaseObject& r);

    static ColorRef _duplicate(ColorRef obj);
    static Color_tmp _duplicate(const Color& r);
    typedef Float Intensity;
};

class Color_tmp : public Color {
public:
    Color_tmp(ColorRef p) { _obj_ = p; }
    Color_tmp(const Color& r);
    Color_tmp(const Color_tmp& r);
    ~Color_tmp();
};

class Color_var {
public:
    ColorRef _obj_;

    Color_var(ColorRef p) { _obj_ = p; }
    operator ColorRef() const { return _obj_; }
    ColorRef operator ->() { return _obj_; }
};

class ColorType : public FrescoObjectType {
protected:
    ColorType();
    virtual ~ColorType();
public:
    virtual void rgb(Color::Intensity& r, Color::Intensity& g, Color::Intensity& b);
    virtual Boolean equal(Color_in c);
    virtual ULong hash();
    ColorRef _obj() { return this; }
    void* _this();
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

class FontType;
typedef FontType* FontRef;
typedef FontRef Font_in;
class Font;
class Font_tmp;
class Font_var;

class Font {
public:
    FontRef _obj_;

    Font() { _obj_ = 0; }
    Font(FontRef p) { _obj_ = p; }
    Font& operator =(FontRef p);
    Font(const Font&);
    Font& operator =(const Font& r);
    Font(const Font_tmp&);
    Font& operator =(const Font_tmp&);
    Font(const Font_var&);
    Font& operator =(const Font_var&);
    ~Font();

    FontRef operator ->() { return _obj_; }

    operator Font_in() const { return _obj_; }
    operator FrescoObject() const;
    static FontRef _narrow(BaseObjectRef p);
    static Font_tmp _narrow(const BaseObject& r);

    static FontRef _duplicate(FontRef obj);
    static Font_tmp _duplicate(const Font& r);
    struct Info {
        Coord width, height;
        Coord left_bearing, right_bearing;
        Coord ascent, descent;
        Coord font_ascent, font_descent;
    };
};

class Font_tmp : public Font {
public:
    Font_tmp(FontRef p) { _obj_ = p; }
    Font_tmp(const Font& r);
    Font_tmp(const Font_tmp& r);
    ~Font_tmp();
};

class Font_var {
public:
    FontRef _obj_;

    Font_var(FontRef p) { _obj_ = p; }
    operator FontRef() const { return _obj_; }
    FontRef operator ->() { return _obj_; }
};

class FontType : public FrescoObjectType {
protected:
    FontType();
    virtual ~FontType();
public:
    virtual Boolean equal(Font_in f);
    virtual ULong hash();
    CharString_tmp name() {
        return _c_name();
    }
    virtual CharStringRef _c_name();
    CharString_tmp encoding() {
        return _c_encoding();
    }
    virtual CharStringRef _c_encoding();
    virtual Coord point_size();
    virtual void font_info(Font::Info& i);
    virtual void char_info(CharCode c, Font::Info& i);
    virtual void string_info(CharString_in s, Font::Info& i);
    FontRef _obj() { return this; }
    void* _this();
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

class RasterType;
typedef RasterType* RasterRef;
typedef RasterRef Raster_in;
class Raster;
class Raster_tmp;
class Raster_var;

class Raster {
public:
    RasterRef _obj_;

    Raster() { _obj_ = 0; }
    Raster(RasterRef p) { _obj_ = p; }
    Raster& operator =(RasterRef p);
    Raster(const Raster&);
    Raster& operator =(const Raster& r);
    Raster(const Raster_tmp&);
    Raster& operator =(const Raster_tmp&);
    Raster(const Raster_var&);
    Raster& operator =(const Raster_var&);
    ~Raster();

    RasterRef operator ->() { return _obj_; }

    operator Raster_in() const { return _obj_; }
    operator FrescoObject() const;
    static RasterRef _narrow(BaseObjectRef p);
    static Raster_tmp _narrow(const BaseObject& r);

    static RasterRef _duplicate(RasterRef obj);
    static Raster_tmp _duplicate(const Raster& r);
    typedef Long Index;
    struct Element {
        Boolean on;
        ColorRef pixel;
        Float blend;
    };
};

class Raster_tmp : public Raster {
public:
    Raster_tmp(RasterRef p) { _obj_ = p; }
    Raster_tmp(const Raster& r);
    Raster_tmp(const Raster_tmp& r);
    ~Raster_tmp();
};

class Raster_var {
public:
    RasterRef _obj_;

    Raster_var(RasterRef p) { _obj_ = p; }
    operator RasterRef() const { return _obj_; }
    RasterRef operator ->() { return _obj_; }
};

class RasterType : public FrescoObjectType {
protected:
    RasterType();
    virtual ~RasterType();
public:
    virtual Boolean equal(Raster_in r);
    virtual ULong hash();
    virtual Raster::Index rows();
    virtual Raster::Index columns();
    virtual Raster::Index origin_x();
    virtual Raster::Index origin_y();
    virtual void peek(Raster::Index row, Raster::Index column, Raster::Element& e);
    virtual void poke(Raster::Index row, Raster::Index column, const Raster::Element& e);
    virtual Coord scale();
    virtual void scale(Coord _p);
    RasterRef _obj() { return this; }
    void* _this();
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

class PainterObjType;
typedef PainterObjType* PainterObjRef;
typedef PainterObjRef PainterObj_in;
class PainterObj;
class PainterObj_tmp;
class PainterObj_var;

class PainterObj {
public:
    PainterObjRef _obj_;

    PainterObj() { _obj_ = 0; }
    PainterObj(PainterObjRef p) { _obj_ = p; }
    PainterObj& operator =(PainterObjRef p);
    PainterObj(const PainterObj&);
    PainterObj& operator =(const PainterObj& r);
    PainterObj(const PainterObj_tmp&);
    PainterObj& operator =(const PainterObj_tmp&);
    PainterObj(const PainterObj_var&);
    PainterObj& operator =(const PainterObj_var&);
    ~PainterObj();

    PainterObjRef operator ->() { return _obj_; }

    operator PainterObj_in() const { return _obj_; }
    operator FrescoObject() const;
    static PainterObjRef _narrow(BaseObjectRef p);
    static PainterObj_tmp _narrow(const BaseObject& r);

    static PainterObjRef _duplicate(PainterObjRef obj);
    static PainterObj_tmp _duplicate(const PainterObj& r);
};

class PainterObj_tmp : public PainterObj {
public:
    PainterObj_tmp(PainterObjRef p) { _obj_ = p; }
    PainterObj_tmp(const PainterObj& r);
    PainterObj_tmp(const PainterObj_tmp& r);
    ~PainterObj_tmp();
};

class PainterObj_var {
public:
    PainterObjRef _obj_;

    PainterObj_var(PainterObjRef p) { _obj_ = p; }
    operator PainterObjRef() const { return _obj_; }
    PainterObjRef operator ->() { return _obj_; }
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
    Brush_tmp brush_attr() {
        return _c_brush_attr();
    }
    virtual BrushRef _c_brush_attr();
    void brush_attr(Brush_in _p) {
        _c_brush_attr(_p);
    }
    virtual void _c_brush_attr(Brush_in _p);
    Color_tmp color_attr() {
        return _c_color_attr();
    }
    virtual ColorRef _c_color_attr();
    void color_attr(Color_in _p) {
        _c_color_attr(_p);
    }
    virtual void _c_color_attr(Color_in _p);
    Font_tmp font_attr() {
        return _c_font_attr();
    }
    virtual FontRef _c_font_attr();
    void font_attr(Font_in _p) {
        _c_font_attr(_p);
    }
    virtual void _c_font_attr(Font_in _p);
    virtual void stroke();
    virtual void fill();
    virtual void line(Coord x0, Coord y0, Coord x1, Coord y1);
    virtual void rect(Coord x0, Coord y0, Coord x1, Coord y1);
    virtual void fill_rect(Coord x0, Coord y0, Coord x1, Coord y1);
    virtual void character(CharCode ch, Coord width, Coord x, Coord y);
    virtual void image(Raster_in r, Coord x, Coord y);
    virtual void stencil(Raster_in r, Coord x, Coord y);
    TransformObj_tmp matrix() {
        return _c_matrix();
    }
    virtual TransformObjRef _c_matrix();
    void matrix(TransformObj_in _p) {
        _c_matrix(_p);
    }
    virtual void _c_matrix(TransformObj_in _p);
    virtual void push_matrix();
    virtual void pop_matrix();
    virtual void transform(TransformObj_in t);
    virtual void clip();
    virtual void clip_rect(Coord x0, Coord y0, Coord x1, Coord y1);
    virtual void push_clipping();
    virtual void pop_clipping();
    virtual Boolean is_visible(Region_in r);
    Region_tmp visible() {
        return _c_visible();
    }
    virtual RegionRef _c_visible();
    virtual void comment(CharString_in s);
    virtual void page_number(CharString_in s);
    PainterObjRef _obj() { return this; }
    void* _this();
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

class DrawingKitType;
typedef DrawingKitType* DrawingKitRef;
typedef DrawingKitRef DrawingKit_in;
class DrawingKit;
class DrawingKit_tmp;
class DrawingKit_var;

class DrawingKit {
public:
    DrawingKitRef _obj_;

    DrawingKit() { _obj_ = 0; }
    DrawingKit(DrawingKitRef p) { _obj_ = p; }
    DrawingKit& operator =(DrawingKitRef p);
    DrawingKit(const DrawingKit&);
    DrawingKit& operator =(const DrawingKit& r);
    DrawingKit(const DrawingKit_tmp&);
    DrawingKit& operator =(const DrawingKit_tmp&);
    DrawingKit(const DrawingKit_var&);
    DrawingKit& operator =(const DrawingKit_var&);
    ~DrawingKit();

    DrawingKitRef operator ->() { return _obj_; }

    operator DrawingKit_in() const { return _obj_; }
    operator FrescoObject() const;
    static DrawingKitRef _narrow(BaseObjectRef p);
    static DrawingKit_tmp _narrow(const BaseObject& r);

    static DrawingKitRef _duplicate(DrawingKitRef obj);
    static DrawingKit_tmp _duplicate(const DrawingKit& r);
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

class DrawingKit_tmp : public DrawingKit {
public:
    DrawingKit_tmp(DrawingKitRef p) { _obj_ = p; }
    DrawingKit_tmp(const DrawingKit& r);
    DrawingKit_tmp(const DrawingKit_tmp& r);
    ~DrawingKit_tmp();
};

class DrawingKit_var {
public:
    DrawingKitRef _obj_;

    DrawingKit_var(DrawingKitRef p) { _obj_ = p; }
    operator DrawingKitRef() const { return _obj_; }
    DrawingKitRef operator ->() { return _obj_; }
};

class DrawingKitType : public FrescoObjectType {
protected:
    DrawingKitType();
    virtual ~DrawingKitType();
public:
    StyleObj_tmp style() {
        return _c_style();
    }
    virtual StyleObjRef _c_style();
    Brush_tmp simple_brush(Coord width) {
        return _c_simple_brush(width);
    }
    virtual BrushRef _c_simple_brush(Coord width);
    Brush_tmp dither_brush(Coord width, Long pattern) {
        return _c_dither_brush(width, pattern);
    }
    virtual BrushRef _c_dither_brush(Coord width, Long pattern);
    Brush_tmp patterned_brush(Coord width, const DrawingKit::Data& pattern) {
        return _c_patterned_brush(width, pattern);
    }
    virtual BrushRef _c_patterned_brush(Coord width, const DrawingKit::Data& pattern);
    Color_tmp color_rgb(Color::Intensity r, Color::Intensity g, Color::Intensity b) {
        return _c_color_rgb(r, g, b);
    }
    virtual ColorRef _c_color_rgb(Color::Intensity r, Color::Intensity g, Color::Intensity b);
    Color_tmp find_color(CharString_in name) {
        return _c_find_color(name);
    }
    virtual ColorRef _c_find_color(CharString_in name);
    Color_tmp resolve_color(StyleObj_in s, CharString_in name) {
        return _c_resolve_color(s, name);
    }
    virtual ColorRef _c_resolve_color(StyleObj_in s, CharString_in name);
    Color_tmp foreground(StyleObj_in s) {
        return _c_foreground(s);
    }
    virtual ColorRef _c_foreground(StyleObj_in s);
    Color_tmp background(StyleObj_in s) {
        return _c_background(s);
    }
    virtual ColorRef _c_background(StyleObj_in s);
    Font_tmp find_font(CharString_in fullname) {
        return _c_find_font(fullname);
    }
    virtual FontRef _c_find_font(CharString_in fullname);
    Font_tmp find_font_match(CharString_in family, CharString_in style, Coord ptsize) {
        return _c_find_font_match(family, style, ptsize);
    }
    virtual FontRef _c_find_font_match(CharString_in family, CharString_in style, Coord ptsize);
    Font_tmp resolve_font(StyleObj_in s, CharString_in name) {
        return _c_resolve_font(s, name);
    }
    virtual FontRef _c_resolve_font(StyleObj_in s, CharString_in name);
    Font_tmp default_font(StyleObj_in s) {
        return _c_default_font(s);
    }
    virtual FontRef _c_default_font(StyleObj_in s);
    Raster_tmp bitmap_file(CharString_in filename) {
        return _c_bitmap_file(filename);
    }
    virtual RasterRef _c_bitmap_file(CharString_in filename);
    Raster_tmp bitmap_data(const DrawingKit::Data& data, Raster::Index rows, Raster::Index columns, Raster::Index origin_row, Raster::Index origin_column) {
        return _c_bitmap_data(data, rows, columns, origin_row, origin_column);
    }
    virtual RasterRef _c_bitmap_data(const DrawingKit::Data& data, Raster::Index rows, Raster::Index columns, Raster::Index origin_row, Raster::Index origin_column);
    Raster_tmp bitmap_char(Font_in f, CharCode c) {
        return _c_bitmap_char(f, c);
    }
    virtual RasterRef _c_bitmap_char(Font_in f, CharCode c);
    Raster_tmp raster_tiff(CharString_in filename) {
        return _c_raster_tiff(filename);
    }
    virtual RasterRef _c_raster_tiff(CharString_in filename);
    TransformObj_tmp transform(TransformObj::Matrix m) {
        return _c_transform(m);
    }
    virtual TransformObjRef _c_transform(TransformObj::Matrix m);
    TransformObj_tmp identity_transform() {
        return _c_identity_transform();
    }
    virtual TransformObjRef _c_identity_transform();
    PainterObj_tmp printer(CharString_in output) {
        return _c_printer(output);
    }
    virtual PainterObjRef _c_printer(CharString_in output);
    DrawingKitRef _obj() { return this; }
    void* _this();
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

inline BrushRef Brush::_duplicate(BrushRef obj) {
    return (BrushRef)_BaseObject__duplicate(obj, &BrushStub::_create);
}
inline Brush& Brush::operator =(BrushRef p) {
    _BaseObject__release(_obj_);
    _obj_ = Brush::_duplicate(p);
    return *this;
}
inline Brush::Brush(const Brush& r) {
    _obj_ = Brush::_duplicate(r._obj_);
}
inline Brush& Brush::operator =(const Brush& r) {
    _BaseObject__release(_obj_);
    _obj_ = Brush::_duplicate(r._obj_);
    return *this;
}
inline Brush::Brush(const Brush_tmp& r) {
    _obj_ = r._obj_;
    ((Brush_tmp*)&r)->_obj_ = 0;
}
inline Brush& Brush::operator =(const Brush_tmp& r) {
    _BaseObject__release(_obj_);
    _obj_ = r._obj_;
    ((Brush_tmp*)&r)->_obj_ = 0;
    return *this;
}
inline Brush::Brush(const Brush_var& e) {
    _obj_ = Brush::_duplicate(e._obj_);
}
inline Brush& Brush::operator =(const Brush_var& e) {
    _BaseObject__release(_obj_);
    _obj_ = Brush::_duplicate(e._obj_);
    return *this;
}
inline Brush::~Brush() {
    _BaseObject__release(_obj_);
}
inline Brush_tmp Brush::_narrow(const BaseObject& r) {
    return _narrow(r._obj_);
}
inline Brush_tmp Brush::_duplicate(const Brush& r) {
    return _duplicate(r._obj_);
}
inline Brush::operator FrescoObject() const {
    return FrescoObject_tmp((FrescoObjectRef)_BaseObject__duplicate((FrescoObjectRef)_obj_, &FrescoObjectStub::_create));
}
inline Brush_tmp::Brush_tmp(const Brush& r) {
    _obj_ = Brush::_duplicate(r._obj_);
}
inline Brush_tmp::Brush_tmp(const Brush_tmp& r) {
    _obj_ = r._obj_;
    ((Brush_tmp*)&r)->_obj_ = 0;
}
inline Brush_tmp::~Brush_tmp() { }

inline ColorRef Color::_duplicate(ColorRef obj) {
    return (ColorRef)_BaseObject__duplicate(obj, &ColorStub::_create);
}
inline Color& Color::operator =(ColorRef p) {
    _BaseObject__release(_obj_);
    _obj_ = Color::_duplicate(p);
    return *this;
}
inline Color::Color(const Color& r) {
    _obj_ = Color::_duplicate(r._obj_);
}
inline Color& Color::operator =(const Color& r) {
    _BaseObject__release(_obj_);
    _obj_ = Color::_duplicate(r._obj_);
    return *this;
}
inline Color::Color(const Color_tmp& r) {
    _obj_ = r._obj_;
    ((Color_tmp*)&r)->_obj_ = 0;
}
inline Color& Color::operator =(const Color_tmp& r) {
    _BaseObject__release(_obj_);
    _obj_ = r._obj_;
    ((Color_tmp*)&r)->_obj_ = 0;
    return *this;
}
inline Color::Color(const Color_var& e) {
    _obj_ = Color::_duplicate(e._obj_);
}
inline Color& Color::operator =(const Color_var& e) {
    _BaseObject__release(_obj_);
    _obj_ = Color::_duplicate(e._obj_);
    return *this;
}
inline Color::~Color() {
    _BaseObject__release(_obj_);
}
inline Color_tmp Color::_narrow(const BaseObject& r) {
    return _narrow(r._obj_);
}
inline Color_tmp Color::_duplicate(const Color& r) {
    return _duplicate(r._obj_);
}
inline Color::operator FrescoObject() const {
    return FrescoObject_tmp((FrescoObjectRef)_BaseObject__duplicate((FrescoObjectRef)_obj_, &FrescoObjectStub::_create));
}
inline Color_tmp::Color_tmp(const Color& r) {
    _obj_ = Color::_duplicate(r._obj_);
}
inline Color_tmp::Color_tmp(const Color_tmp& r) {
    _obj_ = r._obj_;
    ((Color_tmp*)&r)->_obj_ = 0;
}
inline Color_tmp::~Color_tmp() { }

inline FontRef Font::_duplicate(FontRef obj) {
    return (FontRef)_BaseObject__duplicate(obj, &FontStub::_create);
}
inline Font& Font::operator =(FontRef p) {
    _BaseObject__release(_obj_);
    _obj_ = Font::_duplicate(p);
    return *this;
}
inline Font::Font(const Font& r) {
    _obj_ = Font::_duplicate(r._obj_);
}
inline Font& Font::operator =(const Font& r) {
    _BaseObject__release(_obj_);
    _obj_ = Font::_duplicate(r._obj_);
    return *this;
}
inline Font::Font(const Font_tmp& r) {
    _obj_ = r._obj_;
    ((Font_tmp*)&r)->_obj_ = 0;
}
inline Font& Font::operator =(const Font_tmp& r) {
    _BaseObject__release(_obj_);
    _obj_ = r._obj_;
    ((Font_tmp*)&r)->_obj_ = 0;
    return *this;
}
inline Font::Font(const Font_var& e) {
    _obj_ = Font::_duplicate(e._obj_);
}
inline Font& Font::operator =(const Font_var& e) {
    _BaseObject__release(_obj_);
    _obj_ = Font::_duplicate(e._obj_);
    return *this;
}
inline Font::~Font() {
    _BaseObject__release(_obj_);
}
inline Font_tmp Font::_narrow(const BaseObject& r) {
    return _narrow(r._obj_);
}
inline Font_tmp Font::_duplicate(const Font& r) {
    return _duplicate(r._obj_);
}
inline Font::operator FrescoObject() const {
    return FrescoObject_tmp((FrescoObjectRef)_BaseObject__duplicate((FrescoObjectRef)_obj_, &FrescoObjectStub::_create));
}
inline Font_tmp::Font_tmp(const Font& r) {
    _obj_ = Font::_duplicate(r._obj_);
}
inline Font_tmp::Font_tmp(const Font_tmp& r) {
    _obj_ = r._obj_;
    ((Font_tmp*)&r)->_obj_ = 0;
}
inline Font_tmp::~Font_tmp() { }

inline RasterRef Raster::_duplicate(RasterRef obj) {
    return (RasterRef)_BaseObject__duplicate(obj, &RasterStub::_create);
}
inline Raster& Raster::operator =(RasterRef p) {
    _BaseObject__release(_obj_);
    _obj_ = Raster::_duplicate(p);
    return *this;
}
inline Raster::Raster(const Raster& r) {
    _obj_ = Raster::_duplicate(r._obj_);
}
inline Raster& Raster::operator =(const Raster& r) {
    _BaseObject__release(_obj_);
    _obj_ = Raster::_duplicate(r._obj_);
    return *this;
}
inline Raster::Raster(const Raster_tmp& r) {
    _obj_ = r._obj_;
    ((Raster_tmp*)&r)->_obj_ = 0;
}
inline Raster& Raster::operator =(const Raster_tmp& r) {
    _BaseObject__release(_obj_);
    _obj_ = r._obj_;
    ((Raster_tmp*)&r)->_obj_ = 0;
    return *this;
}
inline Raster::Raster(const Raster_var& e) {
    _obj_ = Raster::_duplicate(e._obj_);
}
inline Raster& Raster::operator =(const Raster_var& e) {
    _BaseObject__release(_obj_);
    _obj_ = Raster::_duplicate(e._obj_);
    return *this;
}
inline Raster::~Raster() {
    _BaseObject__release(_obj_);
}
inline Raster_tmp Raster::_narrow(const BaseObject& r) {
    return _narrow(r._obj_);
}
inline Raster_tmp Raster::_duplicate(const Raster& r) {
    return _duplicate(r._obj_);
}
inline Raster::operator FrescoObject() const {
    return FrescoObject_tmp((FrescoObjectRef)_BaseObject__duplicate((FrescoObjectRef)_obj_, &FrescoObjectStub::_create));
}
inline Raster_tmp::Raster_tmp(const Raster& r) {
    _obj_ = Raster::_duplicate(r._obj_);
}
inline Raster_tmp::Raster_tmp(const Raster_tmp& r) {
    _obj_ = r._obj_;
    ((Raster_tmp*)&r)->_obj_ = 0;
}
inline Raster_tmp::~Raster_tmp() { }

inline PainterObjRef PainterObj::_duplicate(PainterObjRef obj) {
    return (PainterObjRef)_BaseObject__duplicate(obj, &PainterObjStub::_create);
}
inline PainterObj& PainterObj::operator =(PainterObjRef p) {
    _BaseObject__release(_obj_);
    _obj_ = PainterObj::_duplicate(p);
    return *this;
}
inline PainterObj::PainterObj(const PainterObj& r) {
    _obj_ = PainterObj::_duplicate(r._obj_);
}
inline PainterObj& PainterObj::operator =(const PainterObj& r) {
    _BaseObject__release(_obj_);
    _obj_ = PainterObj::_duplicate(r._obj_);
    return *this;
}
inline PainterObj::PainterObj(const PainterObj_tmp& r) {
    _obj_ = r._obj_;
    ((PainterObj_tmp*)&r)->_obj_ = 0;
}
inline PainterObj& PainterObj::operator =(const PainterObj_tmp& r) {
    _BaseObject__release(_obj_);
    _obj_ = r._obj_;
    ((PainterObj_tmp*)&r)->_obj_ = 0;
    return *this;
}
inline PainterObj::PainterObj(const PainterObj_var& e) {
    _obj_ = PainterObj::_duplicate(e._obj_);
}
inline PainterObj& PainterObj::operator =(const PainterObj_var& e) {
    _BaseObject__release(_obj_);
    _obj_ = PainterObj::_duplicate(e._obj_);
    return *this;
}
inline PainterObj::~PainterObj() {
    _BaseObject__release(_obj_);
}
inline PainterObj_tmp PainterObj::_narrow(const BaseObject& r) {
    return _narrow(r._obj_);
}
inline PainterObj_tmp PainterObj::_duplicate(const PainterObj& r) {
    return _duplicate(r._obj_);
}
inline PainterObj::operator FrescoObject() const {
    return FrescoObject_tmp((FrescoObjectRef)_BaseObject__duplicate((FrescoObjectRef)_obj_, &FrescoObjectStub::_create));
}
inline PainterObj_tmp::PainterObj_tmp(const PainterObj& r) {
    _obj_ = PainterObj::_duplicate(r._obj_);
}
inline PainterObj_tmp::PainterObj_tmp(const PainterObj_tmp& r) {
    _obj_ = r._obj_;
    ((PainterObj_tmp*)&r)->_obj_ = 0;
}
inline PainterObj_tmp::~PainterObj_tmp() { }

inline DrawingKitRef DrawingKit::_duplicate(DrawingKitRef obj) {
    return (DrawingKitRef)_BaseObject__duplicate(obj, &DrawingKitStub::_create);
}
inline DrawingKit& DrawingKit::operator =(DrawingKitRef p) {
    _BaseObject__release(_obj_);
    _obj_ = DrawingKit::_duplicate(p);
    return *this;
}
inline DrawingKit::DrawingKit(const DrawingKit& r) {
    _obj_ = DrawingKit::_duplicate(r._obj_);
}
inline DrawingKit& DrawingKit::operator =(const DrawingKit& r) {
    _BaseObject__release(_obj_);
    _obj_ = DrawingKit::_duplicate(r._obj_);
    return *this;
}
inline DrawingKit::DrawingKit(const DrawingKit_tmp& r) {
    _obj_ = r._obj_;
    ((DrawingKit_tmp*)&r)->_obj_ = 0;
}
inline DrawingKit& DrawingKit::operator =(const DrawingKit_tmp& r) {
    _BaseObject__release(_obj_);
    _obj_ = r._obj_;
    ((DrawingKit_tmp*)&r)->_obj_ = 0;
    return *this;
}
inline DrawingKit::DrawingKit(const DrawingKit_var& e) {
    _obj_ = DrawingKit::_duplicate(e._obj_);
}
inline DrawingKit& DrawingKit::operator =(const DrawingKit_var& e) {
    _BaseObject__release(_obj_);
    _obj_ = DrawingKit::_duplicate(e._obj_);
    return *this;
}
inline DrawingKit::~DrawingKit() {
    _BaseObject__release(_obj_);
}
inline DrawingKit_tmp DrawingKit::_narrow(const BaseObject& r) {
    return _narrow(r._obj_);
}
inline DrawingKit_tmp DrawingKit::_duplicate(const DrawingKit& r) {
    return _duplicate(r._obj_);
}
inline DrawingKit::operator FrescoObject() const {
    return FrescoObject_tmp((FrescoObjectRef)_BaseObject__duplicate((FrescoObjectRef)_obj_, &FrescoObjectStub::_create));
}
inline DrawingKit_tmp::DrawingKit_tmp(const DrawingKit& r) {
    _obj_ = DrawingKit::_duplicate(r._obj_);
}
inline DrawingKit_tmp::DrawingKit_tmp(const DrawingKit_tmp& r) {
    _obj_ = r._obj_;
    ((DrawingKit_tmp*)&r)->_obj_ = 0;
}
inline DrawingKit_tmp::~DrawingKit_tmp() { }

#endif
