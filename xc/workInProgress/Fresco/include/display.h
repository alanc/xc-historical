/* DO NOT EDIT -- Automatically generated from Interfaces/display.idl */

#ifndef Interfaces_display_h
#define Interfaces_display_h

#include <X11/Fresco/_enter.h>
#include <X11/Fresco/drawing.h>

class ColorType;
typedef ColorType* ColorRef;
class Color;
class _ColorExpr;
class _ColorElem;

class CursorType;
typedef CursorType* CursorRef;
class Cursor;
class _CursorExpr;
class _CursorElem;

class EventType;
typedef EventType* EventRef;
class Event;
class _EventExpr;
class _EventElem;

class FontType;
typedef FontType* FontRef;
class Font;
class _FontExpr;
class _FontElem;

class GlyphTraversalType;
typedef GlyphTraversalType* GlyphTraversalRef;
class GlyphTraversal;
class _GlyphTraversalExpr;
class _GlyphTraversalElem;

class RasterType;
typedef RasterType* RasterRef;
class Raster;
class _RasterExpr;
class _RasterElem;

class ScreenObjType;
typedef ScreenObjType* ScreenObjRef;
class ScreenObj;
class _ScreenObjExpr;
class _ScreenObjElem;

class ViewerType;
typedef ViewerType* ViewerRef;
class Viewer;
class _ViewerExpr;
class _ViewerElem;

class WindowType;
typedef WindowType* WindowRef;
class Window;
class _WindowExpr;
class _WindowElem;

class WindowStyleType;
typedef WindowStyleType* WindowStyleRef;
class WindowStyle;
class _WindowStyleExpr;
class _WindowStyleElem;

class Cursor {
public:
    CursorRef _obj;

    Cursor() { _obj = 0; }
    Cursor(CursorRef p) { _obj = p; }
    Cursor& operator =(CursorRef p);
    Cursor(const Cursor&);
    Cursor& operator =(const Cursor& r);
    Cursor(const _CursorExpr&);
    Cursor& operator =(const _CursorExpr&);
    Cursor(const _CursorElem&);
    Cursor& operator =(const _CursorElem&);
    ~Cursor();

    operator CursorRef() const { return _obj; }
    CursorRef operator ->() { return _obj; }

    operator FrescoObject() const;
    static CursorRef _narrow(BaseObjectRef p);
    static _CursorExpr _narrow(const BaseObject& r);

    static CursorRef _duplicate(CursorRef obj);
    static _CursorExpr _duplicate(const Cursor& r);
};

class _CursorExpr : public Cursor {
public:
    _CursorExpr(CursorRef p) { _obj = p; }
    _CursorExpr(const Cursor& r) { _obj = r._obj; }
    _CursorExpr(const _CursorExpr& r) { _obj = r._obj; }
    ~_CursorExpr();
};

class _CursorElem {
public:
    CursorRef _obj;

    _CursorElem(CursorRef p) { _obj = p; }
    operator CursorRef() const { return _obj; }
    CursorRef operator ->() { return _obj; }
};

class CursorType : public FrescoObjectType {
protected:
    CursorType();
    virtual ~CursorType();
public:

    _CursorExpr _ref();
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

inline CursorRef Cursor::_duplicate(CursorRef obj) {
    return (CursorRef)_BaseObject__duplicate(obj, &CursorStub::_create);
}
inline Cursor& Cursor::operator =(CursorRef p) {
    _BaseObject__release(_obj);
    _obj = Cursor::_duplicate(p);
    return *this;
}
inline Cursor::Cursor(const Cursor& r) {
    _obj = Cursor::_duplicate(r._obj);
}
inline Cursor& Cursor::operator =(const Cursor& r) {
    _BaseObject__release(_obj);
    _obj = Cursor::_duplicate(r._obj);
    return *this;
}
inline Cursor::Cursor(const _CursorExpr& r) {
    _obj = r._obj;
    ((_CursorExpr*)&r)->_obj = 0;
}
inline Cursor& Cursor::operator =(const _CursorExpr& r) {
    _BaseObject__release(_obj);
    _obj = r._obj;
    ((_CursorExpr*)&r)->_obj = 0;
    return *this;
}
inline Cursor::Cursor(const _CursorElem& e) {
    _obj = Cursor::_duplicate(e._obj);
}
inline Cursor& Cursor::operator =(const _CursorElem& e) {
    _BaseObject__release(_obj);
    _obj = Cursor::_duplicate(e._obj);
    return *this;
}
inline Cursor::~Cursor() {
    _BaseObject__release(_obj);
}
inline _CursorExpr Cursor::_narrow(const BaseObject& r) {
    return _narrow(r._obj);
}
inline _CursorExpr Cursor::_duplicate(const Cursor& r) {
    return _duplicate(r._obj);
}
inline Cursor::operator FrescoObject() const {
    return _FrescoObjectExpr((FrescoObjectRef)_BaseObject__duplicate(_obj, &FrescoObjectStub::_create));
}
inline _CursorExpr::~_CursorExpr() { }
inline _CursorExpr CursorType::_ref() { return this; }

class DisplayStyleType;
typedef DisplayStyleType* DisplayStyleRef;
class DisplayStyle;
class _DisplayStyleExpr;
class _DisplayStyleElem;

class DisplayObjType;
typedef DisplayObjType* DisplayObjRef;
class DisplayObj;
class _DisplayObjExpr;
class _DisplayObjElem;

class DisplayObj {
public:
    DisplayObjRef _obj;

    DisplayObj() { _obj = 0; }
    DisplayObj(DisplayObjRef p) { _obj = p; }
    DisplayObj& operator =(DisplayObjRef p);
    DisplayObj(const DisplayObj&);
    DisplayObj& operator =(const DisplayObj& r);
    DisplayObj(const _DisplayObjExpr&);
    DisplayObj& operator =(const _DisplayObjExpr&);
    DisplayObj(const _DisplayObjElem&);
    DisplayObj& operator =(const _DisplayObjElem&);
    ~DisplayObj();

    operator DisplayObjRef() const { return _obj; }
    DisplayObjRef operator ->() { return _obj; }

    operator FrescoObject() const;
    static DisplayObjRef _narrow(BaseObjectRef p);
    static _DisplayObjExpr _narrow(const BaseObject& r);

    static DisplayObjRef _duplicate(DisplayObjRef obj);
    static _DisplayObjExpr _duplicate(const DisplayObj& r);
    typedef Long ScreenNumber;
};

class _DisplayObjExpr : public DisplayObj {
public:
    _DisplayObjExpr(DisplayObjRef p) { _obj = p; }
    _DisplayObjExpr(const DisplayObj& r) { _obj = r._obj; }
    _DisplayObjExpr(const _DisplayObjExpr& r) { _obj = r._obj; }
    ~_DisplayObjExpr();
};

class _DisplayObjElem {
public:
    DisplayObjRef _obj;

    _DisplayObjElem(DisplayObjRef p) { _obj = p; }
    operator DisplayObjRef() const { return _obj; }
    DisplayObjRef operator ->() { return _obj; }
};

class DisplayObjType : public FrescoObjectType {
protected:
    DisplayObjType();
    virtual ~DisplayObjType();
public:
    _DisplayStyleExpr style();
    virtual DisplayStyleRef _c_style();
    _DrawingKitExpr drawing_kit() {
        return _c_drawing_kit();
    }
    virtual DrawingKitRef _c_drawing_kit();
    _ScreenObjExpr screen(DisplayObj::ScreenNumber n);
    virtual ScreenObjRef _c_screen(DisplayObj::ScreenNumber n);
    virtual DisplayObj::ScreenNumber number_of_screens();
    _ScreenObjExpr default_screen();
    virtual ScreenObjRef _c_default_screen();
    _CursorExpr cursor_from_data(Short x, Short y, Long pattern[16], Long mask[16]) {
        return _c_cursor_from_data(x, y, pattern, mask);
    }
    virtual CursorRef _c_cursor_from_data(Short x, Short y, Long pattern[16], Long mask[16]);
    _CursorExpr cursor_from_bitmap(RasterRef b, RasterRef mask) {
        return _c_cursor_from_bitmap(b, mask);
    }
    virtual CursorRef _c_cursor_from_bitmap(RasterRef b, RasterRef mask);
    _CursorExpr cursor_from_font(FontRef f, Long pattern, Long mask) {
        return _c_cursor_from_font(f, pattern, mask);
    }
    virtual CursorRef _c_cursor_from_font(FontRef f, Long pattern, Long mask);
    _CursorExpr cursor_from_index(Long index) {
        return _c_cursor_from_index(index);
    }
    virtual CursorRef _c_cursor_from_index(Long index);
    virtual void run(Boolean b);
    virtual Boolean running();
    virtual Tag add_filter(ViewerRef v, GlyphTraversalRef t);
    virtual void remove_filter(Tag add_tag);
    virtual void need_repair(WindowRef w);
    virtual void repair();
    virtual void flush();
    virtual void flush_and_wait();
    virtual void ring_bell(Float pct_loudness);
    virtual void close();

    _DisplayObjExpr _ref();
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

inline DisplayObjRef DisplayObj::_duplicate(DisplayObjRef obj) {
    return (DisplayObjRef)_BaseObject__duplicate(obj, &DisplayObjStub::_create);
}
inline DisplayObj& DisplayObj::operator =(DisplayObjRef p) {
    _BaseObject__release(_obj);
    _obj = DisplayObj::_duplicate(p);
    return *this;
}
inline DisplayObj::DisplayObj(const DisplayObj& r) {
    _obj = DisplayObj::_duplicate(r._obj);
}
inline DisplayObj& DisplayObj::operator =(const DisplayObj& r) {
    _BaseObject__release(_obj);
    _obj = DisplayObj::_duplicate(r._obj);
    return *this;
}
inline DisplayObj::DisplayObj(const _DisplayObjExpr& r) {
    _obj = r._obj;
    ((_DisplayObjExpr*)&r)->_obj = 0;
}
inline DisplayObj& DisplayObj::operator =(const _DisplayObjExpr& r) {
    _BaseObject__release(_obj);
    _obj = r._obj;
    ((_DisplayObjExpr*)&r)->_obj = 0;
    return *this;
}
inline DisplayObj::DisplayObj(const _DisplayObjElem& e) {
    _obj = DisplayObj::_duplicate(e._obj);
}
inline DisplayObj& DisplayObj::operator =(const _DisplayObjElem& e) {
    _BaseObject__release(_obj);
    _obj = DisplayObj::_duplicate(e._obj);
    return *this;
}
inline DisplayObj::~DisplayObj() {
    _BaseObject__release(_obj);
}
inline _DisplayObjExpr DisplayObj::_narrow(const BaseObject& r) {
    return _narrow(r._obj);
}
inline _DisplayObjExpr DisplayObj::_duplicate(const DisplayObj& r) {
    return _duplicate(r._obj);
}
inline DisplayObj::operator FrescoObject() const {
    return _FrescoObjectExpr((FrescoObjectRef)_BaseObject__duplicate(_obj, &FrescoObjectStub::_create));
}
inline _DisplayObjExpr::~_DisplayObjExpr() { }
inline _DisplayObjExpr DisplayObjType::_ref() { return this; }

class DisplayStyle {
public:
    DisplayStyleRef _obj;

    DisplayStyle() { _obj = 0; }
    DisplayStyle(DisplayStyleRef p) { _obj = p; }
    DisplayStyle& operator =(DisplayStyleRef p);
    DisplayStyle(const DisplayStyle&);
    DisplayStyle& operator =(const DisplayStyle& r);
    DisplayStyle(const _DisplayStyleExpr&);
    DisplayStyle& operator =(const _DisplayStyleExpr&);
    DisplayStyle(const _DisplayStyleElem&);
    DisplayStyle& operator =(const _DisplayStyleElem&);
    ~DisplayStyle();

    operator DisplayStyleRef() const { return _obj; }
    DisplayStyleRef operator ->() { return _obj; }

    operator StyleObj() const;
    operator FrescoObject() const;
    static DisplayStyleRef _narrow(BaseObjectRef p);
    static _DisplayStyleExpr _narrow(const BaseObject& r);

    static DisplayStyleRef _duplicate(DisplayStyleRef obj);
    static _DisplayStyleExpr _duplicate(const DisplayStyle& r);
};

class _DisplayStyleExpr : public DisplayStyle {
public:
    _DisplayStyleExpr(DisplayStyleRef p) { _obj = p; }
    _DisplayStyleExpr(const DisplayStyle& r) { _obj = r._obj; }
    _DisplayStyleExpr(const _DisplayStyleExpr& r) { _obj = r._obj; }
    ~_DisplayStyleExpr();
};

class _DisplayStyleElem {
public:
    DisplayStyleRef _obj;

    _DisplayStyleElem(DisplayStyleRef p) { _obj = p; }
    operator DisplayStyleRef() const { return _obj; }
    DisplayStyleRef operator ->() { return _obj; }
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

    _DisplayStyleExpr _ref();
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

inline DisplayStyleRef DisplayStyle::_duplicate(DisplayStyleRef obj) {
    return (DisplayStyleRef)_BaseObject__duplicate(obj, &DisplayStyleStub::_create);
}
inline DisplayStyle& DisplayStyle::operator =(DisplayStyleRef p) {
    _BaseObject__release(_obj);
    _obj = DisplayStyle::_duplicate(p);
    return *this;
}
inline DisplayStyle::DisplayStyle(const DisplayStyle& r) {
    _obj = DisplayStyle::_duplicate(r._obj);
}
inline DisplayStyle& DisplayStyle::operator =(const DisplayStyle& r) {
    _BaseObject__release(_obj);
    _obj = DisplayStyle::_duplicate(r._obj);
    return *this;
}
inline DisplayStyle::DisplayStyle(const _DisplayStyleExpr& r) {
    _obj = r._obj;
    ((_DisplayStyleExpr*)&r)->_obj = 0;
}
inline DisplayStyle& DisplayStyle::operator =(const _DisplayStyleExpr& r) {
    _BaseObject__release(_obj);
    _obj = r._obj;
    ((_DisplayStyleExpr*)&r)->_obj = 0;
    return *this;
}
inline DisplayStyle::DisplayStyle(const _DisplayStyleElem& e) {
    _obj = DisplayStyle::_duplicate(e._obj);
}
inline DisplayStyle& DisplayStyle::operator =(const _DisplayStyleElem& e) {
    _BaseObject__release(_obj);
    _obj = DisplayStyle::_duplicate(e._obj);
    return *this;
}
inline DisplayStyle::~DisplayStyle() {
    _BaseObject__release(_obj);
}
inline _DisplayStyleExpr DisplayStyle::_narrow(const BaseObject& r) {
    return _narrow(r._obj);
}
inline _DisplayStyleExpr DisplayStyle::_duplicate(const DisplayStyle& r) {
    return _duplicate(r._obj);
}
inline DisplayStyle::operator StyleObj() const {
    return _StyleObjExpr((StyleObjRef)_BaseObject__duplicate(_obj, &StyleObjStub::_create));
}
inline DisplayStyle::operator FrescoObject() const {
    return _FrescoObjectExpr((FrescoObjectRef)_BaseObject__duplicate((StyleObjRef)_obj, &FrescoObjectStub::_create));
}
inline _DisplayStyleExpr::~_DisplayStyleExpr() { }
inline _DisplayStyleExpr DisplayStyleType::_ref() { return this; }

class ScreenObj {
public:
    ScreenObjRef _obj;

    ScreenObj() { _obj = 0; }
    ScreenObj(ScreenObjRef p) { _obj = p; }
    ScreenObj& operator =(ScreenObjRef p);
    ScreenObj(const ScreenObj&);
    ScreenObj& operator =(const ScreenObj& r);
    ScreenObj(const _ScreenObjExpr&);
    ScreenObj& operator =(const _ScreenObjExpr&);
    ScreenObj(const _ScreenObjElem&);
    ScreenObj& operator =(const _ScreenObjElem&);
    ~ScreenObj();

    operator ScreenObjRef() const { return _obj; }
    ScreenObjRef operator ->() { return _obj; }

    operator FrescoObject() const;
    static ScreenObjRef _narrow(BaseObjectRef p);
    static _ScreenObjExpr _narrow(const BaseObject& r);

    static ScreenObjRef _duplicate(ScreenObjRef obj);
    static _ScreenObjExpr _duplicate(const ScreenObj& r);
};

class _ScreenObjExpr : public ScreenObj {
public:
    _ScreenObjExpr(ScreenObjRef p) { _obj = p; }
    _ScreenObjExpr(const ScreenObj& r) { _obj = r._obj; }
    _ScreenObjExpr(const _ScreenObjExpr& r) { _obj = r._obj; }
    ~_ScreenObjExpr();
};

class _ScreenObjElem {
public:
    ScreenObjRef _obj;

    _ScreenObjElem(ScreenObjRef p) { _obj = p; }
    operator ScreenObjRef() const { return _obj; }
    ScreenObjRef operator ->() { return _obj; }
};

class ScreenObjType : public FrescoObjectType {
protected:
    ScreenObjType();
    virtual ~ScreenObjType();
public:
    virtual Coord dpi();
    virtual void dpi(Coord _p);
    _DisplayObjExpr display() {
        return _c_display();
    }
    virtual DisplayObjRef _c_display();
    virtual Coord width();
    virtual Coord height();
    virtual PixelCoord to_pixels(Coord c);
    virtual Coord to_coord(PixelCoord p);
    virtual Coord to_pixels_coord(Coord c);
    virtual void move_pointer(Coord x, Coord y);
    _WindowExpr application(ViewerRef v);
    virtual WindowRef _c_application(ViewerRef v);
    _WindowExpr top_level(ViewerRef v, WindowRef group_leader);
    virtual WindowRef _c_top_level(ViewerRef v, WindowRef group_leader);
    _WindowExpr transient(ViewerRef v, WindowRef transient_for);
    virtual WindowRef _c_transient(ViewerRef v, WindowRef transient_for);
    _WindowExpr popup(ViewerRef v);
    virtual WindowRef _c_popup(ViewerRef v);
    _WindowExpr icon(ViewerRef v);
    virtual WindowRef _c_icon(ViewerRef v);

    _ScreenObjExpr _ref();
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

inline ScreenObjRef ScreenObj::_duplicate(ScreenObjRef obj) {
    return (ScreenObjRef)_BaseObject__duplicate(obj, &ScreenObjStub::_create);
}
inline ScreenObj& ScreenObj::operator =(ScreenObjRef p) {
    _BaseObject__release(_obj);
    _obj = ScreenObj::_duplicate(p);
    return *this;
}
inline ScreenObj::ScreenObj(const ScreenObj& r) {
    _obj = ScreenObj::_duplicate(r._obj);
}
inline ScreenObj& ScreenObj::operator =(const ScreenObj& r) {
    _BaseObject__release(_obj);
    _obj = ScreenObj::_duplicate(r._obj);
    return *this;
}
inline ScreenObj::ScreenObj(const _ScreenObjExpr& r) {
    _obj = r._obj;
    ((_ScreenObjExpr*)&r)->_obj = 0;
}
inline ScreenObj& ScreenObj::operator =(const _ScreenObjExpr& r) {
    _BaseObject__release(_obj);
    _obj = r._obj;
    ((_ScreenObjExpr*)&r)->_obj = 0;
    return *this;
}
inline ScreenObj::ScreenObj(const _ScreenObjElem& e) {
    _obj = ScreenObj::_duplicate(e._obj);
}
inline ScreenObj& ScreenObj::operator =(const _ScreenObjElem& e) {
    _BaseObject__release(_obj);
    _obj = ScreenObj::_duplicate(e._obj);
    return *this;
}
inline ScreenObj::~ScreenObj() {
    _BaseObject__release(_obj);
}
inline _ScreenObjExpr ScreenObj::_narrow(const BaseObject& r) {
    return _narrow(r._obj);
}
inline _ScreenObjExpr ScreenObj::_duplicate(const ScreenObj& r) {
    return _duplicate(r._obj);
}
inline ScreenObj::operator FrescoObject() const {
    return _FrescoObjectExpr((FrescoObjectRef)_BaseObject__duplicate(_obj, &FrescoObjectStub::_create));
}
inline _ScreenObjExpr::~_ScreenObjExpr() { }
inline _ScreenObjExpr ScreenObjType::_ref() { return this; }

class Window {
public:
    WindowRef _obj;

    Window() { _obj = 0; }
    Window(WindowRef p) { _obj = p; }
    Window& operator =(WindowRef p);
    Window(const Window&);
    Window& operator =(const Window& r);
    Window(const _WindowExpr&);
    Window& operator =(const _WindowExpr&);
    Window(const _WindowElem&);
    Window& operator =(const _WindowElem&);
    ~Window();

    operator WindowRef() const { return _obj; }
    WindowRef operator ->() { return _obj; }

    operator FrescoObject() const;
    static WindowRef _narrow(BaseObjectRef p);
    static _WindowExpr _narrow(const BaseObject& r);

    static WindowRef _duplicate(WindowRef obj);
    static _WindowExpr _duplicate(const Window& r);
    struct Placement {
        Coord x, y, width, height;
        Alignment align_x, align_y;
    };
};

class _WindowExpr : public Window {
public:
    _WindowExpr(WindowRef p) { _obj = p; }
    _WindowExpr(const Window& r) { _obj = r._obj; }
    _WindowExpr(const _WindowExpr& r) { _obj = r._obj; }
    ~_WindowExpr();
};

class _WindowElem {
public:
    WindowRef _obj;

    _WindowElem(WindowRef p) { _obj = p; }
    operator WindowRef() const { return _obj; }
    WindowRef operator ->() { return _obj; }
};

class WindowType : public FrescoObjectType {
protected:
    WindowType();
    virtual ~WindowType();
public:
    _ScreenObjExpr screen() {
        return _c_screen();
    }
    virtual ScreenObjRef _c_screen();
    _WindowStyleExpr style();
    virtual WindowStyleRef _c_style();
    _ViewerExpr main_viewer();
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
    virtual void handle_event(EventRef e);
    virtual void grab_pointer(CursorRef c);
    virtual void ungrab_pointer();

    _WindowExpr _ref();
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

inline WindowRef Window::_duplicate(WindowRef obj) {
    return (WindowRef)_BaseObject__duplicate(obj, &WindowStub::_create);
}
inline Window& Window::operator =(WindowRef p) {
    _BaseObject__release(_obj);
    _obj = Window::_duplicate(p);
    return *this;
}
inline Window::Window(const Window& r) {
    _obj = Window::_duplicate(r._obj);
}
inline Window& Window::operator =(const Window& r) {
    _BaseObject__release(_obj);
    _obj = Window::_duplicate(r._obj);
    return *this;
}
inline Window::Window(const _WindowExpr& r) {
    _obj = r._obj;
    ((_WindowExpr*)&r)->_obj = 0;
}
inline Window& Window::operator =(const _WindowExpr& r) {
    _BaseObject__release(_obj);
    _obj = r._obj;
    ((_WindowExpr*)&r)->_obj = 0;
    return *this;
}
inline Window::Window(const _WindowElem& e) {
    _obj = Window::_duplicate(e._obj);
}
inline Window& Window::operator =(const _WindowElem& e) {
    _BaseObject__release(_obj);
    _obj = Window::_duplicate(e._obj);
    return *this;
}
inline Window::~Window() {
    _BaseObject__release(_obj);
}
inline _WindowExpr Window::_narrow(const BaseObject& r) {
    return _narrow(r._obj);
}
inline _WindowExpr Window::_duplicate(const Window& r) {
    return _duplicate(r._obj);
}
inline Window::operator FrescoObject() const {
    return _FrescoObjectExpr((FrescoObjectRef)_BaseObject__duplicate(_obj, &FrescoObjectStub::_create));
}
inline _WindowExpr::~_WindowExpr() { }
inline _WindowExpr WindowType::_ref() { return this; }

class WindowStyle {
public:
    WindowStyleRef _obj;

    WindowStyle() { _obj = 0; }
    WindowStyle(WindowStyleRef p) { _obj = p; }
    WindowStyle& operator =(WindowStyleRef p);
    WindowStyle(const WindowStyle&);
    WindowStyle& operator =(const WindowStyle& r);
    WindowStyle(const _WindowStyleExpr&);
    WindowStyle& operator =(const _WindowStyleExpr&);
    WindowStyle(const _WindowStyleElem&);
    WindowStyle& operator =(const _WindowStyleElem&);
    ~WindowStyle();

    operator WindowStyleRef() const { return _obj; }
    WindowStyleRef operator ->() { return _obj; }

    operator StyleObj() const;
    operator FrescoObject() const;
    static WindowStyleRef _narrow(BaseObjectRef p);
    static _WindowStyleExpr _narrow(const BaseObject& r);

    static WindowStyleRef _duplicate(WindowStyleRef obj);
    static _WindowStyleExpr _duplicate(const WindowStyle& r);
};

class _WindowStyleExpr : public WindowStyle {
public:
    _WindowStyleExpr(WindowStyleRef p) { _obj = p; }
    _WindowStyleExpr(const WindowStyle& r) { _obj = r._obj; }
    _WindowStyleExpr(const _WindowStyleExpr& r) { _obj = r._obj; }
    ~_WindowStyleExpr();
};

class _WindowStyleElem {
public:
    WindowStyleRef _obj;

    _WindowStyleElem(WindowStyleRef p) { _obj = p; }
    operator WindowStyleRef() const { return _obj; }
    WindowStyleRef operator ->() { return _obj; }
};

class WindowStyleType : public StyleObjType {
protected:
    WindowStyleType();
    virtual ~WindowStyleType();
public:
    virtual Boolean double_buffered();
    virtual void double_buffered(Boolean _p);
    _CursorExpr default_cursor() {
        return _c_default_cursor();
    }
    virtual CursorRef _c_default_cursor();
    void default_cursor(CursorRef _p) {
        _c_default_cursor(_p);
    }
    virtual void _c_default_cursor(CursorRef _p);
    _ColorExpr cursor_foreground() {
        return _c_cursor_foreground();
    }
    virtual ColorRef _c_cursor_foreground();
    void cursor_foreground(ColorRef _p) {
        _c_cursor_foreground(_p);
    }
    virtual void _c_cursor_foreground(ColorRef _p);
    _ColorExpr cursor_background() {
        return _c_cursor_background();
    }
    virtual ColorRef _c_cursor_background();
    void cursor_background(ColorRef _p) {
        _c_cursor_background(_p);
    }
    virtual void _c_cursor_background(ColorRef _p);
    _CharStringExpr geometry() {
        return _c_geometry();
    }
    virtual CharStringRef _c_geometry();
    void geometry(CharStringRef _p) {
        _c_geometry(_p);
    }
    virtual void _c_geometry(CharStringRef _p);
    _WindowExpr icon() {
        return _c_icon();
    }
    virtual WindowRef _c_icon();
    void icon(WindowRef _p) {
        _c_icon(_p);
    }
    virtual void _c_icon(WindowRef _p);
    _RasterExpr icon_bitmap() {
        return _c_icon_bitmap();
    }
    virtual RasterRef _c_icon_bitmap();
    void icon_bitmap(RasterRef _p) {
        _c_icon_bitmap(_p);
    }
    virtual void _c_icon_bitmap(RasterRef _p);
    _RasterExpr icon_mask() {
        return _c_icon_mask();
    }
    virtual RasterRef _c_icon_mask();
    void icon_mask(RasterRef _p) {
        _c_icon_mask(_p);
    }
    virtual void _c_icon_mask(RasterRef _p);
    _CharStringExpr icon_name() {
        return _c_icon_name();
    }
    virtual CharStringRef _c_icon_name();
    void icon_name(CharStringRef _p) {
        _c_icon_name(_p);
    }
    virtual void _c_icon_name(CharStringRef _p);
    _CharStringExpr icon_geometry() {
        return _c_icon_geometry();
    }
    virtual CharStringRef _c_icon_geometry();
    void icon_geometry(CharStringRef _p) {
        _c_icon_geometry(_p);
    }
    virtual void _c_icon_geometry(CharStringRef _p);
    virtual Boolean iconic();
    virtual void iconic(Boolean _p);
    _CharStringExpr title() {
        return _c_title();
    }
    virtual CharStringRef _c_title();
    void title(CharStringRef _p) {
        _c_title(_p);
    }
    virtual void _c_title(CharStringRef _p);
    virtual Long xor_pixel();
    virtual void xor_pixel(Long _p);

    _WindowStyleExpr _ref();
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

inline WindowStyleRef WindowStyle::_duplicate(WindowStyleRef obj) {
    return (WindowStyleRef)_BaseObject__duplicate(obj, &WindowStyleStub::_create);
}
inline WindowStyle& WindowStyle::operator =(WindowStyleRef p) {
    _BaseObject__release(_obj);
    _obj = WindowStyle::_duplicate(p);
    return *this;
}
inline WindowStyle::WindowStyle(const WindowStyle& r) {
    _obj = WindowStyle::_duplicate(r._obj);
}
inline WindowStyle& WindowStyle::operator =(const WindowStyle& r) {
    _BaseObject__release(_obj);
    _obj = WindowStyle::_duplicate(r._obj);
    return *this;
}
inline WindowStyle::WindowStyle(const _WindowStyleExpr& r) {
    _obj = r._obj;
    ((_WindowStyleExpr*)&r)->_obj = 0;
}
inline WindowStyle& WindowStyle::operator =(const _WindowStyleExpr& r) {
    _BaseObject__release(_obj);
    _obj = r._obj;
    ((_WindowStyleExpr*)&r)->_obj = 0;
    return *this;
}
inline WindowStyle::WindowStyle(const _WindowStyleElem& e) {
    _obj = WindowStyle::_duplicate(e._obj);
}
inline WindowStyle& WindowStyle::operator =(const _WindowStyleElem& e) {
    _BaseObject__release(_obj);
    _obj = WindowStyle::_duplicate(e._obj);
    return *this;
}
inline WindowStyle::~WindowStyle() {
    _BaseObject__release(_obj);
}
inline _WindowStyleExpr WindowStyle::_narrow(const BaseObject& r) {
    return _narrow(r._obj);
}
inline _WindowStyleExpr WindowStyle::_duplicate(const WindowStyle& r) {
    return _duplicate(r._obj);
}
inline WindowStyle::operator StyleObj() const {
    return _StyleObjExpr((StyleObjRef)_BaseObject__duplicate(_obj, &StyleObjStub::_create));
}
inline WindowStyle::operator FrescoObject() const {
    return _FrescoObjectExpr((FrescoObjectRef)_BaseObject__duplicate((StyleObjRef)_obj, &FrescoObjectStub::_create));
}
inline _WindowStyleExpr::~_WindowStyleExpr() { }
inline _WindowStyleExpr WindowStyleType::_ref() { return this; }

#endif
