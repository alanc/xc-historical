/* DO NOT EDIT -- Automatically generated from Interfaces/types.idl */

#ifndef Interfaces_types_h
#define Interfaces_types_h

#include <X11/Fresco/_enter.h>
#include <X11/Fresco/Ox/object.h>

typedef ULong Tag;

class FrescoObjectType;
typedef FrescoObjectType* FrescoObjectRef;
class FrescoObject;
class _FrescoObjectExpr;
class _FrescoObjectElem;

class FrescoObject {
public:
    FrescoObjectRef _obj;

    FrescoObject() { _obj = 0; }
    FrescoObject(FrescoObjectRef p) { _obj = p; }
    FrescoObject& operator =(FrescoObjectRef p);
    FrescoObject(const FrescoObject&);
    FrescoObject& operator =(const FrescoObject& r);
    FrescoObject(const _FrescoObjectExpr&);
    FrescoObject& operator =(const _FrescoObjectExpr&);
    FrescoObject(const _FrescoObjectElem&);
    FrescoObject& operator =(const _FrescoObjectElem&);
    ~FrescoObject();

    operator FrescoObjectRef() const { return _obj; }
    FrescoObjectRef operator ->() { return _obj; }

    static FrescoObjectRef _narrow(BaseObjectRef p);
    static _FrescoObjectExpr _narrow(const BaseObject& r);

    static FrescoObjectRef _duplicate(FrescoObjectRef obj);
    static _FrescoObjectExpr _duplicate(const FrescoObject& r);
};

class _FrescoObjectExpr : public FrescoObject {
public:
    _FrescoObjectExpr(FrescoObjectRef p) { _obj = p; }
    _FrescoObjectExpr(const FrescoObject& r) { _obj = r._obj; }
    _FrescoObjectExpr(const _FrescoObjectExpr& r) { _obj = r._obj; }
    ~_FrescoObjectExpr();
};

class _FrescoObjectElem {
public:
    FrescoObjectRef _obj;

    _FrescoObjectElem(FrescoObjectRef p) { _obj = p; }
    operator FrescoObjectRef() const { return _obj; }
    FrescoObjectRef operator ->() { return _obj; }
};

class FrescoObjectType : public BaseObjectType {
protected:
    FrescoObjectType();
    virtual ~FrescoObjectType();
public:
    virtual Long ref__(Long references);
    virtual Tag attach(FrescoObjectRef observer);
    virtual void detach(Tag attach_tag);
    virtual void disconnect();
    virtual void notify_observers();
    virtual void update();

    _FrescoObjectExpr _ref();
    virtual TypeObjId _tid();
};

class FrescoObjectStub : public FrescoObjectType {
public:
    FrescoObjectStub(Exchange*);
    ~FrescoObjectStub();

    static BaseObjectRef _create(Exchange*);
    Exchange* _exchange();
protected:
    Exchange* exch_;
};

inline FrescoObjectRef FrescoObject::_duplicate(FrescoObjectRef obj) {
    return (FrescoObjectRef)_BaseObject__duplicate(obj, &FrescoObjectStub::_create);
}
inline FrescoObject& FrescoObject::operator =(FrescoObjectRef p) {
    _BaseObject__release(_obj);
    _obj = FrescoObject::_duplicate(p);
    return *this;
}
inline FrescoObject::FrescoObject(const FrescoObject& r) {
    _obj = FrescoObject::_duplicate(r._obj);
}
inline FrescoObject& FrescoObject::operator =(const FrescoObject& r) {
    _BaseObject__release(_obj);
    _obj = FrescoObject::_duplicate(r._obj);
    return *this;
}
inline FrescoObject::FrescoObject(const _FrescoObjectExpr& r) {
    _obj = r._obj;
    ((_FrescoObjectExpr*)&r)->_obj = 0;
}
inline FrescoObject& FrescoObject::operator =(const _FrescoObjectExpr& r) {
    _BaseObject__release(_obj);
    _obj = r._obj;
    ((_FrescoObjectExpr*)&r)->_obj = 0;
    return *this;
}
inline FrescoObject::FrescoObject(const _FrescoObjectElem& e) {
    _obj = FrescoObject::_duplicate(e._obj);
}
inline FrescoObject& FrescoObject::operator =(const _FrescoObjectElem& e) {
    _BaseObject__release(_obj);
    _obj = FrescoObject::_duplicate(e._obj);
    return *this;
}
inline FrescoObject::~FrescoObject() {
    _BaseObject__release(_obj);
}
inline _FrescoObjectExpr FrescoObject::_narrow(const BaseObject& r) {
    return _narrow(r._obj);
}
inline _FrescoObjectExpr FrescoObject::_duplicate(const FrescoObject& r) {
    return _duplicate(r._obj);
}
inline _FrescoObjectExpr::~_FrescoObjectExpr() { }
inline _FrescoObjectExpr FrescoObjectType::_ref() { return this; }

typedef Long CharCode;

class CharStringType;
typedef CharStringType* CharStringRef;
class CharString;
class _CharStringExpr;
class _CharStringElem;

class CharString {
public:
    CharStringRef _obj;

    CharString() { _obj = 0; }
    CharString(CharStringRef p) { _obj = p; }
    CharString& operator =(CharStringRef p);
    CharString(const CharString&);
    CharString& operator =(const CharString& r);
    CharString(const _CharStringExpr&);
    CharString& operator =(const _CharStringExpr&);
    CharString(const _CharStringElem&);
    CharString& operator =(const _CharStringElem&);
    ~CharString();

    operator CharStringRef() const { return _obj; }
    CharStringRef operator ->() { return _obj; }

    operator FrescoObject() const;
    static CharStringRef _narrow(BaseObjectRef p);
    static _CharStringExpr _narrow(const BaseObject& r);

    static CharStringRef _duplicate(CharStringRef obj);
    static _CharStringExpr _duplicate(const CharString& r);
    class Data {
    public:
        long _maximum, _length; CharCode* _buffer;

        Data() { _maximum = _length = 0; _buffer = 0; }
        Data(long m, long n, CharCode* e) {
            _maximum = m; _length = n; _buffer = e;
        }
        Data(const Data& _s) { _buffer = 0; *this = _s; }
        Data& operator =(const Data&);
        ~Data() { delete [] _buffer; }
    };
    class CharData {
    public:
        long _maximum, _length; Char* _buffer;

        CharData() { _maximum = _length = 0; _buffer = 0; }
        CharData(long m, long n, Char* e) {
            _maximum = m; _length = n; _buffer = e;
        }
        CharData(const CharData& _s) { _buffer = 0; *this = _s; }
        CharData& operator =(const CharData&);
        ~CharData() { delete [] _buffer; }
    };
    typedef ULong HashValue;
};

class _CharStringExpr : public CharString {
public:
    _CharStringExpr(CharStringRef p) { _obj = p; }
    _CharStringExpr(const CharString& r) { _obj = r._obj; }
    _CharStringExpr(const _CharStringExpr& r) { _obj = r._obj; }
    ~_CharStringExpr();
};

class _CharStringElem {
public:
    CharStringRef _obj;

    _CharStringElem(CharStringRef p) { _obj = p; }
    operator CharStringRef() const { return _obj; }
    CharStringRef operator ->() { return _obj; }
};

class CharStringType : public FrescoObjectType {
protected:
    CharStringType();
    virtual ~CharStringType();
public:
    virtual CharString::HashValue hash();
    virtual Long count();
    virtual void copy(CharStringRef s);
    virtual Boolean equal(CharStringRef s);
    virtual Boolean case_insensitive_equal(CharStringRef s);
    virtual void get_data(CharString::Data& d);
    virtual void get_char_data(CharString::CharData& d);
    virtual void put_data(const CharString::Data& d);
    virtual void put_char_data(const CharString::CharData& d);

    _CharStringExpr _ref();
    virtual TypeObjId _tid();
};

class CharStringStub : public CharStringType {
public:
    CharStringStub(Exchange*);
    ~CharStringStub();

    static BaseObjectRef _create(Exchange*);
    Exchange* _exchange();
protected:
    Exchange* exch_;
};

inline CharStringRef CharString::_duplicate(CharStringRef obj) {
    return (CharStringRef)_BaseObject__duplicate(obj, &CharStringStub::_create);
}
inline CharString& CharString::operator =(CharStringRef p) {
    _BaseObject__release(_obj);
    _obj = CharString::_duplicate(p);
    return *this;
}
inline CharString::CharString(const CharString& r) {
    _obj = CharString::_duplicate(r._obj);
}
inline CharString& CharString::operator =(const CharString& r) {
    _BaseObject__release(_obj);
    _obj = CharString::_duplicate(r._obj);
    return *this;
}
inline CharString::CharString(const _CharStringExpr& r) {
    _obj = r._obj;
    ((_CharStringExpr*)&r)->_obj = 0;
}
inline CharString& CharString::operator =(const _CharStringExpr& r) {
    _BaseObject__release(_obj);
    _obj = r._obj;
    ((_CharStringExpr*)&r)->_obj = 0;
    return *this;
}
inline CharString::CharString(const _CharStringElem& e) {
    _obj = CharString::_duplicate(e._obj);
}
inline CharString& CharString::operator =(const _CharStringElem& e) {
    _BaseObject__release(_obj);
    _obj = CharString::_duplicate(e._obj);
    return *this;
}
inline CharString::~CharString() {
    _BaseObject__release(_obj);
}
inline _CharStringExpr CharString::_narrow(const BaseObject& r) {
    return _narrow(r._obj);
}
inline _CharStringExpr CharString::_duplicate(const CharString& r) {
    return _duplicate(r._obj);
}
inline CharString::operator FrescoObject() const {
    return _FrescoObjectExpr((FrescoObjectRef)_BaseObject__duplicate(_obj, &FrescoObjectStub::_create));
}
inline _CharStringExpr::~_CharStringExpr() { }
inline _CharStringExpr CharStringType::_ref() { return this; }

typedef Float Coord;

typedef Long PixelCoord;

enum Axis {
    X_axis, Y_axis, Z_axis
};

struct Vertex {
    Coord x, y, z;
};

typedef Float Alignment;

class ActionType;
typedef ActionType* ActionRef;
class Action;
class _ActionExpr;
class _ActionElem;

class Action {
public:
    ActionRef _obj;

    Action() { _obj = 0; }
    Action(ActionRef p) { _obj = p; }
    Action& operator =(ActionRef p);
    Action(const Action&);
    Action& operator =(const Action& r);
    Action(const _ActionExpr&);
    Action& operator =(const _ActionExpr&);
    Action(const _ActionElem&);
    Action& operator =(const _ActionElem&);
    ~Action();

    operator ActionRef() const { return _obj; }
    ActionRef operator ->() { return _obj; }

    operator FrescoObject() const;
    static ActionRef _narrow(BaseObjectRef p);
    static _ActionExpr _narrow(const BaseObject& r);

    static ActionRef _duplicate(ActionRef obj);
    static _ActionExpr _duplicate(const Action& r);
};

class _ActionExpr : public Action {
public:
    _ActionExpr(ActionRef p) { _obj = p; }
    _ActionExpr(const Action& r) { _obj = r._obj; }
    _ActionExpr(const _ActionExpr& r) { _obj = r._obj; }
    ~_ActionExpr();
};

class _ActionElem {
public:
    ActionRef _obj;

    _ActionElem(ActionRef p) { _obj = p; }
    operator ActionRef() const { return _obj; }
    ActionRef operator ->() { return _obj; }
};

class ActionType : public FrescoObjectType {
protected:
    ActionType();
    virtual ~ActionType();
public:
    virtual void execute();
    virtual Boolean reversible();
    virtual void unexecute();

    _ActionExpr _ref();
    virtual TypeObjId _tid();
};

class ActionStub : public ActionType {
public:
    ActionStub(Exchange*);
    ~ActionStub();

    static BaseObjectRef _create(Exchange*);
    Exchange* _exchange();
protected:
    Exchange* exch_;
};

inline ActionRef Action::_duplicate(ActionRef obj) {
    return (ActionRef)_BaseObject__duplicate(obj, &ActionStub::_create);
}
inline Action& Action::operator =(ActionRef p) {
    _BaseObject__release(_obj);
    _obj = Action::_duplicate(p);
    return *this;
}
inline Action::Action(const Action& r) {
    _obj = Action::_duplicate(r._obj);
}
inline Action& Action::operator =(const Action& r) {
    _BaseObject__release(_obj);
    _obj = Action::_duplicate(r._obj);
    return *this;
}
inline Action::Action(const _ActionExpr& r) {
    _obj = r._obj;
    ((_ActionExpr*)&r)->_obj = 0;
}
inline Action& Action::operator =(const _ActionExpr& r) {
    _BaseObject__release(_obj);
    _obj = r._obj;
    ((_ActionExpr*)&r)->_obj = 0;
    return *this;
}
inline Action::Action(const _ActionElem& e) {
    _obj = Action::_duplicate(e._obj);
}
inline Action& Action::operator =(const _ActionElem& e) {
    _BaseObject__release(_obj);
    _obj = Action::_duplicate(e._obj);
    return *this;
}
inline Action::~Action() {
    _BaseObject__release(_obj);
}
inline _ActionExpr Action::_narrow(const BaseObject& r) {
    return _narrow(r._obj);
}
inline _ActionExpr Action::_duplicate(const Action& r) {
    return _duplicate(r._obj);
}
inline Action::operator FrescoObject() const {
    return _FrescoObjectExpr((FrescoObjectRef)_BaseObject__duplicate(_obj, &FrescoObjectStub::_create));
}
inline _ActionExpr::~_ActionExpr() { }
inline _ActionExpr ActionType::_ref() { return this; }

class TransformObjType;
typedef TransformObjType* TransformObjRef;
class TransformObj;
class _TransformObjExpr;
class _TransformObjElem;

class RegionType;
typedef RegionType* RegionRef;
class Region;
class _RegionExpr;
class _RegionElem;

class Region {
public:
    RegionRef _obj;

    Region() { _obj = 0; }
    Region(RegionRef p) { _obj = p; }
    Region& operator =(RegionRef p);
    Region(const Region&);
    Region& operator =(const Region& r);
    Region(const _RegionExpr&);
    Region& operator =(const _RegionExpr&);
    Region(const _RegionElem&);
    Region& operator =(const _RegionElem&);
    ~Region();

    operator RegionRef() const { return _obj; }
    RegionRef operator ->() { return _obj; }

    operator FrescoObject() const;
    static RegionRef _narrow(BaseObjectRef p);
    static _RegionExpr _narrow(const BaseObject& r);

    static RegionRef _duplicate(RegionRef obj);
    static _RegionExpr _duplicate(const Region& r);
    struct BoundingSpan {
        Coord begin, end;
        Coord origin, length;
        Alignment align;
    };
};

class _RegionExpr : public Region {
public:
    _RegionExpr(RegionRef p) { _obj = p; }
    _RegionExpr(const Region& r) { _obj = r._obj; }
    _RegionExpr(const _RegionExpr& r) { _obj = r._obj; }
    ~_RegionExpr();
};

class _RegionElem {
public:
    RegionRef _obj;

    _RegionElem(RegionRef p) { _obj = p; }
    operator RegionRef() const { return _obj; }
    RegionRef operator ->() { return _obj; }
};

class RegionType : public FrescoObjectType {
protected:
    RegionType();
    virtual ~RegionType();
public:
    virtual Boolean contains(const Vertex& v);
    virtual Boolean contains_plane(const Vertex& v, Axis a);
    virtual Boolean intersects(RegionRef r);
    virtual void copy(RegionRef r);
    virtual void merge_intersect(RegionRef r);
    virtual void merge_union(RegionRef r);
    virtual void subtract(RegionRef r);
    virtual void transform(TransformObjRef t);
    virtual void bounds(Vertex& lower, Vertex& upper);
    virtual void origin(Vertex& v);
    virtual void span(Axis a, Region::BoundingSpan& s);

    _RegionExpr _ref();
    virtual TypeObjId _tid();
};

class RegionStub : public RegionType {
public:
    RegionStub(Exchange*);
    ~RegionStub();

    static BaseObjectRef _create(Exchange*);
    Exchange* _exchange();
protected:
    Exchange* exch_;
};

inline RegionRef Region::_duplicate(RegionRef obj) {
    return (RegionRef)_BaseObject__duplicate(obj, &RegionStub::_create);
}
inline Region& Region::operator =(RegionRef p) {
    _BaseObject__release(_obj);
    _obj = Region::_duplicate(p);
    return *this;
}
inline Region::Region(const Region& r) {
    _obj = Region::_duplicate(r._obj);
}
inline Region& Region::operator =(const Region& r) {
    _BaseObject__release(_obj);
    _obj = Region::_duplicate(r._obj);
    return *this;
}
inline Region::Region(const _RegionExpr& r) {
    _obj = r._obj;
    ((_RegionExpr*)&r)->_obj = 0;
}
inline Region& Region::operator =(const _RegionExpr& r) {
    _BaseObject__release(_obj);
    _obj = r._obj;
    ((_RegionExpr*)&r)->_obj = 0;
    return *this;
}
inline Region::Region(const _RegionElem& e) {
    _obj = Region::_duplicate(e._obj);
}
inline Region& Region::operator =(const _RegionElem& e) {
    _BaseObject__release(_obj);
    _obj = Region::_duplicate(e._obj);
    return *this;
}
inline Region::~Region() {
    _BaseObject__release(_obj);
}
inline _RegionExpr Region::_narrow(const BaseObject& r) {
    return _narrow(r._obj);
}
inline _RegionExpr Region::_duplicate(const Region& r) {
    return _duplicate(r._obj);
}
inline Region::operator FrescoObject() const {
    return _FrescoObjectExpr((FrescoObjectRef)_BaseObject__duplicate(_obj, &FrescoObjectStub::_create));
}
inline _RegionExpr::~_RegionExpr() { }
inline _RegionExpr RegionType::_ref() { return this; }

class StyleValueType;
typedef StyleValueType* StyleValueRef;
class StyleValue;
class _StyleValueExpr;
class _StyleValueElem;

class StyleValue {
public:
    StyleValueRef _obj;

    StyleValue() { _obj = 0; }
    StyleValue(StyleValueRef p) { _obj = p; }
    StyleValue& operator =(StyleValueRef p);
    StyleValue(const StyleValue&);
    StyleValue& operator =(const StyleValue& r);
    StyleValue(const _StyleValueExpr&);
    StyleValue& operator =(const _StyleValueExpr&);
    StyleValue(const _StyleValueElem&);
    StyleValue& operator =(const _StyleValueElem&);
    ~StyleValue();

    operator StyleValueRef() const { return _obj; }
    StyleValueRef operator ->() { return _obj; }

    operator FrescoObject() const;
    static StyleValueRef _narrow(BaseObjectRef p);
    static _StyleValueExpr _narrow(const BaseObject& r);

    static StyleValueRef _duplicate(StyleValueRef obj);
    static _StyleValueExpr _duplicate(const StyleValue& r);
};

class _StyleValueExpr : public StyleValue {
public:
    _StyleValueExpr(StyleValueRef p) { _obj = p; }
    _StyleValueExpr(const StyleValue& r) { _obj = r._obj; }
    _StyleValueExpr(const _StyleValueExpr& r) { _obj = r._obj; }
    ~_StyleValueExpr();
};

class _StyleValueElem {
public:
    StyleValueRef _obj;

    _StyleValueElem(StyleValueRef p) { _obj = p; }
    operator StyleValueRef() const { return _obj; }
    StyleValueRef operator ->() { return _obj; }
};

class StyleValueType : public FrescoObjectType {
protected:
    StyleValueType();
    virtual ~StyleValueType();
public:
    _CharStringExpr name() {
        return _c_name();
    }
    virtual CharStringRef _c_name();
    virtual Boolean uninitialized();
    virtual Long priority();
    virtual void priority(Long _p);
    virtual Boolean is_on();
    virtual Boolean read_boolean(Boolean& b);
    virtual void write_boolean(Boolean b);
    virtual Boolean read_coord(Coord& c);
    virtual void write_coord(Coord c);
    virtual Boolean read_integer(Long& i);
    virtual void write_integer(Long i);
    virtual Boolean read_real(Double& d);
    virtual void write_real(Double d);
    Boolean read_string(CharString& s) {
        return _c_read_string(s._obj);
    }
    virtual Boolean _c_read_string(CharStringRef& s);
    virtual void write_string(CharStringRef s);
    Boolean read_value(FrescoObject& s) {
        return _c_read_value(s._obj);
    }
    virtual Boolean _c_read_value(FrescoObjectRef& s);
    virtual void write_value(FrescoObjectRef s);
    virtual void lock();
    virtual void unlock();

    _StyleValueExpr _ref();
    virtual TypeObjId _tid();
};

class StyleValueStub : public StyleValueType {
public:
    StyleValueStub(Exchange*);
    ~StyleValueStub();

    static BaseObjectRef _create(Exchange*);
    Exchange* _exchange();
protected:
    Exchange* exch_;
};

inline StyleValueRef StyleValue::_duplicate(StyleValueRef obj) {
    return (StyleValueRef)_BaseObject__duplicate(obj, &StyleValueStub::_create);
}
inline StyleValue& StyleValue::operator =(StyleValueRef p) {
    _BaseObject__release(_obj);
    _obj = StyleValue::_duplicate(p);
    return *this;
}
inline StyleValue::StyleValue(const StyleValue& r) {
    _obj = StyleValue::_duplicate(r._obj);
}
inline StyleValue& StyleValue::operator =(const StyleValue& r) {
    _BaseObject__release(_obj);
    _obj = StyleValue::_duplicate(r._obj);
    return *this;
}
inline StyleValue::StyleValue(const _StyleValueExpr& r) {
    _obj = r._obj;
    ((_StyleValueExpr*)&r)->_obj = 0;
}
inline StyleValue& StyleValue::operator =(const _StyleValueExpr& r) {
    _BaseObject__release(_obj);
    _obj = r._obj;
    ((_StyleValueExpr*)&r)->_obj = 0;
    return *this;
}
inline StyleValue::StyleValue(const _StyleValueElem& e) {
    _obj = StyleValue::_duplicate(e._obj);
}
inline StyleValue& StyleValue::operator =(const _StyleValueElem& e) {
    _BaseObject__release(_obj);
    _obj = StyleValue::_duplicate(e._obj);
    return *this;
}
inline StyleValue::~StyleValue() {
    _BaseObject__release(_obj);
}
inline _StyleValueExpr StyleValue::_narrow(const BaseObject& r) {
    return _narrow(r._obj);
}
inline _StyleValueExpr StyleValue::_duplicate(const StyleValue& r) {
    return _duplicate(r._obj);
}
inline StyleValue::operator FrescoObject() const {
    return _FrescoObjectExpr((FrescoObjectRef)_BaseObject__duplicate(_obj, &FrescoObjectStub::_create));
}
inline _StyleValueExpr::~_StyleValueExpr() { }
inline _StyleValueExpr StyleValueType::_ref() { return this; }

class StyleVisitorType;
typedef StyleVisitorType* StyleVisitorRef;
class StyleVisitor;
class _StyleVisitorExpr;
class _StyleVisitorElem;

class StyleObjType;
typedef StyleObjType* StyleObjRef;
class StyleObj;
class _StyleObjExpr;
class _StyleObjElem;

class StyleObj {
public:
    StyleObjRef _obj;

    StyleObj() { _obj = 0; }
    StyleObj(StyleObjRef p) { _obj = p; }
    StyleObj& operator =(StyleObjRef p);
    StyleObj(const StyleObj&);
    StyleObj& operator =(const StyleObj& r);
    StyleObj(const _StyleObjExpr&);
    StyleObj& operator =(const _StyleObjExpr&);
    StyleObj(const _StyleObjElem&);
    StyleObj& operator =(const _StyleObjElem&);
    ~StyleObj();

    operator StyleObjRef() const { return _obj; }
    StyleObjRef operator ->() { return _obj; }

    operator FrescoObject() const;
    static StyleObjRef _narrow(BaseObjectRef p);
    static _StyleObjExpr _narrow(const BaseObject& r);

    static StyleObjRef _duplicate(StyleObjRef obj);
    static _StyleObjExpr _duplicate(const StyleObj& r);
};

class _StyleObjExpr : public StyleObj {
public:
    _StyleObjExpr(StyleObjRef p) { _obj = p; }
    _StyleObjExpr(const StyleObj& r) { _obj = r._obj; }
    _StyleObjExpr(const _StyleObjExpr& r) { _obj = r._obj; }
    ~_StyleObjExpr();
};

class _StyleObjElem {
public:
    StyleObjRef _obj;

    _StyleObjElem(StyleObjRef p) { _obj = p; }
    operator StyleObjRef() const { return _obj; }
    StyleObjRef operator ->() { return _obj; }
};

class StyleObjType : public FrescoObjectType {
protected:
    StyleObjType();
    virtual ~StyleObjType();
public:
    _StyleObjExpr new_style() {
        return _c_new_style();
    }
    virtual StyleObjRef _c_new_style();
    _StyleObjExpr parent_style() {
        return _c_parent_style();
    }
    virtual StyleObjRef _c_parent_style();
    virtual void link_parent(StyleObjRef parent);
    virtual void unlink_parent();
    virtual Tag link_child(StyleObjRef child);
    virtual void unlink_child(Tag link_tag);
    virtual void merge(StyleObjRef s);
    _CharStringExpr name() {
        return _c_name();
    }
    virtual CharStringRef _c_name();
    void name(CharStringRef _p) {
        _c_name(_p);
    }
    virtual void _c_name(CharStringRef _p);
    virtual void alias(CharStringRef s);
    virtual Boolean is_on(CharStringRef name);
    _StyleValueExpr bind(CharStringRef name) {
        return _c_bind(name);
    }
    virtual StyleValueRef _c_bind(CharStringRef name);
    virtual void unbind(CharStringRef name);
    _StyleValueExpr resolve(CharStringRef name) {
        return _c_resolve(name);
    }
    virtual StyleValueRef _c_resolve(CharStringRef name);
    _StyleValueExpr resolve_wildcard(CharStringRef name, StyleObjRef start) {
        return _c_resolve_wildcard(name, start);
    }
    virtual StyleValueRef _c_resolve_wildcard(CharStringRef name, StyleObjRef start);
    virtual Long match(CharStringRef name);
    virtual void visit_aliases(StyleVisitorRef v);
    virtual void visit_attributes(StyleVisitorRef v);
    virtual void visit_styles(StyleVisitorRef v);
    virtual void lock();
    virtual void unlock();

    _StyleObjExpr _ref();
    virtual TypeObjId _tid();
};

class StyleObjStub : public StyleObjType {
public:
    StyleObjStub(Exchange*);
    ~StyleObjStub();

    static BaseObjectRef _create(Exchange*);
    Exchange* _exchange();
protected:
    Exchange* exch_;
};

inline StyleObjRef StyleObj::_duplicate(StyleObjRef obj) {
    return (StyleObjRef)_BaseObject__duplicate(obj, &StyleObjStub::_create);
}
inline StyleObj& StyleObj::operator =(StyleObjRef p) {
    _BaseObject__release(_obj);
    _obj = StyleObj::_duplicate(p);
    return *this;
}
inline StyleObj::StyleObj(const StyleObj& r) {
    _obj = StyleObj::_duplicate(r._obj);
}
inline StyleObj& StyleObj::operator =(const StyleObj& r) {
    _BaseObject__release(_obj);
    _obj = StyleObj::_duplicate(r._obj);
    return *this;
}
inline StyleObj::StyleObj(const _StyleObjExpr& r) {
    _obj = r._obj;
    ((_StyleObjExpr*)&r)->_obj = 0;
}
inline StyleObj& StyleObj::operator =(const _StyleObjExpr& r) {
    _BaseObject__release(_obj);
    _obj = r._obj;
    ((_StyleObjExpr*)&r)->_obj = 0;
    return *this;
}
inline StyleObj::StyleObj(const _StyleObjElem& e) {
    _obj = StyleObj::_duplicate(e._obj);
}
inline StyleObj& StyleObj::operator =(const _StyleObjElem& e) {
    _BaseObject__release(_obj);
    _obj = StyleObj::_duplicate(e._obj);
    return *this;
}
inline StyleObj::~StyleObj() {
    _BaseObject__release(_obj);
}
inline _StyleObjExpr StyleObj::_narrow(const BaseObject& r) {
    return _narrow(r._obj);
}
inline _StyleObjExpr StyleObj::_duplicate(const StyleObj& r) {
    return _duplicate(r._obj);
}
inline StyleObj::operator FrescoObject() const {
    return _FrescoObjectExpr((FrescoObjectRef)_BaseObject__duplicate(_obj, &FrescoObjectStub::_create));
}
inline _StyleObjExpr::~_StyleObjExpr() { }
inline _StyleObjExpr StyleObjType::_ref() { return this; }

class StyleVisitor {
public:
    StyleVisitorRef _obj;

    StyleVisitor() { _obj = 0; }
    StyleVisitor(StyleVisitorRef p) { _obj = p; }
    StyleVisitor& operator =(StyleVisitorRef p);
    StyleVisitor(const StyleVisitor&);
    StyleVisitor& operator =(const StyleVisitor& r);
    StyleVisitor(const _StyleVisitorExpr&);
    StyleVisitor& operator =(const _StyleVisitorExpr&);
    StyleVisitor(const _StyleVisitorElem&);
    StyleVisitor& operator =(const _StyleVisitorElem&);
    ~StyleVisitor();

    operator StyleVisitorRef() const { return _obj; }
    StyleVisitorRef operator ->() { return _obj; }

    operator FrescoObject() const;
    static StyleVisitorRef _narrow(BaseObjectRef p);
    static _StyleVisitorExpr _narrow(const BaseObject& r);

    static StyleVisitorRef _duplicate(StyleVisitorRef obj);
    static _StyleVisitorExpr _duplicate(const StyleVisitor& r);
};

class _StyleVisitorExpr : public StyleVisitor {
public:
    _StyleVisitorExpr(StyleVisitorRef p) { _obj = p; }
    _StyleVisitorExpr(const StyleVisitor& r) { _obj = r._obj; }
    _StyleVisitorExpr(const _StyleVisitorExpr& r) { _obj = r._obj; }
    ~_StyleVisitorExpr();
};

class _StyleVisitorElem {
public:
    StyleVisitorRef _obj;

    _StyleVisitorElem(StyleVisitorRef p) { _obj = p; }
    operator StyleVisitorRef() const { return _obj; }
    StyleVisitorRef operator ->() { return _obj; }
};

class StyleVisitorType : public FrescoObjectType {
protected:
    StyleVisitorType();
    virtual ~StyleVisitorType();
public:
    virtual Boolean visit_alias(CharStringRef name);
    virtual Boolean visit_attribute(StyleValueRef a);
    virtual Boolean visit_style(StyleObjRef s);

    _StyleVisitorExpr _ref();
    virtual TypeObjId _tid();
};

class StyleVisitorStub : public StyleVisitorType {
public:
    StyleVisitorStub(Exchange*);
    ~StyleVisitorStub();

    static BaseObjectRef _create(Exchange*);
    Exchange* _exchange();
protected:
    Exchange* exch_;
};

inline StyleVisitorRef StyleVisitor::_duplicate(StyleVisitorRef obj) {
    return (StyleVisitorRef)_BaseObject__duplicate(obj, &StyleVisitorStub::_create);
}
inline StyleVisitor& StyleVisitor::operator =(StyleVisitorRef p) {
    _BaseObject__release(_obj);
    _obj = StyleVisitor::_duplicate(p);
    return *this;
}
inline StyleVisitor::StyleVisitor(const StyleVisitor& r) {
    _obj = StyleVisitor::_duplicate(r._obj);
}
inline StyleVisitor& StyleVisitor::operator =(const StyleVisitor& r) {
    _BaseObject__release(_obj);
    _obj = StyleVisitor::_duplicate(r._obj);
    return *this;
}
inline StyleVisitor::StyleVisitor(const _StyleVisitorExpr& r) {
    _obj = r._obj;
    ((_StyleVisitorExpr*)&r)->_obj = 0;
}
inline StyleVisitor& StyleVisitor::operator =(const _StyleVisitorExpr& r) {
    _BaseObject__release(_obj);
    _obj = r._obj;
    ((_StyleVisitorExpr*)&r)->_obj = 0;
    return *this;
}
inline StyleVisitor::StyleVisitor(const _StyleVisitorElem& e) {
    _obj = StyleVisitor::_duplicate(e._obj);
}
inline StyleVisitor& StyleVisitor::operator =(const _StyleVisitorElem& e) {
    _BaseObject__release(_obj);
    _obj = StyleVisitor::_duplicate(e._obj);
    return *this;
}
inline StyleVisitor::~StyleVisitor() {
    _BaseObject__release(_obj);
}
inline _StyleVisitorExpr StyleVisitor::_narrow(const BaseObject& r) {
    return _narrow(r._obj);
}
inline _StyleVisitorExpr StyleVisitor::_duplicate(const StyleVisitor& r) {
    return _duplicate(r._obj);
}
inline StyleVisitor::operator FrescoObject() const {
    return _FrescoObjectExpr((FrescoObjectRef)_BaseObject__duplicate(_obj, &FrescoObjectStub::_create));
}
inline _StyleVisitorExpr::~_StyleVisitorExpr() { }
inline _StyleVisitorExpr StyleVisitorType::_ref() { return this; }

class TransformObj {
public:
    TransformObjRef _obj;

    TransformObj() { _obj = 0; }
    TransformObj(TransformObjRef p) { _obj = p; }
    TransformObj& operator =(TransformObjRef p);
    TransformObj(const TransformObj&);
    TransformObj& operator =(const TransformObj& r);
    TransformObj(const _TransformObjExpr&);
    TransformObj& operator =(const _TransformObjExpr&);
    TransformObj(const _TransformObjElem&);
    TransformObj& operator =(const _TransformObjElem&);
    ~TransformObj();

    operator TransformObjRef() const { return _obj; }
    TransformObjRef operator ->() { return _obj; }

    operator FrescoObject() const;
    static TransformObjRef _narrow(BaseObjectRef p);
    static _TransformObjExpr _narrow(const BaseObject& r);

    static TransformObjRef _duplicate(TransformObjRef obj);
    static _TransformObjExpr _duplicate(const TransformObj& r);
    typedef Coord Matrix[4][4];
};

class _TransformObjExpr : public TransformObj {
public:
    _TransformObjExpr(TransformObjRef p) { _obj = p; }
    _TransformObjExpr(const TransformObj& r) { _obj = r._obj; }
    _TransformObjExpr(const _TransformObjExpr& r) { _obj = r._obj; }
    ~_TransformObjExpr();
};

class _TransformObjElem {
public:
    TransformObjRef _obj;

    _TransformObjElem(TransformObjRef p) { _obj = p; }
    operator TransformObjRef() const { return _obj; }
    TransformObjRef operator ->() { return _obj; }
};

class TransformObjType : public FrescoObjectType {
protected:
    TransformObjType();
    virtual ~TransformObjType();
public:
    virtual void load(TransformObjRef t);
    virtual void load_identity();
    virtual void load_matrix(TransformObj::Matrix m);
    virtual void store_matrix(TransformObj::Matrix m);
    virtual Boolean equal(TransformObjRef t);
    virtual Boolean identity();
    virtual Boolean det_is_zero();
    virtual void scale(const Vertex& v);
    virtual void rotate(Float angle, Axis a);
    virtual void translate(const Vertex& v);
    virtual void premultiply(TransformObjRef t);
    virtual void postmultiply(TransformObjRef t);
    virtual void invert();
    virtual void transform(Vertex& v);
    virtual void inverse_transform(Vertex& v);

    _TransformObjExpr _ref();
    virtual TypeObjId _tid();
};

class TransformObjStub : public TransformObjType {
public:
    TransformObjStub(Exchange*);
    ~TransformObjStub();

    static BaseObjectRef _create(Exchange*);
    Exchange* _exchange();
protected:
    Exchange* exch_;
};

inline TransformObjRef TransformObj::_duplicate(TransformObjRef obj) {
    return (TransformObjRef)_BaseObject__duplicate(obj, &TransformObjStub::_create);
}
inline TransformObj& TransformObj::operator =(TransformObjRef p) {
    _BaseObject__release(_obj);
    _obj = TransformObj::_duplicate(p);
    return *this;
}
inline TransformObj::TransformObj(const TransformObj& r) {
    _obj = TransformObj::_duplicate(r._obj);
}
inline TransformObj& TransformObj::operator =(const TransformObj& r) {
    _BaseObject__release(_obj);
    _obj = TransformObj::_duplicate(r._obj);
    return *this;
}
inline TransformObj::TransformObj(const _TransformObjExpr& r) {
    _obj = r._obj;
    ((_TransformObjExpr*)&r)->_obj = 0;
}
inline TransformObj& TransformObj::operator =(const _TransformObjExpr& r) {
    _BaseObject__release(_obj);
    _obj = r._obj;
    ((_TransformObjExpr*)&r)->_obj = 0;
    return *this;
}
inline TransformObj::TransformObj(const _TransformObjElem& e) {
    _obj = TransformObj::_duplicate(e._obj);
}
inline TransformObj& TransformObj::operator =(const _TransformObjElem& e) {
    _BaseObject__release(_obj);
    _obj = TransformObj::_duplicate(e._obj);
    return *this;
}
inline TransformObj::~TransformObj() {
    _BaseObject__release(_obj);
}
inline _TransformObjExpr TransformObj::_narrow(const BaseObject& r) {
    return _narrow(r._obj);
}
inline _TransformObjExpr TransformObj::_duplicate(const TransformObj& r) {
    return _duplicate(r._obj);
}
inline TransformObj::operator FrescoObject() const {
    return _FrescoObjectExpr((FrescoObjectRef)_BaseObject__duplicate(_obj, &FrescoObjectStub::_create));
}
inline _TransformObjExpr::~_TransformObjExpr() { }
inline _TransformObjExpr TransformObjType::_ref() { return this; }

#endif
