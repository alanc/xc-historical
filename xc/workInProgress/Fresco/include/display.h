/* DO NOT EDIT -- Automatically generated from Interfaces/display.idl */

#ifndef Interfaces_display_h
#define Interfaces_display_h

#include <X11/Fresco/_enter.h>
#include <X11/Fresco/drawing.h>

class ColorType;
typedef ColorType* ColorRef;
typedef ColorRef Color_in;
class Color;
class Color_tmp;
class Color_var;

class CursorType;
typedef CursorType* CursorRef;
typedef CursorRef Cursor_in;
class Cursor;
class Cursor_tmp;
class Cursor_var;

class EventType;
typedef EventType* EventRef;
typedef EventRef Event_in;
class Event;
class Event_tmp;
class Event_var;

class FontType;
typedef FontType* FontRef;
typedef FontRef Font_in;
class Font;
class Font_tmp;
class Font_var;

class GlyphTraversalType;
typedef GlyphTraversalType* GlyphTraversalRef;
typedef GlyphTraversalRef GlyphTraversal_in;
class GlyphTraversal;
class GlyphTraversal_tmp;
class GlyphTraversal_var;

class RasterType;
typedef RasterType* RasterRef;
typedef RasterRef Raster_in;
class Raster;
class Raster_tmp;
class Raster_var;

class ScreenObjType;
typedef ScreenObjType* ScreenObjRef;
typedef ScreenObjRef ScreenObj_in;
class ScreenObj;
class ScreenObj_tmp;
class ScreenObj_var;

class ViewerType;
typedef ViewerType* ViewerRef;
typedef ViewerRef Viewer_in;
class Viewer;
class Viewer_tmp;
class Viewer_var;

class WindowType;
typedef WindowType* WindowRef;
typedef WindowRef Window_in;
class Window;
class Window_tmp;
class Window_var;

class WindowStyleType;
typedef WindowStyleType* WindowStyleRef;
typedef WindowStyleRef WindowStyle_in;
class WindowStyle;
class WindowStyle_tmp;
class WindowStyle_var;

class Cursor {
public:
    CursorRef _obj_;

    Cursor() { _obj_ = 0; }
    Cursor(CursorRef p) { _obj_ = p; }
    Cursor& operator =(CursorRef p);
    Cursor(const Cursor&);
    Cursor& operator =(const Cursor& r);
    Cursor(const Cursor_tmp&);
    Cursor& operator =(const Cursor_tmp&);
    Cursor(const Cursor_var&);
    Cursor& operator =(const Cursor_var&);
    ~Cursor();

    CursorRef operator ->() { return _obj_; }

    operator Cursor_in() const { return _obj_; }
    operator FrescoObject() const;
    static CursorRef _narrow(BaseObjectRef p);
    static Cursor_tmp _narrow(const BaseObject& r);

    static CursorRef _duplicate(CursorRef obj);
    static Cursor_tmp _duplicate(const Cursor& r);
};

class Cursor_tmp : public Cursor {
public:
    Cursor_tmp(CursorRef p) { _obj_ = p; }
    Cursor_tmp(const Cursor& r);
    Cursor_tmp(const Cursor_tmp& r);
    ~Cursor_tmp();
};

class Cursor_var {
public:
    CursorRef _obj_;

    Cursor_var(CursorRef p) { _obj_ = p; }
    operator CursorRef() const { return _obj_; }
    CursorRef operator ->() { return _obj_; }
};

class CursorType : public FrescoObjectType {
protected:
    CursorType();
    virtual ~CursorType();
public:
    CursorRef _obj() { return this; }
    void* _this();
    virtual TypeObjId _tid();
};

class CursorStub : public CursorType {
public:
    CursorStub(Exchange*);
    ~CursorStub();

    static BaseObjectRef _create(Exchange*);
    Exchange* _exchange();
protected:
    Exchange* exch_;
};

class DisplayStyleType;
typedef DisplayStyleType* DisplayStyleRef;
typedef DisplayStyleRef DisplayStyle_in;
class DisplayStyle;
class DisplayStyle_tmp;
class DisplayStyle_var;

class DisplayObjType;
typedef DisplayObjType* DisplayObjRef;
typedef DisplayObjRef DisplayObj_in;
class DisplayObj;
class DisplayObj_tmp;
class DisplayObj_var;

class DisplayObj {
public:
    DisplayObjRef _obj_;

    DisplayObj() { _obj_ = 0; }
    DisplayObj(DisplayObjRef p) { _obj_ = p; }
    DisplayObj& operator =(DisplayObjRef p);
    DisplayObj(const DisplayObj&);
    DisplayObj& operator =(const DisplayObj& r);
    DisplayObj(const DisplayObj_tmp&);
    DisplayObj& operator =(const DisplayObj_tmp&);
    DisplayObj(const DisplayObj_var&);
    DisplayObj& operator =(const DisplayObj_var&);
    ~DisplayObj();

    DisplayObjRef operator ->() { return _obj_; }

    operator DisplayObj_in() const { return _obj_; }
    operator FrescoObject() const;
    static DisplayObjRef _narrow(BaseObjectRef p);
    static DisplayObj_tmp _narrow(const BaseObject& r);

    static DisplayObjRef _duplicate(DisplayObjRef obj);
    static DisplayObj_tmp _duplicate(const DisplayObj& r);
    typedef Long ScreenNumber;
};

class DisplayObj_tmp : public DisplayObj {
public:
    DisplayObj_tmp(DisplayObjRef p) { _obj_ = p; }
    DisplayObj_tmp(const DisplayObj& r);
    DisplayObj_tmp(const DisplayObj_tmp& r);
    ~DisplayObj_tmp();
};

class DisplayObj_var {
public:
    DisplayObjRef _obj_;

    DisplayObj_var(DisplayObjRef p) { _obj_ = p; }
    operator DisplayObjRef() const { return _obj_; }
    DisplayObjRef operator ->() { return _obj_; }
};

class DisplayObjType : public FrescoObjectType {
protected:
    DisplayObjType();
    virtual ~DisplayObjType();
public:
    DisplayStyle_tmp style();
    virtual DisplayStyleRef _c_style();
    DrawingKit_tmp drawing_kit() {
        return _c_drawing_kit();
    }
    virtual DrawingKitRef _c_drawing_kit();
    ScreenObj_tmp screen(DisplayObj::ScreenNumber n);
    virtual ScreenObjRef _c_screen(DisplayObj::ScreenNumber n);
    virtual DisplayObj::ScreenNumber number_of_screens();
    ScreenObj_tmp default_screen();
    virtual ScreenObjRef _c_default_screen();
    Cursor_tmp cursor_from_data(Short x, Short y, Long pattern[16], Long mask[16]) {
        return _c_cursor_from_data(x, y, pattern, mask);
    }
    virtual CursorRef _c_cursor_from_data(Short x, Short y, Long pattern[16], Long mask[16]);
    Cursor_tmp cursor_from_bitmap(Raster_in b, Raster_in mask) {
        return _c_cursor_from_bitmap(b, mask);
    }
    virtual CursorRef _c_cursor_from_bitmap(Raster_in b, Raster_in mask);
    Cursor_tmp cursor_from_font(Font_in f, Long pattern, Long mask) {
        return _c_cursor_from_font(f, pattern, mask);
    }
    virtual CursorRef _c_cursor_from_font(Font_in f, Long pattern, Long mask);
    Cursor_tmp cursor_from_index(Long index) {
        return _c_cursor_from_index(index);
    }
    virtual CursorRef _c_cursor_from_index(Long index);
    virtual void run(Boolean b);
    virtual Boolean running();
    virtual Tag add_filter(GlyphTraversal_in t);
    virtual void remove_filter(Tag add_tag);
    virtual void need_repair(Window_in w);
    virtual void repair();
    virtual void flush();
    virtual void flush_and_wait();
    virtual void ring_bell(Float pct_loudness);
    virtual void close();
    DisplayObjRef _obj() { return this; }
    void* _this();
    virtual TypeObjId _tid();
};

class DisplayObjStub : public DisplayObjType {
public:
    DisplayObjStub(Exchange*);
    ~DisplayObjStub();

    static BaseObjectRef _create(Exchange*);
    Exchange* _exchange();
protected:
    Exchange* exch_;
};

class DisplayStyle {
public:
    DisplayStyleRef _obj_;

    DisplayStyle() { _obj_ = 0; }
    DisplayStyle(DisplayStyleRef p) { _obj_ = p; }
    DisplayStyle& operator =(DisplayStyleRef p);
    DisplayStyle(const DisplayStyle&);
    DisplayStyle& operator =(const DisplayStyle& r);
    DisplayStyle(const DisplayStyle_tmp&);
    DisplayStyle& operator =(const DisplayStyle_tmp&);
    DisplayStyle(const DisplayStyle_var&);
    DisplayStyle& operator =(const DisplayStyle_var&);
    ~DisplayStyle();

    DisplayStyleRef operator ->() { return _obj_; }

    operator DisplayStyle_in() const { return _obj_; }
    operator StyleObj() const;
    operator FrescoObject() const;
    static DisplayStyleRef _narrow(BaseObjectRef p);
    static DisplayStyle_tmp _narrow(const BaseObject& r);

    static DisplayStyleRef _duplicate(DisplayStyleRef obj);
    static DisplayStyle_tmp _duplicate(const DisplayStyle& r);
};

class DisplayStyle_tmp : public DisplayStyle {
public:
    DisplayStyle_tmp(DisplayStyleRef p) { _obj_ = p; }
    DisplayStyle_tmp(const DisplayStyle& r);
    DisplayStyle_tmp(const DisplayStyle_tmp& r);
    ~DisplayStyle_tmp();
};

class DisplayStyle_var {
public:
    DisplayStyleRef _obj_;

    DisplayStyle_var(DisplayStyleRef p) { _obj_ = p; }
    operator DisplayStyleRef() const { return _obj_; }
    DisplayStyleRef operator ->() { return _obj_; }
};

class DisplayStyleType : public StyleObjType {
protected:
    DisplayStyleType();
    virtual ~DisplayStyleType();
public:
    virtual Boolean auto_repeat();
    virtual void auto_repeat(Boolean _p);
    virtual Float key_click_volume();
    virtual void key_click_volume(Float _p);
    virtual Float pointer_acceleration();
    virtual void pointer_acceleration(Float _p);
    virtual Long pointer_threshold();
    virtual void pointer_threshold(Long _p);
    DisplayStyleRef _obj() { return this; }
    void* _this();
    virtual TypeObjId _tid();
};

class DisplayStyleStub : public DisplayStyleType {
public:
    DisplayStyleStub(Exchange*);
    ~DisplayStyleStub();

    static BaseObjectRef _create(Exchange*);
    Exchange* _exchange();
protected:
    Exchange* exch_;
};

class ScreenObj {
public:
    ScreenObjRef _obj_;

    ScreenObj() { _obj_ = 0; }
    ScreenObj(ScreenObjRef p) { _obj_ = p; }
    ScreenObj& operator =(ScreenObjRef p);
    ScreenObj(const ScreenObj&);
    ScreenObj& operator =(const ScreenObj& r);
    ScreenObj(const ScreenObj_tmp&);
    ScreenObj& operator =(const ScreenObj_tmp&);
    ScreenObj(const ScreenObj_var&);
    ScreenObj& operator =(const ScreenObj_var&);
    ~ScreenObj();

    ScreenObjRef operator ->() { return _obj_; }

    operator ScreenObj_in() const { return _obj_; }
    operator FrescoObject() const;
    static ScreenObjRef _narrow(BaseObjectRef p);
    static ScreenObj_tmp _narrow(const BaseObject& r);

    static ScreenObjRef _duplicate(ScreenObjRef obj);
    static ScreenObj_tmp _duplicate(const ScreenObj& r);
};

class ScreenObj_tmp : public ScreenObj {
public:
    ScreenObj_tmp(ScreenObjRef p) { _obj_ = p; }
    ScreenObj_tmp(const ScreenObj& r);
    ScreenObj_tmp(const ScreenObj_tmp& r);
    ~ScreenObj_tmp();
};

class ScreenObj_var {
public:
    ScreenObjRef _obj_;

    ScreenObj_var(ScreenObjRef p) { _obj_ = p; }
    operator ScreenObjRef() const { return _obj_; }
    ScreenObjRef operator ->() { return _obj_; }
};

class ScreenObjType : public FrescoObjectType {
protected:
    ScreenObjType();
    virtual ~ScreenObjType();
public:
    virtual Coord dpi();
    virtual void dpi(Coord _p);
    DisplayObj_tmp display() {
        return _c_display();
    }
    virtual DisplayObjRef _c_display();
    virtual Coord width();
    virtual Coord height();
    virtual PixelCoord to_pixels(Coord c);
    virtual Coord to_coord(PixelCoord p);
    virtual Coord to_pixels_coord(Coord c);
    virtual void move_pointer(Coord x, Coord y);
    Window_tmp application(Viewer_in v);
    virtual WindowRef _c_application(Viewer_in v);
    Window_tmp top_level(Viewer_in v, Window_in group_leader);
    virtual WindowRef _c_top_level(Viewer_in v, Window_in group_leader);
    Window_tmp transient(Viewer_in v, Window_in transient_for);
    virtual WindowRef _c_transient(Viewer_in v, Window_in transient_for);
    Window_tmp popup(Viewer_in v);
    virtual WindowRef _c_popup(Viewer_in v);
    Window_tmp icon(Viewer_in v);
    virtual WindowRef _c_icon(Viewer_in v);
    ScreenObjRef _obj() { return this; }
    void* _this();
    virtual TypeObjId _tid();
};

class ScreenObjStub : public ScreenObjType {
public:
    ScreenObjStub(Exchange*);
    ~ScreenObjStub();

    static BaseObjectRef _create(Exchange*);
    Exchange* _exchange();
protected:
    Exchange* exch_;
};

class Window {
public:
    WindowRef _obj_;

    Window() { _obj_ = 0; }
    Window(WindowRef p) { _obj_ = p; }
    Window& operator =(WindowRef p);
    Window(const Window&);
    Window& operator =(const Window& r);
    Window(const Window_tmp&);
    Window& operator =(const Window_tmp&);
    Window(const Window_var&);
    Window& operator =(const Window_var&);
    ~Window();

    WindowRef operator ->() { return _obj_; }

    operator Window_in() const { return _obj_; }
    operator FrescoObject() const;
    static WindowRef _narrow(BaseObjectRef p);
    static Window_tmp _narrow(const BaseObject& r);

    static WindowRef _duplicate(WindowRef obj);
    static Window_tmp _duplicate(const Window& r);
    struct Placement {
        Coord x, y, width, height;
        Alignment align_x, align_y;
    };
};

class Window_tmp : public Window {
public:
    Window_tmp(WindowRef p) { _obj_ = p; }
    Window_tmp(const Window& r);
    Window_tmp(const Window_tmp& r);
    ~Window_tmp();
};

class Window_var {
public:
    WindowRef _obj_;

    Window_var(WindowRef p) { _obj_ = p; }
    operator WindowRef() const { return _obj_; }
    WindowRef operator ->() { return _obj_; }
};

class WindowType : public FrescoObjectType {
protected:
    WindowType();
    virtual ~WindowType();
public:
    ScreenObj_tmp screen() {
        return _c_screen();
    }
    virtual ScreenObjRef _c_screen();
    WindowStyle_tmp style();
    virtual WindowStyleRef _c_style();
    Viewer_tmp main_viewer();
    virtual ViewerRef _c_main_viewer();
    virtual void configure(const Window::Placement& p);
    virtual void get_configuration(Boolean position, Window::Placement& p);
    virtual void configure_notify(Coord width, Coord height);
    virtual void move_notify(Coord left, Coord bottom);
    virtual void map();
    virtual void unmap();
    virtual Boolean is_mapped();
    virtual void map_notify();
    virtual void unmap_notify();
    virtual void iconify();
    virtual void deiconify();
    virtual void raise();
    virtual void lower();
    virtual void redraw(Coord left, Coord bottom, Coord width, Coord height);
    virtual void repair();
    virtual void handle_event(Event_in e);
    virtual void grab_pointer(Cursor_in c);
    virtual void ungrab_pointer();
    WindowRef _obj() { return this; }
    void* _this();
    virtual TypeObjId _tid();
};

class WindowStub : public WindowType {
public:
    WindowStub(Exchange*);
    ~WindowStub();

    static BaseObjectRef _create(Exchange*);
    Exchange* _exchange();
protected:
    Exchange* exch_;
};

class WindowStyle {
public:
    WindowStyleRef _obj_;

    WindowStyle() { _obj_ = 0; }
    WindowStyle(WindowStyleRef p) { _obj_ = p; }
    WindowStyle& operator =(WindowStyleRef p);
    WindowStyle(const WindowStyle&);
    WindowStyle& operator =(const WindowStyle& r);
    WindowStyle(const WindowStyle_tmp&);
    WindowStyle& operator =(const WindowStyle_tmp&);
    WindowStyle(const WindowStyle_var&);
    WindowStyle& operator =(const WindowStyle_var&);
    ~WindowStyle();

    WindowStyleRef operator ->() { return _obj_; }

    operator WindowStyle_in() const { return _obj_; }
    operator StyleObj() const;
    operator FrescoObject() const;
    static WindowStyleRef _narrow(BaseObjectRef p);
    static WindowStyle_tmp _narrow(const BaseObject& r);

    static WindowStyleRef _duplicate(WindowStyleRef obj);
    static WindowStyle_tmp _duplicate(const WindowStyle& r);
};

class WindowStyle_tmp : public WindowStyle {
public:
    WindowStyle_tmp(WindowStyleRef p) { _obj_ = p; }
    WindowStyle_tmp(const WindowStyle& r);
    WindowStyle_tmp(const WindowStyle_tmp& r);
    ~WindowStyle_tmp();
};

class WindowStyle_var {
public:
    WindowStyleRef _obj_;

    WindowStyle_var(WindowStyleRef p) { _obj_ = p; }
    operator WindowStyleRef() const { return _obj_; }
    WindowStyleRef operator ->() { return _obj_; }
};

class WindowStyleType : public StyleObjType {
protected:
    WindowStyleType();
    virtual ~WindowStyleType();
public:
    virtual Boolean double_buffered();
    virtual void double_buffered(Boolean _p);
    Cursor_tmp default_cursor() {
        return _c_default_cursor();
    }
    virtual CursorRef _c_default_cursor();
    void default_cursor(Cursor_in _p) {
        _c_default_cursor(_p);
    }
    virtual void _c_default_cursor(Cursor_in _p);
    Color_tmp cursor_foreground() {
        return _c_cursor_foreground();
    }
    virtual ColorRef _c_cursor_foreground();
    void cursor_foreground(Color_in _p) {
        _c_cursor_foreground(_p);
    }
    virtual void _c_cursor_foreground(Color_in _p);
    Color_tmp cursor_background() {
        return _c_cursor_background();
    }
    virtual ColorRef _c_cursor_background();
    void cursor_background(Color_in _p) {
        _c_cursor_background(_p);
    }
    virtual void _c_cursor_background(Color_in _p);
    CharString_tmp geometry() {
        return _c_geometry();
    }
    virtual CharStringRef _c_geometry();
    void geometry(CharString_in _p) {
        _c_geometry(_p);
    }
    virtual void _c_geometry(CharString_in _p);
    Window_tmp icon() {
        return _c_icon();
    }
    virtual WindowRef _c_icon();
    void icon(Window_in _p) {
        _c_icon(_p);
    }
    virtual void _c_icon(Window_in _p);
    Raster_tmp icon_bitmap() {
        return _c_icon_bitmap();
    }
    virtual RasterRef _c_icon_bitmap();
    void icon_bitmap(Raster_in _p) {
        _c_icon_bitmap(_p);
    }
    virtual void _c_icon_bitmap(Raster_in _p);
    Raster_tmp icon_mask() {
        return _c_icon_mask();
    }
    virtual RasterRef _c_icon_mask();
    void icon_mask(Raster_in _p) {
        _c_icon_mask(_p);
    }
    virtual void _c_icon_mask(Raster_in _p);
    CharString_tmp icon_name() {
        return _c_icon_name();
    }
    virtual CharStringRef _c_icon_name();
    void icon_name(CharString_in _p) {
        _c_icon_name(_p);
    }
    virtual void _c_icon_name(CharString_in _p);
    CharString_tmp icon_geometry() {
        return _c_icon_geometry();
    }
    virtual CharStringRef _c_icon_geometry();
    void icon_geometry(CharString_in _p) {
        _c_icon_geometry(_p);
    }
    virtual void _c_icon_geometry(CharString_in _p);
    virtual Boolean iconic();
    virtual void iconic(Boolean _p);
    CharString_tmp title() {
        return _c_title();
    }
    virtual CharStringRef _c_title();
    void title(CharString_in _p) {
        _c_title(_p);
    }
    virtual void _c_title(CharString_in _p);
    virtual Long xor_pixel();
    virtual void xor_pixel(Long _p);
    WindowStyleRef _obj() { return this; }
    void* _this();
    virtual TypeObjId _tid();
};

class WindowStyleStub : public WindowStyleType {
public:
    WindowStyleStub(Exchange*);
    ~WindowStyleStub();

    static BaseObjectRef _create(Exchange*);
    Exchange* _exchange();
protected:
    Exchange* exch_;
};

inline CursorRef Cursor::_duplicate(CursorRef obj) {
    return (CursorRef)_BaseObject__duplicate(obj, &CursorStub::_create);
}
inline Cursor& Cursor::operator =(CursorRef p) {
    _BaseObject__release(_obj_);
    _obj_ = Cursor::_duplicate(p);
    return *this;
}
inline Cursor::Cursor(const Cursor& r) {
    _obj_ = Cursor::_duplicate(r._obj_);
}
inline Cursor& Cursor::operator =(const Cursor& r) {
    _BaseObject__release(_obj_);
    _obj_ = Cursor::_duplicate(r._obj_);
    return *this;
}
inline Cursor::Cursor(const Cursor_tmp& r) {
    _obj_ = r._obj_;
    ((Cursor_tmp*)&r)->_obj_ = 0;
}
inline Cursor& Cursor::operator =(const Cursor_tmp& r) {
    _BaseObject__release(_obj_);
    _obj_ = r._obj_;
    ((Cursor_tmp*)&r)->_obj_ = 0;
    return *this;
}
inline Cursor::Cursor(const Cursor_var& e) {
    _obj_ = Cursor::_duplicate(e._obj_);
}
inline Cursor& Cursor::operator =(const Cursor_var& e) {
    _BaseObject__release(_obj_);
    _obj_ = Cursor::_duplicate(e._obj_);
    return *this;
}
inline Cursor::~Cursor() {
    _BaseObject__release(_obj_);
}
inline Cursor_tmp Cursor::_narrow(const BaseObject& r) {
    return _narrow(r._obj_);
}
inline Cursor_tmp Cursor::_duplicate(const Cursor& r) {
    return _duplicate(r._obj_);
}
inline Cursor::operator FrescoObject() const {
    return FrescoObject_tmp((FrescoObjectRef)_BaseObject__duplicate((FrescoObjectRef)_obj_, &FrescoObjectStub::_create));
}
inline Cursor_tmp::Cursor_tmp(const Cursor& r) {
    _obj_ = Cursor::_duplicate(r._obj_);
}
inline Cursor_tmp::Cursor_tmp(const Cursor_tmp& r) {
    _obj_ = r._obj_;
    ((Cursor_tmp*)&r)->_obj_ = 0;
}
inline Cursor_tmp::~Cursor_tmp() { }

inline DisplayObjRef DisplayObj::_duplicate(DisplayObjRef obj) {
    return (DisplayObjRef)_BaseObject__duplicate(obj, &DisplayObjStub::_create);
}
inline DisplayObj& DisplayObj::operator =(DisplayObjRef p) {
    _BaseObject__release(_obj_);
    _obj_ = DisplayObj::_duplicate(p);
    return *this;
}
inline DisplayObj::DisplayObj(const DisplayObj& r) {
    _obj_ = DisplayObj::_duplicate(r._obj_);
}
inline DisplayObj& DisplayObj::operator =(const DisplayObj& r) {
    _BaseObject__release(_obj_);
    _obj_ = DisplayObj::_duplicate(r._obj_);
    return *this;
}
inline DisplayObj::DisplayObj(const DisplayObj_tmp& r) {
    _obj_ = r._obj_;
    ((DisplayObj_tmp*)&r)->_obj_ = 0;
}
inline DisplayObj& DisplayObj::operator =(const DisplayObj_tmp& r) {
    _BaseObject__release(_obj_);
    _obj_ = r._obj_;
    ((DisplayObj_tmp*)&r)->_obj_ = 0;
    return *this;
}
inline DisplayObj::DisplayObj(const DisplayObj_var& e) {
    _obj_ = DisplayObj::_duplicate(e._obj_);
}
inline DisplayObj& DisplayObj::operator =(const DisplayObj_var& e) {
    _BaseObject__release(_obj_);
    _obj_ = DisplayObj::_duplicate(e._obj_);
    return *this;
}
inline DisplayObj::~DisplayObj() {
    _BaseObject__release(_obj_);
}
inline DisplayObj_tmp DisplayObj::_narrow(const BaseObject& r) {
    return _narrow(r._obj_);
}
inline DisplayObj_tmp DisplayObj::_duplicate(const DisplayObj& r) {
    return _duplicate(r._obj_);
}
inline DisplayObj::operator FrescoObject() const {
    return FrescoObject_tmp((FrescoObjectRef)_BaseObject__duplicate((FrescoObjectRef)_obj_, &FrescoObjectStub::_create));
}
inline DisplayObj_tmp::DisplayObj_tmp(const DisplayObj& r) {
    _obj_ = DisplayObj::_duplicate(r._obj_);
}
inline DisplayObj_tmp::DisplayObj_tmp(const DisplayObj_tmp& r) {
    _obj_ = r._obj_;
    ((DisplayObj_tmp*)&r)->_obj_ = 0;
}
inline DisplayObj_tmp::~DisplayObj_tmp() { }

inline DisplayStyleRef DisplayStyle::_duplicate(DisplayStyleRef obj) {
    return (DisplayStyleRef)_BaseObject__duplicate(obj, &DisplayStyleStub::_create);
}
inline DisplayStyle& DisplayStyle::operator =(DisplayStyleRef p) {
    _BaseObject__release(_obj_);
    _obj_ = DisplayStyle::_duplicate(p);
    return *this;
}
inline DisplayStyle::DisplayStyle(const DisplayStyle& r) {
    _obj_ = DisplayStyle::_duplicate(r._obj_);
}
inline DisplayStyle& DisplayStyle::operator =(const DisplayStyle& r) {
    _BaseObject__release(_obj_);
    _obj_ = DisplayStyle::_duplicate(r._obj_);
    return *this;
}
inline DisplayStyle::DisplayStyle(const DisplayStyle_tmp& r) {
    _obj_ = r._obj_;
    ((DisplayStyle_tmp*)&r)->_obj_ = 0;
}
inline DisplayStyle& DisplayStyle::operator =(const DisplayStyle_tmp& r) {
    _BaseObject__release(_obj_);
    _obj_ = r._obj_;
    ((DisplayStyle_tmp*)&r)->_obj_ = 0;
    return *this;
}
inline DisplayStyle::DisplayStyle(const DisplayStyle_var& e) {
    _obj_ = DisplayStyle::_duplicate(e._obj_);
}
inline DisplayStyle& DisplayStyle::operator =(const DisplayStyle_var& e) {
    _BaseObject__release(_obj_);
    _obj_ = DisplayStyle::_duplicate(e._obj_);
    return *this;
}
inline DisplayStyle::~DisplayStyle() {
    _BaseObject__release(_obj_);
}
inline DisplayStyle_tmp DisplayStyle::_narrow(const BaseObject& r) {
    return _narrow(r._obj_);
}
inline DisplayStyle_tmp DisplayStyle::_duplicate(const DisplayStyle& r) {
    return _duplicate(r._obj_);
}
inline DisplayStyle::operator StyleObj() const {
    return StyleObj_tmp((StyleObjRef)_BaseObject__duplicate((StyleObjRef)_obj_, &StyleObjStub::_create));
}
inline DisplayStyle::operator FrescoObject() const {
    return FrescoObject_tmp((FrescoObjectRef)_BaseObject__duplicate((StyleObjRef)(FrescoObjectRef)_obj_, &FrescoObjectStub::_create));
}
inline DisplayStyle_tmp::DisplayStyle_tmp(const DisplayStyle& r) {
    _obj_ = DisplayStyle::_duplicate(r._obj_);
}
inline DisplayStyle_tmp::DisplayStyle_tmp(const DisplayStyle_tmp& r) {
    _obj_ = r._obj_;
    ((DisplayStyle_tmp*)&r)->_obj_ = 0;
}
inline DisplayStyle_tmp::~DisplayStyle_tmp() { }

inline ScreenObjRef ScreenObj::_duplicate(ScreenObjRef obj) {
    return (ScreenObjRef)_BaseObject__duplicate(obj, &ScreenObjStub::_create);
}
inline ScreenObj& ScreenObj::operator =(ScreenObjRef p) {
    _BaseObject__release(_obj_);
    _obj_ = ScreenObj::_duplicate(p);
    return *this;
}
inline ScreenObj::ScreenObj(const ScreenObj& r) {
    _obj_ = ScreenObj::_duplicate(r._obj_);
}
inline ScreenObj& ScreenObj::operator =(const ScreenObj& r) {
    _BaseObject__release(_obj_);
    _obj_ = ScreenObj::_duplicate(r._obj_);
    return *this;
}
inline ScreenObj::ScreenObj(const ScreenObj_tmp& r) {
    _obj_ = r._obj_;
    ((ScreenObj_tmp*)&r)->_obj_ = 0;
}
inline ScreenObj& ScreenObj::operator =(const ScreenObj_tmp& r) {
    _BaseObject__release(_obj_);
    _obj_ = r._obj_;
    ((ScreenObj_tmp*)&r)->_obj_ = 0;
    return *this;
}
inline ScreenObj::ScreenObj(const ScreenObj_var& e) {
    _obj_ = ScreenObj::_duplicate(e._obj_);
}
inline ScreenObj& ScreenObj::operator =(const ScreenObj_var& e) {
    _BaseObject__release(_obj_);
    _obj_ = ScreenObj::_duplicate(e._obj_);
    return *this;
}
inline ScreenObj::~ScreenObj() {
    _BaseObject__release(_obj_);
}
inline ScreenObj_tmp ScreenObj::_narrow(const BaseObject& r) {
    return _narrow(r._obj_);
}
inline ScreenObj_tmp ScreenObj::_duplicate(const ScreenObj& r) {
    return _duplicate(r._obj_);
}
inline ScreenObj::operator FrescoObject() const {
    return FrescoObject_tmp((FrescoObjectRef)_BaseObject__duplicate((FrescoObjectRef)_obj_, &FrescoObjectStub::_create));
}
inline ScreenObj_tmp::ScreenObj_tmp(const ScreenObj& r) {
    _obj_ = ScreenObj::_duplicate(r._obj_);
}
inline ScreenObj_tmp::ScreenObj_tmp(const ScreenObj_tmp& r) {
    _obj_ = r._obj_;
    ((ScreenObj_tmp*)&r)->_obj_ = 0;
}
inline ScreenObj_tmp::~ScreenObj_tmp() { }

inline WindowRef Window::_duplicate(WindowRef obj) {
    return (WindowRef)_BaseObject__duplicate(obj, &WindowStub::_create);
}
inline Window& Window::operator =(WindowRef p) {
    _BaseObject__release(_obj_);
    _obj_ = Window::_duplicate(p);
    return *this;
}
inline Window::Window(const Window& r) {
    _obj_ = Window::_duplicate(r._obj_);
}
inline Window& Window::operator =(const Window& r) {
    _BaseObject__release(_obj_);
    _obj_ = Window::_duplicate(r._obj_);
    return *this;
}
inline Window::Window(const Window_tmp& r) {
    _obj_ = r._obj_;
    ((Window_tmp*)&r)->_obj_ = 0;
}
inline Window& Window::operator =(const Window_tmp& r) {
    _BaseObject__release(_obj_);
    _obj_ = r._obj_;
    ((Window_tmp*)&r)->_obj_ = 0;
    return *this;
}
inline Window::Window(const Window_var& e) {
    _obj_ = Window::_duplicate(e._obj_);
}
inline Window& Window::operator =(const Window_var& e) {
    _BaseObject__release(_obj_);
    _obj_ = Window::_duplicate(e._obj_);
    return *this;
}
inline Window::~Window() {
    _BaseObject__release(_obj_);
}
inline Window_tmp Window::_narrow(const BaseObject& r) {
    return _narrow(r._obj_);
}
inline Window_tmp Window::_duplicate(const Window& r) {
    return _duplicate(r._obj_);
}
inline Window::operator FrescoObject() const {
    return FrescoObject_tmp((FrescoObjectRef)_BaseObject__duplicate((FrescoObjectRef)_obj_, &FrescoObjectStub::_create));
}
inline Window_tmp::Window_tmp(const Window& r) {
    _obj_ = Window::_duplicate(r._obj_);
}
inline Window_tmp::Window_tmp(const Window_tmp& r) {
    _obj_ = r._obj_;
    ((Window_tmp*)&r)->_obj_ = 0;
}
inline Window_tmp::~Window_tmp() { }

inline WindowStyleRef WindowStyle::_duplicate(WindowStyleRef obj) {
    return (WindowStyleRef)_BaseObject__duplicate(obj, &WindowStyleStub::_create);
}
inline WindowStyle& WindowStyle::operator =(WindowStyleRef p) {
    _BaseObject__release(_obj_);
    _obj_ = WindowStyle::_duplicate(p);
    return *this;
}
inline WindowStyle::WindowStyle(const WindowStyle& r) {
    _obj_ = WindowStyle::_duplicate(r._obj_);
}
inline WindowStyle& WindowStyle::operator =(const WindowStyle& r) {
    _BaseObject__release(_obj_);
    _obj_ = WindowStyle::_duplicate(r._obj_);
    return *this;
}
inline WindowStyle::WindowStyle(const WindowStyle_tmp& r) {
    _obj_ = r._obj_;
    ((WindowStyle_tmp*)&r)->_obj_ = 0;
}
inline WindowStyle& WindowStyle::operator =(const WindowStyle_tmp& r) {
    _BaseObject__release(_obj_);
    _obj_ = r._obj_;
    ((WindowStyle_tmp*)&r)->_obj_ = 0;
    return *this;
}
inline WindowStyle::WindowStyle(const WindowStyle_var& e) {
    _obj_ = WindowStyle::_duplicate(e._obj_);
}
inline WindowStyle& WindowStyle::operator =(const WindowStyle_var& e) {
    _BaseObject__release(_obj_);
    _obj_ = WindowStyle::_duplicate(e._obj_);
    return *this;
}
inline WindowStyle::~WindowStyle() {
    _BaseObject__release(_obj_);
}
inline WindowStyle_tmp WindowStyle::_narrow(const BaseObject& r) {
    return _narrow(r._obj_);
}
inline WindowStyle_tmp WindowStyle::_duplicate(const WindowStyle& r) {
    return _duplicate(r._obj_);
}
inline WindowStyle::operator StyleObj() const {
    return StyleObj_tmp((StyleObjRef)_BaseObject__duplicate((StyleObjRef)_obj_, &StyleObjStub::_create));
}
inline WindowStyle::operator FrescoObject() const {
    return FrescoObject_tmp((FrescoObjectRef)_BaseObject__duplicate((StyleObjRef)(FrescoObjectRef)_obj_, &FrescoObjectStub::_create));
}
inline WindowStyle_tmp::WindowStyle_tmp(const WindowStyle& r) {
    _obj_ = WindowStyle::_duplicate(r._obj_);
}
inline WindowStyle_tmp::WindowStyle_tmp(const WindowStyle_tmp& r) {
    _obj_ = r._obj_;
    ((WindowStyle_tmp*)&r)->_obj_ = 0;
}
inline WindowStyle_tmp::~WindowStyle_tmp() { }

#endif
