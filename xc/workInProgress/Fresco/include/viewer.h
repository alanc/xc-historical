/* DO NOT EDIT -- Automatically generated from Interfaces/viewer.idl */

#ifndef Interfaces_viewer_h
#define Interfaces_viewer_h

#include <X11/Fresco/_enter.h>
#include <X11/Fresco/glyph.h>

class EventType;
typedef EventType* EventRef;
class Event;
class _EventExpr;
class _EventElem;

class FocusType;
typedef FocusType* FocusRef;
class Focus;
class _FocusExpr;
class _FocusElem;

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

class Event {
public:
    EventRef _obj;

    Event() { _obj = 0; }
    Event(EventRef p) { _obj = p; }
    Event& operator =(EventRef p);
    Event(const Event&);
    Event& operator =(const Event& r);
    Event(const _EventExpr&);
    Event& operator =(const _EventExpr&);
    Event(const _EventElem&);
    Event& operator =(const _EventElem&);
    ~Event();

    operator EventRef() const { return _obj; }
    EventRef operator ->() { return _obj; }

    operator FrescoObject() const;
    static EventRef _narrow(BaseObjectRef p);
    static _EventExpr _narrow(const BaseObject& r);

    static EventRef _duplicate(EventRef obj);
    static _EventExpr _duplicate(const Event& r);
    typedef Long ButtonIndex;
    typedef ULong KeySym;
    class KeyChord {
    public:
        long _maximum, _length; KeySym* _buffer;

        KeyChord() { _maximum = _length = 0; _buffer = 0; }
        KeyChord(long m, long n, KeySym* e) {
            _maximum = m; _length = n; _buffer = e;
        }
        KeyChord(const KeyChord& _s) { _buffer = 0; *this = _s; }
        KeyChord& operator =(const KeyChord&);
        ~KeyChord() { delete [] _buffer; }
    };
    typedef ULong TimeStamp;
    enum Modifier {
        control, shift, capslock, meta
    };
    enum TypeId {
        motion, enter, leave, down, up, key_press, key_release, focus_in, 
        focus_out, other
    };
};

class _EventExpr : public Event {
public:
    _EventExpr(EventRef p) { _obj = p; }
    _EventExpr(const Event& r) { _obj = r._obj; }
    _EventExpr(const _EventExpr& r) { _obj = r._obj; }
    ~_EventExpr();
};

class _EventElem {
public:
    EventRef _obj;

    _EventElem(EventRef p) { _obj = p; }
    operator EventRef() const { return _obj; }
    EventRef operator ->() { return _obj; }
};

class EventType : public FrescoObjectType {
protected:
    EventType();
    virtual ~EventType();
public:
    virtual Event::TypeId type();
    virtual Event::TimeStamp time();
    virtual Coord pointer_x();
    virtual Coord pointer_y();
    virtual Event::ButtonIndex pointer_button();
    virtual Boolean button_is_down(Event::ButtonIndex b);
    virtual Boolean modifier_is_down(Event::Modifier m);
    virtual Event::KeySym key();
    virtual CharCode character();
    virtual void unread();

    _EventExpr _ref();
    virtual TypeObjId _tid();
};

class EventStub : public EventType {
public:
    EventStub(Exchange*);
    ~EventStub();

    static BaseObjectRef _create(Exchange*);
    Exchange* _exchange();
protected:
    Exchange* exch_;
};

inline EventRef Event::_duplicate(EventRef obj) {
    return (EventRef)_BaseObject__duplicate(obj, &EventStub::_create);
}
inline Event& Event::operator =(EventRef p) {
    _BaseObject__release(_obj);
    _obj = Event::_duplicate(p);
    return *this;
}
inline Event::Event(const Event& r) {
    _obj = Event::_duplicate(r._obj);
}
inline Event& Event::operator =(const Event& r) {
    _BaseObject__release(_obj);
    _obj = Event::_duplicate(r._obj);
    return *this;
}
inline Event::Event(const _EventExpr& r) {
    _obj = r._obj;
    ((_EventExpr*)&r)->_obj = 0;
}
inline Event& Event::operator =(const _EventExpr& r) {
    _BaseObject__release(_obj);
    _obj = r._obj;
    ((_EventExpr*)&r)->_obj = 0;
    return *this;
}
inline Event::Event(const _EventElem& e) {
    _obj = Event::_duplicate(e._obj);
}
inline Event& Event::operator =(const _EventElem& e) {
    _BaseObject__release(_obj);
    _obj = Event::_duplicate(e._obj);
    return *this;
}
inline Event::~Event() {
    _BaseObject__release(_obj);
}
inline _EventExpr Event::_narrow(const BaseObject& r) {
    return _narrow(r._obj);
}
inline _EventExpr Event::_duplicate(const Event& r) {
    return _duplicate(r._obj);
}
inline Event::operator FrescoObject() const {
    return _FrescoObjectExpr((FrescoObjectRef)_BaseObject__duplicate(_obj, &FrescoObjectStub::_create));
}
inline _EventExpr::~_EventExpr() { }
inline _EventExpr EventType::_ref() { return this; }

class Viewer {
public:
    ViewerRef _obj;

    Viewer() { _obj = 0; }
    Viewer(ViewerRef p) { _obj = p; }
    Viewer& operator =(ViewerRef p);
    Viewer(const Viewer&);
    Viewer& operator =(const Viewer& r);
    Viewer(const _ViewerExpr&);
    Viewer& operator =(const _ViewerExpr&);
    Viewer(const _ViewerElem&);
    Viewer& operator =(const _ViewerElem&);
    ~Viewer();

    operator ViewerRef() const { return _obj; }
    ViewerRef operator ->() { return _obj; }

    operator Glyph() const;
    operator FrescoObject() const;
    static ViewerRef _narrow(BaseObjectRef p);
    static _ViewerExpr _narrow(const BaseObject& r);

    static ViewerRef _duplicate(ViewerRef obj);
    static _ViewerExpr _duplicate(const Viewer& r);
};

class _ViewerExpr : public Viewer {
public:
    _ViewerExpr(ViewerRef p) { _obj = p; }
    _ViewerExpr(const Viewer& r) { _obj = r._obj; }
    _ViewerExpr(const _ViewerExpr& r) { _obj = r._obj; }
    ~_ViewerExpr();
};

class _ViewerElem {
public:
    ViewerRef _obj;

    _ViewerElem(ViewerRef p) { _obj = p; }
    operator ViewerRef() const { return _obj; }
    ViewerRef operator ->() { return _obj; }
};

class ViewerType : public GlyphType {
protected:
    ViewerType();
    virtual ~ViewerType();
public:
    _ViewerExpr parent_viewer() {
        return _c_parent_viewer();
    }
    virtual ViewerRef _c_parent_viewer();
    _ViewerExpr next_viewer() {
        return _c_next_viewer();
    }
    virtual ViewerRef _c_next_viewer();
    _ViewerExpr prev_viewer() {
        return _c_prev_viewer();
    }
    virtual ViewerRef _c_prev_viewer();
    _ViewerExpr first_viewer() {
        return _c_first_viewer();
    }
    virtual ViewerRef _c_first_viewer();
    _ViewerExpr last_viewer() {
        return _c_last_viewer();
    }
    virtual ViewerRef _c_last_viewer();
    virtual void append_viewer(ViewerRef v);
    virtual void prepend_viewer(ViewerRef v);
    virtual void insert_viewer(ViewerRef v);
    virtual void replace_viewer(ViewerRef v);
    virtual void remove_viewer();
    virtual void set_viewer_links(ViewerRef parent, ViewerRef prev, ViewerRef next);
    virtual void set_first_viewer(ViewerRef v);
    virtual void set_last_viewer(ViewerRef v);
    _FocusExpr request_focus(ViewerRef requestor, Boolean temporary);
    virtual FocusRef _c_request_focus(ViewerRef requestor, Boolean temporary);
    virtual Boolean receive_focus(FocusRef f, Boolean primary);
    virtual void lose_focus(Boolean temporary);
    virtual Boolean first_focus();
    virtual Boolean last_focus();
    virtual Boolean next_focus();
    virtual Boolean prev_focus();
    virtual Boolean handle(GlyphTraversalRef t, EventRef e);
    virtual void close();

    _ViewerExpr _ref();
    virtual TypeObjId _tid();
};

class ViewerStub : public ViewerType {
public:
    ViewerStub(Exchange*);
    ~ViewerStub();

    static BaseObjectRef _create(Exchange*);
    Exchange* _exchange();
protected:
    Exchange* exch_;
};

inline ViewerRef Viewer::_duplicate(ViewerRef obj) {
    return (ViewerRef)_BaseObject__duplicate(obj, &ViewerStub::_create);
}
inline Viewer& Viewer::operator =(ViewerRef p) {
    _BaseObject__release(_obj);
    _obj = Viewer::_duplicate(p);
    return *this;
}
inline Viewer::Viewer(const Viewer& r) {
    _obj = Viewer::_duplicate(r._obj);
}
inline Viewer& Viewer::operator =(const Viewer& r) {
    _BaseObject__release(_obj);
    _obj = Viewer::_duplicate(r._obj);
    return *this;
}
inline Viewer::Viewer(const _ViewerExpr& r) {
    _obj = r._obj;
    ((_ViewerExpr*)&r)->_obj = 0;
}
inline Viewer& Viewer::operator =(const _ViewerExpr& r) {
    _BaseObject__release(_obj);
    _obj = r._obj;
    ((_ViewerExpr*)&r)->_obj = 0;
    return *this;
}
inline Viewer::Viewer(const _ViewerElem& e) {
    _obj = Viewer::_duplicate(e._obj);
}
inline Viewer& Viewer::operator =(const _ViewerElem& e) {
    _BaseObject__release(_obj);
    _obj = Viewer::_duplicate(e._obj);
    return *this;
}
inline Viewer::~Viewer() {
    _BaseObject__release(_obj);
}
inline _ViewerExpr Viewer::_narrow(const BaseObject& r) {
    return _narrow(r._obj);
}
inline _ViewerExpr Viewer::_duplicate(const Viewer& r) {
    return _duplicate(r._obj);
}
inline Viewer::operator Glyph() const {
    return _GlyphExpr((GlyphRef)_BaseObject__duplicate(_obj, &GlyphStub::_create));
}
inline Viewer::operator FrescoObject() const {
    return _FrescoObjectExpr((FrescoObjectRef)_BaseObject__duplicate((GlyphRef)_obj, &FrescoObjectStub::_create));
}
inline _ViewerExpr::~_ViewerExpr() { }
inline _ViewerExpr ViewerType::_ref() { return this; }

class Focus {
public:
    FocusRef _obj;

    Focus() { _obj = 0; }
    Focus(FocusRef p) { _obj = p; }
    Focus& operator =(FocusRef p);
    Focus(const Focus&);
    Focus& operator =(const Focus& r);
    Focus(const _FocusExpr&);
    Focus& operator =(const _FocusExpr&);
    Focus(const _FocusElem&);
    Focus& operator =(const _FocusElem&);
    ~Focus();

    operator FocusRef() const { return _obj; }
    FocusRef operator ->() { return _obj; }

    operator FrescoObject() const;
    static FocusRef _narrow(BaseObjectRef p);
    static _FocusExpr _narrow(const BaseObject& r);

    static FocusRef _duplicate(FocusRef obj);
    static _FocusExpr _duplicate(const Focus& r);
};

class _FocusExpr : public Focus {
public:
    _FocusExpr(FocusRef p) { _obj = p; }
    _FocusExpr(const Focus& r) { _obj = r._obj; }
    _FocusExpr(const _FocusExpr& r) { _obj = r._obj; }
    ~_FocusExpr();
};

class _FocusElem {
public:
    FocusRef _obj;

    _FocusElem(FocusRef p) { _obj = p; }
    operator FocusRef() const { return _obj; }
    FocusRef operator ->() { return _obj; }
};

class FocusType : public FrescoObjectType {
protected:
    FocusType();
    virtual ~FocusType();
public:
    virtual void add_focus_interest(ViewerRef v);
    virtual void receive_focus_below(ViewerRef v, Boolean temporary);
    virtual void lose_focus_below(ViewerRef v, Boolean temporary);
    virtual void map_keystroke(Event::KeySym k, ActionRef a);
    virtual void map_keychord(const Event::KeyChord& k, ActionRef a);

    _FocusExpr _ref();
    virtual TypeObjId _tid();
};

class FocusStub : public FocusType {
public:
    FocusStub(Exchange*);
    ~FocusStub();

    static BaseObjectRef _create(Exchange*);
    Exchange* _exchange();
protected:
    Exchange* exch_;
};

inline FocusRef Focus::_duplicate(FocusRef obj) {
    return (FocusRef)_BaseObject__duplicate(obj, &FocusStub::_create);
}
inline Focus& Focus::operator =(FocusRef p) {
    _BaseObject__release(_obj);
    _obj = Focus::_duplicate(p);
    return *this;
}
inline Focus::Focus(const Focus& r) {
    _obj = Focus::_duplicate(r._obj);
}
inline Focus& Focus::operator =(const Focus& r) {
    _BaseObject__release(_obj);
    _obj = Focus::_duplicate(r._obj);
    return *this;
}
inline Focus::Focus(const _FocusExpr& r) {
    _obj = r._obj;
    ((_FocusExpr*)&r)->_obj = 0;
}
inline Focus& Focus::operator =(const _FocusExpr& r) {
    _BaseObject__release(_obj);
    _obj = r._obj;
    ((_FocusExpr*)&r)->_obj = 0;
    return *this;
}
inline Focus::Focus(const _FocusElem& e) {
    _obj = Focus::_duplicate(e._obj);
}
inline Focus& Focus::operator =(const _FocusElem& e) {
    _BaseObject__release(_obj);
    _obj = Focus::_duplicate(e._obj);
    return *this;
}
inline Focus::~Focus() {
    _BaseObject__release(_obj);
}
inline _FocusExpr Focus::_narrow(const BaseObject& r) {
    return _narrow(r._obj);
}
inline _FocusExpr Focus::_duplicate(const Focus& r) {
    return _duplicate(r._obj);
}
inline Focus::operator FrescoObject() const {
    return _FrescoObjectExpr((FrescoObjectRef)_BaseObject__duplicate(_obj, &FrescoObjectStub::_create));
}
inline _FocusExpr::~_FocusExpr() { }
inline _FocusExpr FocusType::_ref() { return this; }

#endif
