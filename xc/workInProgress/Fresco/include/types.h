/* DO NOT EDIT -- Automatically generated from Interfaces/types.idl */

#ifndef Interfaces_types_h
#define Interfaces_types_h

#include <X11/Fresco/_enter.h>
#include <X11/Fresco/Ox/object.h>

typedef ULong Tag;

class FrescoObjectType;
typedef FrescoObjectType* FrescoObjectRef;
typedef FrescoObjectRef FrescoObject_in;
class FrescoObject;
class FrescoObject_tmp;
class FrescoObject_var;

class FrescoObject {
public:
    FrescoObjectRef _obj_;

    FrescoObject() { _obj_ = 0; }
    FrescoObject(FrescoObjectRef p) { _obj_ = p; }
    FrescoObject& operator =(FrescoObjectRef p);
    FrescoObject(const FrescoObject&);
    FrescoObject& operator =(const FrescoObject& r);
    FrescoObject(const FrescoObject_tmp&);
    FrescoObject& operator =(const FrescoObject_tmp&);
    FrescoObject(const FrescoObject_var&);
    FrescoObject& operator =(const FrescoObject_var&);
    ~FrescoObject();

    FrescoObjectRef operator ->() { return _obj_; }

    operator FrescoObject_in() const { return _obj_; }
    static FrescoObjectRef _narrow(BaseObjectRef p);
    static FrescoObject_tmp _narrow(const BaseObject& r);

    static FrescoObjectRef _duplicate(FrescoObjectRef obj);
    static FrescoObject_tmp _duplicate(const FrescoObject& r);
};

class FrescoObject_tmp : public FrescoObject {
public:
    FrescoObject_tmp(FrescoObjectRef p) { _obj_ = p; }
    FrescoObject_tmp(const FrescoObject& r);
    FrescoObject_tmp(const FrescoObject_tmp& r);
    ~FrescoObject_tmp();
};

class FrescoObject_var {
public:
    FrescoObjectRef _obj_;

    FrescoObject_var(FrescoObjectRef p) { _obj_ = p; }
    operator FrescoObjectRef() const { return _obj_; }
    FrescoObjectRef operator ->() { return _obj_; }
};

class FrescoObjectType : public BaseObjectType {
protected:
    FrescoObjectType();
    virtual ~FrescoObjectType();
public:
    virtual Long ref__(Long references);
    virtual Tag attach(FrescoObject_in observer);
    virtual void detach(Tag attach_tag);
    virtual void disconnect();
    virtual void notify_observers();
    virtual void update();
    FrescoObjectRef _obj() { return this; }
    void* _this();
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

typedef Long CharCode;

class CharStringType;
typedef CharStringType* CharStringRef;
typedef CharStringRef CharString_in;
class CharString;
class CharString_tmp;
class CharString_var;

class CharString {
public:
    CharStringRef _obj_;

    CharString() { _obj_ = 0; }
    CharString(CharStringRef p) { _obj_ = p; }
    CharString& operator =(CharStringRef p);
    CharString(const CharString&);
    CharString& operator =(const CharString& r);
    CharString(const CharString_tmp&);
    CharString& operator =(const CharString_tmp&);
    CharString(const CharString_var&);
    CharString& operator =(const CharString_var&);
    ~CharString();

    CharStringRef operator ->() { return _obj_; }

    operator CharString_in() const { return _obj_; }
    operator FrescoObject() const;
    static CharStringRef _narrow(BaseObjectRef p);
    static CharString_tmp _narrow(const BaseObject& r);

    static CharStringRef _duplicate(CharStringRef obj);
    static CharString_tmp _duplicate(const CharString& r);
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

class CharString_tmp : public CharString {
public:
    CharString_tmp(CharStringRef p) { _obj_ = p; }
    CharString_tmp(const CharString& r);
    CharString_tmp(const CharString_tmp& r);
    ~CharString_tmp();
};

class CharString_var {
public:
    CharStringRef _obj_;

    CharString_var(CharStringRef p) { _obj_ = p; }
    operator CharStringRef() const { return _obj_; }
    CharStringRef operator ->() { return _obj_; }
};

class CharStringType : public FrescoObjectType {
protected:
    CharStringType();
    virtual ~CharStringType();
public:
    virtual CharString::HashValue hash();
    virtual Long count();
    virtual void copy(CharString_in s);
    virtual Boolean equal(CharString_in s);
    virtual Boolean case_insensitive_equal(CharString_in s);
    virtual void get_data(CharString::Data& d);
    virtual void get_char_data(CharString::CharData& d);
    virtual void put_data(const CharString::Data& d);
    virtual void put_char_data(const CharString::CharData& d);
    CharStringRef _obj() { return this; }
    void* _this();
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
typedef ActionRef Action_in;
class Action;
class Action_tmp;
class Action_var;

class Action {
public:
    ActionRef _obj_;

    Action() { _obj_ = 0; }
    Action(ActionRef p) { _obj_ = p; }
    Action& operator =(ActionRef p);
    Action(const Action&);
    Action& operator =(const Action& r);
    Action(const Action_tmp&);
    Action& operator =(const Action_tmp&);
    Action(const Action_var&);
    Action& operator =(const Action_var&);
    ~Action();

    ActionRef operator ->() { return _obj_; }

    operator Action_in() const { return _obj_; }
    operator FrescoObject() const;
    static ActionRef _narrow(BaseObjectRef p);
    static Action_tmp _narrow(const BaseObject& r);

    static ActionRef _duplicate(ActionRef obj);
    static Action_tmp _duplicate(const Action& r);
};

class Action_tmp : public Action {
public:
    Action_tmp(ActionRef p) { _obj_ = p; }
    Action_tmp(const Action& r);
    Action_tmp(const Action_tmp& r);
    ~Action_tmp();
};

class Action_var {
public:
    ActionRef _obj_;

    Action_var(ActionRef p) { _obj_ = p; }
    operator ActionRef() const { return _obj_; }
    ActionRef operator ->() { return _obj_; }
};

class ActionType : public FrescoObjectType {
protected:
    ActionType();
    virtual ~ActionType();
public:
    virtual void execute();
    virtual Boolean reversible();
    virtual void unexecute();
    ActionRef _obj() { return this; }
    void* _this();
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

class TransformObjType;
typedef TransformObjType* TransformObjRef;
typedef TransformObjRef TransformObj_in;
class TransformObj;
class TransformObj_tmp;
class TransformObj_var;

class RegionType;
typedef RegionType* RegionRef;
typedef RegionRef Region_in;
class Region;
class Region_tmp;
class Region_var;

class Region {
public:
    RegionRef _obj_;

    Region() { _obj_ = 0; }
    Region(RegionRef p) { _obj_ = p; }
    Region& operator =(RegionRef p);
    Region(const Region&);
    Region& operator =(const Region& r);
    Region(const Region_tmp&);
    Region& operator =(const Region_tmp&);
    Region(const Region_var&);
    Region& operator =(const Region_var&);
    ~Region();

    RegionRef operator ->() { return _obj_; }

    operator Region_in() const { return _obj_; }
    operator FrescoObject() const;
    static RegionRef _narrow(BaseObjectRef p);
    static Region_tmp _narrow(const BaseObject& r);

    static RegionRef _duplicate(RegionRef obj);
    static Region_tmp _duplicate(const Region& r);
    struct BoundingSpan {
        Coord begin, end;
        Coord origin, length;
        Alignment align;
    };
};

class Region_tmp : public Region {
public:
    Region_tmp(RegionRef p) { _obj_ = p; }
    Region_tmp(const Region& r);
    Region_tmp(const Region_tmp& r);
    ~Region_tmp();
};

class Region_var {
public:
    RegionRef _obj_;

    Region_var(RegionRef p) { _obj_ = p; }
    operator RegionRef() const { return _obj_; }
    RegionRef operator ->() { return _obj_; }
};

class RegionType : public FrescoObjectType {
protected:
    RegionType();
    virtual ~RegionType();
public:
    virtual Boolean contains(const Vertex& v);
    virtual Boolean contains_plane(const Vertex& v, Axis a);
    virtual Boolean intersects(Region_in r);
    virtual void copy(Region_in r);
    virtual void merge_intersect(Region_in r);
    virtual void merge_union(Region_in r);
    virtual void subtract(Region_in r);
    virtual void transform(TransformObj_in t);
    virtual void bounds(Vertex& lower, Vertex& upper);
    virtual void origin(Vertex& v);
    virtual void span(Axis a, Region::BoundingSpan& s);
    RegionRef _obj() { return this; }
    void* _this();
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

class StyleValueType;
typedef StyleValueType* StyleValueRef;
typedef StyleValueRef StyleValue_in;
class StyleValue;
class StyleValue_tmp;
class StyleValue_var;

class StyleValue {
public:
    StyleValueRef _obj_;

    StyleValue() { _obj_ = 0; }
    StyleValue(StyleValueRef p) { _obj_ = p; }
    StyleValue& operator =(StyleValueRef p);
    StyleValue(const StyleValue&);
    StyleValue& operator =(const StyleValue& r);
    StyleValue(const StyleValue_tmp&);
    StyleValue& operator =(const StyleValue_tmp&);
    StyleValue(const StyleValue_var&);
    StyleValue& operator =(const StyleValue_var&);
    ~StyleValue();

    StyleValueRef operator ->() { return _obj_; }

    operator StyleValue_in() const { return _obj_; }
    operator FrescoObject() const;
    static StyleValueRef _narrow(BaseObjectRef p);
    static StyleValue_tmp _narrow(const BaseObject& r);

    static StyleValueRef _duplicate(StyleValueRef obj);
    static StyleValue_tmp _duplicate(const StyleValue& r);
};

class StyleValue_tmp : public StyleValue {
public:
    StyleValue_tmp(StyleValueRef p) { _obj_ = p; }
    StyleValue_tmp(const StyleValue& r);
    StyleValue_tmp(const StyleValue_tmp& r);
    ~StyleValue_tmp();
};

class StyleValue_var {
public:
    StyleValueRef _obj_;

    StyleValue_var(StyleValueRef p) { _obj_ = p; }
    operator StyleValueRef() const { return _obj_; }
    StyleValueRef operator ->() { return _obj_; }
};

class StyleValueType : public FrescoObjectType {
protected:
    StyleValueType();
    virtual ~StyleValueType();
public:
    CharString_tmp name() {
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
        return _c_read_string(s._obj_);
    }
    virtual Boolean _c_read_string(CharStringRef& s);
    virtual void write_string(CharString_in s);
    Boolean read_value(FrescoObject& s) {
        return _c_read_value(s._obj_);
    }
    virtual Boolean _c_read_value(FrescoObjectRef& s);
    virtual void write_value(FrescoObject_in s);
    virtual void lock();
    virtual void unlock();
    StyleValueRef _obj() { return this; }
    void* _this();
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

class StyleVisitorType;
typedef StyleVisitorType* StyleVisitorRef;
typedef StyleVisitorRef StyleVisitor_in;
class StyleVisitor;
class StyleVisitor_tmp;
class StyleVisitor_var;

class StyleObjType;
typedef StyleObjType* StyleObjRef;
typedef StyleObjRef StyleObj_in;
class StyleObj;
class StyleObj_tmp;
class StyleObj_var;

class StyleObj {
public:
    StyleObjRef _obj_;

    StyleObj() { _obj_ = 0; }
    StyleObj(StyleObjRef p) { _obj_ = p; }
    StyleObj& operator =(StyleObjRef p);
    StyleObj(const StyleObj&);
    StyleObj& operator =(const StyleObj& r);
    StyleObj(const StyleObj_tmp&);
    StyleObj& operator =(const StyleObj_tmp&);
    StyleObj(const StyleObj_var&);
    StyleObj& operator =(const StyleObj_var&);
    ~StyleObj();

    StyleObjRef operator ->() { return _obj_; }

    operator StyleObj_in() const { return _obj_; }
    operator FrescoObject() const;
    static StyleObjRef _narrow(BaseObjectRef p);
    static StyleObj_tmp _narrow(const BaseObject& r);

    static StyleObjRef _duplicate(StyleObjRef obj);
    static StyleObj_tmp _duplicate(const StyleObj& r);
};

class StyleObj_tmp : public StyleObj {
public:
    StyleObj_tmp(StyleObjRef p) { _obj_ = p; }
    StyleObj_tmp(const StyleObj& r);
    StyleObj_tmp(const StyleObj_tmp& r);
    ~StyleObj_tmp();
};

class StyleObj_var {
public:
    StyleObjRef _obj_;

    StyleObj_var(StyleObjRef p) { _obj_ = p; }
    operator StyleObjRef() const { return _obj_; }
    StyleObjRef operator ->() { return _obj_; }
};

class StyleObjType : public FrescoObjectType {
protected:
    StyleObjType();
    virtual ~StyleObjType();
public:
    StyleObj_tmp new_style() {
        return _c_new_style();
    }
    virtual StyleObjRef _c_new_style();
    StyleObj_tmp parent_style() {
        return _c_parent_style();
    }
    virtual StyleObjRef _c_parent_style();
    virtual void link_parent(StyleObj_in parent);
    virtual void unlink_parent();
    virtual Tag link_child(StyleObj_in child);
    virtual void unlink_child(Tag link_tag);
    virtual void merge(StyleObj_in s);
    CharString_tmp name() {
        return _c_name();
    }
    virtual CharStringRef _c_name();
    void name(CharString_in _p) {
        _c_name(_p);
    }
    virtual void _c_name(CharString_in _p);
    virtual void alias(CharString_in s);
    virtual Boolean is_on(CharString_in name);
    StyleValue_tmp bind(CharString_in name) {
        return _c_bind(name);
    }
    virtual StyleValueRef _c_bind(CharString_in name);
    virtual void unbind(CharString_in name);
    StyleValue_tmp resolve(CharString_in name) {
        return _c_resolve(name);
    }
    virtual StyleValueRef _c_resolve(CharString_in name);
    StyleValue_tmp resolve_wildcard(CharString_in name, StyleObj_in start) {
        return _c_resolve_wildcard(name, start);
    }
    virtual StyleValueRef _c_resolve_wildcard(CharString_in name, StyleObj_in start);
    virtual Long match(CharString_in name);
    virtual void visit_aliases(StyleVisitor_in v);
    virtual void visit_attributes(StyleVisitor_in v);
    virtual void visit_styles(StyleVisitor_in v);
    virtual void lock();
    virtual void unlock();
    StyleObjRef _obj() { return this; }
    void* _this();
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

class StyleVisitor {
public:
    StyleVisitorRef _obj_;

    StyleVisitor() { _obj_ = 0; }
    StyleVisitor(StyleVisitorRef p) { _obj_ = p; }
    StyleVisitor& operator =(StyleVisitorRef p);
    StyleVisitor(const StyleVisitor&);
    StyleVisitor& operator =(const StyleVisitor& r);
    StyleVisitor(const StyleVisitor_tmp&);
    StyleVisitor& operator =(const StyleVisitor_tmp&);
    StyleVisitor(const StyleVisitor_var&);
    StyleVisitor& operator =(const StyleVisitor_var&);
    ~StyleVisitor();

    StyleVisitorRef operator ->() { return _obj_; }

    operator StyleVisitor_in() const { return _obj_; }
    operator FrescoObject() const;
    static StyleVisitorRef _narrow(BaseObjectRef p);
    static StyleVisitor_tmp _narrow(const BaseObject& r);

    static StyleVisitorRef _duplicate(StyleVisitorRef obj);
    static StyleVisitor_tmp _duplicate(const StyleVisitor& r);
};

class StyleVisitor_tmp : public StyleVisitor {
public:
    StyleVisitor_tmp(StyleVisitorRef p) { _obj_ = p; }
    StyleVisitor_tmp(const StyleVisitor& r);
    StyleVisitor_tmp(const StyleVisitor_tmp& r);
    ~StyleVisitor_tmp();
};

class StyleVisitor_var {
public:
    StyleVisitorRef _obj_;

    StyleVisitor_var(StyleVisitorRef p) { _obj_ = p; }
    operator StyleVisitorRef() const { return _obj_; }
    StyleVisitorRef operator ->() { return _obj_; }
};

class StyleVisitorType : public FrescoObjectType {
protected:
    StyleVisitorType();
    virtual ~StyleVisitorType();
public:
    virtual Boolean visit_alias(CharString_in name);
    virtual Boolean visit_attribute(StyleValue_in a);
    virtual Boolean visit_style(StyleObj_in s);
    StyleVisitorRef _obj() { return this; }
    void* _this();
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

class TransformObj {
public:
    TransformObjRef _obj_;

    TransformObj() { _obj_ = 0; }
    TransformObj(TransformObjRef p) { _obj_ = p; }
    TransformObj& operator =(TransformObjRef p);
    TransformObj(const TransformObj&);
    TransformObj& operator =(const TransformObj& r);
    TransformObj(const TransformObj_tmp&);
    TransformObj& operator =(const TransformObj_tmp&);
    TransformObj(const TransformObj_var&);
    TransformObj& operator =(const TransformObj_var&);
    ~TransformObj();

    TransformObjRef operator ->() { return _obj_; }

    operator TransformObj_in() const { return _obj_; }
    operator FrescoObject() const;
    static TransformObjRef _narrow(BaseObjectRef p);
    static TransformObj_tmp _narrow(const BaseObject& r);

    static TransformObjRef _duplicate(TransformObjRef obj);
    static TransformObj_tmp _duplicate(const TransformObj& r);
    typedef Coord Matrix[4][4];
};

class TransformObj_tmp : public TransformObj {
public:
    TransformObj_tmp(TransformObjRef p) { _obj_ = p; }
    TransformObj_tmp(const TransformObj& r);
    TransformObj_tmp(const TransformObj_tmp& r);
    ~TransformObj_tmp();
};

class TransformObj_var {
public:
    TransformObjRef _obj_;

    TransformObj_var(TransformObjRef p) { _obj_ = p; }
    operator TransformObjRef() const { return _obj_; }
    TransformObjRef operator ->() { return _obj_; }
};

class TransformObjType : public FrescoObjectType {
protected:
    TransformObjType();
    virtual ~TransformObjType();
public:
    virtual void load(TransformObj_in t);
    virtual void load_identity();
    virtual void load_matrix(TransformObj::Matrix m);
    virtual void store_matrix(TransformObj::Matrix m);
    virtual Boolean equal(TransformObj_in t);
    virtual Boolean identity();
    virtual Boolean det_is_zero();
    virtual void scale(const Vertex& v);
    virtual void rotate(Float angle, Axis a);
    virtual void translate(const Vertex& v);
    virtual void premultiply(TransformObj_in t);
    virtual void postmultiply(TransformObj_in t);
    virtual void invert();
    virtual void transform(Vertex& v);
    virtual void inverse_transform(Vertex& v);
    TransformObjRef _obj() { return this; }
    void* _this();
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

inline FrescoObjectRef FrescoObject::_duplicate(FrescoObjectRef obj) {
    return (FrescoObjectRef)_BaseObject__duplicate(obj, &FrescoObjectStub::_create);
}
inline FrescoObject& FrescoObject::operator =(FrescoObjectRef p) {
    _BaseObject__release(_obj_);
    _obj_ = FrescoObject::_duplicate(p);
    return *this;
}
inline FrescoObject::FrescoObject(const FrescoObject& r) {
    _obj_ = FrescoObject::_duplicate(r._obj_);
}
inline FrescoObject& FrescoObject::operator =(const FrescoObject& r) {
    _BaseObject__release(_obj_);
    _obj_ = FrescoObject::_duplicate(r._obj_);
    return *this;
}
inline FrescoObject::FrescoObject(const FrescoObject_tmp& r) {
    _obj_ = r._obj_;
    ((FrescoObject_tmp*)&r)->_obj_ = 0;
}
inline FrescoObject& FrescoObject::operator =(const FrescoObject_tmp& r) {
    _BaseObject__release(_obj_);
    _obj_ = r._obj_;
    ((FrescoObject_tmp*)&r)->_obj_ = 0;
    return *this;
}
inline FrescoObject::FrescoObject(const FrescoObject_var& e) {
    _obj_ = FrescoObject::_duplicate(e._obj_);
}
inline FrescoObject& FrescoObject::operator =(const FrescoObject_var& e) {
    _BaseObject__release(_obj_);
    _obj_ = FrescoObject::_duplicate(e._obj_);
    return *this;
}
inline FrescoObject::~FrescoObject() {
    _BaseObject__release(_obj_);
}
inline FrescoObject_tmp FrescoObject::_narrow(const BaseObject& r) {
    return _narrow(r._obj_);
}
inline FrescoObject_tmp FrescoObject::_duplicate(const FrescoObject& r) {
    return _duplicate(r._obj_);
}
inline FrescoObject_tmp::FrescoObject_tmp(const FrescoObject& r) {
    _obj_ = FrescoObject::_duplicate(r._obj_);
}
inline FrescoObject_tmp::FrescoObject_tmp(const FrescoObject_tmp& r) {
    _obj_ = r._obj_;
    ((FrescoObject_tmp*)&r)->_obj_ = 0;
}
inline FrescoObject_tmp::~FrescoObject_tmp() { }

inline CharStringRef CharString::_duplicate(CharStringRef obj) {
    return (CharStringRef)_BaseObject__duplicate(obj, &CharStringStub::_create);
}
inline CharString& CharString::operator =(CharStringRef p) {
    _BaseObject__release(_obj_);
    _obj_ = CharString::_duplicate(p);
    return *this;
}
inline CharString::CharString(const CharString& r) {
    _obj_ = CharString::_duplicate(r._obj_);
}
inline CharString& CharString::operator =(const CharString& r) {
    _BaseObject__release(_obj_);
    _obj_ = CharString::_duplicate(r._obj_);
    return *this;
}
inline CharString::CharString(const CharString_tmp& r) {
    _obj_ = r._obj_;
    ((CharString_tmp*)&r)->_obj_ = 0;
}
inline CharString& CharString::operator =(const CharString_tmp& r) {
    _BaseObject__release(_obj_);
    _obj_ = r._obj_;
    ((CharString_tmp*)&r)->_obj_ = 0;
    return *this;
}
inline CharString::CharString(const CharString_var& e) {
    _obj_ = CharString::_duplicate(e._obj_);
}
inline CharString& CharString::operator =(const CharString_var& e) {
    _BaseObject__release(_obj_);
    _obj_ = CharString::_duplicate(e._obj_);
    return *this;
}
inline CharString::~CharString() {
    _BaseObject__release(_obj_);
}
inline CharString_tmp CharString::_narrow(const BaseObject& r) {
    return _narrow(r._obj_);
}
inline CharString_tmp CharString::_duplicate(const CharString& r) {
    return _duplicate(r._obj_);
}
inline CharString::operator FrescoObject() const {
    return FrescoObject_tmp((FrescoObjectRef)_BaseObject__duplicate((FrescoObjectRef)_obj_, &FrescoObjectStub::_create));
}
inline CharString_tmp::CharString_tmp(const CharString& r) {
    _obj_ = CharString::_duplicate(r._obj_);
}
inline CharString_tmp::CharString_tmp(const CharString_tmp& r) {
    _obj_ = r._obj_;
    ((CharString_tmp*)&r)->_obj_ = 0;
}
inline CharString_tmp::~CharString_tmp() { }

inline ActionRef Action::_duplicate(ActionRef obj) {
    return (ActionRef)_BaseObject__duplicate(obj, &ActionStub::_create);
}
inline Action& Action::operator =(ActionRef p) {
    _BaseObject__release(_obj_);
    _obj_ = Action::_duplicate(p);
    return *this;
}
inline Action::Action(const Action& r) {
    _obj_ = Action::_duplicate(r._obj_);
}
inline Action& Action::operator =(const Action& r) {
    _BaseObject__release(_obj_);
    _obj_ = Action::_duplicate(r._obj_);
    return *this;
}
inline Action::Action(const Action_tmp& r) {
    _obj_ = r._obj_;
    ((Action_tmp*)&r)->_obj_ = 0;
}
inline Action& Action::operator =(const Action_tmp& r) {
    _BaseObject__release(_obj_);
    _obj_ = r._obj_;
    ((Action_tmp*)&r)->_obj_ = 0;
    return *this;
}
inline Action::Action(const Action_var& e) {
    _obj_ = Action::_duplicate(e._obj_);
}
inline Action& Action::operator =(const Action_var& e) {
    _BaseObject__release(_obj_);
    _obj_ = Action::_duplicate(e._obj_);
    return *this;
}
inline Action::~Action() {
    _BaseObject__release(_obj_);
}
inline Action_tmp Action::_narrow(const BaseObject& r) {
    return _narrow(r._obj_);
}
inline Action_tmp Action::_duplicate(const Action& r) {
    return _duplicate(r._obj_);
}
inline Action::operator FrescoObject() const {
    return FrescoObject_tmp((FrescoObjectRef)_BaseObject__duplicate((FrescoObjectRef)_obj_, &FrescoObjectStub::_create));
}
inline Action_tmp::Action_tmp(const Action& r) {
    _obj_ = Action::_duplicate(r._obj_);
}
inline Action_tmp::Action_tmp(const Action_tmp& r) {
    _obj_ = r._obj_;
    ((Action_tmp*)&r)->_obj_ = 0;
}
inline Action_tmp::~Action_tmp() { }

inline RegionRef Region::_duplicate(RegionRef obj) {
    return (RegionRef)_BaseObject__duplicate(obj, &RegionStub::_create);
}
inline Region& Region::operator =(RegionRef p) {
    _BaseObject__release(_obj_);
    _obj_ = Region::_duplicate(p);
    return *this;
}
inline Region::Region(const Region& r) {
    _obj_ = Region::_duplicate(r._obj_);
}
inline Region& Region::operator =(const Region& r) {
    _BaseObject__release(_obj_);
    _obj_ = Region::_duplicate(r._obj_);
    return *this;
}
inline Region::Region(const Region_tmp& r) {
    _obj_ = r._obj_;
    ((Region_tmp*)&r)->_obj_ = 0;
}
inline Region& Region::operator =(const Region_tmp& r) {
    _BaseObject__release(_obj_);
    _obj_ = r._obj_;
    ((Region_tmp*)&r)->_obj_ = 0;
    return *this;
}
inline Region::Region(const Region_var& e) {
    _obj_ = Region::_duplicate(e._obj_);
}
inline Region& Region::operator =(const Region_var& e) {
    _BaseObject__release(_obj_);
    _obj_ = Region::_duplicate(e._obj_);
    return *this;
}
inline Region::~Region() {
    _BaseObject__release(_obj_);
}
inline Region_tmp Region::_narrow(const BaseObject& r) {
    return _narrow(r._obj_);
}
inline Region_tmp Region::_duplicate(const Region& r) {
    return _duplicate(r._obj_);
}
inline Region::operator FrescoObject() const {
    return FrescoObject_tmp((FrescoObjectRef)_BaseObject__duplicate((FrescoObjectRef)_obj_, &FrescoObjectStub::_create));
}
inline Region_tmp::Region_tmp(const Region& r) {
    _obj_ = Region::_duplicate(r._obj_);
}
inline Region_tmp::Region_tmp(const Region_tmp& r) {
    _obj_ = r._obj_;
    ((Region_tmp*)&r)->_obj_ = 0;
}
inline Region_tmp::~Region_tmp() { }

inline StyleValueRef StyleValue::_duplicate(StyleValueRef obj) {
    return (StyleValueRef)_BaseObject__duplicate(obj, &StyleValueStub::_create);
}
inline StyleValue& StyleValue::operator =(StyleValueRef p) {
    _BaseObject__release(_obj_);
    _obj_ = StyleValue::_duplicate(p);
    return *this;
}
inline StyleValue::StyleValue(const StyleValue& r) {
    _obj_ = StyleValue::_duplicate(r._obj_);
}
inline StyleValue& StyleValue::operator =(const StyleValue& r) {
    _BaseObject__release(_obj_);
    _obj_ = StyleValue::_duplicate(r._obj_);
    return *this;
}
inline StyleValue::StyleValue(const StyleValue_tmp& r) {
    _obj_ = r._obj_;
    ((StyleValue_tmp*)&r)->_obj_ = 0;
}
inline StyleValue& StyleValue::operator =(const StyleValue_tmp& r) {
    _BaseObject__release(_obj_);
    _obj_ = r._obj_;
    ((StyleValue_tmp*)&r)->_obj_ = 0;
    return *this;
}
inline StyleValue::StyleValue(const StyleValue_var& e) {
    _obj_ = StyleValue::_duplicate(e._obj_);
}
inline StyleValue& StyleValue::operator =(const StyleValue_var& e) {
    _BaseObject__release(_obj_);
    _obj_ = StyleValue::_duplicate(e._obj_);
    return *this;
}
inline StyleValue::~StyleValue() {
    _BaseObject__release(_obj_);
}
inline StyleValue_tmp StyleValue::_narrow(const BaseObject& r) {
    return _narrow(r._obj_);
}
inline StyleValue_tmp StyleValue::_duplicate(const StyleValue& r) {
    return _duplicate(r._obj_);
}
inline StyleValue::operator FrescoObject() const {
    return FrescoObject_tmp((FrescoObjectRef)_BaseObject__duplicate((FrescoObjectRef)_obj_, &FrescoObjectStub::_create));
}
inline StyleValue_tmp::StyleValue_tmp(const StyleValue& r) {
    _obj_ = StyleValue::_duplicate(r._obj_);
}
inline StyleValue_tmp::StyleValue_tmp(const StyleValue_tmp& r) {
    _obj_ = r._obj_;
    ((StyleValue_tmp*)&r)->_obj_ = 0;
}
inline StyleValue_tmp::~StyleValue_tmp() { }

inline StyleObjRef StyleObj::_duplicate(StyleObjRef obj) {
    return (StyleObjRef)_BaseObject__duplicate(obj, &StyleObjStub::_create);
}
inline StyleObj& StyleObj::operator =(StyleObjRef p) {
    _BaseObject__release(_obj_);
    _obj_ = StyleObj::_duplicate(p);
    return *this;
}
inline StyleObj::StyleObj(const StyleObj& r) {
    _obj_ = StyleObj::_duplicate(r._obj_);
}
inline StyleObj& StyleObj::operator =(const StyleObj& r) {
    _BaseObject__release(_obj_);
    _obj_ = StyleObj::_duplicate(r._obj_);
    return *this;
}
inline StyleObj::StyleObj(const StyleObj_tmp& r) {
    _obj_ = r._obj_;
    ((StyleObj_tmp*)&r)->_obj_ = 0;
}
inline StyleObj& StyleObj::operator =(const StyleObj_tmp& r) {
    _BaseObject__release(_obj_);
    _obj_ = r._obj_;
    ((StyleObj_tmp*)&r)->_obj_ = 0;
    return *this;
}
inline StyleObj::StyleObj(const StyleObj_var& e) {
    _obj_ = StyleObj::_duplicate(e._obj_);
}
inline StyleObj& StyleObj::operator =(const StyleObj_var& e) {
    _BaseObject__release(_obj_);
    _obj_ = StyleObj::_duplicate(e._obj_);
    return *this;
}
inline StyleObj::~StyleObj() {
    _BaseObject__release(_obj_);
}
inline StyleObj_tmp StyleObj::_narrow(const BaseObject& r) {
    return _narrow(r._obj_);
}
inline StyleObj_tmp StyleObj::_duplicate(const StyleObj& r) {
    return _duplicate(r._obj_);
}
inline StyleObj::operator FrescoObject() const {
    return FrescoObject_tmp((FrescoObjectRef)_BaseObject__duplicate((FrescoObjectRef)_obj_, &FrescoObjectStub::_create));
}
inline StyleObj_tmp::StyleObj_tmp(const StyleObj& r) {
    _obj_ = StyleObj::_duplicate(r._obj_);
}
inline StyleObj_tmp::StyleObj_tmp(const StyleObj_tmp& r) {
    _obj_ = r._obj_;
    ((StyleObj_tmp*)&r)->_obj_ = 0;
}
inline StyleObj_tmp::~StyleObj_tmp() { }

inline StyleVisitorRef StyleVisitor::_duplicate(StyleVisitorRef obj) {
    return (StyleVisitorRef)_BaseObject__duplicate(obj, &StyleVisitorStub::_create);
}
inline StyleVisitor& StyleVisitor::operator =(StyleVisitorRef p) {
    _BaseObject__release(_obj_);
    _obj_ = StyleVisitor::_duplicate(p);
    return *this;
}
inline StyleVisitor::StyleVisitor(const StyleVisitor& r) {
    _obj_ = StyleVisitor::_duplicate(r._obj_);
}
inline StyleVisitor& StyleVisitor::operator =(const StyleVisitor& r) {
    _BaseObject__release(_obj_);
    _obj_ = StyleVisitor::_duplicate(r._obj_);
    return *this;
}
inline StyleVisitor::StyleVisitor(const StyleVisitor_tmp& r) {
    _obj_ = r._obj_;
    ((StyleVisitor_tmp*)&r)->_obj_ = 0;
}
inline StyleVisitor& StyleVisitor::operator =(const StyleVisitor_tmp& r) {
    _BaseObject__release(_obj_);
    _obj_ = r._obj_;
    ((StyleVisitor_tmp*)&r)->_obj_ = 0;
    return *this;
}
inline StyleVisitor::StyleVisitor(const StyleVisitor_var& e) {
    _obj_ = StyleVisitor::_duplicate(e._obj_);
}
inline StyleVisitor& StyleVisitor::operator =(const StyleVisitor_var& e) {
    _BaseObject__release(_obj_);
    _obj_ = StyleVisitor::_duplicate(e._obj_);
    return *this;
}
inline StyleVisitor::~StyleVisitor() {
    _BaseObject__release(_obj_);
}
inline StyleVisitor_tmp StyleVisitor::_narrow(const BaseObject& r) {
    return _narrow(r._obj_);
}
inline StyleVisitor_tmp StyleVisitor::_duplicate(const StyleVisitor& r) {
    return _duplicate(r._obj_);
}
inline StyleVisitor::operator FrescoObject() const {
    return FrescoObject_tmp((FrescoObjectRef)_BaseObject__duplicate((FrescoObjectRef)_obj_, &FrescoObjectStub::_create));
}
inline StyleVisitor_tmp::StyleVisitor_tmp(const StyleVisitor& r) {
    _obj_ = StyleVisitor::_duplicate(r._obj_);
}
inline StyleVisitor_tmp::StyleVisitor_tmp(const StyleVisitor_tmp& r) {
    _obj_ = r._obj_;
    ((StyleVisitor_tmp*)&r)->_obj_ = 0;
}
inline StyleVisitor_tmp::~StyleVisitor_tmp() { }

inline TransformObjRef TransformObj::_duplicate(TransformObjRef obj) {
    return (TransformObjRef)_BaseObject__duplicate(obj, &TransformObjStub::_create);
}
inline TransformObj& TransformObj::operator =(TransformObjRef p) {
    _BaseObject__release(_obj_);
    _obj_ = TransformObj::_duplicate(p);
    return *this;
}
inline TransformObj::TransformObj(const TransformObj& r) {
    _obj_ = TransformObj::_duplicate(r._obj_);
}
inline TransformObj& TransformObj::operator =(const TransformObj& r) {
    _BaseObject__release(_obj_);
    _obj_ = TransformObj::_duplicate(r._obj_);
    return *this;
}
inline TransformObj::TransformObj(const TransformObj_tmp& r) {
    _obj_ = r._obj_;
    ((TransformObj_tmp*)&r)->_obj_ = 0;
}
inline TransformObj& TransformObj::operator =(const TransformObj_tmp& r) {
    _BaseObject__release(_obj_);
    _obj_ = r._obj_;
    ((TransformObj_tmp*)&r)->_obj_ = 0;
    return *this;
}
inline TransformObj::TransformObj(const TransformObj_var& e) {
    _obj_ = TransformObj::_duplicate(e._obj_);
}
inline TransformObj& TransformObj::operator =(const TransformObj_var& e) {
    _BaseObject__release(_obj_);
    _obj_ = TransformObj::_duplicate(e._obj_);
    return *this;
}
inline TransformObj::~TransformObj() {
    _BaseObject__release(_obj_);
}
inline TransformObj_tmp TransformObj::_narrow(const BaseObject& r) {
    return _narrow(r._obj_);
}
inline TransformObj_tmp TransformObj::_duplicate(const TransformObj& r) {
    return _duplicate(r._obj_);
}
inline TransformObj::operator FrescoObject() const {
    return FrescoObject_tmp((FrescoObjectRef)_BaseObject__duplicate((FrescoObjectRef)_obj_, &FrescoObjectStub::_create));
}
inline TransformObj_tmp::TransformObj_tmp(const TransformObj& r) {
    _obj_ = TransformObj::_duplicate(r._obj_);
}
inline TransformObj_tmp::TransformObj_tmp(const TransformObj_tmp& r) {
    _obj_ = r._obj_;
    ((TransformObj_tmp*)&r)->_obj_ = 0;
}
inline TransformObj_tmp::~TransformObj_tmp() { }

#endif
