/* DO NOT EDIT -- Automatically generated from Interfaces/object.idl */

#ifndef Interfaces_object_h
#define Interfaces_object_h

#include <X11/Fresco/Ox/base.h>

class TypeObjType;
typedef TypeObjType* TypeObjRef;
class TypeObj;
class _TypeObjExpr;
class _TypeObjElem;

class TypeObj {
public:
    TypeObjRef _obj;

    TypeObj() { _obj = 0; }
    TypeObj(TypeObjRef p) { _obj = p; }
    TypeObj& operator =(TypeObjRef p);
    TypeObj(const TypeObj&);
    TypeObj& operator =(const TypeObj& r);
    TypeObj(const _TypeObjExpr&);
    TypeObj& operator =(const _TypeObjExpr&);
    TypeObj(const _TypeObjElem&);
    TypeObj& operator =(const _TypeObjElem&);
    ~TypeObj();

    operator TypeObjRef() const { return _obj; }
    TypeObjRef operator ->() { return _obj; }

    static TypeObjRef _narrow(BaseObjectRef p);
    static _TypeObjExpr _narrow(const BaseObject& r);

    static TypeObjRef _duplicate(TypeObjRef obj);
    static _TypeObjExpr _duplicate(const TypeObj& r);
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
        void_type, boolean_type, char_type, short_type, unsigned_short_type, 
        long_type, unsigned_long_type, float_type, double_type, string_type, 
        enum_type, array_type, struct_type, sequence_type, interface_type, 
        typedef_type
    };
};

class _TypeObjExpr : public TypeObj {
public:
    _TypeObjExpr(TypeObjRef p) { _obj = p; }
    _TypeObjExpr(const TypeObj& r) { _obj = r._obj; }
    _TypeObjExpr(const _TypeObjExpr& r) { _obj = r._obj; }
    ~_TypeObjExpr();
};

class _TypeObjElem {
public:
    TypeObjRef _obj;

    _TypeObjElem(TypeObjRef p) { _obj = p; }
    operator TypeObjRef() const { return _obj; }
    TypeObjRef operator ->() { return _obj; }
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
        _c_array_info(type._obj, size);
    }
    virtual void _c_array_info(TypeObjRef& type, Long& size) = 0;
    virtual Long members() = 0;
    _TypeObjExpr member_info(Long index) {
        return _c_member_info(index);
    }
    virtual TypeObjRef _c_member_info(Long index) = 0;
    void sequence_info(TypeObj& type, Long& size) {
        _c_sequence_info(type._obj, size);
    }
    virtual void _c_sequence_info(TypeObjRef& type, Long& size) = 0;
    _TypeObjExpr typedef_info() {
        return _c_typedef_info();
    }
    virtual TypeObjRef _c_typedef_info() = 0;

    _TypeObjExpr _ref();
    virtual TypeObjId _tid();
};

inline TypeObjRef TypeObj::_duplicate(TypeObjRef obj) {
    return (TypeObjRef)_BaseObject__duplicate(obj, 0);
}
inline TypeObj& TypeObj::operator =(TypeObjRef p) {
    _BaseObject__release(_obj);
    _obj = TypeObj::_duplicate(p);
    return *this;
}
inline TypeObj::TypeObj(const TypeObj& r) {
    _obj = TypeObj::_duplicate(r._obj);
}
inline TypeObj& TypeObj::operator =(const TypeObj& r) {
    _BaseObject__release(_obj);
    _obj = TypeObj::_duplicate(r._obj);
    return *this;
}
inline TypeObj::TypeObj(const _TypeObjExpr& r) {
    _obj = r._obj;
    ((_TypeObjExpr*)&r)->_obj = 0;
}
inline TypeObj& TypeObj::operator =(const _TypeObjExpr& r) {
    _BaseObject__release(_obj);
    _obj = r._obj;
    ((_TypeObjExpr*)&r)->_obj = 0;
    return *this;
}
inline TypeObj::TypeObj(const _TypeObjElem& e) {
    _obj = TypeObj::_duplicate(e._obj);
}
inline TypeObj& TypeObj::operator =(const _TypeObjElem& e) {
    _BaseObject__release(_obj);
    _obj = TypeObj::_duplicate(e._obj);
    return *this;
}
inline TypeObj::~TypeObj() {
    _BaseObject__release(_obj);
}
inline _TypeObjExpr TypeObj::_narrow(const BaseObject& r) {
    return _narrow(r._obj);
}
inline _TypeObjExpr TypeObj::_duplicate(const TypeObj& r) {
    return _duplicate(r._obj);
}
inline _TypeObjExpr::~_TypeObjExpr() { }
inline _TypeObjExpr TypeObjType::_ref() { return this; }

class RequestObjType;
typedef RequestObjType* RequestObjRef;
class RequestObj;
class _RequestObjExpr;
class _RequestObjElem;

class RequestObj {
public:
    RequestObjRef _obj;

    RequestObj() { _obj = 0; }
    RequestObj(RequestObjRef p) { _obj = p; }
    RequestObj& operator =(RequestObjRef p);
    RequestObj(const RequestObj&);
    RequestObj& operator =(const RequestObj& r);
    RequestObj(const _RequestObjExpr&);
    RequestObj& operator =(const _RequestObjExpr&);
    RequestObj(const _RequestObjElem&);
    RequestObj& operator =(const _RequestObjElem&);
    ~RequestObj();

    operator RequestObjRef() const { return _obj; }
    RequestObjRef operator ->() { return _obj; }

    static RequestObjRef _narrow(BaseObjectRef p);
    static _RequestObjExpr _narrow(const BaseObject& r);

    static RequestObjRef _duplicate(RequestObjRef obj);
    static _RequestObjExpr _duplicate(const RequestObj& r);
    enum CallStatus {
        initial, ok, unknown_operation, ambiguous_operation, bad_argument_count, 
        bad_argument_type, invoke_failed
    };
};

class _RequestObjExpr : public RequestObj {
public:
    _RequestObjExpr(RequestObjRef p) { _obj = p; }
    _RequestObjExpr(const RequestObj& r) { _obj = r._obj; }
    _RequestObjExpr(const _RequestObjExpr& r) { _obj = r._obj; }
    ~_RequestObjExpr();
};

class _RequestObjElem {
public:
    RequestObjRef _obj;

    _RequestObjElem(RequestObjRef p) { _obj = p; }
    operator RequestObjRef() const { return _obj; }
    RequestObjRef operator ->() { return _obj; }
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
    virtual void put_object(BaseObjectRef obj) = 0;
    _BaseObjectExpr get_object();
    virtual BaseObjectRef _c_get_object() = 0;
    virtual void begin_aggregate() = 0;
    virtual void end_aggregate() = 0;

    _RequestObjExpr _ref();
    virtual TypeObjId _tid();
};

inline RequestObjRef RequestObj::_duplicate(RequestObjRef obj) {
    return (RequestObjRef)_BaseObject__duplicate(obj, 0);
}
inline RequestObj& RequestObj::operator =(RequestObjRef p) {
    _BaseObject__release(_obj);
    _obj = RequestObj::_duplicate(p);
    return *this;
}
inline RequestObj::RequestObj(const RequestObj& r) {
    _obj = RequestObj::_duplicate(r._obj);
}
inline RequestObj& RequestObj::operator =(const RequestObj& r) {
    _BaseObject__release(_obj);
    _obj = RequestObj::_duplicate(r._obj);
    return *this;
}
inline RequestObj::RequestObj(const _RequestObjExpr& r) {
    _obj = r._obj;
    ((_RequestObjExpr*)&r)->_obj = 0;
}
inline RequestObj& RequestObj::operator =(const _RequestObjExpr& r) {
    _BaseObject__release(_obj);
    _obj = r._obj;
    ((_RequestObjExpr*)&r)->_obj = 0;
    return *this;
}
inline RequestObj::RequestObj(const _RequestObjElem& e) {
    _obj = RequestObj::_duplicate(e._obj);
}
inline RequestObj& RequestObj::operator =(const _RequestObjElem& e) {
    _BaseObject__release(_obj);
    _obj = RequestObj::_duplicate(e._obj);
    return *this;
}
inline RequestObj::~RequestObj() {
    _BaseObject__release(_obj);
}
inline _RequestObjExpr RequestObj::_narrow(const BaseObject& r) {
    return _narrow(r._obj);
}
inline _RequestObjExpr RequestObj::_duplicate(const RequestObj& r) {
    return _duplicate(r._obj);
}
inline _RequestObjExpr::~_RequestObjExpr() { }
inline _RequestObjExpr RequestObjType::_ref() { return this; }

#endif
