/* DO NOT EDIT -- Automatically generated from Interfaces/viewer.idl */

#ifndef Interfaces_viewer_h
#define Interfaces_viewer_h

#include <X11/Fresco/_enter.h>
#include <X11/Fresco/glyph.h>

class FocusType;
typedef FocusType* FocusRef;
typedef FocusRef Focus_in;
class Focus;
class Focus_tmp;
class Focus_var;

class EventType;
typedef EventType* EventRef;
typedef EventRef Event_in;
class Event;
class Event_tmp;
class Event_var;

class Event {
public:
    EventRef _obj_;

    Event() { _obj_ = 0; }
    Event(EventRef p) { _obj_ = p; }
    Event& operator =(EventRef p);
    Event(const Event&);
    Event& operator =(const Event& r);
    Event(const Event_tmp&);
    Event& operator =(const Event_tmp&);
    Event(const Event_var&);
    Event& operator =(const Event_var&);
    ~Event();

    EventRef operator ->() { return _obj_; }

    operator Event_in() const { return _obj_; }
    operator FrescoObject() const;
    static EventRef _narrow(BaseObjectRef p);
    static Event_tmp _narrow(const BaseObject& r);

    static EventRef _duplicate(EventRef obj);
    static Event_tmp _duplicate(const Event& r);
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

class Event_tmp : public Event {
public:
    Event_tmp(EventRef p) { _obj_ = p; }
    Event_tmp(const Event& r);
    Event_tmp(const Event_tmp& r);
    ~Event_tmp();
};

class Event_var {
public:
    EventRef _obj_;

    Event_var(EventRef p) { _obj_ = p; }
    operator EventRef() const { return _obj_; }
    EventRef operator ->() { return _obj_; }
};

class EventType : public FrescoObjectType {
protected:
    EventType();
    virtual ~EventType();
public:
    virtual Event::TypeId type();
    virtual Event::TimeStamp time();
    virtual Boolean positional();
    virtual Coord pointer_x();
    virtual Coord pointer_y();
    virtual Event::ButtonIndex pointer_button();
    virtual Boolean button_is_down(Event::ButtonIndex b);
    virtual Boolean modifier_is_down(Event::Modifier m);
    virtual Event::KeySym key();
    virtual CharCode character();
    virtual void unread();
    EventRef _obj() { return this; }
    void* _this();
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

class ViewerType;
typedef ViewerType* ViewerRef;
typedef ViewerRef Viewer_in;
class Viewer;
class Viewer_tmp;
class Viewer_var;

class Viewer {
public:
    ViewerRef _obj_;

    Viewer() { _obj_ = 0; }
    Viewer(ViewerRef p) { _obj_ = p; }
    Viewer& operator =(ViewerRef p);
    Viewer(const Viewer&);
    Viewer& operator =(const Viewer& r);
    Viewer(const Viewer_tmp&);
    Viewer& operator =(const Viewer_tmp&);
    Viewer(const Viewer_var&);
    Viewer& operator =(const Viewer_var&);
    ~Viewer();

    ViewerRef operator ->() { return _obj_; }

    operator Viewer_in() const { return _obj_; }
    operator Glyph() const;
    operator FrescoObject() const;
    static ViewerRef _narrow(BaseObjectRef p);
    static Viewer_tmp _narrow(const BaseObject& r);

    static ViewerRef _duplicate(ViewerRef obj);
    static Viewer_tmp _duplicate(const Viewer& r);
};

class Viewer_tmp : public Viewer {
public:
    Viewer_tmp(ViewerRef p) { _obj_ = p; }
    Viewer_tmp(const Viewer& r);
    Viewer_tmp(const Viewer_tmp& r);
    ~Viewer_tmp();
};

class Viewer_var {
public:
    ViewerRef _obj_;

    Viewer_var(ViewerRef p) { _obj_ = p; }
    operator ViewerRef() const { return _obj_; }
    ViewerRef operator ->() { return _obj_; }
};

class ViewerType : public GlyphType {
protected:
    ViewerType();
    virtual ~ViewerType();
public:
    Viewer_tmp parent_viewer() {
        return _c_parent_viewer();
    }
    virtual ViewerRef _c_parent_viewer();
    Viewer_tmp next_viewer() {
        return _c_next_viewer();
    }
    virtual ViewerRef _c_next_viewer();
    Viewer_tmp prev_viewer() {
        return _c_prev_viewer();
    }
    virtual ViewerRef _c_prev_viewer();
    Viewer_tmp first_viewer() {
        return _c_first_viewer();
    }
    virtual ViewerRef _c_first_viewer();
    Viewer_tmp last_viewer() {
        return _c_last_viewer();
    }
    virtual ViewerRef _c_last_viewer();
    virtual void append_viewer(Viewer_in v);
    virtual void prepend_viewer(Viewer_in v);
    virtual void insert_viewer(Viewer_in v);
    virtual void replace_viewer(Viewer_in v);
    virtual void remove_viewer();
    virtual void set_viewer_links(Viewer_in parent, Viewer_in prev, Viewer_in next);
    virtual void set_first_viewer(Viewer_in v);
    virtual void set_last_viewer(Viewer_in v);
    Focus_tmp request_focus(Viewer_in requestor, Boolean temporary);
    virtual FocusRef _c_request_focus(Viewer_in requestor, Boolean temporary);
    virtual Boolean receive_focus(Focus_in f, Boolean primary);
    virtual void lose_focus(Boolean temporary);
    virtual Boolean first_focus();
    virtual Boolean last_focus();
    virtual Boolean next_focus();
    virtual Boolean prev_focus();
    virtual Boolean handle(GlyphTraversal_in t, Event_in e);
    virtual void close();
    ViewerRef _obj() { return this; }
    void* _this();
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

class Focus {
public:
    FocusRef _obj_;

    Focus() { _obj_ = 0; }
    Focus(FocusRef p) { _obj_ = p; }
    Focus& operator =(FocusRef p);
    Focus(const Focus&);
    Focus& operator =(const Focus& r);
    Focus(const Focus_tmp&);
    Focus& operator =(const Focus_tmp&);
    Focus(const Focus_var&);
    Focus& operator =(const Focus_var&);
    ~Focus();

    FocusRef operator ->() { return _obj_; }

    operator Focus_in() const { return _obj_; }
    operator FrescoObject() const;
    static FocusRef _narrow(BaseObjectRef p);
    static Focus_tmp _narrow(const BaseObject& r);

    static FocusRef _duplicate(FocusRef obj);
    static Focus_tmp _duplicate(const Focus& r);
};

class Focus_tmp : public Focus {
public:
    Focus_tmp(FocusRef p) { _obj_ = p; }
    Focus_tmp(const Focus& r);
    Focus_tmp(const Focus_tmp& r);
    ~Focus_tmp();
};

class Focus_var {
public:
    FocusRef _obj_;

    Focus_var(FocusRef p) { _obj_ = p; }
    operator FocusRef() const { return _obj_; }
    FocusRef operator ->() { return _obj_; }
};

class FocusType : public FrescoObjectType {
protected:
    FocusType();
    virtual ~FocusType();
public:
    virtual void add_focus_interest(Viewer_in v);
    virtual void receive_focus_below(Viewer_in v, Boolean temporary);
    virtual void lose_focus_below(Viewer_in v, Boolean temporary);
    virtual void map_keystroke(Event::KeySym k, Action_in a);
    virtual void map_keychord(const Event::KeyChord& k, Action_in a);
    FocusRef _obj() { return this; }
    void* _this();
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

inline EventRef Event::_duplicate(EventRef obj) {
    return (EventRef)_BaseObject__duplicate(obj, &EventStub::_create);
}
inline Event& Event::operator =(EventRef p) {
    _BaseObject__release(_obj_);
    _obj_ = Event::_duplicate(p);
    return *this;
}
inline Event::Event(const Event& r) {
    _obj_ = Event::_duplicate(r._obj_);
}
inline Event& Event::operator =(const Event& r) {
    _BaseObject__release(_obj_);
    _obj_ = Event::_duplicate(r._obj_);
    return *this;
}
inline Event::Event(const Event_tmp& r) {
    _obj_ = r._obj_;
    ((Event_tmp*)&r)->_obj_ = 0;
}
inline Event& Event::operator =(const Event_tmp& r) {
    _BaseObject__release(_obj_);
    _obj_ = r._obj_;
    ((Event_tmp*)&r)->_obj_ = 0;
    return *this;
}
inline Event::Event(const Event_var& e) {
    _obj_ = Event::_duplicate(e._obj_);
}
inline Event& Event::operator =(const Event_var& e) {
    _BaseObject__release(_obj_);
    _obj_ = Event::_duplicate(e._obj_);
    return *this;
}
inline Event::~Event() {
    _BaseObject__release(_obj_);
}
inline Event_tmp Event::_narrow(const BaseObject& r) {
    return _narrow(r._obj_);
}
inline Event_tmp Event::_duplicate(const Event& r) {
    return _duplicate(r._obj_);
}
inline Event::operator FrescoObject() const {
    return FrescoObject_tmp((FrescoObjectRef)_BaseObject__duplicate((FrescoObjectRef)_obj_, &FrescoObjectStub::_create));
}
inline Event_tmp::Event_tmp(const Event& r) {
    _obj_ = Event::_duplicate(r._obj_);
}
inline Event_tmp::Event_tmp(const Event_tmp& r) {
    _obj_ = r._obj_;
    ((Event_tmp*)&r)->_obj_ = 0;
}
inline Event_tmp::~Event_tmp() { }

inline ViewerRef Viewer::_duplicate(ViewerRef obj) {
    return (ViewerRef)_BaseObject__duplicate(obj, &ViewerStub::_create);
}
inline Viewer& Viewer::operator =(ViewerRef p) {
    _BaseObject__release(_obj_);
    _obj_ = Viewer::_duplicate(p);
    return *this;
}
inline Viewer::Viewer(const Viewer& r) {
    _obj_ = Viewer::_duplicate(r._obj_);
}
inline Viewer& Viewer::operator =(const Viewer& r) {
    _BaseObject__release(_obj_);
    _obj_ = Viewer::_duplicate(r._obj_);
    return *this;
}
inline Viewer::Viewer(const Viewer_tmp& r) {
    _obj_ = r._obj_;
    ((Viewer_tmp*)&r)->_obj_ = 0;
}
inline Viewer& Viewer::operator =(const Viewer_tmp& r) {
    _BaseObject__release(_obj_);
    _obj_ = r._obj_;
    ((Viewer_tmp*)&r)->_obj_ = 0;
    return *this;
}
inline Viewer::Viewer(const Viewer_var& e) {
    _obj_ = Viewer::_duplicate(e._obj_);
}
inline Viewer& Viewer::operator =(const Viewer_var& e) {
    _BaseObject__release(_obj_);
    _obj_ = Viewer::_duplicate(e._obj_);
    return *this;
}
inline Viewer::~Viewer() {
    _BaseObject__release(_obj_);
}
inline Viewer_tmp Viewer::_narrow(const BaseObject& r) {
    return _narrow(r._obj_);
}
inline Viewer_tmp Viewer::_duplicate(const Viewer& r) {
    return _duplicate(r._obj_);
}
inline Viewer::operator Glyph() const {
    return Glyph_tmp((GlyphRef)_BaseObject__duplicate((GlyphRef)_obj_, &GlyphStub::_create));
}
inline Viewer::operator FrescoObject() const {
    return FrescoObject_tmp((FrescoObjectRef)_BaseObject__duplicate((GlyphRef)(FrescoObjectRef)_obj_, &FrescoObjectStub::_create));
}
inline Viewer_tmp::Viewer_tmp(const Viewer& r) {
    _obj_ = Viewer::_duplicate(r._obj_);
}
inline Viewer_tmp::Viewer_tmp(const Viewer_tmp& r) {
    _obj_ = r._obj_;
    ((Viewer_tmp*)&r)->_obj_ = 0;
}
inline Viewer_tmp::~Viewer_tmp() { }

inline FocusRef Focus::_duplicate(FocusRef obj) {
    return (FocusRef)_BaseObject__duplicate(obj, &FocusStub::_create);
}
inline Focus& Focus::operator =(FocusRef p) {
    _BaseObject__release(_obj_);
    _obj_ = Focus::_duplicate(p);
    return *this;
}
inline Focus::Focus(const Focus& r) {
    _obj_ = Focus::_duplicate(r._obj_);
}
inline Focus& Focus::operator =(const Focus& r) {
    _BaseObject__release(_obj_);
    _obj_ = Focus::_duplicate(r._obj_);
    return *this;
}
inline Focus::Focus(const Focus_tmp& r) {
    _obj_ = r._obj_;
    ((Focus_tmp*)&r)->_obj_ = 0;
}
inline Focus& Focus::operator =(const Focus_tmp& r) {
    _BaseObject__release(_obj_);
    _obj_ = r._obj_;
    ((Focus_tmp*)&r)->_obj_ = 0;
    return *this;
}
inline Focus::Focus(const Focus_var& e) {
    _obj_ = Focus::_duplicate(e._obj_);
}
inline Focus& Focus::operator =(const Focus_var& e) {
    _BaseObject__release(_obj_);
    _obj_ = Focus::_duplicate(e._obj_);
    return *this;
}
inline Focus::~Focus() {
    _BaseObject__release(_obj_);
}
inline Focus_tmp Focus::_narrow(const BaseObject& r) {
    return _narrow(r._obj_);
}
inline Focus_tmp Focus::_duplicate(const Focus& r) {
    return _duplicate(r._obj_);
}
inline Focus::operator FrescoObject() const {
    return FrescoObject_tmp((FrescoObjectRef)_BaseObject__duplicate((FrescoObjectRef)_obj_, &FrescoObjectStub::_create));
}
inline Focus_tmp::Focus_tmp(const Focus& r) {
    _obj_ = Focus::_duplicate(r._obj_);
}
inline Focus_tmp::Focus_tmp(const Focus_tmp& r) {
    _obj_ = r._obj_;
    ((Focus_tmp*)&r)->_obj_ = 0;
}
inline Focus_tmp::~Focus_tmp() { }

#endif
