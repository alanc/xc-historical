/* DO NOT EDIT -- Automatically generated from Interfaces/object.idl */

#ifndef Interfaces_object_h
#define Interfaces_object_h

#include <X11/Fresco/Ox/base.h>

class TypeObjType;
typedef TypeObjType* TypeObjRef;
typedef TypeObjRef TypeObj_in;
class TypeObj;
class TypeObj_tmp;
class TypeObj_var;

class TypeObj {
public:
    TypeObjRef _obj_;

    TypeObj() { _obj_ = 0; }
    TypeObj(TypeObjRef p) { _obj_ = p; }
    TypeObj& operator =(TypeObjRef p);
    TypeObj(const TypeObj&);
    TypeObj& operator =(const TypeObj& r);
    TypeObj(const TypeObj_tmp&);
    TypeObj& operator =(const TypeObj_tmp&);
    TypeObj(const TypeObj_var&);
    TypeObj& operator =(const TypeObj_var&);
    ~TypeObj();

    TypeObjRef operator ->() { return _obj_; }

    operator TypeObj_in() const { return _obj_; }
    static TypeObjRef _narrow(BaseObjectRef p);
    static TypeObj_tmp _narrow(const BaseObject& r);

    static TypeObjRef _duplicate(TypeObjRef obj);
    static TypeObj_tmp _duplicate(const TypeObj& r);
    enum ParamMode {
        param_in, param_out, param_inout
    };
    struct ParamInfo {
        string name;
        ParamMode mode;
        TypeObjRef type;
    };
    struct OpInfo {
        string name;
        Long index;
        TypeObjRef result;
        class _ParamInfoSeq {
        public:
            long _maximum, _length; ParamInfo* _buffer;

            _ParamInfoSeq() { _maximum = _length = 0; _buffer = 0; }
            _ParamInfoSeq(long m, long n, ParamInfo* e) {
                _maximum = m; _length = n; _buffer = e;
            }
            _ParamInfoSeq(const _ParamInfoSeq& _s) { _buffer = 0; *this = _s; }
            _ParamInfoSeq& operator =(const _ParamInfoSeq&);
            ~_ParamInfoSeq() { delete [] _buffer; }
        } params;
    };
    enum KindOf {
        void_type, boolean_type, char_type, octet_type, short_type, 
        unsigned_short_type, long_type, unsigned_long_type, longlong_type, 
        unsigned_longlong_type, float_type, double_type, string_type, 
        enum_type, array_type, struct_type, sequence_type, interface_type, 
        typedef_type
    };
};

class TypeObj_tmp : public TypeObj {
public:
    TypeObj_tmp(TypeObjRef p) { _obj_ = p; }
    TypeObj_tmp(const TypeObj& r);
    TypeObj_tmp(const TypeObj_tmp& r);
    ~TypeObj_tmp();
};

class TypeObj_var {
public:
    TypeObjRef _obj_;

    TypeObj_var(TypeObjRef p) { _obj_ = p; }
    operator TypeObjRef() const { return _obj_; }
    TypeObjRef operator ->() { return _obj_; }
};

class TypeObjType : public BaseObjectType {
protected:
    TypeObjType();
    virtual ~TypeObjType();
public:
    virtual string name() = 0;
    virtual Boolean op_info(TypeObj::OpInfo& op, Long index) = 0;
    virtual TypeObj::KindOf kind() = 0;
    virtual Long enum_info() = 0;
    void array_info(TypeObj& type, Long& size) {
        _c_array_info(type._obj_, size);
    }
    virtual void _c_array_info(TypeObjRef& type, Long& size) = 0;
    virtual Long members() = 0;
    TypeObj_tmp member_info(Long index) {
        return _c_member_info(index);
    }
    virtual TypeObjRef _c_member_info(Long index) = 0;
    void sequence_info(TypeObj& type, Long& size) {
        _c_sequence_info(type._obj_, size);
    }
    virtual void _c_sequence_info(TypeObjRef& type, Long& size) = 0;
    TypeObj_tmp typedef_info() {
        return _c_typedef_info();
    }
    virtual TypeObjRef _c_typedef_info() = 0;
    TypeObjRef _obj() { return this; }
    void* _this();
    virtual TypeObjId _tid();
};

class RequestObjType;
typedef RequestObjType* RequestObjRef;
typedef RequestObjRef RequestObj_in;
class RequestObj;
class RequestObj_tmp;
class RequestObj_var;

class RequestObj {
public:
    RequestObjRef _obj_;

    RequestObj() { _obj_ = 0; }
    RequestObj(RequestObjRef p) { _obj_ = p; }
    RequestObj& operator =(RequestObjRef p);
    RequestObj(const RequestObj&);
    RequestObj& operator =(const RequestObj& r);
    RequestObj(const RequestObj_tmp&);
    RequestObj& operator =(const RequestObj_tmp&);
    RequestObj(const RequestObj_var&);
    RequestObj& operator =(const RequestObj_var&);
    ~RequestObj();

    RequestObjRef operator ->() { return _obj_; }

    operator RequestObj_in() const { return _obj_; }
    static RequestObjRef _narrow(BaseObjectRef p);
    static RequestObj_tmp _narrow(const BaseObject& r);

    static RequestObjRef _duplicate(RequestObjRef obj);
    static RequestObj_tmp _duplicate(const RequestObj& r);
    enum CallStatus {
        initial, ok, unknown_operation, ambiguous_operation, bad_argument_count, 
        bad_argument_type, invoke_failed
    };
};

class RequestObj_tmp : public RequestObj {
public:
    RequestObj_tmp(RequestObjRef p) { _obj_ = p; }
    RequestObj_tmp(const RequestObj& r);
    RequestObj_tmp(const RequestObj_tmp& r);
    ~RequestObj_tmp();
};

class RequestObj_var {
public:
    RequestObjRef _obj_;

    RequestObj_var(RequestObjRef p) { _obj_ = p; }
    operator RequestObjRef() const { return _obj_; }
    RequestObjRef operator ->() { return _obj_; }
};

class RequestObjType : public BaseObjectType {
protected:
    RequestObjType();
    virtual ~RequestObjType();
public:
    virtual void set_operation(string s) = 0;
    virtual RequestObj::CallStatus invoke() = 0;
    virtual RequestObj::CallStatus op_info(TypeObj::OpInfo& op) = 0;
    virtual void put_char(Char value) = 0;
    virtual Char get_char() = 0;
    virtual void put_boolean(Boolean value) = 0;
    virtual Boolean get_boolean() = 0;
    virtual void put_octet(Octet value) = 0;
    virtual Octet get_octet() = 0;
    virtual void put_short(Short value) = 0;
    virtual Short get_short() = 0;
    virtual void put_unsigned_short(UShort value) = 0;
    virtual UShort get_unsigned_short() = 0;
    virtual void put_long(Long value) = 0;
    virtual Long get_long() = 0;
    virtual void put_unsigned_long(ULong value) = 0;
    virtual ULong get_unsigned_long() = 0;
    virtual void put_float(Float value) = 0;
    virtual Float get_float() = 0;
    virtual void put_double(Double value) = 0;
    virtual Double get_double() = 0;
    virtual void put_string(string value) = 0;
    virtual string get_string() = 0;
    virtual void put_object(BaseObject_in obj) = 0;
    BaseObject_tmp get_object();
    virtual BaseObjectRef _c_get_object() = 0;
    virtual void begin_aggregate() = 0;
    virtual void end_aggregate() = 0;
    RequestObjRef _obj() { return this; }
    void* _this();
    virtual TypeObjId _tid();
};

inline TypeObjRef TypeObj::_duplicate(TypeObjRef obj) {
    return (TypeObjRef)_BaseObject__duplicate(obj, 0);
}
inline TypeObj& TypeObj::operator =(TypeObjRef p) {
    _BaseObject__release(_obj_);
    _obj_ = TypeObj::_duplicate(p);
    return *this;
}
inline TypeObj::TypeObj(const TypeObj& r) {
    _obj_ = TypeObj::_duplicate(r._obj_);
}
inline TypeObj& TypeObj::operator =(const TypeObj& r) {
    _BaseObject__release(_obj_);
    _obj_ = TypeObj::_duplicate(r._obj_);
    return *this;
}
inline TypeObj::TypeObj(const TypeObj_tmp& r) {
    _obj_ = r._obj_;
    ((TypeObj_tmp*)&r)->_obj_ = 0;
}
inline TypeObj& TypeObj::operator =(const TypeObj_tmp& r) {
    _BaseObject__release(_obj_);
    _obj_ = r._obj_;
    ((TypeObj_tmp*)&r)->_obj_ = 0;
    return *this;
}
inline TypeObj::TypeObj(const TypeObj_var& e) {
    _obj_ = TypeObj::_duplicate(e._obj_);
}
inline TypeObj& TypeObj::operator =(const TypeObj_var& e) {
    _BaseObject__release(_obj_);
    _obj_ = TypeObj::_duplicate(e._obj_);
    return *this;
}
inline TypeObj::~TypeObj() {
    _BaseObject__release(_obj_);
}
inline TypeObj_tmp TypeObj::_narrow(const BaseObject& r) {
    return _narrow(r._obj_);
}
inline TypeObj_tmp TypeObj::_duplicate(const TypeObj& r) {
    return _duplicate(r._obj_);
}
inline TypeObj_tmp::TypeObj_tmp(const TypeObj& r) {
    _obj_ = TypeObj::_duplicate(r._obj_);
}
inline TypeObj_tmp::TypeObj_tmp(const TypeObj_tmp& r) {
    _obj_ = r._obj_;
    ((TypeObj_tmp*)&r)->_obj_ = 0;
}
inline TypeObj_tmp::~TypeObj_tmp() { }

inline RequestObjRef RequestObj::_duplicate(RequestObjRef obj) {
    return (RequestObjRef)_BaseObject__duplicate(obj, 0);
}
inline RequestObj& RequestObj::operator =(RequestObjRef p) {
    _BaseObject__release(_obj_);
    _obj_ = RequestObj::_duplicate(p);
    return *this;
}
inline RequestObj::RequestObj(const RequestObj& r) {
    _obj_ = RequestObj::_duplicate(r._obj_);
}
inline RequestObj& RequestObj::operator =(const RequestObj& r) {
    _BaseObject__release(_obj_);
    _obj_ = RequestObj::_duplicate(r._obj_);
    return *this;
}
inline RequestObj::RequestObj(const RequestObj_tmp& r) {
    _obj_ = r._obj_;
    ((RequestObj_tmp*)&r)->_obj_ = 0;
}
inline RequestObj& RequestObj::operator =(const RequestObj_tmp& r) {
    _BaseObject__release(_obj_);
    _obj_ = r._obj_;
    ((RequestObj_tmp*)&r)->_obj_ = 0;
    return *this;
}
inline RequestObj::RequestObj(const RequestObj_var& e) {
    _obj_ = RequestObj::_duplicate(e._obj_);
}
inline RequestObj& RequestObj::operator =(const RequestObj_var& e) {
    _BaseObject__release(_obj_);
    _obj_ = RequestObj::_duplicate(e._obj_);
    return *this;
}
inline RequestObj::~RequestObj() {
    _BaseObject__release(_obj_);
}
inline RequestObj_tmp RequestObj::_narrow(const BaseObject& r) {
    return _narrow(r._obj_);
}
inline RequestObj_tmp RequestObj::_duplicate(const RequestObj& r) {
    return _duplicate(r._obj_);
}
inline RequestObj_tmp::RequestObj_tmp(const RequestObj& r) {
    _obj_ = RequestObj::_duplicate(r._obj_);
}
inline RequestObj_tmp::RequestObj_tmp(const RequestObj_tmp& r) {
    _obj_ = r._obj_;
    ((RequestObj_tmp*)&r)->_obj_ = 0;
}
inline RequestObj_tmp::~RequestObj_tmp() { }

#endif
